#ifndef OPCODESRANGES_H_
#define OPCODESRANGES_H_






/*=====================================================
 PROCESS CONFPARTY RANGE:     =   1000001 -> 2000000
=======================================================*/
#define CONF_PARTY_FIRST_OPCODE_IN_RANGE			1000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define CONF_PARTY_COMMON_FIRST_OPCODE			1010000
//#define CONF_PARTY_COMMON_LAST_OPCODE				1019999

// ===== with CardMngrIvrCntrl (file MCMS/McmIncld/MPL/Card/CardMngrIvrCntl/OpcodesMcmsCardMngrIvrCntl.h)
#define CONF_PARTY_CARDMNGR_IVR_CNTL_FIRST_OPCODE		1020000

#define CONF_PARTY_CARDMNGR_IVR_CNTL_REQ_FIRST_OPCODE	1020001
#define CONF_PARTY_CARDMNGR_IVR_CNTL_REQ_LAST_OPCODE	1024999

#define CONF_PARTY_CARDMNGR_IVR_CNTL_IND_FIRST_OPCODE	1025000
#define CONF_PARTY_CARDMNGR_IVR_CNTL_IND_LAST_OPCODE	1029998

#define CONF_PARTY_CARDMNGR_IVR_CNTL_LAST_OPCODE		1029999

// ===== with CardMngrTopologyBuilder (file MCMS/McmIncld/MPL/Card/CardMngrTB/OpcodesMcmsCardMngrTB.h)
#define CONF_PARTY_CARDMNGR_TB_FIRST_OPCODE			1030000
#define CONF_PARTY_CARDMNGR_TB_LAST_OPCODE			1039999

// ===== with Audio (file MCMS/McmIncld/MPL/Card/PhisycalPortART/Audio/OpcodesMcmsAudio.h)
#define CONF_PARTY_AUDIO_FIRST_OPCODE				1040000

#define CONF_PARTY_AUDIO_REQ_FIRST_OPCODE			1040001
#define CONF_PARTY_AUDIO_REQ_LAST_OPCODE			1044999

#define CONF_PARTY_AUDIO_IND_FIRST_OPCODE			1045000
#define CONF_PARTY_AUDIO_IND_LAST_OPCODE			1049998

#define CONF_PARTY_AUDIO_LAST_OPCODE				1049999

// ===== with AudioCntl (file MCMS/McmIncld/MPL/Card/PhisycalPortAudioCntl/OpcodesMcmsAudioCntl.h)
#define CONF_PARTY_AUDIO_CNTL_FIRST_OPCODE			1050000

#define CONF_PARTY_AUDIO_CNTL_REQ_FIRST_OPCODE		1050001
#define CONF_PARTY_AUDIO_CNTL_REQ_LAST_OPCODE		1054999

#define CONF_PARTY_AUDIO_CNTL_IND_FIRST_OPCODE		1055000
#define CONF_PARTY_AUDIO_CNTL_IND_LAST_OPCODE		1059998

#define CONF_PARTY_AUDIO_CNTL_LAST_OPCODE			1059999

// ===== with Video (file MCMS/McmIncld/MPL/Card/PhisycalPortVideo/OpcodesMcmsVideo.h)
#define CONF_PARTY_VIDEO_FIRST_OPCODE				1060000

#define CONF_PARTY_VIDEO_REQ_FIRST_OPCODE			1060001
#define CONF_PARTY_VIDEO_REQ_LAST_OPCODE			1064999

#define CONF_PARTY_VIDEO_IND_FIRST_OPCODE			1065000
#define CONF_PARTY_VIDEO_IND_LAST_OPCODE			1069998

#define CONF_PARTY_VIDEO_LAST_OPCODE				1069999

// ===== with CM (file IncludeExternal/MPL/Card/CardMngrIpMedia/OpcodesMcmsCardMngrIpMedia.h)
#define CONF_PARTY_CARDMNGR_MEDIA_FIRST_OPCODE	1070000	
#define CONF_PARTY_CARDMNGR_MEDIA_LAST_OPCODE   1079999

// ===== with MUX / Bonding (file MCMS/McmIncld/MPL/Card/PhisycalPortART/MUX/OpcodesMcmsBonding.h;OpcodesMcmsMux.h)
#define CONF_PARTY_BONDING_FIRST_OPCODE				1080000
#define CONF_PARTY_BONDING_LAST_OPCODE				1080999

#define CONF_PARTY_MUX_FIRST_OPCODE				    1081000
#define CONF_PARTY_MUX_LAST_OPCODE				    1082999

