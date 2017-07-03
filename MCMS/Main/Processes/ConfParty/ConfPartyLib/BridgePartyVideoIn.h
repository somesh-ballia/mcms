#ifndef _BRIDGE_PARTY_VIDEO_IN_
#define _BRIDGE_PARTY_VIDEO_IN_

#include "BridgePartyVideoUniDirection.h"
#include "Image.h"
#include "VideoBridgePartyCntl.h"

class CBridgePartyCntl;
class CRsrcParams;
class CBridgePartyVideoInParams;
class CTaskApi;
class CAvcToSvcTranslator;
class CAvcToSvcParams;

// Timers opcodes
#define VIDEO_DECODER_SYNC_TOUT     ((WORD)300)

// temporary defines for Avc To Svc
#define MAX_ENCODERS_PER_AVC_TO_SVC 3

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoIn
////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoIn : public CBridgePartyVideoUniDirection
{
	CLASS_TYPE_1(CBridgePartyVideoIn, CBridgePartyVideoUniDirection)

public:
	enum STATE {SETUP = (IDLE+1), CONNECTED, DISCONNECTING};

	                       CBridgePartyVideoIn ();
	                       CBridgePartyVideoIn (const CBridgePartyVideoIn&);
	virtual               ~CBridgePartyVideoIn ();
	CBridgePartyVideoIn&   operator=(const CBridgePartyVideoIn& rOtherBridgePartyVideoIn);
	void 				   CopyAvcToSvcTranslatorsParams(CBridgePartyVideoIn& rOtherBridgePartyVideoIn);

	virtual const char*    NameOf() const { return "CBridgePartyVideoIn";}
	virtual void*          GetMessageMap();
	virtual void           Connect();
	virtual void           DisConnect();
	virtual void           Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyMediaParams);
	virtual void           CreateForMove(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyVideoInParams);
	virtual const CImage*  GetPartyImage(void) const { return m_pImage;}

	virtual void           UpdateSelfMute(RequestPriority who, EOnOff eOnOff);
	void                   UpdateNewConfParamsForOpenedPortAfterMove(DWORD confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoInParams);
	void                   UpdateNewConfParams(DWORD confRsrcId, const CBridgePartyMediaParams* pBridgePartyVideoParams);
	virtual BOOL           IsConnected()     { return ((m_state == CONNECTED) ? TRUE : FALSE); }
	virtual BOOL           IsConnecting()    { return ((m_state == SETUP) ? TRUE : FALSE); }
	virtual BOOL           IsDisConnected()  { return ((m_state == IDLE) ? TRUE : FALSE); }
	virtual BOOL           IsDisconnecting() { return  ((m_state == DISCONNECTING) ? TRUE : FALSE); }

	virtual void           UpdateVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void           DumpAllInfoOnConnectionState(CMedString* pMedString, bool isShortPrint = false);

	virtual void           UpdateVideoClarity(WORD isVideoClarityEnabled);
	virtual void           UpdateAutoBrightness(WORD isAutoBrightnessEnabled);
	virtual void           UpdateVisualEffects(CVisualEffectsParams* pVisualEffects, BYTE bInternalUpdateOnly);
	virtual void           UpdatePartyTelePresenceMode(eTelePresencePartyType partyNewTelePresenceMode);


	BYTE                   IsPartyImageSyncLoss(){return m_pImage->isSyncLost(); }
	void                   SetSiteName(const char* visualName);
	const char*            GetSiteName() const;
	void                   ReSync();
	void                   UpdatePartyInParams(CUpdatePartyVideoInitParams* pUpdatePartyVideoInitParams);
	BYTE                   IsAckOnOpenPort(){return m_isConnected;} // for debug trace only

	BYTE                   IsVsw() const;
	void                   SetVsw(BYTE isVSW);

	DWORD                  GetLastSyncStatus() const     {return m_last_sync_status;}
	DWORD                  GetLastSyncResolution() const {return m_last_sync_resolution;}
	eVideoResolution       GetResolutionFromDetectedMode(DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight);

	virtual void           FillAllocStatus(ALLOC_STATUS_PER_PORT_S& allocStatusPerPort);
	CBridgePartyCntl*      GetBridgePartyCntlPtr() const;

	// Avc To Svc
	void 				   AvcToSvcTranslatorConnected( EStat status );
	void                   AvcToSvcTranslatorDisconnected(EStat receivedStatus, DWORD encoderConnectionId, bool bIsVswStream);
