#include "TestMccfCdr.h"

#include "MscPolycomMixer.h"
#include "EventElement.h"
#include "ParticipantInfoNotify.h"

#include "Segment.h"

#include "TraceStream.h"

#include <iostream>
#include <memory>

//////////////////////////////////////////////////////////////////////
CPPUNIT_TEST_SUITE_REGISTRATION(TestMccfCdr);

//////////////////////////////////////////////////////////////////////
void TestMccfCdr::setUp()
{}
//////////////////////////////////////////////////////////////////////
void TestMccfCdr::tearDown()
{}

//////////////////////////////////////////////////////////////////////
void TestMccfCdr::testMccfCdrConstractor()
{
	MscPolycomMixer mscPolycomMixer;
	mscPolycomMixer.m_version = "1.0";
	std::cout << mscPolycomMixer << '\n';

	std::auto_ptr<MscPolycomMixer> pCopy(mscPolycomMixer.NewCopy());
	CPPUNIT_ASSERT(*pCopy == mscPolycomMixer);
}

//////////////////////////////////////////////////////////////////////
void TestMccfCdr::testMccfCdrSerDes()
{
	std::string partyId("12345");
	std::string confId("23456");
	const size_t numberOfChannels = 2;

	MscPolycomMixer mixer1;
	mixer1.m_version = "1.0";

	EventElement* pEventElem = new EventElement;
	mixer1.m_pEvent = pEventElem;

	ParticipantInfoNotify* pPartyInfoNotify = new ParticipantInfoNotify;
	pEventElem->m_pParticipantInfoNotify = pPartyInfoNotify;

	Channels& channels = pPartyInfoNotify->m_channels;

	for (size_t i = 0; i < numberOfChannels; ++i)
	{
		Channel channel;

		channel.m_resolutionFrameRate.m_max.m_width = 520;
		channel.m_resolutionFrameRate.m_max.m_height = 80;
		channel.m_resolutionFrameRate.m_max.m_frameRate = 20;
		channel.m_resolutionFrameRate.m_min.m_width = 400;
		channel.m_resolutionFrameRate.m_min.m_height = 200;
		channel.m_resolutionFrameRate.m_min.m_frameRate = 5;
		channel.m_totalNumberOfPackets.m_number = 4000;
		channel.m_totalNumberOfLostPackets.m_number = 10;
		channel.m_jitter.m_average = 10;
		channel.m_jitter.m_peak = 100;
		channel.m_latency.m_average = 2;

		channel.m_channelName =  i ? "content" : "video";

		channels.m_channel.push_back(channel);
	}

	pPartyInfoNotify->m_partyCallId = partyId;
	pPartyInfoNotify->m_confId = confId;
	pEventElem->m_subscriptionState = eConferenceInfoSubscriptionState_Active; //is it right?

	CSegment seg;

	std::cout << "mixer:\n" << mixer1;
	seg << mixer1;

	MscPolycomMixer mixer2;
	seg >> mixer2;

	std::cout << "mixer2:\n" << mixer2;

	FTRACEINTO << "mixer1:\n" << mixer1 << "mixer2:\n" << mixer2 << "Ok:" << (mixer1 == mixer2);
	CPPUNIT_ASSERT(mixer1 == mixer2);
}

//////////////////////////////////////////////////////////////////////
