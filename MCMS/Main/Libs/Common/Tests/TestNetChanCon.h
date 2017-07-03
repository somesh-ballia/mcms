
#ifndef __TEST_NET_CHAN_CONN_H__
#define __TEST_NET_CHAN_CONN_H__

#include <cppunit/extensions/HelperMacros.h>

class TestNetChanConn : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestNetChanConn );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testSerializeDeserialize );
    CPPUNIT_TEST( testSerializeDeserializeCalling );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testConstructor();
	void testSerializeDeserialize();
    void testSerializeDeserializeCalling();
};


#endif  // __TEST_NET_CHAN_CONN_H__
