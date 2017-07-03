//+========================================================================+
//                            SIPPartyInSimulation.cpp                                 |
//            Copyright 1995 POLYCOM Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyInSimulation.cpp                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#include "Segment.h"
#include "StateMachine.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "Trace.h"
#include "Macros.h"
#include "NStream.h"
#include "NetSetup.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "Conf.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "TaskApi.h"
#include "Party.h"
#include "PartyApi.h"
#include "SipDefinitions.h"
#include "SIPCommon.h"
#include "IpNetSetup.h"
#include "SipNetSetup.h"
#include "CsInterface.h"
#include "SipScm.h"
#include "SipCall.h"
#include "ConfApi.h"
#include "IPParty.h"
#include "SIPControl.h"
#include "Lobby.h"
#include "SIPParty.h"
#include "SIPPartyInSimulation.h"


#ifndef MCMS
#define MCMS 5
#endif


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void SipPartyInSimulationEntryPoint(void* appParam)
{  
  CSipPartyInSimulation* pPartyTaskApp = new CSipPartyInSimulation;
  pPartyTaskApp->Create(*(CSegment*) appParam);
}


////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipPartyInSimulation)
// conf/lobby events
ONEVENT(LOBBYNETIDENT,					IP_DISCONNECTED,	CSipPartyInSimulation::OnLobbyIdent) 
ONEVENT(LOBBYTRANS,						IP_CONNECTING  ,	CSipPartyInSimulation::OnLobbyTransfer) 
ONEVENT(SIP_CONF_ESTABLISH_CALL,		IP_CONNECTING,		CSipPartyInSimulation::OnConfEstablishCall)   
ONEVENT(REJECTCALL,						IP_DISCONNECTED,	CSipPartyInSimulation::OnLobbyReject) 

ONEVENT(CONFDISCONNECT,					IP_DISCONNECTING,	CSipPartyInSimulation::OnConfCloseConnectingCall)
ONEVENT(CONFDISCONNECT,					IP_CONNECTED,		CSipPartyInSimulation::OnConfCloseConnectingCall)
ONEVENT(CONFDISCONNECT,					IP_CONNECTING,		CSipPartyInSimulation::OnConfCloseConnectingCall)
  
ONEVENT(AUDBRDGCONNECT,					IP_CONNECTING,		CSipPartyInSimulation::OnConfPartyReceiveAudBridgeConnected)  
ONEVENT(VIDBRDGCONNECT,					IP_CONNECTING,		CSipPartyInSimulation::OnConfPartyReceiveVidBridgeConnected)  

PEND_MESSAGE_MAP(CSipPartyInSimulation,CParty);   


///////////////////////////////////////////////////////////////////////
CSipPartyInSimulation::CSipPartyInSimulation()
{
	m_pLobbyApi = new CLobbyApi; 
	m_pConfApi  = new CConfApi; 
	m_eResponsibility	= kNoResponsibility;
	m_eLobbyRejectReason = SipCodesSipUnknownStatus;

	
	m_pInitialModeSIP	= new CIpComMode;
	m_pLocalCapSIP		= new CSipCaps;
	m_pNetSetup			= new CSipNetSetup;
	
	m_interfaceType		= SIP_INTERFACE_TYPE;
//	m_state				= IP_CONNECTING;
	
	VALIDATEMESSAGEMAP
}

///////////////////////////////////////////////////////////////////////
CSipPartyInSimulation::~CSipPartyInSimulation()
{
	POBJDELETE(m_pInitialModeSIP);
	POBJDELETE(m_pLocalCapSIP);
	POBJDELETE(m_pNetSetup);

	m_pLobbyApi->DestroyOnlyApi();
	POBJDELETE(m_pLobbyApi);
	m_pConfApi->DestroyOnlyApi();
	POBJDELETE(m_pConfApi);
}

///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::Create(CSegment& appParam)     
{      
	CParty::Create(appParam); 	
	m_pLobbyApi->CreateOnlyApi(*m_pCreatorRcvMbx);
	m_eResponsibility = kLobby;
}


