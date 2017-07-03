//+========================================================================+
//                      TestBridge.cpp                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TestBridge.cpp	                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#include "TestBridge.h"
#include "BridgePartyCntlMock.h"
#include "ConfMock.h"
#include "BridgeMock.h"
#include "PartyMock.h"
#include "BridgeInitParams.h"
#include "Party.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestBridge );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestBridge::setUp()
{
	m_pBridge	= new CBridgeMock;
	m_pConf		= new CConfMock;

	((CConfMock*)m_pConf)->CreateMock();

	strncpy(m_pConfName, "Conf_1", H243_NAME_LEN);
	m_pConfName[H243_NAME_LEN-1]='\0';

	m_eBridgeImplementationType = eAudio_Bridge_V1;


}
//////////////////////////////////////////////////////////////////////
void CTestBridge::tearDown()
{
	POBJDELETE(m_pBridge);
	POBJDELETE(m_pConf);
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testConstructor()
{
	CBridgeMock bridge;
	CPPUNIT_ASSERT( bridge.GetNumParties() == 0 );
	CPPUNIT_ASSERT( bridge.GetPartyList() == NULL );
	CPPUNIT_ASSERT( bridge.GetConfApi() == NULL );
	CPPUNIT_ASSERT( bridge.GetConf() == NULL );
	CPPUNIT_ASSERT( bridge.GetBridgeImplementationType() == eNoType );
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testDestructor()
{
	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1, m_eBridgeImplementationType);

	CBridgeMock* pBridge = new CBridgeMock;

	pBridge->Create(&bridgeInitParams);

	CBridgePartyList*	pPartyList	= pBridge->GetPartyList();
	CConfApi*			pConfApi	= pBridge->GetConfApi();

	POBJDELETE(pBridge);

	CPPUNIT_ASSERT( ! CPObject::IsValidPObjectPtr(pPartyList) );

	CPPUNIT_ASSERT( ! CPObject::IsValidPObjectPtr(pConfApi) );

}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testCreate()
{
	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	CPPUNIT_ASSERT( m_pConf == ((CBridgeMock*)m_pBridge)->GetConf() );

	CPPUNIT_ASSERT( !strcmp(m_pConfName, ((CBridgeMock*)m_pBridge)->GetConfName() ) );

	CPPUNIT_ASSERT( ((CBridgeMock*)m_pBridge)->GetPartyList()->size() == 0  );

	CPPUNIT_ASSERT( ((CBridgeMock*)m_pBridge)->GetBridgeImplementationType() == eAudio_Bridge_V1  );

}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testDestroy()
{
	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	m_pBridge->Destroy();

	CPPUNIT_ASSERT( ! CPObject::IsValidPObjectPtr((((CBridgeMock*)m_pBridge)->GetPartyList())) );

	CPPUNIT_ASSERT( ! CPObject::IsValidPObjectPtr((((CBridgeMock*)m_pBridge)->GetConfApi())) );

}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testCheckIsConnectedAfterConstructorAndCreate()
{
	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1, m_eBridgeImplementationType);

	CBridgeMock* pBridge = new CBridgeMock;
	CPPUNIT_ASSERT( ! pBridge->IsConnected() );

	m_pBridge->Create(&bridgeInitParams);
	CPPUNIT_ASSERT( ! m_pBridge->IsConnected() );

	POBJDELETE(pBridge);
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testCheckIsConnectedAfterCreateAndChangingTheState()
{
	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1,m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ChangeState(IDLE+1);

	CPPUNIT_ASSERT( m_pBridge->IsConnected() );
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testIsPartyConnectedForEmptyBridge()
{
	CTaskApp* pParty = new CParty;

	CPPUNIT_ASSERT( ! m_pBridge->IsPartyConnected(pParty) );

	POBJDELETE(pParty);
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testConnectParty()
{
	CTaskApp* pParty	= new CPartyMock;

	((CPartyMock*)pParty)->CreateMock();

	CBridgePartyCntl* pBridgePartyCntl = new CBridgePartyCntlMock( pParty, "First", 1 );

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName,1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl);

	// simulate answer from BridgePartyCntl after connection
	CSegment params;
	params << (void*&)pParty << (WORD)0;
	((CBridgeMock*)m_pBridge)->OnEndPartyConnect(&params);

	CPPUNIT_ASSERT( m_pBridge->IsPartyConnected(pParty) );

	POBJDELETE(pParty);
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testConnectThreeParties()
{
	CTaskApp* pParty1 = (CTaskApp*)0x0038d9a8;
	CTaskApp* pParty2 = (CTaskApp*)0x0038d9b8;
	CTaskApp* pParty3 = (CTaskApp*)0x0038d9c8;

	CBridgePartyCntl* pBridgePartyCntl_1 = new CBridgePartyCntlMock( pParty1, "First", 1 );
	CBridgePartyCntl* pBridgePartyCntl_2 = new CBridgePartyCntlMock( pParty2, "Second", 2 );
	CBridgePartyCntl* pBridgePartyCntl_3 = new CBridgePartyCntlMock( pParty3, "Third", 3 );

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName,1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_1);

	CPPUNIT_ASSERT( m_pBridge->IsPartyConnected(pParty1) );

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_2);

	CPPUNIT_ASSERT( m_pBridge->IsPartyConnected(pParty2) );

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_3);

	CPPUNIT_ASSERT( m_pBridge->IsPartyConnected(pParty3) );
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testConnectThreePartiesAndCheckPartyNumber()
{
	CTaskApp* pParty1 = (CTaskApp*)0x0038d9a8;
	CTaskApp* pParty2 = (CTaskApp*)0x0038d9b8;
	CTaskApp* pParty3 = (CTaskApp*)0x0038d9c8;

	CBridgePartyCntl* pBridgePartyCntl_1 = new CBridgePartyCntlMock( pParty1, "First", 1 );
	CBridgePartyCntl* pBridgePartyCntl_2 = new CBridgePartyCntlMock( pParty2, "Second", 2 );
	CBridgePartyCntl* pBridgePartyCntl_3 = new CBridgePartyCntlMock( pParty3, "Third", 3 );

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName,1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_1);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_2);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_3);

	CPPUNIT_ASSERT( m_pBridge->GetNumParties() == 3 );
}

