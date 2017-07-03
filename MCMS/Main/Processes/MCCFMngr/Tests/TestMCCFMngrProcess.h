// TestMCCFMngrProcess.h: interface for the CTestTestProcess class.
// Unit tests using TDD of the MCCFMngr Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_MCCFMNGR_H)
#define TEST_MCCFMNGR_H

// part of official UnitTest library
//#pragma warning(disable:4786)
#include <cppunit/extensions/HelperMacros.h>

class CTestMCCFMngrProcess : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestMCCFMngrProcess);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testApiBaseObject);
	CPPUNIT_TEST(testSYNC);
	CPPUNIT_TEST(testIvrMessage);
	CPPUNIT_TEST_SUITE_END();

public:

	virtual void setUp();
	virtual void tearDown();

	void testConstructor();

	void testApiBaseObject();

	void testSYNC();
	void testIvrMessage();
};

#endif // !defined(TEST_MCCFMNGR_H)
