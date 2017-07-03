//+========================================================================+
//                   ContentBridgePartyCntl.cpp                            |
//					 Copyright 2006 Polycom, Inc                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// FILE:       ContentBridgePartyCntl.cpp                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yoella                                                      |
//-------------------------------------------------------------------------|
// Who  | Date  July-2006  | Description                                   |
//-------------------------------------------------------------------------|


#include "ContentBridgePartyCntl.h"


PBEGIN_MESSAGE_MAP(CContentBridgePartyCntl)

	//   connect party control to brdg
	ONEVENT(CONTCONNECT     ,IDLE         ,CContentBridgePartyCntl::OnContentBridgeConnectIDLE)

	//   disconnect party control from brdg
	ONEVENT(CONTDISCONNECT  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeDisConnectCONNECT )
	ONEVENT(CONTDISCONNECT  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeDisConnectCONTENT )
	ONEVENT(CONTDISCONNECT  ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeDisConnectDISCONNECTING )

	//   change presentation rate
	ONEVENT(CHANGE_PRESENTATION_RATE  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeChangePresentationRateCONNECT )
	ONEVENT(CHANGE_PRESENTATION_RATE  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeChangePresentationRateCONTENT)
	ONEVENT(CHANGE_PRESENTATION_RATE  ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeChangePresentationRateDISCONNECTING )

	//   change presentation speaker
	ONEVENT(CHANGE_PRESENTATION_SPEAKER  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerCONNECT )
	ONEVENT(CHANGE_PRESENTATION_SPEAKER  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerCONTENT)
	ONEVENT(CHANGE_PRESENTATION_SPEAKER  ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerDISCONNECTING)

	//   set party rate
	ONEVENT(SET_PARTY_RATE  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeSetRateCONNECT)
	ONEVENT(SET_PARTY_RATE  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeSetRateCONTENT )
	ONEVENT(SET_PARTY_RATE  ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeSetRateDISCONNECTING )

	// Send Freeze to party
	ONEVENT(SEND_FREEZE     ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeSendFreezeCONNECT)
	ONEVENT(SEND_FREEZE     ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeSendFreezeCONTENT)
	ONEVENT(SEND_FREEZE     ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeSendFreezeDISCONNECTING)

	//   send refresh to party
	ONEVENT(SEND_REFRESH  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeSendRefreshCONNECT )
	ONEVENT(SEND_REFRESH  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeSendRefreshCONTENT)
	ONEVENT(SEND_REFRESH  ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeSendRefreshDISCONNECTING)

	//   send Rate Change Done to SlaveLink - to inform the master.
	ONEVENT(SEND_RATE_CHANGE_DONE  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneCONNECT )
	ONEVENT(SEND_RATE_CHANGE_DONE  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneCONTENT)
	ONEVENT(SEND_RATE_CHANGE_DONE  ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneDISCONNECTING)

	ONEVENT(FORWARD_CONTENT_TOKEN_MSG  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeForwardTokenMsgToMasterCONNECT )
	ONEVENT(FORWARD_CONTENT_TOKEN_MSG  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeForwardTokenMsgToMasterCONTENT)


	//   send NS-IND/RoleProviderIdentity to party
	ONEVENT(PROVIDER_IDENTITY  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeProviderIdentityCONNECT)
	ONEVENT(PROVIDER_IDENTITY  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeProviderIdentityCONTENT)
	ONEVENT(PROVIDER_IDENTITY  ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeProviderIdentityDISCONNECTING)

	//   send FLOW_CONTROL_RELEASE_RES to party
//	ONEVENT(FLOW_CONTROL_RELEASE_RES  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeFlowControlReleaseResponse )
//	ONEVENT(FLOW_CONTROL_RELEASE_RES  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeFlowControlReleaseResponse)

	//   send MediaProducerStatus to party
	ONEVENT(MEDIA_PRODUCER  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusCONNECT)
	ONEVENT(MEDIA_PRODUCER  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusCONTENT)
	ONEVENT(MEDIA_PRODUCER  ,ST_DISCONNECTING   ,CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusDISCONNECTING)

//   send NS-IND/NoRoleProvider to party
	ONEVENT(NO_ROLE_PROVIDER  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeNoProviderCONNECT)
	ONEVENT(NO_ROLE_PROVIDER  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeNoProviderCONTENT)
	ONEVENT(NO_ROLE_PROVIDER  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeNoProviderDISCONNECTING)

	ONEVENT(CONTENT_BRDG_PARTY_INTRA_SUPPRESS_TOUT, ST_CONNECT,			CContentBridgePartyCntl::OnTimerPartyIntraSuppressed)
	ONEVENT(CONTENT_BRDG_PARTY_INTRA_SUPPRESS_TOUT, ST_CONTENT,			CContentBridgePartyCntl::OnTimerPartyIntraSuppressed)
	ONEVENT(CONTENT_BRDG_PARTY_INTRA_SUPPRESS_TOUT, ST_DISCONNECTING,	CContentBridgePartyCntl::OnTimerPartyIntraSuppressed)

