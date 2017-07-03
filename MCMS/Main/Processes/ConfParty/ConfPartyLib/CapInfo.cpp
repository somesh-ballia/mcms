//+========================================================================+
//                            CAPINFO.CPP                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CAPINFO.CPP                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |09/07/02     |                                                     |
//+========================================================================+
#include <ostream>
#include "CapInfo.h"
#include "H221.h"
#include "H263.h"
#include "Trace.h"
#include "Macros.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "ObjString.h"
#include "ConfPartyGlobals.h"
#include "H264.h"
#include "EnumsToStrings.h"

// Global variables & functions

extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);
BYTE IsMatchingCapName(const char * str1,const char * str2);
// End of globals

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Initialize static arrays
/*
typedef enum{ 								//Capabilities.h
	// audio dynamic payload numbers
	eStaticPayloadCodec_DynamicPayload1 		= 96,// (H323-encrypted, SIP - AAC-LD)
	eSiren14_24kDynamicPayload 		= 97,
	eREDDynamicPayload              = 97,  //LYNC2013_FEC_RED
	eSiren14_32kDynamicPayload 		= 98,
	eSiren14_48kDynamicPayload 		= 99,
	eStaticPayloadCodec_DynamicPayload2	= 100,
	eRfc2833DtmfDynamicPayload 		= 101, // Must be 101 because Microsoft use 101 for DTMF (Microsoft don't obey our Payload type number).
	eG7221_16kDynamicPayload 		= 102,
	eG7221_24kDynamicPayload 		= 103,
	eG7221_32kDynamicPayload 		= 104,
	eSiren22_32kDynamicPayload 		= 105,
	eSiren22_48kDynamicPayload 		= 106,
	eSiren22_64kDynamicPayload 		= 107,
	eG719_32kDynamicPayload 		= 108,
	eG719_48kDynamicPayload 		= 109,
	eG719_64kDynamicPayload 		= 110,
	eIlbc800020DynamicPayload		= 111,
	eIlbc800030DynamicPayload		= 112,
*//*	eG729AnnexADynamicPayload 		= 110,
	eG729AnnexBDynamicPayload 		= 111,
	eG729AnnexABDynamicPayload 		= 112,*//*
	eG7221C_24kDynamicPayload 		= 113,
	eG7221C_32kDynamicPayload 		= 114,
	eG7221C_48kDynamicPayload 		= 115,
	eG719Stereo_64kDynamicPayload 	= 116,
	eG719Stereo_96kDynamicPayload	= 117,
	eG719Stereo_128kDynamicPayload	= 118,
	eSiren14Stereo_48kDynamicPayload= 119,
	eSiren14Stereo_56kDynamicPayload= 120,
	eSiren14Stereo_64kDynamicPayload= 121,
	eSiren14Stereo_96kDynamicPayload= 122,
	eSiren22Stereo_64kDynamicPayload = 123,
	eSiren22Stereo_96kDynamicPayload = 124,
	eSiren22Stereo_128kDynamicPayload= 125,
	eSirenLPRStereo_128kDynamicPayload= 126,
    eSirenLPR_Scalable_32kDynamicPayload= 127,
    eSirenLPR_Scalable_48kDynamicPayload= 127,
    eSirenLPR_Scalable_64kDynamicPayload= 127,
    eSirenLPRStereo_Scalable_64kDynamicPayload= 127,
    eSirenLPRStereo_Scalable_96kDynamicPayload= 127,
    eSirenLPRStereo_Scalable_128kDynamicPayload= 127,
    eiLBC_13kDynamicPayload = 127,
    eiLBC_15kDynamicPayload = 127,
    eOpus_64kDynamicPayload = 127,
    eOpusStereo_128kDynamicPayload = 127,

}DynamicAudioPayloadEnum;// values of dynamic payload type can't be smaller than 96 and bigger than 127!!
*/

