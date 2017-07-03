/*
 * SIPSlaveParty.cpp
 *
 *  Created on: Feb 15, 2011
 *      Author: mvolovik
 */
#include "Segment.h"
#include "TaskApi.h"
#include "Party.h"
#include "PartyApi.h"
#include "SIPParty.h"
#include "IPParty.h"
#include "SIPSlaveParty.h"
#include "ConfApi.h"
#include "SIPControl.h"
#include "MSSlaveSipParty.h"
//#include "CommConfDB.h"

void MSSlaveSipPartyEntryPoint(void* appParam)
{
	CMSSlaveSipParty* pPartyTaskApp = new CMSSlaveSipParty;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CMSSlaveSipParty)

/*Almost done */
/*for SlaveIn & SlaveOut */
ONEVENT(ALLOCATE_PARTY_RSRC_IND,			PARTYIDLE,				CMSSlaveSipParty::OnConfInformResourceAllocatedIdle)//Idle Case
/*for SlaveOut */
ONEVENT(SIP_CONF_ESTABLISH_CALL,			PARTYIDLE,				CMSSlaveSipParty::OnConfEstablishCallIdle)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			ANYCASE,				CMSSlaveSipParty::OnPartyChannelsConnected)
/* MSSlave Flora comment: do nothing on ANYCASE while receiving EVENT: AUDBRDGCONNECT */
//noa-need to verify what we do in this case in reglaur party
//Flora-why? for MSSlave, this msg are not suppoesed to be received here 
ONEVENT(AUDBRDGCONNECT,						ANYCASE,				CSipParty::NullActionFunction)
//ONEVENT(AUDBRDGCONNECT,					PARTYCONNECTED,		CSipParty::NullActionFunction)
ONEVENT(VIDBRDGCONNECT,						sPARTY_CONNECTING,		CMSSlaveSipParty::OnConfPartyReceiveVidBridgeConnected)
ONEVENT(SIP_CONF_CONNECT_CALL,				ANYCASE,				CMSSlaveSipParty::OnConfConnectCallRmtConnected)
/* MSSlave Flora comment: I refered TipSlave, PartyCntl send it to Party while all Bridge are disconnected  */

/*  MSSlave-Flora Yao-2013/10/31- MSSlave Flora Comment:  do not overload this msg handing here, since it will call the CSipParty handing interface */
//ONEVENT(CONFDISCONNECT, 					PARTYDISCONNECTING, 	CSipParty::OnConfReadyToCloseCall)
ONEVENT(CONFDISCONNECT, 					ANYCASE,				CMSSlaveSipParty::OnConfCloseCall)


/* MSSlave Flora Question: for TipSlave, this msg is from SlavePartyCntl to SlaveParty, to nitify it to send msg: PARTY_SLAVE_TO_MASTER_RECAP_ACK 
    to MainParty after the end of ChangScm,do you think we need it ? 
    Noa-answer do not need it for MSSlave */
//ONEVENT(SLAVE_END_CHANGE_MODE,				ANYCASE,				CMSSlaveSipParty::OnSlavePartyRecapAck)

//VSR
ONEVENT(SIP_PARTY_SINGLE_VSR_MSG_IND,		ANYCASE, 	CSipParty::OnPartyReceivedAvMcuSingleVsrInd)
ONEVENT(VIDEO_IN_SYNCED, 					ANYCASE,	CMSSlaveSipParty::OnVidBrdgVideoInSyncAnycase)
ONEVENT(VIDREFRESH,							ANYCASE,	CMSSlaveSipParty::OnVidBrdgRefreshAnycase)

//LYNC2013_FEC_RED:
ONEVENT(SIP_PARTY_SINGLE_FEC_RED_MSG,	ANYCASE, 	CSipParty::OnPartyReceivedAvMcuSingleFecOrRedMsg)

