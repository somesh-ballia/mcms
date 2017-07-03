#include "TestLyncApi.h"
#include "EventPackage.h"

#include "SystemFunctions.h"
#include "OsFileIF.h"

#include "Trace.h"
#include "TraceStream.h"

#include <iostream>
#include <memory>
#include <unistd.h>
#include <algorithm>

#define PARTY_ID1 1
#define PARTY_ID2 2

using namespace EventPackage;

CPPUNIT_TEST_SUITE_REGISTRATION(CTestLyncApi);

////////////////////////////////////////////////////////////////////////////
//                        CTestLyncApi
////////////////////////////////////////////////////////////////////////////
void CTestLyncApi::testEventManager()
{
#if 0
	COsQueue* queue1 = new COsQueue(ePartyId, 1, eProcessConfParty);
	COsQueue* queue2 = new COsQueue(ePartyId, 2, eProcessConfParty);
	COsQueue* queue3 = new COsQueue(ePartyId, 3, eProcessConfParty);

	EventPackage::Manager& EventPackage = EventPackage::Manager::Instance();

	for (int event = eLyncEvent_DEFAULT+1; event < eLyncEvent_LAST; ++event)
	{
		EventPackage.AddSubscriber(PARTY_ID2, (LyncEvent)event, queue1);
		if (event % 2)
			EventPackage.AddSubscriber(PARTY_ID2, (LyncEvent)event, queue2);
		else
			EventPackage.AddSubscriber(PARTY_ID2, (LyncEvent)event, queue3);
	}
	EventPackage.AddSubscriber(PARTY_ID1, eEventType_MediaStatusUpdated, queue3);
	//lyncEventManager.DetachSubscriber(queue3);

	MergeXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Real.xml", "Processes/ConfParty/Tests/TestLync.Sample.RealUpdated.xml", PARTY_ID2);

	LyncMsi lyncMsi[] = { 2000000, 4000000, 8000000 };
	EventPackage::ApiLync::Instance().AddDSH(PARTY_ID2, lyncMsi, sizeof(lyncMsi)/sizeof(LyncMsi));

	delete queue1;
	delete queue2;
	delete queue3;

	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
	EP_TRACE << "\nOK";

#endif

	COsQueue* queue1 = new COsQueue(ePartyId, 1, eProcessConfParty);
	COsQueue* queue2 = new COsQueue(ePartyId, 2, eProcessConfParty);
	COsQueue* queue3 = new COsQueue(ePartyId, 3, eProcessConfParty);

	EventPackage::Manager& EventPackage = EventPackage::Manager::Instance();

	EventPackage.AddSubscriber(PARTY_ID2, eEventType_UserAdded, std::make_pair(queue1, 101));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_UserDeleted, std::make_pair(queue1, 101));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_UserDisplayTextUpdated, std::make_pair(queue1, 101));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_EndpointAdded, std::make_pair(queue1, 101));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_EndpointDeleted, std::make_pair(queue1, 101));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_EndpointDisplayTextUpdated, std::make_pair(queue1, 102));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_EndpointStatusUpdated, std::make_pair(queue1, 102));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_MediaAdded, std::make_pair(queue1, 102));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_MediaDeleted, std::make_pair(queue1, 102));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_MediaDisplayTextUpdated, std::make_pair(queue1, 102));
	EventPackage.AddSubscriber(PARTY_ID2, eEventType_MediaStatusUpdated, std::make_pair(queue1, 102));

	EventPackage.AddSubscriber(PARTY_ID2, eEventType_EndpointAdded, std::make_pair(queue1, 102));

	//DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Real.xml", 0);
	MergeXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Real.xml", "Processes/ConfParty/Tests/TestLync.Sample.RealUpdated.xml", PARTY_ID2);

	LyncMsi lyncMsi[] = { 2000000, 4000000, 8000000 };
	EventPackage::ApiLync::Instance().AddDSH(PARTY_ID2, lyncMsi, sizeof(lyncMsi)/sizeof(LyncMsi));

	delete queue1;
	delete queue2;
	delete queue3;

	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
	EP_TRACE << "\nOK";
}

