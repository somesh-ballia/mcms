#ifndef VIDEORELAYBRIDGEPARTYCNTL_H_
#define VIDEORELAYBRIDGEPARTYCNTL_H_

#include "RelayIntraData.h"
#include "VideoBridgePartyCntl.h"
#include "BridgePartyVideoRelayMediaParams.h"
#include "ScpNotificationWrapper.h"
#include "ScpPipeMappingNotification.h"


class CVideoRelayBridgePartyCntl : public CVideoBridgePartyCntl
{
	CLASS_TYPE_1(CVideoRelayBridgePartyCntl,CVideoBridgePartyCntl)
public:

	CVideoRelayBridgePartyCntl();
	CVideoRelayBridgePartyCntl(const CVideoRelayBridgePartyCntl& rOtherVideoRelayBridgePartyCntl);
	virtual ~CVideoRelayBridgePartyCntl ();
	CVideoRelayBridgePartyCntl&   operator=(const CVideoRelayBridgePartyCntl& rOtherVideoRelayBridgePartyCntl);
	virtual const char* NameOf() const { return "CVideoRelayBridgePartyCntl"; }


public:
  virtual void                  Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams); //KEREN TO CHANGE PARAMS!!!
  virtual void                  Update(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);//KEREN TO CHANGE PARAMS!!!
  virtual void                  Import(const CBridgePartyInitParams* pBridgePartyInitParams);
  virtual void                  ImportLegacy(const CBridgePartyInitParams* pBridgePartyInitParams, const CVideoBridgePartyCntl* pOldPartyCntl);


  virtual void                  InitiateUpdatePartyParams(const CBridgePartyInitParams* pBridgePartyInitParams);
  virtual void                  NewPartyOut();
  virtual void                  NewPartyIn();


  virtual void                  Connect(BYTE isIVR, BYTE isContentHD1080Supported = FALSE);
 // virtual void                  DisConnect(void);//keren to add disconnect event
  virtual void                  Export(void);
  //virtual void                  Destroy(void);
  void                          UpdateVideoInParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams);
  void                          UpdateVideoOutParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams);
  void                          AckOnIvrScpShowSlide(BYTE bIsAck);
  void                          AckOnIvrScpStopShowSlide(BYTE bIsAck);


  //virtual BOOL                  IsConnectedStandalone() const          { return ((m_state == CONNECTED_STANDALONE) ? TRUE : FALSE); }
  virtual BOOL                  IsPcmOvrlyMsgOn();
  virtual void                  SetPcmOvrlyMsgOn(BOOL i_bIsPcmOvrlyMsg);
  virtual CLayout*              GetReservationLayout(void) const;
  virtual CLayout*              GetCurrentLayout(void);
  virtual CLayout*              GetPrivateReservationLayout(void) const;
  virtual WORD                  GetIsPrivateLayout() const;
  virtual CVisualEffectsParams* GetPartyVisualEffects(void) const;
  virtual const CImage*         GetPartyImage(void) const;
  virtual ePartyLectureModeRole GetPartyLectureModeRole() const;
  BYTE                          IsImageInPartiesLayout(const CImage* pImage);

  virtual void                  UpdateSelfMute(RequestPriority who, EOnOff eOnOff);
  virtual void                  UpdateIndicationIcons();
  //TODO KEREN to verify implement for handle functions & add to statemachine table
//  virtual void                  AddImage(const CTaskApp* pParty);
//  virtual void                  DelImage(const CTaskApp* pParty);
//  virtual void                  ChangeAudioSpeaker(const CTaskApp* pParty);
//  virtual void                  ChangeSpeakers(const CTaskApp* pNewVideoSpeaker, const CTaskApp* pNewAudioSpeaker);
//  virtual void                  MuteImage(void);
//  virtual void                  UnMuteImage(void);
//  virtual void                  UpdateImage();
//  void                          DeletePartyFromConf(const char* pDeletedPartyName);


  virtual void                  FastUpdate(void);
  virtual void                  SetSiteName(const char* visualName);

  virtual void                  UpdateVideoInParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
  virtual void                  UpdateVideoOutParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
  virtual void                  UpdatePartyTelePresenceMode(eTelePresencePartyType partyNewTelePresenceMode);
