#include "ConfAppFeatureObject.h"
#include "ConfAppPartiesList.h"
#include "StatusesGeneral.h"
#include "IvrApiCommandCreator.h"
#include "IVRPlayMessage.h"
#include "ConfPartyOpcodes.h"
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "IVRStartIVR.h"
#include "IVRServiceList.h"
#include "ConfPartyGlobals.h"
#include "IVRManager.h"
#include "PartyApi.h"
#include "SIPParty.h"
#include "VideoDefines.h"
#include "TraceStream.h"
#include "ConfPartyDefines.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "AudRequestStructs.h"
#include "IvrApiStructures.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsVideo.h"

static DWORD      IvrMessageSessionIDCam = 0; // and will be increased for every PlayMessage
static const WORD CONNECT                = 1; // any other state

////////////////////////////////////////////////////////////////////////////
//                        CConfAppFeatureObject
////////////////////////////////////////////////////////////////////////////
CConfAppFeatureObject::CConfAppFeatureObject(TAppFeatures featureCode)
{
  m_featureType = featureCode;
}

//--------------------------------------------------------------------------
CConfAppFeatureObject::~CConfAppFeatureObject()
{
}

//--------------------------------------------------------------------------
int CConfAppFeatureObject::GetIVRMessageID(DWORD messageID, DWORD& subInd)
{
  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CConfAppFeatureObject::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
}


#define PLAY_IVR_MUTE_ALL_MSG_DELAY_TIMER 3001
#define PLAY_IVR_MUTE_ALL_MSG_DELAY_TOUT  100 // old:20ms new:1 second

PBEGIN_MESSAGE_MAP(CConfAppFeatureConfIVR)
  ONEVENT(PLAY_IVR_MUTE_ALL_MSG_DELAY_TIMER, ANYCASE, CConfAppFeatureConfIVR::OnTimerPlayIVRMuteAllMsgDelay)
