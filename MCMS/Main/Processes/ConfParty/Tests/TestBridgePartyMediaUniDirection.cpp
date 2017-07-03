//+========================================================================+
//               BridgePartyMediaUniDirection.cpp                          |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyMediaUniDirection.cpp	                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+



#include "TestBridgePartyMediaUniDirection.h"
#include "BridgePartyMediaUniDirectionMock.h"
#include "BridgePartyCntlMock.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestBridgePartyMediaUniDirection );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyMediaUniDirection::setUp()
{

}
//////////////////////////////////////////////////////////////////////
void CTestBridgePartyMediaUniDirection::tearDown()
{

}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyMediaUniDirection::testConstructor()
{
	CBridgePartyMediaUniDirectionMock* pBridgePartyMediaUniDirection = new CBridgePartyMediaUniDirectionMock;

	CPPUNIT_ASSERT( pBridgePartyMediaUniDirection->GetBridgePartyCntl() == NULL );

	CPPUNIT_ASSERT( pBridgePartyMediaUniDirection->GetHardwareInterface() == NULL );
	POBJDELETE(pBridgePartyMediaUniDirection);
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyMediaUniDirection::testCreate()
{
	CBridgePartyMediaUniDirectionMock* pBridgePartyMediaUniDirection = new CBridgePartyMediaUniDirectionMock;

	ConnectionID ConnectionId	= 1;
	PartyRsrcID ParId			= 2;
	ConfRsrcID ConfId			= 3;
	eLogicalResourceTypes LRT = eLogical_res_none;

	CBridgePartyCntl* pBridgePartyCntl = new CBridgePartyCntlMock( (CTaskApp*)0x0038d9d8, "First", ParId);
	CRsrcParams* pRoutingTblKey		= new CRsrcParams(ConnectionId = 1000, ParId = 0, ConfId = 0, LRT = eLogical_net);

	pBridgePartyMediaUniDirection->Create(pBridgePartyCntl, pRoutingTblKey);

	CPPUNIT_ASSERT( pBridgePartyMediaUniDirection->GetBridgePartyCntl() == pBridgePartyCntl );

	CPPUNIT_ASSERT( pBridgePartyMediaUniDirection->GetHardwareInterface() == NULL );
	POBJDELETE(pBridgePartyMediaUniDirection);
	POBJDELETE(pBridgePartyCntl);
	POBJDELETE(pRoutingTblKey);
}

//////////////////////////////////////////////////////////////////////

