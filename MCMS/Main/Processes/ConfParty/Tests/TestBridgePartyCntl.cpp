//+========================================================================+
//                     TestBridgePartyCntl.cpp                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TestBridgePartyCntl.cpp	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#include "TestBridgePartyCntl.h"
#include "BridgePartyInitParams.h"
#include "BridgePartyCntlMock.h"
#include "PartyMock.h"
#include "ConfMock.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestBridgePartyCntl );

PartyRsrcID partyRsrcID	= 1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyCntl::setUp()
{
	m_pConf		= new CConfMock;

	((CConfMock*)m_pConf)->CreateMock();

	m_pParty	= new CPartyMock;

	((CPartyMock*)m_pParty)->CreateMock();

	m_pBridgePartyCntl = new CBridgePartyCntlMock( (void*)m_pParty, "First", partyRsrcID);
}
//////////////////////////////////////////////////////////////////////
void CTestBridgePartyCntl::tearDown()
{
	POBJDELETE(m_pBridgePartyCntl);
	POBJDELETE(m_pConf);
	POBJDELETE(m_pParty);
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyCntl::testConstructor()
{
	// Actually tested in CTestBridge
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyCntl::testCreate()
{

	char* pPartyName		= "First";
	char* pConfName			= "Conf_1";
	CBridge* pBridge		= (CBridge*)0x00bbbbbb;
	ConfRsrcID  confRsrcID	= 1;
	WORD roomID = 2;
	WORD wNetworkInterface	= H323_INTERFACE_TYPE;


	CBridgePartyInitParams* pBridgePartyInitParams = new CBridgePartyInitParams(pPartyName, (CTaskApp*)m_pParty,
																				partyRsrcID,roomID, wNetworkInterface);
	pBridgePartyInitParams->SetBridge(pBridge);
	pBridgePartyInitParams->SetConf(m_pConf);
	pBridgePartyInitParams->SetConfName(pConfName);
	pBridgePartyInitParams->SetConfRsrcID(confRsrcID);

	m_pBridgePartyCntl->Create(pBridgePartyInitParams);

	CPPUNIT_ASSERT( m_pBridgePartyCntl->GetPartyTaskApp() == m_pParty );
	CPPUNIT_ASSERT( m_pBridgePartyCntl->GetPartyRsrcID() == partyRsrcID );
	CPPUNIT_ASSERT( ! strcmp(m_pBridgePartyCntl->GetName(),pPartyName) );
	//CPPUNIT_ASSERT( ! strcmp(m_pBridgePartyCntl->GetFullName(),"NAME - Conf_1,First") );
	CPPUNIT_ASSERT( ((CBridgePartyCntlMock*)(m_pBridgePartyCntl))->GetBridgePtr() == pBridge );
	CPPUNIT_ASSERT( ((CBridgePartyCntlMock*)(m_pBridgePartyCntl))->GetBridgePartyMediaIn() == NULL );
	CPPUNIT_ASSERT( ((CBridgePartyCntlMock*)(m_pBridgePartyCntl))->GetBridgePartyMediaOut() == NULL );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyCntl::testDestroy()
{

	char* pPartyName		= "First";
	char* pConfName			= "Conf_1";
	CBridge* pBridge		= (CBridge*)0x00bbbbbb;
	ConfRsrcID  confRsrcID	= 1;
	WORD roomID = 2;
	WORD wNetworkInterface	= H323_INTERFACE_TYPE;

	CBridgePartyInitParams* pBridgePartyInitParams = new CBridgePartyInitParams(pPartyName, (CTaskApp*)m_pParty,
																				partyRsrcID,roomID, wNetworkInterface);

	pBridgePartyInitParams->SetBridge(pBridge);
	pBridgePartyInitParams->SetConf(m_pConf);
	pBridgePartyInitParams->SetConfName(pConfName);
	pBridgePartyInitParams->SetConfRsrcID(confRsrcID);

	m_pBridgePartyCntl->Create(pBridgePartyInitParams);

	m_pBridgePartyCntl->Destroy();

	CPPUNIT_ASSERT( m_pBridgePartyCntl->GetPartyTaskApp() == m_pParty );
	CPPUNIT_ASSERT( m_pBridgePartyCntl->GetPartyRsrcID() == partyRsrcID );
	CPPUNIT_ASSERT( ! strcmp(m_pBridgePartyCntl->GetName(),pPartyName) );
	//CPPUNIT_ASSERT( ! strcmp(m_pBridgePartyCntl->GetFullName(),"NAME - Conf_1,First") );
	CPPUNIT_ASSERT( ((CBridgePartyCntlMock*)(m_pBridgePartyCntl))->GetBridgePtr() == pBridge );
	CPPUNIT_ASSERT( ((CBridgePartyCntlMock*)(m_pBridgePartyCntl))->GetBridgePartyMediaIn() == NULL );
	CPPUNIT_ASSERT( ((CBridgePartyCntlMock*)(m_pBridgePartyCntl))->GetBridgePartyMediaOut() == NULL );
	CPPUNIT_ASSERT( ((CBridgePartyCntlMock*)(m_pBridgePartyCntl))->GetPartyApi() == NULL );
	CPPUNIT_ASSERT( ((CBridgePartyCntlMock*)(m_pBridgePartyCntl))->GetConfApi() == NULL );
}

//////////////////////////////////////////////////////////////////////

