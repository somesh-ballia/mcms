//
//#include "TestSIPMSData.h"
//#include "ApiStatuses.h"
//
//
//
//// ************************************************************************************
////
////	CTestCXConfInfoType
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestCXConfInfoType );
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::setUp()
//{
//	m_pInfo = NULL;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::tearDown()
//{
//	POBJDELETE(m_pInfo);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testConstructor()
//{
//	m_pInfo = new CCXConfInfoType();
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXConfInfoType::testConstructor ", m_pInfo != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testGetElementName()
//{
//	m_pInfo = new CCXConfInfoType();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong name returned", (std::string)"com.microsoft.sip.cx-conference-info", (std::string)m_pInfo->GetElementName());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testGetEntity()
//{
//	CCommConf* pCommConf = new CCommConf;
//	m_pInfo = new CCXConfInfoType(pCommConf, "a@b.c");
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "CTestCXConfInfoType::testConstructor ", (std::string)"a@b.c", (std::string)m_pInfo->GetEntity() );
//	POBJDELETE(pCommConf);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSerializeXML()
//{
//	char* pValue = NULL;
//	m_pInfo = new CCXConfInfoType();
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	pElement->get_tagName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong node-name returned", (std::string)"com.microsoft.sip.cx-conference-info",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSerializeXML1()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pUserElement;
//	char* pValue = NULL;
//
//	pElement->get_tagName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong node-name returned", (std::string)"com.microsoft.sip.cx-conference-info",  (std::string)pValue );
//
//
//	pElement->getChildNodeByName(&pUserElement, "user");
//	CPPUNIT_ASSERT_MESSAGE( "properties not found ",  pUserElement != NULL );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_1",  (std::string)pValue );
//
//	pElement->getAttribute("entity", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong entity returned", (std::string)"conf@polycom.com",  (std::string)pValue );
//
//	pElement->getAttribute("version", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong version returned", (std::string)"1",  (std::string)pValue );
//
//
//	pElement->getAttribute("state", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"full",  (std::string)pValue );
//
//	PDELETE(pElement);
//	POBJDELETE(pConfParty);
//	POBJDELETE(pCommConf);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSerializeXMLWith2Parties()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//	CConfParty* pConfParty2 = new CConfParty;
//	pConfParty2->SetName("party_2");
//	pConfParty2->SetPartyId(2);
//	pConfParty2->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty2);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_1",  (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_2",  (std::string)pValue );
//
//	PDELETE(pElement);
//	POBJDELETE(pConfParty);
//	POBJDELETE(pConfParty2);
//	POBJDELETE(pCommConf);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSerializeXMLWith1PartyAndPartialDataForLock()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	//First serialize to get 'full' data
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	PDELETE(pElement);
//	pElement=new CXMLDOMElement();
//
//	m_pInfo->SetLocked(TRUE);
//
//	//Second serialize to get 'partial' data
//	m_pInfo->SerializeXml(pElement, FALSE);
//
//	CXMLDOMElement *pPropertiesElement, *pSubElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//	pPropertiesElement->getChildNodeByName(&pSubElement, "locked");
//	CPPUNIT_ASSERT_MESSAGE( "locked not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong locked returned", (std::string)"true",  (std::string)pValue );
//
//	pElement->getChildNodeByName(&pElement, "user");
//	CPPUNIT_ASSERT_MESSAGE( "user was found ",  pElement == NULL );
//
//	PDELETE(pElement);
//	POBJDELETE(pConfParty);
//	POBJDELETE(pCommConf);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSerializeXMLWith1PartyAndPartialDataForPartyMute()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	//First serialize to get 'full' data
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	PDELETE(pElement);
//	pElement=new CXMLDOMElement();
//
//	m_pInfo->MutePartyMedia(1, eAudio, TRUE);
//
//	//Second serialize to get 'partial' data
//	m_pInfo->SerializeXml(pElement, FALSE);
//
//	CXMLDOMElement *pResultElement, *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->getChildNodeByName(&pPropertiesElement, "properties");
//	CPPUNIT_ASSERT_MESSAGE( "properties was found ",   pPropertiesElement == NULL);
//
//	pElement->getChildNodeByName(&pUserElement, "user");
//	CPPUNIT_ASSERT_MESSAGE( "user not found ",  pUserElement != NULL );
//	pUserElement->getChildNodeByName(&pResultElement, "media-stream");
//	CPPUNIT_ASSERT_MESSAGE("testMutePartyMedia, media-stream not found" , pResultElement != NULL);
//	pResultElement->getAttribute("content", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content returned", (std::string)"audio",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pUserElement, "receive-state");
//	CPPUNIT_ASSERT_MESSAGE("testMutePartyMedia, receive-state not found" , pResultElement != NULL);
//	pUserElement->getAttribute("muted", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong muted returned", (std::string)"true",  (std::string)pValue );
//
//
//	PDELETE(pElement);
//	POBJDELETE(pConfParty);
//	POBJDELETE(pCommConf);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSerializeXMLWithNoChange()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	//First serialize to get 'full' data
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	PDELETE(pElement);
//	pElement=new CXMLDOMElement();
//
//
//	//Second serialize to get 'noChange' data
//	m_pInfo->SerializeXml(pElement, FALSE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->get_tagName(&pValue);
//	CPPUNIT_ASSERT_MESSAGE("wrong node-name returned", pValue == NULL );
//
//	pElement->getChildNodeByName(&pPropertiesElement, "properties");
//	CPPUNIT_ASSERT_MESSAGE( "properties was found ",   pPropertiesElement == NULL);
//
//	pElement->getChildNodeByName(&pUserElement, "user");
//	CPPUNIT_ASSERT_MESSAGE( "user was found ",  pUserElement == NULL );
//
//	PDELETE(pElement);
//	POBJDELETE(pConfParty);
//	POBJDELETE(pCommConf);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testWasUpdated1()
//{
//	m_pInfo = new CCXConfInfoType();
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXConfInfoType::testWasUpdated1 ", m_pInfo->WasUpdated() != FALSE );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testWasUpdated2()
//{
//	CCommConf* pCommConf = new CCommConf;
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//
//	//serialize to reset state
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXConfInfoType::testWasUpdated2 ", m_pInfo->WasUpdated() != TRUE );
//
//	//update a param to set state
//	m_pInfo->SetLocked(TRUE);
//
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXConfInfoType::testWasUpdated2 ", m_pInfo->WasUpdated() != FALSE );
//
//	POBJDELETE(pCommConf);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testGetState1()
//{
//	m_pInfo = new CCXConfInfoType();
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXConfInfoType::testGetState1 ", m_pInfo->GetState() == eFullData );
//}
//
//////////////////////////////////////////////////////////////////////
////After serialize, state should change
//void CTestCXConfInfoType::testGetState2()
//{
//	CCommConf* pCommConf = new CCommConf;
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXConfInfoType::testGetState1 ", m_pInfo->GetState() == eNoChange );
//
//	POBJDELETE(pCommConf);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testAddParty()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//
//	//first verisy no users appear in XML
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->getChildNodeByName(&pPropertiesElement, "properties");
//	CPPUNIT_ASSERT_MESSAGE( "properties not found ",  pPropertiesElement != NULL );
//
//	pElement->getChildNodeByName(&pUserElement, "user");
//	CPPUNIT_ASSERT_MESSAGE( "user was found ",  pUserElement == NULL );
//
//	PDELETE(pElement);
//
//	//now, add one party, find it in XML
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//
//	m_pInfo->AddParty(pConfParty);
//
//	pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_1",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testDelParty1()
//{
//	//first verify we have 1 user in list
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_1",  (std::string)pValue );
//
//	PDELETE(pElement);
//
//	//now, remove party and see that it's gone
//	m_pInfo->DelParty(1, 1);
//	pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	pElement->getChildNodeByName(&pUserElement, "user");
//	CPPUNIT_ASSERT_MESSAGE( "user was found ",  pUserElement == NULL );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testDelParty2()
//{
//	//first verify we have 1 user in list
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_1",  (std::string)pValue );
//
//	PDELETE(pElement);
//
//	//now, remove a wrong party and verify first one is still in the list
//	m_pInfo->DelParty(2, 1);
//	pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_1",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetDisplayName()
//{
//	//first verify we have 1 user in list
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_1",  (std::string)pValue );
//
//	PDELETE(pElement);
//
//	//now, remove a wrong party and verify first one is still in the list
//	m_pInfo->SetDisplayName(1, "tester");
//	pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"tester",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetDisplayPhoneNumber()
//{
//	//first verify we have 1 user in list
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPhoneNumber("9251289");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-phone-number");
//	CPPUNIT_ASSERT_MESSAGE( "display-phone-number not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-phone-number returned", (std::string)"9251289",  (std::string)pValue );
//
//	PDELETE(pElement);
//
//	//now, remove a wrong party and verify first one is still in the list
//	m_pInfo->SetDisplayPhoneNumber(1, "98765432");
//	pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "display-phone-number");
//	CPPUNIT_ASSERT_MESSAGE( "display-phone-number not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-phone-number returned", (std::string)"98765432",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetSipUri()
//{
//	//first verify we have 1 user in list
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);
//	pConfParty->SetSipPartyAddress("sip@proxy.com");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "associated-sip-uri");
//	CPPUNIT_ASSERT_MESSAGE( "associated-sip-uri not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong associated-sip-uri returned", (std::string)"sip:sip@proxy.com",  (std::string)pValue );
//
//	PDELETE(pElement);
//
//	//now, remove a wrong party and verify first one is still in the list
//	m_pInfo->SetSipUri(1, "user@sip.com");
//	pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "associated-sip-uri");
//	CPPUNIT_ASSERT_MESSAGE( "associated-sip-uri not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong associated-sip-uri returned", (std::string)"user@sip.com",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetAdministrator()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetAdministrator(1, TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "role");
//	CPPUNIT_ASSERT_MESSAGE( "role not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong role returned", (std::string)"administrator",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetPartyStatus()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetPartyState(PARTY_WAITING_FOR_DIAL_IN);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "status");
//	CPPUNIT_ASSERT_MESSAGE( "status not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"pending",  (std::string)pValue );
//
//	PDELETE(pElement);
//
//	//now, remove a wrong party and verify first one is still in the list
//	m_pInfo->SetPartyStatus(1, PARTY_CONNECTED, STATUS_PARTY_INCONF,2);
//	pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "status");
//	CPPUNIT_ASSERT_MESSAGE( "status not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"connected",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetDialOutParty()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetConnectionType(DIAL_OUT);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "join-mode");
//	CPPUNIT_ASSERT_MESSAGE( "join-mode not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong join-mode returned", (std::string)"dialed-out",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetDisconnectReason()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetDisconnectReason(1, DISCONNECTED_BY_OPERATOR);
//	m_pInfo->SetPartyStatus(1, PARTY_DISCONNECTED, STATUS_PARTY_INCONF,2);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "disconnect-reason");
//	CPPUNIT_ASSERT_MESSAGE( "disconnect-reason not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong disconnect-reason returned", (std::string)"booted",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetActiveSpeaker()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetActiveSpeaker(1, TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pUserElement, "actively-talking");
//	CPPUNIT_ASSERT_MESSAGE( "actively-talking not found ",  pUserElement != NULL );
//	pUserElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong actively-talking returned", (std::string)"true",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetUriAttribute()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);
//	pConfParty->SetSipPartyAddress("sip@proxy.com");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetUriAttribute(1, "ori@tdd.com");
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getAttribute("uri", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong uri returned", (std::string)"ori@tdd.com",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testMutePartyMedia()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);
//	pConfParty->SetSipPartyAddress("sip@proxy.com");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->MutePartyMedia(1, eAudio, TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pPropertiesElement, *pUserElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pElement->nextChildNode(&pUserElement);
//	pUserElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "user not found ",  (std::string)"user", (std::string)pValue );
//	pUserElement->getChildNodeByName(&pResultElement, "media-stream");
//	CPPUNIT_ASSERT_MESSAGE("testMutePartyMedia, media-stream not found" , pResultElement != NULL);
//	pResultElement->getAttribute("content", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content returned", (std::string)"audio",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pUserElement, "receive-state");
//	CPPUNIT_ASSERT_MESSAGE("testMutePartyMedia, receive-state not found" , pResultElement != NULL);
//	pUserElement->getAttribute("muted", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong muted returned", (std::string)"true",  (std::string)pValue );
//
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetActive()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetActive(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pPropertiesElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pPropertiesElement->getChildNodeByName(&pResultElement, "active");
//	CPPUNIT_ASSERT_MESSAGE( "active not found ",  pResultElement != NULL );
//	pResultElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong active returned", (std::string)"true",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetLocked()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetLocked(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pPropertiesElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pPropertiesElement->getChildNodeByName(&pResultElement, "locked");
//	CPPUNIT_ASSERT_MESSAGE( "locked not found ",  pResultElement != NULL );
//	pResultElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong locked returned", (std::string)"true",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetRollCall()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetRollCall(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pPropertiesElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pPropertiesElement->getChildNodeByName(&pResultElement, "record-names");
//	CPPUNIT_ASSERT_MESSAGE( "record-names not found ",  pResultElement != NULL );
//	pResultElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong record-names returned", (std::string)"true",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetAnnouncements()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetAnnouncements(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pPropertiesElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pPropertiesElement->getChildNodeByName(&pResultElement, "announcements");
//	CPPUNIT_ASSERT_MESSAGE( "announcements not found ",  pResultElement != NULL );
//	pResultElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong announcements returned", (std::string)"true",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXConfInfoType::testSetRecorded()
//{
//	CCommConf* pCommConf = new CCommConf;
//	CRsrvParty* pConfParty = new CRsrvParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPartyId(1);
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pCommConf->Add(*pConfParty);
//
//	m_pInfo = new CCXConfInfoType(pCommConf, "conf@polycom.com");
//	m_pInfo->SetRecorded(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pPropertiesElement;
//	char* pValue = NULL;
//
//	pElement->firstChildNode(&pPropertiesElement);
//	pPropertiesElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "properties not found ",  (std::string)"properties", (std::string)pValue );
//
//	pPropertiesElement->getChildNodeByName(&pResultElement, "record-content");
//	CPPUNIT_ASSERT_MESSAGE( "record-content not found ",  pResultElement != NULL );
//	pResultElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong record-content returned", (std::string)"true",  (std::string)pValue );
//
//	POBJDELETE(pCommConf);
//	POBJDELETE(pConfParty);
//	PDELETE(pElement);
//}
//
//// ************************************************************************************
////
////	CTestCXUserType
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestCXUserType );
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::setUp()
//{
//	m_pUserInfo = NULL;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::tearDown()
//{
//	POBJDELETE(m_pUserInfo);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testConstructor()
//{
//	m_pUserInfo = new CCXUserType();
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXUserType::testConstructor ", m_pUserInfo != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testConstructorWithCConfParty()
//{
//	CRsrvParty* pRsrvParty = new CRsrvParty;
//	pRsrvParty->SetName("party_1");
//	pRsrvParty->SetPhoneNumber("9912431");
//	pRsrvParty->SetSipPartyAddress("party_1@polycom.com");
//	pRsrvParty->SetUndefinedType(UNDEFINED_PARTY);
//	pRsrvParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);
//	pRsrvParty->SetConnectionType(DIAL_OUT);
//
//	CConfParty Party = *pRsrvParty;
//
//	m_pUserInfo = new CCXUserType(&Party);
//	POBJDELETE(pRsrvParty);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getAttribute("uri", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong uri returned", (std::string)"sip:party_1@polycom.com",  (std::string)pValue );
//
//
//	pResultElement->getAttribute("state", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"full",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_1",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "display-phone-number");
//	CPPUNIT_ASSERT_MESSAGE( "display-phone-number element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-phone-number returned", (std::string)"9912431",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "sip-capable");
//	CPPUNIT_ASSERT_MESSAGE( "sip-capable element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong sip-capable returned", (std::string)"true",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "associated-sip-uri");
//	CPPUNIT_ASSERT_MESSAGE( "associated-sip-uri ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong associated-sip-uri returned", (std::string)"sip:party_1@polycom.com",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "role");
//	CPPUNIT_ASSERT_MESSAGE( "role",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong role returned", (std::string)"participant",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "status");
//	CPPUNIT_ASSERT_MESSAGE( "status element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"pending",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "join-mode");
//	CPPUNIT_ASSERT_MESSAGE( "join-mode element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong join-mode returned", (std::string)"dialed-out",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "disconnect-reason");
//	CPPUNIT_ASSERT_MESSAGE( "status element ",  pDataElement == NULL );
//
//	pResultElement->getChildNodeByName(&pDataElement, "actively-talking");
//	CPPUNIT_ASSERT_MESSAGE( "actively-talking element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong actively-talking returned", (std::string)"false",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "media-stream");
//	CPPUNIT_ASSERT_MESSAGE( "media-stream element ",  pDataElement != NULL );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testGetElementName()
//{
//	m_pUserInfo = new CCXUserType();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong name returned", (std::string)"user", (std::string)m_pUserInfo->GetElementName());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testPartyRole1()
//{
//	m_pUserInfo = new CCXUserType();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"participant",  (std::string)m_pUserInfo->GetPartyRole() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testPartyRole2()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetAdministrator(TRUE);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"administrator",  (std::string)m_pUserInfo->GetPartyRole() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testPartyStatus1()
//{
//	m_pUserInfo = new CCXUserType();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"pending",  (std::string)m_pUserInfo->GetPartyStatus() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testPartyStatus2()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetPartyStatus(PARTY_CONNECTED, STATUS_PARTY_INCONF);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"connected",  (std::string)m_pUserInfo->GetPartyStatus() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testPartyJoinMode1()
//{
//	m_pUserInfo = new CCXUserType();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"dialed-in",  (std::string)m_pUserInfo->GetPartyJoinMode() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testPartyJoinMode2()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetDialOutParty();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"dialed-out",  (std::string)m_pUserInfo->GetPartyJoinMode() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testPartyDisconnectReason1()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetDisconnectReason(PARTY_HANG_UP);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"departed",  (std::string)m_pUserInfo->GetPartyDisconnectReason() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSerializeXML()
//{
//	m_pUserInfo = new CCXUserType();
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement;
//	pElement->getChildNodeByName(&pResultElement, "user");
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXConfInfoType::testSerializeXML ",  pResultElement != NULL );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSerializeXML1()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetDisplayName("party1");
//	m_pUserInfo->SetDisplayPhoneNumber("233337");
//	m_pUserInfo->SetUriAttribute("party@polycom.co.il");
//	m_pUserInfo->SetSipUri("party@polycom.co.il");
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getAttribute("uri", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong uri returned", (std::string)"party@polycom.co.il",  (std::string)pValue );
//
//
//	pResultElement->getAttribute("state", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"full",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party1",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "display-phone-number");
//	CPPUNIT_ASSERT_MESSAGE( "display-phone-number element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-phone-number returned", (std::string)"233337",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "sip-capable");
//	CPPUNIT_ASSERT_MESSAGE( "sip-capable element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong sip-capable returned", (std::string)"true",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "associated-sip-uri");
//	CPPUNIT_ASSERT_MESSAGE( "associated-sip-uri ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong associated-sip-uri returned", (std::string)"party@polycom.co.il",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "role");
//	CPPUNIT_ASSERT_MESSAGE( "role",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong role returned", (std::string)"participant",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "status");
//	CPPUNIT_ASSERT_MESSAGE( "status element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"pending",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "join-mode");
//	CPPUNIT_ASSERT_MESSAGE( "join-mode element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong join-mode returned", (std::string)"dialed-in",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "disconnect-reason");
//	CPPUNIT_ASSERT_MESSAGE( "status element ",  pDataElement == NULL );
//
//	pResultElement->getChildNodeByName(&pDataElement, "actively-talking");
//	CPPUNIT_ASSERT_MESSAGE( "actively-talking element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong actively-talking returned", (std::string)"false",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "media-stream");
//	CPPUNIT_ASSERT_MESSAGE( "media-stream element found",  pDataElement == NULL );
//
//	PDELETE(pElement);
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetDisplayName()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetDisplayName("party_3425");
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "display-name");
//	CPPUNIT_ASSERT_MESSAGE( "display-name element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-name returned", (std::string)"party_3425",  (std::string)pValue );
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetDisplayPhoneNumber()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetDisplayPhoneNumber("3425");
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "display-phone-number");
//	CPPUNIT_ASSERT_MESSAGE( "display-phone-number element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong display-phone-number returned", (std::string)"3425",  (std::string)pValue );
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetSipUri1()
//{
//	//first check without SIP data
//	m_pUserInfo = new CCXUserType();
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "sip-capable");
//	CPPUNIT_ASSERT_MESSAGE( "sip-capable element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong sip-capable returned", (std::string)"false",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetSipUri2()
//{
//	//check again with SIP data
//	m_pUserInfo = new CCXUserType();
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//
//	m_pUserInfo->SetSipUri("ua@sa.com");
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "sip-capable");
//	CPPUNIT_ASSERT_MESSAGE( "sip-capable element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong sip-capable returned", (std::string)"true",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pDataElement, "associated-sip-uri");
//	CPPUNIT_ASSERT_MESSAGE( "associated-sip-uri ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong associated-sip-uri returned", (std::string)"ua@sa.com",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetAdministrator()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetAdministrator(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "role");
//	CPPUNIT_ASSERT_MESSAGE( "role element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong role returned", (std::string)"administrator",  (std::string)pValue );
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetPartyStatus()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetPartyStatus(PARTY_CONNECTED_PARTIALY, STATUS_PARTY_INCONF);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "status");
//	CPPUNIT_ASSERT_MESSAGE( "status element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong status returned", (std::string)"connected",  (std::string)pValue );
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetDialOutParty()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetDialOutParty();
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "join-mode");
//	CPPUNIT_ASSERT_MESSAGE( "join-mode element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong join-mode returned", (std::string)"dialed-out",  (std::string)pValue );
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetDiscoonectReason1()
//{
//	m_pUserInfo = new CCXUserType();
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "disconnect-reason");
//	CPPUNIT_ASSERT_MESSAGE( "disconnect-reason element ",  pDataElement == NULL );
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetDiscoonectReason2()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetDisconnectReason(PARTY_HANG_UP);
//	m_pUserInfo->SetPartyStatus(PARTY_DISCONNECTED, STATUS_PARTY_INCONF);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "disconnect-reason");
//	CPPUNIT_ASSERT_MESSAGE( "disconnect-reason element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong disconnect-reason returned", (std::string)"departed",  (std::string)pValue );
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testSetActiveSpeaker()
//{
//	m_pUserInfo = new CCXUserType();
//	m_pUserInfo->SetActiveSpeaker(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "actively-talking");
//	CPPUNIT_ASSERT_MESSAGE( "actively-talking element ",  pDataElement != NULL );
//	pDataElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong actively-talking returned", (std::string)"true",  (std::string)pValue );
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXUserType::testMuteMedia()
//{
//	CConfParty* pConfParty = new CConfParty;
//	pConfParty->SetName("party_1");
//	pConfParty->SetPhoneNumber("9912431");
//	pConfParty->SetSipPartyAddress("party_1@polycom.com");
//	pConfParty->SetUndefinedType(UNDEFINED_PARTY);
//	pConfParty->SetConnectionType(DIAL_OUT);
//
//	m_pUserInfo = new CCXUserType(pConfParty);
//	POBJDELETE(pConfParty);
//
//	m_pUserInfo->MuteMedia(eAudio, TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pUserInfo->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pDataElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pUserInfo->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pDataElement, "media-stream");
//	CPPUNIT_ASSERT_MESSAGE( "media-stream element ",  pDataElement != NULL );
//	pDataElement->getChildNodeByName(&pResultElement, "receive-state");
//	CPPUNIT_ASSERT_MESSAGE( "receive-state element ",  pResultElement != NULL );
//
//	pResultElement->getAttribute("muted", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong muted returned", (std::string)"true",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//
//// ************************************************************************************
////
////	CTestCXMediaType
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestCXMediaType );
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::setUp()
//{
//	m_pMedia = NULL;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::tearDown()
//{
//	POBJDELETE(m_pMedia);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::testConstructor()
//{
//	m_pMedia = new CCXMediaType();
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXMediaType::testConstructor ", m_pMedia != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::testGetElementName()
//{
//	m_pMedia = new CCXMediaType();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong name returned", (std::string)"media-stream", (std::string)m_pMedia->GetElementName());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::testGetContent1()
//{
//	m_pMedia = new CCXMediaType(eVideoT);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "CTestCXMediaType::testGetContent1 ", (std::string)"video", (std::string)m_pMedia->GetContentAsString() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::testGetContent2()
//{
//	m_pMedia = new CCXMediaType(eVoip);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "CTestCXMediaType::testGetContent2 ", (std::string)"voip", (std::string)m_pMedia->GetContentAsString() );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::testSerializeXML()
//{
//	m_pMedia = new CCXMediaType();
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pMedia->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement;
//	pElement->getChildNodeByName(&pResultElement, "media-stream");
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXMediaType::testSerializeXML ",  pResultElement != NULL );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::testSerializeXML1()
//{
//	m_pMedia = new CCXMediaType(eVoip);
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pMedia->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pMediaElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pMedia->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getAttribute("content", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content returned", (std::string)"voip",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pMediaElement, "receive-state");
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXMediaType::testSerializeXML1 ",  pMediaElement != NULL );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaType::testMuteMedia()
//{
//	m_pMedia = new CCXMediaType(eVoip);
//	m_pMedia->MuteMedia(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pMedia->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement, *pMediaElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pMedia->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getChildNodeByName(&pMediaElement, "receive-state");
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXMediaType::testSerializeXML1 ",  pMediaElement != NULL );
//
//	pMediaElement->getAttribute("muted", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong muted returned", (std::string)"true",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//// ************************************************************************************
////
////	CTestCXMediaStateType
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestCXMediaStateType );
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaStateType::setUp()
//{
//	m_pMedia = NULL;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaStateType::tearDown()
//{
//	POBJDELETE(m_pMedia);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaStateType::testConstructor()
//{
//	m_pMedia = new CCXMediaStateType();
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXMediaStateType::testConstructor ", m_pMedia != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaStateType::testGetElementName()
//{
//	m_pMedia = new CCXMediaStateType();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong name returned", (std::string)"receive-state", (std::string)m_pMedia->GetElementName());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaStateType::testSerializeXML()
//{
//	m_pMedia = new CCXMediaStateType();
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pMedia->SerializeXml(pElement);
//
//	CXMLDOMElement *pResultElement;
//	pElement->getChildNodeByName(&pResultElement, "receive-state");
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXMediaStateType::testSerializeXML ",  pResultElement != NULL );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaStateType::testSerializeXML1()
//{
//	m_pMedia = new CCXMediaStateType(TRUE);
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pMedia->SerializeXml(pElement);
//
//	CXMLDOMElement *pResultElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pMedia->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getAttribute("muted", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content returned", (std::string)"true",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXMediaStateType::testSetMute()
//{
//	m_pMedia = new CCXMediaStateType(TRUE);
//	m_pMedia->SetMute(FALSE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pMedia->SerializeXml(pElement);
//
//	CXMLDOMElement *pResultElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pMedia->GetElementName());
//
//	char* pValue = NULL;
//
//	pResultElement->getAttribute("muted", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong content returned", (std::string)"false",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//// ************************************************************************************
////
////	CTestCXInitialState
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestCXInitialState );
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::setUp()
//{
//	m_pState = NULL;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::tearDown()
//{
//	POBJDELETE(m_pState);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::testConstructor()
//{
//	m_pState = new CCXInitialState();
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXInitialState::testConstructor ", m_pState != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::testGetElementName()
//{
//	m_pState = new CCXInitialState();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong name returned", (std::string)"initial-state", (std::string)m_pState->GetElementName());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::testSerializeXML()
//{
//	m_pState = new CCXInitialState();
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pState->SerializeXml(pElement);
//
//	CXMLDOMElement *pResultElement;
//	pElement->getChildNodeByName(&pResultElement, "initial-state");
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXInitialState::testSerializeXML ",  pResultElement != NULL );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::testSerializeXML1()
//{
//	m_pState = new CCXInitialState();
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pState->SerializeXml(pElement);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pState->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "role");
//	CPPUNIT_ASSERT_MESSAGE( "role not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong role returned", (std::string)"participant",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pSubElement, "joined");
//	CPPUNIT_ASSERT_MESSAGE( "joined not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong joined returned", (std::string)"dialed-in",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pSubElement, "rejoined");
//	CPPUNIT_ASSERT_MESSAGE( "rejoined not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong rejoined returned", (std::string)"dialed-in",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pSubElement, "media");
//	CPPUNIT_ASSERT_MESSAGE( "media not found ",  pSubElement != NULL );
//	pSubElement->getChildNodeByName(&pResultElement, "audio");
//	CPPUNIT_ASSERT_MESSAGE( "audio not found ",  pSubElement != NULL );
//
//	pResultElement->getChildNodeByName(&pSubElement, "receive-muted");
//	CPPUNIT_ASSERT_MESSAGE( "receive-muted not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong rejoined returned", (std::string)"false",  (std::string)pValue );
//
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::testSetMute()
//{
//	m_pState = new CCXInitialState();
//	m_pState->SetMute(TRUE);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	CXMLDOMElement *pSubElement = NULL, *pResultElement = NULL;
//	m_pState->SerializeXml(pElement);
//
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pState->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "media");
//	CPPUNIT_ASSERT_MESSAGE( "media not found ",  pSubElement != NULL );
//	pSubElement->getChildNodeByName(&pResultElement, "audio");
//	CPPUNIT_ASSERT_MESSAGE( "audio not found ",  pSubElement != NULL );
//
//	pResultElement->getChildNodeByName(&pSubElement, "receive-muted");
//	CPPUNIT_ASSERT_MESSAGE( "receive-muted not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong rejoined returned", (std::string)"true",  (std::string)pValue );
//
//
//	PDELETE(pElement);
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::testSetAdmin()
//{
//	m_pState = new CCXInitialState();
//	m_pState->SetAdmin();
//
//	char* pValue = NULL;
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pState->SerializeXml(pElement);
//
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pState->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "role");
//	CPPUNIT_ASSERT_MESSAGE( "role not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong role returned", (std::string)"administrator",  (std::string)pValue );
//
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::testSetJoinAsDialOut()
//{
//	m_pState = new CCXInitialState();
//	m_pState->SetJoinAsDialOut();
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pState->SerializeXml(pElement);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pState->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "joined");
//	CPPUNIT_ASSERT_MESSAGE( "joined not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong joined returned", (std::string)"dialed-out",  (std::string)pValue );
//
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXInitialState::testSetRejoinAsDialOut()
//{
//	m_pState = new CCXInitialState();
//	m_pState->SetRejoinAsDialOut();
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pState->SerializeXml(pElement);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pState->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "rejoined");
//	CPPUNIT_ASSERT_MESSAGE( "rejoined not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong rejoined returned", (std::string)"dialed-out",  (std::string)pValue );
//
//
//	PDELETE(pElement);
//}
//
//// ************************************************************************************
////
////	CTestCXPropertiesType
////
//// ************************************************************************************
//
//CPPUNIT_TEST_SUITE_REGISTRATION( CTestCXPropertiesType );
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::setUp()
//{
//	m_pProperties = NULL;
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::tearDown()
//{
//	POBJDELETE(m_pProperties);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testConstructor()
//{
//	m_pProperties = new CCXPropertiesType();
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXPropertiesType::testConstructor ", m_pProperties != NULL );
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testGetElementName()
//{
//	m_pProperties = new CCXPropertiesType();
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong name returned", (std::string)"properties", (std::string)m_pProperties->GetElementName());
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testSerializeXML()
//{
//	m_pProperties = new CCXPropertiesType();
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pProperties->SerializeXml(pElement, TRUE);
//
//	CXMLDOMElement *pResultElement;
//	pElement->getChildNodeByName(&pResultElement, "properties");
//	CPPUNIT_ASSERT_MESSAGE( "CTestCXPropertiesType::testSerializeXML ",  pResultElement != NULL );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testSerializeXML1()
//{
//	CCommConf* pCommConf = new CCommConf();
//	m_pProperties = new CCXPropertiesType(pCommConf);
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pProperties->SerializeXml(pElement, TRUE);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement, *pMediaElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pProperties->GetElementName());
//
//	pResultElement->firstChildNode(&pSubElement);
//	pSubElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "active not found ",  (std::string)"active", (std::string)pValue );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong active returned", (std::string)"false",  (std::string)pValue );
//
//	pResultElement->nextChildNode(&pSubElement);
//	pSubElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "locked not found ",  (std::string)"locked", (std::string)pValue );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong locked returned", (std::string)"false",  (std::string)pValue );
//
//	pResultElement->nextChildNode(&pSubElement);
//	pSubElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "media-stream 1 not found ",  (std::string)"media-stream", (std::string)pValue );
//	pSubElement->getChildNodeByName(&pMediaElement, "receive-state");
//	pSubElement->getAttribute("content", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong receive-state 1 returned", (std::string)"audio",  (std::string)pValue );
//
//	pResultElement->nextChildNode(&pSubElement);
//	pSubElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "media-stream 2 not found ",  (std::string)"media-stream", (std::string)pValue );
//	pSubElement->getChildNodeByName(&pMediaElement, "receive-state");
//	pSubElement->getAttribute("content", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong receive-state 2 returned", (std::string)"voip",  (std::string)pValue );
//
//	pResultElement->nextChildNode(&pSubElement);
//	pSubElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "media-stream 3 not found ",  (std::string)"media-stream", (std::string)pValue );
//	pSubElement->getChildNodeByName(&pMediaElement, "receive-state");
//	pSubElement->getAttribute("content", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong receive-state 3 returned", (std::string)"video",  (std::string)pValue );
//
//	/*pResultElement->nextChildNode(&pSubElement);
//	pSubElement->get_nodeName(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "media-stream 4 not found ",  (std::string)"media-stream", (std::string)pValue );
//	pSubElement->getChildNodeByName(&pMediaElement, "receive-state");
//	pSubElement->getAttribute("content", &pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong receive-state 4 returned", (std::string)"data",  (std::string)pValue );
//*/
//	pResultElement->getChildNodeByName(&pSubElement, "record-names");
//	CPPUNIT_ASSERT_MESSAGE( "record-names not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong record-names returned", (std::string)"false",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pSubElement, "announcements");
//	CPPUNIT_ASSERT_MESSAGE( "announcements not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong announcements returned", (std::string)"false",  (std::string)pValue );
//
//	pResultElement->getChildNodeByName(&pSubElement, "record-content");
//	CPPUNIT_ASSERT_MESSAGE( "record-content not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong record-content returned", (std::string)"false",  (std::string)pValue );
//
//	PDELETE(pElement);
//	POBJDELETE(pCommConf);
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testSetActive()
//{
//	m_pProperties = new CCXPropertiesType();
//	m_pProperties->SetActive(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pProperties->SerializeXml(pElement, TRUE);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pProperties->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "active");
//	CPPUNIT_ASSERT_MESSAGE( "active not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong active returned", (std::string)"true",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testSetLocked()
//{
//	m_pProperties = new CCXPropertiesType();
//	m_pProperties->SetLocked(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pProperties->SerializeXml(pElement, TRUE);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pProperties->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "locked");
//	CPPUNIT_ASSERT_MESSAGE( "locked not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong locked returned", (std::string)"true",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testSetRollCall()
//{
//	m_pProperties = new CCXPropertiesType();
//	m_pProperties->SetRollCall(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pProperties->SerializeXml(pElement, TRUE);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pProperties->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "record-names");
//	CPPUNIT_ASSERT_MESSAGE( "record-names not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong record-names returned", (std::string)"true",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testSetAnnouncements()
//{
//	m_pProperties = new CCXPropertiesType();
//	m_pProperties->SetAnnouncements(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pProperties->SerializeXml(pElement, TRUE);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pProperties->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "announcements");
//	CPPUNIT_ASSERT_MESSAGE( "announcements not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong announcements returned", (std::string)"true",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
//////////////////////////////////////////////////////////////////////
//void CTestCXPropertiesType::testSetRecorded()
//{
//	m_pProperties = new CCXPropertiesType();
//	m_pProperties->SetRecorded(TRUE);
//
//	CXMLDOMElement *pElement=new CXMLDOMElement();
//	m_pProperties->SerializeXml(pElement, TRUE);
//
//	char* pValue = NULL;
//	CXMLDOMElement *pResultElement, *pSubElement;
//	pElement->getChildNodeByName(&pResultElement, (char*)m_pProperties->GetElementName());
//
//	pResultElement->getChildNodeByName(&pSubElement, "record-content");
//	CPPUNIT_ASSERT_MESSAGE( "record-content not found ",  pSubElement != NULL );
//	pSubElement->get_nodeValue(&pValue);
//	CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong record-content returned", (std::string)"true",  (std::string)pValue );
//
//	PDELETE(pElement);
//}
//
