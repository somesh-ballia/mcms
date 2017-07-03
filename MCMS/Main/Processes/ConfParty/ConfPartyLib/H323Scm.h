//+========================================================================+
//                            H323SCM.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323SCM.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 1/02/98    |                                                      |
//+========================================================================+


#ifndef _H323SCM
#define _H323SCM

//#include <stdio.h>
//#include <commdefs.h>

#include "H221.h"
#include "H263.h"
#include "CommConf.h"

#include "CapClass.h"
#include "IpEncryptionDefinitions.h"
#include "COP_Layout_definitions.h"
#include "TipUtils.h"
#include "ConfPartyApiDefines.h"

class CCapH323;
class CSipCaps;

//*********************************
//#include <Ucommode.h>
//#define         SERIALEMBD           (unsigned short)1
//#define         NATIVE               (unsigned short)2
//*********************************

class CSegment;
class COstrStream;


//typedef         unsigned char     BYTE;
//typedef         unsigned short    WORD;

#define		MAX_AUDIO_CAP_LENGTH	300
#define		MAX_VIDEO_CAP_LENGTH	1450 // 450
#define		MAX_MEDIA_CAP_LENGTH	2000 //480
#define 	DEFAULT_LIFE_TIME		0x80000000
#define 	DEFAULT_MKI_VAL_LEN		1
#define 	DEFAULT_CRYPTO_SUITE 	eAes_Cm_128_Hmac_Sha1_80

typedef enum
{
	kUnknownType = 0,
	kVideoSwitch,
	kVSW_Fixed,
	kSoftCp,
	kCp,
	kCpQuad,
	kCop
} EConfType;


//----------------------------------------------------------------------------
	// abstract class
class CMediaModeH323 : public CPObject
{
CLASS_TYPE_1(CMediaModeH323, CPObject)
public:
		// Constructors
	CMediaModeH323();
	virtual ~CMediaModeH323();
	CMediaModeH323(const CMediaModeH323&);

		// Initializations
	void InitAllMediaModeMembers();
	void Create(WORD newType, WORD newDataLength, const BYTE newData[], unsigned char payloadType = _UnKnown);
	void Create(const capBuffer* newMode);
	void Create(const CBaseCap* pCap);

		// Operations
	virtual const char* NameOf() const {return "CMediaModeH323";}
	virtual void  Serialize(WORD format,CSegment& seg) const;
	virtual void  DeSerialize(WORD format,CSegment& seg);
	WORD  GetType(void) const;
	WORD  GetLength(void) const;
	void  CopyData(BYTE data[]) const;
	void  CopyToCapBuffer(capBuffer* pCapBuffer,BYTE bOperUse) const;
	void  CopySdesToCapBuffer(capBuffer* pCapBuffer,BYTE bOperUse) const;
	void  CopyDtlsToCapBuffer(capBuffer* pCapBuffer,BYTE bOperUse) const; //_dtls_
	BYTE  IsValidMode(void) const;
	BYTE  IsModeOff(void) const;
	BYTE  IsMediaOn() const;
    void  SetModeOff();
	BYTE  IsContaining(const CMediaModeH323& other, DWORD valuesToCompare, DWORD* pDetails) const;
	BYTE  IsContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD* pDetails) const;
	BYTE  IsEquals(const CBaseCap& other,DWORD valuesToCompare) const;
	CBaseCap* GetAsCapClass() const;
	void  GetMediaParams(CSecondaryParams &secParams, DWORD details = 0) const;
	void  GetDiffFromDetails(DWORD   details,CSecondaryParams &secParams) const;
	WORD GetH320ModeType(cmCapDataType type) const;
	CMediaModeH323& operator=(const CMediaModeH323 &other);
	void SetMute(BYTE isMuted){ m_isMute = isMuted;}
	BYTE GetMute(){ return m_isMute;}
	BYTE IsRole(ERoleLabel eRole)  const;

	virtual DWORD GetBitRate() const = 0;

	virtual cmCapDataType GetDataType() const = 0;
	void  Dump(std::ostream& msg, const char* title="") const;
	void  DumpStreamsList(std::ostream& msg, const char* title="") const;
	virtual WORD operator==(const CMediaModeH323& other) const = 0;
	BYTE *GetDataCap() const;
	virtual size_t  SizeOf() const = 0;
	void SetSdesDefaultTxData(cmCapDataType eMediaType, BOOL bIsTipMode = FALSE, ERoleLabel eRole = kRolePeople);
	void SetDtlsDefaultTxData(cmCapDataType eMediaType, BOOL bIsTipMode = FALSE, ERoleLabel eRole = kRolePeople);
	void SetTipDefaultsForEncrypt(BOOL& bIsNeedToSendMKI, BOOL& bLifeTimeInUse , BOOL& bFecKeyInUse);
	int GetSha1Type();
	void SetSdesTag(APIU32 tag);
	void SetSdesBase64MasterSaltKey(char *key);
	CSdesCap* GetSdesCap() const;
	CDtlsCap* GetDtlsCap() const;
	void SetSdesCap(CSdesCap* pSdesCap);
	void SetDtlsCap(CDtlsCap* pSdesCap);
	void RemoveSdesCap();
	void RemoveDtlsCap();
	BYTE IsSdesEquals(const CMediaModeH323& other ) const;
	void SetSdesMkiDefaultParams(BYTE bIsNeedToSendMKI);
	void SetSdesLifeTimeDefaultParams(BYTE bLifeTimeInUse);
	void SetSdesFecDefaultParams(BYTE bFecKeyInUse);


	void FillStreamsDescList(payload_en payload_type, STREAM_GROUP_S* pStreamGroup);
	//Amihay: MRM CODE
	virtual void IntersectStreams(const CSipCaps* pCapsSet, cmCapDirection direction, DWORD aPartyId, BYTE bIsMrcSlave);
	virtual void IntersectReceiveStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId, BYTE bIsMrcSlave) = 0;
	virtual void IntersectSendStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId) = 0;
	std::list <StreamDesc>::const_iterator FindStreamDesc(unsigned int SSRC) const;
	virtual void CopyStreamListToStreamGroup(const std::list <StreamDesc>&  rStreams, cmCapDirection direction) = 0;
	ERoleLabel GetRole() const;
	void SetStreamsList(const std::list <StreamDesc>&  rStreams, const DWORD aPartyId);
	void SetPayloadType(unsigned char newPayloadType) {m_payloadType = newPayloadType;}
	unsigned char GetPayloadType() const {return m_payloadType;}
	const std::list <StreamDesc>& GetStreams() const {return m_streams;}
	bool UpdateStreamsScpNotificationParams(CScpNotificationWrapper *pScpNotifyInd);
	void SetIvrSsrc(DWORD ssrc){m_IvrSsrc = ssrc;}
	DWORD GetIvrSsrc()const {return m_IvrSsrc;}


	void ClearStreamsList();
	void AddStreamToStreamsList();

	void CreateStreamsDescList(APIU32 *aSsrcIds, int num);
	void FillStreamsDescPayloadType(payload_en payload_type);
	void FillStreamsDescResolutionAndBitrate(CBaseCap* pNewMediaMode);
	void GetSsrcIds(APIU32*& aSsrcId, int& num);