/* TBD */
#if 0
//ONEVENT(SIP_PARTY_CHANS_CONNECTED,		sPARTY_OPENOUTCHANNELS,	CSipPartyOut::OnPartyChannelsConnectedOpenOut)// case of internal recovery and external recovery as well
ONEVENT(IPPARTYMONITORINGREQ,				ANYCASE,				CMSSlaveSipParty::OnMcuMngrPartyMonitoringReq)
ONEVENT(SIP_CONF_BRIDGES_UPDATED,			ANYCASE,				CMSSlaveSipParty::OnConfBridgesUpdatedUpdateBridges)
ONEVENT(SIP_TRANS_SLAVE_OPEN_CHANS,			ANYCASE, 				CMSSlaveSipParty::OnSlavePartyOpenChannels)
ONEVENT(SIP_TRANS_SLAVE_CLOSE_CHANS,		ANYCASE, 				CMSSlaveSipParty::OnSlavePartyCloseChannels)
ONEVENT(PLAY_MESSAGE_ACK_SLAVE_TO_MASTER, 	ANYCASE,				CMSSlaveSipParty::OnAckPlayMessage)
ONEVENT(ACK_RECORD_PLAY_MESSAGE,			ANYCASE,				CMSSlaveSipParty::OnAckRecordPlayMessage)
#endif


PEND_MESSAGE_MAP(CMSSlaveSipParty,CSipParty);

/////////////////////////////////////////////////////////////////////////////
CMSSlaveSipParty::CMSSlaveSipParty()
{
	m_pNetSetup			= new CSipNetSetup;
	m_isVideoBridgeConnected = FALSE;
	m_pLastTxVsr = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CMSSlaveSipParty::~CMSSlaveSipParty()
{
	POBJDELETE(m_pNetSetup);


}

///////////////////////////////////////////////////////
void  CMSSlaveSipParty::Create(CSegment& appParam)
{
	CSipParty::Create(appParam);

	m_pConfApi = new CConfApi(m_monitorConfId);
	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx,NULL,NULL,1);

}

///////////////////////////////////////////////////////
void CMSSlaveSipParty::OnConfInformResourceAllocatedIdle(CSegment* pParam)
{// this message is used to initiate ringing in dial out
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnConfInformResourceAllocated. Do nothing wait for conf establish call command: Name ",m_partyConfName);
}

