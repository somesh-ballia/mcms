// +========================================================================+
// VideoBridgeInterface.H                                                   |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       VideoBridgeInterface.H                                       |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Talya                                                        |
// -------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                    |
// -------------------------------------------------------------------------|
// +========================================================================+

#ifndef _CVideoBridgeInterface_H_
#define _CVideoBridgeInterface_H_

#include "BridgeInterface.h"
#include "VideoBridge.h"
#include "ConfPartyDefines.h"


class CVideoBridge;
class CVideoBridgeInitParams;
class CBridgePartyVideoParams;
class CTextOnScreenMngr;
class CGathering;
class CMessageOverlayInfo;
class CAvcToSvcParams;

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeInterface
////////////////////////////////////////////////////////////////////////////
class CVideoBridgeInterface : public CBridgeInterface
{
	CLASS_TYPE_1(CVideoBridgeInterface, CBridgeInterface)

public:
	                       CVideoBridgeInterface ();
	virtual               ~CVideoBridgeInterface ();

	virtual void           HandleEvent(CSegment* pMsg);
	virtual const char*    NameOf() const { return "CVideoBridgeInterface";}

	// Api from Conf
	virtual void           DeletePartyFromConf(const char* pDeletedParty);

	// Api from PartyCntl
	virtual void           UpdateMute(PartyRsrcID partyId, EOnOff eOnOff, WORD srcRequest);
	virtual void           UpdateVideoInParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void           UpdateVideoOutParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams);
	virtual void           UpdateVideoRelayInParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams);
	virtual void           UpdateVideoRelayOutParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams);
	virtual void           UpdateVideoOutParams(eXcodeRsrcType eEncoderPartyType, CBridgePartyVideoOutParams* pBridgePartyVideoParams);
	virtual void           RelayEpAskForIntra(CSegment* pParam);
	virtual void           TurnOnOffTelePresence(BYTE onOff, CVisualEffectsParams* pVisualEffects);
	virtual void           UpdatePartyTelePresenceMode(CTaskApp* pParty, eTelePresencePartyType partyNewTelePresenceMode);
	virtual void           SetTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode);// TELEPRESENCE_LAYOUTS
	virtual void           VideoRefresh(PartyRsrcID partyId);
	virtual void           EventModeIntraPreviewReq(PartyRsrcID partyId);
	virtual void           UpdateEMPartyStartCascadeLinkAsLecturerPendingMode(CTaskApp* pParty, WORD cascadeLecturerCopEncoderIndex);
	virtual void           SetSiteName(const char* partyName, const char* visualName);
	virtual void           AckOnIvrScpShowSlide(PartyRsrcID partyId, BYTE bIsAck);
	virtual void           AckOnIvrScpStopShowSlide(PartyRsrcID partyId, BYTE bIsAck);

	//Multiple links for ITP in cascaded conference feature:
	virtual void           UpdateNewSpeakerIndReceivedFromRemoteMCU(PartyRsrcID partyId, DWORD numOfActiveLinks, BYTE itpType);
	virtual void           UpdateNewSpeakerAckIndReceivedFromRemoteMCU(PartyRsrcID partyId);


	// Api from CAM
	virtual void           IvrPartyCommand(PartyRsrcID partyId, OPCODE opcode, CSegment* pDataSeg);
	virtual void           PLC_SetPartyPrivateLayout(PartyRsrcID partyId, LayoutType newPrivateLayoutType);
	virtual void           PLC_PartyReturnToConfLayout(PartyRsrcID partyId);
	virtual void           PLC_ForceToCell(PartyRsrcID partyId, PartyRsrcID partyImageToSee, BYTE cellToForce);
	virtual void           PLC_CancelAllPrivateLayoutForces(PartyRsrcID partyId);
	virtual void           VENUS_SetPartyPrivateLayout(PartyRsrcID partyId, LayoutType newPrivateLayoutType);
	virtual void           DisplayTextOnScreen(PartyRsrcID partyId, BYTE IsConfSecure, char** AudioArr, WORD numAudioParties, char** VideoArr, WORD numVideoParties, BOOL IsChairOnly);
	virtual void           DisplayPartyMsgOnScreen(PartyRsrcID partyId, char** displayStringArr, WORD displayStringArrNo);
	virtual void           DisplayPartyTextListOnScreen(PartyRsrcID partyId, CTextOnScreenMngr* TextMsgList, DWORD timeout = 5* SECOND /*SITE_NAME_DISPLAY_TOUT*/);
	virtual void           StopDisplayPartyTextOnScreen(PartyRsrcID partyId);
	virtual void           DisplayGatheringOnScreen(CGatheringManager* pGatheringManager);
	virtual void           DisplayGatheringOnScreen(const char* pPartyName, CGatheringManager* pGatheringManager);
	virtual bool           IsBridgePartyVideoOutStateIsConnected(const char* pPartyName);
	virtual void           SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects);
	virtual void           SetVisualEffectsParams(const std::string& sPartName, CVisualEffectsParams* pVisualEffects);

	// Api from PCM
	void                   ConnectPartyToPCMEncoder(PartyRsrcID partyId);
	void                   DisconnectPartyFromPCMEncoder(PartyRsrcID partyId);
	DWORD                  GetPartyResolutionInPCMTerms(PartyRsrcID partyId);
	CLayout*               GetPartyLayout(const char* partyName);
	void                   ChangeSpeakerNotation(PartyRsrcID partyId, DWORD imageId);
	void                   UpdateMessageOverlayForParty(PartyRsrcID partyId, CMessageOverlayInfo* pMessageOverlayInfo);
	void                   UpdateMessageOverlayStopForParty(PartyRsrcID partyId);
	void                   UpdateEMPartyStartCascadeLinlAsLecturerPendingMode(CTaskApp* pParty, WORD cascadeLecturerCopEncoderIndex);
	void                   UpgradeToMixAvcSvcForRelayParty(PartyRsrcID partyId);
	void                   UpgradeToMixAvcSvcForNonRelayParty(PartyRsrcID partyId, CAvcToSvcParams *pAvcToSvcParams);
	void                   DowngradeMixAvcSvcRelayParty(PartyRsrcID partyId);
	void                   DowngradeMixAvcSvcNonRelayParty(PartyRsrcID partyId);
	void                   UpdateVidBrdgTelepresenseEPInfo(PartyRsrcID partyId, CTelepresenseEPInfo* pTelepresenseEPInfo, const char* pSiteName); //_e_m_

	CVideoBridgePartyCntlContent* GetContentDecoder();

protected:
	                       CVideoBridgeInterface(const CVideoBridgeInterface&);
	CVideoBridgeInterface& operator=(const CVideoBridgeInterface&);
	void                   CreateImplementation(const CBridgeInitParams* pBridgeInitParams);
	CVideoBridge*          GetBridgeImplementation() {return (CVideoBridge*)m_pBridgeImplementation;}
};

#endif // _CVideoBridgeInterface_H_
