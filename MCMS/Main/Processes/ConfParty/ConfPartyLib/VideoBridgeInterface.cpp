// VideoBridgeInterface.CPP                                                 |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       VideoBridgeInterface.CPP                                     |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Talya                                                        |
// -------------------------------------------------------------------------|
// Who  | Date  July-2005  | Description                                    |
// -------------------------------------------------------------------------|
// +========================================================================+

#include "VideoBridgeVsw.h"
#include "VideoBridgeInterface.h"
#include "VideoBridgeInitParams.h"
#include "BridgePartyInitParams.h"
#include "VideoBridgeCop.h"
#include "VideoBridgeCopEQ.h"
#include "Gathering.h"

////////////////////////////////////////////////////////////////////////////
//                        CVideoBridgeInterface
////////////////////////////////////////////////////////////////////////////
CVideoBridgeInterface::CVideoBridgeInterface ()
{
}

////////////////////////////////////////////////////////////////////////////
CVideoBridgeInterface::~CVideoBridgeInterface ()
{
	POBJDELETE(m_pBridgeImplementation);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::CreateImplementation(const CBridgeInitParams* pVideoBridgeInitParams)
{
	POBJDELETE(m_pBridgeImplementation);

	switch (pVideoBridgeInitParams->GetBridgeImplementationType())
	{
		case eVideoCP_Bridge_V1:        // Implementation of CP bridge
			m_pBridgeImplementation = new CVideoBridgeCP;
			break;

		case eVideoCPContent_Bridge_V1: // Implementation of Legacy bridge
			m_pBridgeImplementation = new CVideoBridgeCPContent;
			break;

		case eVideoSW_Bridge_V1:        // Implementation of VSW bridge
			m_pBridgeImplementation = new CVideoBridgeVSW;
			break;

		case eVideoCOP_Bridge_V1:       // Implementation of COP bridge
			m_pBridgeImplementation = new CVideoBridgeCOP;
			break;

		case eVideoCOPEq_Bridge_V1:     // Implementation of COP EQ bridge
			m_pBridgeImplementation = new CVideoBridgeCOPEq;
			break;

		case eVideoXcode_Bridge_V1:  // Content Transcoding
			m_pBridgeImplementation = new CVideoContentXcodeBridge;
			break;

		default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	PASSERT_AND_RETURN(!GetBridgeImplementation());

	GetBridgeImplementation()->Create((CVideoBridgeInitParams*)pVideoBridgeInitParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::HandleEvent(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);
	OPCODE opcode;
	*pMsg >> opcode;

	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->HandleEvent(pMsg, 0, opcode);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateMute(PartyRsrcID partyId, EOnOff eOnOff, WORD srcRequest)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->UpdateMute(partyId, eOnOff, srcRequest);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateVideoInParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateVideoInParams(pParty, pBridgePartyVideoParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateVideoOutParams(CTaskApp* pParty, CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateVideoOutParams(pParty, pBridgePartyVideoParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateVideoRelayInParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateVideoRelayInParams(pParty, pBridgePartyVideoRelayMediaParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateVideoRelayOutParams(CTaskApp* pParty, CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayMediaParams)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateVideoRelayOutParams(pParty, pBridgePartyVideoRelayMediaParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::RelayEpAskForIntra(CSegment* pParam)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->RelayEpAskForIntra(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateVideoOutParams(eXcodeRsrcType eEncoderPartyType, CBridgePartyVideoOutParams* pBridgePartyVideoParams)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateVideoOutParams(eEncoderPartyType, pBridgePartyVideoParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::TurnOnOffTelePresence(BYTE onOff, CVisualEffectsParams* pVisualEffects)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->TurnOnOffTelePresence(onOff, pVisualEffects);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdatePartyTelePresenceMode(CTaskApp* pParty, eTelePresencePartyType partyNewTelePresenceMode)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdatePartyTelePresenceMode(pParty, partyNewTelePresenceMode);
}
////////////////////////////////////////////////////////////////////////////
// TELEPRESENCE_LAYOUTS
void CVideoBridgeInterface::SetTelepresenceLayoutMode(ETelePresenceLayoutMode newLayoutMode)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->SetTelepresenceLayoutMode(newLayoutMode);
}
////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::VideoRefresh(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->VideoRefresh(partyId);
}

////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature
void CVideoBridgeInterface::UpdateNewSpeakerIndReceivedFromRemoteMCU(PartyRsrcID partyId, DWORD numOfActiveLinks, BYTE itpType)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->UpdateNewSpeakerIndReceivedFromRemoteMCU(partyId, numOfActiveLinks, itpType);
}

////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature
void CVideoBridgeInterface::UpdateNewSpeakerAckIndReceivedFromRemoteMCU(PartyRsrcID partyId)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->UpdateNewSpeakerAckIndReceivedFromRemoteMCU(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::EventModeIntraPreviewReq(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->EventModeIntraPreviewReq(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateEMPartyStartCascadeLinkAsLecturerPendingMode(CTaskApp* pParty, WORD cascadeLecturerCopEncoderIndex)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateEMPartyStartCascadeLinkAsLecturerPendingMode(pParty, cascadeLecturerCopEncoderIndex);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DeletePartyFromConf(const char* pDeletedPartyName)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DeletePartyFromConf(pDeletedPartyName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::SetSiteName(const char* partyName, const char* visualName)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->SetSiteName(partyName, visualName);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::IvrPartyCommand(PartyRsrcID partyId, OPCODE opcode, CSegment* pDataSeg)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->IvrPartyCommand(partyId, opcode, pDataSeg);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::PLC_SetPartyPrivateLayout(PartyRsrcID partyId, LayoutType newPrivateLayoutType)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->PLC_SetPartyPrivateLayout(partyId, newPrivateLayoutType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::PLC_PartyReturnToConfLayout(PartyRsrcID partyId)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->PLC_PartyReturnToConfLayout(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::PLC_ForceToCell(PartyRsrcID partyId, PartyRsrcID partyImageToSee, BYTE cellToForce)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->PLC_ForceToCell(partyId, partyImageToSee, cellToForce);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::PLC_CancelAllPrivateLayoutForces(PartyRsrcID partyId)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->PLC_CancelAllPrivateLayoutForces(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::VENUS_SetPartyPrivateLayout(PartyRsrcID partyId, LayoutType newPrivateLayoutType)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->PLC_SetPartyPrivateLayout(partyId, newPrivateLayoutType);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DisplayTextOnScreen(PartyRsrcID partyId, BYTE IsConfSecure, char** AudioArr, WORD numAudioParties, char** VideoArr, WORD numVideoParties, BOOL IsChairOnly)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DisplayPartyListOnScreen(partyId, IsConfSecure, AudioArr, numAudioParties, VideoArr, numVideoParties, IsChairOnly);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DisplayPartyMsgOnScreen(PartyRsrcID partyId, char** displayStringArr, WORD displayStringArrNo)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DisplayPartyTextOnScreen(partyId, displayStringArr, displayStringArrNo);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DisplayGatheringOnScreen(CGatheringManager* pGatheringManager)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DisplayGatheringOnScreen(pGatheringManager);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DisplayGatheringOnScreen(const char* pPartyName, CGatheringManager* pGatheringManager)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DisplayGatheringOnScreen(pPartyName, pGatheringManager);
}

////////////////////////////////////////////////////////////////////////////
bool CVideoBridgeInterface::IsBridgePartyVideoOutStateIsConnected(const char* pPartyName)
{
	if (m_pBridgeImplementation)
		return ((CVideoBridge*)m_pBridgeImplementation)->IsBridgePartyVideoOutStateIsConnected(pPartyName);
	return false;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->SetVisualEffectsParams(pVisualEffects);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::SetVisualEffectsParams(const std::string& sPartName, CVisualEffectsParams* pVisualEffects)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->SetVisualEffectsParams(sPartName, pVisualEffects);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DisplayPartyTextListOnScreen(PartyRsrcID partyId, CTextOnScreenMngr* TextMsgList, DWORD timeout)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DisplayPartyTextListOnScreen(partyId, TextMsgList, timeout);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::StopDisplayPartyTextOnScreen(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->StopDisplayPartyTextOnScreen(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::ConnectPartyToPCMEncoder(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->ConnectPartyToPCMEncoder(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DisconnectPartyFromPCMEncoder(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DisconnectPartyFromPCMEncoder(partyId);
}

////////////////////////////////////////////////////////////////////////////
DWORD CVideoBridgeInterface::GetPartyResolutionInPCMTerms(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		return ((CVideoBridge*)m_pBridgeImplementation)->GetPartyResolutionInPCMTerms(partyId);
	return (DWORD)-1;
}

////////////////////////////////////////////////////////////////////////////
CLayout* CVideoBridgeInterface::GetPartyLayout(const char* partyName)
{
	if (m_pBridgeImplementation)
		return ((CVideoBridge*)m_pBridgeImplementation)->GetPartyLayout(partyName);
	return NIL(CLayout);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::ChangeSpeakerNotation(PartyRsrcID partyId, DWORD imageId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->ChangeSpeakerNotation(partyId, imageId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateMessageOverlayForParty(PartyRsrcID partyId, CMessageOverlayInfo* pMessageOverlayInfo)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateMessageOverlayForParty(partyId, pMessageOverlayInfo);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateMessageOverlayStopForParty(PartyRsrcID partyId) // VNGR-15750
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateMessageOverlayStopForParty(partyId);
}

/////////////////////////////////////////////////////////////////////////////
CVideoBridgePartyCntlContent* CVideoBridgeInterface::GetContentDecoder()
{
	if (m_pBridgeImplementation)
		return ((CVideoBridge*)m_pBridgeImplementation)->GetContentDecoder();
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::AckOnIvrScpShowSlide(PartyRsrcID partyId, BYTE bIsAck)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->AckOnIvrScpShowSlide(partyId, bIsAck);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::AckOnIvrScpStopShowSlide(PartyRsrcID partyId, BYTE bIsAck)
{
	PASSERT_AND_RETURN(!GetBridgeImplementation());
	GetBridgeImplementation()->AckOnIvrScpStopShowSlide(partyId ,  bIsAck);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpgradeToMixAvcSvcForRelayParty(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpgradeToMixAvcSvcForRelayParty(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpgradeToMixAvcSvcForNonRelayParty(PartyRsrcID partyId, CAvcToSvcParams *pAvcToSvcParams)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpgradeToMixAvcSvcForNonRelayParty(partyId,  pAvcToSvcParams);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DowngradeMixAvcSvcRelayParty(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DowngradeMixAvcSvcRelayParty(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::DowngradeMixAvcSvcNonRelayParty(PartyRsrcID partyId)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->DowngradeMixAvcSvcNonRelayParty(partyId);
}

////////////////////////////////////////////////////////////////////////////
void CVideoBridgeInterface::UpdateVidBrdgTelepresenseEPInfo(PartyRsrcID partyId, CTelepresenseEPInfo* pTelepresenseEPInfo, const char* pSiteName)
{
	if (m_pBridgeImplementation)
		((CVideoBridge*)m_pBridgeImplementation)->UpdateVidBrdgTelepresenseEPInfo(partyId, pTelepresenseEPInfo, pSiteName);
}