///////////////////////////////////////////////////////
void CMSSlaveSipParty::OnConfEstablishCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnConfEstablishCallIdle: Name ",m_partyConfName);
	BYTE bCapsDontMatch	= NO;

	CRsrcParams* pMrmpRsrcParams = NULL;
    CRsrcParams* pMfaRsrcParams = new CRsrcParams;
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	UdpAddresses sUdpAddressesParams;
	CSipNetSetup*	pNetSetup		= new CSipNetSetup;
	CSipCaps*		pLocalCaps		= new CSipCaps;
	CSipCaps*       pMaxLocalCaps   = new CSipCaps; AUTO_DELETE(pMaxLocalCaps);
	CSipCaps*       pRemoteCaps     = new CSipCaps;
	DWORD			confParamLen	= 0;
	char*			strConfInfo		= NULL;
	BYTE			eTransportType	= 0;
	DWORD			addrLen			= 0;

	pNetSetup->DeSerialize(NATIVE,*pParam);
	pLocalCaps->DeSerialize(NATIVE,*pParam);
	m_pTargetMode->DeSerialize(NATIVE,*pParam);
	*pParam >> m_bIsAdvancedVideoFeatures;

	*pParam >> confParamLen;
	if(confParamLen)
	{
		strConfInfo = new char[confParamLen];
		pParam->Get((BYTE*)strConfInfo,confParamLen);
		strConfInfo[confParamLen-1]=0;
	}

	*pParam >> eTransportType;
	BYTE bIsMrmpExists = FALSE;
	*pParam >> bIsMrmpExists;
	if(bIsMrmpExists)
	{
	  pMrmpRsrcParams = new CRsrcParams;
	  pMrmpRsrcParams->DeSerialize(NATIVE, *pParam);
	}

	pMfaRsrcParams->DeSerialize(NATIVE, *pParam);
	pCsRsrcParams->DeSerialize(NATIVE, *pParam);
	pParam->Get((BYTE *) &sUdpAddressesParams, sizeof(UdpAddresses));

	*pParam >> m_mcuNum;
	*pParam >> m_termNum;

	BYTE bNoVideRsrcForVideoParty = 0;
	*pParam >> bNoVideRsrcForVideoParty;
	*pParam >> m_RoomId;
	DWORD  AvMcuLinkType = 0;
	*pParam >> AvMcuLinkType >> m_MSSlaveIndex;
	m_AvMcuLinkType = (eAvMcuLinkType)AvMcuLinkType;

	/* MSSlave Flora Question: we need a member var: msSlaveDir and msSlaveIndex for MSSlaveParty, in which class we should add it to ? */
	/* in class of : CSipParty or in the CMSSlaveSipParty ? */
	//noa - I think in CMSSlaveSipParty
	//m_tipPartyType = (ETipPartyTypeAndPosition)temp;
	
	TRACEINTO << "AvMcuLinkType:" << eAvMcuLinkTypeNames[m_AvMcuLinkType] << ", MSSlaveIndex:" << m_MSSlaveIndex;

	pRemoteCaps->DeSerialize(NATIVE,*pParam);

	pMaxLocalCaps->DeSerialize(NATIVE,*pParam);
	m_pSipCntl->SetMaxLocalCaps(*pMaxLocalCaps);

	m_PartyRsrcID = pCsRsrcParams->GetPartyRsrcId();
	m_ConfRsrcId = pCsRsrcParams->GetConfRsrcId();
	m_pSipCntl->Create(this, pNetSetup, /*pRsrcDesc,*/ YES, m_serviceId, m_RoomId);//only for dial out
	m_pSipCntl->SetLocalCaps(*pLocalCaps);
	m_pSipCntl->SetRemoteIdent(Microsoft_AV_MCU2013);

	/* MSSlave Flora Question: it is different with CSipPartyOutCreate, Why we need to setRemoteCaps here? and it is the same with LocalCap ? */
	//noa- you are correct but it doesn't matter because as soon main will get te remote caps it will forward it to the slaves -simmilar to tip
	/* Flora: i will remain it here then */
	m_pSipCntl->SetRemoteCaps(*pRemoteCaps);
	//m_pSipCntl->SetMaxLocalCaps(*pMaxLocalCaps);//DPA
	//m_pSipCntl->SetFullContentRate(pLocalCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation));//***ppc can be removed when dpa implemented.
	//m_pSipCntl->SetLocalSdesKeysAndTag(m_pTargetMode,m_pTargetModeMaxAllocation);
	m_pSipCntl->SetConfParamInfo(strConfInfo);
	m_pSipCntl->SetTransportType((enTransportType) eTransportType);	//only for dial out
	m_pSipCntl->SetControllerResource(pMfaRsrcParams, pCsRsrcParams, sUdpAddressesParams);
//	m_pSipCntl->SetInternalControllerResource(NULL, pMrmpRsrcParams);
	m_pSipCntl->AddToRoutingTable();
