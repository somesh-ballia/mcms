//+========================================================================+
//                  EpSimProxyConfList.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//+========================================================================+

#ifndef __EPSIMPROXYCONFLIST_H__
#define __EPSIMPROXYCONFLIST_H__


#include "SipCsReq.h"
#include "StateMachine.h"


#define MAX_CONFS_IN_PROXY_LIST 100
#define MAX_CONF_URI_SIZE		100


class CMplMcmsProtocol;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


class CProxyConf : public CStateMachine
{
CLASS_TYPE_1( CProxyConf, CStateMachine )
public:
	// constructors, distructors and operators
	CProxyConf(  );
	~CProxyConf();
	virtual const char* NameOf() const { return "CProxyConf";}

	// basic functions
	void	HandleNewEvent( CMplMcmsProtocol* pMplProtocol );
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	// action functions
	void OnStartElement( CSegment* pParam );
	void OnTimerEndRegistration( CSegment* pParam );
	void OnTimerRemoveRegistration( CSegment* pParam );



public:
	DWORD GetConfID();
	void SetConfID( DWORD confId);
	void InitProxyConfTimer(const COsQueue& rcvMbx);
//	void SetCsApi( CTaskApi* pCSApi );

protected:
	void OnCSRegistrationReq( CMplMcmsProtocol* pMplProtocol );
	void OnCSSubscribeReq( CMplMcmsProtocol* pMplProtocol );
	void OnCSServiceReq( CMplMcmsProtocol* pMplProtocol );
	void OnStamTest( CSegment* pParam );
	void SendCSRegistrationInd( CMplMcmsProtocol* pMplProtocol );
	void SendCSSubscribeResInd( CMplMcmsProtocol* pMplProtocol );
	void SendCSNotifyInd( CMplMcmsProtocol* pMplProtocol );
	void SendCSServiceRespInd( CMplMcmsProtocol* pMplProtocol );
	void SendCommandToCS(DWORD opcode, BYTE* buffer, WORD bufferLen );

protected:
//	CTaskApi	*m_pCSApi;
	WORD		m_timerOn;
	WORD		m_registrationTime;
	DWORD		m_condId;
	WORD		m_registered;
	CTaskApi*	m_proxyApi;
	char		m_confUri[MAX_CONF_URI_SIZE];
	mcReqRegister*	m_req;
	bool 				m_piggybackSupported;



   PDECLAR_MESSAGE_MAP                                     

};
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



class CProxyConfList : public CPObject 
{
CLASS_TYPE_1( CProxyConfList, CPObject )
public:
	// constructors, distructors and operators
	CProxyConfList();
	~CProxyConfList();
	virtual const char* NameOf() const { return "CProxyConfList";}


	// basic functions

public:
	// more global functions
	void HandleNewEvent( CMplMcmsProtocol* pMplProtocol );
	CProxyConf* GetCurrentProxyConf( DWORD proxyConfID  );
	void SetTaskRcvMbx(const COsQueue& rcvMbx);
	void StamTest();
	void SelfRemoveConf( CSegment* pParam );
//	void SetCsApi( CTaskApi* m_pCSApi );

protected:
	COsQueue*	m_rcvMbx;
	CProxyConf*	m_proxyConfArray[MAX_CONFS_IN_PROXY_LIST];
//	CTaskApi* 	m_pCSApi;

};




#endif // __EPSIMPROXYCONFLIST_H__ 