///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::CleanUp()
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::CleanUp: Name ", m_partyConfName);
	DestroyPartyTask();
}


///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnLobbyIdent(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::OnLobbyIdent: Name ",m_partyConfName);
	WORD				  len			= 0;
	sipSdpAndHeaders* pSdpAndHeaders= NULL;
	CSipCaps* pRemoteCaps	= new CSipCaps;

	m_pNetSetup->DeSerialize(NATIVE,*pParam);
	*pParam >> len;
	pSdpAndHeaders = (sipSdpAndHeaders *) new BYTE[len];
	pParam->Get((BYTE*)pSdpAndHeaders,len);

    CCommConf* pComConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
    PASSERTMSG_AND_RETURN(!pComConf, "!pComConf");
	pRemoteCaps->Create(*pSdpAndHeaders, pComConf->GetConfMediaType(), false);

	m_state = IP_CONNECTING;
	m_eResponsibility = kLobby;
	m_pLobbyApi->PartyIdent(this, INITIAL, statOK);  // always initial channel 
	PDELETEA(pSdpAndHeaders);
	POBJDELETE(pRemoteCaps);
}


///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnLobbyReject(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::OnLobbyReject: Name ",m_partyConfName);
	DWORD	reason = 0xFFFFFFFF;
	WORD	len				= 0;
	sipSdpAndHeaders* pSdpAndHeaders= NULL;
	
	m_pNetSetup->DeSerialize(NATIVE,*pParam);
	*pParam >> reason;

	*pParam >> len;
	pSdpAndHeaders = (sipSdpAndHeaders *) new BYTE[len];
	pParam->Get((BYTE*)pSdpAndHeaders,len);


	PTRACE2INT(eLevelInfoNormal,"CSipPartyInSimulation::OnLobbyReject: Reject reason %d",reason);
	m_state = IP_DISCONNECTING;
	switch(reason)
	{
	case cmReasonTypeCallForwarded:
		m_eLobbyRejectReason = SipCodesMovedTemp;
		break;
	case cmReasonTypeNoPermision:
		m_eLobbyRejectReason = SipCodesForbidden;
		break;
	case cmReasonTypeNoBandwidth:
		m_eLobbyRejectReason = SipCodesBusyHere;
		break;
	default:
		DBGPASSERT(reason);
		m_eLobbyRejectReason = SipCodesInternalSrvErr;
		break;
	}
	
	PDELETEA(pSdpAndHeaders);
}


///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnLobbyTransfer(CSegment * pParam)
{
	CSegment rspMsg;
	WORD  returnSyncStatus = 0;
	WORD mode = 0xFFFF;
	*pParam >> mode;
	m_pConfApi->CreateOnlyApi(*m_pConfRcvMbx,NULL,NULL,1);	

	RemoteIdent epType = Regular;
	eIsUseOperationPointsPreset isUseOperationPointesPresets = eIsUseOPP_No;
	BOOL bIsRemoteSlave = FALSE;

	TRACEINTO << "Name: " << m_partyConfName << ", epType: " << epType <<
				 ", isUseOperationPointesPresets: "  << isUseOperationPointesPresets;

	if (mode == PARTYTRANSFER)
	{		
		// sync call to conf
		// VNGR-6679 - Solution


		LyncConnType lyncEpType = No_Lync;

		PTRACE2INT(eLevelError, "CSipPartyInCreate::OnLobbyTransferIdle: epType=", (DWORD)epType);



		WORD res = m_pConfApi->AddInSipParty(m_pNetSetup, NULL, this, *m_pRcvMbx, m_name,
												PARTY_TRANSFER_TOUT, rspMsg,FALSE,FALSE,FALSE,
												lyncEpType,epType, isUseOperationPointesPresets, bIsRemoteSlave);

		if(res == 0)
		{
            rspMsg >> returnSyncStatus;
		}
		
		if (res)
		{
			PTRACE(eLevelError,"CSipPartyInSimulation::OnLobbyTransfer : \'TRANSFER PARTY FAILED (TimeOut) !!!\'");

			m_pLobbyApi->PartyTransfer(this, statTout);
//			m_eDialState = kTransferFailed;
			
			// currently for simulation we mark this code.
//			if (m_pConfApi->TaskExists())
//			{
//				TellConfOnDisconnecting(SIP_TIMER_POPPED_OUT);
//			}
//			else
			{
				m_state = IP_DISCONNECTING;
				CleanUp();
			}
		}
		else if(returnSyncStatus)
		{
			PTRACE(eLevelError,"CSipPartyInSimulation::OnLobbyTransfer : \'TRANSFER PARTY FAILED (Conf Reject) !!!\'");
			m_pLobbyApi->PartyTransfer(this, returnSyncStatus);  
			m_state = IP_DISCONNECTING;
			CleanUp();
		}  
		else 
		{
			PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::OnLobbyTransfer: Transfer party to conf ok, Name ", m_partyConfName);
			m_pLobbyApi->PartyTransfer(this, statOK);  
		}  

		m_eResponsibility = kConf;
	}
	else 
	{
		DBGPASSERT(mode ? mode : YES);
	}
}


