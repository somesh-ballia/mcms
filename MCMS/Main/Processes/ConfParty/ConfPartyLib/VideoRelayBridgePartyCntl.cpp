#include "VideoRelayBridgePartyCntl.h"
#include "ConfApi.h"
#include "BridgePartyVideoRelayOut.h"
#include "BridgePartyVideoRelayIn.h"
#include "VideoBridgePartyInitParams.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "MplMcmsStructs.h"
#include "BridgePartyVideoRelayMediaParams.h"
#include "PartyApi.h"
#include "StlUtils.h"
#include <cmath>

// ~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);

PBEGIN_MESSAGE_MAP(CVideoRelayBridgePartyCntl)
  ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,        SETUP,      	           CVideoRelayBridgePartyCntl::OnEpAskForIntraSETUP)
  ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,        CONNECTED,      	       CVideoRelayBridgePartyCntl::OnEpAskForIntraCONNECTED)
  ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,        CONNECTED_STANDALONE,   CVideoRelayBridgePartyCntl::OnEpAskForIntraCONNECTED_STANDALONE)
  ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,        DISCONNECTING,  	       CVideoRelayBridgePartyCntl::OnEpAskForIntraDISCONNECTING)

  ONEVENT(ADD_AVC_IMAGE,                       SETUP,          	CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageSETUP)
  ONEVENT(ADD_AVC_IMAGE,                       CONNECTED,      	CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageCONNECTED)
  ONEVENT(ADD_AVC_IMAGE,				CONNECTED_STANDALONE,   CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageCONNECTED_STANDALONE)
  ONEVENT(ADD_AVC_IMAGE,                       DISCONNECTING,  	CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageDISCONNECTING)

  ONEVENT(LAST_AVC_IMAGE_REMOVED,              SETUP,          	CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedSETUP)
  ONEVENT(LAST_AVC_IMAGE_REMOVED,              CONNECTED,      	CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedCONNECTED)
  ONEVENT(LAST_AVC_IMAGE_REMOVED,       CONNECTED_STANDALONE,   CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedCONNECTED_STANDALONE)
  ONEVENT(LAST_AVC_IMAGE_REMOVED,              DISCONNECTING,  	CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedDISCONNECTING)

  ONEVENT(UPDATEONIMAGEAVCTOSVC,               SETUP,           CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateOnImageAvcToSvcCONNECTED)
  ONEVENT(UPDATEONIMAGEAVCTOSVC,               CONNECTED,       CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateOnImageAvcToSvcCONNECTED)
  ONEVENT(UPDATEONIMAGEAVCTOSVC,        CONNECTED_STANDALONE,   CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateOnImageAvcToSvcCONNECTED_STANDALONE)
  ONEVENT(UPDATEONIMAGEAVCTOSVC,               DISCONNECTING,   CVideoRelayBridgePartyCntl::NullActionFunction)

  ONEVENT(UPGRADE_TO_MIX_AVC_SVC,              SETUP,          	CVideoRelayBridgePartyCntl::OnPartyUpgradeSvcToAvcTranslatorSETUP)
  ONEVENT(UPGRADE_TO_MIX_AVC_SVC,              CONNECTED,      	CVideoRelayBridgePartyCntl::OnPartyUpgradeSvcToAvcTranslatorCONNECTED)
  ONEVENT(UPGRADE_TO_MIX_AVC_SVC,   	CONNECTED_STANDALONE,   CVideoRelayBridgePartyCntl::OnPartyUpgradeSvcToAvcTranslatorCONNECTED_STANDALONE)
  ONEVENT(UPGRADE_TO_MIX_AVC_SVC,              DISCONNECTING,  	CVideoRelayBridgePartyCntl::OnPartyUpgradeSvcToAvcTranslatorDISCONNECTING)

  ONEVENT(RELAY_EP_INTRA_SUPRESSION_TIMER,        SETUP,      	           	CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionSETUP)
  ONEVENT(RELAY_EP_INTRA_SUPRESSION_TIMER,        CONNECTED,      	      	CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionCONNECTED)
  ONEVENT(RELAY_EP_INTRA_SUPRESSION_TIMER,        CONNECTED_STANDALONE,   	CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionCONNECTED_STANDALONE)
  ONEVENT(RELAY_EP_INTRA_SUPRESSION_TIMER,        DISCONNECTING,  	       	CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionDISCONNECTING)

PEND_MESSAGE_MAP(CVideoRelayBridgePartyCntl, CVideoBridgePartyCntl);

#define SVC_INTRA_MAX_SUPPRESSION_DURATION_MS 30000

/////////////////////////////////////////////////////////////////////////////
CVideoRelayBridgePartyCntl::CVideoRelayBridgePartyCntl()
{
	m_intraSuppressionDurationInMilliseconds = 0;
}
/////////////////////////////////////////////////////////////////////////////
CVideoRelayBridgePartyCntl::CVideoRelayBridgePartyCntl(const CVideoRelayBridgePartyCntl& rOtherVideoRelayBridgePartyCntl)
: CVideoBridgePartyCntl(rOtherVideoRelayBridgePartyCntl)
{

	m_intraSuppressionDurationInMilliseconds = rOtherVideoRelayBridgePartyCntl.m_intraSuppressionDurationInMilliseconds;
	m_SsrcSuppressedIntras = rOtherVideoRelayBridgePartyCntl.m_SsrcSuppressedIntras;
}

/////////////////////////////////////////////////////////////////////////////
CVideoRelayBridgePartyCntl::~CVideoRelayBridgePartyCntl()
{

}