protected:

		// Attributes
	WORD m_type;
	WORD m_dataLength;
	BYTE* m_dataCap;
	BYTE  m_isMute;
	CSdesCap*	m_pSdesData;
	CDtlsCap*	m_pDtlsData; //_dtls_

		// internal use
	void SetType(WORD newType);
	void SetLength(WORD newDataLength);
	void AllocNewDataCap(WORD newDataLength);
	void SetDataCap (const BYTE newData[]);

	//Amihay: MRM CODE
	unsigned char m_payloadType;
	std::list <StreamDesc>  m_streams;
	DWORD m_IvrSsrc;//The value of the SSRC we use in IVR for audio and video
};
//----------------------------------------------------------------------------

class CAudModeH323	: public CMediaModeH323
{
CLASS_TYPE_1(CAudModeH323, CMediaModeH323)
public:
		// Constructors
	CAudModeH323();

		// Operations
	virtual const char* NameOf() const {return "CAudModeH323";}
	virtual DWORD GetBitRate() const;
	virtual cmCapDataType GetDataType() const {return cmCapAudio;}
	void SetFramePP(int maxFramePP,int minFramePP);
	void SetDSHforAvMcu();
	virtual size_t  SizeOf() const;
	virtual WORD operator==(const CMediaModeH323& other) const;
	// Audio Mode
	void SetAudioAlg(CapEnum alg,cmCapDirection eDirection);
	void SetBitRate(DWORD bitRate);
    static WORD GetBitRateFromAudioMode(BYTE audMode);
    void SetSacScm(cmCapDirection eDirection);
//    // TIP
//    virtual CapEnum GetAudioAlg(cmCapDirection eDirection);
protected:
	//Amihay: MRM CODE
	virtual void IntersectReceiveStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId, BYTE bIsMrcSlave);
	virtual void IntersectSendStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId);
	APIU32 GetMixDepth() const;
//	APIU32 GetMaxRecvSsrc() const;
//	APIU32 GetMaxSendSsrc() const;
	virtual void CopyStreamListToStreamGroup(const std::list <StreamDesc>&  rStreams, cmCapDirection direction);


};
//----------------------------------------------------------------------------

class CVidModeH323	: public CMediaModeH323
{
CLASS_TYPE_1(CVidModeH323, CMediaModeH323)
public:
		// Constructors
    CVidModeH323();
	CVidModeH323(BaseCapStruct *pCap,CapEnum type);
	void InitAllVideoMediaModeMembers();

	virtual const char* NameOf() const {return "CVidModeH323";}
	virtual void  Serialize(WORD format,CSegment& seg) const;
	virtual void  DeSerialize(WORD format,CSegment& seg);
	CVidModeH323& operator=(const CVidModeH323 &other);
	// Initializations
			// Operations
    virtual DWORD GetBitRate(void)const;
    DWORD GetMaxFR(void)const;
    DWORD GetH264mode(void)const;
    APIU8 GetPacketizationMode(void)const;
	virtual cmCapDataType GetDataType() const {return cmCapVideo;}
	void SetBitRate(int newBitRate);
	void SetMaxFR(int newMaxFR);
	void SetH264mode(int newH264mode);
	void SetFormatMpi(EFormat eFormat, int mpi);
	APIS8 GetFormatMpi(EFormat eFormat) const;
	APIS16 GetFrameRate(EFormat eFormat) const;
	DWORD  GetFrameRateForRTV() const;
	EFormat GetFormat() const;
	BYTE IsSupportErrorCompensation() const;
//	BYTE IsVidParamSupportedInCP(int maxPossibleFormatsMpi[kSIF]) const;
	void AddLowerResolutionsIfNeeded();
    void SetH264Scm(APIU16 profile, APIU8 level, long mbps, long fs, long dpb, long brAndCpb, long sar, long staticMB, cmCapDirection eDirection, APIU8 packatizationMode = H264_PACKETIZATION_MODE_UNSET);
    void SetVP8Scm(VP8VideoModeDetails VP8VideoDetails,cmCapDirection eDirection,APIS32 BitRate); //N.A. DEBUG VP8
    void SetSvcScm(APIU16 profile, APIU8 level, long mbps, long fs, long dpb, long brAndCpb, long sar, long staticMB, cmCapDirection eDirection);
    void GetH264Scm(APIU16& profile, APIU8& level, long& mbps, long& fs, long& dpb, long& brAndCpb, long& sar, long& staticMB) const;
    void GetVp8Scm(APIS32& fs, APIS32& maxFrameRate) const;
    void GetMSSvcSpecificParams(APIS32& Width, APIS32& Height, APIS32& aspectRatio, APIS32& maxFrameRate) const;
    void GetVP8SpecificParams(APIS32& Width, APIS32& Height, APIS32& aspectRatio, APIS32& maxFrameRate) const;
    void SetMsSvcScm(MsSvcVideoModeDetails MsSvcVidModeDetails,cmCapDirection eDirection,APIS32 BitRate);
    void GetMsSvcScm(long& mbps, long& fs) const;
	DWORD GetRtcpFeedbackMask(void)const;
	void SetRtcpFeedbackMask(DWORD rtcpFbMask);
	BYTE IsTIPContentEnableInH264Scm() const;
	void SetH264Profile(APIU16 profile);
	void GetH264Profile(APIU16& profile) const;
	void GetH264PacketizationMode(APIU8& packetizationMode) const;
	eVideoPartyType GetVideoPartyTypeMBPSandFS(DWORD staticMB,BYTE IsRsrcByFs = FALSE) const;
	void	SetSampleAspectRatio(APIS32 sar);
	void	SetStaticMB(APIS32 staticMB);
	WORD SetHighestH261ScmForCP(DWORD callRate, eVideoQuality videoQuality);
	WORD SetHighestH263ScmForCP(DWORD callRate, eVideoQuality videoQuality);
	WORD SetScmMpi(CapEnum protocol, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi);
    WORD SetH263ScmPlus(BYTE bAnnexP, BYTE bAnnexT, BYTE bAnnexN, BYTE bAnnexI_NS,
						 char vga, char ntsc, char svga, char xga, char qntsc);
	WORD SetH263ScmInterlaced(WORD interlacedResolution, char qcifMpi, char cifMpi);
	void SetHighestH263CapForVswAuto();
	BYTE SetContent(DWORD contentRate, CapEnum Protocol, cmCapDirection eDirection, BOOL isHD1080 = FALSE, BYTE HDContentMpi = 0, BYTE partyMediaType = eAvcPartyType, BOOL isHighProfile = FALSE);
	BYTE SetHDContent(DWORD contentRate, EHDResolution eHDRes, cmCapDirection eDirection, BYTE bContentAsVideo, BYTE HDMpi, BOOL isHighProfile = FALSE);
	BYTE SetTIPContent(DWORD contentRate, cmCapDirection eDirection, BYTE set264ModeAsTipContent = TRUE);

