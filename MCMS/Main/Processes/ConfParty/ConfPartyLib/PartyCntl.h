#ifndef _PARTYCNTL_H__
#define _PARTYCNTL_H__

#include "ConfApi.h"
#include "StateMachine.h"
#include "PartyInterface.h"
#include "PartyRsrcDesc.h"
#include "NetSetup.h"
#include "AllocateStructs.h"
#include "BridgePartyVideoParams.h"
#include "AudioHardwareInterface.h"
#include "H320AudioCaps.h"
#include "VideoDefines.h"
#include "SvcPartyIndParamsWrapper.h"

//#define PERFORMANCE_TEST
#ifdef PERFORMANCE_TEST
	#include "Stopper.h"
#endif //PERFORMANCE_TEST

class CConf;
class CPartyApi;
class CAudioBridgeInterface;
class CVideoBridgeInterface;
class CConfAppMngrInterface;
class CBridgeMoveParams;
class CFECCBridge;
class CContentBridge;
class CTerminalNumberingManager;
class CComMode;
class CConfParty;
class CTelepresenseEPInfo;


#define MAX_VIDEO_BRIDGE_VIDEO_FORMATS   12
#define MAX_VIDEO_BRIDGE_FRAME_RATES_MPI 9
#define MAX_STANDARD_AUDIO_ALGORITHMS    11

#define NUM_OF_CONNECT_DELAY_RETRIES     1
#define EXPORT_RSRC_TOUT                 ((WORD)301)

const WORD DISCONNECTED = 10;

typedef struct
{
  int videoBridgeFormat;
  int ipFormat;
  int isdn263Format;
  int isdn261Format;
} TFormat;

typedef struct
{
  int videoBridgeFrameRate;
  int ipMpi;
  int isdn263Mpi;
  int isdn261FrameRate;
} TFrameRate_toMPI;

typedef struct
{
  EAudioCapAlgorithm audioAlgFromCaps;
  WORD               audioAlgFromCommMode;
} TAudioAlg_Caps_toScm;


typedef enum
{
  eNoUpdate       = 0,
  eUpdateAudioIn  = 1,
  eUpdateAudioOut = 2,
  eUpdateVideoIn  = 4,
  eUpdateVideoOut = 8,
} EUpdateBridgeMediaAndDirection;

// For uni-directional work towards bridges:
typedef enum
{
  eBridgeDisconnected     = 0,
  eSendOpenIn             = 0x01,     // 1
  eSendOpenOut            = 0x10,     // 2
  eSendOpenInAndOut       = 0x11,     // 3
  eInConnected            = 0x101,    // 5
  eOutConnected           = 0x1010,   // 6
  eSendOpenOutInConnected = 0x111,    // 7
  eSendOpenInOutConnected = 0x1011,   // 11
  eInAndOutConnected      = 0x1111,   // 15
  eLastConnectionState    = eInAndOutConnected,
} EBridgeConnectionState;

const WORD NumOfBridgeConnectionState = 9;

typedef struct
{
  EBridgeConnectionState eBridgeConnectionState;
  char*                  name;
} TBridgeConnectionStateNames;


inline EBridgeConnectionState& operator&=(EBridgeConnectionState& left, const EBridgeConnectionState& right)
{
  int temp = (int)left;
  temp &= (int)right;
  left  = (EBridgeConnectionState)temp;
  return left;
}

inline EBridgeConnectionState& operator|=(EBridgeConnectionState& left, const EBridgeConnectionState& right)
{
  int temp = (int)left;
  temp |= (int)right;
  left  = (EBridgeConnectionState)temp;
  return left;
}

inline EMediaDirection& operator|=(EMediaDirection& left, const EMediaDirection right)
{
  int temp = (int)left;
  temp |= (int)right;
  left  = (EMediaDirection)temp;
  return left;
}

#define MAX_NUM_RECV_STREAMS_FOR_MIX_AVC_VIDEO        2
#define MAX_NUM_RECV_STREAMS_FOR_VSW_RELAY            2

////////////////////////////////////////////////////////////////////////////
//                        CPartyCntl
////////////////////////////////////////////////////////////////////////////
class CPartyCntl : public CStateMachine
{
	CLASS_TYPE_1(CPartyCntl, CStateMachine)

	virtual const char* NameOf() const
	{ return GetCompileType(); }

public:

	static eVideoFrameRate         TranslateIntegerFrameRateToVideoBridgeFrameRate(DWORD frameRate);

public:

