#include "NStream.h"
#include "IVRDtmfColl.h"
#include "StateMachine.h"
#include "PartyApi.h"
#include "IVRService.h"
#include "Trace.h"
#include "Party.h"
#include "ConfPartyOpcodes.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsAudio.h"
#include "SysConfig.h"
#include "PrettyTable.h"
#include "IVRCntlLocal.h"

const WORD DTMF_COLL_NOTACTIVE      = 1;
const WORD DTMF_COLL_COLLECTING     = 2;
const WORD DTMF_COLL_COLLECTING_PCM = 3;

const WORD LAST_DTMF_TIMER          = 63000;
const WORD LAST_DTMF_NO_WAIT_TIMER  = 63001;

using namespace std;

PBEGIN_MESSAGE_MAP(CDtmfCollector)
	ONEVENT(AUD_DTMF_IND_VAL,          DTMF_COLL_NOTACTIVE,      CDtmfCollector::OnDtmfNull)
	ONEVENT(AUD_DTMF_IND_VAL,          DTMF_COLL_COLLECTING,     CDtmfCollector::OnDtmfCollect)
	ONEVENT(SIGNALLING_DTMF_INPUT_IND, DTMF_COLL_NOTACTIVE,      CDtmfCollector::OnDtmfNull)
	ONEVENT(SIGNALLING_DTMF_INPUT_IND, DTMF_COLL_COLLECTING,     CDtmfCollector::OnDtmfCollect)
	ONEVENT(RTP_DTMF_INPUT_IND,        DTMF_COLL_NOTACTIVE,      CDtmfCollector::OnDtmfNull)
	ONEVENT(RTP_DTMF_INPUT_IND,        DTMF_COLL_COLLECTING,     CDtmfCollector::OnDtmfCollect)
	ONEVENT(LAST_DTMF_TIMER,           DTMF_COLL_NOTACTIVE,      CDtmfCollector::OnTimerLastDetect)
	ONEVENT(LAST_DTMF_TIMER,           DTMF_COLL_COLLECTING,     CDtmfCollector::OnTimerLastDetect)
	ONEVENT(LAST_DTMF_NO_WAIT_TIMER,   DTMF_COLL_COLLECTING,     CDtmfCollector::OnTimerLastDetectNoWait)
	ONEVENT(AUD_DTMF_IND_VAL,          DTMF_COLL_COLLECTING_PCM, CDtmfCollector::OnDtmfCollectForPCM)
	ONEVENT(SIGNALLING_DTMF_INPUT_IND, DTMF_COLL_COLLECTING_PCM, CDtmfCollector::OnDtmfCollectForPCM)
	ONEVENT(RTP_DTMF_INPUT_IND,        DTMF_COLL_COLLECTING_PCM, CDtmfCollector::OnDtmfCollectForPCM)
