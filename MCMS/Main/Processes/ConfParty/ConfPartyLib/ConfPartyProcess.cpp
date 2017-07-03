#include "ConfPartyProcess.h"
#include "StringsMaps.h"
#include "Capabilities.h"
#include "PartyMonitor.h"
#include "SecondaryParameters.h"
#include "StatusesGeneral.h"
#include "SipConfPartyDefinitions.h"
#include "IpMfaOpcodes.h"
#include "InterfaceType.h"
#include "ConfPartySharedDefines.h"
#include "InitCommonStrings.h"
#include "CommResDB.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "ConfPartyStatuses.h"
#include "ConfPartyGlobals.h"
#include "TraceStream.h"
#include "AllocateStructs.h"
#include "RecordingLinkDB.h"
#include "ManagerApi.h"
#include "ConfPartyOpcodes.h"
#include "CustomizeDisplaySettingForOngoingConfConfiguration.h"
#include "SysConfigKeys.h"
#include "PrecedenceSettings.h"

extern void              ConfPartyManagerEntryPoint(void* appParam);
extern CCommResDB*       GetpProfilesDB();
extern CCommResDB*       GetpMeetingRoomDB();
extern CRecordingLinkDB* GetRecordingLinkDB();
extern CCommResDB*       GetpConfTemplateDB();
extern const char*       GetSystemCardsModeStr(eSystemCardsMode theMode);


// GLOBAL FUNCTIONS
//--------------------------------------------------------------------------
extern CPrecedenceSettings*		GetpPrecedenceSettingsDB();
CProcessBase* CreateNewProcess()
{
	return new CConfPartyProcess;
}

//--------------------------------------------------------------------------/
const CLobbyApi* GetpLobbyApi()
{
	CConfPartyProcess* pCPProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	return (CLobbyApi*)pCPProcess->GetpLobbyApi();
}

//--------------------------------------------------------------------------
TaskEntryPoint CConfPartyProcess::GetManagerEntryPoint()
{
	return ConfPartyManagerEntryPoint;
}

////////////////////////////////////////////////////////////////////////////
//                        CConfPartyProcess
////////////////////////////////////////////////////////////////////////////
CConfPartyProcess::CConfPartyProcess()
{
	CConfPartyProcess* pCPProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	m_pLobbyApi                                    = NULL;
	m_ipServiceListManager                         = new CIpServiceListManager;
	m_systemCardsBasedMode                         = pCPProcess->GetRmxSystemCardsModeDefault();            // default MPMx based
	m_maxNumberOfOngoingConferences                = MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_AMOS; // there is no difference between MPMx and MPM_PLUS
	m_maxNumberOfPartiesInConf                     = MAX_PARTIES_IN_CONF_MPM_PLUS_BASED;
	m_maxNumberOfVideoPartiesInConf                = MAX_VIDEO_PARTIES_IN_CONF_MPMX_BASED;
	m_IceInitializationStatus                      = eIceStatusOFF;
	m_WebRTCIceInitializationStatus				   = eIceStatusOFF;
	m_IsEnableBWPolicyCheck                        = FALSE;
	m_UcMaxVideoRateAllowed                        = 0;
	m_pCustomizeSettingForOngoingConfConfiguration = new CCustomizeDisplaySettingForOngoingConfConfiguration();
	m_pSharedMemoryMap                             = new CSharedMemMap(CONN_TO_CARD_TABLE_NAME, 1, CONN_TO_CARD_TABLE_SIZE);
	m_bSNMPEnabled                                 = FALSE;
	InitializeLayoutSharedMemory();		// Change Layout Improvement - Layout Shared Memory (CL-SM)
	InitializeIndicationIconSharedMemory();		// Indication Icon Change Improvement - Indication Icon Shared Memory (CL-SM)
}

//--------------------------------------------------------------------------
CConfPartyProcess::~CConfPartyProcess()
{
	POBJDELETE(m_ipServiceListManager);
	POBJDELETE(m_pLobbyApi)
	for (IsdnServicesList::iterator it = m_serviecsList.begin(); it != m_serviecsList.end(); ++it)
	{
		if (*it)
		{
			PDELETE(*it)
			*it = NULL;
		}
	}

	m_serviecsList.clear();

	PDELETE(m_pSharedMemoryMap);
	PDELETE(m_pCustomizeSettingForOngoingConfConfiguration);
	FreeLayoutSharedMemory();	// Change Layout Improvement - Layout Shared Memory (CL-SM)
	FreeIndicationIconSharedMemory();	// Indication Icon Change Improvement - Indication Icon Shared Memory (CL-SM)

}