	virtual WORD operator==(const CMediaModeH323& other) const;
	virtual void UpdateParams(CH263VideoCap *pFirstVideoCap);
	virtual size_t  SizeOf() const;

	BYTE  IsAutoResolution() const;
	void  SetAutoResolution(BYTE bIsAuto);
	BYTE  IsAutoProtocol() const;
	void  SetAutoProtocol(BYTE bIsAuto);

	void  SetRtvScm(APIS32 width, APIS32 Height,APIS32 FR ,cmCapDirection eDirection,APIS32 BitRate = 0);
	void  GetRtvScm(long& mbps, long& fs) const;

protected:
	BYTE			m_bIsAutoResolution;
	BYTE			m_bIsAutoProtocol;

protected:
	//Amihay: MRM CODE
	virtual void IntersectReceiveStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId, BYTE bIsMrcSlave);
	virtual void IntersectSendStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId);
	virtual void CopyStreamListToStreamGroup(const std::list <StreamDesc>&  rStreams, cmCapDirection direction);
};


//----------------------------------------------------------------------------
class CDataModeH323	: public  CMediaModeH323
{
CLASS_TYPE_1(CDataModeH323, CMediaModeH323)
public:
		// Constructors
    CDataModeH323();

		// Operations
	virtual const char* NameOf() const {return "CDataModeH323";}
	void SetBitRate(int newBitRate);
	void SetFECCMode(CapEnum feccMode, DWORD feccRate, cmCapDirection eDirection);
    virtual DWORD GetBitRate(void) const;
	virtual cmCapDataType GetDataType() const {return cmCapData;}

	virtual WORD operator==(const CMediaModeH323& other) const;
   	virtual size_t  SizeOf() const;
   	void SetIsSupportScp(BYTE bIsSupportScp);
protected:
	//Amihay: MRM CODE
	virtual void IntersectReceiveStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId, BYTE bIsMrcSlave){/*no streams in data mode - do nothing!*/}
	virtual void IntersectSendStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId){/*no streams in data mode - do nothing!*/}
	virtual void CopyStreamListToStreamGroup(const std::list <StreamDesc>&  rStreams, cmCapDirection direction) {/*no streams in data mode - do nothing!*/}
};
//----------------------------------------------------------------------------
class CBfcpModeH323	: public  CMediaModeH323
{
CLASS_TYPE_1(CBfcpModeH323, CMediaModeH323)
public:
		// Constructors
    CBfcpModeH323();

		// Operations
    EResult SetBfcp(enTransportType transType);
	virtual const char* NameOf() const {return "CBfcpModeH323";}
	virtual DWORD GetBitRate(void) const {return 0;}
	virtual cmCapDataType GetDataType() const {return cmCapBfcp;}
	virtual WORD operator==(const CMediaModeH323& other) const;
   	virtual size_t  SizeOf() const;
   	virtual enTransportType GetTransportType(void) const;
   	void 	SetTransportType(enTransportType transType);

	void 	SetBfcpParameters(eBfcpSetup setup, eBfcpConnection connection, eBfcpFloorCtrl floorCtrl, eBfcpMStreamType mstreamType);
	void 	SetConfUserId(WORD confId, WORD userid);
	void 	SetFloorIdParams(char* pFloorid, char* pStreamid);
	WORD 	GetUserId();
	WORD 	GetConfId();

protected:
	//Amihay: MRM CODE
	virtual void IntersectReceiveStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId, BYTE bIsMrcSlave){/*no streams in data mode - do nothing!*/}
	virtual void IntersectSendStreams(CBaseCap *pCap,payload_en payload_type, DWORD aPartyId){/*no streams in data mode - do nothing!*/}
	virtual void CopyStreamListToStreamGroup(const std::list <StreamDesc>&  rStreams, cmCapDirection direction) {/*no streams in bfcp mode - do nothing!*/}

   	enTransportType	m_transType;


};

//----------------------------------------------------------------------------

class CComModeH323 : public CPObject
{
CLASS_TYPE_1(CComModeH323, CPObject)
public:

		// Constructors
	CComModeH323();
	CComModeH323(const CComModeH323 &other);
	virtual ~CComModeH323();
	virtual const char* NameOf() const { return "CComModeH323";}
		// Initializations
	void CreateSipOptions(CCommConf* pCommConf,BYTE videoProtocol,BYTE partyEncryptionMode = AUTO);