// ===== with PCM  (file IncludeExternal/MPL/Card/CardMngrPCM/OpcodesMcmsPCM.h
#define CONF_PARTY_PCM_FIRST_OPCODE					1083000
#define CONF_PARTY_PCM_LAST_OPCODE					1083500

// ===== with CM (file IncludeExternal/MPL/Card/CardMngrICE/OpcodesMcmsCardMngrICE.h)
#define CONF_PARTY_ICE_FIRST_OPCODE					1090000
#define CONF_PARTY_ICE_LAST_OPCODE   				1090999

// ===== with CM (file IncludeExternal/MPL/Card/CardMngrTIP/OpcodesMcmsCardMngrTIP.h)
#define CONF_PARTY_TIP_FIRST_OPCODE					1100000
#define CONF_PARTY_TIP_LAST_OPCODE   				1100999

// ===== with CM (file IncludeExternal/MPL/Card/CardMngrTIP/OpcodesMcmsCardMngrBFCP.h)
#define CONF_PARTY_BFCP_FIRST_OPCODE				1200000
#define CONF_PARTY_BFCP_LAST_OPCODE   				1200999

// ===== with CM (file IncludeExternal/MPL/Card/PhysicalMrmp/OpcodesMcmsCardMngrMrc.h) /* for softMcu eyaln_to_merge */
#define CONF_PARTY_MRC_FIRST_OPCODE					1110000
#define CONF_PARTY_MRC_LAST_OPCODE   				1110999 /* for softMcu eyaln_to_merge */

// ===== with NET (file MCMS/McmIncld/MPL/Card/PhisycalPortNetISDN/OpcodesMcmsNetQ931.h)
#define CONF_PARTY_NET_Q931_FIRST_OPCODE			1970000

#define CONF_PARTY_NET_Q931_REQ_FIRST_OPCODE		1970001
#define CONF_PARTY_NET_Q931_REQ_LAST_OPCODE			1974999

#define CONF_PARTY_NET_Q931_IND_FIRST_OPCODE		1975000
#define CONF_PARTY_NET_Q931_IND_LAST_OPCODE			1979998

#define CONF_PARTY_NET_Q931_LAST_OPCODE				1979999

//====== IP Media (file MCMS/McmIncld/MPL/Card/CardCommon/IpMfaOpcodes.h)
#define PARTY_IP_MEDIA_FIRST_OPCODE_IN_RANGE					  14000001
#define PARTY_IP_MEDIA_LAST_OPCODE_IN_RANGE					  15000000

// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define CONF_PARTY_INTERNAL_MCMS_FIRST_OPCODE		1980000
#define CONF_PARTY_INTERNAL_MCMS_LAST_OPCODE		1989999

// ===== internal CONFPARTY opcodes
//#define CONF_PARTY_PRIVATE_FIRST_OPCODE			1990000
//#define CONF_PARTY_PRIVATE_LAST_OPCODE			1999999

#define CONF_PARTY_LAST_OPCODE_IN_RANGE				2000000


/*=====================================================
 PROCESS MPLAPI RANGE           =  2000001 -> 3000000
=======================================================*/
#define MPLAPI_FIRST_OPCODE_IN_RANGE				2000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
#define MPLAPI_COMMON_FIRST_OPCODE					2010000
#define MPLAPI_COMMON_LAST_OPCODE					2019999

// ===== with ShelfMngr (file MCMS/McmIncld/MPL/ShelfMngr/OpcodesMcmsShelfMngr.h)
#define MPLAPI_SHELFMNGR_FIRST_OPCODE				2020000

#define MPLAPI_SHELFMNGR_REQ_FIRST_OPCODE			2020001
#define MPLAPI_SHELFMNGR_REQ_LAST_OPCODE			2024999

#define MPLAPI_SHELFMNGR_IND_FIRST_OPCODE			2025000
#define MPLAPI_SHELFMNGR_IND_LAST_OPCODE			2029998

#define MPLAPI_SHELFMNGR_LAST_OPCODE				2029999



// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define MPLAPI_INTERNAL_MCMS_FIRST_OPCODE			2980000
//#define MPLAPI_INTERNAL_MCMS_LAST_OPCODE			2989999

// ===== internal MPLAPI opcodes
//#define MPLAPI_PRIVATE_FIRST_OPCODE				2990000
//#define MPLAPI_PRIVATE_LAST_OPCODE				2999999

#define MPLAPI_LAST_OPCODE_IN_RANGE					3000000


/*=====================================================
  PROCESS LOGGER RANGE           =  3000001 -> 4000000
=======================================================*/
#define LOGGER_FIRST_OPCODE_IN_RANGE				3000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
#define LOGGER_COMMON_FIRST_OPCODE					3010000
#define LOGGER_COMMON_LAST_OPCODE					3019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define LOGGER_INTERNAL_MCMS_FIRST_OPCODE			3980000
#define LOGGER_INTERNAL_MCMS_LAST_OPCODE			3989999