	                               CPartyCntl();
	virtual                       ~CPartyCntl();

	void                           operator=(const CPartyCntl& other);
	virtual void                   SetDataForImportPartyCntl(CPartyCntl *apOtherPartyCntl);

	virtual void                   SetFullName(const char* partyName, const char* confName);

	virtual void                   Disconnect(WORD mode = 0);

	int                            TranslateIpFormatToVideoBridgeFormat(int format) const;
	int                            TranslateIpMpiToVideoBridgeFrameRate(int Mpi) const;
	int                            TranslateVideoBridgeFrameRateToIpMpi(eVideoFrameRate frameRate) const;
	int                            TranslateIsdnFormatToVideoBridgeFormat(int format, WORD VideoAlgorithm) const;
	int                            TranslateIsdnFrameRateToVideoBridgeFrameRate(int Mpi, WORD VideoAlgorithm, CObjString& msg) const;

	WORD                           TranslateAudioAlgCapsToScm(EAudioCapAlgorithm AudioAlg);
	EAudioCapAlgorithm             TranslateAudioAlgScmToCaps(WORD AudioMode);
	BYTE                           CheckDisconnectCases(WORD mode, DWORD disconnectionDelay);
	virtual void                   SendStartMoveToResourceProcess();

	const char*                    GetBridgeConnectionStateStr(EBridgeConnectionState eBridgeConnectionState) const;
	char*                          GetPartyAndClassDetails();

	//SVC AVC MIX
	void                           SetPartyAvcSvcMode(eConfMediaState confState, DWORD confID, DWORD partyId);
	void                           UpdatePartyAvcSvcMode(eConfMediaState confState,DWORD confID, DWORD partyID);
	void                           OnConfSetPartyAvcSvcModeAnycase(CSegment* pParam);

	virtual void                   Destroy();
	virtual void                   UpdateConfEndDisconnect(WORD status);

	virtual void                   SetParty(CTaskApp* pParty);
	virtual void                   SetPartyRsrcId(DWORD partyRsrcId);
	virtual void                   SetConfRsrcId(DWORD confRsrcId);
	virtual void                   SetPartyName(char* name);
	virtual void                   SetMonitorPartyId(DWORD partyId)                                       { m_monitorPartyId = partyId; }
	virtual void                   HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	virtual void                   HandlePartyExternalEvent(CSegment* pMsg, OPCODE opCode);
	virtual void                   DisconnectPartyFromVideoBridge();
	virtual void                   DisconnectPartyFromFECCBridge();
	virtual void                   DisconnectPartyFromContentBridge();
	virtual void                   DisconnectPartyFromXCodeBridge();
	virtual void                   DisconnectPartyFromAudioBridge();
	virtual void                   InitVideoInParams(CBridgePartyVideoInParams* pMediaInParams);
	virtual void                   InitVideoOutParams(CBridgePartyVideoOutParams* pMediaOutParams);
	virtual void                   InitVideoLayoutParams(CBridgePartyVideoOutParams* pMediaOutParams, CConfParty* pConfParty);
	virtual int                    OnContentBrdgDisconnected(CSegment* pParam);
	virtual int                    OnContentBrdgConnected(CSegment* pParam);
	virtual void                   OnXCodeBrdgDisconnected(CSegment* pParam);
	virtual void                   OnXCodeBrdgConnected(CSegment* pParam);
	virtual void                   OnPartyReceivedFaultyRsrc(CSegment* pSeg);
	virtual const char*            GetName() const                                                        { return m_name; }
	virtual const char*            GetFullName() const                                                    { return m_partyConfName; }
	virtual CTaskApp*              GetPartyTaskApp() const                                                { return m_pParty; }
	BYTE						   GetIsWebRtcCall()const												  { return m_bIsWebRtcCall; } // N.A. DEBUG VP8
	DWORD                          GetPartyId() const                                                     { return GetPartyRsrcId(); }
	virtual DWORD                  GetPartyRsrcId() const;
	virtual DWORD                  GetConfRsrcId() const;
	virtual WORD                   IsDisconnect();
	virtual WORD                   ArePartyResourcesAllocated() const;
	virtual BOOL                   IsAgcOn() const;
	virtual BOOL                   GetEchoSuppression() const;
	virtual BOOL                   IsConfTelePresenceMode() const;
	virtual BOOL                   GetKeyboardSuppression() const;
	virtual BOOL                   GetAutoMuteNoisyParties() const;
	virtual DWORD                  GetAudioVolume() const;
	virtual DWORD                  GetListeningAudioVolume() const;
	virtual BOOL                   GetAudioClarity() const;
	virtual WORD                   GetConfSpeakerChangeMode() const;
	virtual BOOL                   GetAutoBrightness() const;