//--------------------------------------------------------------------------
const CLobbyApi* CConfPartyProcess::GetpLobbyApi()
{
	return m_pLobbyApi;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::SetpLobbyApi(CLobbyApi* pLobbyApi)
{
	m_pLobbyApi = pLobbyApi;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::AddExtraStringsToMap()
{
	CStringsMaps::AddMinMaxItem(_0_TO_AV_MSG_SERVICE_NAME_LENGTH, 0, AV_MSG_SERVICE_NAME_LEN - 1);
	CStringsMaps::AddMinMaxItem(_1_TO_AV_MSG_SERVICE_NAME_LENGTH, 1, AV_MSG_SERVICE_NAME_LEN - 1);

	CStringsMaps::AddMinMaxItem(_0_TO_IVR_MSG_NAME_LENGTH, 0, IVR_MSG_NAME_LEN - 1);

	CStringsMaps::AddMinMaxItem(_0_TO_DTMF_STRING_LENGTH, 0, DTMF_STRING_LEN - 1);

	CStringsMaps::AddItem(DELIMITERS_ENUM, 35, "#");
	CStringsMaps::AddItem(DELIMITERS_ENUM, 42, "*");

	CStringsMaps::AddItem(IVR_REQUEST_PWD_ENUM, REQUEST_PASSWORD, "request_password");
	CStringsMaps::AddItem(IVR_REQUEST_PWD_ENUM, REQUEST_DIGIT, "request_digit");
	CStringsMaps::AddItem(IVR_REQUEST_PWD_ENUM, NO_REQUEST, "none");

	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_OPER_ASSISTANCE_PRIVATE, "private_assistance");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_OPER_ASSISTANCE_PUBLIC, "public_assistance");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_SELF_MUTE, "mute_me");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_SELF_UNMUTE, "unmute_me");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_LOCK_CONF, "lock_conference");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_UNLOCK_CONF, "unlock_conference");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_INC_SELF_VOLUME, "increase_my_volume");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_DEC_SELF_VOLUME, "decrease_my_volume");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_MUTE_ALL_BUT_X, "mute_all_xpt_me");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_UNMUTE_ALL_BUT_X, "cancel_mute_all_xpt_me");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_BLOCK_PARTY, "block_party");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_UNBLOCK_PARTY, "cancel_block_party");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_OPER_ASSISTANCE_PRIVATE_CANCEL, "cancel_private_assistance");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_OPER_ASSISTANCE_PUBLIC_CANCEL, "cancel_public_assistance");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_CONF_TERMINATE, "terminate_conference");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_START_ONHOLD, "start_onhold");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_STOP_ONHOLD, "stop_onhold");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_STARTVOTE, "start_vote");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_STOPVOTE, "stop_vote");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_NEWVOTE, "new_vote");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_CANCELVOTE, "cancel_vote");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_MUTE_INCOMING_PARTIES, "mute_incoming_party");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_UNMUTE_INCOMING_PARTIES, "unmute_incoming_party");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_ADDME_QA, "ask_question");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_REMOVEME_QA, "cancel_my_question");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_MARKNEXT_QA, "next_question");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_UNMARKCUR_QA, "end_current_question");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_CLOSE_QA, "close_all_questions");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_CLEAR_QA, "clear_all_questions");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAY_MENU, "play_menu");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_SECURE_CONF, "secure_conference");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_UNSECURE_CONF, "unsecure_conference");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_CHANGE_PASSWORD, "change_password");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_MUTE_ALL_ON, "mute_all");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_MUTE_ALL_OFF, "unmute_all");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_CHANGE_TO_LEADER, "change_to_chairperson");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_START_INVITE, "dial_to_invitee");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_END_FEATURE, "end_feature");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_MOVE_AND_REDIAL, "admit_and_dial");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_DISCONNECT_AND_REDIAL, "disconnect_and_dial");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_MOVE_AND_END, "admit_and_return");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_DELETE_AND_END, "disconnect_and_return");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_CLEAR_NUMB, "clear_number");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_ENABLE_ROLL_CALL, "enable_roll_call");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_DISABLE_ROLL_CALL, "disable_roll_call");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_ROLL_CALL_REVIEW_NAMES, "roll_call_review_names");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_ROLL_CALL_STOP_REVIEW_NAMES, "roll_call_stop_review_names");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_START_DIALOUT, "start_dial_out");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_START_VC, "start_click_and_view");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_START_VENUS, "venus_controller");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_INC_LISTEN_VOLUME, "increase_listen_volume");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_DEC_LISTEN_VOLUME, "decrease_listen_volume");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_OVERRIDE_MUTE_ALL, "override_mute_all");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_RECORDING_PAUSE, "recording_pause");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_RECORDING_RESUME, "recording_resume");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_RECORDING_STOP, "recording_stop");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_NOISY_LINE_MUTE, "mute_noisy_line");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_NOISY_LINE_UNMUTE, "unmute_noisy_line");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_NOISY_LINE_ADJUST, "adjust_noisy_line");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_NOISY_LINE_DISABLE, "disable_noisy_line");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_NOISY_LINE_HELP_MENU, "noisy_line_help_menu");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAYBACK_PAUSE, "playback_pause");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAYBACK_RESUME, "playback_resume");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAYBACK_SKIP_FW, "playback_skip_fw");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAYBACK_SKIP_BW, "playback_skip_bw");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAYBACK_FW_TO_END, "playback_fw_to_end");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAYBACK_RESTART, "playback_restart");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAYBACK_CLOSE, "playback_close");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_START_RESUME_RECORDING, "start_resume_recording");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_STOP_RECORDING, "stop_recording");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PAUSE_RECORDING, "pause_recording");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_PLAYBACK_MENU, "playback_menu");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_SHOW_PARTICIPANTS, "show_participants");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_REQUEST_TO_SPEAK, "request_to_speak");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_SHOW_GATHERING, "show_gathering");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_START_PCM, "start_pcm");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_INVITE_PARTY, "invite_party");
	CStringsMaps::AddItem(DTMF_OPCODE_ENUM, DTMF_DISCONNECT_INVITED_PARTICIPANT, "disconnect_invited_participant");

	CStringsMaps::AddItem(DTMF_PERMISSION_ENUM, DTMF_USER_ACTION, "everyone");
	CStringsMaps::AddItem(DTMF_PERMISSION_ENUM, DTMF_LEADER_ACTION, "chairperson");

	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_LANG_MENU, "language_menu");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_CONF_PASSWORD, "conference_password");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_PIN_CODE, "pin_code");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_OPER_ASSISTANCE, "operator_assistance");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_WELCOME, "welcome_message");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_CONF_LEADER, "conference_chairperson");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_GENERAL, "general");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_BILLING_CODE, "billing_code");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_INVITE_PARTY, "invite_party");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_ROLL_CALL, "roll_call");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_VIDEO, "video_service");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_NUMERIC_CONFERENCE_ID, "numeric_conference_id");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_MUTE_NOISY_LINE, "noisy_line");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_RECORDING, "recording");
	CStringsMaps::AddItem(IVR_FEATURE_OPCODE_ENUM, IVR_FEATURE_PLAYBACK, "playback");

	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_GET_LANGUAGE, "get_language");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_LANGUAGE_RETRY, "get_language_retry");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_GET_CONFERENCE_PASSWORD, "get_conf_password");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CONFERENCE_PASSWORD_RETRY, "get_conf_password_retry");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_GET_DIGIT, "get_digit");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_GET_PIN_CODE, "get_pin_code");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PIN_CODE_RETRY, "get_pin_code_retry");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE, "wait_for_operator_msg");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_SYSTEM_DISCONNECT_MESSAGE, "system_disconnect");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_WELCOME_MSG, "welcome_msg");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ENTRANCE_MSG, "entrance_msg");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_GET_LEADER_IDENTIFIER, "get_chairman_identifier");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_GET_LEADER_PASSWORD, "get_chairman_password");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_LEADER_PASSWORD_RETRY, "get_chairman_password_retry");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_FIRST_PARTY, "first_party");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_SECURE_ON, "secure_on");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_SECURE_OFF, "secure_off");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_LOCK_ON, "lock_on");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_LOCK_OFF, "lock_off");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PARTY_ENTER, "party_enters");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PARTY_EXIT, "party_quits");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CONF_END, "conference_ends");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_RECORD_START, "record_start");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_RECORD_END, "record_stop");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_RECORD_PAUSE, "record_pause");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_RECORD_NOT_AVAILABLE, "record_not_available");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAYBK_START, "playback_start");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAYBK_END, "playback_stop");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAYBK_PAUSE, "playback_pause");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAYBK_NOT_AVAILABLE, "playback_not_available");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CONF_ON_HOLD, "conference_on_hold");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MENU_LEADER, "menu_chairman");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MENU_SIMPLE, "menu_simple");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_BILLING_NUM, "billing_number");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NUM_PARTIES_IN_CONF_BEGIN, "num_parties_in_conf_begin");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NUM_PARTIES_IN_CONF_END, "num_parties_in_conf_end");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_END_TIME_ALERT, "end_time_alert");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CURRENT_SPEAKER, "current_speaker");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NO_RESOURCES, "no_resources");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ADD_ME_QA, "ask_question");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REMOVE_ME_QA, "cancel_my_question");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CLEAR_QA, "clear_all_questions");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NEXT_QA, "next_question");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_EMPTY_QA, "no_questions");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REQUIRES_LEADER, "chairman_required");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_FIRST_TO_JOIN, "first_to_join");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MENU_INVITE, "invite_menu");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MENU_VOTING, "voting_menu");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MENU_QA, "question_menu");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_INVITE_DISABLE, "invite_disable");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_INVITE_CALL, "invite_call");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_INVITE_NO_RESOURCE, "no_resources_for_invitation");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_INVITE_NO_PERMISSION, "no_permissions_for_invitation");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ILLEGAL_NO, "illegal_number");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CONF_LOCK, "lock_conference");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_OPERATOR_ASK_ATTENTION, "operator_asks_for_attension");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CHANGE_PWDS_MENU, "change_password_menu");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CHANGE_PWDS_CONF, "change_conference_password");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CHANGE_PWDS_LEADER, "change_chairman_password");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CHANGE_PWD_CONFIRM, "change_password_confirm");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CHANGE_PWD_INVALID, "change_password_invalid");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CHANGE_PWD_OK, "change_password_ok");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MUTE_ALL_ON, "mute_all_on");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MUTE_ALL_OFF, "mute_all_off");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_CHAIR_DROPPED, "chair_dropped");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_SELF_MUTE, "self_mute");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_SELF_UNMUTE, "self_unmute");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MENU_LEADER_2, "menu_chairman_2");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MENU_SIMPLE_2, "menu_simple_2");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ROLLCALL_REC, "roll_call_record");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ROLLCALL_VERIFY_REC, "roll_call_verify_record");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ROLLCALL_CONFIRM_REC, "roll_call_confirm_record");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ROLLCALL_ENTER, "roll_call_enter");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ROLLCALL_EXIT, "roll_call_exit");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ROLLCALL_BEGIN_OF_NAME_REVIEW, "roll_call_name_preview_start");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ROLLCALL_END_OF_NAME_REVIEW, "roll_call_name_preview_end");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_MAX_PARTICIPANTS, "max_parties");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_GET_NUMERIC_ID, "get_numeric_id");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NUMERIC_ID_RETRY, "get_numeric_id_retry");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NOISY_LINE_HELP_MENU, "noisy_line_help_menu");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NOISY_LINE_MUTE, "noisy_line_mute");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NOISY_LINE_ADJUST, "noisy_line_adjust");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NOISY_LINE_DISABLE, "noisy_line_disable");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NOISY_LINE_UNMUTE, "noisy_line_unmute");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NOISY_LINE_UNMUTE_MESSAGE, "noisy_line_unmute_message");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAY_NOISY_LINE_MESSAGE, "noisy_line_play_message");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_RECORDING_IN_PROGRESS, "recording_in_progress");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REC_STARTED, "rec_started");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REC_STOPPED, "rec_stopped");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REC_PAUSED, "rec_paused");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REC_RESUMED, "rec_resumed");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REC_ILLEGAL_ACCOUNT, "rec_illegal_account");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REC_UNAUTHORIZE, "rec_unauthorize");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REC_USER_OVERFLOW, "rec_user_overflow");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REC_GENERAL, "rec_general");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLCK_SESSIONID, "playback_session_id");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLCK_SESSIONID_ERROR, "playback_session_id_error");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLCK_SESSION_END, "playback_session_end");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLCK_MENU, "playback_menu");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_RECORDING_FAILED, "recording_failed");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLCK_MENU_NOTIFY, "playback_menu_notify");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLCK_PAUSED, "playback_paused");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLCK_RESUME, "playback_resume");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ENTER_DEST_NUM, "enter_destination_id");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_ILLEGAL_DEST_NUM, "incorrect_destination_id");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAY_DIAL_TONE, "dial_tone");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAY_RINGING_TONE, "ringing_tone");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_NO_VIDEO_RESOURCES, "No_Video_Resources_Audio_Only");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_BLIP_ON_CASCADE_LINK, "blip_on_cascade_link");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_INVITE_PARTY, "invite_party");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_REINVITE_PARTY, "reinvite_party");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAY_BUSY_MSG, "disconnect_busy");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAY_NOANSWER_MSG, "disconnect_no_answer");
	CStringsMaps::AddItem(IVR_EVENT_TYPE_ENUM, IVR_EVENT_PLAY_WRONG_NUMBER_MSG, "disconnect_wrong_number");

	CStringsMaps::AddItem(CHANGE_STATUS_ENUM, CONF_NOT_CHANGED, "none");
	CStringsMaps::AddItem(CHANGE_STATUS_ENUM, CONF_FAST_PLUS_SLOW_INFO, "update");
	CStringsMaps::AddItem(CHANGE_STATUS_ENUM, CONF_COMPLETE_INFO, "new");

	CStringsMaps::AddItem(IVR_EXTERNAL_SERVER_ENUM, IVR_EXTERNAL_DB_NONE, "none");
	CStringsMaps::AddItem(IVR_EXTERNAL_SERVER_ENUM, IVR_EXTERNAL_DB_NUMERIC_ID, "numeric_id");
	CStringsMaps::AddItem(IVR_EXTERNAL_SERVER_ENUM, IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD, "party_password");
	CStringsMaps::AddItem(IVR_EXTERNAL_SERVER_ENUM, IVR_EXTERNAL_DB_CHAIR_PASSWORD, "chairman_password");

	CStringsMaps::AddItem(PARTY_CHANGE_STATE_ENUM, PARTY_NOT_CHANGED, "party_not_changed");
	CStringsMaps::AddItem(PARTY_CHANGE_STATE_ENUM, PARTY_COMPLETE_INFO, "party_full_info");
	CStringsMaps::AddItem(PARTY_CHANGE_STATE_ENUM, PARTY_FAST_INFO, "party_fast_info");
	CStringsMaps::AddItem(PARTY_CHANGE_STATE_ENUM, PARTY_FAST_PLUS_SLOW_INFO, "party_fast_plus_slow_info");
	CStringsMaps::AddItem(PARTY_CHANGE_STATE_ENUM, PARTY_NEW_INFO, "party_new_info");
	CStringsMaps::AddItem(PARTY_CHANGE_STATE_ENUM, PARTY_FAST_PLUS_SLOW1_INFO, "party_fast_plus_slow1_info");
	CStringsMaps::AddItem(PARTY_CHANGE_STATE_ENUM, PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO, "party_fast_plus_slow1_plus_slow_info");

	// problems in the stream of IP party
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, BitRateProblem, "bit_rate");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, FractionLossProblem, "fraction_loss");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, JitterProblem, "jitter");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, LatencyProblem, "latency");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, ProtocolProblem, "protocol");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, ResolutionProblem, "resolution");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, FrameRateProblem, "frame_rate");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, AnnexesProblem, "annexes");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, FramePerPacketProblem, "frame_per_pack");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, ActualLossAccProblem, "actual_loss_accumulated");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, ActualLossInterProblem, "actual_loss_interval");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, OutOfOrderAccProblem, "out_of_order_accumulated");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, OutOfOrderInterProblem, "out_of_order_interval");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, FragmentedAccProblem, "fragemented_accumulated");
	CStringsMaps::AddItem(MAP_PROBLEM_ENUM, FragmentedInterProblem, "fragemented_interval");

	// IP channel algorithms
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG711Alaw64kCapCode, "g711Alaw64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG711Alaw56kCapCode, "g711Alaw56k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG711Ulaw64kCapCode, "g711Ulaw64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG711Ulaw56kCapCode, "g711Ulaw56k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG722_64kCapCode, "g722_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG722_56kCapCode, "g722_56k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG722_48kCapCode, "g722_48k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG722Stereo_128kCapCode, "g722Stereo_128k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG728CapCode, "g728");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG729CapCode, "g729");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG729AnnexACapCode, "g729AnnexA");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG729wAnnexBCapCode, "g729wAnnexB");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG729AnnexAwAnnexBCapCode, "g729AnnexAwAnnexB");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7231CapCode, "g7231");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eIS11172AudioCapCode, "IS11172Audio");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eIS13818CapCode, "IS13818");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7231AnnexCapCode, "G7231Annex");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7221_32kCapCode, "g7221_32k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7221_24kCapCode, "g7221_24k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7221_16kCapCode, "g7221_16k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren14_48kCapCode, "siren14_48k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren14_32kCapCode, "siren14_32k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren14_24kCapCode, "siren14_24k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7221C_48kCapCode, "g7221C_48k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7221C_32kCapCode, "g7221C_32k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7221C_24kCapCode, "g7221C_24k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG7221C_CapCode, "g7221C");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren14Stereo_48kCapCode, "siren14S_48k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren14Stereo_56kCapCode, "siren14S_56k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren14Stereo_64kCapCode, "siren14S_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren14Stereo_96kCapCode, "siren14S_96k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren22Stereo_128kCapCode, "siren22S_128k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren22Stereo_96kCapCode, "siren22S_96k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren22Stereo_64kCapCode, "siren22S_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren22_64kCapCode, "siren22_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren22_48kCapCode, "siren22_48k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren22_32kCapCode, "siren22_32k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPR_32kCapCode, "sirenLPR_32k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPR_48kCapCode, "sirenLPR_48k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPR_64kCapCode, "sirenLPR_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPRStereo_64kCapCode, "sirenLPRS_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPRStereo_96kCapCode, "sirenLPRS_96k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPRStereo_128kCapCode, "sirenLPRS_128k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG719_64kCapCode, "g719_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG719_48kCapCode, "g719_48k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG719_32kCapCode, "g719_32k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG719Stereo_128kCapCode, "g719Stereo_128k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG719Stereo_96kCapCode, "g719Stereo_96k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG719Stereo_64kCapCode, "g719Stereo_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eAAC_LDCapCode, "AAC_LD"); // TIP

	CStringsMaps::AddItem(CAP_CODE_ENUM, eH261CapCode, "h261");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eH262CapCode, "h262");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eH263CapCode, "h263");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eH264CapCode, "h264");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eVP8CapCode,  "vp8");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eH26LCapCode, "h26L");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eRtvCapCode, "rtv");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eIS11172VideoCapCode, "IS11172Video");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eGenericVideoCapCode, "genericVideo");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eT120DataCapCode, "t120Data");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eNonStandardCapCode, "nonStandard");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eGenericCapCode, "generic");
	CStringsMaps::AddItem(CAP_CODE_ENUM, ePeopleContentCapCode, "PeopleContent");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eRoleLabelCapCode, "RoleLabel");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eChairControlCapCode, "ChairControl");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eEncryptionCapCode, "Encryption");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eRfc2833DtmfCapCode, "rfc2833Dtmf");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eAnnexQCapCode, "annexQ");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eRvFeccCapCode, "rad_vision_fecc");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eH239ControlCapCode, "H239ControlCapCode");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eDynamicPTRCapCode, "DynamicPTRepCapCode"); /* Dynamic Payload type replacement */
	CStringsMaps::AddItem(CAP_CODE_ENUM, eDBC2CapCode, "DBC2CapCode");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG719_96kCapCode, "g719_96k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eG719_128kCapCode, "g719_128k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPR_Scalable_32kCapCode, "sirenLPR_scalable_32k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPR_Scalable_48kCapCode, "sirenLPR_scalable_48k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPR_Scalable_64kCapCode, "sirenLPR_scalable_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPRStereo_Scalable_64kCapCode, "sirenLPR_stereo_scalable_64k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPRStereo_Scalable_96kCapCode, "sirenLPR_stereo_scalable_96k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSirenLPRStereo_Scalable_128kCapCode, "sirenLPR_stereo_scalable_128k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eiLBC_13kCapCode, "iLBC_13k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eiLBC_15kCapCode, "iLBC_15k");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eOpus_CapCode, "opus");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eOpusStereo_CapCode, "opus");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSvcCapCode, "SVC");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eLPRCapCode, "LPR");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eIcePwdCapCode, "Ice_pwd");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eIceUfragCapCode, "Ice_ufrag");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eIceCandidateCapCode, "Ice_candidate");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eIceRemoteCandidateCapCode, "Ice_remote_candidate");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eRtcpCapCode, "Rtcp");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSdesCapCode, "Sdes");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eBFCPCapCode, "BFCP");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eMCCFCapCode, "MCCF");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eMsSvcCapCode,"ms_svc");
	CStringsMaps::AddItem(CAP_CODE_ENUM, eFECCapCode,  "FEC");   //LYNC2013_FEC_RED
	CStringsMaps::AddItem(CAP_CODE_ENUM, eREDCapCode,  "RED");   //LYNC2013_FEC_RED
	CStringsMaps::AddItem(CAP_CODE_ENUM, eSiren7_16kCapCode, "siren7_16k");

	CStringsMaps::AddItem(CAP_CODE_ENUM, eUnknownAlgorithemCapCode, "UnknownAlgorithm");

	// IP channels Types
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, H225, "h225");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, H245, "h245");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, AUDIO_IN, "audio_in");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, AUDIO_OUT, "audio_out");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, VIDEO_IN, "video_in");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, VIDEO_OUT, "video_out");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, VIDEO_CONT_IN, "video_content_in");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, VIDEO_CONT_OUT, "video_content_out");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, FECC_IN, "fecc_in");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, FECC_OUT, "fecc_out");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, BFCP, "bfcp");
	CStringsMaps::AddItem(IP_CHANNEL_TYPE_ENUM, BFCP_UDP, "bfcp_udp");

	// IP annexes (H263 video protocol)
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexB, "annexB");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexD, "annexD");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexE, "annexE");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexF, "annexF");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexG, "annexG");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexH, "annexH");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexI, "annexI");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexJ, "annexJ");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexK, "annexK");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexL, "annexL");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexN, "annexN");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexO, "annexO");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexP, "annexP");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexQ, "annexQ");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexR, "annexR");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexS, "annexS");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexT, "annexT");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexU, "annexU");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexV, "annexV");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexW, "annexW");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeAnnexI_NS, "AnnexI_NS");
	CStringsMaps::AddItem(ANNEXES_ENUM, typeCustomPic, "custom_picture_format");

	// IP video resolution
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kUnknownFormat, "unknown");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kQCif, "qcif");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kCif, "cif");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k4Cif, "4cif");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k16Cif, "16cif");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kVGA, "vga");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kNTSC, "ntsc");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kSVGA, "svga");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kXGA, "xga");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kSIF, "sif");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, kQVGA, "qvga");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k2Cif, "2cif");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k2SIF, "2sif");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k4SIF, "4sif");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k525SD, "525sd");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k625SD, "625sd");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k720p, "720p");
	CStringsMaps::AddItem(VIDEO_RESOLUTION_ENUM, k1080p, "1080p");

	// IP secondary cause
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, UnKnown, "unknown");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, CapCode, "cap_code");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, Resolution, "resolution");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, FrameRate, "frame_rate");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, LineRate, "line_rate");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, Annexes, "annexes");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, Lacks263Plus, "lacks_263_plus");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, RoleLabel, "role_label");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, PayloadType, "payload_type");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, Level, "level");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, MBPS, "mbps");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, FS, "fs");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, DPB, "dpb");
	CStringsMaps::AddItem(SECONDARY_PROBLEM_ENUM, BrAndCpb, "br_cpb");

	// H.323 - GateKeeper Call State
	CStringsMaps::AddItem(GK_STATE_ENUM, eGKCallNone, "none");
	CStringsMaps::AddItem(GK_STATE_ENUM, eGKAdmission, "arq");
	CStringsMaps::AddItem(GK_STATE_ENUM, eGKAdmitted, "admitted");
	CStringsMaps::AddItem(GK_STATE_ENUM, eGKDisengage, "drq");


	// SIP conference limitation
	CStringsMaps::AddItem(SIP_CONFERENCING_LIMITATION_ENUM, kNotAnAdvancedVideoConference, "unknown");
	CStringsMaps::AddItem(SIP_CONFERENCING_LIMITATION_ENUM, kH264VswFixed, "vsw_set_to_h264_fixed");
	CStringsMaps::AddItem(SIP_CONFERENCING_LIMITATION_ENUM, kEpcVswFixed, "vsw_set_to_epc_fixed");
	CStringsMaps::AddItem(SIP_CONFERENCING_LIMITATION_ENUM, kAutoVsw, "vsw_set_to_auto");

	CStringsMaps::AddItem(ICE_CONNECTION_TYPE_ENUM, kNone, "none");
	CStringsMaps::AddItem(ICE_CONNECTION_TYPE_ENUM, kLocal, "local");
	CStringsMaps::AddItem(ICE_CONNECTION_TYPE_ENUM, kRelay, "relay");
	CStringsMaps::AddItem(ICE_CONNECTION_TYPE_ENUM, kFirewall, "firewall");

	CStringsMaps::AddItem(INTERFACE_TYPE_ENUM, PSTN_INTERFACE_TYPE, "pstn");
	CStringsMaps::AddItem(INTERFACE_TYPE_ENUM, ISDN_INTERFACE_TYPE, "isdn");
	CStringsMaps::AddItem(INTERFACE_TYPE_ENUM, SIP_INTERFACE_TYPE, "sip");
	CStringsMaps::AddItem(INTERFACE_TYPE_ENUM, H323_INTERFACE_TYPE, "h323");
	CStringsMaps::AddItem(INTERFACE_TYPE_ENUM, NONE_INTERFACE_TYPE, "off");
}

