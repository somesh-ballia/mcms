#ifndef __DTMFCOLL_H__
#define __DTMFCOLL_H__

#define MAX_DELIMITERS       4
#define DTMF_MAX_BUFFER_SIZE 32
#define OP_NOOP              0
#define OP_ERROR             200

#include "ConfPartyDefines.h"
#include "StateMachine.h"
#include <set>

class CPartyApi;
class CParty;
class CDTMFCodeList;


////////////////////////////////////////////////////////////////////////////
//                        CDtmfCollector
////////////////////////////////////////////////////////////////////////////
class CDtmfCollector : public CStateMachine
{
	CLASS_TYPE_1(CDtmfCollector, CStateMachine)

public:
	                    CDtmfCollector(CTaskApp* pOwerTask);
	                   ~CDtmfCollector();
	virtual const char* NameOf() const { return "CDtmfCollector";}

	void                Create( CParty* m_pParty, BOOL isExternalIVR = FALSE );
	void                SetDtmfWaitForXDigitsNumber(DWORD dtmfWaitDigitNum, DWORD timeOut, DWORD duration);
	void                SetDtmfDelimiter(char pinCodeDelimiter);
	void                ResetDtmfParams();
	void                ResetOtherCollectorParams();
	void                ResetDtmfBuffer();
	void                FillDtmfBuffer(const char* szDtmfString);
	int                 CheckDtmfForCurrentFeature();
	void                RestartTimeout(DWORD duration);
	CDTMFCodeList*      GetDtmfCodeList()                             {return m_pCDTMFCodeList;}

	void                SetDtmfTable(CDTMFCodeList* pCDTMFCodeList);
	void                StartFeature();
	void                EndFeature();
	void                SetDtmpOpcodePermission(WORD permissionLevel); //{ m_permissionLevel = permissionLevel; }
	void                InformAboutFirstNumbPressed()                 { m_bInformAboutFirstNumbPressed = 1; }
	void                ResetInformAboutFirstNumbPressed()            { m_bInformAboutFirstNumbPressed = 0; }
	WORD                GetDtmpOpcodePermission()                     { return m_permissionLevel; }
	void                SetRollCallFlag(int yesNo)                    { m_InRollCallMode = yesNo; }
	int                 IsInRollCallMode()                            { return m_InRollCallMode; }
	void                EndIvrPartSession()                           { m_inIVRsession = 0; }
	void                SetIvrPartSessionOn()                         { m_inIVRsession = 1; }
	void                SetNoisyLineIvr(int yesNo)                    { m_inNoisyLineIvr = yesNo; }
	void                InformAboutEveryDTMFPressed(WORD onOff)       { m_bSendEveryDTMFPressed = onOff; }

	virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	virtual void*       GetMessageMap();

	// action functions
	void                OnDtmfNull(CSegment* pParam);
	void                OnDtmfCollect(CSegment* pParam);
	void                OnTimerLastDetect(CSegment* pParam);
	void                OnTimerLastDetectNoWait(CSegment* pParam);
	void                OnDtmfCollectForPCM(CSegment* pParam);

public:
	BOOL                IsDtmfInBuffer();
	void                DumpDtmfTable();
	const char*         GetDtmfBuffer() { return m_dtmfAcc; }
	int                 GetDtmfIndex()  { return m_dtmfInd; }
	void                SetPartyRsrcID(DWORD rsrcPartyId);
	void                SetGwDTMFForwarding(BYTE val);
	void                SetInvitedPartyDTMFForwarding(BYTE val);

	void                PcmConnected();
	void                PcmDisconnected();

protected:
	void                RestartTimer();
	int                 InFeatureOpcodeAllowed();
	void                SendDTMFIdentFast(WORD opcode, WORD ind);
	void                SendDTMFIdent(WORD opcode_type, WORD ind = (WORD)-1, char* src = NULL, WORD dtmf_opcode_type = (WORD)-1);
	void                RemoveUsedDtmf(WORD i);
	int                 IsIdentifyDTMFFast(WORD* codeLen);
	const char*         BuildOpcodeString(WORD opcode);
	void                AddTimer(WORD timerId, DWORD duration);
	void                DelTimer(WORD timerId);

protected:
	DWORD               m_waitForDigits;
	DWORD               m_timeout;
	char                m_dtmfAcc[DTMF_MAX_BUFFER_SIZE + 1];
	DWORD               m_dtmfInd;
	DWORD               m_opcode;
	DWORD               m_partyRsrcId;
	CPartyApi*          m_pPartyApi;
	CDTMFCodeList*      m_pCDTMFCodeList;
	char                m_pinCodeDelimiter;
	int                 m_inFeature;
	int                 m_bIsDelimiter;
	int                 m_permissionLevel;

	int                 m_bInformAboutFirstNumbPressed;
	int                 m_InRollCallMode;
	int                 m_inIVRsession;
	int                 m_inNoisyLineIvr;
	int                 m_bSendEveryDTMFPressed;
	BYTE                m_bGwDtmfForward;
	BYTE                m_bInvitePartyDtmfForward;
	BOOL                m_isHidePsw;
    BOOL            	m_bKeepBufferOnLastNoWaitTimeout;
	std::set<WORD>      m_timers;

	PDECLAR_MESSAGE_MAP
};

#endif  // __DTMFCOLL_H__
