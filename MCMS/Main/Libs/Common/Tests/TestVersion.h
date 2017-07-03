#ifndef TESTVERSION_H
#define TESTVERSION_H

#include <cppunit/extensions/HelperMacros.h>

class TestVersion : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestVersion );
	CPPUNIT_TEST( testFileNameStrip );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testFileNameStrip();
};

#endif 