	virtual WORD                   IsFullBitRateConnected()                                               { return m_isFullBitRateConnect; }
	virtual WORD                   GetDisconnectMode()                                                    { return m_disconnectMode; }
	virtual WORD                   GetDialType()                                                          { return m_type; }

	virtual WORD                   IsDisconnectState();

	virtual WORD                   GetVoice() const                                                       { return m_voice; }
	virtual CPartyApi*             GetPartyApi()                                                          { return m_pPartyApi; }
	virtual CNetSetup*             GetNetSetUp()                                                          { return NULL; }
	virtual BYTE                   GetConfAudioSampleRate();
	virtual BYTE                   GetChnlWidth()                                                         { return m_chnlWidth; }
	virtual WORD                   GetNumChnl()                                                           { return m_numChnl; }
	virtual WORD                   GetType()                                                              { return m_type; }
	virtual DWORD                  GetMonitorPartyId()                                                    { return m_monitorPartyId; }
	virtual DWORD                  GetMonitorConfId()                                                     { return m_monitorConfId; }
	virtual BYTE                   GetVideoProtocol()                                                     { return m_videoProtocol; }
	virtual BYTE                   IsPartyMasterOrSlaveNotLecturer()                                      { return FALSE; }
	virtual ECascadePartyType      GetPartyCascadeTypeAndVendor()                                         { return eCascadeNone; /*eCascadeNone;*/}
	virtual BYTE                   IsPartyCascade() const                                                 { return FALSE; }
	virtual BYTE                   IsPartyCascadeWithCopMcu() const                                       { return FALSE; }
	virtual BYTE                   GetPartyCascadeType() const                                            { return CASCADE_NONE; }
	virtual WORD                   GetNodeType()                                                          { return m_nodeType; }
	virtual WORD                   GetInterfaceType() const                                               { return m_interfaceType; }
	void                           SetInterfaceType(WORD interfaceType);

	void                           SetSiteName(const char* pSiteName);
	virtual const char*            GetSiteName() const                                                    { return m_siteName; }
	virtual const char*            GetProductId() const                                                   { return m_productId; }
	virtual const char*            GetVersionId() const                                                   { return m_VersionId; }
	virtual BYTE                   GetIsCascadeToCopMcu()                                                 { return m_IsCascadeToCopMcu; }
	virtual void                   OnPartyUpdateVisualNameAndProductId(CSegment* pParam);
	virtual void                   OnPartySetNodeType(CSegment* pParam);
	virtual BOOL                   IsNeedToUpdateVisualName()                                             { return TRUE; }

	CConf*                         GetConf() const                                                        { return m_pConf; }
	BYTE                           IsUndefinedParty() const;
	BYTE                           IsFoundInAddressBook() const;
	WORD                           IsConfWaitingToEndChangeModeForMove() const                            { return m_confWaitToEndChangeModeForMove; }

	CPartyInterface*               GetPartyCntlResource()                                                 { return m_pPartyHWInterface; }

	CPartyRsrcDesc*                GetPartyCntlAllocatedRsrc()                                            { return m_pPartyAllocatedRsrc; } //mailbox

	void                           SetIsAutoVidBitRate(BYTE isAutoVidBitRate);
	BYTE                           GetIsAutoVidBitRate()                                                  { return m_isAutoVidBitRate; }

	// H239 - Pure virtual function
	virtual BYTE                   IsRemoteAndLocalCapSetHasContent(eToPrint toPrint = eToPrintNo) const = 0;
	virtual BYTE                   IsLegacyContentParty()                   = 0;
	virtual BYTE                   IsRemoteAndLocalHasHDContent1080() const = 0;
	virtual BYTE                   IsRemoteAndLocalHasHDContent720() const  = 0;

	virtual BYTE                   IsRemoteAndLocalCapSetHasBfcpUdp()const {return FALSE;}
	virtual BYTE                   IsFirstContentNegotiation()const {return FALSE;}

