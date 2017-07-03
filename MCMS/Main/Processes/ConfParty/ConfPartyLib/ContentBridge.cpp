#include "ContentBridge.h"
#include "Stopper.h"

PBEGIN_MESSAGE_MAP(CContentBridge)

	//  standard disconnect to all bridges from conf
	ONEVENT(DISCONNECTCONF             ,CONNECT_INACTIVE ,CContentBridge::OnConfDisConnectConfCONNECT)
	ONEVENT(DISCONNECTCONF             ,CONNECT          ,CContentBridge::OnConfDisConnectConfCONNECT)
	ONEVENT(DISCONNECTCONF             ,CHANGERATE       ,CContentBridge::OnConfDisConnectConfCHANGERATE)
	ONEVENT(DISCONNECTCONF             ,CONTENT          ,CContentBridge::OnConfDisConnectConfCONTENT)
	ONEVENT(DISCONNECTCONF             ,DISCONNECTING    ,CContentBridge::OnConfDisConnectConfDISCONNECTING)

	ONEVENT(TERMINATE                  ,IDLE             ,CContentBridge::NullActionFunction)
	ONEVENT(TERMINATE                  ,CONNECT_INACTIVE ,CContentBridge::OnConfTerminate)
	ONEVENT(TERMINATE                  ,CONNECT          ,CContentBridge::OnConfTerminate)
	ONEVENT(TERMINATE                  ,CHANGERATE       ,CContentBridge::OnConfTerminate)
	ONEVENT(TERMINATE                  ,CONTENT          ,CContentBridge::OnConfTerminate)
	ONEVENT(TERMINATE                  ,DISCONNECTING    ,CContentBridge::OnConfTerminateDISCONNECTING)

	//  connect party from conf
	ONEVENT(CONNECTPARTY               ,IDLE             ,CContentBridge::OnConfConnectPartyCONNECT)
	ONEVENT(CONNECTPARTY               ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(CONNECTPARTY               ,CONNECT          ,CContentBridge::OnConfConnectPartyCONNECT)
	ONEVENT(CONNECTPARTY               ,CHANGERATE       ,CContentBridge::OnConfConnectPartyCHANGERATE)
	ONEVENT(CONNECTPARTY               ,CONTENT          ,CContentBridge::OnConfConnectPartyCONTENT)
	ONEVENT(CONNECTPARTY               ,DISCONNECTING    ,CContentBridge::OnConfConnectPartyDISCONNECTING)

	//  disconnect party from conf
	ONEVENT(DISCONNECTPARTY            ,CONNECT_INACTIVE ,CContentBridge::OnConfDisConnectPartyCONNECT_INACTIVE)
	ONEVENT(DISCONNECTPARTY            ,CONNECT          ,CContentBridge::OnConfDisConnectPartyCONNECT)
	ONEVENT(DISCONNECTPARTY            ,CHANGERATE       ,CContentBridge::OnConfDisConnectPartyCHANGERATE)
	ONEVENT(DISCONNECTPARTY            ,CONTENT          ,CContentBridge::OnConfDisConnectPartyCONTENT)
	ONEVENT(DISCONNECTPARTY            ,DISCONNECTING    ,CContentBridge::OnConfDisConnectPartyDISCONNECTING)

	//  token_withdraw message from CONF
	ONEVENT(TOKEN_WITHDRAW             ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(TOKEN_WITHDRAW             ,CONNECT          ,CContentBridge::OnConfTokenWithdrawCONNECT)
	ONEVENT(TOKEN_WITHDRAW             ,CHANGERATE       ,CContentBridge::OnConfTokenWithdrawCHANGERATE)
	ONEVENT(TOKEN_WITHDRAW             ,CONTENT          ,CContentBridge::OnConfTokenWithdrawCONTENT)
	ONEVENT(TOKEN_WITHDRAW             ,DISCONNECTING    ,CContentBridge::OnConfTokenWithdrawDISCONNECTING)

	//  new content rate from conf
	ONEVENT(NEWCONTENTRATE             ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(NEWCONTENTRATE             ,CONNECT          ,CContentBridge::OnConfNewRateCONNECT)
	ONEVENT(NEWCONTENTRATE             ,CHANGERATE       ,CContentBridge::OnConfNewRateCHANGERATE)
	ONEVENT(NEWCONTENTRATE             ,CONTENT          ,CContentBridge::OnConfNewRateCONTENT)
	ONEVENT(NEWCONTENTRATE             ,DISCONNECTING    ,CContentBridge::OnConfNewRateDISCONNECTING)

	//  new content rate and Protocol from conf
	ONEVENT(NEWCONTENTSCM              ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(NEWCONTENTSCM              ,CONNECT          ,CContentBridge::OnConfNewScmCONNECT)
	ONEVENT(NEWCONTENTSCM              ,CHANGERATE       ,CContentBridge::OnConfNewScmCHANGERATE)
	ONEVENT(NEWCONTENTSCM              ,CONTENT          ,CContentBridge::OnConfNewScmCONTENT)
	ONEVENT(NEWCONTENTSCM              ,DISCONNECTING    ,CContentBridge::OnConfNewScmDISCONNECTING)

	ONEVENT(UPDATEONLECTUREMODE        ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(UPDATEONLECTUREMODE        ,CONNECT          ,CContentBridge::OnConfUpdateLectureModeConnect)
	ONEVENT(UPDATEONLECTUREMODE        ,CHANGERATE       ,CContentBridge::OnConfUpdateLectureModeChangeRate)
	ONEVENT(UPDATEONLECTUREMODE        ,CONTENT          ,CContentBridge::OnConfUpdateLectureModeContent)
	ONEVENT(UPDATEONLECTUREMODE        ,DISCONNECTING    ,CContentBridge::NullActionFunction)

	///////////////////////////////////////////////////////////////
	//  party control events

	//  end connect party from party
	ONEVENT(ENDCONNECTPARTY            ,CONNECT_INACTIVE ,CContentBridge::OnEndPartyConnectCONNECT_INACTIVE)
	ONEVENT(ENDCONNECTPARTY            ,CONNECT          ,CContentBridge::OnEndPartyConnectCONNECT)
	ONEVENT(ENDCONNECTPARTY            ,CHANGERATE       ,CContentBridge::OnEndPartyConnectCHANGERATE)
	ONEVENT(ENDCONNECTPARTY            ,CONTENT          ,CContentBridge::OnEndPartyConnectCONTENT)
	ONEVENT(ENDCONNECTPARTY            ,DISCONNECTING    ,CContentBridge::OnEndPartyConnectDISCONNECTING)

	//  end party disconnect from party
	ONEVENT(ENDDISCONNECTPARTY         ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(ENDDISCONNECTPARTY         ,CONNECT          ,CContentBridge::OnEndPartyDisConnectCONNECT)
	ONEVENT(ENDDISCONNECTPARTY         ,CHANGERATE       ,CContentBridge::OnEndPartyDisConnectCHANGERATE)
	ONEVENT(ENDDISCONNECTPARTY         ,CONTENT          ,CContentBridge::OnEndPartyDisConnectCONTENT)
	ONEVENT(ENDDISCONNECTPARTY         ,DISCONNECTING    ,CContentBridge::OnEndPartyDisConnectDISCONNECTING)

	//  party content rate changed from ChangeModeCntl
	ONEVENT(PARTY_CONTENT_RATE_CHANGED ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(PARTY_CONTENT_RATE_CHANGED ,CONNECT          ,CContentBridge::OnPartyRateChangedCONNECT)
	ONEVENT(PARTY_CONTENT_RATE_CHANGED ,CHANGERATE       ,CContentBridge::OnPartyRateChangedCHANGERATE)
	ONEVENT(PARTY_CONTENT_RATE_CHANGED ,CONTENT          ,CContentBridge::OnPartyRateChangedCONTENT)
	ONEVENT(PARTY_CONTENT_RATE_CHANGED ,DISCONNECTING    ,CContentBridge::OnPartyRateChangedDISCONNECTING)

	//  party asks for intra
	ONEVENT(CONTENTVIDREFRESH          ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(CONTENTVIDREFRESH          ,CONNECT          ,CContentBridge::OnPartyRefreshContentCONNECT)
	ONEVENT(CONTENTVIDREFRESH          ,CHANGERATE       ,CContentBridge::OnPartyRefreshContentCHANGERATE)
	ONEVENT(CONTENTVIDREFRESH          ,CONTENT          ,CContentBridge::OnPartyRefreshContentCONTENT)
	ONEVENT(CONTENTVIDREFRESH          ,DISCONNECTING    ,CContentBridge::OnPartyRefreshContentDISCONNECTING)

	// HW content Stream is ready
	ONEVENT(HW_CONTENT_ON_OFF_ACK      ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(HW_CONTENT_ON_OFF_ACK      ,CONNECT          ,CContentBridge::OnPartyHWContentOnOffAckCONNECT)
	ONEVENT(HW_CONTENT_ON_OFF_ACK      ,CHANGERATE       ,CContentBridge::OnPartyHWContentOnOffAckCHANGERATE)
	ONEVENT(HW_CONTENT_ON_OFF_ACK      ,CONTENT          ,CContentBridge::OnPartyHWContentOnOffAckCONTENT)
	ONEVENT(HW_CONTENT_ON_OFF_ACK      ,DISCONNECTING    ,CContentBridge::OnPartyHWContentOnOffAckDISCONNECTING)

	///////////////////////////////////////////////////////////////
	//  party task messages for bridge

	//  token_acquire  message	//   STATES
	ONEVENT(PARTY_TOKEN_ACQUIRE        ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE        ,IDLE             ,CContentBridge::OnPartyTokenAcquireIDLE )
	ONEVENT(PARTY_TOKEN_ACQUIRE        ,CONNECT          ,CContentBridge::OnPartyTokenAcquireCONNECT )
	ONEVENT(PARTY_TOKEN_ACQUIRE        ,CHANGERATE       ,CContentBridge::OnPartyTokenAcquireCHANGERATE)
	ONEVENT(PARTY_TOKEN_ACQUIRE        ,CONTENT          ,CContentBridge::OnPartyTokenAcquireCONTENT)
	ONEVENT(PARTY_TOKEN_ACQUIRE        ,DISCONNECTING    ,CContentBridge::OnPartyTokenAcquireDISCONNECTING)

	//  token_release  message
	ONEVENT(PARTY_TOKEN_RELEASE        ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(PARTY_TOKEN_RELEASE        ,CONNECT          ,CContentBridge::OnPartyTokenReleaseCONNECT)
	ONEVENT(PARTY_TOKEN_RELEASE        ,CHANGERATE       ,CContentBridge::OnPartyTokenReleaseCHANGERATE)
	ONEVENT(PARTY_TOKEN_RELEASE        ,CONTENT          ,CContentBridge::OnPartyTokenReleaseCONTENT)
	ONEVENT(PARTY_TOKEN_RELEASE        ,DISCONNECTING    ,CContentBridge::OnPartyTokenReleaseDISCONNECTING)

	//  token_withdraw_ack  message
	ONEVENT(PARTY_TOKEN_WITHDRAW_ACK   ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(PARTY_TOKEN_WITHDRAW_ACK   ,CONNECT          ,CContentBridge::OnPartyTokenWithdrawAckCONNECT)
	ONEVENT(PARTY_TOKEN_WITHDRAW_ACK   ,CHANGERATE       ,CContentBridge::OnPartyTokenWithdrawAckCHANGERATE)
	ONEVENT(PARTY_TOKEN_WITHDRAW_ACK   ,CONTENT          ,CContentBridge::OnPartyTokenWithdrawAckCONTENT )
	ONEVENT(PARTY_TOKEN_WITHDRAW_ACK   ,DISCONNECTING    ,CContentBridge::OnPartyTokenWithdrawAckDISCONNECTING )

	//  role_provider_identity  message
	ONEVENT(ROLE_PROVIDER_IDENTITY     ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(ROLE_PROVIDER_IDENTITY     ,CONNECT          ,CContentBridge::OnPartyRoleProviderIdentityCONNECT )
	ONEVENT(ROLE_PROVIDER_IDENTITY     ,CHANGERATE       ,CContentBridge::OnPartyRoleProviderIdentityCHANGERATE)
	ONEVENT(ROLE_PROVIDER_IDENTITY     ,CONTENT          ,CContentBridge::OnPartyRoleProviderIdentityCONTENT )
	ONEVENT(ROLE_PROVIDER_IDENTITY     ,DISCONNECTING    ,CContentBridge::OnPartyRoleProviderIdentityDISCONNECTING )

	//  media_producer_status  message
	ONEVENT(MEDIA_PRODUCER_STATUS      ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(MEDIA_PRODUCER_STATUS      ,CONNECT          ,CContentBridge::OnPartyMediaProducerStatusCONNECT)
	ONEVENT(MEDIA_PRODUCER_STATUS      ,CHANGERATE       ,CContentBridge::OnPartyMediaProducerStatusCHANGERATE)
	ONEVENT(MEDIA_PRODUCER_STATUS      ,CONTENT          ,CContentBridge::OnPartyMediaProducerStatusCONTENT)
	ONEVENT(MEDIA_PRODUCER_STATUS      ,DISCONNECTING    ,CContentBridge::OnPartyMediaProducerStatusDISCONNECTING)

	//  token_query  message	//   STATES
	ONEVENT(PARTY_BFCP_TOKEN_QUERY     ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(PARTY_BFCP_TOKEN_QUERY     ,IDLE             ,CContentBridge::NullActionFunction)//OnPartyBfcpTokenQueryIDLE )
	ONEVENT(PARTY_BFCP_TOKEN_QUERY     ,CONNECT          ,CContentBridge::OnPartyBfcpTokenQueryCONNECT )
	ONEVENT(PARTY_BFCP_TOKEN_QUERY     ,CHANGERATE       ,CContentBridge::OnPartyBfcpTokenQueryCHANGERATE)
	ONEVENT(PARTY_BFCP_TOKEN_QUERY     ,CONTENT          ,CContentBridge::OnPartyBfcpTokenQueryCONTENT)
	ONEVENT(PARTY_BFCP_TOKEN_QUERY     ,DISCONNECTING    ,CContentBridge::OnPartyBfcpTokenQueryDISCONNECTING)

	///////////////////////////////////////////////////////////////
	//  TOKEN messages for bridge

	//  new_token_holder message
	ONEVENT(NEWTOKENHOLDER             ,IDLE             ,CContentBridge::NullActionFunction)
	ONEVENT(NEWTOKENHOLDER             ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(NEWTOKENHOLDER             ,CONNECT          ,CContentBridge::OnTokenNewTokenHolderCONNECT)
	ONEVENT(NEWTOKENHOLDER             ,CHANGERATE       ,CContentBridge::OnTokenNewTokenHolderCHANGERATE)
	ONEVENT(NEWTOKENHOLDER             ,CONTENT          ,CContentBridge::OnTokenNewTokenHolderCONTENT)
	ONEVENT(NEWTOKENHOLDER             ,DISCONNECTING    ,CContentBridge::OnTokenNewTokenHolderDISCONNECTING)

	//  no_token_holder message
	ONEVENT(NOTOKENHOLDER              ,IDLE             ,CContentBridge::NullActionFunction)
	ONEVENT(NOTOKENHOLDER              ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(NOTOKENHOLDER              ,CONNECT          ,CContentBridge::OnTokenNoTokenHolderCONNECT)
	ONEVENT(NOTOKENHOLDER              ,CHANGERATE       ,CContentBridge::OnTokenNoTokenHolderCHANGERATE)
	ONEVENT(NOTOKENHOLDER              ,CONTENT          ,CContentBridge::OnTokenNoTokenHolderCONTENT)
	ONEVENT(NOTOKENHOLDER              ,DISCONNECTING    ,CContentBridge::OnTokenNoTokenHolderDISCONNECTING)

	ONEVENT(CONTENTFREEZEPIC           ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(CONTENTFREEZEPIC           ,CONNECT          ,CContentBridge::OnTokenFreezContentCONNECT)
	ONEVENT(CONTENTFREEZEPIC           ,CHANGERATE       ,CContentBridge::OnTokenFreezContentCHANGERATE)
	ONEVENT(CONTENTFREEZEPIC           ,CONTENT          ,CContentBridge::OnTokenFreezContentCONTENT)
	ONEVENT(CONTENTFREEZEPIC           ,DISCONNECTING    ,CContentBridge::OnTokenFreezContentDISCONNECTING)

	//  NOROLEPROVIDER Token message
	ONEVENT(TOKENNOROLEPROVIDER        ,IDLE             ,CContentBridge::NullActionFunction)
	ONEVENT(TOKENNOROLEPROVIDER        ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(TOKENNOROLEPROVIDER        ,CONNECT          ,CContentBridge::OnTokenNoProviderCONNECT)
	ONEVENT(TOKENNOROLEPROVIDER        ,CHANGERATE       ,CContentBridge::OnTokenNoProviderCHANGERATE)
	ONEVENT(TOKENNOROLEPROVIDER        ,CONTENT          ,CContentBridge::OnTokenNoProviderCONTENT)
	ONEVENT(TOKENNOROLEPROVIDER        ,DISCONNECTING    ,CContentBridge::OnTokenNoProviderDISCONNECTING)

	///////////////////////////////////////////////////////////////
	//  timers messages

	//  CHANGERATE timer message
	ONEVENT(CHANGERATETOUT             ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(CHANGERATETOUT             ,CONNECT          ,CContentBridge::OnTimerChangeRateCONNECT)
	ONEVENT(CHANGERATETOUT             ,CHANGERATE       ,CContentBridge::OnTimerChangeRateCHANGERATE)
	ONEVENT(CHANGERATETOUT             ,CONTENT          ,CContentBridge::OnTimerChangeRateCONTENT )
	ONEVENT(CHANGERATETOUT             ,DISCONNECTING    ,CContentBridge::OnTimerChangeRateDISCONNECTING )

	//Restricted content
	ONEVENT(UPDATEEXCLUSIVECONTENT     ,CONNECT_INACTIVE ,CContentBridge::NullActionFunction)
	ONEVENT(UPDATEEXCLUSIVECONTENT     ,CONNECT          ,CContentBridge::OnConfUpdateExclusiveContentConnect)
	ONEVENT(UPDATEEXCLUSIVECONTENT     ,CHANGERATE       ,CContentBridge::OnConfUpdateExclusiveContentChangeRate)
	ONEVENT(UPDATEEXCLUSIVECONTENT     ,CONTENT          ,CContentBridge::OnConfUpdateExclusiveContentContent)
	ONEVENT(UPDATEEXCLUSIVECONTENT     ,DISCONNECTING    ,CContentBridge::NullActionFunction)

	//Restricted content
	ONEVENT(DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER ,CONNECT_INACTIVE  ,CContentBridge::NullActionFunction)
	ONEVENT(DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER ,CONNECT           ,CContentBridge::OnTimerDisableIntraAfterStartContent)
	ONEVENT(DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER ,CHANGERATE        ,CContentBridge::OnTimerDisableIntraAfterStartContent)
	ONEVENT(DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER ,CONTENT           ,CContentBridge::OnTimerDisableIntraAfterStartContent)
	ONEVENT(DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER ,DISCONNECTING     ,CContentBridge::NullActionFunction)

	ONEVENT(UPDATEEXCLUSIVECONTENTMODE ,CONNECT_INACTIVE  ,CContentBridge::NullActionFunction)

	ONEVENT(UPDATEEXCLUSIVECONTENTMODE ,CONNECT           ,CContentBridge::OnConfUpdateExclusiveContentModeConnect)
	ONEVENT(UPDATEEXCLUSIVECONTENTMODE ,CHANGERATE        ,CContentBridge::OnConfUpdateExclusiveContentModeChangeRate)
	ONEVENT(UPDATEEXCLUSIVECONTENTMODE ,CONTENT           ,CContentBridge::OnConfUpdateExclusiveContentModeContent)
	ONEVENT(UPDATEEXCLUSIVECONTENTMODE ,DISCONNECTING     ,CContentBridge::NullActionFunction)

	ONEVENT(CONTENTSLAVELINKINTRATOUT  ,CONNECT_INACTIVE  ,CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressedNoContent)
	ONEVENT(CONTENTSLAVELINKINTRATOUT  ,CONNECT           ,CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressedNoContent)
	ONEVENT(CONTENTSLAVELINKINTRATOUT  ,CHANGERATE        ,CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressedChangeRate)
	ONEVENT(CONTENTSLAVELINKINTRATOUT  ,CONTENT           ,CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressedContent)
	ONEVENT(CONTENTSLAVELINKINTRATOUT  ,DISCONNECTING     ,CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressedNoContent)

	// VNGFE-7734
	ONEVENT(START_CONTENT_LOAD_TIMER_01,DISCONNECTING     ,CContentBridge::NullActionFunction)
	ONEVENT(START_CONTENT_LOAD_TIMER_01,ANYCASE           ,CContentBridge::OnTokenNewTokenHolderContinue01)
	ONEVENT(START_CONTENT_LOAD_TIMER_02,DISCONNECTING     ,CContentBridge::NullActionFunction)
	ONEVENT(START_CONTENT_LOAD_TIMER_02,ANYCASE           ,CContentBridge::OnTokenNewTokenHolderContinue02)
	ONEVENT(START_CONTENT_LOAD_TIMER_03,DISCONNECTING     ,CContentBridge::NullActionFunction)
	ONEVENT(START_CONTENT_LOAD_TIMER_03,ANYCASE           ,CContentBridge::OnTokenNewTokenHolderContinue03)

PEND_MESSAGE_MAP(CContentBridge,CBridge);

////////////////////////////////////////////////////////////////////////////
//                        CContentBridge
////////////////////////////////////////////////////////////////////////////
CContentBridge::CContentBridge(void)
{
	m_byCurrentContentRate              = AMC_0k;
	m_byCurrentContentProtocol          = H264;
	m_pToken                            = NULL;
	m_pCascadeSlavePartiesList          = NULL;
	m_bCascadeSlaveIntraRequestsPending = NO;
	m_isSpeakerHWReadyToContent         = NO;
	m_isExclusiveContentMode            = NO;
	m_isIntraSupressionEnabled          = true;
	m_bIsRcvArtContentOn                = FALSE;
	m_byH264HighProfile			= FALSE; //HP content
}

//--------------------------------------------------------------------------
CContentBridge::~CContentBridge(void)
{
	POBJDELETE(m_pToken);
	DeleteTimer(CONTENTSLAVELINKINTRATOUT);
	if (m_pCascadeSlavePartiesList)
		POBJDELETE(m_pCascadeSlavePartiesList);
}

//--------------------------------------------------------------------------
void* CContentBridge::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CContentBridge::Destroy()
{
	CBridge::Destroy();
}

//--------------------------------------------------------------------------
void CContentBridge::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
BYTE CContentBridge::HaveIAnotherLink(CContentBridgePartyCntl* pLinkParty)
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if ((pPartyCntl != pLinkParty) && (NONE != pPartyCntl->GetCascadeLinkMode()))
				return YES;
		}
	}
	return NO;
}

//--------------------------------------------------------------------------
void CContentBridge::SendMessagesToRVGW(WORD opcode, BYTE McuNum, BYTE TermNum, BYTE sendToSpeaker)
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			PartyRsrcID partyId = pPartyCntl->GetPartyRsrcID();
			CParty* pParty = (CParty*)GetLookupTableParty()->Get(partyId);
			if (pParty)
			{
				if (sendToSpeaker != YES && IsTokenHolder(pParty))
					continue;

				if (pParty->IsRemoteIsRVpresentation())
					pPartyCntl->ForwardToParty(opcode, McuNum, TermNum);
			}
		}
	}
}

//--------------------------------------------------------------------------
void CContentBridge::CreateInActive(const CContentBridgeInitParams* pContentInitParams)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	if (!pContentInitParams)
	{
		PASSERTMSG(1, "Invalid ContentBridgeInitParams");
		m_pConfApi->EndContentBrdgConnect(statInconsistent);
		return;
	}

	CBridge::Create(pContentInitParams);
	AllocateLinkPartyList();
	m_byCurrentContentProtocol = pContentInitParams->GetContentProtocol();
	m_byH264HighProfile = pContentInitParams->GetContentH264HighProfile();  //HP content

	// token initialization
	m_pToken = new CContentToken;

	SetSpeakerHWReadyToContent(NO); // Speaker HW is NOT ready yet
	SetExclusiveContentMode(pContentInitParams->GetIsExclusiveContentMode());
	m_state = CONNECT_INACTIVE;
	m_pConfApi->EndContentBrdgConnect(statOK);
}

//--------------------------------------------------------------------------
void CContentBridge::Create(const CContentBridgeInitParams* pContentInitParams)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	if (!pContentInitParams)
	{
		PASSERTMSG(1, "Invalid ContentBridgeInitParams");
		m_pConfApi->EndContentBrdgConnect(statInconsistent);
		return;
	}

	CBridge::Create(pContentInitParams);
	AllocateLinkPartyList();
	m_byCurrentContentProtocol = pContentInitParams->GetContentProtocol();
	SetExclusiveContentMode(pContentInitParams->GetIsExclusiveContentMode());
	m_byH264HighProfile = pContentInitParams->GetContentH264HighProfile();  //HP content

	// token initialization
	m_pToken = new CContentToken;
	m_pToken->InitToken(m_pConf->GetRcvMbx(), ((CTaskApi*)m_pConfApi));
	SetSpeakerHWReadyToContent(NO); // Speaker HW is NOT ready yet
	m_state = CONNECT;
	m_pConfApi->EndContentBrdgConnect(statOK);
}

//--------------------------------------------------------------------------
void CContentBridge::Create(const CContentBridge* pOtherBridge)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	(CBridge&)(*this) = (CBridge&)(*pOtherBridge);

	if (m_pToken)
		POBJDELETE(m_pToken);

	m_pToken = new CContentToken;
	(CContentToken&)(*m_pToken) = (CContentToken&)(*(pOtherBridge->m_pToken));
	AllocateLinkPartyList();
	if (pOtherBridge->GetCascadeLinkSlavePartyList() != NULL)
		*m_pCascadeSlavePartiesList = *(pOtherBridge->GetCascadeLinkSlavePartyList());

	m_byCurrentContentRate     = pOtherBridge->m_byCurrentContentRate;
	m_byCurrentContentProtocol = pOtherBridge->m_byCurrentContentProtocol;
	m_byH264HighProfile = pOtherBridge->m_byH264HighProfile;  //HP content
	SetSpeakerHWReadyToContent(((CContentBridge*)pOtherBridge)->IsSpeakerHWReadyToContent());
	SetExclusiveContentMode(((CContentBridge*)pOtherBridge)->IsExclusiveContentMode());

	m_isIntraSupressionEnabled = pOtherBridge->m_isIntraSupressionEnabled;
	m_bIsRcvArtContentOn       = pOtherBridge->m_bIsRcvArtContentOn;
}

//--------------------------------------------------------------------------
CContentBridge& CContentBridge::operator=(const CContentBridge& rOtherBridge)
{
	if (&rOtherBridge == this)
		return *this;

	(CBridge&)(*this) = (CBridge&)rOtherBridge;

	if (m_pToken)
		POBJDELETE(m_pToken);

	m_pToken  = new CContentToken;
	*m_pToken = (*rOtherBridge.m_pToken);

	SetSpeakerHWReadyToContent(((CContentBridge&)rOtherBridge).IsSpeakerHWReadyToContent());
	SetSpeakerHWReadyToContent(((CContentBridge&)rOtherBridge).IsSpeakerHWReadyToContent());
	SetExclusiveContentMode(((CContentBridge&)rOtherBridge).IsExclusiveContentMode());

	m_isIntraSupressionEnabled = rOtherBridge.m_isIntraSupressionEnabled;
	m_bIsRcvArtContentOn       = rOtherBridge.m_bIsRcvArtContentOn;

	return *this;
}

//--------------------------------------------------------------------------
void CContentBridge::DisconnectParty(const CBridgePartyDisconnectParams* pBridgePartyDisconnectParams)
{
	CBridge::DisconnectParty(pBridgePartyDisconnectParams);
}

//--------------------------------------------------------------------------
void CContentBridge::DisconnectParty(const CTaskApp* pParty)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (void*)pParty;
	DispatchEvent(DISCONNECTPARTY, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentBridge::ContentRate(const BYTE newContentRate)
{
	CSegment* pSeg = new CSegment;
	*pSeg << newContentRate;
	DispatchEvent(NEWCONTENTRATE, pSeg);
	POBJDELETE(pSeg);
}
//--------------------------------------------------------------------------
void CContentBridge::ContentProtocol(const BYTE newContentProtocol, const BYTE isH264HighProfile)  //HP content
{
	CSegment* pSeg = new CSegment;
	*pSeg << newContentProtocol;
	//HP content
	if (newContentProtocol == H264)
		*pSeg << isH264HighProfile;
	DispatchEvent(NEWCONTENTSCM, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
BYTE CContentBridge::IsValidContentRate(const BYTE rate) const
{
	BYTE result = NO;
	switch (rate)
	{
		case AMC_0k:
		case AMC_64k:
		case AMC_128k:
		case AMC_192k:
		case AMC_256k:
		case AMC_384k:
		case AMC_512k:
		case AMC_768k:
		case AMC_1024k:
		case AMC_1152k:
		case AMC_1280k:
		case AMC_1536k:
		case AMC_2048k:
		case AMC_2560k:
		case AMC_3072k:
		case AMC_4096k:
		{
			result = YES;
			break;
		}
		default:
		{
		    TRACEINTO << "Invalid content rate: " << (int)rate;
		}
	}
	return result;
}

//--------------------------------------------------------------------------
BYTE CContentBridge::IsValidContentProtocol(const BYTE protocol) const
{
	BOOL resultValue = FALSE;

	switch (protocol)
	{
		case H263:
		case H264:
		{
			resultValue = TRUE;
			break;
		}
		default:
		{
		    TRACEINTO << "Invalid content protocol: " << (int)protocol;
		}
	} // switch

	return resultValue;
}

//--------------------------------------------------------------------------
char* CContentBridge::GetContentRateAsString(const BYTE contentRate)
{
	switch (contentRate)
	{
		case AMC_0k   : return "AMC_0k";
		case AMC_64k  : return "AMC_64k";
		case AMC_128k : return "AMC_128k";
		case AMC_192k : return "AMC_192k";
		case AMC_256k : return "AMC_256k";
		case AMC_384k : return "AMC_384k";
		case AMC_512k : return "AMC_512k";
		case AMC_768k : return "AMC_768k";
		case AMC_1024k: return "AMC_1024k";
		case AMC_1152k: return "AMC_1152k";
		case AMC_1280k: return "AMC_1280k";
		case AMC_1536k: return "AMC_1536k";
		case AMC_2048k: return "AMC_2048k";
		case AMC_2560k: return "AMC_2560k";
		case AMC_3072k: return "AMC_3072k";
		case AMC_4096k: return "AMC_4096k";
	}
	return "Unknown";
}
//--------------------------------------------------------------------------
char* CContentBridge::GetContentProtocolAsString(const BYTE contentProtocol) const
{
	switch (contentProtocol)
	{
		case H264: return "H264";
		case H263: return "H263";
	}
	return "Unknown";
}

//--------------------------------------------------------------------------
DWORD CContentBridge::GetContentRate(BYTE contentRate) const
{
	switch (contentRate)
	{
		case AMC_0k   : return 0 * 10;
		case AMC_64k  : return 64 * 10;
		case AMC_128k : return 128 * 10;
		case AMC_192k : return 192 * 10;
		case AMC_256k : return 256 * 10;
		case AMC_384k : return 384 * 10;
		case AMC_512k : return 512 * 10;
		case AMC_768k : return 768 * 10;
		case AMC_1024k: return 1024 * 10;
		case AMC_1152k: return 1152 * 10;
		case AMC_1280k: return 1280 * 10;
		case AMC_1536k: return 1536 * 10;
		case AMC_2048k: return 2048 * 10;
		case AMC_2560k: return 2560 * 10;
		case AMC_3072k: return 3072 * 10;
		case AMC_4096k: return 4096 * 10;
	}
	return 0;
}

//--------------------------------------------------------------------------
BYTE CContentBridge::IsTokenHolder(const CTaskApp* pParty) const
{
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pParty), NO);
	return (m_pToken->GetTokenHolderPartyId() == pParty->GetPartyId()) ? YES : NO;
}

//--------------------------------------------------------------------------
BYTE CContentBridge::IsTokenHeld()
{
	if (m_state == CONNECT_INACTIVE)
		return NO;

	return m_pToken->GetTokenHolderPartyId() ? YES : NO;
}

//--------------------------------------------------------------------------
const CTaskApp* CContentBridge::GetTokenHolder(BYTE& mcuNum, BYTE& terminalNum)
{
	const CParty* pSpeakerParty = (CParty*)m_pToken->GetTokenHolderParty();
	if (pSpeakerParty)
	{
		CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(pSpeakerParty->GetPartyRsrcID());
		if (pSpeakerPartyCntl)
		{
			mcuNum      = pSpeakerPartyCntl->GetMcuNum();
			terminalNum = pSpeakerPartyCntl->GetTermNum();
		}
	}
	return pSpeakerParty;
}

//--------------------------------------------------------------------------
void CContentBridge::IsRateChangeDone()
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (!pPartyCntl)
			continue;

		if ((pPartyCntl->GetPartyContentRate() != m_byCurrentContentRate) &&
		    (!IsXCodeConf() || (IsXCodeConf() && (m_byCurrentContentRate == AMC_0k || pPartyCntl->GetPartyContentRate() == AMC_0k))))
		{
			TRACEINTO
				<< "PartyId:" << pPartyCntl->GetPartyRsrcID()
				<< ", PartyName:" << pPartyCntl->GetName()
				<< ", PartyContentRate:" << GetContentRateAsString(pPartyCntl->GetPartyContentRate())
				<< ", ConfContentRate:" << GetContentRateAsString(m_byCurrentContentRate)
				<< " - Content rate still not changed";
			return;
		}
	}
	TRACEINTO << "ConfName:" << m_pConfName << " - Rate changed";

	DeleteTimer(CHANGERATETOUT);
	ChangePresentationRate();

	if (YES == IsSpeakerHWReadyToContent())
		AskSpeakerForContentIntra();
}

//--------------------------------------------------------------------------
void CContentBridge::AskSpeakerForContentIntra()
{
	const CParty* pSpeaker = (CParty*)m_pToken->GetTokenHolderParty();
	if (!pSpeaker)
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, there is no content token holder";
		return;
	}

	PartyRsrcID partyId = pSpeaker->GetPartyRsrcID();
	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pPartyCntl, "PartyId:" << partyId);

	if (YES == IsSpeakerHWReadyToContent())
		pPartyCntl->SendRefresh();
	else
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, the HW is NOT ready for content yet";
}

//--------------------------------------------------------------------------
BYTE CContentBridge::IsPartyWaitForRateChange(const CTaskApp* pTask)
{
	const CParty* pParty = (CParty*)pTask;
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pParty), FALSE);

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN_VALUE(!pPartyCntl, FALSE);

	TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID()
	          << ", PartyContentRate:" << GetContentRateAsString(pPartyCntl->GetPartyContentRate())
	          << ", ConfContentRate:" << GetContentRateAsString(m_byCurrentContentRate);

	if (pPartyCntl->GetPartyContentRate() != m_byCurrentContentRate &&
	    (!IsXCodeConf() || (IsXCodeConf() && (m_byCurrentContentRate == 0 || pPartyCntl->GetPartyContentRate() == 0))))
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
CTaskApp* CContentBridge::GetPartyIdFromMcuAndTerminalNum(BYTE mcuNum, BYTE terminalNum)
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if (pPartyCntl->GetMcuNum() == mcuNum && pPartyCntl->GetTermNum() == terminalNum)
				return pPartyCntl->GetPartyTaskApp();
		}
	}
	return NULL;
}

//--------------------------------------------------------------------------
void CContentBridge::ChangePresentationRate()
{
	TRACEINTO << "ConfName:" << m_pConfName;
#ifdef CONTENT_LOAD
	CStopper changePresentationRateStopper("CContentBridge::ChangePresentationRate", m_pConfName);
#endif
	// get speaker party
	const CParty* pSpeakerParty = (CParty*)m_pToken->GetTokenHolderParty();

	// if there is no more presentation
	if (m_byCurrentContentRate == AMC_0k)
	{
		if (pSpeakerParty != NULL)
			DBGPASSERT(1);
	}
	else     // if a presentation goes on
	{
		// check if a token has a speaker
		if (pSpeakerParty == NULL)
		{
			AbortPresentation();
			// In case when NoTokenHolder received in CHANGE_RATE state - just stop presentation
			return;
		}

		PartyRsrcID speakerId = pSpeakerParty->GetPartyRsrcID();

		// if speaker' PartyControl don't exist on bridge
		CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(speakerId);
		if (!pSpeakerPartyCntl)
		{
			m_state = CONTENT;
			AbortPresentation();

			TRACEINTO << "SpeakerId:" << speakerId << " - Invalid speaker";

			DBGPASSERT_AND_RETURN(1);
		}

		// if Speaker didn't change it's rate
		// not needed, when presentation start, every parties receive
		if (pSpeakerPartyCntl->GetPartyContentRate() != m_byCurrentContentRate)
		{
			if (IsXCodeConf() && pSpeakerPartyCntl->GetPartyContentRate() > AMC_0k && m_byCurrentContentRate > AMC_0k)
			{
				TRACEINTO << "SpeakerId:" << speakerId << " - Party Content rate can be different from each party in XCode Conference";
			}
			else
			{
				TRACEINTO << "SpeakerId:" << speakerId
				          << ", SpeakerContentRate:" << GetContentRateAsString(pSpeakerPartyCntl->GetPartyContentRate())
				          << ", ConfContentRate:" << GetContentRateAsString(m_byCurrentContentRate);

				m_state = CONTENT;
				AbortPresentation();
				DBGPASSERT_AND_RETURN(1);
			}
		}
	}

	// for parties didn't change rate - remove from bridge for all others - changeRate
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			m_pConfApi->UpdateDB(pPartyCntl->GetPartyTaskApp(), CONTENTPROVIDER, NO);
			if (pPartyCntl->GetPartyContentRate() != m_byCurrentContentRate &&
			    (!IsXCodeConf() || (IsXCodeConf() && (m_byCurrentContentRate == 0 || pPartyCntl->GetPartyContentRate() == 0))))
			{
				PartyRsrcID partyId = pPartyCntl->GetPartyRsrcID();
				// send disconnecting partyID to token (for requester or speaker check)
				m_pToken->PartyDisconnects(partyId);
				if ((m_pCascadeSlavePartiesList->size() > 0) && (pPartyCntl->GetCascadeLinkMode() == CASCADE_MODE_MASTER))  // 'Remove' will return NULL if not found.
					m_pCascadeSlavePartiesList->Remove(partyId);

				// remove from bridge
				CBridge::DisconnectParty(partyId);
			}
			else
				pPartyCntl->ChangePresentationRate();
		}
	}

	// m_pConfApi->EndContentRateChange(m_byCurrentContentRate);
	if (m_byCurrentContentRate != AMC_0k)
	{
		m_pConfApi->UpdateDB((CTaskApp*)pSpeakerParty, CONTENTPROVIDER, YES);
		m_pConfApi->UpdateDB((CTaskApp*)pSpeakerParty, CONTENTSRC, 0);
		m_state = CONTENT;
	}

	else // close presentation
	{
		m_pConfApi->UpdateDB((CTaskApp*)0xffff, CONTENTSRC, 0);
		m_state = CONNECT;
	}
#ifdef CONTENT_LOAD
	changePresentationRateStopper.Stop();
#endif
}

//--------------------------------------------------------------------------
void CContentBridge::ChangePresentationSpeaker()
{
	TRACEINTO << "ConfName:" << m_pConfName;

	// if there is no presentation
	DBGPASSERT_AND_RETURN(m_byCurrentContentRate == AMC_0k);

	// check if a token has a speaker
	const CParty* pSpeakerParty = (CParty*)m_pToken->GetTokenHolderParty();
	if (pSpeakerParty == NULL)
	{
		AbortPresentation();
		DBGPASSERT_AND_RETURN(1);
	}

	PartyRsrcID speakerId = pSpeakerParty->GetPartyRsrcID();

	// if speaker' PartyControl don't exist on bridge
	CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(speakerId);
	if (!pSpeakerPartyCntl)
	{
		AbortPresentation();
		DBGPASSERT_AND_RETURN(1);
	}

	// for all parties - changeSpeaker
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			pPartyCntl->ChangePresentationSpeaker();
			m_pConfApi->UpdateDB(pPartyCntl->GetPartyTaskApp(), CONTENTPROVIDER, NO);
		}
	}

	m_pConfApi->UpdateDB((CTaskApp*)pSpeakerParty, CONTENTPROVIDER, YES);
	m_pConfApi->UpdateDB((CTaskApp*)pSpeakerParty, CONTENTSRC, 0);
}

