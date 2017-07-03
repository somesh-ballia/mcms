//+========================================================================+
//                       IpPrtCtl.H                                        |
//             Copyright 2003 Polycom Israel Ltd.                          |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Israel Ltd. and is protected by law.             |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Israel Ltd.                    |
//-------------------------------------------------------------------------|
// FILE:       IpPrtCtl.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yael                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Yael   | 01/05/03   | Base class for Ip Party control (323 / Sip)       |
//+========================================================================+

#ifndef _IPPARTYCONTROL
#define _IPPARTYCONTROL

#include "StateMachine.h"
#include "PartyCntl.h"
#include "IPUtils.h"
#include "IpScm.h"
#include "H264VideoMode.h"
#include "MsSvcMode.h"
#include "H323CsReq.h"
#include "CopVideoTxModes.h"
#include "H323Util.h"
#include "ConfPartyApiDefines.h"
/*#include "ipnets.h"*/

//#include <u323des.h>

// compile time flag
// __VSW_FIX_MODE__


#define DISPLAY_MIN_CONTENT_RATE_FLAG 0x70000000
#define NUM_OF_CHANGE_MODES_PER_SECOND 6

/* states: must be defined in the base class!!*/
typedef enum {
//CAddIpPartyCntl states:
//const WORD  SETUP		  = 1;
	ALLOCATE_RSC  = 2,	 		// waiting for resource allocator response
	NEED_TO_REALLOCATE_RSC,		// 3
	REALLOCATE_RSC,				// 4
	CONNECT_VIDEO_BRIDGE_AFTER_REALLOC, // 5
	UPDATE_LEGACY_STATUS, 		// 6 Disconnect and reconnect to video bridge, in order to update the content Legacy status
	CREATE_MPL,					// 7
	CONNECT_AUDIO,	 			// 8 waiting for connect ack from both audio bridge and MPL
	PARTY_SETUP,    			// 9 party was asked to establish call
	PARTY_CONNECT_AUDIO,   		// 10 party connected audio channels
	CONNECT_BRIDGES,   			// 11 waiting for connect ack from both audio bridge and video bridge
	PARTY_IN_CONNECTING_STATE, 	// 12 Party is in connecting stage
	CREATE_AVMCU_MNGRS,         // 13 Lync party invites to AVMCU conf

//CIpChangeModeCntl states:
	CHANGEVIDEO,				// 14
	CHANGECONTENT,				// 15
	CHANGEOTHER,				// 16

//CIpChangeModeCntl + CAddIpPartyCntl states:
	CHANGE_ALL_MEDIA, 			// 17 change all the media in parallel and not in sequence
	UPDATEVIDEO, 				// 18 Update video bridge parameters

//CDelIpPartyCntl states:
	DESTROY_PARTY, 				// 19 send destroy message to party
	DELETE_FROM_HW, 			// 20 send delete message to mpl
	DEALLOCATE_RSC,				// 21

// CIpExportPartyCntl states:
	UPDATE_VIDEOBRDG_BEFORE_EXPORT, // 22
	EXPORT_RESOURCE,			// 23
	EXPORT_MPL,					// 24
	EXPORT_RESOURSE_END,		// 25
	EXPORT_BRIDGES,				// 26
	EXPORT_PARTY,				// 27

// CSipChangeModePartyCntl states:

	// Party Re-Cap
	PARTY_RE_CAPS,  			// 28 Receiving re-cap from the party change bridge state
	CONF_RESPONSE_PARTY_RE_CAPS,// 29 response to party re-cap
	CHANGE_BRIDGES, 			// 30 Change bridges as a result of Re-Cap or change mode.
	CONF_REQUEST_PARTY_CHANGE_MODE,// 31 Party Control send change mode request to Party
	DISCONNECT_BRIDGES,             // 32

	UPDATE_BRIDGES_FOR_CHANNEL_HANDLE, // 33
	WAITING_FOR_MSFT_OUT_SLAVES, //34
	WAITING_FOR_MSFT_ALL_SLAVES_DELETED, //35
	DELETE_FROM_MNGR, //35

} EPartyControlState;


const WORD   DUMMY_MUSIC_RATIO	= 5;
const WORD   DUMMY_MUSIC_VOLUME = 5;
const WORD   DUMMY_MSG_VOLUME	= 5;
const BYTE   FORCE_UPDATE_IN    = 1;
const BYTE   FORCE_UPDATE_OUT   = 2;

