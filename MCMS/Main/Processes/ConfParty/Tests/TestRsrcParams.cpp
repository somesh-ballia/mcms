// CTestCRoutingTblKey.cpp: implementation of the CTestRsrcParams class.
//
//////////////////////////////////////////////////////////////////////


#include "TestRsrcParams.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestRsrcParams );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CTestRsrcParams::setUp()
{
	m_pRsrcParams = new CRsrcParams;	
	m_pRsrcParamsOther = new CRsrcParams;
}
//////////////////////////////////////////////////////////////////////
void CTestRsrcParams::tearDown()
{
	 POBJDELETE(m_pRsrcParams);
	 POBJDELETE(m_pRsrcParamsOther);
}
//////////////////////////////////////////////////////////////////////
void CTestRsrcParams::testConstructor()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testConstructor ",
		m_pRsrcParams!= NULL);
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testConstructor ",
		m_pRsrcParams->GetConfRsrcId()== DUMMY_CONF_ID);	
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testConstructor ",
		m_pRsrcParams->GetPartyRsrcId()== DUMMY_PARTY_ID);
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testConstructor ",
		m_pRsrcParams->GetConnectionId()== DUMMY_CONNECTION_ID);	
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testConstructor ",
		m_pRsrcParams->GetLogicalRsrcType()== eLogical_res_none);	
}
//////////////////////////////////////////////////////////////////////
void CTestRsrcParams::testSetConfRsrcID()
{
	m_pRsrcParams->SetConfRsrcId(100);
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testSetConfRsrcID ",
		m_pRsrcParams->GetConfRsrcId()==100);
}
//////////////////////////////////////////////////////////////////////
void CTestRsrcParams::testSetPartyRsrcID()
{
	m_pRsrcParams->SetPartyRsrcId(555);
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testSetPartyRsrcID ",
		m_pRsrcParams->GetPartyRsrcId()==555);
}
//////////////////////////////////////////////////////////////////////
void CTestRsrcParams::testSetConnectionID()
{
	m_pRsrcParams->SetConnectionId(6785);
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testSetConnectionID ",
		m_pRsrcParams->GetConnectionId()==6785);
}
//////////////////////////////////////////////////////////////////////
void CTestRsrcParams::testSetLRT()
{
	m_pRsrcParams->SetLogicalRsrcType(eLogical_rtp);
	CPPUNIT_ASSERT_MESSAGE( "CTestRsrcParams::testSetLRT ",
		m_pRsrcParams->GetLogicalRsrcType()==eLogical_rtp);
}