//	m_pSipCntl->AddToInternalRoutingTable();

	m_pSipCntl->SetIsEnableICE(pNetSetup->GetEnableSipICE());

	m_pSipCntl->SetIsMs2013(eMsft2013AvMCU);
	m_pSipCntl->setMsftStreamOnState(FALSE);

	*m_pNetSetup = *pNetSetup;

	POBJDELETE(pMrmpRsrcParams);
    POBJDELETE(pMfaRsrcParams);
	POBJDELETE(pCsRsrcParams);
	POBJDELETE(pNetSetup);
	POBJDELETE(pLocalCaps);
	POBJDELETE(pRemoteCaps);
	PDELETEA(strConfInfo);

	 if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);

	m_pTargetMode->Dump("Flora degug 2:CMSSlaveSipParty::OnConfEstablishCallIdle - m_pTargetMode is: ",eLevelInfoNormal);
	 

	if (eAvMcuLinkSlaveIn == GetAvMcuLinkType())
	{
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, m_tipPartyType);
	}
	else if (eAvMcuLinkSlaveOut == GetAvMcuLinkType())
	{
		OnPartyInChannelsConnected();
		m_pSipCntl->SetCallConnected();
	}


//	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
//	if (pCommConf)
//	{
//		 CConfParty* pConfParty 	= pCommConf->GetCurrentParty(GetMonitorPartyId());
//		  if( pConfParty && pConfParty->GetIsAVMCUParty())
//		  	  pConfParty->SetCascadeMode(CASCADE_MODE_MASTER);
//	}
//

	m_state = sPARTY_CONNECTING;
	//StartTransaction(kSipTransInviteWithSdpReq, SIP_PARTY_ESTABLISH_CALL);
	 //just for simulation ?
	//CIpComMode* pIpComMode = new CIpComMode;
	//*pIpComMode = *m_pTargetMode;
	//pIpComMode->SetMediaOff(cmCapAudio, cmCapTransmit);
	//pIpComMode->SetMediaOff(cmCapVideo, cmCapTransmit);
	//OnPartyChannelsConnected(pIpComMode);// only in channels connected before the invite in dial out.
	//POBJDELETE(pIpComMode);

}


///////////////////////////////////////////////////////
/*  MSSlave-Flora Yao-2013/10/23- MSSlave Flora Comment: send SIP_PARTY_CHANS_CONNECTED directly to MSSlavePartyCntlAdd */
void CMSSlaveSipParty::OnPartyInChannelsConnected()
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::OnPartyChannelsConnected: Name ",m_partyConfName);
	*m_pCurrentMode = *m_pTargetMode;

	m_pCurrentMode->Dump("Flora degug 3:CMSSlaveSipParty::OnPartyInChannelsConnected - m_pCurrentMode is: ",eLevelInfoNormal);
	// nissim
	unsigned int dummy[2];
    dummy[0]=0;
    dummy[1]=0;
	m_pConfApi->SipPartyChannelsConnected(GetPartyId(),m_pCurrentMode,dummy);
	// nissim
	UpdateDbOnChannelsConnected();
}

/* MSSlave Flora comment: overloading, it is different with Master SipParty, it send directly to the MSPartyCntl: SIP_PARTY_CHANS_CONNECTED */
void CMSSlaveSipParty::OnPartyChannelsConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnPartyChannelsConnected: Name ",m_partyConfName);

	//m_pConfApi->SipPartyChannelsConnected(this,pIpComMode);
	EConfType eConfType = m_pTargetMode->GetConfType();
	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pCurrentMode->SetConfType(eConfType);
    // nissim
    unsigned int dummy[2];
    dummy[0]=0;
    dummy[1]=0;
 	m_pConfApi->SipPartyChannelsConnected(GetPartyId(), m_pCurrentMode, dummy);
    // nissim
	UpdateDbOnChannelsConnected();

	//update encryption
	WORD encryptStatus = (m_pCurrentMode->GetIsEncrypted() == Encryp_On || m_pCurrentMode->GetIsDtlsAvailable())?YES:NO;
	m_pConfApi->UpdateDB(this, PARTY_ENCRYPTION_STATE, encryptStatus, 1);
	//StartIvr();

	//StartIvr();
//	UpdateDbOnChannelsConnected(pParam, TRUE);
}

