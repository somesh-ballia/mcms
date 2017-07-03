// CTestCsInterface.cpp: implementation of the CTestCsInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "MockCMplMcmsProtocol.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "Macros.h"
#include "Segment.h"
#include "StatusesGeneral.h"
#include "IpCsOpcodes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CMockMplMcmsProtocol::CMockMplMcmsProtocol()
{
	mm_bAddCommonHeaderUsed = FALSE;
	mm_opcode = 0;

	mm_bSipSubscribeResponse = FALSE;
	mm_SipSubSize = 0;
	mm_pSipSubscribeResponse = NULL;

	mm_bSipReferResponse = FALSE;
	mm_SipReferSize = 0;
	mm_pSipReferResponse = NULL;

	mm_bSipNotify = FALSE;
	mm_SipNotifySize = 0;
	mm_pSipNotify = NULL;

	mm_bSendMsgToCsApi = FALSE;
	mm_bAddMSgDescHeader = FALSE;
	mm_bAddPortDescriptionHeaderUsed = FALSE;
	mm_partyId = 0;
	mm_confId = 0;
	mm_connectionId = 0;
	mm_bAddCSHeader = FALSE;
	mm_csId = 0;
	mm_srcUnitId = 0;
	mm_destUnitId = 0;
	mm_callIndex = 0;
	mm_channelIndex = 0;
	mm_mcChannelIndex = 0;
	mm_status = 0;	
	mm_bAddData = FALSE;
	mm_dataLen = 0;
	mm_pData = NULL;
	mm_bTraceMplMcmsProtocol = FALSE;
	mm_message = NULL;
	mm_type = 0;


}

//////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::AddCommonHeader(const DWORD opcode,
											const BYTE  protocol_version,
                                            const BYTE  option,
											const BYTE  src_id,
											const BYTE  dest_id,
											const DWORD time_stamp,
										//	const DWORD sequence_num,
											const DWORD payload_len,
											const DWORD payload_offset,
											const DWORD next_header_type,
    										const DWORD next_header_size)
{
	mm_bAddCommonHeaderUsed = TRUE;
	mm_opcode = opcode;

	switch(mm_opcode)
	{
		case(SIP_CS_SIG_SUBSCRIBE_RESP_REQ):
		{
			mm_bSipSubscribeResponse = TRUE;
			break;
		}
		case(SIP_CS_PROXY_NOTIFY_REQ):
		{
			mm_bSipNotify = TRUE;
			break;
		}
		case(SIP_CS_SIG_REFER_RESP_REQ):
		{
			mm_bSipReferResponse = TRUE;
			break;
		}		
	}
}

////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_AddCommonHeader_WasCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddCommonHeader_WasCalled", mm_bAddCommonHeaderUsed);
	mm_bAddCommonHeaderUsed = FALSE;
}

////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Verify_AddCommonHeader_WasNotCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_AddCommonHeader_WasCalled", !mm_bAddCommonHeaderUsed);
}

/////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_AddCommonHeader_WasCalled1(DWORD &opcode)
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddCommonHeader_WasCalled1", mm_bAddCommonHeaderUsed);
	mm_bAddCommonHeaderUsed = FALSE;
	opcode = mm_opcode;
	mm_opcode = 0;
}

/////////////////////////////////////////////////////////
STATUS CMockMplMcmsProtocol::SendMsgToCSApiCommandDispatcher()
{
	mm_bSendMsgToCsApi =  TRUE;
	return STATUS_OK;
}

/////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_SendMsgToCSApiCommandDispatcher_WasCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_SendMsgToCSApiCommandDispatcher_WasCalled", mm_bSendMsgToCsApi);
	mm_bSendMsgToCsApi = FALSE;

}

//////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::AddMessageDescriptionHeader  ( const DWORD  request_id,
														  const DWORD  entity_type,
														  const DWORD  time_stamp, 
														  const DWORD  next_header_type,
														  const DWORD  next_header_size)
{
	mm_bAddMSgDescHeader = TRUE;
}

///////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_AddMessageDescriptionHeader_WasCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddMessageDescriptionHeader_WasCalled", mm_bAddCommonHeaderUsed);
	mm_bAddCommonHeaderUsed = FALSE;

}

