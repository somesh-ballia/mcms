#include "NStream.h"
#include "IVRService.h"
#include "ConfPartyDefines.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "Macros.h"
#include "StringsMaps.h"
#include "ConfPartyGlobals.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "OsFileIF.h"
#include "ApiStatuses.h"
#include <sys/stat.h>
#include "FileList.h"
#include "ApiStatuses.h"
#include "ConfPartyStatuses.h"
#include "SysConfigKeys.h"
#include "IVRSlidesList.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////
//                        CAVmsgService
////////////////////////////////////////////////////////////////////////////
CAVmsgService::CAVmsgService(WORD isRecordingSystem)
{
	m_serviceName[0]     = '\0';
	m_video_file_name[0] = '\0';
	m_pIVRService        = new CIVRService(isRecordingSystem);
}

//--------------------------------------------------------------------------
CAVmsgService::CAVmsgService(const CAVmsgService& other)
              :CPObject(other)
{
	m_pIVRService = NULL;

	*this = other;
}

//--------------------------------------------------------------------------
CAVmsgService& CAVmsgService::operator=(const CAVmsgService& other)
{
	SetName(other.m_serviceName);
	SetVideoFileName(other.m_video_file_name);

	PDELETE(m_pIVRService);
	if (other.m_pIVRService == NULL)
		m_pIVRService = new CIVRService(0);   // should not happened
	else
		m_pIVRService = new CIVRService(*other.m_pIVRService);

	return *this;
}

//--------------------------------------------------------------------------
CAVmsgService::~CAVmsgService()
{
	PDELETE(m_pIVRService);
}

//--------------------------------------------------------------------------
int CAVmsgService::SelfCorrections()
{
	if (m_pIVRService)
		m_pIVRService->SelfCorrections();

	return 0;
}

//--------------------------------------------------------------------------
int CAVmsgService::IsLegalService(WORD chkLevel, string& err)
{
	// checks IVR legality

	// checking service name
	int status = IsLegalServiceName(err);

	// checks IVR Service
	if (status == STATUS_OK)
	{
		if (m_pIVRService)
			status = m_pIVRService->IsLegalService(chkLevel, err, m_video_file_name);
	}

	return status;
}

//--------------------------------------------------------------------------
int CAVmsgService::IsLegalServiceName(string& err)
{
	int len = strlen(m_serviceName);

	if (len >= AV_MSG_SERVICE_NAME_LEN)
	{
		err = "Illegal IVR Service Name Len";
		return STATUS_ILLEGAL_SERVICE_NAME;
	}

	if (0 == len)
	{
		err = "IVR Service Name is missing";
		return STATUS_ILLEGAL_SERVICE_NAME;
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
char* CAVmsgService::Serialize(WORD format, DWORD apiNum)
{
	return NULL;
}

//--------------------------------------------------------------------------
void CAVmsgService::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
}

//--------------------------------------------------------------------------
void CAVmsgService::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CAVmsgService::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pServiceNode = pFatherNode->AddChildNode("IVR_SERVICE");

	// serialize common fields
	CXMLDOMElement* pCommonNode = pServiceNode->AddChildNode("AV_COMMON");
	pCommonNode->AddChildNode("SERVICE_NAME", m_serviceName);
	pCommonNode->AddChildNode("VIDEO_FILE_NAME", m_video_file_name);

	if (m_pIVRService)
		m_pIVRService->SerializeXml(pServiceNode);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CAVmsgService::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pAvCommonChild = NULL;
	CXMLDOMElement* pIvrChild      = NULL;

	GET_MANDATORY_CHILD_NODE(pActionNode, "AV_COMMON", pAvCommonChild)
	GET_CHILD_NODE(pActionNode, "IVR_PARAMS", pIvrChild)

	if (pAvCommonChild)
	{
		GET_VALIDATE_MANDATORY_CHILD(pAvCommonChild, "SERVICE_NAME", m_serviceName, _1_TO_AV_MSG_SERVICE_NAME_LENGTH);
		GET_VALIDATE_CHILD(pAvCommonChild, "VIDEO_FILE_NAME", m_video_file_name, _0_TO_NEW_FILE_NAME_LENGTH);
	}

	GET_CHILD_NODE(pActionNode, "IVR_PARAMS", pIvrChild);
	nStatus = m_pIVRService->DeSerializeXml(pIvrChild, pszError);
	if (nStatus != STATUS_OK)
		return nStatus;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
const char* CAVmsgService::GetName() const
{
	return m_serviceName;
}

//--------------------------------------------------------------------------
void CAVmsgService::SetName(const char* name)
{
	strcpy_safe(m_serviceName, name);
}

//--------------------------------------------------------------------------
WORD CAVmsgService::IsMusic() const
{
	return TRUE;
}

//--------------------------------------------------------------------------
const char* CAVmsgService::GetVideoFileName() const
{
	return m_video_file_name;
}

//--------------------------------------------------------------------------
void CAVmsgService::SetVideoFileName(const char* name)
{
	strcpy_safe(m_video_file_name, name);
}


//--------------------------------------------------------------------------
WORD CAVmsgService::GetIVRServiceFlag() const
{
	return TRUE;
}

//--------------------------------------------------------------------------
const CIVRService* CAVmsgService::GetIVRService() const
{
	return m_pIVRService;
}

//--------------------------------------------------------------------------
void CAVmsgService::SetIVRService(const CIVRService& other)
{
	PDELETE(m_pIVRService);
	m_pIVRService = new CIVRService(other);
}

//--------------------------------------------------------------------------
void CAVmsgService::SetDtmfCodesLen()
{
	if (!m_pIVRService)
		return;

	CDTMFCodeList* pCDTMFCodeList = m_pIVRService->m_pDTMF_codeList;
	if (!pCDTMFCodeList)
		return;

	pCDTMFCodeList->SetDtmfCodesLen();
}

//--------------------------------------------------------------------------
WORD CAVmsgService::CheckLegalDtmfTbl()
{
	if (m_pIVRService)
	{
		CDTMFCodeList* pCDTMFCodeList = m_pIVRService->m_pDTMF_codeList;
		if (pCDTMFCodeList)
			return pCDTMFCodeList->CheckLegalDtmfTbl();
	}

	return 0;
}

//--------------------------------------------------------------------------
WORD CAVmsgService::CheckLegalOperatorAssistance()
{
	if (m_pIVRService)
	{
		const CIVRWelcomeFeature* wf = m_pIVRService->GetWelcomeFeature();
		if (wf)
			if (wf->GetEnableDisable())                     // feature ON
				if (wf->GetWaitForOperatorAfterMsgOnOff())    // 'wait for operator' ON
				{
					const CIVROperAssistanceFeature* assist = m_pIVRService->GetOperAssistanceFeature();
					if (assist)
						if (!assist->GetEnableDisable())
							return 1;
				}
	}

	return 0;
}

//--------------------------------------------------------------------------
const char* CAVmsgService::GetSlideName()
{
	if (m_pIVRService)
	{
		const CIVRVideoFeature* video = m_pIVRService->GetVideoFeature();
		if (video)
		{
			if (video->GetEnableDisable()) // feature ON
			{
				if (0 < strlen(m_video_file_name))
					return m_video_file_name;
			}
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------
BOOL CAVmsgService::IsIdenticalToCurrent(CAVmsgService* other)
{
	if (0 != strncmp(m_serviceName, other->m_serviceName, AV_MSG_SERVICE_NAME_LEN))
		return FALSE;

	if (0 != strncmp(m_video_file_name, other->m_video_file_name, NEW_FILE_NAME_LEN))
		return FALSE;

	if (FALSE == m_pIVRService->IsIdenticalToCurrent(other->m_pIVRService))
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRService
////////////////////////////////////////////////////////////////////////////
CIVRService::CIVRService(WORD isRecordingSystem)
{
	m_serviceName[0]       = '\0';
	m_pLangMenu            = new CIVRLangMenuFeature;
	m_pWelcome             = new CIVRWelcomeFeature;
	m_pConfPassword        = new CIVRConfPasswordFeature;
	m_pConfLeader          = new CIVRConfLeaderFeature;
	m_pPersonalPINCode     = new CIVRPersonalPINCodeFeature;
	m_pOperAssistance      = new CIVROperAssistanceFeature;
	m_pGeneralMsgs         = new CIVRGeneralMsgsFeature;
	m_pBillingCode         = new CIVR_BillingCodeFeature;
	m_pInviteParty         = new CIVRInvitePartyFeature;
	m_pRollCall            = new CIVRRollCallFeature;
	m_pVideo               = new CIVRVideoFeature;
	m_pNumericId           = new CIVRNumericConferenceIdFeature;
	m_pMuteNoisyLine       = new CIVRMuteNoisyLineFeature;
	m_pRecording           = new CIVRRecordingFeature;
	m_pPlayback            = new CIVRPlaybackFeature;
	m_numb_of_language     = 0;
	m_retries_num          = 3;
	m_user_input_timeout   = 5; // sec
	m_szDelimiter[0]       = '#';
	m_szDelimiter[1]       = '\0';
	m_ind_language         = 0;
	m_bIsEntryQueueService = FALSE;
	m_wIvrExternalDB       = IVR_EXTERNAL_DB_NONE;
	m_pDTMF_codeList       = new CDTMFCodeList;

	AddDTMFCodesToList(m_pDTMF_codeList, isRecordingSystem);

	for (int i = 0; i < MAX_LANG_IN_IVR_SERVICE; i++)
		m_pIVRLanguage[i] = NULL;
}

//--------------------------------------------------------------------------
void CIVRService::AddDTMFCodesToList(CDTMFCodeList* pCDTMFCodeList, WORD isRecordingSystem)
{
	// Fill the DTMF table
	CDTMFCode DTMFCode;
    bool isSoftMcu = (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily()) ? true: false;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	bool isGesherNinja = (curProductType == eProductTypeGesher || curProductType == eProductTypeNinja) ? true: false;

	// private operator assistance
	DTMFCode.SetDTMFOpcode(DTMF_OPER_ASSISTANCE_PRIVATE);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("*0");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// public operator assistance
	DTMFCode.SetDTMFOpcode(DTMF_OPER_ASSISTANCE_PUBLIC);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("00");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	DTMFCode.SetDTMFOpcode(DTMF_SELF_MUTE);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("*6");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	DTMFCode.SetDTMFOpcode(DTMF_SELF_UNMUTE);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("#6");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	DTMFCode.SetDTMFOpcode(DTMF_SECURE_CONF);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("*71");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	DTMFCode.SetDTMFOpcode(DTMF_UNSECURE_CONF);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("#71");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	if ((!isSoftMcu) || isGesherNinja)  // Ninja WW1 support
	{
		DTMFCode.SetDTMFOpcode(DTMF_INC_SELF_VOLUME);
		DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
		DTMFCode.SetDTMFStr("*9");
		pCDTMFCodeList->AddDTMFCode(DTMFCode);

		DTMFCode.SetDTMFOpcode(DTMF_DEC_SELF_VOLUME);
		DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
		DTMFCode.SetDTMFStr("#9");
		pCDTMFCodeList->AddDTMFCode(DTMFCode);
	}

	DTMFCode.SetDTMFOpcode(DTMF_MUTE_ALL_BUT_X);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("*5");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	DTMFCode.SetDTMFOpcode(DTMF_UNMUTE_ALL_BUT_X);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("#5");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	DTMFCode.SetDTMFOpcode(DTMF_CHANGE_PASSWORD);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("*77");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	DTMFCode.SetDTMFOpcode(DTMF_MUTE_INCOMING_PARTIES);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("*86");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	DTMFCode.SetDTMFOpcode(DTMF_UNMUTE_INCOMING_PARTIES);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("#86");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Help Menu
	DTMFCode.SetDTMFOpcode(DTMF_PLAY_MENU);
	DTMFCode.SetDTMFStr("*83");
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Roll Call Names Review
	DTMFCode.SetDTMFOpcode(DTMF_ENABLE_ROLL_CALL);
	DTMFCode.SetDTMFStr("*42");
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Roll Call Stop Names Review
	DTMFCode.SetDTMFOpcode(DTMF_DISABLE_ROLL_CALL);
	DTMFCode.SetDTMFStr("#42");
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Roll Call Names Review
	DTMFCode.SetDTMFOpcode(DTMF_ROLL_CALL_REVIEW_NAMES);
	DTMFCode.SetDTMFStr("*43");
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Roll Call Stop Names Review
	DTMFCode.SetDTMFOpcode(DTMF_ROLL_CALL_STOP_REVIEW_NAMES);
	DTMFCode.SetDTMFStr("#43");
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Terminate Conference
	DTMFCode.SetDTMFOpcode(DTMF_CONF_TERMINATE);
	DTMFCode.SetDTMFStr("*87");
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Video Commander
	DTMFCode.SetDTMFOpcode(DTMF_START_VC);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("**");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Venus
	DTMFCode.SetDTMFOpcode(DTMF_START_VENUS);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("*#");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Invite party
	DTMFCode.SetDTMFOpcode(DTMF_INVITE_PARTY);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("*72");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Disconnect invited participant
	DTMFCode.SetDTMFOpcode(DTMF_DISCONNECT_INVITED_PARTICIPANT);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("#72");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);


	// Change regular party to leader
	DTMFCode.SetDTMFOpcode(DTMF_CHANGE_TO_LEADER);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("*78");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	if ((!isSoftMcu) || isGesherNinja)	  // Ninja WW1 support
	{
		// Listening Volume
		DTMFCode.SetDTMFOpcode(DTMF_INC_LISTEN_VOLUME);
		DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
		DTMFCode.SetDTMFStr("*76");
		pCDTMFCodeList->AddDTMFCode(DTMFCode);

		// Listening Volume
		DTMFCode.SetDTMFOpcode(DTMF_DEC_LISTEN_VOLUME);
		DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
		DTMFCode.SetDTMFStr("#76");
		pCDTMFCodeList->AddDTMFCode(DTMFCode);
	}

	// Override mute all
	DTMFCode.SetDTMFOpcode(DTMF_OVERRIDE_MUTE_ALL);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Recording - Start / Resume
	DTMFCode.SetDTMFOpcode(DTMF_START_RESUME_RECORDING);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("*2");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Recording - Stop
	DTMFCode.SetDTMFOpcode(DTMF_STOP_RECORDING);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("*3");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);

	// Recording - Pause
	DTMFCode.SetDTMFOpcode(DTMF_PAUSE_RECORDING);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("*1");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);


	// Recording - Pause
	DTMFCode.SetDTMFOpcode(DTMF_SHOW_PARTICIPANTS);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("*88");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);


	// Request to Speak
	DTMFCode.SetDTMFOpcode(DTMF_REQUEST_TO_SPEAK);
	DTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
	DTMFCode.SetDTMFStr("99");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);


	// Start PCM
	DTMFCode.SetDTMFOpcode(DTMF_START_PCM);
	DTMFCode.SetDTMFPermission(DTMF_LEADER_ACTION);
	DTMFCode.SetDTMFStr("##");
	pCDTMFCodeList->AddDTMFCode(DTMFCode);
}

//--------------------------------------------------------------------------
CIVRService::CIVRService(const CIVRService& other)
            : CPObject(other)
{
	m_pDTMF_codeList   = NULL;
	m_pLangMenu        = NULL;
	m_pWelcome         = NULL;
	m_pConfPassword    = NULL;
	m_pConfLeader      = NULL;
	m_pPersonalPINCode = NULL;
	m_pOperAssistance  = NULL;
	m_pGeneralMsgs     = NULL;
	m_pBillingCode     = NULL;
	m_pInviteParty     = NULL;
	m_pRollCall        = NULL;
	m_pVideo           = NULL;
	m_pNumericId       = NULL;
	m_pMuteNoisyLine   = NULL;
	m_pRecording       = NULL;
	m_pPlayback        = NULL;

	for (int i = 0; i < MAX_LANG_IN_IVR_SERVICE; i++)
		m_pIVRLanguage[i] = NULL;

	*this = other;
}

//--------------------------------------------------------------------------
CIVRService& CIVRService::operator=(const CIVRService& other)
{
	strncpy(m_serviceName, other.m_serviceName, AV_MSG_SERVICE_NAME_LEN);

	PDELETE(m_pDTMF_codeList);
	if (other.m_pDTMF_codeList)
		m_pDTMF_codeList = new CDTMFCodeList(*other.m_pDTMF_codeList);

	PDELETE(m_pLangMenu);
	if (other.m_pLangMenu)
		m_pLangMenu = new CIVRLangMenuFeature(*other.m_pLangMenu);

	PDELETE(m_pWelcome);
	if (other.m_pWelcome)
		m_pWelcome = new CIVRWelcomeFeature(*other.m_pWelcome);

	PDELETE(m_pConfPassword);
	if (other.m_pConfPassword)
		m_pConfPassword = new CIVRConfPasswordFeature(*other.m_pConfPassword);

	PDELETE(m_pConfLeader);
	if (other.m_pConfLeader)
		m_pConfLeader = new CIVRConfLeaderFeature(*other.m_pConfLeader);

	PDELETE(m_pPersonalPINCode);
	if (other.m_pPersonalPINCode)
		m_pPersonalPINCode = new CIVRPersonalPINCodeFeature(*other.m_pPersonalPINCode);

	PDELETE(m_pOperAssistance);
	if (other.m_pOperAssistance)
		m_pOperAssistance = new CIVROperAssistanceFeature(*other.m_pOperAssistance);

	PDELETE(m_pGeneralMsgs);
	if (other.m_pGeneralMsgs)
		m_pGeneralMsgs = new CIVRGeneralMsgsFeature(*other.m_pGeneralMsgs);

	PDELETE(m_pBillingCode);
	if (other.m_pBillingCode)
		m_pBillingCode = new CIVR_BillingCodeFeature(*other.m_pBillingCode);

	PDELETE(m_pInviteParty);
	if (other.m_pInviteParty)
		m_pInviteParty = new CIVRInvitePartyFeature(*other.m_pInviteParty);

	PDELETE(m_pRollCall);
	if (other.m_pRollCall)
		m_pRollCall = new CIVRRollCallFeature(*other.m_pRollCall);

	PDELETE(m_pVideo);
	if (other.m_pVideo)
		m_pVideo = new CIVRVideoFeature(*other.m_pVideo);

	PDELETE(m_pNumericId);
	if (other.m_pNumericId)
		m_pNumericId = new CIVRNumericConferenceIdFeature(*other.m_pNumericId);

	PDELETE(m_pMuteNoisyLine);
	if (other.m_pMuteNoisyLine)
		m_pMuteNoisyLine = new CIVRMuteNoisyLineFeature(*other.m_pMuteNoisyLine);

	PDELETE(m_pRecording);
	if (other.m_pRecording)
		m_pRecording = new CIVRRecordingFeature(*other.m_pRecording);

	PDELETE(m_pPlayback);
	if (other.m_pPlayback)
		m_pPlayback = new CIVRPlaybackFeature(*other.m_pPlayback);

	for (int i = 0; i < MAX_LANG_IN_IVR_SERVICE; i++)
	{
		PDELETE(m_pIVRLanguage[i]);
		if (other.m_pIVRLanguage[i])
			m_pIVRLanguage[i] = new CIVRLanguage(*other.m_pIVRLanguage[i]);
	}

	m_numb_of_language     = other.m_numb_of_language;
	m_ind_language         = other.m_ind_language;
	m_retries_num          = other.m_retries_num;
	m_user_input_timeout   = other.m_user_input_timeout;
	m_bIsEntryQueueService = other.m_bIsEntryQueueService;
	m_wIvrExternalDB       = other.m_wIvrExternalDB;

	strcpy_safe(m_szDelimiter, other.m_szDelimiter);

	return *this;
}

//--------------------------------------------------------------------------
CIVRService::~CIVRService()
{
	PDELETE(m_pDTMF_codeList);
	PDELETE(m_pLangMenu);
	PDELETE(m_pWelcome);
	PDELETE(m_pConfPassword);
	PDELETE(m_pConfLeader);
	PDELETE(m_pPersonalPINCode);
	PDELETE(m_pOperAssistance);
	PDELETE(m_pGeneralMsgs);
	PDELETE(m_pRollCall);
	PDELETE(m_pBillingCode);
	PDELETE(m_pInviteParty);
	PDELETE(m_pVideo);
	PDELETE(m_pNumericId);
	PDELETE(m_pMuteNoisyLine);
	PDELETE(m_pRecording);
	PDELETE(m_pPlayback);

	for (int i = 0; i < MAX_LANG_IN_IVR_SERVICE; i++)
		PDELETE(m_pIVRLanguage[i]);
}

//--------------------------------------------------------------------------
int CIVRService::SelfCorrections()
{
	// correct automatically...
	if (m_user_input_timeout < 2)
		m_user_input_timeout = 2;

	if (m_pNumericId && m_bIsEntryQueueService)
		m_pNumericId->SetEnableDisable(YES);

	if (m_pConfLeader)
		m_pConfLeader->SetConfPwAsLeaderPw(YES);    // in RMX should be always YES

	return 0;
}

int CIVRService::IsLegalServiceForSoftMcu(string& err)
{
	return STATUS_OK;////The IsLegalServiceForSoftMcu validity test was removed in order not to block IBM tests.need to come up with a different solution.
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    bool isSoftMcu = (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily()) ? true: false;
	bool isGesherNinja = (curProductType == eProductTypeGesher || curProductType == eProductTypeNinja) ? true: false;

	if (!isSoftMcu) // if not SoftMcu - skip check
		return STATUS_OK;

	if((!isGesherNinja) && (NULL != m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_INC_SELF_VOLUME)
		|| NULL != m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_DEC_SELF_VOLUME)
		|| NULL != m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_INC_LISTEN_VOLUME)
		|| NULL != m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_DEC_LISTEN_VOLUME)))     // Ninja WW1 support
	{
		err    = " DTMF code not supported in soft mcu";
		return STATUS_IVR_DTMF_CODE_NOT_SUPPORTED_IN_SOFT_MCU;
	}
	else if (NULL != m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_ENABLE_ROLL_CALL)
		|| NULL != m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_DISABLE_ROLL_CALL)
		|| NULL != m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_ROLL_CALL_REVIEW_NAMES)
		|| NULL != m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_ROLL_CALL_STOP_REVIEW_NAMES)
		)

	{
		err    = " DTMF code not supported in soft mcu";
		return STATUS_IVR_DTMF_CODE_NOT_SUPPORTED_IN_SOFT_MCU;
	}
	else if (m_pRollCall->GetEnableDisable()== YES)
	{
		err    = " Roll call not supported in soft mcu";
		return STATUS_IVR_GENERAL_FEATURE_NOT_SUPPORTED_IN_SOFT_MCU;
	}


	return STATUS_OK;
}
//--------------------------------------------------------------------------
int CIVRService::IsLegalService(WORD chkLevel, string& err, char* videoName)
{
	int status = STATUS_OK;

	// find the IVR Service language
	const char*   language     = NULL;
	CIVRLanguage* pIVRLanguage = m_pIVRLanguage[0];
	if (pIVRLanguage)
		language = pIVRLanguage->GetLanguageName();
	else
	{
		TRACEINTO << "CIVRService::IsLegalService - ServiceName:" << m_serviceName << ", Failed, illegal language name";
		err    = " illegal language name";
		status = STATUS_ILLEGAL_LANGUAGE_NAME;
	}

	if (STATUS_OK == status)
		status = IsLegalServiceIVR(err);

	if (STATUS_OK == status)
		status = IsLegalServiceForSoftMcu(err);

	// checks DTMF opcode table (V)
	if (STATUS_OK == status)
		if (m_pDTMF_codeList && !m_bIsEntryQueueService)
			status = m_pDTMF_codeList->IsLegalService(chkLevel, err);

	// checks features (V)
	if (STATUS_OK == status)
		if (m_pWelcome)
			status = m_pWelcome->IsLegalService(chkLevel, err, language);

	// checks features (V)
	if (STATUS_OK == status)
		if (m_pConfPassword && !m_bIsEntryQueueService)
			status = m_pConfPassword->IsLegalService(chkLevel, err, language);

	// checks features (V)
	if (STATUS_OK == status)
		if (m_pConfLeader && !m_bIsEntryQueueService)
			status = m_pConfLeader->IsLegalService(chkLevel, err, language);

	// checks Billing Code (V)
	if (STATUS_OK == status)
		if (!m_bIsEntryQueueService && IsNeedBilling() && m_pGeneralMsgs)
			status = m_pGeneralMsgs->IsLegalBillingCodeMsg(chkLevel, err, language);

	// checks features (V)
	if (STATUS_OK == status)
		if (m_pRollCall && !m_bIsEntryQueueService)
			status = m_pRollCall->IsLegalService(chkLevel, err, language);

	// checks features
	if (STATUS_OK == status)
		if (m_pGeneralMsgs && !m_bIsEntryQueueService)
			status = m_pGeneralMsgs->IsLegalService(chkLevel, err, language);

	// checks features (V)
	if (STATUS_OK == status)
		if (m_pNumericId && m_bIsEntryQueueService)
			status = m_pNumericId->IsLegalService(chkLevel, err, language);

	if (STATUS_OK == status)
		if (m_pVideo)
			status = m_pVideo->IsLegalService(chkLevel, err, language, videoName);

	return status;
}

