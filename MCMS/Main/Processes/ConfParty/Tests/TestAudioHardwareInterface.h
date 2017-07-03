
#ifndef _TEST_AUDIO_HARSWARE_INTERFACE
#define _TEST_AUDIO_HARSWARE_INTERFACE

// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

class CAudioHardwareInterface ;

class CTestAudioHardwareInterface : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestAudioHardwareInterface );
	CPPUNIT_TEST( TestSendOpenPort );

	//...
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown(); 
	void TestSendOpenPort();
	
private:
	CAudioHardwareInterface * m_audioInterface;
};

#endif /* _TEST_AUDIO_HARSWARE_INTERFACE */