/////////////////////////////////////////////////////////////////////////////
CVideoRelayBridgePartyCntl&   CVideoRelayBridgePartyCntl::operator=(const CVideoRelayBridgePartyCntl& rOtherVideoRelayBridgePartyCntl)
{
	if (&rOtherVideoRelayBridgePartyCntl == this)
	    return *this;

	(CVideoBridgePartyCntl&)(*this) = (CVideoBridgePartyCntl&)rOtherVideoRelayBridgePartyCntl;
	m_intraSuppressionDurationInMilliseconds = rOtherVideoRelayBridgePartyCntl.m_intraSuppressionDurationInMilliseconds;
	m_SsrcSuppressedIntras = rOtherVideoRelayBridgePartyCntl.m_SsrcSuppressedIntras;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////

void CVideoRelayBridgePartyCntl::Create(const CVideoBridgePartyInitParams* pVideoRelayBridgePartyInitParams)
{
	PASSERT_AND_RETURN(!pVideoRelayBridgePartyInitParams);
	PASSERT_AND_RETURN(!pVideoRelayBridgePartyInitParams->GetMediaInParams() && !pVideoRelayBridgePartyInitParams->GetMediaOutParams());

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	 // Create base params
	 CBridgePartyCntl::Create(pVideoRelayBridgePartyInitParams);
	 m_pUpdatePartyInitParams = NULL;


	  CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);
	  CBridgePartyVideoRelayMediaParams* videoRelayInParams = (CBridgePartyVideoRelayMediaParams*)pVideoRelayBridgePartyInitParams->GetMediaInParams();
	  if (videoRelayInParams)
	  {
	    NewPartyIn();
		SetTelepresenceInfo(videoRelayInParams->GetTelePresenceEPInfo());
	   ((CBridgePartyVideoRelayIn*)(m_pBridgePartyIn))->Create(this, videoRelayInParams, pVideoRelayBridgePartyInitParams->GetSiteName());

	   //Update DB when connecting muted party
	   CheckIsMutedVideoInAndUpdateDB();
		//int  maxAllowedLayerId = ((CVideoBridgeCP*)GetBridge())->GetMaxAllowedLayerId();
		//TRACEINTO << "Max Allowed LayerId: " << maxAllowedLayerId;
	  }

	  CBridgePartyVideoRelayMediaParams* videoRelayOutParams = (CBridgePartyVideoRelayMediaParams*)pVideoRelayBridgePartyInitParams->GetMediaOutParams();
	  if (videoRelayOutParams)
	  {
	    NewPartyOut();
		if (!videoRelayInParams)
			SetTelepresenceInfo(videoRelayOutParams->GetTelePresenceEPInfo());

	    std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
	    pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
	    CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_relay_video_encoder, pTaskApiVideoOut.get());
	    if (!pRsrcDesc)   // Entry not found in Routing Table
	    {
	      POBJDELETE(m_pBridgePartyOut);
	      PASSERT(1);
	    }
	    else
	    {
	      rsrcParams.SetRsrcDesc(*pRsrcDesc);
	      ((CBridgePartyVideoRelayOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoRelayOutParams);

	    }
	  }
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD intraSuppressionDurationInSeconds;
	sysConfig->GetDWORDDataByKey("SVC_INTRA_SUPPRESSION_DURATION_IN_MILLISECONDS", m_intraSuppressionDurationInMilliseconds);
	TRACEINTO << "SVC intra suppression duration is: " << m_intraSuppressionDurationInMilliseconds << " ms.";

}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::Update(const CVideoBridgePartyInitParams* pVideoBridgePartyInitParams)
{
	PASSERT_AND_RETURN(!pVideoBridgePartyInitParams);
	PASSERT_AND_RETURN(!pVideoBridgePartyInitParams->GetMediaInParams() && !pVideoBridgePartyInitParams->GetMediaOutParams());

	  CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	  PASSERT_AND_RETURN(!pRoutingTable);

	  CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	  CBridgePartyVideoRelayMediaParams* videoInParams = (CBridgePartyVideoRelayMediaParams*)pVideoBridgePartyInitParams->GetMediaInParams();
	  if (videoInParams)
	  {
		if (!m_pBridgePartyIn)
		{
		  NewPartyIn();
		  //Update DB when connecting muted party
		  CheckIsMutedVideoInAndUpdateDB();
		}
		// If we are in disconnecting state we need to save the In Init params
		if (m_state == DISCONNECTING || (m_state == CONNECTED && m_pBridgePartyIn->GetState() == CBridgePartyVideoRelayIn::DISCONNECTING))
		{
		  if (m_pUpdatePartyInitParams != NULL)
		  {
			if (m_pUpdatePartyInitParams->GetMediaInParams() != NULL)
			  ((CUpdatePartyVideoRelayInitParams*)m_pUpdatePartyInitParams)->InitiateMediaInParams((CBridgePartyVideoRelayMediaParams*)pVideoBridgePartyInitParams->GetMediaInParams());
		  }
		  else
		  {
			InitiateUpdatePartyParams(pVideoBridgePartyInitParams);
		  }
		}
	  }

	  CBridgePartyVideoRelayMediaParams* videoRelayOutParams = (CBridgePartyVideoRelayMediaParams*)pVideoBridgePartyInitParams->GetMediaOutParams();
	  if (videoRelayOutParams)
	  {
		// If audio out wasn't created already...
		if (!m_pBridgePartyOut)
		{
			NewPartyOut();
			std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
			pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
			CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, eLogical_relay_video_encoder, pTaskApiVideoOut.get());
			if (!pRsrcDesc)   // Entry not found in Routing Table
			{
			  POBJDELETE(m_pBridgePartyOut);
			  PASSERT(1);
			}
			else
			{
			  rsrcParams.SetRsrcDesc(*pRsrcDesc);
			  ((CBridgePartyVideoRelayOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, videoRelayOutParams);

			}
		  }
		// If we are in disconnecting state we need to save the Out Init params
		if (m_state == DISCONNECTING )//currently there is no DISCONNECTING state in CBridgePartyVideoRelayOut //|| (m_state == CONNECTED && m_pBridgePartyOut->GetState() == CBridgePartyVideoRelayOut::DISCONNECTING))
		{
			  if (m_pUpdatePartyInitParams != NULL)
			  {
				if (m_pUpdatePartyInitParams->GetMediaOutParams() != NULL)
					 ((CUpdatePartyVideoRelayInitParams*) m_pUpdatePartyInitParams)->InitiateMediaOutParams((CBridgePartyVideoRelayMediaParams*)pVideoBridgePartyInitParams->GetMediaOutParams());
			  }
			  else
			  {
				InitiateUpdatePartyParams(pVideoBridgePartyInitParams);
			  }
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::Import(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::Import - move should be disabled for relay participants", m_partyConfName);
}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::ImportLegacy(const CBridgePartyInitParams* pBridgePartyInitParams, const CVideoBridgePartyCntl* pOldPartyCntl)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::ImportLegacy - move should be disabled for relay participants", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::InitiateUpdatePartyParams(const CBridgePartyInitParams* pBridgePartyInitParams)
{
	POBJDELETE(m_pUpdatePartyInitParams);
	m_pUpdatePartyInitParams = new CUpdatePartyVideoRelayInitParams(*pBridgePartyInitParams);


}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::NewPartyOut()
{
  PDELETE(m_pBridgePartyOut);
  m_pBridgePartyOut = new CBridgePartyVideoRelayOut();
}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::NewPartyIn()
{
	PDELETE(m_pBridgePartyIn);
	  m_pBridgePartyIn = new CBridgePartyVideoRelayIn();
}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::Connect(BYTE isIVR, BYTE isContentHD1080Supported)
{
	TRACEINTOFUNC << "isIVR = " << (WORD)isIVR;
	CVideoBridgePartyCntl::Connect(isIVR, isContentHD1080Supported);
}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::Export(void)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::Export - move should be disabled for relay participants", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateVideoInParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams)
{
	//Currently there are no states that we wait on that might require handle in state machine
	//We may had in the future
	if(m_pBridgePartyIn)
		((CBridgePartyVideoRelayIn*)m_pBridgePartyIn)->UpdateVideoParams(pBridgePartyVideoRelayParams);
	else
	{
		PASSERT(1);
		PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::UpdateVideoInParams m_pBridgePartyIn is Null", m_partyConfName);
	}

}

////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateVideoOutParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams)
{
	//Currently there are no states that we wait on that might require handle in state machine
	//We may had in the future
	if(m_pBridgePartyOut)
		((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->UpdateVideoParams(pBridgePartyVideoRelayParams);
	else
	{
		PASSERT(1);
		PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::UpdateVideoOutParams m_pBridgePartyIn is Null", m_partyConfName);
	}
}

/////////////////////////////////////////////////////////////////////////////
BOOL CVideoRelayBridgePartyCntl::IsPcmOvrlyMsgOn()
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::IsPcmOvrlyMsgOn PCM not supported for relay", m_partyConfName);
	return FALSE;

}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SetPcmOvrlyMsgOn(BOOL i_bIsPcmOvrlyMsg)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::SetPcmOvrlyMsgOn PCM not supported for relay", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////////
CLayout* CVideoRelayBridgePartyCntl::GetReservationLayout(void) const
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::GetReservationLayout Layout is not relevant for video relay party", m_partyConfName);
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////

CLayout* CVideoRelayBridgePartyCntl::GetCurrentLayout(void)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::GetCurrentLayout Layout is not relevant for video relay party", m_partyConfName);
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////

CLayout* CVideoRelayBridgePartyCntl::GetPrivateReservationLayout(void) const
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::GetCurrentLayout Layout is not relevant for video relay party", m_partyConfName);
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
WORD CVideoRelayBridgePartyCntl::GetIsPrivateLayout() const
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::GetIsPrivateLayout Layout is not relevant for video relay party", m_partyConfName);
	return 0;

}
/////////////////////////////////////////////////////////////////////////////
CVisualEffectsParams* CVideoRelayBridgePartyCntl::GetPartyVisualEffects(void) const
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::CVideoRelayBridgePartyCntl Visual effects is not relevant for video relay party", m_partyConfName);
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////

const CImage* CVideoRelayBridgePartyCntl::GetPartyImage() const
{
  return (m_pBridgePartyIn) ? ((CBridgePartyVideoRelayIn*)m_pBridgePartyIn)->GetPartyImage() : NULL;
}

///////////////////////////////////////////////////////////////////////////
ePartyLectureModeRole CVideoRelayBridgePartyCntl::GetPartyLectureModeRole() const
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::GetPartyLectureModeRole currently LM is not relevant for video relay party", m_partyConfName);
	return eREGULAR;
}
///////////////////////////////////////////////////////////////////////////
BYTE  CVideoRelayBridgePartyCntl::IsImageInPartiesLayout(const CImage* pImage)
{
	//KEREN TODO
	//Need to call bridgepartyvideorelayout...
	return FALSE;//TMP
}

///////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateSelfMute(RequestPriority who, EOnOff eOnOff)
{
  if (m_pBridgePartyIn)
  {
	((CBridgePartyVideoRelayIn*)m_pBridgePartyIn)->UpdateSelfMute(who, eOnOff);

    DWORD updateStatus;
    switch (who)
    {
      case MCMS_Prior    : { updateStatus = (eOnOff == eOff) ? 0x0F00000E : 0x0F000010; break; }
      case OPERATOR_Prior: { updateStatus = (eOnOff == eOff) ? 0x0000000E : 0x00000010; break; }
      case PARTY_Prior   : { updateStatus = (eOnOff == eOff) ? 0xF000000E : 0xF0000010; break; }
      default            : { return;   /* no need to send updateDB */ }
    }
    m_pConfApi->UpdateDB(m_pParty, MUTE_STATE, updateStatus);
    return;
  }
  PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateSelfMute - Failed 'm_pBridgePartyIn' is invalid, ", m_partyConfName);
}

///////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::FastUpdate(void)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::FastUpdate for video relay we dont support fastupdate but different flow", m_partyConfName);
}

//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SetSiteName(const char* visualName)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::SetSiteName not relevant for video relay", m_partyConfName);

}

//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateVideoInParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateVideoInParams not relevant for video relay, we should have called UpdateVideoInParams(CBridgePartyVideoRelayMediaParams* )", m_partyConfName);
	PASSERT(100);
}

//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateVideoOutParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateVideoOutParams not relevant for video relay, we should have called UpdateVideoOutParams(CBridgePartyVideoRelayMediaParams* )", m_partyConfName);
	PASSERT(100);

}

//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdatePartyTelePresenceMode(eTelePresencePartyType partyNewTelePresenceMode)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::UpdatePartyTelePresenceMode TelePresenceMode not supported for relay", m_partyConfName);
}


//////////////////////////////////////////////////////////////////////////
//void CVideoRelayBridgePartyCntl::IvrCommand(OPCODE opcode, CSegment* pDataSeg)
//{
//	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::IvrCommand IVR currently not supported for relay", m_partyConfName);
//
//}
//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::IvrNotification(OPCODE opcode, CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::IvrNotification IVR currently not supported for relay", m_partyConfName);

}
//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::PLC_SetPartyPrivateLayout(LayoutType newPrivateLayoutType)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::PLC_SetPartyPrivateLayout PLC not supported for relay", m_partyConfName);
}

