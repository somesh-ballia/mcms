//+========================================================================+
//                            CAPINFO.H                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:      CAPINFO.H                                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |09/07/02     |                                                     |
//+========================================================================+
#ifndef _CAPINFO
#define _CAPINFO

#include "PObject.h"
#include "DataTypes.h"
#include "IpCommonDefinitions.h"
#include "IPUtils.h"
#include "Capabilities.h"
#include "IpCommonUtilTrace.h"
#include "ChannelParams.h"
#include "IpChannelParams.h"
#include "VideoDefines.h"
#include "EnumsToStrings.h"

#define MAX_AUDIO_ONLY_CAPS 3
#define MAX_SOFT_CP_CAPS 1

#define _H263_P   (payload_en)340


#define MAX_VIDEO_CAPS 7
#define MAX_EPC_CAPS  16
#define MAX_H239_CAPS 19
#define MAX_MLP_HMLP_CAPS 19
#define MAX_H224_CAPS  2
#define MAX_HMLP_CAPS 8
#define MAX_MLP_CAPS  9
#define MAX_SUPPORTED_AUDIO_CAPS  80
#define MAX_MPI_SUPPORTED_IN_H263 9 //from 320: MPI_1 = 0 until MPI_30 = 8 is 9
//#define MAX_RESERVATION_RATES 21 // defined in the ConfPartySharedDefines
#define MAX_FORMATS_NAMES kLastFormat+1
#define MAX_FRAME_RATE_ROUND_UP    eVideoFrameRateLast+1


#define FIRST_AUDIO_ONLY_CAP (CapEnum)CCapSetInfo::g_audioOnlyCaps[0]
#define FIRST_SOFT_CP_CAP	 (CapEnum)CCapSetInfo::g_softCPCaps[0]

#define FIRST_AUDIO_CAP eG711Alaw64kCapCode
#define FIRST_VIDEO_CAP eH261CapCode
#define FIRST_DATA_CAP  eT120DataCapCode

#define LOWEST_AUDIO_RATE 8*_K_

#define FrameBasedFPP 1
#define NonFrameBasedFPP 20


// LPR
#define	MAX_LPR_CAP	1


typedef struct
{
	WORD  dataType;
	DWORD bitRate;
} TDataCapQuality;

typedef struct
{
	char  amscRateType;
	DWORD bitRate;
} TEpcCapQuality;

typedef struct
{
	char  amcRateType;
	DWORD bitRate;
} TH239CapQuality;

typedef struct
{
	int isdnMpi;
	int ipMpi;
} TH263Mpi;


typedef struct
{
	int reservationRate;
	int ipRate;
} TCallRate;

typedef struct
{
	EFormat eFormat;
	char*   formatName;
}TFormatNames;

typedef struct
{
    eVideoFrameRate  frameRate;
    char*            frameRateText;
}TFrameRate;

typedef enum
{
	eOrderedUnknownFormat = -1,
	eOrderedQCif  = 0,
	eOrderedQVGA,
	eOrderedSIF, //kQNTSC,
	eOrderedCif,
	eOrdered2SIF,
	eOrdered2Cif,
	eOrderedVGA,
	eOrdered4SIF,
	eOrdered525SD,
	eOrderedNTSC,
	eOrdered4Cif,
	eOrdered625SD,
	eOrderedSVGA,
	eOrderedXGA,
	eOrdered720p,
	eOrdered16Cif,
	eOrdered1080p,
	eOrderedLastFormat
} EOrderedFormat;

typedef struct
{
	EFormat     	eFormat;
	EOrderedFormat 	eOrderedFormat;
} TOrderedFormatStruct;

typedef struct
{
	EOrderedFormat 	eOrderedFormat;
	int		width;
	int		height;
}TFormatWidthHeight;

typedef struct
{
    cmCapDataType type;				//all
    BYTE          bSupported;		//all
    CapEnum       IpCapCode;		//all
    payload_en    payloadType;		//all
	size_t        capStructSize;	//all
	APIS32        bitRate;			//audio
	int           framePerPacket;	//audio
	int			  packetQuotient;	//audio
	APIU8         dynamicPayloadType; //all (Static + encrypted)
} TCapQuality;

