#include "IVRFeatures.h"
#include "DataTypes.h"
#include "PartyApi.h"
#include "IVRDtmfColl.h"
#include "IVRService.h"
#include "ConfApi.h"
#include "CommConf.h"
#include "CommConfDB.h"
#include "Party.h"
#include "ConfPartyGlobals.h"
#include "IVRServiceList.h"
#include "psosxml.h"
#include "ProcessBase.h"
#include "ConfPartyManagerLocalApi.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "SysConfigKeys.h"
#include "StatusesGeneral.h"
#include "VideoDefines.h"
#include "ConfAppMngr.h"
#include "SystemFunctions.h"
#include "CommResDB.h"
#include "CommResDBAction.h"
#include "SysConfigKeys.h"
#include "CommResShort.h"
#include "OpcodesMcmsCommon.h"
#include "ConfPartyStatuses.h"
#include "IpServiceListManager.h"
#include "IvrApiStructures.h"
#include "IVRCntlExternal.h"

#include "IVRCntlLocal.h"
extern CCommResDB*            GetpMeetingRoomDB();
extern CCommResDB*            GetpProfilesDB();
extern "C" long               GetMCUIP();
extern CIpServiceListManager* GetIpServiceListMngr();

// #define MAX_MESSAGE_LENGTH_IN_SECONDS		32

#define TIMER_WAIT_FOR_DTMF   1
#define TIMER_WAIT_PHONE_NUM  2
#define DEF_SIZE_16           16

#define EXDB_TYPE_ConfPW      0
#define EXDB_TYPE_ChairPW     1
#define EXDB_TYPE_ConfNID     2
#define EXDB_TYPE_InviteParty 3

const WORD TIME_EXT_DB_TIMEOUT               = 20;
const WORD TIMER_RETRY_MESSAGE               = 30;
const WORD TIMER_WAIT_FOR_WELCOME            = 40;
const WORD TIMER_WAIT_FOR_NO_VIDEO_RESOURCES = 60;

//const WORD   NOTACTIVE		= 1;
//const WORD   ACTIVE			= 2;
//const WORD   WAIT_FOR_DB	= 3;

extern int         GetIvrUserParams(const char* pincode, char* name, int* leader);
extern const char* GetStatusAsString(const int status);

#define MAX_SUB_MESSAGE_NUMBER   10
#define ROLL_CALL_MESSAGE_NUMBER 0xAAAA

#define PROTOCOL_ISDN            1
#define PROTOCOL_PSTN            2
#define PROTOCOL_SIP             3
#define PROTOCOL_H323            4

typedef std::vector<CCommResShort*> ReservArray;

PBEGIN_MESSAGE_MAP(CIvrSubBaseSM)
	ONEVENT(DTMF_STRING_IDENT, NOTACTIVE, CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT, ACTIVE   , CIvrSubBaseSM::OnDtmfString)