/*
typedef enum {
	eNoAudioState		= 0,
	eConnectAudioIn 	= 1,
	eConnectAudioOut	= 2,
	eConnectAudio		= 4,
	eConnectedAudio		= 8,
	eDisconnectAudioIn 	= 16,
	eDisconnectAudioOut	= 32,
	eDisconnectAudio	= 64
} EAudioBridgeState;*/
/*
typedef enum {
	eNoVideoState		= 0,
	eConnectVideoIn 	= 1,
	eConnectVideoOut	= 2,
	eConnectVideo		= 4,
	eConnectedVideo		= 8,
	eDisconnectVideoIn 	= 16,
	eDisconnectVideoOut	= 32,
	eDisconnectVideo	= 64
} EVideoBridgeState;*/

typedef enum {
	eContentSecondaryCauseGeneral = 1,
	eContentSecondaryCauseBelowRate= 2,
	eContentSecondaryCauseBelowResolution = 4
} EContentSecondaryCause;


typedef struct _PartyControlInitParameters
{
	CConf* 									pConf;
	CConfParty*								pConfParty;
	char*									pStrConfGuid;
	char*									pStrConfName;
	COsQueue*								pConfRcvMbx;
	DWORD									confRate;
	BYTE									bIsRedial;
	DWORD									redialInterval;
	COsQueue*								pPartyRcvMbx;
	CAudioBridgeInterface*						pAudioBridgeInterface;
	CVideoBridgeInterface*						pVideoBridgeInterface;
	CConfAppMngrInterface*					pConfAppMngrInterface;
	CFECCBridge*							pFECCBridge;
	CContentBridge*							pContentBridge;
	CTerminalNumberingManager*				pTerminalNumberingManager;
	DWORD									monitorConfId;
	// DWORD									monitorPartyId;
	WORD									TcMode;
	BYTE 									bIsGateWay;
	BYTE									bIsStreaming;
	const char*								pStrAppoitnmentID;
	WORD									voiceType;
	BYTE									standByStart;
	BYTE									bIsTipCompatibleVideo;
	const char* 								pAvServiceNameStr;
	WORD 									welcomeMsgTime;
	DWORD 									connectDelay;
	CCopVideoTxModes*						pCopVideoTxModes;
	char*									pStrAppointmentId;
	BYTE									bIsEncript;
	BYTE									encryptionType;
	CIpComMode* 							pConfIpScm;
	BYTE									bIsRecording;
	WORD									termNum;

	// AV-MCU Lync 2013
	eAvMcuLinkType             AvMcuLinkType;
	BYTE									msSlaveIndex;
	DWORD									msSlaveSsrcRangeStart;
}PartyControlInitParameters;



typedef struct _PartyControlDataParameters
{
	char*					pStrPartyName;
	BYTE					bIsEnableH239;
	BYTE					enterpriseMode;
	DWORD					h323MaxContentAMCRate;
	BYTE					hdResMpi;
	CapEnum					contentProtocol;
	BYTE					bIsTIPContentEnable;
	BYTE					bIsPreferTIP;
	BYTE					bIsHDContent1080Supported;
	eVideoQuality				vidQuality;
	BYTE					maxConfResolution;
	DWORD					callRate;
	CCOPConfigurationList*		pCOPConfigurationList;
	ePresentationProtocol		presentationProtocol;
	CapEnum					contentCap;
	CIpNetSetup*				pIpNetSetup;
	CTaskApp*				pParty;
	BYTE					networkType;
	BYTE					bIsOffer;
	CSipCaps*				pSipRmtCaps;
	eConfMediaType			confMediaType;
	CVideoOperationPointsSet*   pConfOperationPointsSet;
	BYTE					bIsMrcHeader;
	BYTE					bIsMrcCall;
	BYTE					bIsWebRtcCall;
	BYTE					bIsLync;
	eTypeOfLinkParty			linkType;
	DWORD					roomID;
	RemoteIdent				epType;
	LyncConnType			lyncEpType;
	BYTE					bContentMultiResolutionEnabled;
	BYTE					bContentXCodeH263Supported;
	BYTE					bIsASSIPcontentEnable;
	BYTE					bIsHighProfileContent;
	eIsUseOperationPointsPreset isUseOperationPointesPresets;

	ETipPartyTypeAndPosition                tipPartyType;
	DWORD									masterRoomId;
	eVideoPartyType 						masterVideoPartyType;
	DWORD 									peerPartyRsrcID;
	CIpComMode*                 pIpMasterInitialMode;

	// AV-MCU Lync 2013
	eAvMcuLinkType              AvMcuLinkType;
    DWORD									msSlaveIndex;
    DWORD									msSlaveSsrcRangeStart;
	CVidModeH323*				pLocalSdesCap;
}PartyControlDataParameters;


typedef struct{

	eLogicalResourceTypes lrt;
	bool mandatoryVid;
	bool mandatoryAud;

}MixedLogicalResourceInfo;

class CBridgePartyVideoRelayMediaParams;

