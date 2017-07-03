#ifndef TESTIFCONFIG_H_
#define TESTIFCONFIG_H_

#include <cppunit/extensions/HelperMacros.h>


class CTestIfConfig : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestIfConfig );
	CPPUNIT_TEST( testConstructor );
	//CPPUNIT_TEST( testGetExistNiIpAddr );
	CPPUNIT_TEST( testGetNotExistNiIpAddr );
	CPPUNIT_TEST_SUITE_END();
	
public:
	
	void setUp();
	void tearDown();
	
	void testConstructor();
	void testGetNotExistNiIpAddr();
	void testGetExistNiIpAddr();
	void testIsNiExist();
};

#endif /*TESTIFCONFIG_H_*/
