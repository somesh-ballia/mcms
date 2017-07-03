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
#include "IVRCntl.h"
#include "ConfApi.h"
#include "SIPControl.h"

void SipSlavePartyEntryPoint(void* appParam)
{
	CSipSlaveParty* pPartyTaskApp = new CSipSlaveParty;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CSipSlaveParty)

ONEVENT(ALLOCATE_PARTY_RSRC_IND,			PARTYIDLE,				CSipSlaveParty::OnConfInformResourceAllocatedIdle)//Idle Case
ONEVENT(SIP_CONF_ESTABLISH_CALL,			PARTYIDLE,				CSipSlaveParty::OnConfEstablishCallIdle)
ONEVENT(AUDBRDGCONNECT,						sPARTY_CONNECTING,		CSipSlaveParty::OnConfPartyReceiveAudBridgeConnected)
ONEVENT(AUDBRDGCONNECT,						PARTYCONNECTED,			CSipParty::NullActionFunction)
ONEVENT(VIDBRDGCONNECT,						sPARTY_CONNECTING,		CSipSlaveParty::OnConfPartyReceiveVidBridgeConnected)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			ANYCASE,				CSipSlaveParty::OnPartyChannelsConnected)
ONEVENT(SIP_CONF_CONNECT_CALL,				ANYCASE,				CSipSlaveParty::OnConfConnectCallRmtConnected)
//ONEVENT(SIP_PARTY_CHANS_CONNECTED,		sPARTY_OPENOUTCHANNELS,	CSipPartyOut::OnPartyChannelsConnectedOpenOut)// case of internal recovery and external recovery as well
ONEVENT(CONFDISCONNECT,						PARTYDISCONNECTING,		CSipParty::OnConfReadyToCloseCall)
ONEVENT(CONFDISCONNECT,						ANYCASE,				CSipSlaveParty::OnConfCloseCall)
ONEVENT(IPPARTYMONITORINGREQ,				ANYCASE,				CSipSlaveParty::OnMcuMngrPartyMonitoringReq)
ONEVENT(SIP_CONF_BRIDGES_UPDATED,			ANYCASE,				CSipSlaveParty::OnConfBridgesUpdatedUpdateBridges)
ONEVENT(SLAVE_END_CHANGE_MODE,				ANYCASE, 				CSipSlaveParty::OnSlavePartyRecapAck)
ONEVENT(SIP_TRANS_SLAVE_OPEN_CHANS,			ANYCASE, 				CSipSlaveParty::OnSlavePartyOpenChannels)
ONEVENT(SIP_TRANS_SLAVE_CLOSE_CHANS,		ANYCASE, 				CSipSlaveParty::OnSlavePartyCloseChannels)
ONEVENT(PLAY_MESSAGE_ACK_SLAVE_TO_MASTER, 	ANYCASE,				CSipSlaveParty::OnAckPlayMessage)
ONEVENT(ACK_RECORD_PLAY_MESSAGE,			ANYCASE,				CSipSlaveParty::OnAckRecordPlayMessage)



PEND_MESSAGE_MAP(CSipSlaveParty,CSipParty);

/////////////////////////////////////////////////////////////////////////////
CSipSlaveParty::CSipSlaveParty()
{
	m_pNetSetup			= new CSipNetSetup;
	m_isAudioBridgeConnected = FALSE;
	m_isVideoBridgeConnected = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CSipSlaveParty::~CSipSlaveParty()
{
	POBJDELETE(m_pNetSetup);

}

///////////////////////////////////////////////////////
void  CSipSlaveParty::Create(CSegment& appParam)
{
	CSipParty::Create(appParam);

	m_pConfApi = new CConfApi(m_monitorConfId);
	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx,NULL,NULL,1);

}

///////////////////////////////////////////////////////
//void CSipSlaveParty::CleanUp()
//{
//	DestroyPartyTask();
//}

///////////////////////////////////////////////////////
void CSipSlaveParty::OnConfInformResourceAllocatedIdle(CSegment* pParam)
{// this message is used to initiate ringing in dial out
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfInformResourceAllocated. Do nothing wait for conf establish call command: Name ",m_partyConfName);
}

///////////////////////////////////////////////////////
void CSipSlaveParty::OnConfCloseCall(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfCloseCall: Name ",m_partyConfName);
	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);

	m_state = PARTYDISCONNECTING;
	LogicalChannelDisconnect(SDP);

	CleanUp();
}


///////////////////////////////////////////////////////
void CSipSlaveParty::OnConfEstablishCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfEstablishCallIdle: Name ",m_partyConfName);
	BYTE bCapsDontMatch	= NO;

	CRsrcParams* pMrmpRsrcParams = NULL;
    CRsrcParams* pMfaRsrcParams = new CRsrcParams;
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	UdpAddresses sUdpAddressesParams;
	CSipNetSetup*	pNetSetup		= new CSipNetSetup;
	CSipCaps*		pLocalCaps		= new CSipCaps;
