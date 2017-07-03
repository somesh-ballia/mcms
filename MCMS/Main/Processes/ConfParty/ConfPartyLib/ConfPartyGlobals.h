//+========================================================================+
//                            ConfPartyGlobals.H                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConfPartyGlobals.H                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 1/24/96     |                                                     |
//+========================================================================+
#ifndef _CONFPARTYGLOBALS_H__
#define _CONFPARTYGLOBALS_H__


#include <stdint.h>
#include "ConfPartyDefines.h"
#include "CardsStructs.h"
#include "IpScm.h"
#include "Layout.h"
#include "ConfIpParameters.h"
#include "SystemFeatures.h"
#include "LookupTables.h"
#include "ConfAppMngr.h"
#include "IpRtpReq.h"
#include "CallAndChannelInfo.h"

#define ISACTIVEDEBUGMODE 1
#if ISACTIVEDEBUGMODE
	#define ISDEBUGMODE(feature, val) 						IsDebugModeStr(feature, val)
	#define ISDEBUGMODE_RETURN(feature, val) 				if(ISDEBUGMODE(feature, val)) return;
	#define ISDEBUGMODE_RETURN_STAT(feature, val, ret) 		if(ISDEBUGMODE(feature, val)) return ret;
	#define ISDEBUGMODE_SET_STATUS(feature, val, stat) 		if(ISDEBUGMODE(feature, val)) status=stat;
	#define ISDEBUGMODE_SET_VAL(feature, val, par, stat) 	if(ISDEBUGMODE(feature, val)) par=stat;
#else
	#define ISDEBUGMODE(feature, level) 				 false
	#define ISDEBUGMODE_RETURN(feature, level) 			 {}
	#define ISDEBUGMODE_RETURN_STAT(feature, level, ret) {}
	#define ISDEBUGMODE_SET_STATUS(feature, level, stat) {}
	#define ISDEBUGMODE_SET_VAL(feature, val, par, stat) {}
#endif

class CAVmsgServiceList;
class CIVRSlidesList;
class CComMode;
class CConfParty;
class CCommConf;
class CCommRes;
class CVideoOperationPointsSet;
class CRsrcParams;

//AVmsg Service database
extern CAVmsgServiceList* GetpAVmsgServList();
extern void SetpAVmsgServList(CAVmsgServiceList* p);

extern CIVRSlidesList* GetpSlidesList();
extern void SetpSlidesList(CIVRSlidesList* p);

//IVR Directories
extern const char* GetIVRFeatureDirectory(const WORD ivr_feature_opcode, int& status);
extern const char* GetIVREventDirectory(const WORD ivr_event_opcode, int& status);

// Video card caps
extern WORD Get261VideoCardMPI(DWORD bitRate, APIS8* qcifMpi, APIS8* cifMpi, eVideoQuality videoQuality);
extern WORD Get263VideoCardMPI(DWORD bitRate, BYTE* bVF);
extern WORD GetH264VideoHdVswParam(int& supportedProfile, int& supportedLevel, long& customMbps, long& customFS, long& customDpb, long& customBrAndCpb, long& customSar, long& staticMB, BYTE HDResolution);
extern void GetH264VideoTransmitParamForAsymmetricModes(int& supportedLevel, long& customMbps, long& customFS, long& customDpb, long& customBrAndCpb,Eh264VideoModeType h264VidModeType);
extern WORD GetMinimumHd720Fs();
extern WORD GetMinimumHd720At15Mbps();
extern WORD GetMinimumHd720At5Mbps();
extern WORD GetMinimumHd1080Fs();
extern WORD GetMinimumHd1080At15Mbps();
extern WORD GetMinimumHd1080At60Mbps();
extern WORD GetMinimumHd720At50Mbps();
extern WORD GetMinimumHd720At30Mbps();
extern void GetRtvVideoParams(RTVVideoModeDetails& rtvVidModeDetails, DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType resourceMaxVideoMode = eHD1080Symmetric);
extern void GetH264VideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType resourceMaxVideoMode = eHD1080At60Symmetric, BOOL isHighProfile = TRUE);
extern void GetH264AssymetricVideoParams(H264VideoModeDetails& h264VidModeDetails, DWORD callRate,eVideoQuality videoQuality, Eh264VideoModeType resourceMaxVideoMode = eHD1080At60Asymmetric, BOOL isHighProfile = TRUE);
extern void GetVSWH263VideoFormatCapBuf(WORD H263FormatNumber, BYTE* pH263CapSetBuf);

