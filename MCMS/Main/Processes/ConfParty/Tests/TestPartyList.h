
#if !defined(TEST_PARTYLIST_H)
#define TEST_PARTYLIST_H


#include <cppunit/extensions/HelperMacros.h>

#include "Conf.h"


class CTestPartyList   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestPartyList );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( TestInsert );
	CPPUNIT_TEST( TestInsertFails );
	CPPUNIT_TEST( TestGoThroughPartyList );
	CPPUNIT_TEST( GetPartyConnection );
	CPPUNIT_TEST( GetNullPartyConnectionFail );
	CPPUNIT_TEST( RemoveNullPartyConnectionFail );
	CPPUNIT_TEST( GetAtPartyConnectionFailOutOfBound );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void TestInsert();
	void TestInsertFails();
	void TestGoThroughPartyList();
	void GetPartyConnection();
	void GetNullPartyConnectionFail();
	void RemoveNullPartyConnectionFail();
	void GetAtPartyConnectionFailOutOfBound();

	CPartyList* m_pPartyList;
};

#endif // !defined(TEST_PARTYLIST_H)