typedef struct
{
	APIU32				versionID;
	APIU32				minProtectionPeriod;
	APIU32				maxProtectionPeriod;
	APIU32				maxRecoverySet;
	APIU32				maxRecoveryPackets;
	APIU32				maxPacketSize;
} TLprCapSet;

typedef enum{ 								//Capabilities.h
	// video dynamic payload numbers
	eH263PlusDynamicPayload  = 96,
	eH261DynamicPayload = 97,
	eH262DynamicPayload = 98,

	eAnnexQDynamicPayload = 100, // Must be 100 because of Polycom EPs' bug in Sip that always use 100.
	eVP8DynamicPayload = 103,  //N.A. DEBUG VP8
//	eUnusedDynamicPayload = 107,
	eH263DynamicPayload = 108,
	eH264HpDynamicPayload = 109, // High profile
	eH264DynamicPayload = 110, /* H264 base profile with packetization mode set to 1 */
	eGenericVideoDynamicPayload = 111,//eAnnexQDynamicPayload,
	eH264MpDynamicPayload = 112,// TIP Main profile
	eRvFeccDynamicPayload = 113,
	eH264NoPmDynamicPayload = 114, /* H264 base profile with packetization mode set to 0 */
    eSvcDynamicPayload = 115,
	eLprDynamicPayload = 116,
	eT120DynamicPayload = 120,	//124
	eRtvDynamicPayload = 121,
	eMsSvcDynamicPayload = 122,
	eFECDynamicPayload = 123   //LYNC2013_FEC_RED
}DynamicNoneAudioPayloadEnum;// values of dynamic payload type can't be smaller than 96 and bigger than 127!!

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
	eSiren7_16kDynamicPayload 		= 111,
	eIlbc800020DynamicPayload		= 111,
	eIlbc800030DynamicPayload		= 112,
/*	eG729AnnexADynamicPayload 		= 110,
	eG729AnnexBDynamicPayload 		= 111,
	eG729AnnexABDynamicPayload 		= 112,*/
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
    eG722Stereo_128kDynamicPayload              = 127
}DynamicAudioPayloadEnum;// values of dynamic payload type can't be smaller than 96 and bigger than 127!!


class CCapSetInfo : public CPObject
{
CLASS_TYPE_1(CCapSetInfo, CPObject)
public:

    // Constructors
	CCapSetInfo(CapEnum   h323Cap = eUnknownAlgorithemCapCode);
    CCapSetInfo(payload_en payload, APIS32 bitRate);
	CCapSetInfo(const char* strCapName);

    virtual ~CCapSetInfo() {}

	operator CapEnum() const {return g_capQualityTbl[m_index].IpCapCode;}
	operator const char*() const {return ::CapEnumToString((CapEnum)m_index);}

	BYTE operator==(const CCapSetInfo& capInfo) const {return (m_index == capInfo.m_index);}
	CCapSetInfo & operator=(const CCapSetInfo& other);
	CCapSetInfo & operator=(CapEnum h323Cap);

    // Operations
    virtual const char*  NameOf()                    const	{return "CCapSetInfo";}
	cmCapDataType  GetCapType()                const	{return g_capQualityTbl[m_index].type;}
	cmCapDataType  GetSipCapType()                const	;
	CapEnum        GetIpCapCode()            const	{return g_capQualityTbl[m_index].IpCapCode;}
	payload_en     GetPayloadType()            const	{return g_capQualityTbl[m_index].payloadType;}
	const char*    GetH323CapName()            const	{return ::CapEnumToString((CapEnum)m_index);}
	size_t         GetH323CapStructSize()      const	{return g_capQualityTbl[m_index].capStructSize;}
	BYTE           IsUnknownCap()              const	{return IsCapCode(eUnknownAlgorithemCapCode);}
	DWORD          GetBitRate(BYTE* pData = NULL) const;
	BYTE           IsType(cmCapDataType)       const;
	BYTE           IsCapCode(CapEnum)          const;
	BYTE           IsPayloadType(payload_en)   const;
	BYTE           IsSupportedSoftCP()         const;
	int            GetMaxFramePerPacket();
	BYTE           IsSupporedCap();
	BYTE           UpdateFromSystemCfg();
	BYTE           UpdateAccordingToSystemMode();
	void           UpdateAccordingToProductType();

	BYTE		   CheckFramePerPacket(int framePerPacket) const;
	int            GetFramePerPacketQuotient(int framePerPacket) const;
	BYTE		   GetCodecNumberOfChannels();