class CIpPartyCntl : public CPartyCntl
{ //abstract class
	CLASS_TYPE_1(CIpPartyCntl, CPartyCntl)
public:
	// Constructors
	CIpPartyCntl();
	virtual ~CIpPartyCntl();
    virtual void Destroy();

	// Initializations
	CIpPartyCntl& operator=(const CIpPartyCntl& other);
    virtual void SetDataForImportPartyCntl(CPartyCntl *apOtherPartyCntl);

    // Operations
	virtual const char*	NameOf() const {return "CIpPartyCntl";}
	virtual void*	GetMessageMap() { return (void*)m_msgEntries; }
	virtual DWORD   GetRemoteCapsVideoRate(CapEnum protocol) const = 0;
	virtual BYTE	InHighestCommonIsSecondary() /*const*/ {return IsSecondary();}
	virtual BYTE	IsVidModeIncludedInCaps() const {return TRUE;} //implemented in CH323PartyCntl
	virtual BYTE	IsPartyEncrypted() const {return FALSE;}//implemented in CH323PartyCntl
	virtual void	ChangeScm(CIpComMode* pH323Scm) {PASSERT_AND_RETURN(YES);}
	virtual WORD	IsPartyInChangeMode() const {return FALSE;}
	virtual WORD	IsFeccChanOpened() const {return TRUE;}
	virtual BYTE	IsRemoteCapNotHaveVideo() const = 0;
	virtual DWORD   GetMinContentPartyRate(DWORD currContentRate = 0) = 0;
	//virtual BYTE	IsCOP() const;
	virtual void	UpdatePartyAsAudioOnly();

	virtual void SetPartyStateUpdateDbAndCdrAfterEndConnected(BYTE reason = NO);

	virtual CIpComMode* GetOriginalIpScm() const {return m_pOriginalIpScm;}
	virtual void DeleteBothOriginalScm();
	virtual CIpComMode* GetOriginalOrActiveIpScm() const {return  m_pIpInitialMode;}

	virtual void CheckDetails(WORD* pDetails, BYTE* direction) const;
	virtual void SetPartyToSecondaryAndStopChangeMode(BYTE reason,DWORD details = 0,BYTE direction = cmCapTransmit,CSecondaryParams *pSecParams=NULL, BYTE bDisconnectChannels = 0);
	virtual void SetPartySecondaryCause(BYTE reason,WORD details = 0,BYTE direction = cmCapTransmit,CSecondaryParams *pSecParams=NULL);
	void RemoveSecondaryCause(BYTE bKeepH239SecondaryCause);
	virtual void DispatchChangeModeEvent();
	virtual void EndChangeMode();

	virtual void EndIpPartyDisconnect();
	virtual BOOL IsDisconnectionBecauseOfNetworkProblems() const = 0;
    virtual BOOL IsDisconnectionBecauseOfInternalProblems() const;
    virtual BOOL IsDisconnectionBecauseOfRemoteBusy() const;
	virtual BOOL IsRedialForHotBackup() const;
	virtual	BOOL IsRedialImmediately()= 0;

	CComModeH323* GetInitialMode() const {return m_pIpInitialMode;}
	CComModeH323* GetCurrentMode() const {return m_pIpCurrentMode;}
	CCopVideoTxModes* GetCopVideoTxModes() const {return m_pCopVideoTxModes;}

	WORD  GetOriginalConfRate() const {return m_originalConfRate;}

//	BYTE    IsQuad() const;
	BYTE	IsContentChannel() const {return m_bNoContentChannel? YES: NO;}
    DWORD	GetVideoRate() const {return m_videoRate;}
//	DWORD	GetNewVideoContRate() const {return m_newVideoContRate;}
	DWORD	GetTransmitRate(DWORD dataType) const;
	BYTE	IsSecondary() const;

    void	SetVideoRate(DWORD rate) {m_videoRate = ((long)rate>=0)? rate: 0;}
 // 	void	SetNewVideoContRate(DWORD Rate){m_newVideoContRate = ((long)Rate>=0)? Rate: 0;}
	void	SetNewVideoRates(DWORD rate);
	void    UpdateConfEndDisconnect(WORD  status);
	void	InitiateRedialIfNeeded();

	virtual void  InitVideoInParams(CBridgePartyVideoInParams *pMediaInParams, CIpComMode* pScm);
	virtual void  InitVideoOutParams(CBridgePartyVideoOutParams *pMediaOutParams, CIpComMode* pScm, BYTE isLprInitiate = FALSE);
	virtual void  UpdateAudioDelay(CBridgePartyVideoOutParams *pMediaOutParams);
	virtual void  ConnectPartyToAudioBridge(CIpComMode* pScm);
	virtual void  ConnectPartyToVideoBridge(CIpComMode* pScm);
	virtual void  ConnectPartyToVideoBridge_noVideoRelay(CIpComMode* pScm);
	virtual void  ConnectPartyToVideoBridge_videoRelay(CIpComMode* pScm);
	virtual void  ConnectPartyToFECCBridge(CIpComMode* pScm);
	virtual	void  OnFeccBrdgCon(CSegment* pParam);
	virtual	void  OnFeccBridgeDisConnect(CSegment* pParam);

