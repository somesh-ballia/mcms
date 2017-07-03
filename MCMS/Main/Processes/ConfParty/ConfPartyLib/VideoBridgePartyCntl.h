#ifndef _CVIDEOBRIDGEPARTYCNTL_H_
 #define _CVIDEOBRIDGEPARTYCNTL_H_
// +========================================================================+
//                     VideoBridgePartyCntl.H                               |
//             Copyright 1995 Pictel Technologies Ltd.                      |
//                    All Rights Reserved.                                  |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       VideoBridgePartyCntl.H                                       |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Talya                                                        |
// -------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                    |
// +========================================================================+
#include "BridgePartyCntl.h"
#include "Image.h"
#include "UpdatePartyVideoInitParams.h"
#include "TelepresenseEPInfo.h"
#include "TextOnScreenMngr.h"
#include "LPRData.h"

class CBridgePartyInitParams;
class CLayoutHandler;
class CLayout;
class CVisualEffectsParams;
class CVideoBridgePartyInitParams;
class CGathering;
class CMessageOverlayInfo;
class CAutoScanOrder;

enum eContentDecoderAction
{
  eContentDecoderAction_none       = 0,
  eContentDecoderAction_Connect    = 1, // insertThe content info to the decoder
  eContentDecoderAction_Disconnect = 2, // to conf task
  eContentDecoderAction_Destroy    = 3, // to party task
};

typedef std::vector< DWORD> DWORD_VECTOR;

// Timers opcodes
#define VIDEO_BRDG_PARTY_SETUP_TOUT              ((WORD)200)
#define VIDEO_BRDG_PARTY_DISCONNECT_TOUT         ((WORD)201)
#define VIDEO_BRDG_COP_PARTY_INTRA_SUPPRESS_TOUT ((WORD)202)
#define VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT ((WORD)205)

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntl
////////////////////////////////////////////////////////////////////////////
class CVideoBridgePartyCntl : public CBridgePartyCntl
{
	CLASS_TYPE_1(CVideoBridgePartyCntl, CBridgePartyCntl)

public:
	enum STATE {SETUP = (IDLE+1), CONNECTED_STANDALONE, CONNECTED, DISCONNECTING, EXPORT, ALLOCATE, DEALLOCATE, DISCONNECTED};

