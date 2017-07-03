//+========================================================================+
//                  ConnectedVoicePartyCntl.cpp						       |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                    All Rights Reserved.                                 |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConnectedVoicePartyCntl.cpp                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |								   |
//+========================================================================+

#include "NStream.h"
#include "ConnectedIsdnVoicePartyCntl.h"
#include "Trace.h"
#include "ConfPartyOpcodes.h"
#include "AudioBridgeInterface.h"


/////////////////////////////////////////////////////////////////////////////
//                        CConnectedIsdnVoicePartyCntl
/////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CConnectedIsdnVoicePartyCntl)

	ONEVENT(END_AVC_TO_SVC_ART_TRANSLATOR_CONNECT,         ANYCASE,      CConnectedIsdnVoicePartyCntl::OnEndAvcToSvcArtTranslatorConnectAnycase)

	ONEVENT(AVC_SVC_ADDITIONAL_PARTY_RSRC_IND,             ANYCASE,      CConnectedIsdnVoicePartyCntl::OnAvcSvcAdditionalPartyRsrcIndAnycase)

	ONEVENT(SET_PARTY_AVC_SVC_MEDIA_STATE,                 ANYCASE,      CConnectedIsdnVoicePartyCntl::OnSetPartyAvcSvcMediaStateAnycase)

	ONEVENT(END_AUDIO_UPGRADE_TO_MIX_AVC_SVC,              ANYCASE,      CConnectedIsdnVoicePartyCntl::OnEndAudioUpgradeToMixAvcSvcAnycase)

PEND_MESSAGE_MAP(CConnectedIsdnVoicePartyCntl,CIsdnPartyCntl);

/////////////////////////////Action functions///////////////////////////////

////////////////////////////////////////////////////////////////////////////////
CConnectedIsdnVoicePartyCntl::~CConnectedIsdnVoicePartyCntl(void)
{

}
//--------------------------------------------------------------------------
CConnectedIsdnVoicePartyCntl::CConnectedIsdnVoicePartyCntl()
{
    VALIDATEMESSAGEMAP;
}
//--------------------------------------------------------------------------

void*    CConnectedIsdnVoicePartyCntl::GetMessageMap()
{
  return (void*)m_msgEntries;    
}
//--------------------------------------------------------------------------
CConnectedIsdnVoicePartyCntl& CConnectedIsdnVoicePartyCntl::operator =(const CConnectedIsdnVoicePartyCntl & other)
{
  CIsdnPartyCntl::operator=(other);
  return *this;
}

//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::ChangeScm()
{

	TRACEINTO << "PartyId:" << GetPartyRsrcId()
			<< "\nPartyName:" << m_partyConfName
			<< ", End PSTN Party Connection"
			<< "\nConfId:" << GetConfRsrcId()
			<< "\nIsAlreadyUpgradeToMixAvcToSvc:" << (WORD)m_bIsAlreadyUpgradeToMixAvcToSvc
			<< "\nIsPartyInUpgradeProcess:" << (WORD)m_bPartyInUpgradeProcess;

	m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED);
	m_pTaskApi->UpdatePartyStateInCdr(m_pParty);
	if (m_bPartyInUpgradeProcess)
	{
		if (!m_bIsAlreadyUpgradeToMixAvcToSvc)
		{
			DispatchEvent(SET_PARTY_AVC_SVC_MEDIA_STATE,NULL);
		}
	}
}

