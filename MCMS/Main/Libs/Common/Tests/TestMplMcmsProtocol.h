
#ifndef TESTMPLMCMSPROTOCOL_H
#define TESTMPLMCMSPROTOCOL_H


#include <cppunit/extensions/HelperMacros.h>

class TestMplMcmsProtocol : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestMplMcmsProtocol );
	CPPUNIT_TEST( testConstructor );	
	CPPUNIT_TEST( testSerializeDeSerialzeMplApi );
	CPPUNIT_TEST( testSerializeDeSerialzeCSApi );
	CPPUNIT_TEST( testSerializeDeSerialzeTrace );
	CPPUNIT_TEST( TestOperatorAssignment );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testConstructor();
	
	

	void testSerializeDeSerialzeMplApi();
	void testSerializeDeSerialzeCSApi();
	void testSerializeDeSerialzeTrace();
	void TestOperatorAssignment();
    
};

#endif  // TESTMPLMCMSPROTOCOL_H