		// Operations
	void  Serialize(WORD format,CSegment& seg) const;
	void  Serialize(CSegment &seg, cmCapDirection direction,BYTE bOperUse,CCapH323* pCaps = NULL) const;
	void  Serialize(CSegment &seg, const CAudModeH323 &audMode,
					const CVidModeH323 &vidMode,const CVidModeH323 &vidContMode,
					const CDataModeH323 &dataMode,const CBfcpModeH323 &bfcpMode, BYTE bOperUse,CCapH323* pCaps = NULL,cmCapDirection direction = cmCapReceive) const;
		// add 2 new serialize functions to support LPR with SIP parties
	void  Serialize(CSegment &seg, cmCapDirection direction,BYTE bOperUse,CSipCaps* pCaps ) const;
	void  Serialize(CSegment &seg, const CAudModeH323 &audMode,
						const CVidModeH323 &vidMode,const CVidModeH323 &vidContMode,
						const CDataModeH323 &dataMode, const CBfcpModeH323 &bfcpMode, BYTE bOperUse,CSipCaps* pCaps ,cmCapDirection direction = cmCapReceive) const;
	void  DeSerialize(WORD format,CSegment &seg);

	// Application API
	BYTE  IsSecondary() const;

	void  SetConfType(EConfType eConfType) {m_eConfType = eConfType;}
	void  SetConfType(BYTE bIsFreeVideoRate, BYTE bIsSoftCp, BYTE bIsQuad);
	EConfType GetConfType() const {return m_eConfType;}

	void  SetTipContentMode(BYTE eTipContentMode) {m_eTipContentMode = eTipContentMode;}
	BYTE  GetTipContentMode() const {return m_eTipContentMode;}

    void  SetConfMediaType(eConfMediaType aConfMediaType) {m_confMediaType = aConfMediaType;}
    eConfMediaType GetConfMediaType() const {return m_confMediaType;}
	DWORD GetCallRate() const {return m_callRate;}
	void  SetCallRate(DWORD rate);
	void  SetTotalVideoRate(DWORD rate){m_TotalVideoRate=rate;};
	DWORD GetTotalVideoRate() {return m_TotalVideoRate;};
	//FSN-613: Dynamic Content for SVC/Mix Conf
	void  SetDeclareContentRate(DWORD rate){m_declareContentRate=rate;};
	DWORD GetDeclareContentRate() {return m_declareContentRate;};
	
   // Audio Mode
	void SetAudioAccordingToPartyName(const char* pPartyName);
	void SetAudioAlg(CapEnum alg,cmCapDirection eDirection,APIS32 bitrate = 0);
	void SetAudioAlg(BOOL bIsSip, DWORD videoLineRate, const char* pPartyName, BYTE isReplace = 0, BYTE bReduceAudioCodecs = FALSE);
	// FECC
	void SetFECCMode(CapEnum feccMode, DWORD feccRate, cmCapDirection eDirection);
			// Set mode Off/On
	void  SetMediaOff(cmCapDataType type, cmCapDirection direction = cmCapReceive, ERoleLabel eRole = kRolePeople);

			// Check mode Off
	BYTE IsMediaOff(cmCapDataType type, cmCapDirection direction = cmCapReceive, ERoleLabel eRole = kRolePeople) const;

			// Check mode On
	BYTE IsMediaOn(cmCapDataType type, cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople)  const;
	BYTE  IsNewContentModeDiffersOnlyInRes(const CComModeH323& other, cmCapDirection direction) const;
			// Get mode
	const CMediaModeH323& GetMediaMode(cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) const;
	CMediaModeH323& GetMediaMode(cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople);

			// Get mode type
	WORD GetMediaType(cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) const;
			// Get h320 mode type
	WORD GetH320ModeType(cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) const;

            // Bit rate
    DWORD GetMediaBitRate(cmCapDataType type, cmCapDirection direction = cmCapReceive, ERoleLabel eRole = kRolePeople) const;
    DWORD GetTotalBitRate(cmCapDirection direction = cmCapReceive) const;
 	DWORD GetOngoingCallBitRate() const;
   	void  SetVideoBitRate(int newBitRate, cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople);
   	DWORD   GetVideoBitRate(cmCapDirection direction,ERoleLabel eRole) const;

   	void  SetDataBitRate(int newBitRate, cmCapDirection direction = cmCapReceive);

           // Copy to cap buffer
	void  CopyMediaData(BYTE data[],cmCapDataType type, cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) const;
	void  CopyMediaToCapBuffer(capBuffer* pCapBuffer,cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople,BYTE bOperUse = NO) const;
	CBaseCap* GetMediaAsCapClass(cmCapDataType type, cmCapDirection direction,ERoleLabel eRole = kRolePeople) const;	// Allocate memory!

			// Set mode
	void	CopyMediaMode(const CComModeH323& newComMode,
						  cmCapDataType type, cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople);
	void	CopyMediaModeToOppositeDirection(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole = kRolePeople);
	void	SetMediaMode(const CMediaModeH323& newMediaMode,
		                 cmCapDataType type, cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople, bool aKeepStreams = false);
	void	SetMediaMode(const capBuffer* newMediaMode,
		                 cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople, bool aKeepStreams = false);
	void	SetMediaMode(const CBaseCap* pNewMediaMode,
						 cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople, bool aKeepStreams = false);
	void	SetMediaMode(WORD newType,WORD newDataLength,const BYTE newData[],
		                 cmCapDataType type,cmCapDirection direction = cmCapReceiveAndTransmit,ERoleLabel eRole = kRolePeople, bool aKeepStreams = false);
	void	SwitchMediaDirections();

			// Get Length
	WORD	GetMediaLength(cmCapDataType dataType, cmCapDirection direction = cmCapReceive, ERoleLabel eRole = kRolePeople) const;

	// Is media contains other media.
	BYTE    IsMediaContaining(const CComModeH323& other, DWORD valuesToCompare, DWORD* pDetails,
		                      cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole = kRolePeople) const;

	BYTE    IsMediaContaining(const CBaseCap& other, DWORD valuesToCompare, DWORD* pDetails,
							  cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole = kRolePeople) const;

	void    GetMediaParams(cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole,
							   CSecondaryParams &secParams,DWORD details = 0) const;
	void    GetDiffFromDetails(cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole,
							   CSecondaryParams &secParams,DWORD details) const;

	BYTE    IsMediaEquals(const CComModeH323& other,cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole = kRolePeople) const;
	BYTE	IsMediaEquals(const CBaseCap& other,DWORD valuesToCompare,cmCapDataType dataType,cmCapDirection direction,ERoleLabel eRole = kRolePeople) const;