//--------------------------------------------------------------------------
void CTestLyncApi::testXmlLoadSample()
{
#if 0
	DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Polycom.xml", 0);
	DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Basic.xml", 0);
	DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Real.xml", 0);
	DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.RealUpdated.xml", 0);
	DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Rich.xml", 0);
	DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.RichUpdated.xml", 0);
	DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.ConfInvite.xml", 1);
	DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.AddUser.Request.xml", 2);
#endif
	//DumpXmlFile("Processes/ConfParty/Tests/TestLync.Sample.ConfInvite.xml", 1);
}

//--------------------------------------------------------------------------
void CTestLyncApi::testXmlMergeSample()
{
#if 0
	MergeXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Real.xml", "Processes/ConfParty/Tests/TestLync.Sample.RealUpdated.xml", PARTY_ID1);
	MergeXmlFile("Processes/ConfParty/Tests/TestLync.Sample.Rich.xml", "Processes/ConfParty/Tests/TestLync.Sample.RichUpdated.xml", PARTY_ID2);
#endif
}

//--------------------------------------------------------------------------
size_t CTestLyncApi::ReadXmlFile(char* xmlFile, char** xmlBuffer)
{
	const size_t lFileSize = GetFileSize(xmlFile);

	std::ifstream ifs;
	ifs.open(xmlFile, std::ios::in);

	if (ifs)
	{
		*xmlBuffer = new char[lFileSize+1];
		memset(*xmlBuffer, 0, lFileSize+1);

		ifs.read(*xmlBuffer, lFileSize);
		ifs.close();
		return lFileSize;
	}
	return 0;
}

//--------------------------------------------------------------------------
void CTestLyncApi::DumpXmlFile(char* xmlFile, int xmlType)
{
	char* xmlBuffer = NULL;
	ApiBaseObject* pXml = NULL;
	switch (xmlType)
	{
		case 0: pXml = new EventPackage::Conference(); break;
		case 1: pXml = new EventPackage::ConfInvite(); break;
		case 2: pXml = new EventPackage::Request(); break;
		case 3: pXml = new EventPackage::Response(); break;
	}

	size_t xmlLen = ReadXmlFile(xmlFile, &xmlBuffer);
	if (xmlLen)
	{
		pXml->ReadFromXmlStream(xmlBuffer, xmlLen);
		delete[] xmlBuffer;
	}
	EP_TRACE << "\n";
	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
	EP_TRACE << "\nXXX " << xmlFile;
	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
	EP_TRACE << "\n" << *pXml;
	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

//	LyncConfInvite* p = (LyncConfInvite*)pXml;
//	EP_TRACE << "\nm_version:" << p->m_version;
//	EP_TRACE << "\nm_focusUri:" << p->m_focusUri;
//	EP_TRACE << "\nm_conversationId:" << p->m_conversationId;
//	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

	delete pXml;
}

//--------------------------------------------------------------------------
void CTestLyncApi::MergeXmlFile(char* xmlFile1, char* xmlFile2, DWORD partyId)
{
	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

	char* xmlBuffer = NULL;
	size_t xmlLen = 0;

	EventPackage::Manager& lyncEventManager = EventPackage::Manager::Instance();

	//DumpXmlFile(xmlFile1, 0);
	xmlLen = ReadXmlFile(xmlFile1, &xmlBuffer);
	if (xmlLen)
	{
		lyncEventManager.AddConference(partyId, "dummyCallId", xmlBuffer, xmlLen);
		delete[] xmlBuffer;
	}

	//DumpXmlFile(xmlFile2, 0);
	xmlLen = ReadXmlFile(xmlFile2, &xmlBuffer);
	if (xmlLen)
	{
		lyncEventManager.AddConference(INVALID, "dummyCallId", xmlBuffer, xmlLen);
		delete[] xmlBuffer;
	}
	const EventPackage::Conference* pConference = lyncEventManager.GetConference(partyId);

//	EP_TRACE << "\n";
//	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
//	EP_TRACE << "\nXXX " << xmlFile1 << " + " << xmlFile2;
//	EP_TRACE << "\nXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
//	EP_TRACE << "\n" << *pConferenceInfo;
}

