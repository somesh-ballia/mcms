//+========================================================================+
//                            H323Caps.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323Caps.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: IP party programers  (Yael, Guy)                            |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 31.5.06    |                                                      |
//+========================================================================+


#ifndef _H323CAPS
#define _H323CAPS


#include "Capabilities.h"
#include "H323Scm.h"
#include "CapInfo.h"
#include "CopVideoTxModes.h"
#include <stdarg.h>

class CSegment;


struct capHeadStruct
{
	char capName[CapNameLength];
	BaseCapStruct* pCapPtr;
};

struct capMatchList
{
	WORD altNumber;
	BYTE sim;
	BYTE count;
	DWORD* pCapPtr;
	capMatchList* pNextCap;
};

struct candidateList
{
	BYTE count;
	CMediaModeH323* pCapPtr;
	candidateList* pNextCap;
};

struct localCap
{
	int capTypeCode;
	capBuffer* pCapPtr;
};


#define INITIAL_CAPS_LEN	64*FD_SET_SIZE  //4096


// for caps comparison result
//----------------------------
#define IS_SECONDS_CAP_HIGHER(res)        (((res)&0x0000007f)!=0)
#define ARE_CAPS_EQUAL(res)               (((res)&0x000000ff)==0)
#define SET_SECONDS_CAP_HIGHER(res)       ((res) |= 0x00000040)
#define SET_SECOND_CAP_LOWER(res)         ((res) |= 0x00000080)
#define SET_PROTOCOL_NOT_FOUND(res)       ((res) |= 0x00000001)
#define SET_HIGHER_FRAME_RATE(res)        ((res) |= 0x00000002)
#define SET_HIGHER_BITRATE(res)           ((res) |= 0x00000004)
#define SET_RESOLUTION_NOT_SUPPORTED(res) ((res) |= 0x00000008)
#define SET_ANNEX_NOT_SUPPORTED(res)      ((res) |= 0x00000010)
#define SET_CF_NOT_SUPPORTED(res)         ((res) |= 0x00000020)
#define SET_VIDEO_TYPE(res,type)          ((res) |= ((type)<<8))
#define SET_RESOLUTION_H263(res,type)     ((res) |= (((type)+1)<<10))
#define SET_RESOLUTION_H261(res,type)     ((res) |= (((type)-19)<<10))
#define SET_ANNEX_TYPE(res,i)             ((res) |= (((i)+1)<<16
#define SET_CF_TYPE(res,i)                ((res) |= (((i)+1)<<10))

// defines for cascade match
const BYTE NOCAP  		    = 0;
const BYTE SECONDARY	    = 2;
const BYTE AUDIO_VIDEO      = 3;
const BYTE AUDIO_DATA       = 4;
const BYTE FULL_MATCH		= 5;
const BYTE FOUND_VIDEO		= 6;
const BYTE FOUND_DATA		= 7;



class CCapH323 : public CPObject
{
CLASS_TYPE_1(CCapH323, CPObject)
public:
	// Constructors
	CCapH323();
	CCapH323(const CCapH323& other);
//	CapH323(const CCapH221& capH221, DWORD videoRate,DWORD confRate);
	virtual ~CCapH323();

	// Initializations
   void  CreateAllButAudioAndVideo(const CComModeH323* pScm,
							  WORD& numOfT120Cap, WORD& numOfPeopContCap, WORD& numOfH239ControlCap,
							  WORD& numOfContentCap, WORD& numOfDuoCap,  WORD& numOfNsCap,
							  WORD& numOfFeccCap, WORD& numOfLprCap, WORD&numOfEncCap, WORD& numOfDptrCap, EenMediaType encAlg,
							  BYTE isRemoveH239 = FALSE, BYTE isFixContentProtocol = FALSE, BYTE isRemoveAnnexQ = FALSE,
							  BYTE isRemoveRvFecc = FALSE, BYTE isRemoveLpr = FALSE,BYTE isRemoveEPC = FALSE, DWORD serviceId=0,
							  BYTE isRemoveDPTR = FALSE);

//	void  Create(const CComMode& scm, const CCapH221& h221Cap, DWORD videoRate,DWORD confRate, const CComModeH323* pH323Scm = NULL);
	void  Create(ctCapabilitiesStruct* pCap,int size);
 //   void  CreateWithDefaultVideoCaps(const CComMode& scm, const CCapH221& h221Cap, DWORD videoRate, const CComModeH323* pH323Scm);
	void  CreateWithDefaultVideoCaps(DWORD videoRate, CComModeH323* pH323Scm, const char* pPartyName, eVideoQuality vidQuality, BYTE isRecordingLink = FALSE, DWORD serviceId=0,ECopVideoFrameRate highestframerate = eCopVideoFrameRate_None, BYTE maxResolution = eAuto_Res);
	void  BuildCapsWithSpecialCaps(DWORD videoRate, CComModeH323* pH323Scm, const char* pPartyName,
											BYTE isRemoveGenericAudioCaps, BYTE isRemoveH239, BYTE isFixContentProtocol, BYTE isRemoveAnnexQ, BYTE isRemoveRvFecc,
											BYTE isRemoveGenericVideoCap, BYTE isRemoveG722,
											BYTE isRemoveH264, BYTE isRemoveDBC2, BYTE isRemoveOtherThenQCif, BYTE isRemoveG7221C,
											BYTE isRemoteMgcWithLowRateConf = FALSE, BYTE isRemoveLpr = FALSE, BYTE isCascadeOrRecordingLink = FALSE,
											BYTE IsRemoveEPC = FALSE,BYTE isRemoveG7231 = FALSE,BYTE isRemoveG719 = FALSE, DWORD serviceId=0,ECopVideoFrameRate highestframerate = eCopVideoFrameRate_None,CCopVideoTxModes* pCopVideoTxModes = NULL, BYTE isRemoveDPTR = FALSE);
    void  CreateFromOtherSupportedVideo(const CCapH323* other, APIS32 minVideoRate);
//	WORD  SetAudioOnlyCap(const CComMode& scm);
	WORD  SetAudioOnlyCap(const CComModeH323* pScm, const char* pPartyName);
//	void  CreateAudioOnlyCap(const CComMode& scm);
	void  CreateAudioOnlyCap(DWORD videoRate, const CComModeH323* pScm, const char* pPartyName);
//	void  CreateAudioDataCap(const CComMode& scm,const CCapH221& h221Cap);

