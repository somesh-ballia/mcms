#ifndef TESTPARTYCONNECTED_H_
#define TESTPARTYCONNECTED_H_


#include <cppunit/extensions/HelperMacros.h>


class TestPartyConnected : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestPartyConnected );
	CPPUNIT_TEST( testSerializeDeSerialize );
	CPPUNIT_TEST( testPartyDetailedSerializeDeSerialize );
	CPPUNIT_TEST_SUITE_END();
	
public:
	TestPartyConnected();
	virtual ~TestPartyConnected();
	
	void setUp();
	void tearDown();
	
	void testSerializeDeSerialize();
	void testPartyDetailedSerializeDeSerialize();
};

#endif /*TESTPARTYCONNECTED_H_*/
