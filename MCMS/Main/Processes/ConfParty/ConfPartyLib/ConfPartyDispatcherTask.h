// DispatcherTask.h: interface for the CDispatcherTask class.
//
// ////////////////////////////////////////////////////////////////////

#if !defined(_CONFPARTY_DISPATCHERTASK_H__)
#define _CONFPARTY_DISPATCHERTASK_H__

#include "DispatcherTask.h"
#include "RsrcParams.h"

class CSipHeaderList;

enum eDispatchingMethods
{
	eDispatching_none            = 0,
	eDispatching_By_ConnectionId = 1,     // to specific statemachine
	eDispatching_By_ConfId,               // to conf task
	eDispatching_By_PartyId,              // to party task
	eDispatching_Lobby,                   // to lobby task
	eDispatching_Refer,                   // to conf - sip refer
/*IBM - EVENT PACKAGE */
	eDispatching_Sebscribe,               // to conf - sip subscribe
	eDispatching_NotifyAck,               // to conf - sip notify ack
/*IBM - EVENT PACKAGE */
	eDispatching_ConfPartyManager         // to Conf Party Manager
};

class CConfPartyDispatcherTask : public CDispatcherTask
{
	CLASS_TYPE_1(CConfPartyDispatcherTask, CDispatcherTask)

public:

	CConfPartyDispatcherTask(BOOL isSync);
	virtual ~CConfPartyDispatcherTask();
	virtual const char* NameOf() const               { return "CConfPartyDispatcherTask";}

	virtual int         GetTaskMbxBufferSize() const {return 4096*1024-1;} // 1M


	void                InitTask()                   { }
	BOOL                TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	virtual void        HandleOtherIdTypes(CMessageHeader& header);

	void                HandleMPLEvent(CSegment* pMsg);
	void                HandleCsEvent(CSegment* pMsg);
	void                HandleSignalingManagerEvent(CSegment* pManagerMsgSeg);

protected:
	eDispatchingMethods GetSuitedDispatchingMethod(CRsrcParams* Key) const;

	void                DispatchMplEventByPartyIdAndConnectionId(OPCODE opcode, CRsrcParams* pKeyForLookup, CSegment* pMsg) const;
	void                DispatchMplEventByConfId(OPCODE opcode, CRsrcParams* pKeyForLookup, CSegment* pMsg) const;
	void                DispatchMplEventByPartyId(OPCODE opcode, CRsrcParams* pKeyForLookup, CSegment* pMsg) const;
	void                DispatchToLobby(OPCODE opcode, CSegment* pMsg) const;
	void                DispatchToConfPartyManager(OPCODE opcode, CSegment* pMsg) const;
	void                DispatchReferEvent(DWORD data_len, char* pData, OPCODE opcode, CSegment* pMsg) const;
/*IBM - EVENT PACKAGE */
	void                DispatchSubscribeEvent(DWORD data_len, char* pData, OPCODE opcode, CSegment* pMsg) const;
	STATUS              DispatchSubscribeEventAccordingToTargetDialog(CSipHeaderList* pTempHeaders, OPCODE opcode, CSegment* pMsg) const;
	STATUS              DispatchSubscribeEventAccordingToTag(CSipHeaderList* pTempHeaders, OPCODE opcode, CSegment* pMsg) const;
	void                DispatchNotifyAck(ConfRsrcID dwConfId, PartyRsrcID dwPartyId, DWORD data_len, char* pData, OPCODE opcode, CSegment* pMsg) const;
/*IBM - EVENT PACKAGE */
	void                SendMsgToCS(DWORD cs_Id, OPCODE opcode, CSegment* pSeg) const;
	void                SendRejectAnswer(CSegment* pMsg) const;
	void                Send503Reject(CSegment* pMsg) const;
};

#endif // !defined(_CONFPARTY_DISPATCHERTASK_H__)

