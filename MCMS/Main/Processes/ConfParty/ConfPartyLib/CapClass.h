//+========================================================================+
//                            CAPCLASS.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CAPCLASS.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Atara                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |09/07/02     |                                                     |
//+========================================================================+
#ifndef _CAPCLASS
#define _CAPCLASS

//#include  "NStream.h"
#include  "PObject.h"
#include  "ChannelParams.h"
#include  "CapInfo.h"
#include  "ObjString.h"
#include "Macros.h"
#include "SecondaryParameters.h"
#include "VideoOperationPoint.h"
#include "VideoOperationPointsSet.h"
#include "ScpNotificationWrapper.h"

class CVidModeH323;
class CHighestCommonCondition;
class CComModeH323;

// country code & manufacturer code for NonStandard cap
#define NS_T35COUNTRY_CODE_USA      0xB5
#define NS_T35EXTENSION_USA         0x00
#define NS_MANUFACTURER_POLYCOM     0x2331
#define NS_MANUFACTURER_PICTURETEL  0x0001
#define NS_CAP_ACCORD_SENDER        0x6C
#define NS_T35COUNTRY_CODE_NORWAY   0x82
#define NS_T35EXTENSION_NORWAY      0x01
#define NS_MANUFACTURER_NORWAY		0x100
#define NO_NS_T35COUNTRY			0x0
#define NO_NS_T35EXTENSION			0x0
#define NO_NS_MANUFACTURER			0x0

#define  NS_CAP_H323_H263_ANNEX_I	    0x40 //according to a mail from Dave Hein: "The code 0x40 is used to signal Annex I at all resolutions"
#define  NS_CAP_H323_H263_QCIF_ANNEX_I  0x40
#define  NS_CAP_H323_H263_CIF_ANNEX_I   0x41
#define  NS_CAP_H323_H263_4CIF_ANNEX_I  0x42
#define  NS_CAP_H323_H263_QCIF_ANNEX_T  0x44
#define  NS_CAP_H323_H263_CIF_ANNEX_T   0x45
#define  NS_CAP_H323_H263_4CIF_ANNEX_T  0x46
#define  NS_CAP_H323_HIGH_CAPACITY      0x54
#define  NS_CAP_H323_VISUAL_CONCERT_PC  0x5A
#define  NS_CAP_H323_VGA_800X600        0x61
#define  NS_CAP_H323_VGA_1024X768       0x63
#define  NS_CAP_H323_VGA_1280X1024      0x67
#define  NS_CAP_H323_VISUAL_CONCERT_FX  0x72
#define  NS_CAP_H323_VIDEO_STREAMS_2    0x73


//Definitions for dump functions:
#define	CapClassMsgBufSize	 8192
#define	MaxSizeForOneCapDump 2000 /*Max size for one dump, including the strings and the
//spaces(I found that: sizeof(genericCapStruct) = 1448 - This is th biggest one
					   Dump for CBaseCap = 277
					   => MaxSizeForOneCapDump > 1448+277 = 1725
*/

#define RTV_MAX_RATE_HD   15000
#define RTV_MAX_RATE_VGA  6000
#define RTV_MAX_RATE_CIF  2500
#define RTV_MAX_RATE_QCIF 1800


// Function's result
typedef enum
{
	kFailure = 0,
	kSuccess = 1

} EResult;

// Operators
inline EResult& operator&=(EResult& left,const EResult& right)
{
	int iLeft = left, iRight = right;
	iLeft &= iRight;
	left = (EResult)iLeft;
	return left;
}

inline EResult& operator|=(EResult& left,const EResult& right)
{
	int iLeft = left, iRight = right;
	iLeft |= iRight;
	left = (EResult)iLeft;
	return left;
}

inline annexesListEn operator++(annexesListEn& e, int)
{
	annexesListEn eTemp = e;
	e = (annexesListEn)(e+1);
	return eTemp;
}

inline annexesListEn operator--(annexesListEn& e, int)
{
	annexesListEn eTemp = e;
	e = (annexesListEn)(e-1);
	return eTemp;
}
// End of operators

#define CAP_CAST(capType) (capType *)(m_pCapStruct)
#define CUSTOM_FORMATS_ON_MASK    0x000003ff //bits 22-31
#define CUSTOM_FORMATS_OFF_MASK ~(0x000003ff)//bits 22-31
#define WITHOUT_ANNEXF_MASK ~(0x00000000 | typeAnnexF)// annex F bit set to NO
//#define WITH_ANNEXF_MASK ~(0x00000000 & typeAnnexF)// annex F bit set to NO

#define WIDTH  0
#define HEIGHT 1

#define CASCADE_BW_FACTOR 94

//  Comparison
//--------------
// Values To Compare
// Use one or a combination of those values
// in the comparison function.
// (The format includes the custom formats)

typedef enum
{
	kBitRate   					= 0x00000001,
	kFormat    					= 0x00000002,
	kFrameRate 					= 0x00000004,
	kAnnexes   					= 0x00000008,
	kRoleLabel 					= 0x00000010,
	kCapCode   					= 0x00000020,
	kH264Level 					= 0x00000040,
	kH264Additional_MBPS 		= 0x00000080,
	kH264Additional_FS 			= 0x00000100,
	kH264Additional_DPB 		= 0x00000200,
	kH264Additional_BR_AND_CPB 	= 0x00000400,
	kH264Additional				= 0x00000780,
	kH264Profile 				= 0x00000800,
	kBitRateForCascade          = 0x00001000,
	kBitRateWithoutTolerance    = 0x00002000,
	kMaxFR                      = 0x00004000,
	kH264Mode                   = 0x00008000,
	kPacketizationMode			= 0x00010000,
	kMixDepth					= 0x00020000, // Amihay: MRM CODE
	kNumOfStreamDesc			= 0x00040000, // Amihay: MRM CODE
	kTransportType				= 0x00080000,
	kCryptoSuit                 = 0x00100000,//sdes crypto
	kMsSvcParams                = 0x00200000

} ECompareValue;


const BYTE kCompareAllValues = 0xFF;

// First 5 bits of the comparison details
// You can add values up to 0x0F

#define	HIGHER_BIT_RATE       		(DWORD)0x00000001
#define	HIGHER_FORMAT         		(DWORD)0x00000002
#define	HIGHER_FRAME_RATE     		(DWORD)0x00000004
#define NO_H263_PLUS          		(DWORD)0x00000008
#define DIFFERENT_ROLE        		(DWORD)0x00000010 // for people/content
#define DIFFERENT_CAPCODE     		(DWORD)0x00000020
//parameters for H264:
#define HIGHER_LEVEL	      		(DWORD)0x00000040
#define HIGHER_MBPS	          		(DWORD)0x00000080
#define HIGHER_FS	          		(DWORD)0x00000100
#define HIGHER_DPB	          		(DWORD)0x00000200
#define HIGHER_BR_AND_CPB     		(DWORD)0x00000400
#define LOWER_FRAME_RATE      		(DWORD)0x00000800 // should be removed
#define DIFFERENT_PARAMETERS  		(DWORD)0x00001000 // should be removed
#define DIFFERENT_PROFILE     		(DWORD)0x00002000
#define DIFFERENT_H264MODE    		(DWORD)0x00004000
#define HIGHER_MAXFR          		(DWORD)0x00008000
#define DIFF_PACKETIZATION_MODE  	(DWORD)0x00010000
#define DIFFERENT_TRANSPORT_TYPE	(DWORD)0x00080000
#define DIFFERENT_CRYPTO_SUIT	(DWORD)0x00100000//crypto

//#define AUDIO_MIX_DEPTH_DEFAULT 3
#define RMX_AUDIO_MIX_DEPTH_DEFAULT 5
#define SFTMCU_AUDIO_MIX_DEPTH_DEFAULT 3
#define HIGHER_MIX_DEPTH  			(WORD)0x4000 // Amihay: MRM CODE

#define AUDIO_MAX_SEND_SSRC_DEFAULT	AUDIO_MIX_DEPTH_DEFAULT
#define AUDIO_MAX_SEND_SSRC_LINK	1
#define AUDIO_MAX_RECV_SSRC_LINK	1


//#define EMPTY_DETAIL      (WORD)0x000C
//#define EMPTY_DETAIL      (WORD)0x000D
//#define EMPTY_DETAIL      (WORD)0x000E
//#define EMPTY_DETAIL      (WORD)0x000F


#define DETAILS_WITHOUT_FORMAT  (WORD)0xFF0F // The 4 bits: 4-7 of the comparison details are the video format
#define ANNEXES_DETAILS (WORD)0x1F00 // The 5 bits: 8-12 of the comparison details are the annexes
									// I want to change it to the last 5 bit (11-15)

// Polycom H26L
#define Polycom_H26L		  (BYTE)0x9E
#define Polycom_H26L_Profile  (BYTE)0x01
#define H26L_ONDATA_LOCATION		  0
#define H26L_PROFILE_ONDATA_LOCATION  0
#define H26L_MPI_ONDATA_LOCATION	  1
#define H26L_DUAL_MPI_ONDATA_LOCATION 2

// Bfcp defines:
#define BFCP_MAX_CONFID_LENGTH  32
#define BFCP_MAX_USERID_LENGTH  32
#define BFCP_MAX_FLOORID_LENGTH 32
#define BFCP_MAX_STREAM_LENGTH  32


// Extern global variables
extern const int    g_formatDimensions[kLastFormat][2];

// Global functions

// The 4 bits: 4-7 of the comparison details are the video format (EFormat) that the values HIGHER_FORMAT, HIGHER_FRAME_RATE and annexes relate to.
inline WORD ShiftVideoFormat(EFormat eFormat) {return eFormat<<4;}
inline EFormat GetVideoFormatFromDetails(DWORD details){return (EFormat)(details>>4);}

// The 5 bits: 8-12 of the comparison details are the annexes
inline WORD ShiftAnnexDetails(annexesListEn annex) {return ((WORD)annex)<<8;}
inline annexesListEn GetAnnexFromDetails(DWORD details) {return (annexesListEn)(details>>8);}

EFormat CalculateFormat(WORD xWidth, WORD yHeight);
//EFormat GetFormat(const CCapSetH263 & capSet);

void DumpDetailsToStream(cmCapDataType eType, DWORD details, CObjString& msg);
// check if we have only 2000 or less bytes free in the alloc string (need to be flushed).
//inline BYTE CheckMsgSize(COstrStream& msg) {return(msg.pcount() > (CapClassMsgBufSize-MaxSizeForOneCapDump));}
//void FlushMessage(std::ostream & msg);


// End of global


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Pure virtual

class CBaseCap: public CPObject
{
	CLASS_TYPE_1(CBaseCap, CPObject)
public:

    // Constructors
	CBaseCap(BaseCapStruct *pCapStruct = NULL):m_pCapStruct(pCapStruct){}
	virtual ~CBaseCap(){}

