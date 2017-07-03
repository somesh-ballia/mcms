#include "McmsPCMManager.h"
#include "McmsPCMManagerInitParams.h"
#include "Bridge.h"
#include "PCMHardwareInterface.h"
#include "Segment.h"
#include "psosxml.h"
#include "PartyApi.h"
#include "VideoBridgeInterface.h"
#include "ConfAppMngrInterface.h"
#include "OpcodesMcmsPCM.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "AllocateStructs.h"
#include "OpcodesMcmsInternal.h"
#include "ManagerApi.h"
#include "MplMcmsStructs.h"
#include "PcmIndications.h"
#include "PcmCommands.h"
#include "PcmConfirms.h"
#include "ConfIpParameters.h"
#include "IpServiceListManager.h"
#include "Party.h"
#include "MessageOverlayInfo.h"
#include "AddressBook.h"
#include "ConfPartyProcess.h"
#include "MoveInfo.h"
#include "SysConfigKeys.h"

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);
extern const char* feccKeyToString(feccKeyEnum key);
extern CIpServiceListManager* GetIpServiceListMngr();
extern CCommResDB* GetpMeetingRoomDB();
extern BYTE GetTmpPcmDebugFlag();

PBEGIN_MESSAGE_MAP(CMcmsPCMManager)

	ONEVENT(ACK_IND,                             ANYCASE,     CMcmsPCMManager::OnAck)

	ONEVENT(CHAIRPERSON_ENTERED,                 IDLE,        CMcmsPCMManager::OnChairpersonEnteredIdle)
	ONEVENT(CHAIRPERSON_ENTERED,                 HIDING_MENU, CMcmsPCMManager::OnChairpersonEnteredHidingMenu)

	ONEVENT(CHAIRPERSON_LEFT,                    IDLE,        CMcmsPCMManager::OnChairpersonLeftIdle)
	ONEVENT(CHAIRPERSON_LEFT,                    MENU_OFF,    CMcmsPCMManager::OnChairpersonLeftMenuOff)
	ONEVENT(CHAIRPERSON_LEFT,                    MENU_SETUP,  CMcmsPCMManager::OnChairpersonLeftMenuSetup)
	ONEVENT(CHAIRPERSON_LEFT,                    MENU_ON,     CMcmsPCMManager::OnChairpersonLeftMenuOn)
	ONEVENT(CHAIRPERSON_LEFT,                    HIDING_MENU, CMcmsPCMManager::OnChairpersonLeftHidingMenu)

	ONEVENT(CHAIRPERSON_START_MOVE_FROM_CONF,    ANYCASE,     CMcmsPCMManager::OnChairpersonStartMoveFromConfAnycase)

	ONEVENT(FECC_KEY_IND,                        IDLE,        CMcmsPCMManager::OnFeccKeyIdle)
	ONEVENT(FECC_KEY_IND,                        MENU_OFF,    CMcmsPCMManager::OnFeccKeyMenuOff)
	ONEVENT(FECC_KEY_IND,                        MENU_SETUP,  CMcmsPCMManager::OnFeccKeyMenuSetup)
	ONEVENT(FECC_KEY_IND,                        MENU_ON,     CMcmsPCMManager::OnFeccKeyMenuOn)
	ONEVENT(FECC_KEY_IND,                        HIDING_MENU, CMcmsPCMManager::OnFeccKeyHidingMenu)

	ONEVENT(DTMF_IND_PCM,                        IDLE,        CMcmsPCMManager::OnDtmfIndIdle)
	ONEVENT(DTMF_IND_PCM,                        MENU_OFF,    CMcmsPCMManager::OnDtmfIndMenuOff)
	ONEVENT(DTMF_IND_PCM,                        MENU_SETUP,  CMcmsPCMManager::OnDtmfIndMenuSetup)
	ONEVENT(DTMF_IND_PCM,                        MENU_ON,     CMcmsPCMManager::OnDtmfIndMenuOn)
	ONEVENT(DTMF_IND_PCM,                        HIDING_MENU, CMcmsPCMManager::OnDtmfIndHidingMenu)

	ONEVENT(PCM_CLICK_AND_VIEW,                  IDLE,        CMcmsPCMManager::OnStartClickAndViewIdle)
	ONEVENT(PCM_CLICK_AND_VIEW,                  MENU_OFF,    CMcmsPCMManager::OnStartClickAndViewMenuOff)
	ONEVENT(PCM_CLICK_AND_VIEW,                  MENU_SETUP,  CMcmsPCMManager::OnStartClickAndViewMenuSetup)
	ONEVENT(PCM_CLICK_AND_VIEW,                  MENU_ON,     CMcmsPCMManager::OnStartClickAndViewMenuOn)
	ONEVENT(PCM_CLICK_AND_VIEW,                  HIDING_MENU, CMcmsPCMManager::OnStartClickAndViewHidingMenu)

	ONEVENT(CONF_ACTIVE,                         IDLE,        CMcmsPCMManager::NullActionFunction)
	ONEVENT(CONF_ACTIVE,                         ANYCASE,     CMcmsPCMManager::OnPartyStateChangedAnycase)
	ONEVENT(MUTE_STATE,                          IDLE,        CMcmsPCMManager::NullActionFunction)
	ONEVENT(MUTE_STATE,                          ANYCASE,     CMcmsPCMManager::OnMuteStateChangedAnycase)
	ONEVENT(UPDATEVISUALNAME,                    IDLE,        CMcmsPCMManager::NullActionFunction)
	ONEVENT(UPDATEVISUALNAME,                    ANYCASE,     CMcmsPCMManager::OnMuteStateChangedAnycase)

	ONEVENT(PARTY_ADDED,                         ANYCASE,     CMcmsPCMManager::OnPartyAddedAnycase)
	ONEVENT(PARTY_DELETED,                       IDLE,        CMcmsPCMManager::NullActionFunction)
	ONEVENT(PARTY_DELETED,                       ANYCASE,     CMcmsPCMManager::OnPartyDeletedAnycase)
	ONEVENT(DATATOKENRELEASE,                    IDLE,        CMcmsPCMManager::NullActionFunction)
	ONEVENT(DATATOKENRELEASE,                    ANYCASE,     CMcmsPCMManager::OnFeccStoppedAnycase)
	ONEVENT(CPCONFLAYOUT,                        IDLE,        CMcmsPCMManager::NullActionFunction)
	ONEVENT(CPCONFLAYOUT,                        ANYCASE,     CMcmsPCMManager::SendConfLayoutIndication)
	ONEVENT(CPPARTYLAYOUT,                       IDLE,        CMcmsPCMManager::NullActionFunction)
	ONEVENT(CPPARTYLAYOUT,                       ANYCASE,     CMcmsPCMManager::PartyLayoutChanged)
	ONEVENT(UPDATELECTUREMODE,                   IDLE,        CMcmsPCMManager::NullActionFunction)
	ONEVENT(UPDATELECTUREMODE,                   ANYCASE,     CMcmsPCMManager::SendConfLayoutIndication)

	ONEVENT(PARTY_CONNECTED_TO_PCM_ENCODER,      MENU_SETUP,  CMcmsPCMManager::OnPartyConnectedToPCMEncoderMenuSetup)
	ONEVENT(PARTY_CONNECTED_TO_PCM_ENCODER,      ANYCASE,     CMcmsPCMManager::OnPartyConnectedToPCMEncoderAnycase)

	ONEVENT(PARTY_DISCONNECTED_FROM_PCM_ENCODER, HIDING_MENU, CMcmsPCMManager::OnEndPartyDisconnectFromPCMEncoderHidingMenu)
	ONEVENT(PARTY_DISCONNECTED_FROM_PCM_ENCODER, ANYCASE,     CMcmsPCMManager::OnEndPartyDisconnectFromPCMEncoderAnycase)

	ONEVENT(VIDEO_OUT_DISCONNECTED,              ANYCASE,     CMcmsPCMManager::OnPartyVideoOutDisconnectedAnycase)

	ONEVENT(PCM_RESOLUTION_CHANGED,              ANYCASE,     CMcmsPCMManager::OnPcmEncoderImageSizeChanged)

	ONEVENT(PCM_COMMAND,                         ANYCASE,     CMcmsPCMManager::OnPCMCommand)
	ONEVENT(DISCONNECTCONF,                      ANYCASE,     CMcmsPCMManager::OnConfDisconnectAnycase)

	ONEVENT(PCM_SETUP_TOUT,                      MENU_SETUP,  CMcmsPCMManager::OnPCMSetupTimeOutMenuSetup)