///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnConfEstablishCall(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::OnConfEstablishCall: Name ",m_partyConfName);
//	InitTimer(GetRcvMbx());

	BYTE bCapsDontMatch	= NO;
	
	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
	CRsrcParams* pCsRsrcParams  = new CRsrcParams;
	UdpAddresses sUdpAddressesParams;
	CQoS* pQos = new CQoS;	
	DWORD confParamLen  = 0;
	char* strConfInfo	= NULL;
	BYTE eTransportType = 0;
	
	if(m_pNetSetup)
		m_pNetSetup->DeSerialize(NATIVE,*pParam);
	if (m_pLocalCapSIP)
		m_pLocalCapSIP->DeSerialize(NATIVE,*pParam);
	if(m_pInitialModeSIP)
		m_pInitialModeSIP->DeSerialize(NATIVE,*pParam);

	pQos->DeSerialize(NATIVE,*pParam);
	*pParam >> m_bIsAdvancedVideoFeatures;

	*pParam >> confParamLen;
	
	if ( confParamLen )
	{ 
		strConfInfo = new char[confParamLen];
		pParam->Get((BYTE*)strConfInfo, confParamLen);
		strConfInfo[confParamLen-1] = 0;
	}
	
	*pParam >> eTransportType; // not used in dial in
	pRtpRsrcParams->DeSerialize(NATIVE,*pParam);
	pCsRsrcParams->DeSerialize(NATIVE,*pParam);
	pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));

	// test best mode
	CSipComMode* pTargetMode = NULL;
	if (m_pLocalCapSIP && m_pLocalCapSIP->GetNumOfCapSets())
	{
		pTargetMode = m_pLocalCapSIP->FindTargetMode(cmCapReceiveAndTransmit, (const CSipComMode&)*m_pInitialModeSIP);
	}

	if (m_pLocalCapSIP && m_pLocalCapSIP->GetNumOfCapSets())
	{
//update SDP in MGC operator (connection info window)
		PartyOriginalRemoteCaps(m_pLocalCapSIP);// just to simulate monitoring
	}
	else
	{
		PTRACE2(eLevelError,"CSipPartyInSimulation::OnConfEstablishCall: No Remote capabilities!!! Name ",m_partyConfName);
		bCapsDontMatch = YES;
	}

	if (bCapsDontMatch)
	{
		DBGPASSERT(YES);
		TellConfOnDisconnecting(SIP_CAPS_DONT_MATCH);
	}
	else if( m_pInitialModeSIP )
	{
		m_pConfApi->SipPartyRemoteCapsRecieved(GetPartyId(), m_pLocalCapSIP, m_pInitialModeSIP);
	}

	//report on channels connected!!
	OnPartyChannelsConnected();

    POBJDELETE(pRtpRsrcParams);
	POBJDELETE(pCsRsrcParams);
	POBJDELETE(pQos);
	PDELETEA(strConfInfo);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnConfPartyReceiveAudBridgeConnected()
{
	ON(m_isAudioBridgeConnected);

	OnPartyReceivedAck(STATUS_OK);// report to the party control on party connected, change m_state to connected.
}

