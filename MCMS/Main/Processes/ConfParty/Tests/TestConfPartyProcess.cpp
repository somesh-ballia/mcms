// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////


#include "TestConfPartyProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "ConfParty.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestConfPartyProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestConfPartyProcess::setUp()
{
	m_pConfPartyProcess = CProcessBase::GetProcess();
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyProcess::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestConfPartyProcess::testConstructor()
{


	FPTRACE(eLevelInfoNormal,"CTestConfPartyProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		m_pConfPartyProcess != NULL );  

//	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
//		m_pConfPartyProcess->m_pManagerApi != NULL );  

//	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
//		m_pConfPartyProcess->m_pTasks->size() > 5 ); 

//	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
//		m_pConfPartyProcess->m_selfKill == FALSE );	

	CPPUNIT_ASSERT_MESSAGE( "CTestConfPartyProcess::testConstructor ",
		m_pConfPartyProcess != NULL );  
} 

void CTestConfPartyProcess::testTemp()
{
//	CConfParty confParty;
//	confParty.SetToTag("bb");
//	confParty.SetFromTag("aa");
//	std::string tag = confParty.GetPartyTag();
//
//	CPPUNIT_ASSERT(tag == "aa:bb");
//
//	std::cout<<tag;
//
//	std::string fileType="audio/x-wav";
//	CPPUNIT_ASSERT(strncmp(fileType.c_str(), "audio", 5) == 0);
}




