#ifndef TEST_PROCESS_BASE_H
#define TEST_PROCESS_BASE_H

#include <cppunit/extensions/HelperMacros.h>

class TestProcessBase : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestProcessBase );
	CPPUNIT_TEST( testValidateString );
	CPPUNIT_TEST_SUITE_END();
	
public:

	void testValidateString();

};

#endif // TEST_PROCESS_BASE_H