/////////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnConfPartyReceiveVidBridgeConnected()
{
	ON(m_isVideoBridgeConnected);

	OnPartyReceivedAck(STATUS_OK);// report to the party control on party connected, change m_state to connected.
}


///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnPartyReceivedAck(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::OnPartyReceivedAck: Name ",m_partyConfName);
	
	if (status == STATUS_OK)
	{
		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected) ||
			(m_isAudioBridgeConnected && GetIsVoice()))
		{
			m_state		 = IP_CONNECTED;
			BYTE bIsEndAddParty = YES;
			m_pConfApi->SipPartyRemoteConnected(GetPartyId(),  m_pInitialModeSIP, FALSE);

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
			StartIvr();
		}
	}
	else
	{
		DBGPASSERT(status);
		PTRACE2INT(eLevelInfoNormal,"CSipPartyInSimulation::OnPartyReceivedAck: Ack with bad status %d",status);
		TellConfOnDisconnecting(status);
	}
}

///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::DestroyPartyTask()
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	if (m_eResponsibility == kConf)
		m_pConfApi->PartyEndDisConnect(GetPartyId(), statOK);
	else
		DBGPASSERT(YES);

	PartySelfKill();
}

///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnConfCloseConnectingCall(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::OnConfCloseConnectingCall: Name ",m_partyConfName);

	m_state = IP_DISCONNECTING;
	LogicalChannelDisconnect(SDP);

	CleanUp();
}

///////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::LogicalChannelDisconnect(DWORD eChannelType)
{
	if(m_eResponsibility == kConf)
		LogicalChannelDisconnectIpPartyFunction(eChannelType);
}


/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::LogicalChannelDisconnectIpPartyFunction(DWORD eChannelType)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << eChannelType;
	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELDISCONNECT,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);
}


/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::PartyOriginalRemoteCaps(CSipCaps* pRemoteCaps)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::OnPartyOriginalRemoteCaps: Name ", m_partyConfName);

	if ( pRemoteCaps )
	{
		CSegment remoteCapsSeg;
		pRemoteCaps->SerializeCapArrayOnly(remoteCapsSeg, YES);
		m_pConfApi->UpdateDB(this, RMOT323CAP, (DWORD) 0, 1, &remoteCapsSeg);   
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::TellConfOnDisconnecting(int reason,const char* alternativeAddrStr)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::TellConfOnDisconnecting: Name ",m_partyConfName);
	m_disconnectCause = reason;
	m_state = IP_DISCONNECTING;
	LogicalChannelDisconnect(SDP);
	m_pConfApi->PartyDisConnect(reason,this,alternativeAddrStr); 
	m_pConfApi->UpdateDB(this,DISCAUSE,reason,1); 
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::SetPartyMonitorBaseParamsAndConnectChannel(DWORD channelType,DWORD rate,
												mcTransportAddress* partyAdd,mcTransportAddress* mcuAdd ,DWORD protocol,
												DWORD pmIndex,DWORD vendorType)
{
	CCapSetInfo capInfo = (CapEnum)protocol;
	CPrtMontrBaseParams* pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)channelType);
	if (pPrtMonitrParams == NULL)
	{
		FPTRACE(eLevelError, "SetPartyMonitorBaseParamsAndConnectChannel: pPrtMonitrParams = NULL!!!");
		return;
	}

	SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,rate,partyAdd,mcuAdd,(DWORD)capInfo.GetIpCapCode(),pmIndex);						
	LogicalChannelConnect(pPrtMonitrParams,(DWORD)channelType,vendorType);
	POBJDELETE(pPrtMonitrParams);
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor, DWORD channelType, DWORD vendorType)
{
	CSegment* pSeg = new CSegment;  

	*pSeg << vendorType << channelType;

    if (pPrtMonitor)
	pPrtMonitor->Serialize(NATIVE,*pSeg); 

	if((EIpChannelType)channelType == SIGNALING)
		LogicalChannelUpdate(channelType,vendorType);

	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELCONNECT,(DWORD) 0,1,pSeg);

	POBJDELETE(pSeg);	
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::LogicalChannelUpdate(DWORD channelType, DWORD vendorType)
{
	CSegment* pSeg = new CSegment;  

    //VNGFE-6008
    //*pSeg << PARTYNAME << vendorType << channelType;
    *pSeg << m_pParty->GetName() << vendorType << channelType;

	m_pNetSetup->Serialize(NATIVE,*pSeg);

	//VNGR-24921
	//m_pConfApi->UpdateDB(this,IPLOGICALCHANNELUPDATE,(DWORD) 0,1,pSeg);
	m_pConfApi->UpdateDB(NULL,IPLOGICALCHANNELUPDATE,(DWORD) 0,1,pSeg);

	POBJDELETE(pSeg);	
}