// ===== internal LOGGER opcodes
//#define LOGGER_PRIVATE_FIRST_OPCODE				3990000
//#define LOGGER_PRIVATE_LAST_OPCODE				3999999

#define LOGGER_LAST_OPCODE_IN_RANGE					4000000


/*=====================================================
  PROCESS MCUMNGR RANGE       =  4000001 -> 5000000
=======================================================*/
#define MCUMNGR_FIRST_OPCODE_IN_RANGE					4000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
#define MCUMNGR_COMMON_FIRST_OPCODE					4010000
#define MCUMNGR_COMMON_LAST_OPCODE					4019999

// ===== with ShelfMngr (file MCMS/McmIncld/MPL/ShelfMngr/OpcodesMcmsShelfMngr.h)
#define MCUMNGR_SHELFMNGR_FIRST_OPCODE					4020000

#define MCUMNGR_SHELFMNGR_REQ_FIRST_OPCODE				4020001
#define MCUMNGR_SHELFMNGR_REQ_LAST_OPCODE				4024999

#define MCUMNGR_SHELFMNGR_IND_FIRST_OPCODE				4025000
#define MCUMNGR_SHELFMNGR_IND_LAST_OPCODE				4029998

#define MCUMNGR_SHELFMNGR_LAST_OPCODE					4029999



// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define MCUMNGR_INTERNAL_MCMS_FIRST_OPCODE				4980000
#define MCUMNGR_INTERNAL_MCMS_LAST_OPCODE				4989999

// ===== internal MCUMNGR opcodes
//#define MCUMNGR_PRIVATE_FIRST_OPCODE					4990000
//#define MCUMNGR_PRIVATE_LAST_OPCODE					4999999

#define MCUMNGR_LAST_OPCODE_IN_RANGE					5000000


/*=====================================================
  PROCESS CARDS RANGE       =  5000001 -> 6000000
=======================================================*/
#define CARDS_FIRST_OPCODE_IN_RANGE						5000001

// range for indications from MPL to Manager task of Cards process (a unique case for Cards process)
//  (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
#define  CARDS_MNGR_FIRST_OPCODE_IN_RANGE				5001000
#define  CARDS_MNGR_LAST_OPCODE_IN_RANGE				5001999


// ranges for indications from MPL to Dispatcher task of Cards process (a unique case for Cards process)
#define CARDS_DISPATCHER_FIRST_OPCODE_IN_RANGE			5010000

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
#define CARDS_COMMON_FIRST_OPCODE						5010001
#define CARDS_COMMON_LAST_OPCODE						5019999

// ===== with CardMngrIvrCntrl (file MCMS/McmIncld/MPL/Card/CardMngrIvrCntl/OpcodesMcmsCardMngrIvrCntl.h)
#define CARDS_CARDMNGR_IVR_CNTL_FIRST_OPCODE			5020000
#define CARDS_CARDMNGR_IVR_CNTL_LAST_OPCODE				5029999

// ===== with CardMngrMaintenance (file MCMS/McmIncld/MPL/Card/CardMngrMaintenance/OpcodesMcmsCardMngrMaintenance.h)
#define CARDS_CARDMNGR_MNTNCE_FIRST_OPCODE				5030000

#define CARDS_CARDMNGR_MNTNCE_REQ_FIRST_OPCODE			5030001
#define CARDS_CARDMNGR_MNTNCE_REQ_LAST_OPCODE			5034999

#define CARDS_CARDMNGR_MNTNCE_IND_FIRST_OPCODE			5035000
#define CARDS_CARDMNGR_MNTNCE_IND_LAST_OPCODE			5039998

#define CARDS_CARDMNGR_MNTNCE_LAST_OPCODE				5039999

// ===== with ShelfMngr (file MCMS/McmIncld/MPL/ShelfMngr/OpcodesMcmsShelfMngr.h)
#define CARDS_SHELFMNGR_FIRST_OPCODE					5040000

#define CARDS_SHELFMNGR_REQ_FIRST_OPCODE				5040001
#define CARDS_SHELFMNGR_REQ_LAST_OPCODE					5044999

#define CARDS_SHELFMNGR_IND_FIRST_OPCODE				5045000
#define CARDS_SHELFMNGR_IND_LAST_OPCODE					5049998

#define CARDS_SHELFMNGR_LAST_OPCODE						5049998