	//HP content
	virtual BYTE                   IsRemoteAndLocalHasHighProfileContent() const = 0;
	virtual DWORD                  GetPossibleContentRate() const = 0;

	// Update the content bridge only to parties which are already have a pointer to the bridge
	// For example - when switching slave bridge to Master bridge or vice versa
	virtual void                   SetNewContentBridge(CContentBridge* pContentBridge)                    { m_pContentBridge = pContentBridge; }
	virtual const CContentBridge*  GetContentBridge()                                                     { return m_pContentBridge;}

	virtual DWORD                  GetConnetDelay()                                                       { return m_connectDelay; }
	virtual DWORD                  GetState()                                                             { return 0; }
	DWORD                          GetMpiErrorNumber(CSegment* pParam);
	void                           SetDataForExportPartyFail();
	virtual void                   UpdatePartyStateInCdr();
	// H.320 change mode
	virtual DWORD                  VideoCanBeOpened(CComMode* pScm) const;

	BYTE                           GetmIsMemberInVidBridge()                                              { return m_bIsMemberInVidBridge; }
	BYTE                           GetmIsMemberInAudBridge()                                              { return m_bIsMemberInAudBridge; }

	void                           SetPartyVideoMute(CDwordBitMask& muteMask);
	void                           SetPartyAudioMute(CDwordBitMask& muteMask);
	void                           SetPartyAudioBlock(CDwordBitMask& blockeMask);
	virtual WORD                   GetIsDisconnectDelay()                                                 { return m_disconnectDelay; }

	virtual void                   OnPartyPcmStateChangedAnycase(CSegment* pParam);
	eVideoPartyType                GetLastAllocated()                                                     { return m_eLastAllocatedVideoPartyType; }
	virtual void                   PartySendMuteVideo(BYTE isActive)                                      { }
	virtual BYTE                   GetIsVideoMuted()                                                      { return m_bIsVideoMuted; }
	virtual void                   SetIsVideoMuted(BYTE onOff)                                            { m_bIsVideoMuted = onOff; }
	virtual BOOL                   IsLync()                                                               { return FALSE; }

	//for CDR event: PARTICIPANT_DISCONNECT_INFORMATION
	void                           UpdateDetailsForParticipantDisconnectInfoCDREvent(CIpComMode* pScm);
	virtual BOOL                   GetIsTipCall() const                                                   { return m_bIsTipCall;	}
	BYTE                           IsTipSlavePartyType() const;

	BOOL                           IsMsSlaveOut() const                                                   { return m_AvMcuLinkType == eAvMcuLinkSlaveOut; }
	BOOL                           IsMsSlaveIn() const                                                    { return m_AvMcuLinkType == eAvMcuLinkSlaveIn;  }

	void                           SetAvMcuLinkType(eAvMcuLinkType AvMcuLinkType)                         { m_AvMcuLinkType = AvMcuLinkType; }
	eAvMcuLinkType                 GetAvMcuLinkType() const                                               { return m_AvMcuLinkType; }
	DWORD                          GetMainPartyId() const                                                 { return m_MasterRsrcId; }

	RemoteIdent                    GetRemoteVendorIdent()                                                 { return m_eRemoteVendorIdent; }
	void                           SetRemoteVendorIdent(RemoteIdent remoteVendorIdent)                    { m_eRemoteVendorIdent = remoteVendorIdent; }

	void						   SetIsPartyInUpgradeProcess(bool isPartyInUpgradeProcess)				  { m_bPartyInUpgradeProcess = isPartyInUpgradeProcess;}
	bool						   GetIsPartyInUpgradeProcess()				  							  {return m_bPartyInUpgradeProcess;}

	static const TFormat                     g_formatTbl[MAX_VIDEO_BRIDGE_VIDEO_FORMATS];
	static const TFrameRate_toMPI            g_frameRateToMpiTbl[MAX_VIDEO_BRIDGE_FRAME_RATES_MPI];
	static const TBridgeConnectionStateNames g_bridgeConnectionStateNames[NumOfBridgeConnectionState];
	static const TAudioAlg_Caps_toScm        g_AudioAlg_Caps_toScmTbl[11];

	virtual void                   SetNewXCodeBridgeInterface(CVideoBridgeInterface* pXCodeBridgeInterface);
	virtual void                   SetNewXCodeBridgeInterfaceOff();
	virtual const                  CVideoBridgeInterface*  GetXCodetBridgeInterface()                     { return m_pXCodeBridgeInterface; }
	virtual void                   UpdateLegacyContentStatus(BYTE isBlockContent) {};
	void                           OnEndAvcToSvcArtTranslatorDisconnected(CSegment* pParam);