////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::AddPortDescriptionHeader    ( const DWORD  party_id,
													     const DWORD  conf_id,
													     const DWORD  connection_id,
													     const BYTE   logical_resource_type_1, 
													     const BYTE   logical_resource_type_2,
													     const BYTE   future_use1,
													     const BYTE   future_use2,
													     const DWORD  next_header_type,
													     const DWORD  next_header_size )
{

	mm_bAddPortDescriptionHeaderUsed = TRUE;
	mm_partyId = party_id;
	mm_confId = conf_id;
	mm_connectionId = connection_id;
}

/////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_AddPortDescriptionHeader_WasCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddPortDescriptionHeader_WasCalled", mm_bAddPortDescriptionHeaderUsed);
	mm_bAddPortDescriptionHeaderUsed = FALSE;

}

//////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_AddPortDescriptionHeader_WasCalled1(DWORD &partyId, DWORD &confId, DWORD &conId)
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddPortDescriptionHeader_WasCalled1", mm_bAddPortDescriptionHeaderUsed);
	mm_bAddPortDescriptionHeaderUsed = FALSE;
	partyId = mm_partyId;
	confId = mm_confId;
	conId = mm_connectionId;
}

//////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::AddCSHeader		       ( const WORD   cs_id,
													 const WORD   src_unit_id,
													 const WORD   dst_unit_id,										
													 const DWORD  call_index ,
													 const DWORD  service_id ,
													 const DWORD  channel_index ,
													 const DWORD  mc_channel_index,
													 const APIS32 status ,
													 const WORD   next_header_type,
													 const WORD   next_header_offset)
{
	mm_bAddCSHeader = TRUE;
	mm_csId = cs_id;
	mm_srcUnitId = src_unit_id;
	mm_destUnitId = dst_unit_id;
	mm_callIndex = call_index;
	mm_serviceId = service_id;
	mm_channelIndex = channel_index;
	mm_mcChannelIndex = mc_channel_index;
	mm_status = status;	
	
}

////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_AddCSHeader_WasCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddCSHeader_WasCalled", mm_bAddCSHeader);
	mm_bAddCSHeader = FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_AddCSHeader_WasCalled1(	WORD &csId,WORD &srcUnitId,DWORD &serviceId, WORD &destUnitId, DWORD &callIndex, DWORD &channelIndex
		, DWORD &mcChannelIndex	, APIS32 &status)
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddCSHeader_WasCalled1", mm_bAddCSHeader);
	mm_bAddCSHeader = FALSE;
	csId = mm_csId;
	serviceId = mm_serviceId;
	destUnitId = mm_destUnitId;
	callIndex = mm_callIndex;
	channelIndex = mm_channelIndex;
	mcChannelIndex = mm_mcChannelIndex;
	status = mm_status;
}

//////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::AddData			  ( const DWORD  DataLen,
												const char*  pData)
{

	mm_bAddData = TRUE;
	mm_dataLen = DataLen;
	PDELETEA (mm_pData);
	if (mm_dataLen)
	{   
		mm_pData = new char[mm_dataLen];
		memset(mm_pData,'\0',mm_dataLen);
		memcpy(mm_pData,pData,mm_dataLen);
	}

	if(SIP_CS_SIG_SUBSCRIBE_RESP_REQ == mm_opcode && mm_bSipSubscribeResponse)
	{
		mm_SipSubSize = mm_dataLen;
		mm_pSipSubscribeResponse = (mcReqSubscribeResp *)new BYTE[mm_dataLen];
		memcpy(mm_pSipSubscribeResponse, mm_pData, mm_dataLen);
	}

	if(SIP_CS_PROXY_NOTIFY_REQ == mm_opcode && mm_bSipNotify)
	{
		mm_SipNotifySize = mm_dataLen;
		mm_pSipNotify = (mcReqNotify *)new BYTE[mm_dataLen];
		memcpy(mm_pSipNotify, mm_pData, mm_dataLen);
	}

	if(SIP_CS_SIG_REFER_RESP_REQ == mm_opcode && mm_bSipReferResponse)
	{
		mm_SipReferSize = mm_dataLen;
		mm_pSipReferResponse = (mcReqReferResp *)new BYTE[mm_dataLen];
		memcpy(mm_pSipReferResponse, mm_pData, mm_dataLen);
	}

}