// ===== with RTM (Isdn) (file MCMS/McmIncld/MPL/Card/RtmIsdnMaintenance/OpcodesMcmsRtmIsdnMaintenance.h)
#define CARDS_RTM_ISDN_FIRST_OPCODE						5050000

#define CARDS_RTM_ISDN_REQ_FIRST_OPCODE					5050001
#define CARDS_RTM_ISDN_REQ_LAST_OPCODE					5054999

#define CARDS_RTM_ISDN_IND_FIRST_OPCODE					5055000
#define CARDS_RTM_ISDN_IND_LAST_OPCODE					5059998

#define CARDS_RTM_ISDN_LAST_OPCODE						5059998

// ===== with CM (file IncludeExternal/MPL/Card/CardMngrICE/OpcodesMcmsCardMngrICE.h)
#define CARDS_CARDMNGR_ICE_FIRST_OPCODE					5060000
#define CARDS_CARDMNGR_ICE_LAST_OPCODE					5060098

#define CARDS_DISPATCHER_LAST_OPCODE_IN_RANGE			5069999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define CARDS_INTERNAL_MCMS_FIRST_OPCODE				5980000
#define CARDS_INTERNAL_MCMS_LAST_OPCODE					5989999

// ===== internal CARDS opcodes
//#define CARDS_PRIVATE_FIRST_OPCODE					5990000
//#define CARDS_PRIVATE_LAST_OPCODE						5999999

#define CARDS_LAST_OPCODE_IN_RANGE						6000000


/*=====================================================
  PROCESS QAAPI RANGE      =  6000001 -> 7000000
=======================================================*/
#define QAAPI_FIRST_OPCODE_IN_RANGE						6000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define QAAPI_COMMON_FIRST_OPCODE						6010000
//#define QAAPI_COMMON_LAST_OPCODE						6019999

// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define QAAPI_INTERNAL_MCMS_FIRST_OPCODE				6980000
#define QAAPI_INTERNAL_MCMS_LAST_OPCODE					6989999

// ===== internal QAAPI opcodes
//#define QAAPI_PRIVATE_FIRST_OPCODE					6990000
//#define QAAPI_PRIVATE_LAST_OPCODE						6999999

#define QAAPI_LAST_OPCODE_IN_RANGE						7000000


/*=====================================================
  PROCESS CS_API RANGE       =  7000001 -> 8000000
=======================================================*/
#define CS_API_FIRST_OPCODE_IN_RANGE					7000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define CS_API_COMMON_FIRST_OPCODE					7010000
//#define CS_API_COMMON_LAST_OPCODE						7019999

// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define CS_API_INTERNAL_MCMS_FIRST_OPCODE				7980000
#define CS_API_INTERNAL_MCMS_LAST_OPCODE				7989999

// ===== internal CS_API opcodes
//#define CS_API_PRIVATE_FIRST_OPCODE					7990000
//#define CS_API_PRIVATE_LAST_OPCODE					7990000

#define CS_API_LAST_OPCODE_IN_RANGE						8000000



/*=====================================================
PROCESS EncryptionKeyServer RANGE = 8000001 -> 9000000//
=====================================================*/
#define ENCRYPTION_FIRST_OPCODE_IN_RANGE				8000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define ENCRYPTION_COMMON_FIRST_OPCODE				8010000
//#define ENCRYPTION_COMMON_LAST_OPCODE					8019999

// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define ENCRYPTION_INTERNAL_MCMS_FIRST_OPCODE			8980000
#define ENCRYPTION_INTERNAL_MCMS_LAST_OPCODE			8989999

// ===== internal ENCRYPTION opcodes
//#define ENCRYPTION_PRIVATE_FIRST_OPCODE				8990000
//#define ENCRYPTION_PRIVATE_LAST_OPCODE				8999999

#define ENCRYPTION_LAST_OPCODE_IN_RANGE					9000000



/*=====================================================
  PROCESS RESOURCE RANGE       =  9000001 -> 10000000
=======================================================*/
#define RESOURCE_FIRST_OPCODE_IN_RANGE					9000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define RESOURCE_COMMON_FIRST_OPCODE					9010000
//#define RESOURCE_COMMON_LAST_OPCODE					9019999

// ===== with CardMngrTopologyBuilder (file MCMS/McmIncld/MPL/Card/CardMngrTB/OpcodesMcmsCardMngrTB.h)
#define RESOURCE_CARDMNGR_TB_FIRST_OPCODE				9020000
#define RESOURCE_CARDMNGR_TB_LAST_OPCODE				9029999

