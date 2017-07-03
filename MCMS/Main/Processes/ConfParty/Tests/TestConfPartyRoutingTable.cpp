// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

/*
#include "TestConfPartyRoutingTable.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "RsrcParams.h"
#include "StatusesGeneral.h"
#include "TaskApi.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestConfPartyRoutingTable );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::setUp()
{
	m_pConfPartyRoutingTable = new CConfPartyRoutingTable;
	m_pTaskApi1 = new CTaskApi;
	m_pTaskApi2 = new CTaskApi;
	m_pTaskApi3 = new CTaskApi;
	
	m_queue1 = new COsQueue(1); 
	m_queue2 = new COsQueue(2); 
	m_queue3 = new COsQueue(3); 
	
	m_pTaskApi1->CreateOnlyApi(*m_queue1);
	m_pTaskApi2->CreateOnlyApi(*m_queue2);
	m_pTaskApi3->CreateOnlyApi(*m_queue3);
}
//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::tearDown()
{
	POBJDELETE(m_pConfPartyRoutingTable);
	
	if (m_pTaskApi1){
		m_pTaskApi1->DestroyOnlyApi();
		POBJDELETE(m_pTaskApi1);
	}
	if(m_pTaskApi2){
		m_pTaskApi2->DestroyOnlyApi();
		POBJDELETE(m_pTaskApi2);
	}
	
	if(m_pTaskApi3){
		m_pTaskApi3->DestroyOnlyApi();
		POBJDELETE(m_pTaskApi3);
	}
	
	
	
	
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testConstructor()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		m_pConfPartyRoutingTable != NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testInsert()
{

	CPartyRsrcRoutingTblKey Desc(333, 2, eLogical_audio_decoder);
	int status;
	status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testInsert ",
		status == STATUS_OK );  
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(2, 333) == NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFind()
{
	CRsrcParams Desc(333,2,1,eLogical_audio_decoder);
	int status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testInsert ",
		status == STATUS_OK );  

	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc, m_pTaskApi1);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testInsert ",
		status == STATUS_OK );  
	CTaskApi * pTaskApi1 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(2, 333));	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFind ",(pTaskApi1->GetRcvMbx()).m_max_connections ==  m_queue1->m_max_connections);
} 
//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByConnectionId()
{
	CRsrcParams Desc(333,2,1,eLogical_audio_decoder);
	
	int status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConnectionId ",
		status == STATUS_OK );  
	
	CRsrcParams Desc1(333,2,1, eLogical_audio_decoder);
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc1, m_pTaskApi1);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConnectionId ",
		status == STATUS_OK );  
	
	CTaskApi * pTaskApi1 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(2, 333));	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConnectionId ",(pTaskApi1->GetRcvMbx()).m_max_connections ==  m_queue1->m_max_connections);
} 

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByPartyIDandLRT()
{
	CRsrcParams Desc(333,2,1,eLogical_audio_decoder);
	
	int status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIDandLRT ",
		status == STATUS_OK );  
	
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc, m_pTaskApi1);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIDandLRT ",
		status == STATUS_OK );  
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIDandLRT ",
		(m_pConfPartyRoutingTable->GetPartyRsrcDesc(2,eLogical_audio_decoder))->GetConnectionId() == Desc.GetConnectionId() );  
}
*/
/*
//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByPartyID()
{
	CRsrcParams Desc(333,2,1,eLogical_rtp);
	
	CRsrcParams Desc1;
	Desc1.SetConfRsrcId(1);
	Desc1.SetPartyRsrcId(2);
	
	int status = m_pConfPartyRoutingTable->AddRsrcDescToPartyRoutingTbl(Desc);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConnectionId ",
		status == STATUS_OK );  
	
	char* ptr = new char;
	
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc, ptr);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConnectionId ",
		status == STATUS_OK );  
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		m_pConfPartyRoutingTable->GetRsrcMngrPtr(Desc1) == ptr );  

} 
*/
/*
//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByConfID()
{	
	int status = m_pConfPartyRoutingTable->AddConfToRoutingTbl(1,m_pTaskApi1);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConfID ",
		status == STATUS_OK );  	
	CTaskApi * pTaskApi1 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(1));
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConfID ",
		(pTaskApi1->GetRcvMbx()).m_max_connections ==  m_queue1->m_max_connections );   
}
 
//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByConfIDFails()
{	
	int status = m_pConfPartyRoutingTable->AddConfToRoutingTbl(1,m_pTaskApi1);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConfIDFails ",
		status == STATUS_OK );  	
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByConfIDFails ",
		m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(2) == NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testAddRsrcPointerFails()
{
	CRsrcParams Desc(333,2,1,eLogical_audio_decoder);
	int status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc, m_pTaskApi1);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testAddRsrcPointerFails ",
		status == STATUS_FAIL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testRemoveStateMachinePointer()
{
	CRsrcParams Desc(333,2,1,eLogical_audio_decoder);	
	int status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc);
	
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc, m_pTaskApi1);
	
	status = m_pConfPartyRoutingTable->RemoveStateMachinePointerFromRoutingTbl(Desc);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testRemoveStateMachinePointer ",
		status == STATUS_OK );
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testRemoveStateMachinePointer ",
		m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(2, 333) == NULL );
} 

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testRemoveAllPartyRsrcs()
{
	CRsrcParams Desc(333,2,1,eLogical_audio_decoder);
	int status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc);
	
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc, m_pTaskApi1);
	
	m_pConfPartyRoutingTable->RemoveAllPartyRsrcs(2);

	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(2, 333) == NULL );
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testGetRsrcDesc()
{
	CRsrcParams Desc(333,2,1,eLogical_audio_decoder);
	CRsrcParams Desc2(666,2,1,eLogical_net);
	
	int status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc);
	
	
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc, m_pTaskApi1);
	
	status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc2);
	
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc2, m_pTaskApi2);
	
	CRsrcDesc *pRsrcDesc = m_pConfPartyRoutingTable->GetPartyRsrcDesc(2, eLogical_net);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		pRsrcDesc != NULL );
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testGetAllPartyRsrcs()
{
	CRsrcParams Desc(333, 2,1,eLogical_audio_decoder);
	
	CRsrcParams Desc2(666,2,1,eLogical_net);
	
	CRsrcParams Desc3(777, 4,1, eLogical_rtp);
	
	int status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc);
	
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc, m_pTaskApi1);
	
	status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc2);
	
	status = m_pConfPartyRoutingTable->AddPartyRsrcDesc(Desc3);
	
	status = m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(Desc3, m_pTaskApi2);
	
	VECTOR_OF_RSRC_DESC_POINTERS* pVec = m_pConfPartyRoutingTable->GetAllPartyRsrcs(2);
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		pVec != NULL );
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		pVec->size() == 2 );
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		(*pVec->front() == *Desc.GetRsrcDesc()) );
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		(*pVec->back() == *Desc2.GetRsrcDesc()) );
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByWhenThere3Entries()
{
	m_pConfPartyRoutingTable->AddConfToRoutingTbl(2, m_pTaskApi3);
	
	CRsrcParams rsrc0(333, 1,2, eLogical_net);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc0);
	
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc0,m_pTaskApi1);
	
	CRsrcParams rsrc1(444, 1,2, eLogical_audio_decoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc1);

	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc1,m_pTaskApi2);
	
	CRsrcParams rsrc2(555, 1,2,eLogical_audio_encoder );
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc2);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc2,m_pTaskApi2);

	CTaskApi * pTaskApi1 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(1,333)) ;
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		(pTaskApi1->GetRcvMbx()).m_max_connections
		 == m_queue1->m_max_connections );  	
	
	CRsrcParams Desc2(DUMMY_CONNECTION_ID,DUMMY_PARTY_ID,2);
	CTaskApi * pTaskApi3 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(2));
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		(pTaskApi3->GetRcvMbx()).m_max_connections
		 == m_queue3->m_max_connections);	
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByPartyIdWhenThere3EntriesBackward()
{
	CRsrcParams rsrc0(555, 1, 2, eLogical_net);

	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc0);

	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc0,m_pTaskApi1);
	
	CRsrcParams rsrc1(444,1,2, eLogical_audio_decoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc1);
	
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc1,m_pTaskApi2);

	CRsrcParams rsrc2(333,1,2, eLogical_audio_encoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc2);

	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc2,m_pTaskApi2);

	
	m_pConfPartyRoutingTable->AddConfToRoutingTbl(2,m_pTaskApi3);
	CTaskApi * pTaskApi1 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(1,555));
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThere3EntriesBackward ",
		(pTaskApi1->GetRcvMbx()).m_max_connections
		 == m_queue1->m_max_connections ); 
	CTaskApi * pTaskApi3 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(2));
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThere3EntriesBackward ",
		(pTaskApi3->GetRcvMbx()).m_max_connections
		 == m_queue3->m_max_connections);
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByPartyIdWhenThere3Parties()
{
	CRsrcParams rsrc0(333, 1,2, eLogical_net);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc0);
	
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc0,m_pTaskApi1);
	
	CRsrcParams rsrc1(444, 2,2, eLogical_audio_decoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc1);
	
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc1,m_pTaskApi2);

	m_pConfPartyRoutingTable->AddConfToRoutingTbl(2, m_pTaskApi3);

	CRsrcParams rsrc2(555, 3, 2, eLogical_audio_encoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc2);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc2,m_pTaskApi2);
	
	CTaskApi * pTaskApi1 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(1, 333) );
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThere3Parties ",
		(pTaskApi1->GetRcvMbx()).m_max_connections
		 == m_queue1->m_max_connections ); 
	CTaskApi * pTaskApi3 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(2));
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThere3Parties ",
		(pTaskApi3->GetRcvMbx()).m_max_connections
		 == m_queue3->m_max_connections); 	

	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThere3Parties ",
		m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(1, 444) == NULL );  	

}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testFindByPartyIdWhenThereManyConf()
{
	CTaskApi api4,api5,api6,api7,api8,api9,api10,api11;
	COsQueue queue4(4),queue5(5),queue6(6),queue7(7),queue8(8),queue9(9),queue10(10),queue11(11); 
	
	api4.CreateOnlyApi(queue4);
	api5.CreateOnlyApi(queue5);		
	api6.CreateOnlyApi(queue6);
	api7.CreateOnlyApi(queue7);
	api8.CreateOnlyApi(queue8);
	api9.CreateOnlyApi(queue9);
	api10.CreateOnlyApi(queue10);
	api11.CreateOnlyApi(queue11);
	
	m_pConfPartyRoutingTable->AddConfToRoutingTbl(1, m_pTaskApi2);

	CRsrcParams rsrc0(1000,1,1,eLogical_net);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc0);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc0,m_pTaskApi1);

	CRsrcParams rsrc1(1001,1,1,eLogical_rtp);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc1);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc1,m_pTaskApi1);

	CRsrcParams rsrc2(1005,2,1,eLogical_video_encoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc2);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc2,m_pTaskApi2);

	CRsrcParams rsrc3(1006,1,1,eLogical_audio_encoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc3);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc3,m_pTaskApi2);
	
	CRsrcParams rsrc4(1008,5,1,eLogical_audio_encoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc4);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc4,m_pTaskApi2);

	CRsrcParams rsrc5(1007,5,1,eLogical_audio_decoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc5);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc5,m_pTaskApi2);

	CRsrcParams rsrc6(333 ,3,3,eLogical_rtp);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc6);

	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc6,&api6);

	CRsrcParams rsrc7(1004,3,3,eLogical_audio_decoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc7);

	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc7,&api7);

	m_pConfPartyRoutingTable->AddConfToRoutingTbl(3, &api7);

	CRsrcParams rsrc8(1003,6,3,eLogical_video_encoder);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc8);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc8,&api7);

	CRsrcParams rsrc9(1002,7,3,eLogical_net);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc9);
	
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc9,&api9);

	CRsrcParams rsrc10(1020,4,2,eLogical_net);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc10);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc10,&api10);

	CRsrcParams rsrc11(555 ,8,2,eLogical_net);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc11);
	m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc11,&api11);

	CTaskApi * pTaskApi1 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(1,1000));
	CTaskApi * pTaskApi2 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(1));
	CTaskApi * pTaskApi6 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(3,333));
	CTaskApi * pTaskApi7 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(3,1004));
	CTaskApi * pTaskApi9 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(7, 1002));
   CTaskApi * pTaskApi10 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(4,1020));
   CTaskApi * pTaskApi11 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(8,555));
	
	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
		 (pTaskApi1->GetRcvMbx()).m_max_connections
		 == m_queue1->m_max_connections );   
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
		(pTaskApi2->GetRcvMbx()).m_max_connections
		 == m_queue2->m_max_connections ); 	
		 
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
		 (pTaskApi6->GetRcvMbx()).m_max_connections
		 == queue6.m_max_connections );
		 	  	 	
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
		(pTaskApi7->GetRcvMbx()).m_max_connections
		 == queue7.m_max_connections ); 	
	pTaskApi7 = (CTaskApi *)(m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(6,1003));

	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
		(pTaskApi7->GetRcvMbx()).m_max_connections
		 == queue7.m_max_connections ); 	

	pTaskApi7 = (CTaskApi *)(	 m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(3) );
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
	(pTaskApi7->GetRcvMbx()).m_max_connections
		 == queue7.m_max_connections ); 
		 
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
	(pTaskApi9->GetRcvMbx()).m_max_connections
		 == queue9.m_max_connections );   	
		 
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
		(pTaskApi10->GetRcvMbx()).m_max_connections
		 == queue10.m_max_connections );  
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
		(pTaskApi11->GetRcvMbx()).m_max_connections
		 == queue11.m_max_connections );   
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testFindByPartyIdWhenThereManyConf ",
		m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(2) == NULL ); 
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testSkipConfNull()
{
	CRsrcParams rsrc0(333,3,2,eLogical_video_encoder);
	CRsrcParams rsrc1(666,4,2,eLogical_audio_decoder);

	m_pConfPartyRoutingTable->AddConfToRoutingTbl(2,m_pTaskApi1);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc0);
	m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc1);

	CRsrcParams Desc1(DUMMY_CONNECTION_ID,DUMMY_PARTY_ID,2);
	CTaskApi * pTaskApi1 = (CTaskApi *)m_pConfPartyRoutingTable->GetRsrcMngrPtrByConfId(2);
	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		(pTaskApi1->GetRcvMbx()).m_max_connections
			 == m_queue1->m_max_connections ); 	
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyRoutingTable::testIterationsForFind()
{
//	CRsrcParams rsrc, rsrc0, rsrc1, rsrc2, rsrc3, rsrc4;
//	DWORD connectionId = 1000; 
//	char* ptr0;
//	char* ptr1;
//	int num_of_parties = 10;
//	int num_of_confs = 10;
//	for(int i=1; i<=num_of_confs;i++)//conf id
//	{
//		ptr1 = new char[5];
//		//itoa(connectionId, ptr1, 10); SAGI: no itoa in linux
//        sprintf(ptr1,"%d",connectionId);    
//		
//		m_pConfPartyRoutingTable->AddConfToRoutingTbl(i,ptr1);
//
//		for(int j=1; j<=num_of_parties;j++)//party id
//		{
//			ptr0 = new char[5];
//			//itoa((connectionId), ptr0, 10); SAGI: no itoa in linux
//            sprintf(ptr0,"%d",connectionId);
//
//			rsrc0 = CRsrcParams(connectionId++,(i*num_of_parties)+j,i,eLogical_rtp);
//			m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc0);
//			m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc0,ptr0);
//			
//			rsrc1 = CRsrcParams(connectionId++,(i*num_of_parties)+j,i,eLogical_audio_decoder);
//			m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc1);
//			m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc1,ptr1);
//			
//			rsrc2 = CRsrcParams(connectionId++,(i*num_of_parties)+j,i,eLogical_audio_encoder);
//			m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc2);
//			m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc2,ptr1);
//			
//			rsrc3 = CRsrcParams(connectionId++,(i*num_of_parties)+j,i,eLogical_video_encoder);
//			m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc3);
//			m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc3,ptr1);
//			
//			rsrc4 = CRsrcParams(connectionId++,(i*num_of_parties)+j,i,eLogical_video_decoder);
//			m_pConfPartyRoutingTable->AddPartyRsrcDesc(rsrc4);
//			m_pConfPartyRoutingTable->AddStateMachinePointerToRoutingTbl(rsrc4,ptr1);
//		}
//	}
//
//	::SetCounterIterationLessThanForTest(0);
//
//	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testIterationsForFind ",
//		strcmp((const char*)m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(22, 1055),"1055")==0);  
//
//	int count = ::GetCounterIterationLessThanForTest();
//	
//	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testIterationsForFind ",
//		count==9);  
//
//	::SetCounterIterationLessThanForTest(0);
//
//	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testIterationsForFind ",
//		strcmp((const char*)m_pConfPartyRoutingTable->GetRsrcMngrPtrByPartyIdAndConnectionId(11, 1000),"1000")==0);  
//	
//	count = ::GetCounterIterationLessThanForTest();
//	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testIterationsForFind ",
//		count==9);  
//	::SetCounterIterationLessThanForTest(0);

}



*/