	// Operations
	ctCapabilitiesStruct* GetCapStructPtr() const { return m_pCap;}
	CCapH323& operator=(const CCapH323& other);

	// API
    	virtual const char*  NameOf() const;
	void  Serialize(WORD format,CSegment& seg);
	void  DeSerialize(WORD format,CSegment& seg);
	void  SerializeCapArrayOnly(CSegment& seg,BYTE bOperUse);
//	void  GenerateH221Caps(CCapH221* pH221Cap, BYTE bGateWay = FALSE, cmCapDirection direction = cmCapReceiveAndTransmit) const;
	int   SetVideoBitRate(int newBitRate, ERoleLabel eRole = kRolePeople, CapEnum protocolToChange = eUnknownAlgorithemCapCode);
	DWORD GetMaxVideoBitRate(CapEnum videoType,cmCapDirection eDirection,ERoleLabel eRole = kRolePeople) const;
    //DWORD GetMaxVideoBitRateByH320Protocol(BYTE h320VideoProtocol, cmCapDirection eDirection,ERoleLabel eRole= kRolePeople) const;
	DWORD GetMaxContentBitRate() const;
	DWORD GetMaxDuoBitRate() const;
	BYTE  GetMaxContentOrDuoRateInAMSCValue() const;
	BYTE  GetMaxContentRateInAMSCValue() const;
	BYTE  GetMaxDuoRateInAMSCValue() const;
	int   GetMaxAudioFramePerPacket(CapEnum audioType,BYTE implicitCheck = FALSE) const;
	int   GetMinAudioFramePerPacket(CapEnum audioType) const;
	int   GetFormatMpiAndLocation(CapEnum videoType, EFormat eFormat, cmCapDirection eDirection, APIS8& formatMpi) const;
	WORD  GetCapTotalLength(void) const;
	WORD  IsSupportPeopleAndContent() const {return m_bIsEPC || m_bIsPCversion0;}
	WORD  IsEPC()  const {return m_bIsEPC;}
	BYTE  IsDBC2() const;
	WORD  CheckProfile(int profile) const;
	BYTE  IsH239() const {return m_bIsH239;}
	WORD  IsFECC() const;
	BYTE  IsContentRateSupported(BYTE h320RateAMSC) const;
	BYTE  IsDuoRateSupported(BYTE h320RateAMSC) const;
	BYTE  IsContentOrDuoRateSupported(BYTE h320RateAMSC) const;
	void  Set4CifMpi(DWORD videoRate, eVideoQuality vidQuality);
	void  Set4CifMpi(APIS8 mpi){m_h263_4CifMpi = mpi;}
	DWORD GetMaxFsAccordingToProfile(APIU16 profile);
	long GetMaxMbpsAccordingToProfile(APIU16 profile);
	WORD   TransferRemoteCapsToRemoteTxModeAndRemove(CCopVideoTxModes* pCopRemoteVideoTxModes);