//--------------------------------------------------------------------------
int CIVRService::IsLegalServiceIVR(string& err)
{
	// checks delimiter
	if (m_szDelimiter[0] != '*' && m_szDelimiter[0] != '#')
	{
		err = " Illegal IVR delimiter";
		return STATUS_IVR_ILLEGAL_PARAMETERS;
	}

	// checks retries number
	if (m_retries_num > 5)
	{
		err = " Illegal IVR retries number";
		return STATUS_IVR_ILLEGAL_PARAMETERS;
	}

	// checks user input timeout in seconds
	if (m_user_input_timeout > 9)
	{
		err = " Illegal IVR user input timeout";
		return STATUS_IVR_ILLEGAL_PARAMETERS;
	}

	// checks externalDB
	if (m_wIvrExternalDB != IVR_EXTERNAL_DB_NONE)
	{
		// checks if enables in system.cfg
		BOOL bIsEnableExternalDBAccess;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_EXTERNAL_DB_ACCESS, bIsEnableExternalDBAccess);
		if (!bIsEnableExternalDBAccess)
		{
			err = " External DB is not configured in system.cfg: ENABLE_EXTERNAL_DB_ACCESS ";
			return STATUS_IVR_EXTERNAL_DB_DISABLED;
		}

		if (m_bIsEntryQueueService)
		{
			if (m_wIvrExternalDB != IVR_EXTERNAL_DB_NUMERIC_ID) // EQ supports only NID with externalDB
			{
				err = " Illegal External DB for EQ IVR";
				return STATUS_INVALID_IVR_EXTERNAL_DB_PARAMETER;
			}
		}
		else
		{
			if (m_wIvrExternalDB != IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD &&
			    m_wIvrExternalDB != IVR_EXTERNAL_DB_CHAIR_PASSWORD) // Conf supports only PW or CPW with externalDB
			{
				err = " Illegal External DB for IVR";
				return STATUS_INVALID_IVR_EXTERNAL_DB_PARAMETER;
			}

			// the chair PW must be configured in any case
			if (m_pConfLeader)
			{
				if (m_pConfLeader->GetEnableDisable() == NO)
				{
					err = " Illegal External DB Configuration, Chair PW not configured";
					return STATUS_INVALID_IVR_EXTERNAL_DB_CONFIGURATION;
				}
			}

			if (m_wIvrExternalDB == IVR_EXTERNAL_DB_CHAIR_PASSWORD)
			{
				// the conf PW must NOT be configured
				if (m_pConfPassword && m_pConfPassword->GetEnableDisable() == YES)
				{
					err = " Illegal External DB Configuration, conference PW cannot be configured";
					return STATUS_INVALID_IVR_EXTERNAL_DB_CONFIGURATION;
				}
			}
			else  // m_wIvrExternalDB == IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD
			{
				// the conf PW must be configured
				if (m_pConfPassword && m_pConfPassword->GetEnableDisable() == NO)
				{
					err = " Illegal External DB Configuration, conference PW not configured";
					return STATUS_INVALID_IVR_EXTERNAL_DB_CONFIGURATION;
				}
			}
		}
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CIVRService::IsNeedBilling()
{
	if (m_bIsEntryQueueService)
		return 0;

	if (m_pConfLeader)
		if (m_pConfLeader->GetEnableDisable())
			if (m_pConfLeader->GetIsBillingCode())
				return 1;

	return 0;
}

//--------------------------------------------------------------------------
int CIVRService::GetBillingMsgIndex()
{
	if (m_pGeneralMsgs)
		return m_pGeneralMsgs->GetBillingMsgIndex();

	return -1;
}

//--------------------------------------------------------------------------
char* CIVRService::Serialize(WORD format, DWORD apiNum)
{
	return NULL;  // added since the content of the function was remarked.
}

//--------------------------------------------------------------------------
void CIVRService::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	m_pDTMF_codeList->Serialize(format, m_ostr, apiNum);
	m_pLangMenu->Serialize(format, m_ostr, apiNum);
	m_pWelcome->Serialize(format, m_ostr, apiNum);
	m_pConfPassword->Serialize(format, m_ostr, apiNum);
	m_pConfLeader->Serialize(format, m_ostr, apiNum);
	m_pPersonalPINCode->Serialize(format, m_ostr, apiNum);
	m_pOperAssistance->Serialize(format, m_ostr, apiNum);

	m_ostr <<  m_numb_of_language  << "\n";
	for (int i = 0; i < m_numb_of_language; i++)
		m_pIVRLanguage[i]->Serialize(format, m_ostr);

	m_ostr <<  (WORD) m_retries_num  << "\n";
	m_ostr <<  (WORD) m_user_input_timeout  << "\n";

	if (apiNum >= API_NUM_ENTRY_Q_VERSION || format != OPERATOR_MCMS)
		m_ostr <<  (WORD)m_bIsEntryQueueService  << "\n";

	m_ostr <<  m_szDelimiter  << "\n";

	m_pGeneralMsgs->Serialize(format, m_ostr, apiNum);
	m_pRollCall->Serialize(format, m_ostr, apiNum);
	m_pVideo->Serialize(format, m_ostr, apiNum);
	m_pNumericId->Serialize(format, m_ostr, apiNum);

	if (apiNum >= API_NUM_AD_HOC || format != OPERATOR_MCMS)
		m_ostr <<  m_wIvrExternalDB  << "\n";

	m_pMuteNoisyLine->Serialize(format, m_ostr, apiNum);
	m_pRecording->Serialize(format, m_ostr, apiNum);
	m_pPlayback->Serialize(format, m_ostr, apiNum);
	m_pInviteParty->Serialize(format, m_ostr, apiNum);
}