// ===== with CardMngrIpMedia (file MCMS/McmIncld/MPL/Card/CardMngrIpMedia/OpcodesMcmsCardMngrIpMedia.h)
#define RESOURCE_CARDMNGR_IP_MEDIA_FIRST_OPCODE			9030000
#define RESOURCE_CARDMNGR_IP_MEDIA_LAST_OPCODE			9039999

// ===== with CardMngrRecording (file MCMS/McmIncld/MPL/Card/CardMngrIpMedia/OpcodesMcmsCardMngrRecording.h)
#define RESOURCE_CARDMNGR_RECORDING_FIRST_OPCODE		9040000
#define RESOURCE_CARDMNGR_RECORDING_LAST_OPCODE			9049999

// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define RESOURCE_INTERNAL_MCMS_FIRST_OPCODE				9980000
#define RESOURCE_INTERNAL_MCMS_LAST_OPCODE				9989999

// ===== internal RESOURCE opcodes
//#define RESOURCE_PRIVATE_FIRST_OPCODE					9990000
//#define RESOURCE_PRIVATE_LAST_OPCODE					9999999

#define RESOURCE_LAST_OPCODE_IN_RANGE					10000000


/*=====================================================
  PROCESS SIPPROXY RANGE      =  10000001 -> 11000000
=======================================================*/
#define SIPPROXY_FIRST_OPCODE_IN_RANGE					10000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define SIPPROXY_COMMON_FIRST_OPCODE					10010000
//#define SIPPROXY_COMMON_LAST_OPCODE					10019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define SIPPROXY_INTERNAL_MCMS_FIRST_OPCODE				10980000
#define SIPPROXY_INTERNAL_MCMS_LAST_OPCODE				10989999

// ===== internal SIPPROXY opcodes
//#define SIPPROXY_PRIVATE_FIRST_OPCODE					10990000
//#define SIPPROXY_PRIVATE_LAST_OPCODE					10999999

#define SIPPROXY_LAST_OPCODE_IN_RANGE					11000000




/*=====================================================
  PROCESS CONFIGURATOR RANGE      =  11000001 -> 12000000
=======================================================*/
#define CONFIGURATOR_FIRST_OPCODE_IN_RANGE				11000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define CONFIGURATOR_COMMON_FIRST_OPCODE				11010000
//#define CONFIGURATOR_COMMON_LAST_OPCODE				11019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define CONFIGURATOR_INTERNAL_MCMS_FIRST_OPCODE		11980000
//#define CONFIGURATOR_INTERNAL_MCMS_LAST_OPCODE		11989999

// ===== internal CONFIGURATOR opcodes
//#define CONFIGURATOR_PRIVATE_FIRST_OPCODE				11990000
//#define CONFIGURATOR_PRIVATE_LAST_OPCODE				11999999

#define CONFIGURATOR_LAST_OPCODE_IN_RANGE				12000000


/*=====================================================
//GATE_KEEPER_MNGR RANGE       =  12000001 -> 13000000
=======================================================*/
// See all in file MCMS/McmIncld/CS/CsSignaling/IpMngrOpcodes.h)
//#define GATE_KEEPER_MNGR_FIRST_OPCODE_IN_RANGE		12000001
//#define GATE_KEEPER_MNGR_LAST_OPCODE_IN_RANGE			13000000
//#define PARTY_CS_FIRST_OPCODE_IN_RANGE				13000001
//#define PROXY_CS_LAST_OPCODE_IN_RANGE					14500200

/*=====================================================
  PROCESS AUTHENTICATION RANGE  = 15000001 -> 16000000
=======================================================*/
#define AUTHENTICATION_FIRST_OPCODE_IN_RANGE			15000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define AUTHENTICATION_COMMON_FIRST_OPCODE			15010000
//#define AUTHENTICATION_COMMON_LAST_OPCODE				15019999

// ===== with ShelfMngr (file MCMS/McmIncld/MPL/ShelfMngr/OpcodesMcmsShelfMngr.h)
#define AUTHENTICATION_SHELFMNGR_FIRST_OPCODE			15020000
#define AUTHENTICATION_SHELFMNGR_LAST_OPCODE			15029999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define AUTHENTICATION_INTERNAL_MCMS_FIRST_OPCODE		15980000
//#define AUTHENTICATION_INTERNAL_MCMS_LAST_OPCODE		15989999

// ===== internal AUTHENTICATION opcodes
#define AUTHENTICATION_PRIVATE_FIRST_OPCODE				15990000
#define AUTHENTICATION_PRIVATE_LAST_OPCODE				15999999

#define AUTHENTICATION_LAST_OPCODE_IN_RANGE				16000000


/*=====================================================
  CSMngr RANGE				=  16000001 -> 17000000
=======================================================*/
#define CS_MNGR_FIRST_OPCODE_IN_RANGE					16000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define CS_MNGR_COMMON_FIRST_OPCODE					16010000
//#define CS_MNGR_COMMON_LAST_OPCODE					16019999