extern Eh264VideoModeType GetMaxVideoModeBySysCfg();
extern Eh264VideoModeType GetMaxVideoModeByResolutionType( EVideoResolutionType resType, eVideoQuality rsrvVidQuality, DWORD decisionRate = 0);
extern Eh264VideoModeType TranslateCPVideoPartyTypeToMaxH264VideoModeType(eVideoPartyType videoPartyType);
extern eVideoPartyType 	  GetMaxVideoPartyTypeByVideoQuality(eVideoPartyType videoPartyType, eVideoQuality rsrvVidQuality);

// Calculate rate functions for H323 calls
extern DWORD CalculateTdmRate (DWORD videoRate, BYTE bRoundDown = FALSE);// in default round up
DWORD ChangeVideoRateAccordingToPartyTypeIfNeeded(eVideoPartyType vidPartyType, DWORD videoRate);
extern int   CalculateRateForIpCalls(CConfParty* pConfParty, CIpComMode* pPartyScm, DWORD& confRate, DWORD& videoRate, BYTE bIsEncryption);
extern int   CalculateRateForSipOptions(CConfParty* pConfParty, CIpComMode* pPartyScm, DWORD& confRate, DWORD& videoRate, CCommConf* pCommConf);
extern int   ReCalculateRateForIpCpDialInCalls(CConfParty* pConfParty, CIpComMode* pPartyScm,
											  BYTE networkType, DWORD setupRate,
											  DWORD &confRate, DWORD &videoRate,BYTE audZeroRate = 0);
extern int CalculateReservationRates(CCommConf *pCommConf,DWORD& confRate,DWORD& audRate,DWORD& videoRate);
extern void  CalculateUniqueNumber (char *UniqueNumberString, DWORD signalingIpAddress);
extern void  AllocateRejectID (DWORD& RejectId);
extern void  DumpUniqueNumberToString(char *uniqueNumber,int size,const char* separatorBefore,const char* separatorAfter,CObjString& str);

// MCU INTERNAL PROBLEM
extern DWORD CalculateMcuInternalProblemErrorNumber(BYTE MipHardWareConn, BYTE MipMedia, BYTE MipDirection,
															BYTE MipTimerStatus, BYTE MipAction);
extern DWORD CalculateAudioRate(DWORD call_rate);
extern CapEnum CalculateAudioOpusCapEnum(DWORD audioRate);
extern DWORD CalculateAudioRateAccordingToVideoRateOfCopLevel(DWORD videorate);

extern BYTE  isIpVersionMatchBetweenPartyAndService(mcTransportAddress* partyAddr, CConfIpParameters* pServiceParams);

extern BYTE  FindIpVersionScopeIdMatchBetweenPartyAndService(const mcTransportAddress* partyAddr, CConfIpParameters* pServiceParams);
extern BYTE  FindPlaceAccordingtoScopeType(enScopeId scopeId,CConfIpParameters* pServiceParams);

extern BYTE  FindIpVersionScopeIdMatchBetweenPartySignalingAndMedia(const mcTransportAddress* partyAddr, ipAddressV6If* pUdpAddrArr);

// get Xfer rate
extern void GetXferBitrate(  DWORD& xfer_bitrate, CComMode workComMode);

extern BYTE  IsSirenAudioAlg(WORD audioMode);

extern BYTE IsNumeric(const char* string);
extern BYTE IsValidASCII(const char* buffToCheck = NULL,WORD buffLen = 0 ,const char* validate_set = "ASCII_PRINTABLE",BYTE check_null_end = YES);
extern BYTE IsValidStringUTF8(const char* string_to_check,const char* validate_set = "DEFAULT",BYTE print_error_status = YES);

