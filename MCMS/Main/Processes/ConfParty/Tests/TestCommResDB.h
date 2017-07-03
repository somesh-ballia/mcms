#ifndef TEST_COMMRES_DB_H
#define TEST_COMMRES_DB_H

#include <cppunit/extensions/HelperMacros.h>

class CCommResDB;
class CTestCommResDB   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestCommResDB );
	CPPUNIT_TEST(TestAddRes);
	CPPUNIT_TEST(TestUpdate);
	CPPUNIT_TEST(TestCancel);
	CPPUNIT_TEST(TestSerializeXml);
	CPPUNIT_TEST(TestAssignOperator);
	CPPUNIT_TEST(TestGetEQOrigionResrvName);
	
	//...
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown(); 
	
	void TestAddRes();
	void TestUpdate();
	void TestCancel();
	void TestSerializeXml();
	void TestAssignOperator();
	void TestGetEQOrigionResrvName();
private:
	 CCommResDB * m_pCommResDB;
	 std::string m_testFolder;
};

#endif