// ranges for indications from CS to Dispatcher task of CSMngr process (a unique case for CSMngr process)
// ===== with CS (file MCMS/McmIncld/CS/CsSignsling/IpCsOpcodes.h)
#define CS_MNGR_DISPATCHER_FIRST_OPCODE_IN_RANGE		16100001

#define SIGNALING_TASK_NOT_CREATED_REQ					16100061

#define CS_MNGR_DISPATCHER_LAST_OPCODE_IN_RANGE			16200000

// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define CS_MNGR_INTERNAL_MCMS_FIRST_OPCODE				16980000
#define CS_MNGR_INTERNAL_MCMS_LAST_OPCODE				16989999

// ===== internal CS_MNGR opcodes
//#define CS_MNGR_PRIVATE_FIRST_OPCODE					16990000
//#define CS_MNGR_PRIVATE_LAST_OPCODE					16999999

#define CS_MNGR_LAST_OPCODE_IN_RANGE					17000000


/*=====================================================
  PROCESS CDR RANGE      =  17000001 -> 18000000
=======================================================*/
#define CDR_FIRST_OPCODE_IN_RANGE						17000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define CDR_COMMON_FIRST_OPCODE						17010000
//#define CDR_COMMON_LAST_OPCODE						17019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define CDR_INTERNAL_MCMS_FIRST_OPCODE					17980000
#define CDR_INTERNAL_MCMS_LAST_OPCODE					17989999

// ===== internal CDR opcodes
//#define CDR_PRIVATE_FIRST_OPCODE						17990000
//#define CDR_PRIVATE_LAST_OPCODE						17999999

#define CDR_LAST_OPCODE_IN_RANGE						18000000


/*=====================================================
  PROCESS DNSAgent RANGE     =  18000001 -> 19000000
=======================================================*/
#define DNSAGENT_FIRST_OPCODE_IN_RANGE					18000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define DNSAGENT_COMMON_FIRST_OPCODE					18010000
//#define DNSAGENT_COMMON_LAST_OPCODE					18019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define DNSAGENT_INTERNAL_MCMS_FIRST_OPCODE				18980000
#define DNSAGENT_INTERNAL_MCMS_LAST_OPCODE				18989999

// ===== internal DNSAGENT opcodes
//#define DNSAGENT_PRIVATE_FIRST_OPCODE					18990000
//#define DNSAGENT_PRIVATE_LAST_OPCODE					18999999

#define DNSAGENT_LAST_OPCODE_IN_RANGE					19000000


/*=====================================================
  PROCESS APACHE_MODULE RANGE     =  19000001 -> 20000000
=======================================================*/
#define APACHE_MODULE_FIRST_OPCODE_IN_RANGE				19000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define APACHE_MODULE_COMMON_FIRST_OPCODE				19010000
//#define APACHE_MODULE_COMMON_LAST_OPCODE				19019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define APACHE_MODULE_INTERNAL_MCMS_FIRST_OPCODE		19980000
//#define APACHE_MODULE_INTERNAL_MCMS_LAST_OPCODE		19989999

// ===== internal APACHE_MODULE opcodes
//#define APACHE_MODULE_PRIVATE_FIRST_OPCODE			19990000
//#define APACHE_MODULE_PRIVATE_LAST_OPCODE				19999999

#define APACHE_MODULE_LAST_OPCODE_IN_RANGE				20000000



/*=====================================================
  PROCESS GIDEONSIM RANGE        =  20000001 -> 21000000
=======================================================*/
#define GIDEON_SIM_FIRST_OPCODE_IN_RANGE				20000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define GIDEON_SIM_COMMON_FIRST_OPCODE				20010000
//#define GIDEON_SIM_COMMON_LAST_OPCODE					20019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define GIDEON_SIM_INTERNAL_MCMS_FIRST_OPCODE			20980000
//#define GIDEON_SIM_INTERNAL_MCMS_LAST_OPCODE			20989999

// ===== internal GIDEON_SIM opcodes
//#define GIDEON_SIM_PRIVATE_FIRST_OPCODE				20990000
//#define GIDEON_SIM_PRIVATE_LAST_OPCODE				20999999

#define GIDEON_SIM_LAST_OPCODE_IN_RANGE					21000000


