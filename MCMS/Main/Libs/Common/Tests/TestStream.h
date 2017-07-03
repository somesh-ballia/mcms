
#ifndef TESTSTREAM_H
#define TESTSTREAM_H


#include <cppunit/extensions/HelperMacros.h>

class TestStream : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestStream );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testConstructorString );
	CPPUNIT_TEST( testRead );
	CPPUNIT_TEST( testReadFromStdString );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testConstructor();
	void testConstructorString();
	void testRead();
	void testReadFromStdString();
};

#endif  // TESTSTREAM
