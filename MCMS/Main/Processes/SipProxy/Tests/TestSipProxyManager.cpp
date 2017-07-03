// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestSipProxyManager.h"
#include "Trace.h"
#include "SystemFunctions.h"

#include "SIPProxyIpParameters.h"
#include "Segment.h"
#include "IpCsOpcodes.h"



CPPUNIT_TEST_SUITE_REGISTRATION( CTestSipProxyManager );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestSipProxyManager::setUp()
{
	/*m_pSipProxyProcess = new CSipProxyProcess;
	m_pSipProxyProcess->SetUp();*/

	m_pMockMplMcmsProtocol = new CMockMplMcmsProtocol;
	m_pSipProxyManager = new CSipProxyServiceManager(m_pMockMplMcmsProtocol);
	SystemSleep(10,FALSE);
}

//////////////////////////////////////////////////////////////////////
void CTestSipProxyManager::tearDown()
{
	SystemSleep(10,FALSE);
/*	m_pSipProxyProcess->TearDown();
	delete m_pSipProxyProcess;*/
	
	POBJDELETE(m_pSipProxyManager);
	POBJDELETE(m_pMockMplMcmsProtocol);
}

//////////////////////////////////////////////////////////////////////
void CTestSipProxyManager::testConstructor()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyManager::testConstructor ", m_pSipProxyManager != NULL );  
} 

//////////////////////////////////////////////////////////////////////
/*void CTestSipProxyManager::testIpServiceParamFlow()
{
	SIP_PROXY_IP_PARAMS_S NewServiceIpParams;
	NewServiceIpParams.AlternateProxyIp = 0;
	NewServiceIpParams.AlternateProxyPort = 0;
	NewServiceIpParams.Dhcp = NO;
	NewServiceIpParams.DNSStatus = eServerStatusOff;
	NewServiceIpParams.Ip = SystemIpStringToDWORD("172.22.172.141");
	NewServiceIpParams.OutboundProxyIp = SystemIpStringToDWORD("172.22.187.159");
	NewServiceIpParams.OutboundProxyPort = 5060;
	strcpy(NewServiceIpParams.pAltProxyHostName, "");
	strcpy(NewServiceIpParams.pAltProxyName, "");
	strcpy(NewServiceIpParams.pOutboundProxyName, "");
	strcpy(NewServiceIpParams.pProxyHostName, "ip.co.il");
	strcpy(NewServiceIpParams.pProxyName, "");
	NewServiceIpParams.ProxyIp  = SystemIpStringToDWORD("172.22.187.159");
	NewServiceIpParams.ProxyPort = 5060;
	NewServiceIpParams.refreshTout = 3600;
	NewServiceIpParams.RegistrationFlags = 0xFF;
	NewServiceIpParams.serversConfig = eConfSipServerManually;
	NewServiceIpParams.ServiceId = 12;
	strcpy(NewServiceIpParams.ServiceName, "Dummy LCS");
	NewServiceIpParams.transportType = sipTrTcp;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&NewServiceIpParams,sizeof(SIP_PROXY_IP_PARAMS_S));

	m_pSipProxyManager->OnIpServiceParamIndSetup(pSeg);
	m_pSipProxyManager->OnIpServiceParamEnd(pSeg);

	CSegment* pSeg2 = new CSegment;
	*pSeg2	<< (DWORD)1 //confID
			<< "Conf1"	//confName
			<< (BYTE)TRUE		//on-going
			<< (BYTE)FALSE	//MR
			<< (BYTE)FALSE	//EQ
			<< (DWORD)1800;	//duration in seconds
	
	m_pSipProxyManager->OnAddConf(pSeg2);

	DWORD opcode = 0;
	DWORD dataLen = 0;
	m_pMockMplMcmsProtocol->Varify_AddCommonHeader_WasCalled1(opcode);
	char* data = m_pMockMplMcmsProtocol->Varify_AddData_WasCalled1(dataLen);
	m_pMockMplMcmsProtocol->Varify_SendMsgToCSApiCommandDispatcher_WasCalled();

	CPPUNIT_ASSERT_MESSAGE( "testIpServiceParamFlow, wrong opcode ", opcode == SIP_CS_SIG_REGISTER_REQ );  
	CPPUNIT_ASSERT_MESSAGE( "testIpServiceParamFlow, wrong data length ", dataLen == sizeof(SIP_PROXY_IP_PARAMS_S) );
}*/

