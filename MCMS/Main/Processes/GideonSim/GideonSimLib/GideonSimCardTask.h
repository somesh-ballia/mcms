//+========================================================================+
//                   GideonSimCardTask.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimCardTask.h                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+
#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __GIDEONSIMCARDTASK_
#define __GIDEONSIMCARDTASK_


#include "TaskApp.h"
#include "MplMcmsStructs.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCardMngrTB.h"


class CClientSocket;
class CMplMcmsProtocol;
class CGideonSimSwitchLogical;
class CGideonSimMfaLogical;
class CGideonSimBarakLogical;
class CGideonSimRtmLogical;


const BYTE MAX_MFA_LOGICS_IN_LITE_CARD = 1;


/////////////////////////////////////////////////////////////////////////////
//
//   SimBasicCard - base class (abstract) for all simulation cards
//
/////////////////////////////////////////////////////////////////////////////
//  no task creation function (abstract class)

class CSimBasicCard  : public CTaskApp
{
CLASS_TYPE_1(CSimBasicCard,CTaskApp )
public:
			// Constructors
	CSimBasicCard();
	virtual ~CSimBasicCard();
	virtual const char* NameOf() const { return "CSimBasicCard";}

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
	virtual void DispatchMcmsMsg(CSegment* pMsg) const = 0;	

protected:
			// Action functions
//	void OnMngrConnectSocketIdle(CSegment* pMsg);

protected:
			// Utilities
	virtual void  SelfKill(); //Override
	BOOL  IsSingleton() const {return NO;}
	void  InitTask(){;}
	const char* GetTaskName() const;

	void OnTerminalCommand(CMplMcmsProtocol* pMplProtocol) const;

	void  FillMplProtocol( CMplMcmsProtocol* pMplProt,
			const DWORD opcode,const BYTE* pData,const DWORD nDataLen) const;
	

			// Attributes
	WORD  m_wBoardId;
	WORD  m_wSubBoardId;
	char	m_szSerialNumber[MPL_SERIAL_NUM_LEN];

	PDECLAR_MESSAGE_MAP
};



/////////////////////////////////////////////////////////////////////////////
//
//   SimSocketedCard - simulation card class (abstract) with socket connection
//
/////////////////////////////////////////////////////////////////////////////
//  no task creation function (abstract class)

class CSimSocketedCard  : public CSimBasicCard
{
CLASS_TYPE_1(CSimSocketedCard,CSimBasicCard )
public:
			// Constructors
	CSimSocketedCard();
	virtual ~CSimSocketedCard();
	virtual const char* NameOf() const { return "CSimSocketedCard";}

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
// 	virtual void DispatchMcmsMsg(CSegment* pMsg) const = 0;
	void  SendToSocket(CSegment& paramSegment) const; // temp func for Talya
	void  SendToMplApi(CMplMcmsProtocol& rMplProtocol) const;
	void  SendToMplApi(CSegment& rParam) const;

protected:
			// Action functions
	// Manager: connect card
	virtual void OnMngrConnectSocketIdle(CSegment* pMsg);
	// Socket: connection established
	virtual void OnSocketConnectedSetup(CSegment* pMsg);
	// Socket: connection failed
	virtual void OnSocketFailedSetup(CSegment* pMsg);
	// Socket: connection dropped
	virtual void OnSocketDroppedSetup(CSegment* pMsg);
	virtual void OnSocketDroppedConnect(CSegment* pMsg);
	// Socket : message received
	virtual void OnSocketRcvIdle(CSegment* pMsg);
	virtual void OnSocketRcvSetup(CSegment* pMsg);
	virtual void OnSocketRcvConnect(CSegment* pMsg);
	        void OnSocketPauseConnect(CSegment* pMsg);
    virtual void OnSocketResumePause(CSegment* pMsg);
	
	
	// Logical module: forward message to MPL-API
	void OnLogicModuleForwardMsgAnycase(CSegment* pMsg);