/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::OnPartyChannelsConnected()
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::OnPartyChannelsConnected: Name ",m_partyConfName);
//	EConfType eConfType = m_pInitialModeSIP->GetConfType();
//	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
//	m_pCurrentMode->SetConfType(eConfType);

	// nissim
    unsigned int dummy[2];
    dummy[0]=0;
    dummy[1]=0;
    m_pConfApi->SipPartyChannelsConnected(GetPartyId(),m_pInitialModeSIP,dummy);
	// nissim

    UpdateDbOnChannelsConnected();
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::UpdateDbOnChannelsConnected()
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInSimulation::UpdateDbOnChannelsConnected: Name ",m_partyConfName);

	mcTransportAddress localIp;
	memset(&localIp,0,sizeof(mcTransportAddress));
	mcTransportAddress remoteIp;
	memset(&remoteIp,0,sizeof(mcTransportAddress));
	if (m_pNetSetup->GetIpVersion() == eIpVersion4)
		localIp.addr.v4.ip = 0x0105aabb;
	else
		::stringToIp(&localIp,"2001:db8:0:1:213:21ff:feae:4aab");
	
	cmCapDataType  mediaArr[] = {cmCapAudio,cmCapVideo,cmCapData};
	cmCapDirection directionArr[] = {cmCapReceive,cmCapTransmit};
	BYTE bIsTransmit;
	EIpChannelType eChanType;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for(int i=0 ; i<MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		if (m_pNetSetup->GetIpVersion() == eIpVersion4)
			remoteIp.addr.v4.ip = 0x0105ccdd;
		else
			::stringToIp(&remoteIp,"2001:db8:0:1:213:21ff:feae:5555");
		
		for (int j=0; j<2; j++)
		{
			bIsTransmit = (directionArr[j] == cmCapTransmit);
			eChanType   = ::CalcChannelType(mediaType,bIsTransmit,eRole);
			if (m_pInitialModeSIP->IsMediaOn(mediaType,directionArr[j],eRole))
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


/////////////////////////////////////////////////////////////////////////////
void CSipPartyInSimulation::UpdateDbScm()
{
	CSegment localScmSeg;
	CSegment remoteScmSeg;
	
	m_pInitialModeSIP->Serialize(localScmSeg,cmCapTransmit,YES);
	m_pConfApi->UpdateDB(this,LOCAL323COMMODE,0,1,&localScmSeg);
	m_pInitialModeSIP->Serialize(remoteScmSeg,cmCapReceive,YES);
	m_pConfApi->UpdateDB(this,RMOT323COMMODE,0,1,&remoteScmSeg);   
}