//  virtual void                  IvrCommand(OPCODE opcode, CSegment* pDataSeg);
  virtual void                  IvrNotification(OPCODE opcode, CSegment* pParam);
  virtual void                  PLC_SetPartyPrivateLayout(LayoutType newPrivateLayoutType);
  virtual void                  PLC_PartyReturnToConfLayout();
  virtual void                  PLC_ForceToCell(char* partyImageToSee, BYTE cellToForce);
  virtual void                  PLC_CancelAllPrivateLayoutForces();
  virtual void                  VENUS_SetPartyPrivateLayout(LayoutType newPrivateLayoutType);


  virtual void                  PCMNotification(OPCODE opcode, CSegment* pParam);

  virtual void                  ChangeConfLayout(CLayout* pConfLayout, BYTE bAnyway = 0);
  virtual void                  ChangePartyLayout(CVideoLayout& newVideoLayout);
  virtual void                  ChangePartyPrivateLayout(CVideoLayout& newVideoLayout);
  virtual void                  ChangeLayoutPrivatePartyButtonOnly(WORD isPrivate);

  virtual void                  UpdateVisualEffects(CVisualEffectsParams* pVisualEffects);

  virtual void                  ForwardVINToParty(WORD mcuNumber, WORD terminalNumber);

  virtual LayoutType            GetConfLayoutType() const;
  virtual LayoutType            GetPartyCurrentLayoutType() const;

  virtual void                  SetPartyLectureModeRole(ePartyLectureModeRole partyLectureModeRole);

  virtual void                  SendMsgToScreenPerParty(CTextOnScreenMngr* TextMsgList, DWORD timeout = 5* SECOND);
  virtual void                  StopMsgToScreenPerParty();
  virtual void                  UpdateDecoderDetectedMode();

//VSW related functions
//  CTaskApp*                     GetPartySouceInCellZero()                    {return m_pPartySouceInCellZero;}
//    void                          SetPartySouceInCellZero(CTaskApp* partySouce){m_pPartySouceInCellZero = partySouce;}
//
//    void                          MarkAsNewVideoSource(WORD resync);
//    WORD                          IsMarkedAsNewVideoSource()                   {return m_resync;}
//
//    void                          ResyncVideoSource();


  virtual void                  GetRsrcProbAdditionalInfoOnVideoTimerSetup(BYTE& failureCauseDirection, BYTE& failureCauseAction);
  virtual bool                  IsBridgePartyVideoOutStateIsConnected();
  virtual void                  DisplayGatheringOnScreen(CGathering* pGathering);
  virtual void                  SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects);
  virtual void                  UpdateAutoScanOrder(CAutoScanOrder* pAutoScanOrder);
  virtual void                  StartMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo);
  virtual void                  UpdateMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo);
  virtual void                  UpdateVideoClarity(WORD isVideoClarity);
  virtual void                  UpdateAutoBrightness(WORD isAutoBrightness);
  virtual void                  UpdateMessageOverlayStop();
  virtual void                  UpdateSiteNameInfo(CSiteNameInfo* pSiteNameInfo);
  virtual void                  RefreshLayout();
  virtual void                  StartConnectProcess();
  virtual void                  CreatePartyIn();
  virtual void                  CreatePartyOut();
  virtual DWORD                 GetOutVideoRate();


  virtual DWORD                 GetLastVideoInSyncStatus() const;
  virtual void                  ChangeSpeakerNotationForPcmFecc(DWORD imageId);

  virtual BYTE                  IsPartyIntraSuppressed();

  virtual bool                  IsIntraSuppressEnabled(WORD intra_suppression_type) const;
  virtual void                  EnableIntraSuppress(WORD intra_suppression_type);
  virtual void                  DisableIntraSuppress(WORD intra_suppression_type);

  virtual BOOL	              IsVideoRelayParty()									  { return TRUE;}

  //From BridgePartyVideoRelayIn
  void AddImageToConfMix();
  void UpdateVideoBridgeOnRelayImageSvcToAvcTranslated();
  void UpdateOnImageAvcToSvcTranslate();
  void UpdateArtOnTranslateVideoSSRCAck( DWORD status );
  int  GetMaxAllowedLayerId();
  int  GetMaxAllowedLayerIdHighRes();
  void UpgradeSvcToAvcTranslator();
  //void ReplayUpgradeSvcToAvcTranslate( EStat status );
  void UpdateMrmpStreamIsMust( DWORD ssrc, DWORD videoRelayInChannelHandle, BOOL bIsMustSsrc );
  void UpdateMrmpStreamIsMustAck( DWORD status );



  //SvtToAvcTranslations
  void AvcImageAdded();
  void LastAvcImageRemoved();
  CVideoOperationPointsSet*     GetConfVideoOperationPointsSet()const;
  DWORD                         GetNumberOfNonRelayImages() const;

  void                          AskEpsForIntra(AskForRelayIntra& epIntraParams);
  void                          EpAskForIntra(const RelayIntraParam& intraParam);
  void							SendIntraRequestToParty(const std::list<unsigned int>& listSsrc,bool bIsGDR, bool allowSuppression = false);
  void							SendIntraRequestToPartyWithSuppressionTimer(RelayIntraParam& intraParam, bool bNeedTimer);
  bool							UpdateIntraSuppression(RelayIntraParam& intraParamsReceived, RelayIntraParam& intraParamsToSend, bool allowSuppression);
