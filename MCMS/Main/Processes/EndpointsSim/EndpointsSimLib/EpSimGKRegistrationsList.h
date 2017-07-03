//+========================================================================+
//                  EpSimGKRegistrationsList.h                             |
//+========================================================================+

#ifndef __EPSIM_GK_REGISTRATIONS_LIST_H__
#define __EPSIM_GK_REGISTRATIONS_LIST_H__

#include "StateMachine.h"
#include "ChannelParams.h"

#define MAX_REGISTRATIONS_IN_GK_LIST 	1
#define MAX_GK_PARTICIPANTS_LIST		80

class CMplMcmsProtocol;

/////////////////////////////////////////////////////////////////////////////
class CGKParty : public CStateMachine
{
CLASS_TYPE_1( CGKParty, CStateMachine )
public:
	// constructors, distructors and operators
	CGKParty();
	~CGKParty();
	virtual const char* NameOf() const { return "CGKParty";}

	// basic functions
	void	HandleNewEvent( CMplMcmsProtocol* pMplProtocol );
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	// action functions
	void OnStartElement( CSegment* pParam );
//	void OnTimerEndRegistration( CSegment* pParam );
//	void OnTimerRemoveRegistration( CSegment* pParam );
//  void SetCsApi( CTaskApi* pCSApi );

protected:
//	void OnCSRegistrationReq( CMplMcmsProtocol* pMplProtocol );
//	void SendCSRegistrationInd( CMplMcmsProtocol* pMplProtocol );
//	void SendCommandToCS(DWORD opcode, BYTE* buffer, WORD bufferLen );

protected:
//	CTaskApi	*m_pCSApi;
//	WORD		m_timerOn;
//	WORD		m_registrationTime;HandleNewEvent
//	DWORD		m_condId;
//	WORD		m_registered;
	CTaskApi*	m_GKApi;
//	char		m_confUri[MAX_CONF_URI_SIZE];
//	mcReqRegister*	m_req;

   PDECLAR_MESSAGE_MAP                                     
};

//////////////////////////////////////////////////////////////////////////
class CGKRegistration : public CStateMachine 
{
CLASS_TYPE_1( CGKRegistration, CStateMachine )
public:
	// constructors, distructors and operators
	CGKRegistration();
	~CGKRegistration();
	virtual const char* NameOf() const { return "CGKRegistration";}

	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	// basic functions

public:
	// more global functions
	void HandleNewEvent( CMplMcmsProtocol* pMplProtocol );
	void SetTaskRcvMbx(const COsQueue& rcvMbx);
//	void SelfRemoveRegistration( CSegment* pParam );
//	void SetCsApi( CTaskApi* m_pCSApi );
	
protected:
	// action functions
	void OnStamTest( CSegment *pParam );
	void OnRegistrationExpired( CSegment *pParam );
	void OnBrqTimerTout( CSegment *pParam );
	void OnIRQTimerTout( CSegment *pParam );
	
protected:
	void DoServiceIpReq( CMplMcmsProtocol* pMplProtocol );
	void DoServiceRegistrationReq( CMplMcmsProtocol* pMplProtocol );
	void DoPartyConnectReq( CMplMcmsProtocol* pMplProtocol );
	void DoPartyDisconnectReq( CMplMcmsProtocol* pMplProtocol );
	void DoResourceAvailabilityReq( CMplMcmsProtocol* pMplProtocol );
	void gkIfSetUnionXml( xmlUnionPropertiesSt *pUnionProp, int type );
	
	CGKParty*	m_GKParticipants[MAX_GK_PARTICIPANTS_LIST];
	COsQueue*	m_rcvMbx;
//	CTaskApi* 	m_pCSApi;
	int			m_expireTimer;
	int			m_port;
	int			m_crvNumber;
	CMplMcmsProtocol* m_pBrqProtocol;
	CMplMcmsProtocol* m_pIRRProtocol;

   PDECLAR_MESSAGE_MAP                                     
};

//////////////////////////////////////////////////////////////////////////
class CGKeeperList : public CPObject 
{
CLASS_TYPE_1( CGKeeperList, CPObject )
public:
	// constructors, distructors and operators
	CGKeeperList();
	~CGKeeperList();
	virtual const char* NameOf() const { return "CGKeeperList";}

	// more global functions
	void HandleNewEvent( CMplMcmsProtocol* pMplProtocol );
	void SetTaskRcvMbx(const COsQueue& rcvMbx);
	void SelfRemoveRegistration( CSegment* pParam );
	//void SetCsApi( CTaskApi* m_pCSApi );
	void UnRegisterMCU( DWORD tmp );

protected:
	CGKRegistration* m_GKRegistrations[MAX_REGISTRATIONS_IN_GK_LIST];
	COsQueue* m_rcvMbx;
	//CTaskApi*    m_pCSApi;
};

#endif // __EPSIM_GK_REGISTRATIONS_LIST_H__