TCapQuality CCapSetInfo::g_capQualityTbl[MAX_CAP_TYPES] =
{
	{cmCapAudio,       TRUE,  eG711Alaw64kCapCode,                  _PCMA,              sizeof(g711Alaw64kCapStruct),        64*_K_,  NonFrameBasedFPP, 10, eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       FALSE, eG711Alaw56kCapCode,                  _PCMA,              sizeof(g711Alaw56kCapStruct),        56*_K_,  NonFrameBasedFPP, NA, _UnKnown},
	{cmCapAudio,       TRUE,  eG711Ulaw64kCapCode,                  _PCMU,              sizeof(g711Ulaw64kCapStruct),        64*_K_,  NonFrameBasedFPP, 10, eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       FALSE, eG711Ulaw56kCapCode,                  _PCMU,              sizeof(g711Ulaw56kCapStruct),        56*_K_,  NonFrameBasedFPP, NA, _UnKnown},
	{cmCapAudio,       TRUE,  eG722_64kCapCode,                     _G722,              sizeof(g722_64kCapStruct),           64*_K_,  NonFrameBasedFPP, 10, eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       FALSE, eG722_56kCapCode,                     _G722,              sizeof(g722_56kCapStruct),           56*_K_,  NonFrameBasedFPP, NA, _UnKnown},
	{cmCapAudio,       FALSE, eG722_48kCapCode,                     _G722,              sizeof(g722_48kCapStruct),           48*_K_,  NonFrameBasedFPP, NA, _UnKnown},
	{cmCapAudio,       TRUE,  eG722Stereo_128kCapCode,              _G722,              sizeof(g722Stereo_128kCapStruct),    128*_K_, FrameBasedFPP,    1,  eG722Stereo_128kDynamicPayload},
	{cmCapAudio,       FALSE, eG728CapCode,                         _G728,              sizeof(g728CapStruct),               16*_K_,  8,                4,  eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       TRUE,  eG729CapCode,                         _G729,              sizeof(g729CapStruct),               8*_K_,   FrameBasedFPP,    NA, eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       TRUE,  eG729AnnexACapCode,                   _G729,              sizeof(g729AnnexACapStruct),         8*_K_,   2*FrameBasedFPP,  1,  eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       FALSE, eG729wAnnexBCapCode,                  _G729,              sizeof(g729wAnnexBCapStruct),        8*_K_,   FrameBasedFPP,    NA, eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       FALSE, eG729AnnexAwAnnexBCapCode,            _G729,              sizeof(g729AnnexAwAnnexBCapStruct),  8*_K_,   FrameBasedFPP,    NA, eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       FALSE, eG7231CapCode,                        _G7231,             sizeof(g7231CapStruct),              7*_K_,   FrameBasedFPP,    1,  eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       FALSE, eIS11172AudioCapCode,                 _ANY,               sizeof(IS11172AudioCapStruct),       NA,      NonFrameBasedFPP, NA, _UnKnown},
	{cmCapAudio,       FALSE, eIS13818CapCode,                      _ANY,               sizeof(IS13818CapStruct),            NA,      NonFrameBasedFPP, NA, _UnKnown},
	{cmCapAudio,       FALSE, eG7231AnnexCapCode,                   _G7231,             sizeof(G7231AnnexCapStruct),         7*_K_,   FrameBasedFPP,    NA, eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       TRUE,  eG7221_32kCapCode,                    _G7221,             sizeof(g7221_32kCapStruct),          32*_K_,  FrameBasedFPP,    1,  eG7221_32kDynamicPayload},
	{cmCapAudio,       TRUE,  eG7221_24kCapCode,                    _G7221,             sizeof(g7221_24kCapStruct),          24*_K_,  FrameBasedFPP,    1,  eG7221_24kDynamicPayload},
	{cmCapAudio,       TRUE,  eG7221_16kCapCode,                    _G7221,             sizeof(g7221_16kCapStruct),          16*_K_,  FrameBasedFPP,    1,  eG7221_16kDynamicPayload},
	{cmCapAudio,       TRUE,  eSiren14_48kCapCode,                  _Siren14,           sizeof(siren14_48kCapStruct),        48*_K_,  FrameBasedFPP,    1,  eSiren14_48kDynamicPayload},
	{cmCapAudio,       TRUE,  eSiren14_32kCapCode,                  _Siren14,           sizeof(siren14_32kCapStruct),        32*_K_,  FrameBasedFPP,    1,  eSiren14_32kDynamicPayload},
	{cmCapAudio,       TRUE,  eSiren14_24kCapCode,                  _Siren14,           sizeof(siren14_24kCapStruct),        24*_K_,  FrameBasedFPP,    1,  eSiren14_24kDynamicPayload},
	{cmCapAudio,       TRUE,  eG7221C_48kCapCode,                   _G7221,             sizeof(g7221C_48kCapStruct),         48*_K_,  FrameBasedFPP,    1,  eG7221C_48kDynamicPayload},
	{cmCapAudio,       TRUE,  eG7221C_32kCapCode,                   _G7221,             sizeof(g7221C_32kCapStruct),         32*_K_,  FrameBasedFPP,    1,  eG7221C_32kDynamicPayload},
	{cmCapAudio,       TRUE,  eG7221C_24kCapCode,                   _G7221,             sizeof(g7221C_24kCapStruct),         24*_K_,  FrameBasedFPP,    1,  eG7221C_24kDynamicPayload},
	{cmCapAudio,       TRUE,  eG7221C_CapCode,                      _G7221,             sizeof(g7221C_CapStruct),            NA,      FrameBasedFPP,    1,  _UnKnown},
	{cmCapAudio,       FALSE, eSiren14Stereo_48kCapCode,            _Siren14S,          sizeof(siren14Stereo_48kCapStruct),  48*_K_,  FrameBasedFPP,    1,  eSiren14Stereo_48kDynamicPayload},
	{cmCapAudio,       FALSE, eSiren14Stereo_56kCapCode,            _Siren14S,          sizeof(siren14Stereo_56kCapStruct),  56*_K_,  FrameBasedFPP,    1,  eSiren14Stereo_56kDynamicPayload},
	{cmCapAudio,       FALSE, eSiren14Stereo_64kCapCode,            _Siren14S,          sizeof(siren14Stereo_64kCapStruct),  64*_K_,  FrameBasedFPP,    1,  eSiren14Stereo_64kDynamicPayload},
	{cmCapAudio,       FALSE, eSiren14Stereo_96kCapCode,            _Siren14S,          sizeof(siren14Stereo_96kCapStruct),  96*_K_,  FrameBasedFPP,    1,  eSiren14Stereo_96kDynamicPayload},
	{cmCapAudio,       TRUE,  eG719_32kCapCode,                     _G719,              sizeof(g719_32kCapStruct),           32*_K_,  FrameBasedFPP,    1,  eG719_32kDynamicPayload},
	{cmCapAudio,       TRUE,  eG719_48kCapCode,                     _G719,              sizeof(g719_48kCapStruct),           48*_K_,  FrameBasedFPP,    1,  eG719_48kDynamicPayload},
	{cmCapAudio,       TRUE,  eG719_64kCapCode,                     _G719,              sizeof(g719_64kCapStruct),           64*_K_,  FrameBasedFPP,    1,  eG719_64kDynamicPayload},
	{cmCapAudio,       FALSE, eG719_96kCapCode,                     _G719,              sizeof(g719_96kCapStruct),           64*_K_,  FrameBasedFPP,    1,  _UnKnown},
	{cmCapAudio,       FALSE, eG719_128kCapCode,                    _G719,              sizeof(g719_128kCapStruct),          64*_K_,  FrameBasedFPP,    1,  _UnKnown},
	{cmCapAudio,       TRUE,  eSiren22_32kCapCode,                  _Siren22,           sizeof(siren22_32kCapStruct),        32*_K_,  FrameBasedFPP,    1,  eSiren22_32kDynamicPayload},
	{cmCapAudio,       TRUE,  eSiren22_48kCapCode,                  _Siren22,           sizeof(siren22_48kCapStruct),        48*_K_,  FrameBasedFPP,    1,  eSiren22_48kDynamicPayload},
	{cmCapAudio,       TRUE,  eSiren22_64kCapCode,                  _Siren22,           sizeof(siren22_64kCapStruct),        64*_K_,  FrameBasedFPP,    1,  eSiren22_64kDynamicPayload},
	{cmCapAudio,       TRUE,  eG719Stereo_64kCapCode,               _G719S,             sizeof(g719Stereo_64kCapStruct),     64*_K_,  FrameBasedFPP,    1,  eG719Stereo_64kDynamicPayload},
	{cmCapAudio,       TRUE,  eG719Stereo_96kCapCode,               _G719S,             sizeof(g719Stereo_96kCapStruct),     96*_K_,  FrameBasedFPP,    1,  eG719Stereo_96kDynamicPayload},
	{cmCapAudio,       TRUE,  eG719Stereo_128kCapCode,              _G719S,             sizeof(g719Stereo_128kCapStruct),    128*_K_, FrameBasedFPP,    1,  eG719Stereo_128kDynamicPayload},
	{cmCapAudio,       TRUE,  eSiren22Stereo_64kCapCode,            _Siren22S,          sizeof(siren22Stereo_64kCapStruct),  64*_K_,  FrameBasedFPP,    1,  eSiren22Stereo_64kDynamicPayload},
	{cmCapAudio,       TRUE,  eSiren22Stereo_96kCapCode,            _Siren22S,          sizeof(siren22Stereo_96kCapStruct),  96*_K_,  FrameBasedFPP,    1,  eSiren22Stereo_96kDynamicPayload},
	{cmCapAudio,       TRUE,  eSiren22Stereo_128kCapCode,           _Siren22S,          sizeof(siren22Stereo_128kCapStruct), 128*_K_, FrameBasedFPP,    1,  eSiren22Stereo_128kDynamicPayload},
	{cmCapAudio,       FALSE, eSirenLPR_32kCapCode,                 _SirenLPR,          sizeof(sirenLPR_CapStruct),          32*_K_,  FrameBasedFPP,    1,  eSirenLPRStereo_128kDynamicPayload},
	{cmCapAudio,       FALSE, eSirenLPR_48kCapCode,                 _SirenLPR,          sizeof(sirenLPR_CapStruct),          48*_K_,  FrameBasedFPP,    1,  eSirenLPRStereo_128kDynamicPayload},
	{cmCapAudio,       FALSE, eSirenLPR_64kCapCode,                 _SirenLPR,          sizeof(sirenLPR_CapStruct),          64*_K_,  FrameBasedFPP,    1,  eSirenLPRStereo_128kDynamicPayload},
	{cmCapAudio,       FALSE, eSirenLPRStereo_64kCapCode,           _SirenLPR,          sizeof(sirenLPR_CapStruct),          64*_K_,  FrameBasedFPP,    1,  eSirenLPRStereo_128kDynamicPayload},
	{cmCapAudio,       FALSE, eSirenLPRStereo_96kCapCode,           _SirenLPR,          sizeof(sirenLPR_CapStruct),          96*_K_,  FrameBasedFPP,    1,  eSirenLPRStereo_128kDynamicPayload},
	{cmCapAudio,       FALSE, eSirenLPRStereo_128kCapCode,          _SirenLPR,          sizeof(sirenLPR_CapStruct),          128*_K_, FrameBasedFPP,    1,  eSirenLPRStereo_128kDynamicPayload},
	{cmCapAudio,       TRUE,  eAAC_LDCapCode,                       _AAC_LD,            sizeof(AAC_LDCapStruct),             256*_K_, FrameBasedFPP,    1,  eStaticPayloadCodec_DynamicPayload1},
	{cmCapAudio,       TRUE,  eSirenLPR_Scalable_32kCapCode,        _SirenLPR_Scalable, sizeof(sirenLPR_Scalable_CapStruct), 32*_K_,  FrameBasedFPP,    1,  eSirenLPR_Scalable_32kDynamicPayload},
	{cmCapAudio,       TRUE,  eSirenLPR_Scalable_48kCapCode,        _SirenLPR_Scalable, sizeof(sirenLPR_Scalable_CapStruct), 48*_K_,  FrameBasedFPP,    1,  eSirenLPR_Scalable_48kDynamicPayload},
	{cmCapAudio,       TRUE,  eSirenLPR_Scalable_64kCapCode,        _SirenLPR_Scalable, sizeof(sirenLPR_Scalable_CapStruct), 64*_K_,  FrameBasedFPP,    1,  eSirenLPR_Scalable_64kDynamicPayload},
	{cmCapAudio,       TRUE,  eSirenLPRStereo_Scalable_64kCapCode,  _SirenLPR_Scalable, sizeof(sirenLPR_Scalable_CapStruct), 64*_K_,  FrameBasedFPP,    1,  eSirenLPRStereo_Scalable_64kDynamicPayload},
	{cmCapAudio,       TRUE,  eSirenLPRStereo_Scalable_96kCapCode,  _SirenLPR_Scalable, sizeof(sirenLPR_Scalable_CapStruct), 96*_K_,  FrameBasedFPP,    1,  eSirenLPRStereo_Scalable_96kDynamicPayload},
	{cmCapAudio,       TRUE,  eSirenLPRStereo_Scalable_128kCapCode, _SirenLPR_Scalable, sizeof(sirenLPR_Scalable_CapStruct), 128*_K_, FrameBasedFPP,    1,  eSirenLPRStereo_Scalable_128kDynamicPayload},
	{cmCapAudio,       TRUE,  eiLBC_13kCapCode,                     _iLBC,              sizeof(iLBC_13kCapStruct),           16*_K_,  FrameBasedFPP,    1,  eiLBC_13kDynamicPayload},
	{cmCapAudio,       TRUE,  eiLBC_15kCapCode,                     _iLBC,              sizeof(iLBC_15kCapStruct),           16*_K_,  FrameBasedFPP,    1,  eiLBC_15kDynamicPayload},
    	{cmCapAudio,       TRUE,  eOpus_CapCode,_Opus,    sizeof(opus_CapStruct), 64*_K_, FrameBasedFPP     ,1, 						eOpus_64kDynamicPayload},
   	{cmCapAudio,       TRUE,  eOpusStereo_CapCode,_Opus,    sizeof(opusStereo_CapStruct), 128*_K_, FrameBasedFPP     ,1, 				eOpusStereo_128kDynamicPayload},

	{cmCapAudio,       TRUE,  eRfc2833DtmfCapCode,                  _Rfc2833Dtmf,       sizeof(rfc2833DtmfCapStruct),        NA,      NA,               NA, eRfc2833DtmfDynamicPayload},
	{cmCapVideo,       TRUE,  eH261CapCode,                         _H261,              sizeof(h261CapStruct),               NA,      NA,               NA, eH261DynamicPayload},
	{cmCapVideo,       FALSE, eH262CapCode,                         _ANY,               sizeof(h262CapStruct),               NA,      NA,               NA, eH262DynamicPayload},
	{cmCapVideo,       TRUE,  eH263CapCode,                         _H263,              sizeof(h263CapStruct),               NA,      NA,               NA, eH263DynamicPayload},
	{cmCapVideo,       TRUE,  eH264CapCode,                         _H264,              sizeof(h264CapStruct),               NA,      NA,               NA, eH264DynamicPayload},
	{cmCapVideo,       TRUE,  eH26LCapCode,                         _H26L,              sizeof(genericVideoCapStruct),       NA,      NA,               NA, _UnKnown},
	{cmCapVideo,       TRUE,  eRtvCapCode,                          _Rtv,               sizeof(rtvCapStruct),                NA,      NA,               NA, eRtvDynamicPayload},
	{cmCapVideo,       FALSE, eIS11172VideoCapCode,                 _ANY,               sizeof(IS11172VideoCapStruct),       NA,      NA,               NA, _UnKnown},
    	{cmCapVideo,       TRUE,  eVP8CapCode,					_VP8,           sizeof(vp8CapStruct),		        NA,     NA         ,NA, eVP8DynamicPayload}, //N.A. DEBUG VP8
	{cmCapVideo,       FALSE, eGenericVideoCapCode,                 _ANY,               sizeof(genericVideoCapStruct),       NA,      NA,               NA, eGenericVideoDynamicPayload},
	{cmCapVideo,       TRUE,  eSvcCapCode,                          _SVC,               sizeof(svcCapStruct),                NA,      NA,               NA, eSvcDynamicPayload},
	{cmCapData,        TRUE,  eT120DataCapCode,                     _T120,              sizeof(t120DataCapStruct),           NA,      NA,               NA, eT120DynamicPayload},
	{cmCapData,        TRUE,  eAnnexQCapCode,                       _AnnexQ,            sizeof(annexQDataCapStruct),         NA,      NA,               NA, eAnnexQDynamicPayload},
	{cmCapData,        TRUE,  eRvFeccCapCode,                       _RvFecc,            sizeof(rvFeccDataCapStruct),         NA,      NA,               NA, eRvFeccDynamicPayload},
	{cmCapNonStandard, TRUE,  eNonStandardCapCode,                  _NONS,              sizeof(ctNonStandardCapStruct),      NA,      NA,               NA, _UnKnown},
	{cmCapNonStandard, FALSE, eGenericCapCode,                      _ANY,               sizeof(genericCapStruct),            NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  ePeopleContentCapCode,                _H323_P_C,          sizeof(PeopleAndContentCapStruct),   NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eRoleLabelCapCode,                    _H323_P_C,          sizeof(PeopleAndContentCapStruct),   NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eH239ControlCapCode,                  _ANY,               sizeof(h239ControlCapStruct),        NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     FALSE, eChairControlCapCode,                 _ANY,               sizeof(ChairControlCapStruct),       NA,      NA,               NA, _UnKnown},
	{cmCapH235,        TRUE,  eEncryptionCapCode,                   _ANY,               sizeof(encryptionCapStruct),         NA,      NA,               NA, _UnKnown},
	{cmCapVideo,       TRUE,  eDBC2CapCode,                         _H263,              sizeof(genericVideoCapStruct),       NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eLPRCapCode,                          _ANY,               sizeof(lprCapStruct),                NA,      NA,               NA, eLprDynamicPayload},
	{cmCapGeneric,     TRUE,  eDynamicPTRCapCode,                   _ANY,               sizeof(DynamicPTRCapStruct),         NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eIcePwdCapCode,                       _IcePwd,            sizeof(icePwdCapStruct),             NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eIceUfragCapCode,                     _IceUfrag,          sizeof(iceUfragCapStruct),           NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eIceCandidateCapCode,                 _IceCand,           sizeof(iceCandidateCapStruct),       NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eIceRemoteCandidateCapCode,           _IceRemoteCand,     sizeof(iceRemoteCandidateCapStruct), NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eRtcpCapCode,                         _Rtcp,              sizeof(rtcpCapStruct),               NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eSdesCapCode,                         _ANY,               sizeof(sdesCapStruct),               NA,      NA,               NA, _UnKnown},
	{cmCapBfcp,        TRUE,  eBFCPCapCode,                         _UnKnown,           sizeof(bfcpCapStruct),               NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     FALSE, eMCCFCapCode,                         _MCCF,              sizeof(mccfCapStruct),               NA,      NA,               NA, _UnKnown},
	{cmCapGeneric,     TRUE,  eDtlsCapCode,                         _ANY,               sizeof(sdesCapStruct),               NA,      NA,               NA, _UnKnown},
	{cmCapVideo,       TRUE,  eMsSvcCapCode,                        _MS_SVC,            sizeof(msSvcCapStruct),              NA,      NA,               NA, eMsSvcDynamicPayload},
    {cmCapGeneric,	   TRUE,  eFECCapCode,					_FEC,		sizeof(fecCapStruct),				NA,		NA		   ,NA, eFECDynamicPayload},   //LYNC2013_FEC_RED
    {cmCapAudio,	   TRUE,  eREDCapCode,					_RED,		sizeof(redCapStruct),				NA,		NA		   ,1, eREDDynamicPayload},   //LYNC2013_FEC_RED
	{cmCapAudio,       TRUE , eSiren7_16kCapCode,                   _Siren7,       	    sizeof(siren7_16kCapStruct),         16*_K_,  FrameBasedFPP,    1,  eSiren7_16kDynamicPayload},
	{cmCapEmpty,       FALSE, eUnknownAlgorithemCapCode,            _ANY,               0,                                   NA,      NA,               NA, _UnKnown}
};