	virtual	void  UpdatePartyStateAfterVideoBridgeConnected();
	virtual int   OnContentBrdgDisconnected(CSegment* pParam);
    virtual BYTE DisconnectPartyFromDataBridgeIfNeeded();
	virtual int   OnContentBrdgConnected(CSegment* pParam);
	virtual void  OnCAMUpdatePartyInConf(CSegment* pParam);
	virtual BYTE  IsNeedToChangeDueToSwitchFromLecturerToNonLecturer(BYTE copLevel,BYTE takeInitial){return FALSE;};

	// Update Bridges
	int  UpdateAudioOutBridgeIfNeeded();
	int  UpdateAudioInBridgeIfNeeded();
	int  UpdateVideoOutBridgeIfNeeded(BYTE bTakeInitial,BYTE bForceUpdate = FALSE);
	int  UpdateVideoInBridgeIfNeeded(BYTE bTakeInitial, BYTE bForceUpdate = FALSE);
	int  UpdateVideoRelayOutBridgeIfNeeded(BYTE bTakeInitial,BYTE bForceUpdate = FALSE);
	int  UpdateVideoRelayInBridgeIfNeeded(BYTE bTakeInitial, BYTE bForceUpdate = FALSE);
	EUpdateBridgeMediaAndDirection  UpdateVideoBridgesIfNeeded(BYTE bTakeInitial, BYTE bForceUpdateIn = FALSE, BYTE bForceUpdateOut = FALSE);
	EUpdateBridgeMediaAndDirection  UpdateVideoBridgeOutIfNeeded(BYTE bTakeInitial);
	EUpdateBridgeMediaAndDirection  UpdateAudioBridgesIfNeeded();
	EUpdateBridgeMediaAndDirection  UpdateAudioAndVideoBridgesIfNeeded(BYTE bTakeInitial);
	BYTE CheckVideoBridgeEndUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges);
	void SetInitialRecRateAccordingToRes(ECopDecoderResolution eLastCopChangeModeParam,DWORD videoRate);
	virtual void UpdateVideoOutBridgeH239Case(BYTE bTakeInitial);
//	void UpdateCdrRecord(WORD partyState);
//	void CreateAndSendDeallocatePartyResources();
	void HandleVideoBridgeConnectedInd(CSegment* pParam);

	//change bridges state
	BYTE ChangeAudioBridgeStateAccordingToNewMode();
	BYTE ChangeVideoBridgeStateAccordingToNewMode();
	BYTE ChangeDataBridgeStateAccordingToNewMode();
	BYTE IsInitialModeAndCurrentModeNotContaining();

	void OnPartyRefreshVideoAnycase(CSegment* pParam);
	void OnPartyEventModeVideoPreviewRefreshVideoAnycase(CSegment* pParam);

	virtual BYTE IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo) const{return NO;};
	virtual BYTE IsRemoteAndLocalHasEPCContentOnly(){return NO;};
	virtual BYTE  IsRemoteAndLocalHasHDContent1080() const {return NO;};
	virtual BYTE  IsRemoteAndLocalHasHDContent720() const {return NO;};
	virtual BYTE  IsRemoteAndLocalHasHighProfileContent() const {return NO;};
	virtual	DWORD GetState() {return m_state;};

	void SetRedailCounterToZero();

	// VNGFE-787
 	virtual BYTE GetIsCodianVcr() { return m_isCodianVcr;};
	// LPR
	EUpdateBridgeMediaAndDirection UpdateLprVideoOutBridgeRate(DWORD lossProtection, DWORD mtbf, DWORD congestionCeiling
			, DWORD fill, DWORD modeTimeout, BYTE bTakeInitial, DWORD RemoteIdent = Regular);

	EUpdateBridgeMediaAndDirection UpdateFecOrRedVideoOutBridgeRate();

	virtual BOOL IsNeedToChangeResAccordingToRemoteRevision() = 0;
	BOOL IsDisableWideResolution();
	BYTE HandleSpecialEps();
	void SetResolutionTable(CBridgePartyVideoParams *pMediaParams);
	void updateResolutionAccordingToNewVideoRate( CBridgePartyVideoOutParams *pMediaOutParams, CIpComMode* pScm );
	virtual BYTE IsPartyCapsContainsH264SCM(const CMediaModeH323* H323ContentMode,ERoleLabel role) = 0;
	virtual BYTE IsOpenContentTxChannel(BYTE bTakeCurrent) const;

	void SendIntraToParty();
	void OnSendIntraToPartyTout();
	virtual BYTE CheckIfNeedToSendIntra();

	void OnTimerPartyChangeModeLoop();

	void SetNumOfChangeModesInSec(DWORD num){m_NumOfChangeModesInSec = num;};
	DWORD GetNumOfChangeModesInSec(){return m_NumOfChangeModesInSec ;};

	BOOL IsSetNewFRTresholdForParty();
	DWORD  GetForcedMbpsAndFR(long fs,long mbps,long& ForcedMbps);

    virtual void OnConfChangeModeUpgradeIdle(CSegment* pParam);
    virtual DWORD OnRsrcReAllocatePartyRspAdditionalReAllocate(CSegment* pParam, CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* &pMrmpRsrcParams,int allocationType);
    virtual void OnUpgradePartyToMixed(CSegment* pParam);
    virtual void OnEscalateUpgradePartyToMixed(CSegment* pParam);
    virtual bool UpgradeVideoBridgesIfNeeded();
    virtual void OnEndAudioUpgradeToMix(CSegment* pParam);
    virtual void OnEndVideoUpgradeToMix(CSegment* pParam);
	virtual void FinishUpgradeToMix();
	virtual bool CheckBridgesUpgradeCompletion();
	virtual void HandlePendingScmIfNeeded() {}
	
	void SetupConnParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams);
	virtual CIpComMode* NewAndGetPartyCntlScm(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams );
	EMediaTypeUpdate   IsMediaTypeUpdateNeeded() {return m_isAudioDecoderUpdateNeeded; }
	EVideoResolutionType GetMaxResolutionAccordingToVideoModeType(Eh264VideoModeType videoModeType);

	BOOL GetBOOLDataByKey(const std::string& key); //BRIDGE-13154
	void MoveScmToPendingWhileUpgrading(CIpComMode* incomingScm);
	void MoveScmToPendingWhileChangeMode(CIpComMode* incomingScm);