	void           Dump(std::ostream & msg) const;
	void           Dump(CObjString* pMsg) const;

//	APIS16  GetH320Format(EFormat eFormat) const;

	DWORD GetT120DataBitRate(WORD mlpOrHmlpType) const;
//	DWORD GetH224DataBitRate(WORD dataType) const;
	DWORD GetEpcBitRate(BYTE amscRateType/*, BYTE bIsRestricted=0*/) const;
    DWORD GetH239BitRate(BYTE amcRateType/*, BYTE bIsRestricted=0*/) const;

	char  GetAmscRateType(DWORD videoContentBitRate) const;
	char  GetAmcRateType(DWORD videoContentBitRate) const;

	CapEnum GetNextIpCapCodeWithSamePayloadType() const;
	CapEnum GetPrevIpCapCodeWithSamePayloadType() const;
	void SetNextIpCapCodeWithSamePayloadType();
	void SetPrevIpCapCodeWithSamePayloadType();

	void SetNextType();
	void SetNextSoftCPCap();
	void SetNextAudioOnlyCap();

	int  TranslateIsdnMpiToIp(int isdnMpi) const;
	int  TranslateIpMpiToIsdn(int ipMpi) const;

	static int TranslateReservationRateToIpRate(BYTE reservationRate);

	int  TranslateIpRateToReservationRate(int ipRate) const;

	char* GetFormatStr(EFormat eFormat) const;
	char* GetFormatStrForCDREvent(EFormat eFormat) const;
	char* GetMaxFrameRateRoundUpStrForCDREvent(WORD maxFR) const;
	static EOrderedFormat ConvertFormatToOrderedFormat(EFormat eFormat) ;
	static EFormat ConvertOrderedFormatToFormat(EOrderedFormat eOrderedFormat);
	void  GetResolutionWidthAndHeight(EFormat eFormat, int& width, int& height) const;
	static EFormat GetResolutionFormat(int width, int height);

	APIU8  GetDynamicPayloadType(BYTE content, BYTE bIsHighProfile = FALSE, BYTE packetizationMode = H264_NON_INTERLEAVED_PACKETIZATION_MODE);

	APIU32	GetLprVersionID();
	APIU32	GetLprMinProtectionPeriod();
	APIU32	GetLprMaxProtectionPeriod();
	APIU32	GetLprMaxRecoverySet();
	APIU32	GetLprMaxRecoveryPackets();
	APIU32	GetLprMaxPacketSize();

    // Data global arrays:
	static const APIS16 g_videoFormatTbl[MAX_VIDEO_FORMATS][MAX_VIDEO_CAPS];
	static const TDataCapQuality g_T120dataCapQualityTbl[MAX_MLP_HMLP_CAPS];
//	static const TDataCapQuality g_H224dataCapQualityTbl[MAX_H224_CAPS];
	static const TEpcCapQuality  g_EpcCapQualityTbl[MAX_EPC_CAPS];
	static const TH239CapQuality g_H239CapQualityTbl[MAX_H239_CAPS];
	static const int g_audioOnlyCaps[MAX_AUDIO_ONLY_CAPS];
	static const int g_softCPCaps[MAX_SOFT_CP_CAPS];
	static const TH263Mpi g_h263MpiTbl[MAX_MPI_SUPPORTED_IN_H263];
	static const TCallRate g_callRatesTbl[];
	static const TFormatNames g_formatNames[MAX_FORMATS_NAMES];
	static const TFormatNames g_formatNamesForPartyDisconnectCDREvent[MAX_FORMATS_NAMES];
	static const TFrameRate g_maxFrameRateForPartyDisconnectCDREvent[MAX_FRAME_RATE_ROUND_UP];
	static const TFormatWidthHeight g_videoWidthHeight[MAX_FORMATS_NAMES];
	static const TOrderedFormatStruct g_OrderedFormatTable[MAX_FORMATS_NAMES];

	// LPR
	static const TLprCapSet g_lprCapSet[MAX_LPR_CAP];

	//Amihay: MRM CODE
	const char* GetCapName() const {return CapEnumToString((CapEnum)m_index);}

private:

    // Data global arrays:
    static TCapQuality g_capQualityTbl[MAX_CAP_TYPES];


	// Data members:
	int m_index;
};

#endif /* _CAPINFO  */