    // Operations
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople) {return kFailure;}
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual void    GetMediaParams(CSecondaryParams &secParams, DWORD details = 0) const;
	virtual void    GetDiffFromDetails(DWORD details, CSecondaryParams &secParams);
	virtual size_t  SizeOf() const = 0;
	virtual void    AllocStruct(size_t size = 0) = 0;
	virtual void    FreeStruct() {PDELETEA(m_pCapStruct);}
	virtual void    SetStruct (BaseCapStruct* pCapStruct) {m_pCapStruct = pCapStruct;}
	virtual void    Dump(std::ostream& msg) const;
	virtual void    Dump(CObjString* pMsg) const;
	virtual void	Dump(const char* title, WORD level) const;
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
	virtual BYTE    IsSameProfile(const CBaseCap& other) const {return TRUE;}
	virtual BYTE	IsEquals(const CBaseCap& other, DWORD valuesToCompare) const;
	virtual BYTE	CheckValidationAndSetValidValuesIfNeeded(BYTE valuesToCheck,BYTE bSetValidValueInstead=NO,const CBaseCap *pDefaultParams=NULL) {return YES;}
	virtual CBaseCap* GetHighestCommon(const CBaseCap & otherCap) const {return NULL;}	// alloc memory
	cmCapDataType   GetType() const;
	ERoleLabel       GetRole() const;
	cmCapDirection  GetDirection() const;
	BYTE            IsType(cmCapDataType eType) const {return (GetType() == eType);}
	BYTE            IsDirection(cmCapDirection eDirection) const;
	BaseCapStruct*  GetStruct() const {return m_pCapStruct;}
	EResult         IsStructValid()  const {return (m_pCapStruct != NULL) ? kSuccess : kFailure ;}
	virtual APIS16  GetFrameRate(EFormat eFormat = kUnknownFormat) const {return kFailure;}
	EResult         SetRole(ERoleLabel eRole);
	EResult			SetDirection(cmCapDirection eDirection);
	EResult			SwitchDirection();
	CapEnum         GetCapCode() const;
	EResult         SetCapCode(CapEnum eCapCode);
	virtual APIS32	GetBitRate() const {return 0;}
	virtual EFormat GetFormat() const {return kUnknownFormat;}
	virtual APIS8   GetFormatMpi(EFormat eFormat) const {return 0;}
	virtual int     GetFramePerPacket() const {return NA;}
	virtual void	SetAdditionalXmlInfo()	{return;}
	virtual BYTE	IsControlCap() {return FALSE;}

	EResult SetStruct(cmCapDataType  eType,
		cmCapDirection eDirection,
		ERoleLabel     eRole         = kRolePeople);

	capBuffer* GetAsCapBuffer() const; // alloc memory
	operator BYTE*() {return (BYTE*)m_pCapStruct;}

	//Static
	static CBaseCap* AllocNewCap(BYTE *pStruct) {return AllocNewCap(eUnknownAlgorithemCapCode,pStruct);}
	static CBaseCap* AllocNewCap(CapEnum newCapCode, void *pStruct, size_t size = 0);

	// Amihay: MRM CODE
	virtual APIU16  GetProfile() const {return 0;}

	virtual bool IsCapContainsStreamsGroup() const {return false;}
	virtual bool AddRecvStream(APIU32* aSsrcId,int num, bool isUpdate = false) {return false;}
	virtual bool AddSendStream(APIU32* aSsrcId,int num, bool isUpdate = false) {return false;}

	virtual STREAM_GROUP_S* GetRecvStreamsGroup() const {return NULL;}
	virtual STREAM_GROUP_S* GetSendStreamsGroup() const {return NULL;}
	virtual VIDEO_OPERATION_POINT_SET_S* GetOperationPoints() const {return NULL;}

	virtual bool SetRecvStreamsGroup(const STREAM_GROUP_S &rStreamGroup) {return false;}
	virtual bool SetSendStreamsGroup(const STREAM_GROUP_S &rStreamGroup) {return false;}

	virtual void Dump(std::stringstream& msgStr, bool dumpStreams = true) const
	{
		CObjString objString;
		Dump(&objString);
		msgStr << objString;
	}

