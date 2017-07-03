// TestApacheModuleEngine.h: interface for the CTestApacheModuleEngine class.
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_ApacheModuleEngine_H)
#define TEST_ApacheModuleEngine_H



// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestApacheModuleEngine   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestApacheModuleEngine );
	CPPUNIT_TEST( testConnListConstructor );
	CPPUNIT_TEST( testAddRemoveConnection );
	CPPUNIT_TEST( testFindActionName );
	CPPUNIT_TEST( testFindTransName );
	CPPUNIT_TEST( TestVirtualDirectoryList );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConnListConstructor();
	void testAddRemoveConnection();
	void testFindActionName();
	void testFindTransName();
};


#endif // !defined(TEST_ApacheModuleEngine_H)

