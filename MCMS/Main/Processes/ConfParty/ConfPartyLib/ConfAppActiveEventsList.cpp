// +========================================================================+
// Copyright 2005 Polycom Networking Ltd.                                   |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Polycom Networking Ltd. and is protected by law.          |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       ConfAppWaitEventsList.cpp                                    |
// SUBSYSTEM:  ConfParty                                                    |
// +========================================================================+

#include "ConfAppActiveEventsList.h"
#include "ConfAppFeatureObject.h"
#include "StatusesGeneral.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CConfAppActiveEventsList
////////////////////////////////////////////////////////////////////////////
CConfAppActiveEventsList::CConfAppActiveEventsList(CConfAppInfo* confAppInfo, CConfAppPartiesList* participants)
{
  m_confAppInfo  = confAppInfo;
  m_participants = participants;
  m_confIVR      = new CConfAppFeatureConfIVR(confAppInfo, participants);
  m_partyIVR     = new CConfAppFeaturePartyIVR(confAppInfo, participants);
}

////////////////////////////////////////////////////////////////////////////
CConfAppActiveEventsList::CConfAppActiveEventsList(const CConfAppActiveEventsList& other)
  : CPObject(other)
{
  m_confAppInfo  = NULL;
  m_participants = NULL;
  m_confIVR      = NULL;
  m_partyIVR     = NULL;
  *this          = other;
}

////////////////////////////////////////////////////////////////////////////
CConfAppActiveEventsList& CConfAppActiveEventsList::operator=(const CConfAppActiveEventsList& other)
{
  if (&other == this)
  	return *this;

  m_confAppInfo  = other.m_confAppInfo;
  m_participants = other.m_participants;

  PDELETE(m_confIVR);
  if (other.m_confIVR == NULL)
    m_confIVR = NULL;
  else
    m_confIVR = new CConfAppFeatureConfIVR(m_confAppInfo, m_participants);

  PDELETE(m_partyIVR);
  if (other.m_partyIVR == NULL)
    m_partyIVR = NULL;
  else
    m_partyIVR = new CConfAppFeaturePartyIVR(m_confAppInfo, m_participants);

  return *this;
}