////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_AddData_WasCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddData_WasCalled", mm_bAddData);
	mm_bAddData = FALSE;
	
}

////////////////////////////////////////////////////////////////////////////////////
char* CMockMplMcmsProtocol::Varify_AddData_WasCalled1(DWORD &dataLen)
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_AddData_WasCalled1", mm_bAddData);
	mm_bAddData = FALSE;
	dataLen = mm_dataLen;
	char* data = NULL;
	if (dataLen)
	{   
		data = new char[dataLen];
		memcpy(data,mm_pData,dataLen);
	}

	return data;
}

////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Verify_SipSubscribeResponse_WasCalled(mcReqSubscribeResp** pSubResponse)
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_SipSubscribeResponse_WasCalled, addCommonHeader was not called with Subscribe opcode", mm_bSipSubscribeResponse);
	mm_bSipSubscribeResponse = FALSE;
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_SipSubscribeResponse_WasCalled, no Data", mm_SipSubSize);
	if (mm_SipSubSize)
	{   
		*pSubResponse = (mcReqSubscribeResp *)new BYTE[mm_SipSubSize];
		memcpy(*pSubResponse, mm_pSipSubscribeResponse, mm_SipSubSize);
		mm_SipSubSize = 0;
		PDELETEA(mm_pSipSubscribeResponse);
	}
}

////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Verify_SipNotify_WasCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_SipNotify_WasCalled, addCommonHeader was not called", mm_bSipNotify);
	mm_bSipNotify = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Verify_SipNotify_WasNotCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_SipNotify_WasNotCalled, wrong opcode", !mm_bSipNotify);
	mm_bSipNotify = 0;
}

////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Verify_SipNotify_WasCalled(mcReqNotify** pNotifyReq)
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_SipNotify_WasCalled, addCommonHeader was not called", mm_bSipNotify);
	mm_bSipNotify = FALSE;
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_SipNotify_WasCalled, no data", mm_SipNotifySize);
	if (mm_SipNotifySize)
	{   
		*pNotifyReq = (mcReqNotify *)new BYTE[mm_SipNotifySize];
		memcpy(*pNotifyReq, mm_pSipNotify, mm_SipNotifySize);
		mm_SipNotifySize = 0;
		PDELETEA(mm_pSipNotify);
	}
}

////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Verify_SipReferResponse_WasCalled(mcReqReferResp** pReferResponse)
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_SipReferResponse_WasCalled, addCommonHeader was not called", mm_bSipReferResponse);
	mm_bSipReferResponse = FALSE;
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Verify_SipReferResponse_WasCalled, no Data", mm_SipReferSize);
	if (mm_SipReferSize)
	{   
		*pReferResponse = (mcReqReferResp *)new BYTE[mm_SipReferSize];
		memcpy(*pReferResponse, mm_pSipReferResponse, mm_SipReferSize);
		mm_SipReferSize = 0;
		PDELETEA(mm_pSipReferResponse);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::TraceMplMcmsProtocol(const char* message,BOOL Type)
{
	mm_bTraceMplMcmsProtocol = TRUE;
	mm_type = Type;
	PDELETEA (mm_message);
	DWORD len = strlen(message);
	mm_message = new char[len+1];
	memset(mm_message,'\0',len+1);
	memcpy(mm_message,message,len);
	
}
///////////////////////////////////////////////////////////////////////////////////////
void CMockMplMcmsProtocol::Varify_TraceMplMcmsProtocol_WasCalled()
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_TraceMplMcmsProtocol_WasCalled", mm_bTraceMplMcmsProtocol);
	mm_bTraceMplMcmsProtocol = FALSE;

}

///////////////////////////////////////////////////////////////////////////////////////
char* CMockMplMcmsProtocol::Varify_TraceMplMcmsProtocol_WasCalled1(BOOL &type)
{
	CPPUNIT_ASSERT_MESSAGE("CMockMplMcmsProtocol::Varify_TraceMplMcmsProtocol_WasCalled1", mm_bTraceMplMcmsProtocol);
	mm_bTraceMplMcmsProtocol = FALSE;
	type = mm_type;
	char* data = NULL;
	data = new char[strlen(mm_message)+1];
	memset(data,'\0',strlen(mm_message)+1);
	memcpy(data,mm_message,strlen(mm_message));

	return data;

}





