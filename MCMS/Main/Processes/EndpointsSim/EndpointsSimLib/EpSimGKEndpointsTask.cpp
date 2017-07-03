//+========================================================================+
//                 EpSimGKEndpointsTask.cpp                                |
//            Copyright 2006 Polycom Networks Ltd.                         |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// ------------------------------------------------------------------------|
// FILE:       EpSimGKEndpointsTask.cpp                                    |
//+========================================================================+


#include "Trace.h"
#include "Macros.h"
#include "CsStructs.h"

#include "MplMcmsProtocol.h"
#include "SystemFunctions.h"
#include "SocketApi.h"
#include "ClientSocket.h"

#include "EndpointsSim.h"
#include "EpSimGKRegistrationsList.h"
#include "EpSimGKEndpointsTask.h"
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
extern "C" void epSimGKeeperEntryPoint(void* appParam);



/////////////////////////////////////////////////////////////////////////////
//
//   SimCSEndpointsModule - Central Signaling Sim module Task
//
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CSimGKEndpointsModule)

	ONEVENT( SIM_CS_RCV_MSG, 	IDLE,  CSimGKEndpointsModule::OnCSAPICommandToGK)
	//ONEVENT( UPDATE_CS_MBX,  	IDLE,  CSimGKEndpointsModule::OnReceiveMbxCS)
	ONEVENT( GK_REMOVE_SERVICE, IDLE,  CSimGKEndpointsModule::OnGKSelfRemoveRegistration)
	ONEVENT( GUI_TO_GATEKEEPER, IDLE,  CSimGKEndpointsModule::OnGuiGKCommand)

PEND_MESSAGE_MAP(CSimGKEndpointsModule,CTaskApp);


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//  task creation function
void epSimGKeeperEntryPoint(void* appParam)
{
	CSimGKEndpointsModule* pEpSimGKTask = new CSimGKEndpointsModule;
	pEpSimGKTask->Create(*(CSegment*)appParam);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CSimGKEndpointsModule::CSimGKEndpointsModule()
{
//	m_pCSApi = NULL;
	m_GKList = new CGKeeperList();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CSimGKEndpointsModule::~CSimGKEndpointsModule()     // destructor
{
	PTRACE( eLevelInfoNormal, "CSimGKEndpointsModule::~CSimGKEndpointsModule" );

//	POBJDELETE( m_pCSApi );
	POBJDELETE( m_GKList );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void* CSimGKEndpointsModule::GetMessageMap()
{
	return (void*)m_msgEntries;
}

eEPSimTaskType CSimGKEndpointsModule::GetTaskType(void) const
{
    return eGKTypeTaskType;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimGKEndpointsModule::Create(CSegment& appParam)
{
	CCSIDTaskApp::Create(appParam);

	m_GKList->SetTaskRcvMbx(GetRcvMbx());

//	m_pCSApi = new CCSSimTaskApi(GetCsId());
//m_pCSApi->CreateOnlyApi();

	//m_GKList->SetCsApi(m_pCSApi);
}


/////////////////////////////////////////////////////////////////////////////
// App Manager: establish connection with CS-API


void CSimGKEndpointsModule::OnCSAPICommandToGK(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimGKEndpointsModule::OnCSAPICommandToGK - M_GK_EpSim==>Command");

	//  gets the protocol
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pParam,CS_API_TYPE);

	// Handle the event in the list
	m_GKList->HandleNewEvent( pMplProtocol );

	POBJDELETE(pMplProtocol);

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//void CSimGKEndpointsModule::OnReceiveMbxCS(CSegment* pParam)
//{
//	PTRACE(eLevelInfoNormal,"CSimGKEndpointsModule::OnReceiveMbxCS - M_GK_EpSim==>Gets Mailbox ");
//
//	COsQueue RcvMbxCS;
//	RcvMbxCS.DeSerialize( *pParam );
//	if (m_pCSApi)
//		POBJDELETE( m_pCSApi );
//	m_pCSApi = new CTaskApi();
//	m_pCSApi->CreateOnlyApi(RcvMbxCS);
//	if (m_GKList)
//		m_GKList->SetCsApi( m_pCSApi );
//
//}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSimGKEndpointsModule::OnGKSelfRemoveRegistration(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimGKEndpointsModule::OnGKSelfRemoveRegistration - M_GK_EpSim==>End Registaration Time");
	m_GKList->SelfRemoveRegistration( pParam );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CSimGKEndpointsModule::OnGuiGKCommand(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSimGKEndpointsModule::OnGuiGKCommand - M_GK_EpSim==>Command");

	DWORD tmp;
	*pParam >> tmp;

	COsQueue RcvMbxGUI;
	RcvMbxGUI.DeSerialize( *pParam );

	// Handle the event in the list
	m_GKList->UnRegisterMCU( tmp );

//	CTaskApi* pGUIApi = new CTaskApi();
//	pGUIApi->CreateOnlyApi(RcvMbxGUI);
//	pGUIApi->Send();
//	pGUIApi->DestroyOnlyApi();
//	POBJDELETE(pGUIApi);
}

