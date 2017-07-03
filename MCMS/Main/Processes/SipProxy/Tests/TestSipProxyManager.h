// TestSipProxyManager.h: interface for the CTestTestProcess class.
// Unit tests using TDD of the SipProxyManager
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_SipProxyManager_H)
#define TEST_SipProxyManager_H

// part of official UnitTest library

#include <cppunit/extensions/HelperMacros.h>

#include "SipProxyManager.h"
#include "SipProxyProcess.h"
#include "MockCMplMcmsProtocol.h"

// private tests (example)

class CTestSipProxyManager   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSipProxyManager );
	CPPUNIT_TEST( testConstructor );
	//CPPUNIT_TEST( testIpServiceParamFlow );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	//void testIpServiceParamFlow();

protected:
	CSipProxyServiceManager*	m_pSipProxyManager;
	CSipProxyProcess*	m_pSipProxyProcess;
	CMockMplMcmsProtocol* m_pMockMplMcmsProtocol;
};

#endif // !defined(TEST_SipProxyManager_H)

