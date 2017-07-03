
#ifndef _TEST_BRIDGEPARTY_MEDIA_UNIDERECTION_H__
#define _TEST_BRIDGEPARTY_MEDIA_UNIDERECTION_H__

#include <cppunit/extensions/HelperMacros.h>

class CTestBridgePartyMediaUniDirection   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestBridgePartyMediaUniDirection );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testCreate );

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testCreate();

};

#endif /* _TEST_BRIDGEPARTY_MEDIA_UNIDERECTION_H__ */