	BYTE    IsSdesMediaEquals(const CComModeH323& other, cmCapDataType dataType, cmCapDirection direction,ERoleLabel eRole = kRolePeople) const;

	void    SetFormatMpi(EFormat eFormat,int mpi,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople);
	APIS8   GetFormatMpi(EFormat eFormat,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) const;
	APIS16  GetFrameRate(EFormat eFormat,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) const;
	EFormat GetVideoFormat(cmCapDirection direction,ERoleLabel eRole = kRolePeople) const;
    void	SetAudioFramePerPacket(int maxFramePP,int minFramePP,cmCapDirection direction = cmCapReceive);
    void    SetAudioWithDSHforAvMcu();
	DWORD   GetFrameRateForRTV(cmCapDirection direction,ERoleLabel eRole) const;

    void    SetAllModesOff(cmCapDirection direction = cmCapReceiveAndTransmit);
	BYTE    IsSupportErrorCompensation(cmCapDirection direction = cmCapReceive) const;
//	BYTE    IsVidParamSupportedInCP(int maxPossibleFormatsMpi[kSIF], cmCapDirection direction = cmCapReceive) const;

//	BYTE    GetVideoProtocolInH320Values(WORD& vidProtocol, cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
//	BYTE    GetVideoFormatAndMpiInH320Values(WORD& resolution, int& mpi, cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	WORD    GetChangeFromNewScmMode(CComModeH323* pNewScm,cmCapDirection direction,cmCapDataType dataType,ERoleLabel eRole) const;
	void    AddLowerResolutionsIfNeeded(cmCapDirection eDirection = cmCapReceiveAndTransmit);

	BYTE  IsAutoVideoResolution();
	void  SetAutoVideoResolution(BYTE bIsAuto);
	BYTE  IsAutoVideoProtocol();
	void  SetAutoVideoProtocol(BYTE bIsAuto);

	// H239/Content
	BYTE    SetContent(DWORD contentRate, cmCapDirection eDirection, CapEnum Protocol, BOOL isHD1080 = FALSE, BYTE HDContentMpi = 0, BOOL isHighProfile = FALSE);
	BYTE    SetHDContent(DWORD contentRate, cmCapDirection eDirection, EHDResolution eHDRes,BYTE HDMpi,BOOL isHighProfile = FALSE);
	BYTE    SetTIPContent(DWORD contentRate, cmCapDirection eDirection, BYTE set264ModeAsTipContent = TRUE);
	BYTE    SetContentBitRate(DWORD contentRate, cmCapDirection eDirection);
	DWORD   GetContentBitRate(cmCapDirection eDirection) const;
	BYTE    IsContent(cmCapDirection eDirection) const;
	BYTE    RemoveContent(cmCapDirection eDirection);
	BYTE    SetContentProtocol(BYTE Protocol, cmCapDirection eDirection,BOOL isHD1080 = FALSE);

	BYTE    RemoveData(cmCapDirection eDirection);
	//highest common
	void    SetH264Scm(APIU16 profile, APIU8 level, long mbps, long fs, long dpb, long brAndCpb, long sar, long staticMB, cmCapDirection eDirection, APIU8 packatizationMode = H264_PACKETIZATION_MODE_UNSET);
	void    SetVP8Scm(VP8VideoModeDetails VP8VideoDetails,cmCapDirection eDirection, APIS32 BitRate = 0); //N.A. DEBUG VP8
	void    SetSvcScm(APIU16 profile, APIU8 level, long mbps, long fs, long dpb, long brAndCpb, long sar, long staticMB, cmCapDirection eDirection);
	void	SetSacScm(cmCapDirection eDirection);
	void	SetIsSupportScp(BYTE bIsSupportScp);
    void	SetOperationPointsAndRecvStreamsGroup(const CVideoOperationPointsSet* pOperationPoints);
    void    SetOperationPoints(const CVideoOperationPointsSet* pOperationPoints);
    void    SetToOperationPointsOnly(const CVideoOperationPointsSet* pOperationPoints);
    const VideoOperationPoint* GetLowestOperationPoint(DWORD partyId) const;
    const VideoOperationPoint* GetHighestOperationPoint(DWORD partyId);
    void    UpdateStreamsAndPayloadType(CSipCaps* pCapsSet, bool updatePayloadTypeOnly = FALSE);
    void    SetPayloadTypeForMediaMode(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole, payload_en payload_type);
    void    SetStreamsForMediaMode(CBaseCap* pNewMediaMode, cmCapDataType type,cmCapDirection direction,ERoleLabel eRole,payload_en payload_type);
	void    GetH264Scm(APIU16& profile, APIU8& level, long& mbps, long& fs, long& dpb, long& brAndCpb, long& sar, long& staticMB, cmCapDirection eDirection) const;
	BYTE    IsTIPContentEnableInH264Scm() const;
	eVideoPartyType GetVideoPartyType(cmCapDirection eDirection, DWORD staticMB = 0 ,BYTE IsRsrcByFs = FALSE) const;
	void  	GetFSandMBPS(cmCapDirection& eDirection, APIU16& profile, APIU8& level, long& fs, long& mbps, long& sar, long& staticMB,long& dbp) const;
	void    GetRtvFSandMBPS(cmCapDirection& eDirection, long& fs, long& mbps) const;
	void    GetMSSvcSpecificParams(cmCapDirection& eDirection, APIS32& Width, APIS32& Height, APIS32& aspectRatio, APIS32& maxFrameRate) const;
	void    GetVp8Scm(cmCapDirection& eDirection,APIS32& fs, APIS32& maxFrameRate) const;
    void 	GetMsSvcScm(long& mbps, long& fs,cmCapDirection eDirection) const;
	void    SetH264Profile(APIU16 profile, cmCapDirection eDirection);
	APIU16 	GetH264Profile(cmCapDirection eDirection) const;
	APIU8  	GetH264PacketizationMode(cmCapDirection eDirection) const;
	void  	SetH264VideoParams(H264VideoModeDetails h264VidModeDetails, APIS32 sar, cmCapDirection direction = cmCapReceiveAndTransmit, APIU8 packatizationMode = H264_PACKETIZATION_MODE_UNSET);
	void    SetVP8VideoParams(VP8VideoModeDetails vp8VideoDetails, cmCapDirection direction); //N.A. DEBUG VP8
	void    SetRtvVideoParams(RTVVideoModeDetails rtvVidModeDetails, cmCapDirection direction = cmCapReceiveAndTransmit,APIS32 BitRate=0);
	void    SetRtvScm(APIS32 width, APIS32 Height,APIS32 FR ,cmCapDirection eDirection,APIS32 BitRate = 0);
	void    SetMsSvcScm(MsSvcVideoModeDetails MsSvcVidModeDetails,cmCapDirection eDirection,APIS32 BitRate);
	void    SetScmToHdCp(BYTE HDResolution, cmCapDirection direction = cmCapTransmit);
	void    SetScmToCpHD720At60(cmCapDirection direction = cmCapTransmit);
	void    SetScmToCpHD1080At60(cmCapDirection direction  = cmCapTransmit);
	void	SetSampleAspectRatio(APIS32 sar, cmCapDirection direction = cmCapTransmit);
	void	SetStaticMB(APIS32 staticMB, cmCapDirection direction = cmCapTransmit);
	WORD    SetHighestH261ScmForCP(cmCapDirection eDirection, eVideoQuality videoQuality);
	WORD    SetHighestH263ScmForCP(cmCapDirection eDirection, eVideoQuality videoQuality);
    WORD    SetScmMpi(CapEnum protocol, int qcifMpi = -1, int cifMpi = -1, int cif4Mpi = -1, int cif16Mpi = -1, cmCapDirection eDirection = cmCapReceiveAndTransmit);
    WORD    SetH263ScmPlus(BYTE bAnnexP, BYTE bAnnexT, BYTE bAnnexN, BYTE bAnnexI_NS,
						   char vga = -1, char ntsc = -1, char svga = -1, char xga = -1, char qntsc = -1, cmCapDirection eDirection = cmCapReceiveAndTransmit);
	WORD    SetH263ScmInterlaced(WORD interlacedResolution, char qcifMpi = -1, char cifMpi = -1, cmCapDirection eDirection = cmCapReceiveAndTransmit);
	void    SetHighestH263CapForVswAuto(cmCapDirection eDirection = cmCapReceiveAndTransmit);