/////////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment: do not need it any more for MSSlaveParty */
#if 0
void CMSSlaveSipParty::OnConfPartyReceiveAudBridgeConnected()
{
	ON(m_isAudioBridgeConnected);
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnConfPartyReceiveAudBridgeConnected: Name ",m_partyConfName);

	OnPartyReceivedAck(STATUS_OK);// report to the party control on party connected, change m_state to connected.
}
#endif

/////////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment: oveloading it , different handling with */
void CMSSlaveSipParty::OnConfPartyReceiveVidBridgeConnected()
{
	ON(m_isVideoBridgeConnected);
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnConfPartyReceiveVidBridgeConnected: Name ",m_partyConfName);

	OnPartyReceivedAck(STATUS_OK);// report to the party control on party connected, change m_state to connected.
}

/////////////////////////////////////////////////////////////////////////////////
void CMSSlaveSipParty::OnPartyReceivedAck(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnPartyReceivedAck: Name ",m_partyConfName);

	if (status == STATUS_OK)
	{
		if(m_isVideoBridgeConnected)
		{
	//		m_eDialState = kNotInDialState;
			m_state		 = IP_CONNECTED;

			BYTE bIsEndAddParty = NO;
			m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pTargetMode, FALSE);

			/* MSSlave Flora Question here: for code below, i refered to the TipSlaveParty, i do not understand why we need these dummy IP Info here ? */
			//noa - not sure maybe try to run a tip call an check why it gets here..
			
			/*  MSSlave-Flora Yao-2013/10/23- MSSlave Flora Comment: */
			/* I think code below is for Monitoring, but all the data is faked, so i think we need to change it later. */
			/* In the MainParty, the Montiroing data is updated in the function: CSipParty::OnTransRemoteCapsReceivedAnycase, so we need to update the monitoring data? */
			/* But i can not understand here, we did not update any of the localIP and remoteIp , why we need to call SetPartyMonitorBaseParamsAndConnectChannel to set monitoring data? */
			mcTransportAddress localIp;

			mcTransportAddress remoteIp;

			memset(&localIp,0,sizeof(mcTransportAddress));
			memset(&remoteIp,0,sizeof(mcTransportAddress));
			if (m_pNetSetup->GetIpVersion() == eIpVersion4)
			{
				localIp.addr.v4.ip	= 0x0105aabb;
				remoteIp.addr.v4.ip		= 0x0105ccdd;
			}
			else
			{
				::stringToIp(&localIp,"2001:db8:0:1:213:21ff:feae:4aab");
				::stringToIp(&remoteIp,"2001:db8:0:1:213:21ff:fe11:4bbc");
			}

			localIp.port  = 5060;//m_pSipCntl->GetLocalMediaIp();
			remoteIp.port = 5060;//m_pSipCntl->GetRemoteMediaIp(NO); // take the audio ip as default
			DWORD actualRate = 0xFFFFFFFF;
			SetPartyMonitorBaseParamsAndConnectChannel(SIGNALING,actualRate,&remoteIp,&localIp);
	//		UpdateDbOnSipPrivateExtension();
	//		StartIvr();
		}
	}
	else
	{
//		m_eDialState = kBadStatusAckArrived;
		DBGPASSERT(status);
		PTRACE2INT(eLevelInfoNormal,"CMSSlaveSipParty::OnPartyReceivedAck: Ack with bad status %d",status);
		TellConfOnDisconnecting(status, NULL);
	}
}

///////////////////////////////////////////////////////////////////////////////////
void CMSSlaveSipParty::OnConfConnectCallRmtConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnConfConnectCallRmtConnected: Name ",m_partyConfName);
	//m_state = sPARTY_OPENOUTCHANNELS;
	OpenOutChannels();
}
///////////////////////////////////////////////////////////////////////
void CMSSlaveSipParty::OpenOutChannels()
{
	//only open out video people channels
	if ( (GetAvMcuLinkType() == eAvMcuLinkSlaveOut) && m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit) == NO)
	{
		
		CMedString msg1;
		msg1 << "m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit) =" << m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive) 
			<< ", m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit) = " << m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit);
		PTRACE2(eLevelInfoNormal, "Flora debug CMSSlaveSipParty::OpenOutChannels: ", msg1.GetString());
		m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode,cmCapTransmit, FALSE);
	}
	else
		DBGPASSERT(1);// if we can't open out channels (at least audio) its a mistake.
}