//--------------------------------------------------------------------------
void CIVRService::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	WORD tmp;

	m_pDTMF_codeList->DeSerialize(format, m_istr, apiNum);
	m_pLangMenu->DeSerialize(format, m_istr, apiNum);
	m_pWelcome->DeSerialize(format, m_istr, apiNum);
	m_pConfPassword->DeSerialize(format, m_istr, apiNum);
	m_pConfLeader->DeSerialize(format, m_istr, apiNum);
	m_pPersonalPINCode->DeSerialize(format, m_istr, apiNum);
	m_pOperAssistance->DeSerialize(format, m_istr, apiNum);

	m_pInviteParty->SetFeatureOpcode(IVR_FEATURE_INVITE_PARTY);

	m_istr >> m_numb_of_language;
	for (int i = 0; i < m_numb_of_language; i++)
	{
		m_pIVRLanguage[i] = new CIVRLanguage;
		m_pIVRLanguage[i]->DeSerialize(format, m_istr);
	}

	m_istr >> tmp;
	m_retries_num = (BYTE) tmp;

	m_istr >> tmp;
	m_user_input_timeout = (BYTE) tmp;
	if (m_user_input_timeout < 2)
		m_user_input_timeout = 2;

	if (apiNum >= API_NUM_ENTRY_Q_VERSION || format != OPERATOR_MCMS)
	{
		m_istr >> tmp;
		m_bIsEntryQueueService = (BYTE) tmp;
	}

	m_istr.ignore(1); // only if the previous one is not a string
	m_istr.getline(m_szDelimiter, MAX_DELIMETER_LEN+1, '\n');

	m_pGeneralMsgs->DeSerialize(format, m_istr, apiNum);
	m_pRollCall->DeSerialize(format, m_istr, apiNum);
	m_pVideo->DeSerialize(format, m_istr, apiNum);
	m_pNumericId->DeSerialize(format, m_istr, apiNum);

	if (apiNum >= API_NUM_AD_HOC || format != OPERATOR_MCMS)
		m_istr >> m_wIvrExternalDB;

	m_pMuteNoisyLine->DeSerialize(format, m_istr, apiNum);
	m_pRecording->DeSerialize(format, m_istr, apiNum);
	m_pPlayback->DeSerialize(format, m_istr, apiNum);
	m_pInviteParty->DeSerialize(format, m_istr, apiNum);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRService::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pIvrNode = NULL;
	pIvrNode = pFatherNode->AddChildNode("IVR_PARAMS");

	pIvrNode->AddChildNode("IS_ENTRY_QUEUE_SERVICE", m_bIsEntryQueueService, _BOOL);

	// sorted by tabs order in OperWS
	// * global
	pIvrNode->AddChildNode("EXTERNAL_SERVER", m_wIvrExternalDB, IVR_EXTERNAL_SERVER_ENUM);
	pIvrNode->AddChildNode("INPUT_RETRIES_NUMBER", m_retries_num);
	pIvrNode->AddChildNode("INPUT_TIMEOUT_SEC", m_user_input_timeout);
	pIvrNode->AddChildNode("DTMF_DELIMITER", (BYTE)(m_szDelimiter[0]), DELIMITERS_ENUM);

	m_pLangMenu->SerializeXml(pIvrNode);
	for (int i = 0; i < m_numb_of_language; i++)
		m_pIVRLanguage[i]->SerializeXml(pIvrNode);

	// * welcome message
	m_pWelcome->SerializeXml(pIvrNode);
	// * conference chairperson
	m_pConfLeader->SerializeXml(pIvrNode);
	// * conference password
	m_pConfPassword->SerializeXml(pIvrNode);
	// * general
	m_pGeneralMsgs->SerializeXml(pIvrNode);
	// * operator assistance
	m_pOperAssistance->SerializeXml(pIvrNode);
	// * roll call
	m_pRollCall->SerializeXml(pIvrNode);
	// * video services
	m_pVideo->SerializeXml(pIvrNode);
	// * DTMF codes
	m_pDTMF_codeList->SerializeXml(pIvrNode);

	// members without tabs in OperWS
	m_pPersonalPINCode->SerializeXml(pIvrNode);
	m_pNumericId->SerializeXml(pIvrNode);

	m_pMuteNoisyLine->SerializeXml(pIvrNode);

	m_pRecording->SerializeXml(pIvrNode);
	m_pPlayback->SerializeXml(pIvrNode);
	m_pInviteParty->SerializeXml(pIvrNode);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRService::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int  nStatus   = STATUS_OK;
	BYTE Delimiter = 0;

	GET_VALIDATE_CHILD(pActionNode, "IS_ENTRY_QUEUE_SERVICE", &m_bIsEntryQueueService, _BOOL);

	CXMLDOMElement* pFeatureNode = NULL;
	// sorted by tabs order in OperWS
	// * global
	GET_VALIDATE_CHILD(pActionNode, "EXTERNAL_SERVER", &m_wIvrExternalDB, IVR_EXTERNAL_SERVER_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "INPUT_RETRIES_NUMBER", &m_retries_num, _0_TO_BYTE);

	GET_VALIDATE_CHILD(pActionNode, "INPUT_TIMEOUT_SEC", &m_user_input_timeout, _0_TO_BYTE);
	if (m_user_input_timeout < 2)
		m_user_input_timeout = 2;

	memset(m_szDelimiter, '\0', MAX_DELIMETER_LEN);
	GET_VALIDATE_CHILD(pActionNode, "DTMF_DELIMITER", &Delimiter, DELIMITERS_ENUM);
	if (nStatus == STATUS_OK)
		m_szDelimiter[0] = Delimiter;

	GET_CHILD_NODE(pActionNode, "LANGUAGE_MENU", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pLangMenu->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// cleanup languages
	for (int i = 0; i < m_numb_of_language; i++)
		POBJDELETE(m_pIVRLanguage[i]);

	m_numb_of_language = 0;

	CXMLDOMElement* pChild = NULL;

	GET_FIRST_CHILD_NODE(pActionNode, "LANGUAGE", pChild);

	while (pChild && m_numb_of_language < MAX_LANG_IN_IVR_SERVICE)
	{
		m_pIVRLanguage[m_numb_of_language] = new CIVRLanguage;
		nStatus                            = m_pIVRLanguage[m_numb_of_language]->DeSerializeXml(pChild, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(m_pIVRLanguage[m_numb_of_language]);
			return nStatus;
		}

		m_numb_of_language++;
		GET_NEXT_CHILD_NODE(pActionNode, "LANGUAGE", pChild);
	}

	// * welcome message
	GET_CHILD_NODE(pActionNode, "WELCOME_MSG", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pWelcome->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// * conference chairperson
	GET_CHILD_NODE(pActionNode, "CHAIRPERSON", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pConfLeader->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// * conference password
	GET_CHILD_NODE(pActionNode, "CONF_PASSWORD", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pConfPassword->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// * general
	GET_CHILD_NODE(pActionNode, "GENERAL_MSG", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pGeneralMsgs->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// * operator assistance
	GET_CHILD_NODE(pActionNode, "OPERATOR_ASSISTANCE", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pOperAssistance->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// * roll call
	GET_CHILD_NODE(pActionNode, "ROLL_CALL_PARAMS", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pRollCall->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// * video services
	GET_CHILD_NODE(pActionNode, "VIDEO_SERVICE", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pVideo->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// * DTMF codes
	GET_CHILD_NODE(pActionNode, "DTMF_CODES_LIST", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pDTMF_codeList->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;

		CDTMFCode* pVenusDTMFCode = m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_START_VENUS);
		if ((!pVenusDTMFCode ||
		     (0 == strcmp("", pVenusDTMFCode->GetDTMFStr()))) &&
		    (NOT_FIND == m_pDTMF_codeList->FindDTMFCode("*#"))) // VNGR-17942, VNGFE-6263
		{
			if (pVenusDTMFCode)
			{
				pVenusDTMFCode->SetDTMFStr("*#");
			}
			else
			{
				CDTMFCode venusDTMFCode;
				venusDTMFCode.SetDTMFOpcode(DTMF_START_VENUS);
				venusDTMFCode.SetDTMFPermission(DTMF_USER_ACTION);
				venusDTMFCode.SetDTMFStr("*#");
				m_pDTMF_codeList->AddDTMFCode(venusDTMFCode);
			}
		}
	}

	// members without tabs in OperWS
	GET_CHILD_NODE(pActionNode, "PERSONAL_PIN_CODE", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pPersonalPINCode->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "NUMERIC_ID_PARAMS", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pNumericId->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	m_pInviteParty->SetFeatureOpcode(IVR_FEATURE_INVITE_PARTY);

	GET_CHILD_NODE(pActionNode, "NOISY_LINE", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pMuteNoisyLine->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "RECORDING_PARAMS", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pRecording->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "PLAYBACK_PARAMS", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pPlayback->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_CHILD_NODE(pActionNode, "INVITE_PARTICIPANT", pFeatureNode);
	if (pFeatureNode)
	{
		nStatus = m_pInviteParty->DeSerializeXml(pFeatureNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRService::Dump(WORD wSw)
{
}

//--------------------------------------------------------------------------
const char* CIVRService::GetName() const
{
	return m_serviceName;
}

//--------------------------------------------------------------------------
void CIVRService::SetName(const char* name)
{
	strcpy_safe(m_serviceName, name);
}

//--------------------------------------------------------------------------
const CDTMFCodeList* CIVRService::GetDTMFCodeList() const
{
	return m_pDTMF_codeList;
}

//--------------------------------------------------------------------------
void CIVRService::SetDTMFCodeList(const CDTMFCodeList& other)
{
	PDELETE(m_pDTMF_codeList);
	m_pDTMF_codeList = new CDTMFCodeList(other);
}

//--------------------------------------------------------------------------
WORD CIVRService::GetDTMFCodePermission(const WORD DTMF_opcode)
{
	if (m_pDTMF_codeList)
	{
		CDTMFCode* pDTMFCode = m_pDTMF_codeList->GetCurrentDTMFCodeByOpcode(DTMF_opcode);
		if (pDTMFCode)
		{
			WORD permission = pDTMFCode->GetDTMFPermission();
			return permission;
		}
	}

	return DTMF_LEADER_ACTION;
}

//--------------------------------------------------------------------------
void CIVRService::ChangeTable()
{
	CDTMFCodeList* pDTMF_codeListNew = new CDTMFCodeList;

	AddDTMFCodesToList(pDTMF_codeListNew, NO);

	for (int i = 0; i < pDTMF_codeListNew->m_numb_of_DTMF_code; i++)    // Delete the Dtmf string from the new table
	{
		strncpy(pDTMF_codeListNew->m_DTMF_code[i]->m_DTMF_str, "", DTMF_STRING_LEN);
	}

	for (int z = 0; z < pDTMF_codeListNew->m_numb_of_DTMF_code; z++)
	{
		for (int j = 0; j < m_pDTMF_codeList->m_numb_of_DTMF_code; j++)
		{
			if (m_pDTMF_codeList->m_DTMF_code[j]->m_DTMF_opcode == pDTMF_codeListNew->m_DTMF_code[z]->m_DTMF_opcode)
			{
				strncpy(pDTMF_codeListNew->m_DTMF_code[z]->m_DTMF_str, m_pDTMF_codeList->m_DTMF_code[j]->m_DTMF_str, DTMF_STRING_LEN);
				pDTMF_codeListNew->m_DTMF_code[z]->m_DTMF_permission = m_pDTMF_codeList->m_DTMF_code[j]->m_DTMF_permission;
				break;
			}
		}
	}

	int status = pDTMF_codeListNew->CheckLegalDtmfTbl();
	if (status)
	{
		TRACEINTO << "CIVRService::ChangeTable - ServiceName:" << m_serviceName << ", Failed, The exchange IVRDtmfTable didn't succeeded";
		CorrectDTMFTable(pDTMF_codeListNew);
	}

	POBJDELETE(m_pDTMF_codeList)
	m_pDTMF_codeList = pDTMF_codeListNew;
}

//--------------------------------------------------------------------------
void CIVRService::CorrectDTMFTable(CDTMFCodeList* pDTMF_codeListNew)
{
	for (int i = 0; i < pDTMF_codeListNew->m_numb_of_DTMF_code; i++)
	{
		for (int j = 0; j < m_pDTMF_codeList->m_numb_of_DTMF_code; j++)
		{
			if (m_pDTMF_codeList->m_DTMF_code[j]->m_DTMF_opcode == pDTMF_codeListNew->m_DTMF_code[i]->m_DTMF_opcode)
			{
				if (!strncmp(pDTMF_codeListNew->m_DTMF_code[i]->m_DTMF_str, m_pDTMF_codeList->m_DTMF_code[j]->m_DTMF_str, DTMF_STRING_LEN))
				{
					TRACEINTO << "CIVRService::CorrectDTMFTable - DTMF code:" << pDTMF_codeListNew->m_DTMF_code[i]->m_DTMF_str << ", A correction of the DTMF table was done";
					strcpy_safe(pDTMF_codeListNew->m_DTMF_code[i]->m_DTMF_str, "");
					break;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
void CIVRService::ChangeGeneralMsgTable()
{
	CIVRGeneralMsgsFeature* pGeneralMsgF = (CIVRGeneralMsgsFeature*)GetGeneralMsgsFeature();
	CIVRGeneralMsgsFeature* pGeneralMsg  = new CIVRGeneralMsgsFeature; // to create a default table of the general feature
	pGeneralMsgF->ChangeGeneralMsgTable(pGeneralMsg);

	for (int z = 0; z < MAX_IVR_EVENT_IN_FEATURE; z++)
		PDELETE(pGeneralMsg->m_pIVREvent[z]);

	POBJDELETE(pGeneralMsg);
}

//--------------------------------------------------------------------------
// This function create all IVR directories (if needed) under the languages directories
void CIVRService::UpdateIvrDirectories()
{
	TRACEINTO << "CIVRService::UpdateIvrDirectories";

	std::string path = IVR_FOLDER_MAIN;
	path += IVR_FOLDER_MSG;
	path += "/";

	BYTE bNested = 0;

	CFileList* pCFileList = new CFileList;
	if (pCFileList->FillFileList(path.c_str(), bNested) != STATUS_OK)
	{
		TRACEINTO << "CIVRService::UpdateIvrDirectories - Failed to open directory, FolderName:" << path.c_str();
		PDELETE(pCFileList);
		return;
	}

	// Get the first language
	char* language_name = pCFileList->GetFirstFile();
	if (language_name == NULL)
	{
		TRACEINTO << "CIVRService::UpdateIvrDirectories - Failed, Empty languages folder, FolderName:" << path.c_str();
		PDELETE(pCFileList);
		return;
	}

	int status = STATUS_OK;

	// Run over all the languages and create (if needed) the ivr directories
	while (language_name != NULL)
	{
		char* pStr = strstr(language_name, ".");
		if (NULL == pStr)                             // should be NULL
		{
			if (0 < strlen(language_name))
			{
				status = NewIVRLanguage(language_name);   // it may already exist
				if (status != STATUS_OK)
					TRACEINTO << "CIVRService::UpdateIvrDirectories - Failed to update language, Language:" << language_name;
			}
		}
		language_name = pCFileList->GetNextFile();
	}

	PDELETE(pCFileList);
}

//--------------------------------------------------------------------------
const map<WORD, WORD>& CIVRService::getInterfaceOrderMap()
{
	return m_pInviteParty->getInterfaceOrderMap();
}

//--------------------------------------------------------------------------
WORD CIVRService::getDtmfForwardDuration()
{
	return m_pInviteParty->getDtmfForwardDuration();
}

//--------------------------------------------------------------------------
const CIVRLangMenuFeature* CIVRService::GetLangMenuFeature() const
{
	return m_pLangMenu;
}

//--------------------------------------------------------------------------
void CIVRService::SetLangMenuFeature(const CIVRLangMenuFeature& other)
{
	PDELETE(m_pLangMenu);
	m_pLangMenu = new CIVRLangMenuFeature(other);
}

//--------------------------------------------------------------------------
const CIVRWelcomeFeature* CIVRService::GetWelcomeFeature() const
{
	return m_pWelcome;
}

//--------------------------------------------------------------------------
void CIVRService::SetWelcomeFeature(const CIVRWelcomeFeature& other)
{
	PDELETE(m_pWelcome);
	m_pWelcome = new CIVRWelcomeFeature(other);
}

//--------------------------------------------------------------------------
const CIVRConfPasswordFeature* CIVRService::GetConfPasswordFeature() const
{
	return m_pConfPassword;
}

//--------------------------------------------------------------------------
void CIVRService::SetConfPasswordFeature(const CIVRConfPasswordFeature& other)
{
	PDELETE(m_pConfPassword);
	m_pConfPassword = new CIVRConfPasswordFeature(other);
}

//--------------------------------------------------------------------------
const CIVRConfLeaderFeature* CIVRService::GetConfLeaderFeature() const
{
	return m_pConfLeader;
}

//--------------------------------------------------------------------------
void CIVRService::SetConfLeaderFeature(const CIVRConfLeaderFeature& other)
{
	PDELETE(m_pConfLeader);
	m_pConfLeader = new CIVRConfLeaderFeature(other);
}

//--------------------------------------------------------------------------
void CIVRService::SetInvitePartyFeature(const CIVRInvitePartyFeature& other)
{
	PDELETE(m_pInviteParty);
	m_pInviteParty = new CIVRInvitePartyFeature(other);
}

//--------------------------------------------------------------------------
const CIVRInvitePartyFeature* CIVRService::GetInvitePartyFeature() const
{
	return m_pInviteParty;
}

//--------------------------------------------------------------------------
const CIVROperAssistanceFeature* CIVRService::GetOperAssistanceFeature() const
{
	return m_pOperAssistance;
}

//--------------------------------------------------------------------------
void CIVRService::SetOperAssistanceFeature(const CIVROperAssistanceFeature& other)
{
	PDELETE(m_pOperAssistance);
	m_pOperAssistance = new CIVROperAssistanceFeature(other);
}

//--------------------------------------------------------------------------
const CIVRGeneralMsgsFeature* CIVRService::GetGeneralMsgsFeature() const
{
	return m_pGeneralMsgs;
}

//--------------------------------------------------------------------------
void CIVRService::SetGeneralMsgsFeature(const CIVRGeneralMsgsFeature& other)
{
	PDELETE(m_pGeneralMsgs);
	m_pGeneralMsgs = new CIVRGeneralMsgsFeature(other);
}
//--------------------------------------------------------------------------
const CIVR_BillingCodeFeature* CIVRService::GetBillingCodeFeature() const
{
	return m_pBillingCode;
}

//--------------------------------------------------------------------------
void CIVRService::SetBillingCodeFeature(const CIVR_BillingCodeFeature& other)
{
	PDELETE(m_pBillingCode);
	m_pBillingCode = new CIVR_BillingCodeFeature(other);
}

//--------------------------------------------------------------------------
const CIVRRollCallFeature* CIVRService::GetRollCallFeature() const
{
	return m_pRollCall;
}

//--------------------------------------------------------------------------
void CIVRService::SetRollCallFeature(const CIVRRollCallFeature& other)
{
	PDELETE(m_pRollCall);
	m_pRollCall = new CIVRRollCallFeature(other);
}

//--------------------------------------------------------------------------
const CIVRVideoFeature* CIVRService::GetVideoFeature() const
{
	return m_pVideo;
}

//--------------------------------------------------------------------------
void CIVRService::SetVideoFeature(const CIVRVideoFeature& other)
{
	PDELETE(m_pVideo);
	m_pVideo = new CIVRVideoFeature(other);
}

//--------------------------------------------------------------------------
const CIVRNumericConferenceIdFeature* CIVRService::GetNumericConferenceIdFeature() const
{
	return m_pNumericId;
}

//--------------------------------------------------------------------------
void CIVRService::SetNumericConferenceIdFeature(const CIVRNumericConferenceIdFeature& other)
{
	PDELETE(m_pNumericId);
	m_pNumericId = new CIVRNumericConferenceIdFeature(other);
}

//--------------------------------------------------------------------------
const CIVRMuteNoisyLineFeature* CIVRService::GetMuteNoisyLineFeature() const
{
	return m_pMuteNoisyLine;
}

//--------------------------------------------------------------------------
void CIVRService::SetMuteNoisyLineFeature(const CIVRMuteNoisyLineFeature& other)
{
	PDELETE(m_pMuteNoisyLine);
	m_pMuteNoisyLine = new CIVRMuteNoisyLineFeature(other);
}

//--------------------------------------------------------------------------
const CIVRRecordingFeature* CIVRService::GetRecordingFeature() const
{
	return m_pRecording;
}

//--------------------------------------------------------------------------
void CIVRService::SetRecordingFeature(const CIVRRecordingFeature& other)
{
	PDELETE(m_pRecording);
	m_pRecording = new CIVRRecordingFeature(other);
}

//--------------------------------------------------------------------------
const CIVRPlaybackFeature* CIVRService::GetPlaybackFeature() const
{
	return m_pPlayback;
}

//--------------------------------------------------------------------------
void CIVRService::SetPlaybackFeature(const CIVRPlaybackFeature& other)
{
	PDELETE(m_pPlayback);
	m_pPlayback = new CIVRPlaybackFeature(other);
}

//--------------------------------------------------------------------------
CIVRFeature* CIVRService::GetIVRFeature(const WORD feature_opcode) const
{
	CIVRFeature* pIVRFeature = NULL;

	switch (feature_opcode)
	{
		case IVR_FEATURE_LANG_MENU:
			pIVRFeature = (CIVRFeature*)GetLangMenuFeature();
			break;

		case IVR_FEATURE_CONF_PASSWORD:
			pIVRFeature = (CIVRFeature*)GetConfPasswordFeature();
			break;

		case IVR_FEATURE_CONF_LEADER:
			pIVRFeature = (CIVRFeature*)GetConfLeaderFeature();
			break;

		case IVR_FEATURE_OPER_ASSISTANCE:
			pIVRFeature = (CIVRFeature*)GetOperAssistanceFeature();
			break;

		case IVR_FEATURE_WELCOME:
			pIVRFeature = (CIVRFeature*)GetWelcomeFeature();
			break;

		case IVR_FEATURE_GENERAL:
			pIVRFeature = (CIVRFeature*)GetGeneralMsgsFeature();
			break;

		case IVR_FEATURE_BILLING_CODE:
			pIVRFeature = (CIVRFeature*)GetBillingCodeFeature();
			break;

		case IVR_FEATURE_INVITE_PARTY:
			pIVRFeature = (CIVRFeature*)GetInvitePartyFeature();
			break;

		case IVR_FEATURE_ROLL_CALL:
			pIVRFeature = (CIVRFeature*)GetRollCallFeature();
			break;

		case IVR_FEATURE_VIDEO:
			pIVRFeature = (CIVRFeature*)GetVideoFeature();
			break;

		case IVR_FEATURE_NUMERIC_CONFERENCE_ID:
			pIVRFeature = (CIVRFeature*)GetNumericConferenceIdFeature();
			break;

		case IVR_FEATURE_MUTE_NOISY_LINE:
			pIVRFeature = (CIVRFeature*)GetMuteNoisyLineFeature();
			break;

		case IVR_FEATURE_RECORDING:
			pIVRFeature = (CIVRFeature*)GetRecordingFeature();
			break;

		case IVR_FEATURE_PLAYBACK:
			pIVRFeature = (CIVRFeature*)GetPlaybackFeature();
			break;

		default:
			pIVRFeature = NULL;
			break;
	} // switch

	return pIVRFeature;
}

//--------------------------------------------------------------------------
WORD CIVRService::GetIVRLanguageNumber() const
{
	return m_numb_of_language;
}

//--------------------------------------------------------------------------
int CIVRService::AddLanguage(CIVRLanguage& other)
{
	int status = STATUS_OK;

	if (m_numb_of_language >= MAX_LANG_IN_IVR_SERVICE)
		return STATUS_THE_MAXIMUM_NUMBER_OF_LANGUAGES_IN_IVR_SERVICE_IS_EXCEEDED;

	if (FindLanguage(other) != NOT_FIND)
		return STATUS_THIS_LANGUAGE_ALREADY_EXISTS_IN_THE_IVR_SERVICE;

	m_pIVRLanguage[m_numb_of_language] = new CIVRLanguage(other);
	m_pIVRLanguage[m_numb_of_language]->SetLanguagePos(m_numb_of_language+1);

	m_numb_of_language++;

	WORD lanuage_pos = other.GetLanguagePos();

	m_pLangMenu->AddLanguage(lanuage_pos);
	m_pWelcome->AddLanguage(lanuage_pos);
	m_pConfPassword->AddLanguage(lanuage_pos);
	m_pConfLeader->AddLanguage(lanuage_pos);
	m_pPersonalPINCode->AddLanguage(lanuage_pos);
	m_pOperAssistance->AddLanguage(lanuage_pos);
	m_pGeneralMsgs->AddLanguage(lanuage_pos);
	m_pRollCall->AddLanguage(lanuage_pos);
	m_pNumericId->AddLanguage(lanuage_pos);
	m_pMuteNoisyLine->AddLanguage(lanuage_pos);
	m_pRecording->AddLanguage(lanuage_pos);
	m_pPlayback->AddLanguage(lanuage_pos);
	m_pInviteParty->AddLanguage(lanuage_pos);
	return status;
}

//--------------------------------------------------------------------------
int CIVRService::CancelLanguage(const char* language_name)
{
	int status = STATUS_OK;

	if (!language_name)
		return STATUS_ILLEGAL;

	int ind = FindLanguage(language_name);
	if (ind == NOT_FIND)
		return STATUS_IVR_EVENT_IN_FEATURE_NOT_EXISTS;

	WORD lanuage_pos = m_pIVRLanguage[ind]->GetLanguagePos();

	PDELETE(m_pIVRLanguage[ind]);

	int i;
	for (i = 0; i < (int)m_numb_of_language; i++)
	{
		if (m_pIVRLanguage[i] == NULL)
			break;
	}

	int j;
	for (j = i; j < (int)m_numb_of_language-1; j++)
	{
		m_pIVRLanguage[j] = m_pIVRLanguage[j+1];
		m_pIVRLanguage[j]->SetLanguagePos(j+1);
	}

	m_pIVRLanguage[m_numb_of_language-1] = NULL;
	m_numb_of_language--;

	m_pLangMenu->CancelLanguage(lanuage_pos);
	m_pWelcome->CancelLanguage(lanuage_pos);
	m_pConfPassword->CancelLanguage(lanuage_pos);
	m_pConfLeader->CancelLanguage(lanuage_pos);
	m_pPersonalPINCode->CancelLanguage(lanuage_pos);
	m_pOperAssistance->CancelLanguage(lanuage_pos);
	m_pGeneralMsgs->CancelLanguage(lanuage_pos);
	m_pRollCall->CancelLanguage(lanuage_pos);
	m_pNumericId->CancelLanguage(lanuage_pos);
	m_pMuteNoisyLine->CancelLanguage(lanuage_pos);
	m_pRecording->CancelLanguage(lanuage_pos);
	m_pPlayback->CancelLanguage(lanuage_pos);
	m_pInviteParty->CancelLanguage(lanuage_pos);

	return status;
}

//--------------------------------------------------------------------------
int CIVRService::FindLanguage(const CIVRLanguage& other)
{
	for (int i = 0; i < (int)m_numb_of_language; i++)
	{
		if (m_pIVRLanguage[i] != NULL)
		{
			if (!strncmp(m_pIVRLanguage[i]->GetLanguageName(), other.GetLanguageName(), LANGUAGE_NAME_LEN))
				return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CIVRService::FindLanguage(const char* language_name)
{
	if (!language_name)
		return NOT_FIND;

	for (int i = 0; i < (int)m_numb_of_language; i++)
	{
		if (m_pIVRLanguage[i] != NULL)
		{
			if (!strncmp(m_pIVRLanguage[i]->GetLanguageName(), language_name, LANGUAGE_NAME_LEN))
				return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
CIVRLanguage* CIVRService::GetCurrentIVRLanguage(const char* language_name) const
{
	if (!language_name)
		return NULL;

	for (int i = 0; i < (int)m_numb_of_language; i++)
	{
		if (m_pIVRLanguage[i] != NULL)
		{
			if (!strncmp(m_pIVRLanguage[i]->GetLanguageName(), language_name, LANGUAGE_NAME_LEN))
				return m_pIVRLanguage[i];
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------
CIVRLanguage* CIVRService::GetCurrentIVRLanguage(const WORD language_DTMF_code) const
{
	for (int i = 0; i < (int)m_numb_of_language; i++)
	{
		if (m_pIVRLanguage[i] != NULL)
		{
			if (m_pIVRLanguage[i]->GetLanguageDTMFCode() == language_DTMF_code)
				return m_pIVRLanguage[i];
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------
CIVRLanguage* CIVRService::GetCurrentIVRLanguagePos(const WORD pos) const
{
	for (int i = 0; i < (int)m_numb_of_language; i++)
	{
		if (m_pIVRLanguage[i] != NULL)
		{
			if (m_pIVRLanguage[i]->GetLanguagePos() == pos)
				return m_pIVRLanguage[i];
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------
CIVRLanguage* CIVRService::GetFirstLanguage()
{
	m_ind_language = 1;
	return m_pIVRLanguage[0];
}

//--------------------------------------------------------------------------
CIVRLanguage* CIVRService::GetNextLanguage()
{
	if (m_ind_language >= m_numb_of_language) return NULL;

	return m_pIVRLanguage[m_ind_language++];
}

//--------------------------------------------------------------------------
const char* CIVRService::GetMsgFileName(const WORD feature_opcode, const WORD event_opcode, const char* language_name, int& status) const
{
	if (!language_name)
	{
		status = STATUS_ILLEGAL;
		return NULL;
	}

	CIVRLanguage* pIVRLanguage = GetCurrentIVRLanguage(language_name);
	if (!pIVRLanguage)
	{
		status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;
		return NULL;
	}

	status = STATUS_OK;
	const char* pFileName = NULL;

	WORD language_pos = pIVRLanguage->GetLanguagePos();

	switch (feature_opcode)
	{
		case IVR_FEATURE_LANG_MENU:
			pFileName = m_pLangMenu->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_CONF_PASSWORD:
			pFileName = m_pConfPassword->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_CONF_LEADER:
			pFileName = m_pConfLeader->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_PIN_CODE:
			pFileName = m_pPersonalPINCode->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_OPER_ASSISTANCE:
			pFileName = m_pOperAssistance->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_WELCOME:
			pFileName = m_pWelcome->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_GENERAL:
			pFileName = m_pGeneralMsgs->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_BILLING_CODE:
			pFileName = m_pBillingCode->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_INVITE_PARTY:
			pFileName = m_pGeneralMsgs->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_ROLL_CALL:
			pFileName = m_pRollCall->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_NUMERIC_CONFERENCE_ID:
			pFileName = m_pNumericId->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_MUTE_NOISY_LINE:
			pFileName = m_pMuteNoisyLine->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_RECORDING:
			pFileName = m_pRecording->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_PLAYBACK:
			pFileName = m_pPlayback->GetMsgFileName(event_opcode, language_pos, status);
			break;

		default:
			status = STATUS_FEATURE_NOT_EXISTS;
			break;
	} // switch

	return pFileName;
}

//--------------------------------------------------------------------------
const char* CIVRService::GetMsgFileName(const WORD feature_opcode, const WORD event_opcode, int& status) const
{
	const CIVRLanguage* pIVRLanguage = m_pIVRLanguage[0];
	if (pIVRLanguage)
	{
		WORD language_pos = pIVRLanguage->GetLanguagePos();
		return GetMsgFileName(feature_opcode, event_opcode, language_pos, status);
	}

	status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;
	return NULL;
}

//--------------------------------------------------------------------------
const char* CIVRService::GetMsgFileName(const WORD feature_opcode, const WORD event_opcode, WORD language_pos, int& status) const
{
	status = STATUS_OK;

	const char* pFileName = NULL;
	switch (feature_opcode)
	{
		case IVR_FEATURE_LANG_MENU:
			pFileName = m_pLangMenu->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_CONF_PASSWORD:
			pFileName = m_pConfPassword->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_CONF_LEADER:
			pFileName = m_pConfLeader->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_PIN_CODE:
			pFileName = m_pPersonalPINCode->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_OPER_ASSISTANCE:
			pFileName = m_pOperAssistance->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_INVITE_PARTY:
			pFileName = m_pGeneralMsgs->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_WELCOME:
			pFileName = m_pWelcome->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_GENERAL:
			pFileName = m_pGeneralMsgs->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_ROLL_CALL:
			pFileName = m_pRollCall->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_NUMERIC_CONFERENCE_ID:
			pFileName = m_pNumericId->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_MUTE_NOISY_LINE:
			pFileName = m_pMuteNoisyLine->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_RECORDING:
			pFileName = m_pRecording->GetMsgFileName(event_opcode, language_pos, status);
			break;

		case IVR_FEATURE_PLAYBACK:
			pFileName = m_pPlayback->GetMsgFileName(event_opcode, language_pos, status);
			break;

		default:
			status = STATUS_FEATURE_NOT_EXISTS;
			break;
	} // switch

	return pFileName;
}

//--------------------------------------------------------------------------
void CIVRService::SetMsgFileName(const WORD feature_opcode, const WORD event_opcode, const char* language_name, const char* file_name, int& status)
{
	if (!language_name)
		status = STATUS_ILLEGAL;

	CIVRLanguage* pIVRLanguage = GetCurrentIVRLanguage(language_name);
	if (!pIVRLanguage)
	{
		status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;
		return;
	}

	status = STATUS_OK;

	WORD language_pos = pIVRLanguage->GetLanguagePos();

	switch (feature_opcode)
	{
		case IVR_FEATURE_LANG_MENU:
			m_pLangMenu->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_CONF_PASSWORD:
			m_pConfPassword->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_CONF_LEADER:
			m_pConfLeader->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_PIN_CODE:
			m_pPersonalPINCode->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_OPER_ASSISTANCE:
			m_pOperAssistance->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_INVITE_PARTY:
			m_pGeneralMsgs->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_WELCOME:
			m_pWelcome->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_GENERAL:
			m_pGeneralMsgs->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_ROLL_CALL:
			m_pRollCall->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_NUMERIC_CONFERENCE_ID:
			m_pNumericId->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_MUTE_NOISY_LINE:
			m_pMuteNoisyLine->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_RECORDING:
			m_pRecording->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		case IVR_FEATURE_PLAYBACK:
			m_pPlayback->SetMsgFileName(event_opcode, language_pos, file_name, status);
			break;

		default:
			status = STATUS_FEATURE_NOT_EXISTS;
			break;
	} // switch
}

//--------------------------------------------------------------------------
void CIVRService::GetFullPathMsgFileName(char* fullPathMsgName, const WORD feature_opcode, const WORD event_opcode, int& status) const
{
	fullPathMsgName[0] = 0;
	if (status == STATUS_OK)
	{
		const char* msgName = GetMsgFileName(feature_opcode, event_opcode, status);
		if ((msgName == NULL) || (msgName[0] == 0))
		{
			status = STATUS_IVR_MSG_NAME_MISSING;
			return;
		}

		if (!strcmp(msgName, "(None)"))
		{
			strncpy(fullPathMsgName, msgName, 7);
			fullPathMsgName[7] = '\0';
			return;
		}

		if (status == STATUS_OK)
		{
			ALLOCBUFFER(ivrMsgType, NEW_FILE_NAME_LEN);
			// Before cleanup week was: ALLOCBUFFER( ivrMsgType, MAX_FILE_NAME_LEN );
			const char* feature_str = NULL;
			const char* event_str   = NULL;
			const char* language    = NULL;

			CIVRLanguage* pIVRLanguage = m_pIVRLanguage[0];
			if (pIVRLanguage)
				language = pIVRLanguage->GetLanguageName();
			else
				status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;

			if (STATUS_OK == status)
				feature_str = ::GetIVRFeatureDirectory(feature_opcode, status);

			if (STATUS_OK == status)
				event_str = ::GetIVREventDirectory(event_opcode, status);

			if (STATUS_OK == status)
			{
				if (feature_str && event_str && language)
				{
					sprintf(ivrMsgType, "%s/%s/%s/", language, feature_str, event_str);
					sprintf(fullPathMsgName, "Cfg/IVR/msg/%s%s", ivrMsgType, msgName);
				}
				else
					status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;
			}
			DEALLOCBUFFER(ivrMsgType)
		}
	}
}

//--------------------------------------------------------------------------
WORD CIVRService::GetLanguagePos(const char* language_name, int& status) const
{
	WORD pos = 0xFFFF;

	if (!language_name)
	{
		status = STATUS_ILLEGAL;
		return pos;
	}

	status = STATUS_OK;

	CIVRLanguage* pIVRLanguage = GetCurrentIVRLanguage(language_name);
	if (pIVRLanguage)
		pos = pIVRLanguage->GetLanguagePos();
	else
		status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;

	return pos;
}

//--------------------------------------------------------------------------
WORD CIVRService::GetLanguagePos(WORD language_DTMF_code, int& status) const
{
	WORD pos = 0xFFFF;
	status = STATUS_OK;

	CIVRLanguage* pIVRLanguage = GetCurrentIVRLanguage(language_DTMF_code);
	if (pIVRLanguage)
		pos = pIVRLanguage->GetLanguagePos();
	else
		status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;

	return pos;
}

// Add new directory for a new language
//--------------------------------------------------------------------------
int CIVRService::NewIVRLanguage(const char* language_name)
{
	if (!language_name)
		return STATUS_ILLEGAL_LANGUAGE_NAME;

	// checks name len
	int len = strlen(language_name);
	if ((0 == len) || (len > MAX_IVR_LANG_NAME_SIZE))
		return STATUS_ILLEGAL_LANGUAGE_NAME;

	// first character of language name cannot be a digit(psos file system problem)-Oshi
	if (language_name[0] >= '0' && language_name[0] <= '9')
		return STATUS_ILLEGAL_LANGUAGE_NAME;

	int status = STATUS_OK;

	string dirName = IVR_FOLDER_MAIN;
	dirName += IVR_FOLDER_MSG;
	dirName += "/";
	dirName += language_name;

	const char* dir_name = dirName.c_str();

	// creates main language folder
	if (FALSE == CreateDirectory(dir_name, 0755))
	{
		TRACEINTO << "CIVRService::NewIVRLanguage - Failed to create folder, FolderName:" << dir_name;
		return STATUS_FOLDER_CREATE_ERROR;
	}

	// creates folders for every feature
	if (status == STATUS_OK)
		status = m_pWelcome->NewDirectory(dir_name);

	if (status == STATUS_OK)
		status = m_pConfPassword->NewDirectory(dir_name);

	if (status == STATUS_OK)
		status = m_pConfLeader->NewDirectory(dir_name);

	if (status == STATUS_OK)
		status = m_pOperAssistance->NewDirectory(dir_name);

	if (status == STATUS_OK)
		status = m_pGeneralMsgs->NewDirectory(dir_name);

	if (status == STATUS_OK)
		status = m_pRollCall->NewDirectory(dir_name);

	if (status == STATUS_OK)
		status = m_pNumericId->NewDirectory(dir_name);

	if (status == STATUS_OK)
		status = m_pMuteNoisyLine->NewDirectory(dir_name);

	//if (status == STATUS_OK)
	//	status = m_pInviteParty->NewDirectory(dir_name);

	return status;
}

//--------------------------------------------------------------------------
int CIVRService::RemoveIVRLanguage(const char* language_name)
{
	if (!language_name)
		return STATUS_ILLEGAL;

	int status = STATUS_OK;

	ALLOCBUFFER(dir_name, strlen(language_name) + 20);
	// before clean up week was:
	sprintf(dir_name, "Cfg/IVR/msg/%s", language_name);

	DEALLOCBUFFER(dir_name);

	return status;
}

//--------------------------------------------------------------------------
BYTE CIVRService::GetRetriesNum() const
{
	return m_retries_num;
}

//--------------------------------------------------------------------------
void CIVRService::SetRetriesNum(const BYTE retriesNum)
{
	m_retries_num = retriesNum;
}

//--------------------------------------------------------------------------
BYTE CIVRService::GetUserInputTimeout() const
{
	return m_user_input_timeout;
}

//--------------------------------------------------------------------------
void CIVRService::SetUserInputTimeout(const BYTE user_input_timeout)
{
	m_user_input_timeout = user_input_timeout;
}

//--------------------------------------------------------------------------
const char* CIVRService::GetDelimiter() const
{
	return m_szDelimiter;
}

//--------------------------------------------------------------------------
void CIVRService::SetDelimiter(const char* szDelimiter)
{
	strcpy_safe(m_szDelimiter, szDelimiter);
}

//--------------------------------------------------------------------------
BYTE CIVRService::GetEntryQueueService() const
{
	return m_bIsEntryQueueService;
}

//--------------------------------------------------------------------------
void CIVRService::SetEntryQueueService(const BYTE IsEntryQueueService)
{
	m_bIsEntryQueueService = IsEntryQueueService;
}

//--------------------------------------------------------------------------
WORD CIVRService::GetIvrExternalDB() const
{
	return m_wIvrExternalDB;
}
//--------------------------------------------------------------------------
void CIVRService::SetIvrExternalDB(const WORD wExtenalDB)
{
	m_wIvrExternalDB = wExtenalDB;
}

//--------------------------------------------------------------------------
int CIVRService::GetIVRMsgParams(const WORD ivrFeatureOpcode, const WORD ivrEventOpcode,
                                 char* ivrMsgFullPath, WORD* ivrMsgDuration, WORD* ivrMsgCheckSum,
                                 time_t* ivrMsgLastModified, int* updateStatus) const
{
	INT_MESSAGE_HEADER_S header_s;

	// Get IVR message file's full path
	// Tsahi - If it's IVR_FEATURE_GENERAL and IVR_EVENT_BLIP_ON_CASCADE_LINK, then use hard coded path.
	if (ivrFeatureOpcode == IVR_FEATURE_GENERAL && ivrEventOpcode == IVR_EVENT_BLIP_ON_CASCADE_LINK)
	{
		strcpy(ivrMsgFullPath, IVR_MSG_BLIP_ON_CASCADE_TONE);
	}
	else
	{
		int fullPathStatus = STATUS_OK;

		GetFullPathMsgFileName(ivrMsgFullPath, ivrFeatureOpcode, ivrEventOpcode, fullPathStatus);

		TRACECOND_AND_RETURN_VALUE(STATUS_OK != fullPathStatus, "CIVRService::GetIVRMsgParams - Failed", STATUS_FAIL);
	}

	// Get the relevant IVR Feature
	CIVRFeature* pIVRFeature = GetIVRFeature(ivrFeatureOpcode);
	TRACECOND_AND_RETURN_VALUE(!pIVRFeature, "CIVRService::GetIVRMsgParams - Failed, pIVRFeature is invalid", STATUS_FAIL);

	// Get the relevant IVR Event
	CIVREvent* pIVREvent = pIVRFeature->GetCurrentIVREvent(ivrEventOpcode);
	TRACECOND_AND_RETURN_VALUE(!pIVREvent, "CIVRService::GetIVRMsgParams - Failed, pIVREvent is invalid", STATUS_FAIL);

	// Get IVR message
	CIVRMessage* pIVRMessage = pIVREvent->GetFirstMessage();
	TRACECOND_AND_RETURN_VALUE(!pIVRMessage, "CIVRService::GetIVRMsgParams - Failed, pIVRMessage is invalid", STATUS_FAIL);

	// Get IVR message file's last modified value
	time_t msgLastModified = pIVRMessage->GetMsgLastModified();

	// Get last modified value from file
	string sIvrMsgFullPath     = ivrMsgFullPath;
	time_t fileMsgLastModified = GetLastModified(sIvrMsgFullPath);

	// Check if the IVR message file have been changed in the disk since the last update of the list
	if (fileMsgLastModified == msgLastModified)     // No change - return the required parameters
	{
		(*ivrMsgDuration) = pIVRMessage->GetMsgDuration();
		(*ivrMsgCheckSum) = pIVRMessage->GetMsgCheckSum();
		(*updateStatus)   = STATUS_NO_NEED_TO_UPDATE; // No update is necessary, the file params has the same params as in memory
	}
	else
	{
		TRACEINTO << "CIVRService::GetIVRMsgParams - A change has been made in file:" << ivrMsgFullPath;
		CIVRFeature temp;
		int status = temp.CheckLegalFileAndGetParams(ivrMsgFullPath, ivrMsgDuration, ivrMsgCheckSum);
		if (status != STATUS_OK)
			return STATUS_IVR_INVALID_AUDIO_FILE;

		(*ivrMsgLastModified) = fileMsgLastModified;   // A change has been made - return the required parameters from file in disk
		(*updateStatus)       = STATUS_NEED_TO_UPDATE; // All instances of this IVR message in the IVR service list should be updated
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRService::UpdateIVRMsgParamsInService(const WORD ivrFeatureOpcode, const WORD ivrEventOpcode,
                                              const WORD fileMsgDuration, const WORD fileMsgCheckSum,
                                              const time_t fileLastModified)
{
	// Get the relevant IVR Feature
	CIVRFeature* pIVRFeature = GetIVRFeature(ivrFeatureOpcode);
	TRACECOND_AND_RETURN(!pIVRFeature, "CIVRService::UpdateIVRMsgParamsInService - Failed, pIVRFeature is invalid");

	// Get the relevant IVR Event
	CIVREvent* pIVREvent = pIVRFeature->GetCurrentIVREvent(ivrEventOpcode);
	TRACECOND_AND_RETURN(!pIVREvent, "CIVRService::UpdateIVRMsgParamsInService - Failed, pIVREvent is invalid");

	// Get IVR message
	CIVRMessage* pIVRMessage = pIVREvent->GetFirstMessage();
	TRACECOND_AND_RETURN(!pIVRMessage, "CIVRService::UpdateIVRMsgParamsInService - Failed, pIVRMessage is invalid");

	// Update IVR message in list
	pIVRMessage->SetMsgDuration(fileMsgDuration);
	pIVRMessage->SetMsgCheckSum(fileMsgCheckSum);
	pIVRMessage->SetMsgLastModified(fileLastModified);
}

//--------------------------------------------------------------------------
BOOL CIVRService::IsIdenticalToCurrent(CIVRService* other)
{
	if (FALSE == m_pDTMF_codeList->IsIdenticalToCurrent(other->m_pDTMF_codeList))
		return FALSE;

	if (FALSE == m_pLangMenu->IsIdenticalToCurrent(other->m_pLangMenu))
		return FALSE;

	if (FALSE == m_pWelcome->IsIdenticalToCurrent(other->m_pWelcome))
		return FALSE;

	if (FALSE == m_pConfPassword->IsIdenticalToCurrent(other->m_pConfPassword))
		return FALSE;

	if (FALSE == m_pConfLeader->IsIdenticalToCurrent(other->m_pConfLeader))
		return FALSE;

	if (FALSE == m_pGeneralMsgs->IsIdenticalToCurrent(other->m_pGeneralMsgs))
		return FALSE;

	if (FALSE == m_pRollCall->IsIdenticalToCurrent(other->m_pRollCall))
		return FALSE;

	if (FALSE == m_pVideo->IsIdenticalToCurrent(other->m_pVideo))
		return FALSE;

	if (FALSE == m_pNumericId->IsIdenticalToCurrent(other->m_pNumericId))
		return FALSE;

	if (FALSE == m_pInviteParty->IsIdenticalToCurrent(other->m_pInviteParty))
		return FALSE;

	if (m_retries_num != other->m_retries_num)
		return FALSE;

	if (m_user_input_timeout != other->m_user_input_timeout)
		return FALSE;

	if (0 != strncmp(m_szDelimiter, other->m_szDelimiter, MAX_DELIMETER_LEN))
		return FALSE;

	if (m_bIsEntryQueueService != other->m_bIsEntryQueueService)
		return FALSE;

	if (m_wIvrExternalDB != other->m_wIvrExternalDB)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRLanguage
////////////////////////////////////////////////////////////////////////////
CIVRLanguage::CIVRLanguage()
{
	// defaults
	m_language_pos       = 0;
	m_language_DTMF_code = 0;
	m_language_name[0]   = '\0';
}

//--------------------------------------------------------------------------
CIVRLanguage::CIVRLanguage(const CIVRLanguage& other)
             :CPObject(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRLanguage& CIVRLanguage::operator=(const CIVRLanguage& other)
{
	m_language_pos       = other.m_language_pos;
	m_language_DTMF_code = other.m_language_DTMF_code;

	strcpy_safe(m_language_name, other.m_language_name);

	return *this;
}

//--------------------------------------------------------------------------
CIVRLanguage::~CIVRLanguage()
{ }

//--------------------------------------------------------------------------
void CIVRLanguage::Serialize(WORD format, std::ostream& m_ostr)
{
	// assuming format = OPERATOR_MCMS
	m_ostr <<  m_language_pos  << "\n";
	m_ostr <<  m_language_DTMF_code  << "\n";
	m_ostr <<  m_language_name  << "\n";
}

//--------------------------------------------------------------------------
void CIVRLanguage::DeSerialize(WORD format, std::istream& m_istr)
{
	// assuming format = OPERATOR_MCMS
	m_istr >> m_language_pos;
	m_istr >> m_language_DTMF_code;

	m_istr.ignore(1);
	m_istr.getline(m_language_name, LANGUAGE_NAME_LEN+1, '\n');
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRLanguage::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pLangNode = pFatherNode->AddChildNode("LANGUAGE");

	pLangNode->AddChildNode("LANG_NAME", m_language_name);
	pLangNode->AddChildNode("NUMBER", m_language_pos);
	pLangNode->AddChildNode("DTMF_OPCODE", m_language_DTMF_code);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRLanguage::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "LANG_NAME", m_language_name, _1_TO_LANGUAGE_NAME_LENGTH);
	if (nStatus != STATUS_OK)
		return nStatus;

	GET_VALIDATE_CHILD(pActionNode, "NUMBER", &m_language_pos, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode, "DTMF_OPCODE", &m_language_DTMF_code, _0_TO_DWORD);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRLanguage::SetLanguagePos(WORD language_pos)
{
	m_language_pos = language_pos;
}

//--------------------------------------------------------------------------
WORD CIVRLanguage::GetLanguagePos() const
{
	return m_language_pos;
}

//--------------------------------------------------------------------------
void CIVRLanguage::SetLanguageDTMFCode(WORD language_DTMF_code)
{
	m_language_DTMF_code = language_DTMF_code;
}

//--------------------------------------------------------------------------
WORD CIVRLanguage::GetLanguageDTMFCode() const
{
	return m_language_DTMF_code;
}

//--------------------------------------------------------------------------
void CIVRLanguage::SetLanguageName(const char* name)
{
	strcpy_safe(m_language_name, name);
}

//--------------------------------------------------------------------------
const char* CIVRLanguage::GetLanguageName() const
{
	return m_language_name;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRFeature
////////////////////////////////////////////////////////////////////////////
CIVRFeature::CIVRFeature()
{
	m_feature_opcode         = 0;
	m_enable_disable         = 0;
	m_numb_of_IVR_Event      = 0;
	m_ind_event              = 0;
	m_Save_numb_of_IVR_Event = 0;

	for (int i = 0; i < MAX_IVR_EVENT_IN_FEATURE; i++)
		m_pIVREvent[i] = NULL;

}

//--------------------------------------------------------------------------
CIVRFeature::CIVRFeature(const CIVRFeature& other)
            :CPObject(other)
{
	for (int i = 0; i < MAX_IVR_EVENT_IN_FEATURE; i++)
		m_pIVREvent[i] = NULL;

	*this = other;
}

//--------------------------------------------------------------------------
CIVRFeature& CIVRFeature::operator=(const CIVRFeature& other)
{
	m_feature_opcode         = other.m_feature_opcode;
	m_enable_disable         = other.m_enable_disable;
	m_numb_of_IVR_Event      = other.m_numb_of_IVR_Event;
	m_ind_event              = other.m_ind_event;
	m_Save_numb_of_IVR_Event = other.m_Save_numb_of_IVR_Event;

	for (int i = 0; i < MAX_IVR_EVENT_IN_FEATURE; i++)
	{
		PDELETE(m_pIVREvent[i]);
		if (other.m_pIVREvent[i])
			m_pIVREvent[i] = new CIVREvent(*other.m_pIVREvent[i]);
	}

	return *this;
}

//--------------------------------------------------------------------------
CIVRFeature::~CIVRFeature()
{
	for (int i = 0; i < MAX_IVR_EVENT_IN_FEATURE; i++)
		PDELETE(m_pIVREvent[i]);
}

//--------------------------------------------------------------------------
int CIVRFeature::IsLegalService(WORD chkLevel, string& err, const char* language)
{
	// should be pure virtual...
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRFeature::Serialize(WORD format, std::ostream& m_ostr)
{
	m_ostr <<  m_feature_opcode  << "\n";
	m_ostr <<  (WORD)m_enable_disable  << "\n";
	m_ostr <<  m_numb_of_IVR_Event  << "\n";
	for (int i = 0; i < m_numb_of_IVR_Event; i++)
		m_pIVREvent[i]->Serialize(format, m_ostr);
}

//--------------------------------------------------------------------------
void CIVRFeature::DeSerialize(WORD format, std::istream& m_istr)
{
	// assuming format = OPERATOR_MCMS
	WORD tmp;

	m_istr >> m_feature_opcode;
	m_istr >> tmp;
	m_enable_disable = (BYTE) tmp;

	m_istr >> m_numb_of_IVR_Event;
	for (int i = 0; i < m_numb_of_IVR_Event; i++)
	{
		if (i >= MAX_IVR_EVENT_IN_FEATURE)    // someone sent too much events!
		{
			CIVREvent dummy;
			dummy.DeSerialize(format, m_istr);
		}
		else
		{
			if (m_pIVREvent[i] == NULL)
				m_pIVREvent[i] = new CIVREvent;

			m_pIVREvent[i]->DeSerialize(format, m_istr);
		}
	}

	if (m_numb_of_IVR_Event > MAX_IVR_EVENT_IN_FEATURE) // after the "for" loop
		m_numb_of_IVR_Event = MAX_IVR_EVENT_IN_FEATURE;
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pFeatureNode = pFatherNode->AddChildNode("IVR_FEATURE_DETAILS");

	pFeatureNode->AddChildNode("FEATURE_TYPE", m_feature_opcode, IVR_FEATURE_OPCODE_ENUM);
	pFeatureNode->AddChildNode("IS_ENABLED", m_enable_disable, _BOOL);

	for (int i = 0; i < m_numb_of_IVR_Event; i++)
		m_pIVREvent[i]->SerializeXml(pFeatureNode);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "FEATURE_TYPE", &m_feature_opcode, IVR_FEATURE_OPCODE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "IS_ENABLED", &m_enable_disable, _BOOL);

	// cleanup
	for (int i = 0; i < m_numb_of_IVR_Event; i++)
		POBJDELETE(m_pIVREvent[i]);

	m_numb_of_IVR_Event = 0;

	CXMLDOMElement* pEventNode = NULL;

	GET_FIRST_CHILD_NODE(pActionNode, "IVR_EVENT", pEventNode);

	while (pEventNode && m_numb_of_IVR_Event < MAX_IVR_EVENT_IN_FEATURE)
	{
		m_pIVREvent[m_numb_of_IVR_Event] = new CIVREvent;
		nStatus = m_pIVREvent[m_numb_of_IVR_Event]->DeSerializeXml(pEventNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(m_pIVREvent[m_numb_of_IVR_Event]);
			return nStatus;
		}

		m_numb_of_IVR_Event++;

		GET_NEXT_CHILD_NODE(pActionNode, "IVR_EVENT", pEventNode);
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CIVRFeature::GetFeatureOpcode() const
{
	return m_feature_opcode;
}

//--------------------------------------------------------------------------
void CIVRFeature::SetFeatureOpcode(const WORD opcode)
{
	m_feature_opcode = opcode;
}

//--------------------------------------------------------------------------
BYTE CIVRFeature::GetEnableDisable() const
{
	return m_enable_disable;
}

//--------------------------------------------------------------------------
void CIVRFeature::SetEnableDisable(const BYTE yes_no)
{
	m_enable_disable = yes_no;
}

//--------------------------------------------------------------------------
WORD CIVRFeature::GetIVREventNumber() const
{
	return m_numb_of_IVR_Event;
}

//--------------------------------------------------------------------------
int CIVRFeature::AddEvent(CIVREvent& other)
{
	int status = STATUS_OK;

	if (m_numb_of_IVR_Event >= MAX_IVR_EVENT_IN_FEATURE)
		return STATUS_MAX_IVR_EVENT_IN_FEATURE_EXCEEDED;

	if (FindEvent(other) != NOT_FIND)
		return STATUS_IVR_EVENT_IN_FEATURE_EXISTS;

	m_pIVREvent[m_numb_of_IVR_Event] = new CIVREvent(other);

	m_numb_of_IVR_Event++;

	return status;
}

//--------------------------------------------------------------------------
int CIVRFeature::CancelEvent(const WORD event_opcode)
{
	int status = STATUS_OK;

	int ind = FindEvent(event_opcode);
	if (ind == NOT_FIND)
		return STATUS_IVR_EVENT_IN_FEATURE_NOT_EXISTS;

	PDELETE(m_pIVREvent[ind]);

	int i;
	for (i = 0; i < (int)m_numb_of_IVR_Event; i++)
	{
		if (m_pIVREvent[i] == NULL)
			break;
	}

	int j;
	for (j = i; j < (int)m_numb_of_IVR_Event-1; j++)
	{
		m_pIVREvent[j] = m_pIVREvent[j+1];
	}

	m_pIVREvent[m_numb_of_IVR_Event-1] = NULL;
	m_numb_of_IVR_Event--;

	return status;
}

//--------------------------------------------------------------------------
int CIVRFeature::FindEvent(const CIVREvent& other)
{
	for (int i = 0; i < (int)m_numb_of_IVR_Event; i++)
	{
		if (m_pIVREvent[i] != NULL)
		{
			if (m_pIVREvent[i]->GetEventOpcode() == other.GetEventOpcode())
				return i;
		}
	}
	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CIVRFeature::FindEvent(const WORD event_opcode)
{
	for (int i = 0; i < (int)m_numb_of_IVR_Event; i++)
	{
		if (m_pIVREvent[i] != NULL)
		{
			if (m_pIVREvent[i]->GetEventOpcode() == event_opcode)
				return i;
		}
	}
	return NOT_FIND;
}

//--------------------------------------------------------------------------
CIVREvent* CIVRFeature::GetCurrentIVREvent(const WORD event_opcode) const
{
	for (int i = 0; i < (int)m_numb_of_IVR_Event; i++)
	{
		if (m_pIVREvent[i] != NULL)
		{
			if (m_pIVREvent[i]->GetEventOpcode() == event_opcode)
				return m_pIVREvent[i];
		}
	}
	return NULL;
}

//--------------------------------------------------------------------------
CIVREvent* CIVRFeature::GetFirstEvent()
{
	m_ind_event = 1;
	return m_pIVREvent[0];
}

//--------------------------------------------------------------------------
CIVREvent* CIVRFeature::GetNextEvent()
{
	if (m_ind_event >= m_numb_of_IVR_Event)
		return NULL;

	return m_pIVREvent[m_ind_event++];
}

//--------------------------------------------------------------------------
CIVREvent* CIVRFeature::GetIVREventInPos(const WORD pos) const
{
	CIVREvent* pIVREvent = NULL;

	if (pos < MAX_IVR_EVENT_IN_FEATURE && pos < GetIVREventNumber())
		pIVREvent = m_pIVREvent[pos];

	return pIVREvent;
}

//--------------------------------------------------------------------------
const char* CIVRFeature::GetMsgFileName(const WORD event_opcode, const WORD language_pos, int& status) const
{
	const char* pFileName = NULL;

	status = STATUS_OK;

	CIVREvent* pIVREvent = GetCurrentIVREvent(event_opcode);
	if (pIVREvent)
	{
		const CIVRMessage* pIVRMessage = pIVREvent->GetCurrentIVRMessage(language_pos);

		if (pIVRMessage)
			pFileName =  pIVRMessage->GetMsgFileName();
		else
			status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;
	}
	else
		status = STATUS_IVR_EVENT_IN_FEATURE_NOT_EXISTS;

	return pFileName;
}

//--------------------------------------------------------------------------
void CIVRFeature::SetMsgFileName(const WORD event_opcode, const WORD language_pos, const char* file_name, int& status)
{
	const char* pFileName = NULL;

	status = STATUS_OK;

	CIVREvent* pIVREvent = GetCurrentIVREvent(event_opcode);
	if (pIVREvent)
	{
		CIVRMessage* pIVRMessage = pIVREvent->GetCurrentIVRMessage(language_pos);

		if (pIVRMessage)
			pIVRMessage->SetMsgFileName(file_name);
		else
			status = STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;
	}
	else
		status = STATUS_IVR_EVENT_IN_FEATURE_NOT_EXISTS;
}

//--------------------------------------------------------------------------
int CIVRFeature::AddLanguage(const WORD language_pos)
{
	int status = STATUS_OK;
	for (int i = 0; i < (int)m_numb_of_IVR_Event; i++)
		status = m_pIVREvent[i]->AddLanguage(language_pos);

	return status;
}

//--------------------------------------------------------------------------
int CIVRFeature::CancelLanguage(const WORD language_pos)
{
	int status = STATUS_OK;
	for (int i = 0; i < (int)m_numb_of_IVR_Event; i++)
		status = m_pIVREvent[i]->CancelLanguage(language_pos);

	return status;
}

//--------------------------------------------------------------------------
int CIVRFeature::NewDirectory(const char* under_dirName)
{
	TRACEINTO << " m_feature_opcode:" << (DWORD)m_feature_opcode;

	if (!under_dirName)
	{
		TRACEINTO << " Error : under_dirName == NULL";
		return STATUS_ILLEGAL;
	}

	int  status = STATUS_OK;

	char feature_name[NEW_FILE_NAME_LEN];
	strncpy(feature_name, GetIVRFeatureDirectory(m_feature_opcode, status), sizeof(feature_name) - 1);
	feature_name[sizeof(feature_name) - 1] = '\0';

	if (status == STATUS_OK)
	{
		ALLOCBUFFER(dir_name, strlen(under_dirName) + NEW_FILE_NAME_LEN);

		sprintf(dir_name, "%s/%s", under_dirName, feature_name);

		BOOL ret = CreateDirectory(dir_name);
		if (FALSE == ret)
		{
			TRACEINTO << " CreateDirectory Error , dir name: " << dir_name;
			status = STATUS_ILLEGAL;
		}

		for (int i = 0; i < (int)m_numb_of_IVR_Event; i++)
		{
			status = m_pIVREvent[i]->NewDirectory(dir_name);
			if (0 == strcmp("general", feature_name) || 0 == strcmp("rollcall", feature_name) || 0 == strcmp("nid", feature_name)
			    || 0 == strcmp("record", feature_name) || 0 == strcmp("playback", feature_name))
				break;  // the 'General' feature has only one sub-folder: 'gen'
		}
		DEALLOCBUFFER(dir_name);
	}
	else
	{
		TRACEINTO << " error in GetIVRFeatureDirectory, status=" << (DWORD)status;

	}

	return status;
}

//--------------------------------------------------------------------------
int CIVRFeature::CheckLeagalAcaFile(const char* language, WORD feature_opcode, WORD event_ind, WORD chkLevel)
{
	int status = STATUS_OK;

	// gets event opcode
	CIVREvent* pEvent = GetIVREventInPos(event_ind);
	if (!pEvent)
	{
		TRACEINTO << "CIVRFeature::CheckLeagalAcaFile - Failed, Event not valid, EventId:" << event_ind;
		return STATUS_IVR_INTERNAL_ERROR;
	}

	WORD event_opcode = pEvent->GetEventOpcode();

	// get feature path
	const char* feature_str = ::GetIVRFeatureDirectory(feature_opcode, status);
	if (status != STATUS_OK)
	{
		TRACEINTO << "CIVRFeature::CheckLeagalAcaFile - Failed, NULL feature path, FeatureOpcode:" << feature_opcode;
		return STATUS_IVR_INTERNAL_ERROR;
	}

	// gets event path
	const char* event_str = ::GetIVREventDirectory(event_opcode, status);
	if (status != STATUS_OK)
	{
		TRACEINTO << "CIVRFeature::CheckLeagalAcaFile - Failed, NULL event path, Feature:" << feature_str << ", EventOpcode:" << event_opcode;
		return STATUS_IVR_INTERNAL_ERROR;
	}

	// gets file name
	const char* msgName = GetMsgFileName(event_opcode, 0, status);
	if (status != STATUS_OK)
	{
		TRACEINTO << "CIVRFeature::CheckLeagalAcaFile - Failed, NULL event path, Event:" << event_str << ", EventOpcode:" << event_opcode;
		return status;
	}

	// checking name len
	status = CheckLegalIvrFileName(msgName, ".wav");
	if (status != STATUS_OK)
	{
		TRACEINTO << "CIVRFeature::CheckLeagalAcaFile - Failed, Illegal IVR File, FileName:" << msgName;
		return status;
	}

	// gets full pathname of the media file
	std::string fullPathMsgName;
	if (feature_str && event_str && language)
	{
		std::string ivrMsgType = language;
		ivrMsgType      += "/";
		ivrMsgType      += feature_str;
		ivrMsgType      += "/";
		ivrMsgType      += event_str;
		ivrMsgType      += "/";
		ivrMsgType      += msgName;
		fullPathMsgName  = IVR_FOLDER_MAIN;
		fullPathMsgName += IVR_FOLDER_MSG;
		fullPathMsgName += "/";
		fullPathMsgName += ivrMsgType;
	}
	else
	{
		TRACEINTO << "CIVRFeature::CheckLeagalAcaFile - Failed, NULL message path";
		return STATUS_IVR_INTERNAL_ERROR;
	}

	// checks according to check level: 0:no, 1:existence, 2:content
	switch (chkLevel)
	{
		case 0: break;  // don't check media files
		case 1:         // check if media file exists
		{
			if (!IsFileExists(fullPathMsgName))
			{
				TRACEINTO << "CIVRFeature::CheckLeagalAcaFile - Failed, File not exists, FileName:" << fullPathMsgName.c_str();
				status = STATUS_IVR_AUDIO_FILE_DOES_NOT_EXIST;
			}
			break;
		}

		case 2:     // check content of media file
		{
			status = CheckAcaIvrFile(fullPathMsgName.c_str());
			break;
		}
	} // switch

	return status;
}

//--------------------------------------------------------------------------
int CIVRFeature::CheckLegalIvrFileName(const char* msgName, const char* ext)
{
	// checking name len
	int len = strlen(msgName);
	if (0 == len || len > IVR_MAX_FILE_NAME_LEN)
		return STATUS_IVR_MSG_ILLEGAL_NAME;

	// checking legal name characters
	for (int i = 0; i < len; i++)
	{
		char l = msgName[i];
		if ((l != '_' && l != '-' && l != ' ' && l != '.') &&   // not permitted special characters
		    (l < 'a' || l > 'z') &&                             // not a-z
		    (l < 'A' || l > 'Z') &&                             // not A-Z
		    (l < '0' || l > '9'))                               // not a digit
		{
			return STATUS_IVR_MSG_ILLEGAL_NAME;
		}
	}

	// check the file extension if asked for
	if (NULL != ext)
	{
		int extLen = strlen(ext);
		if (extLen > 0)
		{
			if (extLen >= len)  // checking if name > extension
			{
				TRACEINTO << "CIVRFeature::CheckLegalIvrFileName - Failed, Illegal file extension, FileName:" << msgName;
				return STATUS_IVR_MSG_ILLEGAL_NAME;
			}

			int result = strncasecmp(ext, &msgName[len-extLen], extLen);
			if (0 != result)
			{
				TRACEINTO << "CIVRFeature::CheckLegalIvrFileName - Failed, Illegal file extension, FileName:" << msgName;
				return STATUS_IVR_MSG_ILLEGAL_NAME;
			}
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CIVRFeature::CheckAcaIvrFile(const char* fullPathMsgName)
{
	WORD msgDuration = 0;
	WORD checkSum    = 0;

	int status = CheckLegalFileAndGetParams(fullPathMsgName, &msgDuration, &checkSum);
	// checking legal duration of IVR prompt in IVR service
	if (msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
	{
		status = STATUS_FAIL;
		TRACEINTO << "CIVRFeature::CheckLegalIvrFileName - Failed, Duration is more than permitted, FileName:" << fullPathMsgName;
	}

	return status;
}

//--------------------------------------------------------------------------
int CIVRFeature::CheckLegalFileAndGetParams(const char* fullPathMsgName, WORD* ivrMsgDuration, WORD* ivrMsgCheckSum) const
{
	// checks that the struct are without alignment
	if ((RIFF_HEADER_SIZE != sizeof(TRIFFHeader)) ||
	    (WAV_CHUNK_SIZE != sizeof(TWaveChunk)) ||
	    (FMT_CHUNK_DATA_SIZE != sizeof(TFmtChunkData))) // checks that there is no compilation alignment
	{
		PASSERTMSG_AND_RETURN_VALUE(1, "CIVRFeature::CheckLegalFileAndGetParams - Failed, Struct alignment error", STATUS_FAIL);
		return STATUS_FAIL;
	}

	if (NULL == fullPathMsgName)
		return STATUS_FAIL;

	if (!IsFileExists(fullPathMsgName))
	{
		TRACEINTO << "CIVRFeature::CheckLegalFileAndGetParams - Failed, File not exists, FileName:" << fullPathMsgName;
		return STATUS_FILE_NOT_EXISTS;
	}

	FILE* infile = fopen(fullPathMsgName, "rb");
	if (NULL == infile)
	{
		TRACEINTO << "CIVRFeature::CheckLegalFileAndGetParams - Failed, File open error, FileName:" << fullPathMsgName;
		return STATUS_FILE_OPEN_ERROR;
	}

	// the logic is as follows:
	// to read the base header (RIFF) and checks that this is a WAVE file. The WAVE file has
	// at least 2 chunks: "fmt " and "data". In most cases the "fmt " comes before the "data"
	// to search for "fmt " or "data" chunk (read chunks until reach to it or until read error)
	// to verify the "fmt " chunk parameters
	// gets the "data" size

	int status = STATUS_OK;
	std::string error  = "";

	// checks the RIFF header
	if (STATUS_OK == status)
	{
		TRIFFHeader riffHeader;
		int numRead = fread(&riffHeader, 1, sizeof(TRIFFHeader), infile);
		if (numRead != RIFF_HEADER_SIZE)
		{
			status = STATUS_FILE_READ_ERROR;
			error  = "(RIFF header size), : ";
		}

		if (STATUS_OK == status)
			if (0 != strncmp("RIFF", riffHeader.acHeader, 4))
			{
				status = STATUS_IVR_INVALID_AUDIO_FILE;
				error  = "(RIFF), : ";
			}

		if (STATUS_OK == status)
			if (0 != strncmp("WAVE", riffHeader.acType, 4))
			{
				status = STATUS_IVR_INVALID_AUDIO_FILE;
				error  = "(WAVE), : ";
			}

	}

	if (STATUS_OK == status)
	{
		TWaveChunk waveChunk;
		int fmtFound = 0;
		int numRead  = 0;

		while (TRUE)
		{
			// read next WAV chunk (only ID and Size)
			numRead = fread(&waveChunk, 1, sizeof(TWaveChunk), infile);
			if (numRead != WAV_CHUNK_SIZE)    // read error
			{
				status = STATUS_FILE_READ_ERROR;
				error  = "(chunk header read error), : ";
				break;
			}

			// checks WAV chunk legality
			if (0 == waveChunk.unChunkSize)   // chunk size error
			{
				status = STATUS_IVR_INVALID_AUDIO_FILE;
				error  = "(chunk size 0), : ";
				break;
			}

			// seek size to the next chunk (skips the chunk data)
			DWORD seekSize = waveChunk.unChunkSize;                           // seek size to reach to the next chunk start

			// checking the "fmt" chunk
			if (0 == fmtFound && 0 == strncmp("fmt", waveChunk.acChunkId, 3)) // checks "fmt" header
			{
				if (waveChunk.unChunkSize < FMT_CHUNK_DATA_SIZE)                // error in file format
				{
					status = STATUS_IVR_INVALID_AUDIO_FILE;
					error  = "(fmt chunk header error (size)), : ";
					break;
				}

				// read fmt header
				TFmtChunkData fmtChunk;
				numRead = fread(&fmtChunk, 1, sizeof(TFmtChunkData), infile);
				if (numRead != FMT_CHUNK_DATA_SIZE) // read error
				{
					status = STATUS_FILE_READ_ERROR;
					error  = "(fmt chunk read error), : ";
					break;
				}

				// checks fmt parameters
				if ((fmtChunk.usFormatTag != 1) ||          // PCM uncompressed
				    (fmtChunk.usChannels != 1) ||           // MONO
				    (fmtChunk.unSamplesPerSec != 16000) ||  // Samples per second
				    (fmtChunk.usBitsPerSample != 16))       // bits per sample
				{
					status = STATUS_IVR_INVALID_AUDIO_FILE;
					error  = "(fmt parameters), : ";
					break;
				}

				seekSize -= FMT_CHUNK_DATA_SIZE;        // as we read fmt it and remains less to skip ("extra" if exists)

				// sign "fmt" as already read
				fmtFound = 1;
			}

			// checking the "data" chunk
			if (0 == strncmp("data", waveChunk.acChunkId, 4))
			{
				if (0 == fmtFound)                        // unsupported format: data chunk before fmt chunk
				{
					status = STATUS_IVR_INVALID_AUDIO_FILE; // should be unsupported and not illegal
					error  = "(data chunk before fmt chunk), : ";
					break;
				}

				if (0 == waveChunk.unChunkSize)           // illegal data size
				{
					status = STATUS_IVR_INVALID_AUDIO_FILE; // should be unsupported and not illegal
					error  = "(data chunk size is illegal (0)), : ";
					break;
				}

				(*ivrMsgCheckSum) = waveChunk.unChunkSize;                              // just a unique number for this file
				WORD fileMsgDuration = (WORD) ((waveChunk.unChunkSize-1) / 32000) + 1;  // 1 second = 32000 bytes
				(*ivrMsgDuration) = fileMsgDuration;

				break;                                                                  // data and fmt are OK
			}

			// seek to the next chunk start
			if (seekSize)
			{
				int rt = fseek(infile, seekSize, SEEK_CUR);
				if (rt != 0) // seek error
				{
					status = STATUS_FILE_SEEK_ERROR;
					error  = "(seek error), : ";
					break;
				}
			}
		} // while
	}   // if

	fclose(infile);

	if (status != STATUS_OK)
	{
		TRACEINTO << "CIVRFeature::CheckLegalFileAndGetParams - Failed, WAV format error, FileName:" << fullPathMsgName << ", Error:" << error;
	}

	// //////////////////////////////// temp
	if (status != STATUS_OK)
	{
		(*ivrMsgDuration) = (WORD)0;
		(*ivrMsgCheckSum) = 0;
	}

	// //////////////////////////////// temp

	return status;
}

//--------------------------------------------------------------------------
BOOL CIVRFeature::IsIdenticalToCurrent(const CIVRFeature* other) const
{
	if (m_feature_opcode != other->m_feature_opcode)
		return FALSE;

	if (m_enable_disable != other->m_enable_disable)
		return FALSE;

	if (m_numb_of_IVR_Event != other->m_numb_of_IVR_Event)
		return FALSE;

	if (m_Save_numb_of_IVR_Event != other->m_Save_numb_of_IVR_Event)
		return FALSE;

	for (int i = 0; i < m_numb_of_IVR_Event; i++)
	{
		if (FALSE == m_pIVREvent[i]->IsIdenticalToCurrent(other->m_pIVREvent[i]))
			return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVREvent
////////////////////////////////////////////////////////////////////////////
CIVREvent::CIVREvent()
{
	// defaults
	m_event_opcode     = 0;
	m_numb_of_messages = 0;

	for (int i = 0; i < MAX_LANG_IN_IVR_SERVICE; i++)
		m_pIVRMessage[i] = NULL;

	m_ind_message = 0;
}

//--------------------------------------------------------------------------
CIVREvent::CIVREvent(const CIVREvent& other)
          :CPObject(other)
{
	for (int i = 0; i < MAX_LANG_IN_IVR_SERVICE; i++)
		m_pIVRMessage[i] = NULL;

	*this = other;
}

//--------------------------------------------------------------------------
CIVREvent& CIVREvent::operator=(const CIVREvent& other)
{
	m_event_opcode     = other.m_event_opcode;
	m_numb_of_messages = other.m_numb_of_messages;
	m_ind_message      = other.m_ind_message;

	for (int i = 0; i < MAX_LANG_IN_IVR_SERVICE; i++)
	{
		PDELETE(m_pIVRMessage[i]);

		if (other.m_pIVRMessage[i])
			m_pIVRMessage[i] = new CIVRMessage(*other.m_pIVRMessage[i]);
	}

	return *this;
}

//--------------------------------------------------------------------------
CIVREvent::~CIVREvent()
{
	for (int i = 0; i < MAX_LANG_IN_IVR_SERVICE; i++)
		PDELETE(m_pIVRMessage[i]);
}

//--------------------------------------------------------------------------
void CIVREvent::Serialize(WORD format, std::ostream& m_ostr)
{
	// assuming format = OPERATOR_MCMS
	m_ostr <<  m_event_opcode  << "\n";
	m_ostr <<  m_numb_of_messages  << "\n";

	for (int i = 0; i < m_numb_of_messages; i++)
		m_pIVRMessage[i]->Serialize(format, m_ostr);
}

//--------------------------------------------------------------------------
void CIVREvent::DeSerialize(WORD format, std::istream& m_istr)
{
	// assuming format = OPERATOR_MCMS
	m_istr >> m_event_opcode;
	m_istr >> m_numb_of_messages;

	for (int i = 0; i < m_numb_of_messages; i++)
	{
		m_pIVRMessage[i] = new CIVRMessage;
		m_pIVRMessage[i]->DeSerialize(format, m_istr);
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVREvent::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pEventNode = NULL;
	pEventNode = pFatherNode->AddChildNode("IVR_EVENT");

	pEventNode->AddChildNode("EVENT_TYPE", m_event_opcode, IVR_EVENT_TYPE_ENUM);

	for (int i = 0; i < m_numb_of_messages; i++)
		m_pIVRMessage[i]->SerializeXml(pEventNode);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVREvent::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "EVENT_TYPE", &m_event_opcode, IVR_EVENT_TYPE_ENUM);

	// cleanup
	for (int i = 0; i < m_numb_of_messages; i++)
		POBJDELETE(m_pIVRMessage[i]);

	m_numb_of_messages = 0;

	CXMLDOMElement* pMessageNode = NULL;
	GET_FIRST_CHILD_NODE(pActionNode, "IVR_MESSAGE", pMessageNode);

	m_pIVRMessage[0] = new CIVRMessage; // adds an empty IvrMessage
	if (pMessageNode)
	{
		nStatus = m_pIVRMessage[0]->DeSerializeXml(pMessageNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(m_pIVRMessage[0]);
			return nStatus;
		}
	}

	m_numb_of_messages = 1;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CIVREvent::GetEventOpcode() const
{
	return m_event_opcode;
}

//--------------------------------------------------------------------------
void CIVREvent::SetEventOpcode(const WORD event_opcode)
{
	m_event_opcode = event_opcode;
}

//--------------------------------------------------------------------------
WORD CIVREvent::GetIVRMessageNumber() const
{
	return m_numb_of_messages;
}

//--------------------------------------------------------------------------
int CIVREvent::AddMessage(CIVRMessage& other)
{
	int status = STATUS_OK;

	if (m_numb_of_messages >= MAX_LANG_IN_IVR_SERVICE)
		return STATUS_THE_MAXIMUM_NUMBER_OF_LANGUAGES_IN_IVR_SERVICE_IS_EXCEEDED;

	if (FindMessage(other) != NOT_FIND)
		return STATUS_THIS_LANGUAGE_ALREADY_EXISTS_IN_THE_IVR_SERVICE;

	m_pIVRMessage[m_numb_of_messages] = new CIVRMessage(other);
	m_pIVRMessage[m_numb_of_messages]->SetLanguagePos(0);

	m_numb_of_messages++;

	return status;
}

//--------------------------------------------------------------------------
int CIVREvent::CancelMessage(const WORD language)
{
	int status = STATUS_OK;

	int ind = FindMessage(language);
	if (ind == NOT_FIND) return STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;

	PDELETE(m_pIVRMessage[ind]);

	int i;
	for (i = 0; i < (int)m_numb_of_messages; i++)
	{
		if (m_pIVRMessage[i] == NULL)
			break;
	}

	int j;
	for (j = i; j < (int)m_numb_of_messages-1; j++)
	{
		m_pIVRMessage[j] = m_pIVRMessage[j+1];
		m_pIVRMessage[j]->SetLanguagePos(j+1);
	}

	m_pIVRMessage[m_numb_of_messages-1] = NULL;
	m_numb_of_messages--;

	return status;
}

//--------------------------------------------------------------------------
int CIVREvent::FindMessage(const CIVRMessage& other)
{
	for (int i = 0; i < (int)m_numb_of_messages; i++)
	{
		if (m_pIVRMessage[i] != NULL)
		{
			return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CIVREvent::FindMessage(const WORD language_pos)
{
	for (int i = 0; i < (int)m_numb_of_messages; i++)
	{
		if (m_pIVRMessage[i] != NULL)
		{
			return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
CIVRMessage* CIVREvent::GetCurrentIVRMessage(const WORD language_pos) const
{
	for (int i = 0; i < (int)m_numb_of_messages; i++)
	{
		if (m_pIVRMessage[i] != NULL)
		{
			return m_pIVRMessage[i];
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------
CIVRMessage* CIVREvent::GetFirstMessage()
{
	m_ind_message = 1;
	return m_pIVRMessage[0];
}

//--------------------------------------------------------------------------
CIVRMessage* CIVREvent::GetIVRMessageInPos(const WORD pos) const
{
	CIVRMessage* pIVRMessage = NULL;

	if (pos < GetIVRMessageNumber())
		pIVRMessage = m_pIVRMessage[pos];

	return pIVRMessage;
}

//--------------------------------------------------------------------------
int CIVREvent::AddLanguage(const WORD language_pos)
{
	CIVRMessage ivrMessage;
	return AddMessage(ivrMessage);
}

//--------------------------------------------------------------------------
int CIVREvent::CancelLanguage(const WORD language_pos)
{
	return CancelMessage(language_pos);
}

//--------------------------------------------------------------------------
int CIVREvent::NewDirectory(const char* under_dirName)
{
	if (!under_dirName)
		return STATUS_ILLEGAL;

	int status = STATUS_OK;

	char feature_name[NEW_FILE_NAME_LEN];
	strcpy_safe(feature_name, GetIVREventDirectory(m_event_opcode, status));

	if (status == STATUS_OK)
	{
		ALLOCBUFFER(dir_name, strlen(under_dirName) + NEW_FILE_NAME_LEN);

		sprintf(dir_name, "%s/%s", under_dirName, feature_name);

		BOOL ret = CreateDirectory(dir_name);
		if (FALSE == ret)
			status = STATUS_ILLEGAL;

		DEALLOCBUFFER(dir_name);
	}
	return status;
}

//--------------------------------------------------------------------------
BOOL CIVREvent::IsIdenticalToCurrent(CIVREvent* other)
{
	if (m_event_opcode != other->m_event_opcode)
		return FALSE;

	if (m_numb_of_messages != other->m_numb_of_messages)
		return FALSE;

	for (int i = 0; i < m_numb_of_messages; i++)
	{
		if (FALSE == m_pIVRMessage[i]->IsIdenticalToCurrent(other->m_pIVRMessage[i]))
			return FALSE;
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRMessage
////////////////////////////////////////////////////////////////////////////
CIVRMessage::CIVRMessage()
{
	m_language_pos     = 0; // key field
	m_msg_duration     = 10;
	m_msg_file_name[0] = '\0';
	m_msgCheckSum      = (WORD)(-1);
	m_msgLastModified  = 0;
}

//--------------------------------------------------------------------------
CIVRMessage::CIVRMessage(const CIVRMessage& other)
            :CPObject(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRMessage& CIVRMessage::operator=(const CIVRMessage& other)
{
	m_language_pos    = other.m_language_pos;
	m_msg_duration    = other.m_msg_duration;
	m_msgCheckSum     = other.m_msgCheckSum;
	m_msgLastModified = other.m_msgLastModified;

	strcpy_safe(m_msg_file_name, other.m_msg_file_name);

	return *this;
}

//--------------------------------------------------------------------------
CIVRMessage::~CIVRMessage()
{
}

//--------------------------------------------------------------------------
void CIVRMessage::Serialize(WORD format, std::ostream& m_ostr)
{
	// assuming format = OPERATOR_MCMS
	m_ostr <<  (WORD)m_language_pos  << "\n";
	m_ostr <<  m_msg_file_name  << "\n";

	if (format == NATIVE)
		m_ostr << (WORD)m_msg_duration << "\n";
}

//--------------------------------------------------------------------------
void CIVRMessage::DeSerialize(WORD format, std::istream& m_istr)
{
	// assuming format = OPERATOR_MCMS
	WORD tmp;

	m_istr >> tmp;
	m_language_pos = (WORD) tmp;
	m_istr.ignore(1);
	m_istr.getline(m_msg_file_name, IVR_MSG_NAME_LEN+1, '\n');

	if (format == NATIVE)
	{
		m_istr >> tmp;
		m_msg_duration = tmp;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRMessage::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pMessNode = pFatherNode->AddChildNode("IVR_MESSAGE");

	pMessNode->AddChildNode("LANGUAGE_NUMBER", m_language_pos);
	pMessNode->AddChildNode("MESSAGE_FILE_NAME", m_msg_file_name);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRMessage::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "LANGUAGE_NUMBER", &m_language_pos, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode, "MESSAGE_FILE_NAME", m_msg_file_name, _0_TO_IVR_MSG_NAME_LENGTH);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRMessage::SetLanguagePos(WORD language)
{
	m_language_pos = language;
}

//--------------------------------------------------------------------------
WORD CIVRMessage::GetLanguagePos() const
{
	return m_language_pos;
}

//--------------------------------------------------------------------------
void CIVRMessage::SetMsgFileName(const char* name)
{
	strcpy_safe(m_msg_file_name, name);
}

//--------------------------------------------------------------------------
const char* CIVRMessage::GetMsgFileName() const
{
	return m_msg_file_name;
}

//--------------------------------------------------------------------------
void CIVRMessage::SetMsgDuration(const WORD msgDuration)
{
	m_msg_duration = msgDuration;
}

//--------------------------------------------------------------------------
WORD CIVRMessage::GetMsgDuration() const
{
	return m_msg_duration;
}

//--------------------------------------------------------------------------
void CIVRMessage::SetMsgCheckSum(const WORD msgCheckSum)
{
	m_msgCheckSum = msgCheckSum;
}

//--------------------------------------------------------------------------
WORD CIVRMessage::GetMsgCheckSum() const
{
	return m_msgCheckSum;
}

//--------------------------------------------------------------------------
void CIVRMessage::SetMsgLastModified(const time_t msgLastModified)
{
	m_msgLastModified = msgLastModified;
}

//--------------------------------------------------------------------------
time_t CIVRMessage::GetMsgLastModified() const
{
	return m_msgLastModified;
}

//--------------------------------------------------------------------------
BOOL CIVRMessage::IsIdenticalToCurrent(CIVRMessage* other)
{
	if (m_language_pos != other->m_language_pos)
		return FALSE;

	if (0 != strncmp(m_msg_file_name, other->m_msg_file_name, IVR_MSG_NAME_LEN))
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRLangMenuFeature
////////////////////////////////////////////////////////////////////////////
CIVRLangMenuFeature::CIVRLangMenuFeature()
{
	m_feature_opcode   = IVR_FEATURE_LANG_MENU;
	m_current_language = 0;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_GET_LANGUAGE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_LANGUAGE_RETRY);
	AddEvent(ivrEvent);
}

//--------------------------------------------------------------------------
CIVRLangMenuFeature::CIVRLangMenuFeature(const CIVRLangMenuFeature& other)
                    :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRLangMenuFeature& CIVRLangMenuFeature::operator=(const CIVRLangMenuFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);
	m_current_language = other.m_current_language;

	return *this;
}

//--------------------------------------------------------------------------
CIVRLangMenuFeature::~CIVRLangMenuFeature()
{
}

//--------------------------------------------------------------------------
void CIVRLangMenuFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	CIVRFeature::Serialize(format, m_ostr);
	m_ostr <<  (WORD)m_current_language  << "\n";
}

//--------------------------------------------------------------------------
void CIVRLangMenuFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	WORD tmp;

	CIVRFeature::DeSerialize(format, m_istr);
	m_istr >> tmp;
	m_current_language = (WORD) tmp;
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRLangMenuFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pLangNode = pFatherNode->AddChildNode("LANGUAGE_MENU");

	CIVRFeature::SerializeXml(pLangNode);
	pLangNode->AddChildNode("CURRENT_LANGUAGE_NUMBER", m_current_language);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRLangMenuFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode, "CURRENT_LANGUAGE_NUMBER", &m_current_language, _0_TO_DWORD);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CIVRLangMenuFeature::GetCurrentLang() const
{
	return m_current_language;
}

//--------------------------------------------------------------------------
void CIVRLangMenuFeature::SetCurrentLang(const WORD lang)
{
	m_current_language = lang;
}

//--------------------------------------------------------------------------
BOOL CIVRLangMenuFeature::IsIdenticalToCurrent(CIVRLangMenuFeature* other)
{
	if (FALSE == CIVRFeature::IsIdenticalToCurrent(other))
		return FALSE;

	if (m_current_language != other->m_current_language)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRWelcomeFeature
////////////////////////////////////////////////////////////////////////////
CIVRWelcomeFeature::CIVRWelcomeFeature()
{
	m_feature_opcode                   = IVR_FEATURE_WELCOME;
	m_wait_for_operator_after_msgOnOff = NO;
	m_bEntranceMsg                     = NO;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_WELCOME_MSG);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_ENTRANCE_MSG);
	AddEvent(ivrEvent);
}

//--------------------------------------------------------------------------
CIVRWelcomeFeature::CIVRWelcomeFeature(const CIVRWelcomeFeature& other)
                   :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRWelcomeFeature& CIVRWelcomeFeature::operator=(const CIVRWelcomeFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);
	m_wait_for_operator_after_msgOnOff = other.m_wait_for_operator_after_msgOnOff;
	m_bEntranceMsg                     = other.m_bEntranceMsg;

	return *this;
}

//--------------------------------------------------------------------------
CIVRWelcomeFeature::~CIVRWelcomeFeature()
{
}

//--------------------------------------------------------------------------
void CIVRWelcomeFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	CIVRFeature::Serialize(format, m_ostr);
	m_ostr <<  (WORD)m_wait_for_operator_after_msgOnOff  << "\n";
	if (apiNum >= API_NUM_CONFPW_AS_LEADERPW || format != OPERATOR_MCMS)
		m_ostr <<  (WORD)m_bEntranceMsg  << "\n";
}

//--------------------------------------------------------------------------
void CIVRWelcomeFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	WORD tmp;

	CIVRFeature::DeSerialize(format, m_istr);
	m_istr >> tmp;
	m_wait_for_operator_after_msgOnOff = (BYTE) tmp;
	if (apiNum >= API_NUM_CONFPW_AS_LEADERPW || format != OPERATOR_MCMS)
	{
		m_istr >> tmp;
		m_bEntranceMsg = (BYTE) tmp;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRWelcomeFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	// PTRACE(eLevelInfoNormal,"CIVRWelcomeFeature::SerializeXml");
	CXMLDOMElement* pWelcomeNode = pFatherNode->AddChildNode("WELCOME_MSG");

	CIVRFeature::SerializeXml(pWelcomeNode);
	pWelcomeNode->AddChildNode("ON_HOLD_FOR_OPERATOR_ASSISTANCE", m_wait_for_operator_after_msgOnOff, _BOOL);
	pWelcomeNode->AddChildNode("CONF_WELCOME_MSG_ENABLED", m_bEntranceMsg, _BOOL);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRWelcomeFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode, "ON_HOLD_FOR_OPERATOR_ASSISTANCE", &m_wait_for_operator_after_msgOnOff, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "CONF_WELCOME_MSG_ENABLED", &m_bEntranceMsg, _BOOL);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
BYTE CIVRWelcomeFeature::GetWaitForOperatorAfterMsgOnOff() const
{
	return m_wait_for_operator_after_msgOnOff;
}

//--------------------------------------------------------------------------
void CIVRWelcomeFeature::SetWaitForOperatorAfterMsgOnOff(const BYTE wait_for_operator_after_msgOnOff)
{
	m_wait_for_operator_after_msgOnOff = wait_for_operator_after_msgOnOff;
}

//--------------------------------------------------------------------------
BYTE CIVRWelcomeFeature::GetbEntranceMsg() const
{
	return m_bEntranceMsg;
}

//--------------------------------------------------------------------------
void CIVRWelcomeFeature::SetbEntranceMsg(const BYTE bEntranceMsg)
{
	m_bEntranceMsg = bEntranceMsg;
}

//--------------------------------------------------------------------------
int CIVRWelcomeFeature::IsLegalService(WORD chkLevel, std::string& err, const char* language)
{
	int status = STATUS_OK;

	if (!GetEnableDisable())  // feature not in use
		return STATUS_OK;

	status = CheckLeagalAcaFile(language, IVR_FEATURE_WELCOME, 0, chkLevel);
	if (STATUS_OK != status)
	{
		err = " Welcome Message is missing or illegal ";
	}
	return status;
}

//--------------------------------------------------------------------------
BOOL CIVRWelcomeFeature::IsIdenticalToCurrent(CIVRWelcomeFeature* other)
{
	if (FALSE == CIVRFeature::IsIdenticalToCurrent(other))
		return FALSE;

	if (m_bEntranceMsg != other->m_bEntranceMsg)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRConfPasswordFeature
////////////////////////////////////////////////////////////////////////////
CIVRConfPasswordFeature::CIVRConfPasswordFeature()
{
	m_feature_opcode = IVR_FEATURE_CONF_PASSWORD;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_GET_CONFERENCE_PASSWORD);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_CONFERENCE_PASSWORD_RETRY);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_GET_DIGIT);
	AddEvent(ivrEvent);

	// initialize the conference password tab.
	m_DialOutEntryPassword = REQUEST_PASSWORD;
	m_DialInEntryPassword  = REQUEST_PASSWORD;
}

//--------------------------------------------------------------------------
CIVRConfPasswordFeature::CIVRConfPasswordFeature(const CIVRConfPasswordFeature& other)
                        :CIVRFeature(other)
{
	*this                  = other;
	m_DialOutEntryPassword = other.m_DialOutEntryPassword;
	m_DialInEntryPassword  = other.m_DialInEntryPassword;
}

//--------------------------------------------------------------------------
CIVRConfPasswordFeature& CIVRConfPasswordFeature::operator=(const CIVRConfPasswordFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);
	m_DialOutEntryPassword = other.m_DialOutEntryPassword;
	m_DialInEntryPassword  = other.m_DialInEntryPassword;

	return *this;
}

//--------------------------------------------------------------------------
CIVRConfPasswordFeature::~CIVRConfPasswordFeature()
{
}

//--------------------------------------------------------------------------
void CIVRConfPasswordFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	CIVRFeature::Serialize(format, m_ostr);

	if (apiNum >= API_NUM_IVR_PASSWORD || format != OPERATOR_MCMS)
		m_ostr <<  (WORD)m_DialOutEntryPassword  << "\n";

	if (apiNum >= API_NUM_IVR_PASSWORD || format != OPERATOR_MCMS)
		m_ostr <<  (WORD)m_DialInEntryPassword   << "\n";
}

//--------------------------------------------------------------------------
void CIVRConfPasswordFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	WORD tmp = 0;

	// assuming format = OPERATOR_MCMS
	CIVRFeature::DeSerialize(format, m_istr);

	if (apiNum >= API_NUM_IVR_PASSWORD || format != OPERATOR_MCMS)
	{
		m_istr >> tmp;
		m_DialOutEntryPassword = (BYTE)tmp;
	}

	if (apiNum >= API_NUM_IVR_PASSWORD || format != OPERATOR_MCMS)
	{
		m_istr >> tmp;
		m_DialInEntryPassword = (BYTE)tmp;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRConfPasswordFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pPwdNode = pFatherNode->AddChildNode("CONF_PASSWORD");

	CIVRFeature::SerializeXml(pPwdNode);
	pPwdNode->AddChildNode("DIAL_OUT", m_DialOutEntryPassword, IVR_REQUEST_PWD_ENUM);
	pPwdNode->AddChildNode("DIAL_IN", m_DialInEntryPassword, IVR_REQUEST_PWD_ENUM);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRConfPasswordFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode, "DIAL_OUT", &m_DialOutEntryPassword, IVR_REQUEST_PWD_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "DIAL_IN", &m_DialInEntryPassword, IVR_REQUEST_PWD_ENUM);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
BYTE CIVRConfPasswordFeature::GetDialOutEntryPassword() const
{
	return m_DialOutEntryPassword;
}
//--------------------------------------------------------------------------
void CIVRConfPasswordFeature::SetDialOutEntryPassword(const BYTE DialOutEntryPassword)
{
	m_DialOutEntryPassword = DialOutEntryPassword;
}
//--------------------------------------------------------------------------
BYTE CIVRConfPasswordFeature::GetDialInEntryPassword() const
{
	return m_DialInEntryPassword;
}
//--------------------------------------------------------------------------
void CIVRConfPasswordFeature::SetDialInEntryPassword(const BYTE DialInEntryPassword)
{
	m_DialInEntryPassword = DialInEntryPassword;
}

//--------------------------------------------------------------------------
int CIVRConfPasswordFeature::IsLegalService(WORD chkLevel, std::string& err, const char* language)
{
	int status = STATUS_OK;

	if (!GetEnableDisable())  // legal state
		return STATUS_OK;

	if (status == STATUS_OK)
	{
		// checking 3 events (get-PW, PW-Retry, Enter-Any-Key)
		WORD NidEvent;
		for (NidEvent = 0; NidEvent < 3; NidEvent++)
		{
			status = CheckLeagalAcaFile(language, IVR_FEATURE_CONF_PASSWORD, NidEvent, chkLevel);
			if (STATUS_OK != status)
			{
				err = " Illegal PW ACA file ";
				break;
			}
		}
	}

	return status;
}

//--------------------------------------------------------------------------
BOOL CIVRConfPasswordFeature::IsIdenticalToCurrent(const CIVRConfPasswordFeature* other) const
{
	if (FALSE == CIVRFeature::IsIdenticalToCurrent(other))
		return FALSE;

	if (m_DialOutEntryPassword != other->m_DialOutEntryPassword)
		return FALSE;

	if (m_DialInEntryPassword != other->m_DialInEntryPassword)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRConfLeaderFeature
////////////////////////////////////////////////////////////////////////////
CIVRConfLeaderFeature::CIVRConfLeaderFeature()
{
	m_feature_opcode         = IVR_FEATURE_CONF_LEADER;
	m_szLeader_identifier[0] = '#';
	m_szLeader_identifier[1] = '\0';
	m_bConfPwAsLeaderPw      = YES; // force YES in RMX
	m_bIsBillingCode         = NO;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_GET_LEADER_IDENTIFIER);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_GET_LEADER_PASSWORD);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_LEADER_PASSWORD_RETRY);
	AddEvent(ivrEvent);
}

//--------------------------------------------------------------------------
CIVRConfLeaderFeature::CIVRConfLeaderFeature(const CIVRConfLeaderFeature& other)
                      :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRConfLeaderFeature& CIVRConfLeaderFeature::operator=(const CIVRConfLeaderFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);
	strcpy_safe(m_szLeader_identifier, other.m_szLeader_identifier);

	m_bConfPwAsLeaderPw = other.m_bConfPwAsLeaderPw;
	m_bConfPwAsLeaderPw = YES; // force YES in RMX

	m_bIsBillingCode = other.m_bIsBillingCode;
	return *this;
}

//--------------------------------------------------------------------------
CIVRConfLeaderFeature::~CIVRConfLeaderFeature()
{
}

//--------------------------------------------------------------------------
void CIVRConfLeaderFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	WORD tmp = 0;

	CIVRFeature::Serialize(format, m_ostr);
	m_ostr <<  tmp  << "\n";                                                // added for  m_istr.ignore(1);
	m_ostr <<  m_szLeader_identifier  << "\n";

	if (apiNum >= API_NUM_CONFPW_AS_LEADERPW || format != OPERATOR_MCMS)    // amir: to be replaced...
		m_ostr <<  (WORD)m_bConfPwAsLeaderPw  << "\n";

	if (apiNum >= API_NUM_BILLING_CODE || format != OPERATOR_MCMS)
		m_ostr <<  (WORD)m_bIsBillingCode     << "\n";
}

//--------------------------------------------------------------------------
void CIVRConfLeaderFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	WORD tmp;
	// assuming format = OPERATOR_MCMS
	CIVRFeature::DeSerialize(format, m_istr);

	m_istr >> tmp;    // added for m_istr.ignore(1);

	m_istr.ignore(1);
	m_istr.getline(m_szLeader_identifier, MAX_DELIMETER_LEN+1, '\n');

	if (apiNum >= API_NUM_CONFPW_AS_LEADERPW || format != OPERATOR_MCMS)    // amir: to be replaced...
	{
		m_istr >> tmp;
		m_bConfPwAsLeaderPw = (BYTE)tmp;
		m_bConfPwAsLeaderPw = YES;                                            // force YES in RMX
	}

	if (apiNum >= API_NUM_BILLING_CODE || format != OPERATOR_MCMS)          // amir: to be replaced...
	{
		m_istr >> tmp;
		m_bIsBillingCode = (BYTE)tmp;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRConfLeaderFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pLeaderNode = pFatherNode->AddChildNode("CHAIRPERSON");

	CIVRFeature::SerializeXml(pLeaderNode);
	pLeaderNode->AddChildNode("CHAIRMAN_KEY", m_szLeader_identifier);
	pLeaderNode->AddChildNode("CHAIRMAN_PASSWORD_AS_CONF_PASSWORD", m_bConfPwAsLeaderPw, _BOOL);
	pLeaderNode->AddChildNode("BILLING_CODE", m_bIsBillingCode, _BOOL);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRConfLeaderFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int  nStatus   = STATUS_OK;
	BYTE Delimiter = 0;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	memset(m_szLeader_identifier, '\0', MAX_DELIMETER_LEN);

	GET_VALIDATE_CHILD(pActionNode, "CHAIRMAN_KEY", &Delimiter, DELIMITERS_ENUM);
	if (nStatus == STATUS_OK)
		m_szLeader_identifier[0] = Delimiter;

	GET_VALIDATE_CHILD(pActionNode, "CHAIRMAN_PASSWORD_AS_CONF_PASSWORD", &m_bConfPwAsLeaderPw, _BOOL);
	m_bConfPwAsLeaderPw = YES; // force YES in RMX

	GET_VALIDATE_CHILD(pActionNode, "BILLING_CODE", &m_bIsBillingCode, _BOOL);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
const char* CIVRConfLeaderFeature::GetLeaderIdentifier() const
{
	return m_szLeader_identifier;
}

//--------------------------------------------------------------------------
void CIVRConfLeaderFeature::SetLeaderIdentifier(const char* szLeader_identifier)
{
	strcpy_safe(m_szLeader_identifier, szLeader_identifier);
}

//--------------------------------------------------------------------------
BYTE CIVRConfLeaderFeature::GetConfPwAsLeaderPw() const
{
	return m_bConfPwAsLeaderPw;
}

//--------------------------------------------------------------------------
void CIVRConfLeaderFeature::SetConfPwAsLeaderPw(const BYTE bConfPwAsLeaderPw)
{
	m_bConfPwAsLeaderPw = bConfPwAsLeaderPw;
	m_bConfPwAsLeaderPw = YES; // force YES in RMX
}
//--------------------------------------------------------------------------
BYTE CIVRConfLeaderFeature::GetIsBillingCode() const
{
	return m_bIsBillingCode;
}

//--------------------------------------------------------------------------
void CIVRConfLeaderFeature::SetIsBillingCode(const BYTE bIsBillingCode)
{
	m_bIsBillingCode = bIsBillingCode;
}
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
int CIVRConfLeaderFeature::IsLegalService(WORD chkLevel, std::string& err, const char* language)
{
	int status = STATUS_OK;

	if (!GetEnableDisable())  // legal state
		return STATUS_OK;

	if (status == STATUS_OK)
	{
		// checking 3 events (get-PW, PW-Retry, Enter-Any-Key)
		WORD NidEvent;
		for (NidEvent = 0; NidEvent < 3; NidEvent++)
		{
			status = CheckLeagalAcaFile(language, IVR_FEATURE_CONF_LEADER, NidEvent, chkLevel);
			if (STATUS_OK != status)
			{
				err = " Illegal Chair PW ACA file ";
				break;
			}
		}
	}
	return status;
}

//--------------------------------------------------------------------------
BOOL CIVRConfLeaderFeature::IsIdenticalToCurrent(CIVRConfLeaderFeature* other)
{
	if (FALSE == CIVRFeature::IsIdenticalToCurrent(other))
		return FALSE;

	if (0 != strncmp(m_szLeader_identifier, other->m_szLeader_identifier, MAX_DELIMETER_LEN))
		return FALSE;

	if (m_bConfPwAsLeaderPw != other->m_bConfPwAsLeaderPw)
		return FALSE;

	if (m_bIsBillingCode != other->m_bIsBillingCode)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRPersonalPINCodeFeature
////////////////////////////////////////////////////////////////////////////
CIVRPersonalPINCodeFeature::CIVRPersonalPINCodeFeature()
{
	m_feature_opcode              = IVR_FEATURE_PIN_CODE;
	m_quick_login_for_leaderOnOff = NO;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_GET_PIN_CODE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_PIN_CODE_RETRY);
	AddEvent(ivrEvent);
}

//--------------------------------------------------------------------------
CIVRPersonalPINCodeFeature::CIVRPersonalPINCodeFeature(const CIVRPersonalPINCodeFeature& other)
                           :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRPersonalPINCodeFeature& CIVRPersonalPINCodeFeature::operator=(const CIVRPersonalPINCodeFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);

	m_quick_login_for_leaderOnOff = other.m_quick_login_for_leaderOnOff;
	return *this;
}

//--------------------------------------------------------------------------
CIVRPersonalPINCodeFeature::~CIVRPersonalPINCodeFeature()
{
}

//--------------------------------------------------------------------------
void CIVRPersonalPINCodeFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	CIVRFeature::Serialize(format, m_ostr);
	m_ostr <<  (WORD)m_quick_login_for_leaderOnOff  << "\n";
}

//--------------------------------------------------------------------------
void CIVRPersonalPINCodeFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	WORD tmp;

	CIVRFeature::DeSerialize(format, m_istr);
	m_istr >> tmp;
	m_quick_login_for_leaderOnOff = (BYTE) tmp;
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRPersonalPINCodeFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pPinNode = pFatherNode->AddChildNode("PERSONAL_PIN_CODE");

	CIVRFeature::SerializeXml(pPinNode);
	pPinNode->AddChildNode("QUICK_LOGIN_ENABLED", m_quick_login_for_leaderOnOff, _BOOL);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRPersonalPINCodeFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode, "QUICK_LOGIN_ENABLED", &m_quick_login_for_leaderOnOff, _BOOL);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
BYTE CIVRPersonalPINCodeFeature::GetQuickLoginForLeaderOnOff() const
{
	return m_quick_login_for_leaderOnOff;
}

//--------------------------------------------------------------------------
void CIVRPersonalPINCodeFeature::SetQuickLoginForLeaderOnOff(const BYTE quick_login_for_leaderOnOff)
{
	m_quick_login_for_leaderOnOff = quick_login_for_leaderOnOff;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVROperAssistanceFeature
////////////////////////////////////////////////////////////////////////////
CIVROperAssistanceFeature::CIVROperAssistanceFeature()
{
	m_feature_opcode                  = IVR_FEATURE_OPER_ASSISTANCE;
	m_wait_for_operator_on_errorOnOff = NO;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_SYSTEM_DISCONNECT_MESSAGE);
	AddEvent(ivrEvent);
}

//--------------------------------------------------------------------------
CIVROperAssistanceFeature::CIVROperAssistanceFeature(const CIVROperAssistanceFeature& other)
                          :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVROperAssistanceFeature& CIVROperAssistanceFeature::operator=(const CIVROperAssistanceFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);

	m_wait_for_operator_on_errorOnOff = other.m_wait_for_operator_on_errorOnOff;

	return *this;
}

//--------------------------------------------------------------------------
CIVROperAssistanceFeature::~CIVROperAssistanceFeature()
{
}

//--------------------------------------------------------------------------
void CIVROperAssistanceFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	CIVRFeature::Serialize(format, m_ostr);

	m_ostr <<  (WORD)m_wait_for_operator_on_errorOnOff  << "\n";
}

//--------------------------------------------------------------------------
void CIVROperAssistanceFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	WORD tmp;

	// assuming format = OPERATOR_MCMS
	CIVRFeature::DeSerialize(format, m_istr);

	m_istr >> tmp;
	m_wait_for_operator_on_errorOnOff = (BYTE) tmp;
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVROperAssistanceFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pAssstncNode = pFatherNode->AddChildNode("OPERATOR_ASSISTANCE");

	CIVRFeature::SerializeXml(pAssstncNode);
	pAssstncNode->AddChildNode("ENABLED_ON_FAILURE", m_wait_for_operator_on_errorOnOff, _BOOL);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVROperAssistanceFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode, "ENABLED_ON_FAILURE", &m_wait_for_operator_on_errorOnOff, _BOOL);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
BYTE CIVROperAssistanceFeature::GetWaitForOperatorOnErrorOnOff() const
{
	return m_wait_for_operator_on_errorOnOff;
}

//--------------------------------------------------------------------------
void CIVROperAssistanceFeature::SetWaitForOperatorOnErrorOnOff(const BYTE wait_for_operator_on_errorOnOff)
{
	m_wait_for_operator_on_errorOnOff = wait_for_operator_on_errorOnOff;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRInvitePartyFeature
////////////////////////////////////////////////////////////////////////////
CIVRInvitePartyFeature::CIVRInvitePartyFeature()
                       :CIVRFeature()
{
	m_feature_opcode         = IVR_FEATURE_INVITE_PARTY;
	m_dtmfForwardDurationSec = 60;
	m_enable_disable         = YES;
	m_callOrderInterface[1]  = H323_INTERFACE_TYPE;
	m_callOrderInterface[2]  = SIP_INTERFACE_TYPE;
}

//--------------------------------------------------------------------------
CIVRInvitePartyFeature::CIVRInvitePartyFeature(const CIVRInvitePartyFeature& other)
                       :CIVRFeature(other)
{
	*this                    = other;
	m_interfaceCallOrder     = other.m_interfaceCallOrder;
	m_dtmfForwardDurationSec = other.m_dtmfForwardDurationSec;
	m_callOrderInterface     = other.m_callOrderInterface;
}

//--------------------------------------------------------------------------
CIVRInvitePartyFeature& CIVRInvitePartyFeature::operator=(const CIVRInvitePartyFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);
	m_interfaceCallOrder     = other.m_interfaceCallOrder;
	m_dtmfForwardDurationSec = other.m_dtmfForwardDurationSec;
	m_callOrderInterface     = other.m_callOrderInterface;
	return *this;
}

//--------------------------------------------------------------------------
void CIVRInvitePartyFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	m_ostr << m_callOrderInterface.size();

	for (map<WORD, WORD>::iterator interfaceIter = m_callOrderInterface.begin(); interfaceIter != m_callOrderInterface.end(); interfaceIter++)
	{
		m_ostr << (*interfaceIter).first;
		m_ostr << (*interfaceIter).second;
	}

	m_ostr << m_dtmfForwardDurationSec;
}

//--------------------------------------------------------------------------
map<WORD, WORD>& CIVRInvitePartyFeature::getInterfaceOrderMap()
{
	return m_interfaceCallOrder;
}

//--------------------------------------------------------------------------
WORD CIVRInvitePartyFeature::getDtmfForwardDuration()
{
	return m_dtmfForwardDurationSec;
}

//--------------------------------------------------------------------------
void CIVRInvitePartyFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	m_interfaceCallOrder.clear();
	m_callOrderInterface.clear();

	WORD interfaceCalligOrderSize;
	m_istr >> interfaceCalligOrderSize;

	for (int i = 0; i < interfaceCalligOrderSize; i++)
	{
		WORD interface;
		WORD order;

		m_istr >> order;
		m_istr >> interface;
		m_callOrderInterface[order] = interface;
		if (order != NONE_INTERFACE_TYPE)
		{
			m_interfaceCallOrder[interface] = order;
		}
	}

	m_istr >> m_dtmfForwardDurationSec;
}

//--------------------------------------------------------------------------
void CIVRInvitePartyFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pInvitePartyParamsNode = NULL;
	CXMLDOMElement* pFeatureNode           = NULL;
	CXMLDOMElement* pInterfaceNode         = NULL;
	CXMLDOMElement* pInterfaceOrderNode    = NULL;
	CXMLDOMElement* pdtmfForwardNode       = NULL;
	pInvitePartyParamsNode = pFatherNode->AddChildNode("INVITE_PARTICIPANT");

	pInterfaceOrderNode = pInvitePartyParamsNode->AddChildNode("DIAL_OUT_PROTOCOL_TYPE_LIST");

	for (map<WORD, WORD>::iterator interfaceIter = m_callOrderInterface.begin(); interfaceIter != m_callOrderInterface.end(); interfaceIter++)
	{
		pInterfaceNode = pInterfaceOrderNode->AddChildNode("PROTOCOL_TYPE_CONTENT");
		pInterfaceNode->AddChildNode("DIAL_OUT_PROTOCOL_TYPE", (*interfaceIter).second, INTERFACE_TYPE_ENUM);
	}

	pInvitePartyParamsNode->AddChildNode("DTMF_FORWARD_DURATION", m_dtmfForwardDurationSec, _0_TO_WORD);
}

//--------------------------------------------------------------------------
int CIVRInvitePartyFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int  nStatus = STATUS_OK;
	BYTE Delimiter;
	WORD InterfaceOrder = 1;

	CXMLDOMElement* pBaseNode    = NULL;
	CXMLDOMElement* pFeatureNode = NULL;
	m_interfaceCallOrder.clear();
	m_callOrderInterface.clear();

	GET_CHILD_NODE(pActionNode, "DIAL_OUT_PROTOCOL_TYPE_LIST", pFeatureNode);
	if (pFeatureNode)
	{
		CXMLDOMElement* pInterfaceNode = NULL;

		GET_FIRST_CHILD_NODE(pFeatureNode, "PROTOCOL_TYPE_CONTENT", pInterfaceNode);

		if (pInterfaceNode == NULL)
		{
			PTRACE(eLevelError, "CIVRInvitePartyFeature::DeSerializeXml pInterfaceNode == NULL");
		}

		while (pInterfaceNode)
		{
			WORD interfaceType = -1;

			GET_VALIDATE_CHILD(pInterfaceNode, "DIAL_OUT_PROTOCOL_TYPE", &interfaceType, INTERFACE_TYPE_ENUM);

			m_callOrderInterface[InterfaceOrder] = interfaceType;
			if (interfaceType != NONE_INTERFACE_TYPE)
			{
				m_interfaceCallOrder[interfaceType] = InterfaceOrder;
			}

			InterfaceOrder++;
			GET_NEXT_CHILD_NODE(pFeatureNode, "PROTOCOL_TYPE_CONTENT", pInterfaceNode);
		}
	}
	else
	{
		TRACEINTO << "CIVRInvitePartyFeature::DeSerializeXml - Failed, pFeatureNode is NULL";
	}

	GET_VALIDATE_CHILD(pActionNode, "DTMF_FORWARD_DURATION", &m_dtmfForwardDurationSec, _0_TO_WORD);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
CIVRInvitePartyFeature::~CIVRInvitePartyFeature()
{
	m_interfaceCallOrder.clear();
}

//--------------------------------------------------------------------------
BOOL CIVRInvitePartyFeature::IsIdenticalToCurrent(CIVRInvitePartyFeature* other)
{
	return CIVRFeature::IsIdenticalToCurrent(other);
}

//--------------------------------------------------------------------------
int CIVRInvitePartyFeature::IsLegalService(WORD chkLevel, std::string& err, const char* language)
{
	int status = STATUS_OK;

	if (!GetEnableDisable())  // feature not in use
		return STATUS_OK;

	status = CheckLeagalAcaFile(language, IVR_FEATURE_INVITE_PARTY, 0, chkLevel);
	if (STATUS_OK != status)
	{
		err = " Invite Party is missing or illegal ";
	}

	return status;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRGeneralMsgsFeature
////////////////////////////////////////////////////////////////////////////
CIVRGeneralMsgsFeature::CIVRGeneralMsgsFeature()
{
	m_feature_opcode = IVR_FEATURE_GENERAL;
	CIVREvent ivrEvent;

	// general events
	ivrEvent.SetEventOpcode(IVR_EVENT_BILLING_NUM);           // prompt for billing
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_REQUIRES_LEADER);       // requires leader
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_FIRST_TO_JOIN);         // first to join the conference
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_MUTE_ALL_ON);           // all participants are muted
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_MUTE_ALL_OFF);          // all participants are un-muted
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_CHAIR_DROPPED);         // chairperson has dropped the conf
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_END_TIME_ALERT);        // conf is about to end
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_CHANGE_PWDS_MENU);      // modify passwords menu
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_CHANGE_PWDS_CONF);      // enter new conf password
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_CHANGE_PWDS_LEADER);    // enter new leader password
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_CHANGE_PWD_CONFIRM);    // re-enter new password
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_CHANGE_PWD_INVALID);    // new password is invalid
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_CHANGE_PWD_OK);         // password updated
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_SELF_MUTE);             // party muted
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_SELF_UNMUTE);           // party un-muted
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_MENU_LEADER);           // Leader menu
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_MENU_SIMPLE);           // simple menu
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_MAX_PARTICIPANTS);      // Conference has reached max participants
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_RECORDING_IN_PROGRESS); // Conference is being recorded message
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_RECORDING_FAILED);      // Conference is being recorded message
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_SECURE_ON);             // set SECURED on
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_SECURE_OFF);            // set SECURED off
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_CONF_LOCK);             // conference is locked (or secured)
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_ENTER_DEST_NUM);
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_ILLEGAL_DEST_NUM);
	AddEvent(ivrEvent);


	ivrEvent.SetEventOpcode(IVR_EVENT_PLAY_DIAL_TONE);
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_PLAY_RINGING_TONE);
	AddEvent(ivrEvent);


	ivrEvent.SetEventOpcode(IVR_EVENT_NO_VIDEO_RESOURCES);
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_BLIP_ON_CASCADE_LINK);
	AddEvent(ivrEvent);


	ivrEvent.SetEventOpcode(IVR_EVENT_INVITE_PARTY);
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_REINVITE_PARTY);
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_PLAY_BUSY_MSG);
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_PLAY_NOANSWER_MSG);
	AddEvent(ivrEvent);

	ivrEvent.SetEventOpcode(IVR_EVENT_PLAY_WRONG_NUMBER_MSG);
	AddEvent(ivrEvent);

}

//--------------------------------------------------------------------------
CIVRGeneralMsgsFeature::CIVRGeneralMsgsFeature(const CIVRGeneralMsgsFeature& other)
                       :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRGeneralMsgsFeature& CIVRGeneralMsgsFeature::operator=(const CIVRGeneralMsgsFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);

	return *this;
}

//--------------------------------------------------------------------------
CIVRGeneralMsgsFeature::~CIVRGeneralMsgsFeature()
{
}

//--------------------------------------------------------------------------
void CIVRGeneralMsgsFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	if (apiNum >= API_NUM_GENERAL_MSGS || format != OPERATOR_MCMS)    // amir: to be replaced...
	{
		CIVRFeature::Serialize(format, m_ostr);
	}
}

//--------------------------------------------------------------------------
void CIVRGeneralMsgsFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	if (apiNum >= API_NUM_GENERAL_MSGS || format != OPERATOR_MCMS)    // amir: to be replaced...
	{
		CIVRFeature::DeSerialize(format, m_istr);
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRGeneralMsgsFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pGenNode = pFatherNode->AddChildNode("GENERAL_MSG");

	CIVRFeature::SerializeXml(pGenNode);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRGeneralMsgsFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CIVRGeneralMsgsFeature::IsLegalService(WORD chkLevel, std::string& err, const char* language)
{
	int status = STATUS_OK;

	// verify not more than max events
	if (m_numb_of_IVR_Event > MAX_IVR_EVENT_IN_FEATURE) // should not happened...
		m_numb_of_IVR_Event = MAX_IVR_EVENT_IN_FEATURE;

	// checking all events of general, if there is filename then check legality
	for (int w = 0; w < m_numb_of_IVR_Event; w++)
	{
		if (!m_pIVREvent[w])    // should not happened...
			continue;

		// gets file name
		const char* msgName = GetMsgFileName(m_pIVREvent[w]->m_event_opcode, 0, status);
		if (!msgName)
			continue; // file not have to be exists

		if (0 == strlen(msgName))
			continue; // file not have to be exists

		status = CheckLeagalAcaFile(language, IVR_FEATURE_GENERAL, w, chkLevel);
		if (STATUS_OK != status)
		{
			err = " General Message is missing or illegal ";
			break;
		}
	}
	return status;
}

//--------------------------------------------------------------------------
int CIVRGeneralMsgsFeature::IsLegalBillingCodeMsg(WORD chkLevel, std::string& err, const char* language)
{
	int status = STATUS_OK;

	int indBillingInGeneral = GetBillingMsgIndex();
	if (-1 == indBillingInGeneral)
	{
		TRACEINTO << "CIVRGeneralMsgsFeature::IsLegalBillingCodeMsg - Failed, Billing code index not found";
		err    = "Billing Code index not found";
		status = STATUS_IVR_INTERNAL_ERROR;   // index is missing
	}

	if (STATUS_OK == status)
	{
		status = CheckLeagalAcaFile(language, IVR_FEATURE_GENERAL, indBillingInGeneral, chkLevel);
		if (STATUS_OK != status)
			err = " General Message is missing or illegal ";
	}
	return status;
}

//--------------------------------------------------------------------------
void CIVRGeneralMsgsFeature::ChangeGeneralMsgTable(CIVRGeneralMsgsFeature* pGeneralMsg)
{
	// copy the files to the backup in case there was in the original of the general

	for (int h = 0; h < pGeneralMsg->m_numb_of_IVR_Event; h++)
	{
		for (int w = 0; w < m_numb_of_IVR_Event; w++)
		{
			if (pGeneralMsg->m_pIVREvent[h]->m_event_opcode == m_pIVREvent[w]->m_event_opcode)
			{
				if (m_pIVREvent[w]->m_numb_of_messages != 0)
				{
					PDELETE(pGeneralMsg->m_pIVREvent[h]);
					pGeneralMsg->m_pIVREvent[h] =  new CIVREvent(*m_pIVREvent[w]);
				}
				break;
			}
		}
	}

	m_numb_of_IVR_Event = pGeneralMsg->m_numb_of_IVR_Event;

	for (int z = 0; z < MAX_IVR_EVENT_IN_FEATURE; z++) // Delete the original general table
		PDELETE(m_pIVREvent[z]);

	for (int j = 0; j < MAX_IVR_EVENT_IN_FEATURE; j++) // Copy the new general table to the origial pointer
	{
		if (pGeneralMsg->m_pIVREvent[j] == NULL)
			m_pIVREvent[j] = NULL;
		else
		{
			m_pIVREvent[j] = new CIVREvent(*pGeneralMsg->m_pIVREvent[j]);
			if (0 == m_pIVREvent[j]->GetIVRMessageNumber())
			{
				CIVRMessage ivrMessage; // adds an empty IvrMessage
				m_pIVREvent[j]->AddMessage(ivrMessage);
			}
		}
	}
}

//--------------------------------------------------------------------------
int CIVRGeneralMsgsFeature::GetBillingMsgIndex()
{
	WORD numOfEvents = GetIVREventNumber();
	for (int i = 0; i < numOfEvents; i++)
	{
		CIVREvent* event = GetIVREventInPos(i);
		if (event)
			if (event->GetEventOpcode() == IVR_EVENT_BILLING_NUM)
				return i;
	}
	return -1;
}

//--------------------------------------------------------------------------
BOOL CIVRGeneralMsgsFeature::IsIdenticalToCurrent(CIVRGeneralMsgsFeature* other)
{
	return CIVRFeature::IsIdenticalToCurrent(other);
}


////////////////////////////////////////////////////////////////////////////
//                        CDTMFCode
////////////////////////////////////////////////////////////////////////////
CDTMFCode::CDTMFCode()
{
	// defaults
	m_DTMF_opcode     = 0;
	m_DTMF_permission = DTMF_USER_ACTION;
	m_DTMF_str[0]     = '\0';
	m_DTMF_len        = 0;
}

//--------------------------------------------------------------------------
CDTMFCode::CDTMFCode(const CDTMFCode& other)
	: CPObject(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CDTMFCode& CDTMFCode::operator=(const CDTMFCode& other)
{
	m_DTMF_opcode     = other.m_DTMF_opcode;
	m_DTMF_permission = other.m_DTMF_permission;
	m_DTMF_len        = other.m_DTMF_len;

	strcpy_safe(m_DTMF_str, other.m_DTMF_str);

	return *this;
}

//--------------------------------------------------------------------------
CDTMFCode::~CDTMFCode()
{
}

//--------------------------------------------------------------------------
void CDTMFCode::Serialize(WORD format, std::ostream& m_ostr)
{
	// assuming format = OPERATOR_MCMS
	m_ostr <<  m_DTMF_opcode  << "\n";
	m_ostr <<  (WORD) m_DTMF_permission  << "\n";
	m_ostr <<  m_DTMF_str  << "\n";
}

//--------------------------------------------------------------------------
void CDTMFCode::DeSerialize(WORD format, std::istream& m_istr)
{
	WORD tmp;

	// assuming format = OPERATOR_MCMS
	m_istr >> m_DTMF_opcode;

	m_istr >> tmp;
	m_DTMF_permission = (BYTE)tmp;

	m_istr.ignore(1);
	m_istr.getline(m_DTMF_str, DTMF_STRING_LEN+1, '\n');
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CDTMFCode::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pDtmfNode = pFatherNode->AddChildNode("DTMF_CODE");

	pDtmfNode->AddChildNode("OPCODE", m_DTMF_opcode, DTMF_OPCODE_ENUM);
	pDtmfNode->AddChildNode("PERMISSION", m_DTMF_permission, DTMF_PERMISSION_ENUM);
	pDtmfNode->AddChildNode("DTMF_STRING", m_DTMF_str);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CDTMFCode::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "OPCODE", &m_DTMF_opcode, DTMF_OPCODE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "PERMISSION", &m_DTMF_permission, DTMF_PERMISSION_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "DTMF_STRING", m_DTMF_str, _0_TO_DTMF_STRING_LENGTH);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CDTMFCode::SetDTMFOpcode(WORD opcode)
{
	m_DTMF_opcode = opcode;
}

//--------------------------------------------------------------------------
WORD CDTMFCode::GetDTMFOpcode() const
{
	return m_DTMF_opcode;
}

//--------------------------------------------------------------------------
void CDTMFCode::SetDTMFPermission(const BYTE permission)
{
	m_DTMF_permission = permission;
}

//--------------------------------------------------------------------------
WORD CDTMFCode::GetDTMFPermission() const
{
	return m_DTMF_permission;
}

//--------------------------------------------------------------------------
void CDTMFCode::SetDTMFStr(const char* DTMF_str)
{
	strcpy_safe(m_DTMF_str, DTMF_str);
}

//--------------------------------------------------------------------------
const char* CDTMFCode::GetDTMFStr() const
{
	return m_DTMF_str;
}

//--------------------------------------------------------------------------
WORD CDTMFCode::GetDTMFStrLen() const
{
	return m_DTMF_len;
}

//--------------------------------------------------------------------------
void CDTMFCode::SetDTMFStrLen()
{
	m_DTMF_len = strlen(m_DTMF_str);
}


//--------------------------------------------------------------------------
BOOL CDTMFCode::IsIdenticalToCurrent(CDTMFCode* other)
{
	if (m_DTMF_opcode != other->m_DTMF_opcode)
		return FALSE;

	if (m_DTMF_permission != other->m_DTMF_permission)
		return FALSE;

	if (0 != strncmp(m_DTMF_str, other->m_DTMF_str, DTMF_STRING_LEN))
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CDTMFCodeList
////////////////////////////////////////////////////////////////////////////
CDTMFCodeList::CDTMFCodeList()
{
	// defaults
	m_numb_of_DTMF_code = 0;
	m_ind_DTMF          = 0;

	memset(m_DTMF_code, 0, sizeof(m_DTMF_code));
}

//--------------------------------------------------------------------------
CDTMFCodeList::CDTMFCodeList(const CDTMFCodeList& other)
              :CPObject(other)
{
	memset(m_DTMF_code, 0, sizeof(m_DTMF_code));

	*this = other;
}

//--------------------------------------------------------------------------
CDTMFCodeList& CDTMFCodeList::operator=(const CDTMFCodeList& other)
{
	m_numb_of_DTMF_code = other.m_numb_of_DTMF_code;
	m_ind_DTMF          = other.m_ind_DTMF;

	for (int i = 0; i < MAX_DTMF_CODE_NUM; i++)
	{
		PDELETE(m_DTMF_code[i]);
		if (other.m_DTMF_code[i])
			m_DTMF_code[i] = new CDTMFCode(*other.m_DTMF_code[i]);
	}
	return *this;
}

//--------------------------------------------------------------------------
CDTMFCodeList::~CDTMFCodeList()
{
	for (int i = 0; i < MAX_DTMF_CODE_NUM; i++)
		PDELETE(m_DTMF_code[i]);
}

//--------------------------------------------------------------------------
void CDTMFCodeList::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS

	WORD numDTMFCodes = m_numb_of_DTMF_code;
	if (apiNum != 0 && apiNum < 250) // Working with operator 4 that cant handle more than 12 codes
		numDTMFCodes = min((short unsigned int)12, m_numb_of_DTMF_code);

	m_ostr <<  numDTMFCodes  << "\n";

	for (int i = 0; i < numDTMFCodes; i++)
		if (m_DTMF_code[i] != NULL)
			m_DTMF_code[i]->Serialize(format, m_ostr);
}

//--------------------------------------------------------------------------
void CDTMFCodeList::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	m_istr >> m_numb_of_DTMF_code;

	if (m_numb_of_DTMF_code >= MAX_DTMF_CODE_NUM)
		return;

	for (int i = 0; i < m_numb_of_DTMF_code; i++)
	{
		if (NULL == m_DTMF_code[i])
			m_DTMF_code[i] = new CDTMFCode;

		m_DTMF_code[i]->DeSerialize(format, m_istr);
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CDTMFCodeList::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pDtmfNode = NULL;
	pDtmfNode = pFatherNode->AddChildNode("DTMF_CODES_LIST");

	for (int i = 0; i < m_numb_of_DTMF_code; i++)
	{
		if (m_DTMF_code[i] != NULL)
			m_DTMF_code[i]->SerializeXml(pDtmfNode);
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CDTMFCodeList::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	// cleanup
	for (int i = 0; i < m_numb_of_DTMF_code; i++)
		POBJDELETE(m_DTMF_code[i]);

	m_numb_of_DTMF_code = 0;

	CXMLDOMElement* pCodeNode = NULL;
	GET_FIRST_CHILD_NODE(pActionNode, "DTMF_CODE", pCodeNode);

	while (pCodeNode && m_numb_of_DTMF_code < MAX_DTMF_CODE_NUM)
	{
		m_DTMF_code[m_numb_of_DTMF_code] = new CDTMFCode;
		nStatus = m_DTMF_code[m_numb_of_DTMF_code]->DeSerializeXml(pCodeNode, pszError);

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(m_DTMF_code[m_numb_of_DTMF_code]);
			return nStatus;
		}
		m_numb_of_DTMF_code++;

		GET_NEXT_CHILD_NODE(pActionNode, "DTMF_CODE", pCodeNode);
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CDTMFCodeList::IsLegalService(WORD chkLevel, string& err)
{
	// sets the code length for each DTMF in list
	SetDtmfCodesLen();

	// Check DTMF table legality (all DTMF codes are legal)
	int status = CheckLegalDtmfTbl();
	if (STATUS_OK != status)
		err = " Illegal DTMF Opcodes Table ";

	return status;
}

//--------------------------------------------------------------------------
WORD CDTMFCodeList::GetDTMFCodeNumber() const
{
	return m_numb_of_DTMF_code;
}

//--------------------------------------------------------------------------
int CDTMFCodeList::AddDTMFCode(CDTMFCode& other)
{
	int         status    = STATUS_OK;
	const char* eventName = NULL;

	if (m_numb_of_DTMF_code >= MAX_DTMF_CODE_NUM)
		return STATUS_MAX_DTMF_CODE_NUM_EXCEEDED;

	WORD DtmfOpcode = other.GetDTMFOpcode();
	CStringsMaps::GetDescription(DTMF_OPCODE_ENUM, DtmfOpcode, &eventName);

	if (DtmfOpcode != DTMF_MOVE_AND_REDIAL && DtmfOpcode != DTMF_DISCONNECT_AND_REDIAL &&
	    DtmfOpcode != DTMF_MOVE_AND_END && DtmfOpcode != DTMF_DELETE_AND_END &&
	    DtmfOpcode != DTMF_NOISY_LINE_UNMUTE && DtmfOpcode != DTMF_NOISY_LINE_ADJUST &&
	    DtmfOpcode != DTMF_NOISY_LINE_DISABLE && DtmfOpcode != DTMF_NOISY_LINE_MUTE &&
	    DtmfOpcode != DTMF_NOISY_LINE_HELP_MENU)
	{
		if (FindDTMFCode(other) != NOT_FIND)
		{
			if (DtmfOpcode == DTMF_REQUEST_TO_SPEAK)
			{
				TRACEINTO << "CDTMFCodeList::AddDTMFCode - Failed to add DTMF, DtmfOpcode:DTMF_REQUEST_TO_SPEAK";
			}
			return STATUS_DTMF_CODE_EXISTS;
		}
	}

	PDELETE(m_DTMF_code[m_numb_of_DTMF_code]);
	m_DTMF_code[m_numb_of_DTMF_code] = new CDTMFCode(other);
	if (DtmfOpcode == DTMF_REQUEST_TO_SPEAK)
	{
		TRACEINTO << "CDTMFCodeList::AddDTMFCode - Dtmf was added successfully, DtmfOpcode:DTMF_REQUEST_TO_SPEAK";
	}
	m_numb_of_DTMF_code++;

	return status;
}

//--------------------------------------------------------------------------
int CDTMFCodeList::UpdateDTMFCode(const CDTMFCode& other)
{
	int status = STATUS_OK;

	int ind = FindDTMFCode(other);
	if (ind == NOT_FIND)
		return STATUS_DTMF_CODE_NOT_EXISTS;

	POBJDELETE(m_DTMF_code[ind])
	m_DTMF_code[ind] = new CDTMFCode(other);

	return status;
}

//--------------------------------------------------------------------------
int CDTMFCodeList::CancelDTMFCode(const char* DTMF_str)
{
	int status = STATUS_OK;

	int ind = FindDTMFCode(DTMF_str);
	if (ind == NOT_FIND)
		return STATUS_THIS_LANGUAGE_DOES_NOT_EXIST_IN_THE_IVR_SERVICE;

	PDELETE(m_DTMF_code[ind]);

	for (int j = ind; j < (int)m_numb_of_DTMF_code-1; j++)
	{
		m_DTMF_code[j] = m_DTMF_code[j+1];
	}

	m_DTMF_code[m_numb_of_DTMF_code-1] = NULL;
	m_numb_of_DTMF_code--;

	return status;
}

//--------------------------------------------------------------------------
int CDTMFCodeList::FindDTMFCode(const CDTMFCode& other)
{
	if (strlen(other.GetDTMFStr()))
		for (int i = 0; i < (int)m_numb_of_DTMF_code; i++)
		{
			if (m_DTMF_code[i] != NULL)
			{
				if (!strncmp(m_DTMF_code[i]->GetDTMFStr(), other.GetDTMFStr(), DTMF_STRING_LEN))
					return i;
			}
		}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CDTMFCodeList::FindDTMFCode(const char* DTMF_str)
{
	if (strlen(DTMF_str))
		for (int i = 0; i < (int)m_numb_of_DTMF_code; i++)
		{
			if (m_DTMF_code[i] != NULL)
			{
				if (!strncmp(m_DTMF_code[i]->GetDTMFStr(), DTMF_str, DTMF_STRING_LEN))
					return i;
			}
		}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
CDTMFCode* CDTMFCodeList::GetCurrentDTMFCode(const char* DTMF_str)
{
	for (int i = 0; i < (int)m_numb_of_DTMF_code; i++)
	{
		if (m_DTMF_code[i] != NULL)
		{
			if (!strncmp(m_DTMF_code[i]->GetDTMFStr(), DTMF_str, DTMF_STRING_LEN))
				return m_DTMF_code[i];
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------
CDTMFCode* CDTMFCodeList::GetCurrentDTMFCodeByOpcode(WORD DTMF_opcode_str)
{
	for (int i = 0; i < (int)m_numb_of_DTMF_code; i++)
	{
		if (m_DTMF_code[i] != NULL)
		{
			if ((m_DTMF_code[i]->GetDTMFOpcode()) == DTMF_opcode_str)
				return m_DTMF_code[i];
		}
	}

	return NULL;
}
//--------------------------------------------------------------------------
CDTMFCode* CDTMFCodeList::GetFirstDTMFCode()
{
	m_ind_DTMF = 1;
	return m_DTMF_code[0];
}

//--------------------------------------------------------------------------
CDTMFCode* CDTMFCodeList::GetNextDTMFCode()
{
	if (m_ind_DTMF >= m_numb_of_DTMF_code) return NULL;

	return m_DTMF_code[m_ind_DTMF++];
}

//--------------------------------------------------------------------------
WORD CDTMFCodeList::GetNumberOfDtmfCodes() const
{
	return m_numb_of_DTMF_code;
}

//--------------------------------------------------------------------------
void CDTMFCodeList::DeleteList()
{
	for (int i = 0; i < (int)m_numb_of_DTMF_code; i++)
		if (m_DTMF_code[i])
			PDELETE(m_DTMF_code[i]);

	m_numb_of_DTMF_code = 0;
}

//--------------------------------------------------------------------------
WORD CDTMFCodeList::CheckLegalDtmfTbl()
{
	BYTE IsInviteCode_i = 0;
	BYTE IsInviteCode_j = 0;

	// loop on all opcodes
	for (int i = 0; i < m_numb_of_DTMF_code; i++)
	{
		CDTMFCode* i_Dtmfcode = m_DTMF_code[i];
		if (!i_Dtmfcode)
			continue;

		if (0 == i_Dtmfcode->m_DTMF_len)
			continue;

		// gets the DTMF opcode (i)
		WORD iDtmfOpcode = i_Dtmfcode->GetDTMFOpcode();

		// skipping Invite, not in GL1
		if (iDtmfOpcode == DTMF_MOVE_AND_REDIAL || iDtmfOpcode == DTMF_DISCONNECT_AND_REDIAL
		    || iDtmfOpcode == DTMF_MOVE_AND_END || iDtmfOpcode == DTMF_DELETE_AND_END)
			continue;

		// check for legal code length
		WORD codeLen = i_Dtmfcode->m_DTMF_len;
		if (codeLen > MAX_IVR_DTMF_CODE_LEN)
		{
			TRACEINTO << "CDTMFCodeList::CheckLegalDtmfTbl - Failed, Invalid DTMF code length, codeLen:" << codeLen;
			return STATUS_ILLEGAL_DTMF_CODE_LEN;
		}

		// check for legal characters
		const char* codeStr = i_Dtmfcode->m_DTMF_str;
		for (int code = 0; code < codeLen; code++)
			if ((codeStr[code] != '*') && (codeStr[code] != '#') && ((codeStr[code] < '0') || (codeStr[code] > '9')))
			{
				TRACEINTO << "CDTMFCodeList::CheckLegalDtmfTbl - Failed, Illegal character, AsciiCode:" << (int)codeStr[code];
				return STATUS_ILLEGAL_DTMF_CODE_CHARACTER;
			}

		// checks duplicate codes
		for (int j = i+1; j < m_numb_of_DTMF_code; j++)
		{
			CDTMFCode* j_Dtmfcode = m_DTMF_code[j];
			if (!j_Dtmfcode)
				continue;

			if (0 == j_Dtmfcode->m_DTMF_len)
				continue;

			WORD jDtmfOpcode = j_Dtmfcode->GetDTMFOpcode();

			// skipping invite, not in GL1
			if (jDtmfOpcode == DTMF_MOVE_AND_REDIAL || jDtmfOpcode == DTMF_DISCONNECT_AND_REDIAL
			    || jDtmfOpcode == DTMF_MOVE_AND_END || jDtmfOpcode == DTMF_DELETE_AND_END)
				continue;

			// minimum string length
			WORD lenMin = i_Dtmfcode->m_DTMF_len;
			if (i_Dtmfcode->m_DTMF_len > j_Dtmfcode->m_DTMF_len)
				lenMin = j_Dtmfcode->m_DTMF_len;

			const char* i_str = i_Dtmfcode->m_DTMF_str;
			const char* j_str = j_Dtmfcode->m_DTMF_str;

			// just a small optimization...
			if (lenMin > 1)             // probably most of the cases...
			{
				if (i_str[1] != j_str[1]) // probably #xx, #yy
					continue;

				if (i_str[0] != j_str[0]) // probably #xx, *yy
					continue;
			}

			// compare the 2 codes (also if one of them is substring of the another)
			if (0 == strncmp(i_str, j_str, lenMin))
			{
				// Checking Dtmf toggles
				if (iDtmfOpcode == DTMF_SELF_MUTE && jDtmfOpcode == DTMF_SELF_UNMUTE)
				{
					continue;
				}
				else if (iDtmfOpcode == DTMF_MUTE_ALL_BUT_X && jDtmfOpcode == DTMF_UNMUTE_ALL_BUT_X)
				{
					continue;
				}
				else if (iDtmfOpcode == DTMF_SECURE_CONF && jDtmfOpcode == DTMF_UNSECURE_CONF)
				{
					continue;
				}
				else
				{
					PTRACE(eLevelError, "CDTMFCodeList::CheckLegalDtmfTbl - 2 DTMF codes are similar ");
					return STATUS_IVR_ILLEAGAL_DTMF_TBL;  // Not toggle - Illegal dtmf table
				}
			}
		}
	}

	return 0; // Legal dtmf table
}

//--------------------------------------------------------------------------
void CDTMFCodeList::SetDtmfCodesLen()
{
	for (int i = 0; i < m_numb_of_DTMF_code; i++)
	{
		if (!m_DTMF_code[i])
			continue;

		m_DTMF_code[i]->SetDTMFStrLen();
	}
}


//--------------------------------------------------------------------------
BOOL CDTMFCodeList::IsIdenticalToCurrent(CDTMFCodeList* other)
{
	if (m_numb_of_DTMF_code != other->m_numb_of_DTMF_code)
		return FALSE;

	for (int i = 0; i < m_numb_of_DTMF_code; i++)
	{
		if (FALSE == m_DTMF_code[i]->IsIdenticalToCurrent(other->m_DTMF_code[i]))
			return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVR_BillingCodeFeature
////////////////////////////////////////////////////////////////////////////
CIVR_BillingCodeFeature::CIVR_BillingCodeFeature()
{
	m_feature_opcode = IVR_FEATURE_BILLING_CODE;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_BILLING_NUM);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_CONFERENCE_PASSWORD_RETRY);
	AddEvent(ivrEvent);
}

//--------------------------------------------------------------------------
CIVR_BillingCodeFeature::CIVR_BillingCodeFeature(const CIVR_BillingCodeFeature& other)
                        :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVR_BillingCodeFeature& CIVR_BillingCodeFeature::operator=(const CIVR_BillingCodeFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);

	return *this;
}

//--------------------------------------------------------------------------
CIVR_BillingCodeFeature::~CIVR_BillingCodeFeature()
{
}

//--------------------------------------------------------------------------
void CIVR_BillingCodeFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	CIVRFeature::Serialize(format, m_ostr);
}

//--------------------------------------------------------------------------
void CIVR_BillingCodeFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	// assuming format = OPERATOR_MCMS
	CIVRFeature::DeSerialize(format, m_istr);
}

//--------------------------------------------------------------------------
int CIVR_BillingCodeFeature::IsLegalService(WORD chkLevel, std::string& err, const char* language, int indBillingInGeneral)
{
	int status = CheckLeagalAcaFile(language, IVR_FEATURE_GENERAL, indBillingInGeneral, chkLevel);
	if (STATUS_OK != status)
	{
		err    = " Illegal BillingCode ACA file ";
		status = STATUS_IVR_BILLING_CODE_MSG_ERROR;
	}

	return status;
}

//--------------------------------------------------------------------------
BOOL CIVR_BillingCodeFeature::IsIdenticalToCurrent(CIVR_BillingCodeFeature* other)
{
	return CIVRFeature::IsIdenticalToCurrent(other);
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRRollCallFeature
////////////////////////////////////////////////////////////////////////////
CIVRRollCallFeature::CIVRRollCallFeature()
{
	m_feature_opcode = IVR_FEATURE_ROLL_CALL;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_ROLLCALL_REC);                   // Record a message
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_ROLLCALL_VERIFY_REC);            // Verify the recorded message
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_ROLLCALL_CONFIRM_REC);           // Confirm record message
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_ROLLCALL_ENTER);                 // A party enters to the conference
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_ROLLCALL_EXIT);                  // A party leaves the conference
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_ROLLCALL_BEGIN_OF_NAME_REVIEW);  // Before reviewing names
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_ROLLCALL_END_OF_NAME_REVIEW);    // After reviewing names
	AddEvent(ivrEvent);

	m_use_tones = FALSE;
}

//--------------------------------------------------------------------------
CIVRRollCallFeature::CIVRRollCallFeature(const CIVRRollCallFeature& other)
                    :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRRollCallFeature& CIVRRollCallFeature::operator=(const CIVRRollCallFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);
	m_use_tones = other.m_use_tones;
	return *this;
}

//--------------------------------------------------------------------------
CIVRRollCallFeature::~CIVRRollCallFeature()
{
}

//--------------------------------------------------------------------------
void CIVRRollCallFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	if (apiNum >= API_NUM_ROLL_CALL_VER1 || format != OPERATOR_MCMS)
	{
		CIVRFeature::Serialize(format, m_ostr);
		m_ostr << (WORD)m_use_tones << "\n";
	}
}

//--------------------------------------------------------------------------
void CIVRRollCallFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	WORD temp = 0;
	if (apiNum >= API_NUM_ROLL_CALL_VER1 || format != OPERATOR_MCMS)
	{
		CIVRFeature::DeSerialize(format, m_istr);
		m_istr >> temp;
		m_use_tones = (BYTE)temp;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRRollCallFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pRollNode = NULL;
	pRollNode = pFatherNode->AddChildNode("ROLL_CALL_PARAMS");

	CIVRFeature::SerializeXml(pRollNode);
	pRollNode->AddChildNode("USE_TONES_ONLY", m_use_tones, _BOOL);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRRollCallFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode, "USE_TONES_ONLY", &m_use_tones, _BOOL);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRRollCallFeature::Dump() const
{
	std::ostringstream msg;
	msg << "CIVRRollCallFeature::Dump - "
	    << "\n  m_enable_disable    :" << (WORD)m_enable_disable
	    << "\n  m_use_tones         :" << (WORD)m_use_tones
	    << "\n  m_numb_of_IVR_Event :" << (WORD)m_numb_of_IVR_Event;

	for (WORD eventIndex = 0; eventIndex < m_numb_of_IVR_Event; eventIndex++)
	{
		if (m_pIVREvent[eventIndex])
		{
			msg << "\n  event:";
			const char* eventName = NULL;
			CStringsMaps::GetDescription(IVR_EVENT_TYPE_ENUM, (DWORD)(m_pIVREvent[eventIndex]->m_event_opcode), &eventName);
			if (eventName != NULL)
				msg << eventName;
			else
				msg << (DWORD)(m_pIVREvent[eventIndex]->m_event_opcode);

			msg << ", m_numb_of_messages:" <<  m_pIVREvent[eventIndex]->m_numb_of_messages;
			for (WORD messageIndex = 0; messageIndex < m_pIVREvent[eventIndex]->m_numb_of_messages; messageIndex++)
			{
				if (NULL != m_pIVREvent[eventIndex]->m_pIVRMessage)
					msg << ", " << (char*)(m_pIVREvent[eventIndex]->m_pIVRMessage[messageIndex]->m_msg_file_name);
			}
		}
	}
	TRACEINTO << msg.str().c_str();
}

//--------------------------------------------------------------------------
// this function set all events to be valid
// when using tones only and then down-grade version - it should prevent not valid IVR service
BOOL CIVRRollCallFeature::SetMessagesFromTonesOnlyToRollCall()
{
	BOOL isChanged = NO;

	// find enter/exit event with message
	WORD toneEventIndex = (WORD)(-1);
	for (WORD eventIndex = 0; eventIndex < m_numb_of_IVR_Event; eventIndex++)
	{
		if (m_pIVREvent[eventIndex])
		{
			if (IVR_EVENT_ROLLCALL_ENTER == m_pIVREvent[eventIndex]->m_event_opcode && m_pIVREvent[eventIndex]->m_numb_of_messages > 0)
			{
				toneEventIndex = eventIndex;
			}
			else if (IVR_EVENT_ROLLCALL_EXIT == m_pIVREvent[eventIndex]->m_event_opcode && m_pIVREvent[eventIndex]->m_numb_of_messages > 0)
			{
				toneEventIndex = eventIndex;
			}
		}
	}

	WORD need_fixed = 0;
	// adding message to other events with no messages from enter/exit event
	if (toneEventIndex != (WORD)(-1))
	{
		for (WORD eventIndex = 0; eventIndex < m_numb_of_IVR_Event; eventIndex++)
		{
			if (m_pIVREvent[eventIndex])
			{
				if (IVR_EVENT_ROLLCALL_ENTER == m_pIVREvent[eventIndex]->m_event_opcode || IVR_EVENT_ROLLCALL_EXIT == m_pIVREvent[eventIndex]->m_event_opcode)
				{
					continue;
				}

				// non empty file path
				if (m_pIVREvent[eventIndex]->m_numb_of_messages > 0 && NULL != (m_pIVREvent[eventIndex]->m_pIVRMessage[0]) && strlen((char*)(m_pIVREvent[eventIndex]->m_pIVRMessage[0]->m_msg_file_name)) > 0)
				{
					continue;
				}

				// print only once
				if (need_fixed == 0)
				{
					TRACEINTO << "CIVRRollCallFeature::SetMessagesFromTonesOnlyToRollCall - Dump roll call before fix";
					Dump();
					need_fixed = 1;
				}

				// fixing
				if (m_pIVREvent[toneEventIndex]->m_pIVRMessage[0])
				{
					// remove empty file message
					if (m_pIVREvent[eventIndex]->m_numb_of_messages > 0 && NULL != (m_pIVREvent[eventIndex]->m_pIVRMessage[0]))
					{
						m_pIVREvent[eventIndex]->CancelMessage(0);
					}

					m_pIVREvent[eventIndex]->AddMessage(*(m_pIVREvent[toneEventIndex]->m_pIVRMessage[0]));
					isChanged = YES;
					const char* eventName = NULL;
					CStringsMaps::GetDescription(IVR_EVENT_TYPE_ENUM, m_pIVREvent[eventIndex]->m_event_opcode, &eventName);
					if (eventName != NULL)
					{
						TRACEINTO << "CIVRRollCallFeature::SetMessagesFromTonesOnlyToRollCall - Adding message to event: " << eventName;
					}
					else
					{
					  TRACEINTO << "CIVRRollCallFeature::SetMessagesFromTonesOnlyToRollCall - Adding message to event: " << m_pIVREvent[eventIndex]->m_event_opcode;
					}
				}
			}
		}

		// print only once
		if (need_fixed == 1)
		{
			TRACEINTO << "CIVRRollCallFeature::SetMessagesFromTonesOnlyToRollCall - Dump roll call after fix";
			Dump();
		}
	}
	else
	{
		TRACEINTO << "CIVRRollCallFeature::SetMessagesFromTonesOnlyToRollCall - Entry/Exit event message not configured";
	}

	return isChanged;
}

//--------------------------------------------------------------------------
int CIVRRollCallFeature::IsLegalService(WORD chkLevel, std::string& err, const char* language)
{
	SetMessagesFromTonesOnlyToRollCall();

	int status = STATUS_OK;

	if (!GetEnableDisable())
		return STATUS_OK;

	if (status == STATUS_OK)
	{
		// checking 4 events (RC-record, RC-exit, RC-enter, RC-review)
		WORD rcEvent;
		for (rcEvent = 0; rcEvent < 4; rcEvent++)
		{
			status = CheckLeagalAcaFile(language, IVR_FEATURE_ROLL_CALL, rcEvent, chkLevel);
			if (STATUS_OK != status)
			{
				err = " Illegal Roll Call ACA file ";
				break;
			}
		}
	}

	if (STATUS_OK != status)
	{
		Dump();
	}

	return status;
}

//--------------------------------------------------------------------------
BOOL CIVRRollCallFeature::IsIdenticalToCurrent(const CIVRRollCallFeature* other) const
{
	return CIVRFeature::IsIdenticalToCurrent(other);
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRVideoFeature
////////////////////////////////////////////////////////////////////////////
CIVRVideoFeature::CIVRVideoFeature()
{
	m_feature_opcode = IVR_FEATURE_VIDEO;
	m_enable_VC      = NO;
}

//--------------------------------------------------------------------------
CIVRVideoFeature::CIVRVideoFeature(const CIVRVideoFeature& other)
                 :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRVideoFeature& CIVRVideoFeature::operator=(const CIVRVideoFeature& other)
{
	m_enable_VC = other.m_enable_VC;
	CIVRFeature::operator =((CIVRFeature&)other);

	return *this;
}

//--------------------------------------------------------------------------
CIVRVideoFeature::~CIVRVideoFeature()
{
}

//--------------------------------------------------------------------------
int CIVRVideoFeature::IsLegalService(WORD chkLevel, string& err, const char* language, const char* videoName)
{
	if ((NULL != videoName) && (0 != strcmp(videoName, "")))
	{
		STATUS status = CheckLegalIvrFileName(videoName);
		if (status != STATUS_OK)
		{
			err = " Illegal Video file name ";
			return STATUS_IVR_MSG_ILLEGAL_NAME;
		}
		else
		{
			CIVRSlidesList* pSlidesList = ::GetpSlidesList();
			if (pSlidesList)
			{
				status = pSlidesList->CheckSlideFilesExistance(videoName); // the slide may already exists
				if (status != STATUS_OK)
				{
					err = " Illegal Video file name ";
					return STATUS_IVR_VIDEO_FILE_DOES_NOT_EXIST;
				}
			}
		}
	}
	else // slide is NULL
	{
		err = " Missing Video file name ";
		return STATUS_IVR_MSG_NAME_MISSING;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRVideoFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	if (apiNum >= API_NUM_VIDEO_VER1 || format != OPERATOR_MCMS)
	{
		CIVRFeature::Serialize(format, m_ostr);
		m_ostr << (WORD)m_enable_VC << "\n";
	}
}

//--------------------------------------------------------------------------
void CIVRVideoFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	WORD temp;

	if (apiNum >= API_NUM_VIDEO_VER1 || format != OPERATOR_MCMS)
	{
		CIVRFeature::DeSerialize(format, m_istr);
		m_istr >> temp;
		m_enable_VC = (BYTE)temp;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRVideoFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pVideoNode = pFatherNode->AddChildNode("VIDEO_SERVICE");

	CIVRFeature::SerializeXml(pVideoNode);
	pVideoNode->AddChildNode("ENABLE_CLICK_AND_VIEW", m_enable_VC, _BOOL);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRVideoFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode, "ENABLE_CLICK_AND_VIEW", &m_enable_VC, _BOOL);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
BYTE CIVRVideoFeature::GetEnableVC() const
{
	return m_enable_VC;
}

//--------------------------------------------------------------------------
BOOL CIVRVideoFeature::IsIdenticalToCurrent(CIVRVideoFeature* other)
{
	if (FALSE == CIVRFeature::IsIdenticalToCurrent(other))
		return FALSE;

	if (m_enable_VC != other->m_enable_VC)
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRNumericConferenceIdFeature
////////////////////////////////////////////////////////////////////////////
CIVRNumericConferenceIdFeature::CIVRNumericConferenceIdFeature()
{
	m_feature_opcode = IVR_FEATURE_NUMERIC_CONFERENCE_ID;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_GET_NUMERIC_ID);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_NUMERIC_ID_RETRY);    // Verify the recorded message
	AddEvent(ivrEvent);
}

//--------------------------------------------------------------------------
CIVRNumericConferenceIdFeature::CIVRNumericConferenceIdFeature(const CIVRNumericConferenceIdFeature& other)
                               :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRNumericConferenceIdFeature& CIVRNumericConferenceIdFeature::operator=(const CIVRNumericConferenceIdFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);

	return *this;
}

//--------------------------------------------------------------------------
CIVRNumericConferenceIdFeature::~CIVRNumericConferenceIdFeature()
{
}

//--------------------------------------------------------------------------
void CIVRNumericConferenceIdFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	if (apiNum >= API_NUMBER_CONFERENCE_ID || format != OPERATOR_MCMS)
	{
		CIVRFeature::Serialize(format, m_ostr);
	}
}

//--------------------------------------------------------------------------
void CIVRNumericConferenceIdFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	if (apiNum >= API_NUMBER_CONFERENCE_ID || format != OPERATOR_MCMS)
	{
		CIVRFeature::DeSerialize(format, m_istr);
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRNumericConferenceIdFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pNumerNode = pFatherNode->AddChildNode("NUMERIC_ID_PARAMS");

	CIVRFeature::SerializeXml(pNumerNode);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRNumericConferenceIdFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CIVRNumericConferenceIdFeature::IsLegalService(WORD chkLevel, std::string& err, const char* language)
{
	int status = STATUS_OK;

	if (!GetEnableDisable())
	{
		err    = " NID Feature is disabled ";
		status = STATUS_IVR_NUMERIC_ID_DISABLED;
	}

	if (status == STATUS_OK)
	{
		// checking 2 events (get NID, NID-Retry)
		WORD NidEvent;
		for (NidEvent = 0; NidEvent < 2; NidEvent++)
		{
			status = CheckLeagalAcaFile(language, IVR_FEATURE_NUMERIC_CONFERENCE_ID, NidEvent, chkLevel);
			if (STATUS_OK != status)
			{
				err = " Illegal NID ACA file ";
				break;
			}
		}
	}

	return status;
}

//--------------------------------------------------------------------------
BOOL CIVRNumericConferenceIdFeature::IsIdenticalToCurrent(CIVRNumericConferenceIdFeature* other)
{
	return CIVRFeature::IsIdenticalToCurrent(other);
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRMuteNoisyLineFeature
////////////////////////////////////////////////////////////////////////////
CIVRMuteNoisyLineFeature::CIVRMuteNoisyLineFeature()
{
	m_feature_opcode = IVR_FEATURE_MUTE_NOISY_LINE;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_NOISY_LINE_HELP_MENU);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_NOISY_LINE_MUTE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_NOISY_LINE_UNMUTE_MESSAGE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_NOISY_LINE_UNMUTE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_NOISY_LINE_ADJUST);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_NOISY_LINE_DISABLE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_NOISY_LINE_MUTE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_NOISY_LINE_UNMUTE_MESSAGE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_PLAY_NOISY_LINE_MESSAGE);
	AddEvent(ivrEvent);


	m_return_and_unmute            = NO;
	m_return_and_mute              = NO;
	m_adjust_noise_detection       = NO;
	m_disable_noise_detection      = NO;
	m_play_noisy_detection_message = NO;
	m_play_noisy_line_menu         = NO;
}

//--------------------------------------------------------------------------
CIVRMuteNoisyLineFeature::CIVRMuteNoisyLineFeature(const CIVRMuteNoisyLineFeature& other)
                         :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRMuteNoisyLineFeature& CIVRMuteNoisyLineFeature:: operator=(const CIVRMuteNoisyLineFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);
	m_return_and_unmute            = other.m_return_and_unmute;
	m_return_and_mute              = other.m_return_and_mute;
	m_adjust_noise_detection       = other.m_adjust_noise_detection;
	m_disable_noise_detection      = other.m_disable_noise_detection;
	m_play_noisy_detection_message = other.m_play_noisy_detection_message;
	m_play_noisy_line_menu         = other.m_play_noisy_line_menu;

	return *this;
}

//--------------------------------------------------------------------------
CIVRMuteNoisyLineFeature:: ~CIVRMuteNoisyLineFeature()
{
}

//--------------------------------------------------------------------------
void CIVRMuteNoisyLineFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	if (apiNum >= API_NUM_MUTE_NOISY_LINE || format != OPERATOR_MCMS)
	{
		CIVRFeature::Serialize(format, m_ostr);
		m_ostr <<  (WORD)m_return_and_unmute  << "\n";
		m_ostr <<  (WORD)m_return_and_mute   << "\n";
		m_ostr <<  (WORD)m_adjust_noise_detection   << "\n";
		m_ostr <<  (WORD)m_disable_noise_detection  << "\n";
		m_ostr <<  (WORD)m_play_noisy_detection_message   << "\n";
		m_ostr <<  (WORD)m_play_noisy_line_menu   << "\n";
	}
}

//--------------------------------------------------------------------------
void CIVRMuteNoisyLineFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	WORD tmp = 0;

	// assuming format = OPERATOR_MCMS
	if (apiNum >= API_NUM_MUTE_NOISY_LINE || format != OPERATOR_MCMS)
	{
		CIVRFeature::DeSerialize(format, m_istr);
		m_istr >> tmp;
		m_return_and_unmute = (BYTE)tmp;

		m_istr >> tmp;
		m_return_and_mute = (BYTE)tmp;

		m_istr >> tmp;
		m_adjust_noise_detection = (BYTE)tmp;

		m_istr >> tmp;
		m_disable_noise_detection = (BYTE)tmp;

		m_istr >> tmp;
		m_play_noisy_detection_message = (BYTE)tmp;

		m_istr >> tmp;
		m_play_noisy_line_menu = (BYTE)tmp;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRMuteNoisyLineFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pNoisyNode = pFatherNode->AddChildNode("NOISY_LINE");

	CIVRFeature::SerializeXml(pNoisyNode);
	pNoisyNode->AddChildNode("RETURN_AND_UNMUTE", m_return_and_unmute, _BOOL);
	pNoisyNode->AddChildNode("RETURN_AND_MUTE", m_return_and_mute, _BOOL);
	pNoisyNode->AddChildNode("ADJUST_NOISE_DETECTION", m_adjust_noise_detection, _BOOL);
	pNoisyNode->AddChildNode("DISABLE_NOISE_DETECTION", m_disable_noise_detection, _BOOL);
	pNoisyNode->AddChildNode("PLAY_NOISY_LINE_MENU", m_play_noisy_line_menu, _BOOL);
	pNoisyNode->AddChildNode("PLAY_NOISY_LINE_DETECTION_MESSAGE", m_play_noisy_detection_message, _BOOL);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRMuteNoisyLineFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pActionNode, "RETURN_AND_UNMUTE", &m_return_and_unmute, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "RETURN_AND_MUTE", &m_return_and_mute, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "ADJUST_NOISE_DETECTION", &m_adjust_noise_detection, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "DISABLE_NOISE_DETECTION", &m_disable_noise_detection, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "PLAY_NOISY_LINE_MENU", &m_play_noisy_line_menu, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "PLAY_NOISY_LINE_DETECTION_MESSAGE", &m_play_noisy_detection_message, _BOOL);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRMuteNoisyLineFeature::SetReturnAndUnmute(const BYTE IsReturnAndUnmute)
{
	m_return_and_unmute = IsReturnAndUnmute;
}

//--------------------------------------------------------------------------
BYTE CIVRMuteNoisyLineFeature::GetReturnAndUnmute() const
{
	return m_return_and_unmute;
}

//--------------------------------------------------------------------------
void CIVRMuteNoisyLineFeature::SetReturnAndMute(const BYTE IsReturnAndMute)
{
	m_return_and_mute = IsReturnAndMute;
}

//--------------------------------------------------------------------------
BYTE CIVRMuteNoisyLineFeature::GetReturnAndMute() const
{
	return m_return_and_mute;
}

//--------------------------------------------------------------------------
void CIVRMuteNoisyLineFeature::SetAdjustNoiseDetection(const BYTE IsAdjustNoiseDetection)
{
	m_adjust_noise_detection = IsAdjustNoiseDetection;
}

//--------------------------------------------------------------------------
BYTE CIVRMuteNoisyLineFeature::GetAdjustNoiseDetection() const
{
	return m_adjust_noise_detection;
}

//--------------------------------------------------------------------------
void CIVRMuteNoisyLineFeature::SetDisableNoiseDetection(const BYTE IsDisableNoiseDetection)
{
	m_disable_noise_detection = IsDisableNoiseDetection;
}

//--------------------------------------------------------------------------
BYTE CIVRMuteNoisyLineFeature::GetDisableNoiseDetection() const
{
	return m_disable_noise_detection;
}

//--------------------------------------------------------------------------
void CIVRMuteNoisyLineFeature::SetPlayNoisyDetectionMessage(const BYTE isPlayNoisyDetectionMessage)
{
	m_play_noisy_detection_message = isPlayNoisyDetectionMessage;
}

//--------------------------------------------------------------------------
BYTE CIVRMuteNoisyLineFeature::GetPlayNoisyDetectionMessage() const
{
	return m_play_noisy_detection_message;
}

//--------------------------------------------------------------------------
void CIVRMuteNoisyLineFeature::SetMuteNoisyLineMenu(const BYTE isMuteNoisyLineMenu)
{
	m_play_noisy_line_menu = isMuteNoisyLineMenu;
}

//--------------------------------------------------------------------------
BYTE CIVRMuteNoisyLineFeature::GetMuteNoisyLineMenu() const
{
	return m_play_noisy_line_menu;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRRecordingFeature
////////////////////////////////////////////////////////////////////////////
CIVRRecordingFeature::CIVRRecordingFeature()
{
	m_feature_opcode = IVR_FEATURE_RECORDING;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_REC_STARTED);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_REC_STOPPED);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_REC_PAUSED);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_REC_RESUMED);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_REC_ILLEGAL_ACCOUNT);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_REC_UNAUTHORIZE);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_REC_GENERAL);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_REC_USER_OVERFLOW);
	AddEvent(ivrEvent);

	m_Save_numb_of_IVR_Event = GetIVREventNumber();
}