protected:

	CConfIpParameters* GetConfIpParameters(CConfParty* pConfParty);
	char* GetUniqueGuid(char* strConfGUID,CConfParty* pConfParty);
	void SetScmForVswRelay(CIpComMode* pPartyScm,WORD bIsAudioOnly, CVideoOperationPointsSet*pConfOperationPointsSet);
	void SetScmForMrcCall(CIpComMode* pPartyScm,CVideoOperationPointsSet*pConfOperationPointsSet, BYTE cascadeMode);
	void SetScmForAvcMixMode(CIpComMode* pPartyScm, WORD bIsAudioOnly);
	void InitDisplayNameForNetSetup(CIpNetSetup* pIpNetSetup);
	void SetSeviceIdForConfPartyByConnectionType(CConfParty* pConfParty,CIpNetSetup* pIpNetSetup);
	void SetSeviceIdForConfParty(CConfParty* pConfParty);
	void InitSetupTaAddr(CIpNetSetup* pIpNetSetup,char* conf_name,CConfParty* pConfParty,BYTE bIsH323);
	void SetBfcpInSipPartyScm(CIpComMode* pPartyScm);
	void UpdatePartyEncryptionMode(CConfParty* pConfParty,CIpComMode * pPartyScm,BYTE confEncType) const;
	void GetEncryptKeyExchangeModeFlag(BYTE& bAllowDtlsFlag , BYTE& bAllowSdesFlag ) const;
	void SetPartyScmForTip(CIpComMode* pPartyScm, CConfParty* pConfParty, CSipNetSetup* pNetSetup, DWORD serviceId,PartyControlDataParameters &partyControlDataParams);
	Eh264VideoModeType GetTipResolutionTypeAccordingToVideoRate(DWORD videoBitRate);
	BOOL IsNeedRecalcH264ParamsAccordingToPartySettings( CConfParty* pConfParty ,BYTE maxConfResolution,eVideoQuality vidQuality);	
	void GetH264VideoParamsAccordingToPartySettings( H264VideoModeDetails& returnH264VidModeDetails, CConfParty* pConfParty, DWORD decisionRate, BOOL isHighProfile,eVideoQuality vidQuality,BYTE maxConfResolution);
	BYTE CheckIfTipEnableByParams(CIpComMode* pPartyScm, CConfParty* pConfParty, DWORD confRate, DWORD partyRate);
	void SetScmForLync2013Call(CIpComMode* pPartyScm, CConfParty* pConfParty);
	void SetScmForWebRTCCall(CIpComMode* pPartyScm, CConfParty* pConfParty); //N.A. DEBUG VP8
	//Eh264VideoModeType GetMaxH264VideoModeForMsSvcAccordingToSettings(CConfParty* pConfParty);
	//Eh264VideoModeType GetMaxMsSvcVideoModeByFlag(Eh264VideoModeType partyVideoMode);
	WORD TranslateToConfPartyConnectionType(WORD dialType);
	void ChangeScmOfIpPartyInCpOrCopConf(CIpComMode* pPartyScm, CConfParty* pConfParty, BYTE partyVidProtocol, DWORD setupRate, BYTE bCreateNewScm,DWORD confRate,WORD dialType,DWORD serviceId,PartyControlDataParameters& partyControlDataParams, CSipNetSetup* pNetSetup = NULL);
	void    SetNewPartyConnectionMode(WORD& newStatus,WORD WelcomMode,CPartyCntl* pImpConfPartyCntl);
	void    MovePatyResources(DWORD NewMonitorConfId,CPartyCntl* pImpConfPartyCntl,WORD IsAudioPlus,WORD IsVoiceOnlyParty);
	void	HandleData();