//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::PLC_PartyReturnToConfLayout()
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::PLC_SetPartyPrivateLayout PLC not supported for relay", m_partyConfName);

}
//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::PLC_ForceToCell(char* partyImageToSee, BYTE cellToForce)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::PLC_SetPartyPrivateLayout PLC not supported for relay", m_partyConfName);

}
//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::PLC_CancelAllPrivateLayoutForces()
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::PLC_CancelAllPrivateLayoutForces PLC not supported for relay", m_partyConfName);

}
//////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::VENUS_SetPartyPrivateLayout(LayoutType newPrivateLayoutType)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::VENUS_SetPartyPrivateLayout not supported for relay", m_partyConfName);

}
//////////////////////////////////////////////////////////////////////////
void  CVideoRelayBridgePartyCntl::PCMNotification(OPCODE opcode, CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::PCMNotification not supported for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void  CVideoRelayBridgePartyCntl::ChangeConfLayout(CLayout* pConfLayout, BYTE bAnyway)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::ChangeConfLayout no conf layout for relay", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::ChangePartyLayout(CVideoLayout& newVideoLayout)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::ChangePartyLayout no party layout for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::ChangePartyPrivateLayout(CVideoLayout& newVideoLayout)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::ChangePartyPrivateLayout no party layout for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::ChangeLayoutPrivatePartyButtonOnly(WORD isPrivate)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::ChangePartyPrivateLayout no party layout for relay", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateVisualEffects(CVisualEffectsParams* pVisualEffects)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateVisualEffects Visual effects is not relevant for video relay party", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::ForwardVINToParty(WORD mcuNumber, WORD terminalNumber)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::ForwardVINToParty FECC currently not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
LayoutType  CVideoRelayBridgePartyCntl::GetConfLayoutType() const
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::GetConfLayoutType conf layout not relevant for relay", m_partyConfName);
	return CP_NO_LAYOUT;

}

/////////////////////////////////////////////////////////////////////////
LayoutType  CVideoRelayBridgePartyCntl::GetPartyCurrentLayoutType() const
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::GetPartyCurrentLayoutType party layout not relevant for relay", m_partyConfName);
	return CP_NO_LAYOUT;
}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SetPartyLectureModeRole(ePartyLectureModeRole partyLectureModeRole)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::CVideoRelayBridgePartyCntl LM currently not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SendMsgToScreenPerParty(CTextOnScreenMngr* TextMsgList, DWORD timeout)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::SendMsgToScreenPerParty not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::StopMsgToScreenPerParty()
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::StopMsgToScreenPerParty not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateDecoderDetectedMode()
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateDecoderDetectedMode not relevant for relay", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::GetRsrcProbAdditionalInfoOnVideoTimerSetup(BYTE& failureCauseDirection, BYTE& failureCauseAction)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::GetRsrcProbAdditionalInfoOnVideoTimerSetup not relevant for relay", m_partyConfName);
	failureCauseDirection = eMipNoneDirction;
	failureCauseAction    = eMipNoAction;
}

/////////////////////////////////////////////////////////////////////////
bool CVideoRelayBridgePartyCntl::IsBridgePartyVideoOutStateIsConnected()
{
	if (!m_pBridgePartyOut)
	    return false;
	return ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->IsStateIsConnected();

}
/////////////////////////////////////////////////////////////////////////

void CVideoRelayBridgePartyCntl::DisplayGatheringOnScreen(CGathering* pGathering)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::DisplayGatheringOnScreen not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::SetVisualEffectsParams not relevant for relay", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateAutoScanOrder(CAutoScanOrder* pAutoScanOrder)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::SetVisualEffectsParams not relevant for relay", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::StartMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::StartMessageOverlay not relevant for relay", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateMessageOverlay(CMessageOverlayInfo* pMessageOverlayInfo)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::StartMessageOverlay not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateVideoClarity(WORD isVideoClarity)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateVideoClarity not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateAutoBrightness(WORD isAutoBrightness)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateAutoBrightness not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateMessageOverlayStop()
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateMessageOverlayStop not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateSiteNameInfo(CSiteNameInfo* pSiteNameInfo)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::UpdateMessageOverlayStop not relevant for relay", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::RefreshLayout()
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::RefreshLayout not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::StartConnectProcess()
{
	  if ((m_pUpdatePartyInitParams->GetMediaInParams()))
	  {
	    if (!m_pBridgePartyIn)
	    {
	      CreatePartyIn();
	      SetTelepresenceInfo(((CBridgePartyVideoInParams*)m_pUpdatePartyInitParams->GetMediaInParams())->GetTelePresenceEPInfo());
	    }
	    else
	      ((CBridgePartyVideoRelayIn*)(m_pBridgePartyIn))->UpdatePartyInParams(((CUpdatePartyVideoRelayInitParams*)m_pUpdatePartyInitParams));//Keren TODO
	    //Update DB when connecting muted party
	    CheckIsMutedVideoInAndUpdateDB();
	  }

	  if ((m_pUpdatePartyInitParams->GetMediaOutParams()))
	  {
	    if (!m_pBridgePartyOut)
	    {
	      CreatePartyOut();
	      if (!(m_pUpdatePartyInitParams->GetMediaInParams()))
	    	  SetTelepresenceInfo(((CBridgePartyVideoOutParams*)m_pUpdatePartyInitParams->GetMediaOutParams())->GetTelePresenceEPInfo());
	    }
	    else
	      ((CBridgePartyVideoRelayOut*)(m_pBridgePartyOut))->UpdatePartyOutParams(((CUpdatePartyVideoRelayInitParams*)m_pUpdatePartyInitParams));//Keren TODO
	  }
	  BYTE isIvr;
	  isIvr = m_pUpdatePartyInitParams->GetIsIVR();
	  Connect(isIvr);
	  POBJDELETE(m_pUpdatePartyInitParams);
}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::CreatePartyIn()
{
	  NewPartyIn();
     ((CBridgePartyVideoRelayIn*)(m_pBridgePartyIn))->Create(this,m_pUpdatePartyInitParams->GetMediaInParams(), NULL);
}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::CreatePartyOut()
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, m_partyRsrcID, m_confRsrcID);

	NewPartyOut();

	std::auto_ptr<CTaskApi> pTaskApiVideoOut(new CTaskApi(*m_pConfApi));
	pTaskApiVideoOut->CreateOnlyApi(m_pConfApi->GetRcvMbx(), m_pBridgePartyOut);
	CRsrcDesc* pRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(m_partyRsrcID, /*KERN TODO*/eLogical_video_encoder, pTaskApiVideoOut.get());
	if (!pRsrcDesc)   // Entry not found in Routing Table
	{
	   POBJDELETE(m_pBridgePartyOut);
	   PASSERT(1);
	}
	else
	{
	   rsrcParams.SetRsrcDesc(*pRsrcDesc);
	   ((CBridgePartyVideoRelayOut*)(m_pBridgePartyOut))->Create(this, &rsrcParams, m_pUpdatePartyInitParams->GetMediaOutParams());//Keren TODO
	}
}

/////////////////////////////////////////////////////////////////////////
DWORD CVideoRelayBridgePartyCntl::GetOutVideoRate()
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::GetOutVideoRate not relevant for relay", m_partyConfName);
	return 0;

}
/////////////////////////////////////////////////////////////////////////
DWORD CVideoRelayBridgePartyCntl::GetLastVideoInSyncStatus() const
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::GetLastVideoInSyncStatus not relevant for relay", m_partyConfName);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::ChangeSpeakerNotationForPcmFecc(DWORD imageId)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::ChangeSpeakerNotationForPcmFecc not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
BYTE CVideoRelayBridgePartyCntl::IsPartyIntraSuppressed()
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::IsPartyIntraSuppressed currently not relevant for relay", m_partyConfName);
	return false;
}