	// Socket : simulate card reset with/out problems
	virtual void OnMngrResetCardIdle(CSegment* pMsg);
	virtual void OnMngrResetCardSetup(CSegment* pMsg);
	virtual void OnMngrResetCardConnect(CSegment* pMsg);


protected:
			// Utilities
	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const;
	void OnSocketDropped(CSegment* pMsg);

			// Attributes
	CClientSocket*  m_pSocketConnection;

	PDECLAR_MESSAGE_MAP
};



/////////////////////////////////////////////////////////////////////////////
//
//   SimSwitchCard - SWITCH card simulation class implements SWITCH card
//
/////////////////////////////////////////////////////////////////////////////
//  task creation function
extern "C" void gideonSimSwitchCardEntryPoint(void* appParam);

class CSimSwitchCard  : public CSimSocketedCard
{
CLASS_TYPE_1(CSimSwitchCard,CSimSocketedCard)
public:
			// Constructors
	CSimSwitchCard();
	virtual ~CSimSwitchCard();
	virtual const char* NameOf() const { return "CSimSwitchCard";}

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
	virtual void DispatchMcmsMsg(CSegment* pMsg) const;
	
	void OnUserLdapLogin(CSegment* pMsg);

protected:
			// Action functions
	// Socket: connection established
	virtual void OnSocketConnectedSetup(CSegment* pMsg);
	// Switch logical: start up completed
	void OnSwitchEndConnectConnect(CSegment* pMsg);
	void OnSetUnitStatustSimSwitchForKeepAliveIndConnect(CSegment* pMsg);
	void OnSwitchSendRemoveCardIndConnect(CSegment* pMsg);
	void OnSwitchSendInsertCardIndConnect(CSegment* pMsg);
	void OnSetBurnRate(CSegment* pMsg);
	void OnSetBurnAction(CSegment* pMsg);

protected:

	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const;

			// Attributes
	CGideonSimSwitchLogical*	m_pSwitchLogics;

	PDECLAR_MESSAGE_MAP
};



const DWORD MAX_LOGICAL_PARTIES_PER_RTM = 30;

class CLogicalParty : public CPObject
{
CLASS_TYPE_1(CLogicalParty,CPObject)
public:
	CLogicalParty() : m_confId(0), m_partyId(0) {}
	CLogicalParty(DWORD confId,DWORD partyId) : m_confId(confId), m_partyId(partyId) {}
	void CleanUp();
	virtual const char* NameOf() const { return "CLogicalParty";}
	void AddNew(DWORD confId,DWORD partyId){m_partyId=partyId; m_confId=confId;}
	DWORD  m_confId;
	DWORD  m_partyId;
};
/////////////////////////////////////////////////////////////////////////////
//
//   SimMfaCard - MFA card simulation class implements MFA card
//
/////////////////////////////////////////////////////////////////////////////
//  task creation function
extern "C" void gideonSimMfaCardEntryPoint(void* appParam);

class CSimMfaCard  : public CSimSocketedCard
{
CLASS_TYPE_1(CSimMfaCard,CSimSocketedCard )
public:
			// Constructors
	CSimMfaCard();
	virtual ~CSimMfaCard();
	virtual const char* NameOf() const { return "CSimMfaCard";}

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
	virtual void DispatchMcmsMsg(CSegment* pMsg) const;

protected:
			// Action functions
	// Socket: connection established
	virtual void OnSocketConnectedSetup(CSegment* pMsg);
	// Socket: connection failed
	virtual void OnSocketFailedSetup(CSegment* pMsg);
	// MFA logical: start up completed
	void OnMfaEndConnectConnect(CSegment* pMsg);
	// MFA logical: insert RTM-PSTN sub board
 	void OnMfaInsertSubCardConnect(CSegment* pMsg);
 	
