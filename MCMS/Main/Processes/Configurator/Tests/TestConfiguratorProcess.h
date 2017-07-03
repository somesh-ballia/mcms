// TestConfiguratorProcess.h: interface for the CTestTestProcess class.
// Unit tests using TDD of the Configurator Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_CONFIG_PROCESS_H)
#define TEST_CONFIG_PROCESS_H

#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestConfiguratorProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestConfiguratorProcess );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testAddIpInterface );    
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
    void testAddIpInterface();
};

#endif // !defined(TEST_CONFIG_PROCESS_H)
