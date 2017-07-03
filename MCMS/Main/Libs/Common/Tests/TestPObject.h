
#ifndef TESTPOBJECT_H
#define TESTPOBJECT_H


#include <cppunit/extensions/HelperMacros.h>

class TestPObject : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestPObject );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testNewDelete );
	CPPUNIT_TEST( testValidFlag );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testConstructor();
	void testNewDelete();
	void testValidFlag();
};

#endif  // TESTPOBJECT