////////////////////////////////////////////////////////////////////////////
CConfAppActiveEventsList::~CConfAppActiveEventsList()
{
  POBJDELETE(m_confIVR);
  POBJDELETE(m_partyIVR);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::RemoveAllIVREventsUponConfTerminating()
{
  PTRACE2(eLevelInfoNormal, "CConfAppActiveEventsList::RemoveAllIVREventsUponConfTerminating - Blocking further events for this ConfName: ", m_confAppInfo->GetConfName());
  m_partyIVR->RemoveAllFeaturesUponConfTerminating();
  m_confIVR->RemoveAllFeaturesUponConfTerminating();
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StartConfIVRNewFeature(DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID)
{
  // checks if the conference is terminating (if so, nothing to do)
  WORD confState = m_confAppInfo->GetConfState();
  if ((confState == eAPP_CONF_STATE_TERMINATING) || (confState == eAPP_CONF_STATE_TERMINATED))
    return;   // nothing to do

  if (m_confIVR)
  {
    // checks on IVR-Conf if any feature need to do action before the new event starts
    m_confIVR->AskForPreAction(feature, partyRsrcID);

    // start the new feature (IVR-Conf)
    m_confIVR->StartConfNewFeature(eventUniqueNumber, feature, partyRsrcID);
  }
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StopConfIVRNewFeature(DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID)
{
  // checks if the conference is terminating (if so, nothing to do)
  WORD confState = m_confAppInfo->GetConfState();
  if ((confState == eAPP_CONF_STATE_TERMINATING) || (confState == eAPP_CONF_STATE_TERMINATED))
    return;   // nothing to do

  if (m_confIVR)
    m_confIVR->StopConfNewFeature(eventUniqueNumber, feature, partyRsrcID);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StartPartyIVRNewFeature(DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID)
{
  if (m_partyIVR)
  {
    // checks on IVR-Party if any feature need to do action before the new event starts
    m_partyIVR->AskForPreAction(feature, partyRsrcID);

    // start the new feature (party-ivr)
    m_partyIVR->StartPartyNewFeature(eventUniqueNumber, feature, partyRsrcID);
  }
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StopPartyIVRNewFeature(DWORD eventUniqueNumber, TConfAppEvents feature, DWORD partyRsrcID)
{
  if (m_partyIVR)
    m_partyIVR->StopPartyNewFeature(eventUniqueNumber, feature, partyRsrcID);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StopFeature(DWORD eventUniqueNumber, TConfAppEvents featureOpcode, DWORD partyID, WORD confOrParty)
{
//  TRACEINTO << "CConfAppActiveEventsList::StopFeature "
//            << "- ConfName:"    << m_confAppInfo->GetConfName()
//            << ", PartyId:"     << partyID
//            << ", Opcode:"      << m_confAppInfo->GetStringFromOpcode(featureOpcode) << " (#" << featureOpcode << ")"
//            << ", ConfOrParty:" << ((confOrParty == CAM_PARTY_ACTION) ? "CAM_PARTY_ACTION" : "CAM_CONF_ACTION");

  // Stop feature
  if (CAM_PARTY_ACTION == confOrParty)
  {
    if (m_partyIVR)
    	m_partyIVR->StopPartyNewFeature( eventUniqueNumber, featureOpcode, partyID );
  }
  else // CAM_CONF_ACTION
	  if (m_confIVR)
		  m_confIVR->StopConfNewFeature( eventUniqueNumber, featureOpcode, partyID );
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StartNewFeature(DWORD eventUniqueNumber, TConfAppEvents featureOpcode, DWORD partyID, WORD onOff, WORD confOrParty)
{
//  TRACEINTO << " PartyId: "     << partyID
//            << ", Start: "		<< onOff
//            << ", Opcode: "      << m_confAppInfo->GetStringFromOpcode(featureOpcode) << " (#" << featureOpcode << ")"
//            << ", ConfOrParty: " << ((confOrParty == CAM_PARTY_ACTION) ? "PARTY_ACTION" : "CONF_ACTION");

  // checks if any feature need to do action before the new event starts
  AskForPreAction(featureOpcode, partyID);

  // start new feature
  if (CAM_PARTY_ACTION == confOrParty)
  {
    if (m_partyIVR)
    {
      if (onOff == CAM_START_FEATURE)
        m_partyIVR->StartPartyNewFeature(eventUniqueNumber, featureOpcode, partyID);
      else
        m_partyIVR->StopPartyNewFeature(eventUniqueNumber, featureOpcode, partyID);
    }
  }
  else // CONF_ACTION
  if (m_confIVR)
  {
    if (onOff == CAM_START_FEATURE)
      m_confIVR->StartConfNewFeature(eventUniqueNumber, featureOpcode, partyID);
    else
      m_confIVR->StopConfNewFeature(eventUniqueNumber, featureOpcode, partyID);
  }
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::DoSomethingWithEndIVREvent(DWORD uniqueEventNumber, TConfAppEvents featureOpcode, DWORD partyID)
{
  //TRACEINTO << "CConfAppActiveEventsList::DoSomethingWithEndIVREvent - featureOpcode:" << m_confAppInfo->GetStringFromOpcode(featureOpcode) << " (" << (DWORD)featureOpcode << "), PartyId:" << partyID;

  if (!IsEventAllowed( featureOpcode ))                                // check if the conf is terminating/terminated
    return NO_WAIT_TO_EVENT;

  WORD retValue    = WAIT_TO_EVENT;
  WORD action      = CAM_START_FEATURE;
  WORD confOrParty = CAM_PARTY_ACTION;

  switch (featureOpcode)
  {
    case eCAM_EVENT_PARTY_END_ALL_WAIT_FOR_CHAIR:
    {
      if (!IsNeedEndWaitForChair(partyID))
        return NO_WAIT_TO_EVENT;

      retValue      = NO_WAIT_TO_EVENT;
      action        = CAM_STOP_FEATURE;
      confOrParty   = CAM_PARTY_ACTION;
      featureOpcode = eCAM_EVENT_PARTY_WAIT_FOR_CHAIR;  // for end all participants "WAIT FOR CHAIR" state
      break;
    }

    case eCAM_EVENT_CONF_END_WAIT_FOR_CHAIR:
    {
      if (!IsNeedEndWaitForChair(partyID))
        return NO_WAIT_TO_EVENT;

      retValue      = NO_WAIT_TO_EVENT;
      action        = CAM_STOP_FEATURE;
      confOrParty   = CAM_CONF_ACTION;
      partyID   = 0;
      featureOpcode = eCAM_EVENT_CONF_WAIT_FOR_CHAIR;   // to end conf "wait for chair" state
      break;
    }

    case eCAM_EVENT_PARTY_END_SINGLE_PARTY_MUSIC:
    {
      if (!IsNeedEndSinglePartyMusic())
        return NO_WAIT_TO_EVENT;

      retValue      = NO_WAIT_TO_EVENT;
      featureOpcode = eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC;
      action        = CAM_STOP_FEATURE;
      break;
    }

    case eCAM_EVENT_CONF_ENTRY_TONE:
    {
      if (!IsNeedEntryTone(partyID))
        return NO_WAIT_TO_EVENT;

      confOrParty       = CAM_CONF_ACTION;
      retValue          = NO_WAIT_TO_EVENT;
      uniqueEventNumber = 0;
      break;
    }

    case eCAM_EVENT_PARTY_RECORDING_IN_PROGRESS:
    {
      if (!IsNeedRecInProg(partyID))
        return NO_WAIT_TO_EVENT;

      action      = CAM_START_FEATURE;
      confOrParty = CAM_PARTY_ACTION;
      break;
    }

    case eCAM_EVENT_PARTY_RECORDING_FAILED:
    {
      if (!IsNeedRecFailed(partyID))
        return NO_WAIT_TO_EVENT;

      action      = CAM_START_FEATURE;
      confOrParty = CAM_PARTY_ACTION;
      break;
    }

    case eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC:
    {
      if (!IsNeedSinglePartyMusic() || (m_participants->GetIsCascadeLinkParty(partyID)))
        return NO_WAIT_TO_EVENT;
      break;
    }

    case eCAM_EVENT_PARTY_FIRST_TO_JOIN:
    {
      if (!IsNeedFirstToJoin(partyID))
        return NO_WAIT_TO_EVENT;
      break;
    }

    case eCAM_EVENT_PARTY_PLAY_RINGING_TONE:
    {
      if (!IsNeedPlayRingingTone(partyID))
        return NO_WAIT_TO_EVENT;
      break;
    }

    case eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE:
    {
      if (!IsNeedStopPlayRingingTone(partyID))
        return NO_WAIT_TO_EVENT;

      retValue = NO_WAIT_TO_EVENT;
      // in case of req from conf send the stop ringing to the inviter
      if (partyID == INVALID)
        partyID = m_confAppInfo->GetSinglePartyRsrcId();

      featureOpcode = eCAM_EVENT_PARTY_PLAY_RINGING_TONE;
      action        = CAM_STOP_FEATURE;
      break;
    }

    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:
    {
      if (!IsNeedConfWaitForChair(partyID))         // First party in conf starts WaitForChair if needed
        return NO_WAIT_TO_EVENT;

      confOrParty       = CAM_CONF_ACTION;
      retValue          = NO_WAIT_TO_EVENT;
      uniqueEventNumber = 0;
      partyID       = 0;
      break;
    }

    case eCAM_EVENT_PARTY_WAIT_FOR_CHAIR:
    {
      if (!IsNeedPartyWaitForChair())
        return NO_WAIT_TO_EVENT;
      break;
    }

    case eCAM_EVENT_PARTY_IN_CONF_IND:
    {
      // Send PARTY_IN_CONF_IND to party control
      retValue = NO_WAIT_TO_EVENT;
      break;
    }

    case eCAM_EVENT_PARTY_END_VIDEO_IVR:
    {
      // Set m_slideIsNoLongerPermitted flag in CAM Info
      SetFlagSlideIsNoLongerPermitted(partyID, 1);
      retValue = NO_WAIT_TO_EVENT;
      break;
    }

    default:
    {
      PASSERTMSG_AND_RETURN_VALUE(1, "Failed, event is not valid", NO_WAIT_TO_EVENT);
    }
  }                                                     // switch

  int stat = AskForObjection(featureOpcode, partyID, action);
  if (APP_F_OBJECTION == stat)                          // there is objection
    return NO_WAIT_TO_EVENT;

  // do new event
  StartNewFeature(uniqueEventNumber, featureOpcode, partyID, action, confOrParty);
  return retValue;
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::SetFlagSlideIsNoLongerPermitted(DWORD partyRsrcID, DWORD yesNo)
{
  if (m_participants)
  {
    int ind = m_participants->FindPartyIndex(partyRsrcID);
    if (-1 == ind)
    {
      TRACEINTO << "CConfAppActiveEventsList::SetFlagSlideIsNoLongerPermitted - Failed to find party in list, partyRsrcID:" << partyRsrcID;
      return;
    }
    m_participants->m_partyList[ind]->SetSlideIsNoLongerPermitted(yesNo);
  }
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::DoSomethingWithEndPartyEvent(DWORD uniqueEventNumber, TConfAppEvents featureOpcode, DWORD partyID)
{
  TRACEINTO << "CConfAppActiveEventsList::DoSomethingWithEndPartyEvent - featureOpcode:" << m_confAppInfo->GetStringFromOpcode(featureOpcode) << " (" << (DWORD)featureOpcode << "), PartyId:" << partyID;

  if (!IsEventAllowed( featureOpcode ))
    return NO_WAIT_TO_EVENT;

  WORD retValue    = WAIT_TO_EVENT;
  WORD action      = CAM_START_FEATURE;               // 0=off, 1=on
  WORD confOrParty = CAM_PARTY_ACTION;


  switch (featureOpcode)
  {
    case eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED:
    {
      UpdateLeaderInConf();
      if (!IsNeedChairDropped(partyID))
        return NO_WAIT_TO_EVENT;

      confOrParty = CAM_CONF_ACTION;
      retValue    = NO_WAIT_TO_EVENT;
      break;
    }

    case eCAM_EVENT_CONF_CHAIR_DROPPED:
    {
      if (!IsNeedChairDropped(partyID))
      {
        return NO_WAIT_TO_EVENT;
      }

      confOrParty = CAM_CONF_ACTION;
      break;
    }
    case eCAM_EVENT_CONF_EXIT_TONE:
    {
      if (!IsNeedExitTone(partyID))
      {
        return NO_WAIT_TO_EVENT;
      }

      confOrParty = CAM_CONF_ACTION;
      break;
    }
    case eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC:
    {
      if (!IsNeedSinglePartyMusic())
        return NO_WAIT_TO_EVENT;

      partyID = FindSingleParty();                  // ID of the single party that left in the conf
      // Check if party was found (-1 = not found) - maybe in IVR feature
      if (((DWORD)(-1) == partyID) ||
          (m_participants->GetIsCascadeLinkParty(partyID)))
        return NO_WAIT_TO_EVENT;

      uniqueEventNumber = 0;                            // for the single party that left in the conf
      retValue          = NO_WAIT_TO_EVENT;
      break;
    }

    default:
    {
      PASSERTMSG_AND_RETURN_VALUE(1, "Failed, event is not valid", NO_WAIT_TO_EVENT);
    }
  } // switch

  int stat = AskForObjection(featureOpcode, partyID, action);
  if (APP_F_OBJECTION == stat)                        // there is an objection
    return NO_WAIT_TO_EVENT;                          // do nothing and return

  StartNewFeature(uniqueEventNumber, featureOpcode, partyID, action, confOrParty);
  return retValue;
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::DoSomethingWithSetAsChairEvent(DWORD partyRsrcID)
{
  if (!IsEventAllowed( eCAM_EVENT_PARTY_WAIT_FOR_CHAIR ))	// check if the conf is terminating/terminated7
    return;

  WORD           retValue    = WAIT_TO_EVENT;
  WORD           action      = CAM_START_FEATURE;
  WORD           confOrParty = CAM_PARTY_ACTION;
  TConfAppEvents featureOpcode;

  if (IsNeedEndWaitForChair(partyRsrcID))
  {
    featureOpcode = eCAM_EVENT_PARTY_WAIT_FOR_CHAIR;  // for end all participants "WAIT FOR CHAIR" state
    action        = CAM_STOP_FEATURE;
    confOrParty   = CAM_PARTY_ACTION;

    int stat = AskForObjection(featureOpcode, partyRsrcID, action);
    if (APP_F_NO_OBJECTION == stat)                   // there is no objection
      StartNewFeature(0, featureOpcode, partyRsrcID, action, confOrParty);
  }
  if (IsNeedEndWaitForChair(partyRsrcID))
  {
    featureOpcode = eCAM_EVENT_CONF_WAIT_FOR_CHAIR;   // to end conf "wait for chair" state
    partyRsrcID   = 0;
    action        = CAM_STOP_FEATURE;
    confOrParty   = CAM_CONF_ACTION;

    int stat = AskForObjection(featureOpcode, partyRsrcID, action);
    if (APP_F_NO_OBJECTION == stat)                   // there is no objection
      StartNewFeature(0, featureOpcode, partyRsrcID, action, confOrParty);
  }
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::DoSomethingUponPartyLeavesConf()
{
  // update "first to join"
  if (0 == m_participants->GetNumOfInMixParticipants())
  {
    PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::DoSomethingUponPartyLeavesConf - Updating first to join");
    m_confAppInfo->SetFirstPartyMessagePlayed(0);
  }
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::AskForObjection(TConfAppEvents feature, DWORD partyRsrcID, WORD onOff)
{
  if (CAM_STOP_FEATURE == onOff)
    return APP_F_NO_OBJECTION;

  int ret = AskForObjection(feature, partyRsrcID);
  return ret;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::AskForObjection(TConfAppEvents feature, DWORD partyRsrcID)
{
  if (m_confIVR)
    if (APP_F_OBJECTION == m_confIVR->AskForObjection(feature, partyRsrcID))
    {
      PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::AskForObjection - conf OBJECTION!!");
      return APP_F_OBJECTION;
    }

  if (m_partyIVR)
    if (APP_F_OBJECTION == m_partyIVR->AskForObjection(feature, partyRsrcID))
    {
      PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::AskForObjection - party OBJECTION!!");
      return APP_F_OBJECTION;
    }

  return APP_F_NO_OBJECTION;
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::EnterPartyToMix(DWORD partyRsrcID)
{
  PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::EnterPartyToMix ");
  if (m_partyIVR)
    m_partyIVR->EnterPartyToMix(partyRsrcID);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::PlayMessage(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::PlayMessage");
  if (m_partyIVR)
    m_partyIVR->PlayMessage(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::RecordRollCall(CSegment* pParam)
{
  if (m_partyIVR)
    m_partyIVR->RecordRollCall(pParam);
}
////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StopRollCallRecording(CSegment* pParam)
{
	  if (m_partyIVR)
	    m_partyIVR->StopRollCallRecording(pParam);
}
////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StopRollCallRecordingAck(DWORD PartyId,DWORD status)
{
	  if (m_partyIVR)
	    m_partyIVR->StopRollCallRecordingAck(PartyId,status);
}
////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::StopMessage(CSegment* pParam)
{
  if (m_partyIVR)
    m_partyIVR->StopMessage(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::PlayMessageInd(DWORD opcode, DWORD partyRsrcID, DWORD messageID)
{
  // search for event
  DWORD mainInd   = 0;
  DWORD subInd    = 0;

  int findFeatute = FindFeatureForMessageInd(messageID, mainInd, subInd);
  if (STATUS_FAIL == findFeatute)
  {
    PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::PlayMessageInd - message IND not found, ignore");
    return;
  }

  switch (mainInd)
  {
    case F_APP_CONF_IVR:
    {
      PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::PlayMessageInd - conf message IND");
      break;
    }
    case F_APP_PARTY_IVR:
    {
      PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::PlayMessageInd - party message IND");
      break;
    }
  } // switch

}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::FindFeatureForMessageInd(DWORD messageID, DWORD& mainInd, DWORD& subInd)
{
  if (m_confIVR)
    if (STATUS_OK == m_confIVR->GetIVRMessageID(messageID, subInd))
    {
      mainInd = F_APP_CONF_IVR;
      return STATUS_OK;
    }

  if (m_partyIVR)
    if (STATUS_OK == m_partyIVR->GetIVRMessageID(messageID, subInd))
    {
      mainInd = F_APP_PARTY_IVR;
      return STATUS_OK;
    }

  return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID)
{
  if (m_confIVR)
    m_confIVR->AskForPreAction(feature, partyRsrcID);

  if (m_partyIVR)
    m_partyIVR->AskForPreAction(feature, partyRsrcID);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::DtmfReceived(DWORD opcode, CSegment* pParam)
{
  if (m_partyIVR)
    m_partyIVR->DtmfReceived(opcode, pParam);
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList:: IsNeedEndWaitForChair(DWORD partyRsrcID)
{
  if (m_confAppInfo->IsInWaitForChairNow() &&
      (m_participants->IsLeaderInConf()
      /* || m_participants->GetIsCascadeLinkParty(partyRsrcID) This condition is disabled see Bridge-12756*/))
    return 1;

  return 0;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedEndSinglePartyMusic()
{
  if (m_confAppInfo->GetIsGateWay()) {
	TRACEINTO << " - GetIsGateWay";
    return 0;
  }

  if (m_confAppInfo->IsOperatorConf()) {
	TRACEINTO << " - IsOperatorConf";
	return 0;
  }

  TRACEINTO << " - NumInMix=" << (DWORD)m_participants->GetNumOfInMixParticipants() << " , SinglePartyNow=" << (DWORD)m_confAppInfo->GetSinglePartyNow();
  if (1 < m_participants->GetNumOfInMixParticipants() && m_confAppInfo->GetSinglePartyNow())
	return 1;

  return 0;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedSinglePartyMusic()
{
  if (m_confAppInfo->GetIsGateWay())
    return 0;

  if (m_confAppInfo->IsOperatorConf())
    return 0;

  if (m_confAppInfo->IsInWaitForChairNow()) // remove when the IsObjection will be implemented
    return 0;

  // VNGFE-6779
  if (m_confAppInfo->GetConfActiveExitToneCounter())
    return 0;

  if (1 == m_participants->GetNumOfInMixParticipants() && (!m_confAppInfo->GetSinglePartyNow()))
    return 1;

  return 0;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedPlayRingingTone(DWORD partyRsrcID)
{
  if (!m_confAppInfo->GetIsGateWay())
    return 0;

  if (1 == m_participants->GetNumOfInMixParticipants() && (!m_confAppInfo->GetSinglePartyNow()))
  {
    if (m_participants->GetGatewayPartyType(partyRsrcID) > eNormalGWPartyType)
      return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedStopPlayRingingTone(DWORD partyRsrcID)
{
  if (!m_confAppInfo->GetIsGateWay())
    return 0;

  if ((m_confAppInfo->GetSinglePartyNow() && 1 < m_participants->GetNumOfInMixParticipants()) || partyRsrcID == INVALID)
  {
    // check if the single party heares ringing tone
    if (m_participants->GetGatewayPartyType(m_confAppInfo->GetSinglePartyRsrcId()) > eNormalGWPartyType)
      return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////
DWORD CConfAppActiveEventsList::FindSingleParty()
{
  return m_participants->GetPartyInMix();
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedFirstToJoin(DWORD partyRsrcID)
{
  if (m_confAppInfo->GetIsGateWay())
    return 0;

  if ((m_participants->GetNumOfInMixParticipants() == 1) && /* was (m_participants->GetNumOfParticipants() == 1)*/
      m_confAppInfo->IsWaitForChair() &&
      !m_participants->IsLeaderInConf())
  {
    m_confAppInfo->SetFirstPartyMessagePlayed(1);
    return 0;
  }

  if (m_participants->GetIsCascadeLinkParty(partyRsrcID))
  {
    m_confAppInfo->SetFirstPartyMessagePlayed(1);
    return 0;
  }

  if (m_participants->GetIsRecordingLinkParty(partyRsrcID))
    return 0;

  if (m_confAppInfo->IsFirstPartyMessagePlayed())
    return 0;

  return 1;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedPartyWaitForChair()
{
  if (m_confAppInfo->IsInWaitForChairNow())
    return 1;

  return 0;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedConfWaitForChair(DWORD partyRsrcID)
{
  // First party in conf starts WaitForChair if needed
  if (!m_confAppInfo->IsInWaitForChairNow() && m_confAppInfo->IsWaitForChair() && !m_participants->IsLeaderInConf() && (0 == m_participants->GetIsCascadeLinkParty(partyRsrcID)))
  {
		if (m_confAppInfo->IsConfPastWaitForChairStage() == 0)
		{
    m_confAppInfo->SetInWaitForChairNow(1);
    return 1;
  }
	}

  if (m_participants->GetIsCascadeLinkParty(partyRsrcID))
  {
    m_confAppInfo->SetWaitForChair(0);
    return 0;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedEntryTone(DWORD partyRsrcID)
{
  if (m_participants->GetIsCascadeLinkParty(partyRsrcID))
    return 0;

  if (m_participants->GetIsRecordingLinkParty(partyRsrcID))
    return 0;

  //Bridge-15531: Prevent sending multiple entry/exit tone message for Multiple EP screens, send one message only.
  if (m_participants->IsTipParty(partyRsrcID))
  {
	  if (!m_participants->IsTipMaster(partyRsrcID))
		  return 0;
  }
  else
  {
	  if (!m_participants->isMainITPEPOrSingleEP(partyRsrcID))
		return 0;
  }

  if (0 == m_confAppInfo->GetRollCallAnnounceYesNo())
    return 0;

  if (m_confAppInfo->IsInWaitForChairNow())
    return 0;

  if (0 == m_participants->IsRollCallRecordingExists(partyRsrcID, ACTIVE_LIST) && !m_confAppInfo->IsRollCallToneInsteadVoice())
    return 0;

  if (1 == m_participants->GetNumOfInMixParticipants())
  {
    m_participants->SetRollCallEntryTonePlayed(partyRsrcID, true); //BRIDGE-6780
    return 0;
  }

  if (m_participants->GetRollCallEntryTonePlayed(partyRsrcID))
    return 0;

  m_participants->SetRollCallEntryTonePlayed(partyRsrcID, true);
  return 1;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedRecInProg(DWORD partyRsrcID)
{
  // Check if this is a Recording Link
  if (m_participants->GetIsRecordingLinkParty(partyRsrcID))
    return 0;

  // Check if Recording is enabled in conf
  if (!m_confAppInfo->GetEnableRecording())
    return 0;

  // Check if the Recording Link is actually connected
  if (0 == m_confAppInfo->GetRecordingLinkInConf())
    return 0;

  // Check if Recording state is "started" or "paused"
  if (eStartRecording != m_confAppInfo->GetRecordingStatus() && ePauseRecording != m_confAppInfo->GetRecordingStatus())
    return 0;

  //check if Recording Notification is selected on EMA
  //eFeatureRssDialin
  if(0 == m_confAppInfo->GetEnableRecordingNotify())
     return 0;

  return 1;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedRecFailed(DWORD partyRsrcID)
{
  // Check if this is a Recording Link
  if (m_participants->GetIsRecordingLinkParty(partyRsrcID))
    return 0;

  // Check if this is a chairperson or a cascaded link party
  if (!m_participants->GetIsPartyLeader(partyRsrcID) && !m_participants->GetIsCascadeLinkParty(partyRsrcID))
    return 0;

  // Check if Recording is enabled in conf
  if (!m_confAppInfo->GetEnableRecording())
    return 0;

  // Check if Recording state is "stopped"
  if (eStopRecording != m_confAppInfo->GetRecordingStatus())
    return 0;

  // Check if Recording Failed flag is on
  if (NO == m_confAppInfo->GetRecordingFailedFlag())
    return 0;

  return 1;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedChairDropped(DWORD partyRsrcID)
{
  // we need to play this message in case:
  // 1. the chair dropped / move-out
  // 2. there is no other chair in conf
  // 3. "terminate when chair lives" feature is ON
  // 4. The conf is not in "terminating" state
  // 5. The system.cfg flag is suitable

  if (0 == m_participants->GetIsDeletedPartyLeader(partyRsrcID))
		  return 0;

	if (!m_confAppInfo->GetTerminateAfterChairExits())
	   return 0;

	if (0 != m_confAppInfo->GetIsLeaderInConfNow())
		  return 0;

	WORD confState = m_confAppInfo->GetConfState();
	if (confState != eAPP_CONF_STATE_CONNECT)
		  return 0;


  PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::IsNeedChairDropped - Need Chair Dropped Routine ");

  return 1;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsNeedExitTone(DWORD partyRsrcID)
{
  if (m_participants->GetIsCascadeLinkParty(partyRsrcID))
    return 0;

  //Bridge-15531: Prevent sending multiple entry/exit tone message for Multiple EP screens, send one message only.
  if (m_participants->IsTipParty(partyRsrcID))
  {
	  if (!m_participants->IsTipMaster(partyRsrcID))
		  return 0;
  }
  else
  {
	  if (!m_participants->isMainITPEPOrSingleEP(partyRsrcID))
		return 0;
  }

  if (0 == m_confAppInfo->GetRollCallAnnounceYesNo())
    return 0;

  if (0 == m_participants->IsRollCallRecordingExists(partyRsrcID, DISCONNECTED_LIST) && !m_confAppInfo->IsRollCallToneInsteadVoice())
    return 0;

  if (0 == m_participants->GetNumOfInMixParticipants())
    return 0;

  return 1;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsEventAllowed( const TConfAppEvents featureOpcode )
{
	// checks if the conference is terminating (if so, nothing to do)
	if (m_confAppInfo->GetConfState() == eAPP_CONF_STATE_TERMINATING)
	{
		PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::IsEventAllowed - eAPP_CONF_STATE_TERMINATING");
		return 0;
	}

	if (m_confAppInfo->GetConfState() == eAPP_CONF_STATE_TERMINATED)
	{
		PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::IsEventAllowed - eAPP_CONF_STATE_TERMINATED");
		return 0;
	}

//	if (eCAM_EVENT_MIN == featureOpcode)	// backwards compatible
//		return 1;

	if (!IsOpcodeAllowed( featureOpcode ))
		return 0;

	// all conditions that not related to the checking
	return 1;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::IsOpcodeAllowed( const TConfAppEvents featureOpcode )
{
	if (m_confAppInfo->IsNeedToBlockIVR())
	{
	  switch (featureOpcode)
	  {
		case SET_START_RECORDING:
		case SET_STOP_RECORDING:
		case SET_PAUSE_RECORDING:
		case SET_RESUME_RECORDING:
		case eCAM_EVENT_CONF_STOP_RECORDING:
		case eCAM_EVENT_CONF_PAUSE_RECORDING:
		case eCAM_EVENT_CONF_START_RESUME_RECORDING:
		case eCAM_EVENT_CONF_DTMF_FORWARDING:
		case eCAM_EVENT_PARTY_SEND_DTMF:
		case eCAM_EVENT_PARTY_CLIENT_SEND_DTMF:
		case eCAM_EVENT_PARTY_JOIN_CONF_VIDEO:
		case eCAM_EVENT_PARTY_IVR_ENTRANCE:
		case eCAM_EVENT_PARTY_IN_CONF_IND:
		case eCAM_EVENT_SET_AS_LEADER_FROM_API:
		case eCAM_EVENT_CONF_MUTE_ALL:
		case eCAM_EVENT_CONF_UNMUTE_ALL:
		  return 1;

	  default:
		  return 0;
	  }
	}

	// all conditions that not related to the checking
	return 1;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::DoPartyAction(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam)
{
	if (!IsOpcodeAllowed((TConfAppEvents)opcode ))
		return 0;

	int res = 0;

	if (m_partyIVR)
		res = m_partyIVR->DoPartyAction(opcode, partyRsrcID, action, pParam);

  return res;
}

////////////////////////////////////////////////////////////////////////////
int CConfAppActiveEventsList::DoConfAction(DWORD opcode, DWORD partyRsrcID, CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CConfAppActiveEventsList::DoConfAction ");

	if (!IsOpcodeAllowed(  (TConfAppEvents)opcode ))
		return 0;

	if (m_confIVR)
		m_confIVR->DoConfAction(opcode, partyRsrcID, pSeg);

	return 0;
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::RollCallRecorded(CSegment* pParam)
{
  if (m_partyIVR)
    m_partyIVR->RollCallRecorded(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::UpdateLeaderInConf()
{
  if (m_participants->IsLeaderInConf())
    return;

  m_confAppInfo->SetLeaderInConfNow(0);
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::ReplaceTokenWithSequenceNum(CSegment* pParam)
{
  OPCODE opcode;
  DWORD  seqNumToken;
  DWORD  sequenceNum;

  *pParam >> opcode >> seqNumToken >> sequenceNum;

  if (IVR_PLAY_MESSAGE_REQ == opcode)
  {
    int retVal = 0;

    // it can be Conf or Party Play Message
    if (m_confIVR)
    {
      retVal = m_confIVR->ReplaceTokenWithSequenceNum(seqNumToken, sequenceNum);

      if (1 == retVal)  // found
        return;         // no need to keep searching
    }

    if (m_partyIVR)
      retVal = m_partyIVR->ReplaceTokenWithSequenceNum(seqNumToken, sequenceNum);

    if (0 == retVal)    // not found
    {
      TRACEINTO << "CConfAppActiveEventsList::ReplaceTokenWithSequenceNum - seqNumToken:"
        << seqNumToken << ", sequenceNum:" << sequenceNum << ", not found";

      return;           // no need to keep searching
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::HandlePlayMsgAckInd(CSegment* pParam)
{
  DWORD  ack_seq_num;
  STATUS status;

  *pParam >> ack_seq_num >> status;
  int retVal = 0;

  if (STATUS_OK != status)
    PTRACE(eLevelError, "CConfAppActiveEventsList::HandlePlayMsgAckInd - Error, received ACK_IND with failure status");

  if (m_confIVR)
  {
    retVal = m_confIVR->HandlePlayMsgAckInd(ack_seq_num);
    if (1 == retVal)    // found
      return;           // no need to keep searching
  }

  if (m_partyIVR)
    retVal = m_partyIVR->HandlePlayMsgAckInd(ack_seq_num);

  if (0 == retVal)      // not found
  {
    TRACEINTO << "CConfAppActiveEventsList::HandlePlayMsgAckInd - ack_seq_num:" << ack_seq_num << ", not found";
    return;             // no need to keep searching
  }
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::HandleChangeIC()
{
  m_confIVR->HandleChangeIC();
}

////////////////////////////////////////////////////////////////////////////
void CConfAppActiveEventsList::RemoveFeatureByOpcode(DWORD opcode)
{
	m_partyIVR->RemoveFeatureByOpcode(opcode);
}


