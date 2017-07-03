
#ifndef _TEST_AUDIO_RSRC_INTERFACE
#define _TEST_AUDIO_RSRC_INTERFACE

// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

class CAudioRsrcInterface ;

class CTestAudioRsrcInterface : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestAudioRsrcInterface );
	//CPPUNIT_TEST(  );

	//...
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown(); 
private:
	CAudioRsrcInterface * m_audioInterface;
};

#endif /* _TEST_AUDIO_RSRC_INTERFACE */