    void OnSetUnitStatustSimMfaForKeepAliveIndConnect(CSegment* pMsg);
  // STATUS  OnSetUnitStatustSimMfaForKeepAliveIndConnect(CRequest *pRequest);
	// Messages from EndpointsSim app
	void OnEndpointsSimIsdnMsgConnect(CSegment* pMsg);
	void OnEndpointsSimAudioMsgConnect(CSegment* pMsg);
	void OnEndpointsSimMuxMsgConnect(CSegment* pMsg);

    void OnSocketResumePause(CSegment* pMsg);
    void OnSetMediaCardParams(CSegment* pMsg);


	// VASILY:TEMP - for Matvey / Talya
//	virtual void OnMngrConnectSocketIdle(CSegment* pMsg);
//	virtual void OnConfPartyMsgAnycase(CSegment* pMsg);
	void PartyReaction(CMplMcmsProtocol* pMplProt);
	void Ack(COMMON_HEADER_S,MESSAGE_DESCRIPTION_HEADER_S,PORT_DESCRIPTION_HEADER_S);
	void Bad(COMMON_HEADER_S,MESSAGE_DESCRIPTION_HEADER_S,PORT_DESCRIPTION_HEADER_S,DWORD);
	// VASILY:TEMP - end

protected:
			// Utilities
	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const;

			// Attributes
	BOOL  m_isRtmAttached;
	eCardType m_RtmCardType;
	CGideonSimMfaLogical*		m_pMfaLogics;
	CGideonSimRtmLogical*		m_pRtmLogics;

	// VASILY:TEMP - for Matvey / Talya
	CLogicalParty m_LogicalPartiesArr[MAX_LOGICAL_PARTIES_PER_RTM];
	// VASILY:TEMP - end

	PDECLAR_MESSAGE_MAP
};


/////////////////////////////////////////////////////////////////////////////
//
//   SimBarakCard - BARAK card simulation class implements BARAK card
//
/////////////////////////////////////////////////////////////////////////////
//  task creation function
extern "C" void gideonSimBarakCardEntryPoint(void* appParam);

class CSimBarakCard  : public CSimSocketedCard
{
CLASS_TYPE_1(CSimBarakCard,CSimSocketedCard )
public:
			// Constructors
	CSimBarakCard();
	virtual ~CSimBarakCard();
	virtual const char* NameOf() const { return "CSimBarakCard";}

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
	
	virtual void DispatchMcmsMsg(CSegment* pMsg) const;

protected:
			// Action functions
	// Socket: connection established
	virtual void OnSocketConnectedSetup(CSegment* pMsg);
	// Socket: connection failed
	virtual void OnSocketFailedSetup(CSegment* pMsg);
	// Barak logical: start up completed
	void OnBarakEndConnectConnect(CSegment* pMsg);
	// Barak logical: insert RTM-PSTN sub board
 	void OnBarakInsertSubCardConnect(CSegment* pMsg);
 	
    void OnSetUnitStatustSimBarakForKeepAliveIndConnect(CSegment* pMsg);
  // STATUS  OnSetUnitStatustSimBarakForKeepAliveIndConnect(CRequest *pRequest);
	// Messages from EndpointsSim app
	void OnEndpointsSimIsdnMsgConnect(CSegment* pMsg);
	void OnEndpointsSimAudioMsgConnect(CSegment* pMsg);
	void OnEndpointsSimMuxMsgConnect(CSegment* pMsg);
	void OnEndpointsSimScpMsgConnect(CSegment* pMsg);
	void OnMMIndicationsMsgConnect(CSegment* pMsg);
#ifndef __DISABLE_ICE__
	void OnMMIceMsgConnect(CSegment* pMsg);
#endif	//__DISABLE_ICE__

    void OnSocketResumePause(CSegment* pMsg);
    void OnSetMediaCardParams(CSegment* pMsg);
	void OnPCMMsgToMplApi(CSegment* pMsg);
    void OnSetBurnRate(CSegment* pMsg);
    void OnSetBurnAction(CSegment* pMsg);

