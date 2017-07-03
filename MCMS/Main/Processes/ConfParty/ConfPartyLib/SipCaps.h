#ifndef _SIPCAPS
#define _SIPCAPS
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "CopVideoTxModes.h"
#include "SdesManagment.h"
#include "CopVideoTxModes.h"
#include "H264.h"
#include "SipUtils.h"
class CSegment;
class CComMode;
class CBaseCap;
class CSipComMode;
class CSipCall;
class CSdesCap;
class CSipChanDifArr;

#define MAX_MEDIA_CAPSETS 46
#define MAX_SDES_CAPSETS 		8

#define NumOfCandidates		20
#define MaxMsftSvcSdpVideoMlines		6
#define MaxMsftSvcSimulcastVideoMlines	5

#define NO_SESSION_LEVEL_RATE 0xFFFFFFFE


#include "ICEParams.h"

typedef enum
{
	eUseMkiKeyOnly,
	eUseNonMkiKeyOnly,
	eUseBothEncryptionKeys
} 	EEncryptionKeyToUse;


class CSipCaps: public CPObject
{
	CLASS_TYPE_1(CSipCaps,CPObject)

public:
	// Constructors
	CSipCaps();
	CSipCaps(const CSipCaps& other);
	virtual ~CSipCaps();

	void Create(const CIpComMode* pScm, DWORD videoLineRate, const char* pPartyName, eVideoQuality vidQuality,DWORD serviceId,DWORD encryptionKeyToUse, ECopVideoFrameRate highestframerate= eCopVideoFrameRate_None, BYTE maxResolution=eAuto_Res, BOOL bUseNonMkiOrderFirst=FALSE,BYTE Lync2013 = FALSE);
	void Create(const sipSdpAndHeadersSt& sdp, eConfMediaType aConfMediaType, bool aIsMrcCall, BYTE bIsRemoveCodec = TRUE, RemoteIdent aRemoteId = Regular);
	void CreateIgnoringRemoveCodec(const sipSdpAndHeadersSt & sdp, eConfMediaType aConfMediaType, bool aIsMrcCall);