///////////////////////////////////////////////////////
/* MSSlave Flora comment:The same with the Key code of the CSipPartyOutCreate::OnConfCloseCall */
void CMSSlaveSipParty::OnConfCloseCall(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnConfCloseCall: Name ",m_partyConfName);
	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);

	m_state = PARTYDISCONNECTING;
	LogicalChannelDisconnect(SDP);

	CleanUp();
}

////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment:the Key code of the CSipPartyOutCreate::CleanUp() */
void CMSSlaveSipParty::CleanUp()
{
	CSmallString str;
	str << "Name " << m_partyConfName << " Dial state " << m_eDialState;
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::CleanUp: ", str.GetString());

	//m_pSipCntl->CloseCall(YES);
	m_pSipCntl->ViolentCloseCall();
}

////////////////////////////////////////////////////////////////////////////
/* MSSlave Flora comment: the key code of CSipParty::OnConfReadyToCloseCall(for Main CSipPartyOutCreate ) */
void CMSSlaveSipParty::OnConfReadyToCloseCall(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnConfReadyToCloseCall: Name ",m_partyConfName);

	// remove disconnecting timer
	if (IsValidTimer(CONFDISCONNECTOUT))
		DeleteTimer(CONFDISCONNECTOUT);

	CleanUp();
}


/////////////////////////////////////////////////
void  CMSSlaveSipParty::HandleMsftVsrOnEndTransaction(bool isNeedToSendReinvite,ESipTransactionType endedTransType)
{
	PTRACE(eLevelInfoNormal,"CMSSlaveSipParty::HandleMsftVsrOnEndTransaction");

	if (endedTransType == kSipTransRTCPVsrInd)
	{
		// if there is a saved VSR then start a transaction using this new VSR.
		//if (m_bMsftSlaveRecevVsrInActiveTrans)
		if (m_pLastTxVsr)
		{
			TRACEINTO<<"Resend VsrMsgInd";
			CSegment* pParam = new CSegment;
			pParam->Put(reinterpret_cast<const BYTE*>(m_pLastTxVsr), sizeof(ST_VSR_SINGLE_STREAM));

			ESipTransactionType eSipTransType = kSipTransRTCPVsrInd;
			StartTransaction(eSipTransType, SIP_TRANS_VSR_MSG_IND, pParam);
			POBJDELETE(m_pLastTxVsr);
			m_pLastTxVsr = NULL;
		}
	}
}