PEND_MESSAGE_MAP(CContentBridgePartyCntl,CStateMachine);



////////////////////////////////////////////////////////////////////////////
CContentBridgePartyCntl::CContentBridgePartyCntl()
{
	m_byCurrentContentRate = AMC_0k;
	m_byCurrentContentProtocol = H263;
	//m_bIsLinkToMaster = FALSE;
	//m_bCascadeLinkMode = NO;
	m_IsPartyNoiseSuppressed = FALSE;
	m_isIntraSupressionEnabled = true;
	m_PartyIntraRequestsTime = new DWORD_VECTOR ;
	m_byH264HighProfile = FALSE;  //HP content

	VALIDATEMESSAGEMAP;
}

////////////////////////////////////////////////////////////////////////////
CContentBridgePartyCntl::~CContentBridgePartyCntl()
{
	DeleteTimer(CONTENT_BRDG_PARTY_INTRA_SUPPRESS_TOUT);
	PDELETE(m_PartyIntraRequestsTime);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::Destroy( void )
{
	POBJDELETE(m_pPartyApi);
	PDELETE(m_PartyIntraRequestsTime);
}
////////////////////////////////////////////////////////////////////////////
void* CContentBridgePartyCntl::GetMessageMap( void )
{
	return (void*)m_msgEntries;
}

// ------------------------------------------------------------
CContentBridgePartyCntl& CContentBridgePartyCntl::operator= (const CContentBridgePartyCntl& rOtherContentBridgePartyCntl)
{
	if ( &rOtherContentBridgePartyCntl == this )
		return *this;

	(CBridgePartyCntl&)(*this) = (CBridgePartyCntl&)rOtherContentBridgePartyCntl;

	m_byCurrentContentRate		= rOtherContentBridgePartyCntl.m_byCurrentContentRate;
	m_byCurrentContentProtocol	= rOtherContentBridgePartyCntl.m_byCurrentContentProtocol;
	m_byH264HighProfile = rOtherContentBridgePartyCntl.m_byH264HighProfile;  //HP content
	m_mcuNumber					= rOtherContentBridgePartyCntl.m_mcuNumber;
	m_terminalNumber			= rOtherContentBridgePartyCntl.m_terminalNumber;
	//m_bIsLinkToMaster			= rOtherContentBridgePartyCntl.m_bIsLinkToMaster;
	//m_bCascadeLinkMode          = rOtherContentBridgePartyCntl.m_bCascadeLinkMode;
	m_state                     = rOtherContentBridgePartyCntl.m_state;
	m_isIntraSupressionEnabled = rOtherContentBridgePartyCntl.m_isIntraSupressionEnabled;

	if (rOtherContentBridgePartyCntl.m_PartyIntraRequestsTime)
	{
		PDELETE(m_PartyIntraRequestsTime);
		m_PartyIntraRequestsTime = new DWORD_VECTOR;
		DWORD_VECTOR::iterator itr =  rOtherContentBridgePartyCntl.m_PartyIntraRequestsTime->begin();
		while (itr != rOtherContentBridgePartyCntl.m_PartyIntraRequestsTime->end())
		{
			DWORD currentTime = *itr;
			m_PartyIntraRequestsTime->push_back(currentTime);
			itr++;
		}
	}


	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

///////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::Create (const CBridgePartyInitParams* pBridgePartyInitParams)
{

	DBGPASSERT_AND_RETURN(!(pBridgePartyInitParams->IsValidParams()));

	if( ((CContentBridgePartyInitParams*)pBridgePartyInitParams)->IsMyContentRateValid() )
		m_byCurrentContentRate = ((CContentBridgePartyInitParams*)pBridgePartyInitParams)->GetByCurrentContentRate();

	m_byCurrentContentProtocol = ((CContentBridgePartyInitParams*)pBridgePartyInitParams)->GetByCurrentContentProtocol();
	m_byH264HighProfile = ((CContentBridgePartyInitParams*)pBridgePartyInitParams)->GetByCurrentContentH264HighProfile();  //HP content

	((CContentBridgePartyInitParams*)pBridgePartyInitParams)->SetMyPartyNumbers(m_mcuNumber,m_terminalNumber);

	//m_bIsLinkToMaster = ((CContentBridgePartyInitParams*)pBridgePartyInitParams)->IsPartyLinkToMaster();
	m_bCascadeLinkMode = (/*(CContentBridgePartyInitParams*)*/pBridgePartyInitParams)->GetCascadeLinkMode();

	// Create base params
	CBridgePartyCntl::Create(pBridgePartyInitParams);

}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::Connect()
{
	DispatchEvent(CONTCONNECT,NULL);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::DisConnect()
{
	DispatchEvent(CONTDISCONNECT,NULL);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::ChangePresentationRate()
{
	DispatchEvent(CHANGE_PRESENTATION_RATE,NULL);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::ChangePresentationSpeaker()
{
	DispatchEvent(CHANGE_PRESENTATION_SPEAKER,NULL);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::SetRate(const BYTE newPartyRate)
{
	CSegment* pSeg = new CSegment;
	*pSeg	<< newPartyRate;

	DispatchEvent(SET_PARTY_RATE,pSeg);
	POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::SendFreeze()
{
	// send FREEZE to party task
	DispatchEvent(SEND_FREEZE,NULL);
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::SendRefresh()
{
	// send REFRESH to party task
	DispatchEvent(SEND_REFRESH,NULL);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::SendRateChangeDoneToMaster()
{
	DispatchEvent(SEND_RATE_CHANGE_DONE,NULL);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::ForwardContentTokenMsgToMaster(CSegment* pParam)
{
	DispatchEvent(FORWARD_CONTENT_TOKEN_MSG,pParam);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::ProviderIdentity(const BYTE mcuNum,const BYTE terminalNum,
				const BYTE label,const BYTE dataSize,const BYTE* pData)
{
	// send NS-IND/RoleProviderIdentity to party task
	CSegment* pSeg = new CSegment;
	*pSeg	<< mcuNum
			<< terminalNum
			<< label
			<< dataSize;
	for( WORD i=0; i<dataSize; i++ )
		*pSeg << pData[i];
	DispatchEvent(PROVIDER_IDENTITY,pSeg);
	POBJDELETE(pSeg);
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::FlowControlReleaseResponse(const BYTE isAck,const WORD bitRate)
{
	// send
	CSegment* pSeg = new CSegment;
	*pSeg	<< isAck
			<< bitRate;

	DispatchEvent(FLOW_CONTROL_RELEASE_RES,pSeg);
	POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::MediaProducerStatus(const BYTE channelId,const EMediaProducerStatus status)
{
	// MediaProducerStatus to party task
	CSegment* pSeg = new CSegment;
	*pSeg << channelId
		  << (BYTE)status;
	DispatchEvent(MEDIA_PRODUCER,pSeg);
	POBJDELETE(pSeg);
}
///////////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::SetMcuNum(BYTE mcuNum)
{
    m_mcuNumber=mcuNum;

}
/////////////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::SetTermNum(BYTE termNum)
{
    m_terminalNumber=termNum;

}

////////////////////////////////////////////////////////////////////////////
///////////////////       ACTION FUNCTIONS        //////////////////////////
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeConnectIDLE(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeConnectIDLE : Name - ",m_partyConfName);
#endif

	m_pConfApi->PartyContentBrdgConnect(m_partyRsrcID,CONTENTCNTL_MSG,statOK); // msg to party control
	m_state = ST_CONNECT;
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeDisConnectCONNECT(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeDisConnectCONNECT : Name - ",m_partyConfName);
#endif
	OnContentBridgeDisConnect(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeDisConnectCONTENT(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeDisConnectCONTENT : Name - ",m_partyConfName);
#endif
	OnContentBridgeDisConnect(pParam);
//	m_pPartyApi->SetContentSrcOff();
}
/////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeDisConnectDISCONNECTING(CSegment* pParams)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeDisConnectDISCONNECTING - Party is already disconnecting : Name - ", GetFullName());
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeDisConnect(CSegment* pParam)
{
	m_pConfApi->PartyContentBrdgDisConnect(m_partyRsrcID,CONTENTCNTL_MSG,statOK); // msg to control

	ResetPartyIntraSuppression();
	m_state = ST_DISCONNECTING;
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeChangePresentationRateCONNECT(CSegment* pParam)
{
	// this event we receive when start new presentation
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangePresentationRateCONNECT : Name - ",m_partyConfName);
#endif
	m_state = ST_CONTENT;
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeChangePresentationRateCONTENT(CSegment* pParam)
{
	// this event we receive when change rate of existing presentation
	// this event recived after  SET_PARTY_RATE(we can try to merge them - On MGC it was logic to seperate them since
	// switch was performed but in Carmel no switch is needed)
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangePresentationRateCONTENT : Name - ",m_partyConfName);
#endif

	if( m_byCurrentContentRate == AMC_0k )
	{
		ResetPartyIntraSuppression();
		m_state = ST_CONNECT;
		return;
	}
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeChangePresentationRateDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangePresentationRateDISCONNECTING - IGNORED: Name - ",m_partyConfName);
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerCONNECT : Name - ",m_partyConfName);
	DBGPASSERT(1);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerCONTENT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerCONTENT : Name - ",m_partyConfName);
	ResetPartyIntraSuppression();
	DBGPASSERT_AND_RETURN( m_byCurrentContentRate == AMC_0k );
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerDISCONNECTING : Name - ",m_partyConfName);

}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSetRateCONNECT(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSetRateCONNECT : Name - ",m_partyConfName);
#endif

	BYTE	newRate = AMC_0k;
	*pParam	>> newRate;
	m_byCurrentContentRate  = newRate;
}


////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSetRateCONTENT(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "PartyId: " << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSetRateCONTENT : Name - ",m_partyConfName);
#endif

	BYTE	newRate = AMC_0k;
	*pParam	>> newRate;
	m_byCurrentContentRate  = newRate;
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSetRateDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSetRateDISCONNECT : Name - ",m_partyConfName);

}

//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendFreezeCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendFreezeCONTENT - ignored : Name - ",m_partyConfName);
}
//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendFreezeCONTENT(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendFreezeCONTENT : Name - ",m_partyConfName);
#endif
	m_pPartyApi->SendEPCContentFreezePicture();
}
/////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendFreezeDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendFreezeDISCONNECTING - IGNORED: Name - ",m_partyConfName);

}

//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendRefreshCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendRefreshCONNECT - ignored : Name - ",m_partyConfName);
}


////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendRefreshCONTENT(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendRefreshCONTENT : Name - ",m_partyConfName);
#endif
	OnContentBridgeSendRefresh(pParam);
}
//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendRefreshDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendRefreshDISCONNECTING - ignored : Name - ",m_partyConfName);
}

//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneCONNECT - ignored : Name - ",m_partyConfName);
}


////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneCONTENT(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "ConfRsrcID:" << m_confRsrcID << " ,PartyRsrcID:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneCONTENT : Name - ",m_partyConfName);
#endif
	OnContentBridgeSendRateChangeDoneToMaster(pParam);
}
//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneDISCONNECTING - ignored : Name - ",m_partyConfName);
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendRateChangeDoneToMaster(CSegment* pParam)
{
	m_pPartyApi->SendContentRateChangeDone();
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeForwardTokenMsgToMaster(CSegment* pParam)
{
	BYTE	mcuNum, terminalNum, randomNum;
	OPCODE   opcode;

	*pParam >> opcode >> mcuNum	>> terminalNum >> randomNum;
	m_pPartyApi->SendContentMessage(opcode,mcuNum,terminalNum,randomNum,0);
}
//////////////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::ForwardToParty(WORD opcode,BYTE mcuNum,BYTE terminalNum)
{

    m_pPartyApi->SendContentMessage(opcode,mcuNum,terminalNum);

}
//////////////////////////////////////////////////////////////////////////////////////
BYTE CContentBridgePartyCntl::CheckIsPartyIntraSuppressed(BOOL insertNew)
{
	DWORD maxIntraRequestsPerInterval = 1; //default




	DWORD contentIntraRequestsSuppressDuration = 10;//default

	// we use the same flags as in COP
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetDWORDDataByKey("MAX_INTRA_REQUESTS_PER_INTERVAL_CONTENT", maxIntraRequestsPerInterval);
	sysConfig->GetDWORDDataByKey("MAX_INTRA_SUPPRESSION_DURATION_IN_SECONDS_CONTENT", contentIntraRequestsSuppressDuration);

	DWORD currentTime = SystemGetTickCount().GetSeconds();
	DWORD gapBetweenIntras;
	DWORD_VECTOR::iterator itr =  m_PartyIntraRequestsTime->begin();
	while (itr != m_PartyIntraRequestsTime->end())
	{
		gapBetweenIntras = currentTime- *itr;
		if(gapBetweenIntras >= contentIntraRequestsSuppressDuration)
		{
			m_PartyIntraRequestsTime->erase(itr);
		}
		else
			itr++;
	}
	BOOL canInsert = m_PartyIntraRequestsTime->size() < maxIntraRequestsPerInterval;
	if (insertNew && canInsert)
	{
		TRACEINTO << "CContentBridgePartyCntl::CheckIsPartyIntraSuppressed -  recording time of intra request from party: " << m_partyConfName << ", currently num intras is" << m_PartyIntraRequestsTime->size();
		m_PartyIntraRequestsTime->push_back(currentTime);
	}
	DWORD partyNumIntraRequestsInInterval = m_PartyIntraRequestsTime->size();
	BOOL prevSuppression = m_IsPartyNoiseSuppressed;
	if(insertNew && !canInsert/*partyNumIntraRequestsInInterval > maxIntraRequestsPerInterval*/)
	{
		m_IsPartyNoiseSuppressed = TRUE;
	}
	else
	{
		if (m_IsPartyNoiseSuppressed && canInsert)
		{
			TRACEINTO << "CContentBridgePartyCntl::CheckIsPartyIntraSuppressed -  intra request suppression canceled (in this interval) from party: " << m_partyConfName;
			m_IsPartyNoiseSuppressed = FALSE;
		}
	}
	if (m_IsPartyNoiseSuppressed)
		TRACEINTO << "CContentBridgePartyCntl::CheckIsPartyIntraSuppressed -  suppressing intra request (in this interval) from party: " << m_partyConfName << " will be suppressed";
	if (prevSuppression != m_IsPartyNoiseSuppressed)
		m_pConfApi->UpdateDB(m_pParty,PARTY_INTRA_SUPPRESS,m_IsPartyNoiseSuppressed);
	return m_IsPartyNoiseSuppressed;
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::UpdateIntraRequestSupressionOnSend()
{
	DWORD contentIntraRequestsSuppressDuration = 10;//default
	DWORD currentTime = SystemGetTickCount().GetSeconds();
	DWORD earliestSuppressedRequestTime = currentTime;
	DWORD_VECTOR::iterator timeItr = m_PartyIntraRequestsTime->begin();
	// scan the relevant requests for earliest time
	while (timeItr != m_PartyIntraRequestsTime->end())
	{
		if (*timeItr < earliestSuppressedRequestTime)
			earliestSuppressedRequestTime = *timeItr;
		timeItr++;
	}
	DWORD timeForNextInterval = contentIntraRequestsSuppressDuration - (currentTime - earliestSuppressedRequestTime);
	StartTimer(CONTENT_BRDG_PARTY_INTRA_SUPPRESS_TOUT, SECOND*timeForNextInterval);

}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnTimerPartyIntraSuppressed(CSegment* pParams)
{
	PTRACE(eLevelInfoNormal,"CContentBridgePartyCntl::OnTimerPartyIntraSuppressed");BOOL prevSuppressionStatus = m_IsPartyNoiseSuppressed;
	if (prevSuppressionStatus)
	{
		if (!CheckIsPartyIntraSuppressed(FALSE))
		{
			m_PartyIntraRequestsTime->clear();
			PTRACE(eLevelInfoNormal,"CContentBridgePartyCntl::OnTimerPartyIntraSuppressed - clearing suppressed requests queue and requesting intra");
			// we initiate an intra request (from the conf api level, as if received from an EP by the ConfPartyManager API) due to the intra suppression
			m_pConfApi->ContentVideoRefresh(1, YES, GetPartyTaskApp());
		}
		else
		{
			// should not occur- the timer is set by checking the sent intras' times in the current interval.
			// so the timer shouldn't end before the 'CheckIsPartyIntraSuppressed' clears the old timestamps and returns false.
			UpdateIntraRequestSupressionOnSend();
		}
	}
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::ResetPartyIntraSuppression()
{
	PTRACE(eLevelInfoNormal,"CContentBridgePartyCntl::ResetPartyIntraSuppression");
	m_PartyIntraRequestsTime->clear();
	DeleteTimer(CONTENT_BRDG_PARTY_INTRA_SUPPRESS_TOUT);
	m_IsPartyNoiseSuppressed = FALSE;
	m_pConfApi->UpdateDB(m_pParty,PARTY_INTRA_SUPPRESS,m_IsPartyNoiseSuppressed);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeForwardTokenMsgToMasterCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeForwardTokenMsgToMasterCONNECT : Name - ",m_partyConfName);
	OnContentBridgeForwardTokenMsgToMaster(pParam);
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeForwardTokenMsgToMasterCONTENT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeForwardTokenMsgToMasterCONTENT : Name - ",m_partyConfName);
	OnContentBridgeForwardTokenMsgToMaster(pParam);

}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeSendRefresh(CSegment* pParam)
{
	WORD ignore_intra_filtering = FALSE;
	if (m_isIntraSupressionEnabled == false)
	{
		ignore_intra_filtering = TRUE;
	}
	m_pPartyApi->SendContentVideoRefresh(ignore_intra_filtering);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeFlowControlReleaseResponse(CSegment* pParam)
{
	BYTE isAck;
	WORD bitRate;
	*pParam >> isAck >> bitRate;
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeFlowControlReleaseResponse : Name - ",m_partyConfName);
	m_pPartyApi->SendH239FlowControlReleaseRes(isAck, bitRate);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeProviderIdentityCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeProviderIdentityCONNECT: Name - ",m_partyConfName);
	OnContentBridgeProviderIdentity(pParam);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeProviderIdentityCONTENT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeProviderIdentityCONTENT : Name - ",m_partyConfName);
	OnContentBridgeProviderIdentity(pParam);
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeProviderIdentityDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeProviderIdentityDISCONNECTING - ignored : Name - ",m_partyConfName);

}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeProviderIdentity(CSegment* pParam)
{
	BYTE	label, dataSize, mcuNum, terminalNum;
	BYTE*	pData = NULL;

	*pParam >> mcuNum
			>> terminalNum
			>> label
			>> dataSize;
	if( dataSize ) {
		pData = new BYTE[dataSize];
		for( WORD i=0; i<dataSize; i++ )
			*pParam >> pData[i];
	}
	m_pPartyApi->SendContentTokenRoleProviderIdentity(mcuNum,terminalNum,label,dataSize,pData);
	PDELETEA(pData);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusCONNECT : Name - ",m_partyConfName);
	OnContentBridgeMediaProducerStatus(pParam);
}


////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusCONTENT(CSegment* pParam)
{
#ifdef CONTENT_LOAD
	TRACESTRFUNC(eLevelError) << "PartyId:" << m_partyRsrcID;
#else
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusCONTENT : Name - ",m_partyConfName);
#endif
	OnContentBridgeMediaProducerStatus(pParam);
}
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusDISCONNECTING - IGNORED: Name - ",m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeMediaProducerStatus(CSegment* pParam)
{
	BYTE	channelId;
	BYTE	status;

	*pParam >> channelId
			>> status;
	m_pPartyApi->SendContentTokenMediaProducerStatus(channelId,status);
}

//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::NoRoleProvider()
{
	// send NS-IND/NoRoleProvider to party task
	DispatchEvent(NO_ROLE_PROVIDER,NULL);
}

//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeNoProviderCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeNoProviderCONNECT : Name - ",m_partyConfName);
	m_pPartyApi->SendContentTokenNoRoleProvider(m_mcuNumber,m_terminalNumber);

}
////////////////////////////////////////////////////////////////////////////

void CContentBridgePartyCntl::OnContentBridgeNoProviderCONTENT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeNoProviderCONTENT - ignored : Name - ",m_partyConfName);
}
//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnContentBridgeNoProviderDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeNoProviderDISCONNECTING - ignored : Name - ",m_partyConfName);
}
//////////////////////////////////////////////////////////////////////////////
bool CContentBridgePartyCntl::IsIntraSuppressEnabled(WORD intra_suppression_type) const
{
	return m_isIntraSupressionEnabled;
}
//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::EnableIntraSuppress(WORD intra_suppression_type)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::EnableIntraSuppress, ",m_partyConfName);
	m_isIntraSupressionEnabled = true;

}
//////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::DisableIntraSuppress(WORD intra_suppression_type)
{
	m_isIntraSupressionEnabled = false;
}
//////////////////////////////////////////////////////////////////////////////


/*COMMENTS

 //#include "BridgePartyDisconnectParams.h"
//#include "BridgePartyExportParams.h"
//#include "AudioHardwareInterface.h"
//#include "OpcodesMcmsAudio.h"
//#include "OpcodesMcmsAudioCntl.h"
//#include "OpcodesMcmsCardMngrIvrCntl.h"
//#include "HostCommonDefinitions.h"
//#include "Bridge.h"

#include  <MCONCNTL.H>
#include  <GOPCODE.H>
#include  <CONFAPI.H>
#include  <PARTYAPI.H>
#include  <CONF.H>
#include  <NSTREAM.H>
#include  <RW\TPORDVEC.H>
#include  <MBRDG.H>
#include  <USTREAM.H>
#include  <RMUX.H>
#include  <TOKEN.H>
#include  <RHDLCSRV.H>

#ifndef _UCAPPP
#include  <ucappp.h>
#endif
*/


//#define  CONTENT_SWITCH_TIME_CP					20    // 0.2*SECOND
//#define  CONTENT_SWITCH_TIME_SW					SECOND
//#define  CONTENT_SWITCH_TIME_FOR_SW_H323		SECOND / 2  // 0.5*SECOND
//#define  CONTENT_TIME_CONNECT_TS				SECOND / 2
//#define  CONTENT_TIME_CONNECT_TS_H323			0
/*
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::UpdateMuxDesc(CMuxRsrcDesc* pMuxDesc)
{
	CSegment* pSeg = new CSegment;
	*pSeg	<< (void*)pMuxDesc;

	DispatchEvent(UPDATE_MUX_DESC,pSeg);
}
*/


//	ONEVENT(CONTDISCONNECT  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeDisConnectInswitch)
//	ONEVENT(CHANGE_PRESENTATION_RATE  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeChangePresentationRateInswitch)
//	ONEVENT(CHANGE_PRESENTATION_SPEAKER  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerInswitch)
//	ONEVENT(SET_PARTY_RATE  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeSetRateInswitch)
//	ONEVENT(SEND_FREEZE   ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeSendFreezeInswitch)
//	ONEVENT(SEND_REFRESH  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeSendRefreshInswitch)
//	ONEVENT(NO_ROLE_PROVIDER  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeNoProviderInswitch)
//	ONEVENT(FLOW_CONTROL_RELEASE_RES  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeFlowControlReleaseResponse)
//  ONEVENT(PROVIDER_IDENTITY  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeProviderIdentityInswitch )
//	ONEVENT(MEDIA_PRODUCER  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusInswitch)
//	ONEVENT(CONTENTREFRESHTOUT  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnTimerContentRefreshInswitch)



//	//   update mux desc
//	ONEVENT(UPDATE_MUX_DESC  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeUpdateMuxConnect )
//	ONEVENT(UPDATE_MUX_DESC  ,ST_INSWITCH  ,CContentBridgePartyCntl::OnContentBridgeUpdateMuxInswitch)
//	ONEVENT(UPDATE_MUX_DESC  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeUpdateMuxContent)


////   send freese to party
//	ONEVENT(SEND_FREEZE   ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeSendFreezeConnect)
//	ONEVENT(SEND_FREEZE   ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeSendFreezeContent)

//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeDisConnectInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeDisConnectInswitch : Name - ",m_partyConfName);
//	OnContentBridgeDisConnect(pParam);
//	m_pPartyApi->SetContentSrcOff();
//}
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeChangePresentationRateInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangePresentationRateInswitch : Name - ",m_partyConfName);
//	//CARMEL NONEED m_byNotificationReceived |= eChangeRate;
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangePresentationSpeakerInswitch : Name - ",m_partyConfName);
//	//CARMEL NONEEDm_byNotificationReceived |= eChangeSpeaker;
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeChangeStateInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeChangeStateInswitch : Name - ",m_partyConfName);
//
//	WORD	newState = IDLE;
//	*pParam	>> newState;
//	DBGPASSERT_AND_RETURN( newState != ST_CONTENT );
//	m_state  = newState;
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeSetRateInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSetRateInswitch : Name - ",m_partyConfName);
//
//	BYTE	newRate = AMSC_0k;
//	*pParam	>> newRate;
//	m_byCurrentContentRate  = newRate;
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeUpdateMuxInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeUpdateMuxInswitch : Name - ",m_partyConfName);
//	OnContentBridgeUpdateMux(pParam);
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeSendFreezeConnect(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendFreezeConnect : Name - ",m_partyConfName);
//	OnContentBridgeSendFreeze(pParam);
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeSendFreezeInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendFreezeInswitch : Name - ",m_partyConfName);
//	OnContentBridgeSendFreeze(pParam);
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeSendFreezeContent(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendFreezeContent : Name - ",m_partyConfName);
//	OnContentBridgeSendFreeze(pParam);
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeSendRefreshInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeSendRefreshInswitch - ignored : Name - ",m_partyConfName);
//	//OnContentBridgeSendRefresh(pParam);
//}

//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeNoProviderInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeNoProviderInswitch (NULL) : Name - ",m_partyConfName);
//}
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeProviderIdentityInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeProviderIdentityInswitch: Name - ",m_partyConfName);
//	OnContentBridgeProviderIdentity(pParam);
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusInswitch(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeMediaProducerStatusInswitch : Name - ",m_partyConfName);
//	OnContentBridgeMediaProducerStatus(pParam);
//}
	//   send NS-IND/NoRoleProvider to party
//CARMEL - EPC Only	ONEVENT(NO_ROLE_PROVIDER  ,ST_CONNECT   ,CContentBridgePartyCntl::OnContentBridgeNoProviderConnect )
//CARMEL - EPC Only	ONEVENT(NO_ROLE_PROVIDER  ,ST_CONTENT   ,CContentBridgePartyCntl::OnContentBridgeNoProviderContent)

	//  CONTENT_REFRESH timer message
//	ONEVENT(CONTENTREFRESHTOUT  ,ST_CONNECT   ,CContentBridgePartyCntl::OnTimerContentRefreshConnect)
//	ONEVENT(CONTENTREFRESHTOUT  ,ST_CONTENT   ,CContentBridgePartyCntl::OnTimerContentRefreshContent )
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::SendFreeze()
//{
//	// send FREEZE to party task
//	DispatchEvent(SEND_FREEZE,NULL);
//}
//

//CARMEL - EPC Only
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::NoRoleProvider()
//{
//	// send NS-IND/NoRoleProvider to party task
//	DispatchEvent(NO_ROLE_PROVIDER,NULL);
//}

/*CARMEL NONEED
/////////////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::SetCascadeMode(WORD cascadeMode)
{
    m_cascadeMode=cascadeMode;

}
*/
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeUpdateMuxConnect(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeUpdateMuxConnect : Name - ",m_partyConfName);
//	OnContentBridgeUpdateMux(pParam);
//}
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeUpdateMuxContent(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeUpdateMuxContent : Name - ",m_partyConfName);
//	OnContentBridgeUpdateMux(pParam);
//}
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeUpdateMux(CSegment* pParam)
//{
//	//CARMEL NO MUX
//	//CARMEL NO MUXCMuxRsrcDesc*	pMuxDesc = NULL;
//	//CARMEL NO MUX*pParam	>> (void*&)pMuxDesc;
//
//	//CARMEL NO MUXif( !::isValidPObjectPtr(pMuxDesc) )
//		//CARMEL NO MUXPASSERT_AND_RETURN(1);
//
//	//CARMEL NO MUXPOBJDELETE(m_pMuxRsrcDes);
//	//CARMEL NO MUXm_pMuxRsrcDes = new CMuxRsrcDesc(*pMuxDesc);
//}
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeSendFreeze(CSegment* pParam)
//{
//	//CARMEL - NO FREEZ NEEDED ??m_pPartyApi->SendEPCContentFreezePicture();
//}
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnTimerContentRefreshConnect(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnTimerContentRefreshConnect : Name - ",m_partyConfName);
//	DBGPASSERT(1);
//}


/*CARMEL - NO INSWITCH
////////////////////////////////////////////////////////////////////////////
void CContentBridgePartyCntl::OnTimerContentRefreshInswitch(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnTimerContentRefreshInswitch : Name - ",m_partyConfName);

	// send to Brdg EndSwitch notification
	m_pTaskApi->PartyContentBrdgEndSwitch(m_pParty,m_pBrdgCntl,statOK);
	m_state = ST_CONTENT;
}
*/
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnTimerContentRefreshContent(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnTimerContentRefreshContent : Name - ",m_partyConfName);
//	DBGPASSERT(1);
//}
//CARMEL - EPC Only
//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::OnContentBridgeNoProviderConnect(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CContentBridgePartyCntl::OnContentBridgeNoProviderConnect : Name - ",m_partyConfName);
//	m_pPartyApi->SendContentMessage(CONTENT_NO_ROLE_PROVIDER,m_mcuNumber,m_terminalNumber);
//}

//////////////////////////////////////////////////////////////////////////////
//void CContentBridgePartyCntl::Dump() const
//{
//
//	//CARMEL TEMP OPENOSTRSTREAM(msg);
//	msg << "CContentBridgePartyCntl::Dump for party: "<< m_partyConfName << "\n";
//
//	msg << "The ContentPartyCntl is in state: ";
//	switch( m_state )
//	{
//		case IDLE : {
//			msg << "IDLE" << "\n";
//			break;
//		}
//		case ST_CONNECT : {
//			msg << "ST_CONNECT" << "\n";
//			break;
//		}
//		case ST_CONTENT : {
//			msg << "ST_CONTENT" << "\n";
//			break;
//		}
//		case ST_INSWITCH : {
//			msg << "ST_INSWITCH" << "\n";
//			break;
//		}
//		default : {
//			msg << "Error in recontion of the state!! (" << m_state << ")\n";
//			break;
//		}
//	}
//	msg << "Current Content Rate: " << m_byCurrentContentRate << "\n";
////	msg << "The SwitchTime is: " << m_switchTime << "\n";
//			// that is not flag, it's a mask !!!!
//	msg << "Notification Mask: "<< (hex) << m_byNotificationReceived << "\n";
//	PTRACE(INTERACTIVE_TRACE|EPC_TRACE,msg.str());
//	CLOSEOSTRSTREAM;*/
//
//}