//	CSipCaps*       pMaxLocalCaps   = new CSipCaps;
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
	DWORD  temp = 0;
	*pParam >> temp;
	m_tipPartyType = (ETipPartyTypeAndPosition)temp;

	PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::OnConfEstablishCallIdle: m_tipPartyType ",m_tipPartyType);

	m_PartyRsrcID = pCsRsrcParams->GetPartyRsrcId();
	m_ConfRsrcId = pCsRsrcParams->GetConfRsrcId();
	m_pSipCntl->Create(this, pNetSetup, /*pRsrcDesc,*/ YES, m_serviceId, m_RoomId);//only for dial out
	m_pSipCntl->SetLocalCaps(*pLocalCaps);
	m_pSipCntl->SetRemoteCaps(*pLocalCaps);
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
	*m_pNetSetup = *pNetSetup;

	POBJDELETE(pMrmpRsrcParams);
    POBJDELETE(pMfaRsrcParams);
	POBJDELETE(pCsRsrcParams);
	POBJDELETE(pNetSetup);
	POBJDELETE(pLocalCaps);
	PDELETEA(strConfInfo);

	 if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);

	m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, m_tipPartyType);
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

void CSipSlaveParty::OnPartyChannelsConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnPartyChannelsConnected: Name ",m_partyConfName);

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
//	UpdateDbOnChannelsConnected(pParam, TRUE);
}

//void CSipSlaveParty::UpdateDbChannelsStatus(CSegment* pParam, BYTE bIsConnected /* TRUE or FALSE = update connected or disconnected channels*/)
//{
//	EConfType eConfType = m_pTargetMode->GetConfType();
//	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
//	m_pCurrentMode->SetConfType(eConfType);
//	//DWORD contentRateScm = m_pTargetMode->GetContentBitRate(cmCapReceive);
//	//m_pCurrentMode->SetContentBitRate(contentRateScm, cmCapReceiveAndTransmit);
//	//SendUpdateDbChannelsStatusToParty(bIsConnected);
//}
/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::UpdateDbOnChannelsConnected()
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::UpdateDbOnChannelsConnected: Name ",m_partyConfName);


	// IpV6 - Monitoring
	mcTransportAddress localIp;
	memset(&localIp,0,sizeof(mcTransportAddress));
	mcTransportAddress remoteIp;
	memset(&remoteIp,0,sizeof(mcTransportAddress));
	if (m_pNetSetup->GetIpVersion() == eIpVersion4)
		localIp.addr.v4.ip = 0x0;
	else
		::stringToIp(&localIp,"0:0:0:0:0:0:0:0");

	cmCapDataType  mediaArr[] = {cmCapAudio,cmCapVideo};
	cmCapDirection directionArr[] = {cmCapReceive,cmCapTransmit};
	BYTE bIsTransmit;
	EIpChannelType eChanType;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i=0 ; i<MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		if (m_pNetSetup->GetIpVersion() == eIpVersion4)
			remoteIp.addr.v4.ip = 0x0;
		else
			::stringToIp(&remoteIp,"0:0:0:0:0:0:0:0");

		for (int j=0; j<2; j++)
		{
			bIsTransmit = (directionArr[j] == cmCapTransmit);
			eChanType   = ::CalcChannelType(mediaType,bIsTransmit,eRole);
			if (m_pTargetMode->IsMediaOn(mediaType,directionArr[j],eRole))
			{
				DWORD actualRate	=0xFFFFFFFF;
				localIp.port			= 9000;//m_pSipCntl->GetPort(mediaArr[i],cmCapReceive);
				remoteIp.port			= 9500;//m_pSipCntl->GetPort(mediaArr[i],cmCapTransmit);
				SetPartyMonitorBaseParamsAndConnectChannel(eChanType,actualRate,&remoteIp,&localIp);
			}
		}
	}

	UpdateDbScm();
}


////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::UpdateDbScm()
{
	CSegment localScmSeg;
	CSegment remoteScmSeg;

	m_pTargetMode->Serialize(localScmSeg,cmCapTransmit,YES);
	m_pConfApi->UpdateDB(this,LOCAL323COMMODE,0,1,&localScmSeg);
	m_pTargetMode->Serialize(remoteScmSeg,cmCapReceive,YES);
	m_pConfApi->UpdateDB(this,RMOT323COMMODE,0,1,&remoteScmSeg);
}


/////////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnConfPartyReceiveAudBridgeConnected()
{
	ON(m_isAudioBridgeConnected);
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfPartyReceiveAudBridgeConnected: Name ",m_partyConfName);

	OnPartyReceivedAck(STATUS_OK);// report to the party control on party connected, change m_state to connected.
}

/////////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnConfPartyReceiveVidBridgeConnected()
{
	ON(m_isVideoBridgeConnected);
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfPartyReceiveVidBridgeConnected: Name ",m_partyConfName);

	OnPartyReceivedAck(STATUS_OK);// report to the party control on party connected, change m_state to connected.
}