	virtual void                           OnPartyChangeBridgesMuteState(CSegment* pParam);

	//FSN-613: Dynamic Content for SVC/Mix Conf
	BYTE GetIsMrcCall() const {return m_bIsMrcCall;}
	void SetIsMrcCall(BYTE bIsMrcCall) {m_bIsMrcCall = bIsMrcCall;}

protected:

	eSessionType                   GetSessionTypeForResourceAllocator();
	STATUS                         SendReqToResourceAllocator(CSegment* seg, OPCODE opcode);
	virtual STATUS                 SendSyncReqToResourceAllocator(CSegment* seg, OPCODE opcode);

	void                           CreateAndSendAllocatePartyResources(eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType, EAllocationPolicy allocationPolicy, BYTE isPortGaugeFlagOn = FALSE, BYTE IsEnableSipICE = FALSE, DWORD artCapacity = 0, ETipPartyTypeAndPosition tipPartyType = eTipNone, WORD room_id = 0xFFFF, BOOL isBfcpEnabled = FALSE,WORD tipNumOfScreens = 0);
	virtual void                   CreateAndSendReAllocatePartyResources(eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType, EAllocationPolicy allocationPolicy, WORD reAllocRtm = FALSE, WORD isSyncMessage = 0, BYTE IsEnableSipICE = FALSE, DWORD artCapacity = 0, ETipPartyTypeAndPosition tipPartyType = eTipNone, WORD room_id = 0xFFFF,OPCODE opcode=REALLOCATE_PARTY_RSRC_REQ);
	void                           SendReAllocatePartyResources(WORD reAllocRtm, WORD isSyncMessage, OPCODE opcode);
	void                           CreateAndSendReAllocateArtForParty(eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType, EAllocationPolicy allocationPolicy, BYTE IsEnableSipICE, DWORD artCapacity, ETipPartyTypeAndPosition tipPartyType = eTipNone);
	void                           CreateAndSendDeallocatePartyResources(BYTE IsEnableSipICE = FALSE, DWORD RtpArr[] = NULL, DWORD RtcpArr[] = NULL, BOOL bIsPartyAllocated = TRUE);
	void                           OnDisconnectAvcToSvcArtTranslatorCloseTout(CSegment* pParam);

	void                           InsertPartyResourcesToGlobalRsrcRoutingTbl();
	void                           UpdatePartyEntryInGlobalRsrcRoutingTblAfterMove();
	EBridgeConnectionState         TranslateMediaDirectionToBridgeConnectionState(EMediaDirection eBridgeConnState);
	void                           UpdateAudioBridgeConnectionState(EMediaDirection eBridgeConnState);
	void                           UpdateVideoBridgeConnectionState(EMediaDirection eBridgeConnState);
	void                           UpdateAudioBridgeDisconnectionState(EMediaDirection eBridgeConnState);
	void                           UpdateVideoBridgeDisconnectionState(EMediaDirection eBridgeConnState);
	BYTE                           IsAtLeastOneDirectionConnectedToAudioBridge();
	BYTE                           AreTwoDirectionsConnectedToAudioBridge();
	BYTE                           IsOutDirectionConnectedToAudioBridge();
	BYTE                           IsInDirectionConnectedToAudioBridge();
	virtual EMediaDirection        HandleAudioBridgeConnectedInd(CSegment* pParam);
	virtual BYTE                   HandleAudioBridgeDisconnectedInd(CSegment* pParam);
	void                           HandleVideoBridgeConnectedInd(CSegment* pParam);
	EStat                          HandleVideoBridgeDisconnectedInd(CSegment* pParam);

	BYTE                           IsAtLeastOneDirectionConnectedToVideoBridge();
	BYTE                           AreTwoDirectionsConnectedToVideoBridge();
	BYTE                           IsOutDirectionConnectedToVideoBridge();
	BYTE                           IsInDirectionConnectedToVideoBridge();
	virtual void                   SetPartyTypeRelevantInfo(ALLOC_PARTY_REQ_PARAMS_S& allocatePartyParams){ }

