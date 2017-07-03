
#if !defined(TEST_MFA_H)
#define TEST_MFA_H


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestMFA   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestMFA );
	CPPUNIT_TEST( testConstructor );
	//CPPUNIT_TEST( testMFAList );	
	//CPPUNIT_TEST( testMFAAllocAndDealloc );		

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	void testConstructor();
	//void testMFAList();
	//void testMFAAllocAndDealloc();

};


#endif // !defined(TEST_MFA_H)

