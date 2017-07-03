// CTestCsInterface.cpp: implementation of the CTestCsInterface class.
//
//////////////////////////////////////////////////////////////////////


#include "TestCsInterface.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "RsrcParams.h"
#include "IpCsOpcodes.h"


//CPPUNIT_TEST_SUITE_REGISTRATION( CTestCsInterface );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void CTestCsInterface::setUp()
{
	m_pCsInterface = new CCsInterface;
	m_pDesc = new CRsrcParams(333,2,1,eLogical_audio_decoder);
	m_pCsInterface->Create(m_pDesc);
	m_MplMock = new CMockMplMcmsProtocol;
	m_pCsInterface->SetTddMockInterface(m_MplMock);

}
//////////////////////////////////////////////////////////////////////
void CTestCsInterface::tearDown()
{
	POBJDELETE(m_pCsInterface);
	POBJDELETE(m_pDesc);


}
//////////////////////////////////////////////////////////////////////
void CTestCsInterface::testConstructor()
{
	CPPUNIT_ASSERT_MESSAGE( "CTestCsInterface::testConstructor ",
		m_pCsInterface != NULL );  

}
///////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testUpdateResourceParam()
{
	CRsrcParams *Desc = new CRsrcParams(333,2,1,eLogical_audio_decoder);
	m_pCsInterface->UpdateRsrcParams(Desc);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testUpdateResourceParam ",
		m_pCsInterface->GetRsrcParams() == Desc);

}
///////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testGetRsrcParams()
{
	CRsrcParams *Desc = NULL;
	Desc = m_pCsInterface->GetRsrcParams();
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testGetRsrcParams ",
		Desc != NULL);

}
///////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testGetRsrcParams1()
{
	CRsrcParams *Desc = NULL;
	Desc = m_pCsInterface->GetRsrcParams();
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testGetRsrcParams1 ",
		Desc->GetConfRsrcId() == m_pDesc->GetConfRsrcId() &&
		Desc->GetPartyRsrcId() == m_pDesc->GetPartyRsrcId());

}
////////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testGetPartyRsrcId()
{
	DWORD partyRsrcId = m_pCsInterface->GetPartyRsrcId();
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testGetPartyRsrcId ",
		m_pDesc->GetPartyRsrcId() == partyRsrcId);

}
/////////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testGetConfRsrcId()
{
	DWORD confRsrcId = m_pCsInterface->GetConfRsrcId();
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testGetConfRsrcId ",
		m_pDesc->GetConfRsrcId() == confRsrcId);

}
/////////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testSetPartyRsrcId()
{
	DWORD PartyRsrcId = 5;
	m_pCsInterface->SetPartyRsrcId(PartyRsrcId);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testSetPartyRsrcId ",
		m_pCsInterface->GetPartyRsrcId() == PartyRsrcId);

}
/////////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testSetConfRsrcId()
{
	DWORD ConfRsrcId = 5;
	m_pCsInterface->SetConfRsrcId(ConfRsrcId);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testSetConfRsrcId ",
		m_pCsInterface->GetConfRsrcId() == ConfRsrcId);

}
/////////////////////////////////////////////////////////////////////////////////
/*void CTestCsInterface::testGetMplMcmsProtocol()
{
	CMplMcmsProtocol* pPtr = m_pCsInterface->GetMplMcmsProtocol();
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testSetConfRsrcId ",
		pPtr != NULL);

}
*/
//////////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testMplApiWasAddCommonHeaderCalled()
{
	m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,NULL);
	DWORD opcode = 0;
	m_MplMock->Varify_AddCommonHeader_WasCalled1(opcode);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testMplApiWasAddCommonHeaderCalled1 ",
		opcode == H323_CS_SIG_GET_PORT_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testSendMsgToCSApiCommandDispatcher()
{
	m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,NULL);
	m_MplMock->Varify_SendMsgToCSApiCommandDispatcher_WasCalled();

}

///////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testAddMessageDescriptionHeaderCalled()
{
	m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,NULL);
	m_MplMock->Varify_AddMessageDescriptionHeader_WasCalled();
}

////////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testAddPortDescriptionHeader()
{
	m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,NULL);
	DWORD partyId = 0,confId = 0,conId = 0;
	m_MplMock->Varify_AddPortDescriptionHeader_WasCalled1(partyId, confId, conId);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddPortDescriptionHeader1 ",
		partyId == 2);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddPortDescriptionHeader1 ",
		confId == 1);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddPortDescriptionHeader1 ",
		conId == 333);

	
}

////////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testAddCSHeader()
{
	m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,NULL,1,1,1,1,1,1,-1);
	m_MplMock->Varify_AddMessageDescriptionHeader_WasCalled();
	WORD		csId = 0;
	DWORD		serviceId = 0;
	WORD		destUnitId = 0;
	DWORD		callIndex = 0;
	DWORD		channelIndex = 0;
	DWORD		mcChannelIndex = 0;
	APIS32		status = 0;
	WORD		srcUnitId = 0;
	APIS32		testing = -1;
	APIU32		testing1 = 0;
	m_MplMock->Varify_AddCSHeader_WasCalled1(csId, srcUnitId,serviceId,destUnitId, callIndex, channelIndex, mcChannelIndex, status);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddCSHeader1 ",
		csId == 1);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddCSHeader1 ",
		srcUnitId == 0);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddCSHeader1 ",
		serviceId == 1);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddCSHeader1 ",
		destUnitId == 1);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddCSHeader1 ",
		callIndex == 1);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddCSHeader1 ",
		channelIndex == 1);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddCSHeader1 ",
		mcChannelIndex == 1);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddCSHeader1 ",
		status == -1);


}

///////////////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testAddData()
{
	CSegment* pSeg = new CSegment;
	DWORD tt = 5;
	pSeg->Put((BYTE*)(&tt),sizeof(DWORD));	
	DWORD dataLen = 0;
	char* data = NULL;
	m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,pSeg);
	data = m_MplMock->Varify_AddData_WasCalled1(dataLen);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddData1 ",
		dataLen == 4);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testAddData1 ",
		data && *data == 5);
	
	POBJDELETE(pSeg);
	
}

//////////////////////////////////////////////////////////////////////////////
void CTestCsInterface::testTraceMplMcmsProtocol()
{
	m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,NULL);
	m_MplMock->Varify_TraceMplMcmsProtocol_WasCalled();
	m_pCsInterface->SendMsgToCS(H323_CS_SIG_GET_PORT_REQ,NULL);
	char* data = NULL;
	BOOL type = 0;
	data = m_MplMock->Varify_TraceMplMcmsProtocol_WasCalled1(type);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testTraceMplMcmsProtocol ",
		type == 2);
	CPPUNIT_ASSERT_MESSAGE("CTestCsInterface::testTraceMplMcmsProtocol ",
		strcmp(data,"CCsInterface::SendMsgToCS ") == 0);

}



