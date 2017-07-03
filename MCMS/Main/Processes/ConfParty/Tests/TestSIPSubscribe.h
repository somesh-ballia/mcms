// // Boris G remove this tests (tests not relevant).

//
//#ifndef TEST_SIP_CONF_PACKAGE_H
//#define  TEST_SIP_CONF_PACKAGE_H
//
//#include <cppunit/extensions/HelperMacros.h>
//
//#include "SIPConfPack.h"
//#include "SipUtils.h"
//
//#include "ConfApi.h"
//#include "Conf.h"
//#include "SipRefer.h"
//
//#include "MockConfPartyManagerLocalApi.h"
//#include "MockCConfApi.h"
//#include "MockCMplMcmsProtocol.h"
//
//
//
//
///*void ParseNotifyReq(mcReqNotify* pNotifyReq, char* pUserUri, DWORD *ip, WORD *port, WORD *transport, char* pEvent, char* pState,
//					char* pContentType, char*pContent, DWORD *CSeq, char* pCallId, char* pFromTag, char* pToTag);
//*/
//////////////////////////////////////////////////////////////////////
//void ParseNotifyReq(mcReqNotify* pNotifyReq, char* pUserUri, DWORD *ip, WORD *port, WORD *transport, char* pEvent, char* pState,
//												char* pContentType, char* pContent, DWORD *CSeq, char* pCallId, char* pFromTag, char* pToTag)
//{
//
//	sipContentAndHeadersSt* pContentAndHeaders	= (sipContentAndHeadersSt*) &pNotifyReq->sipContentAndHeaders;
//	sipMessageHeaders* pHeaders = (sipMessageHeaders*) ((char*) pContentAndHeaders->contentAndHeaders + pContentAndHeaders->sipHeadersOffset);
//
//
//
//	strncpy(pContent, (char*) pContentAndHeaders->contentAndHeaders, pContentAndHeaders->sipHeadersOffset);
////	strncpy(pContentType, pNotifyReq->contentType, H243_NAME_LEN);
//	*CSeq = pNotifyReq->cseq;
//	*ip = pNotifyReq->transportAddress.transAddr.addr.v4.ip;
//	*port = pNotifyReq->transportAddress.transAddr.port;
//	*transport = pNotifyReq->transportAddress.transAddr.transportType;
//
//	CSipHeaderList * pTemp = new CSipHeaderList(*pHeaders);
//
//	const CSipHeader* pEventHeader = pTemp->GetNextHeader(kEvent);
//	if(pEventHeader)
//		strncpy(pEvent, (char*)pEventHeader->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pSubState = pTemp->GetNextHeader(kSubscrpState);
//	if(pSubState)
//		strncpy(pState, (char*)pSubState->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pToTagHeader = pTemp->GetNextHeader(kToTag);
//	if(pToTagHeader)
//		strncpy(pToTag, (char*)pToTagHeader->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pFromTagHeader = pTemp->GetNextHeader(kFromTag);
//	if(pFromTagHeader)
//		strncpy(pFromTag, (char*)pFromTagHeader->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pCallIdHeader = pTemp->GetNextHeader(kCallId);
//	if(pCallIdHeader)
//		strncpy(pCallId, (char*)pCallIdHeader->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pContentTypeHeader = pTemp->GetNextHeader(kContentType);
//	if(pContentTypeHeader)
//		strncpy(pContentType, (char*)pContentTypeHeader->GetHeaderStr(), H243_NAME_LEN);
//
//
//	const CSipHeader* pToHeader = pTemp->GetNextHeader(kTo);
//	if(pToHeader)
//		strncpy(pUserUri, (char*)pToHeader->GetHeaderStr(), MaxAddressListSize);
//
//
//}
///*
//////////////////////////////////////////////////////////////////////
//void ParseNotifyReq(mcReqNotify* pNotifyReq, char* pUserUri, DWORD *ip, WORD *port, WORD *transport, char* pEvent, char* pState,
//												char* pContentType, char*pContent, DWORD *CSeq, char* pCallId, char* pFromTag, char* pToTag)
//{
//	strncpy(pContent, pNotifyReq->content, H243_NAME_LEN);
//	strncpy(pContentType, pNotifyReq->contentType, H243_NAME_LEN);
//	*CSeq = pNotifyReq->cseq;
//	*ip = pNotifyReq->transportAddress.transAddr.addr.v4.ip;
//	*port = pNotifyReq->transportAddress.transAddr.port;
//	*transport = pNotifyReq->transportAddress.transAddr.transportType;
//
//	CSipHeaderList * pTemp = new CSipHeaderList(pNotifyReq->sipHeaders);
//
//	const CSipHeader* pEventHeader = pTemp->GetNextHeader(kEvent);
//	if(pEventHeader)
//		strncpy(pEvent, (char*)pEventHeader->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pSubState = pTemp->GetNextHeader(kSubscrpState);
//	if(pSubState)
//		strncpy(pState, (char*)pSubState->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pToTagHeader = pTemp->GetNextHeader(kToTag);
//	if(pToTagHeader)
//		strncpy(pToTag, (char*)pToTagHeader->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pFromTagHeader = pTemp->GetNextHeader(kFromTag);
//	if(pFromTagHeader)
//		strncpy(pFromTag, (char*)pFromTagHeader->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pCallIdHeader = pTemp->GetNextHeader(kCallId);
//	if(pCallIdHeader)
//		strncpy(pCallId, (char*)pCallIdHeader->GetHeaderStr(), H243_NAME_LEN);
//
//	const CSipHeader* pToHeader = pTemp->GetNextHeader(kTo);
//	if(pToHeader)
//		strncpy(pUserUri, (char*)pToHeader->GetHeaderStr(), MaxAddressListSize);
//
//}
//*/
//class CTestSIPSubscriber   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestSIPSubscriber );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testToString );
//	CPPUNIT_TEST( testGetBoardId );
//	CPPUNIT_TEST( testGetBoardId2 );
//	CPPUNIT_TEST( testGetFrom );
//	CPPUNIT_TEST( testGetFrom2 );
//	CPPUNIT_TEST( testGetFromTag );
//	CPPUNIT_TEST( testGetFromTag2 );
//	CPPUNIT_TEST( testGetTo );
//	CPPUNIT_TEST( testGetTo2 );
//	CPPUNIT_TEST( testGetToTag );
//	CPPUNIT_TEST( testGetToTag2 );
//	CPPUNIT_TEST( testGetIp );
//	CPPUNIT_TEST( testGetIp2 );
//	CPPUNIT_TEST( testGetPort );
//	CPPUNIT_TEST( testGetPort2 );
//	CPPUNIT_TEST( testGetTransport );
//	CPPUNIT_TEST( testGetTransport2 );
//	CPPUNIT_TEST( testGetExpireTime );
//	CPPUNIT_TEST( testGetExpireTime2 );
//	CPPUNIT_TEST( testRefresh );
//	CPPUNIT_TEST( testRefresh2 );
//	CPPUNIT_TEST( testGetCallId );
//	CPPUNIT_TEST( testGetCallId2 );
//	CPPUNIT_TEST( testGetAndIncrementNotifyVersionCounter1 );
//	CPPUNIT_TEST( testGetAndIncrementNotifyVersionCounter2 );
//	CPPUNIT_TEST( testGetSrcUnitId1 );
//	CPPUNIT_TEST( testGetSrcUnitId2 );
//	CPPUNIT_TEST_SUITE_END();
//
//
//public:
//	void setUp();
//	void tearDown();
//
//	void prepare();
//
//	void testConstructor();
//	void testToString();
//	void testGetBoardId();
//	void testGetBoardId2();
//	void testGetFrom();
//	void testGetFrom2();
//	void testGetFromTag();
//	void testGetFromTag2();
//	void testGetTo();
//	void testGetTo2();
//	void testGetToTag();
//	void testGetToTag2();
//	void testGetIp();
//	void testGetIp2();
//	void testGetPort();
//	void testGetPort2();
//	void testGetTransport();
//	void testGetTransport2();
//	void testGetExpireTime();
//	void testGetExpireTime2();
//	void testRefresh();
//	void testRefresh2();
//	void testGetCallId();
//	void testGetCallId2();
//	void testGetAndIncrementNotifyVersionCounter1();
//	void testGetAndIncrementNotifyVersionCounter2();
//	void testGetSrcUnitId1();
//	void testGetSrcUnitId2();
//
//protected:
//
//	char m_from[H243_NAME_LEN];
//	char m_fromTag[H243_NAME_LEN];
//	char m_to[H243_NAME_LEN];
//	char m_toTag[H243_NAME_LEN];
//	char m_callId[H243_NAME_LEN];
//	DWORD m_Ip, m_callIndex;
//	WORD m_port, m_transport;
//	WORD m_expires, m_srcUnitId;
//	CSIPSubscriber* m_pSubscriber;
//};
//
//
//
//
//class CTestSIPEventPackageManager : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestSIPEventPackageManager );
//	CPPUNIT_TEST( testConstructor );
//	//***** VNGSW-85 CPPUNIT_TEST( testNotifyWasCalledAfterTOUT );
//	//***** VNGSW-85 CPPUNIT_TEST( testNotifyWasNotCalledWhenTOUTisBigger );
//	//***** VNGSW-85 CPPUNIT_TEST( test2ndNotifyIsNotSendDueToTimer );
//	//***** VNGSW-85 CPPUNIT_TEST( testWaitingNotifyIsSentAfterTimer );
//	//***** VNGSW-85 CPPUNIT_TEST( testOnNotifyTimerToutDoesNotSendNotify );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribesIsCollectedandDeletedAfterTout );
//	//***** VNGSW-85 CPPUNIT_TEST( testNotifyWasNotCalledAfterRefresh );
//	//***** VNGSW-85 CPPUNIT_TEST( testDump );
//	//***** VNGSW-85 CPPUNIT_TEST( testEventPackageCalcShortesTimer1 );
//	//***** VNGSW-85 CPPUNIT_TEST( testEventPackageCalcShortesTimer2 );
//	//***** VNGSW-85 CPPUNIT_TEST( testEventPackageCalcShortesTimer3 );
//	CPPUNIT_TEST( testEventPackageCalcShortesTimer4 );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void BuildSubscribe();
//
//
//	void testConstructor();
//	void testNotifyWasCalledAfterTOUT();
//	void testNotifyWasNotCalledWhenTOUTisBigger();
//	void test2ndNotifyIsNotSendDueToTimer();
//	void testWaitingNotifyIsSentAfterTimer();
//	void testOnNotifyTimerToutDoesNotSendNotify();
//	void testSubscribesIsCollectedandDeletedAfterTout();
//	void testNotifyWasNotCalledAfterRefresh();
//	void testDump();
//	void testEventPackageCalcShortesTimer1();
//	void testEventPackageCalcShortesTimer2();
//	void testEventPackageCalcShortesTimer3();
//	void testEventPackageCalcShortesTimer4();
//
//private:
//
//	CMockMplMcmsProtocol*	 m_pMockProtocol;
//	CMockConfApi*			 m_pMockConfApi;
//	CSIPEventPackageManager* m_pMgr;
//	mcIndSubscribe*			 m_pSubscribeMsg;
//	CSipHeaderList*			 m_pHeaderList;
//
//	char m_from[H243_NAME_LEN];
//	char m_fromTag[H243_NAME_LEN];
//	char m_toTag[H243_NAME_LEN];
//	char m_callId[H243_NAME_LEN];
//	DWORD m_Ip;
//	WORD m_port, m_transport;
//	char m_event[H243_NAME_LEN];
//	char m_allow[H243_NAME_LEN];
//	WORD m_expires, m_srcUnitId;
//	void* m_CallObj;
//	CCommConfDB* m_pConfDB;
//};
//
//
//
//
//class CTestSIPEventPackageDispatcher   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestSIPEventPackageDispatcher );
//	CPPUNIT_TEST( testConstructor );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeHasRepliedWithOKForConfPkg );
//	//*******CPPUNIT_TEST( testSubscribeHasRepliedWithOKForSipCX );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeHasRepliedWithRejectDueToWrongEvent );
//	//***** VNGSW-145 CPPUNIT_TEST( testSubscribeHasRepliedWithRejectDueToEmptyIp );
//	//***** VNGSW-145 CPPUNIT_TEST( testSubscribeHasRepliedWithRejectDueToInvalidTransportType );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeHasRepliedWithOKForUDPTransportType );
//	//***** VNGSW-145 CPPUNIT_TEST( testSubscribeHasRepliedWithRejectDueToInvalidAllow );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeHasRepliedWithOKToEmptyAllow );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeConfHasRepliedOKWithGoodParams );
//	//***** VNGSW-85 CPPUNIT_TEST( test2SubscribesConfRepliedOKWithGoodParams );
//	//***** VNGSW-85 CPPUNIT_TEST( test2SubscribesRepliedRejectWithGoodParams );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeHasRepliedWithOKandSameExpires1 );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeHasRepliedWithOKandSameExpires2 );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeHasRepliedWithOKand3600Expires );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeConfIsCollected );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribesAreCollected );
//	//***** VNGSW-85 CPPUNIT_TEST( testRefreshRepliedWithOK );
//	//***** VNGSW-85 CPPUNIT_TEST( testUnsubscribeRepliedWithOK );
//	//***** VNGSW-85 CPPUNIT_TEST( testUnsubscribe );
//	//***** VNGSW-85 CPPUNIT_TEST( testUnsubscribeForUnknownpartyIsOK );
//	//***** VNGSW-85 CPPUNIT_TEST( testTerminateConfCallsAllSubscribersNotify );
//	//***** VNGSW-85 CPPUNIT_TEST( testTerminateConfCallsAllConfSubscribersNotify2 );
//	CPPUNIT_TEST( testTerminateConfCallsAllReferSubscribersNotify2 );
//	//***** VNGSW-85 CPPUNIT_TEST( testSubscribeRejectedAfterTerminateConf );
//	//***** VNGSW-85 CPPUNIT_TEST( testTerminateConfDeletesAllConfSubscribers );
//	//***** VNGSW-85 CPPUNIT_TEST( testTerminateConfDeletesAllReferSubscribers );
//
//	CPPUNIT_TEST( testHasRepliedWithOKForRefer );
//	CPPUNIT_TEST( testHasRepliedWithSameDstUnitIdAndCallIndexForRefer );
//	CPPUNIT_TEST( testReferRejectedDueToWrongAcceptHeader );
//	CPPUNIT_TEST( testReferRejectedSinceFromPartyIsNotInConf );
//	CPPUNIT_TEST( testReferHasRepliedOKWithGoodParams );
//	CPPUNIT_TEST( test2RefersRepliedOKWithGoodParams );
//	CPPUNIT_TEST( testReferIsCollected );
//	CPPUNIT_TEST( testReferSendNotify );
//	CPPUNIT_TEST( testReferAddsSIPParty );
//	//CPPUNIT_TEST( testReferAddsTelParty );
//	//CPPUNIT_TEST( testReferAddsH323Party );
//	CPPUNIT_TEST( testReferWithByeRejectedSinceReferToPartyIsNotInConf );
//	CPPUNIT_TEST( testReferWithByeHasRepliedOKWithGoodParams );
//	/********CPPUNIT_TEST( testReferForSIP_CX_SubscribedParty );
//	CPPUNIT_TEST( testParseAddressForSip1 );
//	CPPUNIT_TEST( testParseAddressForSip2 );
//	CPPUNIT_TEST( testParseAddressForSip3 );
//	CPPUNIT_TEST( testParseAddressForSip4 );
//	CPPUNIT_TEST( testParseAddressForPstn8Dig );
//	CPPUNIT_TEST( testParseAddressForPstn10Dig );
//	CPPUNIT_TEST( testParseAddressForPstn7Dig );
//	CPPUNIT_TEST( testParseAddressForPstn6Dig );
//	CPPUNIT_TEST( testParseAddressForIsdn9Dig );
//	CPPUNIT_TEST( testParseAddressForH3239Dig );
//	CPPUNIT_TEST( testParseAddressForH323 );
//	CPPUNIT_TEST( testParseAddressForH323_2 );
//	CPPUNIT_TEST( testParseAddressForTel );
//	CPPUNIT_TEST( testParseAddressForUnknown );*/
//CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void BuildSubscribe();
//	void BuildRefer();
//
//	void testConstructor();
//	void testSubscribeHasRepliedWithOKForConfPkg();
//	void testSubscribeHasRepliedWithOKForSipCX();
//	void testSubscribeHasRepliedWithRejectDueToWrongEvent();
//	void testSubscribeHasRepliedWithRejectDueToEmptyIp();
//	void testSubscribeHasRepliedWithRejectDueToInvalidTransportType();
//	void testSubscribeHasRepliedWithOKForUDPTransportType();
//	void testSubscribeHasRepliedWithRejectDueToInvalidAllow();
//	void testSubscribeHasRepliedWithOKToEmptyAllow();
//	void testSubscribeConfHasRepliedOKWithGoodParams();
//	void test2SubscribesConfRepliedOKWithGoodParams();
//	void test2SubscribesRepliedRejectWithGoodParams();
//	void testSubscribeHasRepliedWithOKandSameExpires1();
//	void testSubscribeHasRepliedWithOKandSameExpires2();
//	void testSubscribeHasRepliedWithOKand3600Expires();
//	void testSubscribeConfIsCollected();
//	void testSubscribeReferIsCollected();
//	void testSubscribesAreCollected();
//	void testRefreshRepliedWithOK();
//	void testUnsubscribeRepliedWithOK();
//	void testUnsubscribe();
//	void testUnsubscribeForUnknownpartyIsOK();
//	void testTerminateConfCallsAllSubscribersNotify();
//	void testTerminateConfCallsAllConfSubscribersNotify2();
//	void testTerminateConfCallsAllReferSubscribersNotify2();
//	void testSubscribeRejectedAfterTerminateConf();
//	void testTerminateConfDeletesAllReferSubscribers();
//	void testTerminateConfDeletesAllConfSubscribers();
//
//	void testHasRepliedWithOKForRefer();
//	void testHasRepliedWithSameDstUnitIdAndCallIndexForRefer();
//	void testReferRejectedDueToWrongAcceptHeader();
//	void testReferRejectedSinceFromPartyIsNotInConf();
//	void testReferHasRepliedOKWithGoodParams();
//	void test2RefersRepliedOKWithGoodParams();
//	void testReferIsCollected();
//	void testReferSendNotify();
//	void testReferAddsSIPParty();
//	//void testReferAddsTelParty();
//	//void testReferAddsH323Party();
//	void testReferWithByeRejectedSinceReferToPartyIsNotInConf();
//	void testReferWithByeHasRepliedOKWithGoodParams();
//
//	void testReferForSIP_CX_SubscribedParty();
//
//	void testParseAddressForSip1();
//	void testParseAddressForSip2();
//	void testParseAddressForSip3();
//	void testParseAddressForSip4();
//	void testParseAddressForPstn8Dig();
//	void testParseAddressForPstn7Dig();
//	void testParseAddressForPstn6Dig();
//	void testParseAddressForPstn10Dig();
//	void testParseAddressForIsdn9Dig();
//	void testParseAddressForH3239Dig();
//	void testParseAddressForH323();
//	void testParseAddressForH323_2();
//	void testParseAddressForTel();
//	void testParseAddressForUnknown();
////	void testParseAddressForPstn11Dig();
////	void testCallBackToH323();
//
//private:
//
//	CSIPEventPackageDispatcher *m_pMgr;
//	CConf* m_pConf;
//	CMockMplMcmsProtocol* m_pMockProtocol;
//	CMockConfPartyManagerLocalApi* m_pMockConfPartyManagerApi;
//	mcIndSubscribe* m_pSubscribeMsg;
//	mcIndRefer*	 m_pReferMsg;
//	CSipHeaderList* m_pHeaderList;
//
//
//	char m_from[H243_NAME_LEN];
//	char m_fromTag[H243_NAME_LEN];
//	char m_toTag[H243_NAME_LEN];
//	DWORD m_Ip, m_callIndex, m_CSeq;
//	WORD m_port, m_transport, m_watcherId;
//	char m_event[H243_NAME_LEN];
//	char m_allow[H243_NAME_LEN];
//	char m_referTo[H243_NAME_LEN];
//	char m_callId[H243_NAME_LEN];
//	WORD m_expires, m_srcUnitId;
//	void* m_CallObj;
//
//	WORD _status, _expires;
//	DWORD _pHaCall;
//	int _CallIndex;
//	CCommConfDB* m_pConfDB;
//};
//
//
//class CTestSIPCXPackageManager : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestSIPCXPackageManager );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testNotifyBody );
//	CPPUNIT_TEST( testNotifyFullForSubscribe );
//	CPPUNIT_TEST( testNotifyWasCalledAfterTOUT );
//	CPPUNIT_TEST( testNotifyWasCalledAfterTOUT_AllSubs );
//	CPPUNIT_TEST( testNotifyWasCalledAfterTOUT_AllBoards );
//	CPPUNIT_TEST( testOnNotifyTimerToutDoesNotSendNotify );
//	CPPUNIT_TEST( testOnNotifyTimerToutSendNotify );
//	CPPUNIT_TEST( testNotifyWasCalledForFullDataEvenInTimer );
//	CPPUNIT_TEST( testNotifySentAfterSubRefresh );
//	CPPUNIT_TEST( testNotifyWithDeltaIsSentAfterLock );
//	CPPUNIT_TEST( testNotifyWithDeltaIsSentAfterPartyDeleted );
//	CPPUNIT_TEST( testNotifyWithDeltaIsSentToAllSubscribersInBoard );
//	CPPUNIT_TEST( testNotifyWithDeltaIsSentToAllSubscribersInAllBoards );
//	CPPUNIT_TEST( testNotifyWithDeltaIsNotSentToUnSubscribed );
//	CPPUNIT_TEST( testTerminateConfCallsAllConfSubscribersNotify );
//	CPPUNIT_TEST( testTerminateConfCallsAllConfSubscribersNotifyInAllBoards );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void BuildSubscribe();
//
//
//	void testConstructor();
//	void testNotifyBody();
//	void testNotifyFullForSubscribe();
//	void testNotifyWasCalledAfterTOUT();
//	void testNotifyWasCalledAfterTOUT_AllSubs();
//	void testNotifyWasCalledAfterTOUT_AllBoards();
//	void testOnNotifyTimerToutDoesNotSendNotify();
//	void testOnNotifyTimerToutSendNotify();
//	void testNotifyWasCalledForFullDataEvenInTimer();
//	void testNotifySentAfterSubRefresh();
//	void testNotifyWithDeltaIsSentAfterLock();
//	void testNotifyWithDeltaIsSentAfterPartyDeleted();
//	void testNotifyWithDeltaIsSentToAllSubscribersInBoard();
//	void testNotifyWithDeltaIsSentToAllSubscribersInAllBoards();
//	void testNotifyWithDeltaIsNotSentToUnSubscribed();
//	void testTerminateConfCallsAllConfSubscribersNotify();
//	void testTerminateConfCallsAllConfSubscribersNotifyInAllBoards();
//
//private:
//	CMockMplMcmsProtocol*		 m_pMockProtocol;
//	CMockConfApi*			 m_pMockConfApi;
//	CSIPCXPackageManager*	 m_pMgr;
//	mcIndSubscribe*	 m_pSubscribeMsg;
//	CSipHeaderList* m_pHeaderList;
//
//	char m_from[H243_NAME_LEN];
//	DWORD m_callIndex;
//	char m_event[H243_NAME_LEN];
//	char m_allow[H243_NAME_LEN];
//	WORD m_expires;
//	WORD m_watcherId;
//	void* m_CallObj;
//};
//
//#endif // TEST_SIP_CONF_PACKAGE_H