	virtual BYTE                   isConfH239Cascade() const;
	virtual void                   UpdateBridgeFlowControlRateIfNeeded();
	void                           WaitForRsrcAndAskAgain();
	eSessionType                   GetSessionTypeForCop() const;
	//for CDR event: PARTICIPANT_DISCONNECT_INFORMATION
	DWORD                          GetConnectionRate(CIpComMode* pScm);
	void                           SetMaxResolutionAndMaxFrameRate(CIpComMode* pScm);

	virtual DWORD                  SendCreateParty(ENetworkType networkType, BYTE bIsMrcCall = FALSE) const;

	void                           UpdateVidBrdgTelepresenceEPInfoIfNeeded(eTelePresencePartyType eLocalTelePresencePartyType, bool isSiteNameChanged = false); //_e_m_

	bool                           ClearUnusedPartyId(DWORD rsrcPartyId);

	virtual int                    IsNeedToCloseInternalArt(eVideoPartyType newVideoPartyType) const {return false; }
	virtual void                   Deescalate(int numOfArtsToClose) {return;}
	virtual void                   UpdateStreamsListOfCurMode() {return;}

	CConfParty*                    GetConfParty();

	// Attributes
	WORD                           m_termNum; // terminal number
	WORD                           m_type;    // dial in out

	char*                          m_name;
	char                           m_partyConfName[2*H243_NAME_LEN+50];
	CConf*                         m_pConf;

	CPartyApi*                     m_pPartyApi;
	CTaskApp*                      m_pParty;
	CConfApi*                      m_pTaskApi;

	EMoveType                      m_moveType;

	CAudioBridgeInterface*         m_pAudioInterface;
	CVideoBridgeInterface*         m_pVideoBridgeInterface;
  CVideoBridgeInterface*         m_pXCodeBridgeInterface;
	CConfAppMngrInterface*         m_pConfAppMngrInterface;
	CFECCBridge*                   m_pFECCBridge;
	CContentBridge*                m_pContentBridge;
	CTerminalNumberingManager*     m_pTerminalNumberingManager;

	CPartyRsrcDesc*                m_pPartyAllocatedRsrc;
	CPartyInterface*               m_pPartyHWInterface;
	CBridgeMoveParams*             m_pBridgeMoveParams;

	COsQueue*                      m_pDestConfMbx;

	DWORD                          m_monitorConfId;
	DWORD                          m_monitorPartyId;
	WORD                           m_cascadeMode;       // the type of the local MCU: CASCADE_MODE_MASTER or CASCADE_MODE_SLAVE
	WORD                           m_nodeType;
	WORD                           m_voice;             // YES or NO
	char*                          m_service_provider;
	WORD                           m_serviceId;
	DWORD                          m_connectDelay;
	BYTE                           m_connectDelayCounter;
	// VNGR-6603
	WORD                           m_disconnectDelay;

	/* m_isWaitingForRsrcAndAskAgain - For the "Auto" resource allocation mode, to let the RA know if he can start reconfiguration process (if needed) or not.
	 * At the first allocation request the flag is set to YES.
	 * If the RA returns with status STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN in the answer to the allocation request,
	 * We will resend allocation request to the RA after x sec. this time with the flag set to NO.
	 * */
	BYTE                           m_isWaitingForRsrcAndAskAgain;

	WORD                           m_numChnl;       // party network connection
	BYTE                           m_chnlWidth;
	// predicates - brdg connected:
	EBridgeConnectionState         m_eAudBridgeConnState;
	EBridgeConnectionState         m_eVidBridgeConnState;
	// bridges flags
	EUpdateBridgeMediaAndDirection m_eUpdateState;  // for update bridges

	BYTE                           m_bIsMemberInAudBridge;
	BYTE                           m_bIsMemberInVidBridge;

	WORD                           m_isFeccConn;    // is bridge connected
	WORD                           m_isTerminalNumberingConn;
	WORD                           m_isChairConn;

	WORD                           m_isContentConn;
	WORD                           m_isXCodeConn;
	WORD                           m_isvidTSConn;

	WORD                           m_isAudRecover;
	WORD                           m_isVidRecover;
	WORD                           m_isMlpRecover;

