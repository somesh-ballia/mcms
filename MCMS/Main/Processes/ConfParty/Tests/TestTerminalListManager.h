#ifndef TEST_TERMINAL_LIST_MANAGER_H
#define TEST_TERMINAL_LIST_MANAGER_H

#include <cppunit/extensions/HelperMacros.h>
#include <vector>

class CTerminalNumberingManager;
class CParty;

class CTestTerminalListManager   : public CPPUNIT_NS::TestFixture
{
  typedef std::vector<CParty *> PartiesVector;
    
	CPPUNIT_TEST_SUITE( CTestTerminalListManager );
	CPPUNIT_TEST(TestCtor);
	CPPUNIT_TEST(TestListAdd);
	CPPUNIT_TEST(TestListRemove);
	CPPUNIT_TEST(TestAddRemove);
	CPPUNIT_TEST(TestAddRemoveRandom);
	
	//...
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown(); 
	
	void TestCtor();
	void TestListAdd();
	void TestListRemove();
	void TestAddRemove();
	void TestAddRemoveRandom();
	
private:
	CTerminalNumberingManager * m_pTerminalListManager;
};

#endif