//	void                   UpdateVideoBridgeOnNonRelayImageAvcToSvcTranslated();
	const CAvcToSvcParams* GetpAvcToSvcParams(void) const         { return m_pAvcToSvcParams; }
	bool                   IsTranslatorAvcSvcExists();
	void                   SendRelayIntraRequestToAvcToSvcTranslator( DWORD ssrc);
	CVisualEffectsParams*  GetVisualEffects() const               { return m_pPartyVisualEffects; }
	void                   NotifyOnRemovedAvcSvcVideoStreams(DWORD partyRsrcId, DWORD ssrc=0xFFFFFFFF);
	
	bool                   IsMuted() const;
	CDwordBitMask          GetMuteMask() const;
	bool                   IsMuteByOperator()  const;
	bool                   IsMuteByMCMS()  const;
	bool                   IsMuteByParty() const;
	
	void 				   UpgradeAvcToSvcTranslator( CAvcToSvcParams *pAvcToSvcParams );
	void 				   Export();
	virtual void 		   UnregisterInTask();
	virtual void 		   RemoveConfParams();
	void 				   Import();
	int 				   AddAvcToSvcTranslatorsToRoutingTable();
	virtual void 		   RegisterInTask(CTaskApp* myNewTask);
	int 				   UpdateTranslatorsWithNewConfParamsAfterMove( DWORD confRsrcId );
	void				   UpdateLastReqUponTranslatorError( DWORD lastRequestId, DWORD lastReqOp);
	void				   AvcToSvcVswRequestIntraFromEP();
	
	virtual void 		   GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap);
	virtual void 			Dump() const;


protected:
	virtual void           SendOpenDecoder();
	virtual void           SendConnectToRtp();
	virtual void           SendCloseDecoder();

	void                   UpdateDBLocalVideoSyncState(BYTE isSynced);
	virtual void           SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void           SaveAndSendUpdatePartyTelePresenceMode(CSegment* pParam);

	virtual void           MuteYourSelf(RequestPriority who);
	virtual void           UnMuteYourSelf(RequestPriority who);

	void                   CheckAndInformAllClosed();
	void 				   InformConnectEndedWithError( EStat receivedStatus );
	void                   InformInClosed();
	void                   RemoveAvcToSvcStateMachineFromRoutingTable(DWORD encoderConnectionId);
	bool				   IsAllTranslatorsConnectedOK();
	void                   ConnectAvcToSvcTranslators();
	void                   DisconnectAvcToSvcTranslators();
	void				   SetVideoInConnected();
