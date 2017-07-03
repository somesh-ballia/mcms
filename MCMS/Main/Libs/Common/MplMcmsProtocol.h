//+========================================================================+
//                            MplProtocol.H                                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MplProtocol.H                                               |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        

#ifndef _MPL_PROTOCOL
#define _MPL_PROTOCOL


#include "DataTypes.h"
#include "PObject.h"
#include "MplMcmsStructs.h"
#include "TraceHeader.h"
#include "AllocateStructs.h"
#include "ObjString.h"
#include "AuditDefines.h"
#include "HostCommonDefinitions.h"

class CMplMcmsProtocol;
class CSegment;


#define TPKT_VERSION_NUM                0x3
#define MPL_PROTOCOL_VERSION_NUM        1 //TO CANGE THIS NUMBER EVERY TIME THAT WE CHANGE THE PROTOCOL API.
#define STR_PERMANENT_WORDS_SIZE        8

#define MAX_PHYSICAL_INFO_HEADERS       10
#define MAX_PORT_DESCRIPTION_HEADERS    10
#define MAX_MPL_API_MSG_LEN             0xffffffff

#define MPL_API_TYPE                    1
#define CS_API_TYPE                     2



// statuses for errno structure
#define PROTOCOL_STATUS_OK                  0
#define PROTOCOL_STATUS_BAD_NEXT_HEADER     1
#define PROTOCOL_STATUS_BAD_OFFSET_LEN      2


struct MplMcmsProtocolErrno_S
{
	std::string  m_Message;
	int m_Status;
};







class CMplMcmsProtocol : public CPObject
{
	CLASS_TYPE_1(CMplMcmsProtocol,CPObject)
		//Constructors
		
	friend class CMplApiSpecialCommandHandler;
	friend class CEndpointLinker;//the cascade linker need switch the mpl msg, many operation is needed
//	friend class CMplMcmsProtocolTracer;
	virtual const char* NameOf() const { return "CMplMcmsProtocol";}
public:
	CMplMcmsProtocol();
	CMplMcmsProtocol(const CMplMcmsProtocol& other);                     
	virtual ~CMplMcmsProtocol();

    bool operator == (const CMplMcmsProtocol & other)const;
    bool operator != (const CMplMcmsProtocol & other)const;
    
	//Serializations
    void SerializeLogger(CSegment& seg);
    void Serialize(CSegment& seg,BOOL Type=MPL_API_TYPE);
	bool DeSerialize(CSegment& seg,BOOL Type=MPL_API_TYPE);
	void Serialize_TPKT_Header(CSegment& seg) const;
    void DeSerialize_TPKT_Header(CSegment& seg);

    bool DeSerialize(const BYTE* pBufferArr,const DWORD nBufferLen,BOOL Type=MPL_API_TYPE);
    void OldDeserialize(const BYTE* pBuffer,const DWORD nBufferLen,BOOL Type);
    
	
	// Implementation
	
	void SetTPKT_Header  	(char* buf)	;
    
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
	
	void AddPhysicalHeader                    ( const BYTE   box_id,
												const BYTE   board_id,
												const BYTE   sub_board_id     = 0,
												const BYTE   unit_id          = 0,
												const WORD   port_id          = 0,
												const BYTE   accelerator_id   = 0,
												const BYTE   resource_type    = 0,
												const BYTE   future_use1      = 0,
												const BYTE   future_use2      = 0,
												const DWORD  next_header_type = 0,
												const DWORD  next_header_size = 0 );
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







	virtual void AddPortDescriptionHeader	  ( const DWORD  party_id,
												const DWORD  conf_id,
												const DWORD  connection_id=0,
												const BYTE   logical_resource_type_1=0, 
												const BYTE   logical_resource_type_2=0,
												const BYTE   future_use1=0,
												const BYTE   future_use2=0,
												const DWORD  next_header_type=0,
												const DWORD  next_header_size=0,
												const WORD   room_id=DUMMY_ROOM_ID);
   
   virtual void UpdatePortDescriptionHeader	  ( const DWORD  party_id,
												const DWORD  conf_id,
												const DWORD  connection_id=0,
												const BYTE   logical_resource_type_1=0, 
												const BYTE   logical_resource_type_2=0,
												const BYTE   future_use1=0,
												const BYTE   future_use2=0,
												const DWORD  next_header_type=0,
												const DWORD  next_header_size=0,
												const int Counter=0,
												const WORD   room_id=DUMMY_ROOM_ID);
	
    
    /*PHYSICAL_INFO_HEADER_S */ 
	void GetPhysicalInfoHeaderConnToCardTable(const ConnToCardTableEntry* pConnToCardTableEntry);
	
    void AddTraceHeader                       (const TRACE_HEADER_S & header);
        
	
	virtual void AddData					  ( const DWORD  DataLen,
												const char*  pData);
    void AddPayload_len(BOOL Type);

	void SetCommonHeaderOpcode(const OPCODE opcode);
	void SetPortDescriptorHeaderLogicRsrcType1(const BYTE   logical_resource);
	void SetPortDescriptorHeaderRoomId(const WORD   room_id);

