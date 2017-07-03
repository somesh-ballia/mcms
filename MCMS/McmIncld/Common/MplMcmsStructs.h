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
// FILE:       MplMcmsSructs.h                                             |
// SUBSYSTEM:  MplApi                                                      |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// KT  | 13.07.05   | server_id is replced by service_id in CS header      |
//+========================================================================+

#ifndef   __MPLMCMSSTRUCT_H__
#define   __MPLMCMSSTRUCT_H__

#include "DataTypes.h"

#define APIU32_ALIGNMENT                APIU8*4 // 4 bytes  for structs alignment
#define STR_OPCODE_LEN                  96//25*APIU32_ALIGNMENT// 100 bytes
#define MPL_API_LISTEN_SOCKET_PORT_NUM  10005
#define CS_API_LISTEN_SOCKET_PORT_NUM   10008
#define BAD_SPONTANEOUS_IND_DESCRIPTION_MAX_LENGTH                40 // 160 bytes (must be 32bit aligned)

#define INVALID_BOARD_ID 				0XFF

typedef struct
{
   APIU8  version_num;    // Always 3
   APIU8  reserved;       // Always 0
   APIU16 payload_len;
} TPKT_HEADER_S;


typedef struct
{
	APIU8  protocol_version;
	APIU8  option; //(praiority flag,encoding)
	APIU8  src_id;  //look at eMainEntities
	APIU8  dest_id;
	APIU32 opcode;
	APIU32 time_stamp;
	APIU32 sequence_num;
	APIU32 payload_len;
	APIU32 payload_offset;
	APIU32 next_header_type;
	APIU32 next_header_offset;

} COMMON_HEADER_S;


typedef struct
{
	APIU32	log_level;	// values from 1 to 9
	APIU32	entity_type;
	APIU32	unit_id;
	APIU32  conf_id;
	APIU32  party_id;
	APIU32	time_stamp;
	APIU32	opcode;
	char	str_opcode [STR_OPCODE_LEN];
	APIU32  next_header_type;
	APIU32  next_header_size;

} LOGGER_HEADER_S;

typedef struct
{
	APIU8   box_id;
	APIU8   board_id;
	APIU8   sub_board_id;
	APIU8   unit_id;
	APIU8   accelerator_id;
	APIU16  port_id;
	APIU8   resource_type; // look at eResourceTypes
	APIU8   future_use1; // in use for ART channel id in ISDN
	APIU8   future_use2; // in use for ART channel id in ISDN
	APIU8   reserved[2];
	APIU32  next_header_type;
	APIU32  next_header_size;

} PHYSICAL_INFO_HEADER_S;


typedef struct
{
	APIU32  party_id;
	APIU32  conf_id;
	APIU32  connection_id;
	APIU8   logical_resource_type_1; //look at eLogicalResourceTypes
	APIU8   logical_resource_type_2;
	APIU16  room_id;
/* 	APIU8   future_use1; */
/* 	APIU8   future_use2; */
	APIU32  next_header_type;
	APIU32  next_header_size;

} PORT_DESCRIPTION_HEADER_S;


typedef struct
{
	APIU32   request_id;  //change name from seq_number according to Kiril request
	APIU32   entity_type;
	APIU32   time_stamp;
	APIU8    msg_ack_ind;
        APIU8    future_use1;
        APIU16   future_use2;      
	APIU32   next_header_type;
	APIU32   next_header_size;

} MESSAGE_DESCRIPTION_HEADER_S;


typedef struct
{
	COMMON_HEADER_S              tCommonHeader;
	MESSAGE_DESCRIPTION_HEADER_S tMessageDescription;
	PHYSICAL_INFO_HEADER_S       tPhysicalInfoHeader;
} TGeneralMcmsCommonHeader;

typedef struct
{
	TGeneralMcmsCommonHeader 	 tGeneralMcmsCommonHeader;	
	PORT_DESCRIPTION_HEADER_S	 tPortDescriptionHeader;	
} TPortMessagesHeader;

