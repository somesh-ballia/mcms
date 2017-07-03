#ifndef TESTH221STR_H_
#define TESTH221STR_H_


#include <cppunit/extensions/HelperMacros.h>


class TestH221Str : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestH221Str );
	CPPUNIT_TEST( testSerializeDeSerialize );
	CPPUNIT_TEST_SUITE_END();
public:
	TestH221Str();
	virtual ~TestH221Str();
	
	void setUp();
	void tearDown();
	
	void testSerializeDeSerialize();
};

#endif /*TESTH221STR_H_*/
