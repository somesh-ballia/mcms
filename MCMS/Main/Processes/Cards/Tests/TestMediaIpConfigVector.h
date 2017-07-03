// TestMediaIpConfigVector.h: interface for the CTestMediaIpConfigVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_Cards_H)
#define TEST_Cards_H


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestMediaIpConfigVector   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestMediaIpConfigVector );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testIsEmpty );
	CPPUNIT_TEST( testInsertFirst );
	CPPUNIT_TEST( testInsertAndRemoveByServiceId );
	CPPUNIT_TEST( testInsertAndRemoveByObject );
	CPPUNIT_TEST( testFind );
	CPPUNIT_TEST( testAt );
	CPPUNIT_TEST( testGetFirst );
	CPPUNIT_TEST( testGetNext );
	CPPUNIT_TEST( testGetNextUntilEnd );
	CPPUNIT_TEST( testGetNextOverflow );
	CPPUNIT_TEST( testGetNextWithLoop );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testIsEmpty();
	void testInsertFirst();
	void testInsertAndRemoveByServiceId();
	void testInsertAndRemoveByObject();
	void testFind();
	void testAt();
	void testGetFirst();
	void testGetNext();
	void testGetNextUntilEnd();
	void testGetNextOverflow();
	void testGetNextWithLoop();
};

#endif // !defined(TEST_Cards_H)