	// VASILY:TEMP - for Matvey / Talya
//	virtual void OnMngrConnectSocketIdle(CSegment* pMsg);
//	virtual void OnConfPartyMsgAnycase(CSegment* pMsg);
	void PartyReaction(CMplMcmsProtocol* pMplProt);
	void Ack(COMMON_HEADER_S,MESSAGE_DESCRIPTION_HEADER_S,PORT_DESCRIPTION_HEADER_S);
	void Bad(COMMON_HEADER_S,MESSAGE_DESCRIPTION_HEADER_S,PORT_DESCRIPTION_HEADER_S,DWORD);
	// VASILY:TEMP - end

protected:
			// Utilities
	BOOL  IsSingleton() const {return NO;}
	const char* GetTaskName() const;

			// Attributes
	BOOL  m_isRtmAttached;
	eCardType m_RtmCardType;
	CGideonSimBarakLogical*		m_pBarakLogics;
	CGideonSimRtmLogical*		m_pRtmLogics;

	// VASILY:TEMP - for Matvey / Talya
	CLogicalParty m_LogicalPartiesArr[MAX_LOGICAL_PARTIES_PER_RTM];
	// VASILY:TEMP - end

	PDECLAR_MESSAGE_MAP
};

/////////////////////////////////////////////////////////////////////////////
//
//   SimBreezeCard - Breeze card simulation class implements BARAK card
//
/////////////////////////////////////////////////////////////////////////////
//  task creation function
extern "C" void gideonSimBreezeCardEntryPoint(void* appParam);

class CSimBreezeCard  : public CSimBarakCard
{
CLASS_TYPE_1(CSimBreezeCard,CSimBarakCard )
public:
			// Constructors
	CSimBreezeCard() {};
	virtual ~CSimBreezeCard() {};

	const char*  NameOf() const {return "CSimBreezeCard";}
};
/////////////////////////////////////////////////////////////////////////////
//
//   SimGideonLiteCard - Gideon Lite configuration, has SWITCH module and 2 MFA modules
//
/////////////////////////////////////////////////////////////////////////////
//  task creation function
extern "C" void gideonSimGideonLiteCardEntryPoint(void* appParam);

class CSimGideonLiteCard  : public CSimSwitchCard
{
CLASS_TYPE_1(CSimGideonLiteCard,CSimSwitchCard)
public:
			// Constructors
	CSimGideonLiteCard();
	virtual ~CSimGideonLiteCard();
	virtual const char* NameOf() const { return "CSimGideonLiteCard";}

			// Initializations
	void  Create(CSegment& appParam);	// called by entry point
	void* GetMessageMap();

			// Operations
	virtual void DispatchMcmsMsg(CSegment* pMsg) const;

protected:
			// Action functions
	// Socket: connection established
	virtual void OnSocketConnectedSetup(CSegment* pMsg);
	void OnSwitchEndConnectConnect(CSegment* pMsg);
	void OnMfaEndConnectConnect(CSegment* pMsg);
	void OnEndpointsSimIsdnMsgConnect(CSegment* pMsg);
	void OnEndpointsSimAudioMsgConnect(CSegment* pMsg);
	void OnEndpointsSimMuxMsgConnect(CSegment* pMsg);

protected:

	BOOL  IsSingleton() const { return NO; }
	const char* GetTaskName() const;

			// Attributes
	CGideonSimSwitchLogical*	m_pSwitchLogics;
	CGideonSimMfaLogical*		m_pMfaLogicsArr[MAX_MFA_LOGICS_IN_LITE_CARD];
	CGideonSimRtmLogical*		m_pRtmLogicsArr[MAX_MFA_LOGICS_IN_LITE_CARD];

	PDECLAR_MESSAGE_MAP
};



#endif /* __GIDEONSIMCARDTASK_ */

