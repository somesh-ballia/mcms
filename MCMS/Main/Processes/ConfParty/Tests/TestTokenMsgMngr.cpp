#include "TestTokenMsgMngr.h"
#include "TokenMsgMngr.h"
#include "Segment.h"
#include "TokenMsgMngrMock.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestTokenMsgMngr );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::setUp()
{
	m_pTokenMsgMngr = new CTokenMsgMngrMock;
	m_pTokenMsgMngr->EnableTokenMsgMngr();
	CSegment* pParam1 = new CSegment;
	CSegment* pParam2 = new CSegment;
	CSegment* pParam3 = new CSegment;
	m_pTokenMsg_1 = new CTokenMsg(PARTY_TOKEN_ACQUIRE,eMsgIn,pParam1);
	m_pTokenMsg_2 = new CTokenMsg(PARTY_TOKEN_ACQUIRE,eMsgIn,pParam2);
	m_pTokenMsg_3 = new CTokenMsg(PARTY_TOKEN_ACQUIRE,eMsgIn,pParam3);
}
//////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::tearDown()
{
	POBJDELETE(m_pTokenMsgMngr);
//	POBJDELETE(m_pTokenMsg_1);
//	POBJDELETE(m_pTokenMsg_2);
//	POBJDELETE(m_pTokenMsg_3);
}
//////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testConstructor()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestTokenMsgMngr::testConstructor ",
		m_pTokenMsgMngr != NULL );  
} 
//////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testAddTokenMsg()
{
 	m_pTokenMsgMngr->AddTokMsg(m_pTokenMsg_1);

	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 1 );  
} 
//////////////////////////////////////////////////////////////////////
/*void CTestTokenMsgMngr::testAddAndRemoveTokenMsg()
{
	m_pTokenMsgMngr->AddTokMsg(m_pTokenMsg_1);
	m_pTokenMsgMngr->Remove(m_pTokenMsg_1);
	
	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 0 );  
}
//////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testAddTwoTokenMsg()
{
	m_pTokenMsgMngr->AddTokMsg(m_pTokenMsg_1);
	m_pTokenMsgMngr->Remove(m_pTokenMsg_1);
	
	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 0 );  
}

//////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testAddTwoTokenMsgsAndRemoveFirstAndCheckSize()
{
 	m_pTokenMsgMngr->AddTokMsg(m_pTokenMsg_1);
 	m_pTokenMsgMngr->AddTokMsg(m_pTokenMsg_2);
	
 	m_pTokenMsgMngr->Remove(m_pTokenMsg_1);

	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 1 );   
}
//////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testAddThreeTokenMsgsCheckSizeAndRemoveAllOfThemAndCheckSize()
{
	CTokenMsg* pTokenMsg = NULL;

 	m_pTokenMsgMngr->AddTokMsg(m_pTokenMsg_1);
 	m_pTokenMsgMngr->AddTokMsg(m_pTokenMsg_2);
 	m_pTokenMsgMngr->AddTokMsg(m_pTokenMsg_3);
 	
	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 3 );   
	
 	pTokenMsg = m_pTokenMsgMngr->Remove(m_pTokenMsg_1);
	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 2 );   

 	pTokenMsg = m_pTokenMsgMngr->Remove(m_pTokenMsg_2);
	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 1 );   

 	pTokenMsg = m_pTokenMsgMngr->Remove(m_pTokenMsg_3);
	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 0 );   

}
*/
//////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testAddThreeTokenMsgsAndClearAndDestroyTokenMsgList()
{
	CSegment* pParam1 = new CSegment;
	CSegment* pParam2 = new CSegment;
	CSegment* pParam3 = new CSegment;
	CTokenMsg* pTokenMsg_1 = new CTokenMsg(PARTY_TOKEN_ACQUIRE,eMsgIn,pParam1);
	CTokenMsg* pTokenMsg_2 = new CTokenMsg(PARTY_TOKEN_ACQUIRE,eMsgIn,pParam2);
	CTokenMsg* pTokenMsg_3 = new CTokenMsg(PARTY_TOKEN_ACQUIRE,eMsgIn,pParam3);
	
	m_pTokenMsgMngr->AddTokMsg(pTokenMsg_1);
	m_pTokenMsgMngr->AddTokMsg(pTokenMsg_2);
	m_pTokenMsgMngr->AddTokMsg(pTokenMsg_3);
 
 	m_pTokenMsgMngr->ClearAndDestroyList();

	CPPUNIT_ASSERT( m_pTokenMsgMngr->GetSize() == 0 );   
	
	
}
///////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testRecieveAcquireReqWithStreamOFF()
{
	CSegment* pParam = new CSegment;
	m_pTokenMsgMngr->EnableTokenMsgMngr();
	CTokenMsg* pTokenMsg = new CTokenMsg(PARTY_TOKEN_ACQUIRE,eMsgIn,pParam);
	EMsgStatus Status = m_pTokenMsgMngr->NewTokenMsg(pTokenMsg,eHwStreamStateOff);
	
	CPPUNIT_ASSERT( Status == eMsgFree ); 
	
	POBJDELETE(pTokenMsg);
}
////////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testRecieveCONTENT_ROLE_TOKEN_RELEASE__ReqFromParty()
{
	CSegment* pParam = new CSegment;
	
	CTokenMsg* pTokenMsg = new CTokenMsg(CONTENT_ROLE_TOKEN_RELEASE,eMsgIn,pParam);
	EMsgStatus Status = m_pTokenMsgMngr->NewTokenMsg(pTokenMsg,eHwStreamStateOn);
	
	CPPUNIT_ASSERT( Status == eMsgInvalid ); 
	
	POBJDELETE(pTokenMsg);
}
/////////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testOperatorOfTokenMsgList()
{
	BYTE partyRsrcID1	= 1;
	BYTE  confRsrcID1	= 2;
	
	BYTE partyRsrcID2	= 3;
	BYTE confRsrcID2	= 4;
	
	CSegment* pParam1 = new CSegment;
	*pParam1 << partyRsrcID1
			  << confRsrcID1;
			  
	CSegment* pParam2 = new CSegment;
	*pParam1 << partyRsrcID2
			  << confRsrcID2;		  
	
	CTokenMsg* pTokenMsg_1 = new CTokenMsg(CONTENT_ROLE_TOKEN_WITHDRAW,eMsgOut,pParam1);
	CTokenMsg* pTokenMsg_2 = new CTokenMsg(PARTY_TOKEN_ACQUIRE,eMsgIn,pParam2);
	
	m_pTokenMsgMngr->AddTokMsg(pTokenMsg_1);
	m_pTokenMsgMngr->AddTokMsg(pTokenMsg_2);

	CTokenMsgMngrMock* SecondTokenList = new CTokenMsgMngrMock;

	*SecondTokenList = *m_pTokenMsgMngr;
	
	m_pTokenMsgMngr->ClearAndDestroyList();
	
	BYTE res = 0;
	TOKEN_MSG_LIST::iterator itr = SecondTokenList->Begin();
	CTokenMsg* pTMsg1 = new CTokenMsg;
	CTokenMsg* pTMsg2 = new CTokenMsg;
		
		*pTMsg1 =*(*itr);
		if(pTMsg1->GetMsgOpcode() == CONTENT_ROLE_TOKEN_WITHDRAW)
		{
			CSegment* Msg1 = pTMsg1->GetMsgSegment();
			BYTE PartyID1;
			*Msg1>>PartyID1;
			if(PartyID1==1)
			{
				res =1;		
			}
		}

	CPPUNIT_ASSERT( res == 1 ); 
}
/////////////////////////////////////////////////////////////////////////////////////////
void CTestTokenMsgMngr::testOperatorOfTokenMsg()
{
	BYTE partyRsrcID1	= 1;
	BYTE  confRsrcID1	= 2;

	CSegment* pParam1 = new CSegment;
	*pParam1 << partyRsrcID1
			  << confRsrcID1;

	CTokenMsg* pTokenMsg_1 = new CTokenMsg(CONTENT_ROLE_TOKEN_WITHDRAW,eMsgOut,pParam1);

	CTokenMsg* SecondMsg = new CTokenMsg;
	*SecondMsg = *pTokenMsg_1;
	
	POBJDELETE(m_pTokenMsg_1);
	
	BYTE res = 0;
	if(SecondMsg->GetMsgOpcode() == CONTENT_ROLE_TOKEN_WITHDRAW)
	{
		CSegment* Msg1 = SecondMsg->GetMsgSegment();
		BYTE PartyID1;
		*Msg1>>PartyID1;
		if(PartyID1==1)
			res =2;
	}
	
	CPPUNIT_ASSERT( res == 2 );
}	