// Holds the indexes of audio only caps.
const int CCapSetInfo::g_audioOnlyCaps[MAX_AUDIO_ONLY_CAPS] = {
	eG729AnnexACapCode,
	eG711Ulaw64kCapCode,
	eG7231CapCode
};

// Holds the indexes of software CP caps.
const int CCapSetInfo::g_softCPCaps[MAX_SOFT_CP_CAPS] = {eH261CapCode};


// Holds the different macros for each video format
const APIS16 CCapSetInfo::g_videoFormatTbl[MAX_VIDEO_FORMATS][MAX_VIDEO_CAPS] =
{
//   h261       |h262       |h263             |h264     |IS11172Video   |genericVideoCap
//	----------------------------------------------------------------------------------------
	{V_Qcif,	 NA,         H263_QCIF_SQCIF  ,NA       ,	NA,				NA},
	{V_Cif,		 NA,         H263_CIF         ,NA       ,	NA,				NA},
	{NA,         NA,         H263_CIF_4       ,NA       ,	NA,				NA},
	{NA,         NA,         H263_CIF_16      ,NA       ,	NA,				NA},
	{NA,         NA,         VGA              ,NA       ,	NA,				NA},
	{NA,         NA,         NTSC             ,NA       ,	NA,				NA},
	{NA,         NA,         SVGA             ,NA       ,	NA,				NA},
	{NA,         NA,         XGA              ,NA       ,	NA,				NA},
	{NA,         NA,         NTSC_60_FIELDS   ,NA       ,	NA,				NA}//qNTSC
};