//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::OnAvcSvcAdditionalPartyRsrcIndAnycase(CSegment* pParam)
{
	DWORD  structLen = sizeof(ALLOC_PARTY_IND_PARAMS_S_BASE);
	ALLOC_PARTY_IND_PARAMS_S_BASE  stAllocateAdditionalPartyIndParamsBase;
	memset(&stAllocateAdditionalPartyIndParamsBase,0,structLen);


	structLen = sizeof(ALLOC_PARTY_IND_PARAMS_S);
	ALLOC_PARTY_IND_PARAMS_S  stAllocatePartyIndParams;
	memset(&stAllocatePartyIndParams,0,structLen);
	pParam->Get((BYTE*)(&stAllocatePartyIndParams),structLen);

	TRACEINTO << "stAllocatePartyIndParams.svcParams.m_ssrcAudio:" << stAllocatePartyIndParams.svcParams.m_ssrcAudio;

	m_ssrcAudio = stAllocatePartyIndParams.svcParams.m_ssrcAudio;
	TRACEINTO << "ssrcAudio:" << m_ssrcAudio;

	stAllocateAdditionalPartyIndParamsBase = stAllocatePartyIndParams.allocIndBase;

	DumpAllocIndToTrace(stAllocateAdditionalPartyIndParamsBase);
	AddAdditionalPartyRsrcToRsrcDescVector(stAllocateAdditionalPartyIndParamsBase);
	bool bIsTranslatorStarted = StartAvcToSvcArtTranslator();
	if(!bIsTranslatorStarted)
	{
		DBGPASSERT(bIsTranslatorStarted);
		SetPartyConnectedWithProblemAndFaulty();
	}
}
//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::OnEndAvcToSvcArtTranslatorConnectAnycase(CSegment* pParam)
{
	STATUS status;
	*pParam >> status;

	TRACEINTO
		<< "PartyId:" << GetPartyRsrcId()
		<< ", PartyName:" << m_partyConfName
		<< ", ConfId:" << GetConfRsrcId()
		<< ", status: " << status;

	if (STATUS_OK == status)
	{
		m_bIsAckOnAvcToSvcArtTransalatorReceived = true;
		m_pAudioInterface->UpgradeToMixAvcSvc(GetPartyRsrcId());
	}
	else
	{
		DBGPASSERT(status);
		SetPartyConnectedWithProblemAndFaulty();
	}
}
//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::OnSetPartyAvcSvcMediaStateAnycase(CSegment* pParam)
{
	SendAllocateMixResourcesRequest();
}
//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::OnEndAudioUpgradeToMixAvcSvcAnycase(CSegment* pParam)
{
	STATUS status;
	*pParam >> status;

	TRACEINTO
		<< "PartyId:" << GetPartyRsrcId()
		<< ", PartyName:" << m_partyConfName
		<< ", ConfId:" << GetConfRsrcId()
		<< ", status: " << status;

	if (STATUS_OK != status)
	{
		DBGPASSERT(status);
		SetPartyConnectedWithProblemAndFaulty();
	}
}
//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::SetPartyConnectedWithProblemAndFaulty()
{
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	PASSERT_AND_RETURN(!pConfParty);
	pConfParty->SetPartyState(PARTY_CONNECTED_WITH_PROBLEM, GetConfRsrcId());
	m_isFaulty = 1; // Invoking KillPort process in RA
}
//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::SendAllocateMixResourcesRequest()
{
	ALLOC_PARTY_REQ_PARAMS_S stAllocateAdditionalPartyParams;
	memset(&stAllocateAdditionalPartyParams, 0, sizeof(ALLOC_PARTY_REQ_PARAMS_S));

	FillAdditionalAllocPartyParams(stAllocateAdditionalPartyParams);

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&stAllocateAdditionalPartyParams), sizeof(ALLOC_PARTY_REQ_PARAMS_S));

	SendReqToResourceAllocator(seg, AVC_SVC_ADDITIONAL_PARTY_RSRC_REQ);
}
//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::FillAdditionalAllocPartyParams(ALLOC_PARTY_REQ_PARAMS_S &stAllocateAdditionalPartyParams)
{
	stAllocateAdditionalPartyParams.party_id                       = GetPartyRsrcId();
	stAllocateAdditionalPartyParams.monitor_conf_id                = m_monitorConfId;
	stAllocateAdditionalPartyParams.monitor_party_id               = m_monitorPartyId;
	stAllocateAdditionalPartyParams.confMediaType                  = m_pPartyAllocatedRsrc->GetConfMediaType();
	stAllocateAdditionalPartyParams.room_id                        = GetPartyCntlAllocatedRsrc()->GetRoomId();
	stAllocateAdditionalPartyParams.networkPartyType               = eISDN_network_party_type;
	stAllocateAdditionalPartyParams.videoPartyType                 = eVideo_party_type_none;
	stAllocateAdditionalPartyParams.sessionType                    = eCP_session;
	stAllocateAdditionalPartyParams.serviceId                      = m_serviceId;
	stAllocateAdditionalPartyParams.subServiceId                   = m_subServiceId;
	stAllocateAdditionalPartyParams.optionsMask                    = 0;
	stAllocateAdditionalPartyParams.bRmxPortGaugeThresholdExceeded = false;
	stAllocateAdditionalPartyParams.isIceParty                     = false;
	stAllocateAdditionalPartyParams.artCapacity                    = 0;
	stAllocateAdditionalPartyParams.tipPartyType                   = eTipNone;
	stAllocateAdditionalPartyParams.isBFCP                         = false;
	stAllocateAdditionalPartyParams.allocationPolicy               = eNoAllocationPolicy;
	stAllocateAdditionalPartyParams.isWaitForRsrcAndAskAgain       = NO;
	stAllocateAdditionalPartyParams.tipNumOfScreens                = 0;

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	PASSERT_AND_RETURN(!pConfParty);
	strcpy((char*)(stAllocateAdditionalPartyParams.isdn_span_params.serviceName), pConfParty->GetServiceProviderName());

	WORD spanId  = DUMMY_SPAN_ID;
	WORD boardId = DUMMY_BOARD_ID;

	if (DIALIN == m_type)
	{
		boardId = m_netSetUp.m_boardId;
		spanId  = m_netSetUp.m_spanId[0];
	}
	stAllocateAdditionalPartyParams.isdn_span_params.span_id           = spanId;
	stAllocateAdditionalPartyParams.isdn_span_params.board_id          = boardId;
	stAllocateAdditionalPartyParams.isdn_span_params.num_of_isdn_ports = 1; // rons
}
//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::AddAdditionalPartyRsrcToRsrcDescVector(ALLOC_PARTY_IND_PARAMS_S_BASE  &stAllocateAdditionalPartyIndParamsBase)
{
	DWORD numOfAllRsrc = stAllocateAdditionalPartyIndParamsBase.numRsrcs;

	for(DWORD i= 0; i<numOfAllRsrc; i++)
	{
		DWORD connectionId = stAllocateAdditionalPartyIndParamsBase.allocatedRrcs[i].connectionId;
		eLogicalResourceTypes logicalRsrcType = stAllocateAdditionalPartyIndParamsBase.allocatedRrcs[i].logicalRsrcType;
		VECTOR_OF_RSRC_DESC* pCurrentRsrcDescVector = m_pPartyAllocatedRsrc->GetRsrcVector();
		bool bIsNewRsrc = true;
		PASSERT_AND_RETURN(!pCurrentRsrcDescVector);
	    for(DWORD j=0; j<pCurrentRsrcDescVector->size(); j++)
		{
			if((*pCurrentRsrcDescVector)[j].GetConnectionId() == connectionId)
			{
				bIsNewRsrc = false;
			}
		}
		if (bIsNewRsrc)
		{
//			pCurrentRsrcDescVector->push_back(CRsrcDesc(connectionId, logicalRsrcType));
			CRsrcDesc pTmpRsrcDesc (connectionId, logicalRsrcType);
			m_pPartyAllocatedRsrc->AddNewRsrcDesc(&pTmpRsrcDesc);
			TRACEINTO << "PartyId:"<< GetPartyRsrcId()
					  << "logicalRsrcType:" << (WORD)stAllocateAdditionalPartyIndParamsBase.allocatedRrcs[i].logicalRsrcType
					  << "connectionId:" << stAllocateAdditionalPartyIndParamsBase.allocatedRrcs[i].connectionId;
		}
	}
}
//--------------------------------------------------------------------------
void CConnectedIsdnVoicePartyCntl::DumpAllocIndToTrace(ALLOC_PARTY_IND_PARAMS_S_BASE& pParam)
{
  std::ostringstream msg;
  msg << "CConnectedIsdnVoicePartyCntl::DumpAllocIndToTrace"
      << "\nALLOC_PARTY_IND_PARAMS_S:"
      << "\n  status                         :" << CProcessBase::GetProcess()->GetStatusAsString(pParam.status).c_str()
      << "\n  rsrc_conf_id                   :" << pParam.rsrc_conf_id
      << "\n  rsrc_party_id                  :" << pParam.rsrc_party_id
      << "\n  room_id                        :" << pParam.room_id
      << "\n  numRsrcs                       :" << pParam.numRsrcs
      << "\n  videoPartyType                 :" << eVideoPartyTypeNames[pParam.videoPartyType]
      << "\n  networkPartyType               :" << eNetworkPartyTypeNames[pParam.networkPartyType]
      << "\n  isIce                          :" << (WORD)pParam.isIceParty
      << "\n  confMediaType                  :" << pParam.confMediaType;
  for (WORD i = 0; i < pParam.numRsrcs; i++)
    msg << "\n  connectionId[" << (i+1) << "] :" << (pParam.allocatedRrcs[i]).connectionId << ", logicalRsrcType:" << ::LogicalResourceTypeToString((pParam.allocatedRrcs[i]).logicalRsrcType);


  PTRACE(eLevelInfoNormal, msg.str().c_str());
}


