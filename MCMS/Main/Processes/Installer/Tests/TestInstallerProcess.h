// TestInstallerProcess.h: interface for the CTestTestProcess class.
// Unit tests using TDD of the Installer Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_INSTALLER_H)
#define TEST_INSTALLER_H

// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>
#include "InstallerManager.h"

// private tests (example)

class CTestInstallerProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestInstallerProcess );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testConverteString2Version );
	CPPUNIT_TEST( testConverteString2VersionPrivateVersion );
	CPPUNIT_TEST( testCheckNewVersionValidity );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
    void testConverteString2Version();
    void testConverteString2VersionPrivateVersion();
    void testCheckNewVersionValidity();
    
};

#endif // !defined(TEST_INSTALLER_H)