extern BOOL IsValidRsrcID (DWORD dwRsrcID);
extern BOOL IsValidInterfaceType (WORD wInterfaceType);
extern BOOL IsValidAudioVolume (DWORD dwVolume);
extern BOOL IsValidAudioSampleRate (BYTE bAudioSampleRate);
extern BOOL IsValidNoiseDetectionThreshold (BYTE bAudioSampleRate);
extern BOOL IsValidMsftClientType(MSFT_CLIENT_ENUM eMsftClientType);
extern BOOL IsValidOpusCodecBitRate(DWORD maxAverageBitrate);

extern BOOL IsSendVinEnabledInMCU ();

ESTATUS TestEncryMoveValidity(BYTE partyEncryVal, BOOL bIsDefinedParty, BYTE destConfEncryType);
ESTATUS TestCopMoveValidity(CCommConf* pSourceConf, CCommConf* pDestConf);
ESTATUS TestMoveValidityAccordingToConfAndPartyMediaType(CCommConf* pCommConfSourceConf, CCommConf* pCommConfDestConf, CConfParty* pConfParty);
BOOL GetDongleEncryptionValue();
BOOL GetDongleFederalValue();
BOOL GetDonglePstnValue();
BOOL GetDongleTelepresenceValue();
BOOL GetDongleMsValue();
BOOL GetDongleSvcValue();
BOOL GetDongleCifPlusValue();
BOOL GetDongleTipInteropValue();
BOOL GetDongleLicenseExpiredValue();

void ResolveEncryptionParameters(BYTE eConfEncryptionType, const CConfParty* pConfParty, BYTE &resultShouldEncrypt, BYTE& resultShouldDisconnectOnEncryptFailure);
void IsForceAvpOnEncryptWhenPossibleFlag(BOOL& bIsForceAvpAllFlag , BOOL& bIsForceAvpCucmFlag);
void SetDongleEncryptionValue(BOOL dongleEncVal);
void SetDongleFederalValue(BOOL dongleEncVal);
void SetDonglePstnValue(BOOL donglePstnVal);
void SetDongleTelepresenceValue(BOOL dongleTelepresenceVal);
void SetDongleMsValue(BOOL dongleMsVal);
void SetDongleSvcValue(BOOL confPartyLicensing_svc);
void SetDongleCifPlusValue(BOOL confPartyLicensing_avc);
void SetDongleTipInteropValue(BOOL confPartyLicensing_TipInterop);
void SetDongleLicenseExpiredValue(BOOL isLicenseExpired);

ESTATUS TestHDVSWMoveValidity(BYTE destConfHDVal,BYTE SourceConfHDVal,BYTE destConfRate,BYTE SourceConfRate,EHDResolution destConfHdVSWRes,EHDResolution sourceConfHdVswRes);
ESTATUS TestDestConfMoveValidity(DWORD dwTargetConfId,BYTE isAudioOnlyParty);
ESTATUS TestMoveValidity(DWORD dwTargetConfId,DWORD dwSourceConfId,CConfParty* CConfParty,bool isMoveByOperator = true);
ESTATUS TestLPRMoveValidity(BYTE PartyinterfaceType,BYTE SourceConfLPRVal ,BYTE destConfLPRVal,BYTE IsHDVSW);
ESTATUS TestH264ContentValidity(BYTE SourceConfContentProtocolType ,BYTE destConfContentProtocolType);

extern DWORD GetVisualNameCounter();
extern void IncreaseVisualNameCounter();
extern STATUS IsVisualNameConflict(const char* confName ,char * new_name,BYTE isEQConf, DWORD partyID);
extern void GetUpdatedVisualName(const char* confName, char* visualName,char* updatedVisualName);
extern void GetUpdatedVisualNameForPartyInEQ(char* visualName,char* updatedVisualName);

// Fips140 functions
extern eConfPartyFipsSimulationMode TranslateSysConfigDataToEnumForConfParty(std::string& data);

extern LPRParams*              lookupLprParams(unsigned int protection, unsigned int mtbf, unsigned int packetLen, unsigned int rate);

