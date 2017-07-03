#ifndef __TEST_PRODUCT_TYPE_DECIDER_H__
#define __TEST_PRODUCT_TYPE_DECIDER_H__

#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class TestProductTypeDecider : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestProductTypeDecider );
	CPPUNIT_TEST( testIfSkipCmdLine );    
	CPPUNIT_TEST( testProductType );    
	CPPUNIT_TEST( testGetCmdLinePrefix );    
	CPPUNIT_TEST( testGetCmdLine );    
	CPPUNIT_TEST( testGetCycleCmdLine );    
	CPPUNIT_TEST( testGetMountCmdLine );    
	CPPUNIT_TEST( testGetUnmountCmdLine );    
	CPPUNIT_TEST( testFirmwareCheckCmdLine );    
	CPPUNIT_TEST( testIfSkipAddNICRoutingTableRule );    
	CPPUNIT_TEST( testGetSimulatedFlashSize );    
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
    void testIfSkipCmdLine();
    void testProductType();
    void testGetCmdLinePrefix();
    void testGetCmdLine();
    void testGetCycleCmdLine();
    void testGetMountCmdLine();
    void testGetUnmountCmdLine();
    void testFirmwareCheckCmdLine();
    void testIfSkipAddNICRoutingTableRule();
    void testGetSimulatedFlashSize();
};

#endif