//TBD
/*  MSSlave-Flora Yao-2013/10/28- MSSlave Flora Comment: Complete it next stage */
////////////////////////////////////////////////////////////////////////
void CMSSlaveSipParty::OnPartyBadStatusConnecting(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnPartyBadStatusConnecting: Name ",m_partyConfName);
	CSipParty::OnPartyBadStatus(pParam);
}
////////////////////////////////////////////////////////////////////////
void CMSSlaveSipParty::ForwardRemoteH230ToMsSlavesControllerIfNeeded(CSegment* pParam)
{
	TRACEINTO << " do noting for MS slave party";
}
////////////////////////////////////////////////////////////////////////
void CMSSlaveSipParty::OnVidBrdgVideoInSyncAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnVidBrdgVideoInSyncAnycase: Name ",m_partyConfName);
	
	CSegment*  pSeg = new CSegment;
	
	*pSeg << (DWORD)GetMSSlavePartyIndex();
	if (pParam)
	{
		*pSeg << *pParam;
	}
	PTRACE2INT(eLevelInfoNormal,"CMSSlaveSipParty::OnVidBrdgVideoInSyncAnycase:  GetMSSlavePartyIndex() = ",GetMSSlavePartyIndex());

	SendMessageFromMSSlaveToMain(MS_SLAVE_VIDEO_IN_SYNCED, pSeg);
	POBJDELETE(pSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CMSSlaveSipParty::OnVidBrdgRefreshAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnVidBrdgRefreshAnycase: Name ",m_partyConfName);

	CSegment*  pSeg = new CSegment;
	*pSeg << (DWORD)GetMSSlavePartyIndex();

	WORD ignore_filtering = FALSE;
	DWORD remoteSSRC = 0;
	DWORD priorityID = 0;
	
	if(pParam != NULL)
	{
	  *pParam >> ignore_filtering >> remoteSSRC >>  priorityID;
	}

	*pSeg << ignore_filtering;
	*pSeg << remoteSSRC;
	*pSeg << priorityID;
	

	CapEnum protocol = eUnknownAlgorithemCapCode;
	if (m_pCurrentMode && m_pCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))
		protocol = (CapEnum)m_pCurrentMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople);

	DWORD isRTV = (protocol == eRtvCapCode) ? TRUE : FALSE;
	*pSeg << isRTV;

	std::ostringstream msg;
	msg
		<< "\n	GetMSSlavePartyIndex()   :" << GetMSSlavePartyIndex()
		<< "\n	ignore_filtering		 :" << ignore_filtering
		<< "\n	remoteSSRC			  	 :" << remoteSSRC
		<< "\n	priorityID			     :" << priorityID
		<< "\n  isRTV					 :" << isRTV;

	TRACEINTO << msg.str().c_str();

	SendMessageFromMSSlaveToMain(MS_SLAVE_VIDREFRESH, pSeg);
	POBJDELETE(pSeg);
}
#if 0
///////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::LogicalChannelDisconnect(DWORD eChannelType)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << eChannelType;
	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELDISCONNECT,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
/*void CSipSlaveParty::TellConfOnDisconnecting(int reason,const char* alternativeAddrStr)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::TellConfOnDisconnecting: Name ",m_partyConfName);
	m_disconnectCause = reason;
	m_state = IP_DISCONNECTING;
	LogicalChannelDisconnect(SDP);
	m_pConfApi->PartyDisConnect(reason,this,alternativeAddrStr);
	m_pConfApi->UpdateDB(this,DISCAUSE,reason,1);
}*/

////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnPartyBadStatusConnecting(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CMSSlaveSipParty::OnPartyBadStatusConnecting: Name ",m_partyConfName);
	CSipParty::OnPartyBadStatus(pParam);
}