/////////////////////////////////////////////////////////////////////////
bool CVideoRelayBridgePartyCntl::IsIntraSuppressEnabled(WORD intra_suppression_type) const
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::IsIntraSuppressEnabled currently not relevant for relay", m_partyConfName);
	return false;
}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::EnableIntraSuppress(WORD intra_suppression_type)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::EnableIntraSuppress currently not relevant for relay", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::DisableIntraSuppress(WORD intra_suppression_type)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::EnableIntraSuppress currently not relevant for relay", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////
WORD CVideoRelayBridgePartyCntl::IsValidState(WORD state) const
{
  WORD valid_state = TRUE;
  switch (state)
  {
    case EXPORT:
    case ALLOCATE:
    case DEALLOCATE:
    case DISCONNECTED:
    {
      valid_state = FALSE;
      PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::IsValidState - Failed, invalid state, party:", m_partyConfName);
      PASSERT(1);
      break;
    }
  } // switch

  return valid_state;
}
// ------------------------------------------------------------------------------------------
WORD CVideoRelayBridgePartyCntl::IsValidEvent(OPCODE event) const
{
  WORD valid_event = TRUE;
  switch (event)
  {
    case VIDEO_EXPORT:
    case VIDEO_IN_SYNCED:
    case FASTUPDATE:
    case END_CHANGE_LAYOUT:
    case CHANGEPARTYLAYOUT:
    case CHANGEPARTYPRIVATELAYOUT:
    case SETPARTYPRIVATELAYOUTONOFF:
    case UPDATEVISUALEFFECTS:
    case VIDEO_GRAPHIC_OVERLAY_START_REQ:
    case VIDEO_GRAPHIC_OVERLAY_STOP_REQ:
    case PLC_SETPARTYPRIVATELAYOUTTYPE:
    case PLC_RETURNPARTYTOCONFLAYOUT:
    case PLC_FORCECELLZERO:
    case PLC_CANCELALLPRIVATELAYOUTFORCES:
    case UPDATE_PARTY_LECTURE_MODE_ROLE:
    case VIDREFRESH:
    case SEND_H239_VIDEO_CAPS:
    case VIDEO_DECODER_SYNC:
    case PARTYLAYOUTCHANGED:
    case PRIVATELAYOUT_ONOFF_CHANGED:
    case VIDEO_BRDG_PARTY_SETUP_TOUT:
    case VIDEO_BRDG_PARTY_DISCONNECT_TOUT:
    case SET_SITE_AND_VISUAL_NAME:
    case UPDATE_DECODER_DETECTED_MODE:
    case DISPLAY_TEXT_ON_SCREEN:
    case UPDATE_VIDEO_CLARITY:
    case UPDATE_AUTO_BRIGHTNESS:
    case CHANGE_SPEAKER_NOTATION_PCM_FECC:
    case VIDEO_BRDG_COP_PARTY_INTRA_SUPPRESS_TOUT:
    case VIDEO_BRDG_COP_ENCODER_IGNORE_INTRA_TOUT:
    case INDICATION_ICONS_CHANGE:
     {
      valid_event = FALSE;
      TRACEINTO << "CVideoRelayBridgePartyCntl::IsValidEvent - Failed, not valid event, party name:" << m_partyConfName << " opcode:" << event;
      PASSERT(1);
      break;
    }
  } // switch

  return valid_event;
}
/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::Setup()
{
	// In case we are in CONNECTED state and the second video direction start to connect -
	// will remain in CONNECTED state.
	  if (m_state != CONNECTED)
	    m_state = SETUP;

	  // If video IN is not connected or in Setup state...
	  bool isConnectBridgePartyIn = (m_pBridgePartyIn && !m_pBridgePartyIn->IsConnected() && !m_pBridgePartyIn->IsConnecting());

	  // If video OUT is not connected or in Setup state...
	  bool isConnectBridgePartyOut = (m_pBridgePartyOut && !m_pBridgePartyOut->IsConnected() && !m_pBridgePartyOut->IsConnecting());

	  //Currently we dont add timer the event of connection will be after we will receive update for in and out from partycntl after the Signaling neg.
	  //StartTimer(VIDEO_BRDG_PARTY_SETUP_TOUT, VIDEO_BRDG_PARTY_SETUP_TOUT_VALUE);

	  if (isConnectBridgePartyIn)
	    m_pBridgePartyIn->Connect();

	  if (isConnectBridgePartyOut)
	    m_pBridgePartyOut->Connect();

}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::VideoConnectionCompletion(CSegment* pParams, EMediaDirection eConnectedMediaDirection)
{
	PASSERT_AND_RETURN(!m_pBridgePartyIn && !m_pBridgePartyOut);

	EStat           receivedStatus             = statOK;
	EMediaDirection ConnectedDirection         = eNoDirection;
	BOOL            isVideoConnectionCompleted = FALSE;

	*pParams >> (BYTE&)receivedStatus;

	if (statOK != receivedStatus)
	{
		// DeleteTimer(VIDEO_BRDG_PARTY_SETUP_TOUT);  // xxx Amir do we need it?

		DumpMcuInternalProblemDetailed((BYTE)eConnectedMediaDirection, eMipStatusFail, eMipVideo);

		// Inform Video Bridge about connection failure
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, receivedStatus, FALSE, eNoDirection, pParams);
		TRACEINTO <<  " Video Connection Failed, Party Name: " << m_partyConfName << ", Status:" << receivedStatus << " - Failed, because received invalid status";
		return;
	}


	// Video-in is connected
	if (eMediaIn == eConnectedMediaDirection)
	{
	    if (IsUniDirectionConnection(eMediaIn))
	      isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaIn;
	    else if (m_pBridgePartyOut && (m_pBridgePartyOut->IsConnected() || m_pBridgePartyOut->IsDisconnecting()))  // vngr-19742 in case we started to open both but one side is disconnecting we wont receive that its connected but disconnected thus we need to finish the connection of the party(the other side that finished to connect)
	      isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaInAndOut;
	 }

	  // Video-out is connected
	 if (eMediaOut == eConnectedMediaDirection)
	 {
	    if (IsUniDirectionConnection(eMediaOut))
	      isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaOut;
	    else if (m_pBridgePartyIn && (m_pBridgePartyIn->IsConnected() || m_pBridgePartyIn->IsDisconnecting()))     // vngr-19742 in case we started to open both but one side is disconnecting we wont receive that its connected but disconnected thus we need to finish the connection of the party(the other side that finished to connect)
	      isVideoConnectionCompleted = TRUE, ConnectedDirection = eMediaInAndOut;
	  }

	  TRACEINTO << "CVideoBridgePartyCntl::VideoConnectionCompletion - " << m_partyConfName << ", isVideoConnectionCompleted:" << (int)isVideoConnectionCompleted << ", ConnectedDirection:" << ConnectedDirection;

	  if (TRUE == isVideoConnectionCompleted)
	  {

		  m_state = CONNECTED;

	    // Inform Video Bridge
	    //m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE, ConnectedDirection);
		 //In the video relay flow we inform the VB that the party end connected without changing to connected, we dont need to wait for OPEN/CONNECT we change to connected after the SDP neg
	  }
}

/////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::VideoDisConnectionCompletion(CSegment* pParams, EMediaDirection eDisConnectedMediaDirection)
{
	PASSERT_AND_RETURN(!m_pBridgePartyIn && !m_pBridgePartyOut);

	  BOOL            isVideoDisConnectionCompleted = FALSE;
	  BOOL            isHalfDisconnection           = FALSE;
	  EStat           receivedStatus                = statOK;
	  EMediaDirection ConnectedDirection            = eNoDirection;

	  BYTE videoOutClosePortStatus                  = statOK;
	  BYTE videoInClosePortStatus                   = statOK;

	  *pParams >> (BYTE&)receivedStatus;

	  // *** 1. Check if this is FULL disconnection or not - Full disconnection is when all connected directions disconnects!!! ****

	  // Video-in is disconnected
	  if (eMediaIn == eDisConnectedMediaDirection)
	  {
	    // Check if only Video in was connected or if both directions were connected but video-out was already disconnected.
	    if (IsUniDirectionConnection(eMediaIn) || (m_pBridgePartyOut && m_pBridgePartyOut->IsDisConnected()))
	      isVideoDisConnectionCompleted = TRUE;
	  }

	  // Video-out is disconnected
	  if (eMediaOut == eDisConnectedMediaDirection)
	  {
	     // Check if only Video out was connected or if both directions were connected but video-in was already disconnected.
	    if (IsUniDirectionConnection(eMediaOut) || (m_pBridgePartyIn && m_pBridgePartyIn->IsDisConnected()))
	      isVideoDisConnectionCompleted = TRUE;


	  }

	  // *** 2. If this is full disconnection  ***

	  if (TRUE == isVideoDisConnectionCompleted)
	  {
	    ConnectedDirection = eNoDirection;
	    m_state            = IDLE;

	    SetDisConnectingDirectionsReq(eNoDirection);


	    // for debug info in case of the "MCU internal problem"
	    BYTE failureCauseDirection = eMipNoneDirction;


	    // Inform Video Bridge In case of problem
	    if (statVideoInOutResourceProblem == receivedStatus)
	    {
	      CSegment* pSeg = new CSegment;
	      *pSeg << (BYTE)receivedStatus << (BYTE)failureCauseDirection << (BYTE)eMipStatusFail << (BYTE)eMipClose;

	      if (GetConnectionFailureCause() == statOK)
	        DumpMcuInternalProblemDetailed((BYTE)failureCauseDirection, eMipStatusFail, eMipVideo);

	      m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection, pSeg);
	      POBJDELETE(pSeg);
	    }
	    else  // In case we received connect req while disconnecting
	    {
	      if (m_pUpdatePartyInitParams)
	      {
	        PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::VideoDisConnectionCompletion - Both direction were disconnected, now start connection process, ", m_partyConfName);
	        StartConnectProcess();
	      }
	      else
	      {
	        PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::VideoDisConnectionCompletion - Both direction were disconnected, now state is IDLE, ", m_partyConfName);
	        DestroyPartyInOut(ConnectedDirection);

	        // Inform Video Bridge - Add the direction connected state
	        m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection);
	      }
	    }
	  }

	  // *** 3. If this is not full disconnection we need to check if half disconnection
	  // (according to the disconnection req we received from partyCntl)   ****

	  else    // Incase both directio were connected or in setup and only one direction was disconnected ,according to the disconnect direction we recieved from the partyCntl
	  {
	    BYTE failureCauseDirection = eMipNoneDirction;

	    // Incase VideoOut disconnected
	    if (m_pBridgePartyOut && (eDisConnectedMediaDirection == eMediaOut) && (m_pBridgePartyIn && ((m_pBridgePartyIn->IsConnected()) || (m_pBridgePartyIn->IsConnecting()))) && GetDisconnectingDirectionsReq() == eMediaOut)
	    {
	      isHalfDisconnection     = TRUE;
	      ConnectedDirection      = eMediaIn;

	    }
	    // Incase VideoIn disconnected
	    else if (m_pBridgePartyIn && (eDisConnectedMediaDirection == eMediaIn) && (m_pBridgePartyOut && ((m_pBridgePartyOut->IsConnected()) || (m_pBridgePartyOut->IsConnecting()))) && GetDisconnectingDirectionsReq() == eMediaIn)
	    {
	      isHalfDisconnection    = TRUE;
	      ConnectedDirection     = eMediaOut;

	    }

	    if (isHalfDisconnection)
	    {
	      DeleteTimer(VIDEO_BRDG_PARTY_DISCONNECT_TOUT);
	      SetDisConnectingDirectionsReq(eNoDirection);

	     // if received connect req while disconnecting
	     if (m_pUpdatePartyInitParams)
	     {
	          PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::VideoDisConnectionCompletion - Start connection process, ", m_partyConfName);

	          DestroyPartyInOut(ConnectedDirection);
	          StartConnectProcess();
	      }
	      else
	      {
	         PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::VideoDisConnectionCompletion - Disconnected only one direction, the state remain the same, ", m_partyConfName);

	          DestroyPartyInOut(ConnectedDirection);

	          // Inform Video Bridge - Add the direction connected state
	          m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, receivedStatus, FALSE, ConnectedDirection);
	       }
	     }
	  }
}

