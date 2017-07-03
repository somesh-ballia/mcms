// // Boris G remove this tests (tests not relevant).

//#include "TestSipRefer.h"
//#include "SystemFunctions.h"
//#include "IpCsOpcodes.h"
//
//#include "ConfPartyManager.h"
//
//extern void ParseNotifyReq(mcReqNotify* pNotifyReq, char* pUserUri, DWORD *ip, WORD *port, WORD *transport, char* pEvent, char* pState,
//					char* pContentType, char*pContent, DWORD *CSeq, char* pCallId, char* pFromTag, char* pToTag);
//
//
//
//
//
//// ************************************************************************************
////
////	CTestSIPREFERSubscriber
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestSIPREFERSubscriber );
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::setUp()
//{
//	strcpy(m_from,"party_1@polycom.com");
//	strcpy(m_fromTag, "24234sdfsdf54");
//	strcpy(m_to,"conf1@polycom.com");
//	strcpy(m_toTag, "q2431243-678");
//	strcpy(m_referTo, "");
//	strcpy(m_callId, "123455hhgf-66547gfsedf");
//	m_Ip = 5678900;
//	m_port = 5060;
//	m_CSeq = 5621;
//	m_transport = eTransportTypeTcp;
//	m_expires = 3600;
//	m_srcUnitId = 100;
//
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::tearDown()
//{
//	//test
//	POBJDELETE(m_pSubscriber);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::testConstructor()
//{
//	m_pSubscriber = new CSIPREFERSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_CSeq, m_referTo, FALSE, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, eParticipant, 1227);
//	CPPUNIT_ASSERT_MESSAGE( "CTestSIPREFERSubscriber::testConstructor ", m_pSubscriber != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::testGetReferTo()
//{
//	strcpy(m_referTo, "new@polycom.com");
//	m_pSubscriber = new CSIPREFERSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_CSeq, m_referTo, FALSE, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, eParticipant, 1227);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong ReferTo returned", (std::string)m_referTo, (std::string)m_pSubscriber->GetReferTo());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::testGetReferTo2()
//{
//	strcpy(m_referTo, "new2@polycom.com");
//	m_pSubscriber = new CSIPREFERSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_CSeq, m_referTo, FALSE, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, eParticipant, 1227);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong ReferTo returned", (std::string)m_referTo, (std::string)m_pSubscriber->GetReferTo());
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::testIsReferWithBye()
//{
//	strcpy(m_referTo, "new@polycom.com");
//	m_pSubscriber = new CSIPREFERSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_CSeq, m_referTo, TRUE, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, eParticipant, 1227);
//	CPPUNIT_ASSERT_MESSAGE("wrong Refer with Bye", m_pSubscriber->IsReferWithBye());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::testIsReferWithBye2()
//{
//	strcpy(m_referTo, "new@polycom.com");
//	m_pSubscriber = new CSIPREFERSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_CSeq, m_referTo, FALSE, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, eParticipant, 1227);
//	CPPUNIT_ASSERT_MESSAGE("wrong Refer with Bye", !m_pSubscriber->IsReferWithBye());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::testGetCSeq()
//{
//	m_pSubscriber = new CSIPREFERSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_CSeq, m_referTo, FALSE, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, eParticipant, 1227);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", 5621 == m_pSubscriber->GetCSeq());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPREFERSubscriber::testGetCSeq2()
//{
//	m_CSeq = 234324;
//	m_pSubscriber = new CSIPREFERSubscriber(1, m_from, m_fromTag, m_to, m_toTag, m_callId, m_CSeq, m_referTo, FALSE, m_Ip, m_port, m_transport, m_expires, m_srcUnitId, eParticipant, 1227);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", 234324 == m_pSubscriber->GetCSeq());
//}
//
//// ************************************************************************************
////
////	CTestSIPReferEventPackageManager
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestSIPReferEventPackageManager );
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::setUp()
//{
//	m_pHeaderList = NULL;
//	m_pReferMsg = NULL;
//	m_pMockProtocol = new CMockMplMcmsProtocol;
//	m_pMockConfApi = new CMockConfApi;
//	m_pMockConfPartyMgrApi = new CMockConfPartyManagerLocalApi;
//	m_pMgr = new CSIPReferEventPackageManager(NULL, NULL, (CMplMcmsProtocol*)m_pMockProtocol, m_pMockConfPartyMgrApi, m_pMockConfApi);
//	m_pMgr->SetConfName("conf1");
//
//	strcpy(m_from,"party_1@polycom.com");
//	strcpy(m_fromTag, "346543ewr");
//	strcpy(m_toTag, "dgf5323425");
//	strcpy(m_callId, "4562456-15464256adf6543");
//	strcpy(m_referTo, "carol@chicago.example.com");
//	m_Ip = 111111111;
//	m_port = 5060;
//	m_transport = eTransportTypeTcp;
//	strcpy(m_event,"refer");
//	strcpy(m_allow, "message/sipfrag");
//	m_expires = 3600;
//	m_haCall = (void*)0x0a0a5f5f;
//	m_callIndex = 3456789;
//	m_CSeq = 8712;
//	m_srcUnitId = 100;
//
//	m_pCommConf = new CCommConf;
//	m_pCommConf->SetName("conf1");
//	m_pCommConf->SetRcvMbx(NULL);
//	m_pConfParty = new CConfParty;
//	m_pConfParty->SetName("party_1");
//	m_pConfParty->SetSipPartyAddress("party_1@polycom.com");
//	m_pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	m_pCommConf->Add(*m_pConfParty);
//	m_pConfDB = ::GetpConfDB();
//	if (m_pConfDB)
//		m_pConfDB->Add(*m_pCommConf);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::tearDown()
//{
//	if (m_pConfDB)
//		m_pConfDB->Cancel(m_pCommConf->GetName());
//	POBJDELETE(m_pMgr);
//	PDELETE(m_pHeaderList);
//	PDELETEA(m_pReferMsg);
//	POBJDELETE(m_pMockProtocol);
//	POBJDELETE(m_pMockConfApi);
//	POBJDELETE(m_pConfParty);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::BuildRefer()
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
//	m_pHeaderList->AddHeader(kEvent, strlen(m_event), m_event);
//	m_pHeaderList->AddHeader(kAccept, strlen(m_allow), m_allow);
//	m_pHeaderList->AddHeader(kReferTo, strlen(m_referTo), m_referTo);
//	int len = m_pHeaderList->GetTotalLen();
//	m_pReferMsg = (mcIndRefer*)new BYTE[sizeof(mcIndReferBase)-sizeof(sipMessageHeadersBase)+len];
//	m_pReferMsg->transportAddress.transAddr.addr.v4.ip = m_Ip;
//	m_pReferMsg->transportAddress.transAddr.port = m_port;
//	m_pReferMsg->transportAddress.transAddr.transportType = m_transport;
//	m_pReferMsg->remoteCseq = m_CSeq;
//	m_pReferMsg->pCallObj = m_haCall;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testConstructor()
//{
//	CPPUNIT_ASSERT_MESSAGE( "CTestSIPReferEventPackageManager::testConstructor ", m_pMgr != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testNotifyWasCalledAfterTOUT()
//{
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId);
//
//	mcReqReferResp* pReferResponse = NULL;
//	m_pMockProtocol->Verify_SipReferResponse_WasCalled(&pReferResponse);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong status ", (pReferResponse->status==SipCodesOk || pReferResponse->status==SipCodesAccepted));
//	CPPUNIT_ASSERT_MESSAGE( "Wrong haCall ", pReferResponse->pCallObj == m_pReferMsg->pCallObj);
//	CPPUNIT_ASSERT_MESSAGE( "Wrong Cseq ", pReferResponse->remoteCseq == m_pReferMsg->remoteCseq);
//	PDELETEA(pReferResponse);
//
//	m_pMgr->ExpireTime();
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
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"party_1@polycom.com");
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", ip == 111111111);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", port == 5060);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", transport == eTransportTypeTcp);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 1);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"refer; id=8712");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"4562456-15464256adf6543");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"message/sipfrag;version=2.0");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"SIP/2.0 503 Service Unavailable");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"dgf5323425");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"346543ewr");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testNotifyWasNotCalledWhenTOUTisBigger()
//{
//	m_expires = 5;
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_SIG_REFER_RESP_REQ);
//
//	m_pMgr->OnSubscribeTout(NULL);
//	m_pMockProtocol->Verify_AddCommonHeader_WasNotCalled();
//}
//
//////////////////////////////////////////////////////////////////////
////1 second timer betweenNotifies
//void CTestSIPReferEventPackageManager::test2ndNotifyIsNotSendDueToTimer()
//{
//	m_expires = 5;
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_SIG_REFER_RESP_REQ);
//
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId);
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode != SIP_CS_PROXY_NOTIFY_REQ);
//}
//
//////////////////////////////////////////////////////////////////////
////If a notify waited for timer, it should know be sent
//void CTestSIPReferEventPackageManager::testWaitingNotifyIsSentAfterTimer()
//{
//	m_expires = 5;
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId);
//
//	CSIPSubscriber* pSubscriber = m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId);
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_CONNECTING);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//
//	//this one is waiting for timer
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_CONNECTED);
//	m_pMockProtocol->Verify_AddCommonHeader_WasNotCalled();
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
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"party_1@polycom.com");
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", ip == 111111111);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", port == 5060);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", transport == eTransportTypeTcp);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 2);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"refer; id=8712");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"4562456-15464256adf6543");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"message/sipfrag;version=2.0");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"SIP/2.0 200 OK");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"dgf5323425");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"346543ewr");
//}
//
//////////////////////////////////////////////////////////////////////
////If no notify is waiting for timer, it should not be sent
//void CTestSIPReferEventPackageManager::testOnNotifyTimerToutDoesNotSendNotify()
//{
//	m_expires = 5;
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, (DWORD)15, m_srcUnitId);
//
//	CSIPSubscriber* pSubscriber = m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId);
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_CONNECTING);
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
//void CTestSIPReferEventPackageManager::testNotifyHasReferTags()
//{
//	strcpy(m_fromTag, "234234-dfg5-234");
//	strcpy(m_toTag, "34653465sgfh");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	CSIPSubscriber* pSubscriber = m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId);
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_CONNECTING);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
//
//	//this one is waiting for timer
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_CONNECTED);
//	m_pMockProtocol->Verify_AddCommonHeader_WasNotCalled();
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
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"party_1@polycom.com");
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", ip == 111111111);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", port == 5060);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", transport == eTransportTypeTcp);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 2);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"refer; id=8712");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"4562456-15464256adf6543");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"message/sipfrag;version=2.0");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"SIP/2.0 200 OK");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"34653465sgfh");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"234234-dfg5-234");
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testNotifyUsesDstUnitId()
//{
//	strcpy(m_fromTag, "234234-dfg5-234");
//	strcpy(m_toTag, "34653465sgfh");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	CSIPSubscriber* pSubscriber = m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId);
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_CONNECTING);
//
//	DWORD opcode;
//	m_pMockProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
//	CPPUNIT_ASSERT_MESSAGE("wrong opcode", opcode == SIP_CS_PROXY_NOTIFY_REQ);
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
//
//	CPPUNIT_ASSERT_MESSAGE("wrong dstUnitId ", destUnitId == m_srcUnitId);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testReferIsCollectedandDeletedAfterTout()
//{
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//	CPPUNIT_ASSERT_MESSAGE("testReferIsCollectedandDeletedAfterTout, party_1 was not found", m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId));
//
//	m_pMgr->ExpireTime();
//	m_pMgr->OnSubscribeTout(NULL);
//	CPPUNIT_ASSERT_MESSAGE("testReferIsCollectedandDeletedAfterTout, party_1 was found", !m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId));
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testReferWithIPonlyIsCollected()
//{
//	strcpy(m_referTo, "172.22.100.5");
//	m_expires = 4;
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//	CPPUNIT_ASSERT_MESSAGE("testReferWithIPonlyIsCollected, party_1 was not found", m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId));
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testReferIsRejectedSinceRefPartyIsAlreadyConnected()
//{
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//	CPPUNIT_ASSERT_MESSAGE("testReferIsRejectedSinceRefPartyIsAlreadyConnected, party_1 was not found", m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId));
//
//	//add referred party
//	CConfParty* pRefParty = new CConfParty;
//	pRefParty->SetName("carol");
//	pRefParty->SetSipPartyAddress("carol@chicago.example.com");
//	pRefParty->SetUndefinedType(UNDEFINED_PARTY);
//	pRefParty->SetConnectionType(DIAL_OUT);
//	pRefParty->SetPartyState(PARTY_CONNECTED);
//	m_pCommConf->Add(*pRefParty);
//
//	//add 2nd source
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_2");
//	pConfParty->SetSipPartyAddress("party_2@polycom.com");
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	m_pCommConf->Add(*pConfParty);
//
//	strcpy(m_from, "party_2@polycom.com");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	CPPUNIT_ASSERT_MESSAGE("testReferIsRejectedSinceRefPartyIsAlreadyConnected, party_2 was found", !m_pMgr->FindParty("party_2@polycom.com", m_CSeq, m_callId));
//	m_pCommConf->Cancel("party_2");
//	m_pCommConf->Cancel("carol");
//	POBJDELETE(pConfParty);
//	POBJDELETE(pRefParty);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testReferIsAcceptedSinceRefPartyIsDisonnected()
//{
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//	CPPUNIT_ASSERT_MESSAGE("testReferIsRejectedSinceRefPartyIsAlreadyConnected, party_1 was not found", m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId));
//
//	//add referred party
//	CConfParty* pRefParty = new CConfParty;
//	pRefParty->SetName("carol");
//	pRefParty->SetSipPartyAddress("carol@chicago.example.com");
//	pRefParty->SetUndefinedType(UNDEFINED_PARTY);
//	pRefParty->SetConnectionType(DIAL_OUT);
//	pRefParty->SetPartyState(PARTY_DISCONNECTED);
//	m_pCommConf->Add(*pRefParty);
//
//	//add 2nd source
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_2");
//	pConfParty->SetSipPartyAddress("party_2@polycom.com");
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	m_pCommConf->Add(*pConfParty);
//
//	strcpy(m_from, "party_2@polycom.com");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	CPPUNIT_ASSERT_MESSAGE("testReferIsRejectedSinceRefPartyIsAlreadyConnected, party_2 was not found", m_pMgr->FindParty("party_2@polycom.com", m_CSeq, m_callId));
//	m_pCommConf->Cancel("party_2");
//	m_pCommConf->Cancel("carol");
//	POBJDELETE(pConfParty);
//	POBJDELETE(pRefParty);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testReferWithByeIsRejectedSinceRefPartyIsNotFound()
//{
//	strcpy(m_referTo, "carol@chicago.example.com;method=BYE");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//	CPPUNIT_ASSERT_MESSAGE("testReferIsRejectedSincePartyIsAlreadyConnected, party_1 was found", !m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId));
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testDump()
//{
//	char test1[H243_NAME_LEN*2] = "CSIPReferEventPackageManager::Dump type = refer\n-----------------------------------\nparty_1@polycom.com\nb@polycom.com\n";
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	strcpy(m_from,"b@polycom.com");
//	m_Ip = 22222222;
//	m_pConfParty = new CConfParty;
//	m_pConfParty->SetName("b");
//	m_pConfParty->SetSipPartyAddress("b@polycom.com");
//	m_pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	m_pCommConf->Add(*m_pConfParty);
//
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
////	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong string returned", (std::string)m_pMgr->Dump(), (std::string)test);
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testCheckReferWithBye1()
//{
//	strcpy(m_referTo, "carol@chicago.example.com;method=BYE");
//	BuildRefer();
//	char* temp = new char[MaxAddressListSize];
//	CPPUNIT_ASSERT_MESSAGE("Refer was not recognized as refer with bye", m_pMgr->CheckIfReferWithBye(m_pHeaderList, temp));
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("function did not return the referTo string", (std::string)"carol@chicago.example.com", (std::string)temp);
//	delete[] temp;
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testCheckReferWithBye2()
//{
//	BuildRefer();
//	char* temp = new char[MaxAddressListSize];
//	CPPUNIT_ASSERT_MESSAGE("Refer was recognized as refer with bye", !m_pMgr->CheckIfReferWithBye(m_pHeaderList, temp));
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("function did not return the referTo string", (std::string)"carol@chicago.example.com", (std::string)temp);
//	delete[] temp;
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testCheckReferWithBye3()
//{
//	strcpy(m_referTo, "carol@chicago.example.com;method");
//	BuildRefer();
//	char* temp = new char[MaxAddressListSize];
//	CPPUNIT_ASSERT_MESSAGE("Refer was recognized as refer with bye", !m_pMgr->CheckIfReferWithBye(m_pHeaderList, temp));
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("function did not return the referTo string", (std::string)"carol@chicago.example.com", (std::string)temp);
//	delete[] temp;
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testCheckReferWithBye4()
//{
//	strcpy(m_referTo, "");
//	BuildRefer();
//	char* temp = new char[MaxAddressListSize];
//	CPPUNIT_ASSERT_MESSAGE("Refer was recognized as refer with bye", !m_pMgr->CheckIfReferWithBye(m_pHeaderList, temp));
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("function did not return the referTo string", (std::string)"", (std::string)temp);
//	delete[] temp;
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testOnConnectReferredPartyTout()
//{
//	if (!m_pConfDB)
//		return;
//
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//
//
//	CCommConf* pCommConf = m_pConfDB->GetCurrentConf("conf1");
//	CConfParty* pRefParty = new CConfParty;
//	pRefParty->SetName("carol");
//	pRefParty->SetSipPartyAddress("carol@polycom.com");
//	pRefParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pRefParty);
//
//	char* pUserName = new char[H243_NAME_LEN];
//	CSegment* pParam = NULL;
//	m_pMgr->OnConnectReferredPartyTout(pParam);
//	m_pMockConfApi->Verify_ReconnectParty_WasCalled(pUserName);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("ReconnectParty was called with wrong partyName", (std::string)"carol", (std::string)pUserName);
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testOnConnectReferredPartyTout_referBye()
//{
//	if (!m_pConfDB)
//		return;
//
//	CCommConf* pCommConf = m_pConfDB->GetCurrentConf("conf1");
//	CConfParty* pRefParty = new CConfParty;
//	pRefParty->SetName("carol");
//	pRefParty->SetSipPartyAddress("carol@chicago.example.com");
//	pRefParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pRefParty);
//
//	strcpy(m_referTo, "carol@chicago.example.com;method=BYE");
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	char* pUserName = new char[H243_NAME_LEN];
//	WORD discCause = 0;
//	CSegment* pParam = NULL;
//	m_pMgr->OnConnectReferredPartyTout(pParam);
//	m_pMockConfApi->Verify_DropParty_WasCalled(pUserName, &discCause);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("DropParty was called with wrong partyName", (std::string)"carol", (std::string)pUserName);
//	CPPUNIT_ASSERT_MESSAGE("Wrong disconnection cause", DISCONNECTED_BY_CHAIR == discCause);
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testObserverUpdate()
//{
//	m_CSeq = 8787;
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	CSIPSubscriber* pSubscriber = m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId);
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_CONNECTED);
//
//	DWORD ip, CSeq;
//	WORD port, transport;
//	char state[H243_NAME_LEN] = "";
//	char callId[H243_NAME_LEN] = "";
//	char event[H243_NAME_LEN] = "";
//	char contentType[H243_NAME_LEN] = "";
//	char content[H243_NAME_LEN] = "";
//	char toTag[H243_NAME_LEN] = "";
//	char fromTag[H243_NAME_LEN] = "";
//	char uri[MaxAddressListSize] = "";
//
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong uri ", (std::string)uri, (std::string)"party_1@polycom.com");
//	CPPUNIT_ASSERT_MESSAGE("wrong ip", ip == 111111111);
//	CPPUNIT_ASSERT_MESSAGE("wrong port", port == 5060);
//	CPPUNIT_ASSERT_MESSAGE("wrong transport", transport == eTransportTypeTcp);
//	CPPUNIT_ASSERT_MESSAGE("wrong CSeq", CSeq == 1);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong event", (std::string)event, (std::string)"refer; id=8787");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong CallId", (std::string)callId, (std::string)"4562456-15464256adf6543");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong contentType", (std::string)contentType, (std::string)"message/sipfrag;version=2.0");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"SIP/2.0 200 OK");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong fromTag", (std::string)fromTag, (std::string)"dgf5323425");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong toTag", (std::string)toTag, (std::string)"346543ewr");
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testObserverUpdate2()
//{
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	CSIPSubscriber* pSubscriber = m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId);
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_CONNECTING);
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
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"active;expires=300");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"SIP/2.0 100 Trying");
//}
//
/////////////////////////////////////////////////////////////////////
//void CTestSIPReferEventPackageManager::testObserverUpdate3()
//{
//	BuildRefer();
//	m_pHeaderList->BuildMessage(&m_pReferMsg->sipHeaders);
//	m_pMgr->Refer(m_pReferMsg, 15, m_srcUnitId);
//
//	CSIPSubscriber* pSubscriber = m_pMgr->FindParty("party_1@polycom.com", m_CSeq, m_callId);
//	m_pMgr->ObserverUpdate(pSubscriber, PARTYSTATE, PARTY_DISCONNECTING);
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
//	mcReqNotify* pNotifyReq = NULL;
//	m_pMockProtocol->Verify_SipNotify_WasCalled(&pNotifyReq);
//	ParseNotifyReq(pNotifyReq, uri, &ip, &port, &transport, event, state, contentType, content, &CSeq, callId, fromTag, toTag);
//	PDELETE(pNotifyReq);
//
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong state", (std::string)state, (std::string)"terminated;reason=noresource");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content", (std::string)content, (std::string)"SIP/2.0 503 Service Unavailable");
//}
//