/////////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnPartyReceivedAck(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnPartyReceivedAck: Name ",m_partyConfName);

	if (status == STATUS_OK)
	{
		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected) ||
			(m_isAudioBridgeConnected && GetIsVoice()))
		{
	//		m_eDialState = kNotInDialState;
			m_state		 = IP_CONNECTED;

			BYTE bIsEndAddParty = NO;
			m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pTargetMode, FALSE);
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
				localIp.ipVersion = eIpVersion6;
				remoteIp.ipVersion = eIpVersion6;
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
		PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::OnPartyReceivedAck: Ack with bad status %d",status);
		TellConfOnDisconnecting(status, NULL);
	}
}



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
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::TellConfOnDisconnecting: Name ",m_partyConfName);
	m_disconnectCause = reason;
	m_state = IP_DISCONNECTING;
	LogicalChannelDisconnect(SDP);
	m_pConfApi->PartyDisConnect(reason,this,alternativeAddrStr);
	m_pConfApi->UpdateDB(this,DISCAUSE,reason,1);
}*/

////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnPartyBadStatusConnecting(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnPartyBadStatusConnecting: Name ",m_partyConfName);
	CSipParty::OnPartyBadStatus(pParam);
}

/*///////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnConfPartyReceiveAudBridgeConnected()
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfPartyReceiveAudBridgeConnected: Name ",m_partyConfName);
	ON(m_isAudioBridgeConnected);
	HandleBridgeConnectedInd(STATUS_OK);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnConfPartyReceiveVidBridgeConnected()
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfPartyReceiveVidBridgeConnected: Name ",m_partyConfName);
	ON(m_isVideoBridgeConnected);
	HandleBridgeConnectedInd(STATUS_OK);
}*/

/////////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::HandleBridgeConnectedInd(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::HandleBridgeConnectedInd: Name ",m_partyConfName);

	if (status == STATUS_OK)
	{
		// its OK if audio, video and FECC or audio and video without FECC bridges connected or its audio only call or there is no much in the video capability
		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected ) ||
			(m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_pTargetMode->IsMediaOff(cmCapData,cmCapReceiveAndTransmit)) ||
			(m_isAudioBridgeConnected && GetIsVoice()) ||
			(m_isAudioBridgeConnected && m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit)))
		{
			if (IsValidTimer(OPENBRIDGESTOUT))
			{
				DeleteTimer(OPENBRIDGESTOUT);
				PTRACE(eLevelInfoNormal,"CSipSlaveParty::HandleBridgeConnectedInd: DeleteTimer(OPENBRIDGESTOUT) ");
			}

		}
		else if(m_isAudioBridgeConnected)
			PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::HandleBridgeConnectedInd: Is Voice Call? - ",GetIsVoice());
	}
	else
	{
		DBGPASSERT(status);
		PTRACE2INT(eLevelInfoNormal,"CSipSlaveParty::HandleBridgeConnectedInd: Ack with bad status - ",status);
	}


}

///////////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnConfConnectCallRmtConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfConnectCallRmtConnected: Name ",m_partyConfName);
	//m_state = sPARTY_OPENOUTCHANNELS;
	OpenOutChannels();
}

///////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OpenOutChannels()
{
	//open out channels
	if (m_pSipCntl->IsMedia(cmCapAudio,cmCapTransmit) == NO ||
		(m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive) && m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit) == NO) ||
		(m_pSipCntl->IsMedia(cmCapData,cmCapReceive) && m_pSipCntl->IsMedia(cmCapData,cmCapTransmit) == NO))
		m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode,cmCapTransmit, FALSE, m_tipPartyType);
	else
		DBGPASSERT(1);// if we can't open out channels (at least audio) its a mistake.
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::SendFastUpdateReq(ERoleLabel eRole, DWORD remoteSSRC, DWORD priorityID, DWORD msSlavePartyIndex)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << (DWORD)eRole;
//	*pSeg << (DWORD)remoteSSRC;
//	*pSeg << (DWORD)priorityID;

	SendMessageFromSlaveToMaster(m_tipPartyType, SLAVE_SEND_RTCP_FAST_UPDATE, pSeg);
	POBJDELETE(pSeg);
}


////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::CleanUp()
{
	CSmallString str;
	str << "Name " << m_partyConfName << " Dial state " << m_eDialState;
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::CleanUp: ", str.GetString());

	//m_pSipCntl->CloseCall(YES);
	m_pSipCntl->ViolentCloseCall();
}
////////////////////////////////////////////////////////////////////////////
void CSipSlaveParty::OnConfReadyToCloseCall(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipSlaveParty::OnConfReadyToCloseCall: Name ",m_partyConfName);

	// remove disconnecting timer
	if (IsValidTimer(CONFDISCONNECTOUT))
		DeleteTimer(CONFDISCONNECTOUT);

	CleanUp();
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
	TRACEINTO << "PLAY_MESSAGE_ACK_SLAVE_TO_MASTER";
				m_ivrCtrl->RecivedPlayMessageAck();	// now handled by ivrcntl
	//SendMessageFromSlaveToMaster(m_tipPartyType, EXTERNAL_IVR_PLAY_MEDIA_COMPLETE_SLAVE_TO_MASTER, pSeg);
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
/////////////////////////////////////////////////////////////////////////////