PEND_MESSAGE_MAP(CDtmfCollector, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CDtmfCollector
////////////////////////////////////////////////////////////////////////////
CDtmfCollector::CDtmfCollector(CTaskApp* pOwerTask)
               :CStateMachine(pOwerTask)
{
	m_state                        = DTMF_COLL_NOTACTIVE;
	m_timeout                      = 0; // seconds
	m_waitForDigits                = 0;
	m_partyRsrcId                  = 0;
	m_pPartyApi                    = 0;
	m_pCDTMFCodeList               = 0;
	m_pinCodeDelimiter             = ' ';
	m_inFeature                    = 0;
	m_bIsDelimiter                 = 0;
	m_permissionLevel              = DTMF_USER_ACTION;
	m_dtmfInd                      = 0; // start new string
	memset( m_dtmfAcc, 0, DTMF_MAX_BUFFER_SIZE + 1);	// zero the current string
	m_bInformAboutFirstNumbPressed = 0;
	m_InRollCallMode               = 0;
	m_inIVRsession                 = 0;
	m_inNoisyLineIvr               = 0;
	m_bSendEveryDTMFPressed        = 0;
	m_bGwDtmfForward               = FALSE;
	m_bInvitePartyDtmfForward      = FALSE;
    m_bKeepBufferOnLastNoWaitTimeout = FALSE;
	m_isHidePsw                    = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("HIDE_CONFERENCE_PASSWORD", m_isHidePsw);
	ResetDtmfParams();
	
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CDtmfCollector::~CDtmfCollector()
{
	DelTimer(LAST_DTMF_TIMER);
	DelTimer(LAST_DTMF_NO_WAIT_TIMER);

	DeleteAllTimers();
	if (m_pPartyApi)
		m_pPartyApi->DestroyOnlyApi();

	POBJDELETE(m_pPartyApi);
	POBJDELETE(m_pCDTMFCodeList);
}

//--------------------------------------------------------------------------
void* CDtmfCollector::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CDtmfCollector::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	switch (opCode)
	{
		case TIMER:
		{
			PTRACE(eLevelInfoNormal,"CDtmfCollector::HandleEvent : \'TIMER EVENT\'  ");
			break;
		}

		default:
		{ // all other messages
			DispatchEvent(opCode, pMsg);
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::Create( CParty* pParty, BOOL isExternalIVR )
{
	if (!pParty) {
		PTRACE(eLevelError, "CDtmfCollector::Create: Illegal pParty");
		return;
	}

	// set current state
	m_state = DTMF_COLL_COLLECTING;

	// init messages API
	m_pPartyApi = new CPartyApi;
	m_pPartyApi->CreateOnlyApi(pParty->GetRcvMbx(), NULL, (pParty->GetLocalQueue()));
	if (isExternalIVR)
		m_bKeepBufferOnLastNoWaitTimeout = TRUE;
}

//--------------------------------------------------------------------------
void CDtmfCollector::SetPartyRsrcID(DWORD rsrcPartyId)
{
	m_partyRsrcId = rsrcPartyId;
}
//--------------------------------------------------------------------------
void CDtmfCollector::SetDtmfWaitForXDigitsNumber(DWORD dtmfWaitDigitNum, DWORD timeOut, DWORD duration)
{
	// set digits to wait
	m_waitForDigits = dtmfWaitDigitNum;
	if (m_waitForDigits >= DTMF_MAX_BUFFER_SIZE)
		m_waitForDigits = DTMF_MAX_BUFFER_SIZE-1;

	// start timer
	m_timeout = timeOut;

	RestartTimeout(duration);
}

//--------------------------------------------------------------------------
void CDtmfCollector::RestartTimeout(DWORD duration)
{
	DelTimer(LAST_DTMF_TIMER);

	if (m_timeout > 0)
	{
		DWORD timer_val = (m_timeout + duration) * SECOND;
		TRACEINTO << "starting CDtmfCollector LAST_DTMF_TIMER timer for " << timer_val * 10  << " msecs.";
		AddTimer(LAST_DTMF_TIMER, timer_val);
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::SetDtmfDelimiter(char pinCodeDelimiter)
{
	m_pinCodeDelimiter = pinCodeDelimiter;
	m_bIsDelimiter     = (m_pinCodeDelimiter == '#' || m_pinCodeDelimiter == '*');
}

//--------------------------------------------------------------------------
void CDtmfCollector::StartFeature()
{
  PTRACE2INT( eLevelInfoNormal, "CDtmfCollector::StartFeature, state is ", GetState() );
	m_inFeature = 1;
	if (m_bKeepBufferOnLastNoWaitTimeout && strlen(m_dtmfAcc) > 0)
	{
		TRACEINTO << " previously typed dtmfs will be checked - ";
		// checks for DTMF opcode, or for a string (if in feature)
		BOOL isFeatureFound = CheckDtmfForCurrentFeature();
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::EndFeature()
{
	PTRACE(eLevelInfoNormal, "CDtmfCollector::EndFeature");
	//PBTTRACE(TRUE, "ending DTMF collector feature");
	DelTimer(LAST_DTMF_TIMER);

	m_inFeature = 0;
	m_timeout   = 0;
	m_bInformAboutFirstNumbPressed = 0;
}

//--------------------------------------------------------------------------
void CDtmfCollector::ResetOtherCollectorParams()
{
	m_bIsDelimiter                 = 0;
	m_inFeature                    = 0;
	m_pinCodeDelimiter             = ' ';
	m_bInformAboutFirstNumbPressed = 0;
	m_InRollCallMode               = 0;
	m_inNoisyLineIvr               = 0;
	m_bSendEveryDTMFPressed        = 0;
}

//--------------------------------------------------------------------------
void CDtmfCollector::OnDtmfNull(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CDtmfCollector::OnDtmfNull");
}

//--------------------------------------------------------------------------
void CDtmfCollector::OnDtmfCollect(CSegment* pParam)
{
	// stop last detect DTMF timer
	DelTimer(LAST_DTMF_NO_WAIT_TIMER);

	// retrieves the DTMF characters
	DWORD unLength;
	*pParam >> unLength;

	TRACECOND_AND_RETURN(!unLength, "Failed, DTMF string length is 0");

	unLength = min((int)unLength, DTMF_MAX_BUFFER_SIZE);

	// gets the DTMF characters into the accumulator buffer
	char bNum;
	while (m_dtmfInd < DTMF_MAX_BUFFER_SIZE)
	{
		*pParam >> (BYTE &)bNum;
		m_dtmfAcc[m_dtmfInd++] = bNum;
		// if we were asked to inform about first DTMF pressed
		if (m_bInformAboutFirstNumbPressed)
		{
			SendDTMFIdent(FIRST_DTMF_WAS_DIALED, 1);
			m_bInformAboutFirstNumbPressed = 0;
		}

		if (m_bSendEveryDTMFPressed)
			SendDTMFIdent(FIRST_DTMF_WAS_DIALED, 1);

		unLength--;
		if (0 == unLength)          // end of DTMF characters
			break;
	}

	if (m_bSendEveryDTMFPressed)  // send all DTMF to IvrFeature
	{
		m_dtmfInd    = 0;
		m_dtmfAcc[0] = 0;
		TRACEINTO << "Send every DTMF pressed, nothing more to do";
		return; // nothing more to do
	}

	m_dtmfAcc[m_dtmfInd] = 0;

	TRACEINTO << "CollectedDTMF:'" << (m_isHidePsw ? "********" : m_dtmfAcc) << "'";

	if (m_bGwDtmfForward || m_bInvitePartyDtmfForward)
	{
		TRACEINTO << "GwDtmfForward:" << (int)m_bGwDtmfForward << ", InvitePartyDtmfForward:" << (int)m_bInvitePartyDtmfForward << " - Send DTMF forwarding";

		m_dtmfAcc[m_dtmfInd] = '\0';
		m_pPartyApi->SendDTMFForwarding(m_dtmfAcc, m_bGwDtmfForward ? GW_DTMF_FORWARD : INVITED_DTMF_FORWARD);
		m_dtmfInd    = 0;
		m_dtmfAcc[0] = 0;
	}
	else
	{
		// EE-462 enable IVR for MFW
//		if (CProcessBase::GetProcess()->GetProductType() == eProductTypeSoftMCUMfw)
//		{
//			TRACEINTO << "GwDtmfForward:" << (int)m_bGwDtmfForward << ", InvitePartyDtmfForward:" << (int)m_bInvitePartyDtmfForward << " - Send DTMF forwarding because of product type eProductTypeSoftMCUMfw";
//			m_dtmfAcc[m_dtmfInd] = '\0';
//			m_pPartyApi->SendDTMFForwarding(m_dtmfAcc, IVR_SERVER_DTMF_FORWARD);
//			m_dtmfInd    = 0;
//			m_dtmfAcc[0] = 0;
//		}

		// checks for DTMF opcode, or for a string (if in feature)
		CheckDtmfForCurrentFeature();

		// a new DTMF detect (start the timer again)
		RestartTimer();
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::OnDtmfCollectForPCM(CSegment* pParam)
{
	// retrieves the DTMF characters
	DWORD unLength;
	*pParam >> unLength;

	TRACECOND_AND_RETURN(!unLength, "Failed, DTMF string length is 0");

	unLength = min((int)unLength, DTMF_MAX_BUFFER_SIZE);

	// gets the DTMF characters into the accumulator buffer
	char bNum[2];
	bNum[1] = '\0';
	for (DWORD i = 0; i < unLength; i++)
	{
		*pParam >> (BYTE &)bNum[0];
		m_pPartyApi->ForwardDtmfForPCM(bNum);
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::SetGwDTMFForwarding(BYTE val)
{
	if (m_bGwDtmfForward != val)
	{
		// we want to reset the previous data only if there was a change
		m_bGwDtmfForward = val;
		ResetDtmfParams();
		ResetDtmfBuffer();
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::SetInvitedPartyDTMFForwarding(BYTE val)
{
	if (m_bInvitePartyDtmfForward != val)
	{
		// we want to reset the previous data only if there was a change
		m_bInvitePartyDtmfForward = val;
		ResetDtmfParams();
		ResetDtmfBuffer();
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::PcmConnected()
{
	if (m_state != DTMF_COLL_COLLECTING_PCM)
	{
		m_state = DTMF_COLL_COLLECTING_PCM;
		ResetDtmfParams();
		ResetDtmfBuffer();
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::PcmDisconnected()
{
	m_state = DTMF_COLL_COLLECTING;
}

//--------------------------------------------------------------------------
int CDtmfCollector::CheckDtmfForCurrentFeature()
{
	// 1. search if the new DTMF string matches one of the table strings
	// 2. if '1' is TRUE: sends this string / opcode, removes used DTMF
	// 3. if 'wait for X characters' is the current state, if we have X
	// characters then sends this string, removes used DTMF
	// 4. updates parameters.

	m_dtmfAcc[DTMF_MAX_BUFFER_SIZE] = '\0';

	TRACEINTO << "CollectedDTMF:'" << (m_isHidePsw ? "********" : m_dtmfAcc) << "', PartyPermissionLevel:" <<  m_permissionLevel << ", IsInFeature:" << m_inFeature << ", is delimiter: " <<  (int)m_bIsDelimiter << ", m_waitForDigits: " << m_waitForDigits;

	TRACECOND_AND_RETURN_VALUE(!m_dtmfInd, "Failed, DTMF index is 0", 0);

	WORD bIsDelimiter = (m_bIsDelimiter && (0 == m_waitForDigits));
	m_opcode = OP_NOOP;

	int ident = 0;
	DWORD digit_num = 0;
	WORD dtmf_opcode = 0;

	char temp[DTMF_MAX_BUFFER_SIZE+1];
	temp[0] = '\0';
	// in case of DELIMITER mode and middle of feature,
	if (m_inFeature && bIsDelimiter)
	{
		// checks if
		WORD i = 0;
		for (i = 0; i < m_dtmfInd; i++)
		{
			if (m_pinCodeDelimiter == m_dtmfAcc[i])
			{
				ident = 1; // the required number of digits was received
				PTRACE( eLevelInfoNormal, "CDtmfCollector::CheckDtmfForCurrentFeature (found delimiter): Sending CHARACTERS:  " );
				digit_num = i;
				dtmf_opcode = DTMF_STRING_IDENT;
//				// send message.....
//				SendDTMFIdent(DTMF_STRING_IDENT, i);

				// removes the DTMF used from the buffer
				int x = 0;
				for (x = 0; x < i && x < DTMF_MAX_BUFFER_SIZE; x++)
					temp[x] = m_dtmfAcc[x];
				temp[x] = '\0';

				TRACEINTO << "Delimiter:" << (m_isHidePsw ? "********" : temp);

				RemoveUsedDtmf(i+1);

				break;
			}
		}
	}

	if (0 == ident)
	{
		// checks if 'wait for x DTMF' is ON (still need to decide who is "stronger": x or opcode if exists)
		if (m_inFeature && (0 < m_waitForDigits))
		{
			if (m_dtmfInd >= m_waitForDigits)
			{
				ident = 1; // the required number of digits was received
				digit_num = m_waitForDigits;
				dtmf_opcode = DTMF_STRING_IDENT;
//				// send message.....
//				SendDTMFIdent(DTMF_STRING_IDENT, m_waitForDigits);

				WORD x = 0;
				for (x = 0; x < m_waitForDigits; x++)
					temp[x] = m_dtmfAcc[x];
				temp[x] = 0;

				TRACEINTO << "WaitForDigits:" << (m_isHidePsw ? "********" : temp);

				RemoveUsedDtmf(m_waitForDigits);
			}
		}
	}

	if (IsInRollCallMode())
	{
		m_pPartyApi->SendOpcodeToIvrSubFeature(DTMF_RECORD_REQUEST);
	}

	// try to find from the DTMF strings table (if during a vote, consider only STOP_VOTE code)
	if (0 == ident)
	{
		// ***newFastDtmf ---
		WORD codeLen;
		ident = IsIdentifyDTMFFast(&codeLen);
		if (0 != ident)   // predefined was found
		{
			// checks for the opcodes who permitted in a middle of feature
			int featureOpcodeAllowed = InFeatureOpcodeAllowed();
			if ((m_inIVRsession || m_inFeature) && !featureOpcodeAllowed)
			{
				TRACEINTO << "IsInIvrSession:" << m_inIVRsession << ", IsInFeature:" << m_inFeature << ", IsFeatureOpcodeAllowed:" << featureOpcodeAllowed;
				ident = 0;   // set as not identified
			}
			else // either not in a middle of feature or a permitted one
			{
				// send message.....
				m_dtmfAcc[codeLen] = 0;     // clears the rest of the buffer
				WORD x = 0;
				for (x = 0; x < codeLen; x++)
					temp[x] = m_dtmfAcc[x];
				temp[x] = 0;
				TRACEINTO << "CollectedDTMF:'" << (m_isHidePsw ? "********" : m_dtmfAcc) << "', Opcode:" << BuildOpcodeString(m_opcode) << ", IsInNoisyLineIvr:" << m_inNoisyLineIvr;

				if ((m_inNoisyLineIvr == YES) && m_opcode != DTMF_PLAY_MENU)
					m_pPartyApi->SendOpcodeToIvrSubFeature(m_opcode);
				else
				{
					digit_num = codeLen;
					dtmf_opcode = DTMF_OPCODE_IDENT;
					//SendDTMFIdent(DTMF_OPCODE_IDENT, codeLen);
				}

				ResetDtmfBuffer();
			}
		}
	}

	// reset timers
	if (0 != ident)
	{
		WORD cur_opcode = m_opcode;
		ResetDtmfParams();
		if (digit_num != 0)
		{
			SendDTMFIdent(dtmf_opcode, digit_num, temp, cur_opcode);
			TRACEINTO << "DTMF found:" << temp;
		}
		return 1; // DTMF was found
	}

	TRACEINTO << "DTMF not found";

	return 0;  // DTMF was not found
}

//--------------------------------------------------------------------------
void CDtmfCollector::OnTimerLastDetect(CSegment* pParam)
{
	TRACEINTO << "Reset DTMF";

	m_timers.erase(LAST_DTMF_TIMER); // no timer
	m_opcode = OP_ERROR;  // 28-10-02 - in any case!

	SendDTMFIdent(DTMF_STRING_IDENT);

	// reset DTMF string
	ResetDtmfParams();
	ResetDtmfBuffer();
}

//--------------------------------------------------------------------------
void CDtmfCollector::OnTimerLastDetectNoWait(CSegment* pParam)
{
	if (IsValidTimer(LAST_DTMF_TIMER))
	{
		TRACEINTO << "LAST_DTMF_TIMER is active- skipping buffer reset.";
		return;
	}
	else if (m_bKeepBufferOnLastNoWaitTimeout)
		TRACEINTO << "keeping dtmf buffer .";
	else
	{
		PTRACE( eLevelInfoNormal, "CDtmfCollector::OnTimerLastDetectNoWait - clearing dtmf buffer" );
		// reset DTMF string
		ResetDtmfBuffer();
	}
	m_timers.erase(LAST_DTMF_NO_WAIT_TIMER); // no timer

}

//--------------------------------------------------------------------------
void CDtmfCollector::ResetDtmfParams()
{
	DelTimer(LAST_DTMF_TIMER);
	DelTimer(LAST_DTMF_NO_WAIT_TIMER);

	m_opcode = OP_NOOP;
	m_waitForDigits = 0;
}

//--------------------------------------------------------------------------
int CDtmfCollector::IsIdentifyDTMFFast(WORD* codeLen)
{
	TRACECOND_AND_RETURN_VALUE(!m_pCDTMFCodeList, "Failed, invalid DTMF code list", 0);

	CDTMFCode* pDtmfCode = m_pCDTMFCodeList->GetFirstDTMFCode();
	TRACECOND_AND_RETURN_VALUE(!pDtmfCode, "Failed, invalid first DTMF code", 0);

	// search for a string in the table
	int curLen = m_dtmfInd;

	// here we are in 'In Conf' state...
	int numOfStrings = m_pCDTMFCodeList->GetDTMFCodeNumber();

	DumpDtmfTable();

	for (int i = 0; i < numOfStrings; i++)
	{
		if (i > 0)
		{
			pDtmfCode = m_pCDTMFCodeList->GetNextDTMFCode();
			if (!pDtmfCode)
				break;
		}

		const char* str = pDtmfCode->GetDTMFStr();
		if (NULL == str)
			continue;

		if (m_inNoisyLineIvr == 0)
		{
			switch (pDtmfCode->GetDTMFOpcode())
			{
				case DTMF_NOISY_LINE_UNMUTE:
				case DTMF_NOISY_LINE_ADJUST:
				case DTMF_NOISY_LINE_DISABLE:
				case DTMF_NOISY_LINE_MUTE:
				case DTMF_NOISY_LINE_HELP_MENU:
					continue;
				default:
					break;
			}
		}

		if (IsInRollCallMode())
			return 1;

		if (m_dtmfAcc[0] != str[0])
			continue;

		int preLen = strlen(str); // should be predefined string length to save calculations
		if (preLen > curLen)      // current string shorter then the predefined one
			continue;

		if (0 == strncmp(m_dtmfAcc, str, preLen))
		{
			// found, gets the opcode
			WORD opcode     = pDtmfCode->GetDTMFOpcode();
			WORD permission = pDtmfCode->GetDTMFPermission();
			if (m_permissionLevel < permission) // only for a leader permission
			{
				TRACEINTO << "PartyPermissionLevel:" <<  m_permissionLevel << ", DtmfPermissionLevel:" << permission << ", Opcode:" << BuildOpcodeString(m_opcode) << " - DTMF found, but not allowed";
				return 0; // DTMF not found
			}

			m_opcode   = opcode;
			(*codeLen) = preLen;
			return 1; // DTMF found
		}
	}
	PTRACE(eLevelInfoNormal,"CDtmfCollector::IsIdentifyDTMFFast 0 : not indentified ");
	return 0; // DTMF not found
}

//--------------------------------------------------------------------------
void CDtmfCollector::SendDTMFIdent(WORD opcode_type, WORD ind, char* src, WORD dtmf_opcode)
{
	CSegment* seg = new CSegment;

	char *src_buf = (src != NULL && strlen(src)>0)? src : m_dtmfAcc;
	if (ind == (WORD)-1)
		ind = m_dtmfInd;
	if (dtmf_opcode == (WORD)-1)
		dtmf_opcode = m_opcode;

	*seg << (WORD)dtmf_opcode;     // Internal DTMF op_code	(OP_ERROR, OP_NOOP or one from the table)
	*seg << (WORD)ind;          // number of characters
	for (WORD i = 0; i < ind; i++)
		*seg << (BYTE &)src_buf[i];
	TRACEINTO << "sending opcode - " << dtmf_opcode << ", num of digits:" << ind;

	m_pPartyApi->sendPartyArqInd(seg, opcode_type, 0);
}

//--------------------------------------------------------------------------
void CDtmfCollector::SetDtmfTable(CDTMFCodeList* pCDTMFCodeList)
{
	POBJDELETE(m_pCDTMFCodeList);

	if (pCDTMFCodeList)
		m_pCDTMFCodeList = new CDTMFCodeList(*pCDTMFCodeList);
	else
		m_pCDTMFCodeList = new CDTMFCodeList;
}

//--------------------------------------------------------------------------
void CDtmfCollector::RestartTimer()
{
	DelTimer(LAST_DTMF_TIMER);
	DWORD timer_val = 0;
	if (m_timeout > 0)    // middle of feature
	{
		timer_val = m_timeout * SECOND;
		TRACEINTO << "starting CDtmfCollector LAST_DTMF_TIMER timer for " << timer_val*10  << " msecs.";
		AddTimer(LAST_DTMF_TIMER, timer_val);
	}
	else // help state (feature) or InConf state (not in feature)
	{
		DelTimer(LAST_DTMF_NO_WAIT_TIMER);
		timer_val = 3*2 * SECOND;
		TRACEINTO << "starting CDtmfCollector LAST_DTMF_NO_WAIT_TIMER timer for " << timer_val*10  << " msecs.";
		AddTimer(LAST_DTMF_NO_WAIT_TIMER, timer_val);
	}
}

//--------------------------------------------------------------------------
int CDtmfCollector::InFeatureOpcodeAllowed()
{
	// Noisy Line
	if (m_inNoisyLineIvr)
	{
		switch (m_opcode)
		{
			case DTMF_NOISY_LINE_UNMUTE:
			case DTMF_NOISY_LINE_ADJUST:
			case DTMF_NOISY_LINE_DISABLE:
			case DTMF_NOISY_LINE_MUTE:
			case DTMF_NOISY_LINE_HELP_MENU:
				return 1;
			default:
				return 0;
		}
	}
	return 0;
}

//--------------------------------------------------------------------------
void CDtmfCollector::RemoveUsedDtmf(WORD i)
{
	// remove used DTMF from buffer
	if (i == m_dtmfInd) // removes all
	{
		ResetDtmfBuffer();
	}
	else
	{
		WORD j = 0;
		for (j = 0; j < (m_dtmfInd - i); j++)
			m_dtmfAcc[j] = m_dtmfAcc[i + j];

		m_dtmfAcc[j] = 0;
		m_dtmfInd    = j;
	}
}

//--------------------------------------------------------------------------
void CDtmfCollector::ResetDtmfBuffer()
{
	m_dtmfInd = 0;                           // start new string
	memset(m_dtmfAcc, 0, sizeof(m_dtmfAcc)); // zero the current string
}

//--------------------------------------------------------------------------
void CDtmfCollector::FillDtmfBuffer(const char* szDtmfString)
{
	if (szDtmfString)
	{
		int len = strlen(szDtmfString);

		if (len > 0 && len <= DTMF_MAX_BUFFER_SIZE)
		{
			m_dtmfInd = len;
			SAFE_COPY(m_dtmfAcc, szDtmfString);
		}
	}
}

//--------------------------------------------------------------------------
BOOL CDtmfCollector:: IsDtmfInBuffer()
{
	return (BOOL)(m_dtmfInd > 0);
}

//--------------------------------------------------------------------------
void CDtmfCollector::DumpDtmfTable()
{
	TRACECOND_AND_RETURN(!m_pCDTMFCodeList, "Failed, invalid DTMF code list");

	// here we are in 'In Conf' state...
	int numOfStrings = m_pCDTMFCodeList->GetDTMFCodeNumber();
	TRACECOND_AND_RETURN(!numOfStrings, "Failed, DTMF code list is empty");

	if (numOfStrings >= MAX_DTMF_CODE_NUM)
	{
		TRACEINTO << "Size:" << numOfStrings << " - Illegal DTMF code list size";
		numOfStrings = MAX_DTMF_CODE_NUM - 1;
	}

	CPrettyTable<const char*, const char*, WORD> tbl("Opcode", "DTMF", "Permission");

	for (int i = 0; i < numOfStrings; i++)
	{
		// get entry
		CDTMFCode* pDtmfCode = m_pCDTMFCodeList->m_DTMF_code[i];
		if (!pDtmfCode)
		{
			TRACEINTO << "Index:" << i << " - Illegal DTMF code list entry";
			break;
		}

		const char* dtmfString = pDtmfCode->GetDTMFStr();
		WORD dtmfOpcode        = pDtmfCode->GetDTMFOpcode();
		WORD dtmfPermission    = pDtmfCode->GetDTMFPermission();

		tbl.Add(BuildOpcodeString(dtmfOpcode), dtmfString ? dtmfString : "Empty", dtmfPermission);
	}
	tbl.Sort(0);
	TRACEINTO << tbl.Get();
}

//--------------------------------------------------------------------------
const char* CDtmfCollector::BuildOpcodeString(WORD opcode)
{
	static char buffer[256];
	snprintf(buffer, sizeof(buffer), "%s (#%d)", CIvrCntlLocal::DtmfCodeToString(opcode), opcode);
	return buffer;
}

//--------------------------------------------------------------------------
void CDtmfCollector::SetDtmpOpcodePermission(WORD permissionLevel)
{
	//PBTTRACESTREAM(1, "PartyPermissionLevel:" << permissionLevel);
	TRACEINTO << "PartyPermissionLevel:" << permissionLevel;
	m_permissionLevel = permissionLevel;
}

//--------------------------------------------------------------------------
void CDtmfCollector::AddTimer(WORD timerId, DWORD duration)
{
	DelTimer(timerId);
	StartTimer(timerId, duration);
	m_timers.insert(timerId);
}

//--------------------------------------------------------------------------
void CDtmfCollector::DelTimer(WORD timerId)
{
	std::set<WORD>::iterator _ii = m_timers.find(timerId);
	if (_ii == m_timers.end())
		TRACEINTO << "timer id " << timerId << " not found";
	DeleteTimer(timerId);
}