extern eSystemCardsMode        GetSystemCardsBasedMode();
extern eVideoPartyType         GetCPH264ResourceVideoPartyType(DWORD MaxFS, DWORD MaxMBPSBOOL, BOOL isRtv = false, BYTE IsRsrcByFs = FALSE);
eVideoPartyType 			   GetCPHVP8ResourceVideoPartyType(DWORD MaxFS, DWORD MaxMBPS, DWORD FR, BYTE IsRsrcByFs = FALSE);
extern eVideoPartyType         GetCpH2634CifVideoPartyType();
extern eVideoPartyType         GetH264CifResourcesPartyType();
extern eVideoPartyType         GetH261H263ResourcesPartyType(BYTE is4CIF);

extern BOOL                    IsH2634Cif15PreferedOverH264InSharpnessConf(DWORD H264MaxFS, DWORD H264MaxMBPS);
extern LayoutType              TranslateSysConfigStringToLayoutType(std::string layoutStr);
extern BYTE                    IsSetStaticMbForUser(const char* productStr);
extern BYTE                    IsStringMatchFlagOfMultiNames(const char* searchedStr, const std::string& sProducts);

extern BYTE                    IsSetCIFRsrcForUser(const char* searchedSt, DWORD videoRate = 0);
extern BYTE                    IsSetSmartSwitchForUser(const char* productStr);
extern BYTE                    IsSetHD720RsrcForUser(const char* productStr);

extern BOOL                    Is1080pSupportedInOperationPoint(DWORD confRate);


//TIP call from Polycom EPs feature:
extern BYTE CheckIfRemoteSdpIsTipCompatible(const CSipCaps* pCurRemoteCaps,BYTE checkAlsoVideoCap = FALSE);
extern BYTE IsNeedToRejectTheCallForPreferTIPmode(const CSipCaps* pCurRemoteCaps);

extern BYTE                    checkRsrcLimitationsByPartyType(cmCapDirection direction, CIpComMode* CurrentMode, H264VideoModeDetails& h264VidModeDetails);
extern DWORD                   GetMinBitRateForCopLevel(BYTE copLevelFormat, BYTE copLevelFrameRate, BYTE copLevelProtocol, DWORD copLevelBitRate = 0);
extern BYTE                    GetMaxCopLevelForBitRate(DWORD rate);
extern BYTE                    GetMaxCopLevelForBitRateForBaseline(DWORD rate);
extern BYTE                    isNeedToChangeResOfBaselineAccordingToRate(DWORD rate, long currentFS);

extern BYTE                    IsForceResolutionForParty(const char* productStr);

extern void                    IdentifyVersionId(char* pSource, char** pVersionId, char* pProductId, int sourceLen);
extern void                    SetProblematicVersionId(char* pSource, char** pProblematicVersionId);
extern int                     CompareTwoVersionId(char* pPivotVersionID, char* pToCheckVersionId);
extern DWORD                   GetNewRateForUser(const char* productStr, DWORD callrate);
extern EFpsMode                TranslateSysConfigStringToFpsMode(const std::string& sFpsMode);
extern DWORD                   GetMaxRecordingLinks();
extern void                    SetIsCOPdongleSysMode(BOOL i_IsCOPdongleSysMode); // 2 modes cop/cp
extern BOOL                    GetIsCOPdongleSysMode();                          // 2 modes cop/cp
extern void                    SetDongle1500qHDvalue(BOOL confPartyLicensing_HD);
extern BOOL                    GetDongle1500qHDvalue();
extern void                    DumpMcuInternalDetailed(CLargeString& cstr, DWORD faultOpcode);
extern eVideoPartyType         GetLowestVideoAllocationAccordingToSystemMode(char* isH263);
extern APIU16                  GetProfileAccordingToCopProtocol(BYTE protocol);
extern BYTE                    GetEncoderParamsForNewResOnH264BaseLineCap(DWORD rate, ECopVideoFrameRate highestframerate, long& levelValue, long& maxMBPS, long& maxFS, long& maxDPB, long& maxBR, long& maxCPB, long& maxSAR, long& maxStaticMbps);