typedef struct
{
	APIU16 cs_id;                  // Central Signaling Id

	APIU16 src_unit_id;            // CS Source unit Id for internal routing
    APIU16 dst_unit_id;            // CS Destination unit Id for internal routing
    APIU16 reserved;

	APIU32 call_index;             // CS Call index
	APIU32 service_id;             // IP/Conference Service indentifier

	APIU32 channel_index;          // CS Channel index
	APIU32 mc_channel_index;       // MCMS Channle index

	APIS32 status;	               // CS Status returned to MCMS

	APIU16 next_header_type;       // Next header type
	APIU16 next_header_offset;     // Next header offset in bytes


} CENTRAL_SIGNALING_HEADER_S;




/////////////////////////////////////////////////////////////////////
//the following definitions will be included in another file and will be removed
// from here soon ,I will update you about the change.- Thanks,Shlomit.

enum eCsStaticUnits{
    eMcmsSim        = 0,
    eBalancer       = 10,
    eSipBalancer	= 15,
    eConfigurator   = 20,
    eServiceMngr    = 30,
    eStartUp        = 40,
    eGk			    = 50
 };





/*---------------------------------------------------------------------------------
	After addition of new enum field don't forget to add a string to arrays
(see one page down)
---------------------------------------------------------------------------------*/

// enum eMainEntities
// 	{
// 		eMcms,
// 		eMpl,
// 		eCentral_signaling,
// 		eMpl_simulation,
// 		eEma,
// 		eSwitch_entity,  // ('_entity' in order to distinguish it from eSwitch in eCardType enum)
// 		eShelf,

// 		NUM_OF_MAIN_ENTITIES
// 	}  ;

enum eMainEntities
	{
		eMcms = 0,
		eMpl,
		eCentral_signaling,
		eMpl_simulation,
		eEma,
		eCM_Switch,  // ('_entity' in order to distinguish it from eSwitch in eCardType enum)
		eShelf,
		eArtEntity,
		eVideoEntity,
		eCardManagerEntity,
		eRTMEntity,
		eMuxEntity,
		eVmpEntity,
		eAmpEntity,
		eMpProxyEntity,
		NUM_OF_MAIN_ENTITIES
	}  ;

#define MAX_LEN_OF_MAIN_ENTITIES  18//sizeof("eCentral_signaling")
enum eResourceTypes
	{
		ePhysical_res_none          = 0,
		ePhysical_audio_controller,
		ePhysical_art,
		ePhysical_art_light,
		ePhysical_video_encoder,
		ePhysical_video_decoder,
		ePhysical_rtm,
		ePhysical_ivr_controller,
        ePhysical_mux,
        ePhysical_mrmp,
		NUM_OF_RESOURCE_TYPES
	} ;
#define MAX_LEN_OF_RESOURCE_TYPES  26//sizeof("ePhysical_audio_controller")

enum eLogicalResourceTypes
	{
		eLogical_res_none        = 0,
		eLogical_audio_encoder   = 1,
		eLogical_audio_decoder,
        	eLogical_audio_controller,
		eLogical_video_encoder,
		eLogical_video_encoder_content,
		eLogical_video_decoder,
		eLogical_rtp,
		eLogical_ip_signaling,
		eLogical_net,
		eLogical_ivr_controller,
		eLogical_mux,
		eLogical_COP_CIF_encoder,
		eLogical_COP_4CIF_encoder,
		eLogical_COP_Dynamic_decoder,
		eLogical_COP_VSW_encoder,
		eLogical_COP_VSW_decoder,
		eLogical_COP_PCM_encoder,
		eLogical_COP_LM_decoder,
		eLogical_COP_HD720_encoder,
		eLogical_COP_HD1080_encoder, //eLogical_COP_HD1080_Slave_encoder,
		eLogical_COP_dummy_encoder,
		eLogical_PCM_manager,
		eLogical_VSW_dummy_encoder,
		eLogical_VSW_dummy_decoder,
		eLogical_COP_HD1080_decoder,
		eLogical_content_rtp,
		eLogical_relay_rtp,    // for soft mcu
		eLogical_relay_audio_encoder,
		eLogical_relay_audio_decoder,
		eLogical_relay_video_encoder,
		eLogical_relay_avc_to_svc_video_encoder_1,
		eLogical_relay_avc_to_svc_video_encoder_2,
		eLogical_relay_svc_to_avc_rtp,
		eLogical_legacy_to_SAC_audio_encoder, // mix on RMX
        	eLogical_relay_avc_to_svc_rtp,
        	eLogical_relay_avc_to_svc_rtp_with_audio_encoder,
		eLogical_ip_sigOrganizer,             // MS Lync
                eLogical_ip_sigFocus,
                eLogical_ip_sigEventPackage,


		NUM_OF_LOGICAL_RESOURCE_TYPES //must always stay last - used in loop as last
	} ;
