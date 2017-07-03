
#if !defined(TEST_Authentication_H)
#define TEST_Authentication_H


#include <cppunit/extensions/HelperMacros.h>
#include "Authentication.h"


class CTestAuthentication   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestAuthentication );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testSetSerialNumber );
	CPPUNIT_TEST( testSetPlatformType );
	CPPUNIT_TEST( testSetMcuVersion );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testSetSerialNumber();
	void testSetPlatformType();
	void testSetMcuVersion();

	CAuthentication * m_pAuthentication;

};


#endif // !defined(TEST_Authentication_H)