//  void 							SendVideoScpNotificationRequestToPartyCntl(CScpNotificationWrapper& scpNotifyReq);
  void 							SendScpNotificationReqToPartyCntl(CScpNotificationWrapper& scpNotifyReq);
  void 							SendScpPipeMappingToPartyCntl(CScpPipeMappingNotification& scpPipeMappingNotifyReq);
  void 							SendScpIvrShowSlideReqToPartyCntl();
  void							SendScpIvrStopShowSlideReqToPartyCntl();
  void 							UpdateArtOnTranslateVideoSSRC( DWORD ssrc );
  void 							UpdateImageOnTranslateSvcToAvcAck( DWORD status );
  BOOL 						    IsEnableHighVideoResInSvcToAvcMixMode();
  //SVC cascade
  virtual void                  SendUpdateToAudioBridgeOnSeenImage(PartyRsrcID idOfPartyToUpdate, PartyRsrcID idOfSeenParty);


protected:
  virtual void                  Setup(void);
  virtual void                  VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection);
  virtual void                  VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection);


  // Video Bridge Events

  //Disable unrelevant event and states
  virtual WORD                 IsValidState(WORD state) const;
  virtual WORD                 IsValidEvent(OPCODE event) const;

  virtual void                  OnVideoBridgeConnectIDLE(CSegment* pParam);
  virtual void                  OnVideoBridgeConnectSETUP(CSegment* pParam);
  virtual void                  OnVideoBridgeConnectCONNECTED(CSegment* pParam);
  virtual void                  OnVideoBridgeConnectCONNECTED_STANDALONE(CSegment* pParam);
