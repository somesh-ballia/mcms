//+========================================================================+
//                     TestBridgeInterface.cpp                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TestBridgeInterface.cpp	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+



#include "TestBridgeInterface.h"
#include "BridgeInterfaceMock.h"
#include "ConfMock.h"
#include "PartyMock.h"
#include "BridgeMock.h"
#include "BridgePartyCntlMock.h"
#include "SystemFunctions.h"
#include "BridgePartyDisconnectParams.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestBridgeInterface );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestBridgeInterface::setUp()
{
	m_pBridgeInterface = new CBridgeInterfaceMock;
	m_pConf		= new CConfMock;

	((CConfMock*)m_pConf)->CreateMock();

	m_pParty	= new CPartyMock;

	((CPartyMock*)m_pParty)->CreateMock();
	PartyRsrcID PartyId = GetLookupIdParty()->Alloc();
	m_pParty->SetPartyId(PartyId);
	GetLookupTableParty()->Add(PartyId, m_pParty);

	strncpy(m_pConfName, "Conf_1", H243_NAME_LEN);
	m_pConfName[H243_NAME_LEN-1]='\0';

	strncpy(m_pPartyName, "First", H243_NAME_LEN);
	m_pPartyName[H243_NAME_LEN-1]='\0';

	m_partyRsrcID	= PartyId;

	m_confRsrcID	= 1;

	m_wNetworkInterface	=	H323_INTERFACE_TYPE;

	m_eBridgeImplementationType = eAudio_Bridge_V1;


}
//////////////////////////////////////////////////////////////////////
void CTestBridgeInterface::tearDown()
{
	POBJDELETE(m_pBridgeInterface);
	POBJDELETE(m_pConf);

	POBJDELETE(m_pParty);

}

