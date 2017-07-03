#include "TestCallAndChannelInfo.h"
#include "Trace.h"
#include "SystemFunctions.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestCall );
CPPUNIT_TEST_SUITE_REGISTRATION( CTestChannel );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void CTestCall::setUp()
{
	m_pCall = new CCall;
	
}
//////////////////////////////////////////////////////////////////////
void CTestCall::tearDown()
{
	POBJDELETE(m_pCall);
}
//////////////////////////////////////////////////////////////////////
void CTestCall::testConstructor()
{
	CPPUNIT_ASSERT(m_pCall->GetCallIndex() == 0);
	CPPUNIT_ASSERT(m_pCall->GetCallStatus() == 0);
	CPPUNIT_ASSERT(m_pCall->GetConnectionId() == 0);
	CPPUNIT_ASSERT(m_pCall->GetChannelsCounter() == 0);
	for (int i = 0; i < MaxChannelsPerCall; i++)
		CPPUNIT_ASSERT(m_pCall->GetChannelsArray()[i] == NULL);
}
//////////////////////////////////////////////////////////////////////
void CTestCall::testDestructor()
{
	for (int i = 0; i < MaxChannelsPerCall; i++)
		CPPUNIT_ASSERT(! CPObject::IsValidPObjectPtr((m_pCall->GetChannelsArray())[i]));
}
//////////////////////////////////////////////////////////////////////
void CTestCall::testSetCallIndex()
{
	DWORD CallIndex = 5;
	m_pCall->SetCallIndex(CallIndex);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCallIndex ",
		m_pCall->GetCallIndex() == CallIndex);
}
///////////////////////////////////////////////////////////////////////
void CTestCall::testGetCallIndex()
{
	DWORD callIndex = 5;
	m_pCall->SetCallIndex(5);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetCallIndex ",
		m_pCall->GetCallIndex() == callIndex);
}
///////////////////////////////////////////////////////////////////////
void CTestCall::testSetCallStatus()
{
	int CallStatus = -1;
	m_pCall->SetCallStatus(CallStatus);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCallStatus ",
		m_pCall->GetCallStatus() == CallStatus);
}
///////////////////////////////////////////////////////////////////////
void CTestCall::testGetCallStatus()
{
	int CallStatus = 4;
	m_pCall->SetCallStatus(4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCallStatus ",
		m_pCall->GetCallStatus() == CallStatus);
}
////////////////////////////////////////////////////////////////////////
void CTestCall::testSetConnectionId()
{
	WORD conId = 2003;
	m_pCall->SetConnectionId(conId);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetConnectionId ",
		m_pCall->GetConnectionId() == conId);
}
////////////////////////////////////////////////////////////////////////
void CTestCall::testGetConnectionId()
{
	WORD conId = 4678;
	m_pCall->SetConnectionId(4678);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetConnectionId ",
		m_pCall->GetConnectionId() == conId);
}
////////////////////////////////////////////////////////////////////////
void CTestCall::testIncreaseChannelsCounter()
{
	m_pCall->IncreaseChannelsCounter();
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testIncreaseChannelsCounter ",
		m_pCall->GetChannelsCounter() == 1);
}
////////////////////////////////////////////////////////////////////////
void CTestCall::testDecreaseChannelsCounter()
{
	m_pCall->DecreaseChannelsCounter();
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testDecreaseChannelsCounter ",
		m_pCall->GetChannelsCounter() == 0);
	m_pCall->IncreaseChannelsCounter();
	m_pCall->IncreaseChannelsCounter();
	m_pCall->DecreaseChannelsCounter();
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testDecreaseChannelsCounter1 ",
		m_pCall->GetChannelsCounter() == 1);
}
////////////////////////////////////////////////////////////////////////
void CTestCall::testGetChannelsCounter()
{
	m_pCall->IncreaseChannelsCounter();
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetChannelsCounter ",
		m_pCall->GetChannelsCounter() == 1);
}
////////////////////////////////////////////////////////////////////////
void CTestCall::testGetSrcTerminalParams()
{
	mcTerminalParams terParams;
	memset(terParams.partyAddress,0,256);
	DWORD i = 0;
	terParams.endpointType = cmEndpointTypeTerminal;
	terParams.callSignalAddress.ipVersion = eIpVersion4;
	terParams.callSignalAddress.distribution = eDistributionUnicast;
	terParams.callSignalAddress.transportType = eTransportTypeTcp;
	terParams.callSignalAddress.port = 80;
	terParams.callSignalAddress.addr.v4.ip = 122112;
	memcpy(terParams.partyAddress,"aaaabbbb",8);
	terParams.partyAddress[9] = '\0';
	m_pCall->SetSrcTerminalParams(terParams);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).endpointType == cmEndpointTypeTerminal);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.addr.v4.ip == 122112);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		strcmp((m_pCall->GetSrcTerminalParams()).partyAddress , "aaaabbbb") == 0);

	terParams.endpointType = cmEndpointTypeTerminal;
	terParams.callSignalAddress.ipVersion = eIpVersion6;
	memset(terParams.partyAddress,0,256);
	memcpy(terParams.partyAddress,"ccccdddd",8);
	terParams.partyAddress[9] = '\0';
	memcpy(terParams.callSignalAddress.addr.v6.ip,"172.1.1.1.1.1",13);
	terParams.callSignalAddress.addr.v6.ip[13] = '\0';
	terParams.callSignalAddress.addr.v6.scopeId = 500;
	m_pCall->SetSrcTerminalParams(terParams);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).endpointType == cmEndpointTypeTerminal);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.ipVersion == eIpVersion6);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		strcmp((char*)(m_pCall->GetSrcTerminalParams()).callSignalAddress.addr.v6.ip, "172.1.1.1.1.1") == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.addr.v6.scopeId == 500);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcTerminalParams ",
		strcmp((m_pCall->GetSrcTerminalParams()).partyAddress,"ccccdddd") == 0);

}
//////////////////////////////////////////////////////////////////
void CTestCall::testSetSrcTerminalParams()
{
	mcTerminalParams terParams;
	memset(terParams.partyAddress,0,256);
	DWORD i = 0;
	terParams.endpointType = cmEndpointTypeTerminal;
	terParams.callSignalAddress.ipVersion = eIpVersion4;
	terParams.callSignalAddress.distribution = eDistributionUnicast;
	terParams.callSignalAddress.transportType = eTransportTypeTcp;
	terParams.callSignalAddress.port = 80;
	terParams.callSignalAddress.addr.v4.ip = 122112;
	memcpy(terParams.partyAddress,"aaaabbbb",8);
	terParams.partyAddress[9] = '\0';
	m_pCall->SetSrcTerminalParams(terParams);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).endpointType == cmEndpointTypeTerminal);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSrcTerminalParams ",
		(m_pCall->GetSrcTerminalParams()).callSignalAddress.addr.v4.ip == 122112);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSrcTerminalParams ",
		strcmp((m_pCall->GetSrcTerminalParams()).partyAddress , "aaaabbbb") == 0);
}
/////////////////////////////////////////////////////////////////////
void CTestCall::testGetDestTerminalParams()
{
	mcTerminalParams terParams;
	memset(terParams.partyAddress,0,256);
	DWORD i = 0;
	terParams.endpointType = cmEndpointTypeTerminal;
	terParams.callSignalAddress.ipVersion = eIpVersion4;
	terParams.callSignalAddress.distribution = eDistributionUnicast;
	terParams.callSignalAddress.transportType = eTransportTypeTcp;
	terParams.callSignalAddress.port = 80;
	terParams.callSignalAddress.addr.v4.ip = 122112;
	memcpy(terParams.partyAddress,"aaaabbbb",8);
	terParams.partyAddress[9] = '\0';
	m_pCall->SetDestTerminalParams(terParams);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).endpointType == cmEndpointTypeTerminal);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.addr.v4.ip == 122112);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams ",
		strcmp((m_pCall->GetDestTerminalParams()).partyAddress , "aaaabbbb") == 0);

	terParams.endpointType = cmEndpointTypeTerminal;
	terParams.callSignalAddress.ipVersion = eIpVersion6;
	memset(terParams.partyAddress,0,256);
	memcpy(terParams.partyAddress,"ccccdddd",8);
	terParams.partyAddress[9] = '\0';
	memcpy(terParams.callSignalAddress.addr.v6.ip,"172.1.1.1.1.1",13);
	terParams.callSignalAddress.addr.v6.ip[13] = '\0';
	terParams.callSignalAddress.addr.v6.scopeId = 500;
	m_pCall->SetDestTerminalParams(terParams);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams1 ",
		(m_pCall->GetDestTerminalParams()).endpointType == cmEndpointTypeTerminal);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams1 ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.ipVersion == eIpVersion6);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams1 ",
		strcmp((char*)(m_pCall->GetDestTerminalParams()).callSignalAddress.addr.v6.ip, "172.1.1.1.1.1") == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams1 ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.addr.v6.scopeId == 500);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestTerminalParams1 ",
		strcmp((m_pCall->GetDestTerminalParams()).partyAddress,"ccccdddd") == 0);
}
///////////////////////////////////////////////////////////////
void CTestCall::testSetDestTerminalParams()
{
	mcTerminalParams terParams;
	memset(terParams.partyAddress,0,256);
	DWORD i = 0;
	terParams.endpointType = cmEndpointTypeTerminal;
	terParams.callSignalAddress.ipVersion = eIpVersion4;
	terParams.callSignalAddress.distribution = eDistributionUnicast;
	terParams.callSignalAddress.transportType = eTransportTypeTcp;
	terParams.callSignalAddress.port = 80;
	terParams.callSignalAddress.addr.v4.ip = 122112;
	memcpy(terParams.partyAddress,"aaaabbbb",8);
	terParams.partyAddress[9] = '\0';
	m_pCall->SetDestTerminalParams(terParams);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).endpointType == cmEndpointTypeTerminal);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestTerminalParams ",
		(m_pCall->GetDestTerminalParams()).callSignalAddress.addr.v4.ip == 122112);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestTerminalParams ",
		strcmp((m_pCall->GetDestTerminalParams()).partyAddress , "aaaabbbb") == 0);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetSourceInfoAlias()
{
	char* alias = new char[MaxAddressListSize];
	memset(alias,0,MaxAddressListSize);
	memcpy(alias,"testingAlias",12);
	alias[13] = '\0';
	
	m_pCall->SetSourceInfoAlias("testingAlias");
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSourceInfoAlias ",
		strcmp(m_pCall->GetSourceInfoAlias() , alias) == 0);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetSourceInfoAlias()
{
	char* alias = new char[MaxAddressListSize];
	memset(alias,0,MaxAddressListSize);
	memcpy(alias,"testingAlias",12);
	alias[13] = '\0';
	
	m_pCall->SetSourceInfoAlias(alias);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSourceInfoAlias ",
		strcmp(m_pCall->GetSourceInfoAlias() , alias) == 0);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetSrcInfoAliasType()
{
	DWORD srcAliasType[MaxNumberOfAliases];
	DWORD i = 0;
	for (i = 0;i < MaxNumberOfAliases; i++)
		srcAliasType[i] = i;
	m_pCall->SetSrcInfoAliasType(srcAliasType);
	for (i = 0;i < MaxNumberOfAliases; i++)
		CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSrcInfoAliasType ",
			(m_pCall->GetSrcInfoAliasType())[i] == i);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetSrcInfoAliasType()
{
	DWORD srcAliasType[MaxNumberOfAliases];
	int i = 0;
	for (i = 0;i < MaxNumberOfAliases; i++)
		srcAliasType[i] = i;
	m_pCall->SetSrcInfoAliasType(srcAliasType);
	for (i = 0;i < MaxNumberOfAliases; i++)
		CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSrcInfoAliasType ",
			(m_pCall->GetSrcInfoAliasType())[i] == srcAliasType[i]);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetDestinationInfoAlias()
{
	char* alias = new char[MaxAddressListSize];
	memset(alias,0,MaxAddressListSize);
	memcpy(alias,"testingAlias",12);
	alias[13] = '\0';
	
	m_pCall->SetDestinationInfoAlias(alias);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestinationInfoAlias ",
		strcmp(m_pCall->GetDestinationInfoAlias() , "testingAlias") == 0);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetDestinationInfoAlias()
{
	char* alias = new char[MaxAddressListSize];
	memset(alias,0,MaxAddressListSize);
	memcpy(alias,"testingAlias",12);
	alias[13] = '\0';
	
	m_pCall->SetSourceInfoAlias(alias);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestinationInfoAlias ",
		strcmp(m_pCall->GetSourceInfoAlias() , alias) == 0);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetDestInfoAliasType()
{
	DWORD destAliasType[MaxNumberOfAliases];
	DWORD i = 0;
	for (i = 0;i < MaxNumberOfAliases; i++)
		destAliasType[i] = i;
	m_pCall->SetDestInfoAliasType(destAliasType);
	for (i = 0;i < MaxNumberOfAliases; i++)
		CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetDestInfoAliasType ",
			(m_pCall->GetDestInfoAliasType())[i] == i);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetDestInfoAliasType()
{
	DWORD destAliasType[MaxNumberOfAliases];
	int i = 0;
	for (i = 0;i < MaxNumberOfAliases; i++)
		destAliasType[i] = i;
	m_pCall->SetDestInfoAliasType(destAliasType);
	for (i = 0;i < MaxNumberOfAliases; i++)
		CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetDestInfoAliasType ",
			(m_pCall->GetDestInfoAliasType())[i] == destAliasType[i]);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetCanMapAlias()
{
	BOOL bMapAlias = TRUE;
	m_pCall->SetCanMapAlias(bMapAlias);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetCanMapAlias ",
		m_pCall->GetCanMapAlias() == bMapAlias);
}
////////////////////////////////////////////////////////////////////////
void CTestCall::testSetCanMapAlias()
{
	BOOL bMapAlias = TRUE;
	m_pCall->SetCanMapAlias(bMapAlias);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCanMapAlias ",
		m_pCall->GetCanMapAlias() == TRUE);
}
////////////////////////////////////////////////////////////////////////
void CTestCall::testGetCallId()
{
	char* callId = new char[Size16];
	memset(callId,0,Size16);
	memcpy(callId,"testCallId1111",14);
	callId[15] = '\0';
	
	m_pCall->SetCallId(callId);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetCallId ",
		strcmp(m_pCall->GetCallId() , callId) == 0);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetCallId()
{
	char* callId = new char[Size16];
	memset(callId,0,Size16);
	memcpy(callId,"testCallId1111",14);
	callId[15] = '\0';
	
	m_pCall->SetCallId(callId);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCallId ",
		strcmp(m_pCall->GetCallId() , callId) == 0);

}
//////////////////////////////////////////////////////////////
void CTestCall::testGetCallType()
{
	cmCallType callType = cmCallTypeP2P;
	m_pCall->SetCallType(cmCallTypeP2P);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetCallType ",
		m_pCall->GetCallType() == callType);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetCallType()
{
	cmCallType callType = cmCallTypeP2P;
	m_pCall->SetCallType(callType);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCallType ",
		m_pCall->GetCallType() == callType);
}
//////////////////////////////////////////////////////////////
//void CTestCall::testGetCallModelType()
//{
//	DWORD callModelType = 0;
//	m_pCall->SetCallModelType(callModelType);
//	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetCallModelType ",
//		m_pCall->GetCallModelType() == callModelType);
//}
//////////////////////////////////////////////////////////////
//void CTestCall::testSetCallModelType()
//{
//	DWORD callModelType = 1;
//	m_pCall->SetCallModelType(cmCallModelTypeGKRouted);
//	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCallModelType ",
//		m_pCall->GetCallModelType() == callModelType);
//}
//////////////////////////////////////////////////////////////
void CTestCall::testGetIsActiveMc()
{
	BOOL isActiveMc = FALSE;
	m_pCall->SetIsActiveMc(isActiveMc);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetIsActiveMc ",
		m_pCall->GetIsActiveMc() == isActiveMc);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetIsActiveMc()
{
	BOOL isActiveMc = TRUE;
	m_pCall->SetIsActiveMc(TRUE);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetIsActiveMc ",
		m_pCall->GetIsActiveMc() == isActiveMc);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetIsOrigin()
{
	BOOL isOrigin = FALSE;
	m_pCall->SetIsOrigin(isOrigin);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetIsOrigin ",
		m_pCall->GetIsOrigin() == isOrigin);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetIsOrigin()
{
	BOOL isOrigin = TRUE;
	m_pCall->SetIsOrigin(TRUE);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetIsOrigin ",
		m_pCall->GetIsOrigin() == isOrigin);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetIsClosingProcess()
{
	BOOL isClosingProcess = FALSE;
	m_pCall->SetIsClosingProcess(isClosingProcess);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetIsClosingProcess ",
		m_pCall->GetIsClosingProcess() == isClosingProcess);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetIsClosingProcess()
{
	BOOL isClosingProcess = TRUE;
	m_pCall->SetIsClosingProcess(TRUE);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetIsClosingProcess ",
		m_pCall->GetIsClosingProcess() == isClosingProcess);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetCallCloseInitiator()
{
	DWORD callCloseInit = 4;
	m_pCall->SetCallCloseInitiator(callCloseInit);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetCallCloseInitiator ",
		m_pCall->GetCallCloseInitiator() == callCloseInit);
}
//////////////////////////////////////////////////////////////
void CTestCall::testSetCallCloseInitiator()
{
	DWORD callCloseInit = McInitiator;
	m_pCall->SetCallCloseInitiator(callCloseInit);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCallCloseInitiator ",
		m_pCall->GetCallCloseInitiator() == callCloseInit);
}
//////////////////////////////////////////////////////////////
void CTestCall::testGetSetupH245Address()
{
	mcTransportAddress setUpH245Addr;
	DWORD i = 0;
	setUpH245Addr.ipVersion = eIpVersion4;
	setUpH245Addr.distribution = eDistributionUnicast;
	setUpH245Addr.transportType = eTransportTypeTcp;
	setUpH245Addr.port = 80;
	setUpH245Addr.addr.v4.ip = 122112;
	m_pCall->SetSetupH245Address(setUpH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->addr.v4.ip == 122112);

	setUpH245Addr.ipVersion = eIpVersion6;
	memcpy(setUpH245Addr.addr.v6.ip,"172:1:1:1:1:11",13);
	setUpH245Addr.addr.v6.ip[13] = '\0';
	setUpH245Addr.addr.v6.scopeId = 500;
	m_pCall->SetSetupH245Address(setUpH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSetupH245Address1 ",
		(m_pCall->GetSetupH245Address())->ipVersion == eIpVersion6);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSetupH245Address1 ",
		strcmp((char*)(m_pCall->GetSetupH245Address())->addr.v6.ip, "172:1:1:1:1:1") == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSetupH245Address1 ",
		(m_pCall->GetSetupH245Address())->addr.v6.scopeId == 500);
}
///////////////////////////////////////////////////////////////
void CTestCall::testSetSetupH245Address()
{
	mcTransportAddress setUpH245Addr;
	DWORD i = 0;
	setUpH245Addr.ipVersion = eIpVersion4;
	setUpH245Addr.distribution = eDistributionUnicast;
	setUpH245Addr.transportType = eTransportTypeTcp;
	setUpH245Addr.port = 80;
	setUpH245Addr.addr.v4.ip = 122112;
	m_pCall->SetSetupH245Address(setUpH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSetupH245Address ",
		(m_pCall->GetSetupH245Address())->addr.v4.ip == 122112);

	setUpH245Addr.ipVersion = eIpVersion6;
	memcpy(setUpH245Addr.addr.v6.ip,"172:1:1:1:1:1",13);
	setUpH245Addr.addr.v6.ip[13] = '\0';

	setUpH245Addr.addr.v6.scopeId = 500;
	m_pCall->SetSetupH245Address(setUpH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSetupH245Address1 ",
		(m_pCall->GetSetupH245Address())->ipVersion == eIpVersion6);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSetupH245Address1 ",
		strcmp((char*)(m_pCall->GetSetupH245Address())->addr.v6.ip, "172:1:1:1:1:1") == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetSetupH245Address1 ",
		(m_pCall->GetSetupH245Address())->addr.v6.scopeId == 500);
}
///////////////////////////////////////////////////////////////
void CTestCall::testGetAnswerH245Address()
{
	mcXmlTransportAddress XmlAnswerH245Addr;
	DWORD i = 0;
	XmlAnswerH245Addr.transAddr.ipVersion = eIpVersion4;
	XmlAnswerH245Addr.transAddr.distribution = eDistributionUnicast;
	XmlAnswerH245Addr.transAddr.transportType = eTransportTypeTcp;
	XmlAnswerH245Addr.transAddr.port = 80;
	XmlAnswerH245Addr.transAddr.addr.v4.ip = 122112;
	m_pCall->SetAnswerH245Address(XmlAnswerH245Addr);

	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->addr.v4.ip == 122112);

	XmlAnswerH245Addr.transAddr.ipVersion = eIpVersion6;
	memcpy(XmlAnswerH245Addr.transAddr.addr.v6.ip,"172.1.1.1.1.1",13);
	XmlAnswerH245Addr.transAddr.addr.v6.ip[13] = '\0';
	XmlAnswerH245Addr.transAddr.addr.v6.scopeId = 500;
	m_pCall->SetAnswerH245Address(XmlAnswerH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->ipVersion == eIpVersion6);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetAnswerH245Address ",
		strcmp((char*)(m_pCall->GetAnswerH245Address())->addr.v6.ip, "172.1.1.1.1.1") == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->addr.v6.scopeId == 500);
}
///////////////////////////////////////////////////////////////
void CTestCall::testSetAnswerH245Address()
{
	mcXmlTransportAddress XmlAnswerH245Addr;
	DWORD i = 0;
	XmlAnswerH245Addr.transAddr.ipVersion = eIpVersion4;
	XmlAnswerH245Addr.transAddr.distribution = eDistributionUnicast;
	XmlAnswerH245Addr.transAddr.transportType = eTransportTypeTcp;
	XmlAnswerH245Addr.transAddr.port = 80;
	XmlAnswerH245Addr.transAddr.addr.v4.ip = 122112;
	m_pCall->SetAnswerH245Address(XmlAnswerH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address ",
		(m_pCall->GetAnswerH245Address())->addr.v4.ip == 122112);

	XmlAnswerH245Addr.transAddr.ipVersion = eIpVersion6;
	XmlAnswerH245Addr.transAddr.port = 80;
	memcpy(XmlAnswerH245Addr.transAddr.addr.v6.ip,"172.1.1.1.1.1",13);
	XmlAnswerH245Addr.transAddr.addr.v6.ip[13] = '\0';
	XmlAnswerH245Addr.transAddr.addr.v6.scopeId = 500;
	m_pCall->SetAnswerH245Address(XmlAnswerH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address1 ",
		(m_pCall->GetAnswerH245Address())->ipVersion == eIpVersion6);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address11 ",
		(m_pCall->GetAnswerH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address11 ",
		strcmp((char*)(m_pCall->GetAnswerH245Address())->addr.v6.ip, "172.1.1.1.1.1") == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetAnswerH245Address11 ",
		(m_pCall->GetAnswerH245Address())->addr.v6.scopeId == 500);
}
///////////////////////////////////////////////////////////////
void CTestCall::testGetControlH245Address()
{
	mcTransportAddress controlH245Addr;
	DWORD i = 0;
	controlH245Addr.ipVersion = eIpVersion4;
	controlH245Addr.distribution = eDistributionUnicast;
	controlH245Addr.transportType = eTransportTypeTcp;
	controlH245Addr.port = 80;
	controlH245Addr.addr.v4.ip = 122112;
	m_pCall->SetControlH245Address(controlH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address ",
		(m_pCall->GetControlH245Address())->ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address ",
		(m_pCall->GetControlH245Address())->distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address ",
		(m_pCall->GetControlH245Address())->transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address ",
		(m_pCall->GetControlH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address ",
		(m_pCall->GetControlH245Address())->addr.v4.ip == 122112);

	controlH245Addr.ipVersion = eIpVersion6;
	controlH245Addr.port = 80;
	memcpy(controlH245Addr.addr.v6.ip,"172.1.1.1.1.1",13);
	controlH245Addr.addr.v6.ip[13] = '\0';
	controlH245Addr.addr.v6.scopeId = 500;
	m_pCall->SetControlH245Address(controlH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address1 ",
		(m_pCall->GetControlH245Address())->ipVersion == eIpVersion6);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address1 ",
		(m_pCall->GetControlH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address1 ",
		strcmp((char*)(m_pCall->GetControlH245Address())->addr.v6.ip, "172.1.1.1.1.1") == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetControlH245Address1 ",
		(m_pCall->GetControlH245Address())->addr.v6.scopeId == 500);
}
///////////////////////////////////////////////////////////////
void CTestCall::testSetControlH245Address()
{
	mcTransportAddress controlH245Addr;
	DWORD i = 0;
	controlH245Addr.ipVersion = eIpVersion4;
	controlH245Addr.distribution = eDistributionUnicast;
	controlH245Addr.transportType = eTransportTypeTcp;
	controlH245Addr.port = 80;
	controlH245Addr.addr.v4.ip = 122112;
	m_pCall->SetControlH245Address(controlH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address ",
		(m_pCall->GetControlH245Address())->ipVersion == eIpVersion4);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address ",
		(m_pCall->GetControlH245Address())->distribution == eDistributionUnicast);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address ",
		(m_pCall->GetControlH245Address())->transportType == eTransportTypeTcp);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address ",
		(m_pCall->GetControlH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address ",
		(m_pCall->GetControlH245Address())->addr.v4.ip == 122112);

	controlH245Addr.ipVersion = eIpVersion6;
	controlH245Addr.port = 80;
	memcpy(controlH245Addr.addr.v6.ip,"172.1.1.1.1.1",13);
	controlH245Addr.addr.v6.ip[13] = '\0';
	controlH245Addr.addr.v6.scopeId = 500;
	m_pCall->SetControlH245Address(controlH245Addr);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address1 ",
		(m_pCall->GetControlH245Address())->ipVersion == eIpVersion6);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address1 ",
		(m_pCall->GetControlH245Address())->port == 80);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address1 ",
		strcmp((char*)(m_pCall->GetControlH245Address())->addr.v6.ip, "172.1.1.1.1.1") == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetControlH245Address1 ",
		(m_pCall->GetControlH245Address())->addr.v6.scopeId == 500);
}
///////////////////////////////////////////////////////////////
void CTestCall::testGetSpecificChannel()
{
	CChannel* pChannel = NULL;
	CChannel* pRcvChannel = NULL;
	CCapSetInfo capInfo;

	m_pCall->TestUpdateMcChannelParams(TRUE,pChannel,capInfo,kRolePeople,1280,NULL,kUnKnownMediaType,0);
	pRcvChannel = m_pCall->GetSpecificChannel(0, true);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSpecificChannel ",
		(pChannel->GetIndex() == pRcvChannel->GetIndex()));
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSpecificChannel ",
		(pChannel->IsOutgoingDirection() == pRcvChannel->IsOutgoingDirection()));
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSpecificChannel ",
		(pChannel->GetRoleLabel() == pRcvChannel->GetRoleLabel()));
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetSpecificChannel ",
		(pChannel->GetRate() == pRcvChannel->GetRate()));

}
////////////////////////////////////////////////////////////////
void CTestCall::testSetCurrentChannel()
{
	CChannel* pChannel = NULL;
	CChannel* pRcvChannel = NULL;
	APIS8	  pIndex = 0;
	CCapSetInfo capInfo;
	m_pCall->TestUpdateMcChannelParams(1,pChannel,capInfo,kRolePeople,1280,NULL,kUnKnownMediaType,0);

	m_pCall->SetCurrentChannel(1,0,&pRcvChannel,&pIndex);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCurrentChannel ",
		(pChannel->GetIndex() == 0));
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCurrentChannel ",
		(pChannel->IsOutgoingDirection() == pRcvChannel->IsOutgoingDirection()));
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCurrentChannel ",
		(pChannel->GetRoleLabel() == pRcvChannel->GetRoleLabel()));
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetCurrentChannel ",
		(pChannel->GetRate() == pRcvChannel->GetRate()));

}
////////////////////////////////////////////////////////////////
void CTestCall::testRemoveChannel()
{
	CChannel* pChannel = NULL;
	int i = 0;
	CCapSetInfo capInfo;
	m_pCall->TestUpdateMcChannelParams(TRUE,pChannel,capInfo,kRolePeople,1280,NULL,kUnKnownMediaType,0);

	m_pCall->RemoveChannel(i);
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testRemoveChannel ",
		(m_pCall->GetChannelsCounter() == 0));
	CPPUNIT_ASSERT_MESSAGE("CTestCall::testRemoveChannel ",
		(m_pCall->GetChannelsArray()[0] == NULL));

}
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
void CTestChannel::setUp()
{
	m_pChannel = new CChannel;
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::tearDown()
{
	POBJDELETE(m_pChannel);
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testConstructor()
{
	CPPUNIT_ASSERT(m_pChannel->GetIndex() == 0);
	CPPUNIT_ASSERT(m_pChannel->IsOutgoingDirection() == FALSE);
	CPPUNIT_ASSERT(m_pChannel->GetPayloadType() == (DWORD)_UnKnown);
	CPPUNIT_ASSERT(m_pChannel->GetType() == cmCapEmpty);
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testUpdateChannelParams()
{
	CCall *pCall = new CCall;
	CCapSetInfo pCapSetInfo;
	m_pChannel->TestUpdateChannelParams(pCall,0,TRUE,pCapSetInfo,kRolePeople,640,NULL,kUnKnownMediaType,0);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testUpdateChannelParams ",
		(m_pChannel->GetIndex() == 0));
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testUpdateChannelParams ",
		(m_pChannel->IsOutgoingDirection() == TRUE));
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testUpdateChannelParams ",
		(m_pChannel->GetRoleLabel() == (DWORD)kRolePeople));
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testUpdateChannelParams ",
		(m_pChannel->GetRate() == 640));
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testUpdateChannelParams ",
		(m_pChannel->GetEncryptionType() == kUnKnownMediaType));
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testUpdateChannelParams ",
		(m_pChannel->GetStatus() == 0));

}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetIndex()
{
	m_pChannel->SetIndex(5);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetIndex ",
		(m_pChannel->GetIndex() == 5));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetIndex()
{
	DWORD index = 6;
	m_pChannel->SetIndex(index);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetIndex ",
		(m_pChannel->GetIndex() == index));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetStatus()
{
	int status = -1;
	m_pChannel->SetStatus(status);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetStatus ",
		(m_pChannel->GetStatus() == status));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetStatus()
{
	int status = -1;
	m_pChannel->SetStatus(-1);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testSetStatus ",
		(m_pChannel->GetStatus() == status));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetChannelDirection()
{
	BOOL channelDirection = TRUE;
	m_pChannel->SetChannelDirection(channelDirection);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetChannelDirection ",
		(m_pChannel->IsOutgoingDirection() == channelDirection));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetChannelDirection()
{
	BOOL channelDirection = TRUE;
	m_pChannel->SetChannelDirection(TRUE);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testSetChannelDirection ",
		(m_pChannel->IsOutgoingDirection() == channelDirection));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetIsActive()
{
	BOOL isActive = FALSE;
	m_pChannel->SetIsActive(isActive);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetIsActive ",
		(m_pChannel->GetIsActive() == isActive));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetIsActive()
{
	BOOL isActive = FALSE;
	m_pChannel->SetIsActive(isActive);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testSetIsActive ",
		(m_pChannel->GetIsActive() == FALSE));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetChannelCloseInitiator()
{
	DWORD channelCloseInitiator = 1;
	m_pChannel->SetChannelCloseInitiator(channelCloseInitiator);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetChannelCloseInitiator ",
		(m_pChannel->GetChannelCloseInitiator() == (DWORD)McInitiator));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetChannelCloseInitiator()
{
	DWORD channelCloseInitiator = 1;
	m_pChannel->SetChannelCloseInitiator(1);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testSetChannelCloseInitiator ",
		(m_pChannel->GetChannelCloseInitiator() == (DWORD)McInitiator));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetPayloadType()
{
	DWORD paloadType = 4;
	m_pChannel->SetPayloadType(4);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetPayloadType ",
		(m_pChannel->GetPayloadType() == (DWORD)_G7231));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetPayloadType()
{
	DWORD paloadType = 4;
	m_pChannel->SetPayloadType((DWORD)_G7231);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testSetPayloadType ",
		(m_pChannel->GetPayloadType() == 4));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetCapNameEnum()
{
	CapEnum capEnum = eG722_48kCapCode;
	m_pChannel->SetCapNameEnum(capEnum);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetCapNameEnum ",
		(m_pChannel->GetCapNameEnum() == 6));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetCapNameEnum()
{
	CapEnum capEnum = eG722_48kCapCode;
	m_pChannel->SetCapNameEnum(capEnum);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testSetCapNameEnum ",
		(m_pChannel->GetCapNameEnum() == (DWORD)eG722_48kCapCode));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetType()
{
	cmCapDataType type = cmCapNonStandard;
	m_pChannel->SetType(cmCapNonStandard);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetType ",
		(m_pChannel->GetType() == 4));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetType()
{
	cmCapDataType type = cmCapNonStandard;
	m_pChannel->SetType(type);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testSetType ",
		(m_pChannel->GetType() == cmCapNonStandard));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetRoleLabel()
{
	ERoleLabel roleLabel = kRoleContent;
	m_pChannel->SetRoleLabel(kRoleContent);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testGetRoleLabel ",
		(m_pChannel->GetRoleLabel() == 1));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetRoleLabel()
{
	ERoleLabel roleLabel = kRolePeople;
	m_pChannel->SetRoleLabel(roleLabel);
	CPPUNIT_ASSERT_MESSAGE("CTestChannel::testSetRoleLabel ",
		(m_pChannel->GetRoleLabel() == kRolePeople));
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testGetChannelParams()
{
	char* channelParams = new char[10];
	memset(channelParams,'\0',10);
	memcpy(channelParams,"test",4);
	
	m_pChannel->SetChannelParams(4,channelParams);
	for (int i = 0;i < 4 ; i++)
		CPPUNIT_ASSERT_MESSAGE("CTestCall::testGetChannelParams ",
			(m_pChannel->GetChannelParams())[i] == channelParams[i]);
}
//////////////////////////////////////////////////////////////////////
void CTestChannel::testSetChannelParams()
{
	char* channelParams = new char[MaxAddressListSize];
	memset(channelParams,'\0',MaxAddressListSize);
	memcpy(channelParams,"test",4);
	char* pRcvChannelParams = NULL;
	
	m_pChannel->SetChannelParams(4,channelParams);
	for (int i = 0;i < 4 ; i++)
		CPPUNIT_ASSERT_MESSAGE("CTestCall::testSetChannelParams ",
			(m_pChannel->GetChannelParams())[i] == channelParams[i]);
}
//////////////////////////////////////////////////////////////////////



