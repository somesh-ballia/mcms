
#ifndef TEST_OBJSTRING_H
#define TEST_OBJSTRING_H


#include <cppunit/extensions/HelperMacros.h>

class CTestObjString : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestObjString );
	CPPUNIT_TEST( testReplaceChar );
    CPPUNIT_TEST( testRemoveChar );
    CPPUNIT_TEST( testReverse );
    CPPUNIT_TEST( testCopyStr );
    CPPUNIT_TEST( testSpecialChar );
	CPPUNIT_TEST_SUITE_END();
	
public:

	
	void testReplaceChar();
    void testRemoveChar();
    void testReverse();
    void testCopyStr();
    void testSpecialChar();
};

#endif  // TEST_OBJSTRING_H