//--------------------------------------------------------------------------
void CContentBridge::AbortPresentation()
{
	// check if a speaker connected
	PartyRsrcID partyId = m_pToken->GetTokenHolderPartyId();

	TRACEINTO << "ConfName:" << m_pConfName << ", TokenHolderPartyId:" << partyId;

	if (partyId)
	{
		m_pToken->DropTokenHolder(partyId); // Withdraw(pSpeakerParty);
		return;
	}

	m_pConfApi->StopContent();
}

//--------------------------------------------------------------------------
WORD CContentBridge::GetSpeakerProviderIdentity(BYTE& mcuNum, BYTE& termNum)
{
	const CParty* pSpeakerParty = (CParty*)m_pToken->GetTokenHolderParty();
	if (!pSpeakerParty)
		return statIllegal;

	// check if party is a bridge member
	CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(pSpeakerParty->GetPartyRsrcID());
	if (!pSpeakerPartyCntl)
		return statIllegal;

	mcuNum  = pSpeakerPartyCntl->GetMcuNum();
	termNum = pSpeakerPartyCntl->GetTermNum();

	return statOK;
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfConnectPartyCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyConnect(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfConnectPartyCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyConnect(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfConnectPartyCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyConnect(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfConnectPartyDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyConnect(CSegment* pParam)
{
	CContentBridgePartyInitParams partyInitParams;
	partyInitParams.DeSerialize(NATIVE, *pParam);

	// Avoid repeated connection of the party to the bridge
	PartyRsrcID partyId = partyInitParams.GetPartyRsrcID();
	PASSERTSTREAM_AND_RETURN(GetPartyCntl(partyId) != NULL, "PartyId:" << partyId << " - Failed, party already exist in Bridge");

	if (!partyInitParams.IsValidParams())
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Invalid Content party params";
		CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_CONTENT_DISCONNECTED, statInvalidPartyInitParams);
		return;
	}

	TRACEINTO
		<< "PartyId:" << partyId
		<< ", PartyContentProtocol:" << GetContentProtocolAsString(partyInitParams.GetByCurrentContentProtocol())
		<< ", ConfContentProtocol:" << GetContentProtocolAsString(m_byCurrentContentProtocol)
		<< ", PartyContentH264HighProfile:" << (int)(partyInitParams.GetByCurrentContentH264HighProfile())  //HP content
		<< ", ConfContentH264HighProfile:" << (int)m_byH264HighProfile;

	// When connect party to content bridge - content protocol must be equal to bridge protocol
	if ((partyInitParams.GetByCurrentContentProtocol() != m_byCurrentContentProtocol) || (partyInitParams.GetByCurrentContentProtocol() == H264 && partyInitParams.GetByCurrentContentH264HighProfile() != m_byH264HighProfile))  //HP content
	{
		if (IsXCodeConf())
		{
			TRACEINTO << "PartyId:" << partyId << " - Party Content Protocol Can be different from Conference Content Protocol in XCode Conference";
		}
		else
		{
			TRACEINTO << "PartyId:" << partyId << " - Party Content protocol is not equal to Conference Content Protocol, connection rejected";
			CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_CONTENT_DISCONNECTED, statInvalidPartyInitParams);
			return;
		}
	}

	CContentBridgePartyCntl* pContentBrdgPartyCntl = new CContentBridgePartyCntl();
	pContentBrdgPartyCntl->Create(&partyInitParams);
	// for slave link intra suppression
	if (partyInitParams.GetCascadeLinkMode() == CASCADE_MODE_MASTER)
		m_pCascadeSlavePartiesList->Insert((CBridgePartyCntl*)pContentBrdgPartyCntl);

	// Insert the party to the PartyCtl List and activate Connect on it
	CBridge::ConnectParty(pContentBrdgPartyCntl);
}

//--------------------------------------------------------------------------
void CContentBridge::OnEndPartyConnectCONNECT_INACTIVE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	DBGPASSERT_AND_RETURN(1);
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyConnectCONNECT(CSegment* pParam)
{
	PartyRsrcID PartyId;
	WORD status;
	*pParam >> PartyId >> status;

	TRACEINTO << "PartyId:" << PartyId << ", Status:" << status;

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	OnEndPartyConnect(pPartyCntl, status);

	CSegment* pUpdatedParam = new CSegment;
	*pUpdatedParam << PartyId << status;

	CBridge::EndPartyConnect(pUpdatedParam, PARTY_CONTENT_CONNECTED);

	POBJDELETE(pUpdatedParam);
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyConnectCHANGERATE(CSegment* pParam)
{
	PartyRsrcID PartyId;
	WORD status;
	*pParam >> PartyId >> status;

	TRACEINTO << "PartyId:" << PartyId << ", Status:" << status;

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "Party Name:" << pPartyCntl->GetName() << ", Status:" << status;
#else
	TRACEINTO << "Party Name:" << pPartyCntl->GetName() << ", Status:" << status;
#endif

	OnEndPartyConnect(pPartyCntl, status);

	CSegment* pUpdatedParam = new CSegment;
	*pUpdatedParam << PartyId << status;

	CBridge::EndPartyConnect(pUpdatedParam, PARTY_CONTENT_CONNECTED);

	POBJDELETE(pUpdatedParam);

	if (status == statOK)
	{
		// if party rate equal to bridge rate:
		// 1. Send ProviderIdentity msg to the new party
		// 2. Check if all parties have changed their rates
		BYTE mcuNum = 0;
		BYTE termNum = 0;
		WORD speakerStatus = GetSpeakerProviderIdentity(mcuNum, termNum);
		if (speakerStatus == statOK)
			pPartyCntl->ProviderIdentity(mcuNum, termNum, 0x02, 0, NULL);
		else
			DBGPASSERT(1);

		IsRateChangeDone();
	}
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyConnectCONTENT(CSegment* pParam)
{
	PartyRsrcID PartyId;
	WORD status;
	*pParam >> PartyId >> status;

	TRACEINTO << "PartyId:" << PartyId << ", Status:" << status;

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	OnEndPartyConnect(pPartyCntl, status);

	if (statOK != status) // send disconnecting partyID to token (in case it is the requester or speaker)
		m_pToken->PartyDisconnects(PartyId);

	CSegment* pUpdatedParam = new CSegment;
	*pUpdatedParam << PartyId << status;

	CBridge::EndPartyConnect(pUpdatedParam, PARTY_CONTENT_CONNECTED);

	POBJDELETE(pUpdatedParam);

	if (status == statOK)
	{
		BYTE mcuNum = 0;
		BYTE termNum = 0;
		WORD speakerStatus = GetSpeakerProviderIdentity(mcuNum, termNum);
		if (speakerStatus == statOK)
			pPartyCntl->ProviderIdentity(mcuNum, termNum, 0x02, 0, NULL);
		else
			DBGPASSERT(1);

		pPartyCntl->ChangePresentationRate();
	}
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyConnectDISCONNECTING(CSegment* pParam)
{
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyConnect(CContentBridgePartyCntl* pPartyCntl, WORD& status)
{
	const CConf* pConf = GetConf();
	if (!CPObject::IsValidPObjectPtr(pConf))
	{
		status = statInconsistent;
		DBGPASSERT_AND_RETURN(102);
	}

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(pConf->GetName());
	if (!CPObject::IsValidPObjectPtr(pCommConf))
	{
		status = statInconsistent;
		DBGPASSERT_AND_RETURN(103);
	}

	if (!CPObject::IsValidPObjectPtr(pPartyCntl))
	{
		status = statInconsistent;
		DBGPASSERT_AND_RETURN(104);
	}

	if (status == statOK)
	{
		std::ostringstream msg;
		msg
		<< "\n  PartyId               :" << pPartyCntl->GetPartyRsrcID()
		<< "\n  BridgeContentRate     :" << GetContentRateAsString(m_byCurrentContentRate)
		<< "\n  PartyContentRate      :" << GetContentRateAsString(pPartyCntl->GetPartyContentRate())
		<< "\n  BridgeContentProtocol :" << GetContentProtocolAsString(m_byCurrentContentProtocol)
		<< "\n  PartyContentProtocol  :" << GetContentProtocolAsString(pPartyCntl->GetPartyContentProtocol());

		// check current bridge rate - MUST be 0 in CONNECT state
		if ((CONNECT == m_state) && (m_byCurrentContentRate != AMC_0k))
		{
			TRACEINTO << "Failed, invalid bridge content rate" << msg.str().c_str();
			DBGPASSERT(1);
			status = statInconsistent;
			return;
		}

		// if party rate != bridge rate : ASSERT
		if (pPartyCntl->GetPartyContentRate() != m_byCurrentContentRate)
		{
			if (IsXCodeConf() && m_byCurrentContentRate > 0 && pPartyCntl->GetPartyContentRate() > 0)
			{
				TRACEINTO << "Party Content rate Can is not unique for all parties in XCode Conference" << msg.str().c_str();
			}
			else
			{
				TRACEINTO << "Failed, party content rate not equal to bridge content rate" << msg.str().c_str();
				DBGPASSERT(1); // for future - may be send here CHANGERATE to party
				status = statInconsistent;
				return;
			}
		}

		if (pPartyCntl->GetPartyContentProtocol() != m_byCurrentContentProtocol)
		{
			if (IsXCodeConf())
			{
				TRACEINTO << "Party Content Protocol Can be different from Conference Content Protocol in XCode Conference" << msg.str().c_str();
			}
			else
			{
				TRACEINTO << "Failed, party content protocol not equal to bridge content protocol" << msg.str().c_str();
				DBGPASSERT(1);
				status = statInconsistent;
				return;
			}
		}
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectConfCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfDisConnectConf(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectConfCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfDisConnectConf(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectConfCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfDisConnectConf(pParam);

	// Bridge-1127
	const CTaskApp* pLastTokenRequester = m_pToken->GetLastTokenRequesterParty();
	if (pLastTokenRequester != NULL)
		m_pToken->DropLastContentTokenRequester();

	AbortPresentation(); // stop presentation
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectConfDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored (Bridge is already disconnecting)";
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectConf(CSegment* pParam)
{
	m_state = DISCONNECTING; // IDLE;
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectPartyCONNECT_INACTIVE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	DBGPASSERT_AND_RETURN(1);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectPartyCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyDisConnect(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectPartyCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyDisConnect(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectPartyCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyDisConnect(pParam);
}

// --------------------------------------------------------------------------
void CContentBridge::OnConfDisConnectPartyDISCONNECTING(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID     partyId         = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << partyId << ", MediaDirection:" << eMediaDirection;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	PASSERT_AND_RETURN(!pParty);

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		// already disconnected
		m_pConfApi->PartyBridgeResponseMsg(pParty, CONTENTCNTL_MSG, PARTY_CONTENT_DISCONNECTED, statOK, 1);
		return;
	}

	if ((m_pCascadeSlavePartiesList->size() > 0) && (pPartyCntl->GetCascadeLinkMode() == CASCADE_MODE_MASTER))  // 'Remove' will return NULL if not found.
		m_pCascadeSlavePartiesList->Remove(partyId);

	CBridge::DisconnectParty(partyId);
}

// --------------------------------------------------------------------------
void CContentBridge::OnPartyDisConnect(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID     partyId         = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << partyId << ", MediaDirection:" << eMediaDirection;

	CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(partyId);
	PASSERT_AND_RETURN(!pParty);

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	if (IsTokenHolder(pParty))
		SendMessagesToRVGW(CONTENT_ROLE_TOKEN_RELEASE, m_pToken->GetTokenHolderMcuNumber(), m_pToken->GetTokenHolderTermNumber());

	m_pConfApi->UpdateDB(pParty, CONTENTPROVIDER, NO);

	if (IsTokenHolder(pParty))
		SendMessagesToLinks(CONTENT_ROLE_TOKEN_RELEASE, m_pToken->GetTokenHolderMcuNumber(), m_pToken->GetTokenHolderTermNumber());

	// send disconnecting partyID to token (for requester or speaker check - if speaker,HELD function will be called)
	m_pToken->PartyDisconnects(partyId);
	if ((m_pCascadeSlavePartiesList->size() > 0) && (pPartyCntl->GetCascadeLinkMode() == CASCADE_MODE_MASTER))  // 'Remove' will return NULL if not found.
		m_pCascadeSlavePartiesList->Remove(partyId);

	CBridge::DisconnectParty(partyId);
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyDisConnectCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnEndPartyDisConnect(pParam);
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyDisConnectCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnEndPartyDisConnect(pParam);
	IsRateChangeDone();
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyDisConnectCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnEndPartyDisConnect(pParam);
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyDisConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnEndPartyDisConnect(pParam);
}

// --------------------------------------------------------------------------
void CContentBridge::OnEndPartyDisConnect(CSegment* pParam)
{
	// Remove the party from link-to-slave list, if exists.
	PartyRsrcID PartyId;
	WORD status;
	CSegment seg(*pParam);
	seg >> PartyId >> status;

	TRACEINTO << "PartyId:" << PartyId << ", Status:" << status;

	CParty* pParty = GetLookupTableParty()->Get(PartyId);
	PASSERT_AND_RETURN(!pParty);

	CBridgePartyCntl* pPartyCntl = m_pPartyList->Find(PartyId);
	if (pPartyCntl && pPartyCntl->GetCascadeLinkMode() == CASCADE_MODE_MASTER)
		m_pCascadeSlavePartiesList->Remove(PartyId);

	CBridge::EndPartyDisConnect(pParam, PARTY_CONTENT_DISCONNECTED);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewRateCONNECT(CSegment* pParam)
{
	BYTE newContentRate = m_byCurrentContentRate;
	*pParam >> newContentRate;

	TRACEINTO << "ConfName:" << m_pConfName << ", ContentRate:" << GetContentRateAsString(newContentRate);

	// in CONNECT state rate have to be 0
	if (m_byCurrentContentRate != AMC_0k) {
		DBGPASSERT(2000+m_byCurrentContentRate);

	}

	if (newContentRate == m_byCurrentContentRate) {
		DBGPASSERT_AND_RETURN(1000+newContentRate);
	}

	if (!IsValidContentRate(newContentRate)) {
		DBGPASSERT_AND_RETURN(1000+newContentRate);
	}

	m_byCurrentContentRate = newContentRate;
	StartTimer(CHANGERATETOUT, CHANGERATE_TIME);
	m_state = CHANGERATE;

	if (m_bIsRcvArtContentOn)
	{
		SetSpeakerHWReadyToContent(YES);
		m_bIsRcvArtContentOn = FALSE;
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewRateCHANGERATE(CSegment* pParam)
{
	BYTE newContentRate = m_byCurrentContentRate;
	*pParam >> newContentRate;

	TRACEINTO << "ConfName:" << m_pConfName << ", ContentRate:" << GetContentRateAsString(newContentRate);

	if (newContentRate == m_byCurrentContentRate) {
		DBGPASSERT_AND_RETURN(1000+newContentRate);
	}

	if (!IsValidContentRate(newContentRate)) {
		DBGPASSERT_AND_RETURN(1000+newContentRate);
	}

	// delete old CHANGERATE timer
	DeleteTimer(CHANGERATETOUT);

	m_byCurrentContentRate = newContentRate;
	// restart CHANGERATE timer
	StartTimer(CHANGERATETOUT, CHANGERATE_TIME);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewRateCONTENT(CSegment* pParam)
{
	BYTE newContentRate = m_byCurrentContentRate;
	*pParam >> newContentRate;

	TRACEINTO << "ConfName:" << m_pConfName << ", ContentRate:" << GetContentRateAsString(newContentRate);

	if (newContentRate == m_byCurrentContentRate) {
		DBGPASSERT_AND_RETURN(1000+newContentRate);
	}

	if (!IsValidContentRate(newContentRate)) {
		DBGPASSERT_AND_RETURN(1000+newContentRate);
	}

	m_byCurrentContentRate = newContentRate;
	StartTimer(CHANGERATETOUT, CHANGERATE_TIME);
	m_state = CHANGERATE;
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewRateDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewScmCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfNewSCM(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewScmCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	DeleteTimer(CHANGERATETOUT);
	OnConfNewSCM(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewScmCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfNewSCM(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewSCM(CSegment* pParam)
{
	BYTE newContentProtocol = m_byCurrentContentProtocol;
	*pParam >> newContentProtocol;

	//HP content
	BYTE isContentH264HighProfile = FALSE;
	if (newContentProtocol == H264)
		*pParam >> isContentH264HighProfile;

	TRACEINTO << "ConfName:" << m_pConfName << ", ContentProtocol:" << GetContentProtocolAsString(newContentProtocol) <<", isContentH264HighProfile:" << (int)isContentH264HighProfile;

	if (m_byH264HighProfile != isContentH264HighProfile)
		m_byH264HighProfile = isContentH264HighProfile;

	m_byCurrentContentProtocol = newContentProtocol;
	// First withdraw the token in case of content open.
	// and set content rate to 0k.
	OnConfTokenWithdraw(pParam);
	// Send channel inactive to all parties:
	BYTE channelID = 0;
	BroadcastMediaProducerStatus(channelID, CHANNEL_INACTIVE);
	m_byCurrentContentRate = AMC_0k;

	// Remove from bridge all parties that their protocol is different from the current content protocol
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			m_pConfApi->UpdateDB(pPartyCntl->GetPartyTaskApp(), CONTENTPROVIDER, NO);
			if ((pPartyCntl->GetPartyContentProtocol() != m_byCurrentContentProtocol) || (m_byCurrentContentProtocol == H264 && (pPartyCntl->GetPartyContentH264HighProfile() != m_byH264HighProfile)))//HP content
			{
				// If party not already disconnecting or this is link party
				// We won't disconnect link party because
				if ((!pPartyCntl->IsDisConnecting()) && NONE == pPartyCntl->GetCascadeLinkMode())
				{
					PartyRsrcID partyId = pPartyCntl->GetPartyRsrcID();

					TRACEINTO << "PartyId:" << partyId << " - Party content protocol is different from the Conference content protocol, Disconnecting party";

					// send disconnecting partyID to token (for requester or speaker check)
					m_pToken->PartyDisconnects(partyId);
					if ((m_pCascadeSlavePartiesList->size() > 0) && (pPartyCntl->GetCascadeLinkMode() == CASCADE_MODE_MASTER))  // 'Remove' will return NULL if not found.
						m_pCascadeSlavePartiesList->Remove(partyId);

					// remove from bridge
					CBridge::DisconnectParty(partyId);
				}
			}
			// else need to update partycntl content rate....
		}
	}

	m_pConfApi->UpdateDB((CTaskApp*)0xffff, CONTENTSRC, 0);
	m_state = CONNECT;

	// send to CONF task event - stop Content Session
	m_pConfApi->StopContent();
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfNewScmDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenAcquireIDLE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenAcquireCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	CContentBridge::OnPartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenAcquireCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	CParty* pParty;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE label;
	BYTE randomNum;

	*pParam >> (void*&)pParty >> mcuNum >> terminalNum >> label >> randomNum;

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party is not connected to Content Bridge";
		m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		return;
	}

	// if wanted rate is AMC_0k, we don't allow new AcquireToken req
	if (m_byCurrentContentRate == AMC_0k)
	{
		TRACEINTO << "PartyId:" << partyId << " - Content Bridge in CHANGERATE to AMC_0k";

		// Canceled Nak by Matvey 27.02.2003.
		// Reason: When during Content closing we get Acquire. In this case we don't want to send Nak.
		// The reason is that when receiving Nak remote possibly will not make re-transmition
		// of Acquire (after 5sec timer according to spec). And we DO want it to re-transmit Acquire, in order
		// to not loose "Content open" activation from EP.
		// m_pToken->BadAcquire(pParty);

		if (CASCADE_MODE_MASTER == pPartyCntl->GetCascadeLinkMode()) // isRequestSentAlreadyToMaster = YES block the request ==> send BadAcquire to release the bridge
		{
			TRACEINTO << "PartyId:" << partyId << " - Content Bridge in CHANGERATE to AMC_0k! and Link to Slave trying to send Content send BadAcquire to slave to avoid blocking the slave of sending";
			m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		}
		return;
	}

	// merge an issue done for China Version: if we are in lecture mode with fix lecturer, only the lecturer can be the content token holder
	if (!IsConfLectureModeAllowTokenAcquire(pParty))
	{
		TRACEINTO << "PartyId:" << partyId << " - Conference lecture mode doesn't allow party acquire request";
		m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		return;
	}

	if (IsConfRestrictedContentToken(pParty))
	{
		TRACEINTO << "PartyId:" << partyId << " - Content is restricted to be able to block snatching on Cascade";
		m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		return;
	}

	// send message to token
	m_pToken->Acquire(partyId, mcuNum, terminalNum, randomNum);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenAcquireCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenAcquireDISCONNECTING(CSegment* pParam)
{
	CParty* pParty;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE label;
	BYTE randomNum;

	*pParam >> (void*&)pParty >> mcuNum >> terminalNum >> label >> randomNum;

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));
	m_pToken->BadAcquireBridgeDisconnecting(pParty->GetPartyRsrcID(), mcuNum, terminalNum);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenAcquire(CSegment* pParam)
{
	CParty* pParty;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE label;
	BYTE randomNum;

	*pParam >> (void*&)pParty >> mcuNum >> terminalNum >> label >> randomNum;

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();
	PartyRsrcID holderPartyId = m_pToken->GetTokenHolderPartyId();

	bool isSnatchContent = (holderPartyId != 0) && (holderPartyId != partyId);

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party is not connected to Content Bridge";
		m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		return;
	}

	if (!IsConfLectureModeAllowTokenAcquire(pParty))
	{
		TRACEINTO << "PartyId:" << partyId << " - Conference lecture mode doesn't allow party acquire request";
		m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		return;
	}

	if (IsConfRestrictedContentToken(pParty))
	{
		TRACEINTO << "PartyId:" << partyId << " - Content is restricted";
		m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		return;
	}
	if (isSnatchContent && pPartyCntl->GetCascadeLinkMode() != CASCADE_MODE_NONE)
	{
		TRACEINTO << "PartyId:" << partyId << " - is a cascade link  may not snatch content";
		m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		return;
	}
	CContentBridgePartyCntl* pSpeakerPartyCntl = (holderPartyId != (PartyRsrcID)0)? (CContentBridgePartyCntl*)GetPartyCntl(holderPartyId) : NULL;
	if (pSpeakerPartyCntl && (pSpeakerPartyCntl->GetCascadeLinkMode()!= CASCADE_MODE_NONE))
	{
		TRACEINTO << "Current token holder PartyId:" << partyId << " - is a cascade link: cannot snatch content";
		m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		return;
	}
	// send message to CContentBridge token (CContentToken) EVENT can arrive from slave bridge too
	m_pToken->Acquire(partyId, mcuNum, terminalNum, randomNum);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenReleaseCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenRelease(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenReleaseCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenRelease(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenReleaseCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	OnPartyTokenRelease(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenReleaseDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenRelease(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenRelease(CSegment* pParam)
{
	CParty* pParty;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE label;

	*pParam >> (void*&)pParty >> mcuNum >> terminalNum >> label;

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	// DBGPASSERT_AND_RETURN(!IsTokenHolder(pParty));//VNGR-7028 MCS send Release when changing Video speaker even he is NOT the content speaker
	if (m_pToken->GetTokenHolderPartyId() != partyId) // VNGR-7028 +VNGR-9271
	{
		TRACEINTO << "PartyId:" << partyId << " - Party received token release when it is not the speaker, abort the rest of release flow";
		return;
	}

	SendMessagesToRVGW(CONTENT_ROLE_TOKEN_RELEASE, mcuNum, terminalNum);

	// send message to token
	m_pToken->Release(partyId, mcuNum, terminalNum);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRateChangedCONNECT(CSegment* pParam)
{
	CParty* pParty;
	DWORD newPartyRate;
	BYTE parameterID;

	*pParam >> (void*&)pParty >> parameterID >> newPartyRate;

	PASSERT_AND_RETURN(!pParty);

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	BYTE newPartyRateAMC = CUnifiedComMode::TranslateRateToAMCRate(newPartyRate);

	TRACEINTO << "PartyId:" << partyId << ", ContentRate:" << GetContentRateAsString(newPartyRateAMC);

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	DBGPASSERT(pPartyCntl != NULL); // if party was found - we should not change the state to connect.
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRateChangedCHANGERATE(CSegment* pParam)
{
	CParty* pParty;
	DWORD newPartyRate;
	BYTE parameterID;

	*pParam >> (void*&)pParty >> parameterID >> newPartyRate;

	PASSERT_AND_RETURN(!pParty);

	BYTE newPartyRateAMC = CUnifiedComMode::TranslateRateToAMCRate(newPartyRate);

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyId << ", ContentRate:" << GetContentRateAsString(newPartyRateAMC);

	PASSERT_AND_RETURN(!IsValidContentRate(newPartyRateAMC));

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	pPartyCntl->SetRate(newPartyRateAMC);

	// check if all parties change the rate
	IsRateChangeDone();
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRateChangedCONTENT(CSegment* pParam)
{
	CParty* pParty;
	DWORD newPartyRate;
	BYTE parameterID;

	*pParam >> (void*&)pParty >> parameterID >> newPartyRate;

	PASSERT_AND_RETURN(!pParty);

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	BYTE newPartyRateAMC = CUnifiedComMode::TranslateRateToAMCRate(newPartyRate);

	TRACEINTO << "PartyId:" << partyId << ", ContentRate:" << GetContentRateAsString(newPartyRateAMC);

	DBGPASSERT_AND_RETURN(!IsValidContentRate(newPartyRateAMC));

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	DBGPASSERT_AND_RETURN(!pPartyCntl);

	if (newPartyRateAMC != pPartyCntl->GetPartyContentRate())
	{
		if (newPartyRateAMC == m_byCurrentContentRate)
		{
			pPartyCntl->SetRate(newPartyRateAMC);
			pPartyCntl->ChangePresentationRate();
		}
		else if (IsXCodeConf() && (newPartyRateAMC > 0 && m_byCurrentContentRate > 0))
		{
			pPartyCntl->SetRate(newPartyRateAMC);
			pPartyCntl->ChangePresentationRate();
		}
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRateChangedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenWithdrawAckCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenWithdrawAck(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenWithdrawAckCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenWithdrawAck(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenWithdrawAckCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenWithdrawAck(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenWithdrawAckDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenWithdrawAck(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyTokenWithdrawAck(CSegment* pParam)
{
	CTaskApp* pParty;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE label;

	*pParam >> (void*&)pParty >> mcuNum >> terminalNum >> label;

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));
	// send message to token
	m_pToken->WithdrawAck(pParty->GetPartyId());
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRefreshContentCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRefreshContentCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRefreshContentCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Remote EP asks for INTRA";

	CParty* pParty;
	BYTE controlId;
	BYTE isPartyAdded;

	*pParam >> controlId >> isPartyAdded;
	// intra requested from specific party

	CContentBridgePartyCntl* pPartyCntl = NULL;
	if (isPartyAdded)
	{
		*pParam >> (void*&)pParty;  // the party who requested the INTRA
		DBGPASSERT_AND_RETURN(!pParty);

		PartyRsrcID partyId = pParty->GetPartyRsrcID();

		pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
		PASSERTSTREAM_AND_RETURN(!pPartyCntl, "PartyId:" << partyId);

		BOOL isEPInSlaveRMX = AmISlaveBridge() && (pPartyCntl->GetCascadeLinkMode() == NONE);

		TRACEINTO << "PartyId:" << partyId << ", IsEPInSlaveRMX:" << (int)isEPInSlaveRMX << " - Party asks for INTRA";

		if (!IsXCodeConf() && (isEPInSlaveRMX || (m_pCascadeSlavePartiesList->Find(pPartyCntl->GetPartyRsrcID()) != NULL)))
		{
			if (m_bCascadeSlaveIntraRequestsPending || IsValidTimer(CONTENTSLAVELINKINTRATOUT))
			{
				m_bCascadeSlaveIntraRequestsPending = YES;
				TRACEINTO << "PartyId:" << partyId << " - Slave link asks for INTRA while suppressed, ignored";
				return;
			}
		}
		else
		{
			if(IsXCodeConf() && pPartyCntl->GetCascadeLinkMode() != NONE)
			{
				TRACEINTO << "PartyId:" << partyId << " - Do not invoke CContentBridgePartyCntl::CheckIsPartyIntraSuppressed for a link  in XCode Conference";
			}
			else
			{
				if (pPartyCntl->CheckIsPartyIntraSuppressed(TRUE))
				{
					TRACEINTO << "PartyId:" << partyId << " - Party INTRA suppressed due to MAX_INTRA_REQUESTS_PER_INTERVAL_CONTENT";
					return;
				}
			}

			if (IsXCodeConf())
			{
				m_pConfApi->ContentVideoRefresh(controlId, isPartyAdded, pParty, XCODE_BRDG_MSG);
				pPartyCntl->UpdateIntraRequestSupressionOnSend();
				TRACEINTO << "PartyId:" << partyId << " - Forward INTRA request also to XCode Bridge";
				return;
			}
			TRACEINTO << "PartyId:" << partyId << " - INTRA requested by not a slave link party";
		}
	}

	// reset and start
	if (!IsXCodeConf())
	{
		DWORD slaveLinkSuppresionIntervalInSeconds = 0;
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		sysConfig->GetDWORDDataByKey("CONTENT_SLAVE_LINKS_INTRA_SUPPRESSION_IN_SECONDS", slaveLinkSuppresionIntervalInSeconds);
		if (slaveLinkSuppresionIntervalInSeconds > 0)
			StartTimer(CONTENTSLAVELINKINTRATOUT, slaveLinkSuppresionIntervalInSeconds*SECOND);

		// specific party - update I-frame suppression from requesting side
		if (isPartyAdded && pPartyCntl)
			pPartyCntl->UpdateIntraRequestSupressionOnSend();

		TRACEINTO << "ConfName:" << m_pConfName << " - INTRA request sent (INTRA request timer started  as configured)";
	}

	// remote e.p asks for INTRA. send request to presentator party
	AskSpeakerForContentIntra();
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRefreshContentDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyHWContentOnOffAckCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyHWContentOnOffAck(pParam);
	// vngfe-7734,8034 ack for ART_CONTENT_ON_REQ received in connect state before changerate start, because timer added
	SetSpeakerHWReadyToContent(YES); //When ChangeRate Done intra will be send !!!
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyHWContentOnOffAckCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyHWContentOnOffAck(pParam);
	SetSpeakerHWReadyToContent(YES); // When ChangeRate Done intra will be send !!!
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyHWContentOnOffAckCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	OnPartyHWContentOnOffAck(pParam);
	// CONTENT state means CHange rate have already Done so we have to ask for intra here
	if (NO == IsSpeakerHWReadyToContent()) // if it is YES intra had already sent and this msg had already arrived!!!
	{
		SetSpeakerHWReadyToContent(YES);
		AskSpeakerForContentIntra();
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyHWContentOnOffAckDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyHWContentOnOffAck(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyHWContentOnOffAck(CSegment* pParam)
{
	CParty* pParty;
	DWORD contentState;

	*pParam >> (void*&)pParty >> contentState;

	eContentState ePState = (eContentState)contentState;

	if (eStreamOn == ePState)
	{
		m_bIsRcvArtContentOn = TRUE;
		if (m_pToken->GetTokenHolderPartyId()) {
			PASSERT_AND_RETURN(YES != IsTokenHolder(pParty));
		}
	}
	else
		m_bIsRcvArtContentOn = FALSE;

	PASSERT(eStreamOff == ePState);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfTokenWithdrawCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfTokenWithdraw(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfTokenWithdrawCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfTokenWithdraw(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfTokenWithdrawCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfTokenWithdraw(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfTokenWithdrawDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfTokenWithdraw(CSegment* pParam)
{
	// means: withdraw active tokenHolder
	// send Withdraw to token
	PartyRsrcID partyId = m_pToken->GetTokenHolderPartyId();

	TRACEINTO << "ConfName:" << m_pConfName << ", TokenHolderPartyId:" << partyId;

	if (partyId)
	{
		SendMessagesToLinks(CONTENT_ROLE_TOKEN_RELEASE, m_pToken->GetTokenHolderMcuNumber(), m_pToken->GetTokenHolderTermNumber(), NO);
		SendMessagesToRVGW(CONTENT_ROLE_TOKEN_RELEASE, m_pToken->GetTokenHolderMcuNumber(), m_pToken->GetTokenHolderTermNumber(), NO);
		BYTE isImmediate = NO;
		if (!pParam->EndOfSegment())
			*pParam >> isImmediate;

		if (isImmediate)
			m_pToken->DropTokenHolder(partyId, isImmediate);
		else
			m_pToken->Withdraw(partyId);
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRoleProviderIdentityCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyRoleProviderIdentity(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRoleProviderIdentityCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyRoleProviderIdentity(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRoleProviderIdentityCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyRoleProviderIdentity(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRoleProviderIdentityDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyRoleProviderIdentity(CSegment* pParam)
{
	const CTaskApp* pSpeakerParty = m_pToken->GetTokenHolderParty();
	if (!pSpeakerParty)
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - No Token holder, ignored";
		return;
	}

	CTaskApp* pParty;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE label;
	BYTE dataSize;

	*pParam >> (void*&)pParty >> mcuNum >> terminalNum >> label >> dataSize;

	// only speaker party can send NS-IND/RoleProviderIdentity
	if (pParty != pSpeakerParty)
		return;

	BYTE* pData = NULL;
	if (dataSize > 0)
	{
		pData = new BYTE[dataSize];
		for (int i = 0; i < dataSize; i++)
			*pParam >> pData[i];
	}

	BroadcastRoleProviderIdentityMsg(mcuNum, terminalNum, label, dataSize, pData);

	PDELETEA(pData);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyMediaProducerStatusCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyMediaProducerStatus(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyMediaProducerStatusCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyMediaProducerStatus(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyMediaProducerStatusCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyMediaProducerStatus(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyMediaProducerStatusDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyBfcpTokenQueryIDLE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyBfcpTokenQuery(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyBfcpTokenQueryCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyBfcpTokenQuery(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyBfcpTokenQueryCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyBfcpTokenQuery(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyBfcpTokenQueryCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyBfcpTokenQuery(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyBfcpTokenQueryDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
// OnPartyBfcpTokenQuery(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyBfcpTokenQuery(CSegment* pParam)
{
	CParty* pParty;
	BYTE mcuNum;
	BYTE terminalNum;
	BYTE label;
	BYTE dataSize;

	*pParam >> (void*&)pParty >> mcuNum >> terminalNum >> label >> dataSize;

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	TRACEINTO << "PartyId:" << pParty->GetPartyRsrcID() << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	CPartyApi* pPartyApi = new CPartyApi;

	if (m_pToken->GetTokenHolderPartyId() == pParty->GetPartyRsrcID())
	{
		BYTE* pData = NULL;
		if (dataSize > 0)
		{
			pData = new BYTE[dataSize];

			for (int i = 0; i < dataSize; i++)
				*pParam >> pData[i];
		}

		pPartyApi->SendContentTokenRoleProviderIdentity(mcuNum, terminalNum, label, dataSize, pData);

		if (pData)
			PDELETEA(pData);
	}
	else
		pPartyApi->SendContentTokenNoRoleProvider(mcuNum, terminalNum);

	POBJDELETE(pPartyApi);
}

//--------------------------------------------------------------------------
void CContentBridge::OnPartyMediaProducerStatus(CSegment* pParam)
{
	const CTaskApp* pSpeakerParty = m_pToken->GetTokenHolderParty();
	if (!pSpeakerParty)
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - No Token holder, ignored";
		return;
	}

	CTaskApp* pParty;
	BYTE channelID;
	BYTE status;

	*pParam >> (void*&)pParty >> channelID >> status;

	EMediaProducerStatus enum_status = (EMediaProducerStatus)status;

	// only speaker party can send MediaProducerStatus
	if (pParty != pSpeakerParty)
		return;

	BroadcastMediaProducerStatus(channelID, enum_status);
}

//--------------------------------------------------------------------------
void CContentBridge::BroadcastMediaProducerStatus(BYTE channelID, EMediaProducerStatus status)
{
#ifdef CONTENT_LOAD
	CStopper broadcastMediaProducerStatusStopper("CContentBridge::BroadcastMediaProducerStatus", m_pConfName);
#endif
	// SEND MediaProducerStatus to all parties
	std::ostringstream msg;
	msg << "{";

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		pPartyCntl->MediaProducerStatus(channelID, status);
		msg << pPartyCntl->GetPartyRsrcID() << ",";
	}
	msg << "}";
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << msg.str().c_str();
	broadcastMediaProducerStatusStopper.Stop();
#else
	TRACEINTO << msg.str().c_str();
#endif
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNewTokenHolderCONNECT(CSegment* pParam)
{
	SetSpeakerHWReadyToContent(NO);
	// start new presentation
	// check if a speaker connected to Content Bridge
	PartyRsrcID partyId = m_pToken->GetTokenHolderPartyId();
	DBGPASSERT_AND_RETURN(!partyId);

	BYTE mcuNum    = 0;
	BYTE termNum   = 0;
	BYTE channelID = 0;
	*pParam >> mcuNum >> termNum;

	TRACEINTO << "PartyId:" << partyId << ", mcuNum:" << (int)mcuNum << ", terminalNum:" << (int)termNum;

	// check if party is a bridge member
	CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!CPObject::IsValidPObjectPtr(pSpeakerPartyCntl))
	{
		// send Withdraw to token
		m_pToken->Withdraw(partyId);
		BroadcastMediaProducerStatus(channelID, CHANNEL_INACTIVE); // InactiveChannel to all bridge parties ?????is this OK??
		return;
	}
#ifdef 	CONTENT_LOAD
	WORD START_CONTENT_LOAD = 1;
	TRACESTRFUNC(eLevelError) << " CONTENT_LOAD start content " << "New token holder:" << pSpeakerPartyCntl->GetName() << ", PartyId:" << pSpeakerPartyCntl->GetPartyRsrcID() << ", mcuNum:" << (int)mcuNum << ", terminalNum:" << (int)termNum;

#if (0)
	CSegment* msg = new CSegment;
	*msg << (DWORD)(eLevelError);
	CTaskApi::SendMsgWithTrace(eProcessLogger,
							   eManager,
							   msg,
							   LOGGER_SET_LEVEL_EXPLICIT);
#endif

#endif

	BroadcastRoleProviderIdentityForNewTokenHolder(mcuNum, termNum, 0x2, 0, NULL);

#ifdef 	CONTENT_LOAD
	if(START_CONTENT_LOAD){
		StartTimer(START_CONTENT_LOAD_TIMER_01,10);
		return;
	}
#endif

	// Send ActiveChannel to ALL bridge parties
	BroadcastMediaProducerStatus(channelID, CHANNEL_ACTIVE);

	// send to CONF task event - start Content Session
	m_pConfApi->StartContent(pSpeakerPartyCntl->GetPartyContentProtocol());
	IgnoreContentIntraFilteringAfterStartContent();
}
//--------------------------------------------------------------------------
void CContentBridge::OnTokenNewTokenHolderContinue01(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << m_pConfName;
	BroadcastMediaProducerStatus(0, CHANNEL_ACTIVE);
	StartTimer(START_CONTENT_LOAD_TIMER_02,20);
}
//--------------------------------------------------------------------------
void CContentBridge::OnTokenNewTokenHolderContinue02(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << m_pConfName;
	PartyRsrcID partyId = m_pToken->GetTokenHolderPartyId();
	DBGPASSERT_AND_RETURN(!partyId);
	CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!CPObject::IsValidPObjectPtr(pSpeakerPartyCntl))
	{
		// send Withdraw to token
		m_pToken->Withdraw(partyId);
		BroadcastMediaProducerStatus(0, CHANNEL_INACTIVE); // InactiveChannel to all bridge parties ?????is this OK??
		return;
	}

	// send to CONF task event - start Content Session
	m_pConfApi->StartContent(pSpeakerPartyCntl->GetPartyContentProtocol());
	StartTimer(START_CONTENT_LOAD_TIMER_03,20);
}
//--------------------------------------------------------------------------
void CContentBridge::OnTokenNewTokenHolderContinue03(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << m_pConfName;
	IgnoreContentIntraFilteringAfterStartContent();
#if (0)
	CSegment* msg = new CSegment;
	*msg << (DWORD)(eLevelInfoNormal);
	CTaskApi::SendMsgWithTrace(eProcessLogger,
							   eManager,
							   msg,
							   LOGGER_SET_LEVEL_EXPLICIT);
#endif

}
/////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
void CContentBridge::OnTokenNewTokenHolderCHANGERATE(CSegment* pParam)
{
	BYTE mcuNum  = 0;
	BYTE termNum = 0;
	*pParam >> mcuNum >> termNum;

	TRACEINTO << "ConfName:" << m_pConfName << ", mcuNum:" << (int)mcuNum << ", terminalNum:" << (int)termNum;

	SetSpeakerHWReadyToContent(NO);

	BroadcastRoleProviderIdentityForNewTokenHolder(mcuNum, termNum, 0x2, 0, NULL);

	// Romem - Content Snatching
	if (IsXCodeConf())
	{
		// new presentation speaker
		const CParty* pSpeakerParty = (CParty*)m_pToken->GetTokenHolderParty();
		if (!pSpeakerParty)
		{
			TRACEINTO << "ConfName:" << m_pConfName << " - No Token holder, ignored";
			AbortPresentation();
			return;
		}

		PartyRsrcID speakerId = pSpeakerParty->GetPartyRsrcID();

		// check if party is a bridge member
		CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(speakerId);
		if (!pSpeakerPartyCntl)
		{
			// send Withdraw to token
			m_pToken->Withdraw(speakerId);
			return;
		}

		// Fake Start Content in case of Content Snatching in XCode Conf in order to update content decoder (H263/H264)
		TRACEINTO << "ConfName:" << m_pConfName << " - Fake Start Content in case of Content snatching in XCode conference";
		BYTE isContentSnatching = YES;
		m_pConfApi->StartContent(pSpeakerPartyCntl->GetPartyContentProtocol(), isContentSnatching);
	}

	// nothing to do, just wait till changeRate end
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNewTokenHolderCONTENT(CSegment* pParam)
{
	BYTE mcuNum  = 0;
	BYTE termNum = 0;
	*pParam >> mcuNum >> termNum;

	TRACEINTO << "ConfName:" << m_pConfName << ", mcuNum:" << (int)mcuNum << ", terminalNum:" << (int)termNum;

	SetSpeakerHWReadyToContent(NO);
	// new presentation speaker
	const CParty* pSpeakerParty = (CParty*)m_pToken->GetTokenHolderParty();
	if (!pSpeakerParty)
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - No Token holder, ignored";
		AbortPresentation();
		return;
	}

	PartyRsrcID speakerId = pSpeakerParty->GetPartyRsrcID();

	// check if party is a bridge member
	CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(speakerId);
	if (!pSpeakerPartyCntl)
	{
		// send Withdraw to token
		m_pToken->Withdraw(speakerId);
		return;
	}

	TRACEINTO << "SpeakerId:" << speakerId << " - This is a new token holder";

	BroadcastRoleProviderIdentityForNewTokenHolder(mcuNum, termNum, 0x2, 0, NULL);
	BYTE channelID = 0;
	BroadcastMediaProducerStatus(channelID, CHANNEL_ACTIVE);
	ChangePresentationSpeaker();
	// Romem - Content Snatching
	if (IsXCodeConf())
	{
		// Fake Start Content in case of Content Snatching in XCode conference in order to update content decoder (H263/H264)
		TRACEINTO << "ConfName:" << m_pConfName << " - Fake Content Snatching in XCode conference";
		BYTE isContentSnatching = YES;
		m_pConfApi->StartContent(pSpeakerPartyCntl->GetPartyContentProtocol(), isContentSnatching);
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNewTokenHolderDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	PASSERTMSG(1, "Illegal state to receive NEWTOKENHOLDER since ACQUIRE did`t sent to the Content Token in disconnecting state");
}

//--------------------------------------------------------------------------
void CContentBridge::BroadcastRoleProviderIdentityForNewTokenHolder(BYTE mcuNum, BYTE terminalNum, BYTE label, BYTE dataSize, BYTE* pData)
{
	BroadcastRoleProviderIdentityMsg(mcuNum, terminalNum, label, dataSize, pData);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoTokenHolderCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	ResetLastContentRateFromMaster();
	SetSpeakerHWReadyToContent(NO);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoTokenHolderCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	ResetLastContentRateFromMaster();
	//BRIDGE-11899
	DeleteTimer(CHANGERATETOUT);
	m_state = CONTENT;
	OnTokenNoTokenHolderEnd(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoTokenHolderCONTENT(CSegment* pParam)
{
	TRACEINTO;
	OnTokenNoTokenHolderEnd(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoTokenHolderEnd(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError)<< "ConfName:" << m_pConfName;
#else
	TRACEINTO << "ConfName:" << m_pConfName;
#endif
	SetSpeakerHWReadyToContent(NO);
	// Stop current presentation
	// change rate to 0
	const CTaskApp* pSpeakerParty = m_pToken->GetTokenHolderParty();
	if (pSpeakerParty != NIL(CTaskApp))
	{
		DBGPASSERT(1);
	}

	BYTE channelID = 0;
	// BYTE mcuNum = pSpeakerParty->GetMcuNum();
// BYTE terminalNum = pSpeakerParty->GetTermNum();
	// send inactive channel to all bridge parties
	BroadcastMediaProducerStatus(channelID, CHANNEL_INACTIVE);
	// send to CONF task event - stop Content Session
	ResetLastContentRateFromMaster();
	SendMessagesToRVGW(CONTENT_ROLE_TOKEN_RELEASE, m_pToken->GetTokenHolderMcuNumber(), m_pToken->GetTokenHolderTermNumber());
	SendMessagesToLinks(CONTENT_ROLE_TOKEN_RELEASE, m_pToken->GetTokenHolderMcuNumber(), m_pToken->GetTokenHolderTermNumber());
	m_pConfApi->StopContent();
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoTokenHolderDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenFreezContentCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenFreezContentCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenFreezContentCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	// send to all parties VCF to content stream
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->SendFreeze();
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenFreezContentDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerChangeRateCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
	DBGPASSERT(1);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerChangeRateCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	ChangePresentationRate();
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerChangeRateCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
	DBGPASSERT(1);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerChangeRateDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
	DBGPASSERT(1);
}

//--------------------------------------------------------------------------
void CContentBridge::BroadcastRoleProviderIdentityMsg(BYTE mcuNum, BYTE terminalNum, BYTE label, BYTE dataSize, BYTE* pData)
{
	// check if speaker connected to Content Bridge
	const CParty* pSpeakerParty = (CParty*)m_pToken->GetTokenHolderParty();
	DBGPASSERT_AND_RETURN(!pSpeakerParty);

	// check if party is a bridge member
	CContentBridgePartyCntl* pSpeakerPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(pSpeakerParty->GetPartyRsrcID());
	DBGPASSERT_AND_RETURN(!pSpeakerPartyCntl);

	// forward NS-IND to all parties except speaker
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		// If speaker and it is CASCADE_MODE_NONE (not link) - don't send ProviderIdentity
		CParty* pParty = (CParty*)pPartyCntl->GetPartyTaskApp();
		if (pPartyCntl != pSpeakerPartyCntl || pParty->IsRemoteIsSlaveMGCWithContent() || pParty->IsRemoteIsRVpresentation())
			pPartyCntl->ProviderIdentity(mcuNum, terminalNum, label, dataSize, pData);
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfTerminate(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	PASSERTMSG(1, "Illegal state to receive TERMINATE, the Content Bridge should be in disconnecting state");
}

// ------------------------------------------------------------
void CContentBridge::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	EStat eStatus = statOK;

	m_state = IDLE;

	if (0 != GetNumParties())
	{
		// Upon receiving TERMINATE event, Bridge should be empty
		eStatus = statBridgeIsNotEmpty;
	}

	m_pConfApi->EndContentBrdgDisConnect(eStatus);
}

//--------------------------------------------------------------------------
void CContentBridge::CascadePartyTokenAcquire(CSegment* pParam)
{
	CParty* pParty;
	BYTE mcuNum;
	BYTE terminalNum;

	*pParam >> (void*&)pParty >> mcuNum >> terminalNum;

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNum << ", TerminalNum:" << (int)terminalNum;

	// Cascade MIH - block the option to switch ,content acquire is available only if the token is free.
	if (m_pToken->GetTokenHolderPartyId())
	{
		if (m_pToken->GetTokenHolderPartyId() != partyId)
		{
			TRACEINTO << "PartyId:" << partyId << " - In Cascade conference snatch is blocked";
			m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		}
	}
	else
	{
		if (m_byCurrentContentRate == AMC_0k)  // (CHANGERATE == m_state))in CONTENT state we don`t want to send BadAcquire to the holder we want to do it for changeRate to 0 since if not doing so isRequestSentAlreadyToMaster = YES block the request
		{
			TRACEINTO << "PartyId:" << partyId << " - Content Bridge in CHANGERATE to AMC_0k! In Cascade SLAVE(2ndLevel)send BadAcquire to slave(level3) to avoid blocking the slave of sending";
			m_pToken->BadAcquire(partyId, mcuNum, terminalNum);
		}
	}

	return;
}
//--------------------------------------------------------------------------
void CContentBridge::UpdateConfLectureMode()
{
	DispatchEvent(UPDATEONLECTUREMODE, NULL);
}

//--------------------------------------------------------------------------
BYTE CContentBridge::IsConfLectureModeAllowTokenAcquire(CTaskApp* pTask)
{
	// merge an issue done for China Unicom Version: if we are in lecture mode with fix lecturer, only the lecturer can be the content token holder

	const CParty* pParty = (CParty*)pTask;

	BYTE isConfLMAllowTokenAcquire = YES;
	if (!CPObject::IsValidPObjectPtr(pParty))
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Party is not valid";
		return isConfLMAllowTokenAcquire;
	}

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party is not connected to Content Bridge";
		return isConfLMAllowTokenAcquire;
	}

	BOOL bRestrictContentBroadcastToLecturer = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("RESTRICT_CONTENT_BROADCAST_TO_LECTURER", bRestrictContentBroadcastToLecturer);
	if (!bRestrictContentBroadcastToLecturer)
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Content Token is not restricted to Lecturer in system.cfg";
		return isConfLMAllowTokenAcquire;
	}

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetConf()->GetName());

	CLectureModeParams* pLectureModeParams = pCommConf ? pCommConf->GetLectureMode() : NULL;
	if (CPObject::IsValidPObjectPtr(pLectureModeParams))
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Conference with lecture mode";

		BYTE lectureModeOnOff = pLectureModeParams->GetLectureModeType();
		BYTE isLectureModeAudioActivated = pLectureModeParams->GetAudioActivated();
		if (lectureModeOnOff == YES && !isLectureModeAudioActivated)
		{
			TRACEINTO << "ConfName:" << m_pConfName << " - Lecture mode with fix lecturer";
			const char* lecturerName = pLectureModeParams->GetLecturerName();
			const char* partyName    = pPartyCntl->GetName();
			if ((strlen(lecturerName) > 0) && (strlen(partyName) > 0) && (strncmp(partyName, lecturerName, H243_NAME_LEN))) // the new party that wants to acquire the content isn't the lecturer
			{
				TRACEINTO << "LecturerName:" << lecturerName << " (just lecturer can send content), PartyName:" << partyName << " (party wanted to acquire the token)";
				isConfLMAllowTokenAcquire = NO;
			}
		}
	}
	else
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - pLectureMode isn't valid";
	}

	return isConfLMAllowTokenAcquire;
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateLectureModeConnect(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored, the lecture mode is not relevant when there is no content";
	BYTE yesNO;
	*pParam >> yesNO;
	SetExclusiveContentMode(yesNO);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateLectureModeChangeRate(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	BYTE yesNO;
	*pParam >> yesNO;
	OnConfUpdateLectureMode(yesNO);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateLectureModeContent(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	BYTE yesNO;
	*pParam >> yesNO;
	OnConfUpdateLectureMode(yesNO);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateLectureMode(BYTE yesNo)
{
	// merge an issue done for China Unicom version
	// In case the lecture mode changed to lecture mode with fix lecturer,
	// if the current content holder isn't the lecturer we will withdraw it's content
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetConf()->GetName());
	PASSERT_AND_RETURN(!pCommConf);

	CLectureModeParams* pLectureModeParams = pCommConf->GetLectureMode();
	if (CPObject::IsValidPObjectPtr(pLectureModeParams))
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Conference with lecture mode";
		BYTE lectureModeOnOff = pLectureModeParams->GetLectureModeType();
		BYTE isLectureModeAudioActivated = pLectureModeParams->GetAudioActivated();
		if (lectureModeOnOff == 1 && !isLectureModeAudioActivated)
		{
			TRACEINTO << "ConfName:" << m_pConfName << " - Lecture mode is ON with fix lecturer";

			const char* lecturerName = pLectureModeParams->GetLecturerName();
			const CTaskApp* pParty = m_pToken->GetTokenHolderParty();
			if (pParty)
			{
				const char* contentOwnerPartyName = ((CParty*)pParty)->GetName();
				// If the content owner isn't the new lecturer in lecture mode we will withdraw the content
				if (lecturerName != NULL && strcmp(lecturerName, ""))
				{
					if (!IsConfLectureModeAllowTokenAcquire((CParty*)pParty) &&
					    contentOwnerPartyName != NULL && strncmp(contentOwnerPartyName, lecturerName, H243_NAME_LEN))
					{
						// In case the conference is set with lecture mode with fix lecturer, only the lecturer can send content, we will withdraw the content otherwise
						TRACEINTO << "LecturerName:" << lecturerName << ", TokenOwnerName:" << contentOwnerPartyName;
						ContentTokenWithdraw();
					}
				}
			}
			else
			{
				TRACEINTO << "ConfName:" << m_pConfName << " - pParty isn't valid";
			}
		}
	}
	else
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - pLectureMode isn't valid";
	}

	SetExclusiveContentMode(yesNo);
}

//--------------------------------------------------------------------------
void CContentBridge::ContentTokenWithdraw(BYTE isImmediate)
{
	CSegment* pSeg = new CSegment;
	*pSeg << isImmediate;
	DispatchEvent(TOKEN_WITHDRAW, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoProviderCONNECT(CSegment* pParam)
{
	// broadcast NS-IND/NoRoleProvider to all parties
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->NoRoleProvider();
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoProviderCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	DBGPASSERT(1);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoProviderCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	DBGPASSERT(1);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTokenNoProviderDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridge::ResetLastContentRateFromMaster()
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
BYTE CContentBridge::IsConfRestrictedContentToken(CTaskApp* pTask) // Restricted content
{
	const CParty* pParty = (CParty*)pTask;

	BYTE isConfRestrictedContentToken = NO;
	if ((YES == IsExclusiveContentMode()) && ((CHANGERATE == m_state) || (CONTENT == m_state)))
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - EXCLUSIVE_CONTENT_MODE is set to ON, avoid Content snatch (for ICBC usages)";
		return YES;
	}

	if (!CPObject::IsValidPObjectPtr(pParty))
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - pParty is not valid";
		return isConfRestrictedContentToken;
	}

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party is not connected to Content Bridge";
		return isConfRestrictedContentToken;
	}

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetConf()->GetName());

	char* partyName = pPartyCntl->GetName();
	const char* exclusiveContentOwnerName = pCommConf ? pCommConf->GetExclusiveContent() : NULL;
	if ((exclusiveContentOwnerName != NULL) && (partyName != NULL) && (strncmp(partyName, exclusiveContentOwnerName, H243_NAME_LEN)))
	{
		TRACEINTO << "ConfName:" << m_pConfName << ", ExclusiveContentOwnerName:" << exclusiveContentOwnerName << ", PartyName:" << partyName << " (asking for content owner)";
		isConfRestrictedContentToken = YES;
	}

	return isConfRestrictedContentToken;
}

//--------------------------------------------------------------------------
void CContentBridge::UpdateExclusiveContent() // Restricted content
{
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)TRUE;
	DispatchEvent(UPDATEEXCLUSIVECONTENT, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateExclusiveContentConnect(CSegment* pParam) // Restricted content
{
	TRACEINTO << "ConfName:" << m_pConfName << " - The lecture mode is not relevant when there is no content";
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateExclusiveContentChangeRate(CSegment* pParam) // Restricted content
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfUpdateExclusiveContent();
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateExclusiveContentContent(CSegment* pParam) // Restricted content
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfUpdateExclusiveContent();
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateExclusiveContent() // Restricted content
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetConf()->GetName());
	if (!CPObject::IsValidPObjectPtr(pCommConf))
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, CommConf isn't valid";
		return;
	}

	const CParty* pParty = (CParty*)m_pToken->GetTokenHolderParty();
	if (pParty)
	{
		PartyRsrcID partyId = pParty->GetPartyRsrcID();

		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
		if (pPartyCntl)
		{
			const char* exclusiveContentOwnerName = pCommConf->GetExclusiveContent();
			const char* contentOwnerPartyName = pParty->GetName();

			if (exclusiveContentOwnerName != NULL && strcmp(exclusiveContentOwnerName, ""))
			{
				if (contentOwnerPartyName != NULL && strncmp(contentOwnerPartyName, exclusiveContentOwnerName, H243_NAME_LEN))
				{
					TRACEINTO << "ConfName:" << m_pConfName << ", ExclusiveContentOwnerName:" << exclusiveContentOwnerName << ", ContentOwnerPartyName:" << contentOwnerPartyName;
					ContentTokenWithdraw();
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
void CContentBridge::UpdateExclusiveContentMode(BOOL yesNo) // Restricted content
{
	// the update is handled as 'event' in case we may want to differentiate actions by connection status.
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)yesNo;
	TRACEINTO << "ConfName:" << m_pConfName << ", State:" << GetState();
	DispatchEvent(UPDATEEXCLUSIVECONTENTMODE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateExclusiveContentModeConnect(CSegment* pParam) // Restricted content
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfUpdateExclusiveContentMode(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateExclusiveContentModeChangeRate(CSegment* pParam) // Restricted content
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfUpdateExclusiveContentMode(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateExclusiveContentModeContent(CSegment* pParam) // Restricted content
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnConfUpdateExclusiveContentMode(pParam);
}

//--------------------------------------------------------------------------
void CContentBridge::OnConfUpdateExclusiveContentMode(CSegment* pParam) // Restricted content
{
	BYTE isExclusiveContentMode = NO;
	*pParam >> isExclusiveContentMode;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetConf()->GetName());
	if (!CPObject::IsValidPObjectPtr(pCommConf))
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, CommConf isn't valid";
		return;
	}

	TRACEINTO << "ConfName:" << m_pConfName << ", IsExclusiveContentMode:" << (int)isExclusiveContentMode;
	SetExclusiveContentMode(isExclusiveContentMode);
}

//--------------------------------------------------------------------------
void CContentBridge::IgnoreContentIntraFilteringAfterStartContent()
{
	TRACEINTO << " StartTimer " << DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT/SECOND << " seconds";
	DisableIntraSuppress();
	StartTimer(DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER, DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_STOP_CONTENT_TOUT);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerDisableIntraAfterStartContent(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	if (!IsXCodeConf())
		EnableIntraSuppress();
}

//--------------------------------------------------------------------------
void CContentBridge::EnableIntraSuppress()
{
	m_isIntraSupressionEnabled = true;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->EnableIntraSuppress(SUPPRESS_TYPE_ALL);
	}
}

//--------------------------------------------------------------------------
void CContentBridge::DisableIntraSuppress()
{
	m_isIntraSupressionEnabled = false;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
			pPartyCntl->DisableIntraSuppress(SUPPRESS_TYPE_ALL);
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressedChangeRate(CSegment* pParams)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnTimerCascadeSlaveLinkIntraSuppressedContent(pParams);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressedContent(CSegment* pParams)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnTimerCascadeSlaveLinkIntraSuppressed(pParams);
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressed(CSegment* pParams)
{
	m_bCascadeSlaveIntraRequestsPending = NO;
	if (m_bCascadeSlaveIntraRequestsPending)
	{
		AskSpeakerForContentIntra();

		DWORD slaveLinkSuppresionIntervalInSeconds = 0;
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("CONTENT_SLAVE_LINKS_INTRA_SUPPRESSION_IN_SECONDS", slaveLinkSuppresionIntervalInSeconds);
		StartTimer(CONTENTSLAVELINKINTRATOUT, slaveLinkSuppresionIntervalInSeconds*SECOND);
		TRACEINTO << "ConfName:" << m_pConfName << " - Slave link INTRA requested before, slave link INTRA request timer restarted as configured";
	}
	else
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - New slave link INTRA not requested, slave link INTRA request timer not restarted";
	}
}

//--------------------------------------------------------------------------
void CContentBridge::OnTimerCascadeSlaveLinkIntraSuppressedNoContent(CSegment* pParams)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	m_bCascadeSlaveIntraRequestsPending = NO;
}

//--------------------------------------------------------------------------
void CContentBridge::AllocateLinkPartyList()
{
	if (m_pCascadeSlavePartiesList)
		POBJDELETE(m_pCascadeSlavePartiesList);

	m_pCascadeSlavePartiesList = new CBridgePartyList();
}

//--------------------------------------------------------------------------
BYTE CContentBridge::IsXCodeConf()
{
	// Romem's validity
	const CConf* pConf = GetConf();
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pConf), FALSE);

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(pConf->GetName());
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pCommConf), FALSE);

	return pCommConf->GetContentMultiResolutionEnabled();
}



PBEGIN_MESSAGE_MAP(CContentBridgeSlave)

	// Master inform the Slave to Start content.
	ONEVENT(MASTER_START_CONTENT,        IDLE,             CContentBridgeSlave::NullActionFunction)
	ONEVENT(MASTER_START_CONTENT,        CONNECT_INACTIVE, CContentBridgeSlave::OnMasterStartContentCONNECT_INACTIVE)
	ONEVENT(MASTER_START_CONTENT,        CONNECT,          CContentBridgeSlave::OnMasterStartContentCONNECT)
	ONEVENT(MASTER_START_CONTENT,        CHANGERATE,       CContentBridgeSlave::OnMasterStartContentCHANGERATE)
	ONEVENT(MASTER_START_CONTENT,        CONTENT,          CContentBridgeSlave::OnMasterStartContentCONTENT)
	ONEVENT(MASTER_START_CONTENT,        DISCONNECTING,    CContentBridgeSlave::OnMasterStartContentDISCONNECTING)
	// Master inform the Slave On RateChange (Open>0 or Close =0)
	ONEVENT(MASTER_RATE_CHANGE,          IDLE,             CContentBridgeSlave::NullActionFunction)
	ONEVENT(MASTER_RATE_CHANGE,          CONNECT_INACTIVE, CContentBridgeSlave::OnMasterRateChangeCONNECT_INACTIVE)
	ONEVENT(MASTER_RATE_CHANGE,          CONNECT,          CContentBridgeSlave::OnMasterRateChangeCONNECT)
	ONEVENT(MASTER_RATE_CHANGE,          CHANGERATE,       CContentBridgeSlave::OnMasterRateChangeCHANGERATE)
	ONEVENT(MASTER_RATE_CHANGE,          CONTENT,          CContentBridgeSlave::OnMasterRateChangeCONTENT)
	ONEVENT(MASTER_RATE_CHANGE,          DISCONNECTING,    CContentBridgeSlave::OnMasterRateChangeDISCONNECTING)

	ONEVENT(TOKEN_WITHDRAW,              CONNECT_INACTIVE, CContentBridgeSlave::NullActionFunction)
	ONEVENT(TOKEN_WITHDRAW,              CONNECT,          CContentBridgeSlave::OnConfTokenWithdrawAnyCase)
	ONEVENT(TOKEN_WITHDRAW,              CHANGERATE,       CContentBridgeSlave::OnConfTokenWithdrawAnyCase)
	ONEVENT(TOKEN_WITHDRAW,              CONTENT,          CContentBridgeSlave::OnConfTokenWithdrawAnyCase)
	ONEVENT(TOKEN_WITHDRAW,              DISCONNECTING,    CContentBridgeSlave::OnConfTokenWithdrawAnyCase)

	ONEVENT(HW_CONTENT_ON_OFF_ACK,       CONNECT,          CContentBridgeSlave::OnPartyHWContentOnOffAckCONNECT)
	ONEVENT(HW_CONTENT_ON_OFF_ACK,       CHANGERATE,       CContentBridgeSlave::OnPartyHWContentOnOffAckCHANGERATE)
	ONEVENT(HW_CONTENT_ON_OFF_ACK,       CONTENT,          CContentBridgeSlave::OnPartyHWContentOnOffAckCONTECT)

	ONEVENT(NEWTOKENHOLDER,              CONNECT,          CContentBridgeSlave::OnTokenNewTokenHolderCONNECT)
	ONEVENT(NEWTOKENHOLDER,              CHANGERATE,       CContentBridgeSlave::OnTokenNewTokenHolderCHANGERATE)
	// Master inform the Slave on AcquireAck
	ONEVENT(PARTY_TOKEN_ACQUIRE_ACK,     IDLE,             CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE_ACK,     CONNECT_INACTIVE, CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE_ACK,     CONNECT,          CContentBridgeSlave::OnPartyTokenAcquireAckCONNECT)
	ONEVENT(PARTY_TOKEN_ACQUIRE_ACK,     CHANGERATE,       CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE_ACK,     CONTENT,          CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE_ACK,     DISCONNECTING,    CContentBridgeSlave::NullActionFunction)

	ONEVENT(PARTY_TOKEN_ACQUIRE_NAK,     IDLE,             CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE_NAK,     CONNECT_INACTIVE, CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE_NAK,     CONNECT,          CContentBridgeSlave::OnPartyTokenAcquireNakCONNECT)
	ONEVENT(PARTY_TOKEN_ACQUIRE_NAK,     CHANGERATE,       CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE_NAK,     CONTENT,          CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE_NAK,     DISCONNECTING,    CContentBridgeSlave::NullActionFunction)
	// token_withdraw message from Master
	ONEVENT(PARTY_TOKEN_WITHDRAW,        CONNECT_INACTIVE, CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_WITHDRAW,        CONNECT,          CContentBridgeSlave::OnMasterTokenWithdrawCONNECT)
	ONEVENT(PARTY_TOKEN_WITHDRAW,        CHANGERATE,       CContentBridgeSlave::OnMasterTokenWithdrawCHANGERATE)
	ONEVENT(PARTY_TOKEN_WITHDRAW,        CONTENT,          CContentBridgeSlave::OnMasterTokenWithdrawCONTENT)
	ONEVENT(PARTY_TOKEN_WITHDRAW,        DISCONNECTING,    CContentBridgeSlave::OnMasterTokenWithdrawDISCONNECTING)
	// ////////////////////////////////////////////////////////////
	// party task messages for bridge    //////////////////////////
	// ////////////////////////////////////////////////////////////
	// token_acquire  message
	ONEVENT(PARTY_TOKEN_ACQUIRE,         IDLE,             CContentBridgeSlave::OnPartyTokenAcquireIDLE)
	ONEVENT(PARTY_TOKEN_ACQUIRE,         CONNECT_INACTIVE, CContentBridgeSlave::NullActionFunction)
	ONEVENT(PARTY_TOKEN_ACQUIRE,         CONNECT,          CContentBridgeSlave::OnPartyTokenAcquireCONNECT)
	// token_acquire while content is active.
	ONEVENT(PARTY_TOKEN_ACQUIRE,         CHANGERATE,       CContentBridgeSlave::OnPartyTokenAcquireCHANGERATE)
	ONEVENT(PARTY_TOKEN_ACQUIRE,         CONTENT,          CContentBridgeSlave::OnPartyTokenAcquireCONTENT)
	// token_release  message - Parent functions are valid
	// ContentToken Decide on forwarding the token MSG
	ONEVENT(FORWARD_TOKEN_MSG_TO_MASTER, CONNECT_INACTIVE, CContentBridgeSlave::NullActionFunction)
	ONEVENT(FORWARD_TOKEN_MSG_TO_MASTER, IDLE,             CContentBridgeSlave::NullActionFunction)
	ONEVENT(FORWARD_TOKEN_MSG_TO_MASTER, CONNECT,          CContentBridgeSlave::OnForwardTokenMsgToMasterCONNECT)
	ONEVENT(FORWARD_TOKEN_MSG_TO_MASTER, CHANGERATE,       CContentBridgeSlave::NullActionFunction)
	ONEVENT(FORWARD_TOKEN_MSG_TO_MASTER, CONTENT,          CContentBridgeSlave::OnForwardTokenMsgToMasterCONTENT)
	ONEVENT(FORWARD_TOKEN_MSG_TO_MASTER, DISCONNECTING,    CContentBridgeSlave::NullActionFunction)

	ONEVENT(CONNECTPARTY,                CHANGERATE,       CContentBridgeSlave::OnConfConnectPartyCHANGERATE)
	ONEVENT(CONNECTPARTY,                CONTENT,          CContentBridgeSlave::OnConfConnectPartyCONTENT)

PEND_MESSAGE_MAP(CContentBridgeSlave, CContentBridge);

////////////////////////////////////////////////////////////////////////////
//                        CContentBridgeSlave
////////////////////////////////////////////////////////////////////////////
CContentBridgeSlave::CContentBridgeSlave(void)
{
	m_pLinkToMasterPartyCntl = NULL;
}

//--------------------------------------------------------------------------
CContentBridgeSlave::~CContentBridgeSlave(void)
{
	// Since the pointer is the same as the one in the party list it will be deallocated with the list !!!
	// POBJDELETE(m_pLinkToMasterPartyCntl);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::Create(const CContentBridge* pOtherBridge)
{
	(CBridge&)(*this) = (CBridge&)(*pOtherBridge);

	if (m_pToken)
		POBJDELETE(m_pToken);

	m_pToken = new CContentCascadeSlaveToken;
	(CContentToken&)(*m_pToken) = (CContentToken&)(*(pOtherBridge->m_pToken));
	AllocateLinkPartyList();
	if (pOtherBridge->GetCascadeLinkSlavePartyList() != NULL)
		*m_pCascadeSlavePartiesList = *(pOtherBridge->GetCascadeLinkSlavePartyList());

	// m_pLinkToMasterPartyCntl on party connect
	m_byCurrentContentRate     = pOtherBridge->m_byCurrentContentRate;
	m_byCurrentContentProtocol = pOtherBridge->m_byCurrentContentProtocol;
	m_byH264HighProfile = pOtherBridge->m_byH264HighProfile;  //HP content
	SetExclusiveContentMode(pOtherBridge->IsExclusiveContentMode());

	BYTE ready = ((CContentBridge*)pOtherBridge)->IsSpeakerHWReadyToContent();
	SetSpeakerHWReadyToContent(ready);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::Create(const CContentBridgeInitParams* pContentInitParams)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	if (!CPObject::IsValidPObjectPtr(pContentInitParams))
	{
		PASSERTMSG(1, "Invalid ContentBridgeInitParams");
		m_pConfApi->EndContentBrdgConnect(statInconsistent);
		return;
	}

	CBridge::Create(pContentInitParams);

	m_byCurrentContentProtocol = pContentInitParams->GetContentProtocol();
	m_byH264HighProfile = pContentInitParams->GetContentH264HighProfile();  //HP content
	AllocateLinkPartyList();
	// token initialization
	m_pToken = new CContentCascadeSlaveToken;
	m_pToken->InitToken(m_pConf->GetRcvMbx(), ((CTaskApi*)m_pConfApi));
	SetSpeakerHWReadyToContent(NO); // Speaker HW is NOT ready yet
	SetExclusiveContentMode(pContentInitParams->GetIsExclusiveContentMode());
	m_state = CONNECT;
	m_pConfApi->EndContentBrdgConnect(statOK);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnConfConnectPartyCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	BYTE isConnectWhileContent = FALSE;
	OnPartyConnect(pParam, isConnectWhileContent);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnConfConnectPartyCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	// CHINA usages
	BYTE isConnectWhileContent = FALSE;
	// BYTE isConnectWhileContent = TRUE;
	OnPartyConnect(pParam, isConnectWhileContent);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnConfConnectPartyCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	// CHINA usages
	BYTE isConnectWhileContent = FALSE;
	// BYTE isConnectWhileContent = TRUE;
	OnPartyConnect(pParam, isConnectWhileContent);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyConnect(CSegment* pParam, BYTE isConnectWhileContent)
{
	CContentBridgePartyInitParams partyInitParams;
	partyInitParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID partyId = partyInitParams.GetPartyRsrcID();

	// Avoid repeated connection of the party to the bridge
	PASSERTSTREAM_AND_RETURN(GetPartyCntl(partyId) != NULL, "PartyId:" << partyId << " - Already exist in Bridge");

	if (!partyInitParams.IsValidParams())
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Invalid Content party params";
		CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_CONTENT_DISCONNECTED, statInvalidPartyInitParams);
		return;
	}

	TRACEINTO
		<< "PartyId:" << partyId
		<< ", PartyContentProtocol:" << GetContentProtocolAsString(partyInitParams.GetByCurrentContentProtocol())
		<< ", ConfContentProtocol:" << GetContentProtocolAsString(m_byCurrentContentProtocol)
		<< ", PartyContentProfile:" << (int)(partyInitParams.GetByCurrentContentH264HighProfile())
		<< ", ConfContentProfile:" << (int)m_byH264HighProfile;  //HP content


	// When connect party to content bridge - content protocol must be equal to bridge protocol
	if ((partyInitParams.GetByCurrentContentProtocol() != m_byCurrentContentProtocol) || (partyInitParams.GetByCurrentContentProtocol() == H264 && partyInitParams.GetByCurrentContentH264HighProfile() != m_byH264HighProfile))  //HP content
	{
		if (IsXCodeConf())
		{
			TRACEINTO << "PartyId:" << partyId << " - Party Content Protocol Can be different from Conference Content Protocol in XCode Conference";
		}
		else
		{
			TRACEINTO << "PartyId:" << partyId << " - Party Content protocol is not equal to Conference Content Protocol, connection rejected";
			CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_CONTENT_DISCONNECTED, statInvalidPartyInitParams);
			return;
		}
	}

	CContentBridgePartyCntl* pPartyCntl = new CContentBridgePartyCntl();
	pPartyCntl->Create(&partyInitParams);

	if (CASCADE_MODE_SLAVE == pPartyCntl->GetCascadeLinkMode())
	{
		if (CPObject::IsValidPObjectPtr(m_pLinkToMasterPartyCntl))
		{
			TRACEINTO << "PartyId:" << partyId << " - Only one Link-To-Master(SlaveLink) is available in a conference";
			CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_CONTENT_DISCONNECTED, statInvalidPartyInitParams);
			return;
		}
		else if (TRUE == isConnectWhileContent)   // Block the slave to connect to master while there is content on slave to avoid conflicts
		{
			TRACEINTO << "PartyId:" << partyId << " - Slave with active content can`t be connected to the Master";
			CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_CONTENT_DISCONNECTED, statInconsistent);
			return;
		}
		else
		{
			m_pLinkToMasterPartyCntl = pPartyCntl;   // two pointers to the same partyControl
			TRACEINTO << "PartyId:" << partyId << " - Link-To-Master party connected";
		}
	}

	// for slave link INTRA suppression
	if (partyInitParams.GetCascadeLinkMode() == CASCADE_MODE_MASTER)      // links to slave are configured MASTER
		m_pCascadeSlavePartiesList->Insert((CBridgePartyCntl*)pPartyCntl);

	// Insert the party to the PartyCtl List and activate Connect on it
	CBridge::ConnectParty(pPartyCntl);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterStartContentCONNECT_INACTIVE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterStartContentCONNECT(CSegment* pParam)
{
	// 1.open the msg.
	// 2.take the rate from params.
	// 3.update the content bridge with this rate.

	CTaskApp* pParty;
	BYTE mcuNum;
	BYTE termNum;
	DWORD rate;
	BYTE label = LABEL_CONTENT;
	BYTE randomNumber = 0;

	*pParam >> (void*&)pParty >> mcuNum >> termNum >> rate;

	TRACEINTO << "ConfName:" << m_pConfName << ", mcuNum:" << (int)mcuNum << ", terminalNum:" << (int)termNum;

	CSegment* pSeg = new CSegment;
	*pSeg << (void*&)pParty << mcuNum << termNum << label << randomNumber;

	CContentBridge::OnPartyTokenAcquireCONNECT(pSeg);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterStartContentCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	CascadePartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterStartContentCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	CascadePartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterStartContentDISCONNECTING(CSegment* pParam)
{
	// The master got timeOUT on receiving RateIsChanged and will disconnect this slave from its content bridge
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterRateChangeCONNECT_INACTIVE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterRateChangeCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	MasterRateChange(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterRateChangeCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	MasterRateChange(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterRateChangeCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	MasterRateChange(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterRateChangeDISCONNECTING(CSegment* pParam)
{
	// The master got timeOUT on receiving RateIsChanged and will disconnect this slave from its content bridge
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored";
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::MasterRateChange(CSegment* pPartyParams)
{
	CParty* pParty;
	BYTE mcuNumber;
	BYTE terminalnumber;
	BYTE randomNumber;
	DWORD contentRate;
	DWORD oldSegRead = pPartyParams->GetRdOffset();
	*pPartyParams >> (void*&)pParty >> mcuNumber >> terminalnumber >> contentRate;
	DWORD newSegRead = pPartyParams->GetRdOffset();
	pPartyParams->DecRead(newSegRead - oldSegRead);

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party is not connected to Content Bridge";
		return;
	}

	TRACEINTO << "PartyId:" << partyId << ", mcuNum:" << (int)mcuNumber << ", terminalNum:" << (int)terminalnumber;

	if (pPartyCntl != m_pLinkToMasterPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Not received from SlaveLinkParty";
		// fix for 23051	23026 23067	23061 22740
		if (m_pLinkToMasterPartyCntl)
		{
			CParty* pLinkID = (CParty*)m_pLinkToMasterPartyCntl->GetPartyTaskApp();
			TRACEINTO << "MasterPartyName:" << pLinkID->GetName() << ", MasterPartyId:" << m_pLinkToMasterPartyCntl->GetPartyRsrcID();
		}
		else
			DBGPASSERT(1);
	}

	if (contentRate)
		m_pConf->SetLastRateFromMaster(contentRate);
	else
		ResetLastContentRateFromMaster();     // never set the rate from master to ZERO!

	m_pToken->RateChangeFromMaster(pPartyParams);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnConfTokenWithdrawAnyCase(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	DBGPASSERT_AND_RETURN(1); // try to block it before arriving here
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyHWContentOnOffAckCONNECT(CSegment* pParam)
{
	// On basic bridge STREAM ON will always arrive on RateChange since when ack is sent state is changed to change rate and process begin
	// On Slave bridge we are waiting both to Ack from Master and RateChange, to begin the process so HWContentOn may arrive on CONNECT state.
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyHWContentOnOffAck(pParam);
	SetSpeakerHWReadyToContent(YES); // When ChangeRate Done intra will be send !!!
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyHWContentOnOffAckCHANGERATE(CSegment* pParam)
{
	// On basic bridge STREAM ON will always arrive on RateChange since when ack is sent state is changed to change rate and process begin
	// On Slave bridge we are waiting both to Ack from Master and RateChange, to begin the process so HWContentOn may arrive on CONNECT state.
	TRACEINTO << "ConfName:" << m_pConfName;
	CContentBridge::OnPartyHWContentOnOffAckCHANGERATE(pParam);
	// SetSpeakerHWReadyToContent(YES); //When ChangeRate Done intra will be send !!!
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyHWContentOnOffAckCONTECT(CSegment* pParam)
{
	// On basic bridge STREAM ON will always arrive on RateChange since when ack is sent state is changed to change rate and process begin
	// On Slave bridge we are waiting both to Ack from Master and RateChange, to begin the process so HWContentOn may arrive on CONNECT state and sometimes on CONTENT(after changeModeDone)state.
	TRACEINTO << "ConfName:" << m_pConfName;
	CContentBridge::OnPartyHWContentOnOffAckCONTENT(pParam);
	InformMasterOnRateChangeDone();
}

//--------------------------------------------------------------------------/
void CContentBridgeSlave::OnPartyTokenAcquireAckCONNECT(CSegment* pPartyParams)
{
	CParty* pParty;
	BYTE mcuNumber;
	BYTE terminalNumber;
	DWORD oldSegRead = pPartyParams->GetRdOffset();
	*pPartyParams >> (void*&)pParty >> mcuNumber >> terminalNumber;
	DWORD newSegRead = pPartyParams->GetRdOffset();
	pPartyParams->DecRead(newSegRead - oldSegRead);

	DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party is not connected to Content Bridge";
		return;
	}

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNumber << ", TerminalNum:" << (int)terminalNumber;

	if (m_pToken->GetTokenHolderPartyId() != partyId)
	{
		const CParty* pTokenHolderParty = (CParty*)m_pToken->GetTokenHolderParty();

		if (!CPObject::IsValidPObjectPtr(pTokenHolderParty))
		{
			TRACEINTO << "PartyId:" << partyId << " - Slave party is disconnected state (problem)";
			// VNGFE_5887: If the Ack comes after the original requester disconnects, m_currentTokenHolder will be set to none,
			// pTokenRequesterParty will be NULL, and according to H.239 11.2.1: End-user system does not own and does not want the token,
			// cascade slave should respond to presentationTokenResponse(acknowledge) by sending presentationTokenRelease.
			m_pToken->Release(partyId, mcuNumber, terminalNumber);
			return;
		}
		else if (GetPartyCntl(pTokenHolderParty->GetPartyRsrcID()) == NULL)
		{
			TRACEINTO << "PartyId:" << partyId << " - Slave party is disconnecting state (problem)";
			m_pToken->Release(m_pToken->GetTokenHolderPartyId(), m_pToken->GetTokenHolderMcuNumber(), m_pToken->GetTokenHolderTermNumber());
			return;
		}
	}

	// send message to token
	m_pToken->AcquireAckFromMaster(pPartyParams);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyTokenAcquireNakCONNECT(CSegment* pPartyParams)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	((CContentCascadeSlaveToken*)m_pToken)->ResetCurrentTokenHolder();

	// send message to token (as on Ack)
	// m_pToken->AcquireNakFromMaster(pPartyParams);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyTokenAcquireIDLE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyTokenAcquireCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	OnPartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyTokenAcquireCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	CascadePartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnPartyTokenAcquireCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	CascadePartyTokenAcquire(pParam);
}

//--------------------------------------------------------------------------
// Slave party ask for token
void CContentBridgeSlave::OnPartyTokenAcquire(CSegment* pParam)
{
	CParty* pParty;
	BYTE mcuNumber      = 0;
	BYTE terminalnumber = 0;
	BYTE label          = 0;
	BYTE randomNumber   = 0;

	*pParam >> (void*&)pParty >> mcuNumber >> terminalnumber >> label >> randomNumber;

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pParty));

	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)GetPartyCntl(partyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "PartyId:" << partyId << " - Party is not connected to Content Bridge";
		m_pToken->BadAcquire(partyId, mcuNumber, terminalnumber);
		return;
	}

	// VNGFE-5974 - Restrict content for lecturer on slave.
	if (!IsConfLectureModeAllowTokenAcquire(pParty))
	{
		TRACEINTO << "PartyId:" << partyId << " - Conference lecture mode doesn't allow party acquire request";
		m_pToken->BadAcquire(partyId, mcuNumber, terminalnumber);
		return;
	}

	TRACEINTO << "PartyId:" << partyId << ", McuNum:" << (int)mcuNumber << ", TerminalNum:" << (int)terminalnumber;

	// VNGFE-2468 the label was ignored in token function, so the value of the randomNum was set to the value of label
	CSegment* pSeg = new CSegment;
	*pSeg << partyId << mcuNumber << terminalnumber << randomNumber;

	// send message to token
	m_pToken->SlaveAcquire(pSeg);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnForwardTokenMsgToMasterCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	if (CPObject::IsValidPObjectPtr(m_pLinkToMasterPartyCntl))
		m_pLinkToMasterPartyCntl->ForwardContentTokenMsgToMaster(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnForwardTokenMsgToMasterCONTENT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	if (CPObject::IsValidPObjectPtr(m_pLinkToMasterPartyCntl))
		m_pLinkToMasterPartyCntl->ForwardContentTokenMsgToMaster(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterTokenWithdrawCONNECT(CSegment* pParam)
{
	DWORD subOpcode;
	BYTE  mcuNumber;
	BYTE  terminalNumber;
	BYTE  label;
	BYTE  randomNum;

	*pParam >> subOpcode >> mcuNumber >> terminalNumber >> label >> randomNum;

	TRACEINTO << "ConfName:" << m_pConfName << ", SubOcode:" << subOpcode << ", McuNumber:" << (int)mcuNumber << ", TerminalNumber:" << (int)terminalNumber;
	CContentBridge::OnConfTokenWithdrawCONNECT(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterTokenWithdrawCHANGERATE(CSegment* pParam)
{
	DWORD subOpcode;
	BYTE  mcuNumber;
	BYTE  terminalNumber;
	BYTE  label;
	BYTE  randomNum;

	*pParam >> subOpcode >> mcuNumber >> terminalNumber >> label >> randomNum;

	TRACEINTO << "ConfName:" << m_pConfName << ", SubOcode:" << subOpcode << ", McuNumber:" << (int)mcuNumber << ", TerminalNumber:" << (int)terminalNumber;
	CContentBridge::OnConfTokenWithdrawCHANGERATE(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterTokenWithdrawCONTENT(CSegment* pParam)
{
	DWORD subOpcode;
	BYTE  mcuNumber;
	BYTE  terminalNumber;
	BYTE  label;
	BYTE  randomNum;

	*pParam >> subOpcode >> mcuNumber >> terminalNumber >> label >> randomNum;

	TRACEINTO << "ConfName:" << m_pConfName << ", SubOcode:" << subOpcode << ", McuNumber:" << (int)mcuNumber << ", TerminalNumber:" << (int)terminalNumber;
	CContentBridge::OnConfTokenWithdrawCONTENT(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnMasterTokenWithdrawDISCONNECTING(CSegment* pParam)
{
	DWORD subOpcode;
	BYTE  mcuNumber;
	BYTE  terminalNumber;
	BYTE  label;
	BYTE  randomNum;

	*pParam >> subOpcode >> mcuNumber >> terminalNumber >> label >> randomNum;

	TRACEINTO << "ConfName:" << m_pConfName << ", SubOcode:" << subOpcode << ", McuNumber:" << (int)mcuNumber << ", TerminalNumber:" << (int)terminalNumber;
	CContentBridge::OnConfTokenWithdrawDISCONNECTING(pParam);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::IsRateChangeDone()
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			if ((pPartyCntl->GetPartyContentRate() != m_byCurrentContentRate)
			    && (!IsXCodeConf() || (IsXCodeConf() && (m_byCurrentContentRate == AMC_0k || pPartyCntl->GetPartyContentRate() == AMC_0k))))
			{
				TRACEINTO << "PartyId:" << pPartyCntl->GetPartyRsrcID()
				          << ", PartyName:" << pPartyCntl->GetName()
				          << ", PartyContentRate:" << GetContentRateAsString(pPartyCntl->GetPartyContentRate())
				          << ", ConfContentRate:" << GetContentRateAsString(m_byCurrentContentRate)
				          << " - Content rate still not changed";
				return;
			}
		}
	}

	TRACEINTO << "ConfName:" << m_pConfName << " - Done";

	DeleteTimer(CHANGERATETOUT);
	ChangePresentationRate();

	if (YES == IsSpeakerHWReadyToContent())
		InformMasterOnRateChangeDone();
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::InformMasterOnRateChangeDone()
{
	TRACEINTO << "ConfName:" << m_pConfName;
	if (CPObject::IsValidPObjectPtr(m_pLinkToMasterPartyCntl))
		m_pLinkToMasterPartyCntl->SendRateChangeDoneToMaster();
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::OnTokenNewTokenHolderCONNECT(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	BYTE ready = IsSpeakerHWReadyToContent();
	CContentBridge::OnTokenNewTokenHolderCONNECT(pParam);
	// In SlaveBridge STREAM ON can arrive on connect state.so we are saving it before NewTokenHolder set it to NO
	SetSpeakerHWReadyToContent(ready);
}
//--------------------------------------------------------------------------
void CContentBridgeSlave::OnTokenNewTokenHolderCHANGERATE(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;
	BYTE ready = IsSpeakerHWReadyToContent();
	CContentBridge::OnTokenNewTokenHolderCHANGERATE(pParam);
	SetSpeakerHWReadyToContent(ready);
}
//--------------------------------------------------------------------------
void CContentBridgeSlave::ResetLastContentRateFromMaster()
{
	PASSERT_AND_RETURN(!m_pLinkToMasterPartyCntl);

	TRACEINTO << "ConfId: " << m_confRsrcID;
	CTaskApp* pParty = m_pLinkToMasterPartyCntl->GetPartyTaskApp();
	m_pConf->SetLastRateFromMaster (pParty);
}

//--------------------------------------------------------------------------
void CContentBridgeSlave::SendMessagesToLinks(WORD opcode, BYTE McuNum, BYTE TermNum, BYTE sendToSpeaker)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			CParty* pParty = (CParty*)pPartyCntl->GetPartyTaskApp();
			if (sendToSpeaker != YES && IsTokenHolder(pParty))
				continue;

			if (pParty->IsRemoteIsSlaveMGCWithContent())
			{
				TRACEINTO << "PartyId:" << pParty->GetPartyRsrcID() << " - Forward to MGC";
				pPartyCntl->ForwardToParty(opcode, McuNum, TermNum);
			}
		}
	}
}



PBEGIN_MESSAGE_MAP(CContentBridgeMaster)

PEND_MESSAGE_MAP(CContentBridgeMaster, CContentBridge);

////////////////////////////////////////////////////////////////////////////
//                        CContentBridgeMaster
////////////////////////////////////////////////////////////////////////////
CContentBridgeMaster::CContentBridgeMaster(void)
{
}

//--------------------------------------------------------------------------
CContentBridgeMaster::~CContentBridgeMaster(void)
{
}

//--------------------------------------------------------------------------
void CContentBridgeMaster::Create(const CContentBridge* pOtherBridge)
{
	(CBridge&)(*this) = (CBridge&)(*pOtherBridge);

	if (m_pToken)
		POBJDELETE(m_pToken);

	m_pToken = new CContentCascadeMasterToken;
	(CContentToken&)(*m_pToken) = (CContentToken&)(*(pOtherBridge->m_pToken));

	m_byCurrentContentRate     = pOtherBridge->m_byCurrentContentRate;
	m_byCurrentContentProtocol = pOtherBridge->m_byCurrentContentProtocol;
	m_byH264HighProfile = pOtherBridge->m_byH264HighProfile;  //HP content

	AllocateLinkPartyList();
	if (pOtherBridge->GetCascadeLinkSlavePartyList() != NULL)
		*m_pCascadeSlavePartiesList = *(pOtherBridge->GetCascadeLinkSlavePartyList());

	BYTE ready = ((CContentBridge*)pOtherBridge)->IsSpeakerHWReadyToContent();
	SetSpeakerHWReadyToContent(ready);
	SetExclusiveContentMode(pOtherBridge->IsExclusiveContentMode());
}

//--------------------------------------------------------------------------
void CContentBridgeMaster::SendMessagesToLinks(WORD opcode, BYTE McuNum, BYTE TermNum, BYTE sendToSpeaker)
{
	CBridgePartyList::iterator _end = m_pPartyList->end();
	for (CBridgePartyList::iterator _itr = m_pPartyList->begin(); _itr != _end; ++_itr)
	{
		// All party controls in Content Bridge are CContentBridgePartyCntl, so casting is valid
		CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)_itr->second;
		if (pPartyCntl)
		{
			CParty* pParty = (CParty*)pPartyCntl->GetPartyTaskApp();
			if (sendToSpeaker != YES && IsTokenHolder(pParty))
				continue;

			if (pParty->IsRemoteIsSlaveMGCWithContent())
				pPartyCntl->ForwardToParty(opcode, McuNum, TermNum);
		}
	}
}
