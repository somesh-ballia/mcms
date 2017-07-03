// Boris G remove this tests (tests not relevant).

//
//#include "TestSIPSubscribe.h"
//#include "Observer.h"
//#include "SystemFunctions.h"
//#include "IpCsOpcodes.h"
//#include "StatusesGeneral.h"
//
//
//// ************************************************************************************
////
////	CTestSIPSubscriber
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestSIPSubscriber );
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::setUp()
//{
//	strcpy(m_from,"party_1@polycom.com");
//	strcpy(m_fromTag, "34653sdf55");
//	strcpy(m_to,"conf1@polycom.com");
//	strcpy(m_toTag, "345fdghetr-345");
//	strcpy(m_callId, "123455hhgf-66547gfsedf");
//	m_Ip = 5678900;
//	m_callIndex = 234888;
//	m_port = 5060;
//	m_transport = eTransportTypeTcp;
//	m_expires = 3600;
//	m_srcUnitId = 100;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::tearDown()
//{
//	//test
//	POBJDELETE(m_pSubscriber);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testConstructor()
//{
////	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
////	CPPUNIT_ASSERT_MESSAGE( "CTestSIPSubscriber::testConstructor ", m_pSubscriber != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testToString()
//{
////	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
////	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong string returned", (std::string)m_pSubscriber->ToString(), (std::string)"party_1@polycom.com");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetBoardId()
//{
//	//	m_pSubscriber = new CSIPSubscriber(6, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	//	CPPUNIT_ASSERT_MESSAGE("wrong 'boardId' returned", 6 == m_pSubscriber->GetBoardId());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetBoardId2()
//{
//	//	m_pSubscriber = new CSIPSubscriber(7, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	//	CPPUNIT_ASSERT_MESSAGE("wrong 'boardId' returned", 7 == m_pSubscriber->GetBoardId());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetFrom()
//{
////	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
////	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'from' returned", (std::string)m_pSubscriber->GetFrom(), (std::string)"party_1@polycom.com");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetFrom2()
//{
////	strcpy(m_from,"party_2@polycom.com");
////
////	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
////	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'from' returned", (std::string)m_pSubscriber->GetFrom(), (std::string)"party_2@polycom.com");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetFromTag()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'fromTag' returned", (std::string)m_pSubscriber->GetFromTag(), (std::string)"34653sdf55");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetFromTag2()
//{
//	strcpy(m_fromTag,"sdf-45674567-678");
//
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'fromTag' returned", (std::string)m_pSubscriber->GetFromTag(), (std::string)"sdf-45674567-678");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetTo()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'to' returned", (std::string)m_pSubscriber->GetTo(), (std::string)"conf1@polycom.com");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetTo2()
//{
//	strcpy(m_to,"conf2@polycom.com");
//
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'to' returned", (std::string)m_pSubscriber->GetTo(), (std::string)"conf2@polycom.com");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetToTag()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'toTag' returned", (std::string)m_pSubscriber->GetToTag(), (std::string)"345fdghetr-345");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetToTag2()
//{
//	strcpy(m_toTag,"sdf-45674567-678");
//
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'toTag' returned", (std::string)m_pSubscriber->GetToTag(), (std::string)"sdf-45674567-678");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetIp()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong Ip returned", 5678900 == m_pSubscriber->GetIp());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetIp2()
//{
//	m_Ip = 12123456;
//
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong Ip returned", 12123456 == m_pSubscriber->GetIp());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetPort()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong Port returned", 5060 == m_pSubscriber->GetPort());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetPort2()
//{
//	m_port = 5070;
//
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong Port returned", 5070 == m_pSubscriber->GetPort());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetTransport()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong Transport returned", eTransportTypeTcp == m_pSubscriber->GetTransport());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetTransport2()
//{
//	m_transport = eTransportTypeUdp;
//
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong Transport returned", eTransportTypeUdp == m_pSubscriber->GetTransport());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetExpireTime()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//
//	CStructTm curTime;
//	STATUS timeStatus = SystemGetTime(curTime);
//	curTime.m_hour += m_expires/3600;
//	curTime.m_min += (m_expires%3600)/60;
//	curTime.m_sec += (m_expires%3600)%60;
//
//	CPPUNIT_ASSERT_MESSAGE("wrong Time returned", curTime == m_pSubscriber->GetExpireTime());
//
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetExpireTime2()
//{
//	m_expires = 1800;
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//
//	CStructTm curTime;
//	STATUS timeStatus = SystemGetTime(curTime);
//	curTime.m_hour += m_expires/3600;
//	curTime.m_min += (m_expires%3600)/60;
//	curTime.m_sec += (m_expires%3600)%60;
//
//	CPPUNIT_ASSERT_MESSAGE("wrong Time returned", curTime == m_pSubscriber->GetExpireTime());
//
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testRefresh()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//
//	m_expires = 1800;
//	CStructTm curTime;
//	STATUS timeStatus = SystemGetTime(curTime);
//	curTime.m_hour += m_expires/3600;
//	curTime.m_min += (m_expires%3600)/60;
//	curTime.m_sec += (m_expires%3600)%60;
//
//	m_pSubscriber->Refresh(m_Ip, m_port, m_transport, m_callId, m_expires);
//	CPPUNIT_ASSERT_MESSAGE("wrong Time returned", curTime == m_pSubscriber->GetExpireTime());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testRefresh2()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//
//	m_expires = 600;
//	CStructTm curTime;
//	STATUS timeStatus = SystemGetTime(curTime);
//	curTime.m_hour += m_expires/3600;
//	curTime.m_min += (m_expires%3600)/60;
//	curTime.m_sec += (m_expires%3600)%60;
//
//	m_pSubscriber->Refresh(m_Ip, m_port, m_transport, m_callId, m_expires);
//	CPPUNIT_ASSERT_MESSAGE("wrong Time returned", curTime == m_pSubscriber->GetExpireTime());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetCallId()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'CallId' returned", (std::string)m_pSubscriber->GetCallId(), (std::string)"123455hhgf-66547gfsedf");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetCallId2()
//{
//	strcpy(m_callId, "adfg54345345-234");
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong 'CallId' returned", (std::string)m_pSubscriber->GetCallId(), (std::string)"adfg54345345-234");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetAndIncrementNotifyVersionCounter1()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong notifyVersionCounter returned", 2 == m_pSubscriber->GetAndIncrementNotifyVersionCounter());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetAndIncrementNotifyVersionCounter2()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong notifyVersionCounter returned", 2 == m_pSubscriber->GetAndIncrementNotifyVersionCounter());
//	CPPUNIT_ASSERT_MESSAGE("wrong notifyVersionCounter returned", 3 == m_pSubscriber->GetAndIncrementNotifyVersionCounter());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetSrcUnitId1()
//{
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong srcUnitId returned", 100 == m_pSubscriber->GetSrcUnitId());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPSubscriber::testGetSrcUnitId2()
//{
//	m_srcUnitId = 200;
//	m_pSubscriber = new CSIPSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, m_callIndex);
//	CPPUNIT_ASSERT_MESSAGE("wrong srcUnitId returned", 200 == m_pSubscriber->GetSrcUnitId());
//}
//
//// ************************************************************************************
////
////	CTestSIPEventPackageDispatcher
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestSIPEventPackageDispatcher );
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::setUp()
//{
//	m_pMockProtocol = new CMockMplMcmsProtocol;
//	m_pMockConfPartyManagerApi = new CMockConfPartyManagerLocalApi;
//	m_pMgr = new CSIPEventPackageDispatcher(NULL, (CMplMcmsProtocol*)m_pMockProtocol, m_pMockConfPartyManagerApi);
//	strcpy(m_from,"party_1@polycom.com");
//	strcpy(m_fromTag, "345345ge63");
//	strcpy(m_toTag, "ert3465-346y");
//	strcpy(m_callId, "234234fsghdfhg-3546");
//	m_Ip = 5678900;
//	m_port = 5060;
//	m_transport = eTransportTypeTcp;
//	strcpy(m_event,"conference");
//	strcpy(m_allow, "application/conference-info+xml");
//	strcpy(m_referTo, "carol@chicago.example.com");
//	m_expires = 3600;
//	m_CallObj = (void*)0x04556778;
//	m_callIndex = 11223344;
//	m_watcherId = 1345;
//	m_pHeaderList = NULL;
//	m_pSubscribeMsg = NULL;
//	m_pReferMsg = NULL;
//	m_CSeq = 32;
//	m_srcUnitId = 100;
//	_status = 0;
//	_expires = 0;
//	_pHaCall = 0;
//	_CallIndex = 0;
//	m_pConfDB = ::GetpConfDB();
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::tearDown()
//{
//	if (m_pConfDB)
//		m_pConfDB->Cancel("conf1");
//	PDELETE(m_pHeaderList);
//	PDELETEA(m_pSubscribeMsg);
//	PDELETE(m_pReferMsg);
//	PDELETE(m_pMgr);
//	POBJDELETE(m_pMockProtocol);
//	POBJDELETE(m_pMockConfPartyManagerApi);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::BuildSubscribe()
//{
//	PDELETE(m_pHeaderList);
//	PDELETEA(m_pSubscribeMsg);
//
//	m_pHeaderList = new CSipHeaderList(MIN_ALLOC_HEADERS,0);
//	m_pHeaderList->AddHeader(kTo,   strlen("conf1@test"), "conf1@test");
//	m_pHeaderList->AddHeader(kToTag, strlen(m_toTag), m_toTag);
//	m_pHeaderList->AddHeader(kCallId, strlen(m_callId), m_callId);
//	m_pHeaderList->AddHeader(kFrom, strlen(m_from), m_from);
//	m_pHeaderList->AddHeader(kFromTag, strlen(m_fromTag), m_fromTag);
//	m_pHeaderList->AddHeader(kEvent, strlen(m_event), m_event);
//	m_pHeaderList->AddHeader(kAccept, strlen(m_allow), m_allow);
//	int len = m_pHeaderList->GetTotalLen();
//	m_pSubscribeMsg = (mcIndSubscribe*)new BYTE[sizeof(mcIndSubscribeBase)-sizeof(sipMessageHeadersBase)+len];
//	m_pSubscribeMsg->expires = m_expires;
//	m_pSubscribeMsg->transportAddress.transAddr.addr.v4.ip = m_Ip;
//	m_pSubscribeMsg->transportAddress.transAddr.port = m_port;
//	m_pSubscribeMsg->transportAddress.transAddr.transportType = m_transport;
////******	m_pSubscribeMsg->WatcherId = m_watcherId;
//	m_pSubscribeMsg->pCallObj = m_CallObj;
///*	m_pSubscribeMsg->WatcherId = m_watcherId;
//	m_pSubscribeMsg->header.pmHeader.haCall = m_haCall;
//	m_pSubscribeMsg->header.pmHeader.callIndex = m_callIndex;
//	m_pSubscribeMsg->header.pmHeader.status = 0;*/
//	CCommConf* pCommConf = new CCommConf;
//	pCommConf->SetName("conf1");
//	if (m_pConfDB)
//		m_pConfDB->Add(*pCommConf);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::BuildRefer()
//{
//	PDELETE(m_pHeaderList);
//	PDELETEA(m_pReferMsg);
//
//	m_pHeaderList = new CSipHeaderList(MIN_ALLOC_HEADERS,0);
//	m_pHeaderList->AddHeader(kTo,   strlen("conf1@test"), "conf1@test");
//	m_pHeaderList->AddHeader(kToTag, strlen(m_toTag), m_toTag);
//	m_pHeaderList->AddHeader(kFrom, strlen(m_from), m_from);
//	m_pHeaderList->AddHeader(kFromTag, strlen(m_fromTag), m_fromTag);
//	m_pHeaderList->AddHeader(kCallId, strlen(m_callId), m_callId);
//	m_pHeaderList->AddHeader(kReferTo, strlen(m_referTo), m_referTo);
//	m_pHeaderList->AddHeader(kAccept, strlen(m_allow), m_allow);
//	int len = m_pHeaderList->GetTotalLen();
//	m_pReferMsg = (mcIndRefer*)new BYTE[sizeof(mcIndReferBase)-sizeof(sipMessageHeadersBase)+len];
//	m_pReferMsg->transportAddress.transAddr.addr.v4.ip = m_Ip;
//	m_pReferMsg->transportAddress.transAddr.port = m_port;
//	m_pReferMsg->transportAddress.transAddr.transportType = m_transport;
//	m_pReferMsg->pCallObj = m_CallObj;
//	m_pReferMsg->remoteCseq = 7412;
///*	m_pReferMsg->header.pmHeader.haCall = m_haCall;
//	m_pReferMsg->header.pmHeader.callIndex = m_callIndex;
//	m_pReferMsg->header.pmHeader.status = 0;*/
//
//	CCommConf* pCommConf = new CCommConf;
//	pCommConf->SetName("conf1");
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetSipPartyAddress("party_1@polycom.com");
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//	if (m_pConfDB && STATUS_OK != m_pConfDB->Add(*pCommConf))
//		POBJDELETE(pCommConf);
//
//	POBJDELETE(pConfParty);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testConstructor()
//{
//	CPPUNIT_ASSERT_MESSAGE( "CTestSIPEventPackageDispatcher::testConstructor ", m_pMgr != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKForConfPkg()
//{
//	if (!m_pConfDB)
//		return;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", (pSubResponse->status == SipCodesOk || pSubResponse->status == SipCodesAccepted));
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKForSipCX()
//{
//	if (!m_pConfDB)
//		return;
//	CCommConf* pCommConf = new CCommConf;
//	pCommConf->SetName("conf1");
//	if (m_pConfDB)
//		m_pConfDB->Add(*pCommConf);
//
//	strcpy(m_event, "com.microsoft.sip.cx-conference-info");
//	strcpy(m_allow, "application/com.microsoft.sip.cx-conference-info+xml");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesOk);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithRejectDueToWrongEvent()
//{
//	if (!m_pConfDB)
//		return;
//	strcpy(m_event,"try1");
//
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesBadRequest);
//	PDELETE(pSubResponse);
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithRejectDueToEmptyIp()
//{
//	if (!m_pConfDB)
//			return;
//	m_Ip = 0;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesBadRequest);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithRejectDueToInvalidTransportType()
//{
//	m_transport = 0;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesBadRequest);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKForUDPTransportType()
//{
//	m_transport = eTransportTypeUdp;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesOk);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithRejectDueToInvalidAllow()
//{
//	strcpy(m_allow, "check");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesNotAcceptable);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKToEmptyAllow()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesOk);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeConfHasRepliedOKWithGoodParams()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesOk);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong expires ", pSubResponse->expires==m_expires);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall ", pSubResponse->pCallObj==m_pSubscribeMsg->pCallObj);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong CSeq ", pSubResponse->remoteCseq==m_pSubscribeMsg->remoteCseq);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::test2SubscribesConfRepliedOKWithGoodParams()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status == SipCodesOk);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong expires 1", pSubResponse->expires==m_expires);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall 1", pSubResponse->pCallObj==m_pSubscribeMsg->pCallObj);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong CSeq 1", pSubResponse->remoteCseq==m_pSubscribeMsg->remoteCseq);
//	PDELETE(pSubResponse);
//
//	_status = 0;
//	_expires = 0;
//	_CallIndex = 0;
//	_pHaCall = 0;
//	strcpy(m_from,"b@polycom.com");
//	m_Ip = 22222222;
//	m_CallObj = (void*)0x45baaa;
//	m_callIndex = 303020;
//	m_CSeq = 33;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status 2", pSubResponse->status == SipCodesOk);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong expires 2", pSubResponse->expires==m_expires);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall 2", pSubResponse->pCallObj==m_pSubscribeMsg->pCallObj);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong CSeq 2", pSubResponse->remoteCseq==m_pSubscribeMsg->remoteCseq);
//	PDELETE(pSubResponse);
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::test2SubscribesRepliedRejectWithGoodParams()
//{
//	strcpy(m_event,"54");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status 1", pSubResponse->status == SipCodesBadRequest);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong expires 1", pSubResponse->expires==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall 1", pSubResponse->pCallObj==m_pSubscribeMsg->pCallObj);
//	PDELETE(pSubResponse);
//
//	_status = 0;
//	_expires = 0;
//	_CallIndex = 0;
//	_pHaCall = 0;
//	strcpy(m_from,"b@polycom.com");
//	m_Ip = 22222222;
//	strcpy(m_event,"packageforme");
//	m_CallObj = (void*)0x45baaa;
//	m_callIndex = 3067020;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status 2", pSubResponse->status == SipCodesBadRequest);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong expires 2", pSubResponse->expires==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall 2", pSubResponse->pCallObj==m_pSubscribeMsg->pCallObj);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKandSameExpires1()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE("CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKandSameExpires 1 ", m_expires == pSubResponse->expires);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKandSameExpires2()
//{
//	m_expires = 900;
//
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE("CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKandSameExpires 2 ", m_expires == pSubResponse->expires);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKand3600Expires()
//{
//	m_expires = 4000;
//
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE("CTestSIPEventPackageDispatcher::testSubscribeHasRepliedWithOKand3600Expires ", 3600 == pSubResponse->expires);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeConfIsCollected()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	CPPUNIT_ASSERT_MESSAGE("testSubscribeIsCollected, party_1 was not found", m_pMgr->FindParty(m_from, 0, m_callId));
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribesAreCollected()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	char from_2[H243_NAME_LEN] = "";
//	strncpy(from_2, m_from, H243_NAME_LEN);
//	strncpy(m_from, "party_2@polycom.com", H243_NAME_LEN);
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	CPPUNIT_ASSERT_MESSAGE("testSubscribesAreCollected, party_2 was not found", m_pMgr->FindParty(from_2, 0, m_callId));
//	CPPUNIT_ASSERT_MESSAGE("testSubscribesAreCollected, party_1 was not found", m_pMgr->FindParty(m_from, 0, m_callId));
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testRefreshRepliedWithOK()
//{
//	m_expires = 4;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	m_expires = 3600;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status==SipCodesOk);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong expires ", pSubResponse->expires==3600);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall ", pSubResponse->pCallObj==m_pSubscribeMsg->pCallObj);
//	//******CPPUNIT_ASSERT_MESSAGE( "Wrong CallIndex ", _CallIndex==m_pSubscribeMsg->header.pmHeader.callIndex);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testUnsubscribeRepliedWithOK()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	m_expires = 0;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status==SipCodesOk);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong expires ", pSubResponse->expires==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall ", pSubResponse->pCallObj==m_pSubscribeMsg->pCallObj);
//	//*****CPPUNIT_ASSERT_MESSAGE( "Wrong CallIndex ", _CallIndex==m_pSubscribeMsg->header.pmHeader.callIndex);
//	PDELETE(pSubResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testUnsubscribe()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_expires = 0;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	char uri[MaxAddressListSize] = "";
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//	CPPUNIT_ASSERT_MESSAGE("testSubscribesAreCollected, party_1 was found", !m_pMgr->FindParty(m_from, 0, m_callId));
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testUnsubscribeForUnknownpartyIsOK()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_expires = 0;
//	strcpy(m_from, "party44@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	char uri[MaxAddressListSize] = "";
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status==SipCodesOk);
//	PDELETE(pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE("testSubscribesAreCollected, party44 was found", !m_pMgr->FindParty(m_from, 0, m_callId));
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testTerminateConfCallsAllConfSubscribersNotify2()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	m_pMgr->ResetNotifyTimer();
//	m_pMgr->TerminateConf();
//
//	DWORD ip, CSeq;
//	WORD port, transport;
//	char state[H243_NAME_LEN] = "";;
//	char event[H243_NAME_LEN] = "";;
//	char callId[H243_NAME_LEN] = "";;
//	char contentType[H243_NAME_LEN] = "";;
//	char content[H243_NAME_LEN] = "";;
//	char fromTag[H243_NAME_LEN] = "";;
//	char toTag[H243_NAME_LEN] = "";;
//	char uri[MaxAddressListSize] = "";
//
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
////	ParseNotifyEventPackReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)m_from);
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", m_Ip == ip);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", m_port == port);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", m_transport == transport);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 2);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"conference");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"234234fsghdfhg-3546");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/conference-info+xml");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"ert3465-346y");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"345345ge63");
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testTerminateConfCallsAllReferSubscribersNotify2()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId, 1);
//	m_pMgr->TerminateConf();
//
//	DWORD ip, CSeq;
//	WORD port, transport;
//	char state[H243_NAME_LEN] = "";;
//	char event[H243_NAME_LEN] = "";;
//	char callId[H243_NAME_LEN] = "";;
//	char contentType[H243_NAME_LEN] = "";;
//	char content[H243_NAME_LEN] = "";;
//	char fromTag[H243_NAME_LEN] = "";;
//	char toTag[H243_NAME_LEN] = "";
//	char uri[MaxAddressListSize] = "";
//
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
////	ParseNotifyEventPackReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)m_from);
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", m_Ip == ip);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", m_port == port);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", m_transport == transport);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 1);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"refer; id=7412");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"234234fsghdfhg-3546");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"message/sipfrag;version=2.0");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"ert3465-346y");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"345345ge63");
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testTerminateConfDeletesAllConfSubscribers()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	char _from[H243_NAME_LEN] = "";
//	strncpy(_from, m_from, H243_NAME_LEN);
//	strcpy(m_from,"party_2@polycom.com");
//	m_Ip = 22222222;
//	m_port = 4455;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	m_pMgr->TerminateConf();
//
//	CPPUNIT_ASSERT_MESSAGE("testTerminateConfDeletesAllSubscribers, party_1 was not deleted", !m_pMgr->FindParty(_from, 0, m_callId));
//	CPPUNIT_ASSERT_MESSAGE("testTerminateConfDeletesAllSubscribers, party_2 was not deleted", !m_pMgr->FindParty(m_from, 0, m_callId));
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testTerminateConfDeletesAllReferSubscribers()
//{
//	strcpy(m_event, "conference");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//
//	char _from[H243_NAME_LEN] = "";
//	strncpy(_from, m_from, H243_NAME_LEN);
//	strcpy(m_from,"party_2@polycom.com");
//	m_Ip = 22222222;
//	m_port = 4455;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	m_pMgr->TerminateConf();
//
//	CPPUNIT_ASSERT_MESSAGE("testTerminateConfDeletesAllSubscribers, party_1 was not deleted", !m_pMgr->FindParty(_from, 0, m_callId));
//	CPPUNIT_ASSERT_MESSAGE("testTerminateConfDeletesAllSubscribers, party_2 was not deleted", !m_pMgr->FindParty(m_from, 0, m_callId));
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testSubscribeRejectedAfterTerminateConf()
//{
//	m_pMgr->TerminateConf();
//
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	mcReqSubscribeResp* pSubResponse = NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(&pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pSubResponse->status==SipCodesNotAcceptable);
//	PDELETE(pSubResponse);
//}
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testTerminateConfCallsAllSubscribersNotify()
//{
//	strcpy(m_from,"party_2@polycom.com");
//	m_Ip = 22222222;
//	m_port = 4455;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, 22, 1);
//	m_pMgr->ResetNotifyTimer();
//	m_pMgr->TerminateConf();
//
//	DWORD ip, CSeq;
//	WORD port, transport;
//	char state[H243_NAME_LEN]= "";
//	char event[H243_NAME_LEN]= "";
//	char callId[H243_NAME_LEN]= "";
//	char contentType[H243_NAME_LEN]= "";
//	char content[H243_NAME_LEN]= "";
//	char fromTag[H243_NAME_LEN]= "";
//	char toTag[H243_NAME_LEN]= "";
//	char uri[MaxAddressListSize] = "";
//
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
////	ParseNotifyEventPackReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)m_from);
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", m_Ip == ip);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", m_port == port);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", m_transport == transport);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 2);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"conference");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"234234fsghdfhg-3546");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/conference-info+xml");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"ert3465-346y");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"345345ge63");
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testHasRepliedWithOKForRefer()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId, 1);
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", (pReferResponse->status==SipCodesOk || pReferResponse->status==SipCodesAccepted));
//	PDELETE(pReferResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testHasRepliedWithSameDstUnitIdAndCallIndexForRefer()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId, 1);
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//
//	WORD		csId = 0;
//	DWORD		serviceId = 0;
//	WORD		destUnitId = 0;
//	DWORD		callIndex = 0;
//	DWORD		channelIndex = 0;
//	DWORD		mcChannelIndex = 0;
//	APIS32		status = 0;
//	WORD		srcUnitId = 0;
//	APIS32		testing = -1;
//	APIU32		testing1 = 0;
//	m_pMockProtocol->Varify_AddCSHeader_WasCalled1(csId, srcUnitId,serviceId,destUnitId, callIndex, channelIndex, mcChannelIndex, status);
//	CPPUNIT_ASSERT_MESSAGE("wrong DstUnitId ", destUnitId == m_srcUnitId);
//	CPPUNIT_ASSERT_MESSAGE("wrong CallIndex ", callIndex == 15);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferRejectedDueToWrongAcceptHeader()
//{
//	strcpy(m_allow, "application/sdp");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId, 1);
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pReferResponse->status==SipCodesNotAcceptable);
//	PDELETE(pReferResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferRejectedSinceFromPartyIsNotInConf()
//{
//	strcpy(m_from,"party_2@polycom.com");
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId, 1);
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pReferResponse->status==SipCodesNotFound);
//	PDELETE(pReferResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferWithByeRejectedSinceReferToPartyIsNotInConf()
//{
//	strcpy(m_referTo, "carol@chicago.example.com;method=BYE");
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId, 1);
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", pReferResponse->status==SipCodesNotFound);
//	PDELETE(pReferResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferHasRepliedOKWithGoodParams()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId, 1);
//
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", (pReferResponse->status==SipCodesOk || pReferResponse->status==SipCodesAccepted));
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall ", pReferResponse->pCallObj==m_pReferMsg->pCallObj);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong CSeq ", pReferResponse->remoteCseq==m_pReferMsg->remoteCseq);
////****	CPPUNIT_ASSERT_MESSAGE( "Wrong CallIndex ", _CallIndex==m_pReferMsg->header.pmHeader.callIndex);
//	PDELETE(pReferResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::test2RefersRepliedOKWithGoodParams()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId, 1);
//
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status 1", (pReferResponse->status==SipCodesOk || pReferResponse->status==SipCodesAccepted));
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall 1", pReferResponse->pCallObj==m_pReferMsg->pCallObj);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong CSeq 1", pReferResponse->remoteCseq==m_pReferMsg->remoteCseq);
//	PDELETE(pReferResponse);
//
//	_status = 0;
//	_expires = 0;
//	_CallIndex = 0;
//	_pHaCall = 0;
//	strcpy(m_from,"b@polycom.com");
//
//	if (m_pConfDB)
//	{
//		CCommConf* pCommConf = m_pConfDB->GetCurrentConf("conf1");
//		if(pCommConf)
//		{
//			CConfParty* pConfParty = new CConfParty;
//			pConfParty->SetName("party_b");
//			pConfParty->SetSipPartyAddress("b@polycom.com");
//			pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//			pCommConf->Add(*pConfParty);
//		}
//	}
//
//	m_Ip = 22222222;
//	m_CallObj = (void*)0x45baaa;
//	m_callIndex = 303020;
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId, 1);
//
//	pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status 2", (pReferResponse->status==SipCodesOk || pReferResponse->status==SipCodesAccepted));
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall 2", pReferResponse->pCallObj==m_pReferMsg->pCallObj);
////*****	CPPUNIT_ASSERT_MESSAGE( "Wrong CallIndex 2", _CallIndex==m_pReferMsg->header.pmHeader.callIndex);
//	PDELETE(pReferResponse);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferWithByeHasRepliedOKWithGoodParams()
//{
//	strcpy(m_referTo, "carol@chicago.example.com;method=BYE");
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//
//	if (m_pConfDB)
//	{
//		CCommConf* pCommConf = m_pConfDB->GetCurrentConf("conf1");
//		if(pCommConf)
//		{
//			CConfParty* pConfParty = new CConfParty;
//			pConfParty->SetName("carol_(referred)");
//			pConfParty->SetSipPartyAddress("carol@chicago.example.com");
//			pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//			pCommConf->Add(*pConfParty);
//		}
//	}
//
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId, 1);
//
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", (pReferResponse->status==SipCodesOk || pReferResponse->status==SipCodesAccepted));
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall ", pReferResponse->pCallObj==m_pReferMsg->pCallObj);
////***	CPPUNIT_ASSERT_MESSAGE( "Wrong CallIndex ", _CallIndex==m_pReferMsg->header.pmHeader.callIndex);
//	PDELETE(pReferResponse);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForSip1()
//{
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "atara@ip.co.il";
//	char		resoult[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==SIP_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"atara@ip.co.il")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone==NO);
//	POBJDELETE(pEPM);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForSip2()
//{
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "sip:atara@ip.co.il;user=sip";
//	char		resoult[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==SIP_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"atara@ip.co.il")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone==NO);
//	POBJDELETE(pEPM);
//}
//
///**********
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForSip3()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(7);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(2);
//	const char* sipPrefix = ::GetpSystemCfg()->GetSipReferPrefix();
//	::GetpSystemCfg()->SetSipReferPrefix("12");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "125338930@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==SIP_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"5338930@ip.co.il")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone==NO);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferPrefix(sipPrefix);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForSip4()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(6);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(1);
//	const char* sipPrefix = ::GetpSystemCfg()->GetSipReferPrefix();
//	::GetpSystemCfg()->SetSipReferPrefix("1");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "11234567@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==NA);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferPrefix(sipPrefix);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForPstn8Dig()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(7);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(2);
//	const char* sipPrefix = ::GetpSystemCfg()->GetSipReferPrefix();
//	::GetpSystemCfg()->SetSipReferPrefix("12");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "12345678@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==NA);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferPrefix(sipPrefix);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForPstn7Dig()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(7);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(2);
//	const char* sipPrefix = ::GetpSystemCfg()->GetSipReferPrefix();
//	::GetpSystemCfg()->SetSipReferPrefix("12");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "1234567@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==ISDN_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"1234567")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferPrefix(sipPrefix);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForPstn6Dig()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(7);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(2);
//	const char* sipPrefix = ::GetpSystemCfg()->GetSipReferPrefix();
//	::GetpSystemCfg()->SetSipReferPrefix("12");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "123456@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==ISDN_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"123456")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferPrefix(sipPrefix);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForPstn10Dig()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(7);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(2);
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "1234567890@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==NA);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForIsdn9Dig()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(7);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(2);
//	const char* isdnPrefix = ::GetpSystemCfg()->GetSipReferIsdnPrefix();
//	::GetpSystemCfg()->SetSipReferIsdnPrefix("11");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "113456789@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==ISDN_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"3456789")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone==NO);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferIsdnPrefix(isdnPrefix);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForH3239Dig()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(7);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(2);
//	const char* h323Prefix = ::GetpSystemCfg()->GetSipReferH323Prefix();
//	::GetpSystemCfg()->SetSipReferH323Prefix("13");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "133456789@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==H323_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"3456789")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone==NO);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferH323Prefix(h323Prefix);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForH323()
//{
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "h323:atara@ip.co.il;user=h323";
//	char		resoult[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	BYTE interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==H323_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"atara")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone==NO);
//	POBJDELETE(pEPM);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForH323_2()
//{
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "h323:atara";
//	char		resoult[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	BYTE interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==H323_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"atara")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone==NO);
//	POBJDELETE(pEPM);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForTel()
//{
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "tel:1234";
//	char		resoult[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	BYTE interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==ISDN_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"1234")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone);
//	POBJDELETE(pEPM);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForUnknown()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(7);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(2);
//	const char* isdnPrefix = ::GetpSystemCfg()->GetSipReferIsdnPrefix();
//	::GetpSystemCfg()->SetSipReferIsdnPrefix("11");
//	const char* h323Prefix = ::GetpSystemCfg()->GetSipReferH323Prefix();
//	::GetpSystemCfg()->SetSipReferH323Prefix("13");
//	const char* sipPrefix = ::GetpSystemCfg()->GetSipReferPrefix();
//	::GetpSystemCfg()->SetSipReferPrefix("12");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "141234567@ip.co.il;user=phone";
//	char*		resoult = new char[MaxAddressListSize];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==NA);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferIsdnPrefix(isdnPrefix);
//	::GetpSystemCfg()->SetSipReferH323Prefix(h323Prefix);
//	::GetpSystemCfg()->SetSipReferPrefix(sipPrefix);
//}
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testParseAddressForPstn11Dig()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(11);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(1);
//	const char* isdnPrefix = ::GetpSystemCfg()->GetSipReferIsdnPrefix();
//	::GetpSystemCfg()->SetSipReferIsdnPrefix("2");
//	const char* h323Prefix = ::GetpSystemCfg()->GetSipReferH323Prefix();
//	::GetpSystemCfg()->SetSipReferH323Prefix("3");
//	const char* sipPrefix = ::GetpSystemCfg()->GetSipReferPrefix();
//	::GetpSystemCfg()->SetSipReferPrefix("1");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "90523932201@ip.co.il;user=phone";
//	char*		resoult = new char[IP_LIMIT_ADDRESS_CHAR_LEN];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==ISDN_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"90523932201")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferIsdnPrefix(isdnPrefix);
//	::GetpSystemCfg()->SetSipReferH323Prefix(h323Prefix);
//	::GetpSystemCfg()->SetSipReferPrefix(sipPrefix);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testCallBackToH323()
//{
//	int numOfDigInPhone = ::GetpSystemCfg()->GetSipNumOfDigitsInPhoneNumber();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(11);
//	int numOfDigInPrefix = ::GetpSystemCfg()->GetSipNumOfDigitsInPrefix();
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(1);
//	const char* isdnPrefix = ::GetpSystemCfg()->GetSipReferIsdnPrefix();
//	::GetpSystemCfg()->SetSipReferIsdnPrefix("2");
//	const char* h323Prefix = ::GetpSystemCfg()->GetSipReferH323Prefix();
//	::GetpSystemCfg()->SetSipReferH323Prefix("3");
//	const char* sipPrefix = ::GetpSystemCfg()->GetSipReferPrefix();
//	::GetpSystemCfg()->SetSipReferPrefix("1");
//
//	CSIPReferEventPackageManager* pEPM = new CSIPReferEventPackageManager;
//	const char* address = "22222222222@ip.co.il;user=h323";
//	char*		resoult = new char[IP_LIMIT_ADDRESS_CHAR_LEN];
//	BYTE		bIsTelephone;
//	int			interfaceType = pEPM->ParseAddress(address,resoult,bIsTelephone);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong interface type ", interfaceType==H323_INTERFACE_TYPE);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong address resoult ", strcmp(resoult,"22222222222")==0);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong boolean ", bIsTelephone==NO);
//	PDELETEA(resoult);
//	POBJDELETE(pEPM);
//
//	::GetpSystemCfg()->SetSipNumOfDigitsInPhoneNumber(numOfDigInPhone);
//	::GetpSystemCfg()->SetSipNumOfDigitsInPrefix(numOfDigInPrefix);
//	::GetpSystemCfg()->SetSipReferIsdnPrefix(isdnPrefix);
//	::GetpSystemCfg()->SetSipReferH323Prefix(h323Prefix);
//	::GetpSystemCfg()->SetSipReferPrefix(sipPrefix);
//}****/
//
///*
//void CTestSIPEventPackageDispatcher::testReferForSIP_CX_SubscribedParty()
//{
//	CCommConf* pCommConf = new CCommConf;
//	pCommConf->SetName("conf1");
//	::GetpConfDB()->Add(*pCommConf);
//
//	strcpy(m_event, "com.microsoft.sip.cx-conference-info");
//	strcpy(m_allow, "application/com.microsoft.sip.cx-conference-info+xml");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	mcReqSubscribeResp* pSubResponse=NULL;
//	m_pMockProtocol->Verify_SipSubscribeResponse_WasCalled(pSubResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong subscribe status ", pSubResponse->status==SipCodesOk);
//	PDELETE(pSubResponse);
//
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15);
//
//	mcReqReferResp* pReferResponse=NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong refer status ", pReferResponse->status==SipCodesOk);
//	PDELETE(pReferResponse);
//}*/
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferIsCollected()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId, 1);
//
//	CPPUNIT_ASSERT_MESSAGE("testSubscribeIsCollected, party_1 was not found", m_pMgr->FindParty(m_from, 0, m_callId));
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferSendNotify()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId, 1);
//
//	CSIPSubscriber* pSubscriber = m_pMgr->FindParty("party_1@polycom.com", 0, m_callId);
//	CSegment* pSeg = new CSegment;
//	*pSeg<< (DWORD)pSubscriber
//         << (WORD)PARTYSTATE
//		 << (DWORD)PARTY_CONNECTING;
//	m_pMgr->HandleObserverUpdate(pSeg, SIP_REFER);
//
//	DWORD ip, CSeq;
//	WORD port, transport;
//	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char callId[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[H243_NAME_LEN] = "";
//	char fromTag[H243_NAME_LEN] = "";
//	char toTag[H243_NAME_LEN] = "";
//	char uri[MaxAddressListSize] = "";
//
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
////	ParseNotifyEventPackReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"party_1@polycom.com");
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", ip == 5678900);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", port == 5060);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", transport == eTransportTypeTcp);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 1);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"refer; id=7412");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"234234fsghdfhg-3546");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active;expires=300");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"message/sipfrag;version=2.0");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"SIP/2.0 100 Trying");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"ert3465-346y");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"345345ge63");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferAddsSIPParty()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId, 1);
//
//	char uri[MaxAddressListSize] = "";
//	BYTE interfaceType = 0;
//	m_pMockConfPartyManagerApi->Verify_CreateDialOutParty_WasCalled(uri, interfaceType);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"carol@chicago.example.com");
//}
//
//////////////////////////////////////////////////////////////////////
///*void CTestSIPEventPackageDispatcher::testReferAddsTelParty()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	strcpy(m_referTo, "tel:9251289");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15);
//
//	char uri[MaxAddressListSize] = "";
//	BYTE interfaceType = SIP_INTERFACE_TYPE;
//	m_pMockConfPartyManagerApi->Verify_CreateDialOutParty_WasCalled(uri, interfaceType);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"9251289");
//	CPPUNIT_ASSERT_MESSAGE( "wrong interface ", interfaceType == ISDN_INTERFACE_TYPE);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageDispatcher::testReferAddsH323Party()
//{
//	strcpy(m_allow, "application/sdp, message/sipfrag");
//	strcpy(m_referTo, "h323:alias1");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15);
//
//	char uri[MaxAddressListSize] = "";
//	BYTE interfaceType = SIP_INTERFACE_TYPE;
//	m_pMockConfPartyManagerApi->Verify_CreateDialOutParty_WasCalled(uri, interfaceType);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"alias1");
//	CPPUNIT_ASSERT_MESSAGE( "wrong interface ", interfaceType == H323_INTERFACE_TYPE);
//}*/
//
////**************************************************************
////**************************************************************
//
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestSIPEventPackageManager );
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::setUp()
//{
//	m_pHeaderList = NULL;
//	m_pSubscribeMsg = NULL;
//	m_pMockProtocol = new CMockMplMcmsProtocol;
//	m_pMockConfApi = new CMockConfApi;
//	m_pMgr = new CSIPEventPackageManager(NULL, (CMplMcmsProtocol*)m_pMockProtocol, NULL, m_pMockConfApi);
//	strcpy(m_from,"party_1@polycom.com");
//	strcpy(m_fromTag,"2453245rwetr5");
//	strcpy(m_toTag,"235245-7856");
//	strcpy(m_callId, "3453465-rth645361354");
//	m_Ip = 111111111;
//	m_port = 5060;
//	m_transport = eTransportTypeTcp;
//	strcpy(m_event,"conference");
//	strcpy(m_allow, "application/conference-info+xml");
//	m_expires = 3600;
//	m_CallObj = (void*)0x0a0a5f5f;
//	//m_callIndex = 3456789;
//	m_pConfDB = ::GetpConfDB();
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::tearDown()
//{
//	POBJDELETE(m_pMgr);
//	PDELETE(m_pHeaderList);
//	PDELETE(m_pSubscribeMsg);
//	POBJDELETE(m_pMockProtocol);
//	POBJDELETE(m_pMockConfApi);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::BuildSubscribe()
//{
//	PDELETE(m_pHeaderList);
//	PDELETEA(m_pSubscribeMsg);
//
//	m_pHeaderList = new CSipHeaderList(MIN_ALLOC_HEADERS,0);
//	m_pHeaderList->AddHeader(kTo,   strlen("conf1@test"), "conf1@test");
//	m_pHeaderList->AddHeader(kToTag, strlen(m_toTag), m_toTag);
//	m_pHeaderList->AddHeader(kFrom, strlen(m_from), m_from);
//	m_pHeaderList->AddHeader(kFromTag, strlen(m_fromTag), m_fromTag);
//	m_pHeaderList->AddHeader(kEvent, strlen(m_event), m_event);
//	m_pHeaderList->AddHeader(kAccept, strlen(m_allow), m_allow);
//	m_pHeaderList->AddHeader(kCallId, strlen(m_callId), m_callId);
//	int len = m_pHeaderList->GetTotalLen();
//	m_pSubscribeMsg = (mcIndSubscribe*)new BYTE[sizeof(mcIndSubscribeBase)-sizeof(sipMessageHeadersBase)+len];
//	m_pSubscribeMsg->expires = m_expires;
//	m_pSubscribeMsg->transportAddress.transAddr.addr.v4.ip = m_Ip;
//	m_pSubscribeMsg->transportAddress.transAddr.port= m_port;
//	m_pSubscribeMsg->transportAddress.transAddr.transportType = m_transport;
//	m_pSubscribeMsg->pCallObj = m_CallObj;
//	//*****m_pSubscribeMsg->header.pmHeader.status = 0;
//	CCommConf* pCommConf = new CCommConf;
//	pCommConf->SetName("conf1");
//	if (m_pConfDB)
//		m_pConfDB->Add(*pCommConf);
//
//	char confName[20] = "conf1@test";
//	m_pMgr->SetConfName(confName);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testConstructor()
//{
//	CPPUNIT_ASSERT_MESSAGE( "CTestSIPEventPackageManager::testConstructor ", m_pMgr != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testNotifyWasCalledAfterTOUT()
//{
//	m_expires = 4;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	DWORD opcode;
//
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//
//	m_pMgr->ResetNotifyTimer();
//	m_pMgr->OnSubscribeTout(NULL);
//
//	DWORD ip, CSeq;
//	WORD port, transport;
//	char state[H243_NAME_LEN] = "";
//	char callId[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[H243_NAME_LEN] = "";
//	char fromTag[H243_NAME_LEN] = "";
//	char toTag[H243_NAME_LEN] = "";
//	char uri[MaxAddressListSize] = "";
//
//	//m_pMockProtocol->Verify_SipNotify_WasCalled(uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//	DWORD dataLen;
//	char* data = m_pMockProtocol->Varify_AddData_WasCalled1(dataLen);
//	mcReqNotify* pNotifyReq = (mcReqNotify *)new BYTE[dataLen];
//	memcpy(pNotifyReq, data, dataLen);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
////	ParseNotifyEventPackReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"party_1@polycom.com");
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", ip == 111111111);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", port == 5060);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", transport == eTransportTypeTcp);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 2);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"conference");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"3453465-rth645361354");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/conference-info+xml");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"235245-7856");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"2453245rwetr5");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testNotifyWasNotCalledWhenTOUTisBigger()
//{
//	m_expires = 5;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//
//	m_pMgr->OnSubscribeTout(NULL);
//	m_pMockProtocol->Verify_AddCommonHeader_WasNotCalled();
//}
//
//////////////////////////////////////////////////////////////////////
////1 second timer betweenNotifies
//void CTestSIPEventPackageManager::test2ndNotifyIsNotSendDueToTimer()
//{
//	m_expires = 5;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_SIG_SUBSCRIBE_RESP_REQ);
//}
//
//////////////////////////////////////////////////////////////////////
////If a notify waited for timer, it should know be sent
//void CTestSIPEventPackageManager::testWaitingNotifyIsSentAfterTimer()
//{
//	m_expires = 5;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//
//	strcpy(m_from, "party_2@polycom.com");
//	m_Ip=2222;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_SIG_SUBSCRIBE_RESP_REQ);
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//
//	DWORD ip, CSeq;
//	WORD port, transport;
//	char state[H243_NAME_LEN] = "";
//	char callId[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[H243_NAME_LEN] = "";
//	char fromTag[H243_NAME_LEN] = "";
//	char toTag[H243_NAME_LEN] = "";
//	char uri[MaxAddressListSize] = "";
//
//	DWORD dataLen;
//	char* data = m_pMockProtocol->Varify_AddData_WasCalled1(dataLen);
//	mcReqNotify* pNotifyReq = (mcReqNotify *)new BYTE[dataLen];
//	memcpy(pNotifyReq, data, dataLen);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
////	ParseNotifyEventPackReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"party_2@polycom.com");
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", ip == 2222);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", port == 5060);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", transport == eTransportTypeTcp);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 2);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"conference");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"3453465-rth645361354");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active;expires=5");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/conference-info+xml");
//	//CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"235245-7856");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"2453245rwetr5");
//}
//
//////////////////////////////////////////////////////////////////////
////If no notify is waiting for timer, it should not be sent
//void CTestSIPEventPackageManager::testOnNotifyTimerToutDoesNotSendNotify()
//{
//	m_expires = 5;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_AddCommonHeader_WasNotCalled();
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testSubscribesIsCollectedandDeletedAfterTout()
//{
//	m_expires = 4;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//	CPPUNIT_ASSERT_MESSAGE("testSubscribesIsCollectedandDeletedAfterTout, party_1 was not found", m_pMgr->FindParty("party_1@polycom.com"));
//
//	m_pMgr->OnSubscribeTout(NULL);
//	CPPUNIT_ASSERT_MESSAGE("testSubscribesIsCollectedandDeletedAfterTout, party_1 was found", !m_pMgr->FindParty("party_1@polycom.com"));
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testNotifyWasNotCalledAfterRefresh()
//{
//	m_expires = 4;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//
//	m_expires = 3600;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_SIG_SUBSCRIBE_RESP_REQ);
//
//	m_pMgr->OnSubscribeTout(NULL);
//
//	char uri[MaxAddressListSize] = "";
//	m_pMockProtocol->Verify_AddCommonHeader_WasNotCalled();
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testDump()
//{
//	char test1[H243_NAME_LEN*2] = "CSIPEventPackageManager::Dump type = conference\n-----------------------------------\nparty_1@polycom.com\nb@polycom.com\n";
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	strcpy(m_from,"b@polycom.com");
//	m_Ip = 22222222;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
////*********	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong string returned", (std::string)m_pMgr->Dump(), (std::string)test);
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testEventPackageCalcShortesTimer1()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	strcpy(m_from,"party_2@polycom.com");
//	m_Ip = 22222222;
//	m_expires = 1800;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	CPPUNIT_ASSERT_MESSAGE("testEventPackageCalcShortesTimer1 was set to wrong time.", 1800 == m_pMgr->CalcTimer());
//
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testEventPackageCalcShortesTimer2()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	strcpy(m_from,"party_2@polycom.com");
//	m_Ip = 22222222;
//	m_expires = 900;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	CPPUNIT_ASSERT_MESSAGE("CTestSIPEventPackageManager::testEventPackageCalcShortesTimer2 was set to wrong time.", 900 == m_pMgr->CalcTimer());
//
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testEventPackageCalcShortesTimer3()
//{
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	strcpy(m_from,"party_2@polycom.com");
//	m_Ip = 22222222;
//	m_expires = 900;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	strcpy(m_from,"party_3@polycom.com");
//	m_Ip = 33333333;
//	m_expires = 902;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15, eParticipant, 1);
//
//	CPPUNIT_ASSERT_MESSAGE("CTestSIPEventPackageManager::testEventPackageCalcShortesTimer3 was set to wrong time.", 900 == m_pMgr->CalcTimer());
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPEventPackageManager::testEventPackageCalcShortesTimer4()
//{
//	CPPUNIT_ASSERT_MESSAGE("CTestSIPEventPackageManager::testEventPackageCalcShortesTimer4 was set to wrong time.", 0 == m_pMgr->CalcTimer());
//}
//
//
//// ----------------************************************-------------------
//
////CPPUNIT_TEST_SUITE_REGISTRATION( CTestSIPCXPackageManager );
///*
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::setUp()
//{
//	CCommConf* pCommConf = new CCommConf;
//	pCommConf->SetName("conf1");
//
//	::GetpConfDB()->Add(*pCommConf);
//
//	m_pMockProtocol = new CMockMplMcmsProtocol;
//	m_pMockConfApi = new CMockConfApi;
//	m_pMgr = new CSIPCXPackageManager(NULL, (CMplMcmsProtocol*)m_pMockProtocol, NULL, m_pMockConfApi);
//	strcpy(m_from,"party_1@polycom.com");
//	strcpy(m_event,"com.microsoft.sip.cx-conference-info");
//	strcpy(m_allow, "application/com.microsoft.sip.cx-conference-info+xml");
//	m_expires = 3600;
//	m_watcherId = 1345;
//	m_CallObj = (void*)0x0a0a5f5f;
//	m_callIndex = 3456789;
//	m_pHeaderList = NULL;
//	m_pSubscribeMsg = NULL;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::tearDown()
//{
//	POBJDELETE(m_pMgr);
//	PDELETE(m_pHeaderList);
//	PDELETEA(m_pSubscribeMsg);
//	POBJDELETE(m_pMockProtocol);
//	POBJDELETE(m_pMockConfApi);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::BuildSubscribe()
//{
//	PDELETE(m_pHeaderList);
//	PDELETEA(m_pSubscribeMsg);
//
//	m_pHeaderList = new CSipHeaderList(MIN_ALLOC_HEADERS,0);
//	m_pHeaderList->AddHeader(kTo,   strlen("conf1@test"), "conf1@test");
//	m_pHeaderList->AddHeader(kFrom, strlen(m_from), m_from);
//	m_pHeaderList->AddHeader(kEvent, strlen(m_event), m_event);
//	m_pHeaderList->AddHeader(kAccept, strlen(m_allow), m_allow);
//		int len = m_pHeaderList->GetTotalLen();
//	m_pSubscribeMsg = (mcIndSubscribe*)new BYTE[sizeof(mcIndSubscribeBase)-sizeof(sipMessageHeadersBase)+len];
//	m_pSubscribeMsg->expires = m_expires;
////******	m_pSubscribeMsg->WatcherId = m_watcherId;
//	m_pSubscribeMsg->pCallObj = m_CallObj;
//	m_pSubscribeMsg->header.pmHeader.callIndex = m_callIndex;
//	m_pSubscribeMsg->header.pmHeader.status = 0;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testConstructor()
//{
//	CPPUNIT_ASSERT_MESSAGE( "CTestSIPEventPackageManager::testConstructor ", m_pMgr != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyBody()
//{
//	m_expires = 4;
//	m_pMgr->SetConfName("conf1@polycom.com");
//
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];
//	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active;expires=4");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "com.microsoft.sip.cx-conference-info") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyFullForSubscribe()
//{
//	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];
//	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active;expires=3600");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"full\"") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyWasCalledAfterTOUT()
//{
//	m_expires = 4;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->ResetNotifyTimer();
//	m_pMgr->OnSubscribeTout(NULL);
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];
//	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyWasCalledAfterTOUT_AllSubs()
//{
//	m_expires = 4;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//	m_pMgr->ResetNotifyTimer();
//
//	//subscribe 2nd party
//	strcpy(m_from,"party_3@polycom.com");
//	m_expires = 902;
//	m_watcherId = 2000;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->ResetNotifyTimer();
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	m_pMgr->OnSubscribeTout(NULL);
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];
//	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 2);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[1] == 2000);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyWasCalledAfterTOUT_AllBoards()
//{
//	m_expires = 4;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 7);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//	m_pMgr->ResetNotifyTimer();
//
//	//subscribe 2nd party
//	strcpy(m_from,"party_3@polycom.com");
//	m_expires = 902;
//	m_watcherId = 2000;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 2);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->ResetNotifyTimer();
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	m_pMgr->OnSubscribeTout(NULL);
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];
//	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
////If no notify is waiting for timer, it should not be sent
//void CTestSIPCXPackageManager::testOnNotifyTimerToutDoesNotSendNotify()
//{
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//}
//
//////////////////////////////////////////////////////////////////////
////If notify is waiting for timer, it should be sent
//void CTestSIPCXPackageManager::testOnNotifyTimerToutSendNotify()
//{
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->ObserverUpdate(NULL, CONF_LOCK, TRUE);
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMgr->OnLoadManagerAccept();
//
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//}
//
//////////////////////////////////////////////////////////////////////
////Full Notify is sent even if timer is on
//void CTestSIPCXPackageManager::testNotifyWasCalledForFullDataEvenInTimer()
//{
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->ObserverUpdate(NULL, CONF_LOCK, TRUE);
//
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active;expires=5");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"full\"") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifySentAfterSubRefresh()
//{
//	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//	if(pCommConf)
//	{
//		CConfParty* pConfParty = new CConfParty;
//		pConfParty->SetName("party_b");
//		pConfParty->SetPartyId(1);
//		pConfParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);
//		pConfParty->SetSipPartyAddress("party_b@polycom.com");
//		pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//		pCommConf->Add(*pConfParty);
//
//		CConfParty* pConfParty2 = new CConfParty;
//		pConfParty2->SetName("party_2");
//		pConfParty2->SetPartyId(2);
//		pConfParty2->SetNetInterfaceType(SIP_INTERFACE_TYPE);
//		pConfParty2->SetSipPartyAddress("party_2@polycom.com");
//		pConfParty2->SetUndefinedType(UNDEFINED_PARTY);
//		pCommConf->Add(*pConfParty2);
//	}
//
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active;expires=5");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"full\"") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyWithDeltaIsSentAfterLock()
//{
//	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	m_pMgr->ObserverUpdate(NULL, CONF_LOCK, TRUE);
//	m_pMgr->OnLoadManagerAccept();
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"partial\"") != NULL);
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "version=\"2\"") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyWithDeltaIsSentAfterPartyDeleted()
//{
//	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_b");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);
//	pConfParty->SetSipPartyAddress("party_b@polycom.com");
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	m_pMgr->ObserverUpdate(NULL, PARTY_DELETED, 1);
//	m_pMgr->OnLoadManagerAccept();
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"partial\"") != NULL);
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "version=\"2\"") != NULL);
//
//	POBJDELETE(pConfParty);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyWithDeltaIsSentToAllSubscribersInBoard()
//{
//	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	//subscribe 2nd party
//	strcpy(m_from,"party_3@polycom.com");
//	m_expires = 902;
//	m_watcherId = 2000;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//
//	m_pMgr->ObserverUpdate(NULL, CONF_LOCK, TRUE);
//	m_pMgr->OnLoadManagerAccept();
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 2);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[1] == 2000);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"partial\"") != NULL);
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "version=\"2\"") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyWithDeltaIsSentToAllSubscribersInAllBoards()
//{
//	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 6);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	//subscribe 2nd party
//	strcpy(m_from,"party_3@polycom.com");
//	m_expires = 902;
//	m_watcherId = 2000;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 7);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//
//	m_pMgr->ObserverUpdate(NULL, CONF_LOCK, TRUE);
//	m_pMgr->OnLoadManagerAccept();
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"partial\"") != NULL);
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "version=\"2\"") != NULL);
//
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 2000);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"partial\"") != NULL);
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "version=\"2\"") != NULL);
//
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testNotifyWithDeltaIsNotSentToUnSubscribed()
//{
//		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	//subscribe 2nd party
//	strcpy(m_from,"party_3@polycom.com");
//	m_expires = 902;
//	m_watcherId = 2000;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMgr->OnLoadManagerAccept();
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	//unsubscribe first party
//	strcpy(m_from,"party_1@polycom.com");
//	m_expires = 0;
//	m_watcherId = 1345;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//
//	m_pMgr->ObserverUpdate(NULL, CONF_LOCK, TRUE);
//	m_pMgr->OnLoadManagerAccept();
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 2000);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "state=\"partial\"") != NULL);
//	CPPUNIT_ASSERT_MESSAGE("wrong content", strstr(content, "version=\"2\"") != NULL);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testTerminateConfCallsAllConfSubscribersNotify()
//{
//		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	//subscribe 2nd party
//	strcpy(m_from,"party_3@polycom.com");
//	m_expires = 902;
//	m_watcherId = 2000;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 15);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	m_pMgr->TerminateConf();
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 2);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[1] == 2000);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPCXPackageManager::testTerminateConfCallsAllConfSubscribersNotifyInAllBoards()
//{
//	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf("conf1");
//
//	m_expires = 5;
//	m_pMgr->SetConfName("conf1@polycom.com");
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 2);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	CSegment seg;
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	//subscribe 2nd party
//	strcpy(m_from,"party_3@polycom.com");
//	m_expires = 902;
//	m_watcherId = 2000;
//	BuildSubscribe();
//	m_pHeaderList->BuildMessage(&m_pSubscribeMsg->sipHeaders);
//	m_pMgr->Subscribe(m_pSubscribeMsg, 7);
//	m_pMockProtocol->Verify_SipNotify_WasCalled();
//
//	m_pMgr->OnNotifyTimerTout(&seg);
//	m_pMockProtocol->Verify_SipNotify_WasNotCalled();
//
//	m_pMgr->TerminateConf();
//
//	BYTE isCompressed = FALSE;
//	WORD watchersCount = 0;
//	WORD watcherArr[32];	char state[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[1024] = "";
//
//	m_pMockProtocol->Verify_SipDistrNotify_WasCalled(isCompressed, watchersCount, watcherArr, state, event, contentType, content);
//
//	CPPUNIT_ASSERT_MESSAGE("wrong compress flag", isCompressed == FALSE);
//	CPPUNIT_ASSERT_MESSAGE("wrong watchers count", watchersCount == 1);
//	CPPUNIT_ASSERT_MESSAGE("wrong watcher id", watcherArr[0] == 1345);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"com.microsoft.sip.cx-conference-info");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"application/com.microsoft.sip.cx-conference-info+xml");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"");
//}
//*/