//////////////////////////////////////////////////////////////////////
CParty* CTestBridge::CreateParty()
{
	CParty* pParty = new CParty();
	PartyRsrcID PartyId = GetLookupIdParty()->Alloc();
	pParty->SetPartyId(PartyId);
	GetLookupTableParty()->Add(PartyId, pParty);
	return pParty;
}
//////////////////////////////////////////////////////////////////////
void CTestBridge::testDisConnectPartyWhenOnePartyIsConnected()
{
	CParty* pParty = CreateParty();

	CBridgePartyCntl* pBridgePartyCntl = new CBridgePartyCntlMock(pParty, "First", pParty->GetPartyId());

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl);

	((CBridgeMock*)m_pBridge)->DisconnectParty(pParty->GetPartyId());

	// simulate answer from BridgePartyCntl after disconnection
	CSegment params;
	params << pParty->GetPartyId() << (WORD)0;
	((CBridgeMock*)m_pBridge)->OnEndPartyDisConnect(&params);

	CPPUNIT_ASSERT(!m_pBridge->IsPartyConnected(pParty));

	CPPUNIT_ASSERT(m_pBridge->GetNumParties() == 0);

	delete pParty;
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testDisConnectTwoPartiesWhenThreePartiesAreConnected()
{
	CTaskApp* pParty1 = CreateParty();
	CTaskApp* pParty2 = CreateParty();
	CTaskApp* pParty3 = CreateParty();

	CBridgePartyCntl* pBridgePartyCntl_1 = new CBridgePartyCntlMock(pParty1, "First", pParty1->GetPartyId());
	CBridgePartyCntl* pBridgePartyCntl_2 = new CBridgePartyCntlMock(pParty2, "Second", pParty2->GetPartyId());
	CBridgePartyCntl* pBridgePartyCntl_3 = new CBridgePartyCntlMock(pParty3, "Third", pParty3->GetPartyId());

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_1);
	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_2);
	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_3);

	((CBridgeMock*)m_pBridge)->DisconnectParty(pParty2->GetPartyId());

	{ // simulate answer from BridgePartyCntl after disconnection
		CSegment params;
		params << pParty2->GetPartyId() << (WORD)0;
		((CBridgeMock*)m_pBridge)->OnEndPartyDisConnect(&params);
	}

	CPPUNIT_ASSERT(!m_pBridge->IsPartyConnected(pParty2));

	CPPUNIT_ASSERT(m_pBridge->GetNumParties() == 2);

	((CBridgeMock*)m_pBridge)->DisconnectParty(pParty3->GetPartyId());

	{ // simulate answer from BridgePartyCntl after disconnection
		CSegment params;
		params << pParty3->GetPartyId() << (WORD)0;
		((CBridgeMock*)m_pBridge)->OnEndPartyDisConnect(&params);
	}

	CPPUNIT_ASSERT(!m_pBridge->IsPartyConnected(pParty3));

	CPPUNIT_ASSERT(m_pBridge->GetNumParties() == 1);
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testGetPartyCntlByTaskAppPtrWhenThreePartiesAreConnected()
{
	/*
	CTaskApp* pParty1 = (CTaskApp*)0x0038d9a8;
	CTaskApp* pParty2 = (CTaskApp*)0x0038d9b8;
	CTaskApp* pParty3 = (CTaskApp*)0x0038d9c8;

	CBridgePartyCntl* pBridgePartyCntl_1 = new CBridgePartyCntlMock( pParty1, "First", 1 );
	CBridgePartyCntl* pBridgePartyCntl_2 = new CBridgePartyCntlMock( pParty2, "Second", 2 );
	CBridgePartyCntl* pBridgePartyCntl_3 = new CBridgePartyCntlMock( pParty3, "Third", 3 );

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1,m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_1);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl(pParty1) );

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_2);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl(pParty1) );
	CPPUNIT_ASSERT( pBridgePartyCntl_2 == m_pBridge->GetPartyCntl(pParty2) );

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_3);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl(pParty1) );
	CPPUNIT_ASSERT( pBridgePartyCntl_2 == m_pBridge->GetPartyCntl(pParty2) );
	CPPUNIT_ASSERT( pBridgePartyCntl_3 == m_pBridge->GetPartyCntl(pParty3) );
	*/
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testGetPartyCntlByNameWhenThreePartiesAreConnected()
{
	/*
	CTaskApp* pParty1 = (CTaskApp*)0x0038d9a8;
	CTaskApp* pParty2 = (CTaskApp*)0x0038d9b8;
	CTaskApp* pParty3 = (CTaskApp*)0x0038d9c8;

	CBridgePartyCntl* pBridgePartyCntl_1 = new CBridgePartyCntlMock( pParty1, "First", 1 );
	CBridgePartyCntl* pBridgePartyCntl_2 = new CBridgePartyCntlMock( pParty2, "Second", 2 );
	CBridgePartyCntl* pBridgePartyCntl_3 = new CBridgePartyCntlMock( pParty3, "Third", 3 );

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName,1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_1);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl("First") );

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_2);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl("First") );
	CPPUNIT_ASSERT( pBridgePartyCntl_2 == m_pBridge->GetPartyCntl("Second") );

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_3);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl("First") );
	CPPUNIT_ASSERT( pBridgePartyCntl_2 == m_pBridge->GetPartyCntl("Second") );
	CPPUNIT_ASSERT( pBridgePartyCntl_3 == m_pBridge->GetPartyCntl("Third") );*/
}

//////////////////////////////////////////////////////////////////////
void CTestBridge::testGetPartyCntlByPartyRsrcIdWhenThreePartiesAreConnected()
{
	CTaskApp* pParty1 = (CTaskApp*)0x0038d9a8;
	CTaskApp* pParty2 = (CTaskApp*)0x0038d9b8;
	CTaskApp* pParty3 = (CTaskApp*)0x0038d9c8;

	CBridgePartyCntlMock* pBridgePartyCntl_1 = new CBridgePartyCntlMock( pParty1, "First", 1 );
	CBridgePartyCntlMock* pBridgePartyCntl_2 = new CBridgePartyCntlMock( pParty2, "Second", 2 );
	CBridgePartyCntlMock* pBridgePartyCntl_3 = new CBridgePartyCntlMock( pParty3, "Third", 3 );

	pBridgePartyCntl_1->SetPartyRsrcID(1);
	pBridgePartyCntl_2->SetPartyRsrcID(2);
	pBridgePartyCntl_3->SetPartyRsrcID(3);

	CBridgeInitParams bridgeInitParams(m_pConf,  m_pConfName,1, m_eBridgeImplementationType);

	m_pBridge->Create(&bridgeInitParams);

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_1);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl((PartyRsrcID)1) );

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_2);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl((PartyRsrcID)1) );
	CPPUNIT_ASSERT( pBridgePartyCntl_2 == m_pBridge->GetPartyCntl((PartyRsrcID)2) );

	((CBridgeMock*)m_pBridge)->ConnectParty(pBridgePartyCntl_3);
	CPPUNIT_ASSERT( pBridgePartyCntl_1 == m_pBridge->GetPartyCntl((PartyRsrcID)1) );
	CPPUNIT_ASSERT( pBridgePartyCntl_2 == m_pBridge->GetPartyCntl((PartyRsrcID)2) );
	CPPUNIT_ASSERT( pBridgePartyCntl_3 == m_pBridge->GetPartyCntl((PartyRsrcID)3) );
}

//////////////////////////////////////////////////////////////////////