//  virtual void                  OnVideoBridgeConnectDISCONNECTING(CSegment* pParam);
  virtual void                  OnVideoBridgeConnect(CSegment* pParam);

  virtual void 					OnVideoBridgeDisconnectCONNECTED_STANDALONE(CSegment* pParam);
  virtual void                  OnVideoBridgeUpdateVideoInParamsDISCONNECTING(CSegment* pParam);
  virtual void                  OnVideoBridgeUpdateVideoInParams(CSegment* pParam);

  virtual void                  OnVideoBridgeUpdateVideoOutParamsDISCONNECTING(CSegment* pParam);
  virtual void                  OnVideoBridgeUpdateVideoOutParams(CSegment* pParam);

  virtual void                  OnVideoBridgeAddImage(CSegment* pParam);
  virtual void                  OnVideoBridgeUpdateImage(CSegment* pParam);
  virtual void                  OnVideoBridgeMuteImage(CSegment* pParam);
  virtual void                  OnVideoBridgeUnMuteImage(CSegment* pParam);
  virtual void                  OnVideoBridgeChangeSpeakers(CSegment* pParam);
  virtual void                  OnVideoBridgeChangeAudioSpeaker(CSegment* pParam);
  virtual void                  OnVideoBridgeDelImage(CSegment* pParam);
  virtual void                  OnVideoBridgeDeletePartyFromConf(CSegment* pParam);
  virtual void 					OnVideoInDisconnected(CSegment* pParam);

  virtual void                  OnEpAskForIntraSETUP(CSegment* pParam);
  virtual void                  OnEpAskForIntraCONNECTED(CSegment* pParam);
  virtual void 					OnEpAskForIntraCONNECTED_STANDALONE(CSegment* pParam);
  virtual void 					OnEpAskForIntraDISCONNECTING(CSegment* pParam);

  virtual void 					OnVideoBridgeAddAvcImageSETUP(CSegment* pParam);
  virtual void 					OnVideoBridgeAddAvcImageCONNECTED(CSegment* pParam);
  virtual void					OnVideoBridgeAddAvcImageCONNECTED_STANDALONE(CSegment* pParam);
  virtual void 					OnVideoBridgeAddAvcImageDISCONNECTING(CSegment* pParam);
  virtual void 					OnVideoBridgeAddAvcImage(CSegment* pParam);

  virtual void 					OnVideoBridgeLastAvcImageRemovedSETUP(CSegment* pParam);
  virtual void 					OnVideoBridgeLastAvcImageRemovedCONNECTED(CSegment* pParam);
  virtual void 					OnVideoBridgeLastAvcImageRemovedCONNECTED_STANDALONE(CSegment* pParam);
  virtual void 					OnVideoBridgeLastAvcImageRemovedDISCONNECTING(CSegment* pParam);
  virtual void 					OnVideoBridgeLastAvcImageRemoved(CSegment* pParam);

  void 							OnVideoBridgeUpdateOnImageAvcToSvcCONNECTED(CSegment* pParam);
  virtual void 					OnVideoBridgeUpdateOnImageAvcToSvcCONNECTED_STANDALONE(CSegment* pParam);

  virtual void 					OnVideoBridgeShowSlideCONNECTED_STANDALONE(CSegment* pParam);
  virtual void 					OnVideoBridgeStopShowSlideCONNECTED_STANDALONE(CSegment* pParam);
  virtual void					OnVideoBridgeJoinConfCONNECTED_STANDALONE(CSegment* pParam);


  virtual void 					OnPartyUpgradeSvcToAvcTranslatorSETUP(CSegment* pParam);
  virtual void 					OnPartyUpgradeSvcToAvcTranslatorCONNECTED_STANDALONE(CSegment* pParam);
  virtual void 					OnPartyUpgradeSvcToAvcTranslatorCONNECTED(CSegment* pParam);
  virtual void 					OnPartyUpgradeSvcToAvcTranslatorDISCONNECTING(CSegment* pParam);


  void 							OnUpgradeSvcToAvcTranslator();

  virtual void                  TryStartGathering();
  virtual void 			        CheckIsMutedVideoInAndUpdateDB();
  void 							OnEpAskForIntra(CSegment* pParam);
  void 							OnVideoBridgeUpdateOnImageAvcToSvc(CSegment* pParam);

  virtual void 					OnTimerIntraSuppressionSETUP(CSegment* pParam);
  virtual void 					OnTimerIntraSuppressionCONNECTED(CSegment* pParam);
  virtual void 					OnTimerIntraSuppressionCONNECTED_STANDALONE(CSegment* pParam);
  virtual void 					OnTimerIntraSuppressionDISCONNECTING(CSegment* pParam);
  void							OnTimerIntraSuppression(CSegment* pParam);


	//statemachine
	//virtual void*                 GetMessageMap(void); from VBPARTYCNTL


	//virtual void                   Create(const CBridgePartyInitParams* pBridgePartyInitParams);

	//virtual void                   DisConnect();
	//virtual void                   Export();


private:
  SsrcSupressedIntraParams 	m_SsrcSuppressedIntras;
  DWORD						m_intraSuppressionDurationInMilliseconds;


	PDECLAR_MESSAGE_MAP
};
#endif /* VIDEORELAYBRIDGEPARTYCNTL_H_ */