protected:

    // Operations
	EResult SetHeader(cmCapDataType eType,cmCapDirection eDirection, ERoleLabel eRole);
	void    AllocStructBySize(size_t size);

	// Class members
	BaseCapStruct *m_pCapStruct;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CBaseAudioCap: public CBaseCap
{
CLASS_TYPE_1(CBaseAudioCap, CBaseCap)
public:
    // Constructors
	CBaseAudioCap(audioCapStructBase *pAudioCapStruct = NULL):CBaseCap((BaseCapStruct *)pAudioCapStruct){}
	virtual ~CBaseAudioCap(){}

    // Operations
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
    virtual const char*   NameOf() const {return "CBaseAudioCap";}
	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual void    Dump(std::ostream& msg) const;
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
	virtual BYTE	CheckValidationAndSetValidValuesIfNeeded(BYTE valuesToCheck,BYTE bSetValidValueInstead=NO,const CBaseCap *pDefaultParams=NULL);
	virtual EResult SetMaxFramePerPacket(int newValue);
	virtual EResult SetMinFramePerPacket(int newValue);
	virtual int     GetMaxFramePerPacket() const;
	virtual int     GetMinFramePerPacket() const;
	virtual int     GetFramePerPacket() const {return GetMaxFramePerPacket();}
	virtual APIS16  GetFrameRate(EFormat eFormat = kUnknownFormat) const {return GetFramePerPacket();}
	virtual APIS8   GetFormatMpi(EFormat eFormat) const {return GetFramePerPacket();}
	virtual APIS32	GetBitRate() const;
	virtual EResult SetBitRate(APIS32 rate);
	virtual APIS32  GetRtcpMask() const;
	virtual EResult SetRtcpMask(APIS32 rtcpfeedback);

	virtual EResult SetStruct(cmCapDirection eDirection = cmCapReceive,int maxValue = 60,int minValue = 0);
	virtual CBaseCap* GetHighestCommon(const CBaseCap & otherCap) const;
	virtual BYTE    IsSameProfile(const CBaseCap& other) const {return TRUE;}
	EResult SetRoleLabelStruct(BYTE label,cmCapDirection eDirection = cmCapReceive);
	BYTE    GetLabelFromRoleLabelStruct() const;
	EResult SetContentProfile(int profile,cmCapDirection eDirection = cmCapReceive);
	int     GetContentProfile() const;
	EResult SetH239ControlCap(cmCapDirection eDirection);
    EResult SetDynamicPTRepControl(cmCapDirection eDirection);
    BYTE    IsSirenLPRMatch(const CBaseCap& other, DWORD *pDetails) const;
    BYTE 	IsG719Match(const CBaseCap& other, DWORD *pDetails) const;
    BYTE	IsiLBCMatch(const CBaseCap& other, DWORD *pDetails) const;
    BYTE	IsOpusMatch(const CBaseCap& other, DWORD *pDetails) const;

    //Amihay:MRM CODE
	virtual APIU32  GetMixDepth() const {return 1;}
//	virtual APIU32  GetMaxRecvSsrc() const {return 1;}
//	virtual APIU32  GetMaxSendSsrc() const {return 1;}

protected:

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class COpusAudioCap: public CBaseAudioCap
{
CLASS_TYPE_1(COpusAudioCap,CBaseAudioCap )
public:
    // Constructors
	COpusAudioCap(opus_CapStruct *pOpusAudioCapStruct = NULL):CBaseAudioCap((audioCapStructBase *)pOpusAudioCapStruct){/*SetMaxAverageBitRateDefault();*/}


	virtual ~COpusAudioCap(){}

	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual void    Dump(std::ostream& msg) const;
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
	virtual APIS32	GetBitRate() const;
	virtual EResult SetBitRate(APIS32 rate);

protected:
	void SetMaxAverageBitRateDefault();
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CG7221CAudioCap: public CBaseAudioCap
{
CLASS_TYPE_1(CG7221CAudioCap,CBaseAudioCap )
public:
    // Constructors
	CG7221CAudioCap(g7221C_CapStruct *pG7221CAudioCapStruct = NULL):CBaseAudioCap((audioCapStructBase *)pG7221CAudioCapStruct){}
	virtual ~CG7221CAudioCap(){}

	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual void    Dump(std::ostream& msg) const;
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
	virtual APIS32	GetBitRate() const;

	BYTE isCapEnumSupported(CapEnum IpCapEnum)const;
	BYTE isRateSupported(DWORD audioRate) const;

protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SirenLPRAudioCap: public CBaseAudioCap
{
CLASS_TYPE_1(SirenLPRAudioCap,CBaseAudioCap )
public:
    // Constructors
	SirenLPRAudioCap(sirenLPR_CapStruct *pSirenLPRAudioCapStruct = NULL):CBaseAudioCap((audioCapStructBase *)pSirenLPRAudioCapStruct){}


	virtual ~SirenLPRAudioCap(){}

	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual void    Dump(std::ostream& msg) const;
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
	virtual APIS32	GetSirenLPRMask() const;
	virtual EResult	SetSirenLPRMask(int mask);

protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CG7231AudioCap: public CBaseAudioCap
{
CLASS_TYPE_1(CG7231AudioCap,CBaseAudioCap )
public:
    // Constructors
	CG7231AudioCap(g7231CapStruct *pG7231AudioCapStruct = NULL):CBaseAudioCap((audioCapStructBase *)pG7231AudioCapStruct){}
	virtual ~CG7231AudioCap(){}

    // Operations
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
    virtual const char*   NameOf() const {return "CG7231AudioCap";}
	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual void    Dump(std::ostream& msg) const;
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
	virtual EResult SetMaxFramePerPacket(int newValue);
	virtual EResult SetMinFramePerPacket(int newValue);
	virtual int     GetMaxFramePerPacket() const;
	virtual int     GetMinFramePerPacket() const;
	virtual APIS16  GetFrameRate(EFormat eFormat = kUnknownFormat) const {return GetFramePerPacket();}
	virtual EResult SetStruct(cmCapDirection eDirection = cmCapReceive,int maxValue = 1,int minValue = 0);

protected:

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//LYNC2013_FEC_RED:
class CRedCap: public CBaseAudioCap
{
	CLASS_TYPE_1(CRedCap, CBaseAudioCap)
public:
	// Constructors
	CRedCap(redCapStruct *pRedCapStruct = NULL):CBaseAudioCap((audioCapStructBase*)pRedCapStruct){}
	virtual ~CRedCap(){}

	// Operations
	virtual const char*   NameOf() const {return "CRedCap";}
	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	void Dump(std::ostream& msg) const;
	EResult SetStruct(cmCapDataType eType,cmCapDirection eDirection);

	// default implementations for pure functions of CBaseVideoCap:
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
	virtual EResult SetStruct(cmCapDirection eDirection = cmCapReceive,int maxValue = 0,int minValue = 0);

protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Pure virtual

class CBaseVideoCap: public CBaseCap
{
CLASS_TYPE_1(CBaseVideoCap, CBaseCap)
public:
	// Constructors
	CBaseVideoCap(BaseCapStruct *pVideoCapStruct = NULL):CBaseCap(pVideoCapStruct){}
	virtual ~CBaseVideoCap(){}

    // Operations
	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection
		,BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE) = 0;
	virtual EResult SetBitRate(APIS32 rate) = 0;
	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi) = 0;
	virtual EFormat GetFormat() const = 0;
	virtual BYTE    IsFormat(EFormat eFormat) const = 0;
	virtual APIS8   GetFormatMpi(EFormat eFormat) const = 0;
	virtual APIS32  GetBitRate() const = 0;
	virtual APIS16  GetFrameRate(EFormat eFormat = kUnknownFormat) const;
	virtual void	GetMediaParams(CSecondaryParams &secParams, DWORD details = 0) const;
	virtual void    AddLowerResolutionsIfNeeded() {}
	virtual void	NullifySqcifMpi() {}
	virtual BYTE	CheckValidationAndSetValidValuesIfNeeded(BYTE valuesToCheck,BYTE bSetValidValueInstead=NO,const CBaseCap *pDefaultParams=NULL);
	virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppTempCap, BYTE comparePacketizationMode = FALSE) const = 0;
	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const = 0;
	virtual BYTE 	IsCapableOfHD1080() const = 0;
	virtual BYTE 	IsCapableOfHD1080At60()const = 0;
	virtual BYTE 	IsCapableOfHD720At50()const = 0;
	virtual BYTE    IsCapableOfHD720At30()const = 0;
	virtual eVideoPartyType GetCPVideoPartyType() const = 0;
	virtual BYTE    IsThisCapBetter(EFormat* maxFormat, BYTE* bMaxHasAnnex) const;
	virtual BYTE    IsThisCapEqual(EFormat maxFormat) const;
	virtual CBaseCap* GetHighestCommon(const CBaseCap & otherCap) const;	// alloc memory
//	virtual BYTE    IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition) const {return FALSE;}
    virtual BYTE    IsInterlaced(EFormat format = kUnknownFormat) const {return FALSE;}
	virtual CBaseVideoCap* CreateIntersectBetweenTwoVidCaps(CBaseVideoCap* pOtherCap, cmCapDirection direction, BYTE bIntersectRate, BYTE comparePacketizationMode = TRUE);
	virtual EResult CopyBaseQualities(const CBaseVideoCap & otherCap) = 0;
	virtual size_t  SizeOfBaseStruct() const = 0;
	virtual EResult SetHighestCapForVswFromScmAndCardValues(){return kFailure;}
	virtual EResult SetHighestCapForCpFromScmAndCardValues(DWORD videoRate, eVideoQuality videoQuality){return kFailure;}
	virtual EResult SetRtvCapForCpFromH264VideoType(Eh264VideoModeType H264VideoModeType,int videoLineRate,BYTE maxResolution){return kFailure;}
    virtual BYTE    checkIsh263preffered(BYTE bIsCp, eVideoQuality vidQuality);
    APIS32 GetToleraceRatePct(APIS32 maxBitRate) const;
    virtual BYTE    IsSameProfile(const CBaseCap& other) const {return TRUE;}

    virtual BYTE    IsSupportPLI() const {return FALSE;}
    virtual BYTE    IsSupportFIR() const {return FALSE;}
    virtual BYTE    IsSupportTMMBR() const {return FALSE;}
    virtual BYTE    IsSupportNonStandardEncode() const {return FALSE;}
    virtual APIS32  GetRtcpFeedbackMask() const {return 0;}
    virtual EResult SetRtcpFeedbackMask(APIS32 rtcpFbMask) {return kFailure;}

    virtual EFormat GetFormatAccordingToFS(APIS32 fs) const;

protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CH261VideoCap: public CBaseVideoCap
{
CLASS_TYPE_1(CH261VideoCap, CBaseVideoCap)
public:
	// Constructors
	CH261VideoCap(h261CapStruct *pH261VideoCapStruct = NULL):CBaseVideoCap((BaseCapStruct *)pH261VideoCapStruct){}
	virtual ~CH261VideoCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CH261VideoCap";}
	virtual size_t  SizeOf() const;
	virtual size_t  SizeOfBaseStruct() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080 = FALSE, BYTE HDMpi = 0,BOOL isHighProfile = FALSE);
	virtual EResult SetBitRate(APIS32 rate);
	virtual EResult SetBitRateWithoutLimitation(APIS32 rate);
	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi);
	virtual void    SetStandardFormatsMpi(APIS8 qcifMpi, APIS8 cifMpi);
	virtual void    Dump(std::ostream& msg)   const;
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const { return FALSE;};
	virtual BYTE 	IsCapableOfHD1080() const { return FALSE;}
	virtual BYTE 	IsCapableOfHD1080At60() const { return FALSE;}
	virtual BYTE 	IsCapableOfHD720At50() const { return FALSE;};
	virtual BYTE    IsCapableOfHD720At30() const { return FALSE;};
    virtual eVideoPartyType GetCPVideoPartyType() const;
	virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppTempCap, BYTE comparePacketizationMode = FALSE) const;
	virtual EFormat GetFormat() const;
	virtual BYTE    IsFormat(EFormat eFormat) const;
	virtual APIS8   GetFormatMpi(EFormat eFormat) const;
	virtual APIS32  GetBitRate() const;
	virtual void    AddLowerResolutionsIfNeeded();
//	virtual BYTE    IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition) const;

	EResult SetStillImageTransmission(BYTE bStillImageTransmission);
	virtual EResult CopyBaseQualities(const CBaseVideoCap & otherCap);
	virtual EResult SetHighestCapForCpFromScmAndCardValues(DWORD videoRate, eVideoQuality videoQuality);

    virtual BYTE    IsSupportPLI() const;
    virtual BYTE    IsSupportFIR() const;
    virtual BYTE    IsSupportTMMBR() const;
    virtual BYTE    IsSupportNonStandardEncode() const;
    virtual APIS32  GetRtcpFeedbackMask() const;
    virtual EResult SetRtcpFeedbackMask(APIS32 rtcpFbMask);


protected:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CH263VideoCap: public CBaseVideoCap
{
CLASS_TYPE_1(CH263VideoCap, CBaseVideoCap)
public:
    // Constructors
	CH263VideoCap(h263CapStruct *pH263VideoCapStruct = NULL):CBaseVideoCap((BaseCapStruct *)pH263VideoCapStruct){}
	virtual ~CH263VideoCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CH263VideoCap";}
	virtual size_t  SizeOf() const;
	virtual size_t  SizeOfBaseStruct() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive,ERoleLabel eRole = kRolePeople);
	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE);
	virtual EResult SetBitRate(APIS32 rate);
	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi);
	virtual void    SetStandardFormatsMpi(APIS8 qcifMpi, APIS8 cifMpi, APIS8 cif4Mpi, APIS8 cif16Mpi);
    EResult  CopyStandardFormatsMpi(CH263VideoCap* pOther);
    EResult SetH263Plus(BYTE bAnnexF, BYTE bAnnexT, BYTE bAnnexN, BYTE bAnnexI_NS,
						 APIS8 vgaMpi, APIS8 ntscMpi, APIS8 svgaMpi, APIS8 xgaMpi, APIS8 qntscMpi);
    EResult SetH263Interlaced(EFormat interlacedResolution, APIS8 qcifMpi, APIS8 cifMpi);
	virtual EResult SetHighestCapForVswFromScmAndCardValues();
	virtual EResult SetHighestCapForCpFromScmAndCardValues(DWORD videoRateIn100Bits, eVideoQuality videoQuality);
	virtual void	NullifySqcifMpi();
	virtual BYTE	IsOnlySqcifMpi() const;
	virtual void    Dump(std::ostream& msg) const;
    void    DumpAnnexesNamesToStream(CObjString& msg, APIU32 hcAnnexes) const;
	annexesListEn   GetAnnex(APIU32 hcAnnexes) const;
	virtual DWORD   GetAnnexes() const;
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
	virtual EFormat GetFormat() const;
	virtual BYTE    IsFormat(EFormat eFormat) const;
	virtual APIS8   GetFormatMpi(EFormat eFormat) const;
	virtual APIS32  GetBitRate() const;
	virtual void    AddLowerResolutionsIfNeeded();
	virtual BYTE	IsThisCapBetter(EFormat* maxFormat, BYTE* bMaxHasAnnex) const;
//	virtual BYTE    IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition) const;
    virtual BYTE    IsInterlaced(EFormat format = kUnknownFormat) const;
	virtual void	SetAdditionalXmlInfo();
    virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const { return FALSE;};
    virtual BYTE 	IsCapableOfHD1080() const { return FALSE;};
	virtual BYTE 	IsCapableOfHD1080At60() const { return FALSE;}
    virtual BYTE 	IsCapableOfHD720At50() const { return FALSE;};
    virtual BYTE    IsCapableOfHD720At30() const { return FALSE;};
    virtual eVideoPartyType GetCPVideoPartyType() const;



	int GetSizeOfCustoms(char *pStruct);
	int GetSizeOfAnnexes(char *pStruct, BYTE **ppAnnexPtr);
	EResult AddAnnexToCap(annexesListEn eAnnex);

	BYTE    IsAnnex(annexesListEn eAnnex) const;
	BYTE    IsCustomFormat(EFormat eFormat) const;
	EFormat GetCustomFormat() const;
	APIU8   GetCustomFormatMpi() const;
	APIU8   GetCustomFormatMpi(EFormat customFormat) const;
	BYTE    IsSizeLargerThanH263DefaultSize() const {return IsH263Plus();}
	BYTE    IsH263Plus(BYTE isWithoutAnnexF = FALSE) const;
	BYTE    IsAnnexes() const;
	BYTE    IsCustomFormats() const;
	WORD     IsErrorCompensation() const;
    BYTE    GetAnnexesForFormat(EFormat eFormat, BYTE& bIsAnnexF, BYTE& bIsAnnexT, BYTE& bIsAnnexN) const;
	BYTE    Intersection(const CBaseCap& other, BYTE **ppTempCap, BYTE comparePacketizationMode = FALSE) const;
	void    ImproveCap(h263CapStruct *pTemp263) const;
	void    AddOneCapToOther(const CBaseCap& other, h263CapStruct *pTemp263) const;
	void	IntersectionAnnexesAndCustomFormats(const CBaseCap& other, h263CapStruct** ppIntersectStruct) const;
	void    NullifyArray(customPicFormatSt intersectionCustomFormat[]) const;
	EResult AddAnnex(annexes_fd_set *pTempMask,annexesListEn eAnnex) const;
	void	SaveCustom(int numOfCustomFormats,customPicFormatSt intersectionCustomFormat[],EFormat customFormat,const CH263VideoCap *pOtherCap) const;
	void    AddCustomAndAnnex(h263CapStruct	**ppCapStruct,annexes_fd_set tempMask,int numOfCustomFormats,customPicFormatSt intersectionCustomFormat[]);
	EResult RemoveAnAnnexFromMask(annexesListEn eAnnex);
	EResult RemoveTheSingleAnnexFromMask(annexesListEn eAnnex);
	EResult SetAnAnnexInMask(annexesListEn eAnnex);
	EResult CopyBaseQualities(const CBaseVideoCap & otherCap);
	int     GetNumOfAnnexes()const;
	int     GetNumOfCustomFormats() const;

    virtual BYTE    IsSupportPLI() const;
    virtual BYTE    IsSupportFIR() const;
    virtual BYTE    IsSupportTMMBR() const;
    virtual BYTE    IsSupportNonStandardEncode() const;
    virtual APIS32  GetRtcpFeedbackMask() const;
    virtual EResult SetRtcpFeedbackMask(APIS32 rtcpFbMask);