//--------------------------------------------------------------------------
void CConfPartyProcess::Clear()
{
	m_ipServiceListManager->Clear();
}

//--------------------------------------------------------------------------
WORD CConfPartyProcess::numberOfIpServices()
{
	return m_ipServiceListManager->numberOfIpServices();
}

//--------------------------------------------------------------------------
STATUS CConfPartyProcess::insertIpService(CConfIpParameters* pConfIpParameters)
{
	STATUS insertStatus = STATUS_OK;
	insertStatus = m_ipServiceListManager->insertIpService(pConfIpParameters);
	return insertStatus;
}

//--------------------------------------------------------------------------
STATUS CConfPartyProcess::updateIpService(CConfIpParameters* pConfIpParameters)
{
	STATUS insertStatus = STATUS_OK;
	insertStatus = m_ipServiceListManager->updateIpService(pConfIpParameters);
	return insertStatus;
}

//--------------------------------------------------------------------------
STATUS CConfPartyProcess::removeIpService(DWORD serviceID)
{
	CConfIpParameters* pConfIpParameters = m_ipServiceListManager->removeIpService(serviceID);
	if (CPObject::IsValidPObjectPtr(pConfIpParameters))
	{
		POBJDELETE(pConfIpParameters);
		return STATUS_OK;
	}
	else
	{
		TRACEINTO << "Service not found, ServiceId:" << serviceID;
		return STATUS_FAIL;
	}
}

