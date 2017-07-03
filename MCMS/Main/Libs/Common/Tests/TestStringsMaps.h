
#ifndef TESTSTRINGSMAPS_H
#define TESTSTRINGSMAPS_H

#include <cppunit/extensions/HelperMacros.h>

class TestStringsMaps : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestStringsMaps );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testConstructor();
};

#endif  // TESTSTRINGSMAPS