#define MAX_LEN_OF_LOGICAL_RESOURCE_TYPES  39//sizeof("eLogical_audio_controller")

enum eHeaderType
	{
		eHeaderNone     = 0,
		eHeaderTpkt     = 1,
		eHeaderCommon,
		eHeaderPhysical,
		eHeaderPortDesc,
		eHeaderMsgDesc,
		eHeaderCs,
		eHeaderTrace,
        eHeaderAudit,
		eHeaderUnknown, // must be last
		NUM_OF_HEADER_TYPES
	};
#define MAX_LEN_OF_HEADER_TYPES  15//sizeof("eHeaderPortDesc")


typedef struct
{
   APIU32   dummy;
} ESTABLISH_CONNECTION_S;
typedef struct
{
   APIU32   dummy;
} REESTABLISH_CONNECTION_S;

/************************ Acknowledge Struct ********************************/
typedef struct
{
	APIU32	ack_opcode;
	APIU32	ack_seq_num;
	APIU32	status;
	APIU32	reason;
} ACK_BASE_S;

typedef struct
{
	ACK_BASE_S	ack_base;
	APIU32		media_type;
	APIU32		media_direction;
	APIU32      channelHandle;
} ACK_IND_S;

typedef struct
{
	APIU32		reason;
	APIU32		description [BAD_SPONTANEOUS_IND_DESCRIPTION_MAX_LENGTH];

} BAD_SPONTANEOUS_IND_S;

///////////////////////////////////
// IVR message file structure
typedef struct
{
	unsigned char  szIdCode[4];
	unsigned long  dwVersionNumber;
	unsigned long  dwContent;
	unsigned long  dwIconSz;
	unsigned long  dwDataSz;
    unsigned long  dwQCIFSz;

} INT_MESSAGE_HEADER_S;


//-----------------------------------------------------
//  For producing Fault/Alert from Emb and CS
//-----------------------------------------------------
typedef struct
{
	APIU32  messageCode;					// message code : eUserMsgCodeCS/eUserMsgCodeEmb
    APIU32  location;						// Faults list/Alerts list : eUserMsgLocation
    APIU32  operation;						// Add/remove: eUSerMsgOperation
    APIU32  autoRemoval;					// Alert removal upon a certain even: eUserMsgAutoRemoval
    APIU32  process_type;					// Generator of the message (a process of CS/Emb) : eUserMsgProcessType_CS/eUserMsgProcessType_Emb

    APIU32  future_use1;
    APIU32  future_use2;
} USER_MSG_S;

enum eUserMsgCode_CS
{
	eUserMsgCode_Cs_SipTLS_RegistrationHandshakeFailure = 0,
    eUserMsgCode_Cs_SipTLS_FailedToLoadOrVerifyCertificateFiles,
    eUserMsgCode_Cs_SipTLS_RegistrationTransportError,
    eUserMsgCode_Cs_SipTLS_RegistrationServerNotResponding,
    eUserMsgCode_Cs_SipTLS_CertificateHasExpired,
    eUserMsgCode_Cs_SipTLS_CertificateWillExpireInLessThanAWeek,
    eUserMsgCode_Cs_SipTLS_CertificateSubjNameIsNotValid_Or_DnsFailed,

