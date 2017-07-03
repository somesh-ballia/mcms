// TEST_CONFPARTYROUTINGTABLE.h: interface for the CTestConfPartyRsrcTable class.
// Unit tests using TDD of the ConfParty Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_CMOCKMPLMCMSPROTOCOL_H)
#define TEST_CMOCKMPLMCMSPROTOCOL_H

#include <cppunit/extensions/HelperMacros.h>
#include "MplMcmsProtocol.h"
#include "SipCsReq.h"

// private tests (example)

class CMockMplMcmsProtocol   : public CMplMcmsProtocol
{
CLASS_TYPE_1(CMockMplMcmsProtocol,CMplMcmsProtocol )	

public:
	
	CMockMplMcmsProtocol();
	virtual const char* NameOf() const { return "CMockMplMcmsProtocol";}
	virtual void AddCommonHeader			  ( const DWORD opcode,
												const BYTE  protocol_version=0,
												const BYTE  option=0,
												const BYTE  src_id=0,
												const BYTE  dest_id=0,
												const DWORD time_stamp=0,
												// const DWORD sequence_num=0,
												const DWORD payload_len=0,
												const DWORD payload_offset=0,
												const DWORD next_header_type=0,
												const DWORD next_header_size=0);

	virtual void AddMessageDescriptionHeader  ( const DWORD  request_id=0,
												const DWORD  entity_type=0,
												const DWORD  time_stamp=0, 
												const DWORD  next_header_type=0,
												const DWORD  next_header_size=0);

	virtual void AddCSHeader	              ( const WORD   cs_id = 0,
	                                        	const WORD   src_unit_id=0,
                                                const WORD   dst_unit_id=0,										
												const DWORD  call_index = 0,
												const DWORD  service_id = 0,
												const DWORD  channel_index = 0,
												const DWORD  mc_channel_index = 0,
												const APIS32 status = 0,
												const WORD   next_header_type=0,
												const WORD   next_header_offset=0);

	virtual void AddPortDescriptionHeader     ( const DWORD  party_id,
												const DWORD  conf_id,
												const DWORD  connection_id=0,
												const BYTE   logical_resource_type_1=0, 
												const BYTE   logical_resource_type_2=0,
												const BYTE   future_use1=0,
												const BYTE   future_use2=0,
												const DWORD  next_header_type=0,
												const DWORD  next_header_size=0);

	virtual void AddData					  ( const DWORD  DataLen,
												const char*  pData);
	
	virtual STATUS  SendMsgToCSApiCommandDispatcher();

	virtual void TraceMplMcmsProtocol(const char* message=" ",BOOL Type=MPL_API_TYPE);
	
	void Varify_AddCommonHeader_WasCalled();
	void Varify_AddCommonHeader_WasCalled1(DWORD &opcode);
	void Verify_AddCommonHeader_WasNotCalled();
	void Varify_SendMsgToCSApiCommandDispatcher_WasCalled();
	void Varify_AddMessageDescriptionHeader_WasCalled();
	void Varify_AddPortDescriptionHeader_WasCalled();
	void Varify_AddPortDescriptionHeader_WasCalled1(DWORD &partyId, DWORD &confId, DWORD &conId);
	void Varify_AddCSHeader_WasCalled();
	void Varify_AddCSHeader_WasCalled1(	WORD &csId,WORD &srcUnitId,DWORD &serviceId, WORD &destUnitId, DWORD &callIndex, DWORD &channelIndex
		, DWORD &mcChannelIndex	, APIS32 &status);
	void Varify_AddData_WasCalled();
	char* Varify_AddData_WasCalled1(DWORD &dataLen);
	void Varify_TraceMplMcmsProtocol_WasCalled();
	char* Varify_TraceMplMcmsProtocol_WasCalled1(BOOL &type);

	void	Verify_SipSubscribeResponse_WasCalled(mcReqSubscribeResp** pSubResponse);
	void	Verify_SipNotify_WasCalled();
	void	Verify_SipNotify_WasNotCalled();
	void	Verify_SipNotify_WasCalled(mcReqNotify** pNotifyReq);
	void	Verify_SipReferResponse_WasCalled(mcReqReferResp** pReferResponse);


protected:
	BYTE		mm_bAddCommonHeaderUsed;
	DWORD		mm_opcode;

	BYTE		mm_bSipSubscribeResponse;
	DWORD		mm_SipSubSize;
	mcReqSubscribeResp*	mm_pSipSubscribeResponse;

	BYTE		mm_bSipReferResponse;
	DWORD		mm_SipReferSize;
	mcReqReferResp*	mm_pSipReferResponse;

	BYTE		mm_bSipNotify;
	DWORD		mm_SipNotifySize;
	mcReqNotify*	mm_pSipNotify;

	BYTE		mm_bSendMsgToCsApi;

	BYTE        mm_bAddMSgDescHeader;
	
	BYTE		mm_bAddPortDescriptionHeaderUsed;
	DWORD		mm_partyId;
	DWORD		mm_confId;
	DWORD		mm_connectionId;

	BYTE		mm_bAddCSHeader;
	WORD		mm_csId;
	WORD		mm_srcUnitId;
	WORD		mm_serviceId;
	WORD		mm_destUnitId;
	DWORD		mm_callIndex;
	DWORD		mm_channelIndex;
	DWORD		mm_mcChannelIndex;
	APIS32		mm_status;
	
	BYTE		mm_bAddData;
	DWORD		mm_dataLen;
	char*		mm_pData;

	BYTE		mm_bTraceMplMcmsProtocol;
	char*		mm_message;
	BOOL		mm_type;
};

#endif // !defined(TEST_CMOCKMPLMCMSPROTOCOL_H)