    void SetPhysicalInfoHeaderBoard_id(WORD board_id, const int Counter=0);
	
	void  ReadLoggerHeader(char* Mainbuf);

    STATUS  SendMsgToMplApiCommandDispatcher();
	virtual STATUS  SendMsgToCSApiCommandDispatcher();
    
	void  UpdateCommonHeaderToMPL (DWORD recivedSecNum = 0);


    const COMMON_HEADER_S & GetCommonHeaderConst()const{return m_MplMcmsCommonHeader;}
    const MESSAGE_DESCRIPTION_HEADER_S & GetMsgDescriptionHeaderConst()const{return m_MplMcmsMsgDescriptionHeader;}
    const CENTRAL_SIGNALING_HEADER_S & GetCSHeaderConst()const{return m_CS_McmsHeader;}

    const PHYSICAL_INFO_HEADER_S & GetPhysicalHeaderConst()const{return m_MplMcmsPhysicalInfoHeader[0];}
    PHYSICAL_INFO_HEADER_S & GetPhysicalHeader(){return m_MplMcmsPhysicalInfoHeader[0];}
    
    const PORT_DESCRIPTION_HEADER_S & GetPortDescriptorHeaderConst()const{return m_MplMcmsPortDescriptionHeader[0];}
    const TRACE_HEADER_S & GetTraceHeaderConst()const{return m_TraceHeader;}
    TRACE_HEADER_S & GetTraceHeader(){return m_TraceHeader;}
    const AUDIT_EVENT_HEADER_S & GetAuditHeaderConst()const{return m_AuditHeader;}

    
	OPCODE getOpcode() const;
	BYTE  GetPhysicalInfoHeaderCounter() const { return m_PhysicalInfoHeaderCounter; }
    
	void GetCommonHeaderCopy(COMMON_HEADER_S* pStruct) const
			{ memcpy(pStruct,&m_MplMcmsCommonHeader,sizeof(COMMON_HEADER_S)); }
	void GetMsgDescHeaderCopy(MESSAGE_DESCRIPTION_HEADER_S* pStruct) const
			{ memcpy(pStruct,&m_MplMcmsMsgDescriptionHeader,sizeof(MESSAGE_DESCRIPTION_HEADER_S)); }
	void GetPortDescHeaderCopy(PORT_DESCRIPTION_HEADER_S* pStruct) const
			{ memcpy(pStruct,&m_MplMcmsPortDescriptionHeader,sizeof(PORT_DESCRIPTION_HEADER_S)); }
	STATUS GetPhysicalInfoHeaderCopy(const int index,PHYSICAL_INFO_HEADER_S* pStruct) const;

    inline APIU8  getCommonHeaderProtocolVersion() const{return m_MplMcmsCommonHeader.protocol_version;} 
	inline APIU8  getCommonHeaderOption() const{return m_MplMcmsCommonHeader.opcode;} 
	inline APIU8  getCommonHeaderSrc_id() const{return m_MplMcmsCommonHeader.src_id;} 
	inline APIU8  getCommonHeaderDest_id() const{return m_MplMcmsCommonHeader.dest_id;}
	inline OPCODE getCommonHeaderOpcode() const{return m_MplMcmsCommonHeader.opcode;}
	inline APIU32 getCommonHeaderTime_stamp() const{return m_MplMcmsCommonHeader.time_stamp;}
	inline APIU32 getCommonHeaderSequence_num() const{return m_MplMcmsCommonHeader.sequence_num;}
	inline APIU32 getCommonHeaderPayload_len() const{return m_MplMcmsCommonHeader.payload_len;}
	inline APIU32 getCommonHeaderPayload_offset() const{return m_MplMcmsCommonHeader.payload_offset;}
	inline APIU32 getCommonHeaderNext_header_type() const{return m_MplMcmsCommonHeader.next_header_type;}
	inline APIU32 getCommonHeaderNext_header_size() const{return m_MplMcmsCommonHeader.next_header_offset;}

	inline APIU32 getMsgDescriptionHeaderRequest_id() const{return m_MplMcmsMsgDescriptionHeader.request_id;} 
	inline APIU32 getMsgDescriptionHeaderEntity_type() const{return m_MplMcmsMsgDescriptionHeader.entity_type;}
	inline APIU32 getMsgDescriptionHeaderTime_stamp() const{return m_MplMcmsMsgDescriptionHeader.time_stamp;}
	inline APIU32 getMsgDescriptionHeaderNext_header_type() const{return m_MplMcmsMsgDescriptionHeader.next_header_type;} 
	inline APIU32 getMsgDescriptionHeaderNext_header_size() const{return m_MplMcmsMsgDescriptionHeader.next_header_size;}
	