//	void				   InformAvcToSvcTranslatorsDecoderSyncInd();
	void 				   AllTranslatorsUpdateImage();
	int 				   AddTranslaytorToRoutingTbl( DWORD partyRsrcId, CAvcToSvcTranslator* pAvcToSvcTranslator, ConnectionID encoderConnectionId );

	// action functions
	virtual void           OnVideoBridgePartyConnectIDLE(CSegment* pParam);
	virtual void           OnVideoBridgePartyConnectSETUP(CSegment* pParam);
	virtual void           OnVideoBridgePartyConnectCONNECTED(CSegment* pParam);
	virtual void           OnVideoBridgePartyConnectDISCONNECTING(CSegment* pParam);

	virtual void           OnVideoBridgePartyDisConnectIDLE(CSegment* pParam);
	virtual void           OnVideoBridgePartyDisConnectSETUP(CSegment* pParam);
	virtual void           OnVideoBridgePartyDisConnectCONNECTED(CSegment* pParam);
	virtual void           OnVideoBridgePartyDisConnectDISCONNECTING(CSegment* pParam);
	virtual void           OnVideoBridgePartyDisConnect(CSegment* pSeg);

	virtual void           OnMplAckSETUP(CSegment* pParam);
	virtual void           OnMplAckCONNECTED(CSegment* pParam);
	virtual void           OnMplAckDISCONNECTING(CSegment* pParam);
	virtual void           OnMplDecoderSyncSETUP(CSegment* pParam);
	virtual void           OnMplDecoderSyncCONNECTED(CSegment* pParam);

	virtual void           OnMplOpenPortAck(STATUS status);
	virtual void           OnMplConnectAck(STATUS status);
	virtual void           OnMplClosePortAck(STATUS status);

	virtual void           OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateVideoParamsDISCONNECTING(CSegment* pParam);

	virtual void           OnVideoBridgePartyChangeVisualEffectsIDLE(CSegment* pParam);
	virtual void           OnVideoBridgePartyChangeVisualEffectsSETUP(CSegment* pParam);
	virtual void           OnVideoBridgePartyChangeVisualEffectsCONNECTED(CSegment* pParam);
	virtual void           OnVideoBridgePartyChangeVisualEffects(CSegment* pParam);

	virtual void           OnVideoBridgeUpdatePartyTelePresenceModeIDLE(CSegment* pParam);
	virtual void           OnVideoBridgeUpdatePartyTelePresenceModeSETUP(CSegment* pParam);
	virtual void           OnVideoBridgeUpdatePartyTelePresenceModeCONNECTED(CSegment* pParam);
	virtual void           OnVideoBridgeUpdatePartyTelePresenceMode(CSegment* pParam);

	virtual void           OnVideoBridgePartyUpdateVideoClarityIDLE(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateVideoClaritySETUP(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateVideoClarityCONNECTED(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateVideoClarityDISCONNECTING(CSegment* pParam);

	virtual void           OnVideoBridgePartyUpdateAutoBrightnessIDLE(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateAutoBrightnessSETUP(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateAutoBrightnessCONNECTED(CSegment* pParam);
	virtual void           OnVideoBridgePartyUpdateAutoBrightnessDISCONNECTING(CSegment* pParam);

	virtual void           OnAvcToSvcTraslatorConnectedSETUP(CSegment* pParam);
	virtual void           OnAvcToSvcTraslatorConnectedCONNECTED(CSegment* pParam);

	virtual void           OnAvcToSvcTraslatorDisconnectedSETUP(CSegment* pParam);
	virtual void           OnAvcToSvcTraslatorDisconnectedCONNECTED(CSegment* pParam);
	virtual void           OnAvcToSvcTraslatorDisconnectedDISCONNECTING(CSegment* pParam);

	virtual void           OnTimerDecoderSyncCONNECTED(CSegment* pParam);
	virtual void           OnTimerDecoderUpdateCONNECTED(CSegment* pParam);
	virtual void           OnTimerRecurrentIntraRequest(CSegment* pParam);

//	virtual void           OnAvcToSvcEncoderReadyWaitTimerCONNECTED(CSegment* pParam);
//	virtual void           OnAvcToSvcEncoderReadyWaitTimerANYCASE(CSegment* pParam);

	virtual void 		   OnUpgradeToMixAvcToSvcIDLE(CSegment* pParam);
	virtual void 		   OnUpgradeToMixAvcToSvcSETUP(CSegment* pParam);
	virtual void 		   OnUpgradeToMixAvcToSvcCONNECTED(CSegment* pParam);
	virtual void 		   OnUpgradeToMixAvcToSvcDISCONNECTING(CSegment* pParam);

	virtual void		   OnAvcToSvcTraslatorKillANYCASE(CSegment* pParam);

	// Avc To Svc
	bool                   IsTranslateAvcToSvcSupported();
	int                    TranslateAvcToSvc();
	bool                   UpdateAvcSvcParams();
	int                    CreateAndConnectAvcToSvcTranslator();
	void                   SaveAvcToSvcParams(const CAvcToSvcParams*  other);
	void 				   ReplayUpgradeAvcToSvcTranslate( EStat status );
	CVideoOperationPointsSet* GetConfVideoOperationPointsSet() const;

	virtual BYTE   		   GetCloseAvcSvcTranslatorStatus() {return m_closeAvcSvcTranslatorStatus;}
	virtual void   		   SetCloseAvcSvcTranslatorStatus(BYTE status) {if (statOK!=status) m_closeAvcSvcTranslatorStatus=status;}
	virtual void   		   ResetCloseAvcSvcTranslatorStatus() {m_closeAvcSvcTranslatorStatus=statOK;}

	void 				   OnUpgradeAvcToSvcTranslatorNotExistingTranslator( CAvcToSvcParams *pAvcToSvcParams );
	void 				   OnUpgradeAvcToSvcTranslatorExistingTranslator();
//	int 				   GetTranslatorsStatus();
	void 				   SaveUpgradeAvcToSvcTranslatorAndReplyOk(CSegment* pParam);
	void 				   RequestChangeLayoutFromAvcToSvcTranslators();

	CVisualEffectsParams*  m_pPartyVisualEffects;// some parameters in ChangeLayout (AvcSvc) must be the same as Open AVC Encoder

	BYTE                   m_isConnected;        // for debug trace only
	CImage*                m_pImage;
	CSegment*              m_pDecoderSyncSegm;
	eTelePresencePartyType m_eTelePresenceMode;  // PCI bug patch (to be removed in V3.x)
	DWORD                  m_backgroundImageID;  // The background id is needed for optimization - in order to fit to FPGA scaler output resolutions more accurately
	BYTE                   m_isVsw;
	DWORD                  m_last_sync_status;
	eVideoResolution       m_last_sync_resolution;
	BYTE                   m_waitForUpdateDecoderAck;
	bool                   m_bDecoderClosed;
	DWORD                  m_confMediaType;

	// AVC to SVC Translator start
	CAvcToSvcTranslator*   	m_pAvcToSvcTranslator[MAX_ENCODERS_PER_AVC_TO_SVC];
	CAvcToSvcParams*       	m_pAvcToSvcParams;
	bool                   	m_bIsTranslatorCreated;
	BYTE 				   	m_closeAvcSvcTranslatorStatus;	// one for all AvcSvc Translators
  	bool					m_bNeedToReplayUpgradeAvcToSvcTranslator;
  	bool					m_DecoderConnected;
  	bool					m_bTranslatorReportedOnConnectionError;

	// AVC to SVC Translator end

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoInVSW
////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoInVSW : public CBridgePartyVideoIn
{
	CLASS_TYPE_1(CBridgePartyVideoInVSW, CBridgePartyVideoIn)

public:
	                       CBridgePartyVideoInVSW();
	virtual               ~CBridgePartyVideoInVSW();

	virtual const char*    NameOf() const { return "CBridgePartyVideoInVSW";}

	virtual void           OnMplConnectAck(STATUS status);
	virtual void           SendOpenDecoder();
};


////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoInContent
////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoInContent : public CBridgePartyVideoIn
{
public:
	                       CBridgePartyVideoInContent();
	virtual               ~CBridgePartyVideoInContent();

	virtual const char*    NameOf() const { return "CBridgePartyVideoInContent";}

	void                   UpdateVideoParams(CSegment* pParam);

protected:
	void                   OnVideoBridgeConfUpdateVideoParamsIDLE(CSegment* pParam);
	void                   OnVideoBridgeConfUpdateVideoParamsSETUP(CSegment* pParam);
	void                   OnVideoBridgeConfUpdateVideoParamsCONNECTED(CSegment* pParam);
	void                   OnVideoBridgeConfUpdateVideoParamsDISCONNECTING(CSegment* pParam);
	void                   OnVideoBridgeConfUpdateVideoParams(CSegment* pParam);

	virtual void           OnMplDecoderSyncCONNECTED(CSegment* pParam);
	virtual void           OnMplOpenPortAck(STATUS status);
	virtual void           SendOpenDecoder();
	virtual void           SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void           SendUpdateDecoder();

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoInCOP
////////////////////////////////////////////////////////////////////////////
class CBridgePartyVideoInCOP : public CBridgePartyVideoIn
{
public:
	                       CBridgePartyVideoInCOP();
	virtual               ~CBridgePartyVideoInCOP();

	virtual const char*    NameOf() const { return "CBridgePartyVideoInCOP";}

	virtual void           Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyMediaParams);
	virtual WORD           IsValidState(WORD state) const;
	void                   SetDecoderConnectionIdInImage(DWORD decoderConnectionId);
	void                   SetDecoderPartyIdInImage(DWORD decoderPartyId);

	DWORD                  GetDecoderConnectionIdInImage();
	DWORD                  GetDecoderPartyIdInImage();

	void                   SetDspSmartSwitchConnectionId(DWORD decoderConnectionId);
	void                   SetDspSmartSwitchEntityId(DWORD decoderPartyId);


	void                   GetCurrentCopDecoderResolution(DWORD& algorithm, ECopDecoderResolution& copDecoderResolution);
	void                   GetInitialCopDecoderResolution(DWORD& algorithm, ECopDecoderResolution& copDecoderResolution);

	void                   GetInVidParams(CBridgePartyVideoInParams* pInVideoParams);
	ECopDecoderResolution  GetCopDecoderResolutionFromVideoResolution(eVideoResolution videoResolution);
	ECopDecoderResolution  GetCopDecoderResolutionFromDetectedMode(DWORD decoderDetectedModeWidth, DWORD decoderDetectedModeHeight);

	BYTE                   IsSyncWithDecoderResolution();
	void                   SetIsRemoteNeedSmartSwitchAccordingToVendor(BYTE isRemoteusesmartswitchaccordtovndor){m_IsRemoteUseSmartSwitchAccordingToVendor = isRemoteusesmartswitchaccordtovndor;}
	BYTE                   IsRemoteNeedSmartSwitchAccordingToVendor();

protected:
	ECopDecoderResolution  m_initialDecoderMode;
	BYTE                   m_IsRemoteUseSmartSwitchAccordingToVendor;
	virtual void           SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void           OnVideoBridgePartyConnectIDLE(CSegment* pParam);
	virtual void           OnVideoBridgePartyDisConnect(CSegment* pSeg);

	PDECLAR_MESSAGE_MAP
};

#endif // _BRIDGE_PARTY_VIDEO_IN_