protected:

    // Operations
	EResult SetAnnexesStructs();
	EResult SetAnAnnexStruct(annexesListEn eAnnex, int annexIndex);
	EResult SetCustomFormatsDefaults();
	EResult SetCustomFormatsAmount(int amount);
	EResult SetASpecificCustomFormatStruct(int customFormatIndex,
										   APIU16 maxCustomPictureWidth,
										   APIU16 maxCustomPictureHeight,
										   APIU16 minCustomPictureWidth,APIU16 minCustomPictureHeight,
										   APIU8 standardMPI,
										   APIU8 pixelAspectCode[],
										   int length = 1,
										   APIU16 clockConversionCode = 0,
										   APIU8 clockDivisor = 0,
										   APIU16 customMPI = 0);

	EResult SetASpecificCustomFormatStruct(int customFormatIndex,
									       EFormat eFormat,
										   APIU8 standardMPI,
										   APIU8 pixelAspectCode[],
										   int length = 1,
										   APIU16 clockConversionCode = 0,
										   APIU8 clockDivisor = 0,
										   APIU16 customMPI = 0);
	EResult SetASpecificCustomFormatDimensions(int customFormatIndex,EFormat eFormat);
	EResult SetNumOfCustomFormatsInMask(DWORD num);
	h263CapStruct* ReallocIfNeeded(BYTE reallocAnyWay = FALSE);
    void    DumpAnnexesDetailsToStream(std::ostream& msg) const;
    void    DumpAnAnnexDetailsToStream(annexesListEn eAnnex, int annexIndex, std::ostream& msg) const;
    void    DumpCustomFormatsToStream(std::ostream& msg) const;
    int		GetAnnexIndex(annexesListEn annex) const;
    annexesListEn GetAnnexEnAccordingToIndex(int i)const;
	h263OptionsStruct* GetAPointerToASpecificAnnex(int annexIndex) const;
	customPic_St*	   GetAPointerToCustomFormats() const;
	customPicFormatSt* GetAPointerToASpecificCustomFormat(int customFormatIndex) const;
	customPicFormatSt* GetAPointerToASpecificCustomFormat(EFormat eFormat) const;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CMsSvcVideoCap: public CBaseVideoCap
{
CLASS_TYPE_1(CMsSvcVideoCap, CBaseVideoCap)
public:
	// Constructors
	CMsSvcVideoCap(msSvcCapStruct* pMsSvcCapStruct = NULL):CBaseVideoCap((BaseCapStruct *)pMsSvcCapStruct){}


	virtual ~CMsSvcVideoCap(){}

    // Operations
	virtual void    Dump(std::ostream& msg)   const;
    virtual const char*   NameOf() const {return "CMsSvcVideoCap";}
	virtual size_t  SizeOf() const;
	virtual size_t  SizeOfBaseStruct() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual BYTE IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD* pDetails) const;
	virtual BYTE Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode) const;
	virtual EResult SetRtcpFeedbackMask(APIS32 rtcpFbMask);
    
	virtual void SetWidth(APIS32 width);
	virtual APIS32 GetWidth();   
	
	virtual void SetHeight(APIS32 height);
	virtual APIS32 GetHeight();
	
	virtual void SetAspectRatio(APIS32 aspectRatio);
	virtual APIS32 GetAspectRatio();
	
	virtual void SetMaxFrameRate(APIS32 maxFrameRate);
	virtual APIS32 GetMaxFrameRate() const;
	virtual APIS16 GetFrameRate(EFormat eFormat = kUnknownFormat) const {return GetMaxFrameRate();}

	virtual APIS32 GetMaxPixel() const;
	virtual void SetMaxPixelNum(APIS32 maxPixelNum);

	virtual void SetMaxBitRate(APIS32 maxBitRate);
	virtual APIS32 GetMaxBitRate();
	

	 virtual APIS32  GetRtcpFeedbackMask() const;
	
	virtual void SetPacketizationMode(APIS8 packetizationMode);
	virtual APIS8 GetPacketizationMode();
	
	virtual void SetFiller(APIS8 filler);
	virtual APIS8 GetFiller();

	virtual APIS8   GetFormatMpi(EFormat eFormat) const {return kUnknownFormat;};
	virtual APIS32  GetBitRate() const;
	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection, BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE) {return kFailure;};
	virtual BYTE    IsFormat(EFormat eFormat) const {return FALSE;}//there is no format in 264
	virtual EFormat GetFormat() const {return k1080p;};
	virtual EResult SetBitRate(APIS32 rate);//tbd-temp!!!
	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080() const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080At60() const { return FALSE;}
	virtual BYTE 	IsCapableOfHD720At50() const {return FALSE;};
	virtual BYTE 	IsCapableOfHD720At30() const {return FALSE;};
	virtual eVideoPartyType GetCPVideoPartyType() const;//tbd!!!
	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi) {return kFailure;}//there is no mpi in 264
	virtual EResult CopyBaseQualities(const CBaseVideoCap & otherCap);
	eVideoPartyType	GetVideoPartyTypeMBPSandFS(DWORD staticMB) const;
	virtual EResult SetMinBitRate(APIS32 mibBitRate);
	virtual APIS32 GetMinBitRate();
	void SetMsSvcCapForCpFromMsSvcVideoType(MsSvcVideoModeDetails& MsSvcDetails); 
	void GetMbpsAndFsAsDevision(APIS32 &DevMbps,APIS32 &DevFs) const;



//--------------------------------------------------------------------------------------------



};
class CH264VideoCap: public CBaseVideoCap
{
CLASS_TYPE_1(CH264VideoCap, CBaseVideoCap)
public:
	// Constructors
	CH264VideoCap(h264CapStruct* pH264VideoCapStruct = NULL):CBaseVideoCap((BaseCapStruct *)pH264VideoCapStruct){}
	virtual ~CH264VideoCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CH264VideoCap";}
	virtual size_t  SizeOf() const;
	virtual size_t  SizeOfBaseStruct() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
	virtual EResult CopyQualities(const CBaseCap & otherCap);

	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE);
	virtual EResult SetHDContent(ERoleLabel eRole,EHDResolution eHDRes, cmCapDirection eDirection, BYTE HDMpi, BOOL isHighProfile = FALSE);
	virtual EResult SetTIPContent(ERoleLabel eRole, cmCapDirection eDirection,BYTE bSet264ModeAsTipContent = TRUE);

	virtual EResult SetBitRate(APIS32 rate);
	virtual EResult SetMaxFR(APIS32 maxfr);
	virtual EResult SetH264mode(APIS32 h264mode);
	virtual void    Dump(std::ostream& msg)    const;
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
	virtual BYTE    IsSameProfile(const CBaseCap& other) const;
	virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode = TRUE) const;
	virtual BYTE	IntersectH264Profile(h264CapStruct *pThisCapStruct, h264CapStruct *pOtherCapStruct, CH264VideoCap *pIntersectH264Cap) const;
	virtual APIU32  IntersectCustomParam(BYTE customType, APIU8 intersectLevel, APIU8 firstLevel, APIU8 secondLevel, APIS32 firstParam, APIS32 secondParam) const;
	virtual CVidModeH323* CreateVidModeThatSupportedByMcu() const;

	virtual void    GetMediaParams(CSecondaryParams &secParams, DWORD details = 0) const;
	virtual void    GetDiffFromDetails(DWORD details, CSecondaryParams &secParams);
    virtual ESecondaryParam GetH264DiffFromDetails(DWORD details, DWORD& problemValue) const;

	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi) {return kFailure;}//there is no mpi in 264
	virtual APIS8   GetFormatMpi(EFormat eFormat) const;//remove this function in version 2!!!!!
	//APIS8   GetH264Mpi() const;//remove this function in version 2!!!!! (vngfe5040):
	APIS8   GetH264Mpi(const APIS32 MaxFSForFrameRate = 0x7FFFFFFF) const;//remove this function in version 2!!!!!
	void    GetExactFS(APIS32& fs) const;
	APIS8   RoundFrameRateAndTranslateToMpi(APIS16 frameRate) const;



	virtual EFormat GetFormat() const;// {return kUnknownFormat;}//there is no format in 264
	virtual BYTE    IsFormat(EFormat eFormat) const {return FALSE;}//there is no format in 264

	virtual APIS32  GetBitRate() const;
	virtual APIU8   GetLevel() const;
	virtual APIS32	GetMbps() const;
	virtual	APIS32  GetFs() const;
	virtual APIS32	GetDpb() const;
	virtual APIS32  GetBrAndCpb() const;
	virtual APIS32  GetStaticMB() const;
	virtual APIS32	GetSampleAspectRatio() const;
	virtual APIS16  GetFrameRate(EFormat eFormat = kUnknownFormat) const {return GetMbps()/GetFs();}

	virtual void    SetProfile(APIU16 profile);
	virtual void    SetLevel(APIU8 level);
	virtual void	SetMbps(APIS32 mbps);
	virtual	void    SetFs(APIS32 fs);
	virtual void 	SetDpb(APIS32 dpb);
	virtual void    SetBrAndCpb(APIS32 brAndCpb);
	virtual void    SetSampleAspectRatio(APIS32 sar);
	virtual void    SetStaticMB(APIS32 staticMB);
	virtual void    GetAdditionals(APIS32& mbps, APIS32& fs, APIS32& dpb ,APIS32& brAndCpb, APIS32& sar, APIS32& staticMB);
	virtual void    GetAdditionalsAsExplicit(APIS32& mbps, APIS32& fs, APIS32& dpb ,APIS32& brAndCpb, APIS32& sar, APIS32& staticMB);
   	virtual void    GetAdditionalsMbpsFsAsExplicit(DWORD& mbps, DWORD& fs);
    virtual void    SetLevelAndAdditionals(APIU16 profile, APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb ,APIS32 brAndCpb, APIS32 sar, APIS32 staticMB) ;
    virtual void 	SetAccordingToOperationPoint(const VideoOperationPoint &operationPoint, bool shouldUpdateProfile = true);
    virtual void	InitAccordingToOperationPoint(const VideoOperationPoint &operationPoint, bool shouldUpdateProfile = true);

	virtual void    RemoveDefaultAdditionals();

	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const;
	virtual BYTE 	IsCapableOfHD1080() const;
	virtual BYTE 	IsCapableOfHD1080At60() const;
	virtual BYTE 	IsCapableOfHD720At50() const;
	virtual BYTE    IsCapableOfHD720At30() const;
	virtual eVideoPartyType GetCPVideoPartyType() const;
	eVideoPartyType	GetVideoPartyTypeMBPSandFS(DWORD staticMB,BYTE IsRsrcByFs = 0) const;
	BYTE  IsThisCapBetter(APIU16* maxProfile, APIU8* maxLevel, APIS32* maxMbps, APIS32* maxFs, APIS32* maxDpb, APIS32* maxBrAndCpb, BYTE bIgnoreHighProfileCap) const;