//--------------------------------------------------------------------------
CConfIpParameters* CConfPartyProcess::FindIpService(DWORD serviceID)
{
	CConfIpParameters* pConfIpParameters = m_ipServiceListManager->FindIpService(serviceID);
	if (!CPObject::IsValidPObjectPtr(pConfIpParameters))
		TRACEINTO << "Service not found, ServiceId:" << serviceID;

	return pConfIpParameters;
}

//--------------------------------------------------------------------------
CConfIpParameters* CConfPartyProcess::FindServiceByName(const char* serviceName)
{
	CConfIpParameters* pConfIpParameters = m_ipServiceListManager->FindServiceByName(serviceName);
	return pConfIpParameters;
}

//--------------------------------------------------------------------------
CIpServiceListManager* CConfPartyProcess::GetIpServiceListManager()
{
	return m_ipServiceListManager;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::TearDownProcess()
{
	CCommResDB*       profilesDB       = GetpProfilesDB();
	CCommResDB*       mrDB             = GetpMeetingRoomDB();
	CRecordingLinkDB* pRecordingLinkDB = GetRecordingLinkDB();
	CCommResDB*       pConfTemplateDB  = GetpConfTemplateDB();

	CPrecedenceSettings* pPrecedenceSettingsDB = GetpPrecedenceSettingsDB();
	POBJDELETE(profilesDB);
	POBJDELETE(mrDB);
	POBJDELETE(pRecordingLinkDB);
	POBJDELETE(pConfTemplateDB);
	POBJDELETE(pPrecedenceSettingsDB);
	CProcessBase::TearDownProcess();
}

//--------------------------------------------------------------------------
WORD CConfPartyProcess::numberOfIsdnServices()
{
	return m_serviecsList.size();
}

//--------------------------------------------------------------------------
void CConfPartyProcess::AddIsdnService(RTM_ISDN_PARAMS_MCMS_S* pRtmIsdnServiceParams)
{
	RTM_ISDN_PARAMS_MCMS_S* pNewRtmIsdnServiceParams = new RTM_ISDN_PARAMS_MCMS_S;
	WORD                    structLen                = sizeof(RTM_ISDN_PARAMS_MCMS_S);
	memcpy(pNewRtmIsdnServiceParams, pRtmIsdnServiceParams, structLen);
	m_serviecsList.push_back(pNewRtmIsdnServiceParams);
}

//--------------------------------------------------------------------------
void CConfPartyProcess::DeleteIsdnService(const std::string& serviceName)
{
	RTM_ISDN_PARAMS_MCMS_S* pDeletedRtmIsdnServiceParams = RemoveIsdnService(serviceName);

	if (pDeletedRtmIsdnServiceParams != NULL)
	{
		POBJDELETE(pDeletedRtmIsdnServiceParams);
	}
	else
	{
		PASSERTSTREAM(1, "Can not find the service in the list, ServiceName:" << serviceName);
	}
}

//--------------------------------------------------------------------------
RTM_ISDN_PARAMS_MCMS_S* CConfPartyProcess::GetIsdnService(std::string serviceName)
{
	// No services in the system
	if (0 == m_serviecsList.size())
		return NULL;

	if (serviceName == "")
		return m_serviecsList.front(); // return the default service

	IsdnServicesList::iterator serviceIt = m_serviecsList.begin();
	for (; serviceIt != m_serviecsList.end(); ++serviceIt)
	{
		char* pServiceName = (char*)((*serviceIt)->serviceName);
		if (*serviceIt && serviceName == pServiceName)
			return *serviceIt; // Found the service in the list

	}

	// Service name was not found
	PASSERTSTREAM(1, "Can not find the service in the list, ServiceName:" << serviceName);
	return NULL;
}

//--------------------------------------------------------------------------
RTM_ISDN_PARAMS_MCMS_S* CConfPartyProcess::RemoveIsdnService(std::string serviceName)
{
	RTM_ISDN_PARAMS_MCMS_S* pDeletedRtmIsdnServiceParams = NULL;
	// No services in the system
	if (0 == m_serviecsList.size())
		return NULL;

	if (serviceName == "")
		return m_serviecsList.front(); // return the default service

	IsdnServicesList::iterator serviceIt = m_serviecsList.begin();
	for (; serviceIt != m_serviecsList.end(); ++serviceIt)
	{
		char* pServiceName = (char*)((*serviceIt)->serviceName);
		if (*serviceIt && serviceName == pServiceName)
			break; // Found the service in the list
	}

	pDeletedRtmIsdnServiceParams = *serviceIt;
	if (pDeletedRtmIsdnServiceParams != NULL)
	{
		m_serviecsList.erase(serviceIt);
	}
	else
	{
		// Service name was not found
		PASSERTSTREAM(1, "Can not find the service in the list, ServiceName:" << serviceName);
	}

	return pDeletedRtmIsdnServiceParams;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::SetIsdnServiceAsDefault(const std::string& serviceName)
{
	// Put the default service in the head of the vect
	IsdnServicesList::iterator defaultServiceIt = m_serviecsList.begin();
	for (; defaultServiceIt != m_serviecsList.end(); ++defaultServiceIt)
	{
		char* pServiceName = (char*)((*defaultServiceIt)->serviceName);
		if (*defaultServiceIt && serviceName == pServiceName)
			break; // Found the service in the list

	}

	// Make sure you found the service name in the list in the list
	if (defaultServiceIt == m_serviecsList.end())
	{
		PASSERTSTREAM(1, "Can not find default service, ServiceName:" << serviceName);
		return;
	}

	RTM_ISDN_PARAMS_MCMS_S* pDefaultService = *defaultServiceIt;
	// Remove the default service from it's original place
	m_serviecsList.erase(defaultServiceIt);

	// Add it to the front of the list
	m_serviecsList.push_front(pDefaultService);
}

//--------------------------------------------------------------------------
void CConfPartyProcess::SetSystemCardsBasedMode(const eSystemCardsMode systemCardsBasedMode)
{
	if (m_systemCardsBasedMode != systemCardsBasedMode && IsValidSystemCardsBasedMode(systemCardsBasedMode))
	{
		m_systemCardsBasedMode = systemCardsBasedMode;
// UpdateSystemCapacityLimitsAccordingToSystemMode(); - VNGR-21090
	}
}

//--------------------------------------------------------------------------
eSystemCardsMode CConfPartyProcess::GetSystemCardsBasedMode()
{
	return m_systemCardsBasedMode;
}

//--------------------------------------------------------------------------
// System Capacity Limits
void CConfPartyProcess::UpdateSystemCapacityLimitsAccordingToSystemMode()
{
	switch (m_systemCardsBasedMode)
	{
		case (eSystemCardsMode_mpm):
		{
			m_maxNumberOfOngoingConferences = MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_BASED_RMX2000; // no mpm in Amos
			m_maxNumberOfPartiesInConf      = MAX_PARTIES_IN_CONF_MPM_BASED;
			m_maxNumberOfVideoPartiesInConf = MAX_VIDEO_PARTIES_IN_CONF_MPM_BASED;
		}
		break;

		case (eSystemCardsMode_mpm_plus):
		{
			m_maxNumberOfOngoingConferences = MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_AMOS;
			m_maxNumberOfPartiesInConf      = MAX_PARTIES_IN_CONF_MPM_PLUS_BASED;
			m_maxNumberOfVideoPartiesInConf = (::GetIsCOPdongleSysMode()) ? MAX_VIDEO_PARTIES_IN_COP_CONF_MPM_PLUS_BASED : MAX_VIDEO_PARTIES_IN_CONF_MPM_PLUS_BASED;
		}
		break;

		case (eSystemCardsMode_breeze):
		case (eSystemCardsMode_mpmrx):
		{
			m_maxNumberOfOngoingConferences = MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_AMOS;
			m_maxNumberOfPartiesInConf      = (eProductTypeRMX4000 == GetProductType()) ? MAX_PARTIES_IN_CONF_MPM_PLUS_BASED : MAX_PARTIES_IN_CONF_MPMX_BASED;

			if (::GetIsCOPdongleSysMode())
			{
				m_maxNumberOfVideoPartiesInConf = MAX_VIDEO_PARTIES_IN_COP_CONF_MPMX_BASED;
			}
			else if (GetProductType() == eProductTypeSoftMCUMfw)
			{
				m_maxNumberOfPartiesInConf = MAX_PARTIES_IN_CONF_SOFT_MCU_MFW_BASED;
				m_maxNumberOfVideoPartiesInConf = MAX_VIDEO_PARTIES_IN_CONF_SOFT_MCU_MFW_BASED;
			}
			else if (GetProductType() == eProductTypeNinja)
			{
				m_maxNumberOfVideoPartiesInConf = MAX_VIDEO_PARTIES_IN_CONF_NINJA;
			}
			else if (m_systemCardsBasedMode == eSystemCardsMode_mpmrx)
			{
				m_maxNumberOfVideoPartiesInConf = MAX_VIDEO_PARTIES_IN_CONF_MPMRX_BASED;
			}else
			{
				DWORD maxPartiesInConfMpmx = MAX_VIDEO_PARTIES_IN_CONF_MPMX_BASED;
				CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(CFG_KEY_MPMX_MAX_NUMBER_OF_VIDEO_PARTIES_IN_CONF, maxPartiesInConfMpmx);
				m_maxNumberOfVideoPartiesInConf = maxPartiesInConfMpmx;
			}
		}
		break;

		default:
		{
			PASSERTSTREAM(1, "Invalid system cards based mode, Mode:" << m_systemCardsBasedMode);
		}
		break;
	} // switch
}

//--------------------------------------------------------------------------
BOOL CConfPartyProcess::IsValidSystemCardsBasedMode(const eSystemCardsMode systemCardsBasedMode)
{
	BOOL isValidSystemCardsBasedMode = FALSE;
	switch (systemCardsBasedMode)
	{
		case (eSystemCardsMode_breeze):
		case (eSystemCardsMode_mpmrx):
		{
			isValidSystemCardsBasedMode = TRUE;
		}
		break;

		default:
		{
			PASSERTSTREAM(1, "Invalid system cards based mode, Mode:" << ::GetSystemCardsModeStr(systemCardsBasedMode));
		}
		break;
	} // switch

	return isValidSystemCardsBasedMode;
}

//--------------------------------------------------------------------------
WORD CConfPartyProcess::GetMaxNumberOfOngoingConferences()
{
	return m_maxNumberOfOngoingConferences;
}

//--------------------------------------------------------------------------
WORD CConfPartyProcess::GetMaxNumberOfPartiesInConf()
{
	return m_maxNumberOfPartiesInConf;
}

//--------------------------------------------------------------------------
WORD CConfPartyProcess::GetMaxNumberOfVideoPartiesInConf()
{
	return m_maxNumberOfVideoPartiesInConf;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::KillAllConfAndPartyTasks()
{
	TRACEINTO << "";

	LockSemaphore(m_TasksSemaphoreId);
	{
		std::vector<CTaskApp*>::iterator itr = m_pTasks->begin();
		while (itr != m_pTasks->end())
		{
			if ((*itr)->IsTypeOf("CConf"))
			{
				CConfApi* pTaskApi = new CConfApi;
				pTaskApi->CreateOnlyApi((*itr)->GetRcvMbx());
				pTaskApi->ForceKill();
				POBJDELETE(pTaskApi);
			}
			else if ((*itr)->IsTypeOf("CParty"))
			{
				CPartyApi* pTaskApi = new CPartyApi;
				pTaskApi->CreateOnlyApi((*itr)->GetRcvMbx());
				pTaskApi->ForceKill();
				POBJDELETE(pTaskApi);
			}

			itr++;
		}
	}
	UnlockSemaphore(m_TasksSemaphoreId);
	SystemSleep(500);
}

//--------------------------------------------------------------------------
void CConfPartyProcess::AddExtraStatusesStrings()
{ }

//--------------------------------------------------------------------------
int CConfPartyProcess::GetProcessAddressSpace()
{
	DWORD dwAddressSpaceSize        = 0;
	int   iAddressSpaceSizeToReturn = (int)dwAddressSpaceSize * 1024;
	TRACEINTO << "ProcessAddressSpace:" << iAddressSpaceSizeToReturn;

	return iAddressSpaceSizeToReturn;
}

//--------------------------------------------------------------------------
bool CConfPartyProcess::IsFailoverBlockTransaction_SlaveMode(string sAction)
{
	return true; // all ConfParty SET transactions are blocked in Slave mode
}

//--------------------------------------------------------------------------
CSharedMemMap* CConfPartyProcess::GetSharedMemoryMap()
{
	return m_pSharedMemoryMap;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::InitializeEncryptionKeysSharedMemory()
{
	m_encyptedSharedMemoryTables = new EncyptedSharedMemoryTables();
}

//--------------------------------------------------------------------------
EncyptedSharedMemoryTables* CConfPartyProcess::GetEncryptionKeysSharedMemory()
{
	PASSERTMSG(!m_encyptedSharedMemoryTables, "CConfPartyProcess::GetEncryptionKeysSharedMemory is NULL");

	return m_encyptedSharedMemoryTables;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::FreeEncryptionKeysSharedMemory()
{
	POBJDELETE(m_encyptedSharedMemoryTables);
}

// // Change Layout Improvement - Layout Shared Memory (CL-SM)
//--------------------------------------------------------------------------
void CConfPartyProcess::InitializeLayoutSharedMemory()
{
	m_layoutSharedMemoryMap = new CLayoutSharedMemoryMap();
	TRACEINTO << "Create Layout Shared Memory";
}

//--------------------------------------------------------------------------
CLayoutSharedMemoryMap* CConfPartyProcess::GetLayoutSharedMemory()
{
	PASSERTMSG(!m_layoutSharedMemoryMap, "CConfPartyProcess::GetLayoutSharedMemory is NULL");

	return m_layoutSharedMemoryMap;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::FreeLayoutSharedMemory()
{
	POBJDELETE(m_layoutSharedMemoryMap);
}

//--------------------------------------------------------------------------
void CConfPartyProcess::InitializeIndicationIconSharedMemory()
{
	m_indicationIconSharedMemoryMap = new CIndicationIconSharedMemoryMap();
	TRACEINTO << "Create Indication Icon Shared Memory";
}

//--------------------------------------------------------------------------
CIndicationIconSharedMemoryMap* CConfPartyProcess::GetIndicationIconSharedMemory()
{
	PASSERTMSG(!m_indicationIconSharedMemoryMap, "CConfPartyProcess::GetIndicationIconSharedMemory is NULL");

	return m_indicationIconSharedMemoryMap;
}

//--------------------------------------------------------------------------
void CConfPartyProcess::FreeIndicationIconSharedMemory()
{
	POBJDELETE(m_indicationIconSharedMemoryMap);
}


//--------------------------------------------------------------------------
void CConfPartyProcess::CreateTask(const char* taskName)
{
	if (!IsValidPObjectPtr(m_pManagerApi) || !taskName)
		return;

	if (!strcmp(taskName, "ConfPartyAssistMng"))
	{
		GetManagerApi()->SendMsg(NULL, START_MNG_ASSIST_TASK);
	}
}

//--------------------------------------------------------------------------
CCustomizeDisplaySettingForOngoingConfConfiguration* CConfPartyProcess::GetCustomizeDisplaySettingForOngoingConfConfiguration()
{
	return m_pCustomizeSettingForOngoingConfConfiguration;
}

//--------------------------------------------------------------------------
BOOL CConfPartyProcess::IsFeatureSupportedByHardware(const eFeatureName featureName) const
{
	return m_systemFeatures.IsFeatureSupportedByHardware(featureName, m_systemCardsBasedMode, GetProductType());
}