    APIU8   getPhysicalInfoHeaderBox_id(const int Counter=0) const;
	APIU8   getPhysicalInfoHeaderBoard_id(const int Counter=0) const;
	APIU8   getPhysicalInfoHeaderSub_board_id(const int Counter=0) const;
	APIU8   getPhysicalInfoHeaderUnit_id(const int Counter=0) const;
	APIU8   getPhysicalInfoHeaderAccelerator_id(const int Counter=0) const;
	APIU16  getPhysicalInfoHeaderPort_id(const int Counter=0) const;
	APIU8   getPhysicalInfoHeaderResource_type(const int Counter=0) const; 
	APIU8   getPhysicalInfoHeaderFuture_use1(const int Counter=0) const;
	APIU8   getPhysicalInfoHeaderFuture_use2(const int Counter=0) const;
	APIU32  getPhysicalInfoHeaderNext_header_type(const int Counter=0) const;
	APIU32  getPhysicalInfoHeaderNext_header_size(const int Counter=0) const;

	//////
	APIU32  getPortDescriptionHeaderParty_id(const int Counter=0) const;
	APIU32  getPortDescriptionHeaderConf_id(const int Counter=0) const;
	APIU32  getPortDescriptionHeaderConnection_id(const int Counter=0) const;
	APIU8   getPortDescriptionHeaderLogical_resource_type_1(const int Counter=0) const;
	APIU8   getPortDescriptionHeaderLogical_resource_type_2(const int Counter=0) const;
	APIU8   getPortDescriptionHeaderFuture_use1(const int Counter=0) const;
	APIU8   getPortDescriptionHeaderFuture_use2(const int Counter=0) const;
	APIU32  getPortDescriptionHeaderNext_header_type(const int Counter=0) const;
	APIU32  getPortDescriptionHeaderNext_header_size(const int Counter=0) const;
	APIU16  getPortDescriptionHeaderRoom_Id(const int Counter=0) const;

	/////////

	inline APIU16  getCentralSignalingHeaderCsId() const{return m_CS_McmsHeader.cs_id;}
	inline APIU32  getCentralSignalingHeaderCallIndex() const{return m_CS_McmsHeader.call_index;}
	inline APIU32  getCentralSignalingHeaderChannelIndex() const{return m_CS_McmsHeader.channel_index;}
	inline APIU32  getCentralSignalingHeaderMcChannelIndex() const{return m_CS_McmsHeader.mc_channel_index;}
	inline APIS32  getCentralSignalingHeaderStatus() const{return m_CS_McmsHeader.status;}
	inline APIU16	getCentralSignalingHeaderSrcUnitId() const{return m_CS_McmsHeader.src_unit_id;}
	inline APIU16	getCentralSignalingHeaderDestUnitId() const{return m_CS_McmsHeader.dst_unit_id;}
	inline APIU32  getCentralSignalingHeaderServiceId() const{return m_CS_McmsHeader.service_id;}
	inline APIU8  getPortDescriptionHeaderMsg_ack_ind() const{return m_MplMcmsMsgDescriptionHeader.msg_ack_ind; }

	BYTE  getPhysicalInfoHeaderCounter() const;
	BYTE  getPortDescriptionHeaderCounter() const;
	DWORD getDataLen() const;
	
	char* GetData(){return getpData();}
	const char* GetDataConst()const{return m_DataLen == 0 ? NULL : m_pData;}

	char* getpData();
	const char* getpData() const;

	DWORD getSequence_num() const;
	DWORD getCSStatus() const;

	void UpdateSendTimes();
	
	// Check of data size. 
	// in a case of failure the function prints an entire message, assert.
	// returns : STATUS_OK - a message can continue.
	//			 STATUS_WRONG_DATA_SIZE - a message should be rejected.
	STATUS ValidateDataSize(const DWORD expectedSize, BOOL assertOnGreaterSize = true) ;
  STATUS SendMessage(BOOL type,WORD numOfRetries = 0);

    void InitErrno();
    const MplMcmsProtocolErrno_S & Errno()const{return m_Errno;}
  void UpdateMessageDescriptionHeaderReqId(DWORD request_id);
    
	// Attributes
private:	
	
	DWORD GetUpdated_sequence_num();
	
	COMMON_HEADER_S						m_MplMcmsCommonHeader                                       ;
	MESSAGE_DESCRIPTION_HEADER_S		m_MplMcmsMsgDescriptionHeader                               ;
	CENTRAL_SIGNALING_HEADER_S          m_CS_McmsHeader                                    			;
	PHYSICAL_INFO_HEADER_S				m_MplMcmsPhysicalInfoHeader   [MAX_PHYSICAL_INFO_HEADERS]   ;
	BYTE								m_PhysicalInfoHeaderCounter                                 ;
	PORT_DESCRIPTION_HEADER_S			m_MplMcmsPortDescriptionHeader[MAX_PORT_DESCRIPTION_HEADERS];
	BYTE								m_PortDescriptionHeaderCounter                              ;
    TRACE_HEADER_S                      m_TraceHeader;
    AUDIT_EVENT_HEADER_S                m_AuditHeader;
    DWORD								m_DataLen                                                   ;
	char*								m_pData                                                     ;
	static DWORD						m_sequence_num                                              ;

    bool                                m_HeaderInitArray[NUM_OF_HEADER_TYPES]                      ;

    MplMcmsProtocolErrno_S              m_Errno;
};
#endif /* _SOCKET_HEADER */


