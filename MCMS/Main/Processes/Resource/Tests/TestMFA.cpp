// CTestMFA.cpp: implementation of the CTestMFA class.
//
//////////////////////////////////////////////////////////////////////


#include "TestMFA.h"
#include "ResourceProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestMFA );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestMFA::setUp()
{
//	pResourceProcess = new CResourceProcess;
//	pResourceProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestMFA::tearDown()
{
//	SystemSleep(10);
//	pResourceProcess->TearDown();
//	delete pResourceProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestMFA::testConstructor()
{
	CUnitMFA * pMFA = new CUnitMFA( 1/*boardId*/,0 /*unitId*/,eUnitType_Art /*unitType*/);
	
	CPPUNIT_ASSERT_MESSAGE("CTestMFA::testConstructor ", pMFA != NULL);
	CPPUNIT_ASSERT_MESSAGE("Is allocated", pMFA->GetIsAllocated() == 0);
	CPPUNIT_ASSERT_MESSAGE("Get free capacity", pMFA->GetFreeCapacity() == 1000);
	CPPUNIT_ASSERT_MESSAGE("Get connection ID", pMFA->GetConnId() == 0);
	CPPUNIT_ASSERT_MESSAGE("Get Unit Type", pMFA->GetUnitType() == eUnitType_Art);
	delete pMFA;
} 


//////////////////////////////////////////////////////////////////////
//void CTestMFA::testMFAList()
//{
//	CSystemResources* pSystResources = new CSystemResources;
//	CPPUNIT_ASSERT_MESSAGE("CSystemResources init", pSystResources!= NULL);
//	CPPUNIT_ASSERT_MESSAGE("UnitsMFAlist init", pSystResources->GetUnitsMFAList()!= NULL);
//	
//	CUnitMFA * pMFA = new CUnitMFA( 1/*boardId*/,0 /*unitId*/,eUnitType_Art /*unitType*/);
//	(pSystResources->GetUnitsMFAList())->insert(*pMFA);
//	size_t numMFA =(pSystResources->GetUnitsMFAList())->size();
//	CPPUNIT_ASSERT_MESSAGE("UnitsMFAlist insert", numMFA == 1);
//	(pSystResources->GetUnitsMFAList())->erase(*pMFA);
//	numMFA =(pSystResources->GetUnitsMFAList())->size();
//	CPPUNIT_ASSERT_MESSAGE("UnitsMFAlist erase", numMFA == 0);
//	
//	delete pMFA;
//	delete pSystResources;
//} 

//////////////////////////////////////////////////////////////////////
//void CTestMFA::testMFAAllocAndDealloc()
//{
//		
//    WORD rsrcConfId =1;
//    WORD rsrcPartyId=2;
//	WORD serviceId = 0xFF;
//	eResourceTypes physType = ePhysical_art;
//	DWORD monitor_conf_id = 11;
//	eSessionType sessionType = eVOICE_session;
//
//	CSystemResources* pSystResources = new CSystemResources;
//	CPPUNIT_ASSERT_MESSAGE("CSystemResources init", pSystResources!= NULL);
//	CPPUNIT_ASSERT_MESSAGE("UnitsMFAlist init", pSystResources->GetUnitsMFAList()!= NULL);
//
//	CUnitMFA * pMFA = new CUnitMFA( 1/*boardId*/,1 /*unitId*/,eUnitType_Art /*unitType*/);
//	(pSystResources->GetUnitsMFAList())->insert(*pMFA);
//	
//	PhysicalPortDesc* pPortDesc = new PhysicalPortDesc;
//
//	STATUS stat = pSystResources->AllocateART(rsrcConfId,rsrcPartyId,
//					physType, serviceId, pPortDesc, eVOIP_party_type, 0);
//
//	
//	CPPUNIT_ASSERT_MESSAGE("testMFAAlocation", stat == STATUS_OK);
//
//
//    CConfRsrc* pConf =new CConfRsrc(monitor_conf_id,sessionType);
//	pConf->SetRsrcConfId( rsrcConfId );
//	stat = pSystResources->DeAllocateART( pConf, rsrcPartyId, physType, pPortDesc );
//	CPPUNIT_ASSERT_MESSAGE("testMFADeAllocation", stat == STATUS_OK);
//
//   (pSystResources->GetUnitsMFAList())->erase(*pMFA);
//	size_t numMFA =(pSystResources->GetUnitsMFAList())->size();
//	CPPUNIT_ASSERT_MESSAGE("UnitsMFAlist erase", numMFA == 0);
//
//    delete pPortDesc;
//	delete pConf;
//	delete pMFA;
//	delete pSystResources;
//	
//}