//--------------------------------------------------------------------------
CIVRRecordingFeature::CIVRRecordingFeature(const CIVRRecordingFeature& other)
                     :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRRecordingFeature& CIVRRecordingFeature::operator=(const CIVRRecordingFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);

	return *this;
}

//--------------------------------------------------------------------------
CIVRRecordingFeature::~CIVRRecordingFeature()
{
}

//--------------------------------------------------------------------------
void CIVRRecordingFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	if (apiNum >= API_NUM_REC_PLYBCK_TYPE || format != OPERATOR_MCMS)
	{
		CIVRFeature::Serialize(format, m_ostr);
	}
}

//--------------------------------------------------------------------------
void CIVRRecordingFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	if (apiNum >= API_NUM_REC_PLYBCK_TYPE || format != OPERATOR_MCMS)
	{
		CIVRFeature::DeSerialize(format, m_istr);

		m_numb_of_IVR_Event = m_Save_numb_of_IVR_Event;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRRecordingFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pRecordingNode = pFatherNode->AddChildNode("RECORDING_PARAMS");

	CIVRFeature::SerializeXml(pRecordingNode);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRRecordingFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// here we miss some parameters...

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CIVRPlaybackFeature
////////////////////////////////////////////////////////////////////////////
CIVRPlaybackFeature::CIVRPlaybackFeature()
{
	m_feature_opcode = IVR_FEATURE_PLAYBACK;

	CIVREvent ivrEvent;
	ivrEvent.SetEventOpcode(IVR_EVENT_PLCK_SESSIONID);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_PLCK_SESSIONID_ERROR);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_PLCK_SESSION_END);
	AddEvent(ivrEvent);
	ivrEvent.SetEventOpcode(IVR_EVENT_PLCK_MENU);
	AddEvent(ivrEvent);

	m_Save_numb_of_IVR_Event = GetIVREventNumber();
}

