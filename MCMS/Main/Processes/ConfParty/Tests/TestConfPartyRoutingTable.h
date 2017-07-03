#if !defined(TEST_CONFPARTYROUTINGTABLE_H)
#define TEST_CONFPARTYROUTINGTABLE_H


#include <cppunit/extensions/HelperMacros.h>
#include "ConfPartyRoutingTable.h"

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern void SetCounterIterationLessThanForTest( int );
extern int GetCounterIterationLessThanForTest( );

// private tests (example)
#include "TaskApi.h"


class CTestConfPartyRoutingTable   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestConfPartyRoutingTable );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testInsert );
	CPPUNIT_TEST( testFind );
	CPPUNIT_TEST( testFindByConnectionId );
	CPPUNIT_TEST( testFindByPartyIDandLRT );
//	CPPUNIT_TEST( testFindByPartyID );
	CPPUNIT_TEST( testFindByConfID );
	CPPUNIT_TEST( testFindByConfIDFails );
	CPPUNIT_TEST( testAddRsrcPointerFails);
	CPPUNIT_TEST( testRemoveStateMachinePointer );
	CPPUNIT_TEST( testRemoveAllPartyRsrcs );
	CPPUNIT_TEST( testGetRsrcDesc );
	CPPUNIT_TEST( testGetAllPartyRsrcs );
	CPPUNIT_TEST( testFindByWhenThere3Entries );
	CPPUNIT_TEST( testFindByPartyIdWhenThere3EntriesBackward );
	CPPUNIT_TEST( testFindByPartyIdWhenThere3Parties );
	CPPUNIT_TEST( testFindByPartyIdWhenThereManyConf );
	CPPUNIT_TEST( testSkipConfNull );
	CPPUNIT_TEST( testIterationsForFind );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testInsert();
	void testFind();
	void testFindByConnectionId();
	void testFindByPartyIDandLRT();
	void testFindByPartyID();
	void testFindByConfID();
	void testFindByConfIDFails();
	void testAddRsrcPointerFails();
	void testRemoveStateMachinePointer();
	void testRemoveAllPartyRsrcs();
	void testGetRsrcDesc();
	void testGetAllPartyRsrcs();
	void testFindByWhenThere3Entries();
	void testFindByPartyIdWhenThere3EntriesBackward();
	void testFindByPartyIdWhenThere3Parties();
	void testFindByPartyIdWhenThereManyConf();
	void testSkipConfNull();
	void testIterationsForFind();

	CConfPartyRoutingTable* m_pConfPartyRoutingTable;
	CTaskApi * m_pTaskApi1;
	CTaskApi * m_pTaskApi2;
	CTaskApi * m_pTaskApi3;
	
	COsQueue * m_queue1;
	COsQueue * m_queue2;
	COsQueue * m_queue3;
};

#endif // !defined(TEST_CONFPARTYROUTINGTABLE_H)

