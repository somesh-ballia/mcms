#ifndef __SIPCONFPACKAGE_H__
#define __SIPCONFPACKAGE_H__

//===== Include Files =====
#include <vector>
#include "ObjString.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SIPMsftCX.h"
#include "SIPConfEventPKG.h"
#include "MplMcmsProtocol.h"
#include "ConfApi.h"
#include "ConfPartyManagerLocalApi.h"
#include "Conf.h"

#define MAX_BORAD_NUM   222
class CSIPReferEventPackageManager;


////////////////////////////////////////////////////////////////////////////
//                        CSIPSubscriber
////////////////////////////////////////////////////////////////////////////
class CSIPSubscriber : public CPObject
{
	CLASS_TYPE_1(CSIPSubscriber, CPObject )
	// public functions
public:
	CSIPSubscriber();
	CSIPSubscriber(WORD boardId, const char* pFrom, const char* pFromTag, const char* pTo, const char* pToTag, const char* pCallId,
	               DWORD ip, WORD port, WORD transport, WORD expires, WORD srcUnitId, DWORD callIndex, eRoleType role = eParticipant, DWORD cs_Id = 1);
	virtual ~CSIPSubscriber();
	virtual const char* NameOf() const {
		return "CSIPSubscriber";
	}

	friend WORD operator==(const CSIPSubscriber& first, const CSIPSubscriber& second);

	void        Refresh(DWORD ip, WORD port, WORD transport, const char* pCallId, WORD expires);
	char*       ToString();

	WORD        GetBoardId();
	char*       GetFrom();
	char*       GetFromTag();
	char*       GetTo();
	char*       GetToTag();
	char*       GetCallId();
	DWORD       GetIp();
	DWORD       GetCsId();
	WORD        GetPort();
	WORD        GetTransport();
	eRoleType   GetRole();
	WORD        GetSrcUnitId();
	DWORD       GetCallIndex();
	CStructTm   GetExpireTime();
	void        SetExpireTime(CStructTm* newTime);

	int         GetAndIncrementNotifyVersionCounter();

protected:
	char        m_pFromUri[MaxAddressListSize];
	char        m_pFromTag[MaxAddressListSize];
	char        m_pToUri[MaxAddressListSize];
	char        m_pToTag[MaxAddressListSize];
	char        m_pCallId[MaxAddressListSize];

	DWORD       m_fromIp;
	WORD        m_fromPort;
	WORD        m_transportType;                      //TCP UDP
	WORD        m_serviceID, m_srcUnitId;
	DWORD       m_csId;
	DWORD       m_callIndex;
	eRoleType   m_role;
	CStructTm   m_ExpiresTime;
	int         m_notifyVersionCounter;
};


////////////////////////////////////////////////////////////////////////////
//                        CSIPCxSubscriber
////////////////////////////////////////////////////////////////////////////
class CSIPCxSubscriber : public CSIPSubscriber
{
	CLASS_TYPE_1(CSIPCxSubscriber, CSIPSubscriber )
	// public functions
public:
	CSIPCxSubscriber();
	CSIPCxSubscriber(WORD boardId, const char* pFrom, const char* pTo, WORD expires, WORD watcherId, WORD connectionId, eRoleType role);
	virtual const char* NameOf() const {
		return "CSIPCxSubscriber";
	}

	void   Refresh(WORD expires, WORD watcherId);

	WORD   GetWatcherId();
	WORD   GetConnectionId();

protected:

	WORD m_watcherId;
	WORD m_mcmsConnId;

};


////////////////////////////////////////////////////////////////////////////
//                        CSIPEventPackageManager
////////////////////////////////////////////////////////////////////////////
class CSIPEventPackageManager : public CStateMachine
{
	CLASS_TYPE_1(CSIPEventPackageManager, CStateMachine )
	// public functions
public:
	CSIPEventPackageManager(COsQueue* pRcvMbx = NULL, CMplMcmsProtocol* pMplMcmsProtocol = NULL,
	                        CConfPartyManagerLocalApi* pMcuApi = NULL, CConfApi* pConfApi = NULL);
	virtual ~CSIPEventPackageManager();
	virtual const char* NameOf() const {
		return "CSIPEventPackageManager";
	}
	virtual void         HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	// action functions
	virtual void         OnSubscribeTout(CSegment* pParam);
	virtual void         OnNotifyTimerTout(CSegment* pParam);

	virtual void         ObserverUpdate(void* pSubscriber, WORD event, DWORD val);
	virtual void         Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId);
	virtual BYTE         Notify(CSIPSubscriber* pSubscriber, char* content, char* state, BYTE isDistribute = false);
	virtual BYTE         CheckRequestValidity(mcIndSubscribe* pSubscribeInd, enSipCodes* SipStatus);
	virtual CObjString*  BuildNotifyContent(BYTE bFull, BYTE bIsNewSubscriber = FALSE);
	virtual CObjString*  UpdateVersionInContent(CObjString* pStr, int version);
	virtual void         Dump(COstrStream& msg) const;
	virtual void         TerminateConf();
	virtual void         SetConfName(const char* pConfName);
	const char*          GetConfName();
	DWORD                GetCSeq();
	WORD                 CalcTimer();
	CSIPSubscriber*      FindParty(const char* from);
	virtual BYTE         FindParty(void* pSubscriber);

	//Don't use, but for TDD
	void                 ResetNotifyTimer();
	void                 ExpireTime();