PEND_MESSAGE_MAP(CConfAppFeatureConfIVR, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CConfAppFeatureConfIVR
////////////////////////////////////////////////////////////////////////////
CConfAppFeatureConfIVR::CConfAppFeatureConfIVR(CConfAppInfo* confAppInfo, CConfAppPartiesList* participants)
                       :CConfAppFeatureObject(F_APP_CONF_IVR)
{
  m_confAppInfo  = confAppInfo;
  m_participants = participants;
  m_features     = new CFeatureObjectList(confAppInfo, participants);

  VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CConfAppFeatureConfIVR::~CConfAppFeatureConfIVR()
{
  POBJDELETE(m_features);
}

//--------------------------------------------------------------------------
void* CConfAppFeatureConfIVR::GetMessageMap()
{
  return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
  DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::OnTimerPlayIVRMuteAllMsgDelay(CSegment* pParam)
{
  DWORD featureInd = 0;

  *pParam >> (DWORD&)featureInd;

  TRACEINTO << "CConfAppFeatureConfIVR::OnTimerPlayIVRMuteAllMsgDelay - featureInd:" << featureInd;

  // Start new feature (event) for play conference message
  m_features->StartNewConfFeature(featureInd);
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::StartConfNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID)
{
  TRACEINTO << " Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", PartyId:" << partyRsrcID
            << ", EventId:" << eventUniqueNumber;

  // Set replaceOrApend parameter according to feature opcode
  WORD replaceOrAppend = (WORD)(-1);
  switch (opcode)
  {
    case eCAM_EVENT_CONF_CHAIR_DROPPED:
    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:
    {
      replaceOrAppend = CAM_REPLACE_FIRST_FEATURE;
      break;
    }

    case eCAM_EVENT_CONF_ALERT_TONE:
    {
      replaceOrAppend = CAM_APPEND_AFTER_CURRENT_FEATURE;
      break;
    }

    case eCAM_EVENT_CONF_SECURE:
    case eCAM_EVENT_CONF_UNSECURE:
    {
      replaceOrAppend = CAM_APPEND_AFTER_CURRENT_FEATURE;
      break;
    }

    case eCAM_EVENT_CONF_MUTE_ALL:
    case eCAM_EVENT_CONF_UNMUTE_ALL:
    case eCAM_EVENT_CONF_EXIT_TONE:
    case eCAM_EVENT_CONF_ENTRY_TONE:
    case eCAM_EVENT_CONF_NOISY_LINE_DETECTION:
    case eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED:
    case eCAM_EVENT_CONF_RECORDING_FAILED:
    {
      replaceOrAppend = CAM_APPEND_AFTER_LAST_FEATURE;
      break;
    }

    case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:
    {
      replaceOrAppend = CAM_ROLL_CALL_REPLACE_METHOD;
      break;
    }

    case eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK:
    {
      replaceOrAppend = CAM_APPEND_AFTER_LAST_FEATURE;
      break;
    }

    default:
    {
      TRACEINTO << "CFeatureObject::StartConfNewFeature - Failed, opcode not valid, Opcode:" << opcode;
      break;
    }
  }

  // Create new feature (event) for play conference message
  int featureInd = m_features->CreateNewConfFeature(eventUniqueNumber, opcode, partyRsrcID, replaceOrAppend);

  // Do action regardless the IVR prompt for the feature
  switch (opcode)
  {
    case eCAM_EVENT_CONF_MUTE_ALL:
    case eCAM_EVENT_CONF_UNMUTE_ALL:
    {
       	CSegment* pSeg = new CSegment;
    	DWORD isIVR = TRUE; //VNGR-26819 WAS FALSE
    	DWORD isForMuteAllButLecture = FALSE;
       	*pSeg << isIVR;
        *pSeg << isForMuteAllButLecture;
         DoConfAction(opcode, partyRsrcID,pSeg);
         break;
     }

    case eCAM_EVENT_CONF_SECURE:
    case eCAM_EVENT_CONF_UNSECURE:
    {
      DoConfAction(opcode, partyRsrcID);
      break;
    }
  }

  // If the new feature is the first one in the list - START IT!
  if (0 == featureInd)
  {
    // If the feature is eCAM_EVENT_CONF_MUTE_ALL or eCAM_EVENT_CONF_UNMUTE_ALL, delay the play IVR message
    if (opcode == eCAM_EVENT_CONF_MUTE_ALL || opcode == eCAM_EVENT_CONF_UNMUTE_ALL)
    {
    	if (m_participants->GetIsCascadeLinkParty(partyRsrcID))
    	{
    		TRACEINTO << " PartyId: " << partyRsrcID << " is a Cascade-Link-Party, don't play Mute all IVR message";
    		m_features->RemoveFeatureByOpcode(opcode);
    	}
    	else
    	{
      CSegment* pSeg = new CSegment;
      *pSeg << (DWORD)featureInd;
      TRACEINTO << "CFeatureObject::StartConfNewFeature - featureInd:" << featureInd;
      StartTimer(PLAY_IVR_MUTE_ALL_MSG_DELAY_TIMER, PLAY_IVR_MUTE_ALL_MSG_DELAY_TOUT, pSeg);
    	}
    }
    else
    {
      // Start new feature (event) for play conference message
      m_features->StartNewConfFeature(featureInd);
    }
  }
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::StopConfNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID)
{
  m_features->StopNewFeature(eventUniqueNumber, opcode, partyRsrcID, CAM_CONF_ACTION);
}

//--------------------------------------------------------------------------
int CConfAppFeatureConfIVR::GetIVRMessageID(DWORD messageID, DWORD& subInd)
{
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CConfAppFeatureConfIVR::AskForObjection(TConfAppEvents feature, DWORD partyRsrcID)
{
  return m_features->AskForObjection(feature, partyRsrcID);
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID)
{
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::PlayMessage(CSegment* pParam)
{
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::DoConfAction(DWORD opcode, DWORD partyRsrcID, CSegment* pSeg)
{
  TRACEINTO << "CConfAppFeatureConfIVR::DoConfAction "
            << "- Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", PartyId:" << partyRsrcID;

  switch (opcode)
  {
    case eCAM_EVENT_CONF_MUTE_ALL:
    {
      DWORD isIVR = FALSE;
      DWORD isForMuteAllButLecture = FALSE;
      BYTE  isMuteAllButLeader = FALSE;
      if (pSeg)
      {
        *pSeg >> isIVR;
        *pSeg >> isForMuteAllButLecture;
        *pSeg >> isMuteAllButLeader;
      }
      else
      {
        // Don't know if it's allowed or not, add assert temporarily
        PASSERT(1);
      }
      MuteAll(1, partyRsrcID, isIVR, isForMuteAllButLecture, isMuteAllButLeader);
      break;
    }

    case eCAM_EVENT_CONF_UNMUTE_ALL:
    {
      DWORD isIVR = FALSE;
      DWORD isForMuteAllButLecture = FALSE;
      BYTE  isMuteAllButLeader = FALSE;
      if (pSeg)
      {
        *pSeg >> isIVR;
        *pSeg >> isForMuteAllButLecture;
        *pSeg >> isMuteAllButLeader;
      }
      else
      {
        // Don't know if it's allowed or not, add assert temporarily
        PASSERT(1);
      }
      MuteAll(0, partyRsrcID, isIVR, isForMuteAllButLecture, isMuteAllButLeader);
      break;
    }

    case eCAM_EVENT_CONF_SECURE:
      SecureConf(1, partyRsrcID);
      break;

    case eCAM_EVENT_CONF_UNSECURE:
      SecureConf(0, partyRsrcID);
      break;

    case eIVR_SHOW_PARTICIPANTS:
      ShowParticipants(partyRsrcID);
      break;

    case eIVR_SHOW_GATHERING:
      ShowGathering(partyRsrcID);
      break;

    case eCAM_EVENT_CONF_DTMF_FORWARDING:
      if (pSeg)
      {
        ForwardDTMF(partyRsrcID, pSeg);
      }
      else
      {
        PASSERT(1);
      }
      break;

    case eCAM_EVENT_CONF_DISCONNECT_INVITED_PARTICIPANT:
      m_confAppInfo->m_pConfApi->DisconnectInvitedParticipant(partyRsrcID);
      break;
  }
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::MuteAll(WORD onOff, DWORD partyRsrcID, WORD isIVRRequest, WORD isForMuteAllButLecture, BYTE isMuteAllButLeader)
{
  if (!m_confAppInfo->m_pAudBrdgInterface)
    return;

  // get number of participants
  int partiesNum = m_participants->GetNumOfParticipants();
  //bug fix if the party that initiated the mute all was leader and isForMuteAllButLecture was false we didnt send conf the update on mute all
  //in to send it anyway one time - Keren reviewed Amir.
  if (isIVRRequest)
  {
	  m_confAppInfo->m_pConfApi->MuteAllButX(partyRsrcID, onOff);
  }


  for (int i = 0; i < partiesNum; i++)
  {
    // get party state
    DWORD iPartyRsrcID = m_participants->GetPartyByIndex(i);
    if ((DWORD)-1 == iPartyRsrcID)
      continue;

    int partyState = m_participants->GetPartyAudioState(iPartyRsrcID);
    if ((partyState != eAPP_PARTY_STATE_MIX) && (partyState != eAPP_PARTY_STATE_IVR_FEATURE))
      continue;

    if (m_participants->GetIsRecordingLinkParty(iPartyRsrcID))
    {
      TRACEINTO << "CConfAppFeatureConfIVR::MuteAll - Skip recording link party, PartyId:" << iPartyRsrcID;
      continue;
    }

    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
    if (pCommConf)
    {
      const CConfAppPartyParams* pPartyParams = m_participants->GetParty(iPartyRsrcID);
      if (pPartyParams)
      {
        string      partyName  = ((CConfAppPartyParams*)pPartyParams)->GetPartyName();
        CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName.c_str());
        if (IsValidPObjectPtr(pConfParty))
        {
          if (pConfParty->GetCascadeMode())
          {
            TRACEINTO << "CConfAppFeatureConfIVR::MuteAll - Skip cascade party, PartyId:" << iPartyRsrcID;
            continue;
          }
        }
      }
    }

    if (m_participants->GetIsCascadeLinkParty(iPartyRsrcID) && (1 == onOff))
    {
      TRACEINTO << "CConfAppFeatureConfIVR::MuteAll - Skip cascade link party, PartyId:" << iPartyRsrcID;
      continue;
    }

    if (1 == onOff) // mute all
    {
      if (iPartyRsrcID != partyRsrcID)  // mute all except the party who requested the action
      {
		if (m_participants->GetIsPartyLeader(iPartyRsrcID))
        {
			if (isIVRRequest)
			{
            TRACEINTO << "CConfAppFeatureConfIVR::MuteAll - Skip leader party, PartyId:" << iPartyRsrcID;
            continue;
        	}
			else if (isMuteAllButLeader == TRUE)
			{
				m_confAppInfo->m_pAudBrdgInterface->UpdateMute(iPartyRsrcID, eMediaIn, eOff, OPERATOR_REQ_BY_ID);
				continue;
			}
		}
        m_confAppInfo->m_pAudBrdgInterface->UpdateMute(iPartyRsrcID, eMediaIn, eOn, isIVRRequest ? MCMS : OPERATOR_REQ_BY_ID);
      }
      else
      {
        m_confAppInfo->m_pAudBrdgInterface->UpdateMute(iPartyRsrcID, eMediaIn, eOff, isIVRRequest ? MCMS : OPERATOR_REQ_BY_ID);
        if (isIVRRequest)
        	m_confAppInfo->m_pConfApi->MuteAllButX(partyRsrcID, 1);
      }
    }
    else // un-mute all
    {
      if (iPartyRsrcID == partyRsrcID)  // mute all except the party who requested the action
        if (isIVRRequest)
          m_confAppInfo->m_pConfApi->MuteAllButX(partyRsrcID, 0);
      m_confAppInfo->m_pAudBrdgInterface->UpdateMute(iPartyRsrcID, eMediaIn, eOff, isIVRRequest ? MCMS : OPERATOR_REQ_BY_ID);
    }
  }
}


//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::SecureConf(WORD onOff, DWORD partyRsrcID)
{
  TRACEINTO << "CConfAppFeatureConfIVR::SecureConf - PartyId:" << partyRsrcID << ", On/Off:" << onOff;

  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
  PASSERT_AND_RETURN(!pCommConf);

  DWORD dtmfCode;
  if (onOff)
  {
    pCommConf->SetConfSecured(YES);
    dtmfCode = DTMF_SECURE_CONF;
  }
  else
  {
    pCommConf->SetConfSecured(NO);
    dtmfCode = DTMF_UNSECURE_CONF;
  }

  // Show participants text (Video Text)
  WORD permission = m_confAppInfo->GetIvrPermission(dtmfCode);
  if ((WORD)(-1) == permission)                   // error
    permission = DTMF_LEADER_ACTION;              // default permission in case of error

  m_confAppInfo->m_pConfApi->ShowParticipantsToConf(partyRsrcID, (DWORD)permission, dtmfCode);
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::ShowParticipants(DWORD partyRsrcID)
{
  TRACEINTO << "CConfAppFeatureConfIVR::ShowParticipants - PartyId:" << partyRsrcID;

  // Show participants text (Video Text)
  WORD permission = m_confAppInfo->GetIvrPermission(DTMF_SHOW_PARTICIPANTS);
  if ((WORD)(-1) == permission)                     // error
    permission = DTMF_LEADER_ACTION;                // default permission in case of error

  m_confAppInfo->m_pConfApi->ShowParticipantsToConf(partyRsrcID, (DWORD)permission, (DWORD)DTMF_SHOW_PARTICIPANTS);
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::ShowGathering(DWORD partyRsrcID)
{
  TRACEINTO << "CConfAppFeatureConfIVR::ShowGathering - PartyId:" << partyRsrcID;

  // Show participants text (Video Text)
  WORD permission = m_confAppInfo->GetIvrPermission(DTMF_SHOW_PARTICIPANTS);
  if ((WORD)(-1) == permission)                     // error
    permission = DTMF_LEADER_ACTION;                // default permission in case of error

  m_confAppInfo->m_pConfApi->ShowGathering(partyRsrcID, (DWORD)permission, (DWORD)DTMF_SHOW_PARTICIPANTS);
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::ForwardDTMF(DWORD partyRsrcID, CSegment* pSeg)
{
  TRACEINTO << "CConfAppFeatureConfIVR::ForwardDTMF - PartyId:" << partyRsrcID;

  // we need to implement 2 DTMF sending ways:  in-band and out-band
  if (!m_confAppInfo->m_pAudBrdgInterface)          // only for in-band
  {
    TRACEINTO << "m_confAppInfo->m_pAudBrdgInterface is NULL - PartyId:" << partyRsrcID;
    return;
  }

  // get number of participants
  int  partiesNum = m_participants->GetNumOfParticipants();

  // get DTMF string to send
  char sDtmfString[DTMF_STRING_LEN+1];
  *pSeg >> sDtmfString;
  sDtmfString[DTMF_STRING_LEN] = '\0';

  TRACEINTO << "PartyId: " << partyRsrcID << "partiesNum: " << (DWORD)partiesNum << "sDtmfString: " << sDtmfString;

  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
  BOOL bIsChairperson = m_participants->GetIsPartyLeader(partyRsrcID);
  BOOL bIsMfwProductType = FALSE;
  if (eProductTypeSoftMCUMfw == m_confAppInfo->GetMCUproductType())
	  bIsMfwProductType = TRUE;

  for (int i = 0; i < partiesNum; i++)
  {
    // get party ID
    DWORD iPartyRsrcID = m_participants->GetPartyByIndex(i);
    if ((DWORD)-1 == iPartyRsrcID)
    {
    	TRACEINTO << "Party with index: " << (DWORD)i << "doesn't exist moving to next party";
        continue;
    }

    BOOL bIsForwardForMfw = FALSE;
    if (bIsMfwProductType && bIsChairperson)
    {
     	const CConfAppPartyParams* pPartyParams = m_participants->GetParty(iPartyRsrcID);
     	if (pPartyParams && pCommConf)
     	{
     		string partyName  = ((CConfAppPartyParams*)pPartyParams)->GetPartyName();
     		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName.c_str());
     		if (pConfParty && pConfParty->IsReceiveDtmfFromChairperson())
     		{
     			TRACEINTO << "DTMF-Forward-IBM : bIsForwardForMfw = true. Party ID=" << iPartyRsrcID;
   				bIsForwardForMfw = TRUE;
   			}
     		else
     		{
					TRACEINTO << "DTMF-Forward-IBM : Error: Invalid PartyID: PartyID: " << (DWORD)iPartyRsrcID;
     		}
     	}
     }

    // checking if it is not a Cascade link
    if (!bIsForwardForMfw)
    {
		if (!m_participants->GetIsCascadeLinkParty(iPartyRsrcID))
		{
		  if (!IsAlwaysForwardDtmfInGWSessionToIsdn())  // in case it is NOT a cascade link,we have to check if we need to FWD to ISDN
		  {
			TRACEINTO << "PartyId: " << iPartyRsrcID << ", IsCascadeLinkParty:" << (int)m_participants->GetIsCascadeLinkParty(iPartyRsrcID);
			continue;
		  }
		}

		// if connected properly
		int partyState = m_participants->GetPartyAudioState(iPartyRsrcID);
		if (partyState != eAPP_PARTY_STATE_MIX)
		{
		  TRACEINTO << "PartyId: " << iPartyRsrcID << ", audio not in state MIX, state: " << (DWORD)partyState;
		  continue;
		}
    }

    // if the same party as we received the DTMF from then nothing to do
    if (iPartyRsrcID == partyRsrcID)
      continue;

    TRACEINTO << " PartyId: "<< iPartyRsrcID << ", DTMF String: " << sDtmfString;

    m_confAppInfo->m_pConfApi->SendDtmfFromParty(iPartyRsrcID, sDtmfString);
  }
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::RemoveAllFeaturesUponConfTerminating()
{
  m_features->RemoveAllFeaturesUponConfTerminating();
}

//--------------------------------------------------------------------------
int CConfAppFeatureConfIVR::ReplaceTokenWithSequenceNum(DWORD seqNumToken, DWORD sequenceNum)
{
  return m_features->ReplaceTokenWithSequenceNum(seqNumToken, sequenceNum);
}

//--------------------------------------------------------------------------
int CConfAppFeatureConfIVR::HandlePlayMsgAckInd(DWORD ack_seq_num)
{
  return m_features->HandlePlayMsgAckInd(ack_seq_num);
}

//--------------------------------------------------------------------------
void CConfAppFeatureConfIVR::HandleChangeIC()
{
  // Check if there is at least one feature in the active events list for conference
  if (m_confAppInfo->GetConfActiveEventsCounter() > 0)
    m_features->HandleChangeIC();
}

//--------------------------------------------------------------------------////
BOOL CConfAppFeatureConfIVR::IsAlwaysForwardDtmfInGWSessionToIsdn()
{
  BOOL isAlwaysForwardDtmfInGWSessionToIsdn = NO;
  CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
  if (pSysConfig)
    pSysConfig->GetBOOLDataByKey("ALWAYS_FORWARD_DTMF_IN_GW_SESSION_TO_ISDN", isAlwaysForwardDtmfInGWSessionToIsdn);

  return isAlwaysForwardDtmfInGWSessionToIsdn;
}


//--------------------------------------------------------------------------
BOOL CConfAppFeatureConfIVR::IsISDNParty(DWORD partyRsrcID)
{
  BOOL RC = NO;
  CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
  if (pCommConf)
  {
    const CConfAppPartyParams* pPartyParams = m_participants->GetParty(partyRsrcID);
    if (pPartyParams)
    {
      string      partyName  = ((CConfAppPartyParams*)pPartyParams)->GetPartyName();
      CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName.c_str());
      if (IsValidPObjectPtr(pConfParty))
      {
        if ((pConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE))
        {
          RC = YES;
        }
      }
    }
  }
  TRACEINTO << "CConfAppFeatureConfIVR::IsISDNParty - PartyId:" << partyRsrcID << "IsISDNParty:" << RC;
  return RC;
}



////////////////////////////////////////////////////////////////////////////
//                        CConfAppFeaturePartyIVR
////////////////////////////////////////////////////////////////////////////
CConfAppFeaturePartyIVR::CConfAppFeaturePartyIVR(CConfAppInfo* confAppInfo, CConfAppPartiesList* participants)
                        :CConfAppFeatureObject(F_APP_PARTY_IVR)
{
  m_confAppInfo  = confAppInfo;
  m_participants = participants;
  m_features     = new CFeatureObjectList(confAppInfo, participants);
}

//--------------------------------------------------------------------------
CConfAppFeaturePartyIVR::~CConfAppFeaturePartyIVR()
{
  POBJDELETE(m_features);
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::StartPartyNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID)
{
	TRACEINTO
		<< "- Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
		<< ", PartyId:" << partyRsrcID
		<< ", EventId:" << eventUniqueNumber;

	switch (opcode)
	{
		case eCAM_EVENT_PARTY_MUTE:
		case eCAM_EVENT_PARTY_UNMUTE:
		case eCAM_EVENT_PARTY_OVERRIDE_MUTEALL:
			DoActionForParty(opcode, partyRsrcID, CAM_START_FEATURE); // do mute / unmute and keep with message (new feature)
			break;

		case eCAM_EVENT_PARTY_END_VIDEO_IVR:
		case eCAM_EVENT_PARTY_IN_CONF_IND:
		DoActionForParty(opcode, partyRsrcID, CAM_START_FEATURE);
			return;

		case eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE:
			break;
	}

	if (opcode == eCAM_EVENT_PARTY_OVERRIDE_MUTEALL)
		opcode = eCAM_EVENT_PARTY_UNMUTE; // to keep be treated as self unute

	m_features->StartNewPartyFeature(eventUniqueNumber, opcode, partyRsrcID, CAM_PARTY_ACTION);
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::StopPartyNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID)
{
  m_features->StopNewFeature(eventUniqueNumber, opcode, partyRsrcID, CAM_PARTY_ACTION);
}

//--------------------------------------------------------------------------
int CConfAppFeaturePartyIVR::DoPartyAction(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam)
{
  TRACEINTO << "Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", PartyId:" << partyRsrcID;

  switch (opcode)
  {
    case eCAM_EVENT_PARTY_INC_VOLUME:
    case eCAM_EVENT_PARTY_DEC_VOLUME:
    case eCAM_EVENT_PARTY_SHOW_SLIDE:
    case eCAM_EVENT_PARTY_JOIN_CONF_VIDEO:
    case eCAM_EVENT_PARTY_SEND_DTMF:
    case eCAM_EVENT_PARTY_CLIENT_SEND_DTMF: // for Call Generator
    DoActionForParty(opcode, partyRsrcID, action, pParam);
    return 0;
  } // switch

  m_features->DoPartyAction(opcode, partyRsrcID, action, pParam);

  return 0;
}

//--------------------------------------------------------------------------
int CConfAppFeaturePartyIVR::GetIVRMessageID(DWORD messageID, DWORD& subInd)
{
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CConfAppFeaturePartyIVR::AskForObjection(TConfAppEvents feature, DWORD partyRsrcID)
{
  return m_features->AskForObjection(feature, partyRsrcID);
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID)
{
  m_features->AskForPreAction(feature, partyRsrcID, CAM_PARTY_ACTION);
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::PlayMessage(CSegment* pParam)
{
  DWORD partyRsrcID;
  *pParam >> partyRsrcID;
  TRACEINTO << "CConfAppFeaturePartyIVR::PlayMessage - PartyId:" << partyRsrcID;
  m_features->PlayPartyMessage(partyRsrcID, pParam);
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::StopMessage(CSegment* pParam)
{
  DWORD partyRsrcID;
  DWORD mediaType; // audio / video
  *pParam >> partyRsrcID;
  *pParam >> mediaType;
  TRACEINTO << "CConfAppFeaturePartyIVR::StopMessage - PartyId:" << partyRsrcID;
  m_features->StopPartyMessage(partyRsrcID, mediaType);
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::RecordRollCall(CSegment* pParam)
{
  DWORD partyRsrcID;
  *pParam >> partyRsrcID;
  TRACEINTO << "CConfAppFeaturePartyIVR::RecordRollCall - PartyId:" << partyRsrcID;

  CSegment pParamCopy = *pParam;

  m_features->RecordRollCall(partyRsrcID, pParam);

  // vngfe-8707
   int partyIndex = m_participants->FindPartyIndex(partyRsrcID);
   TRACECOND_AND_RETURN(-1 == partyIndex, "CConfAppFeaturePartyIVR::RecordRollCall - Failed to update party info, party not found, PartyId:" << partyRsrcID);
   CConfAppPartyParams* party = m_participants->m_partyList[partyIndex];

   //Bridge-15108 For TIP EP the aux sounds the IVR ie it plays the file and don't record (master records only),
   //EMB sends the IVR_RECORD_ROLL_CALL_IND only for recording and not playing the file, so should not set the state to recording
   //for AUX party or any party that do not record, the indication received from EMB will set the state back to not recording.
   CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage;
   pIVRPlayMessage->DeSerialize(&pParamCopy);

   SIVRPlayMessageStruct* pPlayMsg = &(pIVRPlayMessage->play);

   bool isRecordingfound = false;
   if (pPlayMsg)
   {
	   for (size_t i = 0; i < pPlayMsg->numOfMediaFiles;++i)
	   {
		   if (pPlayMsg->mediaFiles[i].actionType ==  IVR_ACTION_TYPE_RECORD)
		   {
			   isRecordingfound = true;
			   break;
		   }
	   }
   }

   party->SetRollCallInRecording(isRecordingfound);
   POBJDELETE(pIVRPlayMessage);
 }
 //--------------------------------------------------------------------------
 void CConfAppFeaturePartyIVR::StopRollCallRecording(CSegment* pParam)
 {

 	DWORD partyRsrcID = (DWORD)(-1);
 	DWORD status = 0;
 	*pParam >> partyRsrcID >> status;
 	TRACEINTO << "ROLL_CALL_STOP_RECORDING (03) PartyId: " << partyRsrcID << ", status = " << (DWORD)status;
 	if(STATUS_OK == status){
 		m_features->StopRollCallRecording(partyRsrcID, pParam);
 	}else{
 		int partyIndex = m_participants->FindPartyIndex(partyRsrcID);
 		TRACECOND_AND_RETURN(-1 == partyIndex, "CConfAppFeaturePartyIVR::StopRollCallRecording - Failed to update party info, party not found, PartyId:" << partyRsrcID);
 		CConfAppPartyParams* party = m_participants->m_partyList[partyIndex];

 		party->SetRollCallInRecording(false);
 	}

 }
 //--------------------------------------------------------------------------
 void CConfAppFeaturePartyIVR::StopRollCallRecordingAck(DWORD PartyId, DWORD status)
 {
 	int partyIndex = m_participants->FindPartyIndex(PartyId);
 	TRACECOND_AND_RETURN(-1 == partyIndex, "CConfAppFeaturePartyIVR::StopRollCallRecordingAck - Failed, party not found, PartyId:" << PartyId);
 	CConfAppPartyParams* party = m_participants->m_partyList[partyIndex];

 	// delete roll call file
 	string recordingFileName = party->GetRollCallRecFullPath();

 	TRACEINTO << " ROLL_CALL_STOP_RECORDING_ACK (02) partyId = " << PartyId << " , status = " << status << " , delete file: " << recordingFileName;

 	BOOL isFileExists = IsFileExists(recordingFileName);
 	if (IsFileExists(recordingFileName))
 	{
 		if (!DeleteFile(recordingFileName))
 		{
 			TRACEINTO << "Failed to delete Roll Call Recording file: " << recordingFileName;
 		}else{
 			party->SetPartyRollCallName("");
 		}
 	}else{
 		TRACEINTO << "Roll Call Recording file does not exist: " << recordingFileName;
 	}

 	party->SetRollCallRecordingExists(0);
 	party->SetRollCallInRecording(false);

 	(party->m_pPartyApi)->SendOpcodeToIvrSubFeature(STOP_ROLL_CALL_RECORDING_ACK);
 }
//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::EnterPartyToMix(DWORD partyRsrcID)
{
  TRACEINTO << "CConfAppFeaturePartyIVR::EnterPartyToMix - PartyId:" << partyRsrcID;
  CFeatureObject* tempObj = new CFeatureObject(m_confAppInfo, m_participants);
  tempObj->EnterPartyToMix(partyRsrcID);
  POBJDELETE(tempObj);
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::DtmfReceived(DWORD opcode, CSegment* pParam)
{
  DWORD partyRsrcID;
  DWORD partyID;
  char  partyName[H243_NAME_LEN];
  WORD  mediaDirection;
  *pParam >> partyRsrcID;
  *pParam >> partyID;
  *pParam >> partyName;
  *pParam >> mediaDirection;

  m_features->DtmfReceived(opcode, partyRsrcID, pParam);
  return;
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::RollCallRecorded(CSegment* pParam)
{
  DWORD partyRsrcID;
  DWORD partyID;
  char  partyName[H243_NAME_LEN];
  WORD  mediaDirection;
  DWORD status;
  DWORD recordingLength;
  *pParam >> partyRsrcID;
  *pParam >> partyID;
  *pParam >> partyName;
  *pParam >> mediaDirection;
  *pParam >> status;
  *pParam >> recordingLength;

  TRACEINTO << "CConfAppFeaturePartyIVR::RollCallRecorded - PartyId:" << partyRsrcID << ", MediaDirection:" << mediaDirection << ", Status:" << status;

  int partyIndex = m_participants->FindPartyIndex(partyRsrcID);
  TRACECOND_AND_RETURN(-1 == partyIndex, "CConfAppFeaturePartyIVR::RollCallRecorded - Failed, party not found, PartyId:" << partyRsrcID);

  CConfAppPartyParams* party = m_participants->m_partyList[partyIndex];

  // vngfe-8707
  party->SetRollCallInRecording(false);

  if (status == STATUS_OK)
  {
    string partyRollCallName = IVR_FOLDER_ROLLCALL;
    partyRollCallName += party->GetPartyRollCallName();

    // party's Roll Call recording length (duration) as received from card manager
    TRACEINTO << "CConfAppFeaturePartyIVR::RollCallRecorded - RecordingName:" << partyRollCallName.c_str() << ", RecordingLength:" << recordingLength;

    if (0 != recordingLength)
    {
      // Set party's Roll Call recording duration and checksum
      WORD duration;
      WORD checksum;

      // Modified for Ninja WW2
      if (IsSoftMcu())
      {
        status = STATUS_OK;
        duration = (recordingLength - 1) / 32000 + 1;
        checksum = 0;
      }
      else
      {
        status = ::GetpAVmsgServList()->GetFileParams(partyRollCallName.c_str(), &duration, &checksum);
      }
      if ((STATUS_OK == status) && (0 != duration))
      {
        party->SetRollCallRecDuration(duration);
        party->SetPartyRollCallCheckSum(checksum);

        // Set the Roll Call recording flag to exists
        party->SetRollCallRecordingExists(true);
      }

      TRACECOND(STATUS_OK != status, "CConfAppFeaturePartyIVR::RollCallRecorded - Failed, Invalid recording file params, RecordingName:"   << partyRollCallName.c_str());
      TRACECOND(0 == duration      , "CConfAppFeaturePartyIVR::RollCallRecorded - Failed, Invalid recording file duration, RecordingName:" << partyRollCallName.c_str());
    }
  }

  // Send indication to sub feature (Roll Call) that the Roll Call msg was recorded
  // (this indication is sent even if the RECORD_ROLL_CALL_IND was received with STATUS_FAIL
  // or duration=0, for the sub-feature to continue)
  (party->m_pPartyApi)->SendOpcodeToIvrSubFeature(MSG_RECORDED);
}

//--------------------------------------------------------------------------
int CConfAppFeaturePartyIVR::ReplaceTokenWithSequenceNum(DWORD seqNumToken, DWORD sequenceNum)
{
  return m_features->ReplaceTokenWithSequenceNum(seqNumToken, sequenceNum);
}

//--------------------------------------------------------------------------
int CConfAppFeaturePartyIVR::HandlePlayMsgAckInd(DWORD ack_seq_num)
{
  return m_features->HandlePlayMsgAckInd(ack_seq_num);
}
//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::RemoveFeatureByOpcode(DWORD opcode) //AT&T
{
	m_features->RemoveFeatureByOpcode(opcode);
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::DoActionForParty(DWORD opcode, DWORD PartyId, DWORD action, CSegment* pParam)
{
	if (!m_confAppInfo->m_pAudBrdgInterface)
		return;

	switch (opcode)
	{
		case eCAM_EVENT_PARTY_INC_VOLUME:
		case eCAM_EVENT_PARTY_DEC_VOLUME:
		{
			EMediaDirection mediaDirection;
			if (0 == action)                            // action on Decoder
				mediaDirection = eMediaIn;
			else
				mediaDirection = eMediaOut;

			BYTE changeRate = 1;
			if (eCAM_EVENT_PARTY_INC_VOLUME == opcode)  // increase volume
				m_confAppInfo->m_pAudBrdgInterface->IncreaseAudioVolume(PartyId, mediaDirection, changeRate);
			else
				// decrease volume
				m_confAppInfo->m_pAudBrdgInterface->DecreaseAudioVolume(PartyId, mediaDirection, changeRate);
			break;
		}

		case eCAM_EVENT_PARTY_MUTE:
		{
			m_confAppInfo->m_pAudBrdgInterface->UpdateMute(PartyId, eMediaIn, eOn, PARTY);
			break;
		}

		case eCAM_EVENT_PARTY_UNMUTE:
		{
			m_confAppInfo->m_pAudBrdgInterface->UpdateMute(PartyId, eMediaIn, eOff, PARTY);
			break;
		}

		case eCAM_EVENT_PARTY_OVERRIDE_MUTEALL:
		{
			m_confAppInfo->m_pAudBrdgInterface->UpdateMute(PartyId, eMediaIn, eOff, MCMS);
			break;
		}

		case eCAM_EVENT_PARTY_SHOW_SLIDE:
		case eCAM_EVENT_PARTY_JOIN_CONF_VIDEO:
		case eCAM_EVENT_PARTY_END_VIDEO_IVR:
		case eCAM_EVENT_PARTY_SEND_DTMF:
		case eCAM_EVENT_PARTY_CLIENT_SEND_DTMF: // for Call Generator
		{
			m_features->DoActionForParty(opcode, PartyId, action, pParam);
			break;
		}

		case eCAM_EVENT_PARTY_IN_CONF_IND:
		{
			m_confAppInfo->m_pConfApi->SendPartyInConfIndToPartyCntl(PartyId);
			break;
		}

		case eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE:
		{
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CConfAppFeaturePartyIVR::RemoveAllFeaturesUponConfTerminating()
{
  m_features->RemoveAllFeaturesUponConfTerminating();
}


#define FIND_TYPE_RSRC                       1
#define FIND_TYPE_OPCODE                     2
#define FIND_TYPE_OPCODEandRSRC              3

#define TIMER_CHAIR_DROPPED                  122
#define STOP_WAIT_FOR_CHAIR_LIST_DELAY_TIMER 131
#define STOP_WAIT_FOR_CHAIR_LIST_DELAY_TOUT  10 // 100ms

PBEGIN_MESSAGE_MAP(CFeatureObjectList)
  ONEVENT(TIMER_CHAIR_DROPPED                 , CONNECT, CFeatureObjectList::OnTimerChairDropped)
  ONEVENT(STOP_WAIT_FOR_CHAIR_LIST_DELAY_TIMER, CONNECT, CFeatureObjectList::OnTimerStopWaitForChairDelay)
PEND_MESSAGE_MAP(CFeatureObjectList, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CFeatureObjectList
////////////////////////////////////////////////////////////////////////////
CFeatureObjectList::CFeatureObjectList(CConfAppInfo* confAppInfo, CConfAppPartiesList* participants)
{
  m_state        = CONNECT;
  m_confAppInfo  = confAppInfo;
  m_participants = participants;
  m_numOfFeatures = 0;
  memset(m_list, 0, sizeof(m_list));

  VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CFeatureObjectList::~CFeatureObjectList()
{
  for (int ind = 0; ind < MAX_SUB_FEATURES; ind++)
    POBJDELETE(m_list[ind]);
}
//--------------------------------------------------------------------------
void* CFeatureObjectList::GetMessageMap()
{
  return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CFeatureObjectList::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
  DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
int CFeatureObjectList::FindIndexForNewFeature()
{
  for (int ind = 0; ind < MAX_SUB_FEATURES; ind++)
    if (NULL == m_list[ind])
      return ind;

  return -1;
}

//--------------------------------------------------------------------------
int CFeatureObjectList::AddFeature(int index)
{
  PASSERT_AND_RETURN_VALUE(m_numOfFeatures >= MAX_SUB_FEATURES, -1);
  PASSERT_AND_RETURN_VALUE(index >= MAX_SUB_FEATURES, -1);
  PASSERT_AND_RETURN_VALUE(index < 0, -1);

  for (int ind = m_numOfFeatures; ind > index; ind--)
    m_list[ind] = m_list[ind - 1];

  m_list[index] = new CFeatureObject(m_confAppInfo, m_participants);
  m_numOfFeatures++;

  return index;
}

//--------------------------------------------------------------------------
STATUS CFeatureObjectList::RemoveFeature(int index)
{
  PASSERT_AND_RETURN_VALUE(m_numOfFeatures <= 0, STATUS_FAIL);
  PASSERT_AND_RETURN_VALUE(index >= MAX_SUB_FEATURES, STATUS_FAIL);
  PASSERT_AND_RETURN_VALUE(m_numOfFeatures > MAX_SUB_FEATURES, STATUS_FAIL);
  PASSERT_AND_RETURN_VALUE(!m_list[index], STATUS_FAIL);

  DWORD opcode = m_list[index]->GetOpcode();

  TRACEINTO << "CFeatureObjectList::RemoveFeature:"
            << "\n  Opcode     :" << m_confAppInfo->GetStringFromOpcode((TConfAppEvents)opcode) << " (#" << opcode << ")"
            << "\n  PartyId    :" << m_list[index]->GetPartyRscrID()
            << "\n  FeatureType:" << (DWORD)m_list[index]->GetFeatureType()
            << "\n  Index      :" << index
            << "\n  FeaturesNum:" << m_numOfFeatures-1;

  POBJDELETE(m_list[index]);

  for (int ind = index; ind < m_numOfFeatures - 1; ind++)
    m_list[ind] = m_list[ind + 1];

  m_list[m_numOfFeatures - 1] = NULL;

  m_numOfFeatures--;

  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CFeatureObjectList::RemoveFeatureByOpcode(DWORD opcode)
{
	TRACEINTO;

	int ind = FindFeature(FIND_TYPE_OPCODE, opcode);
	if (-1 != ind)
		RemoveFeature(ind);
}
//--------------------------------------------------------------------------
int CFeatureObjectList::AddFeatureToHead()
{
  return AddFeature(0);
}

//--------------------------------------------------------------------------
int CFeatureObjectList::AddFeatureToTail()
{
  return AddFeature(m_numOfFeatures);
}

//--------------------------------------------------------------------------
int CFeatureObjectList::AddFeatureAfterCurrent()
{
  if (0 == m_numOfFeatures)
    return AddFeature(0);
  else
    return AddFeature(1);
}

//--------------------------------------------------------------------------
void CFeatureObjectList::MoveFirstFeatureToEnd()
{
  if (m_numOfFeatures < MAX_SUB_FEATURES)
  {
    m_list[m_numOfFeatures] = m_list[0];

    for (int ind = 0; ind < m_numOfFeatures; ind++)
      m_list[ind] = m_list[ind + 1];

    m_list[m_numOfFeatures] = NULL;
  }
}

//--------------------------------------------------------------------------
void CFeatureObjectList::StopNewFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID, WORD confOrParty)
{
  TRACEINTO << " Opcode:"     << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", PartyId:"    << partyRsrcID
            << ", Conf/Party:" << ((confOrParty == CAM_CONF_ACTION) ? "CONF_ACTION" : "PARTY_ACTION")
            << ", EventId:"    << eventUniqueNumber;

  OnStopFeature(eventUniqueNumber, opcode, partyRsrcID, confOrParty);
}

//--------------------------------------------------------------------------
int CFeatureObjectList::CreateNewConfFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID, WORD replaceOrAppend)
{
  TRACEINTO << " Opcode:"     << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", PartyId:"    << partyRsrcID
            << ", AppendType:" << replaceOrAppend
            << ", EventId:"    << eventUniqueNumber;

  int featureIndex = -1;

  if (opcode == eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED)
  {
    TRACEINTO << "CFeatureObjectList::CreateNewConfFeature - Start 3 seconds timer before terminating conference upon 'chair dropped'";
    // the time should be taken from the system.cfg
    StartTimer(TIMER_CHAIR_DROPPED, 3 * SECOND);  // error in play message
    return featureIndex;
  }

  // Create New Feature
  switch (replaceOrAppend)
  {
    case CAM_REPLACE_FIRST_FEATURE:
    {
      if (m_list[0] && (m_list[0]->GetOpcode() == eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED)) // This feature shouln't be deleted by another feature (only upon end timer)
      {
        featureIndex = AddFeatureToHead();
      }
      else
      {
        featureIndex = AddFeatureAfterCurrent();
        if (1 == featureIndex && m_list[0])         // the list is not empty - there is a feature currently active
          m_list[0]->EndFeatureAndStopPlayMsgUponRequest();
      }
      break;
    }

    case CAM_APPEND_AFTER_LAST_FEATURE:
    {
      featureIndex = AddFeatureToTail();
      break;
    }

    case CAM_APPEND_AFTER_CURRENT_FEATURE:
    {
      featureIndex = AddFeatureAfterCurrent();
      break;
    }

    case CAM_ROLL_CALL_REPLACE_METHOD:
    {
      if ((NULL != m_list[0]) && (eCAM_EVENT_CONF_ROLL_CALL_REVIEW == m_list[0]->GetOpcode()))
      {
        // In case there is another ROLL CALL Review feature that is active now (first in the list)
        featureIndex = AddFeatureToTail();
        m_list[0]->EndFeatureAndStopPlayMsgUponRequest(); // stop current feature
      }
      else
      {
        if (-1 == FindFeature(FIND_TYPE_OPCODE, eCAM_EVENT_CONF_ROLL_CALL_REVIEW))
        {
          // In case there is no other ROLL CALL Review feature in the list
          featureIndex = AddFeatureToTail();
        }
      }
      break;
    }
  }

  if (-1 == featureIndex)
    return -1;

  m_list[featureIndex]->SetOpcode(opcode);
  m_list[featureIndex]->SetPartyRscrID(partyRsrcID);

  DumpFeatures("CreateNewConfFeature");

  m_list[featureIndex]->CreateNewConfFeature(eventUniqueNumber, opcode, partyRsrcID);

  return featureIndex;
}

//--------------------------------------------------------------------------
void CFeatureObjectList::StartNewConfFeature(int featureInd)
{
  if (!m_list[featureInd])
    return;

  DWORD opcode = m_list[featureInd]->GetOpcode();

  if (eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED == opcode)
  {
    // In case this is the only feature in the conference list
    if (m_numOfFeatures <= 1)
      return;

    // Move feature to the end of the list - this feature is independent (no message to play)
    MoveFirstFeatureToEnd();
  }

  if (!m_list[featureInd])
    return;

  // Get the opcode again for cases that the first feaure was moved
  opcode = m_list[featureInd]->GetOpcode();

  TRACEINTO << "CFeatureObjectList::StartNewConfFeature "
            << "- Opcode:" << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")";

  m_list[featureInd]->StartNewConfFeature();
}

//--------------------------------------------------------------------------
void CFeatureObjectList::StartNewPartyFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID, WORD confOrParty)
{
  TRACEINTO << "- Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", PartyId:" << partyRsrcID
            << ", EventId:" << eventUniqueNumber;

  int ind = FindIndexForNewFeature();
  PASSERTMSG_AND_RETURN(ind == -1, "Error, Failed find index for new feature");

  ind = AddFeature(ind);
  PASSERTMSG_AND_RETURN(ind == -1, "Error, Failed add feature");

  m_list[ind]->SetOpcode(opcode);
  m_list[ind]->SetPartyRscrID(partyRsrcID);

  DumpFeatures("StartNewPartyFeature");

  // In case of an independent feature (not from wait event):
  // Check if there is another feature of the same party with m_eventUniqueNumber!=0
  // and: 1. Copy its unique number; 2. Add it back to the wait list of the relevant event;
  // 3. Stop the feature and delete it from the current list.
  if (0 == eventUniqueNumber)
  {
    int otherFeatureIndex = FindFeature(FIND_TYPE_RSRC, partyRsrcID);

    if (otherFeatureIndex != -1)
    {
      DWORD otherFeatureUniqueNumber = m_list[otherFeatureIndex]->GetEventUniqueNumber();

      if (otherFeatureUniqueNumber != 0)
      {
        eventUniqueNumber = otherFeatureUniqueNumber;

        DWORD otherFeatureOpcode = m_list[otherFeatureIndex]->GetOpcode();
        m_confAppInfo->m_pConfApi->AddPartyFeatureToWaitList(partyRsrcID, eventUniqueNumber, otherFeatureOpcode);
        RemoveFeature(otherFeatureIndex);
        // find index of the new feature
        ind = FindFeature(FIND_TYPE_OPCODEandRSRC, partyRsrcID, opcode);
        PASSERTMSG_AND_RETURN(ind == -1, "Error, wrong index after other feature removal");
      }
    }
  }

  if (m_list[ind]) // VNGFE-6017
    m_list[ind]->StartNewPartyFeature(eventUniqueNumber, opcode, partyRsrcID);
}

//--------------------------------------------------------------------------
int CFeatureObjectList::DoPartyAction(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam)
{
  // find the feature
  int ind = -1;

  switch (opcode)
  {
    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_PLC:          // Change Party Layout
    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_PLC:     // Change from Conference to Private or opposite
    case eCAM_EVENT_PARTY_FORCE_PLC:                  // Change from Conference to Private or opposite
      ind = FindFeature(FIND_TYPE_OPCODEandRSRC, partyRsrcID, eCAM_EVENT_PARTY_PLC);
      break;

    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_VENUS:        // Change Party Layout
    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_VENUS:   // Change from Conference to Private or opposite
    case eCAM_EVENT_PARTY_FORCE_VENUS:                // Change from Conference to Private or opposite
      ind = FindFeature(FIND_TYPE_OPCODEandRSRC, partyRsrcID, eCAM_EVENT_PARTY_VENUS);
      break;

    default:
      ind = FindFeature(FIND_TYPE_RSRC, partyRsrcID);
      break;
  }

  if (-1 == ind)
  {
    TRACEINTO << "CFeatureObjectList::DoPartyAction - Failed, Feature not found:"
              << "\n  Opcode     :" << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
              << "\n  PartyId    :" << partyRsrcID
              << "\n  Action     :" << ((action == CAM_STOP_FEATURE) ? "CAM_STOP_FEATURE" : "CAM_START_FEATURE");

    if (eCAM_EVENT_PARTY_START_IVR == opcode)
      DoActionForParty(opcode, partyRsrcID, action);

    return 0;
  }

  return m_list[ind]->DoPartyAction(opcode, partyRsrcID, action, pParam);
}

//--------------------------------------------------------------------------
int CFeatureObjectList::IsFeatureObjectInList(DWORD opcode, DWORD partyRsrcID)
{
  // find the feature
  int ind = FindFeature(FIND_TYPE_OPCODEandRSRC, partyRsrcID, opcode);
  if (-1 == ind)
    return 0;

  return 1;
}

//--------------------------------------------------------------------------
int CFeatureObjectList::PlayPartyMessage(DWORD partyRsrcID, CSegment* pParam)
{
  int ind = FindFeature(FIND_TYPE_RSRC, partyRsrcID);
  if (-1 == ind)
    return 0;

  m_list[ind]->PlayPartyMessage(partyRsrcID, pParam);
  return 0;
}

//--------------------------------------------------------------------------
int CFeatureObjectList::StopPartyMessage(DWORD partyRsrcID, DWORD mediaType)
{
  int ind = FindFeature(FIND_TYPE_RSRC, partyRsrcID);
  if (-1 == ind)
    return 0;

  m_list[ind]->StopPlayMediaCommand(EVENT_PARTY_REQUEST, mediaType);
  return 0;
}

//--------------------------------------------------------------------------
void CFeatureObjectList::RecordRollCall(DWORD partyRsrcID, CSegment* pParam)
{
  int ind = FindFeature(FIND_TYPE_RSRC, partyRsrcID);
  if (-1 == ind)
    return;

  m_list[ind]->RecordRollCall(partyRsrcID, pParam);
}

//--------------------------------------------------------------------------
void CFeatureObjectList::StopRollCallRecording(DWORD partyRsrcID, CSegment* pParam)
{
	 int ind = FindFeature(FIND_TYPE_RSRC, partyRsrcID);
	  if (-1 == ind){
		  TRACEINTO << "feature not found";
	    return;
	  }

	  m_list[ind]->StopRollCallRecording(partyRsrcID, pParam);
}

//--------------------------------------------------------------------------
int CFeatureObjectList::DtmfReceived(DWORD opcode, DWORD partyRsrcID, CSegment* pParam)
{
  int ind = FindFeature(FIND_TYPE_RSRC, partyRsrcID);
  if (-1 == ind)
  {
    CFeatureObject* tempObj = new CFeatureObject(m_confAppInfo, m_participants);
    tempObj->DtmfReceived(opcode, partyRsrcID, pParam);
    POBJDELETE(tempObj);
  }
  else
  {
		TRACEINTO << "feature found for dtmf- partyId:" << partyRsrcID << ", feature opcode: " << m_list[ind]->GetOpcode();
    m_list[ind]->DtmfReceived(opcode, partyRsrcID, pParam);
  }

  return 0;
}

//--------------------------------------------------------------------------
int CFeatureObjectList::FindFeature(WORD findType, DWORD param, DWORD param2)
{
  for (int ind = 0; ind < MAX_SUB_FEATURES; ind++)
  {
    if (NULL != m_list[ind])
    {
      switch (findType)
      {
        case FIND_TYPE_RSRC:
          if (param == m_list[ind]->GetPartyRscrID())
            return ind;
          break;

        case FIND_TYPE_OPCODE:
          if (param == (DWORD)m_list[ind]->GetOpcode())
            return ind;
          break;

        case FIND_TYPE_OPCODEandRSRC:
          if ((param == m_list[ind]->GetPartyRscrID()) && (param2 == (DWORD)m_list[ind]->GetOpcode()))
            return ind;
          break;
      }
    }
  }

  TRACEINTO << "CFeatureObjectList::DoPartyAction - Failed, Feature not found:"
            << "\n  FindType   :" << findType
            << "\n  Param1     :" << param
            << "\n  Param2     :" << param2;

  return -1;
}

//--------------------------------------------------------------------------
void CFeatureObjectList::Dump(DWORD level)
{
  for (int ind = 0; ind < MAX_SUB_FEATURES; ind++)
    if (NULL != m_list[ind])
      m_list[ind]->Dump(level);
}

//--------------------------------------------------------------------------
int CFeatureObjectList::AskForObjection(TConfAppEvents feature, DWORD partyRsrcID)
{
  for (int ind = 0; ind < MAX_SUB_FEATURES; ind++)
  {
    if (NULL != m_list[ind])
    {
      int status = m_list[ind]->AskForObjection(feature, partyRsrcID);
      if (APP_F_OBJECTION == status)  // there is objection to this feature
        return APP_F_OBJECTION;       // otherwise: keep with the loop...
    }
  }
  return APP_F_NO_OBJECTION;
}

//--------------------------------------------------------------------------
void CFeatureObjectList::AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID, WORD ConfOrParty)
{

	if (m_numOfFeatures > 0 &&  m_numOfFeatures <= MAX_SUB_FEATURES)
	{
		if (CAM_PARTY_ACTION == ConfOrParty)
		{
			for (int ind = (int)(m_numOfFeatures-1); ind >= 0;  ind--)	// up to down because "RemoveFeature" may change the order
			{
			    if ((NULL != m_list[ind]) && (partyRsrcID == m_list[ind]->GetPartyRsrcID()))
			    {
			      if (eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC == m_list[ind]->GetOpcode())
			      {
			        switch ((DWORD)feature)
			        {
			          case eCAM_EVENT_PARTY_FIRST_TO_JOIN:
			          case eCAM_EVENT_PARTY_PLAY_MENU:
			          case eCAM_EVENT_PARTY_MUTE:
			          case eCAM_EVENT_PARTY_UNMUTE:
			          case eCAM_EVENT_PARTY_CHANGE_TO_LEADER:
			          case eCAM_EVENT_PARTY_CHANGE_PW:
			          case eCAM_EVENT_PARTY_RECORDING_FAILED:
			          case IVR_EVENT_RECORDING_IN_PROGRESS:
			          {
			            m_list[ind]->StopPartyFeatureWithoutNotifyCAM(m_list[ind]->GetOpcode());
			            RemoveFeature(ind);
			            break;
			          }
			        } // switch
			      }
			    }
			}
		}
	}


  for (int ind = 0; ind < m_numOfFeatures; ind++)
  {
    if ((NULL != m_list[ind]))
    {
      m_list[ind]->AskForPreAction(feature, partyRsrcID, ConfOrParty);
    }
  }
}

//--------------------------------------------------------------------------
void CFeatureObjectList::OnStopFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID, WORD confOrParty)
{
  // VNGFE-4084
  // special opcode treatment
  if ((opcode == eCAM_EVENT_PARTY_WAIT_FOR_CHAIR) && (confOrParty == CAM_PARTY_ACTION))
  {
    StopPartiesWaitForChair(eventUniqueNumber); // end all wait for chair parties
    return;
  }

  // find the feature
  int ind;
  switch (opcode)
  {
    case eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC:
    case eCAM_EVENT_PARTY_PLAY_RINGING_TONE:
    {
      ind = FindFeature(FIND_TYPE_OPCODE, opcode);
      break;
    }

    case eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW:
    {
      ind = FindFeature(FIND_TYPE_OPCODEandRSRC, eCAM_EVENT_CONF_ROLL_CALL_REVIEW);
      break;
    }

    default:
    {
      ind = FindFeature(FIND_TYPE_OPCODEandRSRC, partyRsrcID, opcode);
      break;
    }
  } // switch

  if (-1 == ind)
    return;                                               // not found

  // Set msg_seq_num to 0 so if ack comes first it won't be found
  m_list[ind]->SetIvrMessageSessionIDCam(0);

  TRACEINTO << " - "
            << "- Opcode:"      << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", PartyId:"     << partyRsrcID
            << ", Conf/Party:"  << ((confOrParty == CAM_CONF_ACTION) ? "CAM_CONF_ACTION" : "CAM_PARTY_ACTION")
            << ", Index:"       << ind;

  // stop features
  if (confOrParty == CAM_CONF_ACTION)
  {
    m_list[ind]->StopConfFeature(eventUniqueNumber, opcode, partyRsrcID);
    RemoveFeature(ind);

    // Activate next feature if needed
    if (eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED == opcode)
      return;                                             // No need to activate next feature - this feature is independent

    if (m_confAppInfo->GetConfActiveEventsCounter() > 0)  // there is at least one more feature left in the active events list for conf
    {
      int featureInd = 0;
      StartNewConfFeature(featureInd);
    }
  }
  else                                                    // confOrParty == CAM_PARTY_ACTION
  {
    m_list[ind]->StopPartyFeature(eventUniqueNumber, opcode, partyRsrcID);
    RemoveFeature(ind);
  }
}

//--------------------------------------------------------------------------
void CFeatureObjectList::StopPartiesWaitForChair(DWORD eventUniqueNumber)
{
  // for all parties in "wait for chair state"
  WORD parties_stop = 0;

	if (m_numOfFeatures > 0 &&  m_numOfFeatures <= MAX_SUB_FEATURES)
	{
		for (int ind = (int)(m_numOfFeatures-1); ind >= 0;  ind--)
		{
			if (NULL != m_list[ind])
			{
			  if (eCAM_EVENT_PARTY_WAIT_FOR_CHAIR == m_list[ind]->GetOpcode())
			  {
				DWORD partyRsrcID = m_list[ind]->GetPartyRscrID();

				m_list[ind]->StopPartyFeature(eventUniqueNumber, eCAM_EVENT_PARTY_WAIT_FOR_CHAIR, partyRsrcID);
				RemoveFeature(ind);

				// D.K.VNGR-21060 - We go to sleep for a little while every 50 IVR deletion.
			    // To allow other tasks to be performed in (answering the watch dog and reading the messages we send)
				if (parties_stop % 20 == 0)
				{
					// .VNGR-21060 back - timer instead of sleep - because messages sent the same task
					// SystemSleep(2);
					CSegment* pTimerSeg = new CSegment();
					*pTimerSeg << eventUniqueNumber;
					TRACEINTO << "CFeatureObjectList::StopPartiesWaitForChair - Start timer for " << STOP_WAIT_FOR_CHAIR_LIST_DELAY_TOUT * 10 << " ms";
					StartTimer(STOP_WAIT_FOR_CHAIR_LIST_DELAY_TIMER, STOP_WAIT_FOR_CHAIR_LIST_DELAY_TOUT, pTimerSeg);
					return;
				}
			  }
			}
		}
	}
}

//--------------------------------------------------------------------------
void CFeatureObjectList::OnTimerStopWaitForChairDelay(CSegment* pParam)
{
  DWORD eventUniqueNumber = 0;
  if (pParam)
    *pParam >> eventUniqueNumber;

  StopPartiesWaitForChair(eventUniqueNumber);
}

//--------------------------------------------------------------------------
void CFeatureObjectList::ActionsUponPartyDisconnecting(DWORD partyRsrcID, DWORD audioOrVideo)
{
  if (CAM_AUDIO == audioOrVideo)
  {
    // Remove all features that related to the disconnecting party
	if (m_numOfFeatures > 0 &&  m_numOfFeatures <= MAX_SUB_FEATURES)
	{
		for (int ind = (int)(m_numOfFeatures-1); ind >= 0;  ind--)
	    {
			if (NULL != m_list[ind] && partyRsrcID == m_list[ind]->GetPartyRscrID())
				RemoveFeature(ind);
	    }
    }

    // Stop party IVR if needed
    if (IsPartyInIVR(partyRsrcID, audioOrVideo))
    {
      CFeatureObject* tempObj = new CFeatureObject(m_confAppInfo, m_participants);
      tempObj->SetPartyRscrID(partyRsrcID);

      // Stop play music if needed
      if ((m_confAppInfo->GetSinglePartyNow() == 1) && (m_confAppInfo->GetSinglePartyRsrcId() == partyRsrcID))
        tempObj->StopMusicUponDisconnecting(EVENT_PARTY_REQUEST);

      // Stop IVR
      tempObj->StopIVRUponDisconnecting(EVENT_PARTY_REQUEST);
      POBJDELETE(tempObj);
    }
  }
  else
  {
	  if (IsPartyInIVR(partyRsrcID, audioOrVideo))
	  {
		  // Stop show slide
		  CFeatureObject* tempObj = new CFeatureObject(m_confAppInfo, m_participants);
		  tempObj->SetPartyRscrID(partyRsrcID);
		  tempObj->StopSlideUponDisconnecting();
		  POBJDELETE(tempObj);
	  }
	  // Set the flags m_videoBridgeReadyForSlide & m_partyReadyForSlide to zero
	  int partyIndex = m_participants->FindPartyIndex(partyRsrcID);
	  if (partyIndex < 0)
		  return;

	  m_participants->SetPartyReadyForSlide(partyIndex, false);
	  m_participants->SetVideoBridgeReadyForSlide(partyIndex, false);
  }
}

//--------------------------------------------------------------------------
void CFeatureObjectList::ActionsUponPartyMoving(DWORD partyRsrcID)
{
  ActionsUponPartyDisconnecting(partyRsrcID, CAM_AUDIO);
  ActionsUponPartyDisconnecting(partyRsrcID, CAM_VIDEO);
  // Send update stand-alone to audio bridge
  m_confAppInfo->m_pAudBrdgInterface->IvrUpdatePartyStandalone(partyRsrcID, 1);
}

//--------------------------------------------------------------------------
void CFeatureObjectList::ActionsUponLastPartyDisconnecting()
{
  RemoveAllFeaturesUponConfTerminating();

  // Stop Conf IVR if needed
  if (m_confAppInfo->GetConfActiveEventsCounter() > 0)
  {
    CFeatureObject* tempObj = new CFeatureObject(m_confAppInfo, m_participants);
    // Stop play music if needed
    if (m_confAppInfo->IsInWaitForChairNow() == 1)
      tempObj->StopMusicUponDisconnecting(EVENT_CONF_REQUEST);

    // Stop IVR
    tempObj->StopIVRUponDisconnecting(EVENT_CONF_REQUEST);
    POBJDELETE(tempObj);
  }

  // Reset Active Events Counter
  m_confAppInfo->SetConfActiveEventsCounter(0);
  m_numOfFeatures = 0;

  // Remove all parties from Deleted Parties List and delete the related Roll Call recording files
  m_participants->DeleteAllRollCallFiles();
}

//--------------------------------------------------------------------------
void CFeatureObjectList::DoActionForParty(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam)
{
  switch (opcode)
  {
    case eCAM_EVENT_PARTY_SHOW_SLIDE:
    case eCAM_EVENT_PARTY_JOIN_CONF_VIDEO:
    case eCAM_EVENT_PARTY_END_VIDEO_IVR:
    case eCAM_EVENT_PARTY_START_IVR:
    case eCAM_EVENT_PARTY_SEND_DTMF:
    case eCAM_EVENT_PARTY_CLIENT_SEND_DTMF: // for Call Generator
    {
      if ((eCAM_EVENT_PARTY_CLIENT_SEND_DTMF == opcode) &&
    		  (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) &&
    		  (CProcessBase::GetProcess()->GetProductType() != eProductTypeCallGeneratorSoftMCU))
      {
        TRACEINTO << "CFeatureObjectList::DoActionForParty - Failed, The system is not a CallGenerator";
        return;
      }

      CFeatureObject* tempObj = new CFeatureObject(m_confAppInfo, m_participants);
      tempObj->SetPartyRscrID(partyRsrcID);
      tempObj->DoPartyAction(opcode, partyRsrcID, action, pParam);
      POBJDELETE(tempObj);
      break;
    }
  } // switch
}

//--------------------------------------------------------------------------
BOOL CFeatureObjectList::IsPartyInIVR(DWORD partyRsrcID, DWORD audioOrVideo)
{
  int partyIndex = m_participants->FindPartyIndex(partyRsrcID);
  if (partyIndex < 0)
    return FALSE;

  CConfAppPartyParams* party = m_participants->m_partyList[partyIndex];
  if (CAM_AUDIO == audioOrVideo)
  {
    if ((party->GetPartyAudioState() == eAPP_PARTY_STATE_IVR_ENTRY) || (party->GetPartyAudioState() == eAPP_PARTY_STATE_IVR_FEATURE))
      return TRUE;
  }
  else
  {
    if ((party->GetPartyVideoState() == eAPP_PARTY_STATE_IVR_ENTRY) || (party->GetPartyVideoState() == eAPP_PARTY_STATE_IVR_FEATURE))
      return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------------------
void CFeatureObjectList::RemoveAllFeaturesUponConfTerminating()
{
  for (int ind = 0; ind < MAX_SUB_FEATURES; ind++)
    POBJDELETE(m_list[ind]);

  m_numOfFeatures = 0;
}

//--------------------------------------------------------------------------
int CFeatureObjectList::ReplaceTokenWithSequenceNum(DWORD seqNumToken, DWORD sequenceNum)
{
  for (int ind = 0; ind < MAX_SUB_FEATURES; ind++)
    if (m_list[ind])
    {
      int res = m_list[ind]->ReplaceTokenWithSeqNumIfNeeded(seqNumToken, sequenceNum);
      if (1 == res)
        return 1; // found, no need to search
    }

  return 0;
}

//--------------------------------------------------------------------------
int CFeatureObjectList::HandlePlayMsgAckInd(DWORD ack_seq_num)
{
  for (int ind = 0; ind < MAX_SUB_FEATURES; ind++)
  {
    if (m_list[ind])
    {
      // In these cases we ignore the ACK_IND
      if ((m_list[ind]->GetOpcode() == eCAM_EVENT_PARTY_IVR_ENTRANCE) ||
          (m_list[ind]->GetOpcode() == eCAM_EVENT_CONF_WAIT_FOR_CHAIR) ||
          (m_list[ind]->GetOpcode() == eCAM_EVENT_PARTY_CHANGE_TO_LEADER) ||
          (m_list[ind]->GetOpcode() == eCAM_EVENT_PARTY_CHANGE_PW) ||
          (m_list[ind]->GetOpcode() == eCAM_EVENT_PARTY_PLAY_RINGING_TONE) ||
          (m_list[ind]->GetOpcode() == eCAM_EVENT_INVITE_PARTY) ||
          (m_list[ind]->GetOpcode() == eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER) ||
          (m_list[ind]->GetOpcode() == eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL))
        return 0;

      if (m_list[ind]->GetIvrMessageSessionIDCam() == ack_seq_num)
      {
        m_list[ind]->OnAckEndIvrMsg();
        return 1;                               // found
      }
    }
  }

  DumpFeatures( "HandlePlayMsgAckInd" );
  return 0;                                     // not found
}

//--------------------------------------------------------------------------
void CFeatureObjectList::HandleChangeIC()
{
  if (NULL == m_list[0])	// BRIDGE-11667
	  return;
  DWORD opcode = m_list[0]->GetOpcode();

  switch (opcode)
  {
    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:        // end wait for chair for conf
    {
      m_list[0]->RestartConfWaitForChair();
      break;
    }

    case eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW:  // stop Roll Call review by DTMF - requires stop play message
    case eCAM_EVENT_CONF_CHAIR_DROPPED:         // off
    case eCAM_EVENT_CONF_MUTE_ALL:
    case eCAM_EVENT_CONF_UNMUTE_ALL:
    case eCAM_EVENT_CONF_ALERT_TONE:
    case eCAM_EVENT_CONF_EXIT_TONE:
    case eCAM_EVENT_CONF_ENTRY_TONE:
    case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:      // stop Roll Call review upon ack/timer/replace by other feature
    case eCAM_EVENT_CONF_NOISY_LINE_DETECTION:
    case eCAM_EVENT_CONF_SECURE:
    case eCAM_EVENT_CONF_UNSECURE:
    case eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK:
    {
      m_list[0]->StartIVR(EVENT_CONF_REQUEST, START_IVR_CONF_SIMPLE_GAINS);

      BOOL stopPlayMsg = FALSE;
      m_list[0]->EndFeatureAndStopPlayMsgUponRequest(stopPlayMsg);
    }

    default:
    return;
  } // switch
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void CFeatureObjectList::OnTimerChairDropped(CSegment* pParam)
{
  // checks if there is leader in conference
  if (!m_confAppInfo->GetIsLeaderInConfNow())
  {
    TRACEINTO << "CFeatureObjectList::OnTimerChairDropped - Terminate the conference";
    CSegment* seg = new CSegment;
    m_confAppInfo->m_pConfApi->SendMsg(seg, CHAIR_DROPPED_TERMINATE);
    m_confAppInfo->SetConfState(eAPP_CONF_STATE_TERMINATING);
  }
  else
  {
    TRACEINTO << "CFeatureObjectList::OnTimerChairDropped - Do not terminate the conference (chair exist)";
    m_confAppInfo->m_pConfApi->NotifyCAMTimer(0, eCAM_EVENT_TIMER_END_FEATURE, CAM_CONF_ACTION, 0, eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED);
  }
}

//--------------------------------------------------------------------------
void CFeatureObjectList::DumpFeatures(const char* functionName)
{
	std::ostringstream msg;
	msg << "for " << functionName << " - F#:" << m_numOfFeatures << " ==> \n";

	for (int i = 0; i < m_numOfFeatures; ++i)
	{
		DWORD featureOpcode = m_list[i]->GetOpcode();

		msg << "\t(" << i << ")";

		if (m_list[i])
			msg << m_confAppInfo->GetStringFromOpcode((TConfAppEvents)featureOpcode)
				<< " (#" << featureOpcode << ")" << ", PartyId:" << m_list[i]->GetPartyRscrID();
		else
			msg << "NULL;";

		msg << "\n";
	}

	TRACEINTO << msg.str();
}

//--------------------------------------------------------------------------
#define TIMER_PLAY_MSG         121
#define TIMER_REQUEST_TO_SPEAK 123
#define  REQUEST_TO_SPEAK_TOUT 30*SECOND

PBEGIN_MESSAGE_MAP(CFeatureObject)
  ONEVENT(TIMER_PLAY_MSG        , CONNECT, CFeatureObject::OnTimerEndIvrMsg)
  ONEVENT(TIMER_REQUEST_TO_SPEAK, CONNECT, CFeatureObject::OnTimerRequestToSpeak)
PEND_MESSAGE_MAP(CFeatureObject, CStateMachine);


////////////////////////////////////////////////////////////////////////////
//                        CFeatureObject
////////////////////////////////////////////////////////////////////////////
CFeatureObject::CFeatureObject(CConfAppInfo* confAppInfo, CConfAppPartiesList* participants)
{
  m_state                  = CONNECT;
  m_confAppInfo            = confAppInfo;
  m_participants           = participants;
  m_featureType            = F_APP_FEATURES_LAST;
  m_partyRsrcID            = 0;
  m_eventUniqueNumber      = 0;
  m_waitListOriginated     = 0;
  m_opcode                 = 0;
  m_stopUponFirstDTMF      = 0; // stop play message upon first DTMF
  m_endFeatureUponDTMF     = 0; // close feature upon first DTMF
  m_IvrMessageSessionID    = 0;
  m_IvrMessageSessionIDCam = 0;
}

//--------------------------------------------------------------------------
CFeatureObject::~CFeatureObject()
{
}

//--------------------------------------------------------------------------
void* CFeatureObject::GetMessageMap()
{
  return NULL;
}

//--------------------------------------------------------------------------
void CFeatureObject::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
}

//--------------------------------------------------------------------------
DWORD CFeatureObject::GetNextIvrMessageSessionID()
{
  IvrMessageSessionIDCam++;
  return IvrMessageSessionIDCam;
}

//--------------------------------------------------------------------------
WORD CFeatureObject::GetIsLeaderParty()
{
  return m_participants->GetIsPartyLeader(m_partyRsrcID);
}

//--------------------------------------------------------------------------
WORD CFeatureObject::GetNumberOfMediaForRollCallReview(DWORD* fileIDArray)
{
  return m_participants->GetNumOfInMixParticipants(fileIDArray);
}

//--------------------------------------------------------------------------
void CFeatureObject::OnTimerEndIvrMsg(CSegment* pParam)
{
  PTRACE(eLevelError, "CFeatureObject::OnTimerEndIvrMsg");

  OnEndIvrMsg();
}

//--------------------------------------------------------------------------
void CFeatureObject::OnEndIvrMsg()
{
  // send message to CConfAppMngr: End of IVR
  DWORD confOrParty;
  if (m_featureType == F_APP_CONF_IVR)
    confOrParty = CAM_CONF_ACTION;
  else
    confOrParty = CAM_PARTY_ACTION;

  //AT&T
  //CTaskApp* taskApp = m_confAppInfo->m_pConfApi->GetTaskAppPtr();
  //taskApp->
  ///////////
  m_confAppInfo->m_pConfApi->NotifyCAMTimer(m_partyRsrcID, eCAM_EVENT_TIMER_END_FEATURE, confOrParty, m_eventUniqueNumber, m_opcode);
}

//--------------------------------------------------------------------------
void CFeatureObject::CreateNewConfFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID)
{
  // conf or party type of this object
  m_featureType       = F_APP_CONF_IVR;
  m_eventUniqueNumber = eventUniqueNumber;
  m_partyRsrcID       = partyRsrcID;
  m_opcode            = opcode;

  switch (opcode)
  {
    case eCAM_EVENT_CONF_CHAIR_DROPPED:
    case eCAM_EVENT_CONF_MUTE_ALL:
    case eCAM_EVENT_CONF_UNMUTE_ALL:
    case eCAM_EVENT_CONF_ALERT_TONE:
    case eCAM_EVENT_CONF_EXIT_TONE:
    case eCAM_EVENT_CONF_ENTRY_TONE:
    case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:
    case eCAM_EVENT_CONF_NOISY_LINE_DETECTION:
    case eCAM_EVENT_CONF_RECORDING_FAILED:
    case eCAM_EVENT_CONF_SECURE:
    case eCAM_EVENT_CONF_UNSECURE:
    case eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK:
    {
      // update "confinfo" (increment num of events currently active)
      m_confAppInfo->IncConfActiveEventsCounter();

      if (eCAM_EVENT_CONF_EXIT_TONE == opcode)
      {
        m_confAppInfo->IncConfActiveExitToneCounter();
      }

      break;
    }

    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:
    {
      // update "confinfo" (increment num of events currently active)
      m_confAppInfo->IncConfActiveEventsCounter();
      break;
    }

    case eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED:
    {
      TRACEINTO << "CFeatureObject::CreateNewConfFeature - Start 2 minutes timer before terminating conference upon 'chair dropped'";
      // the time should be taken from the system.cfg
      StartTimer(TIMER_CHAIR_DROPPED, 120*SECOND);    // error in play message
      break;
    }

    default:
    {
      TRACEINTO << "CFeatureObject::CreateNewConfFeature - Failed, opcode not valid, Opcode:" << opcode;
      break;
    }
  } // switch
}

//--------------------------------------------------------------------------
void CFeatureObject::StartNewConfFeature()
{
  WORD isNeedStartIvr = 0;

  switch (m_opcode)
  {
    case eCAM_EVENT_CONF_CHAIR_DROPPED:
    case eCAM_EVENT_CONF_MUTE_ALL:
    case eCAM_EVENT_CONF_UNMUTE_ALL:
    case eCAM_EVENT_CONF_ALERT_TONE:
    case eCAM_EVENT_CONF_EXIT_TONE:
    case eCAM_EVENT_CONF_ENTRY_TONE:
    case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:
    case eCAM_EVENT_CONF_NOISY_LINE_DETECTION:
    case eCAM_EVENT_CONF_RECORDING_FAILED:
    case eCAM_EVENT_CONF_SECURE:
    case eCAM_EVENT_CONF_UNSECURE:
    case eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK:
    {
      // checks if the conference is in the right IVR mode. If not, move it to the relevant IVR mode
      if ((eCAM_EVENT_CONF_EXIT_TONE == m_opcode) || (eCAM_EVENT_CONF_ENTRY_TONE == m_opcode))
      {
        if (m_confAppInfo->GetConfStartIVRMode() != START_IVR_CONF_LOWER_GAINS)
          StartIVR(EVENT_CONF_REQUEST, START_IVR_CONF_LOWER_GAINS);             // move conference to IVR mode (Start-IVR for conference)

      }
      else if (m_confAppInfo->GetConfStartIVRMode() != START_IVR_CONF_SIMPLE_GAINS)
        StartIVR(EVENT_CONF_REQUEST, START_IVR_CONF_SIMPLE_GAINS);              // move conference to IVR mode (Start-IVR for conference)

      // play message to conference
      WORD duration = PlayMessage(EVENT_CONF_REQUEST, isNeedStartIvr, m_opcode);
      if (duration > 0)
      {
        if (duration > MAX_MESSAGE_DURATION_IN_SECONDS)
          duration = MAX_MESSAGE_DURATION_IN_SECONDS;

        StartTimer(TIMER_PLAY_MSG, (duration+3)*SECOND);
      }
      else
        StartTimer(TIMER_PLAY_MSG, 10);                                         // error in play message

      break;
    }

    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:
    {
      // checks if the conference in IVR mode. If not, move it to IVR mode
      if (m_confAppInfo->GetConfStartIVRMode() != START_IVR_CONF_SIMPLE_GAINS)  // move conference to IVR mode (Start-IVR for conference)
        StartIVR(EVENT_CONF_REQUEST, START_IVR_CONF_SIMPLE_GAINS);

      // play message to conference
      WORD duration = PlayMessage(EVENT_CONF_REQUEST, isNeedStartIvr, m_opcode);
      // play music to conference
      StartMusicToConfOrParty(EVENT_CONF_REQUEST);
      break;
    }

    case eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED:
      // nothing to do
      break;

    default:
    {
      TRACEINTO << "CFeatureObject::StartNewConfFeature - Failed, opcode not valid, Opcode:" << m_opcode;
      StartTimer(TIMER_PLAY_MSG, 10);
      break;
    }
  } // switch
}

//--------------------------------------------------------------------------
void CFeatureObject::StartNewPartyFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID)
{
  // conference or party type of this object
  m_featureType       = F_APP_PARTY_IVR;
  m_partyRsrcID       = partyRsrcID;
  m_opcode            = opcode;
  m_eventUniqueNumber = eventUniqueNumber;
  WORD isNeedStartIvr = 0;

  if (0 != m_eventUniqueNumber) // in case the feature originated from event in the wait list
  {
    m_waitListOriginated = 1;
  }

  switch (opcode)
  {
    case eCAM_EVENT_PARTY_WAIT_FOR_CHAIR:
    {
      // the video should remain Slide, the audio should move to Mix
      MovePartyAudioToMix();
      return;
    }

    case eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC:
    {
      if (m_confAppInfo->IsOperatorConf())
      {
        TRACEINTO << "CFeatureObject::StartNewPartyFeature - eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC not activated in operator conference";
        return;
      }

      StartIVRForPartyIfNeeded(START_IVR_PARTY_MUSIC_GAINS);
      StartMusicToConfOrParty(EVENT_PARTY_REQUEST);  // Start Music to Party
      return;
    }

    case eCAM_EVENT_PARTY_FIRST_TO_JOIN:
    case eCAM_EVENT_PARTY_PLAY_MENU:
    case eCAM_EVENT_PARTY_MUTE:
    case eCAM_EVENT_PARTY_UNMUTE:
    case eCAM_EVENT_PARTY_RECORDING_FAILED:
    case eCAM_EVENT_PARTY_RECORDING_IN_PROGRESS:
    {
      StartIVRForPartyIfNeeded(START_IVR_PARTY_MESSAGE_GAINS);
      int duration = PlayMessage(EVENT_PARTY_REQUEST, isNeedStartIvr, opcode);
      if (duration > 0)
      {
        if (duration > MAX_MESSAGE_DURATION_IN_SECONDS)
          duration = MAX_MESSAGE_DURATION_IN_SECONDS;

        TRACEINTO << "CFeatureObject::StartNewPartyFeature - Start timer for " << duration << " ms";
        StartTimer(TIMER_PLAY_MSG, (duration+2)*SECOND);
      }
      else
        StartTimer(TIMER_PLAY_MSG, 10);       // error in play message

      break;
    }

    case eCAM_EVENT_PARTY_PLAY_RINGING_TONE:
    {
      m_confAppInfo->SetSinglePartyNow(1);
      m_confAppInfo->SetIsRingToneOn(1);
      m_confAppInfo->SetSinglePartyRsrcId(m_partyRsrcID);
      StartIVRForPartyIfNeeded(START_IVR_PARTY_MESSAGE_GAINS);
      PlayMessage(EVENT_PARTY_REQUEST, isNeedStartIvr, opcode);
      break;
    }

    case eCAM_EVENT_PARTY_PLC:
    {
      StartStopPlcCommand(VIDEO_GRAPHIC_OVERLAY_START_REQ);
      return;                                 // nothing more to do for now
    }

    case eCAM_EVENT_PARTY_VENUS:
    {
      return;                                 // nothing more to do for now
    }

    case eCAM_EVENT_PARTY_CHANGE_PW:          // handle in IVR-Control
    case eCAM_EVENT_PARTY_SILENCE_IT:         // handle in IVR-Control
    case eCAM_EVENT_PARTY_CHANGE_TO_LEADER:   // request from user via DTMF
    case eCAM_EVENT_INVITE_PARTY:
    case eCAM_EVENT_PARTY_PLAY_BUSY_MSG:
    case eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG:
    case eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG:
    case eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER:
    {
      StartIVRForPartyIfNeeded(START_IVR_PARTY_MESSAGE_GAINS,(opcode==eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER));
      break;
    }

    case eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE:
    {
      StartIVRForPartyIfNeeded(START_IVR_PARTY_MESSAGE_GAINS);

      char msgFullPath[MAX_FULL_PATHNAME];
      WORD msgDuration = 0;
      WORD msgCheckSum = 0;

      const char* ivrServiceName = m_confAppInfo->GetIvrServiceName();
      if (NULL != ivrServiceName)
        ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, IVR_FEATURE_OPER_ASSISTANCE, IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE, msgFullPath, &msgDuration, &msgCheckSum);

      if (msgDuration > 0)
      {
        if (msgDuration > MAX_MESSAGE_DURATION_IN_SECONDS)
          msgDuration = MAX_MESSAGE_DURATION_IN_SECONDS;
        msgDuration += 2;
      }
      else
      {
        // default operator assistance message duration is 5 + 2
        msgDuration = 7;
      }
      TRACEINTO << "CFeatureObject::StartNewPartyFeature - Opcode:eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE, Duration:" << msgDuration;
      StartTimer(TIMER_PLAY_MSG, msgDuration*SECOND);   // error in play message
      break;
    }

    case eIVR_REQUEST_TO_SPEAK:
    {
      BYTE onOff = 1;

      // If there is a timer of the previous request to speak, run it over.
      if (IsValidTimer(TIMER_REQUEST_TO_SPEAK))
        DeleteTimer(TIMER_REQUEST_TO_SPEAK);

      TRACEINTO << "CFeatureObject::StartNewPartyFeature - Opcode:eIVR_REQUEST_TO_SPEAK, Duration:" << REQUEST_TO_SPEAK_TOUT/100;
      StartTimer(TIMER_REQUEST_TO_SPEAK, REQUEST_TO_SPEAK_TOUT);

      int partyIndex = m_participants->FindPartyIndex(m_partyRsrcID);
      PASSERT_AND_RETURN(-1 == partyIndex);

      string    partyName = m_participants->m_partyList[partyIndex]->GetPartyName();
      CSegment* pSeg      = new CSegment;
      *pSeg << (char*)partyName.c_str() << onOff;
      m_confAppInfo->m_pConfApi->UpdateDB(NULL, PARTY_REQUEST_TO_SPEAK, (DWORD) 0, 1, pSeg);
      break;
    }
    case eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL:
    {
      return;                                 // nothing more to do for now
    }
  }
}

//--------------------------------------------------------------------------
int CFeatureObject::DoPartyAction(DWORD opcode, DWORD partyRsrcID, DWORD action, CSegment* pParam)
{

  switch (opcode)
  {
    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_PLC:
    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_PLC:
    case eCAM_EVENT_PARTY_FORCE_PLC:
    case eCAM_EVENT_PARTY_CANCEL_PLC:
      DoPlcAction(opcode, action, pParam);
      break;

    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_VENUS:
    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_VENUS:
    case eCAM_EVENT_PARTY_FORCE_VENUS:
    case eCAM_EVENT_PARTY_CANCEL_VENUS:
      DoVenusAction(opcode, action, pParam);
      break;

    case eCAM_EVENT_PARTY_START_IVR:
      StartIVR(EVENT_PARTY_REQUEST, START_IVR_PARTY_MESSAGE_GAINS);     // start IVR mode
      break;

    case eCAM_EVENT_PARTY_SHOW_SLIDE:
      ShowSlide(pParam);
      break;

    case eCAM_EVENT_PARTY_JOIN_CONF_VIDEO:
      JoinConfVideo();
      break;

    case eCAM_EVENT_PARTY_END_VIDEO_IVR:
      MovePartyVideoToMix();
      break;

    case eCAM_EVENT_PARTY_SEND_DTMF:
      SendDtmf(pParam);
      break;

    case eCAM_EVENT_PARTY_CLIENT_SEND_DTMF:                           // for Call Generator
      if ((CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) &&
          (CProcessBase::GetProcess()->GetProductType() != eProductTypeCallGeneratorSoftMCU))
      {
        TRACEINTO << "CFeatureObjectList::SendDtmfFromClient - Failed, The system is not a CallGenerator";
        return 0;
      }
      SendClientDtmf(pParam);
      break;
  }

  return 0;
}

//--------------------------------------------------------------------------
void CFeatureObject::ShowSlide(CSegment* pParam)
{
  CVideoBridgeInterface* pVidBrdgInterface = m_confAppInfo->m_pVideoBridgeInterface;
  PASSERT_AND_RETURN(!pVidBrdgInterface);

  // getting the party ID
  CIVRPlayMessage*       pIVRPlayMessage = new CIVRPlayMessage;
  SIVRPlayMessageStruct* pPlayMsg        = &(pIVRPlayMessage->play);
  memset((char*)pPlayMsg, 0, sizeof(SIVRPlayMessageStruct));

  std::string slideFileNameStr = "";
  if (pParam != NULL)
	  *pParam >> slideFileNameStr;
  // fill slide parameters
  TRACEINTO << " got slide name of length " << (int)slideFileNameStr.length();
  InitVideoSlideParams(pPlayMsg, (slideFileNameStr.length() > 0)? slideFileNameStr.c_str() : NULL);

  // serialize to segment
  CSegment*              seg = new CSegment;
  pIVRPlayMessage->Serialize(seg);

  // send ShowSlide command
  if (!m_participants->IsPartyVideoStateConnected(m_partyRsrcID))
  {
    TRACEINTO << "CFeatureObject::ShowSlide - Party's video is NOT connected, sending the command to video bridge is aborted, PartyId:" << m_partyRsrcID;
  }
  else
  {
    TRACEINTO << "CFeatureObject::ShowSlide - PartyId:" << m_partyRsrcID;
    pVidBrdgInterface->IvrPartyCommand(m_partyRsrcID, IVR_SHOW_SLIDE_REQ, seg);
    m_participants->SetSlideIsOn(m_partyRsrcID, true);
  }
  POBJDELETE(seg);
  PDELETE(pIVRPlayMessage);
}

//--------------------------------------------------------------------------
void CFeatureObject::InitVideoSlideParams(SIVRPlayMessageStruct* pPlayMsg, const char *slideName)
{
  // fills general parameters
  pPlayMsg->partyOrconfFlag  = IVR_PLAY_MSG_TO_PARTY;
  pPlayMsg->stopPrevOrAppend = IVR_STOP_PREV_MSG;
  pPlayMsg->mediaType        = IVR_MEDIA_TYPE_VIDEO;
  pPlayMsg->numOfRepetition  = 0;
  pPlayMsg->startIVRFlag     = 0;
  pPlayMsg->videoBitRate     = 0;
  pPlayMsg->isTipMode        = FALSE;
  pPlayMsg->numOfMediaFiles  = 1;
  pPlayMsg->mediaFiles       = new SIVRMediaFileParamsStruct[1];
  memset((BYTE*)pPlayMsg->mediaFiles, 0, sizeof(SIVRMediaFileParamsStruct));

  // fills 1 media lile (slide) parameters
  pPlayMsg->mediaFiles[0].actionType   = IVR_ACTION_TYPE_PLAY;
  pPlayMsg->mediaFiles[0].checksum     = 0;
  pPlayMsg->mediaFiles[0].verNum       = 1;
  pPlayMsg->mediaFiles[0].frequentness = m_confAppInfo->GetCachePriority(0, IVR_MEDIA_TYPE_VIDEO);

  // fills slide name (basic)
  if (slideName== NULL)
	  slideName = GetSlideName();
  TRACECOND_AND_RETURN(!slideName, "CFeatureObject::InitVideoSlideParams - Failed, SlideName is NULL");

  pPlayMsg->mediaFiles[0].fileNameLength = strlen(slideName);
  SAFE_COPY(pPlayMsg->mediaFiles[0].fileName, slideName);
}

//--------------------------------------------------------------------------
const char* CFeatureObject::GetSlideName()
{
  CAVmsgService* pAvService = GetAVmsgService();
  if (!pAvService)
    return NULL;

  return pAvService->GetSlideName();
}

//--------------------------------------------------------------------------
CAVmsgService* CFeatureObject::GetAVmsgService()
{
  // get IVR Service name
  const char* ivrServiceName = m_confAppInfo->GetIvrServiceName();
  TRACECOND_AND_RETURN_VALUE(!ivrServiceName, "CFeatureObject::GetAVmsgService - Failed, IVR Service Name is NULL", NULL);

  CAVmsgService* pAvService = ::GetpAVmsgServList()->GetCurrentAVmsgService(ivrServiceName);
  TRACECOND_AND_RETURN_VALUE(!pAvService, "CFeatureObject::GetAVmsgService - Failed, IVR Service is NULL", NULL);

  return pAvService;
}

//--------------------------------------------------------------------------
int CFeatureObject::DoPlcAction(DWORD opcode, DWORD action, CSegment* pParam)
{
  TRACEINTO << "CFeatureObject::DoPlcAction "
              << "- Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
              << ", PartyId:" << m_partyRsrcID
              << ", Action:"  << action;

  // checks if in PLC Feature
  if (m_opcode != eCAM_EVENT_PARTY_PLC)
  {
    TRACEINTO << "CFeatureObject::DoPlcAction - Failed, not in PLC feature"
              << "- Opcode:" << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) m_opcode) << " (#" << m_opcode << ")";
    return 0;
  }

  // checks if Video state is okay
  int partyVideoState = m_participants->GetPartyVideoState(m_partyRsrcID);
  if (partyVideoState != eAPP_PARTY_STATE_MIX)
  {
    TRACEINTO << "CFeatureObject::DoPlcAction - Failed, not in Mix state"
              << "- partyVideoState:" << partyVideoState;
    return 0;
  }

  PASSERT_AND_RETURN_VALUE(!m_confAppInfo->m_pVideoBridgeInterface, 0);

  switch (opcode)
  {
    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_PLC:
      // action == the new layout type
      m_confAppInfo->m_pVideoBridgeInterface->PLC_SetPartyPrivateLayout(m_partyRsrcID, (LayoutType)action);
      break;

    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_PLC:
      // action == change to conference or party layout, CAM_PARTY_LAYOUT: Private, CAM_CONF_LAYOUT: Conf
      if (CAM_CONF_LAYOUT == action)
        m_confAppInfo->m_pVideoBridgeInterface->PLC_PartyReturnToConfLayout(m_partyRsrcID);
      break;

    case eCAM_EVENT_PARTY_FORCE_PLC:
    {
      switch (action)
      {
        case 0: // Cancel ALL private forces from all layouts
        {
          TRACEINTO << "CFeatureObject::DoPlcAction - Cancel all Forces, PartyId:" << m_partyRsrcID;
          m_confAppInfo->m_pVideoBridgeInterface->PLC_CancelAllPrivateLayoutForces(m_partyRsrcID);
          break;
        }

        case 1: // Force next party
        {
          DWORD partyeIdToForce = m_participants->GetPartyToForce(m_partyRsrcID);
          TRACECOND_AND_RETURN_VALUE(0 == partyeIdToForce, "CFeatureObject::DoPlcAction - Failed, party not found, PartyId:" << partyeIdToForce, 0);

          BYTE cellNum = 0;
          if (pParam)
            *pParam >> cellNum;

          TRACEINTO << "CFeatureObject::DoPlcAction - Force next party, PartyIdToForce:" << partyeIdToForce << ", CellToForce:" << cellNum;
          m_confAppInfo->m_pVideoBridgeInterface->PLC_ForceToCell(m_partyRsrcID, partyeIdToForce, cellNum);
          break;
        }

        default:
        {
          TRACEINTO << "CFeatureObject::DoPlcAction - Failed, Illegal action, Action:" << action;
          break;
        }
      }
      break;
    }

    case eCAM_EVENT_PARTY_CANCEL_PLC:
    {
      if (1 == action)      // need to do STOP_PLC
        StartStopPlcCommand(VIDEO_GRAPHIC_OVERLAY_STOP_REQ);

      const CConfAppPartyParams* party = m_participants->GetParty(m_partyRsrcID);
      TRACECOND_AND_RETURN_VALUE(NULL == party, "CFeatureObject::DoPlcAction - Failed, party not found, PartyId:" << m_partyRsrcID, 0);

      // update Party on Cancel PLC
      (party->m_pPartyApi)->UpdateIVRGeneralOpcode(REJECT_PLC);

      // changing the current opcode to cancel
      m_opcode = eCAM_EVENT_PARTY_CANCEL_PLC;
      break;
    }

    default:
      TRACEINTO << "CFeatureObject::DoPlcAction - Failed, Invalid opcode, Opcode:" << opcode;
      break;
  }
  return 0;
}

//--------------------------------------------------------------------------
void CFeatureObject::StartStopPlcCommand(DWORD opcode)
{
  TRACEINTO << "CFeatureObject::StartStopPlcCommand "
              << "- Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
              << ", PartyId:" << m_partyRsrcID;

  // get party state
  int partyState = m_participants->GetPartyVideoState(m_partyRsrcID);
  TRACECOND_AND_RETURN(eAPP_PARTY_STATE_MIX != partyState, "CFeatureObject::StartStopPlcCommand - Failed, Illegal partyState, PartyState:" << partyState << ", PartyId:" << m_partyRsrcID);

  CVideoBridgeInterface* pVidBrdgInterface = m_confAppInfo->m_pVideoBridgeInterface;
  PASSERT_AND_RETURN(!pVidBrdgInterface);

  CSegment* seg = new CSegment;
  *seg << (DWORD)0; // for future, screen type
  if (!m_participants->IsPartyVideoStateConnected(m_partyRsrcID))
  {
    TRACEINTO << "CFeatureObject::StartStopPlcCommand - Party's video is NOT connected, sending the command to video bridge is aborted, PartyId:" << m_partyRsrcID;
  }
  else
  {
    pVidBrdgInterface->IvrPartyCommand(m_partyRsrcID, opcode, seg);
  }
  POBJDELETE(seg);
}

//--------------------------------------------------------------------------
int CFeatureObject::DoVenusAction(DWORD opcode, DWORD action, CSegment* pParam)
{
  TRACEINTO << "CFeatureObject::DoVenusAction "
              << "- Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
              << ", PartyId:" << m_partyRsrcID
              << ", Action:"  << action;

  // checks if in PLC Feature
  TRACECOND_AND_RETURN_VALUE(m_opcode != eCAM_EVENT_PARTY_VENUS, "CFeatureObject::DoVenusAction - Failed, not in Venus feature", 0);

  // checks if Video state is okay
  int partyVideoState = m_participants->GetPartyVideoState(m_partyRsrcID);
  TRACECOND_AND_RETURN_VALUE(partyVideoState != eAPP_PARTY_STATE_MIX, "CFeatureObject::DoVenusAction - Failed, not in Mix state - PartyState:" << partyVideoState, 0);

  PASSERT_AND_RETURN_VALUE(!m_confAppInfo->m_pVideoBridgeInterface, 0);

  switch (opcode)
  {
    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_VENUS:
      // action == the new layout type
      m_confAppInfo->m_pVideoBridgeInterface->PLC_SetPartyPrivateLayout(m_partyRsrcID, (LayoutType)action);
      break;

    case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_VENUS:
      // action == change to conf or party layout, CAM_PARTY_LAYOUT: Private, CAM_CONF_LAYOUT: Conf
      if (CAM_CONF_LAYOUT == action)
        m_confAppInfo->m_pVideoBridgeInterface->PLC_PartyReturnToConfLayout(m_partyRsrcID);
      break;

    case eCAM_EVENT_PARTY_FORCE_VENUS:
    {
      switch (action)
      {
        case 0: // Cancel ALL private forces from all layouts
        {
          TRACEINTO << "CFeatureObject::DoVenusAction - Cancel all Forces, PartyId:" << m_partyRsrcID;
          m_confAppInfo->m_pVideoBridgeInterface->PLC_CancelAllPrivateLayoutForces(m_partyRsrcID);
          break;
        }

        case 1: // Force next party
        {
          DWORD partyeIdToForce = m_participants->GetPartyToForce(m_partyRsrcID);
          TRACECOND_AND_RETURN_VALUE(0 == partyeIdToForce, "CFeatureObject::DoVenusAction - Failed, party not found, PartyId:" << partyeIdToForce, 0);

          BYTE cellNum = 0;
          if (pParam)
            *pParam >> cellNum;

          TRACEINTO << "CFeatureObject::DoVenusAction - Force next party, PartyIdToForce:" << partyeIdToForce << ", CellToForce:" << cellNum;
          m_confAppInfo->m_pVideoBridgeInterface->PLC_ForceToCell(m_partyRsrcID, partyeIdToForce, cellNum);
          break;
        }

        default:
        {
          TRACEINTO << "CFeatureObject::DoVenusAction - Failed, Illegal action, Action:" << action;
          break;
        }
      }
      break;
    }

    case eCAM_EVENT_PARTY_CANCEL_VENUS:
    {
      if (1 == action) // need to do STOP_Venus
        StartStopPlcCommand(VIDEO_GRAPHIC_OVERLAY_STOP_REQ);

      const CConfAppPartyParams* party = m_participants->GetParty(m_partyRsrcID);
      TRACECOND_AND_RETURN_VALUE(NULL == party, "CFeatureObject::DoVenusAction - Failed, party not found, PartyId:" << m_partyRsrcID, 0);

      // update Party on Cancel PLC
      (party->m_pPartyApi)->UpdateIVRGeneralOpcode(REJECT_VENUS);

      // changing the currect opcode to cancel
      m_opcode = eCAM_EVENT_PARTY_CANCEL_VENUS;
      break;
    }

    default:
      TRACEINTO << "CFeatureObject::DoVenusAction - Failed, Invalid opcode, Opcode:" << opcode;
      break;
  }
  return 0;
}

//--------------------------------------------------------------------------
void CFeatureObject::StartStopVenusCommand(DWORD opcode)
{
  TRACEINTO << "CFeatureObject::StartStopVenusCommand "
            << "- Opcode:"  << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", PartyId:" << m_partyRsrcID;

  // get party state
  int partyState = m_participants->GetPartyVideoState(m_partyRsrcID);
  TRACECOND_AND_RETURN(eAPP_PARTY_STATE_MIX != partyState, "CFeatureObject::StartStopVenusCommand - Failed, Illegal partyState, PartyState:" << partyState << ", PartyId:" << m_partyRsrcID);

  CVideoBridgeInterface* pVidBrdgInterface = m_confAppInfo->m_pVideoBridgeInterface;
  PASSERT_AND_RETURN(!pVidBrdgInterface);

  CSegment* seg = new CSegment;
  *seg << (DWORD)0; // for future, screen type
  if (!m_participants->IsPartyVideoStateConnected(m_partyRsrcID))
  {
    TRACEINTO << "CFeatureObject::StartStopVenusCommand - Party's video is NOT connected, sending the command to video bridge is aborted, PartyId:" << m_partyRsrcID;
  }
  else
  {
    pVidBrdgInterface->IvrPartyCommand(m_partyRsrcID, opcode, seg);
  }
  POBJDELETE(seg);
}

//--------------------------------------------------------------------------
int CFeatureObject::PlayMessage(DWORD confOrParty, WORD isNeedStartIvr, DWORD opcode)
{
  TRACEINTO << "CFeatureObject::PlayMessage "
            << "- Opcode:"     << m_confAppInfo->GetStringFromOpcode((TConfAppEvents) opcode) << " (#" << opcode << ")"
            << ", Conf/Party:" << confOrParty;

  CAudioBridgeInterface* pAudBrdgInterface = m_confAppInfo->m_pAudBrdgInterface;
  PASSERT_AND_RETURN_VALUE(!pAudBrdgInterface, 0);

  const char* ivrServiceName = m_confAppInfo->GetIvrServiceName();
  TRACECOND_AND_RETURN_VALUE(!ivrServiceName, "CFeatureObject::PlayMessage - Failed, Cannot retrieve IVR Service name", 0);

  char  msgFullPath[MAX_FULL_PATHNAME];
  DWORD fileIDArray[MAX_ROLL_CALL_PARTY_LIST]; // for roll-call review
  WORD  numOfRollCallFiles = 0;
  WORD  msgDuration        = 0;
  WORD  msgCheckSum        = 0;
  DWORD numOfRepetition    = IVR_PLAY_ONCE;
  WORD  mediaIndex         = 0;
  WORD  stopPrevOrAppend   = IVR_APPEND_MSG;
  WORD  feature_opcode     = IVR_FEATURE_GENERAL;
  WORD  event_opcode       = 0;
  int   status             = STATUS_FAIL;
  WORD  numOfMediaFiles    = 1;

  switch (opcode)
  {
    //
    // ------  conf messages  ---------
    //
    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:
    {
      event_opcode     = IVR_EVENT_REQUIRES_LEADER;
      stopPrevOrAppend = IVR_STOP_PREV_MSG;
      numOfRepetition  = IVR_REPEAT_FOREVER; // play loop
      numOfMediaFiles  = 2;
      break;
    }

    case eCAM_EVENT_CONF_ALERT_TONE:
    {
      event_opcode     = IVR_EVENT_END_TIME_ALERT;
      stopPrevOrAppend = IVR_STOP_PREV_MSG;
      break;
    }

    case eCAM_EVENT_CONF_CHAIR_DROPPED:
    {
      event_opcode     = IVR_EVENT_CHAIR_DROPPED;
      stopPrevOrAppend = IVR_STOP_PREV_MSG;
      break;
    }

    case eCAM_EVENT_CONF_MUTE_ALL:
    {
      event_opcode = IVR_EVENT_MUTE_ALL_ON;
      break;
    }

    case eCAM_EVENT_CONF_UNMUTE_ALL:
    {
      event_opcode = IVR_EVENT_MUTE_ALL_OFF;
      break;
    }

    case eCAM_EVENT_CONF_SECURE:
    {
      event_opcode     = IVR_EVENT_SECURE_ON;
      stopPrevOrAppend = IVR_STOP_PREV_MSG;
      break;
    }

    case eCAM_EVENT_CONF_UNSECURE:
    {
      event_opcode     = IVR_EVENT_SECURE_OFF;
      stopPrevOrAppend = IVR_STOP_PREV_MSG;
      break;
    }

    case eCAM_EVENT_CONF_ENTRY_TONE:
    case eCAM_EVENT_CONF_EXIT_TONE:
    {
      feature_opcode = IVR_FEATURE_ROLL_CALL;
      event_opcode   = IVR_EVENT_ROLLCALL_ENTER;
      if (opcode == eCAM_EVENT_CONF_EXIT_TONE)
        event_opcode = IVR_EVENT_ROLLCALL_EXIT;

      if (TRUE != m_confAppInfo->IsRollCallToneInsteadVoice())
      {
        numOfMediaFiles = 2;                              // use rollcall recording, not tone
        mediaIndex      = 1;
      }
      break;
    }

    case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:
    {
      feature_opcode     = IVR_FEATURE_ROLL_CALL;
      event_opcode       = IVR_EVENT_ROLLCALL_BEGIN_OF_NAME_REVIEW;
      numOfRollCallFiles = GetNumberOfMediaForRollCallReview(fileIDArray);
      numOfMediaFiles    = 1 + numOfRollCallFiles;  // 1 for the first IVR message + names
      break;
    }

    case eCAM_EVENT_CONF_NOISY_LINE_DETECTION:
    {
      feature_opcode  = IVR_FEATURE_MUTE_NOISY_LINE;
      event_opcode    = IVR_EVENT_PLAY_NOISY_LINE_MESSAGE;
      numOfMediaFiles = 2;
      mediaIndex      = 1;
      break;
    }

    case eCAM_EVENT_CONF_RECORDING_FAILED:
    {
      feature_opcode = IVR_FEATURE_GENERAL;
      event_opcode   = IVR_EVENT_RECORDING_FAILED;
      break;
    }

    //
    // ------  party messages ------
    //
    case eCAM_EVENT_PARTY_PLAY_MENU:
    {
      feature_opcode = IVR_FEATURE_GENERAL;
      event_opcode   = IVR_EVENT_MENU_SIMPLE;
      if (GetIsLeaderParty())
        event_opcode = IVR_EVENT_MENU_LEADER;

      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      break;
    }

    case eCAM_EVENT_PARTY_MUTE:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_SELF_MUTE;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      break;
    }


    case eCAM_EVENT_PARTY_UNMUTE:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_SELF_UNMUTE;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      break;
    }


    case eCAM_EVENT_PARTY_FIRST_TO_JOIN:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_FIRST_TO_JOIN;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      // Update Info about "first to join"
      m_confAppInfo->SetFirstPartyMessagePlayed(1);
      break;
    }

    case eCAM_EVENT_PARTY_PLAY_RINGING_TONE:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_PLAY_RINGING_TONE;
      numOfRepetition      = IVR_REPEAT_FOREVER;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      // Update Info about "first to join"
      m_confAppInfo->SetFirstPartyMessagePlayed(1);
      break;
    }

    case eCAM_EVENT_PARTY_PLAY_BUSY_MSG:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_PLAY_BUSY_MSG;
      numOfRepetition      = 1;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      // Update Info about "first to join"
      m_confAppInfo->SetFirstPartyMessagePlayed(1);
      break;
    }

    case eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_PLAY_NOANSWER_MSG;
      numOfRepetition      = 1;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      // Update Info about "first to join"
      m_confAppInfo->SetFirstPartyMessagePlayed(1);
      break;
    }

    case eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_PLAY_WRONG_NUMBER_MSG;
      numOfRepetition      = 1;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      // Update Info about "first to join"
      m_confAppInfo->SetFirstPartyMessagePlayed(1);
      break;
    }

    case eCAM_EVENT_PARTY_RECORDING_FAILED:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_RECORDING_FAILED;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      break;
    }

    case eCAM_EVENT_PARTY_RECORDING_IN_PROGRESS:
    {
      feature_opcode       = IVR_FEATURE_GENERAL;
      event_opcode         = IVR_EVENT_RECORDING_IN_PROGRESS;
      m_stopUponFirstDTMF  = 1;
      m_endFeatureUponDTMF = 1;
      break;
    }

    case eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE:
    {
      break;
    }

    case eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK:
    {
      feature_opcode   = IVR_FEATURE_GENERAL;
      event_opcode     = IVR_EVENT_BLIP_ON_CASCADE_LINK;
      stopPrevOrAppend = IVR_APPEND_MSG;
      break;
    }

    default:
    {
      TRACEINTO << "CFeatureObject::PlayMessage - Failed, Illegal opcode, Opcode:" << opcode;
      break;
    }
  }

  // gets file parameters
  status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName, feature_opcode, event_opcode, msgFullPath, &msgDuration, &msgCheckSum);

  // if IVR Message doesn't exist
  TRACECOND_AND_RETURN_VALUE(STATUS_OK != status, "CFeatureObject::PlayMessage - Failed, Cannot retrieve IVR message params, IVRServiceName:" << ivrServiceName, 0);

  // gets IVR message cache priority
  DWORD                  cachePriority   = m_confAppInfo->GetCachePriority(opcode, IVR_MEDIA_TYPE_AUDIO);
  CIVRPlayMessage*       pIVRPlayMessage = new CIVRPlayMessage;
  SIVRPlayMessageStruct* playMsg         = &(pIVRPlayMessage->play);

  // general parameters
  if (EVENT_PARTY_REQUEST == confOrParty)
    playMsg->partyOrconfFlag = IVR_PLAY_MSG_TO_PARTY;
  else
    playMsg->partyOrconfFlag = IVR_PLAY_MSG_TO_CONF;

  playMsg->stopPrevOrAppend = stopPrevOrAppend;
  playMsg->mediaType        = IVR_MEDIA_TYPE_AUDIO;
  playMsg->numOfRepetition  = numOfRepetition;

  playMsg->numOfMediaFiles  = numOfMediaFiles;
  // media files parameters
  WORD numFiles = playMsg->numOfMediaFiles;
  playMsg->mediaFiles = new SIVRMediaFileParamsStruct[numFiles];
  memset((BYTE*)(playMsg->mediaFiles), 0, numFiles*sizeof(SIVRMediaFileParamsStruct));

  playMsg->mediaFiles[mediaIndex].actionType   = IVR_ACTION_TYPE_PLAY;
  playMsg->mediaFiles[mediaIndex].duration     = msgDuration;
  playMsg->mediaFiles[mediaIndex].checksum     = msgCheckSum;
  playMsg->mediaFiles[mediaIndex].frequentness = cachePriority;

  if (NULL == msgFullPath)
    TRACEINTO << "CFeatureObject::PlayMessage - msgFullPath is NULL";
  else if (0 == strncmp(msgFullPath, "Cfg/", 4)) // Card manager needs the path starting from IVR/ (and not from Cfg/)
  {
    playMsg->mediaFiles[mediaIndex].fileNameLength = strlen(msgFullPath)-4;
    strncpy(playMsg->mediaFiles[mediaIndex].fileName, msgFullPath+4, sizeof(playMsg->mediaFiles[mediaIndex].fileName) - 1);
    playMsg->mediaFiles[mediaIndex].fileName[sizeof(playMsg->mediaFiles[mediaIndex].fileName) - 1] = '\0';
  }
  else
  {
    playMsg->mediaFiles[mediaIndex].fileNameLength = strlen(msgFullPath);
    if (0 != strlen(msgFullPath))
    {
      strncpy(playMsg->mediaFiles[mediaIndex].fileName, msgFullPath, sizeof(playMsg->mediaFiles[mediaIndex].fileName) - 1);
      playMsg->mediaFiles[mediaIndex].fileName[sizeof(playMsg->mediaFiles[mediaIndex].fileName) - 1] = '\0';
    }
    else
    {
      playMsg->mediaFiles[mediaIndex].fileName[0] = '\0';
      TRACEINTO <<"CFeatureObject::PlayMessage - msgFullPath is empty";
    }
  }

  // change some parameters according to the specific feature
  WORD actualNumOfRollCallFiles = 0;
  switch (opcode)
  {
    case eCAM_EVENT_CONF_ENTRY_TONE:
    case eCAM_EVENT_CONF_EXIT_TONE:
    case eCAM_EVENT_CONF_NOISY_LINE_DETECTION:
    {
      if (TRUE == m_confAppInfo->IsRollCallToneInsteadVoice())
        break;  // not adding the recording to the command, playing only the tone

      STATUS retStatus = UpdatePlayMessageWithOneRollCallFile(playMsg, opcode);
      if (retStatus != STATUS_OK)
      {
        TRACEINTO << "CFeatureObject::PlayMessage - Failed, Roll Call file does not exist";
        PDELETE(pIVRPlayMessage);
        return 0;
      }

      break;
    }

    case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:
    {
      actualNumOfRollCallFiles = UpdatePlayMessageForRollCallReview(playMsg, opcode, fileIDArray, numOfRollCallFiles);
      if (0 == actualNumOfRollCallFiles)
      {
        TRACEINTO << "CFeatureObject::PlayMessage - Failed, Actual number of Roll Call files is 0";
        PDELETE(pIVRPlayMessage);
        return 0;
      }

      if (numOfRollCallFiles != actualNumOfRollCallFiles)
        playMsg->numOfMediaFiles = 1 + actualNumOfRollCallFiles;  // 1 for the first IVR message + names

      break;
    }

    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:
    {
      UpdateSilenceEntry(playMsg, 1, 30);
      break;
    }
  } // switch

  // Calculate the total duration of the play message request
  int totalMsgDuration = CalculateTotalPlayMsgDuration(playMsg);

  // send PlayMessage command
  CSegment* seg = new CSegment;
  pIVRPlayMessage->Serialize(seg);

  if (confOrParty == EVENT_CONF_REQUEST)
  {
    m_IvrMessageSessionIDCam = GetNextIvrMessageSessionID();
    pAudBrdgInterface->IvrConfCommand(IVR_PLAY_MESSAGE_REQ, m_IvrMessageSessionIDCam /*seqNumToken*/, seg);
  }
  else
  {
    if (!m_participants->IsPartyAudioStateConnected(m_partyRsrcID))
    {
      TRACEINTO << "CFeatureObject::PlayMessage - Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
    }
    else
    {
      m_IvrMessageSessionIDCam = GetNextIvrMessageSessionID();
      pAudBrdgInterface->IvrPartyCommand(m_partyRsrcID, IVR_PLAY_MESSAGE_REQ, m_IvrMessageSessionIDCam /*seqNumToken*/, seg);
    }
  }

  POBJDELETE(seg);
  PDELETE(pIVRPlayMessage);
  return totalMsgDuration;
}

//--------------------------------------------------------------------------
int CFeatureObject::PlayPartyMessage(DWORD partyRsrcID, CSegment* pParam)
{
	TRACEINTO << "PartyId:" << partyRsrcID;

  // checking if the party is in IVR mode (must be)
  int partyState = m_participants->GetPartyAudioState(partyRsrcID);
  if ((partyState != eAPP_PARTY_STATE_IVR_ENTRY) && // IVR before entering to the conference
      (partyState != eAPP_PARTY_STATE_IVR_FEATURE)) // IVR after entering to the conference
  {
    TRACEINTO << "Failed, Party not in IVR mode, play party message discarded. IVR state: " << (int)partyState;
    return -1;
  }

	// stop upon first DTMF
	m_stopUponFirstDTMF = 1;

  CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage; AUTO_DELETE(pIVRPlayMessage);
  pIVRPlayMessage->DeSerialize(pParam);
  SIVRPlayMessageStruct* pPlayMsg = &(pIVRPlayMessage->play);

  // fill IVR Play Message command
  CSegment* seg = new CSegment;

	if (m_participants->IsTipMaster(partyRsrcID))
	{
		if (pIVRPlayMessage->play.numOfMediaFiles)
			TRACEINTO << "new media file 0 name: " << pIVRPlayMessage->play.mediaFiles->fileName;
		pIVRPlayMessage->Serialize(seg);
		m_confAppInfo->m_pConfApi->PartytoPartyCntlMsgFromMasterToSlave(m_partyRsrcID, eTipSlaveAux,PLAY_MESSAGE_AUX, seg);
	}
	else	// regular or TIP slave_aux (TIP slave video parties don't initiate or receive play messages)
	{
		pIVRPlayMessage->Serialize(seg);
		// sends the Play Message command to the AB module
		CAudioBridgeInterface* pAudBrdgInterface = m_confAppInfo->m_pAudBrdgInterface;
		if (pAudBrdgInterface)
		{
			if (!m_participants->IsPartyAudioStateConnected(partyRsrcID))
			{
			  TRACEINTO << "Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
			}
			else
			{
			  // get the temp PlayMessage SessionID
			  m_IvrMessageSessionIDCam = GetNextIvrMessageSessionID();
			  pAudBrdgInterface->IvrPartyCommand(partyRsrcID, IVR_PLAY_MESSAGE_REQ, m_IvrMessageSessionIDCam /*seqNumToken*/, seg);
			}
		}
		POBJDELETE(seg);
	}

  return 0;
}

//--------------------------------------------------------------------------
void CFeatureObject::RecordRollCall(DWORD partyRsrcID, CSegment* pParam)
{
  TRACEINTO << "CFeatureObject::RecordRollCall - PartyId:" << partyRsrcID;

  int partyIndex = m_participants->FindPartyIndex(partyRsrcID);
  TRACECOND_AND_RETURN(-1 == partyIndex, "CFeatureObject::RecordRollCall - Failed, Party not found, PartyId:" << partyRsrcID);

  CIVRPlayMessage* pIVRPlayMessage = new CIVRPlayMessage;
  pIVRPlayMessage->DeSerialize(pParam);

  SIVRPlayMessageStruct* pPlayMsg = &(pIVRPlayMessage->play);
  CConfAppPartyParams* party = m_participants->m_partyList[partyIndex];

  //BRIDGE-10268 - DTMF interupt should not send StopPlayMediaCommand during RollCall recording.
  // vngfe-7981 - prevent stop roll call recording, to protect MFA crash
  m_stopUponFirstDTMF = 0;

  // Set Roll Call name
  party->ComposeAndSetRollCallName();

  // Change the file name of the third "message" to the path+name of the RollCall Recording
  string partyRollCallName = IVR_FOLDER_ROLLCALL_EMB;
  partyRollCallName += party->GetPartyRollCallName();

  //IVR for TIP  - the recording is the first and the only message //(BRIDGE-6564) and should be activate only for the Master
  //(BRIDGE-6564) avoid AUX do the this "REC" code
  if (m_participants->IsTipParty(partyRsrcID))
  {
	  if ( m_participants->IsTipMaster(partyRsrcID) && (0 == strcmp(pPlayMsg->mediaFiles[0].fileName, "ROLL_CALL_TIP") ) )
	  {
		  TRACEINTO << "Change the file name of the recording message to the path+name of the RollCall Recording - in TIP";
		  SAFE_COPY(pPlayMsg->mediaFiles[0].fileName, partyRollCallName.c_str());
		  pPlayMsg->mediaFiles[0].fileNameLength = strlen(pPlayMsg->mediaFiles[0].fileName);
	  }
  }

  //regular ROLL_CALL, the recording it the 3 message ("please state your name", bip, recording, bip).
  else if ( 0 == strcmp(pPlayMsg->mediaFiles[2].fileName, "ROLL_CALL"))
  {
	  if( (int)pPlayMsg->numOfMediaFiles >= 3 )
	  {
		  SAFE_COPY(pPlayMsg->mediaFiles[2].fileName, partyRollCallName.c_str());
		  pPlayMsg->mediaFiles[2].fileNameLength = strlen(pPlayMsg->mediaFiles[2].fileName);
	  }
  }

  // checking if the party is in IVR mode (must be)
  int partyState = m_participants->GetPartyAudioState(partyRsrcID);
  if ((partyState != eAPP_PARTY_STATE_IVR_ENTRY))   // IVR before entering to the conf
  {
    TRACEINTO << "CFeatureObject::RecordRollCall - Failed, Party not in IVR mode (audio state is " << partyState << "), record RollCall discarded";
    PDELETE(pIVRPlayMessage);
    return;
  }

  // fill IVR Record RollCall command
  CSegment* seg = new CSegment;

  pIVRPlayMessage->Serialize(seg);
  CAudioBridgeInterface* pAudBrdgInterface = m_confAppInfo->m_pAudBrdgInterface;
  if (pAudBrdgInterface)  // sends the Record RollCall command to the AB module
  {
    if (!m_participants->IsPartyAudioStateConnected(partyRsrcID))
      TRACEINTO << "CFeatureObject::RecordRollCall - Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
    else
      pAudBrdgInterface->IvrPartyCommand(partyRsrcID, IVR_RECORD_ROLL_CALL_REQ, 0, seg);
  }

  POBJDELETE(seg);
  PDELETE(pIVRPlayMessage);
}
//--------------------------------------------------------------------------
void CFeatureObject::StopRollCallRecording(DWORD partyRsrcID, CSegment* pParam)
{
	TRACEINTO << "ROLL_CALL_STOP_RECORDING (04) partyRsrcID = " << partyRsrcID << " send to audio bridge";
	CAudioBridgeInterface* pAudBrdgInterface = m_confAppInfo->m_pAudBrdgInterface;
	if (pAudBrdgInterface)
	{
//	    if (!m_participants->IsPartyAudioStateConnected(partyRsrcID))
//      TRACEINTO << "Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
//  else
	      pAudBrdgInterface->IvrPartyCommand(partyRsrcID, IVR_STOP_RECORD_ROLL_CALL_REQ, 0, pParam);
	 }else{
		 TRACEINTO << " ROLL_CALL_STOP_RECORDING pAudBrdgInterface is NULL";
	 }

}
//--------------------------------------------------------------------------
void CFeatureObject::DtmfReceived(DWORD opcode, DWORD partyRsrcID, CSegment* pParam)
{
  TRACEINTO << "- Opcode:"  << opcode
            << ", PartyId:" << partyRsrcID
            << ", stopUponFirstDTMF:" << m_stopUponFirstDTMF;

  int partyIndex = m_participants->FindPartyIndex(partyRsrcID);
  TRACECOND_AND_RETURN(-1 == partyIndex, "CFeatureObject::DtmfReceived - Failed, Party not found, PartyId:" << partyRsrcID);

  // gets the party details
  CConfAppPartyParams* party = m_participants->m_partyList[partyIndex];

  party->SetDtmfIndSource(opcode);                                                                  // the first DTMF detection for that party

  if (party->isValidDtmfIndSource(opcode) == FALSE)
  {
    TRACEINTO << "CFeatureObject::DtmfReceived - Failed, Received DTMF from a different source";    // it is a legal situation, some endpoints do it
    return;
  }

  BYTE dtmf[MAX_DTMF_LEN_FROM_SRC];
  CSegment* pParamDtmf = new CSegment;

  switch (opcode)
  {
    case AUD_DTMF_IND_VAL:
    {
      pParam->Get(dtmf, 4);                                                                         // the Audio DSP always send 4 bytes
      char charDTMF = TranslateAudioDTMF(dtmf[0]);

      /* VNGR-23481: check if the DTMF code is invalid */
      if (charDTMF == -1)
      {
        POBJDELETE(pParamDtmf);
        TRACEINTO << "CFeatureObject::DtmfReceived - Failed, Invalid DTMF code";
        return;
      }

      *pParamDtmf << (DWORD)1;                                                                        // Length - for now only one DTMF at a time from the Audio DSP
      *pParamDtmf << (BYTE&)charDTMF;                                                                 // Fill DTMF params
      break;
    }

    case SIGNALLING_DTMF_INPUT_IND:                                                                   // from SIGNALLING (IP or SIP), can be number of DTMF
    case RTP_DTMF_INPUT_IND:                                                                          // from RTP (IP or SIP), can be number of DTMF
    {
      DWORD dtmfLength;
      *pParam >> dtmfLength;
      if ((dtmfLength == 0) || (dtmfLength > MAX_DTMF_LEN_FROM_SRC))
      {
        TRACEINTO << "CFeatureObject::DtmfReceived - Failed, H245 DTMF illegal length (0 or too long)";
		POBJDELETE(pParamDtmf);
        return;
      }

      pParam->Get((BYTE*)dtmf, dtmfLength);
      *pParamDtmf << (DWORD)dtmfLength;                                                               // Length
      pParamDtmf->Put((BYTE*)dtmf, dtmfLength);
      break;
    }

    default:
      TRACEINTO << "CFeatureObject::DtmfReceived - Failed, Illegal opcode, Opcode:" << opcode;
	  POBJDELETE(pParamDtmf);
      return;
  }

  if (m_stopUponFirstDTMF)
  {
	  if ( m_participants->IsTipMaster(partyRsrcID))
		  m_confAppInfo->m_pConfApi->PartytoPartyCntlMsgFromMasterToSlave(m_partyRsrcID, eTipSlaveAux,STOP_PLAY_MESSAGE_AUX, NULL);
	  else
    StopPlayMediaCommand(EVENT_PARTY_REQUEST, IVR_MEDIA_TYPE_AUDIO);
    m_stopUponFirstDTMF = 0;
  }

  if (m_endFeatureUponDTMF)
  {
    DeleteTimer(TIMER_PLAY_MSG);
    OnTimerEndIvrMsg(NULL);
  }

  // Send the received DTMF to party
  (party->m_pPartyApi)->SendReceivedDtmfToParty(pParamDtmf);
}


#include <fstream>

//--------------------------------------------------------------------------
char CFeatureObject::TranslateAudioDTMF(BYTE dtmf)
{
  BOOL isHidePsw = NO;
  std::string key_hide  = "HIDE_CONFERENCE_PASSWORD";
  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key_hide, isHidePsw);

  if (dtmf > 11)
  {
    TRACEINTO << "CFeatureObject::TranslateAudioDTMF - Invalid DTMF value: " << (int)dtmf << " val";
    return -1;
  }
  else if (!isHidePsw)
    TRACEINTO << "CFeatureObject::TranslateAudioDTMF - DTMF received: " << (int)dtmf << " val";
  else
    TRACEINTO << "CFeatureObject::TranslateAudioDTMF - DTMF received: " << "* val";

  char tran[12] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#' };

// sagi patch for video applications...
  std::cout << "DTMF:" << tran[dtmf] << std::endl;
  std::fstream filestr;
  std::string fname = MCU_TMP_DIR+"/DTMF";
  filestr.open (fname.c_str(), std::fstream::in | std::fstream::out | std::fstream::app);
  filestr << tran[dtmf];
  filestr.close();
// end of patch

  return tran[dtmf];
}

//--------------------------------------------------------------------------
STATUS CFeatureObject::UpdatePlayMessageWithOneRollCallFile(SIVRPlayMessageStruct* playMsg, DWORD opcode)
{
  // get roll-call file name
  WORD checkSum = 0;
  WORD duration = 0;
  const char* rollCallName = m_participants->GetPartyRollCallParams(m_partyRsrcID, checkSum, duration, opcode);
  std::string rcName;
  if ((rollCallName) && (strlen(rollCallName) > 0))
  {
    rcName  = IVR_FOLDER_ROLLCALL_EMB;
    rcName += rollCallName;
    WORD ind = 0;
    UpdateMediaEntry(playMsg, ind, rcName.c_str(), checkSum, duration);   // update struct
  }
  else // Roll Call recording doesn't exist
  {
    return STATUS_FAIL;
  }

  return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CFeatureObject::UpdatePlayMessageForRollCallReview(SIVRPlayMessageStruct* playMsg, DWORD opcode, DWORD* fileIDArray, WORD numOfRollCallFiles)
{
  WORD intarrInd = 0;
  for (int i = 0; i < numOfRollCallFiles; i++)
  {
    // add roll-call media file
    DWORD       partyRsrcID  = fileIDArray[intarrInd++];
    WORD        checkSum     = 0;
    WORD        duration     = 0;
    const char* rollCallName = m_participants->GetPartyRollCallParams(/*m_partyRsrcID*/ partyRsrcID, checkSum, duration, opcode);
    std::string rcName;
    if ((rollCallName) && (strlen(rollCallName) > 0))
    {
      rcName  = IVR_FOLDER_ROLLCALL_EMB;
      rcName += rollCallName;

      // update PlayMessge command
      UpdateMediaEntry(playMsg, i+1, rcName.c_str(), checkSum, duration);   // update struct
    }
    else  // Roll Call recording doesn't exist
    {
      numOfRollCallFiles--;
      i--;
    }
  }

  return numOfRollCallFiles;                                                // Actual number of Roll Call files
}

//--------------------------------------------------------------------------
void CFeatureObject::UpdateMediaEntry(SIVRPlayMessageStruct* playMsg, WORD ind, const char* mediaName, WORD checkSum, WORD duration)
{
  playMsg->mediaFiles[ind].actionType     = IVR_ACTION_TYPE_PLAY;
  playMsg->mediaFiles[ind].duration       = duration;
  playMsg->mediaFiles[ind].checksum       = (DWORD)checkSum;
  playMsg->mediaFiles[ind].fileNameLength = strlen(mediaName);
  SAFE_COPY(playMsg->mediaFiles[ind].fileName, mediaName);
}

//--------------------------------------------------------------------------
void CFeatureObject::UpdateSilenceEntry(SIVRPlayMessageStruct* playMsg, WORD ind, WORD silenceDuration)
{
  playMsg->mediaFiles[ind].actionType = IVR_ACTION_TYPE_SILENCE;
  playMsg->mediaFiles[ind].duration   = silenceDuration; // second
}

//--------------------------------------------------------------------------
WORD CFeatureObject::CalculateTotalPlayMsgDuration(SIVRPlayMessageStruct* playMsg)
{
  WORD totalDuration   = 0;
  int  numOfMediaFiles = playMsg->numOfMediaFiles;
  for (int ind = 0; ind < numOfMediaFiles; ind++)
    totalDuration += playMsg->mediaFiles[ind].duration;

  return totalDuration;
}

//--------------------------------------------------------------------------
void CFeatureObject::StopPlayMediaCommand(DWORD confOrParty, DWORD mediaType)
{
  CSegment* pParams = new CSegment;
  SIVRStopPlayMessage stopPlayMessage;
  memset(&stopPlayMessage, 0, sizeof(SIVRStopPlayMessage));
  stopPlayMessage.mediaType = mediaType;

  if (EVENT_CONF_REQUEST == confOrParty)                // for Conf
  {
    if (mediaType != IVR_MEDIA_TYPE_AUDIO)
    {
      POBJDELETE(pParams);
      return;                                           // no video for conf
    }

    stopPlayMessage.partyOrconfFlag = IVR_PLAY_MSG_TO_CONF;
    pParams->Put((BYTE*)&stopPlayMessage, (WORD)sizeof(SIVRStopPlayMessage));
    if (m_confAppInfo->m_pAudBrdgInterface)
      m_confAppInfo->m_pAudBrdgInterface->IvrConfCommand(IVR_STOP_PLAY_MESSAGE_REQ, 0, pParams);
  }
  else                                                  // for Party
  {
    stopPlayMessage.partyOrconfFlag = IVR_PLAY_MSG_TO_PARTY;
    pParams->Put((BYTE*)&stopPlayMessage, (WORD)sizeof(SIVRStopPlayMessage));
    if (IVR_MEDIA_TYPE_AUDIO == mediaType)              // audio
    {
      if (m_confAppInfo->m_pAudBrdgInterface)
      {
        if (!m_participants->IsPartyAudioStateConnected(m_partyRsrcID))
          TRACEINTO << "CFeatureObject::StopPlayMediaCommand - Failed, Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
        else
          m_confAppInfo->m_pAudBrdgInterface->IvrPartyCommand(m_partyRsrcID, IVR_STOP_PLAY_MESSAGE_REQ, 0, pParams);
      }
    }
    else if (m_confAppInfo->m_pVideoBridgeInterface)    // video
    {
      if (m_partyRsrcID == 0)
      {
        TRACEINTO << "CFeatureObject::StopPlayMediaCommand - Failed, Invalid party id,, stop show slide is not sent";
      }
      else
      {
        m_confAppInfo->m_pVideoBridgeInterface->IvrPartyCommand(m_partyRsrcID, IVR_STOP_SHOW_SLIDE_REQ, pParams);
        m_participants->SetSlideIsOn(m_partyRsrcID, false);
      }
    }
  }

  POBJDELETE(pParams);
}

//--------------------------------------------------------------------------
void CFeatureObject::StartIVR(DWORD confOrParty, WORD startIVRMode)
{
  if (!m_confAppInfo->m_pAudBrdgInterface)
    return;                                                                           // should be okay

  CSegment*    pParams = new CSegment;
  CIVRStartIVR startIVR;
  memset(&startIVR, 0, sizeof(CIVRStartIVR));

  SIVRStartIVRStruct* sStartIVR       = &startIVR.m_startIVR;
  SIVRStartIVRParams* sStartIVRParams = &sStartIVR->params;

  sStartIVR->mediaType = IVR_MEDIA_TYPE_AUDIO;

  if (EVENT_CONF_REQUEST == confOrParty)                                              // for Conf
  {
    sStartIVR->partyOrconfFlag = IVR_PLAY_MSG_TO_CONF;

    if (START_IVR_CONF_SIMPLE_GAINS == startIVRMode)
    {
      sStartIVRParams->privateIVRMsgVolume   = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->privateIVRMusicVolume = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->confIVRMsgVolume      = m_confAppInfo->GetIvrMessageVolume();  // E_AUDIO_GAIN_PRESET_5;
      sStartIVRParams->confIVRMusicVolume    = m_confAppInfo->GetIvrMusicVolume();    // E_AUDIO_GAIN_PRESET_5;
      sStartIVRParams->encoderConfMixVolume  = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->decoderConfMixVolume  = E_AUDIO_GAIN_PRESET_0;
    }

    if (START_IVR_CONF_LOWER_GAINS == startIVRMode)
    {
      sStartIVRParams->privateIVRMsgVolume   = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->privateIVRMusicVolume = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->confIVRMsgVolume      = m_confAppInfo->GetIvrRollCallVolume(); // E_AUDIO_GAIN_PRESET_4;
      sStartIVRParams->confIVRMusicVolume    = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->encoderConfMixVolume  = E_AUDIO_GAIN_PRESET_5;
      sStartIVRParams->decoderConfMixVolume  = E_AUDIO_GAIN_PRESET_5;
    }

    startIVR.Serialize(pParams);
    m_confAppInfo->m_pAudBrdgInterface->IvrConfCommand(IVR_START_IVR_REQ, 0, pParams);

    // update CAM info
    if (START_IVR_CONF_SIMPLE_GAINS == startIVRMode)
      m_confAppInfo->SetConfStartIVRMode(START_IVR_CONF_SIMPLE_GAINS);

    if (START_IVR_CONF_LOWER_GAINS == startIVRMode)
      m_confAppInfo->SetConfStartIVRMode(START_IVR_CONF_LOWER_GAINS);
  }
  else                                                                                // for Party
  {
    sStartIVR->partyOrconfFlag = IVR_PLAY_MSG_TO_PARTY;

    if (START_IVR_PARTY_MESSAGE_GAINS == startIVRMode)
    {
      sStartIVRParams->privateIVRMsgVolume   = m_confAppInfo->GetIvrMessageVolume();  // E_AUDIO_GAIN_PRESET_5;
      sStartIVRParams->privateIVRMusicVolume = m_confAppInfo->GetIvrMusicVolume();    // E_AUDIO_GAIN_PRESET_5;
      sStartIVRParams->confIVRMsgVolume      = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->confIVRMusicVolume    = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->encoderConfMixVolume  = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->decoderConfMixVolume  = E_AUDIO_GAIN_PRESET_0;
    }

    if (START_IVR_PARTY_MUSIC_GAINS == startIVRMode)
    {
      sStartIVRParams->privateIVRMsgVolume   = m_confAppInfo->GetIvrMessageVolume();  // E_AUDIO_GAIN_PRESET_5;
      sStartIVRParams->privateIVRMusicVolume = m_confAppInfo->GetIvrMusicVolume();    // E_AUDIO_GAIN_PRESET_5;
      sStartIVRParams->confIVRMsgVolume      = m_confAppInfo->GetIvrMessageVolume();
      sStartIVRParams->confIVRMusicVolume    = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->encoderConfMixVolume  = E_AUDIO_GAIN_PRESET_0;
      sStartIVRParams->decoderConfMixVolume  = E_AUDIO_GAIN_PRESET_0;
    }

    startIVR.Serialize(pParams);

    if (!m_participants->IsPartyAudioStateConnected(m_partyRsrcID))
    {
      TRACEINTO << "CFeatureObject::StartIVR - Failed, Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
    }
    else
    {
      m_confAppInfo->m_pAudBrdgInterface->IvrPartyCommand(m_partyRsrcID, IVR_START_IVR_REQ, 0, pParams);
      m_participants->SetPartyStartIvrMode(m_partyRsrcID, startIVRMode);
    }
  }

  POBJDELETE(pParams);
}

//--------------------------------------------------------------------------
void CFeatureObject::StopIVR(DWORD confOrParty)
{
  if (!m_confAppInfo->m_pAudBrdgInterface)
    return;

  SIVRStopIVRStruct stopIvr;
  memset(&stopIvr, 0, sizeof(SIVRStopIVRStruct));
  if (EVENT_CONF_REQUEST == confOrParty)
    stopIvr.partyOrconfFlag = IVR_PLAY_MSG_TO_CONF;
  else
    stopIvr.partyOrconfFlag = IVR_PLAY_MSG_TO_PARTY;

  stopIvr.mediaType = IVR_MEDIA_TYPE_AUDIO;

  CSegment* pParams = new CSegment;
  pParams->Put((BYTE*)&stopIvr, (WORD)sizeof (SIVRStopIVRStruct));

  // this will stop the IVR Message and the IVR Music
  if (EVENT_CONF_REQUEST == confOrParty)
  {
    m_confAppInfo->m_pAudBrdgInterface->IvrConfCommand(IVR_STOP_IVR_REQ, 0, pParams);
    // update CAM info
    m_confAppInfo->SetConfStartIVRMode(START_IVR_CONF_STOPPED);
  }
  else  // EVENT_PARTY_REQUEST
  {
    if (!m_participants->IsPartyAudioStateConnected(m_partyRsrcID))
    {
      TRACEINTO << "CFeatureObject::StopIVR - Failed, Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
    }
    else
    {
      m_confAppInfo->m_pAudBrdgInterface->IvrPartyCommand(m_partyRsrcID, IVR_STOP_IVR_REQ, 0, pParams);
      // update party's start IVR mode
      m_participants->SetPartyStartIvrMode(m_partyRsrcID, START_IVR_PARTY_STOPPED);
    }
  }

  POBJDELETE(pParams);
}

//--------------------------------------------------------------------------
int CFeatureObject::AskForObjection(TConfAppEvents feature, DWORD partyRsrcID)
{
  switch (m_opcode)
  {
    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:
    {
      switch (feature)
      {
        case eCAM_EVENT_CONF_MUTE_ALL:
        case eCAM_EVENT_CONF_UNMUTE_ALL:
        case eCAM_EVENT_CONF_ALERT_TONE:
        case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:
        case eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW:
        case eCAM_EVENT_CONF_SECURE:
        case eCAM_EVENT_CONF_UNSECURE:
        case eCAM_EVENT_PARTY_PLC:
          return APP_F_OBJECTION;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
      }
      break;
    }
  }
  return APP_F_NO_OBJECTION;
}

//--------------------------------------------------------------------------
void CFeatureObject::AskForPreAction(TConfAppEvents feature, DWORD partyRsrcID, WORD ConfOrParty)
{
}

//--------------------------------------------------------------------------
void CFeatureObject::StopConfFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID)
{
  switch (opcode)
  {
    case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:        // end wait for chair for conf
    {
      StopConfWaitForChair();
      break;
    }

    case eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW:  // stop Roll Call review by DTMF - requires stop play message
    {
      DeleteTimer(TIMER_PLAY_MSG);
      // Stop play message
      StopPlayMediaCommand(EVENT_CONF_REQUEST, IVR_MEDIA_TYPE_AUDIO);
      // update "confinfo" (decrement num of events currently active)
      m_confAppInfo->DecConfActiveEventsCounter();
      if (m_confAppInfo->GetConfActiveEventsCounter() == 0)
        StopConfIVR();
      break;
    }

    case eCAM_EVENT_CONF_CHAIR_DROPPED:         // off
    case eCAM_EVENT_CONF_MUTE_ALL:
    case eCAM_EVENT_CONF_UNMUTE_ALL:
    case eCAM_EVENT_CONF_ALERT_TONE:
    case eCAM_EVENT_CONF_EXIT_TONE:
    case eCAM_EVENT_CONF_ENTRY_TONE:
    case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:      // stop Roll Call review upon ack/timer/replace by other feature
    case eCAM_EVENT_CONF_NOISY_LINE_DETECTION:
    case eCAM_EVENT_CONF_SECURE:
    case eCAM_EVENT_CONF_UNSECURE:
    case eCAM_EVENT_CONF_RECORDING_FAILED:
    case eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK:
    {
      // update "confinfo" (decrement num of events currently active)
      m_confAppInfo->DecConfActiveEventsCounter();
      if (m_confAppInfo->GetConfActiveEventsCounter() == 0)
        StopConfIVR();

      if (eCAM_EVENT_CONF_EXIT_TONE == opcode)
      {
        m_confAppInfo->DecConfActiveExitToneCounter();
      }

      break;
    }
  }

  // end of this feature, call to the next feature related to current event
  if (0 != m_eventUniqueNumber)                   // only when the feature is result of event (END-IVR, PARTY_DISCONNECTED, etc.)
  {
    m_confAppInfo->m_pConfApi->NotifyCAM(m_partyRsrcID, eCAM_EVENT_PARTY_END_FEATURE, m_eventUniqueNumber);
    return;
  }
}

//--------------------------------------------------------------------------
void CFeatureObject::StopPartyFeature(DWORD eventUniqueNumber, DWORD opcode, DWORD partyRsrcID)
{
  switch (opcode)
  {
    case eCAM_EVENT_PARTY_IVR_ENTRANCE:
      if (m_confAppInfo->GetIsEntryQueue())         // if it is EQ we need to do StopSlide
        MovePartyVideoToMix();
      else
        m_confAppInfo->m_pAudBrdgInterface->IvrUpdatePartyStandalone(m_partyRsrcID, 0);

      return;

    case eCAM_EVENT_PARTY_WAIT_FOR_CHAIR:         // end wait for chair for party
      // StopPartyWaitForChair();
      // return;
      break;

    case eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC:     // end single party music
      StopSinglePartyMusic();
      break;

    case eCAM_EVENT_PARTY_PLC:                    // Stop PLC
      StartStopPlcCommand(VIDEO_GRAPHIC_OVERLAY_STOP_REQ);
      break;

    case eCAM_EVENT_PARTY_VENUS:
      // StartStopVenusCommand(VIDEO_GRAPHIC_OVERLAY_STOP_REQ);//carmit45
      return;

    case eCAM_EVENT_PARTY_CANCEL_PLC:
      break;                                      // nothing to do

    case eCAM_EVENT_PARTY_PARTY_ROLL_CALL_REVIEW: // end roll-call review for party
      break;

    case eCAM_EVENT_PARTY_CHANGE_PW:              // end change password
      break;

    case eCAM_EVENT_INVITE_PARTY:                 // end invite party
    {
      if (m_confAppInfo->GetIsGateWay() &&
          !m_confAppInfo->GetIsRingToneOn() &&
          m_confAppInfo->GetSinglePartyNow())
      {
        //Ready to play the ringtone after invite party feature.
        m_confAppInfo->SetSinglePartyNow(0);
        m_confAppInfo->SetSinglePartyRsrcId((WORD)-1);
      }
      break;
    }

    case eCAM_EVENT_PARTY_PLAY_BUSY_MSG:          // end play busy msg
      break;

    case eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG:      // end play noanswer msg
      break;

    case eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG:  // end play wrong number msg
      break;

    case eCAM_EVENT_PARTY_SILENCE_IT:             // end silence-It
      break;

    case eCAM_EVENT_PARTY_PLAY_MENU:              // end Play Menu
      break;

    case eCAM_EVENT_PARTY_FIRST_TO_JOIN:          // end First-To-Join
      break;

    case eCAM_EVENT_PARTY_MUTE:                   // end Mute my line
      break;

    case eCAM_EVENT_PARTY_UNMUTE:                 // end Unmute my line
      break;

    case eCAM_EVENT_CONF_RECORDING_FAILED:        // end Recording Failed
      break;

    case eCAM_EVENT_PARTY_RECORDING_IN_PROGRESS:  // end Recording in Progress
      break;

    case eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE:
      break;

    case eCAM_EVENT_PARTY_PLAY_RINGING_TONE:
    {
      m_confAppInfo->SetIsRingToneOn(0);
      StopPlayMediaCommand(EVENT_PARTY_REQUEST, IVR_MEDIA_TYPE_AUDIO);
      break;
    }
  } // switch

  // end of this feature, call to the next feature related to current event
  if (0 != m_eventUniqueNumber)                   // only when the feature is result of event (END-IVR, PARTY_DISCONNECTED, etc.)
  {
    m_confAppInfo->m_pConfApi->NotifyCAM(m_partyRsrcID, eCAM_EVENT_PARTY_END_FEATURE, m_eventUniqueNumber);
    return;
  }

  // End of the last feature in event or end of IVR feature that is not originated from event
  m_confAppInfo->m_pConfApi->NotifyCAM(m_partyRsrcID, eCAM_EVENT_PARTY_END_LAST_FEATURE, 0);

  return;
}

//--------------------------------------------------------------------------
void CFeatureObject::StopPartyFeatureWithoutNotifyCAM(DWORD opcode)
{
	switch (opcode)
	{
	case eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC:     // end single party music
		StopSinglePartyMusic();
		break;
	}
}

//--------------------------------------------------------------------------
void CFeatureObject::StartMusicToConfOrParty(WORD confOrParty)
{
  TRACEINTO << "CFeatureObject::StartMusicToConfOrParty "
            << "- PartyId:"    << m_partyRsrcID
            << ", Conf/Party:" << confOrParty;

  // Set isSinglePartyNow flag in CAM Info
  if (EVENT_PARTY_REQUEST == confOrParty)
  {
    m_confAppInfo->SetSinglePartyNow(1);
    m_confAppInfo->SetSinglePartyRsrcId(m_partyRsrcID);
  }

  SIVRPlayMusicStruct playMusic;
  memset(&playMusic, 0, sizeof(SIVRPlayMusicStruct));

  // fills struct
  if (EVENT_CONF_REQUEST == confOrParty)
    playMusic.partyOrconfFlag = IVR_PLAY_MSG_TO_CONF;
  else
    playMusic.partyOrconfFlag = IVR_PLAY_MSG_TO_PARTY;

  playMusic.musicSourceID = START_MUSIC_SOURCE_ID;  // constant for the first version
  SAFE_COPY(playMusic.fileName, IVR_FOLDER_MUSIC_TMP_FILE_EMB);

  CSegment* seg = new CSegment;
  seg->Put((BYTE*)&playMusic, (DWORD)sizeof(SIVRPlayMusicStruct));

  if (m_confAppInfo->m_pAudBrdgInterface)
  {
    if (EVENT_CONF_REQUEST == confOrParty)
      m_confAppInfo->m_pAudBrdgInterface->IvrConfCommand(IVR_PLAY_MUSIC_REQ, 0, seg);
    else
    {
      if (!m_participants->IsPartyAudioStateConnected(m_partyRsrcID))
        TRACEINTO << "CFeatureObject::StartMusicToConfOrParty - Failed, Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
      else
        m_confAppInfo->m_pAudBrdgInterface->IvrPartyCommand(m_partyRsrcID, IVR_PLAY_MUSIC_REQ, 0, seg);
    }
  }
  POBJDELETE(seg);
}

//--------------------------------------------------------------------------
void CFeatureObject::StopMusicToConfOrParty(DWORD confOrParty)
{
  if (!m_confAppInfo->m_pAudBrdgInterface)
    return;

  TRACEINTO << "CFeatureObject::StopMusicToConfOrParty "
            << "- PartyId:"    << m_partyRsrcID
            << ", Conf/Party:" << confOrParty;

  SIVRStopPlayMusicStruct stopMusic;
  memset(&stopMusic, 0, sizeof(SIVRStopPlayMusicStruct));
  if (EVENT_CONF_REQUEST == confOrParty)
    stopMusic.partyOrconfFlag = IVR_PLAY_MSG_TO_CONF;
  else
    stopMusic.partyOrconfFlag = IVR_PLAY_MSG_TO_PARTY;

  CSegment* seg = new CSegment;
  seg->Put((BYTE*)&stopMusic, (WORD)sizeof (SIVRStopPlayMusicStruct));

  if (confOrParty == EVENT_PARTY_REQUEST)
  {
    if (!m_participants->IsPartyAudioStateConnected(m_partyRsrcID))
      TRACEINTO << "CFeatureObject::StopMusicToConfOrParty - Failed, Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
    else
      m_confAppInfo->m_pAudBrdgInterface->IvrPartyCommand(m_partyRsrcID, IVR_STOP_PLAY_MUSIC_REQ, 0, seg);
  }
  else
    m_confAppInfo->m_pAudBrdgInterface->IvrConfCommand(IVR_STOP_PLAY_MUSIC_REQ, 0, seg);

  POBJDELETE(seg);
}

//--------------------------------------------------------------------------
void CFeatureObject::StopConfWaitForChair()
{
  if (0 < m_confAppInfo->GetConfActiveEventsCounter())
  {
    StopMusicToConfOrParty(EVENT_CONF_REQUEST);         // Stop PlayMusic to conf
    StopPlayMediaCommand(EVENT_CONF_REQUEST, IVR_MEDIA_TYPE_AUDIO);
    m_confAppInfo->DecConfActiveEventsCounter();        // update "confinfo" (decrement num of events currently active)
  }

  // update "confinfo" (set off m_inWaitForChairNow & m_isWaitForChair flags)
  m_confAppInfo->SetInWaitForChairNow(0);
  m_confAppInfo->SetWaitForChair(0);                    // After chair enterance the wait for chair feature is disabled

  StopConfIVR();                                        // the IVR is stopped only if needed (no other IVR conf activity in list)

  return;                                               // nothing more to do for now
}

//--------------------------------------------------------------------------
void CFeatureObject::StopConfIVR()
{
  // checks if only that conf feature is running now
  if (0 == m_confAppInfo->GetConfActiveEventsCounter()) // this is the only conf activity regarding IVR
  {
    // move conf to Mix mode (Stop-IVR for Conf)
    StopIVR(EVENT_CONF_REQUEST);
  }
  return;                                               // nothing more to do for now
}

//--------------------------------------------------------------------------
void CFeatureObject::EnterPartyToMix(DWORD partyRsrcID)
{
  m_partyRsrcID = partyRsrcID;
  StopAudioVideoIVRMode();
}

//--------------------------------------------------------------------------
void CFeatureObject::StopAudioVideoIVRMode()
{
  MovePartyAudioToMix();
  MovePartyVideoToMix();
}

//--------------------------------------------------------------------------
void CFeatureObject::MovePartyAudioToMix()
{
  StopIVR(EVENT_PARTY_REQUEST);

  // audio party in MIX state
  m_participants->SetPartyAudioState(m_partyRsrcID, eAPP_PARTY_STATE_MIX);
}

//--------------------------------------------------------------------------
void CFeatureObject::MovePartyVideoToMix()
{
  int partyVideoState = m_participants->GetPartyVideoState(m_partyRsrcID);
  if ((partyVideoState != eAPP_PARTY_STATE_IVR_ENTRY) &&
      (partyVideoState != eAPP_PARTY_STATE_IVR_FEATURE) &&
      (partyVideoState != eAPP_PARTY_STATE_MOVING))
  {
    TRACEINTO << "CFeatureObject::MovePartyVideoToMix - No need for Stop Slide, PartyState:" << partyVideoState;
    return;
  }

  StopPlayMediaCommand(EVENT_PARTY_REQUEST, IVR_MEDIA_TYPE_VIDEO);
  if (m_confAppInfo->GetIsEntryQueue() == 0)
    JoinConfVideo();
}

//--------------------------------------------------------------------------
void CFeatureObject::JoinConfVideo()
{
  if (m_confAppInfo->m_pVideoBridgeInterface)   // video
  {
    if (!m_participants->IsPartyVideoStateConnected(m_partyRsrcID))
    {
      TRACEINTO << "CFeatureObject::JoinConfVideo - Failed, Party's video is NOT connected, sending the command to video bridge is aborted, PartyId:" << m_partyRsrcID;
    }
    else
    {
      m_confAppInfo->m_pVideoBridgeInterface->IvrPartyCommand(m_partyRsrcID, IVR_JOIN_CONF_VIDEO, NULL);

      // video party in connecting to MIX state
      m_participants->SetPartyVideoState(m_partyRsrcID, eAPP_PARTY_STATE_CONNECTING_TO_MIX);
    }
  }
}

//--------------------------------------------------------------------------
void CFeatureObject::StopPartyWaitForChair()
{
  // stop "wait for chair"
  // at this point, the party should stop see the slide
  MovePartyVideoToMix();
}

//--------------------------------------------------------------------------
void CFeatureObject::StopSinglePartyMusic()
{
  StopMusicToConfOrParty(EVENT_PARTY_REQUEST);

  // Set Single Party flags in CAM Info
  m_confAppInfo->SetSinglePartyNow(0);
  m_confAppInfo->SetSinglePartyRsrcId((WORD)-1);
}

//--------------------------------------------------------------------------
void CFeatureObject::StartIVRForPartyIfNeeded(WORD startIVRMode,BOOL bFromTipMaster)
{
  int partyIndex = m_participants->FindPartyIndex(m_partyRsrcID);
  TRACECOND_AND_RETURN(-1 == partyIndex, "CFeatureObject::StartIVRForPartyIfNeeded - Failed, Party not found, PartyId:" << m_partyRsrcID);

  CConfAppPartyParams* pParty = m_participants->m_partyList[partyIndex];

  int partyState = pParty->GetPartyAudioState();
  TRACEINTO << "partyId " << m_partyRsrcID << ", audio state:" << partyState;
  if (partyState == eAPP_PARTY_STATE_MIX ||
      partyState == eAPP_PARTY_STATE_IVR_ENTRY ||
      partyState == eAPP_PARTY_STATE_IVR_FEATURE)
  {
    // checks if the party is in the right IVR mode. If not, move it to the relevant IVR mode
    if (pParty->GetPartyStartIvrMode() != startIVRMode)
    {
      StartIVR(EVENT_PARTY_REQUEST, startIVRMode);
	  if (!bFromTipMaster || partyState == eAPP_PARTY_STATE_MIX)
      	pParty->SetPartyAudioState(eAPP_PARTY_STATE_IVR_FEATURE);
    }
  }
}

//--------------------------------------------------------------------------
void CFeatureObject::StopIVRUponDisconnecting(DWORD confOrParty)
{
  StopIVR(confOrParty);
}

//--------------------------------------------------------------------------
void CFeatureObject::StopSlideUponDisconnecting()
{
  StopPlayMediaCommand(EVENT_PARTY_REQUEST, IVR_MEDIA_TYPE_VIDEO);
}

//--------------------------------------------------------------------------
void CFeatureObject::StopMusicUponDisconnecting(DWORD confOrParty)
{
  if (EVENT_PARTY_REQUEST == confOrParty)
    StopSinglePartyMusic();
  else
  {
    StopMusicToConfOrParty(EVENT_CONF_REQUEST);   // Stop PlayMusic to conf
    // update "confinfo" (set off m_inWaitForChairNow & m_isWaitForChair flags)
    m_confAppInfo->SetInWaitForChairNow(0, confOrParty == EVENT_CONF_REQUEST);
  }
}

//--------------------------------------------------------------------------
int CFeatureObject::ReplaceTokenWithSeqNumIfNeeded(DWORD seqNumToken, DWORD sequenceNum)
{
  if (m_IvrMessageSessionIDCam == seqNumToken)
  {
    m_IvrMessageSessionIDCam = sequenceNum;       // the real one instead of the temporary token
    TRACEINTO << " CFeatureObject::ReplaceTokenWithSeqNumIfNeeded "
              << "- Token:"    << seqNumToken
              << ", Replaced:" << sequenceNum;
    return 1;                                     // found
  }
  return 0;                                       // not found
}

//--------------------------------------------------------------------------
void CFeatureObject::EndFeatureAndStopPlayMsgUponRequest(BOOL stopPlayMsg)
{
  PTRACE(eLevelInfoNormal, "CFeatureObject::EndFeatureAndStopPlayMsgUponRequest ");

  PASSERT_AND_RETURN(F_APP_CONF_IVR != m_featureType); // this function suits conference features only

  DeleteTimer(TIMER_PLAY_MSG);
  // Stop play message if needed
  if (stopPlayMsg)
    StopPlayMediaCommand(EVENT_CONF_REQUEST, IVR_MEDIA_TYPE_AUDIO);

  OnEndIvrMsg();
}

//--------------------------------------------------------------------------
void CFeatureObject::OnAckEndIvrMsg()
{
  DeleteTimer(TIMER_PLAY_MSG);
  OnEndIvrMsg();
}

//--------------------------------------------------------------------------
void CFeatureObject::OnTimerRequestToSpeak(CSegment* pParam)
{
  BYTE onOff = 0;
  m_confAppInfo->m_pConfApi->NotifyCAMTimer(m_partyRsrcID, eCAM_EVENT_TIMER_END_FEATURE, CAM_PARTY_ACTION, m_eventUniqueNumber, m_opcode);

  int partyIndex = m_participants->FindPartyIndex(m_partyRsrcID);
  TRACECOND_AND_RETURN(-1 == partyIndex, "CFeatureObject::OnTimerRequestToSpeak - Failed, Party not found, PartyId:" << m_partyRsrcID);

  string    partyName = m_participants->m_partyList[partyIndex]->GetPartyName();
  CSegment* pSeg      = new CSegment;
  *pSeg << (char*)partyName.c_str() << onOff;
  m_confAppInfo->m_pConfApi->UpdateDB(NULL, PARTY_REQUEST_TO_SPEAK, (DWORD) 0, 1, pSeg);
}

//--------------------------------------------------------------------------
void CFeatureObject::RestartConfWaitForChair()
{
  StartIVR(EVENT_CONF_REQUEST, START_IVR_CONF_SIMPLE_GAINS);
  StartMusicToConfOrParty(EVENT_CONF_REQUEST);
  WORD isNeedStartIvr = 0;
  PlayMessage(EVENT_CONF_REQUEST, isNeedStartIvr, m_opcode);
  return;
}

//--------------------------------------------------------------------------
void CFeatureObject::SendDtmf(CSegment* pParam)
{
  std::string dtmfBuffer;
  *pParam >> dtmfBuffer;

  CSegment* seg = new CSegment;
  *seg << dtmfBuffer;

  // send SendDtmf command
  CAudioBridgeInterface* pAudBrdgInterface = m_confAppInfo->m_pAudBrdgInterface;
  if (pAudBrdgInterface)
  {
    if (!m_participants->IsPartyAudioStateConnected(m_partyRsrcID))
      TRACEINTO << "CFeatureObject::SendDtmf - Failed, Party's audio is NOT connected, sending the command to audio bridge is aborted, PartyId:" << m_partyRsrcID;
    else
    {
      TRACEINTO << "CFeatureObject::SendDtmf "
                << "- Opcode:"  << "AUDIO_PLAY_TONE_REQ" << " (#" << AUDIO_PLAY_TONE_REQ << ")"
                << ", PartyId:" << m_partyRsrcID
                << ", DTMF:"    << dtmfBuffer.c_str();
      pAudBrdgInterface->IvrPartyCommand(m_partyRsrcID, AUDIO_PLAY_TONE_REQ, 0, seg);
    }
  }

  POBJDELETE(seg);
}
//--------------------------------------------------------------------------
void CFeatureObject::Dump(DWORD level)
{
  COstrStream trace_str;

  Dump(trace_str);

  PTRACE2(level, "CFeatureObject::Dump : \n", trace_str.str().c_str());
}

//--------------------------------------------------------------------------
void CFeatureObject::Dump(COstrStream& trace_str)
{
  trace_str << "m_partyRsrcID = " << m_partyRsrcID << "\n";
  trace_str << "m_opcode      = " << (DWORD)m_opcode << "\n";
  trace_str << "m_featureType = " << (DWORD)m_featureType << "\n";
}

//--------------------------------------------------------------------------
void CFeatureObject::SendClientDtmf(CSegment* pParam)
{
  if ((CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) &&
      (CProcessBase::GetProcess()->GetProductType() != eProductTypeCallGeneratorSoftMCU))
  {
    TRACEINTO << "CFeatureObjectList::SendClientDtmf - Failed, The system is not a CallGenerator";
    return;
  }

  std::string dtmfBuffer;
  *pParam >> dtmfBuffer;

  int dtmfLen = dtmfBuffer.length();
  TRACECOND_AND_RETURN(!dtmfLen, "CFeatureObject::SendClientDtmf - Failed, Empty DTMF string");

  if (dtmfLen == 3)
  {
    const char* dtmfStr = dtmfBuffer.c_str();
    if (dtmfStr)
    {
      if (('A' == dtmfStr[0]) &&
          ('B' == dtmfStr[1]) &&
          ('C' == dtmfStr[2]))
      {
        // send "ABC<confId>#<PartyMonitorId>#"
        std::string confIdStr;
        *pParam >> confIdStr;

        TRACECOND_AND_RETURN(!confIdStr.length(), "CFeatureObject::SendClientDtmf - Failed, Empty ConfIdString string, DTMF:" << dtmfBuffer.c_str());

        std::string partyMonitorIdStr;
        *pParam >> partyMonitorIdStr;

        TRACECOND_AND_RETURN(!partyMonitorIdStr.length(), "CFeatureObject::SendClientDtmf - Failed, Empty partyMonitorIdStr string, DTMF:" << dtmfBuffer.c_str());

        dtmfBuffer += confIdStr;
        dtmfBuffer += "#";
        dtmfBuffer += partyMonitorIdStr;
        dtmfBuffer += "#";
      }
    }
  }

  CSegment seg;
  seg << dtmfBuffer;
  SendDtmf(&seg);
}