	WORD  OnTypeRole(cmCapDataType eType,ERoleLabel eRole) const;
	BYTE  OnType(cmCapDataType type) const;
	WORD  OnCap(WORD cap, BYTE implicitCheck = FALSE) const;
	WORD  IsAdvanceAudioCodecSupported(WORD cap,BYTE implicitCheck) const;
	int   GetMaxRateOfAdvanceAudioCodecSupported(WORD cap,BYTE implicitCheck = FALSE) const;
	void  Dump(const char* title, WORD level) const;
	void  Dump(std::ostream & msg) const;
	WORD  GetNumOfAudioCap() const {return m_numOfAudioCap;}
	WORD  GetNumOfVideoCap() const {return m_numOfVideoCap;}
	WORD  GetNumOfContentCap() const {return m_numOfContentCap;}
	WORD  GetNumOfT120Cap()  const {return m_numOfT120Cap; }
	WORD  GetNumOfFeccCap() const {return m_numOfFeccCap;}
    WORD  GetNumOfDataCap() const {return  m_numOfT120Cap +  m_numOfFeccCap;}
	WORD  GetNumOfPCCap()    const {return m_numOfPeopContCap; }
	WORD  GetNumOfCap() const {return m_numOfCaps;}
	BYTE* GetCapArray() {return m_capArray;}
	WORD  IsPeopleAndContent() const {return m_numOfPeopContCap;}
	WORD  IsH263Plus() const {return m_is263Plus;}
	BYTE  IsDropFieldCap();
	WORD  GetNumOfNsCap()    { return m_numOfNsCap; }
	BYTE  IsCapableOfHD720(ERoleLabel eRole = kRolePeople) const;
	BYTE  IsH264CapFound() const;
	BYTE  IsH263CapFound() const;
	BYTE  IsCapableOfHD1080() const;
	BYTE  IsCapableOfHD1080At60() const;
	BYTE  IsCapableOfHDContent1080() const;
	BYTE  IsCapableOfHD720At60() const;
	BYTE  IsCapableOfHDContent720() const;
	BYTE  IsCapableOf4CIF() const;
	eVideoPartyType GetCPVideoPartyType();

	APIS8 Get4CifMpi() const;
	void  SetLevelAndAdditionals(APIU16 profile, APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb ,APIS32 brAndCpb, APIS32 sar, APIS32 staticMB, ERoleLabel eRole);
	void  SetLevelAndAdditionals(H264VideoModeDetails& h264VidModeDetails, APIS32 sar,ERoleLabel eRole = kRolePeople);
    void  SetH263FormatMpi(EFormat format, APIS8 mpi, ERoleLabel eRole);

