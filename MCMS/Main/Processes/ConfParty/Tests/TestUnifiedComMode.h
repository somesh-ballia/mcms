

#if !defined(TEST_UnifiedComMode_H)
#define TEST_UnifiedComMode_H


#include "cppunit/extensions/HelperMacros.h"
#include "UnifiedComMode.h"
#include "ConfPartyProcess.h"


class CTestUnifiedComMode   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestUnifiedComMode );
	CPPUNIT_TEST( CheckEmptyUnifiedComMode );
	CPPUNIT_TEST(TestConfType);
	CPPUNIT_TEST(TestCallRate);
	CPPUNIT_TEST(TestAudioMode);
	CPPUNIT_TEST(TestFECCMode);
	CPPUNIT_TEST(TestH264VideoMode);
	CPPUNIT_TEST(TestH263VideoMode);
	CPPUNIT_TEST(TestH261VideoMode);
	CPPUNIT_TEST(TestVideoFlags);

	//...
	CPPUNIT_TEST_SUITE_END();
public:
	
    CTestUnifiedComMode(){;}
	void setUp();
	void tearDown();
	void CheckEmptyUnifiedComMode();
	void TestCallRate();
	void TestConfType();
	void TestAudioMode();
	void TestFECCMode();
	void TestH264VideoMode();
	void TestH263VideoMode();
	void TestH261VideoMode();
	void TestVideoFlags();

private:
	

};

#endif // !defined(TEST_UnifiedComMode_H)