/*=====================================================
  PROCESS ENDPOINTS SIM RANGE    =   21000001 -> 22000000
=======================================================*/
#define ENDPOINTS_SIM_FIRST_OPCODE_IN_RANGE				21000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define ENDPOINTS_SIM_COMMON_FIRST_OPCODE				21010000
//#define ENDPOINTS_SIM_COMMON_LAST_OPCODE				21019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define ENDPOINTS_SIM_INTERNAL_MCMS_FIRST_OPCODE		21980000
//#define ENDPOINTS_SIM_INTERNAL_MCMS_LAST_OPCODE		21989999

// ===== internal ENDPOINTS_SIM opcodes
//#define ENDPOINTSN_SIM_PRIVATE_FIRST_OPCODE			21990000
//#define ENDPOINTS_SIM_PRIVATE_LAST_OPCODE				21999999

#define ENDPOINTS_SIM_LAST_OPCODE_IN_RANGE				22000000


/*=====================================================
  PROCESS DEMO RANGE             =  22000001 -> 23000000
=======================================================*/
#define DEMO_FIRST_OPCODE_IN_RANGE						22000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define DEMO_COMMON_FIRST_OPCODE						22010000
//#define DEMO_COMMON_LAST_OPCODE						22019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define DEMO_INTERNAL_MCMS_FIRST_OPCODE				22980000
//#define DEMO_INTERNAL_MCMS_LAST_OPCODE				22989999

// ===== internal DEMO opcodes
//#define DEMO_PRIVATE_FIRST_OPCODE						22990000
//#define DEMO_PRIVATE_LAST_OPCODE						22999999

#define DEMO_LAST_OPCODE_IN_RANGE						23000000


/*=====================================================
  PROCESS TESTCLIENT RANGE       =  23000001 -> 24000000
=======================================================*/
#define TESTCLIENT_FIRST_OPCODE_IN_RANGE				23000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define TESTCLIENT_COMMON_FIRST_OPCODE				23010000
//#define TESTCLIENT_COMMON_LAST_OPCODE					23019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define TESTCLIENT_INTERNAL_MCMS_FIRST_OPCODE			23980000
//#define TESTCLIENT_INTERNAL_MCMS_LAST_OPCODE			23989999

// ===== internal TESTCLIENT opcodes
//#define TESTCLIENT_PRIVATE_FIRST_OPCODE				23990000
//#define TESTCLIENT_PRIVATE_LAST_OPCODE				23999999

#define TESTCLIENT_LAST_OPCODE_IN_RANGE					24000000


/*=====================================================
  PROCESS TESTSERVER RANGE       =  24000001 -> 25000000
=======================================================*/
#define TESTSERVER_FIRST_OPCODE_IN_RANGE				24000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define TESTSERVER_COMMON_FIRST_OPCODE				24010000
//#define TESTSERVER_COMMON_LAST_OPCODE					24019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
//#define TESTSERVER_INTERNAL_MCMS_FIRST_OPCODE			24980000
//#define TESTSERVER_INTERNAL_MCMS_LAST_OPCODE			24989999

// ===== internal TESTSERVER opcodes
//#define TESTSERVER_PRIVATE_FIRST_OPCODE				24990000
//#define TESTSERVER_PRIVATE_LAST_OPCODE				24999999

#define TESTSERVER_LAST_OPCODE_IN_RANGE					25000000


/*=====================================================
  MCMS SKELETON RANGE       =  25000001 -> 26000000
=======================================================*/
#define MCMS_SKELETON_FIRST_OPCODE_IN_RANGE				25000001
#define MCMS_SKELETON_LAST_OPCODE_IN_RANGE				26000000


/*=====================================================
  PROCESS INSTALLER RANGE       =  26000001 -> 27000000
=======================================================*/
#define INSTALLER_FIRST_OPCODE_IN_RANGE					26000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define INSTALLER_COMMON_FIRST_OPCODE					26010000
//#define INSTALLER_COMMON_LAST_OPCODE					26019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define INSTALLER_INTERNAL_MCMS_FIRST_OPCODE			26980000
#define INSTALLER_INTERNAL_MCMS_LAST_OPCODE				26989999

// ===== internal INSTALLER opcodes
//#define INSTALLER_PRIVATE_FIRST_OPCODE				26990000
//#define INSTALLER_PRIVATE_LAST_OPCODE					26999999

#define INSTALLER_LAST_OPCODE_IN_RANGE					27000000


/*=====================================================
  PROCESS MCMS_DAEMON RANGE       =  27000001 -> 28000000
=======================================================*/
#define MCMS_DAEMON_FIRST_OPCODE_IN_RANGE				27000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define MCMS_DAEMON_COMMON_FIRST_OPCODE				27010000
//#define MCMS_DAEMON_COMMON_LAST_OPCODE				27019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define MCMS_DAEMON_INTERNAL_MCMS_FIRST_OPCODE			27980000
#define MCMS_DAEMON_INTERNAL_MCMS_LAST_OPCODE			27989999

