#ifndef _TEST_BRIDGEPARTYCNTL_H__
#define _TEST_BRIDGEPARTYCNTL_H__

class CBridgePartyCntl;
class CConf;
class CParty;

// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

class CTestBridgePartyCntl   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestBridgePartyCntl );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testCreate );
	CPPUNIT_TEST( testDestroy );

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testCreate();
	void testDestroy();

private:
	CBridgePartyCntl* m_pBridgePartyCntl;
	CConf*		m_pConf;
	CParty*	m_pParty;
};


#endif /* _TEST_BRIDGEPARTYCNTL_H__ */
