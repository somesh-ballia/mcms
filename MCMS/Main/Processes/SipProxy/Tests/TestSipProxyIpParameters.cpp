// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestSipProxyIpParameters.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "Segment.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestSipProxyIpParameters );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestSipProxyIpParameters::setUp()
{
	m_pServiceParams = NULL;

	m_NewServiceIpParams.AlternateProxyAddress.addr.v4.ip = 0;
	m_NewServiceIpParams.AlternateProxyAddress.port = 0;
	m_NewServiceIpParams.Dhcp = NO;
	m_NewServiceIpParams.DNSStatus = eServerStatusOff;
	
//	m_NewServiceIpParams.IpAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.172.141", eHost);
	
	mcTransportAddress tmpIpAddr;
	::stringToIp(&tmpIpAddr,"172.22.172.141");// my pizza(central signaling) ip
//	newServiceIpParams.IpAddressIpV4.addr.v4.ip =  tmpIpAddr.addr.v4.ip;
//	newServiceIpParams.IpAddressIpV4.ipVersion  = eIpVersion4;
	m_NewServiceIpParams.pAddrList[0].addr.v4.ip =  tmpIpAddr.addr.v4.ip;
	m_NewServiceIpParams.pAddrList[0].ipVersion  = eIpVersion4;
	
	m_NewServiceIpParams.OutboundProxyAddress.addr.v4.ip = SystemIpStringToDWORD("172.22.187.159", eHost);
	m_NewServiceIpParams.OutboundProxyAddress.port = 5060;
	strcpy(m_NewServiceIpParams.pAltProxyHostName, "");
	strcpy(m_NewServiceIpParams.pAltProxyName, "");
	strcpy(m_NewServiceIpParams.pOutboundProxyName, "");
	strcpy(m_NewServiceIpParams.pProxyHostName, "ip.co.il");
	strcpy(m_NewServiceIpParams.pProxyName, "");
	m_NewServiceIpParams.ProxyAddress.addr.v4.ip  = SystemIpStringToDWORD("172.22.187.159", eHost);
	m_NewServiceIpParams.ProxyAddress.port = 5060;
	m_NewServiceIpParams.refreshTout = 3600;
	m_NewServiceIpParams.RegistrationFlags = 0xFF;
	m_NewServiceIpParams.serversConfig = eConfSipServerManually;
	m_NewServiceIpParams.ServiceId = 12;
	strcpy(m_NewServiceIpParams.ServiceName, "Dummy LCS");
	m_NewServiceIpParams.transportType = eTransportTypeTcp;

	m_pServiceParams = new CSipProxyIpParams(m_NewServiceIpParams);
	
	SystemSleep(10,FALSE);
}

//////////////////////////////////////////////////////////////////////
void CTestSipProxyIpParameters::tearDown()
{
	SystemSleep(10,FALSE);
	POBJDELETE(m_pServiceParams);
}

//////////////////////////////////////////////////////////////////////
void CTestSipProxyIpParameters::testConstructor()
{
	POBJDELETE(m_pServiceParams);
	m_pServiceParams = new CSipProxyIpParams;
	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyIpParameters::testConstructor ", m_pServiceParams != NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestSipProxyIpParameters::testConstructorWithStruct()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyIpParameters::testConstructorWithStruct ", m_pServiceParams != NULL );  
} 

//////////////////////////////////////////////////////////////////////
void CTestSipProxyIpParameters::testGetServiceName()
{
	CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong serice Name ", (std::string)"Dummy LCS", (std::string)m_pServiceParams->GetServiceName() );  
} 

//////////////////////////////////////////////////////////////////////
void CTestSipProxyIpParameters::testGetServiceId()
{
	CPPUNIT_ASSERT_MESSAGE( "wrong service Id ", m_pServiceParams->GetServiceId() == 12 );  
} 

//////////////////////////////////////////////////////////////////////
void CTestSipProxyIpParameters::testGetRefreshTout()
{
	CPPUNIT_ASSERT_MESSAGE( "testGetRefreshTout - wrong Refresh tout ", m_pServiceParams->GetRefreshTout() == 3600 );  
} 

//////////////////////////////////////////////////////////////////////
void CTestSipProxyIpParameters::testDeserialize()
{
	POBJDELETE(m_pServiceParams);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&m_NewServiceIpParams,sizeof(SIP_PROXY_IP_PARAMS_S));

	m_pServiceParams = new CSipProxyIpParams;
	m_pServiceParams->Deserialize(pSeg);

	CPPUNIT_ASSERT_EQUAL_MESSAGE( "CTestSipProxyIpParameters::testDeserialize, wrong serice Name ", (std::string)"Dummy LCS", (std::string)m_pServiceParams->GetServiceName() );  
	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyIpParameters::testDeserialize, wrong Refresh tout ", m_pServiceParams->GetRefreshTout() == 3600 );  
	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyIpParameters::testDeserialize, wrong registration state ", m_pServiceParams->IsRegEQs() != FALSE );  
}