// ===== internal MCMS_DAEMON opcodes
//#define MCMS_DAEMON_PRIVATE_FIRST_OPCODE				27990000
//#define MCMS_DAEMON_PRIVATE_LAST_OPCODE				27999999

#define MCMS_DAEMON_LAST_OPCODE_IN_RANGE				28000000


/*=====================================================
  PROCESS SYSTEM_MONITORING RANGE       =  28000001 -> 29000000
=======================================================*/
#define SYSTEM_MONITORING_FIRST_OPCODE_IN_RANGE			28000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define SYSTEM_MONITORING_COMMON_FIRST_OPCODE			28010000
//#define SYSTEM_MONITORING_COMMON_LAST_OPCODE			28019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define SYSTEM_MONITORING_INTERNAL_MCMS_FIRST_OPCODE	28980000
#define SYSTEM_MONITORING_INTERNAL_MCMS_LAST_OPCODE		28989999

// ===== internal MONITORING opcodes
//#define SYSTEM_MONITORING_PRIVATE_FIRST_OPCODE		28990000
//#define SYSTEM_MONITORING_PRIVATE_LAST_OPCODE			28999999

#define SYSTEM_MONITORING_LAST_OPCODE_IN_RANGE			29000000


/*=====================================================
  PROCESS RTM_ISDN_MNGR RANGE       =  29000001 -> 30000000
=======================================================*/
#define RTM_ISDN_MNGR_FIRST_OPCODE_IN_RANGE				29000001

// ===== with several entities (file MCMS/McmIncld/Common/OpcodesMcmsCommon.h)
//#define RTM_ISDN_MNGR_COMMON_FIRST_OPCODE				29010000
//#define RTM_ISDN_MNGR_COMMON_LAST_OPCODE				29019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define RTM_ISDN_MNGR_INTERNAL_MCMS_FIRST_OPCODE		29980000
#define RTM_ISDN_MNGR_INTERNAL_MCMS_LAST_OPCODE			29989999

// ===== internal RTM_ISDN_MNGR opcodes
//#define RTM_ISDN_MNGR_PRIVATE_FIRST_OPCODE			29990000
//#define RTM_ISDN_MNGR_PRIVATE_LAST_OPCODE				29999999

#define RTM_ISDN_MNGR_LAST_OPCODE_IN_RANGE				30000000

/*=====================================================
  MPL INTERNAL_RANGE       =  30000001 -> 31000000
=======================================================*/
#define MPL_INTERNAL_FIRST_OPCODE_IN_RANGE				30000001

#define MPL_INTERNAL_LAST_OPCODE_IN_RANGE				31000000


/*=====================================================
  PROCESS FAILOVER RANGE       =  31000001 -> 32000000
=======================================================*/
#define FAILOVER_FIRST_OPCODE_IN_RANGE					31000001

// ===== with several entities (file IncludeExternal/Common/OpcodesMcmsCommon.h)
//#define FAILOVER_COMMON_FIRST_OPCODE					31010000
//#define FAILOVER_COMMON_LAST_OPCODE					31019999


// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define FAILOVER_INTERNAL_MCMS_FIRST_OPCODE				31080000
#define FAILOVER_INTERNAL_MCMS_LAST_OPCODE				31089999

// ===== internal FAILOVER opcodes
//#define FAILOVER_PRIVATE_FIRST_OPCODE					31990000
//#define FAILOVER_PRIVATE_LAST_OPCODE					31999999

#define FAILOVER_LAST_OPCODE_IN_RANGE					32000000



/*=====================================================
  PROCESS IPMC_INTERFACE RANGE       =  32000001 -> 33000000
=======================================================*/
#define IPMC_INTERFACE_FIRST_OPCODE_IN_RANGE				32000001

#define CHECK_IPMC_VERSION_TOUT							32000002
#define CHANGE_LED_STATE_TIMER							32000003

#define IPMC_INTERFACE_LAST_OPCODE_IN_RANGE				33000000


/*=====================================================
  PROCESS UTILITY RANGE       =  33000001 -> 34000000
=======================================================*/
#define UTILITY_FIRST_OPCODE_IN_RANGE				33000001

// ===== with other Mcms processes (file IncludeInternalMcms/OpcodesMcmsInternal.h)
#define UTILITY_FIRST_OPCODE			33980000
#define UTILITY_LAST_OPCODE				33989999


#define UTILITY_LAST_OPCODE_IN_RANGE				34000000


#define INVALID_OPCODE									0xfffffff




#endif /*OPCODESRANGES_H_*/
