//+========================================================================+
//                            IVRCntl.cpp                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IVRCNTL.cpp                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 25.09.2000 |                                                      |
//+========================================================================+

#include "IVRCntl.h"
#include "IVRFeatures.h"

extern const char* ProductTypeToString(eProductType productType);


PBEGIN_MESSAGE_MAP(CIvrCntl)

//ONEVENT(IVR_END_OF_FEATURE,      ACTIVE,    CIvrCntl::OnEndFeature)

PEND_MESSAGE_MAP(CIvrCntl, CStateMachine);


////////////////////////////////////////////////////////////////////////////
//                        CIvrCntl
////////////////////////////////////////////////////////////////////////////
CIvrCntl::CIvrCntl(CTaskApp* pOwnerTask,const DWORD dwMonitorConfId,const char* pNumericId) :
  CStateMachine(pOwnerTask)
{
	TRACEINTO << " For External control dwMonitorConfId = " <<  dwMonitorConfId ;

  m_pParty               = NULL;
  m_pDtmfCollector       = NULL;
  m_pIvrSubGenSM         = NULL;
  m_state                = NOTACTIVE; // state
  m_stage                = 0;         // current stage in the progress IVR process
  m_pConfApi             = NULL;
  m_startNextFeature     = FALSE;
  m_pNumericConferenceId = pNumericId;

  memset(m_featuresList, 0, sizeof(m_featuresList));

  m_monitorPartyId      = (DWORD)(-1);
  m_rsrcPartyId         = (DWORD)(-1);
  m_dwMonitorConfId     = dwMonitorConfId;
  m_MCUproductType		= CProcessBase::GetProcess()->GetProductType();
  m_bIsResumeAfterHold  = FALSE;
  VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrCntl::~CIvrCntl()
{
  POBJDELETE(m_pIvrSubGenSM);
  POBJDELETE(m_pDtmfCollector);

  if (m_pConfApi)
    m_pConfApi->DestroyOnlyApi();
  POBJDELETE(m_pConfApi);
}

//--------------------------------------------------------------------------
void CIvrCntl::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
  switch (opCode)
  {
    case TIMER:
    {
	  TRACEINTO << " - TIMER EVENT ";
      break;
    }


    case AUD_DTMF_IND_VAL:
    {
      if (m_pDtmfCollector)
        m_pDtmfCollector->HandleEvent(pMsg, msgLen, opCode);

      break;
    }

	case DTMF_IVR_CLEAR_BUFFER:
    {
      if (m_pDtmfCollector)
        m_pDtmfCollector->ResetDtmfBuffer();

      break;
    }


    default:
    {        // all other messages
      DispatchEvent(opCode, pMsg);
      break;
    }
  } // switch
}

//--------------------------------------------------------------------------
void CIvrCntl::Create(CParty* pParty, COsQueue* pConfRcvMbx)
{
  PASSERT_AND_RETURN(!pParty);
  PASSERT_AND_RETURN(!pConfRcvMbx);

  m_pParty = pParty;

  #ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(m_dwMonitorConfId);
  #else
	m_pConfApi = new CConfApi();
  #endif

  m_pConfApi->CreateOnlyApi(*pConfRcvMbx, NULL, NULL, 1);

  m_pDtmfCollector = new CDtmfCollector(pParty);
  m_pDtmfCollector->Create(pParty, IsExternalIVR());

}

//--------------------------------------------------------------------------
void CIvrCntl::SetPartyRsrcID(DWORD rsrcPartyId)
{
	m_rsrcPartyId = rsrcPartyId;
	if (m_pDtmfCollector)
		m_pDtmfCollector->SetPartyRsrcID(m_rsrcPartyId);
}
//--------------------------------------------------------------------------
void CIvrCntl::UpdateConfApi(COsQueue* pConfRcv)
{
  if (m_pConfApi)
  {
    m_pConfApi->DestroyOnlyApi();
    POBJDELETE(m_pConfApi);
  }

  m_pConfApi = new CConfApi;
  m_pConfApi->CreateOnlyApi(*pConfRcv, NULL, NULL, 1);
}

////--------------------------------------------------------------------------
void CIvrCntl::Start(CSegment* pParam)
{
}

////--------------------------------------------------------------------------
void CIvrCntl::Resume(CSegment* pParam)
{
	TRACEINTO;
	m_bIsResumeAfterHold = TRUE;
	Start(pParam);
}

//--------------------------------------------------------------------------
void CIvrCntl::StartNewFeature()
{
	PASSERT_AND_RETURN(!m_pIvrSubGenSM);
    m_pIvrSubGenSM->SetIvrCntl(this);
    m_pIvrSubGenSM->SetDtmfP(m_pDtmfCollector);
    m_pIvrSubGenSM->CreatePartyApi(m_pParty, m_pConfApi);
    m_pIvrSubGenSM->Start();
}

void CIvrCntl::OnEndFeature(CSegment* pParam)
{
  TRACEINTO;
  return;
}
//--------------------------------------------------------------------------
void CIvrCntl::SetStartNextFeature()
{

}

//--------------------------------------------------------------------------
BYTE CIvrCntl::IsEmptyConf()
{
 return YES;
}

