#if !defined(TEST_SYS_CONFIG_H	)
#define TEST_SYS_CONFIG_H	



// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>




class CTestSysConfig   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSysConfig );
    
    CPPUNIT_TEST(testSetGetSysConfig);
    
	CPPUNIT_TEST_SUITE_END();
public:
    
    void testSetGetSysConfig();
};

#endif // !defined(TEST_SYS_CONFIG_H	)