// Multiple links for ITP in cascaded conference feature:
extern BOOL                    GetITPparams(const char* pH323useruserField, BYTE& cascadedLinksNumber, BYTE& index, eTypeOfLinkParty& linkType, DWORD& unrsrvMainLinkDialInNumber, BYTE interfaceType);
extern void                    GetMainLinkName(const char* subPartyName, char* mainPartyName);
extern void                    GetSubLinkName(const char* mainPartyName, BYTE index, char* subPartyName);
extern void                    CreateMainLinkName(const char* originalPartyName, char* mainPartyName);                 // originalPartyName is without '_'
extern void                    CreateSubLinkName(const char* originalPartyName, BYTE index, char* subPartyName);       // originalPartyName is without '_'

//LYNC2013_FEC_RED:
extern Eh264VideoModeType 	   GetMaxH264VideoModeForMsSvcAccordingToSettings( CConfParty* pConfParty, const CCommConf* pCommConf);
extern Eh264VideoModeType 	   GetMaxMsSvcVideoModeByFlag(Eh264VideoModeType partyVideoMode);

extern BYTE                    GetCopFrameRateAccordingtoMbpsAndFs(BYTE level, long maxMBPS, long maxFS);
extern BYTE                    IsPalFrameRate(eVideoFrameRate videoFrameRate);
extern eVideoResolution        TranslateFSToH263H261CopEncoderResolutions(DWORD maxFS);
extern DWORD                   GetPartyArtWeight(DWORD inRateInKunits, DWORD outRateInKunits, BYTE isLpr, BYTE isEncryption, BYTE isAudioStereo, BYTE isSirenFamily);
BOOL                           isVendorSupportFullAudioCaps(const char* cUserAgent, const char* pVersionId = NULL);
extern APIU16                  BuildRtcpCnameMask(BYTE isRoomSwitch);
extern BOOL                    isPartyMeetContentRateThreshold(DWORD confContentRate, DWORD partyContentRate, BYTE confEnterpriseMode, BYTE confPresentationProtocol, DWORD &thresholdRate);
extern BYTE                    IsIntermediateSDRes(const char* productStr);
extern BOOL                    IsRtvBframeEnabled(BOOL isAvmcu);
extern eVideoPartyType         GetVideoPartyTypeAllocationForRtvBframe(eVideoPartyType videoPartyType);
extern void                    RateToTmmbrParams(uint32_t rate, uint32_t* mantissa, uint32_t* exp);
extern void                    TmmbrParamsToRate(uint32_t mantissa, uint32_t exp, uint32_t* rate);

extern BOOL                    IsFeatureSupportedBySystem(const eFeatureName featureName);
extern CPartyImageLookupTable* GetPartyImageLookupTable();
extern CLookupIdParty*         GetLookupIdParty();
extern CLookupTableParty*      GetLookupTableParty();
extern WORD  CleanLookupIdTablefromLookupPartyTable();

extern BOOL                    IsMsFECEnabled();
extern BOOL                    IsLyncRTCPIntraEnabled();
extern BOOL                    IsLyncRTCPIntraForAVMCUEnabled();
extern BOOL                    IsSendPreferenceRequestToAVMCU2010();
extern WORD                    GetMaxConfTemplates();
extern BOOL 				   IsOmitDomainFromPartyName();

unsigned long                  CalcOperationPointMBPS(const VideoOperationPoint& videoOperationPoint);
unsigned long                  CalcOperationPointFS(const VideoOperationPoint& videoOperationPoint);
long                           CalcMBPSforVswRelay(const VideoOperationPoint& videoOperationPoint);
long                           CalcFSforVswRelay(const VideoOperationPoint& videoOperationPoint);
bool                           SetPredefinedH264ParamsForVswRelayIfNeeded(EOperationPointPreset eOPPreset, APIU8 &level, long &fs, long &mbps, eIsUseOperationPointsPreset isUseOperationPointesPresets, const VideoOperationPoint &operationPoint);
bool                           IsOperationPointContainedInOtherOperationPoint(int firstLayerId, int secondLayerId, const CVideoOperationPointsSet* videoOperationPointsSet);
bool                           IsSameResolutionLayerId(int requestedLayerId, int streamLayerId, const CVideoOperationPointsSet* videoOperationPointsSet);
bool                           GetBestSsrcAndLayerIdFromImage(int requestedLayerId, const CImage* pImage, unsigned int& resultSsrc, int& resultLayerId, const CVideoOperationPointsSet* videoOperationPointsSet, bool bAllowOnlyTemporal_T0 = false);
void                           FillVideoOperationSetParams(VIDEO_OPERATION_POINT_SET_S& tVideoOperationPointSet, CVideoOperationPointsSet* videoOperationPointsSet);