//////////////////////////////////////////////////////////////////////
void CTestBridgeInterface::testConstructor()
{
	CPPUNIT_ASSERT( ((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation() == NULL );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgeInterface::testCreate()
{
	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1, m_eBridgeImplementationType);

	BYTE numOfBridgeImplementations = 1;

	m_pBridgeInterface->Create(&bridgeInitParams, numOfBridgeImplementations);

	CPPUNIT_ASSERT( ((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation() != NULL );

	CPPUNIT_ASSERT( m_pConf == ((CBridgeMock*)(((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation()))->GetConf() );

	CPPUNIT_ASSERT( !strcmp(m_pConfName, ((CBridgeMock*)(((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation()))->GetConfName() ) );

	CPPUNIT_ASSERT( ((CBridgeMock*)(((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation()))->GetPartyList()->size() == 0  );

}

//////////////////////////////////////////////////////////////////////
/* This test causes assert. So it looks like test is failed. To be defined how to treat such cases.
void CTestBridgeInterface::testCreateWithZeroBridgeImplementations()
{
	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, m_eBridgeImplementationType);

	BYTE numOfBridgeImplementations = 0;

	m_pBridgeInterface->Create(&bridgeInitParams, numOfBridgeImplementations);

	CPPUNIT_ASSERT( ((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation() == NULL );
}
*/
//////////////////////////////////////////////////////////////////////
void CTestBridgeInterface::testIsBridgeConnected()
{
	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1,m_eBridgeImplementationType);

	BYTE numOfBridgeImplementations = 1;

	m_pBridgeInterface->Create(&bridgeInitParams, numOfBridgeImplementations);

	CPPUNIT_ASSERT( m_pBridgeInterface->IsBridgeConnected() == FALSE );

	((CBridgeMock*)(((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation()))->ChangeState(IDLE+1);

	CPPUNIT_ASSERT( m_pBridgeInterface->IsBridgeConnected() == TRUE );
}
//////////////////////////////////////////////////////////////////////
void CTestBridgeInterface::testIsPartyConnectedForEmptyBridge()
{
	CTaskApp* pParty = (CTaskApp*)0x0038d9d8;
	BYTE numOfBridgeImplementations = 1;

	CBridgePartyCntl* pBridgePartyCntl = new CBridgePartyCntlMock( pParty, "First", 1);

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName, 1,m_eBridgeImplementationType);

	m_pBridgeInterface->Create(&bridgeInitParams, numOfBridgeImplementations);

	CPPUNIT_ASSERT( m_pBridgeInterface->IsPartyConnected(pParty) == FALSE );

}
//////////////////////////////////////////////////////////////////////
void CTestBridgeInterface::testConnectPartyAndCheckIsPartyConnectedAndNumOfParties()
{
	//CTaskApp* pParty = (CTaskApp*)0x0038d9d8;
	BYTE numOfBridgeImplementations = 1;

	CBridgePartyCntl* pBridgePartyCntl = new CBridgePartyCntlMock;

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName,1, m_eBridgeImplementationType);

	CBridge* pBridge		= ((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation();

	CBridgePartyInitParams* pBridgePartyInitParams = new CBridgePartyInitParams(m_pPartyName, (CTaskApp*)m_pParty,
																				m_partyRsrcID,m_partyRoomId, m_wNetworkInterface);

	((CBridgeInterfaceMock*)m_pBridgeInterface)->Create(&bridgeInitParams, numOfBridgeImplementations);

	m_pBridgeInterface->ConnectParty(pBridgePartyInitParams);

	// simulate create and answer from BridgePartyCntl after connection
	pBridgePartyCntl->Create(pBridgePartyInitParams);
	((CBridgeMock*)(((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation()))->ConnectParty(pBridgePartyCntl);
	CSegment params;
	params << (void*&)m_pParty << (WORD)0;
	((CBridgeMock*)(((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation()))->OnEndPartyConnect(&params);

	CPPUNIT_ASSERT( m_pBridgeInterface->IsPartyConnected(m_pParty) == TRUE );

	CPPUNIT_ASSERT( m_pBridgeInterface->GetNumParties() == 1 );

	//POBJDELETE(pBridgePartyCntl); deleted when bridge is destroyed
	POBJDELETE(pBridgePartyInitParams);
}

//////////////////////////////////////////////////////////////////////
void CTestBridgeInterface::testDisConnectPartyAndCheckIsPartyConnectedAndNumOfParties()
{
	BYTE numOfBridgeImplementations = 1;

	CBridgePartyCntl* pBridgePartyCntl = new CBridgePartyCntlMock((CTaskApp*)m_pParty, m_pPartyName, m_partyRsrcID);

	std::ostringstream msg;
	msg << "Party:" << m_pParty << ", PartyName:" << m_pPartyName << ", PartyId:" << m_partyRsrcID;

	//CPPUNIT_ASSERT_MESSAGE(msg.str().c_str(), FALSE);

	CBridgeInitParams bridgeInitParams(m_pConf, m_pConfName,1, m_eBridgeImplementationType);

	CBridge* pBridge		= ((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation();

	CBridgePartyInitParams* pBridgePartyInitParams = new CBridgePartyInitParams(m_pPartyName, (CTaskApp*)m_pParty,
																				m_partyRsrcID,m_partyRoomId, m_wNetworkInterface);

	((CBridgeInterfaceMock*)m_pBridgeInterface)->Create(&bridgeInitParams, numOfBridgeImplementations);

	m_pBridgeInterface->ConnectParty(pBridgePartyInitParams);

	pBridgePartyCntl->Create(pBridgePartyInitParams);
	((CBridgeMock*)(((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation()))->ConnectParty(pBridgePartyCntl);

	CBridgePartyDisconnectParams bridgePartyDisconnectParams(m_partyRsrcID);

	m_pBridgeInterface->DisconnectParty(&bridgePartyDisconnectParams);

	// simulate answer from BridgePartyCntl after disconnection
	CSegment params;
	params << m_partyRsrcID << (WORD)0;
	((CBridgeMock*)(((CBridgeInterfaceMock*)m_pBridgeInterface)->GetBridgeImplementation()))->OnEndPartyDisConnect(&params);

	CPPUNIT_ASSERT( m_pBridgeInterface->IsPartyConnected(m_pParty) == FALSE );

	CPPUNIT_ASSERT( m_pBridgeInterface->GetNumParties() == 0 );

	//POBJDELETE(pBridgePartyCntl); deleted when bridge is destroyed
	POBJDELETE(pBridgePartyInitParams);
}

//////////////////////////////////////////////////////////////////////

