
#if !defined(_TESTSHAREDMEMORY_H__)
#define _TESTSHAREDMEMORY_H__

#include <cppunit/extensions/HelperMacros.h>

class CTestSharedMemory : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSharedMemory );
	CPPUNIT_TEST( testConstructor );
	    //CPPUNIT_TEST( test2Tables );
	CPPUNIT_TEST( testTableOverFlow );
	CPPUNIT_TEST( testArrayConstructor );
	    //CPPUNIT_TEST( test2Arrays );
	CPPUNIT_TEST( testArrayOverFlow );
	
	CPPUNIT_TEST_SUITE_END();

public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void test2Tables();
	void testTableOverFlow();
	void testArrayConstructor();
	void test2Arrays();
	void testArrayOverFlow();
};

#endif // !defined(_TESTSHAREDMEMORY_H__)
