#ifndef __TEST_KEYCODE_SAVELOADER_H__
#define __TEST_KEYCODE_SAVELOADER_H__

#include <cppunit/extensions/HelperMacros.h>

class TestKeyCodeSaveLoader : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestKeyCodeSaveLoader );
	CPPUNIT_TEST( testSaveLoadNormal );
	CPPUNIT_TEST( testReset );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testSaveLoadNormal();
	void testReset();
};

#endif 