	void    GetRtvScm(long& mbps, long& fs,cmCapDirection eDirection) const;


	// Encryption
	void 	CreateLocalComModeECS(EenMediaType encMediaType,EenHalfKeyType halfKeyType);
	void 	SetEncryption(BYTE bShouldEncrypted, BYTE bShouldDisconnectonEncryptFailure);
	void 	SetDtlsEncryption(BYTE bShouldEncrypted);
	void 	SetDtlsAvailable(BYTE bDtlsAvailable);
	void 	SetIsEncrypted(DWORD isEncrypted, BYTE bShouldDisconnectOnEncryptFailure) { SetEncryption((isEncrypted == Encryp_On || isEncrypted == TRUE), bShouldDisconnectOnEncryptFailure);};
//	void	SetIsEncrypted(DWORD isEncrypted) { SetIsEncrypted(isEncrypted, isEncrypted == Encryp_On);};
	DWORD	GetIsEncrypted();
    bool    IsEncrypted() const {return (m_isEncrypted == Encryp_On);}
	DWORD	GetIsDtlsEncrypted();
	BYTE	GetIsDtlsAvailable();
	BYTE	GetIsDisconnectOnEncryptionFailure() const;
	EenMediaType	GetEncryptionAlgType() const;
	void    SetEncryptionAlgType(EenMediaType encAlgType);
	EenHalfKeyType	GetEncryptionHalfKeyType();
	void 	SetEncryptionHalfKeyType(EenHalfKeyType encHalfKeyType);
	//////////////////////////////////////////////
	// For the asymmetric H264 video modes
	void 	SetHd720Enabled(BYTE bIsHd720Enabled);
	BYTE    IsHd720Enabled() const;
	void 	SetHd1080Enabled(BYTE bIsHd1080Enabled);
	BYTE    IsHd1080Enabled() const;
	void 	SetHd720At60Enabled(BYTE bIsHd720At60Enabled);
	BYTE    IsHd720At60Enabled() const;
	void 	SetHd1080At60Enabled(BYTE bIsHd1080At60Enabled);
	BYTE    IsHd1080At60Enabled() const;
	BYTE    IsH264HighProfile(cmCapDirection eDirection) const;
	BYTE    IsH264HighProfileContent(cmCapDirection eDirection) const;
	DWORD 	GetFlowControlRateConstraint();
	DWORD 	GetFlowControlRateConstraintForPresentation();
	void 	SetFlowControlRateConstraint(DWORD flowControlRateConstraint);
	void    SetFlowControlRateConstraintForPresentation(DWORD flowControlRateConstraintForPresentation);

    friend WORD operator== (const CComModeH323& first,const CComModeH323& second);
    friend WORD operator!= (const CComModeH323& first,const CComModeH323& second);
    CComModeH323& operator= (const CComModeH323 &other);

    BYTE IsVidModeRcvAndXmitMediaOn();
    void Dump(const char* title, WORD level  = eLevelInfoNormal) const;
	virtual void Dump(std::ostream &msg) const;

	void CopyStaticAttributes(const CComModeH323& other);
	// this method is required where a media type is created out of a different direction type and direction is needed to be updated
	void SetDirection(cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole = kRolePeople);
	// set content video protocols (H263, H264 or Auto - both H263 and H264)
	void  SetContentProtocolMode(ePresentationProtocol contentProtocolMode) {m_contentProtocol = contentProtocolMode;}
	ePresentationProtocol GetContentProtocolMode() const {return m_contentProtocol;}

	// LPR
	void	SetIsLpr(BYTE isLpr);
	BYTE	GetIsLpr() const;

	//LYNC2013_FEC_RED:
	void SetIsFec(BYTE isFec);
	BYTE GetIsFec() const;
	void SetIsRed(BYTE isRed);
	BYTE GetIsRed() const;

	void    SetHdVswResolution(EHDResolution hdVswRes);
	EHDResolution GetHdVswResolution();

	// Siren14 Stereo
	void    DecideOnConfBitRateForAudioSelection(DWORD & confBitRate);
	BYTE    isModeIncludeStereo();


