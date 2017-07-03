// TestOperatorList.h: interface for the CTestOperatorList class.
// Unit tests using TDD of the Authentication Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_Operator_List_H)
#define TEST_Operator_List_H


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestOperatorList   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestOperatorList );
	CPPUNIT_TEST( testOperatorListConstructor );
	CPPUNIT_TEST( testOperatorListAddCancelOperator );
	CPPUNIT_TEST( testOperatorListChangeOperPassword );
	CPPUNIT_TEST( testOperatorListSaveToLoadFromFile );
	CPPUNIT_TEST(Test4CharChanged_noChange);
	CPPUNIT_TEST(Test4CharChanged_1Change);
	CPPUNIT_TEST(Test4CharChanged_2Change);
	CPPUNIT_TEST(Test4CharChanged_3Change);
	CPPUNIT_TEST(Test4CharChanged_4Change);
	CPPUNIT_TEST(TestStrongPassword_empty);
	CPPUNIT_TEST(TestPasswordHistory_currentPwd);
	CPPUNIT_TEST(TestPasswordHistory_Log1);
	CPPUNIT_TEST(TestPasswordHistory_Log10);
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown();

	void testOperatorListConstructor();
	void testOperatorListAddCancelOperator();
	void testOperatorListChangeOperPassword();
	void testOperatorListSaveToLoadFromFile();

	//Federal
	void Test4CharChanged_noChange();
	void Test4CharChanged_1Change();
	void Test4CharChanged_2Change();
	void Test4CharChanged_3Change();
	void Test4CharChanged_4Change();
	void TestStrongPassword_empty();
	void TestPasswordHistory_currentPwd();
	void TestPasswordHistory_Log1();
	void TestPasswordHistory_Log10();

};

#endif // !defined(TEST_Operator_List_H)