//	virtual BYTE    IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition) const;
	virtual APIS32 GetMaxFR() const;
	virtual APIS32 GetH264mode() const;
	EResult CopyBaseQualities(const CBaseVideoCap & otherCap);
	APIU8 GetPacketizationMode() const;
	void SetPacketizationMode(APIU8);
	eVideoPartyType GetTipVideoPartyTypeAccordingToVideoRate(DWORD videoBitRate) const;

    virtual BYTE    IsSupportPLI() const;
    virtual BYTE    IsSupportFIR() const;
    virtual BYTE    IsSupportTMMBR() const;
    virtual BYTE    IsSupportNonStandardEncode() const;
    virtual APIS32  GetRtcpFeedbackMask() const;
    virtual EResult SetRtcpFeedbackMask(APIS32 rtcpFbMask);


	//Amihay: MRM CODE
	virtual APIU16  GetProfile() const;
	static bool IsH264Video(CapEnum capCode);

protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//VP8 Video Codec //N.A. DEBUG VP8
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CVP8VideoCap: public CBaseVideoCap
{
CLASS_TYPE_1(CVP8VideoCap, CBaseVideoCap)
public:
	// Constructors
	CVP8VideoCap(vp8CapStruct* pVP8VideoCapStruct = NULL):CBaseVideoCap((BaseCapStruct *)pVP8VideoCapStruct){}
	virtual ~CVP8VideoCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CVP8VideoCap";}
	virtual size_t  SizeOf() const;

	virtual void    AllocStruct(size_t size = 0);
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
	virtual EResult CopyQualities(const CBaseCap & otherCap);

	virtual void    Dump(std::ostream& msg)    const;

	//Sets
	virtual EResult SetMaxFR(APIS32 maxfr);
	virtual EResult SetMaxFS(APIS32 maxfs);
	virtual EResult SetBitRate(APIS32 rate);
	virtual EResult SetRtcpFeedbackMask(APIS32 rtcpFbMask);

	//Gets
	virtual APIS32 GetMaxFR() const;
	virtual APIS32 GetMaxFS() const;
	virtual APIS32 GetMaxBitRate() const;
	virtual APIS32 GetBitRate() const;
	virtual APIS32 GetRtcpFeedbackMask() const;

	//pure virtual functions
	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE);
	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi) {return kSuccess;};
	virtual EFormat GetFormat() const {return k720p;};
	virtual BYTE    IsFormat(EFormat eFormat) const {return 0;};
	virtual APIS8   GetFormatMpi(EFormat eFormat) const;
	virtual eVideoPartyType GetCPVideoPartyType() const ;
	eVideoPartyType	GetVideoPartyTypeFRandFS(DWORD staticMB,BYTE IsRsrcByFs = 0) const;

	BYTE IsSupportPLI() const;
	BYTE IsSupportFIR() const;
	BYTE IsSupportTMMBR() const;



	virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppTempCap, BYTE comparePacketizationMode = FALSE) const;
	APIU32 IntersectVP8CustomParam(BYTE customType, APIS32 firstParam, APIS32 secondParam) const;
	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const {return 0;};
	virtual BYTE 	IsCapableOfHD1080() const {return 0;};
	virtual BYTE 	IsCapableOfHD1080At60()const {return 0;};
	virtual BYTE 	IsCapableOfHD720At50()const {return 0;};
	virtual BYTE    IsCapableOfHD720At30()const {return 0;};

	virtual EResult CopyBaseQualities(const CBaseVideoCap & otherCap) {return kSuccess;};
	virtual size_t  SizeOfBaseStruct() const {return 0;};
	void SetVP8CapForCpFromVP8VideoType(VP8VideoModeDetails& Vp8Details);

	//removed these fields for no
	//virtual EResult SetVP8Width(APIS32 width);
	//virtual EResult SetVP8Height(APIS32 height);
	//virtual EResult SetAspectRatio(APIS32 aspectRatio);
	//virtual APIS32 GetHeight() const;
	//virtual APIS32 GetWidth() const;
	//virtual APIS32 GetAspectRatio() const;




protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CGenericVideoCap: public CBaseVideoCap
{
CLASS_TYPE_1(CGenericVideoCap, CBaseVideoCap)
public:
	// Constructors
	CGenericVideoCap(genericVideoCapStruct *pGenericVideoCapStruct = NULL):CBaseVideoCap((BaseCapStruct *)pGenericVideoCapStruct){}
	virtual ~CGenericVideoCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CGenericVideoCap";}
	virtual size_t  SizeOf() const;
	virtual size_t  SizeOfBaseStruct() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
    virtual EResult SetDropFieldCap(cmCapDirection eDirection = cmCapReceive);
	virtual BYTE    IsDropField() const;
	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080() const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080At60() const { return FALSE;}
	virtual BYTE 	IsCapableOfHD720At50() const {return FALSE;};
	virtual BYTE 	IsCapableOfHD720At30() const {return FALSE;};
	virtual eVideoPartyType GetCPVideoPartyType() const{return eVideo_party_type_none;};
	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection, BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE) {return kFailure;};
	virtual EResult SetBitRate(APIS32 rate);
	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi) {return kFailure;};
	virtual void    Dump(std::ostream& msg) const;
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const {return FALSE;};
	virtual EFormat GetFormat() const {return kUnknownFormat;};
	virtual BYTE    IsFormat(EFormat eFormat) const;
	virtual APIS8   GetFormatMpi(EFormat eFormat) const {return kUnknownFormat;};
	virtual APIS32  GetBitRate() const;
	virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppTempCap, BYTE comparePacketizationMode) const {return FALSE;} // not implemented
	EResult CopyBaseQualities(const CBaseVideoCap & otherCap);