// Holds the mlp or hmlp caps with their rates in an order from the highest rate to the lowest
const TDataCapQuality CCapSetInfo::g_T120dataCapQualityTbl[MAX_MLP_HMLP_CAPS] =
{
	{H_Mlp_Cap_384 ,    (DWORD)384  *_K_ },
	{H_Mlp_Cap_320 ,    (DWORD)320  *_K_ },
	{H_Mlp_Cap_256 ,    (DWORD)256  *_K_ },
	{H_Mlp_Cap_192 ,    (DWORD)192  *_K_ },
	{H_Mlp_Cap_128 ,    (DWORD)128  *_K_ },
	{H_Mlp_Cap_64  ,    (DWORD) 64  *_K_ },
	{H_Mlp_Cap_62_4,    (DWORD)(62.4*_K_)},
	{H_Mlp_Cap_14_4,    (DWORD)(14.4*_K_)},
	{Mlp_Cap_46_4  ,    (DWORD)(46.4*_K_)},
	{Mlp_Cap_40,        (DWORD) 40  *_K_ },
	{Mlp_Cap_38_4,      (DWORD)(38.4*_K_)},
	{Mlp_Cap_32,        (DWORD) 32  *_K_ },
	{Mlp_Cap_30_4,      (DWORD)(30.4*_K_)},
	{Mlp_Cap_24,        (DWORD) 24  *_K_ },
	{Mlp_Cap_22_4,      (DWORD)(22.4*_K_)},
	{Mlp_Cap_16  ,      (DWORD) 16  *_K_ },
	{Mlp_Cap_14_4,      (DWORD)(14.4*_K_)},
	{Dxfer_Cap_Mlp_6_4k,(DWORD)(6.4 *_K_)},
	{Dxfer_Cap_Mlp_4k,  (DWORD) 4   *_K_ },
};


/*
const TDataCapQuality CCapSetInfo::g_H224dataCapQualityTbl[MAX_H224_CAPS] =
{
	{Dxfer_Cap_6400,  (DWORD)(6.4 *_K_)},
	{Dxfer_Cap_4800,  (DWORD)(4.8 *_K_)},
};*/


// Holds the video content caps with their rates in an order from the highest rate to the lowest
const TEpcCapQuality CCapSetInfo::g_EpcCapQualityTbl[MAX_EPC_CAPS] =
{
	{AMSC_0k ,    (DWORD)(0   *_K_)},
	{AMSC_64k ,   (DWORD)(64  *_K_)},
	{AMSC_128k ,  (DWORD)(128 *_K_)},
	{AMSC_192k ,  (DWORD)(192 *_K_)},
	{AMSC_256k ,  (DWORD)(256 *_K_)},
	{AMSC_384k ,  (DWORD)(384 *_K_)},
	{AMSC_512k ,  (DWORD)(512 *_K_)},
	{AMSC_768k ,  (DWORD)(768 *_K_)},
	{AMSC_1024k,  (DWORD)(1024*_K_)},
	{AMSC_1152k,  (DWORD)(1152*_K_)},
	{AMSC_1280k,  (DWORD)(1280*_K_)},
	{AMSC_1536k,  (DWORD)(1536*_K_)},
	{AMSC_2048k,  (DWORD)(2048*_K_)},
	{AMSC_2560k,  (DWORD)(2560*_K_)},
	{AMSC_3072k,  (DWORD)(3072*_K_)},
	{AMSC_4096k,  (DWORD)(4096*_K_)},
};

const TH239CapQuality CCapSetInfo::g_H239CapQualityTbl[MAX_H239_CAPS] =
{
	{AMC_0k, 	  (DWORD)(0   *_K_)},
	{AMC_40k,     (DWORD)(40  *_K_)},
	{AMC_64k,     (DWORD)(64  *_K_)},
	{AMC_96k,     (DWORD)(96  *_K_)},
	{AMC_128k,    (DWORD)(128 *_K_)},
	{AMC_192k,    (DWORD)(192 *_K_)},
	{AMC_256k,    (DWORD)(256 *_K_)},
	{AMC_384k,    (DWORD)(384 *_K_)},
	{AMC_512k,    (DWORD)(512 *_K_)},
	{AMC_768k,    (DWORD)(768 *_K_)},
	{AMC_1024k,   (DWORD)(1024*_K_)},
	{AMC_1152k,   (DWORD)(1152*_K_)},
	{AMC_1280k,   (DWORD)(1280*_K_)},
	{AMC_1536k,   (DWORD)(1536*_K_)},
	{AMC_2048k,   (DWORD)(2048*_K_)},
	{AMC_2560k,   (DWORD)(2560*_K_)},
	{AMC_3072k,   (DWORD)(3072*_K_)},
	{AMC_4096k,   (DWORD)(4096*_K_)},
};


const TH263Mpi CCapSetInfo::g_h263MpiTbl[MAX_MPI_SUPPORTED_IN_H263] =
{
	{MPI_1,  1},
	{MPI_2,  2},
	{MPI_3,  3},
	{MPI_4,  4},
	{MPI_5,  5},
	{MPI_6,  6},
	{MPI_10, 10},
	{MPI_15, 15},
	{MPI_30, 30},
};


const TCallRate CCapSetInfo::g_callRatesTbl[MAX_RESERVATION_RATES] =
{
    {Xfer_64,        64},
    {Xfer_2x64,      2*64},
    {Xfer_3x64,      3*64},
    {Xfer_4x64,      4*64},
    {Xfer_5x64,      5*64},
    {Xfer_6x64,      6*64},
    {Xfer_384,       384},
    {Xfer_1536,      1536},
    {Xfer_1920,      1920},
    {Xfer_128,       128},
    {Xfer_192,       192},
    {Xfer_256,       256},
    {Xfer_320,       320},
    {Xfer_512,       512},
    {Xfer_768,       768},
    {Xfer_832,       832},
    {Xfer_1024,      1024},
    {Xfer_1152,      1152},
    {Xfer_1280,      1280},
    {Xfer_1472,      1472},
    {Xfer_1728,      1728},
    {Xfer_2048,      2048},
    {Xfer_96,        96},
    {Xfer_2560,		 2560},
    {Xfer_3072,		 3072},
    {Xfer_3584,		 3584},
    {Xfer_4096,      4096},
    {Xfer_6144,      6144},
    {Xfer_8192,      8192}
};

const TFormatNames CCapSetInfo::g_formatNames[MAX_FORMATS_NAMES] =
{
	// EFormat			//char*
	{kQCif,				"QCif"},
	{kCif,  			"Cif"},
	{k4Cif, 			"4Cif"},
	{k16Cif,			"16Cif"},
	{kVGA,				"VGA"},
	{kNTSC,				"NTSC"},
	{kSVGA, 			"SVGA"},
	{kXGA,				"XGA"},
	{kSIF, 				"SIF"},
	{kQVGA,				"QVGA"},
	{k2Cif,				"2Cif"},
	{k2SIF,				"2SIF"},
	{k4SIF,				"4SIF"},
	{k525SD,			"525SD"},
	{k625SD,			"625SD"},
	{k720p,				"720p"},
	{k1080p,			"1080p"},
	{kLastFormat,		"invalid"}
};
//for cdr event: PARTICIPANT_DISCONNECT_INFORMATION (chinese people want to know just about cif,sd,720,1080), the char* are according to EMA
const TFormatNames CCapSetInfo::g_formatNamesForPartyDisconnectCDREvent[MAX_FORMATS_NAMES] =
{
	// EFormat			//char*
	{kQCif,				"cif"},
	{kCif,  			"cif"},
	{k4Cif, 			"sd"},
	{k16Cif,			"hd_1080"},
	{kVGA,				"sd"},
	{kNTSC,				"sd"},
	{kSVGA, 			"sd"},
	{kXGA,				"sd"},
	{kSIF, 				"cif"},
	{kQVGA,				"cif"},
	{k2Cif,				"sd"},
	{k2SIF,				"sd"},
	{k4SIF,				"sd"},
	{k525SD,			"sd"},
	{k625SD,			"sd"},
	{k720p,				"hd_720"},
	{k1080p,			"hd_1080"},
	{kLastFormat,		"invalid"}
};

const TFrameRate CCapSetInfo::g_maxFrameRateForPartyDisconnectCDREvent[MAX_FRAME_RATE_ROUND_UP] =
{
    // eVideoFrameRate      //char*
    {eVideoFrameRateDUMMY,   "0"},
    {eVideoFrameRate60FPS,   "60"},
    {eVideoFrameRate50FPS,   "50"},
    {eVideoFrameRate30FPS,   "30"},
    {eVideoFrameRate25FPS,   "25"},
    {eVideoFrameRate15FPS,   "15"},
    {eVideoFrameRate12_5FPS, "12.5"},
    {eVideoFrameRate10FPS,   "10"},
    {eVideoFrameRate7_5FPS,  "7.5"},
    {eVideoFrameRate5FPS,    "7.5"},
    {eVideoFrameRate6FPS,    "7.5"},
    {eVideoFrameRate3FPS,    "7.5"},
    {eVideoFrameRateLast,    "invalid"}
};

