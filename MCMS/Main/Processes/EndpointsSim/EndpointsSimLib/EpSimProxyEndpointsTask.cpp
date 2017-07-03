//+========================================================================+
//                EpSimProxyEndpointsTask.cpp                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpSimProxyEndpointsTask.cpp                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+


#include "Trace.h"
#include "Macros.h"
#include "CsStructs.h"

#include "SystemFunctions.h"
#include "SocketApi.h"
#include "ClientSocket.h"
#include "MplMcmsProtocol.h"

#include "EndpointsSim.h"
#include "EpSimProxyConfList.h"
#include "EpSimProxyEndpointsTask.h"

#include "EndpointsSimProcess.h"
#include "CSSimTaskApi.h"
#include "OpcodesMcmsInternal.h"

/////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  SETUP         = 1;
const WORD  STARTUP       = 2;
const WORD  CONNECT       = 3;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class


/////////////////////////////////////////////////////////////////////////////
//
//   EXTERNALS:
//

//  task creation function
extern "C" void epSimProxyEntryPoint(void* appParam);



/////////////////////////////////////////////////////////////////////////////
//
//   SimCSEndpointsModule - Central Signaling Sim module Task
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CSimProxyEndpointsModule)

	ONEVENT( CSAPI_TO_PROXY, IDLE,  CSimProxyEndpointsModule::OnCSAPICommandToProxy)
//	ONEVENT( UPDATE_CS_MBX,  IDLE,  CSimProxyEndpointsModule::OnReceiveMbxCS)
	ONEVENT( PROXY_REMOVE_CONF,  IDLE,  CSimProxyEndpointsModule::OnProxySelfRemoveConf)

PEND_MESSAGE_MAP(CSimProxyEndpointsModule,CTaskApp);




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  task creation function
void epSimProxyEntryPoint(void* appParam)
{
	CSimProxyEndpointsModule* pEpSimProxyTask = new CSimProxyEndpointsModule;
	pEpSimProxyTask->Create(*(CSegment*)appParam);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CSimProxyEndpointsModule::CSimProxyEndpointsModule()
{
//	m_pCSApi = NULL;
	m_proxyList = new CProxyConfList();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CSimProxyEndpointsModule::~CSimProxyEndpointsModule()     // destructor
{
	PTRACE(eLevelInfoNormal, "CSimProxyEndpointsModule::~CSimProxyEndpointsModule" );

//	POBJDELETE( m_pCSApi );
	POBJDELETE( m_proxyList );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void* CSimProxyEndpointsModule::GetMessageMap()
{
	return (void*)m_msgEntries;
}

eEPSimTaskType CSimProxyEndpointsModule::GetTaskType(void) const
{
    return eProxyTaskType;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimProxyEndpointsModule::Create(CSegment& appParam)
{
    CCSIDTaskApp::Create(appParam);

	if (m_proxyList)
	{
		m_proxyList->SetTaskRcvMbx(GetRcvMbx());
		m_proxyList->StamTest();

//		delete m_pCSApi;
//		m_pCSApi = new CCSSimTaskApi(GetCsId());
//        m_pCSApi->CreateOnlyApi();
//
//		m_proxyList->SetCsApi(m_pCSApi);
	}
}


/////////////////////////////////////////////////////////////////////////////
// App Manager: establish connection with CS-API


void CSimProxyEndpointsModule::OnCSAPICommandToProxy(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimProxyEndpointsModule::OnCSAPICommandToProxy");

	//  gets the protocol
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pParam,CS_API_TYPE);

	// Handle the event in the list
	m_proxyList->HandleNewEvent( pMplProtocol );

	POBJDELETE(pMplProtocol);

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//void CSimProxyEndpointsModule::OnReceiveMbxCS(CSegment* pParam)
//{
//	PTRACE(eLevelInfoNormal,"CSimProxyEndpointsModule::OnReceiveMbxCS");
//
//	COsQueue RcvMbxCS;
//	RcvMbxCS.DeSerialize( *pParam );
//	m_pCSApi->CreateOnlyApi(RcvMbxCS);
//}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimProxyEndpointsModule::OnProxySelfRemoveConf(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimProxyEndpointsModule::OnProxySelfRemoveConf");
	m_proxyList->SelfRemoveConf( pParam );

}

