#ifndef TESTCSMNGRMANAGER_H_
#define TESTCSMNGRMANAGER_H_



// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>




class CTestCSMngrManager   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestCSMngrManager );
	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testAddIpService );
//	CPPUNIT_TEST( testUpdateIpService );
//	CPPUNIT_TEST( testDeleteIpService );
	//...
	CPPUNIT_TEST_SUITE_END();	
public:
	CTestCSMngrManager();
	virtual ~CTestCSMngrManager();
	
	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testAddIpService();
	void testUpdateIpService();
	void testDeleteIpService();
};

#endif /*TESTCSMNGRMANAGER_H_*/