	WORD                           m_isFullBitRateConnect;
	WORD                           m_DownSpeedStatus;
	WORD                           m_interfaceType; // ISDN_INTERFACE_TYPE|ATM_INTERFACE_TYPE|H323_INTERFACE_TYPE|SIP_INTERFACE_TYPE
	WORD                           m_disconnectionCause;
	WORD                           m_redial;
	WORD                           m_isDisconnect;
	WORD                           m_disconnectMode;
	WORD                           m_disconnectState;
	WORD                           m_isRecover;
	WORD                           m_isMultiRate;
	WORD                           m_isChairEnabled;
	WORD                           m_isMIH;
	BYTE                           m_videoProtocol; // H261, H263, H264 AUTO
	BYTE                           m_isRemoteCapReady;
	void*                          m_pComConf;
	char                           m_password[H243_NAME_LEN];

	char                           m_siteName[MAX_SITE_NAME_ARR_SIZE];
	char                           m_productId[MAX_SITE_NAME_ARR_SIZE];
	char                           m_VersionId[MAX_SITE_NAME_ARR_SIZE];
	BYTE                           m_IsCascadeToCopMcu;

	char*                          m_avServiceName;
	WORD                           m_avConnectionMode;
	WORD                           m_welcome_msg_time;
	WORD                           m_welcome_music_volume;

	BYTE                           m_isAutoDetect;  // NO for H221 or BONDING mode, YES for AUTODETECT mode
	BYTE                           m_isWaitingForHotSwap;

	// Source ID
	WORD                           m_sourceId;
	CSegment*                      m_pInfoSeg;

	// Relevant for GW conferences
	BYTE                           m_IsGateWay;
	WORD                           m_TcMode;
	WORD                           m_isChangeFromVSToTR;
	WORD                           m_IsMovedParty;
	WORD                           m_isUndefParty;
	WORD                           m_confWaitToEndChangeModeForMove;

	// DBC2
	BYTE                           m_isRemoteDBC2;
	DWORD                          m_destMonitorConfId;   // Monitor ID
	DWORD                          m_destResourceConfId;
	DWORD                          m_destMonitorPartyId;  // Monitor ID
	WORD                           m_isPartyEndRAMoveOK;
	WORD                           m_isFaulty;
	WORD                           m_isRecovery;
	BYTE                           m_isAutoVidBitRate;

	// opcode form CAM
	BYTE                           m_isPartyInConf;
	eVideoPartyType                m_eLastAllocatedVideoPartyType;
	eVideoPartyType                m_eLastAllocRequestVideoPartyType;
	eVideoPartyType                m_eLastReAllocRequestVideoPartyType;
	DWORD                          m_artCapacity;

	WORD                           m_OldState;
	WORD                           m_isCDRPartyConnected;

	//Multiple links for ITP in cascaded conference feature:
	eTelePresencePartyType         m_eTelePresenceMode;
	CTelepresenseEPInfo*           m_telepresenseEPInfo;

	int                            m_iAudioDelayUpdated;  // 0-not updated, 1-set, 2-reset
	WORD                           m_subServiceId;
	DWORD                          m_lastReqId;
	BYTE                           m_isPcmConnected;
	BYTE                           m_bNoVideRsrcForVideoParty;
	BYTE                           m_bIsVideoMuted;

	WORD                           m_RoomId;
	//for CDR event: PARTICIPANT_MAX_USAGE_INFO
	DWORD                          m_maxConnectionRateCurrently;
	EFormat                        m_maxFormatCurrently;
	WORD                           m_maxFrameRateCurrently;
	CSvcPartyIndParamsWrapper      m_SsrcIdsForAvcParty;

	BYTE                           m_bIsMrcCall;
	BYTE						   m_bIsWebRtcCall;
	BYTE                           m_bIsTipCall;
	ETipPartyTypeAndPosition       m_TipPartyType;
	bool                           m_deferUpgrade;
	bool                           m_bPartyInUpgradeProcess;
	RemoteIdent                    m_eRemoteVendorIdent;

	// Flora added for MSSlave:
	eAvMcuLinkType                 m_AvMcuLinkType;
	DWORD                          m_MSSlaveIndex;
	DWORD                          m_MSaudioLocalMsi;
	PartyRsrcID                    m_MasterRsrcId;

	BYTE                           m_isMsftEnv;

	ALLOC_PARTY_REQ_PARAMS_S       m_reallocatePartyParams;

	WORD                           m_reAllocRtm;
	WORD                           m_isReallocSyncMessage;
	OPCODE                         m_reallocOpcode;

#ifdef PERFORMANCE_TEST
public:
	CStopper                       m_Stopper;
#endif //PERFORMANCE_TEST

	PDECLAR_MESSAGE_MAP
};

#endif // _PARTYCNTL_H__