protected:

	void         RemoveFromVector(CSIPSubscriber* pSubscriber);

	void         SendSipSubscribeResponse(mcIndSubscribe* pSubscribeInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId = eSipBalancer, DWORD serviceId = 0);
	void         SendSipNotify(mcReqNotify* pNotifyMsg, WORD srcUnitId = eSipBalancer, DWORD callIndex = 0, DWORD serviceId = 0);
	void         SendMsgToCS(DWORD cs_Id, OPCODE opcode, CSegment* pSeg);

	void         AttachEvents(CCommConf* pCommConf);
	void         DetachEvents(CCommConf* pCommConf);
	const char*  EventToString(const WORD event);


	BYTE                           m_runInTdd;
	CMplMcmsProtocol*              m_pMplMcmsProtocol;
	COsQueue*                      m_pRcvMbx;
	CConfPartyManagerLocalApi*     m_pConfPartyManagerApi;
	CConfApi*                      m_pConfApi;
	BYTE                           m_SubTimer, m_NotifyTimer, m_LoadTimer;
	int                            m_version;
	std::vector< CSIPSubscriber* > m_EventSubscribersList;
	CConfInfoType*                 m_pEventConfInfo;
	CObjString*                    m_pXMLStr;
	DWORD                          m_CSeq;
	char                           m_pConfName[H243_NAME_LEN];
	mcReqNotify*                   m_pNotifyMsg;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CSIPCXPackageManager
////////////////////////////////////////////////////////////////////////////
class CSIPCXPackageManager : public CSIPEventPackageManager
{
	CLASS_TYPE_1(CSIPCXPackageManager, CSIPEventPackageManager )

public:
	CSIPCXPackageManager(COsQueue* pRcvMbx = NULL, CMplMcmsProtocol* pMplMcmsProtocol = NULL,
	                     CConfPartyManagerLocalApi* pMcuApi = NULL, CConfApi* pConfApi = NULL);
	virtual ~CSIPCXPackageManager();
	virtual const char* NameOf() const {
		return "CSIPCXPackageManager";
	}

	virtual void         HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	virtual void         SetConfName(const char* pConfName);
	virtual void         ObserverUpdate(void* pSubscriber, WORD event, DWORD val);
//	virtual void	Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, eRoleType role);
	virtual void         Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId);

	CSIPCxSubscriber*    FindParty2(const char* from, WORD watcherId);
	BYTE                 FindPartyInInfo(DWORD partyId);
	int                  GetVersion();

	// action functions
	virtual BYTE         Notify(CSIPSubscriber* pSubscriber, char* content, char* state);
	virtual DWORD        MediaCheckSum(char* pBuff, int len);
	virtual BYTE         CheckRequestValidity(mcIndSubscribe* pSubscribeInd, enSipCodes* SipStatus);
	virtual CObjString*  BuildNotifyContent(BYTE bFull, BYTE bNewSubscriber = FALSE);
	virtual void         TerminateConf();

	virtual void         OnSubscribeTout(CSegment* pParam);
	virtual void         OnNotifyTimerTout(CSegment* pParam);
	void                 OnLoadManagerAccept();

protected:

	void         OnTimeoutLoadManager(CSegment* pParam);
	void         AttachEvents(CCommConf* pCommConf);
	void         DetachEvents(CCommConf* pCommConf);
	CObjString*  UpdateVersionInContent(CObjString* pStr, int version);
	char*        GetRedirectIp(WORD orgBoardId, WORD newBoardId);
	const char*  EventToString(const WORD event);

	CCXConfInfoType* m_pConfInfo;
	BYTE             m_serviceID[MAX_BORAD_NUM]; //**********
	CObjString*      m_pXMLStr;

	PDECLAR_MESSAGE_MAP
};

////////////////////////////////////////////////////////////////////////////
//                        CSIPEventPackageDispatcher
////////////////////////////////////////////////////////////////////////////
class CSIPEventPackageDispatcher : public CPObject
{
	CLASS_TYPE_1(CSIPEventPackageDispatcher, CPObject )

public:
	CSIPEventPackageDispatcher(COsQueue* pRcvMbx = NULL, CMplMcmsProtocol* pMplMcmsProtocol = NULL, CConfPartyManagerLocalApi* pConfPartyManagerApi = NULL);
	virtual ~CSIPEventPackageDispatcher();
	virtual const char* NameOf() const {
		return "CSIPEventPackageDispatcher";
	}

	void             Subscribe(mcIndSubscribe* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId);
	void             Refer(mcIndRefer* pSubscribeInd, DWORD callIndex, WORD srcUnitId, DWORD serviceId);

	CSIPSubscriber*  FindParty(char* from, DWORD CSeq, char* callId);
	CSIPSubscriber*  FindPartyInSIPCX(const char* pFromStr);
	void             Dump();
	void             TerminateConf();
	void             HandleObserverUpdate(CSegment* pSeg, WORD type);
	void             LoadManagerAccept();

	//Don't use, but for TDD
	void             ResetNotifyTimer();

protected:

	void   SendSipSubscribeResponse(mcIndSubscribe* pSubscribeInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId = 0, DWORD serviceId = 0);
	void   SendSipReferResponse(mcIndRefer* pReferInd, enSipCodes sipCode, DWORD callIndex, WORD srcUnitId, DWORD serviceId);
	void   SendMsgToCS(DWORD cs_Id, OPCODE opcode, CSegment* pSeg);

	BYTE m_terminating;

	CSIPEventPackageManager*      m_pSipConfPackage;
	CSIPCXPackageManager*         m_pSipCXPackage;
	CSIPReferEventPackageManager* m_pSipReferPackage;

	CMplMcmsProtocol* m_pMplMcmsProtocol;
};

#endif
