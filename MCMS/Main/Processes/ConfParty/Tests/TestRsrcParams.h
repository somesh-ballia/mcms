#if !defined(TEST_RSRCPARAMS_H)
#define TEST_RSRCPARAMS_H


#include <cppunit/extensions/HelperMacros.h>

#include "RsrcParams.h"

class CTestRsrcParams   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestRsrcParams );
	CPPUNIT_TEST( testConstructor);
	CPPUNIT_TEST( testSetConfRsrcID );
	CPPUNIT_TEST( testSetPartyRsrcID );
	CPPUNIT_TEST( testSetConnectionID );
	CPPUNIT_TEST( testSetLRT );

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testSetConfRsrcID();
	void testSetPartyRsrcID();
	void testSetConnectionID();
	void testSetLRT();

	CRsrcParams* m_pRsrcParams;
	CRsrcParams* m_pRsrcParamsOther;

};

#endif // !defined(TEST_RSRCPARAMS_H)