    eUserMsgCode_Cs_EdgeServerDnsFailed,
    eUserMsgCode_Cs_SipTLS_RemoteCertificateFailure,
    eUserMsgCode_Cs_SipTLS_RmxCertificateFailure,

    NUM_OF_USER_MSG_MESSAGE_CODES_CS
};



enum eUserMsgCode_Emb
{
	eUserMsgMessage_Emb_Test = 0,
	eUserMsgMessage_Emb_UBootFlashFailure,
    eUserMsgMessage_Emb_FpgaVersLoadesFailure,
	eUserMsgMessage_Emb_FAN_SpeedBelowMinimum,
	eUserMsgMessage_Emb_FAN_NoPower,
	eUserMsgMessage_Emb_FSM_NoCard,
	eUserMsgMessage_Emb_FSM4000_LoadingProblem,
	eUserMsgMessage_Emb_FSM4000_NoLinkToCard1,
	eUserMsgMessage_Emb_FSM4000_NoLinkToCard2,
	eUserMsgMessage_Emb_FSM4000_NoLinkToCard3,
	eUserMsgMessage_Emb_FSM4000_NoLinkToCard4,
	eUserMsgMessage_Emb_FLASH_Erase,
    eUserMsgMessage_Emb_Card1_not_supported,
    eUserMsgMessage_Emb_Card2_not_supported,
    eUserMsgMessage_Emb_Card3_not_supported,
    eUserMsgMessage_Emb_Card4_not_supported,
	eUserMsgMessage_Emb_Card_not_supported_with_old_PSU,
	eUserMsgMessage_Emb_Card_not_supported_with_old_CNTL,


    NUM_OF_USER_MSG_MESSAGE_CODES_EMB
};


enum eUserMsgLocation
{
    eUserMsgLocation_SysAlerts			= 0,			// faults and alert together
	eUserMsgLocation_Faults				= 1,			// faults only

    NUM_OF_USER_MSG_LOCATIONS
};

enum eUserMsgOperation
{
	eUSerMsgOperation_add				= 0,
    eUSerMsgOperation_remove			= 1,

    NUM_OF_USER_MSG_OPERATIONS
};



enum eUserMsgAutoRemoval
{
	eUSerMsgAutoRemoval_none			= 0,
    eUSerMsgAutoRemoval_cardRemove		= 1,

    NUM_OF_USER_MSG_AUTO_REMOVALS
};



enum eUserMsgProcessType_CS
{
	eUserMsgProcessType_Cs_Sip = 0,
    eUserMsgProcessType_Cs_H323,
    eUserMsgProcessType_Cs_Infrastructure,

    NUM_OF_USER_MSG_PROCESS_TYPES_CS
};



enum eUserMsgProcessType_Emb
{
	eUserMsgProcessType_Emb_1 = 0,

    NUM_OF_USER_MSG_PROCESS_TYPES_EMB
};

