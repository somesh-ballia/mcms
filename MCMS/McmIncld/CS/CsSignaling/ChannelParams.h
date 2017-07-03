
// ChannelParams.h
// Avishay Halavy

#ifndef __CHANNELPARAMETERS_H__
#define __CHANNELPARAMETERS_H__

//#include "NonStandard.h"
#include "Capabilities.h"
#include "CsHeader.h"
#include "IpAddressDefinitions.h"

typedef union {
	g7231CapStruct				p7231;
	audioCapStructBase			pAudioBase;
	sirenLPR_CapStruct			pSirenLPR;
	h261CapStruct				p261;
	h263CapStruct				p263;
	h264CapStruct				p264;
	genericVideoCapStruct		pGenericVideo;
	dataCapStructBase			pData;
	ctNonStandardCapStruct		pNonStandard;
	PeopleAndContentCapStruct	pPeopleContent;
	rtvCapStruct				pRtv;
	svcCapStruct				pSvc;
	sirenLPR_Scalable_CapStruct pSirenLPR_Scalable;
	msSvcCapStruct				pMsSvc;
} channelSpecificParameters;

typedef enum {
	NoInitiator = 0,
	McInitiator,
	CtInitiator,
	PmInitiator,
	GkInitiator

} initiatorOfClose;

typedef struct
{
 
    xmlUnionPropertiesSt  unionProps;
    mcTransportAddress	  transAddr;
 } mcXmlTransportAddress;


// PayLoad
typedef enum
{	
    _PCMU			=  0,
    _G7231			=  4,
    _PCMA			=  8,
    _G722			=  9, 
    _G728			= 15,
    _G729			= 18,
    _H261			= 31,
    _H263			= 34,
					
	__FIRST_DPT		= 96,	// first valid dynamic payload type

	__LAST_DPT		= 127,	// last valid dynamic payload type
					
	_T120			= 128,
    _ANY			= 130,	// Private (or unknown) payload
					
	_NONS			= 131,
					
	_SVC            = 199,

	_G7221			= 200,
	_Siren14		= 201,
	_H264			= 202,
	_H26L			= 203,
	_Rfc2833Dtmf	= 204,
	_AnnexQ			= 205,
	_RvFecc			= 206,
	_Siren14S		= 207,
	_G719			= 208,
	_Siren22		= 209,
	_G719S			= 210,
	_Siren22S		= 211,
    _SirenLPR       = 212,

    _SirenLPR_Scalable = 213,
	
    _iLBC = 214,

    // ICE
	_IcePwd			= 215,
	_IceUfrag		= 216,
	_IceCand		= 217,
	_IceRemoteCand	= 218,
	_Rtcp			= 219,

	//SDES and SRTP
	_Sdes			= 220,

	//RTV
	_Rtv			= 221,

	// TIP
	_AAC_LD			= 230,


	// Siren7
	_Siren7         = 231,
	
	
	//WebRTC
	_VP8			= 240,
	_Opus			= 241,

	//for mcms internal use:
	_H323_P_C       = 341,
	_H239           = 342,
	_BFCP			= 343,
	_DynamicPTRep	= 344, /* Dynamic Payload type replacement */

	_MCCF			= 345,
	
	_MS_SVC			= 346,

	_RED 			= 347,   //LYNC2013_FEC_RED
	_FEC			= 348,  //LYNC2013_FEC_RED
	//end of mcms

	_UnKnown		= 0xFF,

} payload_en;


#endif // __CHANNELPARAMETERS_H__