///MUST BE SORTED BY EFormat
const TOrderedFormatStruct CCapSetInfo::g_OrderedFormatTable[MAX_FORMATS_NAMES] =
{
	// EFormat			//EOrderedFormat
	{kQCif,				eOrderedQCif},
	{kCif,  			eOrderedCif},
	{k4Cif, 			eOrdered4Cif},
	{k16Cif,			eOrdered16Cif},
	{kVGA,				eOrderedVGA},
	{kNTSC,				eOrderedNTSC},
	{kSVGA, 			eOrderedSVGA},
	{kXGA,				eOrderedXGA},
	{kSIF, 				eOrderedSIF},
	{kQVGA,				eOrderedQVGA},
	{k2Cif,				eOrdered2Cif},
	{k2SIF,				eOrdered2SIF},
	{k4SIF,				eOrdered4SIF},
	{k525SD,			eOrdered525SD},
	{k625SD,			eOrdered625SD},
	{k720p,				eOrdered720p},
	{k1080p,			eOrdered1080p},
	{kLastFormat,		eOrderedLastFormat}
};

// Holds each resolution width (X) and Height (Y) values
const TFormatWidthHeight CCapSetInfo::g_videoWidthHeight[MAX_FORMATS_NAMES] =
{
		// EOrderedFormat			//width		//Height
		{eOrderedQCif,			176,		144},
		{eOrderedQVGA,			320,		240},
		{eOrderedSIF, 			352,		240},
		{eOrderedCif,  			352,		288},
		{eOrdered2SIF,			704,		240},
		{eOrdered2Cif,			704,		288},
		{eOrderedVGA,			640,		480},
		{eOrdered4SIF,			704,		480},
		{eOrdered525SD,			720,		480},
		{eOrderedNTSC,			720,		480},
		{eOrdered4Cif, 			704,		576},
		{eOrdered625SD,			720,		576},
		{eOrderedSVGA, 			800,		600},
		{eOrderedXGA,			1024,		768},
		{eOrdered720p,			1280,		720},
		{eOrdered16Cif,			1408,		1152},
		{eOrdered1080p,			1920,		1080},
		{eOrderedLastFormat,	0,			0}
};

