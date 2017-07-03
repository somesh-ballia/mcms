

#if !defined(TEST_GlobalVideoHWUtils_H)
#define TEST_GlobalVideoHWUtils_H


#include "cppunit/extensions/HelperMacros.h"
#include "ConfPartyProcess.h"


class CTestGlobalVideoHWUtils: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestGlobalVideoHWUtils );
	CPPUNIT_TEST( CheckH263GlobalVideoHWUtils);
	CPPUNIT_TEST(CheckH264GlobalVideoHWUtils);

	//...
	CPPUNIT_TEST_SUITE_END();
public:
	
    CTestGlobalVideoHWUtils(){}
	void setUp();
	void tearDown();
	void CheckH263GlobalVideoHWUtils();
	void CheckH264GlobalVideoHWUtils();

private:
	

};

#endif // !defined(TEST_UnifiedComMode_H)

