
#ifndef TESTSEGEMENT_H
#define TESTSEGEMENT_H


#include <cppunit/extensions/HelperMacros.h>

class TestSegment: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestSegment );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( test2Strings );
	CPPUNIT_TEST( testStringNumStringNum );
	CPPUNIT_TEST( testPutDynamicSize );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testConstructor();
	void test2Strings();
	void testStringNumStringNum();
	void testPutDynamicSize();

};

#endif  // TESTSEGMENT