	void SetIsShowContentAsVideo(BYTE contetAsVideo){m_bShowContentAsVideo = contetAsVideo;};
	BYTE GetIsShowContentAsVideo() const {return m_bShowContentAsVideo;};
	void ChangeH264ScmForForceParty(const char* partyName);
	void SetCopTxLevel(BYTE copTxLevel){m_copTxLevel = copTxLevel;}
	BYTE GetCopTxLevel() const {return m_copTxLevel;}
	void SetVideoRxModeAccordingDecoderResolution(ECopDecoderResolution decoderResolution, CapEnum capCode,APIU16 capProfile,CCOPConfigurationList* pCOPConfigurationList =NULL,BOOL bIsCascade = FALSE,BOOL bIsPartyMasterOrSlaveLecturer = FALSE);
	DWORD CalcCopMinFlowControlRate (const CCommConf* pCommConf, DWORD dwFlowControlRate,DWORD contentrate = 0);
	int	CopTranslateConfResolutionToConfigureResolution(ECopDecoderResolution decoderResolution);
	CapEnum	CopFindBestLevelAccordingToResolution(ECopDecoderResolution& decoderResolution, CapEnum capCode,APIU16& capProfile,CCOPConfigurationList* pCOPConfigurationList);
	int	GetProtocolFromCapAndProfile(CapEnum capCode,APIU16 profile);
	CapEnum GetCapAndProfileFromProtocol(int protocol,APIU16& profile);
	ECopDecoderResolution ConvertEncoderResToDecoderRes(int EncoderRes,BYTE FrameRate,CapEnum capCode);

	BYTE IsRemoveAudioCodec(CapEnum codec);
	//SDES
	void CreateLocalSipComModeSdes(BYTE bIsEncrypted, BOOL isTipCompatible = FALSE);
	void CreateLocalSipComModeDtls(BYTE bIsEncrypted, BOOL isTipCompatible = FALSE);
	void CreateLocalSipComModeSdesForSpecficMedia(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole);
	void CreateLocalSipComModeDtlsForSpecficMedia(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole);
	void UpdateXmitSdesAudioTag(APIU32 tag);
	void UpdateXmitSdesVideoTag(APIU32 tag);
	void UpdateXmitSdesDataTag(APIU32 tag);
	void UpdateXmitSdesContentTag(APIU32 tag);
	void UpdateRxSdesAudioTag(APIU32 tag);
	void UpdateRxSdesAudioMkiInUse(BYTE bIsNeedToSendMKI);
	void UpdateRxSdesVideoMkiInUse(BYTE bIsNeedToSendMKI);
	void UpdateRxSdesDataMkiInUse(BYTE bIsNeedToSendMKI);
	void UpdateRxSdesContentMkiInUse(BYTE bIsNeedToSendMKI);
	void UpdateTxSdesAudioMkiInUse(BYTE bIsNeedToSendMKI);
	void UpdateTxSdesVideoMkiInUse(BYTE bIsNeedToSendMKI);
	void UpdateTxSdesDataMkiInUse(BYTE bIsNeedToSendMKI);
	void UpdateTxSdesContentMkiInUse(BYTE bIsNeedToSendMKI);
	void UpdateRxSdesVideoTag(APIU32 tag);
	void UpdateRxSdesDataTag(APIU32 tag);
	void UpdateRxSdesContentTag(APIU32 tag);
	void UpdateXmitSdesAudioMasterSaltBase64Key(char* key);
	void UpdateXmitSdesVideoMasterSaltBase64Key(char* key);
	void UpdateXmitSdesDataMasterSaltBase64Key(char* key);
	void UpdateXmitSdesContentMasterSaltBase64Key(char* key);
	CSdesCap *GetSipSdes(cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) ;
	void SetSipSdes(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, CSdesCap* pSdesCap);
	void RemoveSipSdes(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole);
	CDtlsCap *GetSipDtls(cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) ; //_dtls_
	void SetSipDtls(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, CDtlsCap* pDtlsCap); //_dtls_
	void RemoveSipDtls(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole); //_dtls_
	BYTE IsDtlsChannelEnabled(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole);
	void SetSdesMkiDefaultParams(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole, BYTE bIsNeedToSendMKI);
	void SetSdesLifeTimeDefaultParams(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole, BYTE bLifeTimeInUse);
	void SetSdesFecDefaultParams(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole, BYTE bFecKeyInUse);

	BYTE isHDContent1080Supported(cmCapDirection direction) const;
	BYTE isHDContent720Supported(cmCapDirection direction) const;
	// TIP
	void SetTipMode(ETipMode eTipMode);
	BOOL GetIsTipMode() const;
	BOOL IsTipNegotiated() const;
	ETipAuxFPS GetTipAuxFPS() const {return m_TipAuxFPS;}
	void SetTipAuxFPS(ETipAuxFPS tipAuxFPS)  {m_TipAuxFPS = tipAuxFPS;}
	void SetRtcpFeedbackMask(DWORD rtcpFbMask, cmCapDirection direction, ERoleLabel eRole = kRolePeople);
	DWORD GetRtcpFeedbackMask(cmCapDirection direction,ERoleLabel eRole = kRolePeople) const;
	void SetStreamsForPayloadType(CBaseCap* pNewMediaMode, cmCapDataType type,cmCapDirection direction,ERoleLabel eRole,payload_en payload_type);
    void SetStreamsForResolutionAndBitRate(CBaseCap* pNewMediaMode, cmCapDataType type,cmCapDirection direction,ERoleLabel eRole);
    void UpdateStreamsResolutionAndBitRate(CSipCaps* pCapsSet);

    void UpdateStreamsPayloadType(CSipCaps* pCapsSet);
//    bool AddSupplementalInfo(CBaseCap* pCap,cmCapDataType mediaType, cmCapDirection direction,ERoleLabel eRole);
    void SetVswRelayStreamDescAccordingToOperationPoint(int layerId,cmCapDirection direction,ERoleLabel role,DWORD ssrcVideo,bool update);
    void UpdateHdVswForAvcInMixMode(CBaseCap *pBaseInCap=NULL);

