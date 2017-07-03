
#if !defined(TEST_Faults_H)
#define TEST_Faults_H


#include <cppunit/extensions/HelperMacros.h>


class CTestFaultsProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestFaultsProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_Faults_H)

