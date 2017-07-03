
#if !defined(TEST_SipProxyIpParameters_H)
#define TEST_SipProxyIpParameters_H

#include <cppunit/extensions/HelperMacros.h>

#include "SIPProxyIpParameters.h"


class CTestSipProxyIpParameters   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSipProxyIpParameters );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testConstructorWithStruct );
	CPPUNIT_TEST( testGetServiceName );
	CPPUNIT_TEST( testGetServiceId );
	CPPUNIT_TEST( testGetRefreshTout );
	CPPUNIT_TEST( testDeserialize );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testConstructorWithStruct();
	void testGetServiceName();
	void testGetServiceId();
	void testGetRefreshTout();
	void testDeserialize();

protected:
	CSipProxyIpParams* m_pServiceParams;
	SIP_PROXY_IP_PARAMS_S m_NewServiceIpParams;
};

#endif // !defined(TEST_SipProxyIpParameters_H)