//	void	CloseFecc(BYTE DisconnectDataChannels=TRUE);
//	virtual BYTE  IsFeccValid();
//	virtual BYTE  IsScmLsd();
	void	SetConfParamInfo();

    virtual void UpdateCurrentModeNoMedia(WORD channelType, ERoleLabel eRole);
	virtual void UpdateCurrentModeNoVideo();
	virtual void SetPartyToAudioOnly();
	virtual void UpdateCurrentModeInDB();
	virtual void ImplementUpdateSecondaryInPartyControlLevel() = 0;
	virtual void ImplementSecondaryInPartyLevel() = 0;
	virtual void UpdateCurrentModeMediaOff(WORD channelType, ERoleLabel eRole,cmCapDirection direction);
	virtual void UpdateInitialModeMediaOff(WORD channelType, ERoleLabel eRole,cmCapDirection direction);
	virtual CIpComMode*	GetScmForVideoBridgeConnection(cmCapDirection direction) = 0;
	virtual BYTE IsCapableOfVideo() = 0;
	virtual BYTE IsCapableOfHD720() = 0;
	virtual BYTE IsCapableOfHD1080() = 0;
	virtual BYTE IsCapableOf4CIF() = 0;
	virtual eVideoPartyType GetLocalVideoPartyType(BYTE partyTypeWithH263 = TRUE);
	eVideoPartyType GetMaxCurrentCallVideoPartyTypeForAllocation(CIpComMode* pScm,BYTE isSip = FALSE);
	virtual eVideoPartyType GetCPVideoPartyTypeAccordingToCapabilities()= 0;

	virtual BYTE IsNeedToChangeVideoResourceAllocation(eVideoPartyType eCurrentVideoPartyType);


	virtual DWORD GetConfRate()const   = 0 ;
	virtual DWORD GetSetupRate() = 0;
	H264VideoModeDetails GetH264ModeAccordingToVideoPartyType(eVideoPartyType videoPartyType);
	MsSvcVideoModeDetails GetMsSvcModeAccordingToVideoPartyType(eVideoPartyType videoPartyType);

	void CheckResourceAllocationTowardsRequest(BYTE isSip = 0);//Check towards requested allocation and update if needed:

	void SetMbpsForStaticMbCalculation(CIpComMode* pScm, DWORD&initialMBPS, long& currentScmMbps, long& fs, DWORD& staticMB);
	eVideoPartyType SetNewRemoteTransPartyTypeAccordngToVendorIfNeeded(CIpComMode* pScm,eVideoPartyType currentpartytype);
	void GetStaticMbForDsp(CIpComMode* pScm, DWORD& staticMB);
	virtual void UpdateH264ModeInLocalCaps(H264VideoModeDetails h264VidModeDetails,ERoleLabel eRole = kRolePeople) = 0;
	virtual APIS32 GetLocalCapsMbps(ERoleLabel eRole) = 0;
    virtual void Disable4CifInLocalCaps() = 0;
    virtual DWORD GetMaxFsAccordingtoProfile(APIU16 profile) = 0;
    virtual void GetRemoteCapsParams( WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile = 0) = 0;
    virtual void RemoveH263H261FromLocalCaps() = 0;
  	virtual BYTE AreLocalCapsSupportMedia(cmCapDataType eDataType) = 0;
	BYTE HandleReallocateResponse(CSegment* pParam);
	DWORD GetPartyRate();
	BOOL IsRemoteHasContentXGA()  ;
	BYTE IsPartyLegacyForTipContent(DWORD partyContentRate) ;

	//void  InitParams(cmCapDirection direction, WORD* pIsdnAlgorithm, DWORD* pBitRate, WORD* pVideoBridgeFormat, int* pVideoBridgeFrameRate);
	void  InitVideoParams(cmCapDirection direction, CBridgePartyVideoParams *pMediaParams, CIpComMode* pScm, BYTE isLprInitiate = FALSE);


	BYTE  DisconnectPartyFromAudioBridgeIfNeeded();
	BYTE  IsNeedToDisconnectVideoIn(CIpComMode* pScm);
	BYTE  IsNeedToDisconnectVideoOut(CIpComMode* pScm);
	BYTE  DisconnectPartyFromVideoBridgeIfNeeded(CIpComMode* pScm);
	BYTE  IsNeedToConnectToVideoBridge(CIpComMode* pScm);
	virtual BYTE  IsContentRateNeedToBeChanged();

	virtual void UpdateBridgeFlowControlRateIfNeeded(CLPRParams* lprParams = NULL);
    BYTE IsChangeIncomingMode (eChangeModeState changeMdoeState);
    void CopVideoBridgeChangeIn(CSegment* pParam, CIpComMode* pNewScm);
    void CopVideoBridgeChangeOut(CSegment* pParam);
    //for CDR new rate event.
    void  UpdateNewRateForCdrIfNeeded();
    DWORD CalculateArtCapacityAccordingToScm(CIpComMode* pScm, BOOL isAddVideoAndAudio);
    void CalculateArtCapacityAccordingToScmCop(CIpComMode* pScm, DWORD& outRate, DWORD& inRate, bool& isVideo, BYTE& isSirenFamily, BYTE isAddvideoAndAudio);
    inline virtual eVideoPartyType GetVideoPartyTypeForRtvBframe(){return eVideo_party_type_dummy;}
    void FillStreamSSRCForMedia(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoParams,cmCapDataType aMediaType,cmCapDirection direction,int reason,ERoleLabel eRole = kRolePeople);
    bool FillStreamSSRCForMedia_noRelay(CAvcToSvcParams* pAvcToSvcParams);
    virtual DWORD GetScpNotificationRemoteSeqNumber();
    BOOL IsRelayChannelReadyForMedia(cmCapDirection direction);
    void  InitVideoOutTipPolycomIfNeeded(CBridgePartyVideoOutParams* pOutVideoParams, CIpComMode* pScm); //_t_p_
    void GetIsForce720Pand2MForPLCMFlag(BYTE& bForce720PFlag , BYTE& bForce2MFlag ) const;

    virtual DWORD UpdateScmWithResources(SVC_PARTY_IND_PARAMS_S  &aSvcParams,eVideoPartyType allocatedVideoPartyType, BOOL isAvcVswInMixedMode);
    virtual bool IsPortConnected(eLogicalResourceTypes lrt);
    virtual BOOL GetEnableICE() { return FALSE;}
    virtual DWORD GetFirs2013Ssrc(BYTE index);

    BOOL FixVideoBitRateIfNeeded(CConfParty* pConfParty, CIpComMode* pPartyScm , CIpNetSetup* pNetSetup,  BOOL initParty, eVideoPartyType videoPartyType=eVideo_party_type_dummy);

    eVideoPartyType GetRelayResourceLevel(bool isVsw, CIpComMode* apScm) const;
    virtual void UpdateLocalCapsForHdVswInMixMode(const VideoOperationPoint *pVideoOperationPoint) {};

    BYTE GetIsMsftEnv();
    virtual int IsNeedToCloseInternalArt(eVideoPartyType newVideoPartyType) const;
    virtual int GetNumOfInternalArts(eVideoPartyType aVideoPartyType) const;
    virtual void Deescalate(int numOfArtsToClose);
    virtual void UpdateStreamsListOfCurMode();
    DWORD CheckResourceAllocationForMixMode();
    void UpdateResourceTableAfterRealloc(CPartyRsrcDesc* apPartyAllocatedRsrc);

    //BRIDGE-5753
	Eh264VideoModeType 	   GetTipResolutionTypeAccordingToMaxResolutionAndVidRate(EVideoResolutionType maxResolution, DWORD videoBitRate,CConfParty* pConfParty);
	void			 	   FixTipScmVideoBitRateIfNeeded(CConfParty* pConfParty , CIpComMode* pPartyScm , CSipNetSetup* pNetSetup, Eh264VideoModeType selectedVideoMode);
	BOOL 				   IsNeedToChangeTipVidRate(Eh264VideoModeType selectedVideoMode, DWORD vidBitrate);
	DWORD 				   GetMaxVidBitrateForHD720();
	void 				   ChangeVideoBitrateForTipIfNeeded(CConfParty* pConfParty, CIpComMode* pPartyScm , DWORD& vidBitrate);
	BOOL 				   GetFlagTipResAccordingToConfVidQuality();
	BOOL 				   GetIsNeedToChangeTipResAccordingToConfVidQuality(CConfParty* pConfParty);