//--------------------------------------------------------------------------
void CIvrCntl::SetIsLeader(WORD bUpdateParty, WORD bUpdateDB, BYTE bUpdateDTMFCollector)
{

}

//--------------------------------------------------------------------------
void CIvrCntl::MuteParty(CConfApi* pConfApi, CTaskApp* pParty, BYTE bPlayMessage)
{
}

//--------------------------------------------------------------------------
void CIvrCntl::UnMuteParty(CConfApi* pConfApi, CTaskApp* pParty)
{

}

//--------------------------------------------------------------------------
void CIvrCntl::SetChangeConfPasswordFeature()
{

}

//--------------------------------------------------------------------------
void CIvrCntl::SetChangeLeaderPasswordFeature()
{

}

//--------------------------------------------------------------------------
void CIvrCntl::SetChangePasswordFailed()
{

}

//--------------------------------------------------------------------------
void CIvrCntl::SetChangePasswordFeature()
{

}
//--------------------------------------------------------------------------
void CIvrCntl::MovePartyToInConf()
{

}

//--------------------------------------------------------------------------
void CIvrCntl::CancelSetLeader()
{

}


//--------------------------------------------------------------------------
void CIvrCntl::ResetIvr()
{

}

//--------------------------------------------------------------------------
void CIvrCntl::StopIVR()
{

}


//
WORD CIvrCntl::IsLeaderReqForStartConf()
{
	return NO;
}

//--------------------------------------------------------------------------
void CIvrCntl::OnStartIVR()
{
  CConfApi* pConfApi = m_pConfApi;

  // set icon in IVR mode, remove from 'operator help queue'
  pConfApi->UpdateDB(m_pParty, PARTYAVSTATUS, STATUS_PARTY_IVR, 1);
}

//--------------------------------------------------------------------------
void CIvrCntl::BillingCodeToCDR(const char* billingCode)
{

}

//--------------------------------------------------------------------------
void CIvrCntl::GetGeneralSystemMsgsParams(WORD event_op_code, char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum)
{

}


////--------------------------------------------------------------------------
BYTE CIvrCntl::GetChangePasswordType()
{
   return 0;
}

//-------------------------------------------------------------------
void CIvrCntl::SetChangePasswordType(BYTE type)
{

}

////--------------------------------------------------------------------------
const char* CIvrCntl::GetNewPassword()
{
	return NULL;
}

//--------------------------------------------------------------------------
void CIvrCntl::SetNewPassword(const char* pass)
{

}

//--------------------------------------------------------------------------
const char* CIvrCntl::GetInvitePartyAddress()
{
		return NULL;
}

//--------------------------------------------------------------------------
void CIvrCntl::SetInvitePartyAddress(const char* sPartyAddress)
{

}

//--------------------------------------------------------------------------
void CIvrCntl::SetLinkParty()
{

}


//--------------------------------------------------------------------------
void CIvrCntl::HandleDtmfForwarding(CSegment* pParam)
{

}

//--------------------------------------------------------------------------
void CIvrCntl::SetGwDTMFForwarding(BYTE val)
{

}

//--------------------------------------------------------------------------
void CIvrCntl::SetInvitePartyDTMFForwarding(BYTE val)
{

}

//--------------------------------------------------------------------------
void CIvrCntl::onInvitePartyDTMFForwardingTout()
{

}


//--------------------------------------------------------------------------
void CIvrCntl::setNoVideRsrcForVideoParty(BYTE bNoVideRsrcForVideoParty)
{

}

////--------------------------------------------------------------------------
int CIvrCntl::IsPSTNCall()
{
  return 0;
}

//--------------------------------------------------------------------------
void CIvrCntl::PcmConnected()
{
//  m_pDtmfCollector->PcmConnected();
}

//--------------------------------------------------------------------------
void CIvrCntl::PcmDisconnected()
{

}

//--------------------------------------------------------------------------
WORD CIvrCntl::IsRecordingLinkParty()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, 0);

	return pConfParty->GetRecordingLinkParty();
}


//--------------------------------------------------------------------------
void CIvrCntl::SipConfNIDConfirmationInd(DWORD sts) // will be called from party response
{

}


//--------------------------------------------------------------------------
void CIvrCntl::ResetFeaturesList()
{

}

//--------------------------------------------------------------------------
bool CIvrCntl::IsConfOrLeaderPasswordRequired()const
{
                return false;
}

//--------------------------------------------------------------------------
void CIvrCntl::ResetFeaturesListForCallFromGW()
{
}

//--------------------------------------------------------------------------
void CIvrCntl::SetCascadeLinkInConf(BOOL isCascadeLinkInConf)
{
}



//--------------------------------------------------------------------------
void  CIvrCntl::SetTurnOffIvrAfterMove(bool turnOffIvrAfterMove)
{

}
//--------------------------------------------------------------------------

void CIvrCntl::OnPlayMusic(CSegment* pParam)
{

		//TRACECOND_AND_RETURN_VALUE(, "Failed, Feature does not exist", 1);
}
//--------------------------------------------------------------------------

void CIvrCntl::RecivedPlayMessageAck()
{
	TRACEINTO;
}
//--------------------------------------------------------------------------
void CIvrCntl::RecivedShowSlideAck()
{
	TRACEINTO;
}
//--------------------------------------------------------------------------