/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::SendFastUpdateReq(ERoleLabel eRole)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << (DWORD)eRole;

	SendMessageFromSlaveToMaster(m_tipPartyType, SLAVE_SEND_RTCP_FAST_UPDATE, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnMcuMngrPartyMonitoringReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnMcuMngrPartyMonitoringReq : Name = ",m_partyConfName);

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	APIU32 RefreshPeriodForPMinMiliSeconds = 0;
	std::string key = "PARTY_MONITORING_REFRESH_PERIOD";
	pSysConfig->GetDWORDDataByKey(key, RefreshPeriodForPMinMiliSeconds);
	RefreshPeriodForPMinMiliSeconds = RefreshPeriodForPMinMiliSeconds * 1000; // Convert to mili seconds
	if (RefreshPeriodForPMinMiliSeconds < 500)
		RefreshPeriodForPMinMiliSeconds = 500; // Minimum monitoring interval is 500 mili seconds.
	TICKS		curTicks;
	CTaskApp	*pTaskApp;

	*pParam >> (void *&)pTaskApp;

    //PartyMonitoring requests will be sent to a party
    //in a minimum interval at least 0.5 second
	curTicks = SystemGetTickCount();
    TICKS diff;
    diff = curTicks - m_lastPmTicks;

    //GetIntegerPartForTrace return value is 1/100 of a second
    if (diff.GetIntegerPartForTrace() * 10 >= RefreshPeriodForPMinMiliSeconds)
	{
		if (m_pSipCntl->CheckPartyMonitoringReq())
		{
			PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnMcuMngrPartyMonitoringReq send request to Master from slave ", eTIPPartyTypeNames[m_tipPartyType]);
			SendMonitoringReq();
		}
		else
			PTRACE(eLevelInfoNormal,"CSipSlaveParty::OnMcuMngrPartyMonitoringReq - No request was sent - Party is not connected yet");
		m_lastPmTicks = curTicks;
    }
	else
	{
		CMedString msg;
		msg << "Do not send PartyMonitor - less then = " << RefreshPeriodForPMinMiliSeconds << " miliseconds\n";
		PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnMcuMngrPartyMonitoringReq: ", msg.GetString());
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::SendMonitoringReq()
{
	PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::SendMonitoringReq: ", m_tipPartyType);

	SendMessageFromSlaveToMaster(m_tipPartyType, SLAVE_SEND_MONITORING_REQ, NULL);
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnConfBridgesUpdatedUpdateBridges()
{
	PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::OnConfBridgesUpdatedUpdateBridges - slave party type: ", m_tipPartyType);

	BYTE bIsEndConfChangeMode = TRUE;

	m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pCurrentMode, bIsEndConfChangeMode);// inform Party control of end change mode.
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnSlavePartyRecapAck()
{
	PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::OnSlavePartyRecapAck - slave party type: ", m_tipPartyType);

	SendMessageFromSlaveToMaster(m_tipPartyType, PARTY_SLAVE_TO_MASTER_RECAP_ACK, NULL);
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnSlavePartyCloseChannels(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::OnSlavePartyCloseChannels - slave party type: ", m_tipPartyType);

	DWORD channelMask = 0;

	*pParam >> channelMask;


	if( channelMask & 0x0001 )
		m_pSipCntl->SipCloseChannelReq(AUDIO_IN);
	if( channelMask & 0x0002 )
		m_pSipCntl->SipCloseChannelReq(AUDIO_OUT);
	if( GetIsVoice() )
		return;

	if( channelMask & 0x0004 )
		m_pSipCntl->SipCloseChannelReq(VIDEO_IN);
	if( channelMask & 0x0008 )
		m_pSipCntl->SipCloseChannelReq(VIDEO_OUT);

	//will open if we will ever open FECC in TIP slave
	//if( channelMask & 0x0040 )
	//	m_pSipCntl->SipCloseChannelReq(FECC_IN);
	//if( channelMask & 0x0080 )
	//	m_pSipCntl->SipCloseChannelReq(FECC_OUT);



}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnSlavePartyOpenChannels(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::OnSlavePartyOpenChannels - slave party type: ", m_tipPartyType);

	DWORD eDirection;
	*pParam >> eDirection;
	m_pTargetMode->DeSerialize(NATIVE,*pParam);

	m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode, (cmCapDirection)eDirection, FALSE, m_tipPartyType);

}

/////////////////////////////////////////////////////////////////////////////

void CSipSlaveParty::OnAckPlayMessage(CSegment* pSeg)
{
	TRACEINTO;
	SendMessageFromSlaveToMaster(m_tipPartyType, PLAY_MESSAGE_ACK_SLAVE_TO_MASTER, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnAckRecordPlayMessage(CSegment* pSeg)
{
	TRACEINTO;
	SendMessageFromSlaveToMaster(m_tipPartyType, RECORD_PLAY_MESSAGE_ACK_SLAVE_TO_MASTER, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::UpdateTipParamForIvr()
{
	m_bIsTipCall = TRUE;
	m_tipPartyType = (ETipPartyTypeAndPosition)2; //salves and not master or none (we will check >1)
}
#endif

/////////////////////////////////////////////////////////////////////////////