	                              CVideoBridgePartyCntl();
	                              CVideoBridgePartyCntl(const CVideoBridgePartyCntl& rOtherBridgePartyCntl);
	virtual                      ~CVideoBridgePartyCntl();
	CVideoBridgePartyCntl&        operator=(const CVideoBridgePartyCntl& rOtherBridgePartyCntl);
	virtual const char*           NameOf() const {return "CVideoBridgePartyCntl";}

public:
	virtual void                  Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	virtual void                  Update(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	virtual void                  Import(const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void                  ImportLegacy(const CBridgePartyInitParams* pBridgePartyInitParams, const CVideoBridgePartyCntl* pOldPartyCntl);

	virtual void                  InitiateUpdatePartyParams(const CBridgePartyInitParams* pBridgePartyInitParams);
	virtual void                  NewPartyOut();
	virtual void                  NewPartyIn();

	virtual BYTE                  IsLegacyParty() {return NO;}
	virtual char*                 StateToString();

	virtual void*                 GetMessageMap(void);
	virtual void                  Connect(BYTE isIVR, BYTE isContentHD1080Supported = FALSE);
	virtual void                  DisConnect(void);
	virtual void                  Export(void);
	virtual void                  Destroy(void);

	virtual BOOL                  IsConnectedStandalone() const          { return ((m_state == CONNECTED_STANDALONE) ? TRUE : FALSE); }
	virtual BOOL                  IsPcmOvrlyMsgOn()                      { return m_bIsPcmOvrlyMsg; }             // VNGR-15750-fix
	virtual void                  SetPcmOvrlyMsgOn(BOOL i_bIsPcmOvrlyMsg){ m_bIsPcmOvrlyMsg = i_bIsPcmOvrlyMsg; } // VNGR-15750-fix
	virtual CLayout*              GetReservationLayout(void) const;
	virtual CLayout*              GetCurrentLayout(void);
	virtual CLayout*              GetPrivateReservationLayout(void) const;
	virtual WORD                  GetIsPrivateLayout() const;
	virtual CVisualEffectsParams* GetPartyVisualEffects(void) const;
	virtual const CImage*         GetPartyImage(void) const;
	
	virtual EIconType             GetRecordingType() const;
	virtual WORD                  GetIsAudioParticipantsIconActive() const;
	virtual WORD                  GetNumAudioParticipantsInConf() const;
	virtual BOOL                  isInGatheringMode() const;
	
	virtual ePartyLectureModeRole GetPartyLectureModeRole() const;
	virtual BYTE                  IsConfSameLayout(void) const;
	BYTE                          IsImageInPartiesLayout(DWORD partyRscId);

	virtual void                  UpdateSelfMute(RequestPriority who, EOnOff eOnOff);

	virtual void                  AddImage(const CTaskApp* pParty);
	virtual void                  DelImage(const CTaskApp* pParty);
	virtual void                  ChangeAudioSpeaker(const CTaskApp* pParty);
	virtual void                  ChangeSpeakers(const CTaskApp* pNewVideoSpeaker, const CTaskApp* pNewAudioSpeaker);
	virtual void                  FastUpdate();
	virtual void                  MuteImage();
	virtual void                  UnMuteImage();
	virtual void                  UpdateImage();
	virtual void                  UpdateIndicationIcons(BOOL bUseSharedMemForIndicationIcon = FALSE);
	virtual void                  UpdateOnImageSvcToAvcTranslate();

	virtual void                  SetSiteName(const char* visualName);
	const char*                   GetSiteName();
	virtual void                  SetITPSiteName(const char* visualName);
	const char*                   GetITPSiteName();

	virtual void                  UpdateVideoInParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void                  UpdateVideoOutParams(CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void                  UpdatePartyTelePresenceMode(eTelePresencePartyType partyNewTelePresenceMode);
	virtual void                  IvrCommand(OPCODE opcode, CSegment* pDataSeg);
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
	virtual void                  ChangeLayoutOfTPRoomSublink(CLayout* pConfLayout, BORDERS_PARAM_S* pTPSpecificBorders);

	void                          ActionsAfterTelepresenceModeChanged(WORD isPrivate);

	virtual void                  UpdateVisualEffects(CVisualEffectsParams* pVisualEffects, BOOL bInternalUpdateOnly = FALSE);

	virtual void                  ForwardVINToParty(WORD mcuNumber, WORD terminalNumber, PartyRsrcID partyId);

	virtual LayoutType            GetConfLayoutType() const;
	virtual LayoutType            GetPartyCurrentLayoutType() const;

	virtual void                  SetPartyLectureModeRole(ePartyLectureModeRole partyLectureModeRole);

	void                          DeletePartyFromConf(const char* pDeletedPartyName);
	virtual void                  SendMsgToScreenPerParty(CTextOnScreenMngr* TextMsgList, DWORD timeout = 5* SECOND);
	virtual void                  StopMsgToScreenPerParty();
	virtual void                  SendVSRToSipParty(ST_VSR_MUTILPLE_STREAMS* multipleVsr);
	virtual void                  UpdateDecoderDetectedMode();

	CTaskApp*                     GetPartySouceInCellZero()                    {return m_pPartySouceInCellZero;}
	void                          SetPartySouceInCellZero(CTaskApp* partySouce){m_pPartySouceInCellZero = partySouce;}

	void                          MarkAsNewVideoSource(WORD resync);
	WORD                          IsMarkedAsNewVideoSource() {return m_resync;}

	void                          ResyncVideoSource();
	BYTE                          IsWaitingForChangeLayoutAck();
	virtual void                  GetRsrcProbAdditionalInfoOnVideoTimerSetup(BYTE& failureCauseDirection, BYTE& failureCauseAction);
	void                          OnVideoBridgeDisplayTextOnScreenCONNECTED(CSegment* pParam);
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
	void                          StartTelepresenceConnectionTimer();
        void                          StopTelepresenceConnectionTimer();
	virtual void                  CreatePartyIn();
	virtual void                  CreatePartyOut();
	BYTE                          IsConnectedState() const;
	virtual DWORD                 GetOutVideoRate();
	DWORD                         GetVideoMSI() const;
	void                          SetVideoMSI(DWORD videoMSI);
	bool                          GetIsForce1X1() const;

	virtual DWORD                 GetLastVideoInSyncStatus() const;
	virtual void                  ChangeSpeakerNotationForPcmFecc(DWORD imageId);
	void                          RemoveResourcesFromRoutingTable();
	virtual void                  ForwardFlowControlCommand(DWORD newRate, CLPRParams* LprParams = NULL);

	virtual BYTE                  IsPartyIntraSuppressed();

	virtual bool                  IsIntraSuppressEnabled(WORD intra_suppression_type) const;
	virtual void                  EnableIntraSuppress(WORD intra_suppression_type);
	virtual void                  DisableIntraSuppress(WORD intra_suppression_type);

	virtual BOOL                  IsVideoRelayParty()   { return FALSE;}
	virtual BOOL                  IsPartyValidForLayoutIndication();
	virtual void				  GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)const;

	const CTelepresenseEPInfo&          GetTelepresenceInfo() { return m_telepresenceInfo;}
	void                          SetTelepresenceInfo(const CTelepresenseEPInfo* tpInfo);

	void                          SetTelepresenceInfoLinkRole(BYTE linkRole) {m_telepresenceInfo.SetLinkRole(linkRole);}
	void                          SetTelepresenceInfoRoomID(DWORD roomID) {m_telepresenceInfo.SetRoomID(roomID);}
	void                          SetTelepresenceInfoNumOfLinks(DWORD numOfLinks) {m_telepresenceInfo.SetNumOfLinks(numOfLinks);}
	void                          SetTelepresenceInfoLinkNum(DWORD linkNum) {m_telepresenceInfo.SetLinkNum(linkNum);}
	void                          SetTelepresenceInfoEPtype(eTelePresencePartyType EPtype) {m_telepresenceInfo.SetEPtype(EPtype);}
	void                          SetTelepresenceInfoWaitForUpdate (BOOL bWaitForUpdate) {m_telepresenceInfo.SetWaitForUpdate(bWaitForUpdate);}

	const CBridge*                GetBridge()           { return m_pBridge; }
	CVideoOperationPointsSet*     GetConfVideoOperationPointsSet() const;
	void                          UpdateVideoBridgeOnNonRelayImageAvcToSvcTranslated();
	// VNGR-26449 - unencrypted conference message
	void                          StartSecureMessageTimer(int numOfUnencrypted, CMessageOverlayInfo* pMessageOverlayInfo);
	void                          StartMessageOverlayTimer(int numOfUnencrypted, CMessageOverlayInfo* pMessageOverlayInfo);
	void                          StopSecureMessageTimer();
	void                          StopMessageOverlayEndTimer();
	bool                          GetIsMessageOverlayEndTimerWorks();
	bool                          GetIsSecureMessageTimerWorks();
	void                          SetIsPermanentSecureMessageForParty(bool isPermanentSecureMessageForParty){ m_isPermanentSecureMessageForParty = isPermanentSecureMessageForParty; }
	bool                          GetIsPermanentSecureMessageForParty()                                     { return m_isPermanentSecureMessageForParty; }
	void                          SetNumOfUnencrypted(int numOfUnencrypted)                                 { m_numOfUnencrypted = numOfUnencrypted; }
	void                          SetMessageOverlayText(string messageOverlayText)                          {m_messageOverlayText = messageOverlayText; }
	string                        GetMessageOverlayText()                                                   { return m_messageOverlayText; }
	void                          SetIsMessageOverlayPermanent(bool isMessageOverlayPermanent)              { m_isMessageOverlayPermanent = isMessageOverlayPermanent;}
	bool                          GetIsMessageOverlayPermanent()                                            { return m_isMessageOverlayPermanent; }
	int                           GetNumOfUnencrypted()                                                     { return m_numOfUnencrypted; }
	WORD                          CalcMessageOverlayTime(CMessageOverlayInfo* pMessageOverlayInfo);
	bool                          IsTranslatorAvcSvcExists();
	void                          SendRelayIntraRequestToAvcToSvcTranslator(const std::list<unsigned int>& listSsrc, bool bIsGDR);
	void                          UpgradeAvcToSvcTranslator(CAvcToSvcParams *pAvcToSvcParams);
	void                          DisconnectSvcToAvcTranslator();
	void                          ReplayUpgradeSvcToAvcTranslate(EStat status);
	void                          ReplayDowngradeSvcToAvcTranslate(EStat status);
	bool                          IsRecordingLinkParty();
	void                          SetAVMCUMasterPartyRsrcID(PartyRsrcID masterPartyRscId){m_MS_masterPartyRsrcID = masterPartyRscId;}
	PartyRsrcID                   GetAVMCUMasterPartyRsrcID(){return m_MS_masterPartyRsrcID;}
	void                          SetMSAVMCUaudioLocalMsi(DWORD AudioMSI){m_MSaudioLocalMsi = AudioMSI;}
	DWORD                         GetMSAVMCUaudioLocalMsi(){return m_MSaudioLocalMsi;}
	void                          SetMSAVMCIndex(DWORD MSAVMCUIndex){m_MsAvMcuIndex=MSAVMCUIndex;}
	DWORD                         GetMSAVMCIndex(){return m_MsAvMcuIndex;}
	bool                          IsAVMCUMain() const;
	bool                          IsEPFromSameAVMCU(PartyRsrcID masterPartyRsrcID);
	bool                          IsAVMCUParty() const;
	bool			IsLastLayoutForRLLocked();
	BOOL                          GetPartyResumeFromHoldInIVR();
	// TELEPRESENCE_LAYOUTS
	ETelePresenceLayoutMode		  GetTelepresenceLayoutMode() const {return m_telepresenceLayoutMode;} // get party TelepresenceLayoutMode
	void						  ChangeTelepresenceLayoutMode(ETelePresenceLayoutMode newTelepresenceLayoutMode, bool init_change_layout); // change the party TelepresenceLayoutMode
	ETelePresenceLayoutMode 	  GetPartyTelepresenceLayoutModeConfiguration()const; // get TelepresenceLayoutMode from conference
	void 						  UpdateTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode); // call dispatch event
	void 						  OnVideoBridgeUpdateTelepresenceLayoutModeIDLE(CSegment* pParam);
	void 						  OnVideoBridgeUpdateTelepresenceLayoutModeSETUP(CSegment* pParam);
	void 						  OnVideoBridgeUpdateTelepresenceLayoutModeCONNECTED(CSegment* pParam);
	void 						  OnVideoBridgeUpdateTelepresenceLayoutModeCONNECTED_STANDALONE(CSegment* pParam);

	void UpdateLayoutHandlerType();	//for Telepresence

protected:
	virtual void                  Setup(void);

	virtual void                  VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection);
	virtual void                  VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection);
	// Video Bridge Events
	virtual void                  OnVideoBridgeConnectIDLE(CSegment* pParam);
	virtual void                  OnVideoBridgeConnectSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeConnectCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeConnectCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeConnectDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeConnect(CSegment* pParam);

	virtual void                  OnVideoBridgeDisconnectCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeDisconnectCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeDisconnectSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeDisconnectDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeDisconnect(CSegment* pParam);
	virtual void                  OnVideoBridgeDisconnectIDLE(CSegment* pParam);

	virtual void                  OnVideoBridgeExportSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeExportCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeExportDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeExportCONNECTED_STANDALONE(CSegment* pParam);

	virtual void                  OnVideoBridgeUpdateVideoInParamsSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoInParamsCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoInParamsCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoInParamsDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoInParams(CSegment* pParam);

	virtual void                  OnVideoBridgeUpdateVideoOutParamsSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoOutParamsCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoOutParamsCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoOutParamsDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoOutParams(CSegment* pParam);

	virtual void                  OnVideoBridgeAddImageCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeAddImageSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeAddImage(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateImage(CSegment* pParam);
	virtual void                  OnVideoBridgeFastUpdateSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeFastUpdateCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeFastUpdateCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeFastUpdate(CSegment* pParam);
	virtual void                  OnVideoBridgeMuteImageSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeMuteImageCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeMuteImage(CSegment* pParam);
	virtual void                  OnVideoBridgeUnMuteImageSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeUnMuteImageCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeUnMuteImage(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeSpeakersSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeSpeakersCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeSpeakers(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeAudioSpeakerSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeAudioSpeakerCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeAudioSpeaker(CSegment* pParam);
	virtual void                  OnVideoBridgeDelImageSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeDelImageCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeDelImage(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeConfLayoutSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeConfLayoutCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeConfLayout(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeLayoutOfTPRoomSublinkSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeLayoutOfTPRoomSublinkCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeLayoutOfTPRoomSublinkSTANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeLayoutOfTPRoomSublink(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyLayoutSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyLayoutCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyLayoutCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyLayoutDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyLayout(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyPrivateLayoutSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyPrivateLayoutCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyPrivateLayoutCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeChangePartyPrivateLayout(CSegment* pParam);
	virtual void                  OnVideoBridgeSetPrivateLayoutOnOffSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeSetPrivateLayoutOnOffCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeSetPrivateLayoutOnOffCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeSetPrivateLayoutOnOff(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeVisualEffectsSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeVisualEffectsCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeVisualEffectsCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeVisualEffects(CSegment* pParam);

	virtual void                  OnVideoBridgeShowSlideCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeShowSlideCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeStopShowSlideCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeStopShowSlideCONNECTED(CSegment* pParam);

	virtual void                  OnVideoBridgeJoinConfCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeJoinConfCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeJoinConfIDLE(CSegment* pParam);

	virtual void                  OnVideoBridgeStartPLCCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeStopPLCCONNECTED(CSegment* pParam);

	virtual void                  OnVideoBridgePLC_SetPrivateLayoutTypeCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgePLC_ReturnToConfLayoutCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgePLC_ForceCellCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgePLC_CancelAllPrivateLayoutForcesCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdatePartyLectureModeRoleSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdatePartyLectureModeRoleCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdatePartyLectureModeRoleCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdatePartyLectureModeRoleDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdatePartyLectureModeRole(CSegment* pParam);
	virtual void                  OnVideoBridgeDeletePartyFromConfSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeDeletePartyFromConfCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeDeletePartyFromConf(CSegment* pParam);

	virtual void                  OnVideoBridgeUpdateVideoClarityIDLE(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoClaritySETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoClarityCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoClarityCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoClarityDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateVideoClarity(CSegment* pParam);

	virtual void                  OnVideoBridgeUpdateAutoBrightnessIDLE(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateAutoBrightnessSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateAutoBrightnessCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateAutoBrightnessCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateAutoBrightnessDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateAutoBrightness(CSegment* pParam);

	virtual void                  OnVideoBridgeChangeSpeakerNotationForPcmFeccIDLE(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeSpeakerNotationForPcmFeccSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeSpeakerNotationForPcmFeccCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeChangeSpeakerNotationForPcmFeccDISCONNECTING(CSegment* pParam);

	virtual void                  OnVideoOutConnectedSETUP(CSegment* pParam);
	virtual void                  OnVideoOutConnectedCONNECTED(CSegment* pParam);
	virtual void                  OnVideoOutConnectedDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoInConnectedSETUP(CSegment* pParam);
	virtual void                  OnVideoInConnectedCONNECTED(CSegment* pParam);
	virtual void                  OnVideoInConnectedDISCONNECTING(CSegment* pParam);

	virtual void                  OnVideoInSyncedSETUP(CSegment* pParam);
	virtual void                  OnVideoInSyncedCONNECTED(CSegment* pParam);
	virtual void                  OnVideoInSynced(CSegment* pParam);
	virtual void                  TryStartGathering();

	virtual void                  OnVideoInDisconnectedSETUP(CSegment* pParam);
	virtual void                  OnVideoInDisconnectedCONNECTED(CSegment* pParam);
	virtual void                  OnVideoInDisconnectedDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoInDisconnected(CSegment* pParam);
	virtual void                  OnVideoOutDisconnectedSETUP(CSegment* pParam);
	virtual void                  OnVideoOutDisconnectedCONNECTED(CSegment* pParam);
	virtual void                  OnVideoOutDisconnectedDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoOutDisconnected(CSegment* pParam);
	virtual void                  OnVideoInUpdatedSETUP(CSegment* pParam);
	virtual void                  OnVideoInUpdatedCONNECTED(CSegment* pParam);
	virtual void                  OnVideoInUpdatedDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoInUpdatedCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoInUpdated(CSegment* pParam);

	virtual void                  OnVideoOutUpdatedSETUP(CSegment* pParam);
	virtual void                  OnVideoOutUpdatedCONNECTED(CSegment* pParam);
	virtual void                  OnVideoOutUpdatedDISCONNECTING(CSegment* pParam);
	virtual void                  OnVideoOutUpdatedCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoOutUpdated(CSegment* pParam);

	virtual void                  OnPartyImageUpdatedSETUP(CSegment* pParam);
	virtual void                  OnPartyImageUpdatedCONNECTED(CSegment* pParam);
	virtual void                  OnPartyImageUpdatedDISCONNECTING(CSegment* pParam);
	virtual void                  OnPartyImageUpdatedCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnPartyImageUpdated(CSegment* pParam);

	virtual void                  OnPartyResumeFromHoldCONNECTED(CSegment* pParam);
	virtual void                  OnPartyResumeFromHoldANYCASE(CSegment* pParam);
	virtual void                  OnVideoOutVideoRefresh(CSegment* pParam);
	virtual void                  OnVideoInSendH239Caps(CSegment* pParam);

	virtual void                  OnVideoOutPartyLayoutChanged(CSegment* pParam);
	virtual void                  OnVideoOutPrivateLayoutOnOffChanged(CSegment* pParam);

	virtual void                  OnVideoInVideoDecoderSyncChanged(CSegment* pParam);

	virtual void                  OnVideoOutEndChangeLayout(CSegment* pParam);
	virtual void                  OnVideoInUpdateDecoderDetectedModeCONNECTED(CSegment* pParam);

	virtual void                  OnTimerPartySetupSETUP(CSegment* pParams);
	virtual void                  OnTimerPartySetupCONNECTED(CSegment* pParams);
	virtual void                  OnTimerPartySetup(CSegment* pParams);
	virtual void                  OnTimerPartyDisconnectSETUP(CSegment* pParams);
	virtual void                  OnTimerPartyDisconnectCONNECTED(CSegment* pParams);
	virtual void                  OnTimerPartyDisconnect(CSegment* pParams);
	virtual void                  OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams);

	virtual void                  OnVideoBridgeIndicationIconsChangeCONNECTED(CSegment* pParam);

	virtual void                  OnTimerMessageOverlayCONNECTED(CSegment* pParam);
	// VNGR-26449 - unencrypted conference message
	virtual void                  OnTimerSecureMessageCONNECTED(CSegment* pParam);
	virtual void                  OnTimerMessageOverlayEndCONNECTED(CSegment* pParam);

	virtual void                  OnTimerMultipleTPConnectionsCONNECTED(CSegment* pParam);

	void                          OnSetSiteNameConnected(CSegment* pParams);
	void                          OnSetSiteNameDISCONNECTING(CSegment* pParams);

	virtual void                  OnVideoBridgeUpdateOnImageSvcToAvc(CSegment* pParam);

	virtual void 				  OnPartyUpgradeAvcToSvcTranslatorCONNECTED(CSegment* pParam);
	virtual void				  OnPartyUpgradeAvcToSvcTranslatorCONNECTED_STANDALONE(CSegment* pParam);
	virtual void 				  OnPartyUpgradeAvcToSvcTranslatorSETUP(CSegment* pParam);
	virtual void 				  OnPartyUpgradeAvcToSvcTranslatorDISCONNECTING(CSegment* pParam);

	void 						  OnUpgradeSvcToAvcTranslator();

	virtual BOOL                  IsUpdateLayoutAsPrivateInDB();
	virtual BOOL                  IsPartyNoiseSuppressed()                  { return m_IsPartyNoiseSuppressed; }

	virtual BOOL                  IsIntraSuppressionSupportedForThisParty() { return FALSE; }
	void                          OnTimerPartyIntraSuppressed(CSegment* pParams);

	virtual void                  OnTimerIgnoreIntraSETUP(CSegment* pParams);
	virtual void                  OnTimerIgnoreIntraCONNECTED(CSegment* pParams);
	virtual DWORD                 GetMaxIntraRequestsPerInterval()       { return 0; }
	virtual DWORD                 GetIntraSuppressionDurationInSeconds() { return 0; }
	void                          OnTimerIgnoreIntra();
//	void                          SendIntraToAvcToSvcIfExists();
	
	virtual void 			      CheckIsMutedVideoInAndUpdateDB();
	
	void                          OnPartyUpgradeAvcToSvcTranslator(CSegment* pParam);

	virtual void                  OnVideoBridgeUpdateLayoutHandlerTypeSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateLayoutHandlerTypeCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateLayoutHandlerTypeCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeUpdateLayoutHandlerType(CSegment* pParam);

	virtual void  OnTimerTelepresenceInfoUpdateCONNECTED(CSegment* pParam);
	bool IsTelepresenceLayoutsManagedInternally();

protected:

	CTaskApp*                     m_pPartySouceInCellZero;
	WORD                          m_resync;
	BOOL                          m_bIsAfterMove;
	BOOL                          m_bIsPcmOvrlyMsg;                            // VNGR-15750-fix
	bool                          m_bVideoInSyncedReadyForGathering;
	bool                          m_bVideoOutConnectedReadyForGathering;
	CUpdatePartyVideoInitParams*  m_pUpdatePartyInitParams;
	BOOL                          m_IsPartyNoiseSuppressed;
	DWORD_VECTOR*                 m_PartyIntraRequestsTime;
	BYTE                          m_isIntraForLinksSuppressed;                 // when this flag is true and we get intra request from MGC link we will block the intra for X seconds
	BYTE                          m_isIntraRequestReceivedFromWhileSuppressed;
	bool                          m_isIntraSupressionEnabled;
	CTelepresenseEPInfo           m_telepresenceInfo;
	// VNGR-26449 - unencrypted conference message
	bool                          m_isPermanentSecureMessageForParty;
	int                           m_numOfUnencrypted;
	string                        m_messageOverlayText;
	bool                          m_isMessageOverlayPermanent;
	char                          m_ITPSiteName[H243_NAME_LEN];
	DWORD                         m_MS_masterPartyRsrcID;
	DWORD                         m_MSaudioLocalMsi;
	BYTE                          m_MsAvMcuIndex;
	BOOL                          m_bIsResumingIVR;
	ETelePresenceLayoutMode       m_telepresenceLayoutMode; // TELEPRESENCE_LAYOUTS

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntlVSW
////////////////////////////////////////////////////////////////////////////
class CVideoBridgePartyCntlVSW : public CVideoBridgePartyCntl
{
	CLASS_TYPE_1(CVideoBridgePartyCntlVSW, CVideoBridgePartyCntl)

public:
	                              CVideoBridgePartyCntlVSW();
	virtual                      ~CVideoBridgePartyCntlVSW();
	virtual const char*           NameOf() const                            { return "CVideoBridgePartyCntlVSW";}
	virtual void                  Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	virtual void                  Update(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);

	virtual LayoutType            GetConfLayoutType() const;

	void                          ResetImage0(DWORD partyRscId);
	void                          ResetRsrvImage0(DWORD partyRscId);
	void                          SetPartyForce(CVideoLayout& pPartyLayout);
	virtual void                  SetPartyFlowControlRate(DWORD rate)       {m_partyFlowControlRate = rate;}
	virtual DWORD                 GetPartyFlowControlRate() const           {return m_partyFlowControlRate;}
	virtual void                  SetIsCascadeParty(BOOL bCascade)          {m_bIsCascadeParty = bCascade;}
	virtual BOOL                  GetIsCascadeParty() const                 {return m_bIsCascadeParty;}
	virtual void                  ForwardFlowControlCommand(DWORD newRate, CLPRParams* LprParams = NULL);

	virtual void                  CreatePartyIn();
	virtual void                  CreatePartyOut();

protected:
	virtual void                  OnVideoBridgeFastUpdate(CSegment* pParam);
	virtual void                  OnVideoBridgeFastUpdateCONNECTED_STANDALONE(CSegment* pParam);
	void                          OnVideoOutSyncIndCONNECTED(CSegment* pParam);
	void                          OnVideoOutVideoRefreshCONNECTED_STANDALONE(CSegment* pParam);  // intra request of slide

	virtual BOOL                  IsIntraSuppressionSupportedForThisParty() { return TRUE; }
	void                          OnTimerPartyIntraSuppressed(CSegment* pParams);                // suppression of a noisy participant

	virtual DWORD                 GetMaxIntraRequestsPerInterval();
	virtual DWORD                 GetIntraSuppressionDurationInSeconds();

	DWORD                         m_partyFlowControlRate;
	BOOL                          m_bIsCascadeParty;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntlLegacy
////////////////////////////////////////////////////////////////////////////
class CVideoBridgePartyCntlLegacy : public CVideoBridgePartyCntl
{
	CLASS_TYPE_1(CVideoBridgePartyCntlLegacy, CVideoBridgePartyCntl)

public:
	                             CVideoBridgePartyCntlLegacy ();
	                             CVideoBridgePartyCntlLegacy (const CVideoBridgePartyCntlLegacy& rOtherBridgePartyCntlLegacy);
	virtual                     ~CVideoBridgePartyCntlLegacy ();
	CVideoBridgePartyCntlLegacy& operator=(const CVideoBridgePartyCntl& rVideoBridgePartyCntl);
	virtual const char*          NameOf() const { return "CVideoBridgePartyCntlLegacy";}

	void                         SetParamsRelatedToMove(BYTE oldPartyState);
	virtual BYTE                 IsLegacyParty() {return YES;}
	virtual void                 AddContentImage();
	virtual void                 DelContentImage();

protected:
	virtual void                 NewPartyOut();

	void                         OnVideoBridgeAddContentImageCONNECTED(CSegment* pParam);
	void                         OnVideoBridgeAddContentImageSETUP(CSegment* pParam);
	void                         OnVideoBridgeAddContentImage(CSegment* pParam);

	void                         OnVideoBridgeDelContentImage(CSegment* pParam);
	void                         OnVideoBridgeDelContentImageCONNECTED(CSegment* pParam);
	void                         OnVideoBridgeDelContentImageSETUP(CSegment* pParam);

	virtual BOOL                 IsUpdateLayoutAsPrivateInDB();

PDECLAR_MESSAGE_MAP
  const CBridge*                GetBridge()     {return m_pBridge;}
};


////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntlContent
////////////////////////////////////////////////////////////////////////////
class CVideoBridgePartyCntlContent : public CVideoBridgePartyCntl
{
	CLASS_TYPE_1(CVideoBridgePartyCntlContent, CVideoBridgePartyCntl)

public:
	                              CVideoBridgePartyCntlContent(CBridge* pBridge);
	                              CVideoBridgePartyCntlContent(const CVideoBridgePartyCntlContent& rOtherBridgePartyCntlContent);
	virtual                      ~CVideoBridgePartyCntlContent();
	virtual const char*           NameOf() const { return "CVideoBridgePartyCntlContent";}
	virtual void                  Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);

	void                          SetBridge(CBridge* pBridge) {m_pBridge = pBridge;}

	virtual void                  Setup();
	void                          Allocate();
	void                          AllocateResources();
	void                          DeAllocate();
	void                          DeAllocateResources();
	void                          CreateAndSendDeallocatePartyResources();
	STATUS                        SendReqToResourceAllocator(CSegment* seg, OPCODE opcode);

	virtual void                  OnVideoBridgeConnectIDLE(CSegment* pParam);
	void                          OnVideoBridgeConnectALLOCATE(CSegment* pParam);
	virtual void                  OnVideoBridgeConnectSETUP(CSegment* pParam);
	void                          OnVideoBridgeConnectCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeConnectDISCONNECTING(CSegment* pParam);
	void                          OnVideoBridgeConnectDISCONNECTED(CSegment* pParam);
	void                          OnVideoBridgeConnectDEALLOCATE(CSegment* pParam);


	virtual void                  VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection);
	virtual void                  VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection);

	virtual void                  OnVideoBridgeDisconnectSETUP(CSegment* pParam);
	virtual void                  OnVideoBridgeDisconnectCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeDisconnectDISCONNECTING(CSegment* pParam);
	void                          OnVideoBridgeDisconnectDISCONNECTED(CSegment* pParam);
	void                          OnVideoBridgeDisconnectALLOCATE(CSegment* pParam);
	void                          OnVideoBridgeDisconnectDEALLOCATE(CSegment* pParam);


	void                          OnVideoBridgeDestroyIDLE(CSegment* pParam);
	void                          OnVideoBridgeDestroyALLOCATE(CSegment* pParam);
	void                          OnVideoBridgeDestroySETUP(CSegment* pParam);
	void                          OnVideoBridgeDestroyCONNECTED(CSegment* pParam);
	void                          OnVideoBridgeDestroyDISCONNECTING(CSegment* pParam);
	void                          OnVideoBridgeDestroyDISCONNECTED(CSegment* pParam);
	void                          OnVideoBridgeDestroyDEALLOCATE(CSegment* pParam);


	virtual void                  OnVideoBridgeDisconnect(CSegment* pParam);
	void                          OnVideoBridgeDestroy(CSegment* pParam);

	void                          OnRsrcDeallocatePartyRspDEALLOCATE(CSegment* pParams);
	void                          OnRsrcAllocatePartyRspALLOCATE(CSegment* pParams);
	void                          OnTimerRADisconnectDEALLOCATE(CSegment* pParams);


	virtual void                  OnTimerPartySetup(CSegment* pParams);

	virtual void                  OnTimerPartyDisconnectSETUP(CSegment* pParams);
	virtual void                  OnTimerPartyDisconnectCONNECTED(CSegment* pParams);
	virtual void                  OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams);

	virtual void                  OnVideoOutVideoRefresh(CSegment* pParam);
	virtual void                  OnVideoInDisconnected(CSegment* pParam);

	void                          OnVideoInSynced(CSegment* pParam);
	void                          UpdateDecoderDetectedMode();
	void                          SetConnectDisconnectDestroyFlags(eContentDecoderAction action);
	void                          ResetConnectDisconnectDestroyFlags();
	void                          SetDecoderParamsBeforeConnect();

	STATUS                        SendSyncReqToRsrcAllocAndSaveParams(CSegment* seg, OPCODE opcode);
	void                          InsertPartyResourcesToGlobalRsrcRoutingTbl();
	void                          HandleRsrcAllocatePartyFailure();
	void                          ResetDecoderFailureStatusInConf();
	void                          UpdateVideoInParamsOnChange();

	PartyRsrcID                   GetPartyRsrcId();
	ConfRsrcID                    GetConfRsrcId();

	void                          Destroy();

	BOOL                          IsDestroy()    {return m_isDestroy;}
	BOOL                          IsDisconnect() {return m_isDisconnect;}
	BOOL                          IsConnect()    {return m_isConnect;}

	BOOL                          IsContentDecoderSynced();
                                CVideoBridgePartyCntlContent(){}
  void                          SetMonitoringPartyId(DWORD monitorPartyId){m_monitorPartyId = monitorPartyId;}
  DWORD                         GetMonitoringPartyId(){return m_monitorPartyId;}
 void                           SetPartyAllocateRsrc(CPartyRsrcDesc* pPartyRsrcDesc){m_pPartyAllocatedRsrc = pPartyRsrcDesc;}
 CPartyRsrcDesc*                GetPartyAllocateRsrc(){return m_pPartyAllocatedRsrc;}

protected:
	BOOL                          m_isConnect;              // StartContent or
	BOOL                          m_isDisconnect;           // StopContent
	BOOL                          m_isDestroy;              // DeleteConference
	DWORD                         m_monitorPartyId;         // When allocating resources we need party monitor ID
	WORD                          m_isFaulty;               // If YES ==> Problem with the resource Invoking KillPort process in RA.
	BYTE                          m_isContentHD1080Supported;
	CPartyRsrcDesc*               m_pPartyAllocatedRsrc;

	PDECLAR_MESSAGE_MAP
protected:
};
////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgePartyCntlCOP
////////////////////////////////////////////////////////////////////////////
class CVideoBridgePartyCntlCOP : public CVideoBridgePartyCntl
{
	CLASS_TYPE_1(CVideoBridgePartyCntlCOP, CVideoBridgePartyCntl)

public:
	                              CVideoBridgePartyCntlCOP();
	                              CVideoBridgePartyCntlCOP(const CVideoBridgePartyCntlCOP& rOtherBridgePartyCntl);
	virtual                      ~CVideoBridgePartyCntlCOP();
	CVideoBridgePartyCntlCOP&     operator=(const CVideoBridgePartyCntlCOP& rVideoBridgePartyCntl);
	virtual const char*           NameOf() const { return "CVideoBridgePartyCntlCOP";}

	void                          OnVideoOutConnectedToPCM(CSegment* pParam);
	void                          OnVideoOutConnectedToPCMSETUP(CSegment* pParam);
	void                          OnVideoOutConnectedToPCMCONNECTED(CSegment* pParam);
	void                          OnVideoOutConnectedToPCMDISCONNECTING(CSegment* pParam);

	void                          OnVideoOutDisconnectedFromPCM(CSegment* pParam);
	void                          OnVideoOutDisconnectedFromPCMSETUP(CSegment* pParam);
	void                          OnVideoOutDisconnectedFromPCMCONNECTED(CSegment* pParam);
	void                          OnVideoOutDisconnectedFromPCMDISCONNECTING(CSegment* pParam);

	void                          OnPCMVideoOutUpdated(CSegment* pParam);

public:
	virtual void                  Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	DWORD                         GetCopEncoderEntityId();

	virtual void                  Update(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
	virtual void                  NewPartyOut();
	virtual void                  NewPartyIn();

	void                          VideoRefresh(); // In case of COP the VDCop will ask the right VBPartyCntl to ask for intra from the right partyApi since the Decoder doesn't now the PartyApi of the party Connected to it
	virtual WORD                  IsValidState(WORD state) const;
	virtual WORD                  IsValidEvent(OPCODE event) const;

	void                          SetDecoderConnectionIdInImage(DWORD decoderConnectionId);
	void                          SetDecoderPartyIdInImage(DWORD decoderPartyId);

	DWORD                         GetDecoderConnectionIdInImage();
	DWORD                         GetDecoderPartyIdInImage();

	void                          SetDspSmartSwitchConnectionId(DWORD decoderConnectionId);
	void                          SetDspSmartSwitchEntityId(DWORD decoderPartyId);


	void                          GetCurrentCopDecoderResolution(DWORD& algorithm, ECopDecoderResolution& copDecoderResolution);
	void                          GetInitialCopDecoderResolution(DWORD& algorithm, ECopDecoderResolution& copDecoderResolution);
	void                          GetInParamFromPartyCntl(CBridgePartyVideoInParams* pInVideoParams);

	void                          ConnectToPCMEncoder(DWORD pcmEncoderConnectionId, DWORD pcmEncoderPartyId);
	void                          DisconnectFromPCMEncoder();

	BOOL                          IsVideoInDisconnected();

	void                          CopPartyConfVideoRefresh();

	void                          SetPartyFlowControlRate(DWORD rate) {m_partyFlowControlRate = rate;}
	DWORD                         GetPartyFlowControlRate() const     {return m_partyFlowControlRate;}
	virtual void                  ForwardFlowControlCommand(DWORD newRate, CLPRParams* LprParams = NULL, BYTE forOutChannel = TRUE);
	BOOL                          IsConnectedOrConnectingPCM();
	void                          UpdatePartyOnStopFlowControlConstraint();

	BYTE                          IsUpdateEncoderActive();

	void                          UpdateLevelEncoderInDB(WORD levelEncoder);

	BYTE                          IsReadyToStartLecturer() const;
	BYTE                          IsSyncWithDecoderResolution();

	BYTE                          IsRemoteNeedSmartSwitchAccordingToVendor();
	void                          SetIsRemoteNeedSmartSwitchAccordingToVendor(BYTE isRemoteNeedSmartSwitchAccordingToVendor);

	BYTE                          IsPartyCascadeLinkSupportAsymmetricEMCascade();
	WORD                          GetCopEncoderIndexOfCascadeLinkLecturer();

protected:
	void                          OnVideoBridgeFastUpdate(CSegment* pParam);
	void                          OnVideoBridgeFastUpdateCONNECTED_STANDALONE(CSegment* pParam);

	virtual void                  OnVideoBridgeConnectIDLE(CSegment* pParam);
	virtual void                  OnVideoBridgeConnectCONNECTED(CSegment* pParam);
	virtual void                  OnVideoBridgeConnect(CSegment* pParam);

	virtual void                  OnVideoBridgeDisconnectCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoBridgeJoinConfCONNECTED_STANDALONE(CSegment* pParam);
	virtual void                  OnVideoInDisconnected(CSegment* pParam);

	virtual BOOL                  IsIntraSuppressionSupportedForThisParty() { return TRUE; }

	virtual DWORD                 GetMaxIntraRequestsPerInterval();
	virtual DWORD                 GetIntraSuppressionDurationInSeconds();

	DWORD                         m_partyFlowControlRate;  // suppression of a noisy participant

	PDECLAR_MESSAGE_MAP
};

/////////////////////////////////////////////////////////////////////////////////////
//////////////////   XCode    /////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


class CVideoBridgePartyCntlXCode : public CVideoBridgePartyCntlCOP
{
CLASS_TYPE_1(CVideoBridgePartyCntlXCode,CVideoBridgePartyCntlCOP)
public:
                                CVideoBridgePartyCntlXCode();
                                CVideoBridgePartyCntlXCode(const CVideoBridgePartyCntlXCode& rOtherBridgePartyCntl);
  virtual                      ~CVideoBridgePartyCntlXCode();
  CVideoBridgePartyCntlXCode&     operator=(const CVideoBridgePartyCntlXCode& rVideoBridgePartyCntl);
  virtual const char*           NameOf() const { return "CVideoBridgePartyCntlXCode";}


public:
  virtual void                  Create(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
  virtual void                  Setup();
  virtual void                  Update(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams);
  virtual void                  NewPartyOut();
  virtual void                  NewPartyIn();
  virtual void                  VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection);
  virtual void                  VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection);

  // Romeme - TBD is needed
  DWORD GetXCodeEncoderEntityId();
  WORD  GetXCodeEncoderIndex();
  DWORD GetXCodeEncoderConnectionID();

/*
 void VideoRefresh(); //In case of COP the VDCop will ask the right VBPartyCntl to ask for intra from the right partyApi since the Decoder doesn't now the PartyApi of the party Connected to it


BYTE IsUpdateEncoderActive();
  ;*/


protected:
  virtual void OnVideoBridgeConnectIDLE(CSegment* pParam);
  virtual void OnVideoBridgeConnectSETUP(CSegment* pParam);
  virtual void OnVideoBridgeConnectCONNECTED(CSegment* pParam);
  virtual void OnVideoBridgeConnect(CSegment* pParam);
  virtual void OnTimerPartyDisconnect(CSegment* pParams);
  virtual void OnTimerPartySetup(CSegment* pParams);
  virtual void OnTimerPartyDisconnectDISCONNECTING(CSegment* pParams);
  virtual void OnVideoBridgeDisconnect(CSegment* pParams);
  virtual void OnVideoBridgeDisconnectIDLE(CSegment* pParam);
	PDECLAR_MESSAGE_MAP

};

#endif //_CVideoBridgePartyCntl_H_