protected:
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CDBC2VideoCap: public CGenericVideoCap
{
	// in the data field of DBC2 at the generic video structure we should initiate only the first byte.
//  DBC2 capability parameters :
// ==============================

//  |8|7  |6            |5            |4                    |3|2|1     |
//	| |Res|motionVectors|canInterleave|requiresEncapsulation|maxOverlap|
//  |0|0  |0            |0            |1                    |0|1|0     |

public:
	// Constructors
	CDBC2VideoCap(genericVideoCapStruct *pDBC2VideoCapStruct = NULL):CGenericVideoCap(pDBC2VideoCapStruct){}
	virtual ~CDBC2VideoCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CDBC2VideoCap";}
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
	virtual void    Dump(std::ostream& msg) const;

protected:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CBaseDataCap: public CBaseCap
{
CLASS_TYPE_1(CBaseDataCap,CBaseCap )
public:
	// Constructors
	CBaseDataCap(dataCapStructBase *pDataCapStruct = NULL):CBaseCap((BaseCapStruct *)pDataCapStruct){}
	virtual ~CBaseDataCap(){}

    // Operations
	virtual void    AllocStruct(size_t size = 0);
    virtual const char*   NameOf() const {return "CBaseDataCap";}
	virtual size_t SizeOf() const;
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual EResult SetBitRate(APIS32 rate);
	virtual APIS32  GetBitRate() const;
	virtual CBaseCap* GetHighestCommon(const CBaseCap & otherCap) const;   //alloc memory
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
	virtual void    Dump(std::ostream& msg) const;

	//Amihay: MRM CODE
	virtual bool IsSupportScp()	const;
	virtual void SetIsSupportScp(BYTE yesNo);
protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CT120DataCap: public CBaseDataCap
{
CLASS_TYPE_1(CT120DataCap, CBaseDataCap)
public:
	// Constructors
	CT120DataCap(dataCapStructBase *pT120DataCapStruct = NULL):CBaseDataCap((dataCapStructBase *)pT120DataCapStruct){}
	virtual ~CT120DataCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CT120DataCap";}
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare,DWORD *pDetails) const;

protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CH224DataCap: public CBaseDataCap
{
CLASS_TYPE_1(CH224DataCap, CBaseDataCap)
public:
	// Constructors
	CH224DataCap(dataCapStructBase *pH224DataCapStruct = NULL):CBaseDataCap((dataCapStructBase *)pH224DataCapStruct){}
	virtual ~CH224DataCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CH224DataCap";}
	EResult SetFromIpScm(const CComModeH323* pScm);
protected:
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CNonStandardCap: public CBaseCap
{
CLASS_TYPE_1(CNonStandardCap,CBaseCap )
public:
    // Constructors
	CNonStandardCap(ctNonStandardCapStruct *pNonStandardCapStruct = NULL):CBaseCap((BaseCapStruct *)pNonStandardCapStruct){}
	virtual ~CNonStandardCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CNonStandardCap";}
	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual void    Dump(std::ostream& msg) const;
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;

	APIU8   GetT35CountryCode() const;
	APIU8   GetT35Extension()   const;
	APIU16  GetManufacturerCode() const;
	char    GetData(int i) const;
	BYTE    IsData(BYTE data) const;
    BYTE    IsNonStandardAnnex(annexesListEn eAnnex) const;
	EResult SetDuoNS();
	EResult SetAnnexI_NsStruct();

	virtual EResult SetStruct(APIU8 t35CountryCode,APIU8 t35Extension,APIU16 manufacturerCode,char data[],int dataLength);

protected:

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CGenericCap: public CBaseCap
{
CLASS_TYPE_1(CGenericCap, CBaseCap)
public:
    // Constructors
	CGenericCap(genericCapStruct *pGenericCapStruct = NULL):CBaseCap((BaseCapStruct *)pGenericCapStruct){}
	virtual ~CGenericCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CGenericCap";}
	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual void    Dump(std::ostream& msg) const {CBaseCap::Dump(msg);}
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
//	virtual BYTE	IsControlCap() {return TRUE;}

protected:

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CEncryptionCap: public CBaseCap
{
CLASS_TYPE_1(CEncryptionCap, CBaseCap)
public:
    // Constructors
	CEncryptionCap(encryptionCapStruct *pEncCapStruct = NULL):CBaseCap((BaseCapStruct *)pEncCapStruct){}
	virtual ~CEncryptionCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CEncryptionCap";}
	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	virtual void    Dump(std::ostream& msg) const;
	virtual EResult SetStruct(EenMediaType encAlg,APIU16 entry);
	APIU16 GetEntry() const;
//	virtual BYTE	IsControlCap() {return TRUE;}

};

class CLprCap: public CBaseVideoCap
{
CLASS_TYPE_1(CLprCap, CBaseVideoCap)
public:
	// Constructors
	CLprCap(lprCapStruct *pLprCapStruct = NULL):CBaseVideoCap((BaseCapStruct *)pLprCapStruct){}
	virtual ~CLprCap(){}

    // Operations
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
	EResult SetDefaultsLpr(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople, cmCapDataType eType = cmCapGeneric);
    const char*   NameOf() const {return "CLprCap";}
    virtual size_t  SizeOf() const;
	void    AllocStruct(size_t size = 0);
	void    Dump(std::ostream& msg) const;
	EResult CopyQualities(const CBaseCap & otherCap);
	virtual APIS32	GetBitRate() const {return 0;}
	virtual EFormat GetFormat() const {return kUnknownFormat;}
	virtual APIS8   GetFormatMpi(EFormat eFormat) const {return 0;}
	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection = cmCapReceive,BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE){return kFailure;};
	virtual EResult SetBitRate(APIS32 rate){return kFailure;};
	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi) {return kFailure;};
	virtual BYTE    IsFormat(EFormat eFormat) const {return NO;};
	virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppTempCap, BYTE comparePacketizationMode) const {return FALSE;}; // not implemented
	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080() const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080At60() const { return FALSE;}
	virtual BYTE 	IsCapableOfHD720At50() const {return FALSE;};
	virtual BYTE    IsCapableOfHD720At30() const { return FALSE;};
	virtual eVideoPartyType GetCPVideoPartyType() const{return eVideo_party_type_none;};
	virtual size_t  SizeOfBaseStruct() const;
	EResult CopyBaseQualities(const CBaseVideoCap & otherCap);

	APIU32	GetLprVersionID() const;
	APIU32	GetLprMinProtectionPeriod() const;
	APIU32	GetLprMaxProtectionPeriod() const;
	APIU32	GetLprMaxRecoverySet() const;
	APIU32	GetLprMaxRecoveryPackets() const;
	APIU32	GetLprMaxPacketSize() const;
	void	SetLprVersionID(APIU32 versionId);
	void	SetLprMinProtectionPeriod(APIU32 minProtectionPeriod);
	void	SetLprMaxProtectionPeriod(APIU32 maxProtectionPeriod);
	void	SetLprMaxRecoverySet(APIU32 maxRecoverySet);
	void	SetLprMaxRecoveryPackets(APIU32 maxRecoveryPackets);
	void	SetLprMaxPacketSize(APIU32 maxPacketSize);
//	virtual BYTE	IsControlCap() {return TRUE;}

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//LYNC2013_FEC_RED:
class CFecCap: public CBaseVideoCap
{
CLASS_TYPE_1(CFecCap, CBaseVideoCap)
public:
	// Constructors
	CFecCap(fecCapStruct *pFecCapStruct = NULL):CBaseVideoCap((fecCapStruct *)pFecCapStruct){}
	virtual ~CFecCap(){}

	// Operations
	virtual const char*   NameOf() const {return "CFecCap";}
	virtual size_t  SizeOf() const;
	virtual void    AllocStruct(size_t size = 0);
	void Dump(std::ostream& msg) const;
	//EResult SetStruct(cmCapDataType eType,cmCapDirection eDirection);

	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
	EResult SetDefaultsFec(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople, cmCapDataType eType = cmCapGeneric);

	// default implementations for pure functions of CBaseVideoCap:
	virtual EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection
			,BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE) {return kFailure;};
	virtual EResult SetBitRate(APIS32 rate) {return kFailure;};
	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi) {return kFailure;};
	virtual EFormat GetFormat() const {return kUnknownFormat;};
	virtual BYTE    IsFormat(EFormat eFormat) const {return NO;};
	virtual APIS8   GetFormatMpi(EFormat eFormat) const {return 0;};
	virtual APIS32  GetBitRate() const {return 0;};
	virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppTempCap, BYTE comparePacketizationMode = FALSE) const {return FALSE;}; // not implemented
	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080() const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080At60()const {return FALSE;};
	virtual BYTE 	IsCapableOfHD720At50()const {return FALSE;};
	virtual BYTE    IsCapableOfHD720At30()const {return FALSE;};
	virtual eVideoPartyType GetCPVideoPartyType() const {return eVideo_party_type_none;};

	virtual EResult CopyBaseQualities(const CBaseVideoCap & otherCap);
	virtual size_t  SizeOfBaseStruct() const;

protected:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CSdesCap: public CBaseCap
{
CLASS_TYPE_1(CSdesCap, CBaseCap)
public:
	// Constructors
	CSdesCap(sdesCapStruct *pSdesCapStruct = NULL):CBaseCap((BaseCapStruct *)pSdesCapStruct){}
	virtual ~CSdesCap(){}

    // Operations
	EResult SetDefaults(cmCapDataType eType, cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
	EResult SetStruct(cmCapDataType eType, cmCapDirection eDirection, ERoleLabel eRole);
    const char*   NameOf() const {return "CSdesCap";}
	size_t  SizeOf() const;
	size_t SizeOfBaseStruct() const;
	void    AllocStruct(size_t size = 0);
	void    Dump(std::ostream& msg) const;
	sdesCapStruct* ReallocIfNeeded(BYTE reallocAnyWay = FALSE);
	EResult CopyQualities(const CBaseCap & otherCap);
	virtual BYTE IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;

	APIU32 GetSdesTag() const;
	APIU16 GetSdesCryptoSuite() const;
	APIU32 GetSdesNumOfKeysParam() const;
	BOOL GetIsSdesUnencryptedSrtp() const;
	BOOL GetIsSdesUnencryptedSrtcp() const;
	BOOL GetIsSdesUnauthenticatedSrtp() const;
	BOOL GetIsSdesKdrInUse() const;
	APIU8 GetSdesKdr() const;
	BOOL GetIsSdesWshInUse() const;
	APIU16 GetSdesWsh() const;
	BOOL GetIsSdesFecOrderInUse() const;
	APIU16 GetSdesFecOrder() const;
	BOOL GetIsSdesFecKeyInUse() const;
	APIU16 GetSdesKeyMethod(int keyNumber) const;
	char* GetSdesBase64KeySalt(int keyNumber) const;
	BOOL GetIsSdesLifeTimeInUse(int keyNumber) const;
	APIU32 GetSdesLifeTime(int keyNumber) const;
	BOOL GetIsSdesMkiInUse(int keyNumber) const;
	APIU8 GetSdesMkiValue(int keyNumber) const;
	BOOL GetIsSdesMkiValueLenInUse(int keyNumber) const;
	APIU8 GetSdesMkiValueLen(int keyNumber) const;

	void SetSdesXmlDynamicProps();
	void SetSdesTag(APIU32 sdesTag);
	void SetSdesCryptoSuite(APIU16 cryptoSuite);
	void SetNumOfKeysParam(APIU32 numKeyParams);
	void SetIsSdesUnencryptedSrtp(BOOL bIsSdesUnencryptedSrtp);
	void SetIsSdesUnencryptedSrtcp(BOOL bIsSdesUnencryptedSrtcp);
	void SetIsSdesUnauthenticatedSrtp(BOOL bIsSdesUnauthenticatedSrtp);
	void SetIsSdesKdrInUse(BOOL bIsKdrInUse);
	void SetSdesKdr(APIU8 sdesKdr);
	void SetIsSdesWshInUse(BOOL bIsWshInUse);
	void SetSdesWsh(APIU16 sdesWsh);
	void SetIsSdesFecOrderInUse(BOOL bIsFecOrderInUse);
	void SetSdesFecOrder(APIU16 sdesFecOrder);
	void SetIsSdesFecKeyInUse(BOOL bIsFecKeyInUse);
	void SetSdesKeyMethod(int keyNumber, APIU16 keyMethod);
	void SetSdesBase64KeySalt(int keyNumber, char* pBase64MasterSalt);
	void SetIsSdesLifeTimeInUse(int keyNumber, BOOL bIsLifeTimeInUse);
	void SetSdesLifeTime(int keyNumber,APIU32 lifetime);
	void SetIsSdesMkiInUse(int keyNumber, BOOL bIsMkiInUse);
	void SetSdesMkiValue(int keyNumber, APIU8 mkiValue);
	void SetIsSdesMkiValueLenInUse(int keyNumber, BOOL bIsMkiValueLenInUse);
	void SetSdesMkiValueLen(int keyNumber, APIU8 mkiValueLen);
	virtual BYTE	IsControlCap() {return TRUE;}
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//_dtls_
class CDtlsCap: public CSdesCap
{
CLASS_TYPE_1(CDtlsCap, CSdesCap)
public:
	// Constructors
	CDtlsCap(sdesCapStruct *pSdesCapStruct = NULL):CSdesCap(pSdesCapStruct){}
	virtual ~CDtlsCap(){}

    // Operations
    const char*   NameOf() const {return "CDtlsCap";}
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CICEPwdCap: public CBaseCap
{
CLASS_TYPE_1(CICEPwdCap, CBaseCap)
public:
	// Constructors
	CICEPwdCap(icePwdCapStruct *pIcePwdCapStruct = NULL):CBaseCap((BaseCapStruct *)pIcePwdCapStruct){}
	virtual ~CICEPwdCap(){}

    // Operations
	 virtual const char*   NameOf() const {return "CICEPwdCap";}
	 virtual size_t  SizeOf() const;
	 virtual void    AllocStruct(size_t size = 0);
	 void Dump(std::ostream& msg) const;
	 EResult SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr);
	 virtual BYTE	IsControlCap() {return TRUE;}

protected:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CICEUfragCap: public CBaseCap
{
CLASS_TYPE_1(CICEUfragCap, CBaseCap)
public:
	// Constructors
	CICEUfragCap(iceUfragCapStruct *pIceUfragCapStruct = NULL):CBaseCap((BaseCapStruct *)pIceUfragCapStruct){}
	virtual ~CICEUfragCap(){}

    // Operations
	 virtual const char*   NameOf() const {return "CICEUfragCap";}
	 virtual size_t  SizeOf() const;
	 virtual void    AllocStruct(size_t size = 0);
	 void Dump(std::ostream& msg) const;
	 EResult SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr);
	 virtual BYTE	IsControlCap() {return TRUE;}

protected:
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CICECandidateCap: public CBaseCap
{
CLASS_TYPE_1(CICECandidateCap, CBaseCap)
public:
	// Constructors
	CICECandidateCap(iceCandidateCapStruct *pIceCandidateCapStruct = NULL):CBaseCap((BaseCapStruct *)pIceCandidateCapStruct){}
	virtual ~CICECandidateCap(){}

    // Operations
	 virtual const char*   NameOf() const {return "CICECandidateCap";}
	 virtual size_t  SizeOf() const;
	 virtual void    AllocStruct(size_t size = 0);
	 void Dump(std::ostream& msg) const;
	 EResult SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr, APIS8 candidateType);
	 virtual BYTE	IsControlCap() {return TRUE;}

protected:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CICERemoteCandidateCap: public CBaseCap
{
CLASS_TYPE_1(iceRemoteCandidateCapStruct, CBaseCap)
public:
	// Constructors
CICERemoteCandidateCap(iceRemoteCandidateCapStruct *pIceRmtCandidateCapStruct = NULL):CBaseCap((BaseCapStruct *)pIceRmtCandidateCapStruct){}
	virtual ~CICERemoteCandidateCap(){}

    // Operations
	 virtual const char*   NameOf() const {return "CICERemoteCandidateCap";}
	 virtual size_t  SizeOf() const;
	 virtual void    AllocStruct(size_t size = 0);
	 void Dump(std::ostream& msg) const;
	 EResult SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr);
	 virtual BYTE	IsControlCap() {return TRUE;}

protected:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CRtcpCap: public CBaseCap
{
CLASS_TYPE_1(CRtcpCap, CBaseCap)
public:
	// Constructors
	CRtcpCap(rtcpCapStruct *pRtcpCapStruct = NULL):CBaseCap((BaseCapStruct *)pRtcpCapStruct){}
	virtual ~CRtcpCap(){}

    // Operations
	 virtual const char*   NameOf() const {return "CRtcpCap";}
	 virtual size_t  SizeOf() const;
	 virtual void    AllocStruct(size_t size = 0);
	 void Dump(std::ostream& msg) const;
	 EResult SetStruct(cmCapDataType eType,cmCapDirection eDirection,char* DataStr);
	 virtual BYTE	IsControlCap() {return TRUE;}

protected:
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CBfcpCap: public CBaseCap
{
CLASS_TYPE_1(CBfcpCap, CBaseCap)
public:
	// Constractors
	CBfcpCap(bfcpCapStruct *pBfcpCapStruct = NULL):CBaseCap((BaseCapStruct *)pBfcpCapStruct){}
	virtual ~CBfcpCap(){}

	// Operations
	EResult 		SetBfcp(enTransportType transType);
	virtual const char*   NameOf() const {return "CBfcpCap";}
	virtual void 	AllocStruct(size_t size = 0);
	virtual size_t  SizeOf() const;
	virtual void    Dump(std::ostream& msg) const;
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceiveAndTransmit,ERoleLabel eRole = kRolePeople);
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;

	void DumpFloorId(std::ostream& msg, bfcpFlooridStruct &hBfcpFlooridStruct) const;
	void SetDefaultsFloorId(bfcpFlooridStruct &hBfcpFlooridStruct);
	void CopyQualitiesFloorId(bfcpFlooridStruct &hBfcpFlooridStruct, bfcpFlooridStruct &hOtherBfcpFlooridStruct);

	void			SetFloorCntl(eBfcpFloorCtrl bfcpFloorCtrl);
	eBfcpFloorCtrl	GetFloorCntl() const;
	void 			SetConfId(char* pConfId);
	void			SetConfId(WORD confId);
	char*			GetConfId() const;
	void			SetUserId(char* pUserId);
	void			SetUserId(WORD userId);
	char*			GetUserId() const;
	void			SetFloorIdParams(int floorIndex, char* pFloorId, char* pStreamId0, char* pStreamId1, char* pStreamId2, char* pStreamId3);
	void			GetFloorIdParams(int floorIndex, char* pFloorId, char* pStreamId0, char* pStreamId1, char* pStreamId2, char* pStreamId3);
	void			SetInfoEnabled(BYTE bInfoEnabled);
	BYTE			GetInfoEnabled() const;
	void			SetInfoTime(APIU16 uInfoTime);
	APIU16			GetInfoTime() const;
	void 			SetSetup(APIU8 setup);
	APIU8 			GetSetup() const;
	void 			SetConnection(APIU8 connection);
	APIU8 			GetConnection() const;
	void 			SetMStreamType(eBfcpMStreamType mstreamType);
	eBfcpMStreamType GetMStreamType() const;

	void 			SetTransportType(enTransportType transType);
	enTransportType GetTransportType() const;
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CRtvVideoCap: public CBaseVideoCap
{
CLASS_TYPE_1(CRtvVideoCap, CBaseVideoCap)
public:
	// Constructors
	CRtvVideoCap(rtvCapStruct* pRtvVideoCapStruct = NULL):CBaseVideoCap((BaseCapStruct *)pRtvVideoCapStruct){}
	virtual ~CRtvVideoCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CRtvVideoCap";}
	virtual size_t  SizeOf() const;
    virtual void    AllocStruct(size_t size = 0);

	virtual size_t  SizeOfBaseStruct() const;

	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
	virtual EResult CopyQualities(const CBaseCap & otherCap);

	EResult SetFpsToAllItems(cmCapDirection eDirection,ERoleLabel eRole, unsigned long FR);//only to be used in vsr case!

	virtual EResult SetBitRate(APIS32 rate);
	virtual void    Dump(std::ostream& msg)    const;

	virtual APIS32  GetBitRate() const;
	virtual EFormat GetFormat() const;
	virtual APIS8   GetFormatMpi(EFormat eFormat) const;
	EResult SetContent(ERoleLabel eRole, cmCapDirection eDirection,BOOL isHD1080 = FALSE, BYTE HDMpi = 0, BOOL isHighProfile = FALSE);

	virtual EResult SetFormatMpi(EFormat eFormat, APIS8 mpi) {return kFailure;}//there is no mpi in RTV
	virtual BYTE    IsFormat(EFormat eFormat) const {return FALSE;}//there is no format in rtv

	virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode) const;

	virtual BYTE 	IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const;
	virtual BYTE 	IsCapableOfHD1080() const {return FALSE;};
	virtual BYTE 	IsCapableOfHD1080At60() const { return FALSE;}
	virtual BYTE    IsCapableOfHD720At30() const { return FALSE;};
	virtual BYTE 	IsCapableOfHD720At50() const {return FALSE;};
	virtual eVideoPartyType GetCPVideoPartyType() const;
	EResult CopyBaseQualities(const CBaseVideoCap & otherCap);

	void SetRtvWidthAndHeight(APIS32 width, APIS32 Height,APIS32 Fps,DWORD BitRate);
	virtual EResult  SetRtvCapForCpFromH264VideoType(Eh264VideoModeType H264VideoModeType,int videoLineRate,BYTE maxResolution);
	BYTE IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
	EResult FindMaxCapInCapSet(rtvCapStruct* pCapStruct,rtvCapItemS& rtvCapItem) const;
	void SetRtvCapForCpFromRtvVideoType(RTVVideoModeDetails rtvVidModeDetails,int videoLineRate);
	EResult SetRtvCap(RTVVideoModeDetails rtvVidModeDetails,int videoLineRate,rtvCapStruct* pCapStruct);
	void GetMbpsAndFsAsDevision(APIS32 &mbps,APIS32 &Fs) const;
	EResult SetRateInMaxCapInCapSet(rtvCapStruct* pCapStruct,APIS32 rate) const;
	eVideoPartyType GetVideoPartyTypeMBPSandFS(DWORD staticMB) const;
	void CopyCapToOtherCap(CRtvVideoCap *pOtherCap);
	EResult GetRtvCap(RTVVideoModeDetails& rtvVidModeDetails,DWORD& BitRate);
	EResult GetRtvCapFRAccordingToFS(DWORD& FrameRate,DWORD FS);
	EResult GetBitRateAccordingToResolution(DWORD width, DWORD height, DWORD& bitRate) const;
	EResult GetBitRateAccordingToResolutionInCapSet(rtvCapStruct* pCapStruct,DWORD width, DWORD hieght, DWORD& bitRate) const;

	void  AddRTVcapsToCapset(Eh264VideoModeType MaxVideoModeType,rtvCapStruct* pCapStruct,int videoLineRate);
	void  AddRTVcapsToCapsetByVideoMode(RTVVideoModeDetails rtvVidModeDetails,int videoLineRate,rtvCapStruct* pCapStruct);
	static eVideoPartyType GetCPRtvResourceVideoPartyTypeByRate(DWORD videoLineRate);
	DWORD GetFrameRateForRTV() const;

    virtual BYTE    IsSupportPLI() const;
    virtual BYTE    IsSupportFIR() const;
    virtual BYTE    IsSupportTMMBR() const;
    virtual BYTE    IsSupportNonStandardEncode() const;
    virtual APIS32  GetRtcpFeedbackMask() const;
    virtual EResult SetRtcpFeedbackMask(APIS32 rtcpFbMask);

};
// TIP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class CAAC_LDCap: public CBaseAudioCap {
	CLASS_TYPE_1(CAAC_LDCap, CBaseAudioCap)
public:
	// Constructors
	CAAC_LDCap(AAC_LDCapStruct *pAAC_LDCapStruct = NULL):CBaseAudioCap((audioCapStructBase *)pAAC_LDCapStruct) {}
	virtual ~CAAC_LDCap() {
	}

	// Operations
	virtual const char* NameOf() const {
		return "CAAC_LDCap";
	}
	virtual void AllocStruct(size_t size = 0);
	virtual size_t SizeOf() const;
	virtual void Dump(std::ostream& msg) const;
	virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,
			ERoleLabel eRole = kRolePeople);
	virtual EResult SetStruct(cmCapDirection eDirection = cmCapReceive,
			int maxValue = 2, int minValue = 0);
	virtual EResult CopyQualities(const CBaseCap & otherCap);
	virtual CBaseCap* GetHighestCommon(const CBaseCap & otherCap) const;
	//	virtual EResult SetMaxFramePerPacket(int newValue);
	//	virtual EResult SetMinFramePerPacket(int newValue);
	//	virtual int     GetMaxFramePerPacket() const;
	//	virtual int     GetMinFramePerPacket() const;
	virtual APIS16 GetFrameRate(EFormat eFormat = kUnknownFormat) const {
		return GetFramePerPacket();
	}
	virtual EResult SetBitRate(APIS32 rate);
	virtual APIS32  GetBitRate() const;

	void SetMimeType(char*);
	char* GetMimeType() const;
	void SetSampleRate(APIU32 sampleRate = 48000);
	APIU32 GetSampleRate() const;
	void SetProfileLevelId(APIU16 profileLevelId = 16);
	APIU16 GetProfileLevelId() const;
	void SetStreamType(WORD streamType = 5);
	APIU16 GetStreamType() const;
	void SetMode(char *psMode);
	char* GetMode() const;
	void SetConfig(char *psMode);
	char* GetConfig() const;
	void SetSizeLength(WORD sizeLength = 13);
	APIU16 GetSizeLength() const;
	void SetIndexLength(WORD indexLength = 3);
	APIU16 GetIndexLength() const;
	void SetIndexDeltaLength(WORD indexDeltaLength = 3);
	APIU16 GetIndexDeltaLength() const;
	void SetConstantDuration(APIU32 constantDuration = 480);
	APIU32 GetConstantDuration() const;
};



/////////////////////////////////////////////////////////////////////////////////////////////////////
// Amihay: MRM CODE

#define MAX_RECV_STREAMS_FOR_VIDEO 3
#define MAX_RESOLUTION_WIDTH  9
#define MAX_RESOLUTION_HEIGHT MAX_RESOLUTION_WIDTH
#define MAX_FRAME_RATE 4

enum eVideoOperationPointSetType
{
	eVideoOperationPointSetTypeUndefined,
	eSimulcastOfResolutions,
	eVideoOperationPointSetTypeMaxNumOfValues
};

enum eChannelType
{
	eChannelTypeUndefined,
	eAudioChannel,
	eVideoChannel,
	eDataChannel,
	eVideoContentChannel,
	eChannelTypeMaxNumOfValues
};

enum eChannelDirection
{
	eChannelDirectionUndefined,
	eIncomingChannel,
	eOutgoingChannel,
	eChannelDirectionMaxNumOfValues
};

struct MbpsFsPair
{
	int MBPS;
	int FS;
	unsigned int level;
	char strLevel[15];
};

struct WidthHeightRateTriad
{
	int width;
	int height;
	int frameRate;
	unsigned int level;
};

struct StreamDesc
{
	StreamDesc();
	StreamDesc(const StreamDesc &other);
	~StreamDesc();
	void InitDefaults();
	void Serialize(WORD format,CSegment& seg);
	void DeSerialize(WORD format,CSegment& seg);
	eChannelType m_type;
	unsigned int m_payloadType;
	bool m_specificSourceSsrc;
	unsigned int m_bitRate;
	unsigned int m_frameRate;
	unsigned int m_height;
	unsigned int m_width;
	unsigned int m_pipeIdSsrc;
	unsigned int m_sourceIdSsrc;
	unsigned int m_priority;
	bool         m_isLegal;
	bool         m_isAvcToSvcVsw;

	CScpPipeWrapper m_scpNotificationParams;
};



class ProfileToLevelTranslator : public CPObject
{
	  CLASS_TYPE_1(ProfileToLevelTranslator, CPObject)
public:
	static unsigned int ConvertResolutionAndRateToLevel(int frame_height, int frame_width, int frame_rate);
	static unsigned int GetHeighestLevel(const CVideoOperationPointsSet* operationPoints);
	static unsigned int GetHeighestBitRate(const CVideoOperationPointsSet* operationPoints);
	static UINT16 GetHeighestProfile(const CVideoOperationPointsSet* operationPoints);
	static UINT16 GetHeighestProfile(UINT16 firstProfile, UINT16 secondProfile);
	static WORD SvcProfileToH264(const WORD aVideoProfile);
	unsigned int ConvertResolutionAndRateToLevelEx(long FS, long MBPS);
    virtual const char* NameOf() const {return  "ProfileToLevelTranslator";}

};


class CSacAudioCap: public CBaseAudioCap
{
CLASS_TYPE_1(CSacAudioCap,CBaseAudioCap )
public:
    // Constructors
    CSacAudioCap(sirenLPR_Scalable_CapStruct *pSirenLPRScalableAudioCapStruct = NULL):CBaseAudioCap((audioCapStructBase *)pSirenLPRScalableAudioCapStruct){}
    virtual ~CSacAudioCap(){}

    virtual size_t  SizeOf() const;
    virtual void    AllocStruct(size_t size = 0);
    virtual void    Dump(std::ostream& msg) const;
    virtual EResult SetDefaults(cmCapDirection eDirection = cmCapTransmit,ERoleLabel eRole = kRolePeople);
	virtual EResult SetStruct(cmCapDirection eDirection = cmCapReceive, int maxValue = 2, int minValue = 0);
    virtual EResult CopyQualities(const CBaseCap & otherCap);
    void ReplaceSendRecvStreams();
    virtual void    SetMixDepth(BYTE aMixDepth);
//    virtual void    SetMaxRecvSsrc(BYTE aMaxRecvSsrc);
//    virtual void    SetMaxSendSsrc(BYTE aMaxSendSsrc);
    virtual bool    SetRecvSsrcId(APIU32 aSsrcId);
    virtual void    SetSampleRate(APIU32 sampleRate);
    virtual APIU32  GetRecvSsrcId() const;
    virtual bool    GetSsrcId(cmCapDirection eDirection, APIU32*& aSsrcId, int* num);
    virtual APIS32  GetMaxValue() const;
    virtual APIS32  GetMinValue() const;
    virtual APIU32  GetSirenLPRMask() const;
    virtual APIU32  GetSampleRate() const;
    virtual APIU32  GetMixDepth() const;
//	virtual APIU32  GetMaxRecvSsrc() const;
//	virtual APIU32  GetMaxSendSsrc() const;
    virtual bool IsCapContainsStreamsGroup() const {return true;}
    virtual STREAM_GROUP_S* GetRecvStreamsGroup() const;
    virtual STREAM_GROUP_S* GetSendStreamsGroup() const;
    virtual bool SetRecvStreamsGroup(const STREAM_GROUP_S &rStreamGroup);
    virtual bool SetSendStreamsGroup(const STREAM_GROUP_S &rStreamGroup);
    bool GetRecvSsrcId(APIU32* aSsrcId);

    virtual bool AddRecvStream(APIU32* aSsrcId,int num, bool isUpdate = false);
    virtual bool AddSendStream(APIU32* aSsrcId,int num, bool isUpdate = false);

    virtual bool    AddRecvStream(APIU32 aSsrcId);
    virtual bool    AddSendStream(APIU32 aSsrcId);
    virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
    virtual CBaseCap* GetHighestCommon(const CBaseCap & otherCap) const;

    static bool IsSacAudio(CapEnum capCode);

protected:
};

class CSvcVideoCap: public CH264VideoCap
{
CLASS_TYPE_1(CSvcVideoCap, CH264VideoCap)
public:
    // Constructors
    CSvcVideoCap(svcCapStruct* pSvcVideoCapStruct = NULL):CH264VideoCap((h264CapStruct *)pSvcVideoCapStruct) {}
    virtual ~CSvcVideoCap(){}

    // Operations
    virtual const char*   NameOf() const {return "CSvcVideoCap";}
    virtual size_t  SizeOf() const;
    virtual size_t  SizeOfBaseStruct() const;
    virtual void    AllocStruct(size_t size = 0);
    virtual EResult SetDefaults(cmCapDirection eDirection = cmCapReceive, ERoleLabel eRole = kRolePeople);
    virtual EResult CopyQualities(const CBaseCap & otherCap);
    void ReplaceSendRecvStreams();
    EResult CopyBaseQualities(const CBaseVideoCap & otherCap);
    virtual void    Dump(std::ostream& msg)    const;
    //virtual void    Dump(std::stringstream& msgStr, bool dumpStreams = true) const;
    virtual void    DumpStreamGroup(std::ostream& msg, STREAM_GROUP_S &rStreamGroup) const;
    virtual VIDEO_OPERATION_POINT_SET_S* GetOperationPoints() const;
    virtual void    SetOperationPoints( const  CVideoOperationPointsSet*);
    virtual int     GetNumberOfOperationPoints() const;
    virtual BYTE    IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD *pDetails) const;
    virtual BYTE    Intersection(const CBaseCap& other, BYTE** ppIntersectData, BYTE comparePacketizationMode = TRUE) const;
    virtual BYTE	IntersectH264Profile(h264CapStruct *pThisCapStruct, h264CapStruct *pOtherCapStruct, CH264VideoCap *pIntersectH264Cap) const;
    bool AddRecvStream(APIU32* aSsrcId,int num,bool isUpdate = false);
    virtual void    SetStreams(const VIDEO_OPERATION_POINT_SET_S &operationPoints);

    bool GetRecvSsrcId(APIU32*& aSsrcId,int* num);
    bool GetSendSsrcId(APIU32*& aSsrcId,int* num);

    virtual bool IsCapContainsStreamsGroup() const {return true;}
    virtual STREAM_GROUP_S* GetRecvStreamsGroup() const;
    virtual STREAM_GROUP_S* GetSendStreamsGroup() const;
    virtual bool SetRecvStreamsGroup(const STREAM_GROUP_S &rStreamGroup);
    virtual bool SetSendStreamsGroup(const STREAM_GROUP_S &rStreamGroup);

    void SetOperationPoints(const VIDEO_OPERATION_POINT_SET_S &operationPoints);

    void IntersectReceiveStreams(svcCapStruct &rLocalCapStruct, STREAM_GROUP_S &rRemoteReceiveStreams) const;
    void IntersectSendStreams(svcCapStruct &rLocalCapStruct, STREAM_GROUP_S &rRemoteSendStreams) const;
    virtual bool IsStreamsEqual(const CBaseCap& other) const;
    bool IsStreamsEqual(STREAM_GROUP_S &streams1, STREAM_GROUP_S &streams2) const;
    // int MatchLayerId(VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int width, unsigned int hieght, unsigned int frameRate) const;
    // bool AddRecvStreamAccordingToLayerId(VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int layerId, unsigned int ssrc) const;
    bool AddRecvStream(unsigned int ssrc, unsigned int frameWidth, unsigned int frameHeight, unsigned int maxFrameRate) const;
    bool AddSendStream(unsigned int ssrc, unsigned int frameWidth, unsigned int frameHeight, unsigned int maxFrameRate) const;
    bool AddRecvStreamAccordingToOperationPoints(VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int ssrc, unsigned int width, unsigned int hieght, unsigned int frameRate) const;
    bool AddSendStreamAccordingToOperationPoints(VIDEO_OPERATION_POINT_SET_S &rOperationPoints, unsigned int ssrc, unsigned int width, unsigned int hieght, unsigned int frameRate) const;

protected:
};


#endif /* _CAPCLASS  */
