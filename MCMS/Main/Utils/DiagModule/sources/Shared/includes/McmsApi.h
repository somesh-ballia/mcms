/*
*****************************************************************************
*
* Copyright (C) 2005 POLYCOM NETWORKS Ltd.
* This file contains confidential information proprietary to POLYCOM NETWORKSO
*  Ltd. The use or disclosure of any information contained
* in this file without the written consent of an officer of POLYCOM NETWORKS
* Ltd is expressly forbidden.
*
*****************************************************************************

*****************************************************************************

 Module Name: McmsApi.h

 General Description:  Module "" contains:

      1. 

 Generated By:	Yigal Mizrahi       Date: 26.4.2005
                tzuck               Date: 27.6.2005

*****************************************************************************/
#ifndef _MCMSAPI_H_
#define _MCMSAPI_H_

/***** Include Files *****/
#include "DiagDataTypes.h"
#include "MplMcmsStructs.h"
#include "time.h"
#include "LinuxSystemCallsApi.h"

/*
typedef struct
{
	COMMON_HEADER_S              tCommonHeader;
	MESSAGE_DESCRIPTION_HEADER_S tMessageDescription;
	PHYSICAL_INFO_HEADER_S       tPhysicalInfoHeader;
}   TGeneralMcmsCommonHeader;

typedef struct
{
	TGeneralMcmsCommonHeader 	 tGeneralMcmsCommonHeader;	
	PORT_DESCRIPTION_HEADER_S	 tPortDescriptionHeader;	
}   TPortMessagesHeader;
*/

typedef struct
{
   TPortMessagesHeader	tPortMessagesHeader;
   ACK_IND_S       		tAckMessageContent ;
}TPortMessageAckType;

typedef struct
{
   TGeneralMcmsCommonHeader 	 tGeneralMcmsCommonHeader;
   ACK_IND_S           			 tAckMessageContent ;
}TGeneralMessageAckType;


typedef struct 
{
	UINT32 ulStatus ;
	UINT32 ulReason ;
	UINT32 ulMediaType ;
	UINT32 ulMediaDirection ;
}tAckParams ;


typedef struct
{
    TPortMessagesHeader	tPortMessagesHeader;
    USER_MSG_S          tAlarmFaultMsg ;
}TPortMessageAlarmFaultType;



#define General_Message_Ack_Type    1
#define Port_Message_Ack_Type	2
#define Close_Udp_Message_Ack_Type	3
#define Connect_Message_Ack_Type	4

#define GET_COMMON_HEADER_PROTOCOL_VERSION(p)      		 	(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.protocol_version)
#define GET_COMMON_HEADER_OPTION(p)               		 	(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.option)
#define GET_COMMON_HEADER_SRC_ID(p)                		 	(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.src_id)
#define GET_COMMON_HEADER_DEST_ID(p)               		 	(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.dest_id)
#define GET_COMMON_HEADER_OPCODE(p)                		 	SWAPL(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.opcode)
#define GET_COMMON_HEADER_TIME_STAMP(p)            		 	SWAPL(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.time_stamp)
#define GET_COMMON_HEADER_SEQUENCE_NUM(p)         		 	SWAPL(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.sequence_num)
#define GET_COMMON_HEADER_PAYLOD_LEN(p)            		 	SWAPL(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.payload_len)
#define GET_COMMON_HEADER_PAYLOAD_OFFSET(p)        		 	SWAPL(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.payload_offset)
#define GET_COMMON_HEADER_NEXT_HEADER_TYPE(p)      		 	SWAPL(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.next_header_type)
#define GET_COMMON_HEADER_NEXT_HEADER_SIZE(p)      		 	SWAPL(((TGeneralMcmsCommonHeader*)p)->tCommonHeader.next_header_offset)