//--------------------------------------------------------------------------
CIVRPlaybackFeature::CIVRPlaybackFeature(const CIVRPlaybackFeature& other)
                    :CIVRFeature(other)
{
	*this = other;
}

//--------------------------------------------------------------------------
CIVRPlaybackFeature& CIVRPlaybackFeature::operator=(const CIVRPlaybackFeature& other)
{
	CIVRFeature::operator =((CIVRFeature&)other);

	return *this;
}

//--------------------------------------------------------------------------
CIVRPlaybackFeature::~CIVRPlaybackFeature()
{
}

//--------------------------------------------------------------------------
void CIVRPlaybackFeature::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
	if (apiNum >= API_NUM_REC_PLYBCK_TYPE || format != OPERATOR_MCMS)
	{
		CIVRFeature::Serialize(format, m_ostr);
	}
}

//--------------------------------------------------------------------------
void CIVRPlaybackFeature::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
	if (apiNum >= API_NUM_REC_PLYBCK_TYPE || format != OPERATOR_MCMS)
	{
		CIVRFeature::DeSerialize(format, m_istr);

		m_numb_of_IVR_Event = m_Save_numb_of_IVR_Event;
	}
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
void CIVRPlaybackFeature::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pPlaybackNode = pFatherNode->AddChildNode("PLAYBACK_PARAMS");

	CIVRFeature::SerializeXml(pPlaybackNode);
}

//--------------------------------------------------------------------------
// schema file name:  obj_av_msg_service.xsd
int CIVRPlaybackFeature::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pBaseNode = NULL;
	GET_CHILD_NODE(pActionNode, "IVR_FEATURE_DETAILS", pBaseNode);
	if (pBaseNode)
	{
		nStatus = CIVRFeature::DeSerializeXml(pBaseNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}

	// here we miss some parameters...

	return STATUS_OK;
}
