#if !defined(TEST_PROCESS_SETTINGS_H)
#define TEST_PROCESS_SETTINGS_H



// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>




class CTestProcessSettings   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestProcessSettings );
    
    CPPUNIT_TEST(testSetGetProcessSettings);
    
    CPPUNIT_TEST(testGetSetRemove);
    
	CPPUNIT_TEST_SUITE_END();
public:
    
    void testSetGetProcessSettings();
    void testGetSetRemove();
};

#endif // !defined(TEST_PROCESS_SETTINGS_H)