//	DWORD 					GetPrIdByResolutionType(EVideoResolutionType resolutionType);
	DWORD 					GetPrIdLync2013();

	CIpComMode*	 m_pIpInitialMode;
	CIpComMode*  m_pIpCurrentMode;
    CVideoOperationPointsSet m_operationPointsSet;
    CSvcPartyIndParamsWrapper m_SsrcIdsForAvcParty;

	CCopVideoTxModes* m_pCopVideoTxModes;

//	CVerParam*	 m_pVerParam;
	BYTE		 m_bNoContentChannel;
	DWORD		 m_conferenceContentRate; // Only for EPC/duo
	DWORD	     m_videoRate;
//    DWORD	     m_newVideoContRate;
	WORD         m_originalConfRate;

	// Attributes
	CQoS*		m_pQos;

	// for change mode of h323 & sip
	WORD		m_avChangeModeRequest;
	WORD		m_avNewModeRequest;
	BYTE		m_bIsNewScm;
	BOOL		m_isTipFallbackFlow;

	// for attend mode of h323 & sip
	CIpComMode* m_pOriginalIpScm;

	WORD		   m_SecondaryCause;
	CSecondaryParams* m_pSecondaryParams;

	// conf parameters information (for trace etc.)
	CSmallString m_strConfParamInfo;

	// change mode:
	eChangeModeInitiator m_changeModeInitiator;
	// For H239 - Video enc rate change
	DWORD m_currentVideoEncRate;

	BYTE 	    m_presentationStreamOutIsUpdated;

    // Parameters to create the Party Task
    void (*m_entryPoint)(void*);
    WORD m_McuNumber;
    BYTE m_isRecording;
    BYTE m_isStreaming;
    BYTE* m_pExchangeServerConfId;
    const char * m_confName;
    const char * m_password;
    COsQueue* m_pConfRcvMbx;



	// VNGFE-787
	BYTE		   m_isCodianVcr;

    WORD	m_isSentH239Out;

    BYTE 	m_isLprActive;
    BYTE	m_isContentDba;
    BYTE    m_isHDVSWEnabled;

    WORD   m_numOfHotbackupRedial;
    BOOL   m_bIsFirstConnectionAfterHotBackupRestore;
    BYTE   m_eLastCopChangeModeParam; //decoder resolution or encoder index
    ECopChangeModeType m_eLastCopChangeModeType;
    BYTE m_lastCopForceEncoderLevel;
    CapEnum m_eFirstRxVideoCapCode; // The cap code that is used in the first connection to the video receive bridge.
    APIU16   m_eFirstRxVideoProfile;
    WORD    m_copResourceIndexOfCascadeLinkLecturer;
    BYTE    m_bCascadeIsLecturer;

	DWORD	m_lastConnectionRate; //for CDR new event PARTICIPANT CONNECTION RATE

	//===========================================================================
	// VNGR-23965 - Lowering log strain, logging only for when min rate changes
	//===========================================================================
	DWORD m_lastMinContentRate;
	BYTE  m_isBframeRscEnabled;
	DWORD m_NumOfChangeModesInSec;
	TICKS m_TimeOfChangeModeInTicks;

    unsigned int m_incomingVideoChannelHandle;
    unsigned int m_outgoingVideoChannelHandle;
    bool m_isSignalingFirstTransactionCompleted; // MRC parties must wait until signaling will end before update video bridge

    BYTE m_bVideoRelayOutReady;
    BYTE m_bVideoRelayInReady;
    BYTE m_bVideoUpdateCount;

    BOOL m_ConnectedToVideoAsLegacy;
	BOOL m_bIsNeedToChangeVideoOutTipPolycom;

	UdpAddresses   m_udpAddresses;

	bool m_bIsBridgeUpgradedAudio;
	bool m_bIsBridgeUpgradedVideo;
	CRsrcParams* m_avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
       CRsrcParams* m_pMrmpRsrcParams;
    bool m_bIsUseSpeakerSsrcForTx_TRUE_sent;
    CIpComMode* m_pendingScmForUpgrade;

    CIpComMode* m_pendingScmForChangeMode;

	EMediaTypeUpdate  m_isAudioDecoderUpdateNeeded;
//     (entryPoint, *pConfRcvMbx, *pConfRcvMbx, m_monitorPartyId,
//  					m_monitorConfId, m_name, confName, m_serviceId,
//  					termNum, mcuNumber, password,  m_voice, 1, isRecording,m_isAutoVidBitRate);
	PDECLAR_MESSAGE_MAP
};

#define ALLOCATION_TYPE_UPGRADE 1
#define ALLOCATION_TYPE_REALLOC 2

#define VIDEO_UPDATE_COUNT0 0
#define VIDEO_UPDATE_COUNT1 1
#define VIDEO_UPDATE_COUNT2 2

#endif //_IPPARTYCONTROL