const TLprCapSet CCapSetInfo::g_lprCapSet[MAX_LPR_CAP] =
{
	  // versionID	minProtectionPeriod		maxProtectionPeriod		maxRecoverySet	maxRecoveryPackets	maxPacketSize
		{1,			150,					150,					52,				10,					1260		}
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Constructors:

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Checks if the value is legal and sets the table index.
//---------------------------------------------------------------------------------------------------
CCapSetInfo::CCapSetInfo(CapEnum h323Cap)
{
	if (h323Cap < eUnknownAlgorithemCapCode)
		m_index = h323Cap;
	else
		m_index = eUnknownAlgorithemCapCode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Get cap set adjested to SIP (LPR in SIP is video and in H323 is generic.
//---------------------------------------------------------------------------------------------------
cmCapDataType CCapSetInfo::GetSipCapType() const
{
	//PTRACE2INT(eLevelInfoNormal,"Shira CCapSetInfo::GetSipCapType", g_capQualityTbl[m_index].IpCapCode);
	if (g_capQualityTbl[m_index].IpCapCode == eLPRCapCode || g_capQualityTbl[m_index].IpCapCode == eFECCapCode)
		return cmCapVideo;
	else
		return GetCapType();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Looks for the payload type with the specific rate.
//---------------------------------------------------------------------------------------------------
CCapSetInfo::CCapSetInfo(payload_en payload, APIS32 bitRate)
{
    m_index = eUnknownAlgorithemCapCode;

	if ((payload == _H263_P) && (bitRate == 0))
		m_index = eH263CapCode;

    else if (payload != _ANY)
    {
        for (int i=0;i<eUnknownAlgorithemCapCode;i++)
		{
			if (payload == g_capQualityTbl[i].payloadType)
			{
				if (bitRate == 0)
				{
					m_index = i;
					break;
				}
				else if (bitRate == g_capQualityTbl[i].bitRate)
				{
					m_index = i;
					break;
				}
			}
		}
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Looks for the cap enum string prefix according to CapCodeArray.
//---------------------------------------------------------------------------------------------------
CCapSetInfo::CCapSetInfo(const char* strCapName)
{
	m_index = eUnknownAlgorithemCapCode;
	if (strCapName)
	{
		for (int i = 0; i < eUnknownAlgorithemCapCode; ++i)
		{
			if (IsMatchingCapName(strCapName, CapEnumToString((CapEnum)i)))
			{
				m_index = i;
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CCapSetInfo & CCapSetInfo::operator=(const CCapSetInfo& other)
{
	if (&other != this)
		m_index = other.m_index;
	return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
CCapSetInfo & CCapSetInfo::operator=(CapEnum h323Cap)
{
	if ((h323Cap < eUnknownAlgorithemCapCode)&&(h323Cap >= 0))
		m_index = h323Cap;
	return *this;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Operations:

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Finds the next h323 cap code with the same type and set its index.
//             When not found sets the last index in the table.
//---------------------------------------------------------------------------------------------------
void CCapSetInfo::SetNextType()
{
	BYTE bFound = FALSE;

	for(int i = m_index+1; i < eUnknownAlgorithemCapCode; i++)
	{
		if (g_capQualityTbl[i].type == g_capQualityTbl[m_index].type)
		{
			m_index = i;
			bFound = TRUE;
			break;
		}
	}

	if (bFound == FALSE)
		m_index = eUnknownAlgorithemCapCode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Returns the next h323 cap code with the same payload type.
//             When not found returns the last index in the table.
//---------------------------------------------------------------------------------------------------

CapEnum CCapSetInfo::GetNextIpCapCodeWithSamePayloadType() const
{
	CapEnum eRes = eUnknownAlgorithemCapCode;

	for(int i = m_index+1; i < eUnknownAlgorithemCapCode; i++)
	{
		if (g_capQualityTbl[i].payloadType == g_capQualityTbl[m_index].payloadType)
		{
			eRes = (CapEnum)i;
			break;
		}
	}

	return eRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Returns the previous h323 cap code with the same payload type.
//             When not found returns the last index in the table.
//---------------------------------------------------------------------------------------------------
CapEnum CCapSetInfo::GetPrevIpCapCodeWithSamePayloadType() const
{
	CapEnum eRes = eUnknownAlgorithemCapCode;

	for(int i = m_index-1; i >= 0; i--)
	{
		if (g_capQualityTbl[i].payloadType == g_capQualityTbl[m_index].payloadType)
		{
			eRes = (CapEnum)i;
			break;
		}
	}

	return eRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Finds the next h323 cap code with the same payload type and set its index.
//             When not found sets the last index in the table.
//---------------------------------------------------------------------------------------------------

void CCapSetInfo::SetNextIpCapCodeWithSamePayloadType()
{
	m_index = GetNextIpCapCodeWithSamePayloadType();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Finds the previous h323 cap code with the same payload type and set its index
//             When not found sets the last index in the table.
//---------------------------------------------------------------------------------------------------
void CCapSetInfo::SetPrevIpCapCodeWithSamePayloadType()
{
	m_index = GetPrevIpCapCodeWithSamePayloadType();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Get bitrate
//---------------------------------------------------------------------------------------------------
DWORD CCapSetInfo::GetBitRate(BYTE* pData) const
{
	DWORD rate = g_capQualityTbl[m_index].bitRate;

	if (pData) {
		switch (GetIpCapCode()) {
			case eOpus_CapCode:
			case eOpusStereo_CapCode:
			{
				opus_CapStruct *pOpus = (opus_CapStruct *)pData;
				rate = pOpus->maxAverageBitrate;
				PTRACE2INT(eLevelInfoNormal, "CCapSetInfo::GetBitRate with data for opus, rate = ", rate);
			}
			break;

			default:
				rate = g_capQualityTbl[m_index].bitRate;
		}
	}

	return rate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if it's the cap set type
//---------------------------------------------------------------------------------------------------
BYTE CCapSetInfo::IsType(cmCapDataType type) const
{
	BYTE bRes = (g_capQualityTbl[m_index].type == type);
	return bRes;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if it's the h323 cap code
//---------------------------------------------------------------------------------------------------

BYTE CCapSetInfo::IsCapCode(CapEnum IpCapCode) const
{
	BYTE bRes = (g_capQualityTbl[m_index].IpCapCode == IpCapCode);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if it's the payload type
//---------------------------------------------------------------------------------------------------

BYTE CCapSetInfo::IsPayloadType(payload_en payloadType) const
{
	BYTE bRes = (g_capQualityTbl[m_index].payloadType == payloadType);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Check frame per packet if it is quitient as it expected.
//---------------------------------------------------------------------------------------------------
BYTE CCapSetInfo::CheckFramePerPacket(int framePerPacket) const
{
	int res = framePerPacket;

	if(g_capQualityTbl[m_index].packetQuotient != NA)
	{
		res /= g_capQualityTbl[m_index].packetQuotient;
		res *= g_capQualityTbl[m_index].packetQuotient;
	}
	else
	{
		PASSERTMSG((DWORD)m_index,"The alg is not supported"); //The alg is not supported!!
		res = NA;
	}

	if(framePerPacket == res)
		return TRUE;
	else
		return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the quotient of audio frame per packet according to audio alg.
//---------------------------------------------------------------------------------------------------
int CCapSetInfo::GetFramePerPacketQuotient(int framePerPacket) const
{
	int res = framePerPacket;

	if(g_capQualityTbl[m_index].packetQuotient != NA)
	{
		res /= g_capQualityTbl[m_index].packetQuotient;
		res *= g_capQualityTbl[m_index].packetQuotient;
	}
	else
	{
		PASSERTMSG((DWORD)m_index,"The alg is not supported"); //The alg is not supported!!
		res = NA;
	}

/*
	if (framePerPacket > 3)
	{
		if (g_capQualityTbl[m_index].bitRate >= 48*_K_)
		{
			res /= 10;
			res *= 10;
		}
		else
		{
			res /= 4;
			res *= 4;
		}
	}
*/
	return res;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CCapSetInfo::GetCodecNumberOfChannels()
{
	return ::GetCodecNumberOfChannels(GetIpCapCode());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the next audio only cap. If not found sets the last table index.
//---------------------------------------------------------------------------------------------------
void CCapSetInfo::SetNextAudioOnlyCap()
{
	BYTE bFound = FALSE;

	for (int i = 0; i < (MAX_AUDIO_ONLY_CAPS - 1); i++)
	{
		if (g_audioOnlyCaps[i] == m_index)
		{
			m_index = g_audioOnlyCaps[i+1];
			bFound = TRUE;
			break;
		}
	}

	if (bFound == FALSE)
		m_index = eUnknownAlgorithemCapCode;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Sets the next software CP cap. If not found sets the last table index.
//---------------------------------------------------------------------------------------------------
void CCapSetInfo::SetNextSoftCPCap()
{
	BYTE bFound = FALSE;

	for (int i = 0; i < (MAX_SOFT_CP_CAPS - 1); i++)
	{
		if (g_softCPCaps[i] == m_index)
		{
			m_index = g_softCPCaps[i+1];
			bFound = TRUE;
			break;
		}
	}

	if (bFound == FALSE)
		m_index = eUnknownAlgorithemCapCode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if this video algorithem supports software CP.
//---------------------------------------------------------------------------------------------------
BYTE CCapSetInfo::IsSupportedSoftCP() const
{
	BYTE bRes = FALSE;

	for (int i = 0; i < MAX_SOFT_CP_CAPS; i++)
	{
		if (m_index == g_softCPCaps[i])
		{
			bRes = TRUE;
			break;
		}
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Tells if algorithm is supported. Check system.cfg if needed.
//---------------------------------------------------------------------------------------------------
BYTE  CCapSetInfo::IsSupporedCap()
{
	BYTE bRes = FALSE;
	UpdateFromSystemCfg();
	UpdateAccordingToSystemMode();
	UpdateAccordingToProductType();
	bRes = g_capQualityTbl[m_index].bSupported;
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the max frame per packet of alg
//---------------------------------------------------------------------------------------------------
int CCapSetInfo::GetMaxFramePerPacket()
{
	UpdateFromSystemCfg();
	int res = g_capQualityTbl[m_index].framePerPacket;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Updates table information from system.cfg
//---------------------------------------------------------------------------------------------------
BYTE CCapSetInfo::UpdateFromSystemCfg()
{
	static BYTE g_bSysCfgChecked = FALSE;

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (!pSysConfig)
		return FALSE;

	BOOL bIsIpDebugStatus = 0;
	pSysConfig->GetBOOLDataByKey("IP_DEBUG_STATUS", bIsIpDebugStatus);
	if (bIsIpDebugStatus || !g_bSysCfgChecked)
	{
			pSysConfig->GetBOOLDataByKey("G711", g_capQualityTbl[eG711Ulaw64kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G711_1", g_capQualityTbl[eG711Alaw64kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G722", g_capQualityTbl[eG722_64kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G722_Stereo", g_capQualityTbl[eG722Stereo_128kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G722_1", g_capQualityTbl[eG7221_32kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("G722_1", g_capQualityTbl[eG7221_24kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G722_1_16K", g_capQualityTbl[eG7221_16kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G7231", g_capQualityTbl[eG7231CapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G729", g_capQualityTbl[eG729AnnexACapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("SIREN7",  g_capQualityTbl[eSiren7_16kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN14", g_capQualityTbl[eSiren14_48kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN14", g_capQualityTbl[eSiren14_32kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN14", g_capQualityTbl[eSiren14_24kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("SIREN14_Stereo", g_capQualityTbl[eSiren14Stereo_48kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN14_Stereo", g_capQualityTbl[eSiren14Stereo_64kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN14_Stereo", g_capQualityTbl[eSiren14Stereo_96kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("SIREN22", g_capQualityTbl[eSiren22_64kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN22", g_capQualityTbl[eSiren22_48kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN22", g_capQualityTbl[eSiren22_32kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("SIREN22_Stereo", g_capQualityTbl[eSiren22Stereo_128kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN22_Stereo", g_capQualityTbl[eSiren22Stereo_96kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("SIREN22_Stereo", g_capQualityTbl[eSiren22Stereo_64kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("ENABLE_SIRENLPR", g_capQualityTbl[eSirenLPR_32kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("ENABLE_SIRENLPR", g_capQualityTbl[eSirenLPR_48kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("ENABLE_SIRENLPR", g_capQualityTbl[eSirenLPR_64kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("ENABLE_SIRENLPR", g_capQualityTbl[eSirenLPRStereo_64kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("ENABLE_SIRENLPR", g_capQualityTbl[eSirenLPRStereo_96kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("ENABLE_SIRENLPR", g_capQualityTbl[eSirenLPRStereo_128kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G719", g_capQualityTbl[eG719_64kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("G719", g_capQualityTbl[eG719_48kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("G719", g_capQualityTbl[eG719_32kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G719_Stereo", g_capQualityTbl[eG719Stereo_128kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("G719_Stereo", g_capQualityTbl[eG719Stereo_96kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("G719_Stereo", g_capQualityTbl[eG719Stereo_64kCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G7221_C", g_capQualityTbl[eG7221C_48kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("G7221_C", g_capQualityTbl[eG7221C_32kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("G7221_C", g_capQualityTbl[eG7221C_24kCapCode].bSupported);
			pSysConfig->GetBOOLDataByKey("G7221_C", g_capQualityTbl[eG7221C_CapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("G728_IP", g_capQualityTbl[eG728CapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("RFC2833_DTMF", g_capQualityTbl[eRfc2833DtmfCapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("H261", g_capQualityTbl[eH261CapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("H263", g_capQualityTbl[eH263CapCode].bSupported);

			pSysConfig->GetBOOLDataByKey("H264", g_capQualityTbl[eH264CapCode].bSupported);
				//key = "vp8";
				//pSysConfig->GetBOOLDataByKey(key, g_capQualityTbl[eVP8CapCode].bSupported);

			g_bSysCfgChecked = TRUE;
			return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Updates table information according to system mode
//---------------------------------------------------------------------------------------------------
BYTE  CCapSetInfo::UpdateAccordingToSystemMode()
{
	BYTE bRes = FALSE;
	static BYTE g_bSysModeChecked = FALSE;

	if (g_bSysModeChecked == FALSE)
	{
		eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
		TRACEINTO << "SystemMode:" << GetSystemCardsModeStr(systemCardsBasedMode);
		if (systemCardsBasedMode == eSystemCardsMode_mpm)
		{
			g_capQualityTbl[eG719_32kCapCode].bSupported = NO;
			g_capQualityTbl[eG719_48kCapCode].bSupported = NO;
			g_capQualityTbl[eG719_64kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren22_32kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren22_48kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren22_64kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren14Stereo_48kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren14Stereo_56kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren14Stereo_64kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren14Stereo_96kCapCode].bSupported = NO;
			g_capQualityTbl[eG719Stereo_64kCapCode].bSupported = NO;
			g_capQualityTbl[eG719Stereo_96kCapCode].bSupported = NO;
			g_capQualityTbl[eG719Stereo_128kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren22Stereo_64kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren22Stereo_96kCapCode].bSupported = NO;
			g_capQualityTbl[eSiren22Stereo_128kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPR_32kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPR_48kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPR_64kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPRStereo_64kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPRStereo_96kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPRStereo_128kCapCode].bSupported = NO;
			g_capQualityTbl[eG728CapCode].bSupported = NO;
		}
		if (systemCardsBasedMode == eSystemCardsMode_mpm_plus)
		{
			/* SirenLPR is not suppported on MPM+ cards */
			g_capQualityTbl[eSirenLPR_32kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPR_48kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPR_64kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPRStereo_64kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPRStereo_96kCapCode].bSupported = NO;
			g_capQualityTbl[eSirenLPRStereo_128kCapCode].bSupported = NO;
		}

		bRes = TRUE;
		g_bSysModeChecked = TRUE;
	}

	return bRes;
}
//---------------------------------------------------------------------------------------------------
void CCapSetInfo::UpdateAccordingToProductType( )
{
	static BYTE g_bProductTypeChecked = FALSE;

	if (g_bProductTypeChecked == FALSE)
	{
//		if(eProductTypeSoftMCUMfw == CProcessBase::GetProcess()->GetProductType() )
//		{
//			g_capQualityTbl[eSirenLPR_32kCapCode].bSupported = NO;
//			g_capQualityTbl[eSirenLPR_48kCapCode].bSupported = NO;
//			g_capQualityTbl[eSirenLPR_64kCapCode].bSupported = NO;
//			g_capQualityTbl[eSirenLPRStereo_64kCapCode].bSupported = NO;
//			g_capQualityTbl[eSirenLPRStereo_96kCapCode].bSupported = NO;
//			g_capQualityTbl[eSirenLPRStereo_128kCapCode].bSupported = NO;
//		}

		if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
		{
			PTRACE(eLevelInfoNormal, "CCapSetInfo::UpdateAccordingToProductType : Disable unsupported codecs for Soft MCU and Ninja");

//			g_capQualityTbl[eG719_32kCapCode].bSupported = NO;
//			g_capQualityTbl[eG719_48kCapCode].bSupported = NO;
//			g_capQualityTbl[eG719_64kCapCode].bSupported = NO;
			g_capQualityTbl[eG719_96kCapCode].bSupported = NO;
//			g_capQualityTbl[eG719Stereo_64kCapCode].bSupported = NO;
//			g_capQualityTbl[eG719Stereo_96kCapCode].bSupported = NO;
//			g_capQualityTbl[eG719Stereo_128kCapCode].bSupported = NO;

//			g_capQualityTbl[eSiren14Stereo_48kCapCode].bSupported = NO;
//			g_capQualityTbl[eSiren14Stereo_56kCapCode].bSupported = NO;
//			g_capQualityTbl[eSiren14Stereo_64kCapCode].bSupported = NO;
//			g_capQualityTbl[eSiren14Stereo_96kCapCode].bSupported = NO;

			g_capQualityTbl[eG728CapCode].bSupported = NO;
			g_capQualityTbl[eG7231CapCode].bSupported = NO;

//			g_capQualityTbl[eH263CapCode].bSupported = NO;
//			g_capQualityTbl[eH261CapCode].bSupported = NO;
//			g_capQualityTbl[eRtvCapCode].bSupported = NO;
		}
		g_bProductTypeChecked = TRUE;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the h320 macro for a specific video format.
//---------------------------------------------------------------------------------------------------
/*
APIS16 CCapSetInfo::GetH320Format(EFormat eFormat) const
{
	APIS16 res = NA;

	PASSERT_AND_RETURN_VALUE(eFormat >= MAX_VIDEO_FORMATS, res);

	if (IsType(cmCapVideo))
	{
		int formatTblIndex = g_capQualityTbl[m_index].IpCapCode - eH261CapCode;
		if(formatTblIndex >= MAX_VIDEO_CAPS)
			PASSERTMSG(1, "formatTblIndex exceed MAX_VIDEO_CAPS");

		res = g_videoFormatTbl[eFormat][formatTblIndex];
	}
	return res;
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the bit rate of a T120 cap
//---------------------------------------------------------------------------------------------------
DWORD CCapSetInfo::GetT120DataBitRate(WORD mlpOrHmlpType) const
{
	DWORD res = 0;
	if (IsType(cmCapData))
	{
		for (int i = 0; i<MAX_MLP_HMLP_CAPS; i++)
		{
			if (mlpOrHmlpType == g_T120dataCapQualityTbl[i].dataType)
			{
				res = g_T120dataCapQualityTbl[i].bitRate;
				break;
			}
		}
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the bit rate of a H224 data cap
//---------------------------------------------------------------------------------------------------
/*DWORD CCapSetInfo::GetH224DataBitRate(WORD dataType) const
{
	DWORD res = 0;
	if (IsType(cmCapData))
	{
		for (int i = 0; i<MAX_H224_CAPS; i++)
		{
			if (dataType == g_H224dataCapQualityTbl[i].dataType)
			{
				res = g_H224dataCapQualityTbl[i].bitRate;
				break;
			}
		}
	}
	return res;
}*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the bit rate of a video content cap
//---------------------------------------------------------------------------------------------------
DWORD CCapSetInfo::GetEpcBitRate(BYTE amscRateType/*, BYTE bIsRestricted*/) const
{
	DWORD res = 0;
	if (IsType(cmCapVideo) && (amscRateType < MAX_EPC_CAPS))
	{
		res = g_EpcCapQualityTbl[amscRateType].bitRate;
	//	if(bIsRestricted)
	//		res = res * 7 / 8; //In restriced call the content rate is 7/8 from the total rate.
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CCapSetInfo::GetH239BitRate(BYTE amcRateType/*, BYTE bIsRestricted*/) const
{
	DWORD res = 0;
	if (IsType(cmCapVideo) && (amcRateType < MAX_H239_CAPS))
	{
		res = g_H239CapQualityTbl[amcRateType].bitRate;
	//	if(bIsRestricted)
	//		res = res * 7 / 8; //In restriced call the content rate is 7/8 from the total rate.
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Gets the amsc rate type (macro) of a video content cap
//---------------------------------------------------------------------------------------------------
char CCapSetInfo::GetAmscRateType(DWORD videoContentBitRate) const
{
	BYTE res = 0;
	if (IsType(cmCapVideo) && (videoContentBitRate < 1536*_K_))
	{
		for (int i=0; i<MAX_EPC_CAPS; i++)
		{
			if (g_EpcCapQualityTbl[i].bitRate == videoContentBitRate)
			{
				res = g_EpcCapQualityTbl[i].amscRateType;
				break;
			}
		}
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
char CCapSetInfo::GetAmcRateType(DWORD videoContentBitRate) const
{
	BYTE res = 0;
	if (IsType(cmCapVideo) && (videoContentBitRate < 1536*_K_))
	{
		for (int i=0; i<MAX_H239_CAPS; i++)
		{
			if (g_H239CapQualityTbl[i].bitRate == videoContentBitRate)
			{
				res = g_H239CapQualityTbl[i].amcRateType;
				break;
			}
		}
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int CCapSetInfo::TranslateIsdnMpiToIp(int isdnMpi) const
{
	int ipMpi = -1;
	if (isdnMpi < MAX_MPI_SUPPORTED_IN_H263)
		ipMpi = g_h263MpiTbl[isdnMpi].ipMpi;
	return ipMpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int CCapSetInfo::TranslateIpMpiToIsdn(int ipMpi) const
{
	int isdnMpi = -1;
	for (int i=MPI_1; (i<MAX_MPI_SUPPORTED_IN_H263) && (isdnMpi == -1); i++)
	{
		if (g_h263MpiTbl[i].ipMpi == ipMpi)
			isdnMpi = g_h263MpiTbl[i].isdnMpi;
	}
	return isdnMpi;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int CCapSetInfo::TranslateReservationRateToIpRate(BYTE reservationRate) // static
{
	size_t i = reservationRate;

	if (i < MAX_RESERVATION_RATES && (size_t (g_callRatesTbl[i].reservationRate)) == i)
		return g_callRatesTbl[i].ipRate;

	for (i = 0; i < MAX_RESERVATION_RATES; ++i)
	{
		if (g_callRatesTbl[i].reservationRate == reservationRate)
			return g_callRatesTbl[i].ipRate;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//WARNING!!!
//The opposite translation is not correct (for example, 6*64 = 384, and they are 2 different rows in the table).
int CCapSetInfo::TranslateIpRateToReservationRate(int ipRate) const
{
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
char* CCapSetInfo::GetFormatStr(EFormat eFormat) const
{
	if ((eFormat != kUnknownFormat) && (eFormat<= kLastFormat))
		return g_formatNames[eFormat].formatName;
	else
		return "Unknown Format";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
char* CCapSetInfo::GetFormatStrForCDREvent(EFormat eFormat) const
{
	//PTRACE2INT(eLevelInfoNormal,"CCapSetInfo::GetFormatStrForCDREvent eFormat: ",eFormat);
	//PTRACE2(eLevelInfoNormal,"CCapSetInfo::GetFormatStrForCDREvent 1-",g_formatNames[eFormat].formatName);
	PTRACE2(eLevelInfoNormal,"CCapSetInfo::GetFormatStrForCDREvent 2-",g_formatNamesForPartyDisconnectCDREvent[eFormat].formatName);
	if ((eFormat != kUnknownFormat) && (eFormat<= kLastFormat))
		return g_formatNamesForPartyDisconnectCDREvent[eFormat].formatName;
	else
		return "Unknown Format";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
char* CCapSetInfo::GetMaxFrameRateRoundUpStrForCDREvent(WORD maxFR) const
{
    //PTRACE(eLevelInfoNormal,"CCapSetInfo::GetMaxFrameRateStrForCDREvent");

    // need to round up the value according to EMA - FrameRateType:
    eVideoFrameRate maxFrameRateRoundUp;

    if (maxFR < 8)
        maxFrameRateRoundUp = eVideoFrameRate7_5FPS;
    else if (maxFR <= 10)
        maxFrameRateRoundUp = eVideoFrameRate10FPS;
    else if (maxFR < 13)
        maxFrameRateRoundUp = eVideoFrameRate12_5FPS;
    else if (maxFR <= 15)
        maxFrameRateRoundUp = eVideoFrameRate15FPS;
    else if (maxFR <= 25)
        maxFrameRateRoundUp = eVideoFrameRate25FPS;
    else if (maxFR <= 30)
        maxFrameRateRoundUp = eVideoFrameRate30FPS;
    else if (maxFR <= 50)
        maxFrameRateRoundUp = eVideoFrameRate50FPS;
    else
        maxFrameRateRoundUp = eVideoFrameRate60FPS;

    //PTRACE2INT(eLevelInfoNormal,"CCapSetInfo::GetMaxFrameRateRoundUpStrForCDREvent maxFR: ",maxFR);
    PTRACE2(eLevelInfoNormal,"CCapSetInfo::GetMaxFrameRateRoundUpStrForCDREvent: ",g_maxFrameRateForPartyDisconnectCDREvent[maxFrameRateRoundUp].frameRateText);

    return g_maxFrameRateForPartyDisconnectCDREvent[maxFrameRateRoundUp].frameRateText;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
EOrderedFormat CCapSetInfo::ConvertFormatToOrderedFormat(EFormat eFormat)
{
	if ((eFormat != kUnknownFormat) && (eFormat<= kLastFormat))
		return g_OrderedFormatTable[eFormat].eOrderedFormat;
	else
		return eOrderedUnknownFormat;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EFormat CCapSetInfo::ConvertOrderedFormatToFormat(EOrderedFormat eOrderedFormat)
{
	if ((eOrderedFormat != eOrderedUnknownFormat) && (eOrderedFormat<= eOrderedLastFormat))
	{
		for (int i = eOrderedQCif; i < eOrderedLastFormat ; i++)
		{
			if (g_OrderedFormatTable[i].eOrderedFormat == eOrderedFormat)
				return g_OrderedFormatTable[i].eFormat;
		}
	}
	return kUnknownFormat;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void CCapSetInfo::GetResolutionWidthAndHeight(EFormat eFormat, int& width, int& height) const
{
	EOrderedFormat eOrderedFormat = ConvertFormatToOrderedFormat(eFormat);

	width = 0;
	height = 0;
	if ((eOrderedFormat != eOrderedUnknownFormat) && (eOrderedFormat<= eOrderedLastFormat))
	{
		width = g_videoWidthHeight[eOrderedFormat].width;
		height = g_videoWidthHeight[eOrderedFormat].height;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EFormat CCapSetInfo::GetResolutionFormat(int width, int height)
{
	EFormat eFormat = kUnknownFormat;
	for(int i = eOrderedQCif; i < eOrderedLastFormat; i++)
	{
		if((width < g_videoWidthHeight[i].width ) && (height < g_videoWidthHeight[i].height))
		{
			eFormat = CCapSetInfo::ConvertOrderedFormatToFormat((EOrderedFormat)i);
			return eFormat;
		}
	}
	return eFormat;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Dumps the cap information into a stream
//---------------------------------------------------------------------------------------------------
void  CCapSetInfo::Dump(std::ostream& msg) const
{
	cmCapDataType type = GetCapType();

	if (((type >= cmCapAudio) && (type <= cmCapNonStandard)) || (type == cmCapH235))
		msg << g_typeStrings[type]<<" cap: "<<GetH323CapName();
	else
		msg << " cap: "<<GetH323CapName();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Dumps the cap information into a stream
//---------------------------------------------------------------------------------------------------
void  CCapSetInfo::Dump(CObjString* pMsg) const
{
	cmCapDataType type = GetCapType();

	if (((type >= cmCapAudio) && (type <= cmCapNonStandard)) || (type == cmCapH235))
		*pMsg << g_typeStrings[type]<<" cap: "<<GetH323CapName();
	else
		*pMsg << " cap: "<<GetH323CapName();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU8 CCapSetInfo::GetDynamicPayloadType(BYTE content, BYTE profile, BYTE packetizationMode)
{
	APIU8 dynamicPayLoad = 0;
	if ((m_index <=  eUnknownAlgorithemCapCode) && (m_index >=eG711Alaw64kCapCode))
	{
		if (content == 2 )
			dynamicPayLoad = eH263PlusDynamicPayload;
		else if (profile == H264_Profile_High)
			dynamicPayLoad = eH264HpDynamicPayload;
		else if (profile == H264_Profile_Main)// TIP
			dynamicPayLoad = eH264MpDynamicPayload;
		else if (profile == H264_Profile_BaseLine && packetizationMode == H264_SINGLE_NAL_PACKETIZATION_MODE) /* H264 with packetization mode = 0*/
			dynamicPayLoad = eH264NoPmDynamicPayload;
		else
			dynamicPayLoad = g_capQualityTbl[m_index].dynamicPayloadType;
	}
	return dynamicPayLoad;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32	CCapSetInfo::GetLprVersionID()
{
	return g_lprCapSet[0].versionID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32	CCapSetInfo::GetLprMinProtectionPeriod()
{
	return g_lprCapSet[0].minProtectionPeriod;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32	CCapSetInfo::GetLprMaxProtectionPeriod()
{
	return g_lprCapSet[0].maxProtectionPeriod;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32	CCapSetInfo::GetLprMaxRecoverySet()
{
	return g_lprCapSet[0].maxRecoverySet;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32	CCapSetInfo::GetLprMaxRecoveryPackets()
{
	return g_lprCapSet[0].maxRecoveryPackets;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
APIU32	CCapSetInfo::GetLprMaxPacketSize()
{
	return g_lprCapSet[0].maxPacketSize;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// global functions

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Description: Compare 2 strings of caps.
//             For example - 1) "eG722_64kCapCode", "g722_64k", "g722-64k" are all the same.
//                           2) "h263", "eH263CapCode", "h263VideoCapability" are also the same.
//---------------------------------------------------------------------------------------------------
BYTE IsMatchingCapName(const char * str1,const char * str2)
{
	BYTE bRes = FALSE;
	if (str1 && str2)
	{
		int totalLength1 = strlen(str1);
		int totalLength2 = strlen(str2);

		if ((totalLength1 >= 4) && (totalLength2 >= 4))
		{
			char * newstr1 = new char[totalLength1+1];
			char * newstr2 = new char[totalLength2+1];

			strncpy(newstr1,str1, totalLength1+1);
			strncpy(newstr2,str2, totalLength2+1);
			newstr1[totalLength1] = '\0';
			newstr2[totalLength2] = '\0';

			char * strCapCode1 = strstr(newstr1,"CapCode");
			char * strCapCode2 = strstr(newstr2,"CapCode");

			if (strCapCode1)
				*strCapCode1 = '\0';
			if (strCapCode2)
				*strCapCode2 = '\0';

			char * strMinus1 = strchr(newstr1,'-');
			char * strMinus2 = strchr(newstr2,'-');

			if (strMinus1)
				*strMinus1 = '_';
			if (strMinus2)
				*strMinus2 = '_';

			bRes = (strstr(newstr1,newstr2)||strstr(newstr2,newstr1));

			PDELETEA(newstr1);
			PDELETEA(newstr2);
		}
	}
	return bRes;
}


