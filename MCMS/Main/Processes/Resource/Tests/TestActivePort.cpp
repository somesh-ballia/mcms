// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////


#include "TestActivePort.h"
#include "ResourceProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestActivePort );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestActivePort::setUp()
{
//	pResourceProcess = new CResourceProcess;
//	pResourceProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestActivePort::tearDown()
{
//	SystemSleep(10);
//	pResourceProcess->TearDown();
//	delete pResourceProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestActivePort::testConstructor()
{
	CActivePort * ap = new CActivePort;
	CPPUNIT_ASSERT_MESSAGE("CTestActivePort::testConstructor ", ap != NULL);
	CPPUNIT_ASSERT_MESSAGE("party id init", ap->GetPartyId() == 0);
	delete ap;
//	PTRACE(eLevelInfoNormal,"CTestResourceProcess::testConstructor");
//
//	CPPUNIT_ASSERT_MESSAGE( "CTestResourceProcess::testConstructor ",
//		CProcessBase::GetProcess() != NULL );

}


//////////////////////////////////////////////////////////////////////
void CTestActivePort::testPartyId()
{
	CActivePort * ap = new CActivePort;
	CPPUNIT_ASSERT_MESSAGE("party id init", ap->GetPartyId() == 0);
	delete ap;
}

//////////////////////////////////////////////////////////////////////
void CTestActivePort::testSystemResourcesCons()
{
	CSystemResources * sr = new CSystemResources;
	CPPUNIT_ASSERT_MESSAGE("testSystemResourcesCons", sr != NULL);
	PartyDataStruct partyData;
	memset(&partyData, 0, sizeof(partyData));
	partyData.m_videoPartyType = eVideo_party_type_none;
	partyData.m_networkPartyType = eIP_network_party_type;
	STATUS stat = sr->AllocateART(1,//WORD rsrcConfId,
                                  2,//WORD rsrcPartyId,
                                  0,//DWORD partyCapacity
                                  ePhysical_art,// eResourceTypes physType,
                                  1, // WORD serviceId
                                  1,  // WORD subserviceId,
                                  new PhysicalPortDesc, partyData, 0,1,0);// PhysicalPortDesc* pPortDesc)

	// should fail since we didn't enabled cards yet
	CPPUNIT_ASSERT_MESSAGE("testSystemResourcesCons", stat != STATUS_OK);
	delete sr;
}