    EResult SetBfcp(enTransportType transType, cmCapDirection eDirection = cmCapReceiveAndTransmit);
    void 	SetBfcpTransportType(enTransportType transType, cmCapDirection eDirection = cmCapReceiveAndTransmit);
    enTransportType GetBfcpTransportType(cmCapDirection eDirection = cmCapReceiveAndTransmit) const;
    void 	SetBfcpParameters(eBfcpSetup setup, eBfcpConnection connection, eBfcpFloorCtrl floorCtrl, eBfcpMStreamType mstreamType);
    void 	SetConfUserIdForBfcp(WORD confId, WORD userid);
    void 	SetFloorIdParamsForBfcp(char* pFloorid, char* pStreamid);
    WORD 	GetBfcpUserId();
    WORD 	GetBfcpConfId();

    EOperationPointPreset GetOperationPointPreset() const {return m_eOPPreset;}
    void SetOperationPointPreset(EOperationPointPreset eOPPreset) {m_eOPPreset = eOPPreset;}

    eIsUseOperationPointsPreset GetIsUseOperationPointesPresets() const {return m_eIsUseOperationPointesPresets;}
    void SetIsUseOperationPointesPresets(eIsUseOperationPointsPreset isUseOperationPointesPresets) {m_eIsUseOperationPointesPresets = isUseOperationPointesPresets;}

    void CopyStreamListToStreamGroup(cmCapDataType type, ERoleLabel eRole, cmCapDirection sourceDirection, cmCapDirection targetMediaModeDirection);

protected:

		// Attributes
	CAudModeH323	m_audModeRcv;
	CAudModeH323	m_audModeXmit;

	CVidModeH323	m_vidModeRcv;
	CVidModeH323	m_vidModeXmit;

	CVidModeH323    m_vidContModeRcv;
	CVidModeH323    m_vidContModeXmit;

    CDataModeH323   m_dataModeRcv;
    CDataModeH323   m_dataModeXmit;

    CBfcpModeH323   m_bfcpModeRcv;
    CBfcpModeH323   m_bfcpModeXmit;

    //IS_PREFER_TIP_MODE:
    BYTE            m_eTipContentMode;

	EConfType		m_eConfType;
	eConfMediaType  m_confMediaType;
	DWORD			m_callRate;

	EenMediaType 	m_encMediaTypeAlg;
	EenHalfKeyType  m_halfKeyType;
	DWORD			m_isEncrypted;
	DWORD			m_isDtlsEncrypted; //_dtls_
	BYTE			m_bDtlsAvailable; //_dtls_
	BYTE			m_bDisconnectOnEncryptionFailure;
	////////For the HD asymmetric modes
	BYTE			m_bIsHd720Enabled;
	BYTE			m_bIsHd1080Enabled;
	BYTE			m_bIsHd720At60Enabled;
	BYTE			m_bIsHd1080At60Enabled;

	DWORD			m_flowControlRateConstraint;   // 0 = No rate limitation  (for people role)
	DWORD			m_flowControlRateConstraintForPresentation;
	DWORD           m_TotalVideoRate;

	// LPR
	BYTE 			m_isLpr;

	//LYNC2013_FEC_RED:
	BYTE 			m_isFec;
	BYTE 			m_isRed;

	ePresentationProtocol m_contentProtocol;
	DWORD			    m_declareContentRate;  //FSN-613: Dynamic Content for SVC/Mix Conf
	EHDResolution   m_HdVswResolution;
	BYTE            m_bShowContentAsVideo;

	BYTE			m_copTxLevel; // 0xFF = no level.

	// TIP
	ETipMode		m_eTipMode;
	ETipAuxFPS		m_TipAuxFPS; // Tip auxiliary FPS

	CVideoOperationPointsSet m_operationPoints;
	EOperationPointPreset    m_eOPPreset; // operation point preset for vsw-mix avc-svc
	eIsUseOperationPointsPreset m_eIsUseOperationPointesPresets;
	DWORD           m_partyId;

	ePMediaType		m_partyMediaType;

public:
	//Amihay: MRM CODE
	void IntersectStreams(const CSipCaps* pCapsSet,cmCapDataType type,cmCapDirection direction,ERoleLabel eRole, BYTE bIsMrcSlave);
	void SetStreamsListForMediaMode(const std::list <StreamDesc>&  rStreams,cmCapDataType type,cmCapDirection direction,ERoleLabel eRole);
	const std::list <StreamDesc>&  GetStreamsListForMediaMode(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole) const;
	void RemoveStreamsListForMediaMode(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole);
	unsigned char GetPayloadType(cmCapDataType type,cmCapDirection direction,ERoleLabel eRole) const;
	void SetMediaModeSvc(capBuffer* pCapBuffer, cmCapDataType mediaType, cmCapDirection direction, ERoleLabel eRole = kRolePeople);
	const CMediaModeH323& GetMediaModeSvc(cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople) const;
	CMediaModeH323& GetMediaModeSvc(cmCapDataType type,cmCapDirection direction = cmCapReceive,ERoleLabel eRole = kRolePeople);
	BOOL GetSsrcIds(cmCapDataType aMediaType,cmCapDirection direction ,APIU32*& aSsrcIds, int* aNumOfSsrcIds);
	BOOL SetSsrcIds(cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole, APIU32 *aSsrcIds, int aNumOfSsrcIds, bool aIsUpdate);
	virtual VIDEO_OPERATION_POINT_SET_S* GetOperationPoints(cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole = kRolePeople) const;
    virtual CVideoOperationPointsSet* GetOperationPoints() {return &m_operationPoints;}
	virtual STREAM_GROUP_S* GetStreamsGroup(cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole = kRolePeople) const;
	virtual void SetStreamsGroup(STREAM_GROUP_S& rStreamGroup, cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole = kRolePeople);
	bool  IsHdVswInMixMode() const;

	BOOL CreateStreamsDescList(cmCapDataType aMediaType,cmCapDirection direction, ERoleLabel eRole, APIU32 *aSsrcIds, int aNumOfSsrcIds);
	virtual void SetPartyId(DWORD aPartyId) { m_partyId = aPartyId;}
    DWORD GetPartyId() {return m_partyId;}
	DWORD GetIvrSsrc(cmCapDataType type)const;
    void SetPartyMediaType(ePMediaType type) {m_partyMediaType = type;}
    ePMediaType GetPartyMediaType() {return m_partyMediaType;}
};

//----------------------------------------------------------------------------

#endif // _H323SCM