PEND_MESSAGE_MAP(CIvrSubBaseSM, CStateMachine);


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubBaseSM
////////////////////////////////////////////////////////////////////////////
CIvrSubBaseSM::CIvrSubBaseSM()
{
	m_state                = NOTACTIVE;
	m_pDtmf                = 0;
	m_ivrCntl              = 0;
	m_pPartyApi            = 0;
	m_pParty               = 0;
	m_pConfApi             = 0;
	m_feature_opcode       = 0;
	m_event_opcode         = 0;
	m_language             = 0;
	m_errorCounter         = 0;
	m_timeoutCounter       = 0;
	m_maxRetryTimes        = 4;
	m_DTMF_digits          = 0;
	m_DTMF_timeout         = 0;
	m_msg_file_name[0]     = '\0';
	m_pIvrService          = NULL;
	m_retryMessageDuration = 0;
	m_pinCodeDelimiter     = ' ';
	m_messagesName         = NULL;
	m_msgDuration          = 0;
	m_bStopUponDTMF        = 0;
	m_timerRetryNum        = 0;
	m_initTimer            = 0;

	m_bEnableExternalDB    = 0;
	m_bAllowDtmfBargeIn    = FALSE;

	memset(m_dtmf, 0, sizeof(m_dtmf));
	memset(m_passwordForCascadeLink, 0, sizeof(m_passwordForCascadeLink));
	memset(m_numericIdForCascadeLink, 0, sizeof(m_numericIdForCascadeLink));

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubBaseSM::~CIvrSubBaseSM()
{
	if (m_pPartyApi)
	{
		m_pPartyApi->DestroyOnlyApi();
		POBJDELETE(m_pPartyApi);
	}

	if (m_pDtmf)
		m_pDtmf->EndFeature();

	if (m_initTimer)
	{
		if (m_timerRetryNum)
			DeleteTimer(TIMER_RETRY_MESSAGE);

		DeleteAllTimers();
	}
}

//--------------------------------------------------------------------------
void* CIvrSubBaseSM::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	if (opCode != TIMER)
			DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::SetIvrCntl(CIvrCntl* ivrCntl)
{
	m_ivrCntl = ivrCntl;
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::SetLanguage(WORD language)
{
	PASSERTSTREAM_AND_RETURN(language >= MAX_LANG_IN_IVR_SERVICE, "language" << language << " - Illegal language");

	m_language = language;
}

//--------------------------------------------------------------------------
int CIvrSubBaseSM::GetIVRMessageParameters(char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum)
{
	const char* ivrServiceName = m_pIvrService->GetName();
	int needToUpdate   = 0;

	int status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, m_feature_opcode, m_event_opcode, msgFullPath, msgDuration, msgCheckSum, &needToUpdate);
	if (STATUS_OK == status) // update the local IVRService object with new parameters
		m_pIvrService->UpdateIVRMsgParamsInService(m_feature_opcode, m_event_opcode, *msgDuration, *msgCheckSum, 0);

	return status;
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::PlayMessage(DWORD messageMode, WORD loop_delay)
{
	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);

	int  status      = GetIVRMessageParameters(msgFullPath, &msgDuration, &msgCheckSum);
	if (STATUS_OK != status)
	{
		msgFullPath[0] = '\0';
		msgDuration    = 0;
		msgCheckSum    = (WORD)(-1);
		TRACEINTO << "Failed to retrieve IVR message params";
	}

	TRACEINTO
			<< "eventOpcode:"     << m_event_opcode
			<< ", featureOpcode:" << m_feature_opcode
			<< ", messageMode:"   << messageMode
			<< ", loopDelay:"     << loop_delay
			<< ", messagePath:"   << msgFullPath;

	// gets IVR message priority
	WORD cachePriority = GetCachePriority(m_feature_opcode, m_event_opcode);

	// send start message
	TRACEINTO << "is tip call: " << m_pParty->GetIsTipCall() << " partyType: " << m_pParty->GetTipPartyType();
//	if( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
//	{
//		TRACEINTO << "forward message to AUX";
//		CSegment* pParam = new CSegment();
//		*pParam << msgFullPath << msgDuration << msgCheckSum << messageMode << cachePriority << loop_delay;
//		m_pParty->ForwardEventToTipSlaves(pParam, PLAY_MESSAGE_AUX);
//	}
//	else
//	{
//		if (m_pConfApi)
//			m_pConfApi->StartMessage(m_pParty->GetPartyRsrcID(), msgFullPath, msgDuration, msgCheckSum, (WORD)messageMode, cachePriority, loop_delay);
//	}
	if (m_pConfApi)
		m_pConfApi->StartMessage(m_pParty->GetPartyRsrcID(), msgFullPath, msgDuration, msgCheckSum, (WORD)messageMode, cachePriority, loop_delay);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::StopPlayMessage()
{
	TRACEINTO << "is tip call: " << m_pParty->GetIsTipCall() << " partyType: " << m_pParty->GetTipPartyType();
	if( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		TRACEINTO << "forward message to AUX";
		m_pParty->ForwardEventToTipSlaves(NULL, STOP_PLAY_MESSAGE_AUX);
	}
	else
	{
		if (m_pConfApi)
			m_pConfApi->StopMessage(m_pParty->GetPartyRsrcID());
	}
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::StartBase(WORD bRetry, DWORD messageMode, WORD loop_delay)
{
	PTRACE(eLevelInfoNormal, "CIvrSubBaseSM::StartBase");

	// number of retries
	m_state         = ACTIVE;
	m_maxRetryTimes = m_pIvrService->GetRetriesNum();
	if (m_maxRetryTimes > 5)
		m_maxRetryTimes = 5;

	m_pDtmf->ResetDtmfParams(); // reset DTMF parameters
	m_pDtmf->SetDtmfWaitForXDigitsNumber(m_DTMF_digits, m_DTMF_timeout, m_msgDuration);
	m_pDtmf->SetDtmfDelimiter(m_pinCodeDelimiter);
	m_pDtmf->StartFeature();

	if (0 == m_pDtmf->CheckDtmfForCurrentFeature()) // if 1 then the buffer is ready with DTMF so we don't need to play message
	{
		if (bRetry)
		{
			PlayRetryMessage(IVR_STATUS_PLAY_ONCE);     // retry message, play once
			if (m_retryMessageDuration < 3)             // to be on the safe side...
				m_retryMessageDuration = 3;

			if (m_retryMessageDuration > 30)            // to be on the safe side...
				m_retryMessageDuration = 30;

				char str[16];
				sprintf(str, "%i", (int)(m_retryMessageDuration+2));
				PTRACE2(eLevelInfoNormal, "CIvrSubBaseSM::StartBase - RetryDuration: ", str);

			// for password, leader and billing
			if (m_initTimer)
			{
				m_timerRetryNum = m_retryMessageDuration + 2;
				StartTimer(TIMER_RETRY_MESSAGE, 1*SECOND); // checks every 1 second
				return;
			}

			SystemSleep(100 * (m_retryMessageDuration + 2));

			// if the user starts inserting DTMF (at least one) then we do not play the message
			if (!m_pDtmf->IsDtmfInBuffer())
			{
				// start the timeout from beginning
				m_pDtmf->RestartTimeout(m_msgDuration);

				// play again the first feature message
				PlayMessage(messageMode);
			}
		}
		else // not a retry message
		{
			PlayMessage(messageMode, loop_delay);
		}
	}
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::OnTimeRetryMessage(CSegment* pParam)
{
	if (!m_pDtmf)
		return;

	int bIsDTMF = m_pDtmf->IsDtmfInBuffer();  // checking if there is a DTMF in the buffer
	m_timerRetryNum--;
	if (m_timerRetryNum > 0)                  // number of message duration in seconds
	{
		if (0 == bIsDTMF)                       // buffer is still empty, no DTMF was pressed
		{
			StartTimer(TIMER_RETRY_MESSAGE, 1*SECOND);
			return;                               // waiting for the next timer
		}
		else
			m_timerRetryNum = 0;                  // a DTMF was pressed

	}

	if (!bIsDTMF)                             // no DTMF pressed yet
	{
		PTRACE(eLevelInfoNormal, "CIvrSubBaseSM::OnTimeRetryMessage - End of timer: RestartTimeout ");
		m_pDtmf->RestartTimeout(m_msgDuration);
		PlayMessage(IVR_STATUS_PLAY_ONCE);      // ask for entering the suitable DTMF
	}
	else
		PTRACE(eLevelInfoNormal, "CIvrSubBaseSM::OnTimeRetryMessage - End of timer ");
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::OnDtmfString(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubBaseSM::OnDtmfString DTMF message ");
	DeleteTimer(TIMER_RETRY_MESSAGE);
	if (m_timerRetryNum > 0)    // reset retry timer if exists
	{
		m_timerRetryNum = 0;
		DeleteTimer(TIMER_RETRY_MESSAGE);
		PTRACE(eLevelInfoNormal, "CIvrSubBaseSM::OnDtmfString - delete retry timer ");
	}

	// init flags
	WORD status        = 0;                           // finish status: OK
	WORD bIsEndFeature = TRUE;                        // send end of feature

	// checks the string legality
	WORD result = IsLegalString(pParam);
	if (DTMF_OK != result)                            // error
	{
		switch (result)
		{
			case DTMF_IGNORE_ERROR:                       // do noting
			return;

			case DTMF_TIMEOUT_ERROR:                      // timeout error (no characters)
			{
				m_timeoutCounter++;                         // increases timeout counter
				if (m_maxRetryTimes > m_timeoutCounter)     // allowed to retry
				{
					StartBase(FALSE, IVR_STATUS_PLAY_ONCE);   // play message again
					bIsEndFeature = FALSE;                    // not end of the feature
				}
				else
				{
					if (getEventOpCode() != IVR_EVENT_INVITE_PARTY
					    && getEventOpCode() != IVR_EVENT_REINVITE_PARTY)
					{
						status = DTMF_TIMEOUT_ERROR;     // finish status: ERROR
					}
				}
				break;
			}

			case DTMF_STRING_ERROR:
			{
				PTRACE(eLevelInfoNormal, "CIvrSubBaseSM::OnDtmfString - DTMF error. ");
				m_errorCounter++;                             // increases error counter
				m_timeoutCounter = 0;                         // resets timeout counter
				if (m_maxRetryTimes > m_errorCounter)         // allowed to retry
				{
					m_pDtmf->ResetDtmfBuffer();                 // reset DTMF buffer
					StartBase(TRUE, IVR_STATUS_PLAY_ONCE);      // wrong UserId, retries to get UserID again
					bIsEndFeature = FALSE;                      // not end of the feature
				}
				else
				{
					status = DTMF_STRING_ERROR;                   // finish status: ERROR
				}
				break;
			}

			case TOO_MANY_ERRORS:
			{
				PTRACE(eLevelError, "CIvrSubBaseSM::OnDtmfString - max retries, too many timeout ");
				status = TOO_MANY_ERRORS;   // finish status: ERROR
				break;
			}
		} // switch
	}

	// sends "end feature" if needed (if case of 'OK' or 'error', but not in case of retry)
	if (bIsEndFeature)
		OnEndFeature(status);
}

//--------------------------------------------------------------------------
WORD CIvrSubBaseSM::DoSomething()
{
	return 0;
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::ParsePreDefinedIvrString(const char* szPreDefinedIvrString)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubBaseSM::ParsePreDefinedIvrString - IvrString:", szPreDefinedIvrString);

	char  numericIdAndPasswordString[DTMF_MAX_BUFFER_SIZE] = {0};
	char  numericIdString           [DTMF_MAX_BUFFER_SIZE] = {0}; // This is the buffer we set in the DTMF collector
	char* isH323String = (char*)strstr(szPreDefinedIvrString, "#");

	if (isH323String)
	{
		if ('#' == isH323String[1])
			strncpy(numericIdAndPasswordString, isH323String+2, sizeof(numericIdAndPasswordString)-1); // H.323 party skip the "##"
		else
			strncpy(numericIdAndPasswordString, isH323String+1, sizeof(numericIdAndPasswordString)-1);

		numericIdAndPasswordString[sizeof(numericIdAndPasswordString)-1] = '\0';
	}

	// NumericIdAndPasswordString Can be "1234##5678" for numeric id + password or "1234" for numeric id only or password only
	if (isH323String)
	{
		int TwoStrings = 0; // Flag whether the predefined IVR string consists of two strings

		// PasswordString will point to the string which starts with "#" if exists, or null if not exists.
		const char* passwordString = strstr(numericIdAndPasswordString, "#");
		const char* tempString     = NULL;

		if (strcmp(NameOf(), "CIvrSubCascadeLink") == 0)
		{
			m_passwordForCascadeLink[0]  = '\0';
			m_numericIdForCascadeLink[0] = '\0';
			int length = 0;

			if (passwordString != NULL)
			{
				if ('#' == passwordString[1])
					strncpy(m_passwordForCascadeLink, passwordString+2, sizeof(m_passwordForCascadeLink)-1); // skip the "##"
				else
					strncpy(m_passwordForCascadeLink, passwordString+1, sizeof(m_passwordForCascadeLink)-1); // skip the "#"

				m_passwordForCascadeLink[sizeof(m_passwordForCascadeLink)-1] = '\0';
			}

			char* tempNidForCascadeLink = strtok(numericIdAndPasswordString, "#");
			if (tempNidForCascadeLink != NULL)
			{
				strncpy(m_numericIdForCascadeLink, tempNidForCascadeLink, sizeof(m_numericIdForCascadeLink)-1);
				m_numericIdForCascadeLink[sizeof(m_numericIdForCascadeLink)-1] = '\0';
			}

			return;
		}

		if (passwordString != NULL)                                    // If the string contains "#"
		{
			if (strncmp(passwordString, "#", DTMF_MAX_BUFFER_SIZE) == 0) // If the string is "#"
			{
				tempString = numericIdAndPasswordString;                   // One string only terminated by "#"
			}
			else
			{
				// PasswordString points to "#5678" and  TempString points to "1234" as in the example above
				tempString = strtok(numericIdAndPasswordString, "#");
				TwoStrings = 1;
			}
		}
		else
			tempString = numericIdAndPasswordString; // One string only without "#" at the end

		if (strcmp(NameOf(), "CIvrSubConfPassword") == 0)
		{
			// In case this is conference password feature
			if (passwordString != NULL && TwoStrings)  // And in case we have two strings
			{
				if ('#' == passwordString[1])
					tempString = passwordString+2;         // Set TempString to point at the password and skip the "##" at the start
				else
					tempString = passwordString+1;
			}
		}

		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
		if (pCommConf)
		{
			CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
			if (pConfParty && TwoStrings == 0)
				pConfParty->SetPreDefinedIvrString(""); // to avoid use that string again in other features

		}

		int len = 0;
		if (tempString)
		{
			len = strlen(tempString);
			if (strstr(tempString, "#")) // If the string consists "#"
				len--;
		}

		if (len < DTMF_MAX_BUFFER_SIZE - 2 && len)
		{
			// Add a delimiter at the end of the string
			char ivrServiceName[AV_MSG_SERVICE_NAME_LEN] = {0};
			char ivrDelimiter[MAX_DELIMETER_LEN]         = {0};

			if (pCommConf)
				strncpy(ivrServiceName, pCommConf->GetpAvMsgStruct()->GetAvMsgServiceName(), sizeof(ivrServiceName) - 1);

			ivrServiceName[sizeof(ivrServiceName) - 1] = '\0';

			CAVmsgService* pAVmsgService = ::GetpAVmsgServList()->GetCurrentAVmsgService(ivrServiceName);
			if (pAVmsgService)
			{
				const CIVRService* pIVRService = pAVmsgService->GetIVRService();
				if (pIVRService)
				{
					strncpy(ivrDelimiter, pIVRService->GetDelimiter(), sizeof(ivrDelimiter) - 1);
					ivrDelimiter[sizeof(ivrDelimiter) - 1] = '\0';
				}
			}

			strncpy(numericIdString, tempString, DTMF_MAX_BUFFER_SIZE - 1);
			numericIdString[DTMF_MAX_BUFFER_SIZE - 1] = '\0';

			if (('#' == ivrDelimiter[0]) || ('*' == ivrDelimiter[0]))
				numericIdString[len] = ivrDelimiter[0];     // '#' or '*';
			else
				numericIdString[len] = '#';                 // should not happened

			numericIdString[len+1] = '\0';
		}

		PTRACE2(eLevelInfoNormal, "CIvrSubBaseSM::ParsePreDefinedIvrString - Send to DTMF buffer:", numericIdString);
		m_pDtmf->FillDtmfBuffer(numericIdString);
	}
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::OnIvrSubBaseNull(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubBaseSM::OnIvrSubBaseNull : Null Function:", NameOf());
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::EndFeature(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << (WORD)status;

	m_pPartyApi->SendMsg(seg, IVR_END_OF_FEATURE);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::EndFeatureError(WORD status)
{
	CSegment* seg = new CSegment;
	*seg << (WORD)status;

	m_pPartyApi->SendMsg(seg, IVR_EROR_FEATURE);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::CreatePartyApi(CParty* pParty, CConfApi* pConfApi)
{
	// conf API
	m_pConfApi = pConfApi;

	// party API
	m_pPartyApi = new CPartyApi;
	m_pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());

	m_pParty = pParty;
}

//--------------------------------------------------------------------------
int CIvrSubBaseSM::GetDtmfResult(CSegment* pParam, WORD& opcode, WORD& rDigits)
{
	int i = 0;
	m_dtmf[0] = 0;

	*pParam >> opcode;        // (OP_ERROR, OP_NOOP or one from the table)
	*pParam >> rDigits;

	if (opcode == OP_ERROR)   // timeout without any DTMF
	{
		if (IsDTMFWODelimeterEnabled() && rDigits>0)
		{
			TRACEINTO <<"DTMF is read without delimeter changing the state to OP_NOOP";
			opcode = OP_NOOP;
		}
		else
		{
			PTRACE(eLevelError, "CIvrSubBaseSM::GetDtmfResult: OP_ERROR F_IVR===>ConfPW");
			return 1;
		}
	}

	if (rDigits >= MAX_DTMF_STRING_LENGTH)
	{
		PTRACE(eLevelError, "CIvrSubBaseSM::GetDtmfResult: Error: MAX_DTMF_STRING_LENGTH F_IVR===>ConfPW");
		return 1;
	}

	if (opcode == OP_NOOP)
	{
		if ((rDigits != m_DTMF_digits) && (0 < m_DTMF_digits))    // illegal value
		{
			PTRACE(eLevelError, "CIvrSubBaseSM::GetDtmfResult: Error: rDigits F_IVR===>ConfPW");
			return 2;
		}

		for (i = 0; i < rDigits; i++)
		{
			BYTE tmp;
			*pParam >> tmp;
			m_dtmf[i] = tmp;
		}

		m_dtmf[i] = 0;
	}

	return 0;
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::SetIvrService(CIVRService* pIvrService)
{
	m_pIvrService = pIvrService;
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::PlayOtherMessage(DWORD messageMode, WORD feature_op_code, const char* file_name)
{
	// save previous message name
	ALLOCBUFFER(save_file_name, NEW_FILE_NAME_LEN); // temp
	strncpy(save_file_name, m_msg_file_name, NEW_FILE_NAME_LEN);
	WORD save_feature_op_code = m_event_opcode;

	// set the new message name
	if (file_name)
	{
		strncpy(m_msg_file_name, file_name, sizeof(m_msg_file_name) - 1);
		m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
	}

	m_event_opcode = feature_op_code;

	// play the message
	PlayMessage(messageMode);


	// restore previous message name
	strncpy(m_msg_file_name, save_file_name, sizeof(m_msg_file_name) - 1);
	m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
	m_event_opcode                               = save_feature_op_code;
	DEALLOCBUFFER(save_file_name);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::SendCode(WORD opcode)
{
	if (!m_pPartyApi)
		return;

	CSegment* seg = new CSegment;

	*seg << (WORD)opcode;     //
	*seg << (WORD)0;          // number of characters

	m_pPartyApi->sendPartyArqInd(seg, DTMF_OPCODE_IDENT, 1);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::OnEndFeature(WORD status)
{
	m_pDtmf->EndFeature();
	if (DISCONNECT_IVR_PROVIDER_STS == status)
	{
		PTRACE(eLevelError, "CIvrSubBaseSM::OnEndFeature - status DISCONNECT_IVR_PROVIDER_STS for IvrProviderEQ");
		EndFeature(status);
		return;
	}

	if (0 == status)
	{
		int stt = DoSomething();  // if end OK: do something
		EndFeature(stt);          // end of feature
	}
	else
		EndFeatureError(status);
}

//--------------------------------------------------------------------------
int CIvrSubBaseSM::IsIvrPtrs()
{
	if ((m_pIvrService) && (m_pDtmf) && (m_ivrCntl))
		return 1;

	PTRACE(eLevelError, "CIvrSubBaseSM::IsIvrPtrs: Illegal IVR pointers  ");
	return 0;
}

//--------------------------------------------------------------------------
WORD CIvrSubBaseSM::GetMsgDuration(const CIVRFeature* feature, const WORD event_opcode)
{
	// time delay to let the message be herd
	WORD       duration = 12; // some default
	CIVREvent* civre    = feature->GetCurrentIVREvent(event_opcode);
	if (civre)
	{
		CIVRMessage* civrm = civre->GetCurrentIVRMessage(m_language);
		if (civrm)
			duration = civrm->GetMsgDuration();
		else
		{
			PTRACE(eLevelError, "CIvrSubBaseSM::GetMsgDuration - CIVRMessage == NULL  ");
		}
	}
	else
	{
		PTRACE(eLevelError, "CIvrSubBaseSM::GetMsgDuration - CIVREvent == NULL  ");
	}

	return duration;
}

//--------------------------------------------------------------------------
char CIvrSubBaseSM::GetIvrDelimiter(char defDelimiter)
{
	if (m_pIvrService)
	{
		const char* ivrDelimiter = m_pIvrService->GetDelimiter();
		if (ivrDelimiter)
			if ('*' == ivrDelimiter[0] || '#' == ivrDelimiter[0])
				return ivrDelimiter[0];
	}
	return defDelimiter;
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::StopPlayTone(void)
{
	PTRACE(eLevelInfoNormal, "CIvrSubBaseSM::StopPlayTone - Stop playing tone");
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::StartBaseTimer(void)
{
	if (m_pParty)
	{
		m_initTimer = 1;
	}
}

//--------------------------------------------------------------------------
int CIvrSubBaseSM::Get_debug_message_use(void)
{
#ifdef WINNT    // for NT EndPoint simulation
	return debug_message_use;
#endif // ifdef WINNT
	return 0;
}

//--------------------------------------------------------------------------
WORD CIvrSubBaseSM::IsActive()
{
	return (m_state == ACTIVE);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::TraceInt(WORD trace_flag, const char* str, int val)
{
	ALLOCBUFFER(IntStr, 16);
	sprintf(IntStr, "%d", val);
	PTRACE2(trace_flag, str, IntStr);
	DEALLOCBUFFER(IntStr);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::CdrPartyDtmfFailureIndication(const char* correctData)
{
	DWORD       confID         = m_pParty->GetMonitorConfId();
	const char* partyName      = m_pParty->GetName();
	DWORD       partyMonitorID = m_pParty->GetMonitorPartyId();
	const char* dtmfCode       = (const char*)m_dtmf;
	DWORD       FailureState   = m_feature_opcode;

	CCommConf*  pCommConf      = new CCommConf;
	pCommConf->PartyDtmfFailureIndicationToCDR(confID, partyName, partyMonitorID, dtmfCode, correctData, FailureState);

	POBJDELETE(pCommConf);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::GetTypeName(char* typeName, WORD reqType)
{
	typeName[0] = '\0';

	switch (reqType)
	{
		case EXDB_TYPE_ConfPW: strcpy(typeName, "F_IVR===>ConfPW"); break;
		case EXDB_TYPE_ChairPW: strcpy(typeName, "F_IVR===>ChairPW"); break;
		case EXDB_TYPE_ConfNID: strcpy(typeName, "F_IVR===>NID"); break;
		case EXDB_TYPE_InviteParty: strcpy(typeName, "F_IVR===>INVITE PARTY"); break;
	} // switch
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::ReqPwdConfirmation(WORD reqType, const char* numericConfId,
                                       DWORD partyId, const char* pwd, Phone* pPhone1, Phone* pPhone2,
                                       BYTE isLeader, BYTE isGuest, const char* protocolTypeStr, IP_EXT_DB_STRINGS* ipStringsStruct)
{
	char typeName[40];
	GetTypeName(typeName, reqType);
	PTRACE2(eLevelInfoNormal, "CIvrSubBaseSM::ReqPwdConfirmation ", typeName);

	CXMLDOMElement* pRootNode = NULL;

	if (EXDB_TYPE_ConfNID == reqType) // Numeric-ID
		pRootNode = new CXMLDOMElement("REQUEST_CONF_DETAILS");
	else
		pRootNode = new CXMLDOMElement("REQUEST_PARTY_DETAILS");

	CXMLDOMElement* pTempNode;
	CSysConfig*     sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	pTempNode = pRootNode->AddChildNode("LOGIN");

	char mcuIp[100];
	strcpy(mcuIp, "");
	pTempNode->AddChildNode("MCU_IP", mcuIp);

	// get login string
	char        login[100];
	std::string data;
	std::string key = CFG_KEY_EXTERNAL_DB_LOGIN;
	sysConfig->GetDataByKey(key, data);
	strncpy(login, data.c_str(), 99);
	login[99] = '\0';
	if (0 < strlen(login))
		pTempNode->AddChildNode("USER_NAME", login);

	data = "";

	// get pw string
	char pw[100];
	key = CFG_KEY_EXTERNAL_DB_PASSWORD;
	sysConfig->GetDataByKey(key, data);
	strncpy(pw, data.c_str(), 99);
	pw[99] = '\0';
	if (0 < strlen(pw))
		pTempNode->AddChildNode("PASSWORD", pw);

	// partyRsrcID
	pRootNode->AddChildNode("TOKEN", m_pParty->GetPartyRsrcID()); // be sent to the external-DB and return as is for internal usage

	pTempNode = pRootNode->AddChildNode("ACTION");

	if (EXDB_TYPE_ConfNID == reqType)                             // Numeric-ID
		pTempNode = pTempNode->AddChildNode("CREATE");
	else
		pTempNode = pTempNode->AddChildNode("ADD");

	pTempNode->AddChildNode("NUMERIC_ID", numericConfId);

	if (EXDB_TYPE_ConfNID != reqType) // NOT Numeric-ID
	{
		pTempNode->AddChildNode("PASSWORD", pwd);

		if (isLeader == TRUE)
		{
			pTempNode->AddChildNode("GUEST", "FALSE");
			pTempNode->AddChildNode("LEADER", "TRUE");
		}
		else
		{
			pTempNode->AddChildNode("LEADER", "FALSE");

			if (isGuest == FALSE)
				pTempNode->AddChildNode("GUEST", "FALSE");
			else
				pTempNode->AddChildNode("GUEST", "TRUE");
		}
	}

	CXMLDOMElement* pPartyPhonesNode = NULL;

	pPartyPhonesNode = pTempNode->AddChildNode("ACTUAL_PARTY_PHONES");

	if (pPhone1)
		pPartyPhonesNode->AddChildNode("PHONE1", pPhone1->phone_number);
	else
		pPartyPhonesNode->AddChildNode("PHONE1", "");

	if (pPhone2)
		pPartyPhonesNode->AddChildNode("PHONReqPwdConfirmationE2", pPhone2->phone_number);
	else
		pPartyPhonesNode->AddChildNode("PHONE2", "");

	if (protocolTypeStr != NULL)
		if (strlen(protocolTypeStr) > 0)
			pTempNode->AddChildNode("PROTOCOL_TYPE", protocolTypeStr);

	if (ipStringsStruct != 0)
	{
		if (0 != ipStringsStruct->ipAddress[0])
			pTempNode->AddChildNode("IP_ADDRESS", ipStringsStruct->ipAddress);

		std::string data;
		char        strSize[16];
		strSize[0] = '\0';

		for (int i = 0; i < NUM_OF_IDENTIFIERS; i++)
		{
			if (0 != ipStringsStruct->identifier[i][0])
			{
				sprintf(strSize, "%d", i+1);

				data  = "IDENTIFIER";
				data += strSize;
				pTempNode->AddChildNode(data.c_str(), ipStringsStruct->identifier[i]);
			}
		}
	}

	pTempNode = pTempNode->AddChildNode("PARTY_ID", partyId);
	// end IBM addition

	char* pStrReq = NULL;
	pRootNode->DumpDataAsLongStringEx(&pStrReq);

	DWORD opcode = 0;
	if (EXDB_TYPE_ConfNID != reqType) // for REQUEST_PARTY_DETAILS
		opcode = EXT_DB_PWD_CONFIRM;
	else                              // for "REQUEST_CONF_DETAILS"
		opcode = EXT_DB_CONF_CONFIRM;

	if (pStrReq)
	{
		const COsQueue& rcvMbx = m_pParty->GetRcvMbx();
		DWORD           len    = strlen(pStrReq);
		if (len > 0)
		{
			CSegment* seg = new CSegment;
			*seg << opcode
			     << len
			     << pStrReq;
			rcvMbx.Serialize(*seg);

			// gets the SQL Application mailbox
			CManagerApi apiQA_Api(eProcessQAAPI);
			STATUS      res = apiQA_Api.SendMsg(seg, SEND_REQ_TO_EXT_DB);

			PTRACE2(eLevelInfoNormal, "CIvrSubBaseSM::ReqPwdConfirmation - F_IVR===>Base, Send command to eProcessQAAPI ", m_pParty->GetName());
		}
		else
			PTRACE2(eLevelError, "CIvrSubBaseSM::ReqPwdConfirmation - F_IVR===>Base, pStrReq length = 0 ", m_pParty->GetName());

		delete [] pStrReq;
	}
	else
		PTRACE2(eLevelError, "CIvrSubBaseSM::ReqPwdConfirmation - F_IVR===>Base, pStrReq= NULL ", m_pParty->GetName());

	PDELETE(pRootNode);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::FillIpIdentifiers(CConfParty* pConfParty, IP_EXT_DB_STRINGS* ipStringsStruct)
{
	// gets ip address if exists
	m_pParty->GetIpCallIdentifiers(ipStringsStruct);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::OnStopIvr()
{
}

//--------------------------------------------------------------------------
WORD CIvrSubBaseSM::GetCachePriority(WORD feature_opcode, WORD event_opcode)
{
	// m_feature_opcode, m_event_opcode

	WORD priority = PRIORITY_0_CACHE_IVR;

	switch (feature_opcode)
	{
		case IVR_FEATURE_NUMERIC_CONFERENCE_ID:
		case IVR_FEATURE_WELCOME:
		case IVR_FEATURE_CONF_PASSWORD:
		case IVR_FEATURE_ROLL_CALL:
			priority = PRIORITY_5_CACHE_IVR;
			break;
		case IVR_FEATURE_CONF_LEADER:
			priority = PRIORITY_2_CACHE_IVR;
			break;
		case IVR_FEATURE_BILLING_CODE:
			priority = PRIORITY_1_CACHE_IVR;
			break;
		case IVR_FEATURE_GENERAL:
			priority = PRIORITY_1_CACHE_IVR;
			break;
		case IVR_FEATURE_AUDIO_EXTERNAL:
			priority = PRIORITY_0_CACHE_IVR;
		default:
			break;
	} // switch

	return priority;
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::OnTimerDiscIvrProviderExtDB() // ivrProviderEQ
{
	PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnTimerDiscIvrProviderExtDB - Start for IvrProviderEQ");

	OnEndFeature(DISCONNECT_IVR_PROVIDER_STS);
}

//--------------------------------------------------------------------------
void CIvrSubBaseSM::ReqConfNIDConfirmationSIP(const char* numericConfId)
{
	TRACEINTO<<"CIvrSubBaseSM::ReqConfNIDConfirmationSIP - START for IvrProviderEQ";

	m_pPartyApi->ReqConfNIDConfirmationSIP(numericConfId);
}

//--------------------------------------------------------------------------
bool CIvrSubBaseSM::IsDTMFWODelimeterFlagEnabled()
{
	bool bEnableDTMFWithoutDelimeter = false;
	std::string key = CFG_KEY_ENABLE_DTMF_NUMBER_WO_DELIMITER;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(key, bEnableDTMFWithoutDelimeter);
	return bEnableDTMFWithoutDelimeter;
}



PBEGIN_MESSAGE_MAP(CIvrSubConfPassword)

  ONEVENT(DTMF_STRING_IDENT     ,NOTACTIVE    ,CIvrSubBaseSM::OnIvrSubBaseNull) 
  ONEVENT(DTMF_STRING_IDENT     ,ACTIVE       ,CIvrSubBaseSM::OnDtmfString) 
  ONEVENT(DTMF_STRING_IDENT     ,WAIT_FOR_DB  ,CIvrSubBaseSM::NullActionFunction) 
  ONEVENT(TIMER_RETRY_MESSAGE   ,ACTIVE       ,CIvrSubBaseSM::OnTimeRetryMessage) 
  ONEVENT(EXT_DB_RESPONSE_TOUT  ,WAIT_FOR_DB  ,CIvrSubConfPassword::OnTimeNoExtDBResponse) 
  ONEVENT(EXT_DB_PWD_CONFIRM    ,WAIT_FOR_DB  ,CIvrSubConfPassword::OnExtDBResponse) 
  ONEVENT(EXT_DB_REQUEST_FAILED ,WAIT_FOR_DB  ,CIvrSubConfPassword::OnExtDBFailure) 
  
PEND_MESSAGE_MAP(CIvrSubConfPassword,CStateMachine);   

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubConfPassword
////////////////////////////////////////////////////////////////////////////
CIvrSubConfPassword::CIvrSubConfPassword(const CIVRConfPasswordFeature* pConfPasswordFeature, const char* pConfPassword, BOOL bEnableExternalDB)
{
	if (pConfPassword)
	{
		strncpy(m_pConfPassword, pConfPassword, MAX_PASSWORD_LEN);
		m_pConfPassword[MAX_PASSWORD_LEN] = 0;
	}
	else
		m_pConfPassword[0] = 0;

	m_pConfPasswordFeature = new CIVRConfPasswordFeature(*pConfPasswordFeature);
	m_internalState        = -1;
	m_useExternalDB        = IVR_EXTERNAL_DB_NONE;
	m_bEnableExternalDB    = bEnableExternalDB;
	m_skipPrompt           = FALSE;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubConfPassword::~CIvrSubConfPassword()
{
	POBJDELETE(m_pConfPasswordFeature);
}

//--------------------------------------------------------------------------
void* CIvrSubConfPassword::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubConfPassword::Start()
{
	PTRACE2(eLevelInfoNormal, "CIvrSubConfPassword::Start - F_IVR===>ConfPW ", m_pParty->GetName());
	if (!IsIvrPtrs())
		return;

	m_pDtmf->SetIvrPartSessionOn();   // saying we are in IVR session (not changed until end of IVR session)

	StartBaseTimer();                 // start timer capability (InitTimer)

	m_bStopUponDTMF = 1;              // stop message upon first DTMF
	m_DTMF_digits   = 0;
	m_DTMF_timeout  = m_pIvrService->GetUserInputTimeout();
	if (m_DTMF_timeout == 0)
		m_DTMF_timeout = 1;

	m_pinCodeDelimiter = GetIvrDelimiter('#');

	// temp
	int status = STATUS_OK;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	if (pCommConf == NULL)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start: Password, conf NULL pointer - F_IVR===>ConfPW");
		return;
	}

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
	if (pConfParty == NULL)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start: Password, party NULL pointer - F_IVR===>ConfPW");
		return;
	}

	const char* file_name = NULL;

	BYTE bIvrEntryType = 0;

	if (pConfParty->GetConnectionTypeOper() == DIAL_OUT)
	{
		bIvrEntryType = m_pConfPasswordFeature->GetDialOutEntryPassword();
		switch (bIvrEntryType)
		{
			case (REQUEST_PASSWORD):
			{
				file_name = m_pConfPasswordFeature->GetMsgFileName(IVR_EVENT_GET_CONFERENCE_PASSWORD, m_language, status);
				m_event_opcode  = IVR_EVENT_GET_CONFERENCE_PASSWORD;
				m_internalState = REQUEST_PASSWORD;
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start PassWord DIAL_OUT, REQUEST_PASSWORD F_IVR===>ConfPW");
				break;
			}
			case (REQUEST_DIGIT):
			{
				file_name = m_pConfPasswordFeature->GetMsgFileName(IVR_EVENT_GET_DIGIT, m_language, status);
				m_event_opcode  = IVR_EVENT_GET_DIGIT;
				m_internalState = REQUEST_DIGIT;
				m_DTMF_digits   = 1;
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start PassWord DIAL_OUT, REQUEST_DIGIT F_IVR===>ConfPW");
				break;
			}
			case (NO_REQUEST):
			{
				m_internalState = NO_REQUEST;
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start PassWord DIAL_OUT, Illegal value  F_IVR===>ConfPW");
				return;
			}
		} // switch
	}
	else if (pConfParty->GetConnectionTypeOper() == DIAL_IN)
	{
		bIvrEntryType = m_pConfPasswordFeature->GetDialInEntryPassword();
		switch (bIvrEntryType)
		{
			case (REQUEST_PASSWORD):
			{
				file_name = m_pConfPasswordFeature->GetMsgFileName(IVR_EVENT_GET_CONFERENCE_PASSWORD, m_language, status);
				m_event_opcode  = IVR_EVENT_GET_CONFERENCE_PASSWORD;
				m_internalState = REQUEST_PASSWORD;
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start PassWord dial in, REQUEST_PASSWORD F_IVR===>ConfPW");
				break;
			}
			case (REQUEST_DIGIT):
			{
				file_name = m_pConfPasswordFeature->GetMsgFileName(IVR_EVENT_GET_DIGIT, m_language, status);
				m_event_opcode  = IVR_EVENT_GET_DIGIT;
				m_internalState = REQUEST_DIGIT;
				m_DTMF_digits   = 1;
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start PassWord dial in, REQUEST_DIGIT F_IVR===>ConfPW");
				break;
			}
			case (NO_REQUEST):
			{
				m_internalState = NO_REQUEST;
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start PassWord dial in, Illegal value F_IVR===>ConfPW");
				return;
			}
		} // switch
	}

	if (file_name)
	{
		strncpy(m_msg_file_name, file_name, sizeof(m_msg_file_name) - 1);
		m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
	}

	m_feature_opcode = IVR_FEATURE_CONF_PASSWORD;

	// update duration if needed
	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);
	GetIVRMessageParameters(msgFullPath, &msgDuration, &msgCheckSum);   // update the duration


	// message duration
	m_msgDuration = GetMsgDuration(m_pConfPasswordFeature, m_event_opcode);
	if (m_msgDuration < 2)
		m_msgDuration = 5;

	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
		m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	m_useExternalDB = IVR_EXTERNAL_DB_NONE;
	if (m_bEnableExternalDB)
		m_useExternalDB = (BYTE)m_pIvrService->GetIvrExternalDB();

	if (IVR_EXTERNAL_DB_NONE == m_useExternalDB)
		PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start - External-DB Not in use F_IVR===>ConfPW");

	const char* szPreDefinedIvrString = pConfParty->GetPreDefinedIvrString();

	if (strncmp(szPreDefinedIvrString, "", DTMF_MAX_BUFFER_SIZE))
	{
		PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start - Start-szPreDefinedIvrString F_IVR===>ConfPW");
		ParsePreDefinedIvrString(szPreDefinedIvrString);
	}

	// /////////////////////////////////////////////////////////////////////////////////////////
	// Check the conditions for skipping conference pwd prompt in "Same Time" environment (IBM)
	// /////////////////////////////////////////////////////////////////////////////////////////
	// 1. system.cfg flag should be set to YES
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key       = CFG_KEY_SKIP_PROMPT;
	BOOL bSkipPrompt;
	sysConfig->GetBOOLDataByKey(key, bSkipPrompt);

	// 2. Connection type should be DIAL_IN
	BYTE connectionType = pConfParty->GetConnectionType();

	// 3. Conference was created by PCAS by a non-web (Same Time) flow - name format: RAS200I_xxx
	BOOL bPcasNonWebFlow = FALSE;
	const char* confName = pCommConf->GetName();
	if (0 == strncmp(confName, "RAS200I_", 8))
		if (0 != strncmp(confName+8, "web_", 4))
			bPcasNonWebFlow = TRUE;

	// 4. External DB access is configured
	BOOL bExternalDbConfigured = FALSE;
	if (IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD == m_useExternalDB)
		bExternalDbConfigured = TRUE;

	if (bSkipPrompt && (DIAL_IN == connectionType) && bPcasNonWebFlow && bExternalDbConfigured)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::Start - Skip conf password prompt scenario: sent '#' to dtmf buffer - F_IVR===>ConfPW");
		m_pDtmf->FillDtmfBuffer("#");
		m_skipPrompt = TRUE;
	}

	// start feature
	StartBase(FALSE, IVR_STATUS_PLAY_ONCE);
}

//--------------------------------------------------------------------------
int CIvrSubConfPassword::IsLegalString(CSegment* pParam)
{
	PTRACE(eLevelError, "CIvrSubConfPassword::IsLegalString - F_IVR===>ConfPW");

	WORD opcode;
	WORD rDigits;
	if (0 != GetDtmfResult(pParam, opcode, rDigits))
	{
		PTRACE(eLevelError, "CIvrSubConfPassword::IsLegalString: Illegal DTMF string F_IVR===>ConfPW");
		if (rDigits == 0)                   // no digit in the string
			return DTMF_TIMEOUT_ERROR;

		return DTMF_STRING_ERROR;           // opcode error or string length error
	}

	if (m_internalState == REQUEST_DIGIT) // the user requested to enter 1 digit
		return DTMF_OK;

	if (!m_pParty)                        // should be valid
	{
		PASSERT(1);
		PTRACE(eLevelError, "CIvrSubConfPassword::IsLegalString: Illegal Party Internal Pointer F_IVR===>ConfPW");
		return DTMF_STRING_ERROR;
	}

	if ((0 == rDigits) && (IVR_EXTERNAL_DB_NONE == m_useExternalDB))  // Error: empty PW (the user pressed only "#"). It is legal in External-DB
	{
		PTRACE(eLevelError, "CIvrSubConfPassword::IsLegalString: Illegal Empty DTMF string F_IVR===>ConfPW");
		return DTMF_STRING_ERROR;
	}

	ALLOCBUFFER(str, 120);    // trace buffer
	str[0] = 0;
	WORD returnValue = DTMF_OK;
	BYTE isLeader    = FALSE;
	BYTE isGuest     = FALSE;
	BOOL isHidePsw   = NO;
	unsigned char invisible_psw [] = "********";
	std::string   key_hide         = "HIDE_CONFERENCE_PASSWORD";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);

	if (opcode == OP_NOOP)    // string was requested
	{
		if (0 != m_dtmf[0])
		{
			PTRACE2(eLevelError, "CIvrSubConfPassword::IsLegalString: DTMF string F_IVR===>ConfPW = ",
			        (!isHidePsw ? (const char*)m_dtmf : "********"));

			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
			if (pCommConf)
			{
				const char* pConfPassword = pCommConf->GetEntryPassword();
				if (pConfPassword)
					strncpy(m_pConfPassword, pConfPassword, sizeof(m_pConfPassword)-1);

				m_pConfPassword[sizeof(m_pConfPassword)-1] = '\0';
			}

			if (0 != strcmp(m_pConfPassword, (const char*)m_dtmf))       // if not conference password
			{
				int notFound = 1;
				// checking if the DTMF is the Leader password...
				const CIVRConfLeaderFeature* leader   = m_pIvrService->GetConfLeaderFeature();
				if (NULL != leader)
				{
					if (YES == (leader->GetEnableDisable()))
					{
						BYTE bIsConfPwAsLeaderPw = (m_pIvrService->GetConfLeaderFeature())->GetConfPwAsLeaderPw();
						if (0 != bIsConfPwAsLeaderPw)
						{
							const char* pConfLeaderPassword = m_ivrCntl->GetLeaderPassword();
							if (pConfLeaderPassword)
							{
								if (0 == strcmp(pConfLeaderPassword, (const char*)m_dtmf))
								{
									isLeader = TRUE;
									if (IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD != m_useExternalDB) // enter as a leader
									{
										m_ivrCntl->SetIsLeader(1, 1);
										m_pDtmf->SetDtmpOpcodePermission(DTMF_LEADER_ACTION);
										sprintf(str, "Password found (enters as leader): [%s]", (isHidePsw ? "********" : pConfLeaderPassword));
										notFound = 0;
									}
								}
							}
						}
					}
					else
					{
						PTRACE2(eLevelInfoNormal, "CIvrSubConfPassword::IsLegalString: Leader Feature Disabled F_IVR===>ConfPW name=", m_pParty->GetName());
					}
				}

				if (notFound)
				{
					if (IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD != m_useExternalDB)  // otherwise it may be a private pin code
					{
						sprintf(str, "expected PW: %s, found PW: %s", (isHidePsw ? "********" : m_pConfPassword),
						        (isHidePsw ? invisible_psw : m_dtmf));
						CdrPartyDtmfFailureIndication((char*)m_pConfPassword);
						returnValue = DTMF_STRING_ERROR;
					}
				}
			}
			else
			{
				isGuest = TRUE;
				sprintf(str, "Password found (%s)", (isHidePsw ? invisible_psw : m_dtmf));
			}
		}
		else
		{
			strncpy(str, "User entered empty Password ", 120);
			if (IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD != m_useExternalDB)
				returnValue = DTMF_STRING_ERROR;
		}
	}

	// check passwords with external DB if needed
	if (IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD == m_useExternalDB)
	{
		int status = STATUS_OK;

		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
		if (!pCommConf)
		{
			PTRACE2(eLevelInfoNormal, "CIvrSubConfPassword::IsLegalString -  pCommConf = NULL PassWord --->ConfPW", m_pParty->GetName());
			returnValue = DTMF_STRING_ERROR;
			status      = STATUS_FAIL;
		}

		if (STATUS_OK == status)
		{
			CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
			if (pConfParty)
			{
				PTRACE2(eLevelInfoNormal, "CIvrSubConfPassword::IsLegalString, accessing external DB F_IVR===>ConfPW, party = ", m_pParty->GetName());
				Phone* pPhone1 = NULL;
				Phone* pPhone2 = NULL;
				Phone* pPhone3 = NULL;
				Phone* pPhone4 = NULL;
				Phone* pPhone5 = NULL;
				DWORD  partyId = pConfParty->GetPartyId();
				std::string protocolTypeStr = "H320";

				IP_EXT_DB_STRINGS ipStringsStruct;
				memset((char*)&ipStringsStruct, 0, sizeof(IP_EXT_DB_STRINGS));
				int debugPstn     = 0;

				BYTE interfaceType = pConfParty->GetNetInterfaceType();
				switch (interfaceType)
				{
					case SIP_INTERFACE_TYPE:
					{
						FillIpIdentifiers(pConfParty, &ipStringsStruct);
						// protocolType = PROTOCOL_SIP;
						protocolTypeStr = "SIP";
						break;
					}

					case H323_INTERFACE_TYPE:
					{
						FillIpIdentifiers(pConfParty, &ipStringsStruct);
						// protocolType = PROTOCOL_H323;
						protocolTypeStr = "H323";
						break;
					}

					case ISDN_INTERFACE_TYPE:
					{
						if (m_ivrCntl->IsPSTNCall())
							// protocolType = PROTOCOL_PSTN;
							protocolTypeStr = "PSTN";
						else
							// protocolType = PROTOCOL_ISDN;
							protocolTypeStr = "H320";

						if (pConfParty->GetConnectionType() == DIAL_OUT)
						{
							pPhone1 = pConfParty->GetFirstCallingPhoneNumber();
							pPhone2 = pConfParty->GetNextCallingPhoneNumber();
						}
						else
						{
							pPhone3 = pConfParty->GetActualPartyPhoneNumber(0);
							pPhone2 = pConfParty->GetActualPartyPhoneNumber(1);
							pPhone1 = pConfParty->GetActualMCUPhoneNumber(0); // rons

							PTRACE2(eLevelError, "CIvrSubConfPassword::IsLegalString -  pConfParty->GetActualPartyPhoneNumber(0)", pPhone3->phone_number);
							PTRACE2(eLevelError, "CIvrSubConfPassword::IsLegalString -  pConfParty->GetActualPartyPhoneNumber(1)", pPhone2->phone_number);
							PTRACE2(eLevelError, "CIvrSubConfPassword::IsLegalString -  pConfParty->GetActualMCUPhoneNumber(0)", pPhone1->phone_number);
						}
						break;
					}

					default:
						PTRACE2(eLevelError, "CIvrSubConfPassword::IsLegalString -  Illegal InterfaceType F_IVR===>ConfPW", m_pParty->GetName());
						break;
				} // switch

				// prepare and send
				ReqPwdConfirmation(EXDB_TYPE_ConfPW, pCommConf->GetNumericConfId(), partyId, (const char*)m_dtmf, pPhone1, pPhone2, isLeader, isGuest, /*protocolType*/ protocolTypeStr.c_str(), &ipStringsStruct);
				m_pDtmf->ResetDtmfParams();   // Delete DTMF collector timers

				if (1 == debugPstn)
				{
					PDELETE(pPhone1);
				}

				// update state, start timer and return
				m_state = WAIT_FOR_DB;
				StartTimer(EXT_DB_RESPONSE_TOUT, TIME_EXT_DB_TIMEOUT*SECOND);
				returnValue = DTMF_IGNORE_ERROR;
			}
			else
			{
				returnValue = DTMF_STRING_ERROR;
				PTRACE2(eLevelInfoNormal, "CIvrSubConfPassword::IsLegalString -  pConfParty = NULL F_IVR===>ConfPW", m_pParty->GetName());
			}
		}
	}

	// print to trace
	ALLOCBUFFER(strParty, H243_NAME_LEN_DEFINED_AGAIN + 50);
	sprintf(strParty, " ---> F_IVR===>ConfPW %s: ", m_pParty->GetName());
	PTRACE2(eLevelInfoNormal, strParty, str);
	DEALLOCBUFFER(str);
	DEALLOCBUFFER(strParty);

	return returnValue;
}

//--------------------------------------------------------------------------
WORD CIvrSubConfPassword::DoSomething()
{
	// do something on success
	m_ivrCntl->SetStartNextFeature();
	return 0;
}

//--------------------------------------------------------------------------
void CIvrSubConfPassword::PlayRetryMessage(DWORD messageMode)
{
	m_retryMessageDuration = GetMsgDuration(m_pConfPasswordFeature, IVR_EVENT_CONFERENCE_PASSWORD_RETRY);
	int status = STATUS_OK;
	const char* file_name = m_pConfPasswordFeature->GetMsgFileName(IVR_EVENT_CONFERENCE_PASSWORD_RETRY, m_language, status);
	PlayOtherMessage(messageMode, IVR_EVENT_CONFERENCE_PASSWORD_RETRY, file_name);
}

//--------------------------------------------------------------------------
// ///////////////////////////////////////////////////////////////////////////
void CIvrSubConfPassword::OnTimeNoExtDBResponse(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubConfPassword::OnTimeNoExtDBResponse- F_IVR===>ConfPW, party=", m_pParty->GetName());
	OnEndFeature(TOO_MANY_ERRORS);
}

//--------------------------------------------------------------------------
int CIvrSubConfPassword::OnExtDBResponse(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, party=", m_pParty->GetName());

	CXMLDOMDocument* pDom      = new CXMLDOMDocument; AUTO_DELETE(pDom);
	CXMLDOMElement*  pTempNode = NULL, * pNode = NULL;
	BYTE             id        = 1, bLeader = FALSE, bVIP = FALSE, bEndFeature = TRUE;
	int              status    = STATUS_OK, nStatus = SEC_OK;
	char*            nodeName  = NULL, * userInfo1 = NULL, * userInfo2 = NULL, * userInfo3 = NULL,
	* userInfo4                = NULL, * name = NULL, * desc = NULL;
	ALLOCBUFFER(pszError, H243_NAME_LEN);     AUTO_DELETE_ARRAY(pszError);
	ALLOCBUFFER(missingField, H243_NAME_LEN); AUTO_DELETE_ARRAY(missingField);
	DWORD XMLStringLen = 0;

	DeleteTimer(EXT_DB_RESPONSE_TOUT);

	*pParam >> XMLStringLen;
	ALLOCBUFFER(pXMLString, XMLStringLen+1);  AUTO_DELETE_ARRAY(pXMLString);
	*pParam >> pXMLString;
	pXMLString[XMLStringLen] = '\0';

	if (pDom->Parse((const char**)&pXMLString) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();

		nStatus = pRoot->get_nodeName(&nodeName);
		if (!strncmp(nodeName, "CONFIRM_PARTY_DETAILS", 22))
		{
			GET_CHILD_NODE(pRoot, "RETURN_STATUS", pNode);
			if (pNode)
			{
				GET_VALIDATE_CHILD(pNode, "ID", &id, _0_TO_10_DECIMAL);
				if (nStatus != SEC_OK)
					sprintf(missingField, "'ID'");

				GET_VALIDATE_CHILD(pNode, "DESCRIPTION", &desc, DESCRIPTION_LENGTH);
			}
			else if (nStatus != SEC_OK)
				sprintf(missingField, "'RETURN_STATUS'");

			if (nStatus == SEC_OK)
			{
				// don't check rest of the XML fields if ID!=0
				if (id == 0)
				{
					GET_CHILD_NODE(pRoot, "ACTION", pTempNode)
					if (pTempNode)
					{
						GET_CHILD_NODE(pTempNode, "ADD", pNode);
						if (pNode)
						{
							GET_VALIDATE_CHILD(pNode, "LEADER", &bLeader, _BOOL);
							if (nStatus != SEC_OK)
							{
								sprintf(missingField, "'LEADER'");
							}
							else
							{
								GET_VALIDATE_CHILD(pNode, "NAME", &name, _0_TO_H243_NAME_LENGTH);
								GET_VALIDATE_CHILD(pNode, "VIP", &bVIP, _BOOL);
								GET_CHILD_NODE(pNode, "CONTACT_INFO_LIST", pTempNode);

								if (pTempNode)
								{
									CXMLDOMElement* pContactInfoNode;

									GET_FIRST_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

									if (pContactInfoNode)
									{
										GET_VALIDATE(pContactInfoNode, &userInfo1, ONE_LINE_BUFFER_LENGTH);

										GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

										if (pContactInfoNode)
										{
											GET_VALIDATE(pContactInfoNode, &userInfo2, ONE_LINE_BUFFER_LENGTH);

											GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

											if (pContactInfoNode)
											{
												GET_VALIDATE(pContactInfoNode, &userInfo3, ONE_LINE_BUFFER_LENGTH);

												GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

												if (pContactInfoNode)
													GET_VALIDATE(pContactInfoNode, &userInfo4, ONE_LINE_BUFFER_LENGTH);
											}
										}
									}
								}
							}
						}
						else if (nStatus != SEC_OK)
							sprintf(missingField, "'ADD'");
					}
					else if (nStatus != SEC_OK)
						sprintf(missingField, "'ACTION'");
				}
			}
		}
		else
		{
			PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - F_IVR===>ConfPW, confirmation transaction is not recognized ");
			nStatus = STATUS_ILLEGAL;
		}
	}

	if (strlen(missingField) > 0 || nStatus == STATUS_ILLEGAL)
	{
		PTRACE2(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, XML string does not include the mandatory field ", missingField);
		PASSERT(STATUS_ILLEGAL);
		OnEndFeature(TOO_MANY_ERRORS);
	}
	else
	{
		if (desc && strcmp(desc, ""))
			PTRACE2(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Description = ", desc);

		m_state = ACTIVE;
		// if party is not authorized
		switch (id)
		{
			case (0):
			{
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, DB replied - OK. ");
				// if (nStatus!=STATUS_NODE_MISSING)
				if (bLeader)
				{
					m_ivrCntl->SetIsLeader(1, 1);
					m_pDtmf->SetDtmpOpcodePermission(DTMF_LEADER_ACTION);
				}

				CConfParty* pConfParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(m_pParty->GetMonitorConfId(), m_pParty->GetMonitorPartyId());
				if (pConfParty)
				{
					pConfParty->SetIsVip(bVIP);
					CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
					std::string key       = CFG_KEY_VISUAL_NAME_CONVENTION;
					DWORD       dwVisualNameConvention;
					sysConfig->GetDWORDDataByKey(key, dwVisualNameConvention);
					if (1 == dwVisualNameConvention)
					{
						if (name)
						{
							char* NewNameToSearch = new char[H243_NAME_LEN];
							int   counter         = 1;
							strncpy(NewNameToSearch, name, H243_NAME_LEN);

							do
							{
								status = ::GetpConfDB()->SearchPartyVisualName(m_pParty->GetMonitorConfId(), NewNameToSearch);
								if (status == STATUS_OK)
								{
									sprintf(NewNameToSearch, "%s(%03d)", name, counter);
									counter++;
								}
								else
								{
									break;
								}
							}
							while (status == STATUS_OK);

							if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
							{
								status = STATUS_OK;
								pConfParty->SetVisualPartyName(NewNameToSearch); // NEED TO GO THROUGH updateDB - meanwhile there is no call to the CDR from here
							}

							CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

							DEALLOCBUFFER(NewNameToSearch);
						}
					}
					else
					{
						if (name)
						{
							int status = ::GetpConfDB()->SearchPartyVisualName(m_pParty->GetMonitorConfId(), name);

							if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
								pConfParty->SetVisualPartyName(name); // NEED TO GO THROUGH updateDB - meanwhile there is no call to the CDR from here

						}
					}

					pConfParty->SetUserDefinedInfo(userInfo1, 0);
					// in order to activate PCAS event when the CONTACT_INFO arrive from the external DB
					pConfParty->UpdateExtDBUserInfo(userInfo1, 0);

					pConfParty->SetUserDefinedInfo(userInfo2, 1);
					pConfParty->SetUserDefinedInfo(userInfo3, 2);
					pConfParty->SetUserDefinedInfo(userInfo4, 3);
				}
				break;
			}
			case (1):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal MCU user name / password .");
				PASSERT(1);
				status = TOO_MANY_ERRORS;
				break;
			}
			case (2):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Request timed out.");
				status = TOO_MANY_ERRORS;
				break;
			}
			case (3):
			{
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal password.");
				if (m_skipPrompt)
				{
					PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Skip Promp - no retries!");
					status = TOO_MANY_ERRORS;
					break;
				}

				m_errorCounter++;                             // increses error counter
				m_timeoutCounter = 0;                         // resets tomeout counter
				if (m_maxRetryTimes > m_errorCounter)         // allowed to retry
				{
					m_pDtmf->ResetDtmfBuffer();                 // reset DTMF buffer
					StartBase(TRUE, IVR_STATUS_PLAY_ONCE);      // wrong UserId, retries to get UserID again
					bEndFeature = FALSE;
				}
				else
				{
					status = TOO_MANY_ERRORS;   // finish status: ERROR
				}

				break;
			}
			case (4):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal CLI - The originator CLI is not permitted to start conferences.");
				status = TOO_MANY_ERRORS;
				break;
			}
			case (5):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal CLI/NID/Password - The combination of the CLI, NID, Password (or any 2 of them) is not permitted.");
				status = TOO_MANY_ERRORS;
				break;
			}
			case (6):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Internal Error in DB.");
				status = TOO_MANY_ERRORS;
				break;
			}
			default:
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal ID response from DB.");
				PASSERT(7);
				break;
			}
		} // switch

	}

	PDELETE(pDom);
	DEALLOCBUFFER(missingField);
	DEALLOCBUFFER(pXMLString);
	DEALLOCBUFFER(pszError);

	if (bEndFeature)
		OnEndFeature(status);

	return status;
}

//--------------------------------------------------------------------------
void CIvrSubConfPassword::OnExtDBFailure(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubConfPassword::OnExtDBFailure - F_IVR===>ConfPW, party=", m_pParty->GetName());
	OnEndFeature(TOO_MANY_ERRORS);
}

//--------------------------------------------------------------------------
bool CIvrSubConfPassword::IsDTMFWODelimeterEnabled()
{
	return IsDTMFWODelimeterFlagEnabled();
}


PBEGIN_MESSAGE_MAP(CIvrSubWelcome)
  ONEVENT(DTMF_STRING_IDENT     , NOTACTIVE, CIvrSubBaseSM::OnIvrSubBaseNull)
  ONEVENT(DTMF_STRING_IDENT     , ACTIVE   , CIvrSubBaseSM::OnDtmfString)
  ONEVENT(TIMER_WAIT_FOR_WELCOME, ACTIVE   , CIvrSubWelcome::ContinueWelcome)
  ONEVENT(FIRST_DTMF_WAS_DIALED , ACTIVE   , CIvrSubWelcome::OnDTMFReceivedACTIVE)
PEND_MESSAGE_MAP(CIvrSubWelcome,CStateMachine);   

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubWelcome
////////////////////////////////////////////////////////////////////////////
CIvrSubWelcome::CIvrSubWelcome(WORD messageType)
{
	m_messageType = messageType;
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubWelcome::~CIvrSubWelcome()
{
}

//--------------------------------------------------------------------------
void* CIvrSubWelcome::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubWelcome::Start()
{
	PTRACE(eLevelInfoNormal, "CIvrSubWelcome::Start - F_IVR===>Welcome,  ");
	if (!IsIvrPtrs())
		return;

	// set flag if start of IVR session
	m_pDtmf->SetIvrPartSessionOn();

	m_DTMF_digits      = 0;
	m_DTMF_timeout     = 0;
	m_pinCodeDelimiter = ' ';

	m_feature_opcode = IVR_FEATURE_WELCOME;
	if (MSG_WELCOME_TYPE == m_messageType)
		m_event_opcode = IVR_EVENT_WELCOME_MSG;
	else
		m_event_opcode = IVR_EVENT_ENTRANCE_MSG;

	int                       status          = STATUS_OK;
	const CIVRWelcomeFeature* pWelcomeFeature = m_pIvrService->GetWelcomeFeature();
	const char*               file_name       = pWelcomeFeature->GetMsgFileName(m_event_opcode, m_language, status);

	if (file_name && strncmp(file_name, "(None)", NEW_FILE_NAME_LEN))
	{
		strncpy(m_msg_file_name, file_name, sizeof(m_msg_file_name) - 1);
		m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
		PTRACE2(eLevelInfoNormal, "CIvrSubWelcome::Start - F_IVR===>Welcome,FileName: ", m_msg_file_name);

		// also update the message duration if needed
		StartBase(FALSE, IVR_STATUS_PLAY_ONCE);

		// time delay to let the message be herd
		m_msgDuration = GetMsgDuration(pWelcomeFeature, m_event_opcode);
		ALLOCBUFFER(str, H243_NAME_LEN_DEFINED_AGAIN + 100);  // buffer for trace
		str[0] = 0;
		if (m_pParty)                                         // should be OK
			sprintf(str, "duration: %i, Party: %s ", (int)m_msgDuration, m_pParty->GetName());

		PTRACE2(eLevelInfoNormal, "CIvrSubWelcome::Start - F_IVR===>Welcome, ", str);

		if (m_msgDuration < 2)
			m_msgDuration = 5;

		if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
			m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

		str[0] = 0;
		if (m_pParty)   // should be OK
			sprintf(str, "Party: %s ", m_pParty->GetName());

		PTRACE2(eLevelInfoNormal, "CIvrSubWelcome::Start - F_IVR===>Welcome, start waiting message duration ", str);

		DEALLOCBUFFER(str);

		StartTimer(TIMER_WAIT_FOR_WELCOME, (m_msgDuration+1) * SECOND);
		m_pDtmf->InformAboutFirstNumbPressed(); // will skip to the next feature upon first DTMF received
		return;
	}

	ContinueWelcome(NULL);
}

//--------------------------------------------------------------------------
void CIvrSubWelcome::OnDTMFReceivedACTIVE(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubWelcome::OnDTMFNumberReceivedCOLLECT - F_IVR===>Welcome");

	DeleteTimer(TIMER_WAIT_FOR_WELCOME);  // delete timer
	ContinueWelcome(NULL);                // continue to the next feature
}

//--------------------------------------------------------------------------
void CIvrSubWelcome::ContinueWelcome(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubWelcome::ContinueWelcome - F_IVR===>Welcome");

	// next feature
	m_ivrCntl->SetStartNextFeature();
	m_pDtmf->EndFeature();
	EndFeature(0);
}

//--------------------------------------------------------------------------
int CIvrSubWelcome::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}

//--------------------------------------------------------------------------
void CIvrSubWelcome::PlayRetryMessage(DWORD messageMode)
{
}



PBEGIN_MESSAGE_MAP(CIvrSubLeader)
	ONEVENT(DTMF_STRING_IDENT     ,NOTACTIVE    ,CIvrSubLeader::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT     ,ACTIVE       ,CIvrSubLeader::OnDtmfString)
	ONEVENT(DTMF_STRING_IDENT     ,WAIT_FOR_DB  ,CIvrSubBaseSM::NullActionFunction)
	ONEVENT(TIMER_RETRY_MESSAGE   ,ACTIVE       ,CIvrSubBaseSM::OnTimeRetryMessage)
	ONEVENT(EXT_DB_RESPONSE_TOUT  ,WAIT_FOR_DB  ,CIvrSubLeader::OnTimeNoExtDBResponse)
	ONEVENT(EXT_DB_PWD_CONFIRM    ,WAIT_FOR_DB  ,CIvrSubLeader::OnExtDBResponse)
	ONEVENT(EXT_DB_REQUEST_FAILED ,WAIT_FOR_DB  ,CIvrSubLeader::OnExtDBFailure)
PEND_MESSAGE_MAP(CIvrSubLeader,CStateMachine);   

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubLeader
////////////////////////////////////////////////////////////////////////////
CIvrSubLeader::CIvrSubLeader(const CIVRConfLeaderFeature* pLeader, BOOL bEnableExternalDB, BOOL isUserInitiated/* = FALSE*/)
{
	m_pLeaderFeature      = new CIVRConfLeaderFeature(*pLeader);
	m_pConfLeaderPassword = 0;
	m_internalState       = STATE_ENTER;
	m_leaderEnterDtmf     = ' ';
	m_leaderIdentError    = 0;
	m_useExternalDB       = IVR_EXTERNAL_DB_NONE;
	m_bEnableExternalDB   = bEnableExternalDB;
	m_isUserInitiated     = isUserInitiated;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubLeader::~CIvrSubLeader()
{
	POBJDELETE(m_pLeaderFeature);
}

//--------------------------------------------------------------------------
void* CIvrSubLeader::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubLeader::Start()
{
	// Leader --->ChairPW

	bool endFeature = false; //BRIDGE-3414
	bool isPreDefined = false;
	PTRACE(eLevelInfoNormal, "CIvrSubLeader::Start - F_IVR===>ChairPW ");
	if (!IsIvrPtrs())
		return;

	StartBaseTimer();      // start timer capability (InitTimer)

	m_bStopUponDTMF   = 1; // stop message upon first DTMF
	m_leaderEnterDtmf = m_pLeaderFeature->GetLeaderIdentifier()[0];

	//BRIDGE-3414
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	CConfParty* pConfParty = pCommConf? pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId()): NULL;
	const char* szPreDefinedIvrString = pConfParty? pConfParty->GetPreDefinedIvrString(): NULL;
	if (szPreDefinedIvrString && strncmp(szPreDefinedIvrString, "", DTMF_MAX_BUFFER_SIZE))
	{
		TRACEINTO << "Pre-defined dial string found: \"" << szPreDefinedIvrString << "\". party IVR state: " << (int) pConfParty->GetOrdinaryParty() << ", party state: " << (int)pConfParty->GetPartyState();
		if(strcmp(szPreDefinedIvrString, "#**")==0) // because we asked for chair password in EQ, and we don't want to ask again!!
		{
			if (pConfParty->GetOrdinaryParty() == STATUS_PARTY_IVR && !m_isUserInitiated)
			{
				TRACEINTO << "Stop IvrSubLeader because we already asked for chair password in EQ, External Ivr";
				endFeature = true;
			}
		}

		else
		{
			isPreDefined = true;
			TRACEINTO << "Start-szPreDefinedIvrString F_IVR===>chairPW, szPreDefinedIvrString: " << szPreDefinedIvrString;
			ParsePreDefinedIvrString(szPreDefinedIvrString);
			m_pConfLeaderPassword = m_ivrCntl->GetLeaderPassword();
			TRACEINTO << "m_pConfLeaderPassword: " << m_pConfLeaderPassword;
			m_internalState = STATE_CODE;
		}
	}
	TRACEINTO << "endFeature: " << (int)endFeature << ", isPreDefined: " << (int)isPreDefined;
	if(endFeature == false)
	{
		if(isPreDefined == true)
			StartInternal(0, IVR_EVENT_GET_LEADER_PASSWORD);
		else
			StartInternal(1, IVR_EVENT_GET_LEADER_IDENTIFIER);
	}

	else
		EndNotLeader();                         // this party is not a leader
}

//--------------------------------------------------------------------------
void CIvrSubLeader::StartInternal(WORD digits, WORD op_msg)
{
	m_DTMF_digits = digits;
	if (1 == digits)  // waiting for 1 digit to identify user as leader
		m_pinCodeDelimiter = ' ';
	else              // getting leader code stage: number of digits is unknown
		m_pinCodeDelimiter = GetIvrDelimiter('#');

	m_DTMF_timeout = m_pIvrService->GetUserInputTimeout();
	if (m_DTMF_timeout == 0)
		m_DTMF_timeout = 1;

	// temp
	int status = STATUS_OK;
	const char* file_name = m_pLeaderFeature->GetMsgFileName(op_msg, m_language, status);

	if (file_name)
	{
		strncpy(m_msg_file_name, file_name, sizeof(m_msg_file_name) - 1);
		m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
	}

	m_feature_opcode = IVR_FEATURE_CONF_LEADER;
	m_event_opcode   = op_msg;

	// update duration if needed
	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);
	GetIVRMessageParameters(msgFullPath, &msgDuration, &msgCheckSum);   // update the duration

	// message duration
	m_msgDuration = GetMsgDuration(m_pLeaderFeature, m_event_opcode);
	if (m_msgDuration < 2)
		m_msgDuration = 5;

	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
		m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	m_useExternalDB = IVR_EXTERNAL_DB_NONE;
	if (m_bEnableExternalDB)
		m_useExternalDB = (BYTE)m_pIvrService->GetIvrExternalDB();

	// start feature
	StartBase(FALSE, IVR_STATUS_PLAY_ONCE);
}

//--------------------------------------------------------------------------
void CIvrSubLeader::PlayRetryMessage(DWORD messageMode)
{
	m_retryMessageDuration = GetMsgDuration(m_pLeaderFeature, IVR_EVENT_LEADER_PASSWORD_RETRY);
	int status = STATUS_OK;
	const char* file_name = m_pLeaderFeature->GetMsgFileName(IVR_EVENT_LEADER_PASSWORD_RETRY, m_language, status);
	PlayOtherMessage(messageMode, IVR_EVENT_LEADER_PASSWORD_RETRY, file_name);
}

//--------------------------------------------------------------------------
int CIvrSubLeader::IsLegalString(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubLeader::IsLegalString - F_IVR===>ChairPW ", m_pParty->GetName());

	WORD opcode;
	WORD rDigits;

	if (0 != GetDtmfResult(pParam, opcode, rDigits))
	{
		PTRACE2(eLevelInfoNormal, "CIvrSubLeader::IsLegalString -  GetDtmfResult Failed - F_IVR===>ChairPW  party = ", m_pParty->GetName());
		if (rDigits == 0)
			return DTMF_TIMEOUT_ERROR;

		return DTMF_STRING_ERROR;
	}

	char str[200]; // buffer for trace
	str[0] = '\0';
	BYTE isLeader = FALSE;

	BOOL isHidePsw = NO;
	std::string key_hide  = "HIDE_CONFERENCE_PASSWORD";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);
	unsigned char invisible_psw [] = "********";

	int returnValue = DTMF_OK;
	if (opcode == OP_NOOP)
	{
		if (m_pConfLeaderPassword)
		{
			if (0 != m_pConfLeaderPassword[0])    // if NULL then error?
			{
				if (0 != strcmp(m_pConfLeaderPassword, (const char*)m_dtmf))
				{
					snprintf(str, sizeof(str), "expected leader PW: %s, found PW: %s", (isHidePsw ? "********" : m_pConfLeaderPassword), (isHidePsw ? invisible_psw : m_dtmf));
					CdrPartyDtmfFailureIndication((char*)m_pConfLeaderPassword);
					returnValue = DTMF_STRING_ERROR;  // not correct leader password
				}
				else
				{
					isLeader = TRUE;
					if (IVR_EXTERNAL_DB_CHAIR_PASSWORD != m_useExternalDB)
					{
						m_ivrCntl->SetIsLeader(1, 1);     // correct leader password
						if (m_ivrCntl->GetStage() == IVR_STAGE_CHANGE_TO_LEADER)
							m_pConfApi->SetPartyAsLeader(m_pParty->GetName(), eOn);

						m_pDtmf->SetDtmpOpcodePermission(DTMF_LEADER_ACTION);
						snprintf(str, sizeof(str), "found PW: %s", (isHidePsw ? "********" : m_pConfLeaderPassword));
					}
				}
			}
			else
				strncpy(str, "m_pConfLeaderPassword is Empty", 200);
		}
		else
			strncpy(str, "m_pConfLeaderPassword is NULL", 200);
	}

	// checks Leader PW with external DB
	if (IVR_EXTERNAL_DB_CHAIR_PASSWORD == m_useExternalDB)
	{
		int status = STATUS_OK;
		PTRACE2(eLevelInfoNormal, "CIvrSubLeader::IsLegalString -  External-DB - F_IVR===>ChairPW, party=", m_pParty->GetName());

		if (STATUS_OK == status)
		{
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
			if (pCommConf)
			{
				CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
				if (pConfParty)
				{
					PTRACE2(eLevelInfoNormal, "CIvrSubLeader::IsLegalString - accessing external DB - F_IVR===>ChairPW, party = ", m_pParty->GetName());
					Phone* pPhone1 = NULL;
					Phone* pPhone2 = NULL;
					DWORD  partyId = pConfParty->GetPartyId();
					std::string protocolTypeStr = "H.320";

					IP_EXT_DB_STRINGS ipStringsStruct;
					memset((char*)&ipStringsStruct, 0, sizeof(IP_EXT_DB_STRINGS));

					BYTE interfaceType = pConfParty->GetNetInterfaceType();
					switch (interfaceType)
					{
						case SIP_INTERFACE_TYPE:
						{
							FillIpIdentifiers(pConfParty, &ipStringsStruct);
							// protocolType = PROTOCOL_SIP;
							protocolTypeStr = "SIP";
							break;
						}

						case H323_INTERFACE_TYPE:
						{
							FillIpIdentifiers(pConfParty, &ipStringsStruct);
							// protocolType = PROTOCOL_H323;
							protocolTypeStr = "H.323";
							break;
						}

						case ISDN_INTERFACE_TYPE:
						{
							if (m_ivrCntl->IsPSTNCall())
								// protocolType = PROTOCOL_PSTN;
								protocolTypeStr = "PSTN";
							else
								// protocolType = PROTOCOL_ISDN;
								protocolTypeStr = "H.320";

							if (pConfParty->GetConnectionType() == DIAL_OUT)
							{
								pPhone1 = pConfParty->GetFirstCallingPhoneNumber();
								pPhone2 = pConfParty->GetNextCallingPhoneNumber();
							}
							else
							{
								pPhone1 = pConfParty->GetActualPartyPhoneNumber(0);
								pPhone2 = pConfParty->GetActualPartyPhoneNumber(1);
							}
							break;
						}

						default:
							PTRACE2(eLevelError, "CIvrSubLeader::IsLegalString -  Illegal InterfaceType F_IVR===>ConfPW", m_pParty->GetName());
							break;
					} // switch

					// prepare and send

					ReqPwdConfirmation(EXDB_TYPE_ChairPW, pCommConf->GetNumericConfId(), partyId, (const char*)m_dtmf, pPhone1, pPhone2, isLeader, FALSE, protocolTypeStr.c_str(), &ipStringsStruct);
					m_pDtmf->ResetDtmfParams();   // Delete DTMF collector timers

					m_state = WAIT_FOR_DB;
					StartTimer(EXT_DB_RESPONSE_TOUT, TIME_EXT_DB_TIMEOUT*SECOND);
					returnValue = DTMF_IGNORE_ERROR;
				}
				else
					PTRACE2(eLevelInfoNormal, "CIvrSubLeader::IsLegalString -  pConfParty=NULL - F_IVR===>ChairPW ", m_pParty->GetName());
			}
			else
				PTRACE2(eLevelInfoNormal, "CIvrSubLeader::IsLegalString -  pCommConf=NULL - F_IVR===>ChairPW ", m_pParty->GetName());
		}
	}

	// print to trace
	PTRACE2(eLevelInfoNormal, "CIvrSubLeader::IsLegalString - F_IVR===>ChairPW", str);

	return returnValue;
}

//--------------------------------------------------------------------------
void CIvrSubLeader::OnTimeNoExtDBResponse(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubLeader::OnTimeNoExtDBResponse - F_IVR===>ChairPW, party=", m_pParty->GetName());
	OnEndFeature(TOO_MANY_ERRORS);
}

//--------------------------------------------------------------------------
int CIvrSubLeader::OnExtDBResponse(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubLeader::OnExtDBResponse - F_IVR===>ChairPW, party=", m_pParty->GetName());

	DeleteTimer(EXT_DB_RESPONSE_TOUT);

	CXMLDOMDocument* pDom      = new CXMLDOMDocument; AUTO_DELETE(pDom);
	CXMLDOMElement*  pTempNode = NULL, * pNode = NULL;
	BYTE             id        = 1, bLeader = FALSE, bVIP = FALSE, bEndFeature = TRUE;
	int              status    = STATUS_OK, nStatus = SEC_OK;
	char*            nodeName  = NULL, * userInfo1 = NULL, * userInfo2 = NULL, * userInfo3 = NULL,
	* userInfo4                = NULL, * name = NULL, * desc = NULL;
	ALLOCBUFFER(pszError, H243_NAME_LEN); AUTO_DELETE_ARRAY(pszError);
	ALLOCBUFFER(missingField, H243_NAME_LEN); AUTO_DELETE_ARRAY(missingField);
	DWORD XMLStringLen = 0;

	*pParam >> XMLStringLen;
	ALLOCBUFFER(pXMLString, XMLStringLen+1); AUTO_DELETE_ARRAY(pXMLString);
	*pParam >> pXMLString;
	pXMLString[XMLStringLen] = '\0';

	if (pDom->Parse((const char**)&pXMLString) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();

		nStatus = pRoot->get_nodeName(&nodeName);
		if (!strncmp(nodeName, "CONFIRM_PARTY_DETAILS", 22))
		{
			GET_CHILD_NODE(pRoot, "RETURN_STATUS", pNode);
			if (pNode)
			{
				GET_VALIDATE_CHILD(pNode, "ID", &id, _0_TO_10_DECIMAL);
				if (nStatus != SEC_OK)
					sprintf(missingField, "'ID'");

				GET_VALIDATE_CHILD(pNode, "DESCRIPTION", &desc, DESCRIPTION_LENGTH);
			}
			else if (nStatus != SEC_OK)
				sprintf(missingField, "'RETURN_STATUS'");

			if (nStatus == SEC_OK)
			{
				// don't check rest of the XML fields if ID!=0
				if (id == 0)
				{
					GET_CHILD_NODE(pRoot, "ACTION", pTempNode);
					if (pTempNode)
					{
						GET_CHILD_NODE(pTempNode, "ADD", pNode);
						if (pNode)
						{
							GET_VALIDATE_CHILD(pNode, "LEADER", &bLeader, _BOOL);
							if (nStatus != SEC_OK)
							{
								sprintf(missingField, "'LEADER'");
							}
							else
							{
								GET_VALIDATE_CHILD(pNode, "NAME", &name, _0_TO_H243_NAME_LENGTH);
								GET_VALIDATE_CHILD(pNode, "VIP", &bVIP, _BOOL);

								GET_CHILD_NODE(pNode, "CONTACT_INFO_LIST", pTempNode);

								if (pTempNode)
								{
									CXMLDOMElement* pContactInfoNode;

									GET_FIRST_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

									if (pContactInfoNode)
									{
										GET_VALIDATE(pContactInfoNode, &userInfo1, ONE_LINE_BUFFER_LENGTH);

										GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

										if (pContactInfoNode)
										{
											GET_VALIDATE(pContactInfoNode, &userInfo2, ONE_LINE_BUFFER_LENGTH);

											GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

											if (pContactInfoNode)
											{
												GET_VALIDATE(pContactInfoNode, &userInfo3, ONE_LINE_BUFFER_LENGTH);

												GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

												if (pContactInfoNode)
													GET_VALIDATE(pContactInfoNode, &userInfo4, ONE_LINE_BUFFER_LENGTH);
											}
										}
									}
								}
							}
						}
						else if (nStatus != SEC_OK)
							sprintf(missingField, "'ADD'");
					}
					else if (nStatus != SEC_OK)
						sprintf(missingField, "'ACTION'");
				}
			}
		}
		else
		{
			PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponseF_IVR===>ChairPW, confirmation transaction is not recognized ");
			nStatus = STATUS_ILLEGAL;
		}
	}

	if (nStatus == STATUS_ILLEGAL || strlen(missingField) > 0)
	{
		PTRACE2(eLevelError, "CIvrSubLeader::OnExtDBResponseF_IVR===>ChairPW, XML string does not include the mandatory field: ", missingField);
		PASSERT(STATUS_ILLEGAL);
		OnEndFeature(TOO_MANY_ERRORS);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CIvrSubLeader::OnExtDBResponse - nStatus=OK - F_IVR===>ChairPW ");

		if (desc && strcmp(desc, ""))
			PTRACE2(eLevelError, "CIvrSubLeader::OnExtDBResponse - F_IVR===>ChairPW, Description = ", desc);

		m_state = ACTIVE;
		// if party is not authorized
		switch (id)
		{
			case (0):
			{
				PTRACE(eLevelInfoNormal, "CIvrSubLeader::OnExtDBResponse - DB replied - OK - F_IVR===>ChairPW ");
				// if (nStatus!=STATUS_NODE_MISSING)
				if (bLeader)
				{
					m_ivrCntl->SetIsLeader(1, 1);
					m_pDtmf->SetDtmpOpcodePermission(DTMF_LEADER_ACTION);
				}

				CConfParty* pConfParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(m_pParty->GetMonitorConfId(), m_pParty->GetMonitorPartyId());
				if (pConfParty)
				{
					if (name)
					{
						int status = ::GetpConfDB()->SearchPartyVisualName(m_pParty->GetMonitorConfId(), name);
						if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
							pConfParty->SetVisualPartyName(name); // NEED TO GO THROUGH updateDB - meanwhile there is no call to the CDR from here
					}

					pConfParty->SetUserDefinedInfo(userInfo1, 0);
					// in order to activate PCAS event when the CONTACT_INFO arrive from the external DB
					pConfParty->UpdateExtDBUserInfo(userInfo1, 0);
					pConfParty->SetUserDefinedInfo(userInfo2, 1);
					pConfParty->SetUserDefinedInfo(userInfo3, 2);
					pConfParty->SetUserDefinedInfo(userInfo4, 3);
				}
				break;
			}

			case (1):
			{
				PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - Illegal MCU user name / password - F_IVR===>ChairPW ");
				PASSERT(1);
				status = TOO_MANY_ERRORS;
				break;
			}

			case (2):
			{
				PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - Request timed out - F_IVR===>ChairPW ");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (3):
			{
				PTRACE(eLevelInfoNormal, "CIvrSubLeader::OnExtDBResponse - Illegal password - F_IVR===>ChairPW ");
				m_errorCounter++;                             // increses error counter
				m_timeoutCounter = 0;                         // resets tomeout counter
				if (m_maxRetryTimes > m_errorCounter)         // allowed to retry
				{
					m_pDtmf->ResetDtmfBuffer();                 // reset DTMF buffer
					StartBase(TRUE, IVR_STATUS_PLAY_ONCE);      // wrong UserId, retries to get UserID again
					bEndFeature = FALSE;
				}
				else
				{
					status = TOO_MANY_ERRORS;   // finish status: ERROR
				}

				break;
			}

			case (4):
			{
				PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - F_IVR===>ChairPW - Illegal CLI - The originator CLI is not permitted to start conferences.");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (5):
			{
				PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - F_IVR===>ChairPW - Illegal CLI/NID/Password - The combination of the CLI, NID, Password (or any 2 of them) is not permitted.");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (6):
			{
				PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - Internal Error in DB - F_IVR===>ChairPW ");
				status = TOO_MANY_ERRORS;
				break;
			}

			default:
			{
				PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - Illegal ID response from DB - F_IVR===>ChairPW ");
				PASSERT(7);
				status = TOO_MANY_ERRORS;
				break;
			}
		} // switch
	}

	PDELETE(pDom);
	DEALLOCBUFFER(missingField);
	DEALLOCBUFFER(pXMLString);
	DEALLOCBUFFER(pszError);

	if (bEndFeature)
	{
		PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - End Of Feature - F_IVR===>ChairPW ");
		OnEndFeature(status);
	}
	else
	{
		PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - Not End Of Feature - F_IVR===>ChairPW ");
	}

	return status;
}

//--------------------------------------------------------------------------
void CIvrSubLeader::OnExtDBFailure(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubLeader::OnExtDBFailure - F_IVR===>ChairPW, party=", m_pParty->GetName());
	OnEndFeature(TOO_MANY_ERRORS);
}

//--------------------------------------------------------------------------
void CIvrSubLeader::OnDtmfString(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubLeader::OnDtmfString - F_IVR===>ChairPW, DTMF message ");

	// in case of getting leader password stage
	if (m_internalState == STATE_CODE)    // getting leader code state
	{
		CIvrSubBaseSM::OnDtmfString(pParam);
		return;
	}

	// get DTMF string result
	WORD opcode;
	WORD rDigits;
	GetDtmfResult(pParam, opcode, rDigits);

	// timeout (0 digits): user is not a leader
	if (0 == rDigits)                         // timeout, leader option skiped by the user
	{
		EndNotLeader();                         // this party is not a leader
	}
	else                                      // rDigits > 0 : either user is a leader or it is a dtmf error
	{
		if (m_leaderEnterDtmf == m_dtmf[0])     // entering into leader code
		{
			// start message
			StartInternal(0, IVR_EVENT_GET_LEADER_PASSWORD);

			// gets the leader password
			m_pConfLeaderPassword = m_ivrCntl->GetLeaderPassword();

			// move to "get leader code" state
			m_internalState = STATE_CODE;
		}
		else                                          // error getting the correct delimiter
		{
			m_leaderIdentError++;                       // error counter
			if (m_leaderIdentError >= m_maxRetryTimes)
			{
				EndNotLeader();                           // this party will be entered as not leader!
			}
			else                                        // try again ('#' for enter as leader)
			{
				StartBase(FALSE, IVR_STATUS_PLAY_ONCE);   // play message again
			}
		}
	}

	return;
}

//--------------------------------------------------------------------------
WORD CIvrSubLeader::DoSomething()
{
	// do something on success
	m_ivrCntl->SetStartNextFeature(); // temporary, to skip to the next feature
	return 0;
}

//--------------------------------------------------------------------------
void CIvrSubLeader::EndNotLeader()
{
	// participant which is not a leader

	// end DTMD feature
	m_pDtmf->EndFeature();

	if (m_ivrCntl->IsLeaderReqForStartConf())
	{
		// currently, until we implement the Conf on Hold option, only the leader can
		// start the conf.
		if (m_ivrCntl->IsEmptyConf())   // this is the first party
		{
			PTRACE(eLevelError, "CIvrSubLeader::EndNotLeader - F_IVR===>ChairPW, can't enter before the leader. ");
			EndFeature(1);                // Error: not alowed to statr to conf
			return;
		}
	}

	// either conf without a leader or the leader is already in conf
	DoSomething();
	EndFeature(0);            // end of feature: OK (no leader)
}

//--------------------------------------------------------------------------
bool CIvrSubLeader::IsDTMFWODelimeterEnabled()
{
	return IsDTMFWODelimeterFlagEnabled();
}

PBEGIN_MESSAGE_MAP(CIvrSubBillingCode)
	ONEVENT(DTMF_STRING_IDENT     ,NOTACTIVE    ,CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT     ,ACTIVE       ,CIvrSubBaseSM::OnDtmfString)
	ONEVENT(TIMER_RETRY_MESSAGE   ,ACTIVE       ,CIvrSubBaseSM::OnTimeRetryMessage)
PEND_MESSAGE_MAP(CIvrSubBillingCode,CStateMachine);   

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubBillingCode
////////////////////////////////////////////////////////////////////////////
CIvrSubBillingCode::CIvrSubBillingCode(const CIVR_BillingCodeFeature* pBillingCodeFeature)
{
	m_pBillingCode[0]     = 0;
	m_pBillingCodeFeature = new CIVR_BillingCodeFeature(*pBillingCodeFeature);
}

//--------------------------------------------------------------------------
CIvrSubBillingCode::~CIvrSubBillingCode()
{
	POBJDELETE(m_pBillingCodeFeature);
}

//--------------------------------------------------------------------------
void* CIvrSubBillingCode::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubBillingCode::Start()
{
	PTRACE(eLevelInfoNormal, "CIvrSubBillingCode::Start - F_IVR===>Billing ");
	if (!IsIvrPtrs())
		return;

	StartBaseTimer();     // start timer capability (InitTimer)

	m_bStopUponDTMF = 1;  // stop message upon first DTMF
	m_DTMF_digits   = 0;
	m_DTMF_timeout  = m_pIvrService->GetUserInputTimeout();
	if (m_DTMF_timeout == 0)
		m_DTMF_timeout = 1;

	m_pinCodeDelimiter = GetIvrDelimiter('#');

	m_feature_opcode = IVR_FEATURE_GENERAL;
	m_event_opcode   = IVR_EVENT_BILLING_NUM;

	// update duration if needed
	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);
	GetIVRMessageParameters(msgFullPath, &msgDuration, &msgCheckSum);   // update the duration

	// get the file name (?what to do in case there is no filename?...)
	int notOK = 1;
	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	if (generalMsgs)
	{
		int status = STATUS_OK;
		const char* file_name = generalMsgs->GetMsgFileName(IVR_EVENT_BILLING_NUM, m_language, status);
		if (file_name)
		{
			strncpy(m_msg_file_name, file_name, sizeof(m_msg_file_name) - 1);
			m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
			m_messagesName = m_msg_file_name; // to find upon the message entry index
			notOK = 0;
		}

		m_msgDuration = GetMsgDuration((CIVRGeneralMsgsFeature*)generalMsgs, IVR_EVENT_BILLING_NUM);
	}

	if (1 == notOK)                     // message "billing" not exists or problem with "general" feature
	{
		PTRACE(eLevelError, "CIvrSubBillingCode::Start - F_IVR===>Billing - Error, skipping this feature ");
		m_ivrCntl->SetStartNextFeature(); // temporary, to skip to the next feature
		EndFeature(STATUS_OK);            // end this feature
		return;
	}

	if (m_msgDuration < 2)
		m_msgDuration = 5;

	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
		m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	// start feature
	StartBase(FALSE, IVR_STATUS_PLAY_ONCE);
}

//--------------------------------------------------------------------------
int CIvrSubBillingCode::IsLegalString(CSegment* pParam)
{
	// in the Billng case: any result is OK, even an empty string
	WORD opcode;
	WORD rDigits;
	GetDtmfResult(pParam, opcode, rDigits);   // copy the DTMF to internal array

	return DTMF_OK;
}

//--------------------------------------------------------------------------
WORD CIvrSubBillingCode::DoSomething()
{
	ALLOCBUFFER(billingCode, MAX_DTMF_STRING_LENGTH);  // MAX_DTMF_STRING_LENGTH is the max size of the data member m_dtmf
	strncpy(billingCode, (const char*)m_dtmf, MAX_DTMF_STRING_LENGTH);
	billingCode[MAX_DTMF_STRING_LENGTH-1] = '\0';

	m_ivrCntl->BillingCodeToCDR(billingCode);

	if (0 == strlen(billingCode))
		strcpy(billingCode, "Empty");

	PTRACE2(eLevelInfoNormal, "CIvrSubBillingCode::DoSomething - F_IVR===>Billing, Billing to CDR ", billingCode);

	m_ivrCntl->SetStartNextFeature();

	DEALLOCBUFFER(billingCode);
	return 0;
}

//--------------------------------------------------------------------------
void CIvrSubBillingCode::PlayRetryMessage(DWORD messageMode)
{
}


PBEGIN_MESSAGE_MAP(CIvrSubGeneral)
	ONEVENT(DTMF_STRING_IDENT     ,NOTACTIVE    ,CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT     ,ACTIVE       ,CIvrSubBaseSM::OnDtmfString)
PEND_MESSAGE_MAP(CIvrSubGeneral,CStateMachine);   

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubGeneral
////////////////////////////////////////////////////////////////////////////
CIvrSubGeneral::CIvrSubGeneral(WORD type) : m_general_type(type)
{
	m_error = 0;
	OFF(m_need_to_stop_previous_msg);
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubGeneral::CIvrSubGeneral()
{
	m_error        = 0;
	m_general_type = 0;
	OFF(m_need_to_stop_previous_msg);
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubGeneral::~CIvrSubGeneral()
{
}

//--------------------------------------------------------------------------
void* CIvrSubGeneral::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubGeneral::Start()
{
	char str[DEF_SIZE_16];
	sprintf(str, "%i", (int)m_general_type);
	PTRACE2(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General,  general type=", str);
	if (!IsIvrPtrs())
		return;

	// ---------- first set DTMF for wanted input ---------
	str[DEF_SIZE_16-1] = 0;
	switch (m_general_type)
	{
		case (GEN_TP_LOCK_SECURE):
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_CONF_LOCK;
			m_bStopUponDTMF    = 0;
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = ' ';
			strncpy(str, "Secure", DEF_SIZE_16-1);
			break;
		}

		case (GEN_TP_CHANGE_PWDS_MENU):
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_CHANGE_PWDS_MENU;
			m_bStopUponDTMF    = 1;
			m_DTMF_digits      = 1;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = ' ';
			strncpy(str, "ChangePWMenu", DEF_SIZE_16-1);
			break;
		}

		case (GEN_TP_CHANGE_CONF_PWD):
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_CHANGE_PWDS_CONF;
			m_bStopUponDTMF    = 1; // stop message upon first DTMF
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = GetIvrDelimiter('#');
			strncpy(str, "ChangeConfPW", DEF_SIZE_16-1);
			break;
		}

		case (GEN_TP_CHANGE_LEADER_PWD):
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_CHANGE_PWDS_LEADER;
			m_bStopUponDTMF    = 1; // stop message upon first DTMF
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = GetIvrDelimiter('#');
			strncpy(str, "ChangeLdrPW", DEF_SIZE_16-1);
			break;
		}

		case (GEN_TP_CHANGE_PWDS_CONFIRM):
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_CHANGE_PWD_CONFIRM;
			m_bStopUponDTMF    = 1; // stop message upon first DTMF
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = GetIvrDelimiter('#');
			strncpy(str, "ConfirmPW", DEF_SIZE_16-1);
			break;
		}

		case (GEN_TP_CHANGE_PWDS_OK):
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_CHANGE_PWD_OK;
			m_bStopUponDTMF    = 0;
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = ' ';
			strncpy(str, "PWOK", DEF_SIZE_16-1);
			break;
		}

		case (GEN_TP_CHANGE_PWDS_INVALID):
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_CHANGE_PWD_INVALID;
			m_bStopUponDTMF    = 0;
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = ' ';
			strncpy(str, "InvalidPW", DEF_SIZE_16-1);
			break;
		}

		case (GEN_TP_MAX_PARTICIPANTS):
		{
			// checks if the IVR file is exists
			if (STATUS_OK != CheckIfIvrMsgsExistInFeature())
			{
				PTRACE(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, - IVR File is missing.");
				PTRACE(eLevelError, "CIvrSubGeneral::Start - F_IVR===>General, - Max Participants: Disconnect Party!");
				OnEndFeature(TOO_MANY_ERRORS);
				return;
			}

			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_MAX_PARTICIPANTS;
			m_bStopUponDTMF    = 0;
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = ' ';
			strncpy(str, "MaxParties", DEF_SIZE_16-1);
			break;
		}

		case GEN_TP_INVITE_PARTY:
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_INVITE_PARTY;
			m_bStopUponDTMF    = 1; // stop message upon first DTMF
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = GetIvrDelimiter('#');
			strncpy(str, "InvitePartyMenu", DEF_SIZE_16-1);
			break;
		}

		case GEN_TP_GW_REINVITE_PARTY:
		case GEN_TP_DTMF_REINVITE_PARTY:
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_REINVITE_PARTY;
			m_bStopUponDTMF    = 1; // stop message upon first DTMF
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = GetIvrDelimiter('#');
			strncpy(str, "ReInvitePartyMenu", DEF_SIZE_16-1);
			break;
		}

		case GEN_TP_PLAY_BUSY_MSG:
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_PLAY_BUSY_MSG;
			m_bStopUponDTMF    = 1;
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = GetIvrDelimiter('#');
			strncpy(str, "PlayBusyMsg", DEF_SIZE_16-1);
			break;
		}

		case GEN_TP_PLAY_NOANSWER_MSG:
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_PLAY_NOANSWER_MSG;
			m_bStopUponDTMF    = 1;
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = GetIvrDelimiter('#');
			strncpy(str, "PlayNoanswerMsg", DEF_SIZE_16-1);
			break;
		}

		case GEN_TP_PLAY_WRONG_NUMBER_MSG:
		{
			m_feature_opcode   = IVR_FEATURE_GENERAL;
			m_event_opcode     = IVR_EVENT_PLAY_WRONG_NUMBER_MSG;
			m_bStopUponDTMF    = 1;
			m_DTMF_digits      = 0;
			m_DTMF_timeout     = m_pIvrService->GetUserInputTimeout();
			m_pinCodeDelimiter = GetIvrDelimiter('#');
			strncpy(str, "PlayWrongNumberMsg", DEF_SIZE_16-1);
			break;
		}

		// default is no DTMF.
		default:
		{
			strncpy(str, "Illegal", DEF_SIZE_16-1);
		}
	} // end switch

	PTRACE2(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, general type=", str);

	// update duration if needed
	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);
	GetIVRMessageParameters(msgFullPath, &msgDuration, &msgCheckSum);   // update the duration

	// gets the file name and the message duration
	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	if (generalMsgs)
	{
		int status = STATUS_OK;
		const char* file_name = generalMsgs->GetMsgFileName(m_event_opcode, m_language, status);
		if (file_name)
		{
			strncpy(m_msg_file_name, file_name, sizeof(m_msg_file_name) - 1);
			m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
			m_messagesName = m_msg_file_name; // to find upon the message entry index
		}

		m_msgDuration = GetMsgDuration((CIVRGeneralMsgsFeature*)generalMsgs, m_event_opcode);
	}

	// time delay to let the message be herd
	if (m_msgDuration < 2)
		m_msgDuration = 5;

	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
		m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	// start feature
	StartBase(FALSE, IVR_STATUS_PLAY_ONCE);

	// -------- then set action to be taken after message --------

	switch (m_general_type)
	{
		case (GEN_TP_LOCK_SECURE):
		{
			SystemSleep(100*m_msgDuration);
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, - after sleep.");
			OnEndFeature(TOO_MANY_ERRORS);
			break;
		}

		case (GEN_TP_CHANGE_PWDS_INVALID):
		{
			SystemSleep(100*m_msgDuration);
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, - after sleep.");
			OnEndFeature(0);
			break;
		}

		case (GEN_TP_CHANGE_PWDS_OK):
		{
			SystemSleep(100*m_msgDuration);
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, - after sleep.");
			OnEndFeature(0);
			break;
		}

		case (GEN_TP_MAX_PARTICIPANTS):
		{
			SystemSleep(100*m_msgDuration);
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, - after sleep.");
			OnEndFeature(TOO_MANY_ERRORS);
			break;
		}

		case (GEN_TP_PLAY_BUSY_MSG):
		{
			SystemSleep(100*m_msgDuration);
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, - after sleep.");
			OnEndFeature(0);
			break;
		}

		case (GEN_TP_PLAY_NOANSWER_MSG):
		{
			SystemSleep(100*m_msgDuration);
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, - after sleep.");
			OnEndFeature(0);
			break;
		}

		case (GEN_TP_PLAY_WRONG_NUMBER_MSG):
		{
			SystemSleep(100*m_msgDuration);
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::Start - F_IVR===>General, - after sleep.");
			OnEndFeature(0);
			break;
		}

		default:
			return;
	} // end switch
}

//--------------------------------------------------------------------------
int CIvrSubGeneral::CheckIfIvrMsgsExistInFeature()
{
	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	if (!generalMsgs)
		return STATUS_FAIL;

	int status = STATUS_OK;

	switch (m_general_type)
	{
		case (GEN_TP_CHANGE_PWDS_MENU):
		{
			#define NUM_IVR_MSGS_IN_PW 6
			int pwOpcode[NUM_IVR_MSGS_IN_PW] = {IVR_EVENT_CHANGE_PWDS_MENU, IVR_EVENT_CHANGE_PWDS_CONF, IVR_EVENT_CHANGE_PWDS_LEADER,
				                                  IVR_EVENT_CHANGE_PWD_CONFIRM, IVR_EVENT_CHANGE_PWD_INVALID, IVR_EVENT_CHANGE_PWD_OK };
			for (int i = 0; i < NUM_IVR_MSGS_IN_PW; i++)
			{
				const char* file_name = generalMsgs->GetMsgFileName(pwOpcode[i], m_language, status);
				if (file_name)
				{
					if (0 == strlen(file_name))
						return STATUS_FAIL;
				}
				else
					return STATUS_FAIL;
			}
			break;
		}

		case (GEN_TP_MAX_PARTICIPANTS):
		{
			const char* file_name = generalMsgs->GetMsgFileName(IVR_EVENT_MAX_PARTICIPANTS, m_language, status);
			if (file_name)
			{
				if (0 == strlen(file_name))
					return STATUS_FAIL;
			}
			else
				return STATUS_FAIL;
			break;
		}
	} // end switch

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CIvrSubGeneral::IsLegalString(CSegment* pParam)
{
	WORD opcode;
	WORD rDigits;

	switch (m_general_type)
	{
		case (GEN_TP_CHANGE_PWDS_INVALID):
			return DTMF_OK;

		case (GEN_TP_CHANGE_PWDS_MENU):
		case (GEN_TP_CHANGE_CONF_PWD):
		case (GEN_TP_CHANGE_LEADER_PWD):
		case (GEN_TP_CHANGE_PWDS_CONFIRM):
		{
			if ((m_maxRetryTimes == m_timeoutCounter+1) ||
			    (m_maxRetryTimes == m_errorCounter+1))
			{
				m_error = 1;
				PTRACE(eLevelInfoNormal, "CIvrSubGeneral::IsLegalString - F_IVR===>General - max retry times");
				return DTMF_OK;
			}
		}
	} // switch

	if (0 != GetDtmfResult(pParam, opcode, rDigits))
	{
		if (rDigits == 0)
		{
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::IsLegalString - F_IVR===>General - 0 digits");
			return DTMF_TIMEOUT_ERROR;
		}
		PTRACE(eLevelInfoNormal, "CIvrSubGeneral::IsLegalString - F_IVR===>General - string error");
		return DTMF_STRING_ERROR;
	}

	switch (m_general_type)
	{
		case (GEN_TP_INVITE_PARTY):
		case (GEN_TP_GW_REINVITE_PARTY):
		case (GEN_TP_DTMF_REINVITE_PARTY):
		{
			if (rDigits == 0)
			{
				PTRACE(eLevelInfoNormal, "CIvrSubGeneral::IsLegalString - F_IVR===>General - 0 digits");
				sleep(1);
				return DTMF_TIMEOUT_ERROR;
			}
			return DTMF_OK;
		}

		case (GEN_TP_CHANGE_PWDS_OK):
			return DTMF_STRING_ERROR;

		case (GEN_TP_CHANGE_PWDS_MENU):
		{
			if (opcode == OP_NOOP)
			{
				if ((0 == strcmp("1", (const char*)m_dtmf)) ||
				    (0 == strcmp("2", (const char*)m_dtmf)) ||
				    (0 == strcmp("9", (const char*)m_dtmf)))
					return DTMF_OK;
				else
				{
					PTRACE(eLevelInfoNormal, "CIvrSubGeneral::IsLegalString - F_IVR===>General, Not legal string (1,2,9), try again");
					m_pDtmf->SetDtmfWaitForXDigitsNumber(m_DTMF_digits, m_DTMF_timeout, m_msgDuration);
					return DTMF_IGNORE_ERROR;
				}
			}
		}

		case (GEN_TP_CHANGE_CONF_PWD):
		{
			if (opcode == OP_NOOP)
			{
				WORD status = CheckLeagalPW();  // check password is valid
				if (status != STATUS_OK)
				{
					PTRACE2(eLevelInfoNormal, "CIvrSubGeneral::IsLegalString - F_IVR===>General, Not legal PW - ", (char*)m_dtmf);
					TraceInt(eLevelInfoNormal, "CIvrSubGeneral::IsLegalString - F_IVR===>General, ConfPW setting failed status=", status);
					m_error = 1;
				}
				return DTMF_OK;
			}
			return DTMF_OK;
		}

		case (GEN_TP_CHANGE_LEADER_PWD):
		{
			if (opcode == OP_NOOP)
			{
				WORD status = CheckLeagalPW();    // check password is valid
				if (status != STATUS_OK)
				{
					TraceInt(eLevelInfoNormal, "CIvrSubGeneral::IsLegalString - F_IVR===>General, LeaderPW setting failed, status=", status);
					m_error = 1;
				}
				return DTMF_OK;
			}
			return DTMF_OK;
		}

		case (GEN_TP_CHANGE_PWDS_CONFIRM):
		{
			if (opcode == OP_NOOP)
				return DTMF_OK;
		}

		case (GEN_TP_CHANGE_PWDS_INVALID):
				return DTMF_OK;

		default:
			return DTMF_OK;
	} // end switch
}

//--------------------------------------------------------------------------
STATUS CIvrSubGeneral::CheckLeagalPW()
{
	PTRACE2(eLevelInfoNormal, "CIvrSubGeneral::CheckLeagalPW  - F_IVR===>General,  ", (char*)m_dtmf);

	// gets the new PW length
	WORD len = strlen((const char*)m_dtmf);

	// checks DTMF string content
	int i = 0;
	for (i = 0; i < len; i++)
	{
		if ((m_dtmf[i] < '0') || (m_dtmf[i] > '9'))
			return STATUS_ILLEGAL;
	}

	// checks if in range
	if (len > MAX_PASSWORD_LENGTH)  // no minimum length, it can be 0
		return STATUS_IVR_ILLEGAL_PW_LENGTH;

	// all OK
	return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CIvrSubGeneral::DoSomething()
{
	PTRACE2INT(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - F_IVR===>General m_general_type:", m_general_type);

	switch (m_general_type)
	{
		case (GEN_TP_CHANGE_PWDS_MENU):
		{
			// if change conf password
			if (0 == strcmp("1", (const char*)m_dtmf))
			{
				m_ivrCntl->SetChangePasswordType(1);
				m_ivrCntl->SetChangeConfPasswordFeature();
			}

			// if change leader password
			if (0 == strcmp("2", (const char*)m_dtmf))
			{
				m_ivrCntl->SetChangePasswordType(2);
				m_ivrCntl->SetChangeLeaderPasswordFeature();
			}

			// if exit feature
			m_ivrCntl->SetStartNextFeature();
			return 0;
		}

		case (GEN_TP_CHANGE_CONF_PWD):
		case (GEN_TP_CHANGE_LEADER_PWD):
		{
			if (!m_error)
			{
				m_ivrCntl->SetNewPassword((char*)m_dtmf);
				m_ivrCntl->SetStartNextFeature();
			}
			else
			{
				PTRACE2(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - F_IVR===>General, was error", (char*)m_dtmf);
				m_error = 0;
				m_ivrCntl->SetChangePasswordFailed();
				m_ivrCntl->SetStartNextFeature();
			}

			return 0;
		}

		case (GEN_TP_CHANGE_PWDS_CONFIRM):
		{
			if (!m_error)
			{
				if (!strncmp(m_ivrCntl->GetNewPassword(), (char*)m_dtmf, CONFERENCE_ENTRY_PASSWORD_LEN))
				{
					CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
					if (pCommConf)
					{
						if (m_ivrCntl->GetChangePasswordType() == 1)
							pCommConf->SetEntryPassword((char*)m_dtmf);

						if (m_ivrCntl->GetChangePasswordType() == 2)
							pCommConf->SetH243Password((char*)m_dtmf);

						if (pCommConf->IsMeetingRoom())
						{
							CCommResDB* pCommResDB = ::GetpMeetingRoomDB();
							// this should be changed, the MR should retrieve according to conf name
							CCommRes*   pCommRes = pCommResDB->GetCurrentRsrv(pCommConf->GetName());
							// CCommRes* pCommRes = pCommResDB->GetCurrentRsrv(m_pParty->GetConfId());
							if (pCommRes)
							{
								pCommRes->SetH243Password(pCommConf->GetH243Password());
								pCommRes->SetEntryPassword(pCommConf->GetEntryPassword());
								if (pCommResDB->Update(*pCommRes))
									PTRACE(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - F_IVR===>General, Failed to Update meeting room in files - ");
								else
									PTRACE(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - F_IVR===>General, MR was Updated OK ");
							}
							POBJDELETE(pCommRes);
						}
					}
				}
				else
				{
					PTRACE2(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - F_IVR===>General, Confirm failed, first:", (char*)m_ivrCntl->GetNewPassword());
					PTRACE2(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - F_IVR===>General, Confirm failed, last:", (char*)m_dtmf);
					m_ivrCntl->SetChangePasswordFailed();
				}
			}
			else
			{
				PTRACE2(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - F_IVR===>General, Confirm: was error", (char*)m_dtmf);
				m_error = 0;
				m_ivrCntl->SetChangePasswordFailed();
			}

			m_ivrCntl->SetStartNextFeature();
			return 0;
		} // end case GEN_TP_CHANGE_PWDS_CONFIRM

		case (GEN_TP_CHANGE_PWDS_OK):
		{
			m_ivrCntl->SetStartNextFeature();
			return 0;
		}

		case (GEN_TP_CHANGE_PWDS_INVALID):
		{
			m_ivrCntl->SetChangePasswordFeature();
			m_ivrCntl->SetStartNextFeature();
			return 0;
		}
		break;

		case (GEN_TP_INVITE_PARTY):
		{
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - GEN_TP_INVITE_PARTY");
			m_ivrCntl->SetInvitePartyAddress((char*)m_dtmf);
			// const CIVRInvitePartyFeature* pInviteParty = m_pIvrService->GetInvitePartyFeature();
			m_pConfApi->InviteParty(m_pParty->GetPartyRsrcID(),
			                        m_pParty->GetMonitorPartyId(),
			                        (char*)m_ivrCntl->GetInvitePartyAddress(),
			                        m_pIvrService->getInterfaceOrderMap());
			m_ivrCntl->SetStartNextFeature();
			return 0;
		}
		break;

		case (GEN_TP_GW_REINVITE_PARTY):
		case (GEN_TP_DTMF_REINVITE_PARTY):
		{
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - GEN_TP_GW_REINVITE_PARTY/GEN_TP_DTMF_REINVITE_PARTY");
			m_ivrCntl->SetInvitePartyAddress((char*)m_dtmf);
			m_pConfApi->InviteParty(m_pParty->GetPartyRsrcID(),
			                        m_pParty->GetMonitorPartyId(),
			                        (char*)m_ivrCntl->GetInvitePartyAddress(),
			                        m_pIvrService->getInterfaceOrderMap());
			m_ivrCntl->SetStartNextFeature();
			return 0;
		}
		break;

		case (GEN_TP_PLAY_BUSY_MSG):
		{
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - GEN_TP_PLAY_BUSY_MSG");
			m_ivrCntl->SetStartNextFeature();
			return 0;
		}
		break;

		case (GEN_TP_PLAY_NOANSWER_MSG):
		{
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - GEN_TP_PLAY_NOANSWER_MSG");
			m_ivrCntl->SetStartNextFeature();
			return 0;
		}
		break;

		case (GEN_TP_PLAY_WRONG_NUMBER_MSG):
		{
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - GEN_TP_PLAY_WRONG_NUMBER_MSG");
			m_ivrCntl->SetStartNextFeature();
			return 0;
		}
		break;

		default:
			PTRACE(eLevelInfoNormal, "CIvrSubGeneral::DoSomething - default");
			return 0;
	} // end switch
}

//--------------------------------------------------------------------------
void CIvrSubGeneral::PlayRetryMessage(DWORD messageMode)
{
	int  status;
	WORD event = 0;

	switch (m_general_type)
	{
		case (GEN_TP_CHANGE_PWDS_MENU):
		{
			event = 0;
			break;
		}

		case (GEN_TP_CHANGE_CONF_PWD):
		case (GEN_TP_CHANGE_LEADER_PWD):
		{
			event = IVR_EVENT_CHANGE_PWD_INVALID;
			break;
		}
	} // end switch

	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	if (generalMsgs && event > 0)
	{
		m_retryMessageDuration = GetMsgDuration((CIVRGeneralMsgsFeature*)generalMsgs, event);
		status = STATUS_OK;
		const char* file_name = generalMsgs->GetMsgFileName(event, m_language, status);
		PlayOtherMessage(messageMode, event, file_name);
	}
}

//--------------------------------------------------------------------------
void CIvrSubGeneral::PlayMessage(DWORD messageMode, WORD loop_delay)
{
	switch (m_general_type)
	{
		case (GEN_TP_GW_REINVITE_PARTY):
		{
			PlayGwReinviteMessage(messageMode, loop_delay);
			break;
		}

		default:
			CIvrSubBaseSM::PlayMessage(messageMode, loop_delay);
			break;
	} // switch
}

//--------------------------------------------------------------------------
void CIvrSubGeneral::PlayGwReinviteMessage(DWORD messageMode, WORD loop_delay)
{
	PTRACE(eLevelInfoNormal, "CIvrSubGeneral::PlayGwReinviteMessage");

	// Allocation of messages id array for the sequence message
	IVRMsgDescriptor* pArrayOfMessagesToPlay = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];
	const char*       ivrServiceName         = m_pIvrService->GetName();

	// Initialize the array
	for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
	{
		pArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
		pArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
		pArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
	}

	WORD wNumberOfMessagesToPlay = 0;

	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);

	int  status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, IVR_FEATURE_GENERAL, IVR_EVENT_REINVITE_PARTY,
	                                                     msgFullPath, &msgDuration, &msgCheckSum);

	if (STATUS_OK != status)
	{
		msgFullPath[0] = '\0';
		msgDuration    = 0;
		msgCheckSum    = (WORD)(-1);
		PTRACE(eLevelError, "CIvrSubGeneral::PlayGwReinviteMessage( reinvite_party message) - F_IVR===>Reinvite, GetIVRMsgParams failed");
	}
	else
	{
		strncpy(pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, msgFullPath, MAX_FULL_PATH_LEN);
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
		wNumberOfMessagesToPlay++;
	}

	m_msgDuration = msgDuration;

	msgFullPath[0] = '\0';
	msgDuration    = 0;
	msgCheckSum    = (WORD)(-1);

	status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, IVR_FEATURE_GENERAL, IVR_EVENT_PLAY_DIAL_TONE,
	                                                msgFullPath, &msgDuration, &msgCheckSum);

	if (STATUS_OK != status)
	{
		msgFullPath[0] = '\0';
		msgDuration    = 0;
		msgCheckSum    = (WORD)(-1);
		PTRACE(eLevelError, "CIvrSubGeneral::PlayGwReinviteMessage( dial_tone message ) - F_IVR===>Reinvite, GetIVRMsgParams failed");
	}
	else
	{
		strncpy(pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, msgFullPath, MAX_FULL_PATH_LEN);
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
		wNumberOfMessagesToPlay++;
	}

	// Play the sequence message in loop
	if (0 != wNumberOfMessagesToPlay)
	{
		if (m_need_to_stop_previous_msg)
			m_pConfApi->StopMessage(m_pParty->GetPartyRsrcID());

		m_pConfApi->StartSequenceMessages(m_pParty->GetPartyRsrcID(), pArrayOfMessagesToPlay, wNumberOfMessagesToPlay, PRIVATE_MSG);
		ON(m_need_to_stop_previous_msg);
	}
	else
		PTRACE(eLevelError, "CIvrSubGeneral::PlayGwReinviteMessage - F_IVR===>Reinvite, Reinvite messages are not configured");

	PDELETEA(pArrayOfMessagesToPlay);
}



const WORD RECORDING = 4;


PBEGIN_MESSAGE_MAP(CIvrSubRollCall)
	ONEVENT(MSG_RECORDED,        RECORDING, CIvrSubRollCall::OnPartyMessageRecorded)
	ONEVENT(END_ROLL_CALL,       RECORDING, CIvrSubRollCall::OnTimerEndRollCallRecording)
	ONEVENT(RECORD_REQUEST,      ACTIVE,    CIvrSubRollCall::PlayRecordMessage)
	ONEVENT(END_ROLL_CALL,       ACTIVE,    CIvrSubRollCall::OnTimerEndRollCallActive)
	ONEVENT(DTMF_RECORD_REQUEST, ACTIVE,    CIvrSubRollCall::OnDtmfPlayRecordMessage)
	ONEVENT(DTMF_RECORD_REQUEST, RECORDING, CIvrSubRollCall::OnDtmfStopRecordingByDtmf)
	ONEVENT(DTMF_STRING_IDENT,   ANYCASE,   CIvrSubRollCall::OnDtmfStringIdentNull)
	ONEVENT(STOP_ROLL_CALL_RECORDING,   	RECORDING,   	CIvrSubRollCall::OnCamStopRollCallRecordingRECORDING)
	ONEVENT(STOP_ROLL_CALL_RECORDING,   	ACTIVE,   		CIvrSubRollCall::OnCamStopRollCallRecordingACTIVE)
	ONEVENT(STOP_ROLL_CALL_RECORDING_ACK,   ANYCASE,   		CIvrSubRollCall::OnCamStopRollCallRecordingAck)
	ONEVENT(STOP_ROLL_CALL_TIMER,   		ANYCASE,   		CIvrSubRollCall::OnTimerStopRollCallRecordingAck)
PEND_MESSAGE_MAP(CIvrSubRollCall,CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubRollCall
////////////////////////////////////////////////////////////////////////////
CIvrSubRollCall::CIvrSubRollCall(const CIVRRollCallFeature* pRollCall)
{
	m_pRollCallFeature      = new CIVRRollCallFeature(*pRollCall);
	m_ArrayOfMessages       = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];
	m_ArrayOfMessagesToPlay = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];
	m_Messages_Number       = 0;
	m_TimerRollCall         = 0;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubRollCall::~CIvrSubRollCall()
{
	POBJDELETE(m_pRollCallFeature);

	m_pDtmf->SetRollCallFlag(0);

	if (m_TimerRollCall)
		DeleteTimer(END_ROLL_CALL);

	PDELETEA(m_ArrayOfMessages);
	PDELETEA(m_ArrayOfMessagesToPlay);
}

//--------------------------------------------------------------------------
void* CIvrSubRollCall::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
int CIvrSubRollCall::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::Start()
{
	PTRACE(eLevelInfoNormal, "CIvrSubRollCall::Start - F_IVR===>RollCall ");

	StartBaseTimer();

	if (!IsIvrPtrs())
		return;

	m_state          = ACTIVE;
	m_feature_opcode = IVR_FEATURE_ROLL_CALL;
	m_pDtmf->StartFeature();
	m_pDtmf->SetRollCallFlag(1);

	PlayRecordMessage(NULL);
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::OnDtmfPlayRecordMessage(CSegment* pParam)
{
	DeleteTimer(END_ROLL_CALL);

	if (m_TimerRollCall > 0)
		m_TimerRollCall--;

	DispatchEvent(RECORD_REQUEST, NULL);
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::OnDtmfStopRecordingByDtmf(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubRollCall::OnDtmfStopRecordingByDtmf - F_IVR===>RollCall ");
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::PlayRecordMessage(CSegment* pParam)
{
	WORD NumberOfMessagesToPlay = 4;       // (1)Record Roll Call; (2)Tone; (3)Dummy message for recording; (4)Tone

	// Initialize the m_ArrayOfMessagesToPlay (the array which will be sent during play message)
	for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
	{
		m_ArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
		m_ArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
		m_ArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
	}

	// Fill 1st message parameters (Record Roll Call message)
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);
	const char* ivrServiceName = m_pIvrService->GetName();

	int status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, m_feature_opcode, IVR_EVENT_ROLLCALL_REC,
	                                                    m_ArrayOfMessagesToPlay[0].ivrMsgFullPath, &msgDuration, &msgCheckSum);
	if (STATUS_OK != status)
	{
		m_ArrayOfMessagesToPlay[0].ivrMsgFullPath[0] = '\0';
		m_ArrayOfMessagesToPlay[0].ivrMsgDuration    = 0;
		m_ArrayOfMessagesToPlay[0].ivrMsgCheckSum    = (WORD)(-1);
		PTRACE(eLevelError, "CIvrSubRollCall::PlayRecordMessage - F_IVR===>RollCall, GetIVRMsgParams IVR_EVENT_ROLLCALL_REC failed");
	}
	else
	{
		m_ArrayOfMessagesToPlay[0].ivrMsgDuration = msgDuration;
		m_ArrayOfMessagesToPlay[0].ivrMsgCheckSum = msgCheckSum;
	}

	// Fill 2nd & 4th message parameters (Tone message)
	WORD duration = 0;
	WORD checksum = (WORD)(-1);

	status = ::GetpAVmsgServList()->GetFileParams(IVR_MSG_ROLL_CALL_TONE, &duration, &checksum);
	if (STATUS_OK != status)
	{
		m_ArrayOfMessagesToPlay[1].ivrMsgFullPath[0] = '\0';
		m_ArrayOfMessagesToPlay[1].ivrMsgDuration    = 0;
		m_ArrayOfMessagesToPlay[1].ivrMsgCheckSum    = (WORD)(-1);

		m_ArrayOfMessagesToPlay[3].ivrMsgFullPath[0] = '\0';
		m_ArrayOfMessagesToPlay[3].ivrMsgDuration    = 0;
		m_ArrayOfMessagesToPlay[3].ivrMsgCheckSum    = (WORD)(-1);

		PTRACE(eLevelError, "CIvrSubRollCall::PlayRecordMessage - F_IVR===>RollCall, GetIVRMsgParams IVR_MSG_ROLL_CALL_TONE failed");
	}
	else
	{
		// Tone message before recording roll call
		strcpy(m_ArrayOfMessagesToPlay[1].ivrMsgFullPath, IVR_MSG_ROLL_CALL_TONE);
		m_ArrayOfMessagesToPlay[1].ivrMsgDuration = (DWORD)duration;
		m_ArrayOfMessagesToPlay[1].ivrMsgCheckSum = (DWORD)checksum;

		strcpy(m_ArrayOfMessagesToPlay[3].ivrMsgFullPath, IVR_MSG_ROLL_CALL_TONE);
		m_ArrayOfMessagesToPlay[3].ivrMsgDuration = (DWORD)duration;
		m_ArrayOfMessagesToPlay[3].ivrMsgCheckSum = (DWORD)checksum;
	}

	// Fill 3rd message parameters (Dummy message for recording)
	strcpy(m_ArrayOfMessagesToPlay[2].ivrMsgFullPath, "ROLL_CALL");       // A string which indicates this is a roll call recording message
	m_ArrayOfMessagesToPlay[2].ivrMsgDuration = 2;                        // Roll call recording duration - anat - should be taken from system.cfg
	m_ArrayOfMessagesToPlay[2].ivrMsgCheckSum = 0;                        // Dummy checksum value for roll call recording message

	// gets IVR message cache priority
	WORD cachePriority = GetCachePriority(m_feature_opcode, m_event_opcode);

	TRACEINTO << "is tip call: " << m_pParty->GetIsTipCall() << " partyType: " << m_pParty->GetTipPartyType();
	if( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		TRACEINTO << "forward message to AUX - rollCall";
		NumberOfMessagesToPlay = 2; //only the 2 first files (play_message without record)
		CSegment* pParam = new CSegment();
		*pParam << m_ArrayOfMessagesToPlay << NumberOfMessagesToPlay << cachePriority ;
		m_pParty->ForwardEventToTipSlaves(pParam, PLAY_ROLL_CALL_AUX);
		m_state = RECORDING;
	}
	else
	{
		if (m_pConfApi)
		{
			// vngfe-7981 - stop message during roll call recording cause MFA crash
			m_bStopUponDTMF = 0;

			bool isRecordOnly = false;
			m_pConfApi->StartRecordMessage(m_pParty->GetPartyRsrcID(), m_ArrayOfMessagesToPlay, NumberOfMessagesToPlay, cachePriority, isRecordOnly);
		}

		m_state = RECORDING;


	}

	DWORD dwRecordingTimeout = 2000;
	StartTimer(END_ROLL_CALL, dwRecordingTimeout);
	m_TimerRollCall++;
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::PlayOnlyRecordMessageWithoutPlayMessage()
{
	TRACEINTO << "m_alreadyRecorded: " << m_alreadyRecorded;

	if(m_alreadyRecorded != true) //check if we need to recording the party name
	{
		WORD NumberOfMessagesToPlay = 1;       //Dummy message for recording

		// Initialize the m_ArrayOfMessagesToPlay (the array which will be sent during play message)
		for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
		{
			m_ArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
			m_ArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
			m_ArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
		}

		// Fill 3rd message parameters (Dummy message for recording)
		strcpy(m_ArrayOfMessagesToPlay[0].ivrMsgFullPath, "ROLL_CALL_TIP");       // A string which indicates this is a roll call recording message
		m_ArrayOfMessagesToPlay[0].ivrMsgDuration = 2;                        // Roll call recording duration - anat - should be taken from system.cfg
		m_ArrayOfMessagesToPlay[0].ivrMsgCheckSum = 0;                        // Dummy checksum value for roll call recording message

		// gets IVR message cache priority
		WORD cachePriority = GetCachePriority(m_feature_opcode, m_event_opcode);

		if (m_pConfApi)
		{
			bool isRecordOnly = true;
			m_pConfApi->StartRecordMessage(m_pParty->GetPartyRsrcID(), m_ArrayOfMessagesToPlay, NumberOfMessagesToPlay, cachePriority, isRecordOnly);
			TRACEINTO << "set m_alreadyRecorded to true";
			m_alreadyRecorded = true;
		}

		DWORD dwRecordingTimeout = 2000;
		StartTimer(END_ROLL_CALL, dwRecordingTimeout);
		m_TimerRollCall++;
	}
	//we got ack for the last message in the ROLL_CALL feature
	else
	{
		// if the card manager sent tone message ack and we already recorded, we delete the timer that ends the feature
		if (m_TimerRollCall)
		{
			DeleteTimer(END_ROLL_CALL);
			m_TimerRollCall--;
		}

		m_state = ACTIVE;
		m_alreadyRecorded = false;

		EndFeature(0);
		m_ivrCntl->SetStartNextFeature();
	}
}
//--------------------------------------------------------------------------
void CIvrSubRollCall::OnPartyMessageRecorded(CSegment* pParam)
{
	TRACEINTO << "is tip call: " << m_pParty->GetIsTipCall() << " partyType: " << m_pParty->GetTipPartyType();
	//if we in TIP call we should send to AUX play_message req with the 4 file (after master was recorded)
	if( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		WORD NumberOfMessagesToPlay = 1;       // Tone

		// Initialize the m_ArrayOfMessagesToPlay (the array which will be sent during play message)
		for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
		{
			m_ArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
			m_ArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
			m_ArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
		}

		// Fill message parameters (Tone message)
		WORD duration = 0;
		WORD checksum = (WORD)(-1);

		int status = ::GetpAVmsgServList()->GetFileParams(IVR_MSG_ROLL_CALL_TONE, &duration, &checksum);
		if (STATUS_OK != status)
		{
			m_ArrayOfMessagesToPlay[0].ivrMsgFullPath[0] = '\0';
			m_ArrayOfMessagesToPlay[0].ivrMsgDuration    = 0;
			m_ArrayOfMessagesToPlay[0].ivrMsgCheckSum    = (WORD)(-1);

			PTRACE(eLevelError, "CIvrSubRollCall::PlayRecordMessage - F_IVR===>RollCall, GetIVRMsgParams IVR_MSG_ROLL_CALL_TONE failed");
		}
		else
		{
			strcpy(m_ArrayOfMessagesToPlay[0].ivrMsgFullPath, IVR_MSG_ROLL_CALL_TONE);
			m_ArrayOfMessagesToPlay[0].ivrMsgDuration = (DWORD)duration;
			m_ArrayOfMessagesToPlay[0].ivrMsgCheckSum = (DWORD)checksum;
		}

		// gets IVR message cache priority
		WORD cachePriority = GetCachePriority(m_feature_opcode, m_event_opcode);

		TRACEINTO << "forward message to AUX - rollCall tone message";
		CSegment* pParam = new CSegment();
		*pParam << m_ArrayOfMessagesToPlay << NumberOfMessagesToPlay << cachePriority ;
		m_pParty->ForwardEventToTipSlaves(pParam, PLAY_ROLL_CALL_AUX);
	}

	//in TIP we wait for the tone message ack and not only for the record indication
	//and in not TIP we wait card manager to send record indication only
	else
	{
		// if the card manager sent record indication we delete the timer that ends the feature
		if (m_TimerRollCall)
		{
			DeleteTimer(END_ROLL_CALL);
			m_TimerRollCall--;
		}

		m_state = ACTIVE;

		EndFeature(0);
		m_ivrCntl->SetStartNextFeature();
	}
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::OnTimerEndRollCallActive(CSegment* pParam)
{
	EndFeature(0);
	if (m_TimerRollCall > 0)
		m_TimerRollCall--;

	m_ivrCntl->SetStartNextFeature();
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::OnTimerEndRollCallRecording(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubRollCall::OnTimerEndRollCallRecording - F_IVR===>RollCall, Record indication was not received", m_pParty->GetName());

	m_state = ACTIVE;
	EndFeature(0);

	if (m_TimerRollCall > 0)
		m_TimerRollCall--;

	m_ivrCntl->SetStartNextFeature();
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::PlayRetryMessage(DWORD messageMode)
{
}

//--------------------------------------------------------------------------
void CIvrSubRollCall::OnDtmfStringIdentNull(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubRollCall::OnDtmfStringIdentNull : Null Function:", NameOf());
}

// vngfe-8707
//--------------------------------------------------------------------------
void CIvrSubRollCall::OnCamStopRollCallRecordingRECORDING(CSegment* pParam)
{
	TRACEINTO << " ROLL_CALL_STOP_RECORDING (02) partyRsrcID = " << m_pParty->GetPartyRsrcID() << " start STOP_ROLL_CALL_TIMER timer (party)";
	StopRecording(pParam);
}
//--------------------------------------------------------------------------
void CIvrSubRollCall::OnCamStopRollCallRecordingACTIVE(CSegment* pParam)
{
	TRACEINTO << " ROLL_CALL_STOP_RECORDING (02) partyRsrcID = " << m_pParty->GetPartyRsrcID() << " no active roll call recording (party)";
	m_pConfApi->StopRollCallRecording(m_pParty->GetPartyRsrcID(),STATUS_FAIL);

}
//--------------------------------------------------------------------------
void CIvrSubRollCall::StopRecording(CSegment* pParam)
{
	DeleteTimer(END_ROLL_CALL);
	m_TimerRollCall--;
	StartTimer(STOP_ROLL_CALL_TIMER,2*SECOND);
	m_pConfApi->StopRollCallRecording(m_pParty->GetPartyRsrcID(),STATUS_OK);
}
//--------------------------------------------------------------------------
void CIvrSubRollCall::OnCamStopRollCallRecordingAck(CSegment* pParam)
{
	TRACEINTO << " ROLL_CALL_STOP_RECORDING_ACK (04) EndFeature, partyRsrcID = " << m_pParty->GetPartyRsrcID();

	DeleteTimer(STOP_ROLL_CALL_TIMER);
	EndFeature(0);
	m_ivrCntl->SetStartNextFeature();

}
//--------------------------------------------------------------------------
void CIvrSubRollCall::OnTimerStopRollCallRecordingAck(CSegment* pParam)
{
	TRACEINTO << " ROLL_CALL_STOP_RECORDING_ACK (04) , partyRsrcID = " << m_pParty->GetPartyRsrcID();
	// inform CAM
	m_pConfApi->StopRollCallRecordingAckTimer(m_pParty->GetPartyRsrcID());
	//EndFeature
	EndFeature(0);
	m_ivrCntl->SetStartNextFeature();
}

//--------------------------------------------------------------------------



const WORD ST_CONNECT = 5;


PBEGIN_MESSAGE_MAP(CIvrSubDialOutToRemote)
	ONEVENT(FIRST_DTMF_WAS_DIALED, ST_CONNECT, CIvrSubDialOutToRemote::OnDTMFNumberReceivedCOLLECT)
	ONEVENT(TIMER_WAIT_FOR_DTMF,   ST_CONNECT, CIvrSubDialOutToRemote::OnTimerDTMF)
PEND_MESSAGE_MAP(CIvrSubDialOutToRemote, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CTokenUser
////////////////////////////////////////////////////////////////////////////
CIvrSubDialOutToRemote::CIvrSubDialOutToRemote(void)
{
	m_endDone = 0;
}

//--------------------------------------------------------------------------
CIvrSubDialOutToRemote::~CIvrSubDialOutToRemote()
{
}

//--------------------------------------------------------------------------
void* CIvrSubDialOutToRemote::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
int CIvrSubDialOutToRemote::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}

//--------------------------------------------------------------------------
void CIvrSubDialOutToRemote::Start()
{
	m_state = ST_CONNECT;
	// checking if the DTMF already pressed
	if (m_pDtmf->GetDtmfIndex() > 0)
	{
		WORD opcode  = OP_NOOP;
		WORD rDigits = 1;
		const char* CDtmfCollector = m_pDtmf->GetDtmfBuffer();
		m_dtmf[0] = CDtmfCollector[0];
		FindAndSendDigit(opcode, rDigits);
	}
	else
	{
		// we don't have the DTMF yet
		m_pDtmf->InformAboutFirstNumbPressed();

		StartBaseTimer(); // start timer capability (InitTimer)
		StartTimer(TIMER_WAIT_FOR_DTMF, 15 * SECOND);
	}
}

//--------------------------------------------------------------------------
void CIvrSubDialOutToRemote::PlayRetryMessage(DWORD messageMode)
{
}

//--------------------------------------------------------------------------
void CIvrSubDialOutToRemote::OnDTMFNumberReceivedCOLLECT(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubDialOutToRemote::OnDTMFNumberReceivedCOLLECT");

	if (m_endDone)  // normally not happen
		return;

	DeleteTimer(TIMER_WAIT_FOR_DTMF);

	WORD opcode, rDigits;
	GetDtmfResult(pParam, opcode, rDigits);
	FindAndSendDigit(opcode, rDigits);
}

//--------------------------------------------------------------------------
void CIvrSubDialOutToRemote::FindAndSendDigit(WORD opcode, WORD rDigits)
{
	if (opcode == OP_NOOP && rDigits - 1 <= PHONE_NUMBER_DIGITS_LEN)
	{
		BYTE bPartysNumber = 120;
		if (m_dtmf[0] > '0' && m_dtmf[0] <= '9')
		{
			bPartysNumber = m_dtmf[0] - '0';
		}
		else if (m_dtmf[0] == '*')
		{
			bPartysNumber = 0xFF;
		}
		else if (m_dtmf[0] == '#')
		{
			bPartysNumber = 0xFE;
		}

		if (bPartysNumber != 120)
			m_pConfApi->ConnectDialOutPartyByNumb(bPartysNumber);
	}

	m_pDtmf->ResetDtmfBuffer();
	m_pDtmf->EndFeature();
	m_endDone = 1;
	EndFeature(0); // end OK
}

//--------------------------------------------------------------------------
void CIvrSubDialOutToRemote::OnTimerDTMF(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubDialOutToRemote::OnTimerDTMF  ");

	if (m_endDone)  // normally not happen
		return;

	m_pDtmf->ResetDtmfBuffer();
	m_pDtmf->EndFeature();
	m_endDone = 1;
	EndFeature(0); // end OK
}


PBEGIN_MESSAGE_MAP(CIvrSubNumericConferenceId)
	ONEVENT(DTMF_STRING_IDENT,        NOTACTIVE,   CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT,        ACTIVE,      CIvrSubBaseSM::OnDtmfString)
	ONEVENT(TIMER_RETRY_MESSAGE,      ACTIVE,      CIvrSubBaseSM::OnTimeRetryMessage)
	ONEVENT(EXT_DB_RESPONSE_TOUT,     WAIT_FOR_DB, CIvrSubNumericConferenceId::OnTimeNoExtDBResponse)
	ONEVENT(EXT_DB_CONF_CONFIRM,      WAIT_FOR_DB, CIvrSubNumericConferenceId::OnExtDBResponse)
	ONEVENT(EXT_DB_REQUEST_FAILED,    WAIT_FOR_DB, CIvrSubNumericConferenceId::OnExtDBFailure)
	ONEVENT(SIP_CONF_NID_CONFIRM_IND, WAIT_FOR_DB, CIvrSubNumericConferenceId::OnSIPconfNIDConfirmInd)   // IvrProviderEQ
	ONEVENT(EXTDB_IVR_PROV_TIMER,     WAIT_FOR_DB, CIvrSubBaseSM::OnTimerDiscIvrProviderExtDB)           // IvrProviderEQ
PEND_MESSAGE_MAP(CIvrSubNumericConferenceId, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubNumericConferenceId
////////////////////////////////////////////////////////////////////////////
CIvrSubNumericConferenceId::CIvrSubNumericConferenceId(const CIVRNumericConferenceIdFeature* pNumericConferenceIdFeature, const char* pNumericId, BOOL bEnableExternalDB)
{
	if (pNumericId)
	{
		strncpy(m_pNumericConferenceid, pNumericId, sizeof(m_pNumericConferenceid) - 1);
		m_pNumericConferenceid[sizeof(m_pNumericConferenceid) - 1] = '\0';
	}
	else
		m_pNumericConferenceid[0] = 0;

	m_pNumericConferenceIdFeature = new CIVRNumericConferenceIdFeature(*pNumericConferenceIdFeature);

	m_ArrayOfMessages = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];
	m_Msg_Files_Name  = new char*[MAX_SUB_MESSAGE_NUMBER];

	for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
		m_Msg_Files_Name[i] = NULL;

	m_Messages_Number   = 0;
	m_useExternalDB     = IVR_EXTERNAL_DB_NONE;
	m_bEnableExternalDB = bEnableExternalDB;


	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubNumericConferenceId::~CIvrSubNumericConferenceId()
{
	POBJDELETE(m_pNumericConferenceIdFeature);

	PDELETEA(m_ArrayOfMessages);

	if (m_Msg_Files_Name)
	{
		for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
			PDELETEA(m_Msg_Files_Name[i]);

		PDELETEA(m_Msg_Files_Name);
	}
}

//--------------------------------------------------------------------------
void* CIvrSubNumericConferenceId::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubNumericConferenceId::Start()
{
	PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::Start: F_IVR===>NID", m_pParty->GetName());

	StartBaseTimer();           // start timer capability (InitTimer)

	if (!IsIvrPtrs())
		return;

	m_DTMF_digits  = 0;
	m_DTMF_timeout = m_pIvrService->GetUserInputTimeout();
	if (m_DTMF_timeout == 0)
		m_DTMF_timeout = 1;

	m_pinCodeDelimiter = GetIvrDelimiter('#');
	m_state            = ACTIVE;
	m_feature_opcode   = IVR_FEATURE_NUMERIC_CONFERENCE_ID;
	m_bStopUponDTMF    = TRUE;
	m_pDtmf->StartFeature();

	const char* file_name = NULL;

	int status = STATUS_OK;

	m_Messages_Number = 2;           // 2 - conference id only

	const CIVRNumericConferenceIdFeature* NumericConferenceIdMsgs = m_pIvrService->GetNumericConferenceIdFeature();

	CIVRLanguage* pIVRLanguage = m_pIvrService->GetFirstLanguage();

	int i = 0;
	for (i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
	{
		m_ArrayOfMessages[i].ivrMsgFullPath[0] = '\0';
		m_ArrayOfMessages[i].ivrMsgDuration    = 0;
		m_ArrayOfMessages[i].ivrMsgCheckSum    = (WORD)(-1);
	}

	// get files parameter
	for (i = 0; i < m_Messages_Number; i++)
	{
		switch (i)
		{
			case (0):
			{
				m_event_opcode = IVR_EVENT_GET_NUMERIC_ID;
				break;
			}

			case (1):
			{
				m_event_opcode = IVR_EVENT_NUMERIC_ID_RETRY;
				break;
			}

			default:
			{
				m_event_opcode = (WORD)(-1);
				break;
			}
		} // switch

		if (m_event_opcode == (WORD)(-1))
		{
			PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::Start - Illegal Message opcode   F_IVR===>NID ");
		}
		else
			file_name = NumericConferenceIdMsgs->GetMsgFileName(m_event_opcode, m_language, status);

		if (file_name && strcmp(file_name, ""))
		{
			m_Msg_Files_Name[i] = new char[NEW_FILE_NAME_LEN];
			strncpy(m_Msg_Files_Name[i], file_name, NEW_FILE_NAME_LEN);
			PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::Start -  F_IVR===>NID   filename: ", file_name);
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::Start - Error getting filename  F_IVR===>NID ");
			break;
		}

		WORD msgDuration = 0;
		WORD msgCheckSum = (WORD)(-1);

		int status = GetIVRMessageParameters(m_ArrayOfMessages[i].ivrMsgFullPath, &msgDuration, &msgCheckSum);
		if (STATUS_OK != status)
		{
			m_ArrayOfMessages[i].ivrMsgFullPath[0] = '\0';
			m_ArrayOfMessages[i].ivrMsgDuration    = 0;
			m_ArrayOfMessages[i].ivrMsgCheckSum    = (WORD)(-1);
			PTRACE(eLevelError, "CIvrSubNumericConferenceId::Start - GetIVRMsgParams failed  F_IVR===>NID ");
		}
		else
		{
			m_ArrayOfMessages[i].ivrMsgDuration = msgDuration;
			m_ArrayOfMessages[i].ivrMsgCheckSum = msgCheckSum;
			PTRACE(eLevelError, "CIvrSubNumericConferenceId::Start - GetIVRMsgParams okay  F_IVR===>NID ");
		}
	}

	m_event_opcode = IVR_EVENT_GET_NUMERIC_ID;

	if (m_Msg_Files_Name[0])
	{
		strncpy(m_msg_file_name, m_Msg_Files_Name[0], sizeof(m_msg_file_name) - 1);
		m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
		m_messagesName = m_msg_file_name;           // to find upon the message entry index
	}

	// message duration
	m_msgDuration = m_ArrayOfMessages[0].ivrMsgDuration;

	if (m_msgDuration < 2)
		m_msgDuration = 5;

	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
		m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	m_useExternalDB = IVR_EXTERNAL_DB_NONE;
	if (m_bEnableExternalDB)
		m_useExternalDB = (BYTE)m_pIvrService->GetIvrExternalDB();

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	if (!pCommConf)
	{
		PASSERT(2);
		PTRACE(eLevelError, "CIvrSubNumericConferenceId::Start - pCommConf=NULL  F_IVR===>NID ");
		return;
	}

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

	if (!pConfParty)
	{
		PASSERT(3);
		PTRACE(eLevelError, "CIvrSubNumericConferenceId::Start -  pConfParty=NULL  F_IVR===>NID ");
		return;
	}

	const char* szPreDefinedIvrString = pConfParty->GetPreDefinedIvrString();

	if (strncmp(szPreDefinedIvrString, "", DTMF_MAX_BUFFER_SIZE))
	{
		PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::Start - Start-szPreDefinedIvrString F_IVR===>NID");
		ParsePreDefinedIvrString(szPreDefinedIvrString);
	}

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key       = CFG_KEY_SKIP_NID;
	BOOL bSkipNid;
	sysConfig->GetBOOLDataByKey(key, bSkipNid);

	if (bSkipNid && (0 == strncmp(pCommConf->GetName(), "SKIP_NID", 8)))
	{
		PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::Start - PSTN SKIP NID - Automatically moves to conference with NID 4444!!! F_IVR===>NID");
		ParsePreDefinedIvrString("##4444");
	}

	StartBase(FALSE, IVR_STATUS_PLAY_ONCE);
}

//--------------------------------------------------------------------------
void CIvrSubNumericConferenceId::PlayRetryMessage(DWORD messageMode)
{
	m_retryMessageDuration = m_ArrayOfMessages[1].ivrMsgDuration;
	int status = STATUS_OK;
	const char* file_name = m_pNumericConferenceIdFeature->GetMsgFileName(IVR_EVENT_NUMERIC_ID_RETRY, m_language, status);

	if (file_name && !strcmp(file_name, ""))
	{
		if (m_Msg_Files_Name[1])
		{
			strncpy(m_msg_file_name, m_Msg_Files_Name[1], sizeof(m_msg_file_name) - 1);
			m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
			m_messagesName = m_msg_file_name;           // to find upon the message entry index
		}

		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::PlayRetryMessage  F_IVR===>NID Message Name: ", m_messagesName);
		PlayOtherMessage(messageMode, IVR_EVENT_CONFERENCE_PASSWORD_RETRY, m_messagesName);
	}
	else
	{
		PTRACE(eLevelError, "CIvrSubNumericConferenceId::PlayRetryMessage: filename=NULL  F_IVR===>NID");
		PlayOtherMessage(messageMode, IVR_EVENT_NUMERIC_ID_RETRY, file_name);
	}
}

//--------------------------------------------------------------------------
DWORD CIvrSubNumericConferenceId::FindConfAccordingToNid(const char* szNid)
{
	DWORD wResult = 0xFFFFFFFF;

	if (!szNid || szNid[0] == '\0')           // empty string
		return wResult;

	CCommConfDB* pConfDB   = ::GetpConfDB();
	CCommConf*   pCommConf = pConfDB->GetFirstCommConf();

	// Search on going conference database
	while (pCommConf != NULL)
	{
		if (pCommConf->GetMeetMePerEntryQ() && !pCommConf->GetEntryQ())
		{
			if (pCommConf->GetNumericConfId() != NULL)
			{
				if (!strncmp(szNid, pCommConf->GetNumericConfId(), NUMERIC_CONFERENCE_ID_LEN))
				{
					if (pCommConf->IsDefinedIVRService())
					{
						PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::FindConfAccordingToNid - F_IVR===>NID found Conf, party = ", PARTYNAME);
						wResult = pCommConf->GetMonitorConfId();
						break;
					}
					else
					{
						PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::FindConfAccordingToNid - Destination Conf has no IVR F_IVR===>NID");
						return 0xFFFFFFFF;           // If the conference has no IVR, return 0xFFFFFFFF
					}
				}
			}
		}

		pCommConf = pConfDB->GetNextCommConf();
	}

	return wResult;
}

//--------------------------------------------------------------------------
DWORD CIvrSubNumericConferenceId::FindMRAccordingToNid(const char* szNid, WORD& nidOfEqOrSipFactory)
{
	DWORD wResult = 0xFFFFFFFF;
	nidOfEqOrSipFactory = 0;

	if (!szNid || szNid[0] == '\0')           // empty string
		return wResult;

	// Search the meeting room database

	CCommResDB* pMeetingRoomDB = ::GetpMeetingRoomDB();

	if (pMeetingRoomDB)
	{
		const ReservArray& rsrvArray = pMeetingRoomDB->GetReservArray();
		ReservArray::const_iterator iter_end = rsrvArray.end();
		for (ReservArray::const_iterator iter = rsrvArray.begin(); iter != iter_end; ++iter)
		{
			if (NULL != (*iter))
			{
				if ((*iter)->GetNumericConfId() != NULL)
				{
					if (!strncmp(szNid, (*iter)->GetNumericConfId(), NUMERIC_CONFERENCE_ID_LEN))
					{
						if (!(*iter)->IsEntryQ() && !(*iter)->IsSIPFactory())
						{
							DWORD tempWResult = (*iter)->GetConferenceId();
							CCommRes* pCommRes = pMeetingRoomDB->GetCurrentRsrv(tempWResult);           // Allocation within GetCurrentRsrv function
							if (pCommRes)
							{
								PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::FindMRAccordingToNid - F_IVR===>NID found MR, party = ", PARTYNAME);
								wResult = tempWResult;
								POBJDELETE(pCommRes);
								break;
							}
							else
							{
								PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::FindMRAccordingToNid - pCommRes of Destination MR is NULL, F_IVR===>NID");
								return 0xFFFFFFFF;           // If pCommRes is NULL, return 0xFFFFFFFF
							}
						}
						else                             // if the MR that was found is an EQ or a SIP Factory
						{
							PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::FindMRAccordingToNid - Numeric ID was found in EQ or SIP Factory, F_IVR===>NID");
							nidOfEqOrSipFactory = 1;
						}
					}
				}
			}
		}
	}

	return wResult;
}

//--------------------------------------------------------------------------
int CIvrSubNumericConferenceId::IsLegalString(CSegment* pParam)
{
	// from Carmel The numeric ID works in one way:
	// 1:	Searching for NID in on-going conference or MR: if found then move it to there
	// 2:	If External-DB then keep with external-DB ...
	// 3:	If Ad-Hoc then creates a new conf from Templete and then move it to there

	PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::IsLegalString - F_IVR ===>> NID  partyname = ", PARTYNAME);

	// gets the NID string
	WORD opcode;
	WORD rDigits;
	BYTE IsAdHocGW = FALSE;
	BYTE IsGW      = FALSE;

	if (0 != GetDtmfResult(pParam, opcode, rDigits))
	{
		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString - Error  F_IVR===>NID  ", PARTYNAME);
		if (rDigits == 0)                   // no digit in the string
			return DTMF_TIMEOUT_ERROR;

		return DTMF_STRING_ERROR;           // opcode error or string length error
	}

	BOOL isHidePsw = NO;
	std::string key_hide  = "HIDE_CONFERENCE_PASSWORD";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);

	ALLOCBUFFER(tmp, 2*H243_NAME_LEN+ONE_LINE_BUFFER_LEN);
	if (!isHidePsw)
		sprintf(tmp, "[%s] - user entered NID [%s].   F_IVR===>NID ", PARTYNAME, m_dtmf);
	else
		sprintf(tmp, "[%s] - user entered NID [%s].   F_IVR===>NID ", PARTYNAME, "********");

	PTRACE2(eLevelInfoNormal, " ---> ", tmp);
	DEALLOCBUFFER(tmp);

	if (!m_pParty)
	{
		PASSERT(1);
		PTRACE(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: Illegal Party Address  F_IVR===>NID ");
		return DTMF_STRING_ERROR;
	}

	const char* partyName = m_pParty->GetName();

	int UserNumericIdLen = strlen((const char*)m_dtmf);
	if ((0 == UserNumericIdLen) || (0 == rDigits))           // the user entered ("#") only without digits
	{
		PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::IsLegalString: Error: Empty DTMF string F_IVR===>NID name=", partyName);
		return DTMF_STRING_ERROR;
	}

	// Check validity of characters in string - only digits are allowed
	if (!IsNumeric((const char*)m_dtmf))
	{
		PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::IsLegalString: Error: DTMF string is not numeric F_IVR===>NID name=", partyName);
		return DTMF_STRING_ERROR;
	}

	CCommConf* pEqCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	if (!pEqCommConf)
	{
		PASSERT(2);
		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: NULL pointer  F_IVR===>NID ", partyName);
		return DTMF_STRING_ERROR;
	}

	CConfParty* pConfParty = pEqCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
	if (!pConfParty)
	{
		PASSERT(3);
		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: NULL pointer  F_IVR===>NID ", partyName);
		return DTMF_STRING_ERROR;
	}

	if (opcode != OP_NOOP)           // string was requested
	{
		PASSERT(4);
		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: Error: opcode != OP_NOOP  F_IVR===>NID ", partyName);
		return DTMF_STRING_ERROR;
	}

	if (!m_pIvrService)
	{
		PASSERT(5);
		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: Error: m_pIvrService=NULL  F_IVR===>NID ", partyName);
		return DTMF_STRING_ERROR;
	}

	if (!IsGW && !m_pIvrService->m_bIsEntryQueueService)
	{
		PASSERT(6);
		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: the service is not eq, F_IVR===>NID - ", partyName);
		return DTMF_STRING_ERROR;
	}

	// alloc buffer for string error
	ALLOCBUFFER(str, 120); AUTO_DELETE_ARRAY(str);                                                             // trace buffer
	str[0] = 0;
	WORD returnValue = DTMF_OK;
	CConfPartyManagerLocalApi confPartyManagerLocalApi;                                 // (CConfPartyManagerLocalApi*)CProcessBase::GetProcess()->GetManagerApi();
	int status = STATUS_OK;

	DWORD dwSourceConfId = m_ivrCntl->GetMonitorConfId();           // current EQ conf-ID
	ETargetConfType targetConfType = eDummyType;

	DWORD dwPartId  = m_pParty->GetMonitorPartyId();                // party-ID
	DWORD dwConf_id = 0xFFFFFFFF;

	// In AdHocGW - don't search the numericId in ongoing conf and in external db
	// IvrProviderEQ - won't do the search in DB

	if (IsAdHocGW != TRUE && !m_ivrCntl->IsIvrProviderEQPartyInConf())
	{
		if (pConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE && pConfParty->GetVoice())
		{
			// 1. For PSTN party: system.cfg flag (CFG_KEY_USE_GK_PREFIX_FOR_PSTN_CALLS) should be set to YES
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			std::string key = CFG_KEY_USE_GK_PREFIX_FOR_PSTN_CALLS;
			BOOL bUseGKPrefixForPstnCalls = FALSE;
			sysConfig->GetBOOLDataByKey(key, bUseGKPrefixForPstnCalls);
			if (bUseGKPrefixForPstnCalls)
			{
				const char* newDtmf;
				newDtmf = ::GetIpServiceListMngr()->FindServiceAndGetStringWithoutPrefix((const char*)m_dtmf, PARTY_H323_ALIAS_H323_ID_TYPE);
				if (newDtmf != NULL)
				{
					TRACEINTO << "CIvrSubNumericConferenceId::IsLegalString: Numeric Id is extracted from entire DTMF! Whole DTMF: " << m_dtmf << " Actual Numeric ID: "<< newDtmf << " - Party Name: "<< PARTYNAME;
					BYTE revisedDtmf[MAX_DTMF_STRING_LENGTH+1];
					strncpy((char*)revisedDtmf, newDtmf, strlen(newDtmf)+1);
					memset((char*)m_dtmf, '\0', MAX_DTMF_STRING_LENGTH+1);
					strncpy((char*)m_dtmf, (char*)revisedDtmf, strlen((char*)revisedDtmf)+1);
				}
			}
		}

		// 2. Check if the Numric ID matches one of the On-Going conferences
		dwConf_id = FindConfAccordingToNid((const char*)m_dtmf);
		if (0xFFFFFFFF != dwConf_id)           // Numeric ID found in an on-going conference
		{
			targetConfType = eOnGoingConf;
			if (!isHidePsw)
				sprintf(str, "Numeric id was found in on-going conf (%s), target-conf-ID: %d", m_dtmf, dwConf_id);
			else
				sprintf(str, "Numeric id was found in on-going conf (%s), target-conf-ID: %d", "********", dwConf_id);
		}
		else                                     // Numeric ID was NOT found in an on-going conference, trying in MR
		{
			WORD nidOfEqOrSipFactory = 0;
			dwConf_id = FindMRAccordingToNid((const char*)m_dtmf, nidOfEqOrSipFactory);
			if (dwConf_id != 0xFFFFFFFF)           // Numeric ID was found in a Meeting Room
			{
				targetConfType = eMeetingRoom;
				sprintf(str, "Numeric id was found in MR (%s), target-conf-ID: %d", m_dtmf, dwConf_id);
			}
			else if (1 == nidOfEqOrSipFactory)
			{
				PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::IsLegalString: Numeric id was found in EQ or SIP Factory, request is denied!! F_IVR===>NID ", partyName);
				return DTMF_STRING_ERROR;
			}
		}
	}

	// if we have found the conf id (on-going or MR) then we move the Party to there
	if (dwConf_id != 0xFFFFFFFF)           // Numeric ID was found in an on-going or MR conference
	{
		// move party to its destination conference
		if (targetConfType == eOnGoingConf)
		{
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(dwConf_id);
			if (pCommConf)
			{
				CConfApi* pDestConfApi = new CConfApi;
				pDestConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()), NULL, NULL, 1);
				STATUS    rspStatus = STATUS_OK;
				OPCODE    rspOpcode = STATUS_OK;
				CSegment  rspMsg;
				rspStatus = pDestConfApi->IsDestConfReadyForMove(rspMsg, 5*SECOND, rspOpcode);
				pDestConfApi->DestroyOnlyApi();
				POBJDELETE(pDestConfApi);

				if ((rspStatus != STATUS_OK) || (rspOpcode != STATUS_OK))
				{
					PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: Error: Conf is not ready ", pCommConf->GetName());
					return TOO_MANY_ERRORS;           // disconnect the Party
				}
			}
		}

		// inform CAM on start Move
		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: Party Name: ", m_pParty->GetName());
		char*     partyName = (char*)(m_pParty->GetName());
		CSegment* pSeg      = new CSegment;
		*pSeg << partyName;
		m_pConfApi->SendCAMGeneralNotifyCommand((DWORD)0, IVR_PARTY_START_MOVE_FROM_EQ, pSeg);
		POBJDELETE(pSeg);

		// move party to its destination conference
		confPartyManagerLocalApi.MovePartyToConfOrMeetingRoom(dwSourceConfId, dwPartId, targetConfType, dwConf_id, NULL);
	}
	else // Numeric id was not found in any on-going conference or MR
	{
		// 2.  checks if External-DB is configure in this EQ
		if ((IVR_EXTERNAL_DB_NUMERIC_ID == m_useExternalDB || m_ivrCntl->IsIvrProviderEQPartyInConf()) && IsAdHocGW != TRUE)           // if External-DB configure in EQ
		{
			// Check NumericId with  external DB
			PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::IsLegalString, accessing external DB, F_IVR===>NID, party = ", m_pParty->GetName());
			Phone* pPhone1 = NULL;
			Phone* pPhone2 = NULL;
			Phone* pPhone3 = NULL;           // rons
			DWORD  partyId = pConfParty->GetPartyId();

			std::string protocolTypeStr = "H.320";
			IP_EXT_DB_STRINGS ipStringsStruct;
			memset((char*)&ipStringsStruct, 0, sizeof(IP_EXT_DB_STRINGS));

			BYTE interfaceType = pConfParty->GetNetInterfaceType();
			switch (interfaceType)
			{
				case SIP_INTERFACE_TYPE:
				{
					FillIpIdentifiers(pConfParty, &ipStringsStruct);
					protocolTypeStr = "SIP";
					break;
				}

				case H323_INTERFACE_TYPE:
				{
					FillIpIdentifiers(pConfParty, &ipStringsStruct);
					protocolTypeStr = "H.323";
					break;
				}

				case ISDN_INTERFACE_TYPE:
				{
					if (m_ivrCntl->IsPSTNCall())
						protocolTypeStr = "PSTN";
					else
						protocolTypeStr = "H.320";

					pPhone1 = pConfParty->GetActualPartyPhoneNumber(0);
					pPhone2 = pConfParty->GetActualPartyPhoneNumber(1);
					break;
				}

				default:
					PTRACE2(eLevelError, "CIvrSubNumericConferenceId::IsLegalString -  Illegal InterfaceType F_IVR===>ConfPW", m_pParty->GetName());
					break;
			} // switch

			strncpy(m_pNumericConferenceid, (const char*)m_dtmf, sizeof(m_pNumericConferenceid) - 1);
			m_pNumericConferenceid[sizeof(m_pNumericConferenceid) - 1] = '\0';

			// call ReqConfNIDConfirmationSIP + timer
			if (interfaceType == SIP_INTERFACE_TYPE && m_ivrCntl->IsIvrProviderEQPartyInConf())
			{
				TRACEINTO << "CIvrSubNumericConferenceId::IsLegalString - IvrProviderEQ send req to SIP";
				ReqConfNIDConfirmationSIP(m_pNumericConferenceid);

				CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
				if (pSysConfig)
				{
					DWORD timeout_in_sec = 0;
					pSysConfig->GetDWORDDataByKey(CFG_KEY_EXT_DB_IVR_PROV_TIME_SECONDS, timeout_in_sec);
					TRACEINTO<<"CIvrSubNumericConferenceId::IsLegalString - START TIMER for "<<timeout_in_sec<<" seconds - IvrProviderEQ";
					m_state = WAIT_FOR_DB;
					StartTimer(EXTDB_IVR_PROV_TIMER, timeout_in_sec*SECOND);
				}
				else
				{
					PTRACE(eLevelError, "CIvrSubNumericConferenceId::IsLegalString - CSysConfig - Null pointer.");
				}
			}
			else
			{
				// sends request to External-DB
				ReqPwdConfirmation(EXDB_TYPE_ConfNID, m_pNumericConferenceid, partyId, NULL, pPhone1, pPhone2, 0, 0, /*protocolType*/ protocolTypeStr.c_str(), &ipStringsStruct);
				m_pDtmf->ResetDtmfParams();           // Delete DTMF collector timers
				m_state = WAIT_FOR_DB;
				StartTimer(EXT_DB_RESPONSE_TOUT, TIME_EXT_DB_TIMEOUT*SECOND);
			}

			returnValue = DTMF_IGNORE_ERROR;
		}
		else           // External-DB not configure in EQ, checks for AD-Hoc option
		{
			// 3. Check if it is an Ad-Hoc EQ
			if (pEqCommConf->GetAdHoc())
			{
				// Get min Numeric ID length from system.cfg
				CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				std::string key       = CFG_KEY_NUMERIC_CONF_ID_MIN_LEN;
				DWORD       dwCfgUserNumericIdLenMin;
				sysConfig->GetDWORDDataByKey(key, dwCfgUserNumericIdLenMin);

				// Get max Numeric ID length from system.cfg
				key = CFG_KEY_NUMERIC_CONF_ID_MAX_LEN;
				DWORD dwCfgUserNumericIdLenMax;
				sysConfig->GetDWORDDataByKey(key, dwCfgUserNumericIdLenMax);

				// Check if the Numeric ID length is legal (according to system.cfg file)
				if (((DWORD)UserNumericIdLen < dwCfgUserNumericIdLenMin) || ((DWORD)UserNumericIdLen > dwCfgUserNumericIdLenMax))
				{
					PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::IsLegalString: F_IVR===>NID Illegal numeric id length: ", (const char*)m_dtmf);
					CdrPartyDtmfFailureIndication("Unknown");
					returnValue = DTMF_STRING_ERROR;
				}

				if (DTMF_OK == returnValue)
				{
					// Get Ad-Hoc profile ID
					DWORD     dwAdHocProfileId = pEqCommConf->GetAdHocProfileId();
					CCommRes* pConfProfile     = NULL;

					// If the Ad-Hoc is checked and no profile is configured return error
					if (dwAdHocProfileId == 0xFFFFFFFF)
					{
						PTRACE(eLevelError, "CIvrSubNumericConferenceId::IsLegalString - dwAdHocProfileId=NULL ");
						returnValue = DTMF_STRING_ERROR;
					}
					else
					{
						// Get Profile according to Ad-Hoc Profile ID
						pConfProfile = ::GetpProfilesDB()->GetCurrentRsrv(dwAdHocProfileId);           // get & allocate
						if (!pConfProfile)
						{
							PASSERT(4);
							PTRACE(eLevelError, "CIvrSubNumericConferenceId::IsLegalString: NULL pointer - No profile for Ad-Hoc conf");
							returnValue = DTMF_STRING_ERROR;
						}
					}

					if (DTMF_OK == returnValue)
					{
						PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::IsLegalString - Creating Ad-Hoc conf - ", m_pParty->GetName());
						// Create the Ad Hoc confernce name - "Numeric id(CLI)"
						ALLOCBUFFER(targetConfName, H243_NAME_LEN);
						Phone* pPhone = pConfParty->GetActualPartyPhoneNumber(0);
						if (pPhone)
							sprintf(targetConfName, "%s(%s)", (const char*)m_dtmf, pPhone->phone_number);
						else
							strcpy(targetConfName, (const char*)m_dtmf);

						STATUS status;
						// Update Basic Ad Hoc Conf Params
						// create Ad-Hoc GW conf with H323 Audio only dial out party (for DMA)
						if (IsAdHocGW == TRUE)
						{
							ALLOCBUFFER(targetGWConfName, H243_NAME_LEN)
							ALLOCBUFFER(targetGWConfDisplayName, H243_NAME_LEN)

							DWORD nidCounter = ::GetpProfilesDB()->NextAdHocGwNidCounter();

							sprintf(targetGWConfName, "%s_(%03d)", "DMAGW", nidCounter);
							pConfProfile->SetAdHocConfBasicParams(targetGWConfName, "", 0xFFFFFFFF);

							if (pConfParty->GetVisualPartyName()[0] != '\0')
							{
								ALLOCBUFFER(partyVisualName, H243_NAME_LEN - 15);
								strncpy(partyVisualName, pConfParty->GetVisualPartyName(), H243_NAME_LEN - 15);
								sprintf(targetGWConfDisplayName, "%s_%s_(%03d)", "GW", partyVisualName, nidCounter);
								DEALLOCBUFFER(partyVisualName);
							}
							else
							{
								Phone* pPhone = pConfParty->GetActualPartyPhoneNumber(0);
								if (pPhone)
									sprintf(targetGWConfDisplayName, "%s_(%s)_(%03d)", "GW", pPhone->phone_number, nidCounter);
								else
									strcpy(targetGWConfDisplayName, targetGWConfName);
							}

							targetGWConfDisplayName[H243_NAME_LEN-1] = '\0';
							pConfProfile->SetDisplayName(targetGWConfDisplayName);

							DEALLOCBUFFER(targetGWConfName);

							pConfProfile->SetIsGateway(YES);

							pConfProfile->SetIsGWDialOutToH323(YES);
							// All GW sessions are cp
							pConfProfile->SetHDVSW(NO);
							// direct dial in is disabled in GW sessions
							pConfProfile->SetMeetMePerConf(NO);
							// all GW sessions are terminated immediatly with last party remains
							pConfProfile->SetAutomaticTermination(YES);
							pConfProfile->SetLastQuitType(eTerminateWithLastRemains);
							pConfProfile->SetTimeAfterLastQuit(0);

							CRsrvParty* p323GateWayPartyOut = new CRsrvParty();
							BYTE        netInterfaceType    = H323_INTERFACE_TYPE;
							// get global pointer to service proivers list
							// will get the default h.323 service
							CConfIpParameters* pIpParams = ::GetIpServiceListMngr()->GetRelevantService("", netInterfaceType);           // GetFirstServiceInList();
							if (pIpParams)
							{
								char* serviceNameStr = ((char*)pIpParams->GetServiceName());
								p323GateWayPartyOut->SetConnectionType(DIAL_OUT);
								p323GateWayPartyOut->SetNetInterfaceType(netInterfaceType);
								p323GateWayPartyOut->SetServiceProviderName(serviceNameStr);
								p323GateWayPartyOut->SetUndefinedType(UNRESERVED_PARTY);
								p323GateWayPartyOut->SetH323PartyAliasType(PARTY_H323_ALIAS_E164_TYPE);
								p323GateWayPartyOut->SetH323PartyAlias((const char*)m_dtmf);
								p323GateWayPartyOut->SetVoice(YES);           // the party should be audio only

								ALLOCBUFFER(h243name, H243_NAME_LEN);
								sprintf(h243name, "%s_%s", targetGWConfDisplayName, "323Out");
								p323GateWayPartyOut->SetName(h243name);
								DEALLOCBUFFER(h243name)

								p323GateWayPartyOut->SetPartyId(pConfProfile->NextPartyId());
								status = pConfProfile->Add(*p323GateWayPartyOut);
							}
							else
							{
								status = STATUS_NET_SERVICE_NOT_FOUND;
							}

							POBJDELETE(p323GateWayPartyOut);
							DEALLOCBUFFER(targetGWConfDisplayName);
						}
						else
						{
							pConfProfile->SetAdHocConfBasicParams(targetConfName, (const char*)m_dtmf, dwAdHocProfileId);
						}

						status = pConfProfile->TestValidity();

						if (STATUS_OK != status)
							returnValue = TOO_MANY_ERRORS;
						else
						{
							// Set the target conference type to Ad-Hoc
							targetConfType = eAdHoc;

							// inform CAM on start Move
							CSegment* pSeg = new CSegment;
							*pSeg << m_pParty->GetName();
							m_pConfApi->SendCAMGeneralNotifyCommand((DWORD)0, IVR_PARTY_START_MOVE_FROM_EQ, pSeg);
							POBJDELETE(pSeg);

							// Send a reqest to create an Ad-Hoc conference and move the party into it as a leader
							confPartyManagerLocalApi.MovePartyToConfOrMeetingRoom(dwSourceConfId, dwPartId, targetConfType,
							                                                      (DWORD)(-1), pConfProfile);
							// Set the party to be leader in the target conference
							m_pParty->SetIsLeader(YES);           // m_pParty is a pointer to CParty
						}

						DEALLOCBUFFER(targetConfName);
					}

					POBJDELETE(pConfProfile);
				}
			}
			else           // Conf not found, not an External-DB, Not an Ad-Hoc EQ
			{
				PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::IsLegalString: conf not found and it is not an Ad-Hoc nor E-DB");
				returnValue = DTMF_STRING_ERROR;
			}// else (pEqCommConf->GetAdHoc())
		}// else (IVR_EXTERNAL_DB_NUMERIC_ID == m_useExternalDB)
	}// else (dwConf_id != 0xFFFFFFFF)

	// print to trace
	ALLOCBUFFER(strParty, H243_NAME_LEN_DEFINED_AGAIN + 50);
	sprintf(strParty, " ---> %s: ", m_pParty->GetName());
	PTRACE2(eLevelInfoNormal, strParty, str);
	DEALLOCBUFFER(str);
	DEALLOCBUFFER(strParty);

	return returnValue;
}

//--------------------------------------------------------------------------
WORD CIvrSubNumericConferenceId::DoSomething()
{
	// do something on success
	m_ivrCntl->SetStartNextFeature();
	return 0;
}

//--------------------------------------------------------------------------
void CIvrSubNumericConferenceId::OnTimeNoExtDBResponse(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubNumericConferenceId::OnTimeNoExtDBResponse -  F_IVR===>NID, party=", m_pParty->GetName());
	OnEndFeature(TOO_MANY_ERRORS);
}

//--------------------------------------------------------------------------
int CIvrSubNumericConferenceId::OnExtDBResponse(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID party=", m_pParty->GetName());

	DeleteTimer(EXT_DB_RESPONSE_TOUT);

	CXMLDOMDocument* pDom      = new CXMLDOMDocument; AUTO_DELETE(pDom);
	CXMLDOMElement*  pTempNode = NULL, * pNode = NULL;
	BYTE             id        = 1, bLeader = FALSE, bEndFeature = TRUE;
	int              status    = DTMF_OK, nStatus = SEC_OK;
	char*            nodeName  = NULL, * owner = NULL, * info1 = NULL, * info2 = NULL, * info3 = NULL,
	* name                     = NULL, * billingData = NULL, * password = NULL, * entryPwd = NULL, * desc = NULL, * display_name = NULL;
	ALLOCBUFFER(pszError, H243_NAME_LEN); AUTO_DELETE_ARRAY(pszError);
	ALLOCBUFFER(missingField, H243_NAME_LEN); AUTO_DELETE_ARRAY(missingField);
	DWORD           XMLStringLen   = 0;
	char*           maxParties     = NULL, * minParties = NULL;
	ETargetConfType targetConfType = eDummyType;
	CCommConf*      pEqCommConf    = NULL;
	CConfParty*     pConfParty     = NULL;

	if (DTMF_OK == status)
	{
		pEqCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

		if (!pEqCommConf)
		{
			PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse - NULL pointer, F_IVR===>NID");
			status = DTMF_STRING_ERROR;
		}
	}

	if (DTMF_OK == status)
	{
		pConfParty = pEqCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
		if (!pConfParty)
		{
			PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse -  NULL pointer, F_IVR===>NID");
			status = DTMF_STRING_ERROR;
		}
	}

	*pParam >> XMLStringLen;
	ALLOCBUFFER(pXMLString, XMLStringLen+1); AUTO_DELETE_ARRAY(pXMLString);
	*pParam >> pXMLString;
	pXMLString[XMLStringLen] = '\0';

	if (pDom->Parse((const char**)&pXMLString) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();

		nStatus = pRoot->get_nodeName(&nodeName);
		if (!strncmp(nodeName, "CONFIRM_CONF_DETAILS", 22))
		{
			GET_CHILD_NODE(pRoot, "RETURN_STATUS", pNode);
			if (pNode)
			{
				GET_VALIDATE_CHILD(pNode, "ID", &id, _0_TO_10_DECIMAL);
				if (nStatus != SEC_OK)
					sprintf(missingField, "'ID'");

				GET_VALIDATE_CHILD(pNode, "DESCRIPTION", &desc, DESCRIPTION_LENGTH);
			}
			else if (nStatus != SEC_OK)
				sprintf(missingField, "'RETURN_STATUS'");

			if (nStatus == SEC_OK)
			{
				GET_CHILD_NODE(pRoot, "ACTION", pTempNode);
				if (pTempNode)
				{
					GET_CHILD_NODE(pTempNode, "CREATE", pNode);
					if (pNode)
					{
						GET_VALIDATE_CHILD(pNode, "NAME", &name, _0_TO_H243_NAME_LENGTH);
						GET_VALIDATE_CHILD(pNode, "MAX_PARTIES", &maxParties, MAX_PARTIES_ENUM);
						GET_VALIDATE_CHILD(pNode, "MIN_NUM_OF_PARTIES", &minParties, _0_TO_60_DECIMAL);
						GET_VALIDATE_CHILD(pNode, "PASSWORD", &password, _0_TO_H243_NAME_LENGTH);

						if (password && (strlen(password) > MAX_PASSWORD_LENGTH))
						{
							PTRACE2(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse, Password length > Password length in system.cfg. F_IVR===>NID, Password = ", password);
							nStatus = ERR_FAIL;
						}

						GET_VALIDATE_CHILD(pNode, "ENTRY_PASSWORD", &entryPwd, _0_TO_H243_NAME_LENGTH);

						if (entryPwd && (strlen(entryPwd) > MAX_PASSWORD_LENGTH))
						{
							PTRACE2(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse, Entry Password length > Password length in system.cfg. F_IVR===>NID, Entry Password = ", password);
							nStatus = ERR_FAIL;
						}

						GET_VALIDATE_CHILD(pNode, "BILLING_DATA", &billingData, _0_TO_H243_NAME_LENGTH);
						GET_VALIDATE_CHILD(pNode, "OWNER", &owner, REMARK_LENGTH);

						GET_CHILD_NODE(pNode, "CONTACT_INFO_LIST", pTempNode);

						if (pTempNode)
						{
							CXMLDOMElement* pContactInfoNode;

							GET_FIRST_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);
							if (pContactInfoNode)
							{
								GET_VALIDATE(pContactInfoNode, &info1, ONE_LINE_BUFFER_LENGTH);

								GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

								if (pContactInfoNode)
								{
									GET_VALIDATE(pContactInfoNode, &info2, ONE_LINE_BUFFER_LENGTH);

									GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

									if (pContactInfoNode)
										GET_VALIDATE(pContactInfoNode, &info3, ONE_LINE_BUFFER_LENGTH);
								}
							}
						}

						GET_VALIDATE_CHILD(pNode, "DISPLAY_NAME", &display_name, _0_TO_H243_NAME_LENGTH);
					}
				}
			}
		}
		else
		{
			PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse, F_IVR===>NID, confirmation transaction is not recognazed ");
			nStatus = STATUS_ILLEGAL;
		}
	}

	if (nStatus == STATUS_ILLEGAL || strlen(missingField) > 0)
	{
		PTRACE2(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse, F_IVR===>NID, XML string does not include the mandatory field: ", missingField);
		PASSERT(STATUS_ILLEGAL);
		OnEndFeature(TOO_MANY_ERRORS);
	}
	else if (DTMF_OK == status)
	{
		CConfPartyManagerLocalApi confPartyManagerLocalApi;

		if (desc && !strcmp(desc, ""))
			PTRACE2(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, Description = ", desc);

		m_state = ACTIVE;

		switch (id)
		{
			case (0):
			{
				PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnExtDBResponse - DB replied - OK. , F_IVR===>NID");
				// get MCU-Manager API

				// move party to its destination conference
				DWORD dwSourceConfId = m_ivrCntl->GetMonitorConfId();           // current EQ conf-ID
				DWORD dwPartId       = m_pParty->GetMonitorPartyId();           // party-ID
				PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, numeric id to search: ", m_pNumericConferenceid);

				// 1. Check if the Numric ID matches one of the On-Going conferences
				DWORD dwConf_id = 0xFFFFFFFF;

				dwConf_id = FindConfAccordingToNid(m_pNumericConferenceid);

				if (dwConf_id != 0xFFFFFFFF)           // Numeric ID was found in an on-going conference.
				{
					targetConfType = eOnGoingConf;
					PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnExtDBResponse: move to EQ access conf");

					CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(dwConf_id);
					if (pCommConf)
					{
						CConfApi* pDestConfApi = new CConfApi;
						pDestConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()), NULL, NULL, 1);
						STATUS    rspStatus = STATUS_OK;
						OPCODE    rspOpcode;
						CSegment  rspMsg;
						rspStatus = pDestConfApi->IsDestConfReadyForMove(rspMsg, 5*SECOND, rspOpcode);
						pDestConfApi->DestroyOnlyApi();
						POBJDELETE(pDestConfApi);

						if ((rspStatus != STATUS_OK) || (rspOpcode != STATUS_OK))
						{
							PTRACE2(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse: Error: Conf is not ready ", pCommConf->GetName());
							status = TOO_MANY_ERRORS;
						}
					}

					// inform CAM on start Move
					if (m_pIvrService->m_bIsEntryQueueService)
					{
						CSegment* pSeg = new CSegment();
						*pSeg << m_pParty->GetName();
						m_pConfApi->SendCAMGeneralNotifyCommand((DWORD)0, IVR_PARTY_START_MOVE_FROM_EQ, pSeg);
						POBJDELETE(pSeg);
					}

					// move party to its destination conference
					confPartyManagerLocalApi.MovePartyToConfOrMeetingRoom(dwSourceConfId, dwPartId, targetConfType, dwConf_id, NULL);
				}
				else           // Numeric id was not found in any on-going conference
				{
					PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID,  Conf or MR with the current NID was not found ");
					// 2. Check if it is an Ad-Hoc EQ
					if (pEqCommConf->GetAdHoc())
					{
						// Get Ad-Hoc profile ID
						DWORD dwAdHocProfileId = pEqCommConf->GetAdHocProfileId();
						// If the Ad-Hoc is checked and no profile is configured return error
						if (dwAdHocProfileId == 0xFFFFFFFF)
						{
							PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse: No Profile selected in Ad-Hoc");
							status = DTMF_STRING_ERROR;
						}

						// Get Profile according to Ad-Hoc Profile ID
						CCommRes* pConfProfile = ::GetpProfilesDB()->GetCurrentRsrv(dwAdHocProfileId);           // get & allocate
						if (!pConfProfile)
						{
							PASSERT(4);
							PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse: NULL pointer");
							status = DTMF_STRING_ERROR;
						}

						if (DTMF_OK == status)
						{
							// Set Ad Hoc target confernce nam
							ALLOCBUFFER(ConfName, H243_NAME_LEN);
							if (!name)
							{
								// Create the Ad Hoc confernce name - "Numeric id(CLI)"
								Phone* pPhone = pConfParty->GetActualPartyPhoneNumber(0);
								if (pPhone)
									sprintf(ConfName, "%s(%s)", m_pNumericConferenceid, pPhone->phone_number);
								else
									sprintf(ConfName, "%s", m_pNumericConferenceid);
							}
							else           // If the conference name was sent from the database
								sprintf(ConfName, "%s", name);

							pConfProfile->SetAdHocConfBasicParams(ConfName, m_pNumericConferenceid, dwAdHocProfileId);
							pConfProfile->SetAdHocConfExtDbBasedParams(display_name, maxParties, minParties, password,
							                                           entryPwd, billingData, owner,
							                                           info1, info2, info3);

							STATUS valStatus = pConfProfile->TestValidity();
							DEALLOCBUFFER(ConfName);

							if (STATUS_OK != valStatus)
								status = TOO_MANY_ERRORS;
							else
							{
								// Set the target conference type to Ad-Hoc
								targetConfType = eAdHoc;

								// inform CAM on start Move
								CSegment* pSeg = new CSegment;
								*pSeg << m_pParty->GetName();
								m_pConfApi->SendCAMGeneralNotifyCommand((DWORD)0, IVR_PARTY_START_MOVE_FROM_EQ, pSeg);
								POBJDELETE(pSeg);

								// Send a reqest to create an Ad-Hoc conference and move the party into it
								confPartyManagerLocalApi.MovePartyToConfOrMeetingRoom(dwSourceConfId, dwPartId, targetConfType,
								                                                      (DWORD)(-1), pConfProfile);
							}
						}

						POBJDELETE(pConfProfile);
					}
					else                                      // Not an Ad-Hoc EQ
					{
						// 3. Check if the Numric ID matches one of the MRs
						dwConf_id = 0xFFFFFFFF;
						WORD nidOfEqOrSipFactory = 0;           // not relevant for external DB

						dwConf_id = FindMRAccordingToNid(m_pNumericConferenceid, nidOfEqOrSipFactory);

						if (dwConf_id != 0xFFFFFFFF)            // Numeric ID was found in a Meeting Room
						{
							targetConfType = eMeetingRoom;
							PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnExtDBResponse: move to EQ access MR");

							// inform CAM on start Move
							if (m_pIvrService->m_bIsEntryQueueService)
							{
								CSegment* pSeg = new CSegment;
								*pSeg << m_pParty->GetName();
								m_pConfApi->SendCAMGeneralNotifyCommand((DWORD)0, IVR_PARTY_START_MOVE_FROM_EQ, pSeg);
								POBJDELETE(pSeg);
							}

							// move party to its destination conference
							confPartyManagerLocalApi.MovePartyToConfOrMeetingRoom(dwSourceConfId, dwPartId, targetConfType, dwConf_id, NULL);
						}
						else
						{
							PTRACE(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, conf not found and it is not an Ad-Hoc");
							status = TOO_MANY_ERRORS;
						}
					}
				}
				break;
			}

			case (1):
			{
				PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, Illegal MCU user name / password .");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (2):
			{
				PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, Request timed out.");
				status = TOO_MANY_ERRORS;
				break;
			}

			// if party is not authorized
			case (3):
			{
				PTRACE2(eLevelInfoNormal, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, wrong NID. ", m_pNumericConferenceid);
				CdrPartyDtmfFailureIndication("Unknown");
				status = DTMF_STRING_ERROR;
				break;
			}

			case (4):
			{
				PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, Illegal CLI - The originator CLI is not permitted to start conferences.");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (5):
			{
				PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, Illegal CLI/NID - The combination of the CLI and NID is not permitted.");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (6):
			{
				PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, Internal Error in DB.");
				status = TOO_MANY_ERRORS;
				break;
			}

			default:
			{
				PTRACE(eLevelError, "CIvrSubNumericConferenceId::OnExtDBResponse - F_IVR===>NID, Illegal ID response from DB.");
				status = TOO_MANY_ERRORS;
				break;
			}
		}           // switch
	}

	DEALLOCBUFFER(pszError);
	DEALLOCBUFFER(missingField);
	DEALLOCBUFFER(pXMLString);
	PDELETE(pDom);

	if (DTMF_STRING_ERROR == status)
	{
		m_errorCounter++;                                  // increses error counter
		m_timeoutCounter = 0;                              // resets tomeout counter
		if (m_maxRetryTimes > m_errorCounter)              // allowed to retry
		{
			m_pDtmf->ResetDtmfBuffer();                      // reset DTMF buffer
			StartBase(TRUE, IVR_STATUS_PLAY_ONCE);           // wrong UserId, retries to get UserID again
			bEndFeature = FALSE;
		}
		else
		{
			status = TOO_MANY_ERRORS;           // finish status: ERROR
		}
	}

	if (bEndFeature)
		OnEndFeature(status);

	return status;
}

//--------------------------------------------------------------------------
void CIvrSubNumericConferenceId::OnExtDBFailure(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubNumericConferenceId::OnExtDBFailure - F_IVR===>NID, party=", m_pParty->GetName());
	OnEndFeature(TOO_MANY_ERRORS);
}

//--------------------------------------------------------------------------
void CIvrSubNumericConferenceId::OnSIPconfNIDConfirmInd(CSegment* pParam)
{
	DWORD sts;
	*pParam >> sts;

	TRACEINTO<<"CIvrSubNumericConferenceId::OnSIPconfNIDConfirmInd - ivrProviderEQ status="<<sts;

	DeleteTimer(EXTDB_IVR_PROV_TIMER);

	switch (sts)
	{
		case eStsOk:
		case eStsReject:
			m_state = NOTACTIVE;
			break;

		case eStsRetry:
			m_errorCounter++;                               // increses error counter
			m_timeoutCounter = 0;                           // resets tomeout counter
			m_state          = ACTIVE;
			m_pDtmf->ResetDtmfBuffer();                     // reset DTMF buffer
			StartBase(TRUE, IVR_STATUS_PLAY_ONCE);          // wrong UserId, retries to get UserID again
			break;
	} // switch
}

//--------------------------------------------------------------------------
int CIvrSubNumericConferenceId::IsQuickLeaderPW(CCommRes* pCommConf)
{
	return 0;
}

//--------------------------------------------------------------------------
bool CIvrSubNumericConferenceId::IsDTMFWODelimeterEnabled()
{
	return IsDTMFWODelimeterFlagEnabled();
}


PBEGIN_MESSAGE_MAP(CIvrSubMuteNoisyLine)
	ONEVENT(DTMF_STRING_IDENT,          NOTACTIVE, CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT,          ACTIVE,    CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(TIMER_RETRY_MESSAGE,        ACTIVE,    CIvrSubBaseSM::OnTimeRetryMessage)
	ONEVENT(DTMF_NOISY_LINE_UNMUTE,     ACTIVE,    CIvrSubMuteNoisyLine::OnDtmfNoisyLineUnmute)
	ONEVENT(DTMF_NOISY_LINE_ADJUST,     ACTIVE,    CIvrSubMuteNoisyLine::OnDtmfNoisyLineAdjust)
	ONEVENT(DTMF_NOISY_LINE_DISABLE,    ACTIVE,    CIvrSubMuteNoisyLine::OnDtmfNoisyLineDisable)
	ONEVENT(DTMF_NOISY_LINE_MUTE,       ACTIVE,    CIvrSubMuteNoisyLine::OnDtmfNoisyLineMute)
	ONEVENT(DTMF_NOISY_LINE_HELP_MENU,  ACTIVE,    CIvrSubMuteNoisyLine::PlayMuteNoisyLineMenu)
	ONEVENT(START_MUTE_NOISY_LINE_MENU, ACTIVE,    CIvrSubMuteNoisyLine::PlayMuteNoisyLinePreMenu)
PEND_MESSAGE_MAP(CIvrSubMuteNoisyLine, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubMuteNoisyLine
////////////////////////////////////////////////////////////////////////////
CIvrSubMuteNoisyLine::CIvrSubMuteNoisyLine(const CIVRMuteNoisyLineFeature* pMuteNoisyLine)
{
	m_pMuteNoisyLine    = new CIVRMuteNoisyLineFeature(*pMuteNoisyLine);
	m_pIvrDefConfTable  = NULL;
	m_pSubConfDtmfTable = NULL;
}

//--------------------------------------------------------------------------
CIvrSubMuteNoisyLine::~CIvrSubMuteNoisyLine()
{
	POBJDELETE(m_pMuteNoisyLine);
	PrepareEndFeature();
	m_pIvrDefConfTable = NULL; // Reset the pointer.
}

//--------------------------------------------------------------------------
void* CIvrSubMuteNoisyLine::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::Start()
{
	PTRACE2(eLevelInfoNormal, "CIvrSubMuteNoisyLine::Start - F_IVR===>NoisyLine, Party name: ", m_pParty->GetName());

	// Mute noisy party before starting the Silence IT feature
	m_ivrCntl->MuteParty(m_pConfApi, m_pParty, 0);

	// Create the noisy line dtmf table
	BYTE bDtmfTableStatus = CreateSubConfDtmfTable();

	if (bDtmfTableStatus != STATUS_OK)
	{
		PASSERT(bDtmfTableStatus);
		OnEndFeature(STATUS_OK);
		return;
	}

	StartBaseTimer();
	int status = STATUS_OK;
	m_state = ACTIVE;

	if (CPObject::IsValidPObjectPtr(m_pMuteNoisyLine))
	{
		if (m_pMuteNoisyLine->GetEnableDisable() == YES)
		{
			if (m_pIvrDefConfTable == NULL)
			{
				CDTMFCodeList* pIvrDefConfTable = m_pDtmf->GetDtmfCodeList();
				if (pIvrDefConfTable)
					m_pIvrDefConfTable = new CDTMFCodeList(*pIvrDefConfTable);
			}

			CIVRLanguage* pIVRLanguage = m_pIvrService->GetFirstLanguage();
			if (!CPObject::IsValidPObjectPtr(pIVRLanguage))
			{
				PASSERT(1);
				OnEndFeature(STATUS_OK); // Check if OnEndFeature Free m_pIvrDefConfTable  ????
				return;
			}

			// Initialize dtmf collector
			if (m_pDtmf)
			{
				m_pDtmf->SetDtmfTable(m_pSubConfDtmfTable);
				m_pDtmf->SetNoisyLineIvr(YES);
				m_pDtmf->StartFeature();
				m_pDtmf->SetDtmfWaitForXDigitsNumber(0, 0, 0);
			}

			// Check if the party has roll call recording and noisy line message to play to the conference
			if (m_pMuteNoisyLine->GetPlayNoisyDetectionMessage())
			{
				// Send CAM a command to activate the noisy line indication message to conference
				m_pConfApi->SendStartFeature(m_pParty->GetPartyRsrcID(), EVENT_CONF_REQUEST, eCAM_EVENT_CONF_NOISY_LINE_DETECTION);

				// Play Mute NoisyLine Pre Menu to party
				PlayMuteNoisyLinePreMenu();
			}
			else
				PlayMuteNoisyLinePreMenu();
		}
		else
		{
			PTRACE2(eLevelInfoNormal, "CIvrSubMuteNoisyLine::Start - F_IVR===>NoisyLine, the feature is disabled. Party name: ", m_pParty->GetName());
			OnEndFeature(STATUS_OK);
			return;
		}
	}
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::PlayMuteNoisyLinePreMenu(void)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubMuteNoisyLine::PlayMuteNoisyLinePreMenu - F_IVR===>NoisyLine, Party name: ", m_pParty->GetName());

	CIVRLanguage* pIVRLanguage = m_pIvrService->GetFirstLanguage();
	if (!CPObject::IsValidPObjectPtr(pIVRLanguage))
	{
		PASSERT(1);
		OnEndFeature(STATUS_OK);
		return;
	}

	if (!CPObject::IsValidPObjectPtr(m_pMuteNoisyLine))
	{
		PASSERT(2);
		OnEndFeature(STATUS_OK);
		return;
	}

	// Allocation of messages id array for the sequence message
	IVRMsgDescriptor* pArrayOfMessagesToPlay = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];

	// Initialize the array
	for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
	{
		pArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
		pArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
		pArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
	}

	WORD wNumberOfMessagesToPlay = 0;

	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);

	if (m_pMuteNoisyLine->GetMuteNoisyLineMenu())
	{
		GetMuteNoisyLineMsgsParams(IVR_EVENT_NOISY_LINE_HELP_MENU, pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, &msgDuration, &msgCheckSum);

		if (('\0' != pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0]) && (0 != msgDuration) && ((WORD)(-1) != msgCheckSum))
		{
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
			wNumberOfMessagesToPlay++;
		}
		else
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::PlayMuteNoisyLinePreMenu - F_IVR===>NoisyLine, Message file is not configured");
	}

	if (m_pMuteNoisyLine->GetReturnAndUnmute())
	{
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0] = '\0';
		msgDuration = 0;
		msgCheckSum = (WORD)(-1);

		GetMuteNoisyLineMsgsParams(IVR_EVENT_NOISY_LINE_UNMUTE, pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, &msgDuration, &msgCheckSum);

		if (('\0' != pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0]) && (0 != msgDuration) && ((WORD)(-1) != msgCheckSum))
		{
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
			wNumberOfMessagesToPlay++;
		}
		else
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::PlayMuteNoisyLinePreMenu - F_IVR===>NoisyLine, Message file is not configured");
	}

	// Play the sequence message in loop
	if (0 != wNumberOfMessagesToPlay)
		m_pConfApi->StartSequenceMessages(m_pParty->GetPartyRsrcID(), pArrayOfMessagesToPlay, wNumberOfMessagesToPlay, PRIVATE_MSG, IVR_STATUS_PLAY_LOOP);
	else
		PTRACE(eLevelError, "CIvrSubMuteNoisyLine::PlayMuteNoisyLinePreMenu - F_IVR===>NoisyLine, Mute noisy line messages are not configured");

	PDELETEA(pArrayOfMessagesToPlay);
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::PlayMuteNoisyLineMenu(void)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubMuteNoisyLine::PlayMuteNoisyLineMenu - F_IVR===>NoisyLine, Party name: ", m_pParty->GetName());

	CIVRLanguage* pIVRLanguage = m_pIvrService->GetFirstLanguage();
	if (!CPObject::IsValidPObjectPtr(pIVRLanguage))
	{
		PASSERT(1);
		OnEndFeature(STATUS_OK);
		return;
	}

	if (!CPObject::IsValidPObjectPtr(m_pMuteNoisyLine))
	{
		PASSERT(2);
		OnEndFeature(STATUS_OK);
		return;
	}

	// Allocation of messages id array for the sequence message
	IVRMsgDescriptor* pArrayOfMessagesToPlay = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];
	// Initialize the array
	for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
	{
		pArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
		pArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
		pArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
	}

	WORD wNumberOfMessagesToPlay = 0;

	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);


	// Add the message id array before sending the request
	if (m_pMuteNoisyLine->GetReturnAndUnmute())
	{
		GetMuteNoisyLineMsgsParams(IVR_EVENT_NOISY_LINE_UNMUTE, pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, &msgDuration, &msgCheckSum);

		if (('\0' != pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0]) && (0 != msgDuration) && ((WORD)(-1) != msgCheckSum))
		{
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
			wNumberOfMessagesToPlay++;
		}
		else
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::PlayMuteNoisyLineMenu - F_IVR===>NoisyLine, Message file is not configured");
	}

	if (m_pMuteNoisyLine->GetAdjustNoiseDetection())
	{
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0] = '\0';
		msgDuration = 0;
		msgCheckSum = (WORD)(-1);

		GetMuteNoisyLineMsgsParams(IVR_EVENT_NOISY_LINE_ADJUST, pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, &msgDuration, &msgCheckSum);

		if (('\0' != pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0]) && (0 != msgDuration) && ((WORD)(-1) != msgCheckSum))
		{
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
			wNumberOfMessagesToPlay++;
		}
		else
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::PlayMuteNoisyLineMenu - F_IVR===>NoisyLine, Message file is not configured");
	}

	if (m_pMuteNoisyLine->GetDisableNoiseDetection())
	{
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0] = '\0';
		msgDuration = 0;
		msgCheckSum = (WORD)(-1);

		GetMuteNoisyLineMsgsParams(IVR_EVENT_NOISY_LINE_DISABLE, pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, &msgDuration, &msgCheckSum);

		if (('\0' != pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0]) && (0 != msgDuration) && ((WORD)(-1) != msgCheckSum))
		{
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
			wNumberOfMessagesToPlay++;
		}
		else
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::PlayMuteNoisyLineMenu - F_IVR===>NoisyLine, Message file is not configured");
	}

	if (m_pMuteNoisyLine->GetReturnAndMute())
	{
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0] = '\0';
		msgDuration = 0;
		msgCheckSum = (WORD)(-1);

		GetMuteNoisyLineMsgsParams(IVR_EVENT_NOISY_LINE_MUTE, pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, &msgDuration, &msgCheckSum);

		if (('\0' != pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath[0]) && (0 != msgDuration) && ((WORD)(-1) != msgCheckSum))
		{
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
			pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
			wNumberOfMessagesToPlay++;
		}
		else
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::PlayMuteNoisyLineMenu - F_IVR===>NoisyLine, Message file is not configured");
	}

	// Play the sequence message in loop
	if (0 != wNumberOfMessagesToPlay)
		m_pConfApi->StartSequenceMessages(m_pParty->GetPartyRsrcID(), pArrayOfMessagesToPlay, wNumberOfMessagesToPlay, PRIVATE_MSG);
	else
		PTRACE(eLevelError, "CIvrSubMuteNoisyLine::PlayMuteNoisyLineMenu - F_IVR===>NoisyLine, Mute noisy line messages are not configured");

	PDELETEA(pArrayOfMessagesToPlay);
}

//--------------------------------------------------------------------------
int CIvrSubMuteNoisyLine::IsLegalString(CSegment* pParam)
{
	return 1;
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::PlayRetryMessage(DWORD messageMode)
{
}

//--------------------------------------------------------------------------
WORD CIvrSubMuteNoisyLine::DoSomething()
{
	return 0;
}

//--------------------------------------------------------------------------
BYTE CIvrSubMuteNoisyLine::CreateSubConfDtmfTable()
{
	if (m_pSubConfDtmfTable) // The table is already exists do not build it again but just
		return STATUS_OK;      // return an indication that everything is all right.

	m_pIvrDefConfTable = m_pDtmf->GetDtmfCodeList();
	// Create the noisy line table from the main table at the DtmfCollector
	if (m_pIvrDefConfTable)
	{
		m_pSubConfDtmfTable = new CDTMFCodeList;
		CDTMFCode* Dtmfcode            = m_pIvrDefConfTable->GetFirstDTMFCode();
		int        number_of_DTMF_code = m_pIvrDefConfTable->GetNumberOfDtmfCodes();

		for (int i = 0; i < number_of_DTMF_code - 1 && Dtmfcode != NULL; i++)
		{
			WORD DtmfOpcode = Dtmfcode->GetDTMFOpcode();

			if (DtmfOpcode == DTMF_NOISY_LINE_MUTE || DtmfOpcode == DTMF_NOISY_LINE_UNMUTE
			    || DtmfOpcode == DTMF_NOISY_LINE_ADJUST || DtmfOpcode == DTMF_NOISY_LINE_DISABLE
			    || DtmfOpcode == DTMF_NOISY_LINE_HELP_MENU)
			{
				m_pSubConfDtmfTable->AddDTMFCode(*Dtmfcode);
			}

			Dtmfcode = m_pIvrDefConfTable->GetNextDTMFCode();
		}

		m_pIvrDefConfTable = NULL;
		return STATUS_OK;
	}
	else
		return STATUS_ILLEGAL;
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::OnDtmfNoisyLineUnmute(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubMuteNoisyLine::OnDtmfString - F_IVR===>NoisyLine, DTMF message ");
	StopPlayMessage();

	if (CPObject::IsValidPObjectPtr(m_ivrCntl))
	{
		m_ivrCntl->MovePartyToInConf();
		m_ivrCntl->UnMuteParty(m_pConfApi, m_pParty);
	}

	// Remove the noisy line inidication
	if (CPObject::IsValidPObjectPtr(m_pConfApi))
	{
		m_pConfApi->UpdateDB(m_pParty, NOISE_DETECTION, PARTY_CONNECTED, 1);
		// Update the party control and not the dsp
		UpdateNoiseDetection(NO, NO, NO);
	}

	OnEndFeature(STATUS_OK);
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::OnDtmfNoisyLineAdjust(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubMuteNoisyLine::OnDtmfNoisyLineAdjust - F_IVR===>NoisyLine ");
	StopPlayMessage();

	UpdateNoiseDetection(YES, YES, YES);

	if (CPObject::IsValidPObjectPtr(m_ivrCntl))
	{
		m_ivrCntl->MovePartyToInConf();
		m_ivrCntl->UnMuteParty(m_pConfApi, m_pParty);
	}

	// Remove the noisy line inidication
	if (CPObject::IsValidPObjectPtr(m_pConfApi))
		m_pConfApi->UpdateDB(m_pParty, NOISE_DETECTION, PARTY_CONNECTED, 1);

	OnEndFeature(STATUS_OK);
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::OnDtmfNoisyLineDisable(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubMuteNoisyLine::OnDtmfNoisyLineDisable - F_IVR===>NoisyLine");
	StopPlayMessage();

	UpdateNoiseDetection(NO, NO, YES);

	if (CPObject::IsValidPObjectPtr(m_ivrCntl))
	{
		m_ivrCntl->MovePartyToInConf();
		m_ivrCntl->UnMuteParty(m_pConfApi, m_pParty);
	}

	// Remove the noisy line inidication
	if (CPObject::IsValidPObjectPtr(m_pConfApi))
		m_pConfApi->UpdateDB(m_pParty, NOISE_DETECTION, PARTY_CONNECTED, 1);

	OnEndFeature(STATUS_OK);
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::OnDtmfNoisyLineMute(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubMuteNoisyLine::OnDtmfNoisyLineMute - F_IVR===>NoisyLine ");
	StopPlayMessage();

	if (CPObject::IsValidPObjectPtr(m_ivrCntl))
	{
		// Return the party to the conference
		m_ivrCntl->MovePartyToInConf();

		// The message (if configured) will be played in the sequence message
		BYTE bPlayMessage = NO;
		m_ivrCntl->MuteParty(m_pConfApi, m_pParty, bPlayMessage);

		// Allocation of messages id array for the sequence message
		IVRMsgDescriptor* pArrayOfMessagesToPlay = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];
		// Initialize the array
		for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
		{
			pArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
			pArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
			pArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
		}

		WORD wNumberOfMessagesToPlay = 0;

		WORD msgDuration = 0;
		WORD msgCheckSum = (WORD)(-1);

		m_ivrCntl->GetGeneralSystemMsgsParams(IVR_EVENT_SELF_MUTE, pArrayOfMessagesToPlay[0].ivrMsgFullPath, &msgDuration, &msgCheckSum);

		// Add the message id to the sequence message array only if valid
		if (('\0' != pArrayOfMessagesToPlay[0].ivrMsgFullPath[0]) && (0 != msgDuration) && ((WORD)(-1) != msgCheckSum))
		{
			pArrayOfMessagesToPlay[0].ivrMsgDuration = msgDuration;
			pArrayOfMessagesToPlay[0].ivrMsgCheckSum = msgCheckSum;
			wNumberOfMessagesToPlay++;
		}

		msgDuration = 0;
		msgCheckSum = (WORD)(-1);

		// This message should be always configured
		GetMuteNoisyLineMsgsParams(IVR_EVENT_NOISY_LINE_UNMUTE_MESSAGE, pArrayOfMessagesToPlay[1].ivrMsgFullPath, &msgDuration, &msgCheckSum);

		if (('\0' != pArrayOfMessagesToPlay[1].ivrMsgFullPath[0]) && (0 != msgDuration) && ((WORD)(-1) != msgCheckSum))
		{
			pArrayOfMessagesToPlay[1].ivrMsgDuration = msgDuration;
			pArrayOfMessagesToPlay[1].ivrMsgCheckSum = msgCheckSum;
			wNumberOfMessagesToPlay++;
		}
		else
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::OnDtmfNoisyLineMute - F_IVR===>NoisyLine, Message file is not configured");

		// Update the party control and not the dsp
		UpdateNoiseDetection(NO, NO, NO);

		if (0 != wNumberOfMessagesToPlay)
			m_pConfApi->StartSequenceMessages(m_pParty->GetPartyRsrcID(), pArrayOfMessagesToPlay, wNumberOfMessagesToPlay, PRIVATE_MSG);
		else
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::OnDtmfNoisyLineMute - F_IVR===>NoisyLine, Mute noisy line messages are not configured");

		PDELETEA(pArrayOfMessagesToPlay);
	}

	OnEndFeature(STATUS_OK);
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::PrepareEndFeature()
{
	if (m_pIvrDefConfTable != NULL)
	{
		m_pDtmf->SetDtmfTable(m_pIvrDefConfTable); // Set the collector to point on the default table of the IVR.
		m_pIvrDefConfTable->DeleteList();
		POBJDELETE(m_pIvrDefConfTable);
	}

	if (m_pSubConfDtmfTable) // Destroy the DTMF codes table
	{
		m_pSubConfDtmfTable->DeleteList();
		POBJDELETE(m_pSubConfDtmfTable);
	}

	// Update the dtmf collector noisy line flag
	m_pDtmf->SetNoisyLineIvr(NO);
}

//--------------------------------------------------------------------------
// Function parameters:
// bUpdateDsp = YES - Send update noise detection request to the dsp and update party control
// NO  - Update party control only (Don't send update noise detection request)
// bNoiseDetection = YES/NO  - Enable or Disable the noisy line detection in the request
// wNoiseDetectionThreshold = YES/NO Increase/descrease noisy line detection
// ///////////////////////////////////////////////////////////////////////////
void CIvrSubMuteNoisyLine::UpdateNoiseDetection(WORD noiseDetection, WORD noiseDetectionThreshold, WORD updateDsp)
{
	PTRACE(eLevelInfoNormal, "CIvrSubMuteNoisyLine::UpdateNoiseDetection - F_IVR===>NoisyLine ");

	if (CPObject::IsValidPObjectPtr(m_pConfApi))
		m_pConfApi->UpdatePartyNoiseDetection(m_pParty->GetPartyRsrcID(), noiseDetection, noiseDetectionThreshold, updateDsp);
}

//--------------------------------------------------------------------------
void CIvrSubMuteNoisyLine::GetMuteNoisyLineMsgsParams(WORD event_op_code, char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum)
{
	msgFullPath[0] = '\0';
	(*msgDuration) = 0;
	(*msgCheckSum) = (WORD)(-1);

	if (m_pMuteNoisyLine)
	{
		const char* ivrServiceName = m_pIvrService->GetName();

		int status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, IVR_FEATURE_MUTE_NOISY_LINE, event_op_code,
		                                                    msgFullPath, msgDuration, msgCheckSum);
		if (STATUS_OK != status)
		{
			msgFullPath[0] = '\0';
			(*msgDuration) = 0;
			(*msgCheckSum) = (WORD)(-1);
			PTRACE(eLevelError, "CIvrSubMuteNoisyLine::GetMuteNoisyLineMsgsParams - F_IVR===>NoisyLine, GetIVRMsgParams failed");
		}
	}
}



PBEGIN_MESSAGE_MAP(CIvrSubPLC)
	ONEVENT(DTMF_STRING_IDENT,     NOTACTIVE, CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(TIMER_WAIT_FOR_DTMF,   ACTIVE,    CIvrSubPLC::OnTimerDTMF)
	ONEVENT(FIRST_DTMF_WAS_DIALED, ACTIVE,    CIvrSubPLC::OnDTMF)
PEND_MESSAGE_MAP(CIvrSubPLC, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubPLC
////////////////////////////////////////////////////////////////////////////
CIvrSubPLC::CIvrSubPLC()
{
	m_pIvrPrevTable    = NULL;
	m_currentMainIndex = 0;
	m_subMainIndex     = 0;

	for (int ind = 0; ind < MAX_PLC_TABLE_TYPES; ind++)
	{
		m_typeNum[ind] = 1;
		m_plcTableOpcodes[ind][0] = CP_LAYOUT_1X1;  // just default legal value
	}

	PrepareTables();
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubPLC::~CIvrSubPLC()
{
	POBJDELETE(m_pIvrPrevTable);
	m_pDtmf->InformAboutEveryDTMFPressed(0);
}

//--------------------------------------------------------------------------
void* CIvrSubPLC::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubPLC::RejectPLC()
{
	PTRACE(eLevelInfoNormal, "CIvrSubPLC::RejectPLC - F_IVR===>PLC, Reject PLC ");
	EndPlcFeature();
}

//--------------------------------------------------------------------------
void CIvrSubPLC::Start()
{
	PTRACE(eLevelInfoNormal, "CIvrSubPLC::Start - F_IVR===>PLC  ");
	if (!IsIvrPtrs())
		return;

	m_state       = ACTIVE;
	m_DTMF_digits = 1;  // expected to 1 DTMF

	// saves previous DTMF table
	POBJDELETE(m_pIvrPrevTable);

	CDTMFCodeList* pTempIvrPrevTable = m_pDtmf->GetDtmfCodeList();
	if (pTempIvrPrevTable)
		m_pIvrPrevTable = new CDTMFCodeList(*pTempIvrPrevTable);    // allocate

	// insert empty DTMF table
	m_pDtmf->SetDtmfTable(NULL);

	// sign as in-feature
	m_pDtmf->ResetDtmfParams();
	m_pDtmf->StartFeature();
	m_pDtmf->InformAboutEveryDTMFPressed(1);

	// sends START_PLC command
	m_pConfApi->StartStopPLC(m_pParty->GetPartyRsrcID(), 1);

	// starts Timer
	StartTimer(TIMER_WAIT_FOR_DTMF, 8 * SECOND);
}

//--------------------------------------------------------------------------
void CIvrSubPLC::OnDTMF(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubPLC::OnDTMF - F_IVR===>PLC ");

	// end timer (wait for DTMF)
	DeleteTimer(TIMER_WAIT_FOR_DTMF);

	// get the DTMF
	WORD opcode, rDigits;
	int  status = GetDtmfResult(pParam, opcode, rDigits);
	if ((0 != status) || (1 != rDigits))
		PTRACE2(eLevelError, "CIvrSubPLC::OnDTMF - F_IVR===>PLC, Error", PARTYNAME);
	else
		DoPlcAction();

	// starts Timer
	StartTimer(TIMER_WAIT_FOR_DTMF, 8 * SECOND);
}

//--------------------------------------------------------------------------
void CIvrSubPLC::DoPlcAction()
{
	if (m_dtmf[0] == '*')
		return; // nothing to do for start (SRS)
	else if (m_dtmf[0] == '#')
		DoSulamitAction();
	else if (m_dtmf[0] == '0')
		DoZeroAction();
	else
	{
		int num = (char)m_dtmf[0] - '9' + 9;
		DoNumberAction(num);
	}
}

//--------------------------------------------------------------------------
void CIvrSubPLC::DoStarAction()
{
	PTRACE(eLevelInfoNormal, "CIvrSubPLC::DoStarAction - F_IVR===>PLC, Star Action ");
	// Force for current layout (and change to Private Layout if needed in the VideoBridge)
	if (m_pConfApi)
		m_pConfApi->PlcAction(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_FORCE_PLC, 1);
}

//--------------------------------------------------------------------------
void CIvrSubPLC::DoZeroAction()
{
	PTRACE(eLevelInfoNormal, "CIvrSubPLC::DoZeroAction - F_IVR===>PLC, Zero Action ");
	// 1. cancel all private force
	if (m_pConfApi)
		m_pConfApi->PlcAction(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_FORCE_PLC, 0);

	// 2. return to conf layout
	if (m_pConfApi)
		m_pConfApi->PlcAction(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_PLC, CAM_CONF_LAYOUT);
}

//--------------------------------------------------------------------------
void CIvrSubPLC::DoSulamitAction()
{
	PTRACE(eLevelInfoNormal, "CIvrSubPLC::DoSulamitAction - F_IVR===>PLC, Sulamit Action ");

	if (m_pConfApi) // previously was the action for '*'
		m_pConfApi->PlcAction(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_FORCE_PLC, 1);
}

//--------------------------------------------------------------------------
void CIvrSubPLC::DoNumberAction(int num)
{
	TraceInt(eLevelInfoNormal, "CIvrSubPLC::DoNumberAction - F_IVR===>PLC, Number:  ", num);

	// only numbers from 1 to 9 without 7
	if ((num > 9) || (num < 1) || (7 == num))
	{
		PTRACE2(eLevelError, "CIvrSubPLC::DoNumberAction - F_IVR===>PLC, Illegal number ", PARTYNAME);
		return; // nothing to do
	}

	// update the indexes
	if (num != m_currentMainIndex)  // change main type (e.g. from 4 to 8)
	{
		if (num >= MAX_PLC_TABLE_TYPES)
			return;

		m_currentMainIndex = num;
		m_subMainIndex     = 0;
	}
	else                    // roll in the same type (e.g. 6 after 6)
	{
		m_subMainIndex++;
		if (m_subMainIndex == m_typeNum[m_currentMainIndex])
			m_subMainIndex = 0; // roll back to the first one

	}

	// send Change Private Layout Command
	SendChangeLayout();
}

//--------------------------------------------------------------------------
void CIvrSubPLC::SendChangeLayout()
{
	DWORD layoutCode = m_plcTableOpcodes[m_currentMainIndex][m_subMainIndex];
	TraceInt(eLevelInfoNormal, "CIvrSubPLC::SendChangeLayout - F_IVR===>PLC, Private-Action: ", layoutCode);

	if (layoutCode >= CP_NO_LAYOUT)
	{
		PTRACE(eLevelError, "CIvrSubMuteNoisyLine::SendChangeLayout - F_IVR===>PLC, layout overflow!!");
		return;
	}

	// send change layout
	if (m_pConfApi)
		m_pConfApi->PlcAction(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_CHANGE_LAYOUT_PLC, layoutCode);
}

//--------------------------------------------------------------------------
void CIvrSubPLC::PrepareTables()
{
	// prepare index table (MAX_PLC_TABLE_TYPES)

	for (int ind = 0; ind < MAX_PLC_TABLE_TYPES; ind++)
	{
		switch (ind)
		{
			case 0:                                         // 0
				m_typeNum[ind] = 1;                           // illegal entry
				if (m_typeNum[ind] > MAX_TYPE_ENTRIES) break;
					m_plcTableOpcodes[ind][0] = CP_LAYOUT_1X1;  // like 1
				break;

			case 1: // 1
				m_typeNum[ind]            = 1;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_1X1;
				break;

			case 2: // 2
				if (4 > MAX_TYPE_ENTRIES)
					break;

				m_typeNum[ind]            = 4;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_1x2VER;
				m_plcTableOpcodes[ind][1] = CP_LAYOUT_1x2HOR;
				m_plcTableOpcodes[ind][2] = CP_LAYOUT_2X1;
				m_plcTableOpcodes[ind][3] = CP_LAYOUT_1X2;
				break;

			case 3: // 3
				if (3 > MAX_TYPE_ENTRIES)
					break;

				m_typeNum[ind]            = 3;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_1P2VER;
				m_plcTableOpcodes[ind][1] = CP_LAYOUT_1P2HOR;
				m_plcTableOpcodes[ind][2] = CP_LAYOUT_1P2HOR_UP;
				break;

			case 4: // 4
				if (4 > MAX_TYPE_ENTRIES)
					break;

				m_typeNum[ind]            = 4;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_2X2;
				m_plcTableOpcodes[ind][1] = CP_LAYOUT_1P3VER;
				m_plcTableOpcodes[ind][2] = CP_LAYOUT_1P3HOR;
				m_plcTableOpcodes[ind][3] = CP_LAYOUT_1P3HOR_UP;
				break;

			case 5: // 5
				if (3 > MAX_TYPE_ENTRIES)
					break;

				m_typeNum[ind]            = 3;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_1P4VER;
				m_plcTableOpcodes[ind][1] = CP_LAYOUT_1P4HOR;
				m_plcTableOpcodes[ind][2] = CP_LAYOUT_1P4HOR_UP;
				break;

			case 6: // 6
				m_typeNum[ind]            = 1;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_1P5;
				break;

			case 7: // 7
				m_typeNum[ind]            = 1;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_1P5;    // 7 is like 6 !!!!
				break;

			case 8: // 8
				m_typeNum[ind]            = 1;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_1P7;
				break;

			case 9: // 9
				if (5 > MAX_TYPE_ENTRIES)
					break;

				m_typeNum[ind]            = 5;
				m_plcTableOpcodes[ind][0] = CP_LAYOUT_3X3;//modified by Richer for BRIDGE-9535
				m_plcTableOpcodes[ind][1] = CP_LAYOUT_1P7;
				m_plcTableOpcodes[ind][2] = CP_LAYOUT_2P8;
				m_plcTableOpcodes[ind][3] = CP_LAYOUT_1P12;
				m_plcTableOpcodes[ind][4] = CP_LAYOUT_4X4;
				break;
		} // switch
	}
}

//--------------------------------------------------------------------------
void CIvrSubPLC::OnTimerDTMF(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubPLC::OnTimerDTMF - F_IVR===>PLC, End of PLC ");

	// sends STOP_PLC command
	m_pConfApi->StartStopPLC(m_pParty->GetPartyRsrcID(), 0);  // stop command

	EndPlcFeature();
}

//--------------------------------------------------------------------------
void CIvrSubPLC::EndPlcFeature()
{
	// restore old DTMF opcodes table: create if needed
	m_pDtmf->SetDtmfTable(m_pIvrPrevTable); // can be NULL
	POBJDELETE(m_pIvrPrevTable);

	// end feature
	m_pDtmf->ResetDtmfBuffer();
	m_pDtmf->EndFeature();
	m_pDtmf->InformAboutEveryDTMFPressed(0);

	OnEndFeature(STATUS_OK);

	m_state = NOTACTIVE;
}

//--------------------------------------------------------------------------
int CIvrSubPLC::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}

//--------------------------------------------------------------------------
void CIvrSubPLC::PlayRetryMessage(DWORD messageMode)
{
}

//--------------------------------------------------------------------------
void CIvrSubPLC::OnStopIvr()
{
	EndPlcFeature();
}



PBEGIN_MESSAGE_MAP(CIvrSubCascadeLink)
	ONEVENT(TIMER_WAIT_FOR_DTMF, ACTIVE, CIvrSubCascadeLink::OnTimerDTMF)
	ONEVENT(DTMF_STRING_IDENT,   ACTIVE, CIvrSubBaseSM::OnDtmfString)
PEND_MESSAGE_MAP(CIvrSubCascadeLink, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubCascadeLink
////////////////////////////////////////////////////////////////////////////
CIvrSubCascadeLink::CIvrSubCascadeLink()
{
	m_startWaitForDigit = 0;
	m_confPW[0]         = '\0';
	m_NID[0]            = '\0';
	m_DtmfTimer1        = 120;
	m_DtmfTimer2        = 60;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubCascadeLink::~CIvrSubCascadeLink()
{
	if (m_startWaitForDigit)
		DeleteTimer(TIMER_WAIT_FOR_DTMF);
}

//--------------------------------------------------------------------------
void* CIvrSubCascadeLink::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubCascadeLink::Start()
{
	PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::Start ");
	if (!IsIvrPtrs())
		return;

	// init parameters
	m_DTMF_digits  = DTMF_MAX_BUFFER_SIZE-1; // was 16   ///tried 0
	m_DTMF_timeout = 120;                    // seconds
	m_msgDuration  = 0;
	char delimiter = ' ';                    // /tried '#'

	// init timer
	StartBaseTimer();
	StartTimer(TIMER_WAIT_FOR_DTMF, m_DtmfTimer1 * SECOND);
	m_startWaitForDigit = 1;

	m_state = ACTIVE;

	// init dtmf collector
	// m_pDtmf->ResetDtmfParams(); the DTMF can come before the feature starts
	m_pDtmf->SetDtmfWaitForXDigitsNumber(m_DTMF_digits, m_DTMF_timeout, m_msgDuration);
	m_pDtmf->SetDtmfDelimiter(delimiter);
	m_pDtmf->StartFeature();

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	if (!pCommConf)
	{
		PASSERT(2);
		PTRACE(eLevelError, "CIvrSubCascadeLink::Start - pCommConf=NULL  F_IVR===>NID ");
		return;
	}

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
	if (!pConfParty)
	{
		PASSERT(3);
		PTRACE(eLevelError, "CIvrSubCascadeLink::Start -  pConfParty=NULL  F_IVR===>NID ");
		return;
	}

	const char* szPreDefinedIvrString = pConfParty->GetPreDefinedIvrString();
	if (strncmp(szPreDefinedIvrString, "", DTMF_MAX_BUFFER_SIZE))
	{
		PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::Start - Start-szPreDefinedIvrString F_IVR===>NID");
		ParsePreDefinedIvrString(szPreDefinedIvrString);
		OnGetNID(1);
		return;
	}

	const int wDTMFIndex = m_pDtmf->GetDtmfIndex();
	if (wDTMFIndex > 0)       // the DTMF buffer is not empty
		OnDTMF();
}

//--------------------------------------------------------------------------
void CIvrSubCascadeLink::OnDTMF()
{
	if (m_startWaitForDigit)
		DeleteTimer(TIMER_WAIT_FOR_DTMF);

	// an error condition (# of DTMF exceeded without find the end string sign (##))
	if (m_pDtmf->GetDtmfIndex() > DTMF_MAX_BUFFER_SIZE-2) // not permit more than 30 digits
	{
		// end with error
		PTRACE(eLevelError, "CIvrSubCascadeLink::OnDTMF: DTMF exceeded  ");
		OnEndFeature(TOO_MANY_ERRORS);
		return;
	}

	// checking if the "end string" is received
	const char* pDTMFBuffer = m_pDtmf->GetDtmfBuffer();
	const int   wDTMFIndex  = m_pDtmf->GetDtmfIndex();

	// format can be either "NID#PW## or NID##
	if ((wDTMFIndex > 2) && (wDTMFIndex < DTMF_MAX_BUFFER_SIZE-1))
	{
		const char* ptemp = &pDTMFBuffer[wDTMFIndex-2];   // temp string pointer
		if (('#' == ptemp[0]) && ('#' == ptemp[1]))       // "##" end of string
		{
			OnGetNID();
			return;
		}
	}

	// after the first DTMF, we expect the other to come quicker
	PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::OnDTMF  Set timer again  ");
	StartTimer(TIMER_WAIT_FOR_DTMF, m_DtmfTimer2*SECOND); // set the timer again for the next DTMF
}

//--------------------------------------------------------------------------
void CIvrSubCascadeLink::OnTimerDTMF(CSegment* pParam)
{
	PTRACE(eLevelError, "CIvrSubCascadeLink::OnTimerDTMF ");
	// trace received DTMF
	if (m_pDtmf->GetDtmfIndex() > 0)
		PTRACE2(eLevelError, "CIvrSubCascadeLink::OnTimerDTMF - DTMF received  ", m_pDtmf->GetDtmfBuffer());
	else
		PTRACE(eLevelError, "CIvrSubCascadeLink::OnTimerDTMF - 0 DTMF received");

	OnEndFeature(TOO_MANY_ERRORS);
}

//--------------------------------------------------------------------------
void CIvrSubCascadeLink::OnGetNID(WORD isFromDialString)
{
	PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::OnGetNID  ");

	if (isFromDialString)
	{
		if (strlen(m_passwordForCascadeLink))
		{
			strncpy(m_confPW, m_passwordForCascadeLink, sizeof(m_confPW) - 1);
			m_confPW[sizeof(m_confPW) - 1] = '\0';
		}

		if (strlen(m_numericIdForCascadeLink))
		{
			strncpy(m_NID, m_numericIdForCascadeLink, sizeof(m_NID) - 1);
			m_NID[sizeof(m_NID) - 1] = '\0';
		}
	}
	else
		SetNIDAndPW();

	// search for conference with that NID and PW
	ETargetConfType targetConfType = eDummyType;
	DWORD dwConf_id = FindOnGoingConfAccordingNid();
	if (dwConf_id != 0xFFFFFFFF)
		targetConfType = eOnGoingConf;
	else
	{
		WORD nidOfEqOrSipFactory = 0;
		dwConf_id = FindMRAccordingToNid(nidOfEqOrSipFactory);
		if (dwConf_id != 0xFFFFFFFF)
			targetConfType = eMeetingRoom;
		else if (1 == nidOfEqOrSipFactory)
		{
			PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::OnGetNID: Numeric id was found in EQ or SIP Factory, request is denied!!");
			OnEndFeature(TOO_MANY_ERRORS);
			return;
		}
	}

	if ((dwConf_id != 0xFFFFFFFF) && (targetConfType != eDummyType))    // conference was found
	{
		BOOL bEnableCascadeJoinWithoutPw = 1;                      // for DIAL_OUT cascade link - PW is never required
		std::string key = CFG_KEY_ENABLE_CASCADED_LINK_TO_JOIN_WITHOUT_PASSWORD;
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		sysConfig->GetBOOLDataByKey(key, bEnableCascadeJoinWithoutPw);

		if ((bEnableCascadeJoinWithoutPw == 1) ||           // Cascaded link join without PW
		    (IsPasswordOk(dwConf_id, targetConfType) == 1)) // No PW is needed or PW OK
		{
			PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::OnGetNID: No PW is needed or PW OK ");
			MovePartyToItsConf(dwConf_id, targetConfType);
		}
		else  // exists but not match
		{
			PTRACE(eLevelError, "CIvrSubCascadeLink::OnGetNID: Illegal PW  ");
			OnEndFeature(TOO_MANY_ERRORS);
			return;
		}
	}
	else  // target conference not found
	{
		PTRACE(eLevelError, "CIvrSubCascadeLink::OnGetNID: conf not found  ");
		OnEndFeature(TOO_MANY_ERRORS);
		return;
	}

	// end feature OK
	OnEndFeature(STATUS_OK);
}

//--------------------------------------------------------------------------
void CIvrSubCascadeLink::SetNIDAndPW()
{
	const char* phoneNumber = m_pDtmf->GetDtmfBuffer();

	// gets the NID
	int ind = 0;
	for (ind = 0; ind < (DTMF_MAX_BUFFER_SIZE-1); ind++)
	{
		if (phoneNumber[ind] == '#')
			break;

		m_NID[ind] = phoneNumber[ind];
	}

	m_NID[ind] = '\0';

	// gets the PW
	int pwInd = 0;
	ind++;
	for (; ind < (DTMF_MAX_BUFFER_SIZE-1); ind++)
	{
		if (phoneNumber[ind] == '#')  // the second one
			break;

		m_confPW[pwInd++] = phoneNumber[ind];
	}

	m_confPW[pwInd] = '\0';
}

//--------------------------------------------------------------------------
void CIvrSubCascadeLink::MovePartyToItsConf(DWORD dwConf_id, ETargetConfType targetConfType)
{
	// move party to its destination conference
	CConfPartyManagerLocalApi confPartyManagerLocalApi;
	DWORD dwSourceConfId = m_ivrCntl->GetMonitorConfId(); // current EQ conf-ID
	DWORD dwPartId       = m_pParty->GetMonitorPartyId(); // party-ID

	PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::MovePartyToItsConf: move party to conf  ");

	// move party to its destination conference
	if (targetConfType == eOnGoingConf)
	{
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(dwConf_id);
		if (pCommConf)
		{
			CConfApi* pDestConfApi = new CConfApi;
			pDestConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()), NULL, NULL, 1);
			STATUS    rspStatus = STATUS_OK;
			OPCODE    rspOpcode = STATUS_OK;
			CSegment  rspMsg;
			rspStatus = pDestConfApi->IsDestConfReadyForMove(rspMsg, 5*SECOND, rspOpcode);
			pDestConfApi->DestroyOnlyApi();
			POBJDELETE(pDestConfApi);

			if ((rspStatus != STATUS_OK) || (rspOpcode != STATUS_OK))
			{
				PTRACE2(eLevelError, "CIvrSubCascadeLink::MovePartyToItsConf: Error: Conf is not ready ", pCommConf->GetName());
				OnEndFeature(TOO_MANY_ERRORS);    // disconnect the Party
				return;
			}
		}
	}

	// inform CAM on start Move
	CSegment* pSeg = new CSegment;
	*pSeg << m_pParty->GetName();
	m_pConfApi->SendCAMGeneralNotifyCommand((DWORD)0, IVR_PARTY_START_MOVE_FROM_EQ, pSeg);
	POBJDELETE(pSeg);
	// move party to its destination conference
	confPartyManagerLocalApi.MovePartyToConfOrMeetingRoom(dwSourceConfId, dwPartId, targetConfType, dwConf_id, NULL);
}

//--------------------------------------------------------------------------
int CIvrSubCascadeLink::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}

//--------------------------------------------------------------------------
void CIvrSubCascadeLink::PlayRetryMessage(DWORD messageMode)
{
}

//--------------------------------------------------------------------------
WORD CIvrSubCascadeLink::DoSomething()
{
	return 0;
}

//--------------------------------------------------------------------------
DWORD CIvrSubCascadeLink::FindOnGoingConfAccordingNid()
{
	DWORD wResult = 0xFFFFFFFF;

	if (m_NID == NULL)
		return wResult;

	if (m_NID[0] == '\0') // empty string
		return wResult;

	CCommConfDB* pConfDB   = ::GetpConfDB();
	CCommConf*   pCommConf = pConfDB->GetFirstCommConf();

	// Search on going conference database
	while (pCommConf != NULL)
	{
		if ((pCommConf->GetMeetMePerEntryQ()) && !pCommConf->GetEntryQ())
		{
			if (pCommConf->GetNumericConfId() != NULL)
			{
				if (!strncmp(m_NID, pCommConf->GetNumericConfId(), NUMERIC_CONFERENCE_ID_LEN))
				{
					if (pCommConf->IsDefinedIVRService())
					{
						PTRACE2(eLevelInfoNormal, "CIvrSubCascadeLink::FindOnGoingConfAccordingNid - F_IVR===>NID found Conf, party = ", PARTYNAME);
						wResult = pCommConf->GetMonitorConfId();
						break;
					}
					else
					{
						PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::FindOnGoingConfAccordingNid - Destination Conf has no IVR F_IVR===>NID");
						return 0xFFFFFFFF; // If the conference has no IVR, return 0xFFFFFFFF
					}
				}
			}
		}

		pCommConf = pConfDB->GetNextCommConf();
	}

	return wResult;
}

//--------------------------------------------------------------------------
DWORD CIvrSubCascadeLink::FindMRAccordingToNid(WORD& nidOfEq)
{
	DWORD wResult = 0xFFFFFFFF;

	if (!m_NID || m_NID[0] == '\0')   // empty string
		return wResult;

	// Search the meeting room database

	CCommResDB* pMeetingRoomDB = ::GetpMeetingRoomDB();

	if (pMeetingRoomDB)
	{
		const ReservArray& rsrvArray = pMeetingRoomDB->GetReservArray();
		ReservArray::const_iterator iter_end = rsrvArray.end();
		for (ReservArray::const_iterator iter = rsrvArray.begin(); iter != iter_end; ++iter)
		{
			if (NULL != (*iter))
			{
				if ((*iter)->GetNumericConfId() != NULL)
				{
					if (!strncmp(m_NID, (*iter)->GetNumericConfId(), NUMERIC_CONFERENCE_ID_LEN))
					{
						if (!(*iter)->IsEntryQ())
						{
							DWORD     tempWResult = (*iter)->GetConferenceId();
							CCommRes* pCommRes    = pMeetingRoomDB->GetCurrentRsrv(tempWResult); // Allocation within GetCurrentRsrv function
							if (pCommRes)
							{
								PTRACE2(eLevelInfoNormal, "CIvrSubCascadeLink::FindMRAccordingToNid - F_IVR===>NID found MR, party = ", PARTYNAME);
								wResult = tempWResult;
								POBJDELETE(pCommRes);
								break;
							}
							else
							{
								PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::FindMRAccordingToNid - pCommRes of Destination MR is NULL, F_IVR===>NID");
								return 0xFFFFFFFF; // If pCommRes is NULL, return 0xFFFFFFFF
							}
						}
						else                   // if the MR that was found is an EQ
						{
							PTRACE(eLevelInfoNormal, "CIvrSubCascadeLink::FindMRAccordingToNid - Numeric ID was found in EQ, F_IVR===>NID");
							nidOfEq = 1;
						}
					}
				}
			}
		}
	}

	return wResult;
}

//--------------------------------------------------------------------------
int CIvrSubCascadeLink::IsPasswordOk(DWORD dwConf_id, ETargetConfType targetConfType)
{
	int          isPWOkay = 1;

	CCommResDB*  pMeetingRoomDB = NULL;
	CCommConfDB* pConfDB        = NULL;
	CCommRes*    pCommRes       = NULL;

	if (targetConfType == eMeetingRoom)
	{
		pMeetingRoomDB = ::GetpMeetingRoomDB();
		pCommRes       = pMeetingRoomDB->GetCurrentRsrv(dwConf_id);
	}
	else
	{
		pConfDB  = ::GetpConfDB();
		pCommRes = pConfDB->GetCurrentConf(dwConf_id);
	}

	if (pCommRes)
	{
		// Check if PW feature is enabled in the IVR service
		BYTE        bPwFeatureEnabled = 0;
		const char* ivrServiceName    = pCommRes->GetpAvMsgStruct()->GetAvMsgServiceName();
		if (ivrServiceName)
		{
			CIVRService* pIVRService = ::GetpAVmsgServList()->GetIVRServiceByName(ivrServiceName);
			if (pIVRService)
				bPwFeatureEnabled = pIVRService->GetConfPasswordFeature()->GetEnableDisable();
		}

		const char* entryPW = pCommRes->GetEntryPassword();
		if ((entryPW != NULL) && bPwFeatureEnabled)
		{
			if (entryPW[0] != 0)                      // PW as set in target conference
			{
				isPWOkay = 0;
				if (m_confPW[0] != 0)                   // PW as received from party
				{
					if (0 == strcmp(m_confPW, entryPW))   // found the target conference
						isPWOkay = 1;
				}
			}
		}
	}

	return isPWOkay;
}


PBEGIN_MESSAGE_MAP(CIvrSubOperatorAssist)
PEND_MESSAGE_MAP(CIvrSubOperatorAssist, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubOperatorAssist
////////////////////////////////////////////////////////////////////////////
CIvrSubOperatorAssist::CIvrSubOperatorAssist(const CIVROperAssistanceFeature* pOperAssist, DWORD request_type) : m_request_type(request_type)
{
	m_pOperAssistFeature = new CIVROperAssistanceFeature(*pOperAssist);

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubOperatorAssist::~CIvrSubOperatorAssist()
{
	POBJDELETE(m_pOperAssistFeature);
}

//--------------------------------------------------------------------------
void* CIvrSubOperatorAssist::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
const char* CIvrSubOperatorAssist::NameOf() const
{
	return "CIvrSubOperatorAssist";
}

//--------------------------------------------------------------------------
void CIvrSubOperatorAssist::Start()
{
	PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Start: [operator_assistance_trace] Operator Help ---> ");
	if (!IsIvrPtrs())
	{
		PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Start - [operator_assistance_trace] Operator IsIvrPtrs failed");
		return;
	}

	BYTE  is_set_in_service      = m_pOperAssistFeature->GetWaitForOperatorOnErrorOnOff();
	BYTE  is_operator_conf_exist = m_pOperAssistFeature->m_enable_disable;

	DWORD repeated              = IVR_STATUS_PLAY_ONCE;
	BYTE  failed_on_error       = 0;
	BYTE  end_collector_feature = 0;

	switch (m_request_type)
	{
		case IVR_EVENT_WAIT_FOR_OPER_ON_CONF_PWD_FAIL:
		case IVR_EVENT_WAIT_FOR_OPER_ON_CHAIR_PWD_FAIL:
		case IVR_EVENT_WAIT_FOR_OPER_ON_NID_FAIL:
		{
			repeated        = IVR_STATUS_PLAY_LOOP;
			failed_on_error = 1;
			break;
		}

		case IVR_EVENT_WAIT_FOR_OPER_ON_PARTY_PRIVATE_REQ:
		case IVR_EVENT_WAIT_FOR_OPER_ON_PARTY_PUBLIC_REQ:
		{
			// repeated = IVR_STATUS_PLAY_ONCE;
			end_collector_feature = 1; // party hear message once and can continue receive dtmf commands
			break;
		}

		default:
		{
			PTRACE2INT(eLevelInfoNormal, "CIvrSubOperatorAssist::Start [operator_assistance_trace] wrong request_type ", m_request_type);
			// disconnect
			break;
		}
	} // end switch

	if (!is_set_in_service)
	{
		if (failed_on_error)
		{
			PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Start [operator_assistance_trace] OperatorAssistent is not set in service - disconnect");
			Disconnect();
			return;
		}
		else
		{
			PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Start [operator_assistance_trace] OperatorAssistent is not set in service - ignore - no message");
		}
	}

	if (!is_operator_conf_exist)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Start [operator_assistance_trace] OperatorAssistent is operator conf not exist (OperatorAssistent on)");
	}

	m_DTMF_digits      = 0;
	m_DTMF_timeout     = 0;
	m_pinCodeDelimiter = ' ';
	// get file name
	int         status    = STATUS_OK;
	const char* file_name = m_pOperAssistFeature->GetMsgFileName(IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE, m_language, status);
	if (file_name)
	{
		strncpy(m_msg_file_name, file_name, sizeof(m_msg_file_name) - 1);
		m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::WaitForOperator - empty file name [operator_assistance_trace]");
	}

	m_feature_opcode = IVR_FEATURE_OPER_ASSISTANCE;
	m_event_opcode   = IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE;


	if (repeated == IVR_STATUS_PLAY_LOOP)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Start [operator_assistance_trace] IVR_STATUS_PLAY_LOOP ");
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Start [operator_assistance_trace] IVR_STATUS_PLAY_ONCE ");
	}

	WORD wait_between_repeats = 25; // 25 seconds

	StartBase(FALSE, repeated, wait_between_repeats);

	// in case public help need to free dtmf collector for next feature
	if (end_collector_feature)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Start: EndFeature [operator_assistance_trace]");
		m_pDtmf->EndFeature();
		EndFeature(STATUS_OK);
	}
}

//--------------------------------------------------------------------------
void CIvrSubOperatorAssist::PlayRetryMessage(DWORD messageMode)
{
}

//--------------------------------------------------------------------------
void CIvrSubOperatorAssist::Disconnect()
{
	// play disconnect message
	PTRACE(eLevelInfoNormal, "CIvrSubOperatorAssist::Disconnect  ");

	// disconnect
	m_pDtmf->EndFeature();
	EndFeature(1);        // end with error
}

//--------------------------------------------------------------------------
int CIvrSubOperatorAssist::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}



PBEGIN_MESSAGE_MAP(CIvrSubInvite)
	ONEVENT(DTMF_STRING_IDENT,   NOTACTIVE, CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT,   ACTIVE,    CIvrSubBaseSM::OnDtmfString)
	ONEVENT(TIMER_RETRY_MESSAGE, ACTIVE,    CIvrSubBaseSM::OnTimeRetryMessage)
PEND_MESSAGE_MAP(CIvrSubInvite, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubInvite
////////////////////////////////////////////////////////////////////////////
CIvrSubInvite::CIvrSubInvite()
{
	m_error = 0;
	OFF(m_need_to_stop_previous_msg);
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubInvite::~CIvrSubInvite()
{
}

//--------------------------------------------------------------------------
void* CIvrSubInvite::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
const char* CIvrSubInvite::NameOf() const
{
	return "CIvrSubInvite";
}

//--------------------------------------------------------------------------
void CIvrSubInvite::Start()
{
	PTRACE2(eLevelInfoNormal, "CIvrSubInvite::Start - F_IVR===>Invite ", m_pParty->GetName());
	if (!IsIvrPtrs())
		return;

	// init timer
	StartBaseTimer();

	m_feature_opcode = IVR_FEATURE_GENERAL;
	m_event_opcode   = IVR_EVENT_ENTER_DEST_NUM;
	m_bStopUponDTMF  = TRUE;
	m_DTMF_digits    = 0;
	m_DTMF_timeout   = m_pIvrService->GetUserInputTimeout();
	if (m_DTMF_timeout == 0)
		m_DTMF_timeout = 1;

	m_pinCodeDelimiter = GetIvrDelimiter('#');
	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);
	GetIVRMessageParameters(msgFullPath, &msgDuration, &msgCheckSum);   // update the duration

	m_msgDuration = GetMsgDuration(m_pIvrService->GetGeneralMsgsFeature(), m_event_opcode);

	// start feature
	StartBase(FALSE, IVR_STATUS_PLAY_ONCE);
}

//--------------------------------------------------------------------------
int CIvrSubInvite::CheckIfIvrMsgsExistInFeature()
{
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CIvrSubInvite::IsLegalString(CSegment* pParam)
{
	PTRACE(eLevelError, "CIvrSubInvite::IsLegalString - F_IVR===>Invite");

	WORD opcode;
	WORD rDigits;
	if (0 != GetDtmfResult(pParam, opcode, rDigits))
	{
		PTRACE(eLevelError, "CIvrSubInvite::IsLegalString: Illegal DTMF string F_IVR===>Invite");
		if (rDigits == 0) // no digit in the string
		{
			return DTMF_TIMEOUT_ERROR;
		}

		return DTMF_STRING_ERROR; // opcode error or string length error
	}

	if (!m_pParty) // should be valid
	{
		PASSERT(1);
		PTRACE(eLevelError, "CIvrSubInvite::IsLegalString: Illegal Party Internal Pointer F_IVR===>Invite");
		return DTMF_STRING_ERROR;
	}

	if (opcode == OP_NOOP)    // string was requested
	{
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
		if (!pCommConf)
		{
			PTRACE2(eLevelInfoNormal, "CIvrSubInvite::IsLegalString -  pCommConf = NULL F_IVR===>Invite", m_pParty->GetName());
			return DTMF_STRING_ERROR;
		}

		if (0 != m_dtmf[0])
		{
			PTRACE2(eLevelError, "CIvrSubInvite::IsLegalString: DTMF string F_IVR===>Invite = ", (const char*)m_dtmf);
			// need to replace all the '*' to '.' except the first (to distiguish between PSTN *NUM and IP 1*2*3*4)
			for (WORD i = 1; i < rDigits; i++)
			{
				if (m_dtmf[i] == '*')
					m_dtmf[i] = '.';
			}

			PTRACE2(eLevelError, "CIvrSubInvite::IsLegalString: DTMF string After manipulation: ", (const char*)m_dtmf);

			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
			confApi.SetDialStringForGW((const char*)m_dtmf);
			confApi.DestroyOnlyApi();
			return DTMF_OK;
		}
		else
		{
			PTRACE(eLevelError, "CIvrSubInvite::IsLegalString - User entered empty Password ");
			return DTMF_STRING_ERROR;
		}
	}
	else
		return DTMF_STRING_ERROR;
}

//--------------------------------------------------------------------------
WORD CIvrSubInvite::CheckLeagalPW()
{
	// all OK
	return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CIvrSubInvite::DoSomething()
{
	m_ivrCntl->SetStartNextFeature();
	return 0;
}

//--------------------------------------------------------------------------
void CIvrSubInvite::PlayRetryMessage(DWORD messageMode)
{
	PTRACE(eLevelInfoNormal, "CIvrSubInvite::PlayRetryMessage");

	// Allocation of messages id array for the sequence message
	IVRMsgDescriptor* pArrayOfMessagesToPlay = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];
	const char*       ivrServiceName         = m_pIvrService->GetName();
	m_retryMessageDuration = 0;
	// Initialize the array
	for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
	{
		pArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
		pArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
		pArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
	}

	WORD wNumberOfMessagesToPlay = 0;

	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);

	int  status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, IVR_FEATURE_GENERAL, IVR_EVENT_ILLEGAL_DEST_NUM,
	                                                     msgFullPath, &msgDuration, &msgCheckSum);

	if (STATUS_OK != status)
	{
		msgFullPath[0] = '\0';
		msgDuration    = 0;
		msgCheckSum    = (WORD)(-1);
		PTRACE(eLevelError, "CIvrSubInvite::PlayRetryMessage( incorrect_destination_id message ) - F_IVR===>Invite, GetIVRMsgParams failed");
	}
	else
	{
		strncpy(pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, msgFullPath, MAX_FULL_PATH_LEN);
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
		wNumberOfMessagesToPlay++;
	}

	m_retryMessageDuration += msgDuration;
	msgFullPath[0] = '\0';
	msgDuration    = 0;
	msgCheckSum    = (WORD)(-1);

	status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, IVR_FEATURE_GENERAL, IVR_EVENT_PLAY_DIAL_TONE,
	                                                msgFullPath, &msgDuration, &msgCheckSum);

	if (STATUS_OK != status)
	{
		msgFullPath[0] = '\0';
		msgDuration    = 0;
		msgCheckSum    = (WORD)(-1);
		PTRACE(eLevelError, "CIvrSubInvite::PlayRetryMessage( dial_tone message ) - F_IVR===>Invite, GetIVRMsgParams failed");
	}
	else
	{
		strncpy(pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, msgFullPath, MAX_FULL_PATH_LEN);
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
		wNumberOfMessagesToPlay++;
	}

	if (0 != wNumberOfMessagesToPlay)
		m_pConfApi->StartSequenceMessages(m_pParty->GetPartyRsrcID(), pArrayOfMessagesToPlay, wNumberOfMessagesToPlay, PRIVATE_MSG);
	else
		PTRACE(eLevelError, "CIvrSubInvite::PlayRetryMessage - F_IVR===>Invite, Invite messages are not configured");

	PDELETEA(pArrayOfMessagesToPlay);
}

//--------------------------------------------------------------------------
void CIvrSubInvite::PlayMessage(DWORD messageMode, WORD loop_delay)
{
	PTRACE(eLevelInfoNormal, "CIvrSubInvite::PlayMessage");
	// Allocation of messages id array for the sequence message
	IVRMsgDescriptor* pArrayOfMessagesToPlay = new IVRMsgDescriptor[MAX_SUB_MESSAGE_NUMBER];
	const char*       ivrServiceName         = m_pIvrService->GetName();

	// Initialize the array
	for (int i = 0; i < MAX_SUB_MESSAGE_NUMBER; i++)
	{
		pArrayOfMessagesToPlay[i].ivrMsgFullPath[0] = '\0';
		pArrayOfMessagesToPlay[i].ivrMsgDuration    = 0;
		pArrayOfMessagesToPlay[i].ivrMsgCheckSum    = (WORD)(-1);
	}

	WORD wNumberOfMessagesToPlay = 0;

	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);

	int  status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, IVR_FEATURE_GENERAL, IVR_EVENT_ENTER_DEST_NUM,
	                                                     msgFullPath, &msgDuration, &msgCheckSum);

	if (STATUS_OK != status)
	{
		msgFullPath[0] = '\0';
		msgDuration    = 0;
		msgCheckSum    = (WORD)(-1);
		PTRACE(eLevelError, "CIvrSubInvite::PlayMessage( enter_destination_id message) - F_IVR===>Invite, GetIVRMsgParams failed");
	}
	else
	{
		strncpy(pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, msgFullPath, MAX_FULL_PATH_LEN);
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
		wNumberOfMessagesToPlay++;
	}

	m_msgDuration = msgDuration;

	msgFullPath[0] = '\0';
	msgDuration    = 0;
	msgCheckSum    = (WORD)(-1);

	status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, IVR_FEATURE_GENERAL, IVR_EVENT_PLAY_DIAL_TONE,
	                                                msgFullPath, &msgDuration, &msgCheckSum);

	if (STATUS_OK != status)
	{
		msgFullPath[0] = '\0';
		msgDuration    = 0;
		msgCheckSum    = (WORD)(-1);
		PTRACE(eLevelError, "CIvrSubInvite::PlayMessage( dial_tone message ) - F_IVR===>Invite, GetIVRMsgParams failed");
	}
	else
	{
		strncpy(pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgFullPath, msgFullPath, MAX_FULL_PATH_LEN);
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgDuration = msgDuration;
		pArrayOfMessagesToPlay[wNumberOfMessagesToPlay].ivrMsgCheckSum = msgCheckSum;
		wNumberOfMessagesToPlay++;
	}

	// Play the sequence message in loop
	if (0 != wNumberOfMessagesToPlay)
	{
		if (m_need_to_stop_previous_msg)
			m_pConfApi->StopMessage(m_pParty->GetPartyRsrcID());

		m_pConfApi->StartSequenceMessages(m_pParty->GetPartyRsrcID(), pArrayOfMessagesToPlay, wNumberOfMessagesToPlay, PRIVATE_MSG);
		ON(m_need_to_stop_previous_msg);
	}
	else
		PTRACE(eLevelError, "CIvrSubInvite::PlayMessage - F_IVR===>Invite, Invite messages are not configured");

	PDELETEA(pArrayOfMessagesToPlay);
}



PBEGIN_MESSAGE_MAP(CIvrSubNoVideoResources)
	ONEVENT(DTMF_STRING_IDENT,                 NOTACTIVE, CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT,                 ACTIVE,    CIvrSubBaseSM::OnDtmfString)
	ONEVENT(TIMER_WAIT_FOR_NO_VIDEO_RESOURCES, ACTIVE,    CIvrSubNoVideoResources::ContinueNoVideoResources)
	ONEVENT(FIRST_DTMF_WAS_DIALED,             ACTIVE,    CIvrSubNoVideoResources::OnDTMFReceivedACTIVE)
PEND_MESSAGE_MAP(CIvrSubNoVideoResources, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubNoVideoResources
////////////////////////////////////////////////////////////////////////////
CIvrSubNoVideoResources::CIvrSubNoVideoResources(WORD messageType)
{
	m_messageType = messageType;
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubNoVideoResources::~CIvrSubNoVideoResources()
{
}

//--------------------------------------------------------------------------
void* CIvrSubNoVideoResources::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubNoVideoResources::Start()
{
	PTRACE(eLevelInfoNormal, "CIvrSubNoVideoResources::Start - F_IVR===>No Video Resources,  ");
	if (!IsIvrPtrs())
		return;

	// set flag if start of IVR session
	m_pDtmf->SetIvrPartSessionOn();

	m_DTMF_digits      = 0;
	m_DTMF_timeout     = 0;
	m_pinCodeDelimiter = ' ';

	m_feature_opcode = IVR_FEATURE_GENERAL;
	m_event_opcode   = IVR_EVENT_NO_VIDEO_RESOURCES;
	// m_event_opcode   = IVR_EVENT_CONF_LOCK;
	int status = STATUS_OK;

	// update duration if needed
	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);
	GetIVRMessageParameters(msgFullPath, &msgDuration, &msgCheckSum);   // update the duration

	// get the file name (?what to do in case there is no filename?...)
	int notOK = 1;
	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	if (generalMsgs)
	{
		int         status    = STATUS_OK;
		const char* file_name = generalMsgs->GetMsgFileName(m_event_opcode, m_language, status);
		if (file_name && *file_name)
		{
			strncpy(m_msg_file_name, file_name, sizeof(m_msg_file_name) - 1);
			m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
			m_messagesName = m_msg_file_name; // to find upon the message entry index
			notOK          = 0;
		}

		m_msgDuration = GetMsgDuration((CIVRGeneralMsgsFeature*)generalMsgs, m_event_opcode);
	}

	if (1 == notOK) // message "No Video Resources" not exists or problem with "general" feature
	{
		PTRACE(eLevelError, "CIvrSubNoVideoResources::Start - F_IVR===>No Video Resources- Error, skiping this feature ");
		ContinueNoVideoResources(NULL);
		return;
	}

	if (m_msgDuration < 2)
		m_msgDuration = 5;

	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
		m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	// start feature
	StartBase(FALSE, IVR_STATUS_PLAY_ONCE);
	m_pDtmf->InformAboutFirstNumbPressed();
	StartTimer(TIMER_WAIT_FOR_NO_VIDEO_RESOURCES, (m_msgDuration+1) * SECOND);
}

//--------------------------------------------------------------------------
void CIvrSubNoVideoResources::OnDTMFReceivedACTIVE(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubNoVideoResources::OnDTMFNumberReceivedCOLLECT - F_IVR===>Welcome");

	DeleteTimer(TIMER_WAIT_FOR_NO_VIDEO_RESOURCES); // delete timer
	ContinueNoVideoResources(NULL);                 // continue to the next feature
}

//--------------------------------------------------------------------------
void CIvrSubNoVideoResources::ContinueNoVideoResources(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubNoVideoResources::ContinueNoVideoResources - F_IVR===>ContinueNoVideoResources");

	// next feature
	m_ivrCntl->SetStartNextFeature();
	m_pDtmf->EndFeature();
	EndFeature(0);
}

//--------------------------------------------------------------------------
int CIvrSubNoVideoResources::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}

//--------------------------------------------------------------------------
void CIvrSubNoVideoResources::PlayRetryMessage(DWORD messageMode)
{
}


PBEGIN_MESSAGE_MAP(CIvrSubVenus)
	ONEVENT(DTMF_STRING_IDENT,     ACTIVE,    CIvrSubVenus::OnDTMF)
	ONEVENT(DTMF_STRING_IDENT,     NOTACTIVE, CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(TIMER_WAIT_FOR_DTMF,   ACTIVE,    CIvrSubVenus::OnTimerDTMF)
	ONEVENT(FIRST_DTMF_WAS_DIALED, NOTACTIVE, CIvrSubBaseSM::OnIvrSubBaseNull)
PEND_MESSAGE_MAP(CIvrSubVenus, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubVenus
////////////////////////////////////////////////////////////////////////////
CIvrSubVenus::CIvrSubVenus()
{
	m_pIvrPrevTable    = NULL;
	m_currentMainIndex = 0;
	m_subMainIndex     = 0;

	for (int ind = 0; ind < MAX_PLC_TABLE_TYPES; ind++)
	{
		m_typeNum[ind]            = 1;
		m_plcTableOpcodes[ind][0] = CP_LAYOUT_1X1;  // just default legal value
	}

	PrepareTables();
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubVenus::~CIvrSubVenus()
{
	POBJDELETE(m_pIvrPrevTable);
	m_pDtmf->InformAboutEveryDTMFPressed(0);
}

//--------------------------------------------------------------------------
void* CIvrSubVenus::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubVenus::RejectVenus()
{
	PTRACE(eLevelInfoNormal, "CIvrSubVenus::RejectVenus - F_IVR===>Venus, Reject Venus ");
	EndVenusFeature();
}

//--------------------------------------------------------------------------
void CIvrSubVenus::Start()
{
	PTRACE(eLevelInfoNormal, "CIvrSubVenus::Start - F_IVR===>Venus  ");
	if (!IsIvrPtrs())
		return;

	m_state       = ACTIVE;
	m_DTMF_digits = 4;  // expected to 4 DTMF

	// saves previous DTMF table
	POBJDELETE(m_pIvrPrevTable);

	CDTMFCodeList* pTempIvrPrevTable = m_pDtmf->GetDtmfCodeList();
	if (pTempIvrPrevTable)
		m_pIvrPrevTable = new CDTMFCodeList(*pTempIvrPrevTable);    // allocate

	// insert empty DTMF table
	m_pDtmf->SetDtmfTable(NULL);

	// sign as in-feature
	m_pDtmf->ResetDtmfParams();
	m_pDtmf->StartFeature();
	m_pDtmf->SetDtmfWaitForXDigitsNumber(4, 8, 0);

	// sends START_Venus command
	m_pConfApi->StartStopVenus(m_pParty->GetPartyRsrcID(), 1);

	// starts Timer
	StartTimer(TIMER_WAIT_FOR_DTMF, 8 * SECOND);
}

//--------------------------------------------------------------------------
void CIvrSubVenus::OnDTMF(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubVenus::OnDTMF - F_IVR===>Venus  - begin");

	// end timer (wait for DTMF)
	DeleteTimer(TIMER_WAIT_FOR_DTMF);

	// get the DTMF
	WORD       opcode, nDigits;
	CMedString sLog;
	int        status = GetDtmfResult(pParam, opcode, nDigits);
	sLog << "opcode = " << opcode << ";   nDigits = " << nDigits;
	if (0 != status)
		PTRACE2(eLevelError, "CIvrSubVenus::OnDTMF - F_IVR===>Venus, Error", PARTYNAME);
	else
	{
		std::string sTmp;
		for (int i = 0; i < nDigits && i < 4; ++i)
		{
			sTmp    += (char)(m_dtmf[i]);
			m_sDTMF += (char)(m_dtmf[i]);
		}

		sLog << ";   dtmf string = " << sTmp.c_str() << ";   m_sDTMF = " << m_sDTMF.c_str();
		PTRACE2(eLevelInfoNormal, "CIvrSubVenus::OnDTMF : ", sLog.GetString());
		if (m_sDTMF.size() >= 4)
		{
			DoVenusAction();
			m_pConfApi->StartStopVenus(m_pParty->GetPartyRsrcID(), 0);  // stop command
			EndVenusFeature();
			PTRACE(eLevelInfoNormal, "CIvrSubVenus::OnDTMF - return");
			return;
		}
	}

	// starts Timer
	StartTimer(TIMER_WAIT_FOR_DTMF, 8 * SECOND);
	PTRACE(eLevelInfoNormal, "CIvrSubVenus::OnDTMF - end");
}

//--------------------------------------------------------------------------
void CIvrSubVenus::DoVenusAction()
{
	PTRACE(eLevelInfoNormal, "CIvrSubVenus::DoVenusAction - begin");

	if (m_sDTMF.substr(0, 2) == "01")
		DoChangeLayoutAction(m_sDTMF.substr(2, 2));

	PTRACE(eLevelInfoNormal, "CIvrSubVenus::DoVenusAction - begin");
}

//--------------------------------------------------------------------------
void CIvrSubVenus::DoChangeLayoutAction(const std::string& sLayoutCode)
{
	PTRACE(eLevelInfoNormal, "CIvrSubVenus::DoChangeLayoutAction - begin");
	PTRACE2(eLevelInfoNormal, "CIvrSubVenus::DoChangeLayoutAction - F_IVR===>Venus, Layout code:  ", sLayoutCode.c_str());

	if (sLayoutCode == "00")
	{
		if (m_pConfApi)
		{
			// 1. cancel all private force
			m_pConfApi->VenusAction(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_FORCE_VENUS, 0);
			// 2. return to conf layout
			m_pConfApi->VenusAction(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_VENUS, CAM_CONF_LAYOUT);
		}

		PTRACE(eLevelInfoNormal, "CIvrSubVenus::DoChangeLayoutAction - Set Conf Layout - return");
		return;
	}

	DWORD dwLayoutCode = m_mapLayouts[sLayoutCode];
	TraceInt(eLevelInfoNormal, "CIvrSubVenus::SendChangeLayout - F_IVR===>Venus, Private-Action: ", dwLayoutCode);

	if (dwLayoutCode >= CP_NO_LAYOUT)
	{
		PTRACE(eLevelError, "CIvrSubMuteNoisyLine::SendChangeLayout - F_IVR===>Venus, layout overflow!! - return");
		return;
	}

	// send change layout
	if (m_pConfApi)
		m_pConfApi->VenusAction(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_CHANGE_LAYOUT_VENUS, dwLayoutCode);

	PTRACE(eLevelInfoNormal, "CIvrSubVenus::DoChangeLayoutAction - end");
}

//--------------------------------------------------------------------------
void CIvrSubVenus::PrepareTables()
{
	m_mapLayouts["01"] = CP_LAYOUT_1X1;
	m_mapLayouts["02"] = CP_LAYOUT_1X2;
	m_mapLayouts["03"] = CP_LAYOUT_2X1;
	m_mapLayouts["04"] = CP_LAYOUT_1x2HOR;
	m_mapLayouts["05"] = CP_LAYOUT_1x2VER;
	m_mapLayouts["06"] = CP_LAYOUT_1P2VER;
	m_mapLayouts["07"] = CP_LAYOUT_1P2HOR;
	m_mapLayouts["08"] = CP_LAYOUT_1P2HOR_UP;
	m_mapLayouts["09"] = CP_LAYOUT_2X2;
	m_mapLayouts["10"] = CP_LAYOUT_1P3VER;
	m_mapLayouts["11"] = CP_LAYOUT_1P3HOR;
	m_mapLayouts["12"] = CP_LAYOUT_1P3HOR_UP;
	m_mapLayouts["13"] = CP_LAYOUT_1P5;
	m_mapLayouts["14"] = CP_LAYOUT_1P4VER;
	m_mapLayouts["15"] = CP_LAYOUT_1P4HOR;
	m_mapLayouts["16"] = CP_LAYOUT_1P4HOR_UP;
	m_mapLayouts["17"] = CP_LAYOUT_1P7;
	m_mapLayouts["18"] = CP_LAYOUT_3X3;
	m_mapLayouts["19"] = CP_LAYOUT_1P8CENT;
	m_mapLayouts["20"] = CP_LAYOUT_1P8HOR_UP;
	m_mapLayouts["21"] = CP_LAYOUT_1P8UP;
	m_mapLayouts["22"] = CP_LAYOUT_4X4;
	m_mapLayouts["23"] = CP_LAYOUT_1P12;
	m_mapLayouts["24"] = CP_LAYOUT_2P8;
	m_mapLayouts["25"] = CP_LAYOUT_1X2_FLEX;
	m_mapLayouts["26"] = CP_LAYOUT_1P2HOR_RIGHT_FLEX;
	m_mapLayouts["27"] = CP_LAYOUT_1P2HOR_LEFT_FLEX;
	m_mapLayouts["28"] = CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX;
	m_mapLayouts["29"] = CP_LAYOUT_1P2HOR_UP_LEFT_FLEX;
	m_mapLayouts["30"] = CP_LAYOUT_2X2_UP_RIGHT_FLEX;
	m_mapLayouts["31"] = CP_LAYOUT_2X2_UP_LEFT_FLEX;
	m_mapLayouts["32"] = CP_LAYOUT_2X2_DOWN_RIGHT_FLEX;
	m_mapLayouts["33"] = CP_LAYOUT_2X2_DOWN_LEFT_FLEX;
	m_mapLayouts["34"] = CP_LAYOUT_2X2_RIGHT_FLEX;
	m_mapLayouts["35"] = CP_LAYOUT_2X2_LEFT_FLEX;
}

//--------------------------------------------------------------------------
void CIvrSubVenus::OnTimerDTMF(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CIvrSubVenus::OnTimerDTMF - F_IVR===>Venus, End of Venus ");

	// sends STOP_Venus command
	m_pConfApi->StartStopVenus(m_pParty->GetPartyRsrcID(), 0);  // stop command

	EndVenusFeature();
}

//--------------------------------------------------------------------------
void CIvrSubVenus::EndVenusFeature()
{
	PTRACE(eLevelInfoNormal, "CIvrSubVenus::EndVenusFeature - F_IVR===>Venus, End of Venus ");
	// restore old DTMF opcodes table: create if needed
	m_pDtmf->SetDtmfTable(m_pIvrPrevTable); // can be NULL
	POBJDELETE(m_pIvrPrevTable);

	// end feature
	m_pDtmf->ResetDtmfBuffer();
	m_pDtmf->EndFeature();
	m_pDtmf->InformAboutEveryDTMFPressed(0);
	m_sDTMF = "";

	OnEndFeature(STATUS_OK);

	m_state = NOTACTIVE;
}

//--------------------------------------------------------------------------
int CIvrSubVenus::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}

//--------------------------------------------------------------------------
void CIvrSubVenus::PlayRetryMessage(DWORD messageMode)
{
}

//--------------------------------------------------------------------------
void CIvrSubVenus::OnStopIvr()
{
	EndVenusFeature();
}


PBEGIN_MESSAGE_MAP(CIvrSubInviteParty)
	ONEVENT(DTMF_STRING_IDENT,     NOTACTIVE,   CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT,     ACTIVE,      CIvrSubBaseSM::OnDtmfString)
	ONEVENT(DTMF_STRING_IDENT,     WAIT_FOR_DB, CIvrSubBaseSM::NullActionFunction)
	ONEVENT(TIMER_RETRY_MESSAGE,   ACTIVE,      CIvrSubBaseSM::OnTimeRetryMessage)
	ONEVENT(EXT_DB_RESPONSE_TOUT,  WAIT_FOR_DB, CIvrSubInviteParty::OnTimeNoExtDBResponse)
	ONEVENT(EXT_DB_PWD_CONFIRM,    WAIT_FOR_DB, CIvrSubInviteParty::OnExtDBResponse)
	ONEVENT(EXT_DB_REQUEST_FAILED, WAIT_FOR_DB, CIvrSubInviteParty::OnExtDBFailure)
PEND_MESSAGE_MAP(CIvrSubInviteParty, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubInviteParty
////////////////////////////////////////////////////////////////////////////
CIvrSubInviteParty::CIvrSubInviteParty(const CIVRInvitePartyFeature* pInvitePartyFeature, const char* pPartyDialingNum, BOOL bEnableExternalDB)
{
	if (pPartyDialingNum)
	{
		strncpy(m_pPartyDialingNum, pPartyDialingNum, MAX_PASSWORD_LEN);
		m_pPartyDialingNum[MAX_PASSWORD_LEN] = 0;
	}
	else
		m_pPartyDialingNum[0] = 0;

	m_pInvitePartyFeature = new CIVRInvitePartyFeature(*pInvitePartyFeature);
	m_internalState       = -1;
	m_useExternalDB       = IVR_EXTERNAL_DB_NONE;
	m_bEnableExternalDB   = bEnableExternalDB;
	m_skipPrompt          = FALSE;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrSubInviteParty::~CIvrSubInviteParty()
{
	POBJDELETE(m_pInvitePartyFeature);
}

//--------------------------------------------------------------------------
void* CIvrSubInviteParty::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CIvrSubInviteParty::Start()
{
	PTRACE2(eLevelInfoNormal, "CIvrSubInviteParty::Start - F_IVR===>InviteParty ", m_pParty->GetName());
	if (!IsIvrPtrs())
		return;

	m_pDtmf->SetIvrPartSessionOn();   // saying we are in IVR session (not changed until end of IVR session)

	StartBaseTimer();                 // start timer capability (InitTimer)

	m_bStopUponDTMF = 1;              // stop message upon first DTMF
	m_DTMF_digits   = 0;
	m_DTMF_timeout  = m_pIvrService->GetUserInputTimeout();
	if (m_DTMF_timeout == 0)
		m_DTMF_timeout = 1;

	m_pinCodeDelimiter = GetIvrDelimiter('#');

	// temp
	int status = STATUS_OK;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	if (pCommConf == NULL)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubInviteParty::Start: Password, conf NULL pointer - F_IVR===>InviteParty");
		return;
	}

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
	if (pConfParty == NULL)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubInviteParty::Start: party NULL pointer - F_IVR===>InviteParty");
		return;
	}

	BYTE bIvrEntryType = 0;

	m_feature_opcode = IVR_FEATURE_CONF_PASSWORD;

	// update duration if needed
	char msgFullPath[MAX_FULL_PATH_LEN];
	msgFullPath[0] = '\0';
	WORD msgDuration = 0;
	WORD msgCheckSum = (WORD)(-1);
	GetIVRMessageParameters(msgFullPath, &msgDuration, &msgCheckSum);   // update the duration


	// message duration
	m_msgDuration = GetMsgDuration(m_pInvitePartyFeature, m_event_opcode);
	if (m_msgDuration < 2)
		m_msgDuration = 5;

	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
		m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	m_useExternalDB = IVR_EXTERNAL_DB_NONE;
	if (m_bEnableExternalDB)
		m_useExternalDB = (BYTE)m_pIvrService->GetIvrExternalDB();

	if (IVR_EXTERNAL_DB_NONE == m_useExternalDB)
		PTRACE(eLevelInfoNormal, "CIvrSubInviteParty::Start - External-DB Not in use F_IVR===>InviteParty");

	const char* szPreDefinedIvrString = pConfParty->GetPreDefinedIvrString();

	if (strncmp(szPreDefinedIvrString, "", DTMF_MAX_BUFFER_SIZE))
	{
		PTRACE(eLevelInfoNormal, "CIvrSubInviteParty::Start - Start-szPreDefinedIvrString F_IVR===>InviteParty");
		ParsePreDefinedIvrString(szPreDefinedIvrString);
	}

	// /////////////////////////////////////////////////////////////////////////////////////////
	// Check the conditions for skipping conference pwd prompt in "Same Time" environment (IBM)
	// /////////////////////////////////////////////////////////////////////////////////////////
	// 1. system.cfg flag should be set to YES
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key       = CFG_KEY_SKIP_PROMPT;
	BOOL bSkipPrompt;
	sysConfig->GetBOOLDataByKey(key, bSkipPrompt);

	// 2. Connection type should be DIAL_IN
	BYTE connectionType = pConfParty->GetConnectionType();

	// 3. Conference was created by PCAS by a non-web (Same Time) flow - name format: RAS200I_xxx
	BOOL        bPcasNonWebFlow = FALSE;
	const char* confName        = pCommConf->GetName();
	if (0 == strncmp(confName, "RAS200I_", 8))
		if (0 != strncmp(confName+8, "web_", 4))
			bPcasNonWebFlow = TRUE;

	// 4. External DB access is configured
	BOOL bExternalDbConfigured = FALSE;
	if (IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD == m_useExternalDB)
		bExternalDbConfigured = TRUE;

	if (bSkipPrompt && (DIAL_IN == connectionType) && bPcasNonWebFlow && bExternalDbConfigured)
	{
		PTRACE(eLevelInfoNormal, "CIvrSubInviteParty::Start - Skip conf password prompt scenario: sent '#' to dtmf buffer - F_IVR===>InviteParty");
		m_pDtmf->FillDtmfBuffer("#");
		m_skipPrompt = TRUE;
	}

	// start feature
	StartBase(FALSE, IVR_STATUS_PLAY_ONCE);
}

//--------------------------------------------------------------------------
int CIvrSubInviteParty::IsLegalString(CSegment* pParam)
{
	PTRACE(eLevelError, "CIvrSubInviteParty::IsLegalString - F_IVR===>InviteParty");

	WORD opcode;
	WORD rDigits;
	if (0 != GetDtmfResult(pParam, opcode, rDigits))
	{
		PTRACE(eLevelError, "CIvrSubInviteParty::IsLegalString: Illegal DTMF string F_IVR===>InviteParty");
		if (rDigits == 0)                   // no digit in the string
			return DTMF_TIMEOUT_ERROR;

		return DTMF_STRING_ERROR;           // opcode error or string length error
	}

	if (m_internalState == REQUEST_DIGIT) // the user requested to enter 1 digit
		return DTMF_OK;

	if (!m_pParty)                        // should be valid
	{
		PASSERT(1);
		PTRACE(eLevelError, "CIvrSubInviteParty::IsLegalString: Illegal Party Internal Pointer F_IVR===>InviteParty");
		return DTMF_STRING_ERROR;
	}

	if ((0 == rDigits) && (IVR_EXTERNAL_DB_NONE == m_useExternalDB))  // Error: empty PW (the user pressed only "#"). It is legal in External-DB
	{
		PTRACE(eLevelError, "CIvrSubInviteParty::IsLegalString: Illegal Empty DTMF string F_IVR===>ConfPW");
		return DTMF_STRING_ERROR;
	}

	ALLOCBUFFER(str, 120);    // trace buffer
	str[0] = 0;
	WORD returnValue = DTMF_OK;
	BYTE isLeader    = FALSE;
	BYTE isGuest     = FALSE;
	BOOL isHidePsw   = NO;
	unsigned char invisible_psw [] = "********";
	std::string   key_hide         = "HIDE_CONFERENCE_PASSWORD";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);

	if (opcode == OP_NOOP)    // string was requested
	{
		if (0 != m_dtmf[0])
		{
			PTRACE2(eLevelError, "CIvrSubInviteParty::IsLegalString: DTMF string F_IVR===>InviteParty = ", (const char*)m_dtmf);
		}
		else
		{
			strncpy(str, "User entered empty dailing string ", 120);
			if (IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD != m_useExternalDB)
				returnValue = DTMF_STRING_ERROR;
		}
	}

	// print to trace
	PTRACE(eLevelInfoNormal, str);
	DEALLOCBUFFER(str);

	return returnValue;
}

//--------------------------------------------------------------------------
WORD CIvrSubInviteParty::DoSomething()
{
	// do something on success
	m_ivrCntl->SetStartNextFeature();
	return 0;
}

//--------------------------------------------------------------------------
void CIvrSubInviteParty::PlayRetryMessage(DWORD messageMode)
{
}

//--------------------------------------------------------------------------
void CIvrSubInviteParty::OnTimeNoExtDBResponse(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubConfPassword::OnTimeNoExtDBResponse- F_IVR===>ConfPW, party=", m_pParty->GetName());
	OnEndFeature(TOO_MANY_ERRORS);
}

//--------------------------------------------------------------------------
int CIvrSubInviteParty::OnExtDBResponse(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, party=", m_pParty->GetName());

	CXMLDOMDocument* pDom      = new CXMLDOMDocument; AUTO_DELETE(pDom);
	CXMLDOMElement*  pTempNode = NULL, * pNode = NULL;
	BYTE             id        = 1, bLeader = FALSE, bVIP = FALSE, bEndFeature = TRUE;
	int              status    = STATUS_OK, nStatus = SEC_OK;
	char*            nodeName  = NULL, * userInfo1 = NULL, * userInfo2 = NULL, * userInfo3 = NULL,
	* userInfo4                = NULL, * name = NULL, * desc = NULL;
	ALLOCBUFFER(pszError, H243_NAME_LEN); AUTO_DELETE_ARRAY(pszError);
	ALLOCBUFFER(missingField, H243_NAME_LEN); AUTO_DELETE_ARRAY(missingField);
	DWORD XMLStringLen = 0;

	DeleteTimer(EXT_DB_RESPONSE_TOUT);

	*pParam >> XMLStringLen;
	ALLOCBUFFER(pXMLString, XMLStringLen+1); AUTO_DELETE_ARRAY(pXMLString);
	*pParam >> pXMLString;
	pXMLString[XMLStringLen] = '\0';

	if (pDom->Parse((const char**)&pXMLString) == SEC_OK)
	{
		CXMLDOMElement* pRoot = pDom->GetRootElement();

		nStatus = pRoot->get_nodeName(&nodeName);
		if (!strncmp(nodeName, "CONFIRM_PARTY_DETAILS", 22))
		{
			GET_CHILD_NODE(pRoot, "RETURN_STATUS", pNode);
			if (pNode)
			{
				GET_VALIDATE_CHILD(pNode, "ID", &id, _0_TO_10_DECIMAL);
				if (nStatus != SEC_OK)
					sprintf(missingField, "'ID'");

				GET_VALIDATE_CHILD(pNode, "DESCRIPTION", &desc, DESCRIPTION_LENGTH);
			}
			else if (nStatus != SEC_OK)
				sprintf(missingField, "'RETURN_STATUS'");

			if (nStatus == SEC_OK)
			{
				// don't check rest of the XML fields if ID!=0
				if (id == 0)
				{
					GET_CHILD_NODE(pRoot, "ACTION", pTempNode)
					if (pTempNode)
					{
						GET_CHILD_NODE(pTempNode, "ADD", pNode);
						if (pNode)
						{
							GET_VALIDATE_CHILD(pNode, "LEADER", &bLeader, _BOOL);
							if (nStatus != SEC_OK)
							{
								sprintf(missingField, "'LEADER'");
							}
							else
							{
								GET_VALIDATE_CHILD(pNode, "NAME", &name, _0_TO_H243_NAME_LENGTH);
								GET_VALIDATE_CHILD(pNode, "VIP", &bVIP, _BOOL);
								GET_CHILD_NODE(pNode, "CONTACT_INFO_LIST", pTempNode);

								if (pTempNode)
								{
									CXMLDOMElement* pContactInfoNode;

									GET_FIRST_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

									if (pContactInfoNode)
									{
										GET_VALIDATE(pContactInfoNode, &userInfo1, ONE_LINE_BUFFER_LENGTH);

										GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

										if (pContactInfoNode)
										{
											GET_VALIDATE(pContactInfoNode, &userInfo2, ONE_LINE_BUFFER_LENGTH);

											GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

											if (pContactInfoNode)
											{
												GET_VALIDATE(pContactInfoNode, &userInfo3, ONE_LINE_BUFFER_LENGTH);

												GET_NEXT_CHILD_NODE(pTempNode, "CONTACT_INFO", pContactInfoNode);

												if (pContactInfoNode)
													GET_VALIDATE(pContactInfoNode, &userInfo4, ONE_LINE_BUFFER_LENGTH);
											}
										}
									}
								}
							}
						}
						else if (nStatus != SEC_OK)
							sprintf(missingField, "'ADD'");
					}
					else if (nStatus != SEC_OK)
						sprintf(missingField, "'ACTION'");
				}
			}
		}
		else
		{
			PTRACE(eLevelError, "CIvrSubLeader::OnExtDBResponse - F_IVR===>ConfPW, confirmation transaction is not recognized ");
			nStatus = STATUS_ILLEGAL;
		}
	}

	if (strlen(missingField) > 0 || nStatus == STATUS_ILLEGAL)
	{
		PTRACE2(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, XML string does not include the mandatory field ", missingField);
		PASSERT(STATUS_ILLEGAL);
		OnEndFeature(TOO_MANY_ERRORS);
	}
	else
	{
		if (desc && strcmp(desc, ""))
			PTRACE2(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Description = ", desc);

		m_state = ACTIVE;
		// if party is not authorized
		switch (id)
		{
			case (0):
			{
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, DB replied - OK. ");
				// if (nStatus!=STATUS_NODE_MISSING)
				if (bLeader)
				{
					m_ivrCntl->SetIsLeader(1, 1);
					m_pDtmf->SetDtmpOpcodePermission(DTMF_LEADER_ACTION);
				}

				CConfParty* pConfParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(m_pParty->GetMonitorConfId(), m_pParty->GetMonitorPartyId());
				if (pConfParty)
				{
					pConfParty->SetIsVip(bVIP);
					CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
					std::string key       = CFG_KEY_VISUAL_NAME_CONVENTION;
					DWORD       dwVisualNameConvention;
					sysConfig->GetDWORDDataByKey(key, dwVisualNameConvention);
					if (1 == dwVisualNameConvention)
					{
						if (name)
						{
							char* NewNameToSearch = new char[H243_NAME_LEN];
							int   counter         = 1;
							strncpy(NewNameToSearch, name, H243_NAME_LEN);

							do
							{
								status = ::GetpConfDB()->SearchPartyVisualName(m_pParty->GetMonitorConfId(), NewNameToSearch);
								if (status == STATUS_OK)
								{
									sprintf(NewNameToSearch, "%s(%03d)", name, counter);
									counter++;
								}
								else
								{
									break;
								}
							}
							while (status == STATUS_OK);

							if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
							{
								status = STATUS_OK;
								pConfParty->SetVisualPartyName(NewNameToSearch); // NEED TO GO THROUGH updateDB - meanwhile there is no call to the CDR from here
							}

							CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

							DEALLOCBUFFER(NewNameToSearch);
						}
					}
					else
					{
						if (name)
						{
							int status = ::GetpConfDB()->SearchPartyVisualName(m_pParty->GetMonitorConfId(), name);

							if (status == STATUS_PARTY_VISUAL_NAME_NOT_EXISTS)
								pConfParty->SetVisualPartyName(name); // NEED TO GO THROUGH updateDB - meanwhile there is no call to the CDR from here

						}
					}

					pConfParty->SetUserDefinedInfo(userInfo1, 0);
					// in order to activate PCAS event when the CONTACT_INFO arrive from the external DB
					pConfParty->UpdateExtDBUserInfo(userInfo1, 0);
					pConfParty->SetUserDefinedInfo(userInfo2, 1);
					pConfParty->SetUserDefinedInfo(userInfo3, 2);
					pConfParty->SetUserDefinedInfo(userInfo4, 3);
				}

				break;
			}

			case (1):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal MCU user name / password .");
				PASSERT(1);
				status = TOO_MANY_ERRORS;
				break;
			}

			case (2):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Request timed out.");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (3):
			{
				PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal password.");
				if (m_skipPrompt)
				{
					PTRACE(eLevelInfoNormal, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Skip Promp - no retries!");
					status = TOO_MANY_ERRORS;
					break;
				}

				m_errorCounter++;                             // increses error counter
				m_timeoutCounter = 0;                         // resets tomeout counter
				if (m_maxRetryTimes > m_errorCounter)         // allowed to retry
				{
					m_pDtmf->ResetDtmfBuffer();                 // reset DTMF buffer
					StartBase(TRUE, IVR_STATUS_PLAY_ONCE);      // wrong UserId, retries to get UserID again
					bEndFeature = FALSE;
				}
				else
				{
					status = TOO_MANY_ERRORS;   // finish status: ERROR
				}

				break;
			}

			case (4):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal CLI - The originator CLI is not permitted to start conferences.");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (5):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal CLI/NID/Password - The combination of the CLI, NID, Password (or any 2 of them) is not permitted.");
				status = TOO_MANY_ERRORS;
				break;
			}

			case (6):
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Internal Error in DB.");
				status = TOO_MANY_ERRORS;
				break;
			}

			default:
			{
				PTRACE(eLevelError, "CIvrSubConfPassword::OnExtDBResponse - F_IVR===>ConfPW, Illegal ID response from DB.");
				PASSERT(7);
				break;
			}
		} // switch
	}

	PDELETE(pDom);
	DEALLOCBUFFER(missingField);
	DEALLOCBUFFER(pXMLString);
	DEALLOCBUFFER(pszError);

	if (bEndFeature)
		OnEndFeature(status);

	return status;
}

//--------------------------------------------------------------------------
void CIvrSubInviteParty::OnExtDBFailure(CSegment* pParam)
{
	PTRACE2(eLevelError, "CIvrSubConfPassword::OnExtDBFailure - F_IVR===>ConfPW, party=", m_pParty->GetName());
	OnEndFeature(TOO_MANY_ERRORS);
}

/////////////////////////////////////////////////////////////////////////////
//					CIvrSubAudioExternal: MESSAGE_MAP
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CIvrSubAudioExternal)

//  ONEVENT(DTMF_STRING_IDENT        ,NOTACTIVE  ,CIvrSubAudioExternal::OnIvrSubBaseNull)
//  ONEVENT(DTMF_STRING_IDENT        ,ACTIVE     ,CIvrSubAudioExternal::OnDtmfString)
//  ONEVENT(TIMER_WAIT_FOR_WELCOME   ,ACTIVE     ,CIvrSubAudioExternal::ContinueWelcome)
	ONEVENT(FIRST_DTMF_WAS_DIALED	 ,ANYCASE	 ,CIvrSubAudioExternal::OnFirstDTMFReceivedACTIVE)

PEND_MESSAGE_MAP(CIvrSubAudioExternal,CStateMachine);


/////////////////////////////////////////////////////////////////////////////
//					CIvrSubAudioExternal
/////////////////////////////////////////////////////////////////////////////
CIvrSubAudioExternal::CIvrSubAudioExternal(const char* fileName,DWORD duration, const std::string& dialogID)
{
	if (fileName)
    {
	    strncpy(m_msg_file_name, fileName, sizeof(m_msg_file_name) - 1);
		m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
	}

	m_msgDuration = duration;
	m_dialogID = dialogID;

	TRACEINTO << "F_IVR===>AudioExternal, FileName:" << m_msg_file_name << ", dialogID:" << m_dialogID; // Also to print duration- PASSERT(555777)

	VALIDATEMESSAGEMAP;
}


/////////////////////////////////////////////////////////////////////////////
//					~CIvrSubAudioExternal
/////////////////////////////////////////////////////////////////////////////
CIvrSubAudioExternal::~CIvrSubAudioExternal()
{

}

/////////////////////////////////////////////////////////////////////////////
void*  CIvrSubAudioExternal::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CIvrSubAudioExternal::Start()
{
	PTRACE( eLevelInfoNormal, "CIvrSubAudioExternal::Start - F_IVR===>AudioExternal " );
	if (!m_ivrCntl)
		return;

	m_feature_opcode = IVR_FEATURE_AUDIO_EXTERNAL;

	if (m_msgDuration < 2)
			m_msgDuration = 5;
	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
			m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	m_state = ACTIVE;// PASSERT(555777)

	ALLOCBUFFER(str, H243_NAME_LEN_DEFINED_AGAIN + 100);	// buffer for trace
	str[0] = 0;
	if (m_pParty)		// should be OK
		sprintf( str, " duration: %i, Party: %s state: %d", (int)m_msgDuration, m_pParty->GetName(), m_state );

	TRACEINTO << "F_IVR===>AudioExternal, " << str << " state: " << m_state;

	DEALLOCBUFFER(str);

	PlayMessage( TRUE ,30); //30 is default for loop delay TODO: ???

	//PASSERT(555777)
	//StartTimer(TIMER_WAIT_FOR_WELCOME,  (m_msgDuration+1) * SECOND);
	//m_pDtmf->InformAboutFirstNumbPressed();	// will skip to the next feature upon first DTMF received
	return;
}

/////////////////////////////////////////////////////////////////////////////
void  CIvrSubAudioExternal::OnFirstDTMFReceivedACTIVE( CSegment* pSeg )
{
	TRACEINTO << "received first dtmf. m_bAllowDtmfBargeIn=" << (int)m_bAllowDtmfBargeIn << ", state: " << m_state;
	if (m_bAllowDtmfBargeIn)
	{
		TRACEINTO << "Stopping message.";
		StopPlayMessage();
		CSegment *pSeg = new CSegment();
		*pSeg << m_dialogID;
		m_ivrCntl->HandleEvent(pSeg, pSeg->GetLen(), EXTERNAL_IVR_DTMF_BARGE_IN);
		POBJDELETE(pSeg);
	}
}

/////////////////////////////////////////////////////////////////////////////
int  CIvrSubAudioExternal::IsLegalString( CSegment* pParam )
{
	return DTMF_OK;
}

/////////////////////////////////////////////////////////////////////////////
void  CIvrSubAudioExternal::PlayRetryMessage( DWORD messageMode )
{
}

/////////////////////////////////////////////////////////////////////////////
void  CIvrSubAudioExternal::PlayMessage( DWORD messageMode, WORD loop_delay)
{

  PTRACE2INT(eLevelInfoNormal,"CIvrSubAudioExternal::PlayMessage, mode = ",messageMode);

  char msgFullPath[MAX_FULL_PATH_LEN];
  msgFullPath[0] = '\0';

  WORD msgDuration = 0;
  WORD msgCheckSum = (WORD)(-1);

  WORD cachePriority = GetCachePriority( m_feature_opcode, m_event_opcode );

  if (m_pConfApi)
  {
		PTRACE(eLevelInfoNormal,"CIvrSubAudioExternal::PlayMessage - need to send");
		m_pConfApi->StartMessage( m_pParty->GetPartyRsrcID(), m_msg_file_name, m_msgDuration, 50, (WORD)messageMode, PRIORITY_5_CACHE_IVR ,loop_delay);
  }
}


/***************************************************************************************/

/////////////////////////////////////////////////////////////////////////////
//					CIvrSubVideoExternal: MESSAGE_MAP
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CIvrSubVideoExternal)

//  ONEVENT(DTMF_STRING_IDENT        ,NOTACTIVE  ,CIvrSubVideoExternal::OnIvrSubBaseNull)
//  ONEVENT(DTMF_STRING_IDENT        ,ACTIVE     ,CIvrSubVideoExternal::OnDtmfString)
//  ONEVENT(TIMER_WAIT_FOR_WELCOME   ,ACTIVE     ,CIvrSubVideoExternal::ContinueWelcome)
	ONEVENT(FIRST_DTMF_WAS_DIALED	 ,ANYCASE	 ,CIvrSubVideoExternal::OnFirstDTMFReceivedACTIVE)

PEND_MESSAGE_MAP(CIvrSubVideoExternal,CStateMachine);


/////////////////////////////////////////////////////////////////////////////
//					CIvrSubVideoExternal
/////////////////////////////////////////////////////////////////////////////
CIvrSubVideoExternal::CIvrSubVideoExternal(const char* fileName,DWORD duration, const std::string& dialogID)
{
	if (fileName)
	{
		strncpy(m_msg_file_name, fileName, sizeof(m_msg_file_name) - 1);
		m_msg_file_name[sizeof(m_msg_file_name) - 1] = '\0';
	}

	m_msgDuration = duration;
	m_dialogID = dialogID;

	PTRACE2( eLevelInfoNormal, "CIvrSubVideoExternal::CIvrSubVideoExternal - F_IVR===>VideoExternal,FileName: ",  m_msg_file_name );//Also to print duration- PASSERT(555777)

	VALIDATEMESSAGEMAP;
}


/////////////////////////////////////////////////////////////////////////////
//					~CIvrSubVideoExternal
/////////////////////////////////////////////////////////////////////////////
CIvrSubVideoExternal::~CIvrSubVideoExternal()
{

}

/////////////////////////////////////////////////////////////////////////////
void*  CIvrSubVideoExternal::GetMessageMap()
{
  return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void  CIvrSubVideoExternal::Start()
{
	PTRACE( eLevelInfoNormal, "CIvrSubVideoExternal::Start - F_IVR===>VideoExternal " );
	if (!m_ivrCntl)
		return;

	m_feature_opcode = IVR_FEATURE_VIDEO_EXTERNAL;

	if (m_msgDuration < 2)
			m_msgDuration = 5;
	if (m_msgDuration > MAX_IVR_AUDIO_MSG_DURATION)
			m_msgDuration = MAX_IVR_AUDIO_MSG_DURATION;

	//m_state = ACTIVE; PASSERT(555777)

	ALLOCBUFFER(str, H243_NAME_LEN_DEFINED_AGAIN + 100);	// buffer for trace
	str[0] = 0;
	if (m_pParty)		// should be OK
		sprintf( str, "slide duration: %i, Party: %s ", (int)m_msgDuration, m_pParty->GetName() );
	else
		PASSERTMSG(999, "CIvrSubVideoExternal::Start - no m_pParty!!");

	PTRACE2( eLevelInfoNormal, "CIvrSubVideoExternal::Start - F_IVR===>VideoExternal, ", str );

	DEALLOCBUFFER(str);

	PlayMessage(IVR_STATUS_PLAY_ONCE, 30); //30 is default for loop delay ??? noneed  TODO: ??

	//PASSERT(555777)
	//StartTimer(TIMER_WAIT_FOR_WELCOME,  (m_msgDuration+1) * SECOND);
	//m_pDtmf->InformAboutFirstNumbPressed();	// will skip to the next feature upon first DTMF received
	return;
}

/////////////////////////////////////////////////////////////////////////////
void  CIvrSubVideoExternal::OnFirstDTMFReceivedACTIVE( CSegment* pParam )
{
	TRACEINTO << "received first dtmf. m_bAllowDtmfBargeIn=" << (int)m_bAllowDtmfBargeIn;
	if (m_bAllowDtmfBargeIn)
	{
		PASSERTSTREAM(m_feature_opcode == 0, "received first dtmf before image \"" << m_msg_file_name << "\" was displayed.");
		CSegment *pSeg = new CSegment();
		*pSeg << m_dialogID;
		m_ivrCntl->HandleEvent(pSeg, pSeg->GetLen(), EXTERNAL_IVR_DTMF_BARGE_IN);
	}
}
/////////////////////////////////////////////////////////////////////////////
int CIvrSubVideoExternal::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CIvrSubVideoExternal::PlayRetryMessage(DWORD messageMode)
{
}

/////////////////////////////////////////////////////////////////////////////
void CIvrSubVideoExternal::PlayMessage(DWORD messageMode, WORD loop_delay)
{
	TRACEINTO << "ConfApi:" << m_pConfApi << ", mode:" << messageMode << ", party resource id:" << m_pParty->GetPartyRsrcID();

	CSegment seg;
	m_pParty->GetRcvMbx().Serialize(seg);

	if (m_pConfApi)
		m_pConfApi->IvrPartyNotification(m_pParty->GetPartyRsrcID(), m_pParty, m_pParty->GetName(), PARTY_VIDEO_IVR_MODE_CONNECTED, &seg, eMediaInAndOut, m_msg_file_name);
}

/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CIvrSubExternalInputCollect)
	ONEVENT(DTMF_STRING_IDENT,        NOTACTIVE,   CIvrSubBaseSM::OnIvrSubBaseNull)
	ONEVENT(DTMF_STRING_IDENT,        ACTIVE,      CIvrSubBaseSM::OnDtmfString)
	ONEVENT(TIMER_RETRY_MESSAGE,      ACTIVE,      CIvrSubBaseSM::OnTimeRetryMessage)
	ONEVENT(FIRST_DTMF_WAS_DIALED	 ,ANYCASE,      CIvrSubExternalInputCollect::OnFirstDTMFReceivedACTIVE)
PEND_MESSAGE_MAP(CIvrSubExternalInputCollect, CStateMachine);

////////////////////////////////////////////////////////////////////////////
CIvrSubExternalInputCollect::CIvrSubExternalInputCollect(const std::string& dialogIdStr, char strDelimiter, WORD dtmfTimeOut, WORD dtmfMaxDigits, BOOL isClearBuffer /*char strEscapeChar */)
{
	m_pinCodeDelimiter = strDelimiter;
	m_DTMF_timeout = dtmfTimeOut; // seconds
	m_DTMF_digits = dtmfMaxDigits;
	m_dialogID = dialogIdStr; //BRIDGE-3510
	m_bIsClearBuffer = isClearBuffer;
}

/////////////////////////////////////////////////////////////////////////////
CIvrSubExternalInputCollect::~CIvrSubExternalInputCollect()
{
}

/////////////////////////////////////////////////////////////////////////////
void  CIvrSubExternalInputCollect::Start()
{
	TRACEINTO <<  "CIvrSubExternalInputCollect::Start w/ timeout " << m_DTMF_timeout << ", and delimiter " << m_pinCodeDelimiter;
	if (!m_ivrCntl)
		return;
	m_maxRetryTimes = 0;
	m_pDtmf->SetIvrPartSessionOn();
	m_feature_opcode = IVR_FEATURE_COLLECT_EXTERNAL;
	PASSERTMSG_AND_RETURN(!IsValidPObjectPtr(m_pDtmf), "dtmf collector is invalid!");
	// buffer clearing moved to the start of the dialog
	m_pDtmf->SetDtmfDelimiter(m_pinCodeDelimiter);
	m_pDtmf->InformAboutEveryDTMFPressed(FALSE);
	m_pDtmf->SetDtmfWaitForXDigitsNumber(0,m_DTMF_timeout,0);
	m_pDtmf->StartFeature();

	m_state = ACTIVE;
}

/////////////////////////////////////////////////////////////////////////////
int CIvrSubExternalInputCollect::IsLegalString(CSegment* pParam)
{
	// currently any result is OK, even an empty string
	// TODO implement validity test based on the dialog's "grammar"
	WORD opcode;
	WORD rDigits;
	int status = DTMF_OK;
	status = GetDtmfResult(pParam, opcode, rDigits);   // copy the DTMF to internal array
	TRACEINTO << " got dtmf result \"" << (const char*)m_dtmf << "\", with result status " << status;
	if (status != DTMF_OK)
	{
		if (status == 1)
			status = DTMF_TIMEOUT_ERROR;
		else
			status = DTMF_STRING_ERROR;
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
WORD CIvrSubExternalInputCollect::DoSomething()
{
	return EndCollectFeature(STATUS_OK);
}

/////////////////////////////////////////////////////////////////////////////
void CIvrSubExternalInputCollect::EndFeatureError(WORD status)
{
	EndCollectFeature(status);
	//CIvrSubBaseSM::EndFeatureError(status);
}

/////////////////////////////////////////////////////////////////////////////
WORD CIvrSubExternalInputCollect::EndCollectFeature(WORD status)
{
	TRACEINTO;
	ALLOCBUFFER(receivedDtmfStr, MAX_DTMF_STRING_LENGTH);  // MAX_DTMF_STRING_LENGTH is the max size of the data member m_dtmf
	strncpy(receivedDtmfStr, (const char*)m_dtmf, MAX_DTMF_STRING_LENGTH);
	receivedDtmfStr[MAX_DTMF_STRING_LENGTH-1] = '\0';
	std::string statusStr;
	switch (status)
	{
	case DTMF_OK:
		statusStr = "match";
		break;
	case DTMF_STRING_ERROR:
		statusStr = "nomatch";
		break;
	case DTMF_TIMEOUT_ERROR:
		statusStr = "stopped";
		break;
	case TOO_MANY_ERRORS:
		statusStr = "error";
		break;
	default:
		statusStr = "match";
	}
	std::string trace = "CIvrSubExternalInputCollect::EndCollectFeature - F_IVR===>ExternalInputCollect, collected:  ";
	trace += receivedDtmfStr;
	trace += " with status ";
	trace += statusStr;
	PTRACE(eLevelInfoNormal, trace.c_str());

	((CIvrCntlExternal*)m_ivrCntl)->OnCollectDigitsResult(m_dialogID, receivedDtmfStr, statusStr);

//	if (0 == strlen(receivedDtmfStr))
//		strcpy(receivedDtmfStr, "Empty");

	DEALLOCBUFFER(receivedDtmfStr);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CIvrSubExternalInputCollect::EndFeature(WORD status)
{
	TRACEINTO << " status:" << status;
	if (status == DTMF_TIMEOUT_EXTERNAL_IVR_TIMER)
		m_pDtmf->ResetDtmfParams();
	else
		CIvrSubBaseSM::EndFeature(status);
}

/////////////////////////////////////////////////////////////////////////////
void CIvrSubExternalInputCollect::PlayRetryMessage(DWORD messageMode)
{
}

/////////////////////////////////////////////////////////////////////////////
void CIvrSubExternalInputCollect::PlayMessage(DWORD messageMode, WORD loop_delay)
{
}
/////////////////////////////////////////////////////////////////////////////
void CIvrSubExternalInputCollect::StopPlayMessage()
{
}

/////////////////////////////////////////////////////////////////////////////
void CIvrSubExternalInputCollect::OnFirstDTMFReceivedACTIVE(CSegment* pParam)
{
	TRACEINTO << "received first dtmf while already collecting. no action required.";
}

/////////////////////////////////////////////////////////////////////////////
void* CIvrSubExternalInputCollect::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
int CIvrSubExternalInputCollect::GetDtmfResult(CSegment* pParam, WORD& opcode, WORD& rDigits)
{
	int i = 0;
	m_dtmf[0] = 0;

	*pParam >> opcode;        // (OP_ERROR, OP_NOOP or one from the table)
	*pParam >> rDigits;

	WORD digits_num = (rDigits < m_DTMF_digits)? rDigits : m_DTMF_digits;
	for (i = 0; i < rDigits ; i++)
	{
		BYTE tmp;
		*pParam >> tmp;
		m_dtmf[i] = tmp;
	}

	m_dtmf[i] = 0;

	if (opcode == OP_ERROR)   // timeout without any DTMF
	{
		PTRACE(eLevelError, "CIvrSubBaseSM::GetDtmfResult: OP_ERROR F_IVR===>Collect");
		return 1;
	}

	if (rDigits >= MAX_DTMF_STRING_LENGTH)
	{
		PTRACE(eLevelError, "CIvrSubBaseSM::GetDtmfResult: Error: MAX_DTMF_STRING_LENGTH F_IVR===>Collect");
		return 1;
	}

	if (opcode == OP_NOOP)
	{
		if ((rDigits > m_DTMF_digits) && (0 != m_DTMF_digits))    // illegal value
		{
			PTRACE(eLevelError, "CIvrSubBaseSM::GetDtmfResult: Error: rDigits F_IVR===>Collect");
			return 2;
		}
	}

	return 0;
}

//IVR for TIP

/////////////////////////////////////////////////////////////////////////////
//					CIvrSubWait
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CIvrSubWait)
	ONEVENT(END_WAIT, ANYCASE, CIvrSubWait::OnEndWait)
PEND_MESSAGE_MAP(CIvrSubWait, CStateMachine);

/////////////////////////////////////////////////////////////////////////////

CIvrSubWait::CIvrSubWait()
{
	VALIDATEMESSAGEMAP;
}
/////////////////////////////////////////////////////////////////////////////

CIvrSubWait::~CIvrSubWait()
{

}
/////////////////////////////////////////////////////////////////////////////

void*  CIvrSubWait::GetMessageMap()
{
  return (void*)m_msgEntries;
}
/////////////////////////////////////////////////////////////////////////////

void CIvrSubWait::Start()
{
	TRACEINTO;
	m_state = ACTIVE;

	//check if master already finished his features
	if (m_ivrCntl->IsMasterEndFeatures() == true)
	{
		TRACEINTO << "master already finished the features, so the slaves also need to finish";
		OnEndWait(); // so the slave should also finish
	}
}
/////////////////////////////////////////////////////////////////////////////

WORD CIvrSubWait::DoSomething()
{
	TRACEINTO;
	// do something on success
	m_ivrCntl->SetStartNextFeature(); //to skip to the next feature
	return 0;
}
/////////////////////////////////////////////////////////////////////////////

int CIvrSubWait::IsLegalString(CSegment* pParam)
{
	return DTMF_OK;
}
/////////////////////////////////////////////////////////////////////////////

void CIvrSubWait::OnEndWait()
{
	TRACEINTO;
	OnEndFeature(STATUS_OK);
}
/////////////////////////////////////////////////////////////////////////////

void CIvrSubWait::PlayRetryMessage(DWORD messageMode)
{
}
/////////////////////////////////////////////////////////////////////////////