	BYTE  BuildComMode(CComModeH323* pTargetModeH323,WORD confKind,BYTE bIsDataOnly,BYTE bIsCheckVideoOnly);
	BYTE  BuildComMode(CCapH323* pLocalCapH323,WORD confKind, CComModeH323* pTargetModeH323,BYTE bIsCheckVideoOnly = 0);
	BYTE  LookForMatchingLocalAudioCap(CCapH323* pLocalCapH323,audioCapStructBase* pAudioCap,WORD& sortedEntry) const;
	BYTE  LookForMatchingLocalSirenLPRCap(CCapH323* pLocalCapH323,sirenLPR_CapStruct* pAudioCap,WORD& sortedEntry) const;
	BYTE  LookForMatchingLocalG7221CCap(CCapH323* pLocalCapH323,g7221C_CapStruct* pAudioCap,WORD& sortedEntry) const;
	BYTE  LookForMatchingLocalG723Cap(CCapH323* pLocalCapH323,g7231CapStruct* pG723Cap,WORD& sortedEntry) const;
	BYTE  LookForMatchingLocalH261Cap(CCapH323* pLocalCapH323,h261CapStruct* pH261Cap,WORD& sortedEntry, BYTE& numOfH261Found);
	BYTE  LookForMatchingLocalH263Cap(CCapH323* pLocalCapH323,h263CapStruct* pH263Cap,WORD& sortedEntry, BYTE& numOfH263Found,ERoleLabel label);
	BYTE  LookForMatchingLocalH264Cap(CCapH323* pLocalCapH323, h264CapStruct* pH264Cap, WORD& sortedEntry, BYTE& numOfH264Found);
	BYTE  LookForMatchingLocalH26LCap(CCapH323* pLocalCapH323,genericVideoCapStruct* pH26LCap, WORD& sortedEntry, BYTE& numOfH26LFound);
	BYTE  LookForMatchingLocalDataCap(CCapH323* pLocalCapH323,dataCapStructBase* pT120Cap,WORD& sortedEntry) const;
	void  CheckMatrix(int position,int capTypeCode,capBuffer *pCapBuffer);
	void  BuildSortedCap(void);
	void  ZeroingMatchListArray();
	void  AddNewEntry(int capTypeCode, BYTE sim ,int altNumber,capBuffer *pCapBuffer);
	void  ReleaseMatchList();
	localCap* GetSortedArray() {return m_sortedCap;}
	capBuffer *GetAudioMatch(CCapH323* pRmtCap) const;
	void  EnsureAudioTargetMode(CAudModeH323 &pXmitMode323,CCapH323* pRmtCap) const;
	void  EnsureVideoTargetMode(CVidModeH323 &pXmitMode323,CCapH323* pRmtCap) const;
	void  EnsureContentTargetMode(CVidModeH323 &pXmitMode323,CCapH323* pRmtCap) const;
	void  EnsureDataTargetMode(CDataModeH323 &pXmitMode323,CCapH323* pRmtCap) const;
	void  UpdateTargetMode(CAudModeH323 &pXmitMode323) const;
	void  ImproveTargetMode(CVidModeH323 &pXmitMode323,cmCapDirection direction) const;
	BYTE  FindMatching(CComModeH323 *pTargetModeH323,WORD confKind,BYTE bIsDataOnly,BYTE bIsCheckVideoOnly = 0) const;
	void  AddNewEntry(CVidModeH323 *pCapBuffer,candidateList **ppHead) const;
	void  ReleaseLinkedList(candidateList **ppHead) const;
	CMediaModeH323  *CheckTheBestCap(candidateList &pHead, int type) const;
	int  LookForVideoPair(BYTE audiosim,CComModeH323 *pInitial323,int confKind) const;
	int  LookForDataPair(BYTE audiosim,CComModeH323 *pInitial323) const;
	int	 LookVideo(BYTE audiosim,CComModeH323 *pInitial323,int altNumber,ERoleLabel label,int confKind) const;
	BYTE IsVideoCapCodeMatch(CCapH323 *pRmtCap) const;
	BYTE HandleH263Pair(CComModeH323* pComModeH323,h263CapStruct *pRemoteH263Cap,ERoleLabel label,WORD confKind,
							  APIS32 maxBitRate,h263CapStruct *pInitiateH263Cap=NULL) const;
	CVidModeH323 *LookForMatchingH261Pair(CComModeH323 *pInitial323,DWORD *pRmtCap,ERoleLabel label,WORD confKind) const;
	CVidModeH323 *LookForMatchingH263Pair(CComModeH323 *pInitial323,DWORD *pRmtCap,ERoleLabel label,WORD confKind) const;
	CVidModeH323 *LookForMatchingH26LPair(CComModeH323 *pInitial323,DWORD *pRmtCap,ERoleLabel label,WORD confKind) const;
	CVidModeH323 *LookForMatchingH264Pair(CComModeH323 *pInitial323,DWORD *pRmtCap,ERoleLabel label,WORD confKind) const;
	WORD  IsExplicit() const;
	int	  FindAltNumber(int position,CapEnum capCode) const;
	int	  FindAltNumber(cmCapDataType capType,ERoleLabel label) const;
	int	  FindAltNumber(CapEnum capCode,ERoleLabel label) const;
	void  SetPeopleContentAlt();
	BYTE  CheckRole(ERoleLabel eRole,int index,CBaseCap *pCap)const;
	BYTE  LookForDataPair(BYTE audiosim,CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323) const;
	BYTE  LookForVideoPair(BYTE audiosim,CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323,WORD confKind);
	BYTE  LookFor261VideoPair(localCap localSortedCap[],int index,BYTE audiosim,CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323,WORD confKind);
	BYTE  LookFor263VideoPair(localCap localSortedCap[],int index,BYTE audiosim,CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323,WORD confKind,int peopleAltNumber,ERoleLabel label);
	BYTE  LookFor264VideoPair(localCap localSortedCap[],int index,BYTE audiosim,CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323,WORD confKind);
	BYTE  LookFor26LVideoPair(localCap localSortedCap[],int index,BYTE audiosim,CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323,WORD confKind);
	BYTE  FindMatchingCap(CCapH323* pLocalCapH323,CComModeH323* pTargetModeH323,WORD confKind,BYTE bIsCheckVideoOnly = 0);
	void  AddNewEntry(BYTE sim ,WORD localMatchingIndex,capBuffer *pCapBuffer);
	BYTE  IsChannelsParamsOK (const channelSpecificParameters * pParams, payload_en payloadType, CSecondaryParams &secParams,const char * strChannelName = NULL,BYTE isAnnexes = FALSE) const;
	void  UpdateNumOfCaps(CapEnum algorithmCapCode,ERoleLabel eRole = kRolePeople);
	WORD  RemoveProtocolFromCapSet(payload_en payloadType);
	WORD  RemoveProtocolFromCapSet(CapEnum capEnum);
	void  RemoveOtherProtocols(CapEnum h323CapCode, ERoleLabel eRole = kRolePeople);
	void  RemoveOtherFormats(EFormat eWantedFormat, CapEnum h323CapCode = eUnknownAlgorithemCapCode, ERoleLabel eRole = kRolePeople);
	void  SetFormatsMpi(CapEnum protocol, ERoleLabel eRole, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi);
	void  Remove4CifFromH263VideoCap();
	void  SetMaxDataRate(DWORD newRate) {m_dataMaxBitRate = newRate;}
	DWORD GetMaxDataRate() const {return m_dataMaxBitRate;}
	DWORD GetMaxContRate() const{return m_maxContRate;}
	DWORD GetMaxContTdmRate() const {return m_maxContTdmRate;}
    void  ReArrangeAudioCaps(DWORD audioRate, DWORD confRate, DWORD videoRate);
	void  ReBuildCapsWithAddedSpecificCodecs(DWORD audioRate, DWORD confRate, DWORD videoRate, int numOfAddedCaps, ...);
	const BaseCapStruct* GetAudioDesiredMode() const;
	DWORD GetAudioDesiredRate() const;
	BYTE FindSecondBestCap( CCapH323 *pRmtCapH323, CapEnum &h323OutCapCode, cmCapDataType eType = cmCapVideo);
	BYTE IsECS()  const;
	WORD  GetDynamicPTRepControl(){return m_bIsDPTR;}