///////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::AddImageToConfMix()
{
	 // Send Msg To Video Bridge
	  m_pConfApi->AddVideoRelayImageToMix(m_pParty, VIDEO_BRIDGE_MSG, statOK);
}
///////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateVideoBridgeOnRelayImageSvcToAvcTranslated()
{
	 // Send Msg To Video Bridge
	  m_pConfApi->UpdateOnRelayImageSvcToAvcTranslated(m_pParty, VIDEO_BRIDGE_MSG, statOK);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::AskEpsForIntra(AskForRelayIntra& epIntraParams)
{
	((CVideoBridgeCP*)m_pBridge)->AskRelayEpsForIntra(epIntraParams);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CVideoRelayBridgePartyCntl::EpAskForIntra(const RelayIntraParam& intraParam)
{
	CSegment seg;
	intraParam.Serialize(&seg);
	DispatchEvent(RELAY_ENDPOINT_ASK_FOR_INTRA, &seg);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SendIntraRequestToParty(const std::list<unsigned int>& listSsrc,bool bIsGDR, bool allowSuppression/* = false*/)
{
	RelayIntraParam intraParam, intraParamToSend;
	intraParam.m_partyRsrcId = m_partyRsrcID;
	intraParam.m_listSsrc = listSsrc;
	intraParam.m_bIsGdr = bIsGDR;

    std::string  strParams =  CStlUtils::ContainerToString(intraParam.m_listSsrc);
    COstrStream msg;
    msg << "DEBUG_INTRA: partyRsrcId=" << intraParam.m_partyRsrcId << ",isGDR="<<intraParam.m_bIsGdr<<", size=" << intraParam.m_listSsrc.size() << ", list=" << strParams.c_str() << ", allow intra suppression if relevant:" << allowSuppression;

//	CSegment* pSeg = new CSegment;

	bool need_timer_start = UpdateIntraSuppression(intraParam, intraParamToSend, allowSuppression);

	if (intraParamToSend.m_partyRsrcId == (PartyRsrcID)-1)
	{
		msg << "\n===>intra request suppressed, requested from party ID " << m_partyRsrcID;
		return;
	}
	else
		msg << "\n===>sending intra request with " << intraParamToSend.m_listSsrc.size() << " ssrc's";

	TRACEINTO << msg.str();

	SendIntraRequestToPartyWithSuppressionTimer(intraParamToSend, need_timer_start);

}

void CVideoRelayBridgePartyCntl::SendIntraRequestToPartyWithSuppressionTimer(RelayIntraParam& intraParam, bool bNeedTimer)
{
	COstrStream msg;

	DWORD size_listSsrc = intraParam.m_listSsrc.size();
	msg << " sending intra request for " << size_listSsrc << "  ssrc's - ";
	if (size_listSsrc > 0)
	{
		std::list<unsigned int>::const_iterator it = intraParam.m_listSsrc.begin();
		for ( ; it != intraParam.m_listSsrc.end(); ++it)
			msg << *it << " ";
		if ((m_intraSuppressionDurationInMilliseconds != 0) && bNeedTimer)
		{
			CSegment* pTimerSeg = new CSegment;
			*pTimerSeg << size_listSsrc;
			for (it = intraParam.m_listSsrc.begin() ; it != intraParam.m_listSsrc.end(); ++it)
				*pTimerSeg << *it;

			msg << " suppression timer started for the ssrc's, duration:" << m_intraSuppressionDurationInMilliseconds << " milliseconds";;
			StartTimer(RELAY_EP_INTRA_SUPRESSION_TIMER, m_intraSuppressionDurationInMilliseconds/10, pTimerSeg);
		}
		else
			msg << " timer not needed - suppression timer not started!";
	}
	else
		PASSERTMSG(TRUE,  "request with 0 ssrc's being sent!");
	TRACEINTOFUNC << msg.str();
	CSegment* pSeg = new CSegment;
	intraParam.Serialize(pSeg);

	m_pPartyApi->AskRelayEndPointForIntra(pSeg);
}

bool CVideoRelayBridgePartyCntl::UpdateIntraSuppression(RelayIntraParam& intraParamsReceived, RelayIntraParam& intraParamsToSend, bool allowSuppression)
{
	bool needToSend = false;
	std::list<unsigned int>::const_iterator ssrc_it = intraParamsReceived.m_listSsrc.begin();
	SsrcSupressedIntraParams::iterator intra_it;

	// initialize outgoing intra request with full params
	intraParamsToSend = intraParamsReceived;

	// clear ssrc list and ID. these will be filled only once we check they should not be suppressed.
	intraParamsToSend.m_listSsrc.clear();
	intraParamsToSend.m_partyRsrcId = (PartyRsrcID)-1;
	COstrStream  str;
	bool is_suppression_enabled = m_intraSuppressionDurationInMilliseconds > 0;

	// check if a timer has already been activated (some intra's already suppressed). if it has, we won't need to start a new suppression timer, we'll wait untill the current one expires
	bool need_new_timer = true;
	bool new_suppressions_activated = false;
	SsrcSupressedIntraParams::iterator sit;
	for (sit=m_SsrcSuppressedIntras.begin(); need_new_timer && sit != m_SsrcSuppressedIntras.end(); ++sit)
	{
		IntraSuppressionParams& suppressedIntra = sit->second;
		if (suppressedIntra.m_bIsSuppressionActive)
			need_new_timer = false;
	}
	if (!is_suppression_enabled)
		str << " SVC intra suppression not used. clearing suppression for ssrc's.";
	str << " is suppression timer already in effect for any requested ssrc: " << !need_new_timer;
	// check ssrc's in the received intra request for existing suppression
	for ( ; ssrc_it != intraParamsReceived.m_listSsrc.end(); ++ssrc_it)
	{

		str << ".  received intra for ssrc " << *ssrc_it;
		if ((intra_it = m_SsrcSuppressedIntras.find(*ssrc_it)) != m_SsrcSuppressedIntras.end())
		{
			IntraSuppressionParams& suppressedIntra = intra_it->second;
			if (!is_suppression_enabled)
			{
				str << ". clearing intra suppression. (intra will be sent)";
				suppressedIntra.ClearSuppression();
				needToSend = true;
				intraParamsToSend.m_listSsrc.push_back((unsigned int)*ssrc_it);
				continue;
			}
			str  << " is already suppressing:" <<  suppressedIntra.m_bIsSuppressionActive
					  << " have requests been received during suppression: " << suppressedIntra.m_bIsRequestReceivedDuringSuppression;
			// ssrc  found to be currently suppressed
			if (suppressedIntra.m_bIsSuppressionActive && allowSuppression)
			{
				str  << "\n===> intras for ssrc " << suppressedIntra.m_ssrc << " now suppressed!";
				suppressedIntra.m_bIsRequestReceivedDuringSuppression = true;
				suppressedIntra.m_bIsGdr &= intraParamsReceived.m_bIsGdr;
				// GUY_TODO - how to handle different isSsrc in accumulated intra requests
				suppressedIntra.m_bIsSsrc &= intraParamsReceived.m_bIsSsrc;
			}
			// either not currently suppressed or we don't allow suppression for this flow: either way, SEND! (and activate suppression for next incoming intra)
			else
			{
				str  << "\n===> suppression time for ssrc " << suppressedIntra.m_ssrc << " has been reset to " << m_intraSuppressionDurationInMilliseconds << " from now.";
				needToSend = true;
				intraParamsToSend.m_listSsrc.push_back((unsigned int)*ssrc_it);
				suppressedIntra.ActivateSuppression(m_intraSuppressionDurationInMilliseconds);
				new_suppressions_activated = true;
			}
		}
		// first intra request for this ssrc - we send the request and activate suppression
		else
		{
			str << ". intra request is for new ssrc ";
			needToSend = true;
			intraParamsToSend.m_listSsrc.push_back((unsigned int)*ssrc_it);
			IntraSuppressionParams newSuppressedIntra(*ssrc_it, intraParamsReceived.m_bIsGdr, intraParamsReceived.m_bIsSsrc);
			if (is_suppression_enabled)
			{
				new_suppressions_activated = true;
				newSuppressedIntra.ActivateSuppression(m_intraSuppressionDurationInMilliseconds);
			}
			else
				newSuppressedIntra.ClearSuppression();
			m_SsrcSuppressedIntras.insert(std::make_pair(newSuppressedIntra.m_ssrc, newSuppressedIntra));
		}
	}

	TRACEINTOFUNC << str.str();
	// need to send- update the rsrc id (if there's no need to send, rsrc id remains -1)
	if (needToSend)
		intraParamsToSend.m_partyRsrcId = m_partyRsrcID;
	return (need_new_timer && new_suppressions_activated);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionSETUP(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionSETUP ", m_partyConfName);
	OnTimerIntraSuppression(pParam);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionCONNECTED ", m_partyConfName);
	OnTimerIntraSuppression(pParam);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionCONNECTED_STANDALONE(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionCONNECTED_STANDALONE ", m_partyConfName);
	OnTimerIntraSuppression(pParam);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnTimerIntraSuppressionDISCONNECTING ignore disconnecting", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnTimerIntraSuppression(CSegment* pParam)
{
	std::list<unsigned int> timed_out_ssrc_list;
	DWORD size_list = 0;
	(*pParam) >> size_list;

	SsrcSupressedIntraParams::iterator intra_it;
	DWORD closest_upcoming_expiration, currentTime = SystemGetTickCount().GetMiliseconds();
	bool bIsUsingIntraSuppression = m_intraSuppressionDurationInMilliseconds != 0;
	// start with longest duration
	closest_upcoming_expiration = currentTime + m_intraSuppressionDurationInMilliseconds;
	COstrStream msg;
	for (DWORD i=0; i < size_list; ++i)
	{
		unsigned int ssrc = 0;
		(*pParam) >> ssrc;
		intra_it = m_SsrcSuppressedIntras.find(ssrc);
		if (intra_it == m_SsrcSuppressedIntras.end())
		{
			PASSERTSTREAM(TRUE, " could not find ssrc " << ssrc << " on CVideoRelayBridgePartyCntl::OnTimerIntraSuppression");
			continue;
		}
		else
		{
			IntraSuppressionParams& suppressedIntraSsrc = intra_it->second;
			if (suppressedIntraSsrc.m_bIsSuppressionActive && suppressedIntraSsrc.m_bIsRequestReceivedDuringSuppression)
			{
				msg << "ssrc " << ssrc << " found to have suppressed intras. sending intra and reactivating suppression";
				// we send a separate intra with its own RelayIntraParam because the Gdr/Ssrc values might be different for each
				RelayIntraParam intraParam;
				intraParam.m_partyRsrcId = m_partyRsrcID;
				intraParam.m_bIsGdr = suppressedIntraSsrc.m_bIsGdr;
				intraParam.m_bIsSsrc = suppressedIntraSsrc.m_bIsSsrc;
				intraParam.m_listSsrc.push_back(ssrc);
				SendIntraRequestToPartyWithSuppressionTimer(intraParam, false);	// we will check if the timer needs to be reactivated at the end of this function.
				// since we now sent an intra request for this ssrc, we will mark it as suppressed (if suppression is used).
				if (bIsUsingIntraSuppression)
					suppressedIntraSsrc.ActivateSuppression(m_intraSuppressionDurationInMilliseconds);
			}
			else
			{
				msg << "ssrc " << ssrc << " found without suppressed intras. intra suppression cleared";
				suppressedIntraSsrc.ClearSuppression();
			}
		}
	}

	// check which intra suppression is coming next and activate the suppression timer forremaining duration
	SsrcSupressedIntraParams::iterator sit;
	std::list<unsigned int> upcoming_ssrc_list;
	msg << "\nchecking for closest ssrc expirations, starting point (max expitation) is " << closest_upcoming_expiration;

	// check upcoming suppression timers- vailidty and nearest timer
	for (sit=m_SsrcSuppressedIntras.begin(); sit != m_SsrcSuppressedIntras.end(); ++sit)
	{
		IntraSuppressionParams& suppressedIntra = sit->second;
		if (suppressedIntra.m_bIsSuppressionActive /*&& suppressedIntra.m_bIsRequestReceivedDuringSuppression*/)
		{
			msg << "\nssrc " << suppressedIntra.m_ssrc << " suppressed until " << suppressedIntra.m_suppressionExpirationTime;
			// might be more than one ssrc that is suppressed until the same time (if they received intra request together):
			// to make sure we aren't dealing with junk values we check with SVC_INTRA_MAX_SUPPRESSION_DURATION
			//bool sanity_check  =  (suppressedIntra.m_suppressionExpirationTime >= currentTime) && ((DWORD)(suppressedIntra.m_suppressionExpirationTime - currentTime) < SVC_INTRA_MAX_SUPPRESSION_DURATION_MS);
			int diff = abs((int)(suppressedIntra.m_suppressionExpirationTime - currentTime));
			bool sanity_check  = diff < SVC_INTRA_MAX_SUPPRESSION_DURATION_MS;
			if (!sanity_check)
			{
				PASSERTSTREAM(!sanity_check, "current ssrc suppression expiration value appears corrupted. clearing suppression for ssrc" << suppressedIntra.m_ssrc);
				suppressedIntra.ClearSuppression();
				continue;
			}
			// find the most closest upcoming supression timeout
			if (suppressedIntra.m_suppressionExpirationTime > 0 && suppressedIntra.m_suppressionExpirationTime < closest_upcoming_expiration  )
				closest_upcoming_expiration = suppressedIntra.m_suppressionExpirationTime;
		}
		else
			msg << "\nssrc " << suppressedIntra.m_ssrc << " not suppressed";
	}

	// collect ssrc's who's timers are to expire within the 50ms range of the nearest timer
	for (sit=m_SsrcSuppressedIntras.begin(); sit != m_SsrcSuppressedIntras.end(); ++sit)
	{
		IntraSuppressionParams& suppressedIntra = sit->second;
		int diff = abs((int)(suppressedIntra.m_suppressionExpirationTime - closest_upcoming_expiration));
		if ((suppressedIntra.m_bIsSuppressionActive) && (suppressedIntra.m_suppressionExpirationTime > 0) && (diff <= 50 /* 50 ms is close enough */ ))
			upcoming_ssrc_list.push_back(suppressedIntra.m_ssrc);
	}

	// send timer for relevant ssrc's
	if (upcoming_ssrc_list.size() > 0)
	{
		msg << "\nsuppression timer started for sent ssrc's -";
		CSegment* pTimerSeg = new CSegment;
		DWORD size_listSsrc = upcoming_ssrc_list.size();
		*pTimerSeg << size_listSsrc;
		std::list<unsigned int>::const_iterator it;
		for (it = upcoming_ssrc_list.begin() ; it != upcoming_ssrc_list.end(); ++it)
		{
			*pTimerSeg << *it;
			msg << *it << " ";
		}
		DWORD timer_ms = closest_upcoming_expiration - currentTime;
		msg << ", duration:" << timer_ms << " milliseconds";
		StartTimer(RELAY_EP_INTRA_SUPRESSION_TIMER, timer_ms/10, pTimerSeg);
	}
	else
		msg << " no upcoming supression timeouts- timer not restarted. waiting for next intra...";

	TRACEINTOFUNC << msg.str();


}
//void CVideoRelayBridgePartyCntl::SendVideoScpNotificationRequestToPartyCntl(CScpNotificationWrapper& scpNotifyReq)
//{
//	scpNotifyReq.Dump(" DEBUG_SCP : SendVideoScpNotificationRequestToPartyCntl ");
//
//	CSegment* pSeg = new CSegment;
//
//	scpNotifyReq.Serialize(NATIVE, *pSeg);
//
//	m_pPartyApi->SendVideoScpNotificationReq(pSeg);
//}

void CVideoRelayBridgePartyCntl::SendScpNotificationReqToPartyCntl(CScpNotificationWrapper& scpNotifyReq)
{
//	scpNotifyReq.Dump(" DEBUG_SCP : SendScpNotificationReqToPartyCntl ");

	CSegment* pSeg = new CSegment;
	scpNotifyReq.Serialize(NATIVE, *pSeg);

	TRACEINTO << "Send Notification request SCP_NOTIFICATION_REQ to partyId: " << GetPartyRsrcID() << "name: "<< m_pConfApi->NameOf();

	if(IsValidPObjectPtr(m_pConfApi))
	{
		m_pConfApi->SendScpNotificationReqToPartyCntl(GetPartyRsrcID() /*m_pParty->GetPartyId()*/, pSeg);// BRIDGE-6232 -it is error to use the TaskApp pointer!
	}
	else
	{
		FPASSERTMSG_AND_RETURN(1,"m_pConfApi is  NULL");
	}

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SendScpPipeMappingToPartyCntl(CScpPipeMappingNotification& scpPipeMappingNotifyReq)
{
	CSegment* pSeg = new CSegment;
	scpPipeMappingNotifyReq.Serialize(NATIVE, *pSeg);

	//TRACEINTO << "Send Notification RsrcID Instead App Pointer 002";//GetPartyRsrcID()
	m_pConfApi->SendScpPipesMappingNotificationReqToPartyCntl(GetPartyRsrcID()/*m_pParty->GetPartyId()*/, pSeg);	// BRIDGE-6232 -it is error to use the TaskApp pointer!

	POBJDELETE(pSeg);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::AvcImageAdded()
{
	DispatchEvent(ADD_AVC_IMAGE, NULL);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::LastAvcImageRemoved()
{
	DispatchEvent(LAST_AVC_IMAGE_REMOVED, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
CVideoOperationPointsSet* CVideoRelayBridgePartyCntl::GetConfVideoOperationPointsSet()const
{
	if(IsValidPObjectPtr(m_pBridge))
	{
		return ((CVideoBridgeCP * )m_pBridge)->GetConfVideoOperationPointsSet();

	}
	PASSERT(101);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD  CVideoRelayBridgePartyCntl::GetNumberOfNonRelayImages() const
{
	if(IsValidPObjectPtr(m_pBridge))
	{
		return ((CVideoBridgeCP * )m_pBridge)->GetNumberOfNonRelayImages();

	}
	PASSERT(101);
	return 0;
}


//============================================================================
// Handle Events
void  CVideoRelayBridgePartyCntl::OnVideoBridgeConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeConnect", m_partyConfName);


	if (!m_pBridgePartyIn && !m_pBridgePartyOut)
	{
	    m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE, eNoDirection);
	    return;
	}
	Setup();
	EMediaDirection ConnectedDirection         = eNoDirection;

	if(m_pBridgePartyOut)
	{
		if(m_pBridgePartyIn)
		  ConnectedDirection =   eMediaInAndOut;
		else
		  ConnectedDirection =   eMediaOut;
	}
	else
	{
	  if(m_pBridgePartyIn)
		  ConnectedDirection = eMediaIn;
	}

	//Although  CVideoRelayBridgePartyCntl maybe not in connected state we update VB/PartyCntl that it end connected
	//Because in video relay flow we change the state just after Party complete the negotiation and we are updated with the right paramters
	//We notify the partycnt that is connected so partycntl doesnt need to wait on it
	// Inform Video Bridge
	m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statOK, FALSE, ConnectedDirection);
}

/////////////////////////////////////////////////////////////////////////////
void  CVideoRelayBridgePartyCntl::OnVideoBridgeConnectIDLE(CSegment* pParam)
{

	BYTE isIVR;
	*pParam >> isIVR;
	TRACEINTOFUNC << " ConfName = " << m_partyConfName << ", IsIVR:" << (int)isIVR;

	if (isIVR)
	{
		m_state = CONNECTED_STANDALONE;

		EMediaDirection eDirection = eNoDirection;
		if (m_pBridgePartyIn)
		  eDirection |= eMediaIn;

		if (m_pBridgePartyOut)
		  eDirection |= eMediaOut;

		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY_IVR_MODE, statOK, FALSE, eDirection);
	 }
	else
	{
		OnVideoBridgeConnect(pParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CVideoRelayBridgePartyCntl::OnVideoBridgeConnectSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeConnectSETUP", m_partyConfName);
	OnVideoBridgeConnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CVideoRelayBridgePartyCntl::OnVideoBridgeConnectCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeConnectCONNECTED", m_partyConfName);
	OnVideoBridgeConnect(pParam);
}
/////////////////////////////////////////////////////////////////////////////
void  CVideoRelayBridgePartyCntl::OnVideoBridgeConnectCONNECTED_STANDALONE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeConnectCONNECTED_STANDALONE", m_partyConfName);
	BYTE isIVR;
	*pParam >> isIVR;
	TRACEINTOFUNC << " ConfName = " << m_partyConfName << ", IsIVR:" << (int)isIVR;

	if (isIVR)
	{
		EMediaDirection eDirection = eNoDirection;
		if (m_pBridgePartyIn)
		  eDirection |= eMediaIn;

		if (m_pBridgePartyOut)
		  eDirection |= eMediaOut;

		// Inform Video Bridge
		m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY_IVR_MODE, statOK, FALSE, eDirection);
	 }
	else
	{
		OnVideoBridgeConnect(pParam);
	}
}
/////////////////////////////////////////////////////////////////////////////
//void  CVideoRelayBridgePartyCntl::OnVideoBridgeConnectDISCONNECTING(CSegment* pParam)
//{
//	//TODO Keren
//	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoRelayBridgeConnectDISCONNECTING", m_partyConfName);
//	m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDCONNECTPARTY, statInconsistent, FALSE);
//
//}


/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateVideoInParams(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateVideoInParams not relevant for video relay, we should have called UpdateVideoInParams(CBridgePartyVideoRelayMediaParams* )", m_partyConfName);
	PASSERT(100);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateVideoInParamsDISCONNECTING not relevant for video relay, we should have called UpdateVideoInParams(CBridgePartyVideoRelayMediaParams* )", m_partyConfName);
	PASSERT(100);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateVideoOutParamsDISCONNECTING not relevant for video relay, we should have called UpdateVideoInParams(CBridgePartyVideoRelayMediaParams* )", m_partyConfName);
	PASSERT(100);

}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateVideoOutParams(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateVideoOutParams not relevant for video relay, we should have called UpdateVideoInParams(CBridgePartyVideoRelayMediaParams* )", m_partyConfName);
	PASSERT(100);

}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeAddImage(CSegment* pParam)
{
	if(m_pBridgePartyOut)
		  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->AddImage();
	else
		PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeAddImage m_pBridgePartyOut is NULL", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateImage(CSegment* pParam)
{
	if(m_pBridgePartyOut)
		  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->UpdateImage();
	else
		PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateImage m_pBridgePartyOut is NULL", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeMuteImage(CSegment* pParam)
{
	if(m_pBridgePartyOut)
			  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->MuteImage();
	else
		PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeMuteImage m_pBridgePartyOut is NULL", m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeUnMuteImage(CSegment* pParam)
{
	if(m_pBridgePartyOut)
			  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->UnMuteImage();
	else
		PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeUnMuteImage m_pBridgePartyOut is NULL", m_partyConfName);
}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeChangeSpeakers(CSegment* pParam)
{
	if(m_pBridgePartyOut)
			  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->ChangeSpeakers();
	else
		PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeChangeSpeakers m_pBridgePartyOut is NULL", m_partyConfName);
}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeChangeAudioSpeaker(CSegment* pParam)
{
	if(m_pBridgePartyOut)
			  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->ChangeAudioSpeaker();
	else
		PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeChangeAudioSpeaker m_pBridgePartyOut is NULL", m_partyConfName);
}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeDelImage(CSegment* pParam)
{
	if(m_pBridgePartyOut)
			  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->DelImage();
	else
		PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeDelImage m_pBridgePartyOut is NULL", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeDeletePartyFromConf(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeDeletePartyFromConf currently nothing to do in case of relay", m_partyConfName);

}
void CVideoRelayBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED_STANDALONE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED_STANDALONE - ", m_partyConfName);

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	BOOL            IsVideoFullyDisconnect = FALSE;
	EMediaDirection ConnectedDirection     = eNoDirection;
	CRsrcParams*    pRsrcParams            = NULL;

	if (pRoutingTable != NULL)
	{
		switch (GetDisconnectingDirectionsReq())
		{
		// Disconnect Out
		case eMediaOut:
		{
			if (!m_pBridgePartyOut)
			{
				PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED_STANDALONE - The connection 'eMediaOut' already disconnected, ", m_partyConfName);
				return;
			}
			pRsrcParams = m_pBridgePartyOut->GetRsrcParams();
			PASSERT_AND_RETURN(!pRsrcParams);
			PASSERT_AND_RETURN(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams));

			// If In exist and only out disconnects state will remain CONNECTED_STANDALONE
			if (!m_pBridgePartyIn)
				IsVideoFullyDisconnect = TRUE;
			else
				ConnectedDirection = eMediaIn;
			break;
		}

		// Disconnect In
		case eMediaIn:
		{
			if (!m_pBridgePartyIn)
			{
				PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED_STANDALONE - The connection 'eMediaIn' already disconnected, ", m_partyConfName);
				return;
			}

			// If Out exist and only In disconnects state will remain CONNECTED_STANDALONE
			if (!m_pBridgePartyOut)
				IsVideoFullyDisconnect = TRUE;
			else
				ConnectedDirection = eMediaOut;
			break;
		}

		// Disconnect In&Out
		case eMediaInAndOut:
		{
			if (m_pBridgePartyOut)
			{
				pRsrcParams = m_pBridgePartyOut->GetRsrcParams();
				PASSERT_AND_RETURN(!pRsrcParams);
				PASSERT_AND_RETURN(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pRsrcParams));
			}

			IsVideoFullyDisconnect = TRUE;
			ConnectedDirection     = eNoDirection;
			break;
		}

		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
		}
	}

	// Only if both direction disconnects
	if (IsVideoFullyDisconnect)
		m_state = IDLE;
	else
	{
		PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeDisconnectCONNECTED_STANDALONE - Not all direction disconnected, ", m_partyConfName);
		DestroyPartyInOut(ConnectedDirection);
	}

	// Inform Video Bridge
	m_pConfApi->PartyBridgeResponseMsgById(m_partyRsrcID, VIDEO_BRIDGE_MSG, ENDDISCONNECTPARTY, statOK, FALSE, ConnectedDirection);
}
/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoInDisconnected(CSegment* pParam)
{
   PASSERT_AND_RETURN(!m_pBridgePartyIn);
   PASSERT_AND_RETURN(!m_pBridgePartyIn->IsDisConnected());
   //No need to remove from routing table currently no resources
   VideoDisConnectionCompletion(pParam, eMediaIn);
}

/////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::TryStartGathering()
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::TryStartGathering - not relevant for relay participants", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnEpAskForIntraSETUP(CSegment* pParam)
{
	OnEpAskForIntra( pParam );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CVideoRelayBridgePartyCntl::OnEpAskForIntraCONNECTED_STANDALONE(CSegment* pParam)
{
	OnEpAskForIntra( pParam );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CVideoRelayBridgePartyCntl::OnEpAskForIntraCONNECTED(CSegment* pParam)
{
	OnEpAskForIntra( pParam );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CVideoRelayBridgePartyCntl::OnEpAskForIntraDISCONNECTING(CSegment* pParam)
{
	 TRACEINTO << "PartyId:" << GetPartyRsrcID() <<  ", ConfId:" << GetConfRsrcID() <<"  ignored";
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CVideoRelayBridgePartyCntl::OnEpAskForIntra(CSegment* pParam)
{
	if(m_pBridgePartyOut)
	{
		if (pParam)
		{
			RelayIntraParam intraParam;
			intraParam.DeSerialize(pParam);
			((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->EpAskForIntra(intraParam);
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal, "VideoRelayBridgePartyCntl::OnEpAskForIntraCONNECTED m_pBridgePartyOut= NULL, party:", m_partyConfName);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageSETUP(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageSETUP ", m_partyConfName);
	OnVideoBridgeAddAvcImage(pParam);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageCONNECTED ", m_partyConfName);
	OnVideoBridgeAddAvcImage(pParam);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageCONNECTED_STANDALONE(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageCONNECTED_STANDALONE ", m_partyConfName);
	OnVideoBridgeAddAvcImage(pParam);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImageDISCONNECTING ignore disconnecting", m_partyConfName);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CVideoRelayBridgePartyCntl::OnVideoBridgeAddAvcImage(CSegment* pParam)
{
	if(IsValidPObjectPtr(m_pBridgePartyIn))
	{
		((CBridgePartyVideoRelayIn*)(m_pBridgePartyIn))->UpdateSvcToAvcTranslatorOnFirstAvcImage();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedSETUP(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedSETUP ", m_partyConfName);
	OnVideoBridgeLastAvcImageRemoved(pParam);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedCONNECTED ", m_partyConfName);
	OnVideoBridgeLastAvcImageRemoved(pParam);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedCONNECTED_STANDALONE(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedCONNECTED_STANDALONE ", m_partyConfName);
	OnVideoBridgeLastAvcImageRemoved(pParam);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelWarn, "CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemovedDISCONNECTING ignore disconnecting", m_partyConfName);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeLastAvcImageRemoved(CSegment* pParam)
{
	if(IsValidPObjectPtr(m_pBridgePartyIn))
	{
		((CBridgePartyVideoRelayIn*)(m_pBridgePartyIn))->UpdateSvcToAvcTranslatorOnNoAvcImageInConf();
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoRelayBridgePartyCntl::UpdateOnImageAvcToSvcTranslate()
{
	DispatchEvent(UPDATEONIMAGEAVCTOSVC, NULL);

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateOnImageAvcToSvcCONNECTED_STANDALONE(CSegment* pParam)
{
	OnVideoBridgeUpdateOnImageAvcToSvc( pParam );
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateOnImageAvcToSvcCONNECTED(CSegment* pParam)
{
	OnVideoBridgeUpdateOnImageAvcToSvc( pParam );
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateOnImageAvcToSvc(CSegment* pParam)
{
  TRACECOND_AND_RETURN(!m_pBridgePartyOut, "CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateOnImageAvcToSvc - Failed, " << m_partyConfName << ", State:" << StateToString());

  PTRACE2(eLevelInfoNormal, "CVideoRelayBridgePartyCntl::OnVideoBridgeUpdateOnImageAvcToSvc ", m_partyConfName);
 ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->UpdateOnImageAvcToSvcTranslate();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeShowSlideCONNECTED_STANDALONE(CSegment* pParam)
{
  TRACECOND_AND_RETURN(!m_pBridgePartyOut, "CVideoRelayBridgePartyCntl::OnVideoBridgeShowSlideCONNECTED_STANDALONE - Failed, " << m_partyConfName << ", State:" << StateToString());
  TRACEINTO << "PartyId:" << GetPartyRsrcID() <<  ", ConfId:" << GetConfRsrcID();
  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->ShowSlide(pParam);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SendScpIvrShowSlideReqToPartyCntl()
{

	TRACEINTO << " RsrcID Instead App Pointer 003, PartyId:" << GetPartyRsrcID() <<  ", ConfId:" << GetConfRsrcID();
	m_pConfApi->SendScpIvrShowSlideToPartyCntl(GetPartyRsrcID()/*m_pParty->GetPartyId()*/);			// BRIDGE-6232 -it is error to use the TaskApp pointer!

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeStopShowSlideCONNECTED_STANDALONE(CSegment* pParam)
{
  TRACECOND_AND_RETURN(!m_pBridgePartyOut, "CVideoRelayBridgePartyCntl::OnVideoBridgeStopShowSlideCONNECTED_STANDALONE - Failed, " << m_partyConfName << ", State:" << StateToString());
  ((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->StopShowSlide(pParam);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SendScpIvrStopShowSlideReqToPartyCntl()
{

	TRACEINTO << " RsrcID Instead App Pointer 004, PartyId:" << GetPartyRsrcID() <<  ", ConfId:" << GetConfRsrcID();
	m_pConfApi->SendScpIvrStopShowSlideToPartyCntl(GetPartyRsrcID()/*m_pParty->GetPartyId()*/);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnVideoBridgeJoinConfCONNECTED_STANDALONE(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
	OnVideoBridgeConnect(pParam);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::AckOnIvrScpShowSlide(BYTE bIsAck)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcID() <<  ", ConfId:" << GetConfRsrcID();
	if(m_pBridgePartyOut)
	{
		((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->AckOnIvrScpShowSlide(bIsAck);
	}
	else
	{
		TRACEINTO << "m_pBridgePartyOut not valid" << (DWORD)m_pBridgePartyOut;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::AckOnIvrScpStopShowSlide(BYTE bIsAck)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcID() <<  ", ConfId:" << GetConfRsrcID();
	if(m_pBridgePartyOut)
	{
		((CBridgePartyVideoRelayOut*)m_pBridgePartyOut)->AckOnIvrScpStopShowSlide(bIsAck);
	}
	else
	{
		TRACEINTO << "m_pBridgePartyOut not valid" << (DWORD)m_pBridgePartyOut;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateIndicationIcons()
{
	TRACESTRFUNC(eLevelDebug) << "not relevant for Relay PartyId:" << GetPartyRsrcID();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::CheckIsMutedVideoInAndUpdateDB()
{
	TRACEINTO << m_partyConfName;
	if (CPObject::IsValidPObjectPtr(m_pBridgePartyIn))
	{
		bool bIsMuted = ((CBridgePartyVideoRelayIn *)m_pBridgePartyIn)->IsMuted();
		if(bIsMuted)
		{
			if( ((CBridgePartyVideoRelayIn *)m_pBridgePartyIn)->IsMuteByParty())
			{
				TRACEINTO << m_partyConfName << "Update DB on PARTY mute";
				DWORD updateStatus;
				updateStatus = 0xF0000010;
				m_pConfApi->UpdateDB(m_pParty, MUTE_STATE, updateStatus);

			}
			if(((CBridgePartyVideoRelayIn *)m_pBridgePartyIn)->IsMuteByMCMS())
			{
				TRACEINTO << m_partyConfName << "Update DB on MCMS mute";
				DWORD updateStatus;
				updateStatus =0x0F000010;
				m_pConfApi->UpdateDB(m_pParty, MUTE_STATE, updateStatus);
			}
			if(((CBridgePartyVideoRelayIn *)m_pBridgePartyIn)->IsMuteByOperator())
			{
				TRACEINTO << m_partyConfName << "Update DB on OPERATOR mute";
				DWORD updateStatus;
				updateStatus =0x00000010;
				m_pConfApi->UpdateDB(m_pParty, MUTE_STATE, updateStatus);

			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateArtOnTranslateVideoSSRC( DWORD ssrc )
{
	if (m_pConfApi)
		m_pConfApi->UpdateArtOnTranslateVideoSSRC( m_partyRsrcID, ssrc );
	else
	{
		PASSERT(101);
		TRACEINTO << "SVC-Translator-Error : m_pConfApi=NULL !! , SSRC: " << ssrc;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateArtOnTranslateVideoSSRCAck( DWORD status )
{
	if (m_pBridgePartyIn)
		((CBridgePartyVideoRelayIn*)(m_pBridgePartyIn))->UpdateArtOnTranslateVideoSSRCAck( status );
	else
	{
		TRACEINTO << "SVC-Translator-Error : m_pBridgePartyIn=NULL !! , status: " << status;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateMrmpStreamIsMust( DWORD ssrc, DWORD videoRelayInChannelHandle, BOOL bIsMustSsrc )
{
	if (m_pConfApi)
		m_pConfApi->UpdateMrmpStreamIsMust( m_partyRsrcID, ssrc, videoRelayInChannelHandle, bIsMustSsrc );
	else
	{
		PASSERT(102);
		TRACEINTO << "SVC-Translator-Error : m_pConfApi=NULL !! , SSRC: " << ssrc;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpdateMrmpStreamIsMustAck( DWORD status )
{
	if (m_pBridgePartyIn)
		((CBridgePartyVideoRelayIn*)(m_pBridgePartyIn))->UpdateMrmpStreamIsMustAck( status );
	else
	{
		TRACEINTO << "SVC-Translator-Error : m_pBridgePartyIn=NULL !! , status: " << status;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
int CVideoRelayBridgePartyCntl::GetMaxAllowedLayerId()
{
	return ((CVideoBridgeCP*)GetBridge())->GetMaxAllowedLayerId();
}

int CVideoRelayBridgePartyCntl::GetMaxAllowedLayerIdHighRes()
{
	return ((CVideoBridgeCP*)GetBridge())->GetMaxAllowedLayerIdHighRes();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::UpgradeSvcToAvcTranslator()
{
	TRACEINTO << " -Connecting Svc to Avc translator";

	DispatchEvent(UPGRADE_TO_MIX_AVC_SVC, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnPartyUpgradeSvcToAvcTranslatorSETUP(CSegment* pParam)
{
	OnUpgradeSvcToAvcTranslator();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnPartyUpgradeSvcToAvcTranslatorCONNECTED_STANDALONE(CSegment* pParam)
{
	TRACEINTO << " in Stand-Alone ";
	OnUpgradeSvcToAvcTranslator();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnPartyUpgradeSvcToAvcTranslatorCONNECTED(CSegment* pParam)
{
	OnUpgradeSvcToAvcTranslator();
}

void CVideoRelayBridgePartyCntl::OnUpgradeSvcToAvcTranslator()
{
	if (!m_pBridgePartyIn)
	{
		TRACEINTO << " -  SVC-Translator-Error - m_pBridgePartyIn == NULL - , m_state: " << (DWORD)m_state;	// it is not always error when IN not exists
		ReplayUpgradeSvcToAvcTranslate(statIllegal);
	}
	else	// Klocwork correction, adding "else"
		((CBridgePartyVideoRelayIn*)(m_pBridgePartyIn))->UpgradeSvcToAvcTranslator();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::OnPartyUpgradeSvcToAvcTranslatorDISCONNECTING(CSegment* pParam)         // IVR_JOIN_CONF_VIDEO
{
		ReplayUpgradeSvcToAvcTranslate(statIllegal);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideoRelayBridgePartyCntl::SendUpdateToAudioBridgeOnSeenImage(PartyRsrcID idOfPartyToUpdate, PartyRsrcID idOfSeenParty)
{
	if(m_pConfApi)
	{
		m_pConfApi->SendUpdateToAudioBridgeOnSeenImage(idOfPartyToUpdate,idOfSeenParty );
	}
	else
		PASSERT(GetPartyRsrcID());
}


BOOL CVideoRelayBridgePartyCntl::IsEnableHighVideoResInSvcToAvcMixMode()
{
	const CCommConf* pCommConf = m_pBridge->GetConf()->GetCommConf();
	PASSERTMSG_AND_RETURN_VALUE(!pCommConf, "SVC-Translator-Error: Failed, 'pCommConf' is NULL", false);

	const BOOL b = pCommConf->GetEnableHighVideoResInSvcToAvcMixMode();

	return b;
}


