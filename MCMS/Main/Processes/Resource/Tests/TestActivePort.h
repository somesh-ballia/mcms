
#if !defined(TEST_ACTIVE_PORT_H)
#define TEST_ACTIVE_PORT_H


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestActivePort   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestActivePort );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testPartyId );	
//	CPPUNIT_TEST( testSystemResourcesCons );		

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	void testConstructor();
	void testPartyId();
	void testSystemResourcesCons();

};


#endif // !defined(TEST_ACTIVE_PORT_H)

