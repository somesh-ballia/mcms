//+========================================================================+
//                     TestBridgePartyList.cpp                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       TestBridgePartyList.cpp	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+



#include "TestBridgePartyList.h"
#include "BridgePartyList.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestBridgePartyList );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::setUp()
{
	m_pBridgePartyList = new CBridgePartyList;
	m_pBridgePartyCntl_1 = new CBridgePartyCntlMock((void*)0x0038d9d8, "First" , 1);
	m_pBridgePartyCntl_2 = new CBridgePartyCntlMock((void*)0x0038e4f8, "Second", 2);
	m_pBridgePartyCntl_3 = new CBridgePartyCntlMock((void*)0x0038d890, "Third" , 3);
}
//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::tearDown()
{
	POBJDELETE(m_pBridgePartyList);
	POBJDELETE(m_pBridgePartyCntl_1);
	POBJDELETE(m_pBridgePartyCntl_2);
	POBJDELETE(m_pBridgePartyCntl_3);
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testConstructor()
{
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddOneBridgePartyCntlAndCheckSize()
{
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 1 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddAndRemoveOneBridgePartyCntlAndCheckSize()
{
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);

 	m_pBridgePartyList->Remove(m_pBridgePartyCntl_1->GetPartyRsrcID());

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 0 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddAndRemoveOneBridgePartyCntlAndCheckRemovedPartyPtr()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);

 	pBridgePartyCntl = m_pBridgePartyList->Remove(m_pBridgePartyCntl_1->GetPartyRsrcID());

	CPPUNIT_ASSERT( pBridgePartyCntl == m_pBridgePartyCntl_1 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddTwoBridgePartyCntlsAndCheckSize()
{
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 2 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddTwoBridgePartyCntlsAndRemoveFirstAndCheckRemovedParty()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);

 	pBridgePartyCntl = m_pBridgePartyList->Remove(m_pBridgePartyCntl_1->GetPartyRsrcID());

	CPPUNIT_ASSERT( pBridgePartyCntl == m_pBridgePartyCntl_1 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddTwoBridgePartyCntlsAndRemoveFirstAndCheckSize()
{
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);

 	m_pBridgePartyList->Remove(m_pBridgePartyCntl_1->GetPartyRsrcID());

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 1 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddTwoBridgePartyCntlsAndRemoveSecondAndCheckRemovedParty()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);

 	pBridgePartyCntl = m_pBridgePartyList->Remove(m_pBridgePartyCntl_2->GetPartyRsrcID());

	CPPUNIT_ASSERT( pBridgePartyCntl == m_pBridgePartyCntl_2 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddTwoBridgePartyCntlsAndRemoveSecondAndCheckSize()
{
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);

 	m_pBridgePartyList->Remove(m_pBridgePartyCntl_2->GetPartyRsrcID());

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 1 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddThreeBridgePartyCntlsAndRemoveSecondAndCheckRemovedParty()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_3);

 	pBridgePartyCntl = m_pBridgePartyList->Remove(m_pBridgePartyCntl_2->GetPartyRsrcID());

	CPPUNIT_ASSERT( pBridgePartyCntl == m_pBridgePartyCntl_2 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddTreeBridgePartyCntlsAndRemoveSecondAndCheckSize()
{
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_3);

 	m_pBridgePartyList->Remove(m_pBridgePartyCntl_2->GetPartyRsrcID());

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 2 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddThreeBridgePartyCntlsCheckSizeAndRemoveAllOfThemAndCheckSize()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_3);

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 3 );

 	pBridgePartyCntl = m_pBridgePartyList->Remove(m_pBridgePartyCntl_2->GetPartyRsrcID());
	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 2 );

 	pBridgePartyCntl = m_pBridgePartyList->Remove(m_pBridgePartyCntl_1->GetPartyRsrcID());
	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 1 );

 	pBridgePartyCntl = m_pBridgePartyList->Remove(m_pBridgePartyCntl_3->GetPartyRsrcID());
	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 0 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddTreeBridgePartyCntlsAndFindSecondAndCheckSize()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_3);

 	pBridgePartyCntl = m_pBridgePartyList->Find((const CParty*)m_pBridgePartyCntl_2->GetPartyTaskApp());

	CPPUNIT_ASSERT( pBridgePartyCntl == m_pBridgePartyCntl_2 );
	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 3 );

}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddTreeBridgePartyCntlsAndFindSecondByNameAndCheckSize()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

	CBridgePartyCntl* pCompareByNameBridgePartyCntl = new CBridgePartyCntlMock((void* )0, "Second", 2);

	pCompareByNameBridgePartyCntl->SetCompareFlag(eCompareByName);

 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_3);

 	pBridgePartyCntl = m_pBridgePartyList->Find(pCompareByNameBridgePartyCntl->GetName());

	CPPUNIT_ASSERT( pBridgePartyCntl == m_pBridgePartyCntl_2 );
	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 3 );

	POBJDELETE(m_pBridgePartyCntl_3);
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddFiveBridgePartyCntlsAndRemovedThirdAndCheckSizeAndFindFourthByName()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

	CBridgePartyCntl* pBridgePartyCntl_4 = new CBridgePartyCntlMock((void*)0x0038d9f8, "Fourth", 4);
	CBridgePartyCntl* pBridgePartyCntl_5 = new CBridgePartyCntlMock((void*)0x0038e4e8, "Fifth" , 5);


	CBridgePartyCntl* pCompareByNameBridgePartyCntl = new CBridgePartyCntlMock((void* )0, "Fourth", 4);

	pCompareByNameBridgePartyCntl->SetCompareFlag(eCompareByName);

	CBridgePartyCntl* pRemoveBridgePartyCntl = new CBridgePartyCntlMock((void*)0x0038d890, "", 3); // Init with "Third" party ID

 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);
 	m_pBridgePartyList->Insert(m_pBridgePartyCntl_3);
 	m_pBridgePartyList->Insert(pBridgePartyCntl_4);
 	m_pBridgePartyList->Insert(pBridgePartyCntl_5);

 	pBridgePartyCntl = m_pBridgePartyList->Remove(pRemoveBridgePartyCntl->GetPartyRsrcID());

	CPPUNIT_ASSERT( pBridgePartyCntl == m_pBridgePartyCntl_3 );
	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 4 );

 	pBridgePartyCntl = m_pBridgePartyList->Find(pCompareByNameBridgePartyCntl->GetName());
	CPPUNIT_ASSERT( pBridgePartyCntl == pBridgePartyCntl_4 );


	POBJDELETE(pBridgePartyCntl_4);
	POBJDELETE(pBridgePartyCntl_5);
	POBJDELETE(pCompareByNameBridgePartyCntl);
	POBJDELETE(pRemoveBridgePartyCntl);
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddFiveBridgePartyCntlsAndFindAt4()
{
	CBridgePartyCntl* pBridgePartyCntl_4 = new CBridgePartyCntlMock((void*)0x0038d9f8, "Fourth", 4);
	CBridgePartyCntl* pBridgePartyCntl_5 = new CBridgePartyCntlMock((void*)0x0038e4e8, "Fifth", 5);

	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);
	m_pBridgePartyList->Insert(m_pBridgePartyCntl_3);
	m_pBridgePartyList->Insert(pBridgePartyCntl_4);
	m_pBridgePartyList->Insert(pBridgePartyCntl_5);

	CBridgePartyCntl* pBridgePartyCntl = m_pBridgePartyList->Find(pBridgePartyCntl_4->GetPartyRsrcID());

	CPPUNIT_ASSERT(pBridgePartyCntl == pBridgePartyCntl_4);
	CPPUNIT_ASSERT(m_pBridgePartyList->size() == 5);

	POBJDELETE(pBridgePartyCntl_4);
	POBJDELETE(pBridgePartyCntl_5);
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddFiveBridgePartyCntlsAndFindAt0()
{
	CBridgePartyCntl* pBridgePartyCntl_4 = new CBridgePartyCntlMock((void*)0x0038d9f8, "Fourth", 4);
	CBridgePartyCntl* pBridgePartyCntl_5 = new CBridgePartyCntlMock((void*)0x0038e4e8, "Fifth", 5);

	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);
	m_pBridgePartyList->Insert(m_pBridgePartyCntl_2);
	m_pBridgePartyList->Insert(m_pBridgePartyCntl_3);
	m_pBridgePartyList->Insert(pBridgePartyCntl_4);
	m_pBridgePartyList->Insert(pBridgePartyCntl_5);

	CBridgePartyCntl* pBridgePartyCntl = m_pBridgePartyList->Find(m_pBridgePartyCntl_1->GetPartyRsrcID());

	CPPUNIT_ASSERT(pBridgePartyCntl == m_pBridgePartyCntl_1);
	CPPUNIT_ASSERT(m_pBridgePartyList->size() == 5);

	POBJDELETE(pBridgePartyCntl_4);
	POBJDELETE(pBridgePartyCntl_5);
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testZeroPtrOfBridgePartyCntlAndCheckSize()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

 	m_pBridgePartyList->Insert(pBridgePartyCntl);

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 0 );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testFindInEmptyList()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

 	pBridgePartyCntl = m_pBridgePartyList->Find((const CParty*)m_pBridgePartyCntl_1->GetPartyTaskApp());

	CPPUNIT_ASSERT( pBridgePartyCntl == NULL );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddOneBridgePartyCntlAndTryToRemoveAnother()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);

 	pBridgePartyCntl = m_pBridgePartyList->Remove(m_pBridgePartyCntl_2->GetPartyRsrcID());

	CPPUNIT_ASSERT( pBridgePartyCntl == NULL );
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddOneBridgePartyCntlAndTryToRemoveZeroPtr()
{
	/*
	CBridgePartyCntl* pBridgePartyCntl = NULL;

	m_pBridgePartyList->Insert(m_pBridgePartyCntl_1);

 	pBridgePartyCntl = m_pBridgePartyList->Remove(NULL);

	CPPUNIT_ASSERT( pBridgePartyCntl == NULL );*/
}

//////////////////////////////////////////////////////////////////////
void CTestBridgePartyList::testAddThreeBridgePartyCntlsClearAndDestroyPartyList()
{
	CBridgePartyCntl* pBridgePartyCntl = NULL;

	CBridgePartyCntl* pBridgePartyCntl_1 = new CBridgePartyCntlMock( (void*)0x0038d9d8, "Local_First" , 10);
	CBridgePartyCntl* pBridgePartyCntl_2 = new CBridgePartyCntlMock( (void*)0x0038e4f8, "Local_Second", 11);
	CBridgePartyCntl* pBridgePartyCntl_3 = new CBridgePartyCntlMock( (void*)0x0038d890, "Local_Third" , 12);

 	m_pBridgePartyList->Insert(pBridgePartyCntl_1);
 	m_pBridgePartyList->Insert(pBridgePartyCntl_2);
 	m_pBridgePartyList->Insert(pBridgePartyCntl_3);

 	m_pBridgePartyList->ClearAndDestroy();

	CPPUNIT_ASSERT( m_pBridgePartyList->size() == 0 );

}