int                            GetTidFromLayerId(int streamLayerId, const CVideoOperationPointsSet* pVideoOperationPointsSet);
BOOL                           IsRelaySupported(eConfMediaType confMediaType);

BOOL                           IsSoftMcu();
BOOL                           IsIvrForSVCEnabled();

void SerializeNonMandatoryRsrcParams(CSegment* pParam, CRsrcParams* &pRsrcParams, const char *aName = "");
void SerializeNonMandatoryRsrcParamsArray(CSegment* pParam, CRsrcParams** pRsrcParamsArray, int aArrayLen, const char *aName = "");
void DeSerializeNonMandatoryRsrcParams(CSegment* pParam, CRsrcParams* &apRsrcParams, const char *aName = "");
int GetSdesCapEnumFromSystemFlag();
BYTE GetResetSdesForVendorFlag();
eVideoPartyType translateVideoPartyTypeToH264VideoMode(Eh264VideoModeType h264videomode);
Eh264VideoModeType translateToH264VideoModeVideoPartyType(eVideoPartyType  videoPartyType);


typedef struct
{
	DWORD m_boardId;
	DWORD m_subBoardId;
	DWORD m_unitId;
} stBoardUnitParams;

bool operator<(const stBoardUnitParams& lhs, const stBoardUnitParams& rhs);

extern void  Set1080p60mbps(DWORD mbps);
extern DWORD Get1080p60mbps();
extern bool IsDebugModeStr( const char *featureStr, DWORD dwDebugNum );
extern void SetDebugValue( const std::string& featureNameStr, DWORD dwDebugValue);
extern DWORD GetCurrentLoggerNumber();
extern void SetCurrentLoggerNumber(DWORD loggerNumber);
extern BOOL isMsftSvc2013Supported();
extern EVideoResolutionType convertVideoTypeToResType(eVideoPartyType videoPartyType);
extern EVideoResolutionType GetResolutionTypeByPrId(DWORD prID);
extern DWORD GetPrIdByResolutionType(EVideoResolutionType resolutionType);


extern void UpdateTrafficShapingParams(TUpdateRtpSpecificChannelParams& rtp, const BYTE isMrcCall = FALSE);

struct VB_PARTY_VIDEO_LIST_S
{
	size_t                   list_size;
	ALLOC_STATUS_PER_PORT_S* party_video_list;
};

const char* ConfTypeToString(EConfType confType);
const char* VideoPartyTypeToString(eVideoPartyType videoPartyType);
const char* RemoteIdentToString(RemoteIdent remoteIdent);
const char* MediaDirectionToString(EMediaDirection mediaDirection);
const char* AppPartyStateToString(TAppPartyState appPartyState);
const char* IpChannelTypeToString(EIpChannelType ipChannelType);
const char* CsChannelStateToString(ECsChannelState csChannelState);
const char* ArtRtpPortChannelStateToString(EArtRtpPortChannelState artRtpPortChannelState);
const char* CmUdpChannelStateToString(ECmUdpChannelState cmUdpChannelState);

//13003
extern DWORD GetRTVMaxBitRateForForceCIFParticipant();
extern size_t ReadXmlFile(char* xmlFile, char** xmlBuffer);
BOOL GetFlagEnableRssControlViaInfo();
int HtmlToUtf8( char** ppInString, char* pOutString,int bytecount);

BOOL GetBOOLDataByKey(const std::string& key);

// Compile-time BASE^EXP calculation, needs static BASE and EXP
template <WORD BASE, BYTE EXP> struct Power	{enum {result = BASE * Power<BASE, EXP-1>::result};};
template <WORD BASE> struct Power<BASE, 0>	{enum {result = 1};};


#endif   /*  _CONFPARTYGLOBALS_H__ */