#define GET_MESSAGE_DESCRIPTION_HEADER_REQUEST_ID(p)        SWAPL(((TGeneralMcmsCommonHeader*)p)->tMessageDescription.request_id)
#define GET_MESSAGE_DESCRIPTION_HEADER_ENTITY_TYPE(p)       SWAPL(((TGeneralMcmsCommonHeader*)p)->tMessageDescription.entity_type)
#define GET_MESSAGE_DESCRIPTION_HEADER_TIME_STEMP(p)        SWAPL(((TGeneralMcmsCommonHeader*)p)->tMessageDescription.time_stamp)
#define GET_MESSAGE_DESCRIPTION_HEADER_NEXT_HEADER_TYPE(p)  SWAPL(((TGeneralMcmsCommonHeader*)p)->tMessageDescription.next_header_type)
#define GET_MESSAGE_DESCRIPTION_HEADER_NEXT_HEADER_SIZE(p)  SWAPL(((TGeneralMcmsCommonHeader*)p)->tMessageDescription.next_header_size)


#define GET_PHYSICAL_INFO_HEADER_BOX_ID(p)         		 	(((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.box_id)
#define GET_PHYSICAL_INFO_HEADER_BOARD_ID(p)       		 	(((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.board_id)
#define GET_PHYSICAL_INFO_HEADER_SUB_BOARD_ID(p)   		 	(((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.sub_board_id)
#define GET_PHYSICAL_INFO_HEADER_UNIT_ID(p)        		 	(((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.unit_id)
#define GET_PHYSICAL_INFO_HEADER_PORT_ID(p)        		 	(((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.port_id)
#define GET_PHYSICAL_INFO_HEADER_RESOURCE_TYPE(p)  			(((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.resource_type)
#define GET_PHYSICAL_INFO_NEXT_HEADER_TYPE(p)      		 	SWAPL(((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.next_header_type)
#define GET_PHYSICAL_INFO_NEXT_HEADER_SIZE(p)      		    SWAPL(((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.next_header_size)


#define GET_PORT_DESCRIPTION_HEADER_PARTY_ID(p)				SWAPL(((TPortMessagesHeader*)p)->tPortDescriptionHeader.party_id)
#define GET_PORT_DESCRIPTION_HEADER_CONF_ID(p)				SWAPL(((TPortMessagesHeader*)p)->tPortDescriptionHeader.conf_id)
#define GET_PORT_DESCRIPTION_HEADER_CONNECTION_ID(p)		SWAPL(((TPortMessagesHeader*)p)->tPortDescriptionHeader.connection_id)
#define GET_PORT_DESCRIPTION_HEADER_LOG_RSC1_TYPE(p)		(((TPortMessagesHeader*)p)->tPortDescriptionHeader.logical_resource_type_1)
#define GET_PORT_DESCRIPTION_HEADER_LOG_RSC2_TYPE(p)		(((TPortMessagesHeader*)p)->tPortDescriptionHeader.logical_resource_type_2)
#define GET_PORT_DESCRIPTION_HEADER_NEXT_HEADER_TYPE(p)     SWAPL(((TPortMessagesHeader*)p)->tPortDescriptionHeader.next_header_type)
#define GET_PORT_DESCRIPTION_HEADER_NEXT_HEADER_SIZE(p)     SWAPL(((TPortMessagesHeader*)p)->tPortDescriptionHeader.next_header_size)

#define SET_COMMON_HEADER_PROTOCOL_VERSION(p, val)      	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.protocol_version = val )
#define SET_COMMON_HEADER_OPTION(p, val)               		( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.option = val )
#define SET_COMMON_HEADER_SRC_ID(p, val)                	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.src_id = val )
#define SET_COMMON_HEADER_DEST_ID(p, val)          		 	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.dest_id = val )
#define SET_COMMON_HEADER_OPCODE(p, val)                	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.opcode = SWAPL(val) )
#define SET_COMMON_HEADER_TIME_STAMP(p, val)            	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.time_stamp = SWAPL(val) )
#define SET_COMMON_HEADER_SEQUENCE_NUM(p, val)         		( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.sequence_num = SWAPL(val) )
#define SET_COMMON_HEADER_PAYLOD_LEN(p, val)            	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.payload_len = SWAPL(val) )
#define SET_COMMON_HEADER_PAYLOAD_OFFSET(p, val)        	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.payload_offset = SWAPL(val) )
#define SET_COMMON_HEADER_NEXT_HEADER_TYPE(p, val)      	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.next_header_type = SWAPL(val) )
#define SET_COMMON_HEADER_NEXT_HEADER_SIZE(p, val)      	( ((TGeneralMcmsCommonHeader*)p)->tCommonHeader.next_header_offset = SWAPL(val) )

