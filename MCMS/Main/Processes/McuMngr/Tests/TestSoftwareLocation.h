
#if !defined(TEST_SoftwareLocation_H)
#define TEST_SoftwareLocation_H

#include <cppunit/extensions/HelperMacros.h>
#include "SoftwareLocation.h"


class CTestSoftwareLocation   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSoftwareLocation );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testSetHostName );
	CPPUNIT_TEST( testSetHostIp );
	CPPUNIT_TEST( testSetLocation );
	CPPUNIT_TEST( testSetUrlType );
	CPPUNIT_TEST( testSetUserName );
	CPPUNIT_TEST( testSetPassword );
	CPPUNIT_TEST( testSetVLanId );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testSetHostName();
	void testSetHostIp();
	void testSetLocation();
	void testSetUrlType();
	void testSetUserName();
	void testSetPassword();
	void testSetVLanId();

	CSoftwareLocation * m_pSoftwareLocation;

};


#endif // !defined(TEST_SoftwareLocation_H)

