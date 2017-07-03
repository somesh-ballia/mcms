

#ifndef TESTMCCFCDR_H_
#define TESTMCCFCDR_H_

#include <cppunit/extensions/HelperMacros.h>

class TestMccfCdr : public CPPUNIT_NS::TestFixture
{

	CPPUNIT_TEST_SUITE( TestMccfCdr );
	CPPUNIT_TEST( testMccfCdrConstractor );
	CPPUNIT_TEST( testMccfCdrSerDes );
	CPPUNIT_TEST_SUITE_END();

public:

	void setUp();
	void tearDown();

	void testMccfCdrSerDes();
	void testMccfCdrConstractor();

};

#endif /* TESTMCCFCDR_H_ */