	//Highest Common:
	const BaseCapStruct* GetBestCapStruct(CapEnum videoType, cmCapDirection eDirection, EFormat minFormat, ERoleLabel eRole, BYTE bCheckProfile) const;
    const capBuffer* GetFirstMediaCapBuffer(cmCapDataType dataType, ERoleLabel eRole) const;
    DWORD GetFirstMaxVideoBitRate();
//	BYTE IsMediaContaining(CComModeH323* pScm, BYTE valuesToCompare, cmCapDataType dataType,
//							cmCapDirection direction, ERoleLabel eRole) const;
	BYTE AreLocalCapsContaining(CComModeH323* pScm, DWORD valuesToCompare, cmCapDataType dataType,
					cmCapDirection direction, ERoleLabel eRole) const;

	BYTE AreRemoteCapsContaining(CComModeH323* pScm, DWORD valuesToCompare,
					cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const;
	// Added for change mode
	BYTE AreLocalCapsEqual(CComModeH323* pScm, DWORD valuesToCompare,
							cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const;
	BYTE AreRemoteCapsEqual(CComModeH323* pScm, DWORD valuesToCompare,
					cmCapDataType dataType, cmCapDirection direction, ERoleLabel eRole) const;
	BYTE IsEqual(const CMediaModeH323& rScmMediaMode, DWORD valuesToCompare, cmCapDirection direction, ERoleLabel eRole) const;
	void BuildNewCapsFromComModeAndCaps(const CComModeH323& pNewScm, DWORD confType,WORD numOfFecc,CapEnum feccMedaiType, const CCapH323* pOtherCaps = NULL, DWORD serviceId=0, ECopVideoFrameRate highestframerate = eCopVideoFrameRate_None);
	void CopyNoneAudioCaps(CCapH323* pOldCap);
	void CopyCaps(CCapH323 *pCap, cmCapDataType dataType, WORD numOfCap, ERoleLabel role = kRolePeople);
	void CopyCaps(CCapH323 *pCap, CapEnum capEnum, WORD numOfCap, ERoleLabel role = kRolePeople);
	void CopyCaps(CCapH323 *pCap,cmCapDataType dataType,CapEnum capEnum, WORD numOfCap, ERoleLabel role = kRolePeople);
	CBaseVideoCap* FindIntersectionBetweenCapsAndVideoScm(CComModeH323* pScm, ERoleLabel eRole = kRolePeople, BYTE bCheckRate = FALSE)  const;
	CBaseVideoCap* FindIntersectionBetweenTwoCaps(CCapH323* pOtherCaps, CapEnum protocol,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople, BYTE bCheckRate = FALSE);
	CBaseVideoCap* FindIntersectionBetweenTwoCaps(CBaseVideoCap* pRmtCap, CapEnum protocol,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople, BYTE bCheckRate = FALSE);
	void IntersectHighestCommon(const CCapH323& other, WORD& bIsHighestCommonParamsChanged, ERoleLabel eRole = kRolePeople);
	WORD IsHighestCommonConditionSupported(CHighestCommonCondition& hcCondition, ERoleLabel eRole = kRolePeople);
    BYTE AreCapsSupportVideoProtocolAndRate(CapEnum protocol, DWORD rate, ERoleLabel role = kRolePeople);
	BYTE AreCapsSupportProtocol(CapEnum protocol,cmCapDataType eType,ERoleLabel role = kRolePeople, APIS32 H264mode = 1, APIU16 H264profile = 0) const;
	BYTE GetMaxH264Level(ERoleLabel eRole);
    void GetMaxH264CustomParameters(BYTE level, WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile = 0);
    void GetMaxH264StaticMB(WORD& maxStaticMB, ERoleLabel eRole);
    WORD GetMaxMpi(WORD protocol, WORD resolution, char& maxMpi, ERoleLabel eRole = kRolePeople);
    WORD GetMaxH263Annexes(WORD resolution, BYTE& bIsAnnexF, BYTE& bIsAnnexT, BYTE& bIsAnnexN, BYTE& bIsAnnexI_NS, ERoleLabel eRole = kRolePeople);
	WORD IsNsAnnexI();
	BYTE IsAnnex(annexesListEn annex, ERoleLabel eRole = kRolePeople);
    WORD IsVidImageFormat(WORD protocol, EFormat format, ERoleLabel eRole = kRolePeople) const;
	APIU8 GetMpi(WORD protocol, EFormat format, ERoleLabel eRole = kRolePeople);
	//void  SetRestric(const CComMode& scm);
	//BYTE  GetRestric() const;
	void SetEncryptionAlg(EenMediaType encrypAlg) {m_encAlg = encrypAlg;}
	EenMediaType GetEncryptionAlg() const {return m_encAlg;}
	BYTE IsPartyEncrypted() const;
	BOOL IsEncrypedCap(int index) const;

	int  GetEncryptedPosition(int position,capBuffer **ppCapBuffer=NULL) const;
	void UpdateEncryptedCaps(int encryptedPosition,capBuffer* pCapBuffer);
	void FixEncryptedCapsBackward (int position);
	void SetEncryptionAlgAccordingToScm(EenMediaType encMediaTypeAlg);
	void GetVideoMode(CMediaModeH323& pXmitMode323, CapEnum mediaType, cmCapDirection direction);

	DWORD UpdateCorrectVideoRateAfterRemovingGenericCap(DWORD confRate);

	DWORD GetPreferedAudioRateAccordingToVideo(DWORD vidRate);

	void RemoveSpecificDataTypeFromCaps(cmCapDataType dataType);
    BYTE 	IsContaining(const CMediaModeH323& rScmMediaMode, DWORD valuesToCompare, cmCapDirection direction, ERoleLabel eRole) const;
    void FindBestVidTxModeForCop(CCopVideoTxModes* pCopVideoTxModes, CComModeH323* pScm, BYTE definedProtocol, DWORD definedRate) const;
    void FindBestVidTxModeForCopThatMatchesWithRemoteTxMode(CCopVideoTxModes* pCopVideoLocalTxModes,CCopVideoTxModes* pCopVideoRemoteTxModes, CComModeH323* pScm, BYTE definedProtocol, DWORD definedRate) const;
    BYTE FindBestVidTxModeForCopLecturerLink(CCopVideoTxModes* pCopVideoTxModes, CComModeH323* pScm, WORD copLecturerLevelIndex,BYTE definedProtocol, DWORD definedRate) const;
    WORD AddVidTxModesForCop(CCopVideoTxModes* pCopVideoTxModes);
	//26.12.2006 Changes by VK. Stress Test
	void SetStressTestSpecificCaps(CapEnum eAudioCap, CapEnum eVideoCap, DWORD dwConfRate);
	WORD SetVideoCapStressTest(CapEnum videoprotocol, DWORD videoRate, WORD Mpi);
	void RemovePeopleCapSet(CapEnum capEnum);

	BYTE IsFoundOrH263H261();

	BYTE  IsLPR() const {return m_bIsLpr;}
	CLprCap* GetLprCapability(CapEnum protocol);
    void SetSingleVideoProtocolIfNeeded (BYTE protocol);
    void SetVideoCapsExactlyAccordingToScm(const CComModeH323* pScm);
    const capBuffer* GetFirstMediaCapBufferAccording2CapEnum(CapEnum protocol, ERoleLabel eRole = kRolePeople) const;
    BYTE IsSupportH264HighProfile() const;
    BOOL IsH264HighProfileContent() const;
    BOOL IsH264BaseProfileContent() const;  //HP content
    BYTE IsSupportAnnexQ() const;

    //TIP:
    BYTE IsTipCompatibleContentSupported() const;

    WORD    RemoveTransmitCapsFromCapBuffer();
#ifdef __H323_SIM__
	void SetSpecificCaps();
	WORD SetVideoCap(CapEnum videoprotocol, DWORD videoRate, WORD Mpi);
#endif// __H323_SIM__


	// mix mode
	void UpdateCapsForHdVswInMixedMode(const CComModeH323* pScm, const VideoOperationPoint*  pOperationPoint);   //FSN-613: Dynamic Content for SVC/Mix Conf

protected:


	// Attributes
	WORD										m_numOfCaps;
	WORD										m_offsetWrite;
	WORD										m_size;
	ctCapabilitiesStruct*						m_pCap;
	BYTE										m_capArray[MAX_CAP_TYPES];
	WORD										m_numOfAudioCap;
	WORD										m_numOfVideoCap;
	WORD                                        m_numOfContentCap;
	WORD                                        m_numOfDuoCap;
	WORD										m_numOfT120Cap;
	WORD										m_numOfFeccCap;
	WORD										m_numOfPeopContCap;
	WORD										m_numOfH239ControlCap;
	WORD										m_numOfDynamicPTRControlCap; /* Dynamic Payload type replacement */
	WORD										m_numOfNsCap;
	WORD										m_numOfEncrypCap; //In case of encryption cpnf
	EenMediaType								m_encAlg;
	int											m_contentAltNumber;
	int											m_peopleAltNumber;
	localCap									m_sortedCap[2*MAX_CAP_TYPES];
	capMatchList*								m_capMatchList[MAX_CAP_TYPES];
	BYTE										m_is263Plus;
    DWORD                                       m_dataMaxBitRate;
	// EPC
	DWORD                                       m_maxContRate; //this is the max rate in the conf
	DWORD                                       m_maxContTdmRate;
	WORD										m_bIsH239;
	WORD										m_bIsDPTR; /* Dynamic Payload Type Replacement Capability */
	WORD										m_bIsEPC;
	WORD										m_bIsPCversion0;//people&content version 0
//	BYTE										bIsRestricted;
	BYTE										m_bIsDBC2;
	APIS8										m_h263_4CifMpi; // -1 if disabled
	eVideoQuality								m_videoQuality; // needed for video cards mpi values settings;

	WORD										m_numOfLprCap;
	WORD										m_bIsLpr;


	// internal use
	void  AllocateNewBuffer(capBuffer* pCapBuffer, WORD len);
	WORD  SetAudio(const CComModeH323* pScm, DWORD videoRate, const char* pPartyName,BYTE isRemoteMgcWithLowRateConf , BYTE isRemoveGenericAudioCaps = FALSE, BYTE isRemoveG722 = FALSE, BYTE isRemoveG7221C = FALSE, BYTE isRemoveG729 = FALSE, BYTE isRemoveG723 = FALSE,BYTE isRemoveG719= FALSE, BYTE isRemoveSirenStereo=FALSE);//BRIDGE-12398 isRemoveSirenStereo
	WORD  SetAudioAccordingToRate(DWORD audioBitRate, DWORD confBitRate, DWORD videoRate,BYTE isRemoteMgcWithLowRateConf, BYTE isRemoveGenericAudioCaps = FALSE, BYTE isRemoveG722 = FALSE,BYTE isRemoveG7221C = FALSE,BYTE isRemoveG729 = FALSE, BYTE isRemoveG723 = FALSE,BYTE isRemoveG719 = FALSE,BYTE isRemoveSirenLPR = FALSE, BYTE isRemoveSirenStereo = FALSE); //BRIDGE-12398 isRemoveSirenStereo
	WORD  SetAudioAccordingToPartyName(const char* pPartyName, BYTE isRemoveGenericAudioCaps = FALSE, BYTE isRemoveG722 = FALSE);
//	WORD  SetVideo(const CCapH221& h221Cap, DWORD videoRate,DWORD confRate, const CComMode &scm);
	WORD  SetContent(const CComModeH323* pScm, BYTE isRemoveH239, BYTE isFixContentProtocol,BYTE isRemoveEPC/*, BYTE bContetnAsVideo*/);
	WORD  SetRoleLabelCapCode(BYTE label);
	WORD  SetFecc(const CComModeH323* pScm, BYTE isRemoveAnnexQ = FALSE, BYTE isRemoveRvFecc = FALSE, DWORD serviceId=0);
	WORD  SetFeccCap(const CComModeH323* pScm, CapEnum dataType);
	WORD  SetPeopleContent(const CComModeH323* pScm, BYTE isRemoveContent,int version, int profile);
	WORD  SetContentProfile(int profile,cmCapDirection eDirection = cmCapReceive);
	WORD  SetH239Control(const CComModeH323* pScm, BYTE isRemoveH239 = FALSE);
	WORD  SetH239ControlCap(cmCapDirection eDirection = cmCapReceive);
	WORD  SetDynamicPTRepControl(const CComModeH323* pScm, BYTE isRemoveDPTR = FALSE);
	WORD  SetDynamicPTRepControlCap(cmCapDirection eDirection = cmCapTransmit);
	WORD  SetDropFieldCap();

	WORD  SetLpr(const CComModeH323* pScm, BYTE isRemoveLpr = FALSE);
	WORD  SetLprControlCap(cmCapDirection eDirection = cmCapReceive);

	void  EndOfCapsConstruction (WORD numOfAudioCap,WORD numOfVidCap,WORD numOfContentCap,WORD numOfDuoCap,WORD numOfT120Cap,WORD numOfFeccCap,WORD numOfPeopContCap, WORD numOfH239ControlCap, WORD numOfEncCap, WORD numOfNsCap = 0, WORD numOfLprCap = 0, WORD numOfDptrCap = 0);
	//WORD  SetVideoCapsFromH320Scm(const CComMode& scm, DWORD videoRate);
	//WORD  SetAudioCapsFromH320Scm(const CComMode& scm);
	void  BuildRemoteCapabilities(capBuffer* pCapRmtBuf);
	void  InitCapArrayStruct(int capTypeCode,char* dataCap,capHeadStruct* pEntry);
	WORD  GetCurrentCapsBufSize() const;
	WORD  SetNonStandardAnnex(annexesListEn annex = typeAnnexI_NS);
	WORD  SetEncryption();
//	WORD  SetFeccCap(const CCapH221& h221Cap, CapEnum dataType);//yael??
	WORD  SetAudioCap(CapEnum audCapType);
	BYTE  UpdateComModeMPI(int localH263CapMpi,int remoteH263CapMpi,WORD confKind,
        					capBuffer* pCapPtr, CComModeH323* pTargetModeH323, capMatchList* phead);
    void  UpdateComMode(CComModeH323* pTargetModeH323, WORD newType, WORD newDataLength, const BYTE newData[],ERoleLabel label=kRolePeople);
	BYTE  UpdateComModeCustomAnnexesMPI( CComModeH323* pTargetModeH323, capBuffer *pCapBuffer);

	DWORD   CompareParameters(BYTE *pFirstCap, BYTE *pSecondCap,DWORD valuesToCompare,CapEnum type) const;
	void   IntersectionParameters(CComModeH323 *pInitial323,BYTE *pFirstCap,BYTE *pSecondCap,CapEnum type,ERoleLabel label) const;
	BYTE   IntersectionH263Annex(CVidModeH323 *pXmitMode,h263CapStruct *pSecondH263Cap, h263CapStruct *pLocalH263Cap) const;
	void   AddOneCapToOther(CVidModeH323*pXmitInitiate,h263CapStruct *pFirstH263Cap,h263CapStruct *pSecondH263Cap,CapEnum type,ERoleLabel label) const;
	void   ImproveH263Parameters(CComModeH323 *pInitial323,h263CapStruct *pFirstH263Cap,h263CapStruct *pSecondH263Cap,ERoleLabel label) const;

	void	AddCapToCapBuffer(CapEnum algorithmCapCode, int structSize, BaseCapStruct* AlgorithmCapStructPtr);
	WORD    RemoveProtocolFromCapSet(payload_en payloadType, CapEnum capEnum);
	BYTE    RemoveEncryptedCapFromCapBuffer(int index);
	WORD	RemoveCapFromCapBuffer(CapEnum algorithmCapCode,ERoleLabel eRole = kRolePeople,BOOL bH263plus = FALSE,APIU16 profileToRemove = 0 );
	void    RemoveOneCapset(capBuffer* pCapBuffer, int capIndex, CapEnum algorithmCapCode);
//	WORD	SetH261Cap(const CCapH221& h221Cap, DWORD videoRate ,BOOL bOnlyOneResolution, BOOL ConfType);

	WORD    SetH261DefaultCap(DWORD videoRate,const char* pPartyName);
//  WORD    SetH263Cap(const CCapH221& h221Cap, DWORD videoRate,BOOL ConfType, const CComMode& scm);
  	WORD    SetH263DefaultCap(DWORD videoRate, EConfType confType, DWORD callRate, BYTE isRemoveOtherThenQCif = FALSE);
//  WORD    SetH264Cap(const CCapH221& h221Cap, DWORD videoRate, BOOL ConfType);
	WORD    SetH264DefaultCap(DWORD videoRate);
//	WORD	SetH26LCap(const CCapH221& h221Cap, DWORD videoRate ,BOOL ConfType);

	BOOL    CheckDBC2();
	WORD    SetDBC2Cap();
	BOOL    FindType (customPic_St *pFirstH263CustomSt, int typeOfFirst[H263_Custom_Number], int typeOfSecond, APIS32 secondMpi);
	void    BuildCapMatrixFromSortedCapBuf();
   	int     GetBestH264CapStructNum(cmCapDirection eDirection, ERoleLabel eRole, BYTE bCheckProfile) const;


	//Highest Common:
	WORD SetVideoCapsFromAnotherCaps(const CCapH323* pOtherCap, ERoleLabel eRole, BYTE bCanAddEPC = TRUE, DWORD confType = confTypeUnspecified);
	WORD SetVideoCapsFromH323Scm(const CComModeH323& rScm, ERoleLabel eRole, BYTE bCanAddEPC = TRUE, DWORD confType = confTypeUnspecified, BYTE isRemoveGenericVideoCap = FALSE, BYTE isRemoveOtherThenQCif = FALSE,ECopVideoFrameRate highestframerate = eCopVideoFrameRate_None, BYTE maxResolution = eAuto_Res);
	WORD SetMediaCapsFromH323Scm(const CComModeH323& rScm, cmCapDataType dataType,BYTE bIsFecc=FALSE,CapEnum mediaCap=eUnknownAlgorithemCapCode);

    BYTE InsertToResultsCapsIfNotContained(CBaseCap* pCandidateCap);
	WORD CreateDefaultVideoCaps(const CComModeH323* pH323Scm, BYTE bIsAutoProtocol, DWORD videoRate, const char* pPartyNam, BYTE isRemoveContent,
								BYTE isRemoveGenericVideoCap = FALSE, BYTE isRemoveH264 = FALSE, BYTE isRemoveOtherThenQCif = FALSE,
								BYTE isRemoveDBC2 = FALSE,ECopVideoFrameRate highestframerate = eCopVideoFrameRate_None,
								CCopVideoTxModes* pCopVideoTxModes = NULL, BYTE maxResolution = eAuto_Res);

	APIU32 GetPreferedAudioRate();

private:
	void  Initialize(int size = 0);
	void  SetNumOfCapsToZero();


};



#endif /* _H323CAPS  */