#define SET_PHYSICAL_INFO_HEADER_BOX_ID(p, val)    		 	( ((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.box_id = val )
#define SET_PHYSICAL_INFO_HEADER_BOARD_ID(p, val)  		 	( ((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.board_id = val )
#define SET_PHYSICAL_INFO_HEADER_SUB_BOARD_ID(p, val)	 	( ((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.sub_board_id = val )
#define SET_PHYSICAL_INFO_HEADER_UNIT_ID(p, val)     	 	( ((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.unit_id = val )
#define SET_PHYSICAL_INFO_HEADER_PORT_ID(p, val)     	 	( ((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.port_id = val )
#define SET_PHYSICAL_INFO_HEADER_RESOURCE_TYPE(p, val) 		( ((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.resource_type = val )
#define SET_PHYSICAL_INFO_NEXT_HEADER_TYPE(p, val) 		 	( ((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.next_header_type = SWAPL(val) )
#define SET_PHYSICAL_INFO_NEXT_HEADER_SIZE(p, val)      	( ((TGeneralMcmsCommonHeader*)p)->tPhysicalInfoHeader.next_header_size = SWAPL(val) )

#define SET_MESSAGE_DESCRIPTION_HEADER_REQUEST_ID(p, val)	( ((TGeneralMcmsCommonHeader*)p)->tMessageDescription.request_id = SWAPL(val))


#define SET_PORT_DESCRIPTION_HEADER_PARTY_ID(p, val)		( ((TPortMessagesHeader*)p)->tPortDescriptionHeader.party_id = SWAPL(val) )
#define SET_PORT_DESCRIPTION_HEADER_CONF_ID(p, val)			( ((TPortMessagesHeader*)p)->tPortDescriptionHeader.conf_id = SWAPL(val) )
#define SET_PORT_DESCRIPTION_HEADER_CONNECTION_ID(p, val)	( ((TPortMessagesHeader*)p)->tPortDescriptionHeader.connection_id = SWAPL(val) )
#define SET_PORT_DESCRIPTION_HEADER_LOG_RSC1_TYPE(p, val)	( ((TPortMessagesHeader*)p)->tPortDescriptionHeader.logical_resource_type_1 = val )
#define SET_PORT_DESCRIPTION_HEADER_LOG_RSC2_TYPE(p, val)	( ((TPortMessagesHeader*)p)->tPortDescriptionHeader.logical_resource_type_2 = val )
#define SET_PORT_DESCRIPTION_HEADER_NEXT_HEADER_TYPE(p, val) ( ((TPortMessagesHeader*)p)->tPortDescriptionHeader.next_header_type = SWAPL(val) )
#define SET_PORT_DESCRIPTION_HEADER_NEXT_HEADER_SIZE(p, val) ( ((TPortMessagesHeader*)p)->tPortDescriptionHeader.next_header_size = SWAPL(val) )

extern UINT32 *AllocateMcmsApiMsg(UINT32 length, UINT32 opcode); //move to yigal's code
extern UINT32 *AllocatePortMcmsApiMsg(UINT32 length, UINT32 opcode);
extern UINT32 *CmBuildAckMsg(void *pOriginalMsg,tAckParams *ptAckParams,UINT32 ulAckMsgType);

#endif //_MCMSAPI_H_