PEND_MESSAGE_MAP(CMcmsPCMManager, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CMcmsPCMManager
////////////////////////////////////////////////////////////////////////////
CMcmsPCMManager::CMcmsPCMManager ()
{
	m_currentChairRsrcId     = DUMMY_PARTY_ID;
	m_terminalId             = (BYTE)-1;
	m_menuState              = 0;
	m_feccTokenHolder        = DUMMY_PARTY_ID;
	m_pPcmHardwareInterface  = NULL;
	m_pRsrcParams            = NULL;
	m_pPartyApi              = NULL;
	m_pConfApi               = NULL;
	m_termListInfoIndication = NULL;
	m_imageParams            = e320x240_4x3;
	m_recordingState         = eStopRecording;
	OFF(m_wasConnectPcmSent);
	OFF(m_needToFreeRsrcDisconnectPcm);
	MAX_NUM_OF_MENUS = 4;

	m_bPcmFeccEnable = YES;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
		pSysConfig->GetBOOLDataByKey(CFG_KEY_PCM_FECC, m_bPcmFeccEnable);

	InitMap();
}

//--------------------------------------------------------------------------
CMcmsPCMManager::~CMcmsPCMManager()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (pRoutingTbl != NULL && m_pRsrcParams && m_pRsrcParams->GetPartyRsrcId() == DUMMY_PARTY_ID)
		pRoutingTbl->RemovePartyRsrc(*m_pRsrcParams);

	POBJDELETE(m_pPcmHardwareInterface);
	POBJDELETE(m_pRsrcParams);
	POBJDELETE(m_pPartyApi);
	POBJDELETE(m_pConfApi);
	POBJDELETE(m_termListInfoIndication);
	DestroyMap();
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::InitMap()
{
	const char* objName = NameOf();

	ADD_PCM_ENTRY("pop_menu_status",             CPcmPopMenuStatusCommand,     CMcmsPCMManager::OnPopMenuStatus)
	ADD_PCM_ENTRY("set_all_audio_mute_in",       CPcmSetAllAudioMuteInCommand, CMcmsPCMManager::OnSetAllAudioMute)
	ADD_PCM_ENTRY("set_focus",                   CPcmSetFocusCommand,          CMcmsPCMManager::OnSetFocus)
	ADD_PCM_ENTRY("set_cp_layout",               CPcmSetCpLayoutCommand,       CMcmsPCMManager::OnSetCpLayout)
	ADD_PCM_ENTRY("set_audio_mute_in",           CPcmSetAudioMuteInCommand,    CMcmsPCMManager::OnSetAudioMuteIn)
	ADD_PCM_ENTRY("set_audio_mute_out",          CPcmSetAudioMuteOutCommand,   CMcmsPCMManager::OnSetAudioMuteOut)
	ADD_PCM_ENTRY("set_video_mute_in",           CPcmSetVideoMuteInCommand,    CMcmsPCMManager::OnSetVideoMuteIn)
	ADD_PCM_ENTRY("set_invite_term",             CPcmSetInviteTermCommand,     CMcmsPCMManager::OnSetInviteTerm)
	ADD_PCM_ENTRY("set_drop_term",               CPcmSetDropTermCommand,       CMcmsPCMManager::OnSetDropTerm)
	ADD_PCM_ENTRY("stop_conf",                   CPcmStopConfCommand,          CMcmsPCMManager::OnStopConf)
	ADD_PCM_ENTRY("fecc_control",                CPcmFeccControlCommand,       CMcmsPCMManager::OnFeccControl)
	ADD_PCM_ENTRY("set_conf_layout_mode",        CPcmSetConfLayoutTypeCommand, CMcmsPCMManager::OnSetConfLayoutType)
	ADD_PCM_ENTRY("record",                      CPcmRecordCommand,            CMcmsPCMManager::OnRecord)
	ADD_PCM_ENTRY("local_addr_book",             CPcmLocalAddrBookCommand,     CMcmsPCMManager::OnLocalAddrBook)
	ADD_PCM_ENTRY("set_display_setting",         CPcmSetDisplaySettingCommand, CMcmsPCMManager::OnSetDisplaySetting)

	// ******************	 unsupported messages for COP  (V4.6) *********************************
	ADD_PCM_ENTRY("register",                    CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("preview_conf",                CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("create_conf",                 CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("login_conf",                  CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("get_display_setting_list",    CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("get_focus_term",              CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)

	ADD_PCM_ENTRY("set_adjust_border",           CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("play_ivr",                    CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("check_nid",                   CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("if_term_in_participant_list", CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("request_chairperson",         CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
	ADD_PCM_ENTRY("directory",                   CPcmCommandDummy,             CMcmsPCMManager::PcmNullActionFunction)
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::DestroyMap()
{
	for (PCM_MAP::iterator it = m_pcmMap.begin(); it != m_pcmMap.end(); it++)
	{
		PCM_FUNC_OBJ tmpObj = it->second;
		POBJDELETE(tmpObj.PcmCommand);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
const char* CMcmsPCMManager::StateToString(BYTE state)
{
	switch (state)
	{
		case IDLE       : return "IDLE";
		case MENU_OFF   : return "MENU_OFF";
		case MENU_SETUP : return "MENU_SETUP";
		case MENU_ON    : return "MENU_ON";
		case HIDING_MENU: return "HIDING_MENU";
		default         : return "UNKNOWN STATE";
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::Create(const CMcmsPCMMngrInitParams* pMcmsPCMMngrInitParams)
{
	strcpy_safe(m_pConfName, pMcmsPCMMngrInitParams->GetConfName());

	TRACEINTO << "ConfName:" << m_pConfName;

	m_pConf                 = (CConf*)pMcmsPCMMngrInitParams->GetConf();
	m_confRsrcId            = pMcmsPCMMngrInitParams->GetConfRsrcId();
	m_pAudioBridgeInterface = pMcmsPCMMngrInitParams->GetAudioBridgeInterface();
	m_pVideoBridgeInterface = pMcmsPCMMngrInitParams->GetVideoBridgeInterface();
	m_pConfAppMngrInterface = pMcmsPCMMngrInitParams->GetConfAppMngrInterface();

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(m_pConf->GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif
	m_pConfApi->CreateOnlyApi(m_pConf->GetRcvMbx(), this);
	m_pConfApi->SetLocalMbx(m_pConf->GetLocalQueue());

	// create an empty party api (only pointer without party's mail box)
	m_pPartyApi              = new CPartyApi;
	m_pChair                 = NULL;
	m_currentChairName       = "";
	m_pCommConf              = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
	m_termListInfoIndication = new CPcmTermListInfoIndication(m_terminalId);
	m_pRsrcParams            = NULL;
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::Disconnect()
{
	TRACEINTO << "ConfName:" << m_pConfName;

	DispatchEvent(DISCONNECTCONF, NULL);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnConfDisconnectAnycase(CSegment* pParam)
{
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::AttachEvents()
{
	TRACEINTO << "ConfName:" << m_pConfName;

	COsQueue*   pRcvMbx    = &(m_pConf->GetRcvMbx());
	CConfParty* pConfParty = m_pCommConf->GetFirstParty();
	while (IsValidPObjectPtr(pConfParty))
	{
		PartyMonitorID partyId = pConfParty->GetPartyId();
		// Party events
		pConfParty->AttachObserver(pRcvMbx, MUTE_STATE, OBSERVER_TYPE_PCM, partyId);
		pConfParty->AttachObserver(pRcvMbx, UPDATEVISUALNAME, OBSERVER_TYPE_PCM, partyId);
		if (!strcmp(pConfParty->GetName(), m_currentChairName.c_str()))
			pConfParty->AttachObserver(pRcvMbx, CPPARTYLAYOUT, OBSERVER_TYPE_PCM, partyId);

		pConfParty = m_pCommConf->GetNextParty();
	}

	// Conf events
	m_pCommConf->AttachObserver(pRcvMbx, CONF_ACTIVE, OBSERVER_TYPE_PCM);
	m_pCommConf->AttachObserver(pRcvMbx, CPCONFLAYOUT, OBSERVER_TYPE_PCM);
	m_pCommConf->AttachObserver(pRcvMbx, UPDATELECTUREMODE, OBSERVER_TYPE_PCM);
	m_pCommConf->AttachObserver(pRcvMbx, PARTY_ADDED, OBSERVER_TYPE_PCM);
	m_pCommConf->AttachObserver(pRcvMbx, PARTY_DELETED, OBSERVER_TYPE_PCM);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::DetachEvents()
{
	TRACEINTO << "ConfName:" << m_pConfName;

	COsQueue*   pRcvMbx    = &(m_pConf->GetRcvMbx());
	CConfParty* pConfParty = m_pCommConf->GetFirstParty();
	while (IsValidPObjectPtr(pConfParty))
	{
		pConfParty->DetachObserver(pRcvMbx);
		pConfParty = m_pCommConf->GetNextParty();
	}

	// Conf events
	m_pCommConf->DetachObserver(pRcvMbx);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::HandleObserverUpdate(CSegment* pSeg, WORD type)
{
	void* pSubscriber = NULL;
	WORD  event = 0;
	DWORD val = 0;

	*pSeg >> (DWORD&)pSubscriber >> event >> val;
	CSegment* pParam = new CSegment;
	*pParam << val << pSubscriber;

	DispatchEvent(event, pParam);

	POBJDELETE(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnMuteStateChangedAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnMuteStateChangedAnycase======");

	SendMuteStatusIndications(TRUE);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPartyAddedAnycase(CSegment* pParam)
{
	PartyMonitorID partyId = 0;
	*pParam >> partyId;

	COsQueue*   pRcvMbx    = &(m_pConf->GetRcvMbx());
	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyId);
	if (CPObject::IsValidPObjectPtr(pConfParty))
	{
		pConfParty->AttachObserver(pRcvMbx, MUTE_STATE, OBSERVER_TYPE_PCM, partyId);
		pConfParty->AttachObserver(pRcvMbx, UPDATEVISUALNAME, OBSERVER_TYPE_PCM, partyId);
	}

	if (m_state != IDLE) {
		PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnPartyAddedAnycase======");

		SendMuteStatusIndications();
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPartyDeletedAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnPartyDeletedAnycase======");
	SendMuteStatusIndications();
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPartyStateChangedAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnPartyStateChangedAnycase======");
	SendMuteStatusIndications();
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::PartyLayoutChanged(CSegment* pParam)
{
	DWORD val = 0;
	PartyMonitorID partyId = DUMMY_PARTY_ID;

	*pParam >> val >> partyId;

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyId);
	PASSERTMSG_AND_RETURN(!IsValidPObjectPtr(pConfParty), "CMcmsPCMManager::PartyLayoutChanged - Failed, value 'pConfParty' is not valid");
	PASSERTMSG_AND_RETURN(strcmp(pConfParty->GetName(), m_currentChairName.c_str()), "CMcmsPCMManager::PartyLayoutChanged - Failed, event received not from current chair");

	CVideoLayout* pPartyLayout = (pConfParty->GetIsPrivateLayout()) ? pConfParty->GetCurPrivateVideoLayout() : pConfParty->GetVideoPartyConfLayout();
	CPcmConfLayoutModeInfoIndication layoutModeInfoIndication(m_terminalId);
	layoutModeInfoIndication.SetLayoutParams(m_pCommConf);

	if (pPartyLayout)
	{
		TRACEINTO << "=====CMcmsPCMManager::   MonitorPartyId:" << partyId << ", newLayout:" << pPartyLayout->GetScreenLayout();
		// override conference layout with new party's layout
		layoutModeInfoIndication.SetPrivateLayout(pPartyLayout);
		SendPcmIndication(&layoutModeInfoIndication);
	}
	else
		TRACEINTO << "MonitorPartyId:" << partyId << ", newLayout:NULL";
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnFeccStoppedAnycase(CSegment* pParam)
{
	PartyRsrcID partyId = DUMMY_PARTY_ID;
	*pParam >> partyId;

	TRACEINTO << "PartyId:" << partyId << ", feccTokenHolder:" << m_feccTokenHolder;
	if (m_feccTokenHolder != DUMMY_PARTY_ID)
	{
		if (partyId != m_feccTokenHolder)
			PASSERT(partyId);

		m_feccTokenHolder = DUMMY_PARTY_ID;
		SendFeccEndIndication();
		m_pVideoBridgeInterface->ChangeSpeakerNotation(partyId, INVALID_IMAGE_ID); // return to conf notation
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendPcmIndication(CPcmMessage* pcmMessage)
{
	strmap* mpMsg  = new strmap;
	string* strMsg = new string;

	pcmMessage->SerializeXmlToStr(*mpMsg, *strMsg);

	// VNGR-15603 m_pPcmHardwareInterface might be NULL when indications
	// from conf received (recording state, layout changed , ...),
	// but no one is in pcm session (CP) --> no need to do nothing in such cases (Eitan 06/2010)
	if (m_pPcmHardwareInterface)
		m_pPcmHardwareInterface->SendMessageToPCM(PCM_INDICATION, strMsg->c_str());

	PDELETE(strMsg);
	PDELETE(mpMsg);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendPcmConfirm(CPcmMessage* pcmMessage)
{
	strmap mpMsg;
	string strMsg;
	pcmMessage->SerializeXmlToStr(mpMsg, strMsg);

	if (m_pPcmHardwareInterface) // VNGR-18280 , no one is in pcm session (CP) --> no need to do nothing in such cases
		m_pPcmHardwareInterface->SendMessageToPCM(PCM_CONFIRM, strMsg.c_str());
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendMessageToPCM(DWORD opcode, const char* msgStr)
{
	CSegment*  paramSeg = new CSegment;
	if (msgStr)
	{
		DWORD msgLen = strlen(msgStr);
		paramSeg->Put((BYTE*)msgStr, msgLen + 1);
		paramSeg->DumpHex();

		m_pPcmHardwareInterface->SendMsgToMPL(opcode, paramSeg);
	}

	POBJDELETE(paramSeg);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnAck(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	CProcessBase* process = CProcessBase::GetProcess();
	TRACEINTO << "Opcode:" << process->GetOpcodeAsString(AckOpcode);

	switch (AckOpcode)
	{
		case TB_MSG_CONNECT_REQ:
		case TB_MSG_CONNECT_PCM_REQ: // Ack from CM in CP
		{
			CSegment* pSeg = new CSegment;
			*pSeg << m_currentChairRsrcId << status;
			DispatchEvent(PARTY_CONNECTED_TO_PCM_ENCODER, pSeg);
			POBJDELETE(pSeg);
			break;
		}

		case TB_MSG_DISCONNECT_PCM_REQ: // Ack from CM in CP
		{
			CSegment* pSeg = new CSegment;
			*pSeg << status;
			DispatchEvent(PARTY_DISCONNECTED_FROM_PCM_ENCODER, pSeg);
			POBJDELETE(pSeg);
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonEntered(CSegment* pParam)
{
	COsQueue pPartyMbx;
	DWORD    newChairRsrcId = DUMMY_PARTY_ID;

	*pParam >> newChairRsrcId;
	pPartyMbx.DeSerialize(*pParam);

	if (newChairRsrcId != m_currentChairRsrcId)
	{
		UpdateHardwareInterfaceAndRoutingTable(newChairRsrcId);
		m_pPartyApi->CreateOnlyApi(pPartyMbx);
		if (m_bPcmFeccEnable == YES)
			m_pPartyApi->PartyActionsOnLeaderChanged(TRUE); // update parties RTP

		m_pPartyApi->SendLeaderStatus(TRUE);              // update DB if needed
		m_currentChairRsrcId = newChairRsrcId;
		if (IsPartyOutConnected(m_currentChairRsrcId))
		{
			m_pChair = GetChairPartyTaskApp(newChairRsrcId);
			PASSERTMSG_AND_RETURN(!IsValidPObjectPtr(m_pChair), "CMcmsPCMManager::OnChairpersonEntered - Failed, value 'm_pChair' is not valid");

			m_currentChairName = ((CParty*)m_pChair)->GetName();
			SendDataIndications();
			SendDirectLoginConf();
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CMcmsPCMManager::OnChairpersonEntered - New chair is not connected to Video Out do nothing!!!");
		}
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonEnteredIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnChairpersonEnteredIdle======");
	OnChairpersonEntered(pParam);
	m_state = MENU_OFF;
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonEnteredHidingMenu(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnChairpersonEnteredHidingMenu ( WARNING: ack on disconnection from VBridge has not received yet!)======");
	OnChairpersonEntered(pParam);
}

//--------------------------------------------------------------------------
DWORD CMcmsPCMManager::OnChairpersonLeft(CSegment* pParam)
{
	// CTaskApp* pParty = NULL;
	DWORD leavingChairRsrcId = DUMMY_PARTY_ID;

	// *pParam >> (void*&)pParty;
	*pParam >> leavingChairRsrcId;

	CSmallString cstr;
	cstr << "test chair left - leaving party id: " << leavingChairRsrcId;
	if (leavingChairRsrcId == m_currentChairRsrcId)
	{
		m_pPartyApi->PartyActionsOnLeaderChanged(FALSE); // update parties RTP
		m_pPartyApi->SendLeaderStatus(FALSE);            // update DB if needed
		UpdateHardwareInterfaceAndRoutingTable(DUMMY_PARTY_ID);
		m_currentChairRsrcId = DUMMY_PARTY_ID;
		m_currentChairName   = "";
		// SendTerminalEndCall();
		return leavingChairRsrcId;
	}
	else
	{
		PASSERT(1);  // should not get here!
		return DUMMY_PARTY_ID;
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonLeftIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnChairpersonLeftIdle======");
	TRACEINTOLVLERR << " IMPORTANT - should not get here!";
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonLeftMenuOff(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnChairpersonLeftMenuOff======");
	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
	m_state = IDLE;
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonLeftMenuSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnChairpersonLeftMenuSetup======");

	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
	if (IsValidTimer(PCM_SETUP_TOUT))
		DeleteTimer(PCM_SETUP_TOUT);

	DisconnectPartyFromPCMEncoder(leavingChairRsrcId);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonLeftMenuOn(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnChairpersonLeftMenuOn======");
	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
	DisconnectPartyFromPCMEncoder(leavingChairRsrcId);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonLeftHidingMenu(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnChairpersonLeftHidingMenu======");
	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnChairpersonStartMoveFromConfAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnChairpersonStartMoveFromConfAnycase do nothing!!!======");
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnFeccKeyIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyIdle ======\n\t probably this party is not chair");
	// PASSERT(1); // should not get here!
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnFeccKeyMenuOff(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyMenuOff");
	OnFeccKey(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnFeccKeyMenuSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyMenuSetup");
	OnFeccKey(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnFeccKeyMenuOn(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyMenuOn");
	OnFeccKey(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnFeccKeyHidingMenu(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyHidingMenu");
	OnFeccKey(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnFeccKey(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKey");

	DWORD partyRsrcId = DUMMY_PARTY_ID;
	WORD  key;

	*pParam >> partyRsrcId;
	*pParam >> key;

	if (partyRsrcId != m_currentChairRsrcId)
	{
        	CMedString cstr;
		cstr << "CMcmsPCMManager::OnFeccKey  partyRsrcId != m_currentChairRsrcId  partyRsrcId:" << partyRsrcId << " m_currentChairRsrcId:" << m_currentChairRsrcId;
		PTRACE(eLevelInfoNormal, cstr.GetString());
		FeccKeyRecievedFromWrongChair(partyRsrcId);
		return;
	}

	if (!IsPartyOutConnected(m_currentChairRsrcId))
	{
		PTRACE(eLevelInfoNormal, "CMcmsPCMManager::OnFeccKey party is not connected to video do nothing!");
		return;
	}

	string                   keyStr = ::feccKeyToString((feccKeyEnum)key);
	CPcmControlKeyIndication ind(m_terminalId);
	ind.SetKey(keyStr);
	SendPcmIndication(&ind);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnDtmfIndIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnDtmfIndIdle");
	PASSERT(1); // should not get here!
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnDtmfIndMenuOff(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnDtmfIndMenuOff");
	PASSERT(1); // should not get here!
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnDtmfIndMenuSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnDtmfIndMenuSetup");
	OnDtmfInd(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnDtmfIndMenuOn(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnDtmfIndMenuOn");
	OnDtmfInd(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnDtmfIndHidingMenu(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnDtmfIndHidingMenu");
	OnDtmfInd(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnStartClickAndViewIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnStartClickAndViewIdle - SHOULD NOT GET HERE");
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnStartClickAndViewMenuOff(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnStartClickAndViewMenuOff");
	PartyRsrcID partyId = DUMMY_PARTY_ID;
	*pParam >> partyId;
	if (partyId != m_currentChairRsrcId)
	{
		PASSERTSTREAM_AND_RETURN(1, "PartyId: " << partyId << ", ChairPartyId: " << m_currentChairRsrcId);
	}

	string str = ::feccKeyToString(eFeccKeyZoomIn);
	CPcmControlKeyIndication ind(m_terminalId);
	ind.SetKey(str);
	SendPcmIndication(&ind);

	string str1 = "1";
	CPcmControlKeyIndication ind1(m_terminalId);
	ind1.SetKey(str1);
	SendPcmIndication(&ind1);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnStartClickAndViewMenuSetup(CSegment* pParam)
{
	// in this state all dtmfs should be forwarded to PCM
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnStartClickAndViewMenuSetup - SHOULD NOT GET HERE");
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnStartClickAndViewMenuOn(CSegment* pParam)
{
	// in this state all dtmfs should be forwarded to PCM
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnStartClickAndViewMenuOn - SHOULD NOT GET HERE");
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnStartClickAndViewHidingMenu(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnStartClickAndViewHidingMenu");

	string str = "1";
	CPcmControlKeyIndication ind(m_terminalId);
	ind.SetKey(str);
	SendPcmIndication(&ind);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnDtmfInd(CSegment* pParam)
{
	PartyRsrcID PartyId = DUMMY_PARTY_ID;
	DWORD dtmfLen = 0;
	pParam->DumpHex();

	*pParam >> PartyId;
	*pParam >> dtmfLen;

	if (dtmfLen)
	{
		char dtmf[dtmfLen+1];
		memset(dtmf, '\0', dtmfLen+1);
		pParam->Get((BYTE*)dtmf, dtmfLen);
		dtmf[dtmfLen] = '\0'; // no need because of the memset - but we do it so clockwork will not see it as error

		if (PartyId != m_currentChairRsrcId)
		{
			PASSERTSTREAM_AND_RETURN(1, "PartyId: " << PartyId << ", ChairPartyId: " << m_currentChairRsrcId);
		}

		if (strcspn(dtmf, "1234567890*#") == strlen(dtmf))
		{
			TRACEINTO << "DTMF:" << dtmf << " - Unsupported";
			return;
		}

		string str = dtmf;
		CPcmControlKeyIndication ind(m_terminalId);
		ind.SetKey(str);
		SendPcmIndication(&ind);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPartyConnectedToPCMEncoderMenuSetup(CSegment* pParam)
{
	DeleteTimer(PCM_SETUP_TOUT);

	CMedString cstr;
	cstr << "=====CMcmsPCMManager::OnPartyConnectedToPCMEncoderMenuSetup";

	PartyRsrcID partyRsrcId;
	STATUS status;
	*pParam >> partyRsrcId >> status;

	if (partyRsrcId != m_currentChairRsrcId)
	{
		cstr << " illegal party rsrc id: " << partyRsrcId << " (chair id: " << m_currentChairRsrcId << ")";
		PASSERTMSG(partyRsrcId, cstr.GetString());
	}

	cstr << " status: " << CProcessBase::GetProcess()->GetStatusAsString(status);
	if (status)
		PASSERTMSG(status, cstr.GetString());
	else
		PTRACE(eLevelInfoNormal, cstr.GetString());

	if (m_needToFreeRsrcDisconnectPcm)
	{
		PTRACE(eLevelInfoNormal, "PCM resource is no longer needed (chair left/moved , error?) , disconnect pcm");
		OFF(m_needToFreeRsrcDisconnectPcm);
		DisconnectPartyFromPCMEncoder(partyRsrcId);
		return;
	}

	m_state = MENU_ON;
	OFF(m_wasConnectPcmSent);

	if (!m_menuState)
		PTRACE(eLevelInfoNormal, "CMcmsPCMManager::OnPartyConnectedToPCMEncoderMenuSetup PCM module did not send pop_menu_status = 1 yet!!!");

	m_pPartyApi->PartyConnectedToPCM();             // start forward DTMF to PCM module
	m_pPartyApi->PartyActionsOnLeaderChanged(TRUE); // update parties RTP
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPartyConnectedToPCMEncoderAnycase(CSegment* pParam)
{
	if (IsValidTimer(PCM_SETUP_TOUT))
		DeleteTimer(PCM_SETUP_TOUT);

	PartyRsrcID  partyRsrcId;
	STATUS  status;
	*pParam >> partyRsrcId >> status;

	CMedString cstr;
	cstr << "*** SHOULD NOT GET HERE!!! (this opcode expexted at state: MENU_SETUP , current state: " << StateToString(m_state) << "\n";
	cstr << "=====CMcmsPCMManager::OnPartyConnectedToPCMEncoderAnycase  , status: " << CProcessBase::GetProcess()->GetStatusAsString(status);
	PTRACE(eLevelInfoNormal, cstr.GetString());

	if (partyRsrcId != m_currentChairRsrcId)
	{
		cstr.Clear();
		cstr << "illegal party rsrc id: " << partyRsrcId << " (chair id: " << m_currentChairRsrcId << ")";
		PASSERTMSG(partyRsrcId, cstr.GetString());
	}

	PASSERT(status);

	if (m_needToFreeRsrcDisconnectPcm)
	{
		PTRACE(eLevelInfoNormal, "PCM resource is no longer needed (chair left/moved , error?) , disconnect pcm");
		OFF(m_needToFreeRsrcDisconnectPcm);
		DisconnectPartyFromPCMEncoder(partyRsrcId);
		return;
	}

	m_state = MENU_ON; // ???
	OFF(m_wasConnectPcmSent);

	if (!m_menuState)
		PTRACE(eLevelInfoNormal, "CMcmsPCMManager::OnPartyConnectedToPCMEncoderAnycase PCM module did not send pop_menu_status = 1 yet!!!");

	m_pPartyApi->PartyConnectedToPCM();             // start forward DTMF to PCM module
	m_pPartyApi->PartyActionsOnLeaderChanged(TRUE); // update parties RTP
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnEndPartyDisconnectFromPCMEncoderHidingMenu(CSegment* pParam)
{
	PartyRsrcID partyRsrcId;
	STATUS  status;
	*pParam >> partyRsrcId >> status;

	CMedString cstr;
	cstr << "=====CMcmsPCMManager::OnEndPartyDisconnectFromPCMEncoderHidingMenu  , status: " << CProcessBase::GetProcess()->GetStatusAsString(status);
	PTRACE(eLevelInfoNormal, cstr.GetString());
	cstr.Clear();
	PASSERT(status);
	if (DUMMY_PARTY_ID == m_currentChairRsrcId)
	{
		cstr << "no chair in conf changing to state IDLE";
		m_state = IDLE;
	}
	else
	{
		cstr << "there is chair in conf changing to state MENU_OFF";
		m_state = MENU_OFF;
	}

	PTRACE(eLevelInfoNormal, cstr.GetString());

	if (m_menuState)
		PTRACE(eLevelInfoNormal, "CMcmsPCMManager::OnEndPartyDisconnectFromPCMEncoderHidingMenu PCM module did not send pop_menu_status = 0 yet!!!");
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnEndPartyDisconnectFromPCMEncoderAnycase(CSegment* pParam)
{
	PartyRsrcID  partyRsrcId;
	STATUS  status;
	*pParam >> partyRsrcId >> status;

	CMedString cstr;
	cstr << "*** SHOULD NOT GET HERE!!! (this opcode expected at state: MENU_ON , current state: " << StateToString(m_state) << "\n";
	cstr << "=====CMcmsPCMManager::OnEndPartyDisconnectFromPCMEncoderAnycase  , status: " << CProcessBase::GetProcess()->GetStatusAsString(status);
	PTRACE(eLevelInfoNormal, cstr.GetString());
	cstr.Clear();
	PASSERT(status);
	if (DUMMY_PARTY_ID == m_currentChairRsrcId)
	{
		cstr << "no chair in conf changing to state IDLE";
		m_state = IDLE;
	}
	else
	{
		cstr << "there is chair in conf changing to state MENU_OFF";
		m_state = MENU_OFF;
	}

	PTRACE(eLevelInfoNormal, cstr.GetString());

	if (m_menuState)
		PTRACE(eLevelInfoNormal, "CMcmsPCMManager::OnEndPartyDisconnectFromPCMEncoderAnycase PCM module did not send pop_menu_status = 0 yet!!!");
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPartyVideoOutDisconnectedAnycase(CSegment* pParam)
{
	PartyRsrcID  partyRsrcId;
	STATUS  status;
	*pParam >> partyRsrcId >> status;

	CMedString cstr;
	cstr << "=====CMcmsPCMManager::OnPartyVideoOutDisconnectedAnycase  , status: " << CProcessBase::GetProcess()->GetStatusAsString(status);
	PTRACE(eLevelInfoNormal, cstr.GetString());
	cstr.Clear();
	PASSERT(status);

	if (DUMMY_PARTY_ID == m_currentChairRsrcId)
	{
		cstr << "no chair in conf changing to state IDLE";
		m_state = IDLE;
	}
	else if (partyRsrcId == m_currentChairRsrcId)
	{
		cstr << "chair was disconnected from video out changing to state MENU_OFF";
		m_state = MENU_OFF;
		m_pPartyApi->PartyDisconnectedFromPCM();   // stop forward DTMF to PCM module
		SendTerminalEndCall();
		// SendVideoOutIndications
	}
	else
	{
		PASSERT(partyRsrcId);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPCMSetupTimeOutMenuSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnPCMSetupTimeOutMenuSetup========");
	PASSERT(1);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPcmEncoderImageSizeChanged(CSegment* pParam)
{
	PartyRsrcID partyRsrcId;
	pcmImageParams imageParams;
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnPcmEncoderImageSizeChanged=======");

	*pParam >> partyRsrcId;
	*pParam >> (DWORD&)imageParams;

	if (m_imageParams != imageParams)
	{
		m_imageParams = imageParams;
		SendImageSizeIndication(m_imageParams);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::DumpStrMap(strmap& mpMsg)
{
	CLargeString cstr;
	int i = 1;
	for (strmap::iterator it = mpMsg.begin(); it != mpMsg.end(); it++)
	{
		cstr << i <<") key: " << it->first.c_str();
		for (int j = it->first.size(); j < 25; j++)
			cstr << "";

		cstr << " value: " << it->second.c_str() << "\n";
		i++;
	}

	PTRACE(eLevelInfoNormal, cstr.GetString());
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnPCMCommand(CSegment* pParam)
{
	// PTRACE(eLevelInfoNormal,"=====CMcmsPCMManager::OnPCMCommand======");

	DWORD strLen = pParam->GetWrtOffset() - pParam->GetRdOffset();
	ALLOCBUFFER(buf, strLen+1);
	pParam->Get((BYTE*)buf, strLen);
	buf[strLen] = '\0';

	strmap mpMsg;
	CDstCommonFunctions g_CommonFun;
	bool res = g_CommonFun.xml_XmlStrToMap(buf, mpMsg);
	DEALLOCBUFFER(buf);

	CSmallString errStr;
	if (res != true)
	{
		errStr << "xml_XmlStrToMap FAILED!\n";
		PASSERTMSG(1, errStr.GetString());
		return;
	}

	string command = mpMsg["_xml_msg_id"]; // mpMsg["_xml_msg_id"] must be valid (validation is taking place in xml_XmlStrToMap)

	PCM_MAP::iterator it;
	it = m_pcmMap.find(command);

	if (it != m_pcmMap.end())
	{
		CPcmCommand* pcmCommand = m_pcmMap[command].PcmCommand->Clone();
		ACT_FUNC     actionFunc = m_pcmMap[command].actFunc;

		pcmCommand->DeSerializeXml(mpMsg);

		if (!pcmCommand->IsValidPcmMessage(errStr))
		{
			DumpStrMap(mpMsg);
			PASSERTMSG(1, errStr.GetString());
			return;
		}

		if (pcmCommand->GetTerminalId() != m_terminalId)
		{
			DumpStrMap(mpMsg);
			errStr << "CMcmsPCMManager::OnPCMCommand illegal terminal id! receieved: " << mpMsg["TERM_ID"].c_str() << " (expected: " << m_terminalId <<")";
			PASSERTMSG(1, errStr.GetString());
			return;
		}

		STATUS status = (this->*(actionFunc))(pcmCommand);

		CPcmConfirm pcmConfirm(*pcmCommand);
		pcmConfirm.SetResult(status);
		pcmConfirm.SetActionName(command);
		SendPcmConfirm(&pcmConfirm);

		POBJDELETE(pcmCommand);
	}
	else
	{
		errStr << "CMcmsPCMManager::OnPCMCommand command not found in PCM_MAP ( recieved command: " << command.c_str() << " )";
		PASSERTMSG(1, errStr.GetString());
		return;
	}
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::PcmNullActionFunction(CPcmCommand* pcmCommand)
{
	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnPopMenuStatus(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnPopMenuStatus======");

	m_menuState = ((CPcmPopMenuStatusCommand*)pcmCommand)->GetStatus();

	// Temp - ToDo Eitan add this event to state machine!!
	switch (m_state)
	{
		case (MENU_OFF):
		{
			if (m_menuState)
				ConnectPartyToPCMEncoder(m_currentChairRsrcId);
			break;
		}

		case (MENU_SETUP):
		case (MENU_ON):
		{
			if (!m_menuState)
				DisconnectPartyFromPCMEncoder(m_currentChairRsrcId);
			break;
		}

		default:
			break;
	} // switch

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetAllAudioMute(CPcmCommand* pcmCommand)
{
	BYTE   isSetAudioMute = (((CPcmSetAllAudioMuteInCommand*)pcmCommand)->GetValue() == true) ? TRUE : FALSE;
	EOnOff eOnOff;
	string muteStr = isSetAudioMute ? "Mute" : "Unmute";

	PTRACE2(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetAllAudioMute====== ", muteStr.c_str());

	CConfParty* pCurConfParty = m_pCommConf->GetFirstParty();
	const char* lecturerName  = NULL;
	BYTE isLectureInConf  = FALSE;
	CLectureModeParams* pLectureModeParams = m_pCommConf->GetLectureMode();
	if (pLectureModeParams)
	{
		lecturerName = pLectureModeParams->GetLecturerName();
		if (lecturerName[0] != '\0')
			isLectureInConf = TRUE;
	}

	// VNGR-23292
	if (!isSetAudioMute)
		m_pConfApi->SetMuteIncomingLectureMode(0);

	DWORD partyState;
	while (CPObject::IsValidPObjectPtr(pCurConfParty))
	{
		// don't mute/unmute the chair (VNGR-16174 - or any other chair in conf)
		if (!pCurConfParty->GetIsLeader())  // strcmp(pCurConfParty->GetName(), m_currentChairName.c_str())
		{
			if (!isLectureInConf || (isLectureInConf && strcmp(pCurConfParty->GetName(), lecturerName)))
			{
				partyState = pCurConfParty->GetPartyState();
				if (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY ||
				    partyState == PARTY_CONNECTED_WITH_PROBLEM || partyState == PARTY_CONNECTED_PARTIALY)
				{
					if (pCurConfParty->IsAudioMutedByOperator() != isSetAudioMute)
					{
						eOnOff = isSetAudioMute ? eOn : eOff;
						DWORD mediaMask = 0x00000001;
						m_pConfApi->MuteMedia(pCurConfParty->GetName(), eOnOff, mediaMask);
					}
				}
			}
		}
		pCurConfParty = m_pCommConf->GetNextParty();
	} // end while

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetCpLayout(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetCpLayout======");

	BYTE apiLayoutType = ((CPcmSetCpLayoutCommand*)pcmCommand)->GetApiLayoutType();
	if (apiLayoutType != CP_NO_LAYOUT)
	{
		CVideoLayout videoLayout;
		videoLayout.SetScreenLayout(apiLayoutType);

		m_pConfApi->SetVideoConfLayoutSeeMeAll(videoLayout);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CMcmsPCMManager::OnSetCpLayout, layout type sent from PCM is not supported in RMX!");
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetAudioMuteIn(CPcmCommand* pcmCommand)
{
	BYTE   isSetAudioMute  = (((CPcmSetAudioMuteInCommand*)pcmCommand)->GetValue() == true) ? TRUE : FALSE;
	string partyToMuteName = ((CPcmSetAudioMuteInCommand*)pcmCommand)->GetTermToMuteName();
	EOnOff eOnOff;

	PTRACE2INT(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetAudioMuteIn======", isSetAudioMute);

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyToMuteName.c_str());
	if (!CPObject::IsValidPObjectPtr(pConfParty))
	{
		pConfParty = m_pCommConf->GetCurrentPartyAccordingToVisualName(partyToMuteName.c_str());
		if (!CPObject::IsValidPObjectPtr(pConfParty))
		{
			PTRACE2(eLevelInfoNormal, "Can't find party in conf DB, name: ", partyToMuteName.c_str());
			return STATUS_FAIL;
		}
	}

	if (!pConfParty->GetIsLeader())
	{
		DWORD partyState = pConfParty->GetPartyState();
		if (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY ||
		    partyState == PARTY_CONNECTED_WITH_PROBLEM || partyState == PARTY_CONNECTED_PARTIALY)
		{
			if (pConfParty->IsAudioMutedByOperator() != isSetAudioMute)
			{
				eOnOff = isSetAudioMute ? eOn : eOff;
				DWORD mediaMask = 0x00000001;
				m_pConfApi->MuteMedia(pConfParty->GetName(), eOnOff, mediaMask);
			}
		}
	}
	else
	{
		TRACEINTO << "CMcmsPCMManager::OnSetAudioMuteIn party: " << pConfParty->GetName() << " is chairperson, do not mute/unmute";
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetAudioMuteOut(CPcmCommand* pcmCommand)
{
	BYTE   isSetAudioMute  = (((CPcmSetAudioMuteOutCommand*)pcmCommand)->GetValue() == true) ? TRUE : FALSE;
	string partyToMuteName = ((CPcmSetAudioMuteOutCommand*)pcmCommand)->GetTermToMuteName();
	EOnOff eOnOff;

	PTRACE2INT(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetAudioMuteOut(Block audio)======", isSetAudioMute);

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyToMuteName.c_str());
	if (!CPObject::IsValidPObjectPtr(pConfParty))
	{
		pConfParty = m_pCommConf->GetCurrentPartyAccordingToVisualName(partyToMuteName.c_str());
		if (!CPObject::IsValidPObjectPtr(pConfParty))
		{
			PTRACE2(eLevelInfoNormal, "Can't find party in conf DB, name: ", partyToMuteName.c_str());
			return STATUS_FAIL;
		}
	}

	if (!pConfParty->GetIsLeader())
	{
		DWORD partyState = pConfParty->GetPartyState();
		if (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY ||
		    partyState == PARTY_CONNECTED_WITH_PROBLEM || partyState == PARTY_CONNECTED_PARTIALY)
		{
			if (pConfParty->IsAudioBlocked() != isSetAudioMute)
			{
				eOnOff = isSetAudioMute ? eOn : eOff;
				DWORD mediaMask = 0x00000001;
				m_pConfApi->BlockMedia(pConfParty->GetName(), eOnOff, mediaMask);
			}
		}
	}
	else
	{
		TRACEINTO << "CMcmsPCMManager::OnSetAudioMuteOut party: " << pConfParty->GetName() << " is chairperson, do not mute/unmute";
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetVideoMuteIn(CPcmCommand* pcmCommand)
{
	BYTE   isSetVideoMute  = (((CPcmSetVideoMuteInCommand*)pcmCommand)->GetValue() == true) ? TRUE : FALSE;
	string partyToMuteName = ((CPcmSetVideoMuteInCommand*)pcmCommand)->GetTermToMuteName();
	EOnOff eOnOff;

	PTRACE2INT(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetVideoMuteIn======", isSetVideoMute);

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyToMuteName.c_str());
	if (!CPObject::IsValidPObjectPtr(pConfParty))
	{
		pConfParty = m_pCommConf->GetCurrentPartyAccordingToVisualName(partyToMuteName.c_str());
		if (!CPObject::IsValidPObjectPtr(pConfParty))
		{
			PTRACE2(eLevelInfoNormal, "Can't find party in conf DB , name: ", partyToMuteName.c_str());
			return STATUS_FAIL;
		}
	}

	if (!pConfParty->GetIsLeader())
	{
		DWORD partyState = pConfParty->GetPartyState();
		if (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY ||
		    partyState == PARTY_CONNECTED_WITH_PROBLEM || partyState == PARTY_CONNECTED_PARTIALY)
		{
			if (pConfParty->IsVideoMutedByOperator() != isSetVideoMute)
			{
				eOnOff = isSetVideoMute ? eOn : eOff;
				DWORD mediaMask = 0x00000002;
				m_pConfApi->MuteMedia(pConfParty->GetName(), eOnOff, mediaMask);
			}
		}
	}
	else
	{
		TRACEINTO << "CMcmsPCMManager::OnSetVideoMuteIn party: " << pConfParty->GetName() << " is chairperson, do not mute/unmute";
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetInviteTerm(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetInviteTerm======");

	CAddressBook* pAddressBook = CAddressBook::Instance();
	PASSERT_AND_RETURN_VALUE(!pAddressBook, STATUS_FAIL);

	strIntMap* terminalNamesTypesMap = ((CPcmSetInviteTermCommand*)pcmCommand)->GetInvitedTermDetails();
	bool isFromAddressBook = ((CPcmSetInviteTermCommand*)pcmCommand)->IsFromAddressBook();
	multiset<CRsrvParty*, CompareByPartyName>* pPartiesSet = pAddressBook->GetAddressBookPartiesSet();
	for (strIntMap::iterator it = terminalNamesTypesMap->begin(); it != terminalNamesTypesMap->end(); ++it)
	{
		const char* invitedPartyName = it->first.c_str();
		int invitedPartyNetworkType = it->second;
		if (isFromAddressBook == true)
		{
			// search for the party in address book
			CRsrvParty* pTmpParty = NULL;
			for (multiset<CRsrvParty*, CompareByPartyName>::iterator itr = pPartiesSet->begin(); itr != pPartiesSet->end(); itr++)
			{
				pTmpParty = *itr;
				if (strncasecmp(invitedPartyName, pTmpParty->GetName(), H243_NAME_LEN) == 0)
					break;
			}

			// test validation and add to conf
			if (pTmpParty)
			{
				PTRACE2(eLevelInfoNormal, "CMcmsPCMManager::OnSetInviteTerm - Invite from address book, PartyName:", pTmpParty->GetName());
				pTmpParty->SetAdditionalInfo("InvitedByPcm");
				pTmpParty->SetUndefinedType(UNRESERVED_PARTY);
				DWORD status = ValidateAndAddPartyToConf(pTmpParty);
				if (status != STATUS_OK)
				{
					TRACEINTO << "ValidateAndAddPartyToConf failed!  , status: " << CProcessBase::GetProcess()->GetStatusAsString(status);
					OnConfInviteResult(false);
				}
			}
			// party is not in address book
			else
			{
				PTRACE2(eLevelInfoNormal, "CMcmsPCMManager::OnSetInviteTerm - Invite from address book, but cannot find party in address book, PartyName:", invitedPartyName);
				OnConfInviteResult(false);
				return STATUS_FAIL;
			}
		}
		else  // user entered DTMF, try invite with the number the user entered
		{
			m_pConfApi->PCMInviteParty(invitedPartyName, invitedPartyNetworkType);
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetDropTerm(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetDropTerm======");

	vector<string>* namesToDropVector = ((CPcmSetDropTermCommand*)pcmCommand)->GetNamesVector();
	BYTE kickAll = (((CPcmSetDropTermCommand*)pcmCommand)->GetKickAll() == true) ? TRUE : FALSE;

	int status = STATUS_OK;

	/***  SEND Disconnect party to conference ***/
	CCommConf* pCurCommConf = ::GetpConfDB()->GetCurrentConfByNameOrByNumericId(m_pConfName);
	int listSize = namesToDropVector->size();
	if (pCurCommConf)
	{
		for (int i = 0; i < listSize; i++)
		{
			const char* partyNameInPCM = namesToDropVector->at(i).c_str();
			CConfParty* pCurConfParty  = pCurCommConf->GetCurrentParty(partyNameInPCM);
			if (!CPObject::IsValidPObjectPtr(pCurConfParty))
			{
				pCurConfParty = m_pCommConf->GetCurrentPartyAccordingToVisualName(partyNameInPCM);
				if (!CPObject::IsValidPObjectPtr(pCurConfParty))
				{
					PTRACE2(eLevelInfoNormal, "Can't find party in conf DB, name: ", partyNameInPCM);
					continue;
				}
			}

			const char* curPartyName = pCurConfParty->GetName();
			DWORD partyID = pCurConfParty->GetPartyId();

			if (pCurConfParty->IsUndefinedParty())
				m_pConfApi->DropParty(curPartyName, 0 /*delete*/);
			else
				m_pConfApi->DropParty(curPartyName, 1 /*disconnect*/);

			// cdr
			pCurCommConf->OperatorPartyAction(curPartyName, partyID, "", OPERATOR_DISCONNECTE_PARTY);
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetConfLayoutType(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetConfLayoutType======");
	STATUS status = STATUS_OK;

	int layoutMode  = ((CPcmSetConfLayoutTypeCommand*)pcmCommand)->GetLayoutMode();
	int lectureRole = ((CPcmSetConfLayoutTypeCommand*)pcmCommand)->GetLectureRole();

	CLectureModeParams* pNewLectureModeParams = new CLectureModeParams;
	BYTE oldSameLayoutValue = m_pCommConf->GetIsSameLayout();

	if (layoutMode == 1)
	{
		pNewLectureModeParams->SetLectureModeType(0);
		pNewLectureModeParams->SetTimerOnOff(0);
	}
	else
	{
		pNewLectureModeParams->SetLectureModeType(1); // Lecture Mode
		pNewLectureModeParams->SetTimerOnOff(1);
		if (lectureRole != -1)
		{
			CConfParty* pConfParty = m_pCommConf->GetCurrentParty(lectureRole);
			if (pConfParty)
				pNewLectureModeParams->SetLecturerName(pConfParty->GetName());
			else
			{
				PTRACE(eLevelInfoNormal, "CMcmsPCMManager::OnSetConfLayoutType Invalid pConfParty!!!");
				POBJDELETE(pNewLectureModeParams);
				return STATUS_FAIL;
			}
		}
	}

	m_pCommConf->SetSetLectureModeSameLayoutRules(pNewLectureModeParams);
	status = m_pCommConf->TestLectureModeValidity(pNewLectureModeParams);

	if (status == STATUS_OK)
	{
		m_pConfApi->UpdateLectureMode(pNewLectureModeParams);
	}
	else
	{
		m_pCommConf->SetIsSameLayout(oldSameLayoutValue);
	}

	POBJDELETE(pNewLectureModeParams);

	return status;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnRecord(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnRecord======");
	string commandOpcodeStr = ((CPcmRecordCommand*)pcmCommand)->GetCmd();

	DWORD  recordingCommand;
	if (commandOpcodeStr == "start")
	{
		if (m_recordingState == ePauseRecording)
			recordingCommand = SET_RESUME_RECORDING;
		else
			recordingCommand = SET_START_RECORDING;
	}
	else if (commandOpcodeStr == "stop")
	{
		recordingCommand = SET_STOP_RECORDING;
	}
	else if (commandOpcodeStr == "pause")
	{
		recordingCommand = SET_PAUSE_RECORDING;
	}
	else
	{
		CSmallString cstr;
		cstr << "CMcmsPCMManager::OnRecord unknown opcode str: " << commandOpcodeStr.c_str();
		PASSERTMSG(1, cstr.GetString());
		return STATUS_FAIL;
	}

	if (m_pCommConf->GetEnableRecording())
	{
		DWORD partyRsrcID = (DWORD)(-1); // not relevant
		DWORD confOrPartyAction = EVENT_CONF_REQUEST;
		DWORD param = (DWORD)(-1); // not in use here

		m_pConfApi->SendCAMGeneralActionCommand(partyRsrcID, confOrPartyAction, recordingCommand, param);
	}
	else
	{
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnLocalAddrBook(CPcmCommand* pcmCommand)
{
	DWORD status = STATUS_OK;
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnLocalAddrBook======");

	int    count          = ((CPcmLocalAddrBookCommand*)pcmCommand)->GetCount();
	string firstPartyName = ((CPcmLocalAddrBookCommand*)pcmCommand)->GetMatchStr();
	string lastPartyName  = ((CPcmLocalAddrBookCommand*)pcmCommand)->GetMatchStrEnd();

	CAddressBook* pAddressBook = CAddressBook::Instance();
	if (!pAddressBook)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	multiset<CRsrvParty*, CompareByPartyName>* pPartiesSet = pAddressBook->GetAddressBookPartiesSet();
	multiset<CRsrvParty*, CompareByPartyName>::iterator it, itFirst, itLast, itStr;
	CLargeString cstr2;
	cstr2 << "AddressBook: < ";
	for (itStr = pPartiesSet->begin(); itStr != pPartiesSet->end(); ++itStr)
	{
		cstr2 << (*itStr)->GetName() << " , ";
	}

	cstr2 << " >\n";
	PTRACE(eLevelInfoNormal, cstr2.GetString());

	if (firstPartyName == "" || lastPartyName == "")
	{
		itFirst = pPartiesSet->begin();
		itLast  = pPartiesSet->end();
	}
	else
	{
		CRsrvParty dummyPartyFirst;
		dummyPartyFirst.SetName(firstPartyName.c_str());
		itFirst = pPartiesSet->lower_bound(&dummyPartyFirst);

		for (itLast = itFirst; itLast != pPartiesSet->end(); itLast++)
		{
			int result = strncasecmp((*itLast)->GetName(), lastPartyName.c_str(), 1);
			TRACEINTO << "comparing: " << (*itLast)->GetName() << " to: " << lastPartyName.c_str() << " result: " << result;
			if (result > 0)
			{
				break;
			}
		}
	}

	CLargeString cstr;
	cstr << "before loop - firstPartyName: " << firstPartyName.c_str() << " lastPartyName: " << lastPartyName.c_str() << "\n";
	cstr << "itFirst: ";
	if (itFirst != pPartiesSet->end())
		cstr << ((CRsrvParty*)(*itFirst))->GetName();
	else
		cstr << "pPartiesSet->end()";

	cstr << " itLast: ";
	if (itLast != pPartiesSet->end())
		cstr << ((CRsrvParty*)(*itLast))->GetName();
	else
		cstr << "pPartiesSet->end()";

	PTRACE(eLevelInfoNormal, cstr.GetString());
	cstr.Clear();


	cstr2 << "sending parties: \n ";

	int  numPartiesInMessage = 0;
	int  maxPartiesInMessage = 100;
	bool isFinished          = false;
	for (int numMessagesToSend = 0; numMessagesToSend < (count / maxPartiesInMessage)+1 && isFinished == false; numMessagesToSend++)
	{
		CPcmLocalAddressBookIndication pcmLocalAddressBookIndication(m_terminalId);
		numPartiesInMessage = 0;

		for (it = itFirst; it != itLast && numPartiesInMessage < maxPartiesInMessage; it++)
		{
			pcmLocalAddressBookIndication.AddPartyToVector(*it);
			cstr2 << (*it)->GetName() << " , ";
			numPartiesInMessage++;
		}

		cstr2 << "\n";
		cstr << "after inner loop , iteration num: " << (numMessagesToSend+1) << "\n";
		cstr << "itFirst: " << ((itFirst != pPartiesSet->end()) ? (*itFirst)->GetName() : "pPartiesSet->end()");
		cstr << " itLast: " << ((itLast != pPartiesSet->end()) ? (*itLast)->GetName() : "pPartiesSet->end()");
		cstr << " it: " << ((it != pPartiesSet->end()) ? (*it)->GetName() : "pPartiesSet->end()");
		PTRACE(eLevelInfoNormal, cstr.GetString());
		cstr.Clear();

		pcmLocalAddressBookIndication.SetSequenceNum(numMessagesToSend);
		pcmLocalAddressBookIndication.SetCount(numPartiesInMessage);
		if (numPartiesInMessage < maxPartiesInMessage)
		{
			isFinished = true;
		}
		else
		{
			if (it != pPartiesSet->end())
			{
				itFirst    = it;
				isFinished = false;
			}
			else
			{
				isFinished = true;
			}
		}

		pcmLocalAddressBookIndication.SetIsFinished(isFinished);
		SendPcmIndication(&pcmLocalAddressBookIndication);
	} // for numMessagesToSend

	PTRACE(eLevelInfoNormal, cstr2.GetString());
	return status;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::ValidateAndAddPartyToConf(CRsrvParty* pRsrvParty)
{
	DWORD status     = STATUS_OK;
	DWORD respStatus = STATUS_OK;
	DWORD confStat   = m_pCommConf->GetStatus();
	if (confStat & CONFERENCE_RESOURCES_DEFICIENCY)
	{
		TRACEINTO << "CMcmsPCMManager::ValidateAndAddPartyToConf - CONFERENCE_RESOURCES_DEFICIENCY";
		return STATUS_INSUFFICIENT_RSRC;
	}

	WORD confNumParts     = m_pCommConf->GetNumParties();
	BYTE isRecordingParty = pRsrvParty->GetRecordingLinkParty();
	if (STATUS_OK == status)
	{
		WORD confMaxParts = m_pCommConf->GetMaxParties();
		if (m_pCommConf->IncludeRecordingParty())
			confNumParts = confNumParts -1;   // we don't include the recording party in the max participants limitation.

		if ((confMaxParts <= confNumParts) && (confMaxParts != 255) && !isRecordingParty)
		{
			// Reject only if we do not have IVR service or we got a defiend party
			if (!m_pCommConf->IsDefinedIVRService() || !pRsrvParty->IsUndefinedParty())
			{
				TRACESTR(eLevelError) << "CMcmsPCMManager::ValidateAndAddPartyToConf Conference has already : "
				                      << m_pCommConf->GetNumParties()<< ", max parties allowed is: "<< confMaxParts;
				status = STATUS_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE;
			}
		}
	}

	CConfPartyProcess* pProcess = (CConfPartyProcess*)CProcessBase::GetProcess();

	WORD maxPartiesInConfPerSystemMode = pProcess->GetMaxNumberOfPartiesInConf();
	BYTE isAudioOnlyParty = pRsrvParty->GetVoice();
	if (confNumParts >= maxPartiesInConfPerSystemMode)
	{
		TRACEINTO << "CMcmsPCMManager::ValidateAndAddPartyToConf - STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED";
		status = STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
	}

	if (!isAudioOnlyParty)
	{
		WORD confNumVideoParts = m_pCommConf->GetNumVideoParties();
		WORD maxVideoPartiesInConfPerSystemMode = pProcess->GetMaxNumberOfVideoPartiesInConf();
		if (confNumVideoParts >= maxVideoPartiesInConfPerSystemMode)
		{
			PTRACE2(eLevelInfoNormal, "ConfPartyManager::ValidateAndAddPartyToConf : max video participants exceeded Name : ", pRsrvParty->GetName());
			status = STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
		}
	}

	if (STATUS_OK == status)
	{
		// Only in case of DialOut it should be blocked.
		if (m_pCommConf->IsConfSecured() && pRsrvParty->GetConnectionType() == DIAL_OUT)
		{
			TRACESTR(eLevelError) <<"CMcmsPCMManager::ValidateAndAddPartyToConf To the Secured conference can not be add any parties: ";
			status = STATUS_ADD_PARTICIPANT_TO_SECURE_CONF;
		}
	}

	if (STATUS_OK == status)
	{
		status = m_pCommConf->TestPartyRsrvValidity(pRsrvParty);
		TRACEINTO << "CMcmsPCMManager::ValidateAndAddPartyToConf m_pCommConf->TestPartyRsrvValidity = " << status;
	}

	// status = pRsrvParty->CheckReservRangeValidity(errorCode/*, apiNum*/);

	/*** VALIDITY of conferenceId and CHECK does party already exist ***/
	// check in ongoing conference
	const char* party_name = pRsrvParty->GetName();
	if (STATUS_OK == status)
	{
		status = ::GetpConfDB()->SearchPartyName(m_pConfName, party_name);
		if (status == STATUS_OK)
		{
			TRACEINTO << "CMcmsPCMManager::ValidateAndAddPartyToConf STATUS_PARTY_NAME_EXISTS";
			status = STATUS_PARTY_NAME_EXISTS;
		}
		else if (status == STATUS_PARTY_DOES_NOT_EXIST)
			status = STATUS_OK;
	}

	if (STATUS_OK == status) // if the party name is identical to visual name that already exist
	{
		status = ::GetpConfDB()->SearchPartyVisualName(m_pConfName, party_name);
		if (status == STATUS_OK)
			status = STATUS_PARTY_NAME_EXISTS;
		else if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
			status = STATUS_OK;
	}

	// Alert the user if we have another party with the same ip
	if (STATUS_OK == status)
	{
		status = m_pCommConf->SearchPartyByIPOrAlias(pRsrvParty->GetIpAddress()
		                                             , pRsrvParty->GetH323PartyAlias()
		                                             , pRsrvParty->GetH323PartyAliasType());

		if (STATUS_PARTY_DOES_NOT_EXIST == status)
			status = STATUS_OK;
		else
		{
			// IpV6
			char ipAddr[64];
			::ipToString(pRsrvParty->GetIpAddress(), ipAddr, 1);
			TRACESTR(eLevelInfoNormal) << "CMcmsPCMManager::ValidateAndAddPartyToConf Found Party with the same IP: "
			                           << ipAddr << " or same aliase name: "<< pRsrvParty->GetH323PartyAlias();
			respStatus = status;
			status     = STATUS_OK;
		}
	}

	// AUDIO BRIDGE TEMPORARY
	if (status == STATUS_OK)
	{
		if (m_pCommConf->IsAudioConf())
			// TEMPYOELLA if (pRsrvParty->IsIpNetInterfaceType())
			pRsrvParty->SetVoice(YES);
	}

	// In HD VSW conference MCU force the defined endopoints bitrate to be auto
	if ((status == STATUS_OK) ||
	    (status == (STATUS_PARTY_NAME_EXISTS | WARNING_MASK)))
	{
		if (m_pCommConf->GetIsHDVSW())
			pRsrvParty->SetVideoRate(0xFFFFFFFF);  // automatic
	}

	if ((status == STATUS_OK) ||
	    ((CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator) &&
	     (status == (STATUS_PARTY_IP_ALIAS_ALREADY_EXISTS | WARNING_MASK))))
	{
		DWORD nextPartyId = m_pCommConf->NextPartyId();
		// SET new partyId to party
		if (pRsrvParty->GetPartyId() <= HALF_MAX_DWORD || pRsrvParty->GetPartyId() == 0xFFFFFFFF)
		{
			((CRsrvParty*)pRsrvParty)->SetPartyId(nextPartyId);
		}
	}

	CMoveConfDetails confMoveDetails(m_pCommConf);
	CMoveInfo* partyMoveInfo = pRsrvParty->GetMoveInfo();
	partyMoveInfo->Create(confMoveDetails, pRsrvParty->IsOperatorParty());

	// SEND Add party to conference
	if ((status == STATUS_OK) ||
	    (status == (STATUS_PARTY_NAME_EXISTS | WARNING_MASK)))  // those are ok statuses as well
	{
		m_pConfApi->AddParty(*pRsrvParty);
		if (pRsrvParty->GetCascadeMode() == CASCADE_MODE_SLAVE && !pRsrvParty->GetVoice())
		{
			m_pCommConf->UpdateLectureModeAndLayoutBecauseSlaveInConf(pRsrvParty->GetName());
			m_pConfApi->UpdateAutoLayout(NO);
			m_pConfApi->SetVideoConfLayoutSeeMeAll(*m_pCommConf->GetVideoLayout());
			m_pConfApi->UpdateLectureMode(m_pCommConf->GetLectureMode());
		}
	}

	if (!isRecordingParty)
	{
		// Write to CDR  if event was sent from Operator
		if (status == STATUS_OK)
		{
			m_pCommConf->OperatorAddParty(pRsrvParty, "PCM", OPERATOR_ADD_PARTY);
			m_pCommConf->OperatorAddPartyCont1(pRsrvParty, "PCM", OPERATOR_ADD_PARTY_CONTINUE_1);
			if (pRsrvParty->GetIpAddress().ipVersion == eIpVersion6)
			{
				m_pCommConf->OperatorIpV6PartyCont1(pRsrvParty, "PCM", USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS);
			}

			m_pCommConf->OperatorAddPartyCont2(pRsrvParty, OPERATOR_ADD_PARTY_CONTINUE_2);
			m_pCommConf->UpdateUserDefinedInformation(pRsrvParty);
			m_pCommConf->OperatorAddPartyEventToCdr(pRsrvParty, (pRsrvParty->GetServiceProviderName()), eOperatorAddPartyAction_OperatorAddPartyToOngoingConference);
		}
	}

	if (STATUS_OK != respStatus)
		status = respStatus;

	return status;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnStopConf(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnStopConf======");
	m_pConfApi->Destroy();

	// cdr
	m_pCommConf->OperatorTerminate("");
	m_pCommConf->SetStatus(CONFERENCE_TERMINATING);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnFeccControl(CPcmCommand* pcmCommand)
{
	int paneIndex = ((CPcmFeccControlCommand*)pcmCommand)->GetPaneIndex();

	CLayout* confLayout = m_pVideoBridgeInterface->GetPartyLayout(m_currentChairName.c_str());
	TRACECOND_AND_RETURN_VALUE(!confLayout, "Failed, Invalid conf layout", STATUS_OK);

	int subImageId = pcmCommand->TranslatePcmPaneIndexToMcmsSubImageId(confLayout->GetLayoutType(), paneIndex);

	TRACEINTO << "paneIndex:" << paneIndex << ", subImageId:" << subImageId;

	TRACECOND_AND_RETURN_VALUE(subImageId == -1, "Failed, Invalid sub-image id, subImageId:" << subImageId, STATUS_FAIL);
	TRACECOND_AND_RETURN_VALUE(subImageId == MAX_SUB_IMAGES_IN_LAYOUT, "Failed, Invalid sub-image id, subImageId:" << subImageId, STATUS_OK);

	CVidSubImage* subImage = (*confLayout)[subImageId];
	PASSERTSTREAM_AND_RETURN_VALUE(!subImage, "Failed, Invalid video sub-image, subImageId:" << subImageId, STATUS_OK);

	DWORD partyRsrcId = subImage->GetImageId();
	if (partyRsrcId)
	{
		CPartyCntl* pPartyCntl = m_pConf->GetPartyCntl(partyRsrcId);
		PASSERTSTREAM_AND_RETURN_VALUE(!pPartyCntl, "Failed, Invalid party control, subImageId:" << subImageId, STATUS_OK);

		CTaskApp*   pPartyToMove = pPartyCntl->GetPartyTaskApp();
		PASSERTSTREAM_AND_RETURN_VALUE(!pPartyToMove, "Failed, Invalid party task, subImageId:" << subImageId, STATUS_OK);

		WORD mcuNumber = 0;
		WORD terminalNumber = 0;
		PartyRsrcID partyId = pPartyToMove->GetPartyId();
		m_pConf->GetPartyTerminalNumber(pPartyToMove, mcuNumber, terminalNumber);

		TRACEINTO << "mcuNumber:" << mcuNumber << ", terminalNumber:" << terminalNumber << " - Send VIN to party";

		m_pPartyApi->SendVIN(mcuNumber, terminalNumber, partyId);

		// simulate fecc token request from party
		m_pPartyApi->OnIpDataTokenMsg(kTokenRequest, LSD_6400, 1);
		m_feccTokenHolder = m_currentChairRsrcId;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManager::OnSetFocus(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnSetFocus======");

	int paneIndex = ((CPcmSetFocusCommand*)pcmCommand)->GetFocusPos();

	CTaskApp* pPartyToMove = NULL;
	CLayout*  confLayout   = m_pVideoBridgeInterface->GetPartyLayout(m_currentChairName.c_str());
	if (m_feccTokenHolder != DUMMY_PARTY_ID)
		PTRACE2INT(eLevelInfoNormal, "Warning! set focus received but fecc token is held (possible mismatch between the moved cell and the highlighted cell), token holder: ", m_feccTokenHolder);

	if (confLayout != NIL(CLayout))
	{
		int imageId = pcmCommand->TranslatePcmPaneIndexToMcmsSubImageId(confLayout->GetLayoutType(), paneIndex);
		if (imageId == -1)
		{
			PTRACE(eLevelInfoNormal, "Invalid image id");
			return STATUS_FAIL;
		}
		m_pVideoBridgeInterface->ChangeSpeakerNotation(m_currentChairRsrcId, imageId);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "Invalid conf layout");
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendDataIndications()
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::SendDataIndications======");

	// SendLanguageIndication(); - language indication not in use caused the pcm to reset!
	SendMenuIndications();
	m_imageParams = (pcmImageParams)m_pVideoBridgeInterface->GetPartyResolutionInPCMTerms(m_currentChairRsrcId);
	SendImageSizeIndication(m_imageParams);
	SendConfInfoIndication();
	SendConfLayoutIndication();
	SendServiceSettingIndication();
	SendMuteStatusIndications(TRUE);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendMenuIndications()
{
	CPcmInitialMenuBufferIndication initialMenuIndication(m_terminalId);
	SendPcmIndication(&initialMenuIndication);

	CPcmMenuStateIndication menuStateIndication(m_terminalId); // default value is menu on
	SendPcmIndication(&menuStateIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendImageSizeIndication(pcmImageParams imageParams)
{
	CPcmImageSizeIndication imageSizeIndication(m_terminalId);

	switch (imageParams)
	{
		case (e320x240_4x3):
		{
			imageSizeIndication.SetImageType("320*240");
			imageSizeIndication.SetImageRatio("4:3");
			break;
		}

		case (e640x480_4x3):
		{
			imageSizeIndication.SetImageType("640*480");
			imageSizeIndication.SetImageRatio("4:3");
			break;
		}

		case (e1024x576_16x9):
		{
			imageSizeIndication.SetImageType("1024*576");
			imageSizeIndication.SetImageRatio("16:9");
			break;
		}

		default:
		{
			PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::SendImageSizeIndication Illegal image size!");
			return;
		}
	} // switch

	SendPcmIndication(&imageSizeIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendConfInfoIndication()
{
	CPcmConfInfoIndication confInfoIndication(m_terminalId);
	confInfoIndication.InitInternalParams(m_pCommConf);

	SendPcmIndication(&confInfoIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendConfLayoutIndication()
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::SendConfLayoutIndication");

	CPcmConfLayoutModeInfoIndication layoutModeInfoIndication(m_terminalId);

	layoutModeInfoIndication.SetLayoutParams(m_pCommConf);

	SendPcmIndication(&layoutModeInfoIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendServiceSettingIndication()
{
	CPcmServiceSettingIndication serviceSettingIndication(m_terminalId);

	const char* conf_serv_name = m_pCommConf->GetServiceNameForMinParties();
	CConfIpParameters* pConfIpParameters = ::GetIpServiceListMngr()->GetRelevantService(conf_serv_name, H323_INTERFACE_TYPE);
	// GetRelevantService always return service: by name, default, or first in list
	if (pConfIpParameters == NULL)
	{
		PASSERT(101);
		return;
	}

	if (pConfIpParameters)
	{
		CONF_IP_PARAMS_S* serviceParams = pConfIpParameters->GetConfIpParamsStruct();
		WORD protocolType = serviceParams->service_protocol_type;
		if (protocolType == eIPProtocolType_H323)
			serviceSettingIndication.SetServiceSetting(0);
		else if (protocolType == eIPProtocolType_SIP)
			serviceSettingIndication.SetServiceSetting(1);
		else if (protocolType == eIPProtocolType_SIP_H323)
			serviceSettingIndication.SetServiceSetting(2);
		else
			PASSERT(1); // illegal service settings

		SendPcmIndication(&serviceSettingIndication);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendMuteStatusIndications(BYTE sendIfNoChange)
{
	if (m_termListInfoIndication->FillOngoingPartiesVector(m_pCommConf) || sendIfNoChange)
	{
		SendPcmIndication(m_termListInfoIndication);

		CPcmAllAudioMuteStateIndication allAudioMuteStateIndication(m_terminalId);
		allAudioMuteStateIndication.CalculateAllMuteFlag(m_pCommConf);
		// allAudioMuteStateIndication.SetAllMuteButXFlag(m_pCommConf->IsMuteAllButX(m_currentChairRsrcId));
		SendPcmIndication(&allAudioMuteStateIndication);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendDirectLoginConf()
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::SendDirectLoginConf======");
	CPcmDirectLoginConfIndication directLoginIndication(m_terminalId);
	directLoginIndication.InitInternalParams(m_pCommConf->GetName(), m_pCommConf->GetMonitorConfId(), m_currentChairName);
	SendPcmIndication(&directLoginIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendTerminalEndCall()
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::SendTerminalEndCall======");
	CPcmTerminalEndCallIndication terminaEndCallIndication(m_terminalId);
	SendPcmIndication(&terminaEndCallIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendFeccEndIndication()
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::SendFeccEndIndication======");
	CPcmFeccEndIndication feccEndIndication(m_terminalId);
	SendPcmIndication(&feccEndIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::OnConfInviteResult(bool res)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnConfInviteResult======");
	CPcmInviteResultIndication pcmInviteResultIndication(m_terminalId);
	pcmInviteResultIndication.SetResult(res);
	SendPcmIndication(&pcmInviteResultIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendRecordingStateIndication(WORD recordState)
{
	if (recordState != m_recordingState)
	{
		PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::SendRecordingStateIndication");

		m_recordingState = recordState;
		CPcmRecordStateIndication pcmRecordStateIndication(m_terminalId);
		pcmRecordStateIndication.SetRecordingStateStrFromWord(m_recordingState);
		SendPcmIndication(&pcmRecordStateIndication);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::SendLanguageIndication()
{
	CPcmLanguageSettingIndication pcmLanguageSetting(m_terminalId);
	pcmLanguageSetting.SetLanguageFromSysConfig();
	SendPcmIndication(&pcmLanguageSetting);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::ConnectPartyToPCMEncoder(DWORD partyId)
{
	if (IsPartyOutConnected(partyId))
	{
		if (partyId != DUMMY_PARTY_ID)
		{
			StartTimer(PCM_SETUP_TOUT, PCM_SETUP_TOUT_VALUE);
			m_state = MENU_SETUP;
			m_pVideoBridgeInterface->ConnectPartyToPCMEncoder(partyId);
		}
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::DisconnectPartyFromPCMEncoder(DWORD partyId)
{
	if (partyId != DUMMY_PARTY_ID)
	{
		m_state = HIDING_MENU;
		m_pPartyApi->PartyDisconnectedFromPCM();   // stop forward DTMF to PCM module

		if (m_bPcmFeccEnable == NO)
			m_pPartyApi->PartyActionsOnLeaderChanged(FALSE);

		if (IsPartyOutConnected(partyId))
		{
			m_pVideoBridgeInterface->DisconnectPartyFromPCMEncoder(partyId);
		}
		else
		{
			DBGPASSERT(partyId);
			PTRACE(eLevelInfoNormal, "======CMcmsPCMManager::DisconnectPartyFromPCMEncoder party is not connected to video out");
			m_state = MENU_OFF;
		}
	}
}

//--------------------------------------------------------------------------
BOOL CMcmsPCMManager::IsPartyOutConnected(DWORD partyId)
{
	BOOL partyInConnected, partyOutConnected = FALSE;
	m_pVideoBridgeInterface->ArePortsOpened(partyId, partyInConnected, partyOutConnected);

	return partyOutConnected;
}

//--------------------------------------------------------------------------
CTaskApp* CMcmsPCMManager::GetChairPartyTaskApp(DWORD partyRsrcId)
{
	return m_pVideoBridgeInterface->GetPartyTaskApp(partyRsrcId);
}

//--------------------------------------------------------------------------
void CMcmsPCMManager::UpdateHardwareInterfaceAndRoutingTable(DWORD newPartyRsrcId)
{
	CPartyRsrcRoutingTblKey oldRoutingKey = CPartyRsrcRoutingTblKey(*m_pRsrcParams);
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (NULL == pRoutingTbl)
	{
		PASSERT_AND_RETURN(103);
	}

	int status = pRoutingTbl->UpdatePartyRsrcIdInRoutingTbl(oldRoutingKey, newPartyRsrcId);
	if (status != STATUS_OK) // Entry not found in Routing Table
		PASSERT(104);

	m_pRsrcParams->SetPartyRsrcId(newPartyRsrcId);
	m_pPcmHardwareInterface->SetPartyRsrcId(newPartyRsrcId);
}


PBEGIN_MESSAGE_MAP(CMcmsPCMManagerCOP)
	ONEVENT(DISCONNECTCONF, ANYCASE, CMcmsPCMManagerCOP::OnConfDisconnectAnycase)
PEND_MESSAGE_MAP(CMcmsPCMManagerCOP, CMcmsPCMManager);

////////////////////////////////////////////////////////////////////////////
//                        CMcmsPCMManagerCOP
////////////////////////////////////////////////////////////////////////////
CMcmsPCMManagerCOP::CMcmsPCMManagerCOP()
{
}

//--------------------------------------------------------------------------
CMcmsPCMManagerCOP::~CMcmsPCMManagerCOP ()
{
	// DetachEvents();
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCOP::Create(const CMcmsPCMMngrInitParams* pMcmsPCMMngrInitParams)
{
	CMcmsPCMManager::Create(pMcmsPCMMngrInitParams);

	PTRACE(eLevelInfoNormal, "===========CMcmsPCMManagerCOP::Create==================");

	CopRsrcsParams* pCopRsrcs = m_pConf->GetCopResources();
	CopRsrcDesc rsrcDesc = pCopRsrcs->constRsrcs[ePcmManager];
	m_terminalId = pCopRsrcs->pcmMenuId;

	if (m_terminalId >= MAX_NUM_OF_MENUS)
		PASSERTMSG(m_terminalId, "illegal term_id for pcm menu");

	m_termListInfoIndication->SetTerminalId(m_terminalId);

	if (IsValidPObjectPtr(m_pCommConf))
	{
		AttachEvents();
	}
	else
	{
		PASSERTMSG(1, "CMcmsPCMManager::Create can not find m_pCommConf in CommConfDb");
		// self kill???
	}

	m_pRsrcParams = new CRsrcParams(rsrcDesc.connectionId,
	                                rsrcDesc.rsrcEntityId,
	                                m_confRsrcId,
	                                rsrcDesc.logicalRsrcType);

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (NULL == pRoutingTbl)
	{
		PASSERT_AND_RETURN(103);
	}

	CTaskApi* pTaskApiMcmsPcmManager = new CTaskApi(*m_pConfApi);

	pTaskApiMcmsPcmManager->CreateOnlyApi(m_pConfApi->GetRcvMbx(), this);
	// pRoutingTbl->DumpTable();
	int stat = pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pRsrcParams, pTaskApiMcmsPcmManager);
	PASSERT(stat);
	// pRoutingTbl->DumpTable();

	m_pPcmHardwareInterface = new CPCMHardwareInterface(*m_pRsrcParams);

	POBJDELETE(pTaskApiMcmsPcmManager);
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManagerCOP::OnPopMenuStatus(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCOP::OnPopMenuStatus======");

	return CMcmsPCMManager::OnPopMenuStatus(pcmCommand);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCOP::OnConfDisconnectAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMcmsPCMManagerCOP::OnConfDisconnectAnycase:  Name - ", m_pConfName);
	DetachEvents();
}


PBEGIN_MESSAGE_MAP(CMcmsPCMManagerCP)

	ONEVENT(CHAIRPERSON_ENTERED,                 IDLE,                CMcmsPCMManagerCP::OnChairpersonEnteredIdle)
	ONEVENT(CHAIRPERSON_ENTERED,                 ANYCASE,             CMcmsPCMManagerCP::OnChairpersonEnteredAnycase)

	ONEVENT(FECC_KEY_IND,                        MENU_OFF,            CMcmsPCMManagerCP::OnFeccKeyMenuOff)
	ONEVENT(FECC_KEY_IND,                        ALLOCATE_PCM_RSRC,   CMcmsPCMManagerCP::OnFeccKeyAllocatePcm)
	ONEVENT(FECC_KEY_IND,                        MENU_SETUP,          CMcmsPCMManagerCP::OnFeccKeyMenuSetup)
	ONEVENT(FECC_KEY_IND,                        MENU_ON,             CMcmsPCMManagerCP::OnFeccKeyMenuOn)
	ONEVENT(FECC_KEY_IND,                        HIDING_MENU,         CMcmsPCMManagerCP::OnFeccKeyHidingMenu)

	ONEVENT(ALLOCATE_PCM_RSRC_IND,               ALLOCATE_PCM_RSRC,   CMcmsPCMManagerCP::OnRsrsAllocPcmRspAllocate)
	ONEVENT(DEALLOCATE_PCM_RSRC_IND,             DEALLOCATE_PCM_RSRC, CMcmsPCMManagerCP::OnRsrsDeAllocPcmRspDeAllocate)

	ONEVENT(CHAIRPERSON_LEFT,                    IDLE,                CMcmsPCMManager::OnChairpersonLeftIdle)
	ONEVENT(CHAIRPERSON_LEFT,                    MENU_OFF,            CMcmsPCMManagerCP::OnChairpersonLeftMenuOff)
	ONEVENT(CHAIRPERSON_LEFT,                    MENU_SETUP,          CMcmsPCMManagerCP::OnChairpersonLeftMenuSetup)
	ONEVENT(CHAIRPERSON_LEFT,                    MENU_ON,             CMcmsPCMManagerCP::OnChairpersonLeftMenuOn)
	ONEVENT(CHAIRPERSON_LEFT,                    HIDING_MENU,         CMcmsPCMManagerCP::OnChairpersonLeftHidingMenu)

	ONEVENT(CHAIRPERSON_START_MOVE_FROM_CONF,    ALLOCATE_PCM_RSRC,   CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfAllocatePcm)
	ONEVENT(CHAIRPERSON_START_MOVE_FROM_CONF,    MENU_SETUP,          CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfMenuSetup)
	ONEVENT(CHAIRPERSON_START_MOVE_FROM_CONF,    MENU_ON,             CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfMenuOn)

	ONEVENT(CONF_ACTIVE,                         MENU_OFF,            CMcmsPCMManagerCP::NullActionFunction)
	ONEVENT(CONF_ACTIVE,                         ALLOCATE_PCM_RSRC,   CMcmsPCMManagerCP::NullActionFunction)
	ONEVENT(CONF_ACTIVE,                         DEALLOCATE_PCM_RSRC, CMcmsPCMManagerCP::NullActionFunction)
	ONEVENT(MUTE_STATE,                          MENU_OFF,            CMcmsPCMManagerCP::NullActionFunction)
	ONEVENT(MUTE_STATE,                          ALLOCATE_PCM_RSRC,   CMcmsPCMManagerCP::NullActionFunction)
	ONEVENT(MUTE_STATE,                          DEALLOCATE_PCM_RSRC, CMcmsPCMManagerCP::NullActionFunction)
	ONEVENT(MUTE_STATE,                          IDLE,                CMcmsPCMManager::NullActionFunction)
	ONEVENT(MUTE_STATE,                          ANYCASE,             CMcmsPCMManager::OnMuteStateChangedAnycase)
	ONEVENT(UPDATEVISUALNAME,                    IDLE,                CMcmsPCMManager::NullActionFunction)
	ONEVENT(UPDATEVISUALNAME,                    ANYCASE,             CMcmsPCMManager::OnMuteStateChangedAnycase)

	ONEVENT(PARTY_ADDED,                         ANYCASE,             CMcmsPCMManager::OnPartyAddedAnycase)
	ONEVENT(PARTY_DELETED,                       IDLE,                CMcmsPCMManager::NullActionFunction)
	ONEVENT(PARTY_DELETED,                       ANYCASE,             CMcmsPCMManager::OnPartyDeletedAnycase)

	ONEVENT(PCM_SETUP_TOUT,                      ALLOCATE_PCM_RSRC,   CMcmsPCMManagerCP::OnPCMSetupTimeOutAllocatePcm)
	ONEVENT(PCM_SETUP_TOUT,                      MENU_SETUP,          CMcmsPCMManagerCP::OnPCMSetupTimeOutMenuSetup)
	ONEVENT(PCM_SETUP_TOUT,                      HIDING_MENU,         CMcmsPCMManagerCP::OnPCMSetupTimeOutHidingMenu)
	ONEVENT(PARTY_DISCONNECTED_FROM_PCM_ENCODER, HIDING_MENU,         CMcmsPCMManagerCP::OnEndPartyDisconnectFromPCMEncoderHidingMenu)
	ONEVENT(PARTY_DISCONNECTED_FROM_PCM_ENCODER, ANYCASE,             CMcmsPCMManagerCP::OnEndPartyDisconnectFromPCMEncoderAnycase)
	ONEVENT(DISCONNECTCONF,                      ANYCASE,             CMcmsPCMManagerCP::NullActionFunction)

PEND_MESSAGE_MAP(CMcmsPCMManagerCP, CMcmsPCMManager);

////////////////////////////////////////////////////////////////////////////
//                        CMcmsPCMManagerCP
////////////////////////////////////////////////////////////////////////////
CMcmsPCMManagerCP::CMcmsPCMManagerCP ()
{
	// InitMap();
}

//--------------------------------------------------------------------------
CMcmsPCMManagerCP::~CMcmsPCMManagerCP ()
{
	ClearChairsMap();
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::Create(const CMcmsPCMMngrInitParams* pMcmsPCMMngrInitParams)
{
	CMcmsPCMManager::Create(pMcmsPCMMngrInitParams);

	PTRACE(eLevelInfoNormal, "===========CMcmsPCMManagerCP::Create==================");
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::HandleObserverUpdate(CSegment* pSeg, WORD type)
{
	switch (m_state)
	{
		case (IDLE):
		case (MENU_OFF):
		case (ALLOCATE_PCM_RSRC):
		case (DEALLOCATE_PCM_RSRC):
		{
			break;
		}

		default:
		{
			CMcmsPCMManager::HandleObserverUpdate(pSeg, type);
			break;
		}
	} // switch
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonEntered(CSegment* pParam)
{
	// PTRACE(eLevelInfoNormal,"=====CMcmsPCMManager::OnChairpersonEntered======");
	// CTaskApp* pParty = NULL;
	COsQueue pPartyMbx;
	DWORD newChairRsrcId = DUMMY_PARTY_ID;

	*pParam >> newChairRsrcId;
	pPartyMbx.DeSerialize(*pParam);

	if (!IsChair(newChairRsrcId))
	{
		CPartyApi*  pPartyApi = new CPartyApi;
		pPartyApi->CreateOnlyApi(pPartyMbx);

		BOOL bPCM_FECC = YES;
		BOOL isCopLicense = ::GetIsCOPdongleSysMode();
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		if (pSysConfig && !isCopLicense)
			pSysConfig->GetBOOLDataByKey(CFG_KEY_PCM_FECC, bPCM_FECC);

		if (bPCM_FECC == YES)
			pPartyApi->PartyActionsOnLeaderChanged(TRUE); // update parties RTP

		// pPartyApi->SendLeaderStatus(TRUE); //update DB if needed

		chairpersonsMap[newChairRsrcId] = pPartyApi;
		PrintChairsMap();
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonEnteredIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonEnteredIdle======");
	OnChairpersonEntered(pParam);
	m_state = MENU_OFF;
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonEnteredAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonEnteredAnycase======");
	OnChairpersonEntered(pParam);
}

//--------------------------------------------------------------------------
DWORD CMcmsPCMManagerCP::OnChairpersonLeft(CSegment* pParam)
{
	// CTaskApp* pParty = NULL;
	DWORD leavingChairRsrcId = DUMMY_PARTY_ID;

	// *pParam >> (void*&)pParty;
	*pParam >> leavingChairRsrcId;

	CSmallString cstr;
	PTRACE2INT(eLevelInfoNormal, " chair left - leaving party id: ", leavingChairRsrcId);
	if (IsChair(leavingChairRsrcId))
	{
		PrintChairsMap();
		CHAIRS_MAP::iterator it = chairpersonsMap.find(leavingChairRsrcId);

		CPartyCntl* pPartyCntl = NULL;
		CConfParty* pConfParty = NULL;
		CPartyApi*  pPartyApi  = it->second;
		pPartyCntl = m_pConf->GetPartyCntl(leavingChairRsrcId);

		if (pPartyCntl)
		{
			pConfParty = m_pCommConf->GetCurrentParty(pPartyCntl->GetName());
		}

		if (pConfParty)
		{
			DWORD state = pConfParty->GetPartyState();

			char* stateStr = "UNKNOWN";
			switch (state)
			{
				case PARTY_IDLE                  : stateStr = "IDLE";                   break;
				case PARTY_CONNECTED             : stateStr = "CONNECTED";              break;
				case PARTY_DISCONNECTED          : stateStr = "DISCONNECTED";           break;
				case PARTY_WAITING_FOR_DIAL_IN   : stateStr = "WAITING_FOR_DIAL_IN";    break;
				case PARTY_CONNECTING            : stateStr = "CONNECTING";             break;
				case PARTY_DISCONNECTING         : stateStr = "DISCONNECTING";          break;
				case PARTY_CONNECTED_PARTIALY    : stateStr = "CONNECTED_PARTIALY";     break;
				case PARTY_DELETED_BY_OPERATOR   : stateStr = "DELETED_BY_OPERATOR";    break;
				case PARTY_SECONDARY             : stateStr = "SECONDARY";              break;
				case PARTY_STAND_BY              : stateStr = "STAND_BY";               break;
				case PARTY_CONNECTED_WITH_PROBLEM: stateStr = "CONNECTED_WITH_PROBLEM"; break;
				case PARTY_REDIALING             : stateStr = "REDIALING";              break;
			} // switch

			TRACEINTO << "CMcmsPCMManagerCP::OnChairpersonLeft - Party State = " << stateStr;

			if (state != PARTY_DISCONNECTING && state != PARTY_DISCONNECTED)
			{
				pPartyApi->PartyActionsOnLeaderChanged(FALSE); // update parties RTP
			}
		}

		pPartyApi->DestroyOnlyApi();
		POBJDELETE(pPartyApi);

		chairpersonsMap.erase(it);
		PrintChairsMap();
		return leavingChairRsrcId;
	}
	else
	{
		PASSERT(1);  // should not get here!
		return DUMMY_PARTY_ID;
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonLeftMenuOff(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonLeftMenuOff======");
	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
	if (chairpersonsMap.empty())
		m_state = IDLE;
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonLeftAllocatePcm(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonLeftAllocatePcm======");

	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
	// if the chair that actually sees menu was removed
	if (m_currentChairRsrcId == leavingChairRsrcId)
	{
		PTRACE(eLevelInfoNormal, "current chair is leaving conf and menu is being allocated now - need to deallocate the rsrc");
		ON(m_needToFreeRsrcDisconnectPcm);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonLeftMenuSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonLeftMenuSetup======");

	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
	// if the chair that actually sees menu was removed
	if (m_currentChairRsrcId == leavingChairRsrcId)
	{
		PTRACE(eLevelInfoNormal, "current chair is leaving conf and pcm menu is in setup stage (sent CONNECT_PCM, did not receive ack yet) - need to disconnect ");
		ON(m_needToFreeRsrcDisconnectPcm);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonLeftMenuOn(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonLeftMenuOn======");
	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
	// if the chair that actually sees menu was removed
	if (m_currentChairRsrcId == leavingChairRsrcId)
		DisconnectPartyFromPCMEncoder(leavingChairRsrcId);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonLeftHidingMenu(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonLeftHidingMenu======");
	DWORD leavingChairRsrcId = OnChairpersonLeft(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfAllocatePcm(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfAllocatePcm======");
	DWORD leavingChairRsrcId = DUMMY_PARTY_ID;
	*pParam >> leavingChairRsrcId;

	if (m_currentChairRsrcId == leavingChairRsrcId)
	{
		PTRACE(eLevelInfoNormal, "current chair is moving from conf and menu is being allocated now - need to deallocate the rsrc");
		ON(m_needToFreeRsrcDisconnectPcm);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfMenuSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfMenuSetup======");
	DWORD leavingChairRsrcId = DUMMY_PARTY_ID;
	*pParam >> leavingChairRsrcId;

	if (m_currentChairRsrcId == leavingChairRsrcId)
	{
		PTRACE(eLevelInfoNormal, "current chair is moving from conf and pcm menu is in setup stage (sent CONNECT_PCM, did not receive ack yet) - need to disconnect ");
		ON(m_needToFreeRsrcDisconnectPcm);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfMenuOn(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnChairpersonStartMoveFromConfMenuOn======");
	DWORD leavingChairRsrcId = DUMMY_PARTY_ID;
	*pParam >> leavingChairRsrcId;

	if (m_currentChairRsrcId == leavingChairRsrcId)
	{
		PTRACE(eLevelInfoNormal, "current chair is moving from conf and pcm menu is active - disconnect pcm ");
		DisconnectPartyFromPCMEncoder(leavingChairRsrcId);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnPCMSetupTimeOutAllocatePcm(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnPCMSetupTimeOutAllocatePcm======");
	PASSERT(m_currentChairRsrcId);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnPCMSetupTimeOutMenuSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnPCMSetupTimeOutMenuSetup===(no response from MPL to TB_MSG_CONNECT_PCM_REQ!!!!)===");
	PASSERT(m_currentChairRsrcId);
	DeAllocatePcmRsrc();
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnPCMSetupTimeOutHidingMenu(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnPCMSetupTimeOutHidingMenu===no response from MPL to TB_MSG_DISCONNECT_PCM_REQ!!!!)===");
	PASSERT(m_currentChairRsrcId);
	DeAllocatePcmRsrc();
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnFeccKeyMenuOff(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnFeccKeyMenuOff");

	DWORD partyRsrcId = DUMMY_PARTY_ID;
	WORD  key;

	*pParam >> partyRsrcId;
	*pParam >> key;

	if (IsChair(partyRsrcId))
	{
		PTRACE2INT(eLevelInfoNormal, " found chair in local list, setting it as m_currentChair - rsrc id: ", partyRsrcId);
		m_currentChairRsrcId = partyRsrcId;
		CHAIRS_MAP::iterator it = chairpersonsMap.find(partyRsrcId);
		CPartyApi* pPartyApi = it->second;
		m_pPartyApi->CreateOnlyApi(pPartyApi->GetRcvMbx());

		if (IsPartyOutConnected(m_currentChairRsrcId))
		{
			m_pChair = GetChairPartyTaskApp(partyRsrcId);
			if (IsValidPObjectPtr(m_pChair))
			{
				m_currentChairName = ((CParty*)m_pChair)->GetName();
			}
			else
			{
				PASSERT(1);
			}

			m_tmpKey = key;
			AllocatePcmRsrc();
		}
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnFeccKeyAllocatePcm(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyAllocatePcm FECC will be ignored until we get response from Rsrc!!!!");
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnFeccKeyMenuSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyMenuSetup");
	OnFeccKey(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnFeccKeyMenuOn(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyMenuOn");
	OnFeccKey(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnFeccKeyHidingMenu(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManager::OnFeccKeyHidingMenu");
	OnFeccKey(pParam);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::FeccKeyRecievedFromWrongChair(DWORD partyRsrcId)
{
	if (IsChair(partyRsrcId))
	{
		CMessageOverlayInfo* msg = new CMessageOverlayInfo;
		msg->SetMessageText("Maximum number of PCM sessions exceeded");
		msg->SetMessageOnOff(TRUE);
		msg->SetMessageFontSize_Old(eMedium);
		msg->SetMessageNumOfRepetitions(1);

		m_pVideoBridgeInterface->UpdateMessageOverlayForParty(partyRsrcId, msg);

		POBJDELETE(msg);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::DeAllocatePcmRsrc()
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::DeAllocatePcmRsrc===========");
	m_state = DEALLOCATE_PCM_RSRC;

	DetachEvents();

	ALLOC_PCM_RSRC_REQ_PARAMS_S allocPcmStruct;

	allocPcmStruct.rsrc_conf_id  = m_pConf->GetConfId();
	allocPcmStruct.rsrc_party_id = m_currentChairRsrcId;

	CMedString cstr;
	cstr << "ALLOC_PCM_RSRC_REQ_PARAMS_S(used also for deallocate req)";
	cstr << "\nrsrc_conf_id: " << allocPcmStruct.rsrc_conf_id;
	cstr << "\nrsrc_party_id: " << allocPcmStruct.rsrc_party_id;
	PTRACE(eLevelInfoNormal, cstr.GetString());


	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&allocPcmStruct), sizeof(ALLOC_PCM_RSRC_REQ_PARAMS_S));

	STATUS    res = SendReqToResourceAllocator(seg, DEALLOCATE_PCM_RSRC_REQ);

	PASSERT(res);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::AllocatePcmRsrc()
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::AllocatePcmRsrc===========");

	StartTimer(PCM_SETUP_TOUT, PCM_SETUP_TOUT_VALUE);
	m_state = ALLOCATE_PCM_RSRC;
	ALLOC_PCM_RSRC_REQ_PARAMS_S allocPcmStruct;

	allocPcmStruct.rsrc_conf_id  = m_pConf->GetConfId();
	allocPcmStruct.rsrc_party_id = m_currentChairRsrcId;

	CMedString cstr;
	cstr << "ALLOC_PCM_RSRC_REQ_PARAMS_S";
	cstr << "\nrsrc_conf_id: " << allocPcmStruct.rsrc_conf_id;
	cstr << "\nrsrc_party_id: " << allocPcmStruct.rsrc_party_id;

	PTRACE(eLevelInfoNormal, cstr.GetString());

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&allocPcmStruct), sizeof(ALLOC_PCM_RSRC_REQ_PARAMS_S));

	STATUS    res = SendReqToResourceAllocator(seg, ALLOCATE_PCM_RSRC_REQ);

	PASSERT(res);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnRsrsAllocPcmRspAllocate(CSegment* pParam)
{
	if (IsValidTimer(PCM_SETUP_TOUT))
		DeleteTimer(PCM_SETUP_TOUT);

	ALLOC_PCM_RSRC_IND_PARAMS_S allocatedPcmStruct;

	pParam->Get((BYTE*)&allocatedPcmStruct, sizeof(ALLOC_PCM_RSRC_IND_PARAMS_S));

	CMedString                  cstr;
	cstr << "CMcmsPCMManagerCP::OnRsrsAllocPcmRspAllocate\n";
	cstr << "ALLOC_PCM_RSRC_IND_PARAMS_S: ";
	cstr << "\nstatus: " << allocatedPcmStruct.status;
	cstr << "\nrsrc_conf_id: " << allocatedPcmStruct.rsrc_conf_id;
	cstr << "\nrsrc_party_id: " << allocatedPcmStruct.rsrc_party_id;
	cstr << "\nallocatedPcmRsrc.connectionId:    " << allocatedPcmStruct.allocatedPcmRsrc.connectionId;
	cstr << "\nallocatedPcmRsrc.logicalRsrcType: " << ::LogicalResourceTypeToString(allocatedPcmStruct.allocatedPcmRsrc.logicalRsrcType);
	cstr << "\npcmMenuId: " << allocatedPcmStruct.pcmMenuId;

	PTRACE(eLevelInfoNormal, cstr.GetString());

	if (allocatedPcmStruct.status != STATUS_OK)
	{
		PASSERT(allocatedPcmStruct.status);
		// send message overlay "no pcm resources..."
		FeccKeyRecievedFromWrongChair(m_currentChairRsrcId);
		m_state = MENU_OFF;
	}

	if (allocatedPcmStruct.rsrc_conf_id != m_pConf->GetConfId() || allocatedPcmStruct.rsrc_party_id != m_currentChairRsrcId)
	{
		PASSERTMSG(1, "inconsistent parameters between the req and the ind (conf id / party id)");
		// send message overlay "no pcm resources..."
		FeccKeyRecievedFromWrongChair(m_currentChairRsrcId);
		m_state = MENU_OFF;
		return;
	}

	if (m_needToFreeRsrcDisconnectPcm)
	{
		PTRACE(eLevelInfoNormal, "PCM resource is no longer needed (chair left/moved , error?) , deallocate pcm");
		DeAllocatePcmRsrc();
		return;
	}

	m_state = MENU_SETUP;
	m_terminalId = allocatedPcmStruct.pcmMenuId;

	if (m_terminalId >= MAX_NUM_OF_MENUS)
		PASSERTMSG(m_terminalId, "illegal term_id for pcm menu");

	m_termListInfoIndication->SetTerminalId(m_terminalId);

	if (IsValidPObjectPtr(m_pCommConf))
	{
		AttachEvents();
	}
	else
	{
		PASSERTMSG(1, "CMcmsPCMManagerCP::OnRsrsAllocPcmRspAllocate can not find m_pCommConf in CommConfDb");
		// self kill???
	}

	ALLOCATED_RSRC_PARAM_S allocatedPcmRsrc = allocatedPcmStruct.allocatedPcmRsrc;
	m_pRsrcParams = new CRsrcParams(allocatedPcmRsrc.connectionId,
	                                m_currentChairRsrcId,
	                                m_confRsrcId,
	                                allocatedPcmRsrc.logicalRsrcType);


	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (NULL == pRoutingTbl)
	{
		PASSERT_AND_RETURN(103);
	}

	CPartyRsrcRoutingTblKey routingKey(*m_pRsrcParams);
	pRoutingTbl->AddPartyRsrcDesc(routingKey);

	CTaskApi* pTaskApiMcmsPcmManager = new CTaskApi(*m_pConfApi);

	pTaskApiMcmsPcmManager->CreateOnlyApi(m_pConfApi->GetRcvMbx(), this);
	// pRoutingTbl->DumpTable();
	int stat = pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pRsrcParams, pTaskApiMcmsPcmManager);
	PASSERT(stat);
	// pRoutingTbl->DumpTable();

	m_pPcmHardwareInterface = new CPCMHardwareInterface(*m_pRsrcParams);

	POBJDELETE(pTaskApiMcmsPcmManager);

	// inform party cntl pcm is connected (rsrc was allocated - need to deallocate in disconnection)
	m_pConfApi->InformPartyControlOnPcmState(m_pChair->GetPartyId(), TRUE);

	CSegment* seg = new CSegment;
	*seg << m_currentChairRsrcId << m_tmpKey;

	SendDataIndications();
	SendDirectLoginConf();
	OnFeccKey(seg);

	POBJDELETE(seg);

	TRACEINTO << "Start MENU_SETUP Timer";
	StartTimer(PCM_SETUP_TOUT, PCM_SETUP_TOUT_VALUE);	// BRIDGE-6911 in case PCM (Embedded) not response with PCM_COMMAND
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnRsrsDeAllocPcmRspDeAllocate(CSegment* pParam)
{
	DEALLOC_PARTY_IND_PARAMS_S deAllocatedPcmStruct;

	pParam->Get((BYTE*)&deAllocatedPcmStruct, sizeof(DEALLOC_PARTY_IND_PARAMS_S));

	CMedString cstr;
	cstr << "CMcmsPCMManagerCP::OnRsrsDeAllocPcmRspDeAllocate";
	cstr << "\nDEALLOC_PARTY_IND_PARAMS_S: ";
	cstr << "\nstatus: " << deAllocatedPcmStruct.status;
	PTRACE(eLevelInfoNormal, cstr.GetString());

	if (deAllocatedPcmStruct.status != STATUS_OK)
	{
		PASSERT(deAllocatedPcmStruct.status);
	}

	// inform party cntl pcm is disconnected (rsrc was deallocated - no need to deallocate in disconnection)
	m_pConfApi->InformPartyControlOnPcmState(m_pChair->GetPartyId(), FALSE);

	m_currentChairRsrcId = DUMMY_PARTY_ID;
	m_currentChairName   = "";
	m_pChair             = NULL;

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (NULL == pRoutingTbl)
	{
		PASSERT_AND_RETURN(103);
	}

	if (m_pRsrcParams)
	{
		pRoutingTbl->RemovePartyRsrc(*m_pRsrcParams);
	}

	POBJDELETE(m_pPcmHardwareInterface);
	POBJDELETE(m_pRsrcParams);

	if (chairpersonsMap.size() > 0)
		m_state = MENU_OFF;
	else
		m_state = IDLE;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManagerCP::SendReqToResourceAllocator(CSegment* seg, OPCODE opcode)
{
	CProcessBase* process = CProcessBase::GetProcess();
	if (!process)
	{
		PASSERT(101);
	}

	CManagerApi api(eProcessResource);
	const StateMachineDescriptor stateMachine = GetStateMachineDescriptor();

	STATUS res = api.SendMsg(seg, opcode, &m_pConfApi->GetRcvMbx(), &stateMachine);

	return res;
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::SendMenuIndications()
{
	CPcmInitialMenuBufferIndication initialMenuIndication(m_terminalId);
	SendPcmIndication(&initialMenuIndication);

	CPcmMenuStateIndication menuStateIndication(m_terminalId, /*isActive*/ true, /* isDisenable */ true, /*isCopActive */ false); // default value is menu on
	SendPcmIndication(&menuStateIndication);
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::ConnectPartyToPCMEncoder(DWORD partyId)
{
	if (IsPartyOutConnected(partyId))
	{
		if (partyId != DUMMY_PARTY_ID)
		{
			// VNGR-15750-fix
			m_pVideoBridgeInterface->UpdateMessageOverlayStopForParty(m_currentChairRsrcId);

			m_state = MENU_SETUP;

			CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
			if (NULL == pRoutingTbl)
			{
				PASSERT_AND_RETURN(103);
			}

			StartTimer(PCM_SETUP_TOUT, PCM_SETUP_TOUT_VALUE);
			CRsrcDesc* partiesEncoderRsrcDesc = pRoutingTbl->GetPartyRsrcDesc(partyId, eLogical_video_encoder);
			if (NULL == partiesEncoderRsrcDesc) {
				PASSERT_AND_RETURN(104);
			}

			m_pPcmHardwareInterface->SendConnectPCMMenu(partiesEncoderRsrcDesc->GetConnectionId(), m_terminalId);
			ON(m_wasConnectPcmSent);
			// need to close with Arik!!!
			// m_pVideoBridgeInterface->ConnectPartyToPCMEncoder(partyId);
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal, "**CMcmsPCMManagerCP::ConnectPartyToPCMEncoder, party is not connected to video encoder!!!");
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::DisconnectPartyFromPCMEncoder(DWORD partyId)
{
	if (partyId != DUMMY_PARTY_ID)
	{
		m_state = HIDING_MENU;
		m_pPartyApi->PartyDisconnectedFromPCM();   // stop forward DTMF to PCM module

		if (m_bPcmFeccEnable == NO)
			m_pPartyApi->PartyActionsOnLeaderChanged(FALSE);

		if (!IsPartyOutConnected(partyId))
			PTRACE(eLevelInfoNormal, "======CMcmsPCMManagerCP::DisconnectPartyFromPCMEncoder party is not connected to video out! ");

		CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
		if (NULL == pRoutingTbl)
		{
			PASSERT(103);
			DeAllocatePcmRsrc();
			return;
		}

		CRsrcDesc* partiesEncoderRsrcDesc = pRoutingTbl->GetPartyRsrcDesc(partyId, eLogical_video_encoder);
		if (NULL == partiesEncoderRsrcDesc)
		{
			PASSERT(104);
			DeAllocatePcmRsrc();
			return;
		}

		m_pPcmHardwareInterface->SendDisconnectPCMMenu(partiesEncoderRsrcDesc->GetConnectionId(), m_terminalId);

		// need to close with Arik!!!
		// m_pVideoBridgeInterface->DisconnectPartyFromPCMEncoder(partyId);
	}
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnEndPartyDisconnectFromPCMEncoderHidingMenu(CSegment* pParam)
{
	STATUS status;
	*pParam >> status;

	std::ostringstream msg;
	msg << "CMcmsPCMManagerCP::OnEndPartyDisconnectFromPCMEncoderHidingMenu:"
	    << "\n  status            = " << CProcessBase::GetProcess()->GetStatusAsString(status) << ((status) ? " (DSP answered with status failure because the port already closed VNGR-20705)" : "")
	    << "\n  menuState         = " << m_menuState << (m_menuState ? " (PCM module did not send pop_menu_status=0 yet)" : "");

	PTRACE(eLevelInfoNormal, msg.str().c_str());
	DeAllocatePcmRsrc();
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnEndPartyDisconnectFromPCMEncoderAnycase(CSegment* pParam)
{
	STATUS     status;
	*pParam >> status;

	CMedString cstr;
	cstr << "*** SHOULD NOT GET HERE!!! (this opcode expected at state: MENU_ON , current state: " << StateToString(m_state) << "\n";
	cstr << "=====CMcmsPCMManagerCP::OnEndPartyDisconnectFromPCMEncoderAnycase  , status: " << CProcessBase::GetProcess()->GetStatusAsString(status);
	PTRACE(eLevelInfoNormal, cstr.GetString());

	PASSERT(status);

	if (m_menuState)
		PTRACE(eLevelInfoNormal, "CMcmsPCMManagerCP::OnEndPartyDisconnectFromPCMEncoderAnycase PCM module did not send pop_menu_status = 0 yet!!!");

	DeAllocatePcmRsrc();
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManagerCP::OnPopMenuStatus(CPcmCommand* pcmCommand)
{
	CMedString cstr;
	cstr << "=====CMcmsPCMManagerCP::OnPopMenuStatus======";
	m_menuState = ((CPcmPopMenuStatusCommand*)pcmCommand)->GetStatus();
	cstr << "\nmenu state: " << m_menuState << "  state: " << m_state;
	PTRACE(eLevelInfoNormal, cstr.GetString());

	// Temp - ToDo Eitan add this event to state machine!!
	switch (m_state)
	{
		case (MENU_SETUP):
		{
			if (m_menuState)
				ConnectPartyToPCMEncoder(m_currentChairRsrcId);
			else if (m_wasConnectPcmSent)
				DisconnectPartyFromPCMEncoder(m_currentChairRsrcId); // ????
			break;
		}

		case (MENU_ON):
		{
			if (!m_menuState)
				DisconnectPartyFromPCMEncoder(m_currentChairRsrcId);
			break;
		}

		default:
			break;
	} // switch

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManagerCP::OnSetDisplaySetting(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnSetDisplaySetting======");

	int paneIndex = ((CPcmSetDisplaySettingCommand*)pcmCommand)->GetPaneIndex();

	CLayout* confLayout = m_pVideoBridgeInterface->GetPartyLayout(m_currentChairName.c_str());

	if (confLayout != NIL(CLayout))
	{
		int cellToForce = pcmCommand->TranslatePcmPaneIndexToMcmsSubImageId(confLayout->GetLayoutType(), paneIndex);
		if (cellToForce == -1)
		{
			PTRACE(eLevelInfoNormal, "Invalid image id");
			return STATUS_FAIL;
		}

		DWORD partyIdToForce = m_pConfAppMngrInterface->GetPartyToForce(m_currentChairRsrcId);
		if (partyIdToForce != 0)
		{
			m_pVideoBridgeInterface->PLC_ForceToCell(m_currentChairRsrcId, partyIdToForce, cellToForce);
		}
		return STATUS_OK;
	}
	else
	{
		return STATUS_FAIL;
	}
}

//--------------------------------------------------------------------------
STATUS CMcmsPCMManagerCP::OnSetCpLayout(CPcmCommand* pcmCommand)
{
	PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnSetCpLayout======");

	int layoutType = ((CPcmSetCpLayoutCommand*)pcmCommand)->GetLayoutType();
	switch (layoutType)
	{
		case (0): // return to conf layout
		{
			PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnSetCpLayout case 0 - set conference layuot =============");
			m_pConfApi->SetVideoPrivateLayoutButtonOnly(m_currentChairName.c_str(), NO);
			// SetVideoConfLayoutSeeMeParty??
			break;
		}

		case (1): // auto layout
		{
			PTRACE(eLevelInfoNormal, "=====CMcmsPCMManagerCP::OnSetCpLayout case 1 - set auto layuot(conference?) =============");
			m_pConfApi->UpdateAutoLayout(YES);
			break;
		}

		default:
		{
			BYTE apiLayoutType = ((CPcmSetCpLayoutCommand*)pcmCommand)->GetApiLayoutType();
			if (apiLayoutType != CP_NO_LAYOUT)
			{
				CVideoLayout videoLayout;
				videoLayout.SetScreenLayout(apiLayoutType);

				m_pConfApi->SetVideoPrivateLayout(m_currentChairName.c_str(), videoLayout);
			}
			else
			{
				PTRACE(eLevelInfoNormal, "CMcmsPCMManagerCP::OnSetCpLayout, layout type sent from PCM is not supported in RMX!");
				return STATUS_FAIL;
			}
			break;
		}
	} // switch

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::OnConfDisconnectAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CMcmsPCMManagerCP::OnConfDisconnectAnycase:  Name - ", m_pConfName);
	CHAIRS_MAP::iterator it = chairpersonsMap.begin();
	while (it != chairpersonsMap.end())
	{
		CSegment seg;
		seg << it->first;
		DispatchEvent(CHAIRPERSON_LEFT, &seg);

		it = chairpersonsMap.begin();
	}

	// DetachEvents();
}

//--------------------------------------------------------------------------
BYTE CMcmsPCMManagerCP::IsChair(DWORD partyRsrcId)
{
	if (chairpersonsMap.find(partyRsrcId) != chairpersonsMap.end())
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::PrintChairsMap()
{
	std::ostringstream msg;
	msg << "CMcmsPCMManagerCP::PrintChairsMap\n";
	msg << "=================================\n";
	for (CHAIRS_MAP::iterator _ii = chairpersonsMap.begin(); _ii != chairpersonsMap.end(); ++_ii)
		msg << "PartyId: " << _ii->first << " PartyApi: " << (DWORD)_ii->second << "\n";

	PTRACE(eLevelInfoNormal, msg.str().c_str());
}

//--------------------------------------------------------------------------
void CMcmsPCMManagerCP::ClearChairsMap()
{
	CHAIRS_MAP::iterator it = chairpersonsMap.begin();

	while (it != chairpersonsMap.end())
	{
		CPartyApi* pPartyApi = it->second;
		pPartyApi->DestroyOnlyApi();
		POBJDELETE(pPartyApi);
		chairpersonsMap.erase(it);
		it = chairpersonsMap.begin();
	}

	chairpersonsMap.clear();
}