	virtual const char* NameOf() const {return "CSipCaps";}
  	int GetNumOfCapSets() const {return m_numOfAudioCapSets + m_numOfVideoCapSets + m_numOfFeccCapSets + m_numOfContentCapSets + m_numOfBfcpCapSets;}
	int GetNumOfMediaCapSets(cmCapDataType eMediaType,cmCapDirection eDirection=cmCapReceiveAndTransmit, ERoleLabel eRole = kRolePeople) const;
	int GetNumOfSdesMediaCapSets(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	void CompleteDataFromOtherCap(const CSipCaps& defaultParams);
	void Complete2013DataFromOtherCapForMsSvc(const CSipCaps& defaultParams,DWORD rmtMsftRxBitRate);
	CBaseCap* GetCapSetAccordingToPayload(const CCapSetInfo& capInfo,WORD payload, ERoleLabel eRole = kRolePeople) const; //alloc memory

	int	CalcCapBuffersSize(cmCapDirection eDirection,BYTE bOperUse, BOOL fAddMediaLine = TRUE) const;
	int CalcCapBuffersSize(cmCapDataType eType,cmCapDirection eDirection,BYTE bOperUse, BOOL fAddMediaLine = TRUE, ERoleLabel eRole = kRolePeople) const;
	int	CalcSdesOnlyCapBuffersSize(cmCapDirection eDirection, BYTE bOperUse, BOOL fAddMediaLine = TRUE) const;
	int	CalcSdesOnlyCapBuffersSize(cmCapDataType eType, cmCapDirection eDirection, BYTE bOperUse, BOOL fAddMediaLine = TRUE, ERoleLabel eRole = kRolePeople) const;
	int CalcIceCapBufferSize(ICESessionsTypes SessionType, cmCapDirection eDirection) const;
	int	CalcCapBuffersSizeWithSdes(cmCapDirection eDirection,BYTE bOperUse, BOOL fAddMediaLine = TRUE) const;
	int CalcCapBuffersSizeWithSdes(cmCapDataType eType,cmCapDirection eDirection,BYTE bOperUse, BOOL fAddMediaLine = TRUE, ERoleLabel eRole = kRolePeople) const;
	int	CopyCapBuffersToBuffer(BYTE* buffer, int bufSize, int* pNumOfCaps, cmCapDirection eDirection, BYTE bSetOppositeDirection, BYTE bOperUse, int bufSdesSize, int* pNumOfSdesCaps, BOOL fAddMediaLine = TRUE) const;

	int	CopyCapBuffersWithSdesToBuffer(BYTE* buffer, int bufSize, int* pNumOfCaps, cmCapDirection eDirection, BYTE bSetOppositeDirection, BYTE bOperUse, BOOL fAddMediaLine = TRUE) const;
	int	AddCapsToCapStruct(cmCapDirection eDirection, BYTE bSetOppositeDirection, sipMediaLinesEntrySt* pMediaLinesEntry, int structSize, eMediaLineSubType bfcpTransportType,
							int addAudioCap = YES, int addVideoCap = YES, int addDataCap = YES, int addContentCap = YES, int addBfcpCap = YES, BOOL bOverrideSavpWithAvp = FALSE) const;
	int	AddSingleCapToCapStruct(cmCapDataType eMediaType, int arrIndex, cmCapDirection eDirection, BYTE bSetOppositeDirection, sipMediaLinesEntrySt* pCapabilities, int structSize, eMediaLineSubType bBfcpTransportType, ERoleLabel eRole = kRolePeople) const; // V4.1c <--> V6 merge const;
	int	AddMediaToCapStruct(cmCapDataType eMediaType,cmCapDirection eDirection,BYTE bSetOppositeDirection,sipMediaLinesEntrySt* pCapabilities,int structSize, eMediaLineSubType bfcpTransportType) const;
	DWORD     GetAudioDesiredRate();
	CBaseCap* GetCapSet(cmCapDataType eMediaType,int arrIndex = 0, ERoleLabel eRole = kRolePeople) const;	//alloc memory

	CBaseCap* GetCapSet(const CCapSetInfo& capInfo, int index, ERoleLabel eRole, APIU16 eProfile = 0) const;//alloc memory
	CBaseCap* GetSdesCapSet(cmCapDataType eMediaType,int arrIndex, ERoleLabel eRole) const;//alloc memory
	CBaseCap* GetHighestCommon(cmCapDirection eDirection, const CBaseCap& rOtherCap) const;		//alloc memory

	const capBuffer* GetCapSetAsCapBuffer(cmCapDataType eMediaType,int arrIndex, ERoleLabel eRole = kRolePeople) const;
	const capBuffer* GetCapSetAsCapBuffer(const CCapSetInfo& capInfo,int index=0, ERoleLabel eRole = kRolePeople) const;
	BYTE IsContainingCapSet(cmCapDirection eDirection, const CBaseCap& rCap, DWORD valuesToCompare, DWORD* pDetails, int* pArrInd) const;
	BYTE IsContainedInCapSet(const CBaseCap& rCap, DWORD valuesToCompare, DWORD* pDetails, int* pArrInd) const;
//	BYTE IsContainingEqualsCapSet(cmCapDirection eDirection,const CBaseCap& rCap,BYTE valuesToCompare,WORD* pDetails, int* pArrInd) const;
//	BYTE IsContainingScm(const CSipComMode* pScm) const;
//	BYTE IsContainingMode(const CSipComMode* pScm,cmCapDataType eType,cmCapDirection eDirection,BYTE valuesToCompare) const;
	BYTE IsContainingCapBuffer(cmCapDirection eDirection, const capBuffer& rCapBuffer, DWORD valuesToCompare, int* pArrInd,BYTE checkCapDirection) const;
	BYTE IsContainingCapBufferForSdes(cmCapDirection eDirection, const capBuffer& rCapBuffer, DWORD valuesToCompare, int* pArrInd) const;
	BYTE IsCapSet(const CCapSetInfo& capInfo, ERoleLabel eRole = kRolePeople) const;
	BYTE IsMedia(cmCapDataType eMediaType,cmCapDirection eDirection = cmCapReceiveAndTransmit, ERoleLabel eRole = kRolePeople) const {return (GetNumOfMediaCapSets(eMediaType,eDirection,eRole) != 0);}
	BYTE IsMediaMuted(cmCapDataType eMediaType,cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	BYTE GetDirection(cmCapDataType eMediaType, ERoleLabel eRole = kRolePeople, BOOL fullDirectionIfNotExist = FALSE) const;
	BYTE IsContainingPayloadType(const CCapSetInfo& capInfo, APIU8 uiPayloadType, int* pArrInd, ERoleLabel eRole = kRolePeople) const;
	payload_en GetPayloadTypeByDynamicPreference(const CCapSetInfo& capInfo, WORD profile, ERoleLabel eRole,APIS32 H264mode = H264_standard, APIU8 packetizationMode = 0) const;
	CapEnum FindAlgAccordingToPayload(cmCapDataType eMediaType, WORD payload, ERoleLabel eRole = kRolePeople) const;
	APIU16 FindH264ProfileFromPayload(cmCapDataType eMediaType, WORD payload) const;
	DWORD GetMaxFsAccordingToProfile(APIU16 profile, ERoleLabel eRole = kRolePeople);
	APIU8 FindH264PacketizationModeFromPayload(cmCapDataType eMediaType, WORD payload) const;
	APIS32 FindMaxFsFromPayload(cmCapDataType eMediaType, WORD payload) const;
	EResult GetRtvCapBitRateAccordingToResolution(DWORD Width,DWORD Height, DWORD& bitrate) const ;
	BYTE IsCapsContainingVideo() const;

	void Serialize(WORD format,CSegment& seg) const;
	void DeSerialize(WORD format,CSegment& seg);
	void SerializeCapArrayOnly(CSegment& seg, BYTE bOperUse);
	void AddCapSet(cmCapDataType eMediaType, const capBuffer* pCapSet);
	void AddCapSet(cmCapDataType eMediaType, const CBaseCap* pCap);
	void AddSdesCapSet(cmCapDataType eMediaType, const capBuffer* pCapSet, ERoleLabel eRole = kRolePeople);
	void AddDtlsCapSet(cmCapDataType eMediaType, const capBuffer* pCapSet, ERoleLabel eRole = kRolePeople);
	void AddSdesCapSet(cmCapDataType eMediaType, const CBaseCap* pCap, ERoleLabel eRole);
	void AddDtlsCapSet(cmCapDataType eMediaType, const CBaseCap* pCap, ERoleLabel eRole);
	CSdesCap* GetSdesCap(cmCapDataType eMediaType, ERoleLabel eRole, int arrIndex = 0) const;
	CSdesCap* GetSdesCapFromAlternate(cmCapDataType eMediaType,const CSipCaps& pAlternativeCaps, APIU16 cryptoSuite ,ERoleLabel eRole) const;
	CDtlsCap* GetDtlsCap(cmCapDataType eMediaType, ERoleLabel eRole, int arrIndex = 0) const;
	void RemoveCapSet(const CCapSetInfo& capInfo, ERoleLabel eRole = kRolePeople);
	capBuffer* RemoveCapSet(cmCapDataType eMediaType, int arrIndex, ERoleLabel eRole = kRolePeople);
	capBuffer* RemoveCapSetForSdes(cmCapDataType eMediaType, int arrIndex, ERoleLabel eRole = kRolePeople);
	void RemoveH264SpecifProfileCapSet(const CCapSetInfo& capInfo, ERoleLabel eRole,APIU16 profile);
	CSipComMode* FindBestMode(cmCapDirection eDirection, const CSipComMode& rPreferredMode, const CSipCaps& rAlternativeCaps, BYTE &bWithinProtocolLimitation, BYTE bIsOffere, BYTE bIsMrcSlave) const; //alloc memory
	CSipComMode* FindTargetMode(cmCapDirection eDirection, const CSipComMode& rPreferredMode) const; //alloc memory
	void CheckAndRemoveCapSetsWithInvalidPayloadType(BYTE &bRemovedAudio, BYTE &bRemovedVideo);

	void CreateWithDefaultCaps(DWORD videoRate, CIpComMode* pScm);
	void SetAudio(const CIpComMode* pScm, DWORD videoLineRate, BYTE isAudioOnly, const char* pPartyName);
	void SetVideo(const CIpComMode* pScm, DWORD videoLineRate,DWORD serviceId,ECopVideoFrameRate highestframerate = eCopVideoFrameRate_None, BYTE maxResolution=eAuto_Res);
	void SetFecc(const CIpComMode* pScm, const char* pPartyName, DWORD serviceId);
	void SetContent(const CIpComMode* pScm, const char* pPartyName);
	void SetBfcp(const CIpComMode* pScm, const char* pPartyName);
	void AddSingleAudioCap(CCapSetInfo capInfo, APIS32 bitrate = 0);
	void AddSingleVideoCapForVideoRelayAvc(CCapSetInfo capInfo, const CIpComMode* pScm, int videoLineRate,ECopVideoFrameRate highestframerate,const VideoOperationPoint*  pOperationPoint);
	void UpdateCapsForHdVswInMixedMode(CCapSetInfo capInfo, const CIpComMode* pScm, const VideoOperationPoint*  pOperationPoint);
	void AddSingleVideoCap(CCapSetInfo capInfo, const CIpComMode* pScm, int videoLineRate,ECopVideoFrameRate highestframerate = eCopVideoFrameRate_None, BYTE maxResolution=eAuto_Res, const VideoOperationPoint* pOperationPoint=NULL);
	void AddH264HPCap(const CIpComMode* pScm);
	void AddSingleFeccCap(const CIpComMode* pScm, CapEnum dataType);
	void AddSingleContentCap(const CIpComMode* pScm, CapEnum dataType, BYTE isH264HiProfile=FALSE); //HP content
	void SetAudioAccordingToPartyName(const char* pPartyName);
	void CleanMedia(cmCapDataType eMediaType, ERoleLabel eRole = kRolePeople);
	void CleanAll();

//	void SwitchMediaDirections();
	void DumpToString(CSuperLargeString& str) const;
	void DumpMediaToString(cmCapDataType eMediaType,CSuperLargeString& str, ERoleLabel eRole = kRolePeople) const;
	void DumpAVMCUVideoSdesCapsToString(CSuperLargeString& str) const;
//	void UpdateDynamicPayloadTypes(const CSipCall& call);

	DWORD GetMaxVideoBitRate(const CCapSetInfo& capInfo, ERoleLabel eRole = kRolePeople) const;
	DWORD GetMaxVideoBitRate(cmCapDirection eDirection = cmCapReceiveAndTransmit,ERoleLabel eRole = kRolePeople) const;

	int GetIndexInArr(const CCapSetInfo& capInfo,int index=0,ERoleLabel eRole = kRolePeople) const;
	int GetIndexInArrAccordingToCapSetAndProfile(const CCapSetInfo& capInfo,int index =0, ERoleLabel eRole = kRolePeople,APIU16 profile = H264_Profile_BaseLine) const;

 	BYTE FixRemoteCapsBySystemSettings(sipMessageHeaders* pSipHeaders, BYTE bFixVideoRate);
	BYTE FixFPSByUserAgent(const char* cUserAgent);

	//BYTE FixLocalCapsBySystemSettings(CComMode* pScm);
	BYTE FixVideoBitrateAndResolution();
	BYTE FixCIFDeclarationForCP();
	long GetTotalRate(void) const;

	BYTE IsCapableOfHD720() const;
	BYTE IsCapableOfHD1080() const;
	BYTE  IsCapableOfHDContent1080() const;
    BYTE  IsCapableOfHDContent720() const;
	BYTE IsSupportHighProfile() const;
	BYTE IsSupportBaseProfile() const;
	BYTE IsSupportMainProfile() const;
	BYTE IsHighProfileContent() const;
	BYTE IsCapableOfHD720At50() const;
	BYTE IsCapableOfHD1080At60() const;
	BYTE IsCapableOf4CIF() const;
	// FIELD 7091
		APIS32 GetRtcpFbMask(ERoleLabel role) const;

	eVideoPartyType GetCPVideoPartyType()const;
	APIS8 Get4CifMpi() const;
	void  Set4CifMpi(DWORD videoRate, eVideoQuality vidQuality);
	void  Set4CifMpi(APIS8 mpi){m_h263_4CifMpi = mpi;}

	void SetLevelAndAdditionals(H264VideoModeDetails& h264VidModeDetails, ERoleLabel eRole);
	void SetLevelAndAdditionalsForMainProfile(H264VideoModeDetails& h264VidModeDetails, ERoleLabel eRole);
	void SetNewProfileInsteadOfOldProfile(APIU8 newProfile,APIU8 oldProfile);
//	4.1C <--> 6 merge void SetLevelAndAdditionals(APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb ,APIS32 brAndCpb, APIS32 sar, APIS32 staticMB, ERoleLabel eRole);
	BYTE GetMaxH264Level(ERoleLabel eRole) const;
	CH264VideoCap* GetMaxH264LevelCap(ERoleLabel eRole) const;
    void GetMaxH264CustomParameters(BYTE level, WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile = 0 );
	void SetVideoRateInallCaps(DWORD newRate, ERoleLabel eRole = kRolePeople);
	DWORD GetVideoRateInMatchingCap(CBaseVideoCap* otherCap, ERoleLabel eRole = kRolePeople);
	void Reomve4cifFromCaps();
	void SetMpiFormatInCaps(EFormat format,BYTE val);

	CSipCaps&	operator=(const CSipCaps& other);
	WORD CheckValidPayloadType(APIU8 nPayloadType);

	// Siren14 Stereo
	void    DecideOnConfBitRateForAudioSelection(DWORD & confBitRate);
//	void ReduceRedundantAudioCaps();
	void RemoveAudioCapsAccordingToList();
	void RemoveCapsForTIPCall();
	void AddLprCap(ERoleLabel eRole, eConfMediaType aConfMediaType, bool aIsMrcCall);
	CLprCap* GetLprCap();
	BYTE GetIsLpr() const {return m_bIsLpr;};
	void AddSingleSdesCap(cmCapDataType eMediaType, BOOL bForceNoMki, APIU16 cryptoSuit, APIU32 sdesTag, ERoleLabel eRole = kRolePeople);
	void AddSdesCaps(cmCapDataType eMediaType, ERoleLabel eRole = kRolePeople);
	void AddSingleDtlsCap(cmCapDataType eMediaType, BOOL bForceNoMki, ERoleLabel eRole = kRolePeople);
	void SetTipDefaultsForEncrypt(BOOL& bIsNeedToSendMKI, BOOL& bLifeTimeInUse , BOOL& bFecKeyInUse);
	void UpdateSdesTag(cmCapDataType eMediaType,APIU32 tag, ERoleLabel eRole = kRolePeople);
	void UpdateSdesMasterSaltBase64Key(cmCapDataType eMediaType,char* key, BOOL bDisableDefaults, ERoleLabel eRole = kRolePeople);
	void GetSdesMasterSaltBase64Key(cmCapDataType eMediaType, char *pKey, int len, ERoleLabel eRole = kRolePeople);
	int GetSdesTag(cmCapDataType eMediaType, BOOL bIsTipMode, ERoleLabel eRole = kRolePeople, APIU16 pcryptoSuite = DEFAULT_CRYPTO_SUITE);

	//LYNC2013_FEC_RED:
	void AddFecCap(/*ERoleLabel eRole, eConfMediaType aConfMediaType, bool aIsMrcCall*/);
	void AddRedCap();
	BYTE GetIsFec() const {return m_bIsFec;};
	BYTE GetIsRed() const {return m_bIsRed;};
	//CFecCap* GetFecCap();
	//CRedCap* GetRedCap();

	BOOL GetSdesIsMkiInUseByTag(int tag , cmCapDataType eMediaType , ERoleLabel eRole);
	void UpdateSdesMkiInUseRx(CSipComMode* pBestMode ,APIU32 audioTag , APIU32 videoTag, APIU32 dataTag , APIU32 contentTag);
	void UpdateSdesMkiInUseTx(CSipComMode* pBestMode ,APIU32 audioTag , APIU32 videoTag, APIU32 dataTag , APIU32 contentTag);

	//int GetSdesCapEnumFromSystemFlag();

	CSipComMode* FindSdesBestMode(const CSipComMode& pPrefferedMode, const CSipCaps& pAlternativeCaps, const CSipComMode& pTempBestMode, BYTE bIsOffere) const;
	void UpdateSdesBestModeTransmitUnencryptedSrtcp(CSipComMode* pBestMode, CSdesCap *pSdesCap, cmCapDataType type, ERoleLabel eRole) const;
	BOOL IsSdesCapValid(CSdesCap *pSdesCap) const;
//	void UpdateSingleSdesCapSet(cmCapDataType eMediaType, const CBaseCap* pCap);
	BYTE IsSdesEquals(CSdesCap *pSdesCap,cmCapDataType eMediaType, cmCapDirection eDirection, ERoleLabel eRole = kRolePeople) const;
	//ICE
	BOOL SetICEsdp(const sipSdpAndHeadersSt* sdp, std::ostream &ostr,CSipChanDifArr* pIceChannelDifArr,DWORD opcode,BYTE IsSecondary, DWORD serviceId,BOOL IsEncryptedCall);
	BOOL BuildMLineCapsMsg(const capBuffer *pCapBuffer,std::ostream &ostr,ICESessionsTypes IceSessionsTypes,BOOL &IsFindRTCP,DWORD port,int& RtpCandidateNum);
	void BuildLocalIceSdp(char* pSdpString,DWORD sdpSize, CIceParams* pIceParams);
	void BuildLocalMSIceSdp(char* pSdpString,DWORD sdpSize, CIceParams* pIceParams);
	void AddICECapSet(ICESessionsTypes IceSessionsTypes, const capBuffer* pCapSet);

	//VNGFE-5845
	bool IsICESrvflxCandidate(const char* pCapBuffer , bool& isRtp , mcTransportAddress& addrIPSrvflx);
	bool SwitchICESrvflxIPAddr(const char* pInBuffer, const mcTransportAddress addrIPSrvflxToReplace, char* pOutBuffer);
	bool HandleSrflxCandidate(const char* inDataStr,char* outDataStr,ICESessionsTypes IceSessionsTypes );
	//BRIDGE-10631
	bool  FixSrflxCandidateType(const char* inDataStr,char* outDataStr);

	void HandleOneSdpLine(char* OneLineString,ICESessionsTypes IceSessionsTypes, CIceParams* pIceParams);
	void DumpIceCapsToString(CLargeString& str) const;
	CBaseCap* GetIceCapSet(ICESessionsTypes SessionType,int arrIndex) const;
	int GetNumOfIceAudioCaps();
	int GetNumOfIceVideoCaps();
	int GetNumOfIceDataCaps();
	int GetNumOfIceGeneralCaps();
	capBuffer* GetIceAudioCapList(WORD index);
	capBuffer* GetIceVideoCapList(WORD index);
	capBuffer* GetIceDataCapList(WORD index);
	capBuffer* GetIceGeneralCapList(WORD index);
	void AddSingleICECap(ICESessionsTypes IceSessionsTypes,CCapSetInfo capInfo,char* DataStr, APIS8 candidateType);
	int GetNumOfMediaIceCapSets(ICESessionsTypes SessionType) const;
	void DumpICEMediaToString(ICESessionsTypes SessionTyp,CLargeString& str) const;
    WORD  GetRtpPortNumberFromIceSdp(char* OneLineString);
	WORD  FindRtcpPortNumberFromIceSdp(char* OneLineString);
	void SetNumOfIceCapSets(int num, ICESessionsTypes IceSessionsTypes);
	void CleanIceMedia(ICESessionsTypes SessionType);
	 void CleanIceCapSets();
	 void AddRtcpAttribute(std::ostream &ostr,DWORD RtcpPort);
	 void FindRtcpPortInIceCaps(const capBuffer *pCapBuffer,int RtpCandidateNum,DWORD& RtcpPort);
	 void FindRtpAndRtcpPortsInCandidateHostLine(char* OneLineString,BYTE FindRtpPort,BYTE FindRtcpPort,DWORD RtpPort,int& RtpCandidateNum, DWORD& RtcpPort);

	 void SetHostIpAddress(ICESessionsTypes IceSessionsTypes,mcTransportAddress* HostIp);
	 void FindIpAddrInCandidateHostLine(char* OneLineString,ICESessionsTypes IceSessionsTypes, APIS8 candidateType);
	 void SwapPayloadTypes(const CCapSetInfo& capInfo,ERoleLabel eRole, WORD payload1, WORD profile1, WORD payload2);


//	 void CopyTmpArrToCaps(ICESessionsTypes IceSessionsTypes);
//	 void AddICECapToTmpArray(const capBuffer* pCapSet,char* DataStr);
//	 void FindFundationNumInLocalCandidateLine(char* OneLineString,BYTE& IsRtpLine,BYTE& IsRtcpLine,int& CandidateNum);
//	 void AddCapToRtpArr(int FundationNum,const capBuffer* pCapSet);
//	 void AddCapToRtcpArr(int FundationNum,const capBuffer* pCapSet);

	 mcTransportAddress* GetHostAudioIpAddr(){return &m_AudioHostPartyAddr; }
	 mcTransportAddress* GetHostVideoIpAddr(){return &m_VideoHostPartyAddr; }
	 mcTransportAddress* GetHostDataIpAddr(){return &m_DataHostPartyAddr; }

    //COP
    void FindBestVidTxModeForCop(CCopVideoTxModes* pCopVideoTxModes, CSipComMode* pScm, BYTE definedProtocol, DWORD definedRate) const;
    void FixUnkownProfileInCapsIfNeeded();

    void SetSingleVideoProtocolIfNeeded (BYTE protocol);
    void SetFormatsMpi(CapEnum protocol, ERoleLabel eRole, int qcifMpi, int cifMpi, int cif4Mpi, int cif16Mpi);
    void SetVideoCapsExactlyAccordingToScm(const CIpComMode* pScm);

	void CreatePartialAudioCaps();
	void CreatePartialAudioCapsForAudioOnly();//BRIDGE-11697

	void SetConfUserIdForBfcp(WORD confid, WORD userid);
	BYTE IsBfcpSupported() const;
	void SetFloorIdParamsForBfcp(char* pFloorid, char* pStreamid);
	void SetBfcpParameters(eBfcpSetup setup, eBfcpConnection connection, eBfcpFloorCtrl floorCtrl, eBfcpMStreamType mstreamType);
	eBfcpSetup GetBfcpSetupAttribute() const;
	eBfcpConnection GetBfcpConnectionAttribute() const;
	eBfcpFloorCtrl GetBfcpFloorCtrlAttribute() const;
	eBfcpMStreamType GetBfcpMStreamType() const;
	void SetBfcpTransportType(enTransportType transType);
	enTransportType GetBfcpTransportType() const;
	WORD GetBfcpUserId() const;
	WORD GetBfcpConfId() const;
	void GetBfcpFloorIdParams(int floorIndex, char* pFloorId, char* pStreamId0, char* pStreamId1, char* pStreamId2, char* pStreamId3) const;

	CapEnum GetBestContentProtocol() const;
	BYTE IsH239() const {return m_numOfContentCapSets;}

    void SetRtvParams(RTVVideoModeDetails& rtvVidModeDetails, ERoleLabel eRole,DWORD videoRate ,BYTE isForceFps = FALSE);

    void GetRtvCap(RTVVideoModeDetails& rtvVidModeDetails,DWORD& BitRate) const;
    void GetRtvCapFRAccordingToFS(DWORD& BitRate,DWORD FS);
	// TIP
	BYTE GetIsContainingCapCode(cmCapDataType eMediaType, CapEnum capCode) const;
	APIU16 GetH264ProfileFromCapCode(cmCapDataType eMediaType) const;
	long GetVideoRate(void) const;
	BYTE GetIsTipResolution() const;
	ETipAuxFPS GetTipAuxFPS() const {return m_TipAuxFPS;}
	void SetTipAuxFPS(ETipAuxFPS tipAuxFPS)  {m_TipAuxFPS = tipAuxFPS;}
	BYTE IsCapableTipAux5Fps() const;
	BYTE IsTipCompatibleContentSupported() const;
	void TipFixRemoteRateByTipNegotiation(WORD nTipStreams);
	void RemoveAudioCapsAccordingToRtvFlag(bool is_single_core);

	BYTE IsRemoteSdpContainsICE(const sipSdpAndHeadersSt* sdp);
	void SetNumOfSdesCapSets(int num, cmCapDataType eMediaType, ERoleLabel eRole);
	void CleanSdesMedia(cmCapDataType eMediaType, ERoleLabel eRole = kRolePeople);
	void CleanAVMCUSdesCaps();


	void SetSsrcIds(cmCapDataType eMediaType, cmCapDirection direction, ERoleLabel eRole, APIU32 *aSsrcIds, int aNumOfSsrcIds,bool isUpdate = false);
	void FindBestModeForSvc(cmCapDirection (&directionArr)[2], BYTE (&bResArr)[MAX_SIP_MEDIA_TYPES][2], CapEnum (&algFound)[MAX_SIP_MEDIA_TYPES],
			int &i, int &j, cmCapDirection eOppositeDirection, ERoleLabel eRole, cmCapDataType mediaType,
			const CSipComMode& rPreferredMode, const CBaseCap* pPreferredMedia, CSipComMode* pBestMode, BYTE bIsMrcSlave) const;
	void FindBestModeForAvcInVSWRelay(cmCapDirection (&directionArr)[2], BYTE (&bResArr)[MAX_SIP_MEDIA_TYPES][2], CapEnum (&algFound)[MAX_SIP_MEDIA_TYPES],
			int &i, int &j, cmCapDirection eOppositeDirection, ERoleLabel eRole, cmCapDataType mediaType,
			const CSipComMode& rPreferredMode, const CBaseCap* pPreferredMedia,const CSipCaps& rAlternativeCaps ,CSipComMode* pBestMode)const;
	CBaseCap* GetHighestRemoteAndLocalCaps(cmCapDirection eDirection, ERoleLabel eRole, cmCapDataType mediaType, const CBaseCap& rOtherCap,const CSipCaps& rAlternativeCaps) const;
	bool IsSimilarOperationPoints(const CSipComMode& rPreferredMode) const;
	bool IsSimilarOperationPoints(const VIDEO_OPERATION_POINT_SET_S* pOpStruct, const VIDEO_OPERATION_POINT_SET_S *pOtherOpStruct) const;
	void DumpOperationPointsStructs(const VIDEO_OPERATION_POINT_SET_S* pOpStruct, const VIDEO_OPERATION_POINT_SET_S *pOtherOpStruct) const;

	static bool IsH264Video(CapEnum capCode);
	void RemoveSdesCapsDifferentFromCryptoSuiteAndMKI(APIU16 cryptoSuite,  BOOL bIsMkiInUse, cmCapDataType mediaType, ERoleLabel eRole);
	void UpdateSdesTagFromBestMode(CSdesCap *mSdesCap, cmCapDataType mediaType, ERoleLabel eRole);
	void GetSdesMediaCaps(cmCapDataType eMediaType, int* pNumOfCaps, capBuffer*** ppMediaCapList, ERoleLabel eRole = kRolePeople) const;
	void RemoveSdesCaps(cmCapDataType mediaType, ERoleLabel eRole);
	BOOL IsContainingOperationPointForAvcVSWRelay(const VideoOperationPoint &operationPoint, EOperationPointPreset eOPPreset, eIsUseOperationPointsPreset isUseOperationPointesPresets) const;
	void FindBestOperationPointForAvcVSWRelay(cmCapDirection direction, CSipComMode* pBestMode) const;
	BYTE IsH263DynamicPayloadTypeOnly(ERoleLabel eRole) const;
	void ReplaceSendRecvStreams();
	void CreateIceFakeCandidates(UdpAddresses udpParams);
	CH264VideoCap* GetVideoCapFromOperationPoint(const VideoOperationPoint &operationPoint) const;

	//msft svc
	void setMsftSsrcAudio(DWORD ssrcAudio);
	void setMsftSsrcVideo(DWORD ssrcVideoFirst, DWORD ssrcVideoLast, int LineNum);
	DWORD getMsftSsrcAudio() const {return m_msftSsrcAudio;}
	DWORD getMsftSsrcVideoFirst(int LineNum) const {return m_msftSsrcVideo[LineNum-1][0];}
	DWORD getMsftSsrcVideoLast(int LineNum) const {return m_msftSsrcVideo[LineNum-1][1];}
	DWORD getMsftMsiVideo(int LineNum) const {return m_msftMsiVideo[LineNum-1];}


	DWORD GetMsftMsiAudio() { return m_msftMsiAudio; }
	void  SetMsftMsiAudio(DWORD msiAudio);
	void  SetMsftMsiVideo(DWORD msiVideo, int lineNum);
	DWORD getMsftRxVideoBwLimitation(){return m_msftVideoRxBw;}
	void  setMsftRxVideoBwLimitation(DWORD bw){m_msftVideoRxBw = bw;};
	BYTE  GetMsSvcVidMode(MsSvcVideoModeDetails& MsSvcDetails) const;
	BYTE  SetMsSvcVidMode(MsSvcVideoModeDetails& MsSvcDetails);

	CapEnum FindSirenLPRInCaps(cmCapDataType eMediaType, ERoleLabel eRole) const;
	void  ReplaceCapWithOtherCapByPayload(CapEnum otherCap, WORD payload);
	void  AddDSHForAvMcu();
	CLprCap* GetContentLprCap();

	CSdesCap* GetVideoSdesCapAVMCU(DWORD mLine) const; // alloc memory
	DWORD GetNumOfMsftAVMCUSdesCaps(){return m_numOfMsftAVMCUSdesCaps;};
	DWORD GetEncryptionKeyToUse(){return m_encryptionKeyToUse;}

	sdesCryptoSuiteEnum TranslateShaLength(eTypeOfSha1Length Sh1Length);
	void AddSdesCapsByMki(cmCapDataType eMediaType, ERoleLabel eRole, int Sh1Length, EEncryptionKeyToUse mkiEncryptionKeyToUse, BOOL bUseNonMkiOrderFirst,  int& nTagNum);
	static EEncryptionKeyToUse GetUseMkiEncrytionFlag(); //BRIDGE-10123

protected:
	int			  m_numOfAudioCapSets;
	capBuffer*	  m_audioCapList[MAX_MEDIA_CAPSETS];

	int			  m_numOfVideoCapSets;
	capBuffer*	  m_videoCapList[MAX_MEDIA_CAPSETS];
	APIS8		  m_h263_4CifMpi;
	eVideoQuality m_videoQuality; // needed for video cards mpi values settings;

	int			m_numOfFeccCapSets;
	capBuffer*	m_feccCapList[MAX_MEDIA_CAPSETS];

	int           m_numOfContentCapSets;
	capBuffer*    m_contentCapList[MAX_MEDIA_CAPSETS];

	int           m_numOfBfcpCapSets;
	capBuffer*    m_bfcpCapList[MAX_MEDIA_CAPSETS];

	BYTE        m_bIsLpr;

	//LYNC2013_FEC_RED:
	BYTE        m_bIsFec;
	BYTE        m_bIsRed;

	//SDES
	int			  m_numOfAudioSdesCapSets;
	capBuffer*	  m_audioSdesCapList[MAX_SDES_CAPSETS];

	int			  m_numOfVideoSdesCapSets;
	capBuffer*	  m_videoSdesCapList[MAX_SDES_CAPSETS];

	int			  m_numOfFeccSdesCapSets;
	capBuffer*	  m_feccSdesCapList[MAX_SDES_CAPSETS];

	int			  m_numOfContentSdesCapSets;
	capBuffer*	  m_contentSdesCapList[MAX_SDES_CAPSETS];

	//DTLS
	int			  m_numOfAudioDtlsCapSets;
	capBuffer*	  m_audioDtlsCapList[MAX_SDES_CAPSETS];

	int			  m_numOfVideoDtlsCapSets;
	capBuffer*	  m_videoDtlsCapList[MAX_SDES_CAPSETS];

	int			  m_numOfFeccDtlsCapSets;
	capBuffer*	  m_feccDtlsCapList[MAX_SDES_CAPSETS];

	int			  m_numOfContentDtlsCapSets;
	capBuffer*	  m_contentDtlsCapList[MAX_SDES_CAPSETS];

	//ICE
	int 		m_numOfIceAudioCaps;
	capBuffer*  m_IceAudioCapList[MAX_MEDIA_CAPSETS];
	mcTransportAddress  m_AudioHostPartyAddr;
	mcTransportAddress  m_AudioSrflxPartyAddr; //VNGFE-5845

	int 		m_numOfIceVideoCaps;
	capBuffer*  m_IceVideoCapList[MAX_MEDIA_CAPSETS];
	mcTransportAddress  m_VideoHostPartyAddr;
	mcTransportAddress  m_VideoSrflxPartyAddr; //VNGFE-5845

	int 		m_numOfIceDataCaps;
	capBuffer*  m_IceDataCapList[MAX_MEDIA_CAPSETS];
	mcTransportAddress  m_DataHostPartyAddr;
	mcTransportAddress  m_DataSrflxPartyAddr; //VNGFE-5845

	int 		m_numOfIceGeneralCaps;
	capBuffer*  m_IceGeneralCapList[MAX_MEDIA_CAPSETS];

//	capBuffer* 	m_RtpCandidateArr[NumOfCandidates];
//	capBuffer* 	m_RtcpCandidateArr[NumOfCandidates];


	mcXmlTransportAddress m_dummyMediaIp;

	ETipAuxFPS	m_TipAuxFPS; // Tip auxiliary FPS

	DWORD       m_sessionLevelRate;     //AS= line value in the session, coming from sipSdpAndHeadersSt in Create for callee, or coming from SCM callRate for caller.

	//APIU32      m_sessionLevelMsVideoRateTx;//in kbit unit max BW for Tx - for a=x-mediabw
	//APIU32      m_sessionLevelMsVideoRateRx;//in kbit unit max BW for Rx - for a=x-mediabw

	DWORD 		m_msftSsrcAudio;
	DWORD 		m_msftSsrcVideo[MaxMsftSvcSdpVideoMlines][2];

	DWORD 		m_msftMsiAudio;								// msi of audio media line Lync2013 call
	DWORD 		m_msftMsiVideo[MaxMsftSvcSdpVideoMlines];   // msi of each video media line Lync2013 call


	capBuffer*  m_msftAVMCUSdesCaps[MaxMsftSvcSdpVideoMlines]; // sdes caps of video mlines -AVMCU
	DWORD			m_numOfMsftAVMCUSdesCaps;

	DWORD       m_msftVideoRxBw; //this is the max video rate allowed to be sent to MS 2013 client.//there
	DWORD 		m_encryptionKeyToUse;
	BOOL 		m_bUseNonMkiOrderFirst;

	void SetNumOfCapSets(int num, cmCapDataType eMediaType, ERoleLabel eRole = kRolePeople);
	void GetMediaCaps(cmCapDataType eMediaType, int* pNumOfCaps, capBuffer*** ppMediaCapList, ERoleLabel eRole = kRolePeople) const;
	void GetMediaIceCaps(ICESessionsTypes SessionType, int* pNumOfIceCaps, capBuffer*** ppMediaIceCapList) const;

	CSdesCap* SetBestModeFromSdesCaplternate(const CSipCaps& pAlternativeCaps, APIU16 rCryptoSuite, cmCapDataType eMediaType,CSipComMode& pTempBestMode, ERoleLabel eRole) const;
	void GetDtlsMediaCaps(cmCapDataType eMediaType, int* pNumOfCaps, capBuffer*** ppMediaCapList, ERoleLabel eRole = kRolePeople) const;
	void SetLevelAndAdditionals(APIU16 profile, APIU8 level, APIS32 mbps, APIS32 fs, APIS32 dpb ,APIS32 brAndCpb, APIS32 sar, APIS32 staticMB, ERoleLabel eRole);

	BOOL ValidateOffset(int offsetWrite, int structSize, char *text) const;
	int AddTmpMediaLine(cmCapDirection eDirection, BYTE bSetOppositeDirection, sipMediaLineSt *pMediaLine, sipMediaLineSt *pTmpMediaLine,
							int structSize, int fAudio, int fVideo, int fData, int fContent, int fBfcp) const;
	int	AddCapsToCapStruct(cmCapDirection eDirection, BYTE bSetOppositeDirection, sipMediaLineSt* pMediaLines, int structSize,
							int addAudioCap = YES, int addVideoCap = YES, int addDataCap = YES, int addContentCap = YES, int addBfcpCap = YES) const;
	void CapSetBuffer(BYTE bSetOppositeDirection, CBaseCap *pCap, CCapSetInfo &capInfo, char *buffer, int capPos, int capEnum, int len) const;
	BOOL IsAudioCodecInParitalList(CapEnum dataType, bool audioOnly);
	eMediaLineSubType GetMLineSubType(char* pMLineString) const;
	BYTE GetVideoMLineSdesCapSetForAVMCU(DWORD lineIndex, const sipMediaLineSt *pMediaLine);


private:
	BYTE operator==(const CSipCaps& other) const; //in order not to use it
};

#endif
