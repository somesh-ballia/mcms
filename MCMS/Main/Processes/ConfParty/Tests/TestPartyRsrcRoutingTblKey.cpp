// TestPartyRsrcRoutingTblKey.cpp: implementation of the CTestPartyRsrcRoutingTblKey class.
//
//////////////////////////////////////////////////////////////////////


#include "TestPartyRsrcRoutingTblKey.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestPartyRsrcRoutingTblKey );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CTestPartyRsrcRoutingTblKey::setUp()
{
	m_pRsrcParams = new CPartyRsrcRoutingTblKey;	
	m_pRsrcParamsOther = new CPartyRsrcRoutingTblKey;
}
//////////////////////////////////////////////////////////////////////
void CTestPartyRsrcRoutingTblKey::tearDown()
{
	 POBJDELETE(m_pRsrcParams);
	 POBJDELETE(m_pRsrcParamsOther);
}
//////////////////////////////////////////////////////////////////////
void CTestPartyRsrcRoutingTblKey::testConstructor()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testConstructor ",
		m_pRsrcParams!= NULL);
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testConstructor ",
		m_pRsrcParams->GetPartyRsrcId()== DUMMY_PARTY_ID);
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testConstructor ",
		m_pRsrcParams->GetConnectionId()== DUMMY_CONNECTION_ID);	
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testConstructor ",
		m_pRsrcParams->GetLogicalRsrcType()== eLogical_res_none);	
}

//////////////////////////////////////////////////////////////////////
void CTestPartyRsrcRoutingTblKey::testSetPartyRsrcID()
{
	m_pRsrcParams->SetPartyRsrcId(555);
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testSetPartyRsrcID ",
		m_pRsrcParams->GetPartyRsrcId()==555);
}
//////////////////////////////////////////////////////////////////////
void CTestPartyRsrcRoutingTblKey::testSetConnectionID()
{
	m_pRsrcParams->SetConnectionId(6785);
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testSetConnectionID ",
		m_pRsrcParams->GetConnectionId()==6785);
}
//////////////////////////////////////////////////////////////////////
void CTestPartyRsrcRoutingTblKey::testSetLRT()
{
	m_pRsrcParams->SetLogicalRsrcType(eLogical_rtp);
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testSetLRT ",
		m_pRsrcParams->GetLogicalRsrcType()==eLogical_rtp);
}

//////////////////////////////////////////////////////////////////////
void CTestPartyRsrcRoutingTblKey::testOperatorLessWithConnectionId()
{
	m_pRsrcParams->SetPartyRsrcId(2);
	m_pRsrcParams->SetConnectionId(100);
	
	m_pRsrcParamsOther->SetPartyRsrcId(2);
	m_pRsrcParamsOther->SetConnectionId(200);
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testOperatorLessWithConnectionId ",
		(*m_pRsrcParams < *m_pRsrcParamsOther));
}
//////////////////////////////////////////////////////////////////////
void CTestPartyRsrcRoutingTblKey::testOperatorLessWithPartyId()
{ 
	m_pRsrcParams->SetPartyRsrcId(100);
	m_pRsrcParamsOther->SetPartyRsrcId(200);
	CPPUNIT_ASSERT_MESSAGE( "CTestPartyRsrcRoutingTblKey::testOperatorLessWithPartyId ",
		*m_pRsrcParams < *m_pRsrcParamsOther);
}


