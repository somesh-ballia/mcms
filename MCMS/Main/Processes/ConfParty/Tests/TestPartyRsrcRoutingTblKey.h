
#if !defined(TESTPARTYRSRCROUTINGTBLKEY_H)
#define TESTPARTYRSRCROUTINGTBLKEY_H


#include <cppunit/extensions/HelperMacros.h>

#include "ConfPartyRoutingTable.h"

class CTestPartyRsrcRoutingTblKey   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestPartyRsrcRoutingTblKey );
	CPPUNIT_TEST( testConstructor);
	CPPUNIT_TEST( testSetPartyRsrcID );
	CPPUNIT_TEST( testSetConnectionID );
	CPPUNIT_TEST( testSetLRT );
	CPPUNIT_TEST( testOperatorLessWithConnectionId );
	CPPUNIT_TEST( testOperatorLessWithPartyId );


	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testSetPartyRsrcID();
	void testSetConnectionID();
	void testSetLRT();
	void testOperatorLessWithConnectionId();
	void testOperatorLessWithPartyId();

	CPartyRsrcRoutingTblKey* m_pRsrcParams;
	CPartyRsrcRoutingTblKey* m_pRsrcParamsOther;

};

#endif // !defined(TESTPARTYRSRCROUTINGTBLKEY_H)

