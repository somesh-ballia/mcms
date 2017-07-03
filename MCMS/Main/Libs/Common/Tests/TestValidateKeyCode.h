#ifndef __TEST_VALIDATE_KEYCODE_H__
#define __TEST_VALIDATE_KEYCODE_H__

#include <cppunit/extensions/HelperMacros.h>

class TestValidateKeyCode : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestValidateKeyCode );
	CPPUNIT_TEST( testBitOperators );
	CPPUNIT_TEST( testValidateGoodKeyCode );
	CPPUNIT_TEST( testValidateBadKeyCode );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testBitOperators();
	void testValidateGoodKeyCode();
	void testValidateBadKeyCode();
};

#endif 
