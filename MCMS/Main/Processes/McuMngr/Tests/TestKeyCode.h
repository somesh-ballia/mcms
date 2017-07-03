#ifndef __TEST_KEYCODE_H__
#define __TEST_KEYCODE_H__

#include <cppunit/extensions/HelperMacros.h>

class CTestKeyCode   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestKeyCode );
	CPPUNIT_TEST( testKeyCodeGeneration );
	CPPUNIT_TEST( testValidateKeyCodeOption );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testKeyCodeGeneration();
	void testValidateKeyCodeOption();
};

#endif