enum eUserMsgSubErrorCode1_CS{
  TLS_CERTIFICATE_VALID						= 0,			// certificate valid
  TLS_UNABLE_TO_GET_ISSUER_CERT				= 0x1,			// unable to get issuer certificate 
  TLS_UNABLE_TO_GET_CRL						= 0x2,			// the CRL of a certificate could not be found
  TLS_UNABLE_TO_DECRYPT_CERT_SIGNATURE		= 0x4,			// unable to decrypt certificate's signature 
  TLS_UNABLE_TO_DECRYPT_CRL_SIGNATURE		= 0x8,			// unable to decrypt CRL's signature 
  TLS_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY	= 0x10,			// unable to decode issuer public key 
  TLS_CERT_SIGNATURE_FAILURE				= 0x20,			// certificate signature failure 
  TLS_CRL_SIGNATURE_FAILURE					= 0x40,			// CRL signature failure 
  TLS_CERT_NOT_YET_VALID					= 0x80,			// certificate is not yet valid 
  TLS_CERT_HAS_EXPIRED						= 0x100,		// certificate has expired 
  TLS_CRL_NOT_YET_VALID						= 0x200,		// CRL is not yet valid 
  TLS_CRL_HAS_EXPIRED						= 0x400,		// CRL has expired 
  TLS_ERROR_IN_CERT_NOT_BEFORE_FIELD		= 0x800,		// format error in certificate's notBefore field 
  TLS_ERROR_IN_CERT_NOT_AFTER_FIELD			= 0x1000,		// format error in certificate's notAfter field 
  TLS_ERROR_IN_CRL_LAST_UPDATE_FIELD		= 0x2000,		// format error in CRL's lastUpdate field 
  TLS_ERROR_IN_CRL_NEXT_UPDATE_FIELD		= 0x4000,		// format error in CRL's nextUpdate field 
  TLS_OUT_OF_MEM							= 0x8000,		// out of memory 
  TLS_DEPTH_ZERO_SELF_SIGNED_CERT			= 0x10000,		// self signed certificate 
  TLS_SELF_SIGNED_CERT_IN_CHAIN				= 0x20000,		// self signed certificate in certificate chain 
  TLS_UNABLE_TO_GET_ISSUER_CERT_LOCALLY		= 0x40000,		// unable to get local issuer certificate 
  TLS_UNABLE_TO_VERIFY_LEAF_SIGNATURE		= 0x80000,		// unable to verify the first certificate 
  TLS_CERT_CHAIN_TOO_LONG					= 0x100000,		// certificate chain too long 
  TLS_CERT_REVOKED							= 0x200000,		// certificate revoked 
  TLS_INVALID_CA							= 0x400000,		// invalid CA certificate 
  TLS_PATH_LENGTH_EXCEEDED					= 0x800000,		// path length constraint exceeded 
  TLS_INVALID_PURPOSE						= 0x1000000,	// unsupported certificate purpose 
  TLS_CERT_UNTRUSTED						= 0x2000000,	// certificate not trusted 
  TLS_CERT_REJECTED							= 0x4000000,	// certificate rejected 
  TLS_SUBJECT_ISSUER_MISMATCH				= 0x8000000,	// subject issuer mismatch 
  TLS_AKID_SKID_MISMATCH					= 0x10000000,	// authority and subject key identifier mismatch 
  TLS_AKID_ISSUER_SERIAL_MISMATCH			= 0x20000000,	// authority and issuer serial number mismatch 
  TLS_KEYUSAGE_NO_CERTSIGN					= 0x40000000	// key usage does not include certificate signing 
  
} ;

enum eUserMsgSubErrorOCSPCode2_CS{
  TLS_SPC_OCSPRESULT_CERTIFICATE_VALID      = 0,			// ocsp certificate valid
  TLS_SPC_OCSPRESULT_CERTIFICATE_REVOKED    = 0x1,			// ocsp certificate revoked
  TLS_SPC_OCSPRESULT_ERROR_INVALIDRESPONSE	= 0x2,			// ocsp invalid response
  TLS_SPC_OCSPRESULT_ERROR_CONNECTFAILURE   = 0x4,			// ocsp connect failure
  TLS_SPC_OCSPRESULT_ERROR_SIGNFAILURE      = 0x8,			// ocsp sign failure
  TLS_SPC_OCSPRESULT_ERROR_BADOCSPADDRESS   = 0x10,			// ocsp bad address
  TLS_SPC_OCSPRESULT_ERROR_OUTOFMEMORY      = 0x20,			// ocsp out of memory
  TLS_SPC_OCSPRESULT_ERROR_UNKNOWN          = 0x40,			// ocsp unknown error
  TLS_SPC_OCSPRESULT_ERROR_UNAUTHORIZED     = 0x80,			// ocsp unautorized
  TLS_SPC_OCSPRESULT_ERROR_SIGREQUIRED      = 0x100,		// ocsp sign required
  TLS_SPC_OCSPRESULT_ERROR_TRYLATER         = 0x200,		// ocsp try later
  TLS_SPC_OCSPRESULT_ERROR_INTERNALERROR    = 0x400,		// ocsp internal error
  TLS_SPC_OCSPRESULT_ERROR_MALFORMEDREQUEST = 0x800			// ocsp mal formed request
} ;


#endif /* __MPLMCMSSTRUCT_H__ */

