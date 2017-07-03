//+========================================================================+
//                            SipPartyOutSimulation.cpp                    |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SipPartyOutSimulation.cpp                                   |
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
#include "SIPParty.h"
#include "SipPartyOutSimulation.h"

void SipPartyOutEntryPointSimulation(void* appParam)
{ 
  CSipPartyOutSimulation* pPartyTaskApp = new CSipPartyOutSimulation;
  pPartyTaskApp->Create(*(CSegment*)appParam);
}

PBEGIN_MESSAGE_MAP(CSipPartyOutSimulation)
ONEVENT(SIP_CONF_ESTABLISH_CALL,			IP_DISCONNECTED,	CSipPartyOutSimulation::OnConfEstablishCall)
ONEVENT(AUDBRDGCONNECT,						IP_CONNECTING,		CSipPartyOutSimulation::OnConfPartyReceiveAudBridgeConnected)  
ONEVENT(VIDBRDGCONNECT,						IP_CONNECTING,		CSipPartyOutSimulation::OnConfPartyReceiveVidBridgeConnected)  

ONEVENT(CONFDISCONNECT,						IP_DISCONNECTING,	CSipPartyOutSimulation::OnConfCloseConnectingCall)
ONEVENT(CONFDISCONNECT,						IP_CONNECTED,		CSipPartyOutSimulation::OnConfCloseConnectingCall)
ONEVENT(CONFDISCONNECT,						IP_CONNECTING,		CSipPartyOutSimulation::OnConfCloseConnectingCall)

PEND_MESSAGE_MAP(CSipPartyOutSimulation,CParty);   



/////////////////////////////////////////////////////////////////////////////////
CSipPartyOutSimulation::CSipPartyOutSimulation()
{
	m_pConfApi  = new CConfApi; 
	
	m_pInitialModeSIP	= new CIpComMode;
	m_pLocalCapSIP		= new CSipCaps;
	m_pNetSetup			= new CSipNetSetup;
	
	m_interfaceType		= SIP_INTERFACE_TYPE;

	VALIDATEMESSAGEMAP
}

/////////////////////////////////////////////////////////////////////////////////
CSipPartyOutSimulation::~CSipPartyOutSimulation()
{
	POBJDELETE(m_pInitialModeSIP);
	POBJDELETE(m_pLocalCapSIP);
	POBJDELETE(m_pNetSetup);
	POBJDELETE(m_pConfApi);
}


/////////////////////////////////////////////////////////////////////////////////
void  CSipPartyOutSimulation::Create(CSegment& appParam)     
{      
	CParty::Create(appParam);   
	
	m_pConfApi = new CConfApi; 
	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx, NULL, NULL, 1); 

	// update initial number of retrials
	DWORD redialnum = GetSystemCfgFlagInt<DWORD>(CFG_KEY_NUMBER_OF_REDIAL); //::GetpConfCfg()->GetNumRedials();
	m_pConfApi->UpdateDB(this, NUMRETRY,(DWORD) redialnum,1); 
}

/////////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::CleanUp()
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::CleanUp: Name ",m_partyConfName);
	DestroyPartyTask();
}


/////////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::OnConfEstablishCall(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::OnConfEstablishCall: Name ",m_partyConfName);
//	InitTimer(GetRcvMbx());

	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	UdpAddresses sUdpAddressesParams;
	CQoS*			pQos			= new CQoS;	 	
	DWORD			confParamLen	= 0;
	char*			strConfInfo		= NULL;
	BYTE			eTransportType	= 0;
	DWORD			addrLen			= 0;

	if(m_pNetSetup)
		m_pNetSetup->DeSerialize(NATIVE,*pParam);
	if(m_pLocalCapSIP)
		m_pLocalCapSIP->DeSerialize(NATIVE,*pParam);
	if(m_pInitialModeSIP)
		m_pInitialModeSIP->DeSerialize(NATIVE,*pParam);
	pQos->DeSerialize(NATIVE,*pParam);
	*pParam >> m_bIsAdvancedVideoFeatures;
	
	// test best mode
	CSipComMode* pTargetMode = NULL;
	if (m_pLocalCapSIP && m_pLocalCapSIP->GetNumOfCapSets())
	{
		pTargetMode = m_pLocalCapSIP->FindTargetMode(cmCapReceiveAndTransmit, (const CSipComMode&)*m_pInitialModeSIP);
	//}

		//debug - Uri A. 
		CSuperLargeString str;
		m_pLocalCapSIP->DumpToString(str);
		PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::OnConfEstablishCall, Local Caps: ",str.GetString());
	}
	else if(NULL == m_pLocalCapSIP)
		PASSERTMSG(1, "m_pLocalCapSIP is NULL!!!"); 	

	
	//debug - Uri A.
	COstrStream msg1;
	if(m_pInitialModeSIP)
	{
		m_pInitialModeSIP->Dump(msg1);
		//YossiG Klocwork NPD issue
		PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::OnConfEstablishCall, Local Initial: ", msg1.str().c_str());
	}

	*pParam >> confParamLen;
	if(confParamLen)
	{ 
		strConfInfo = new char[confParamLen];
		pParam->Get((BYTE*)strConfInfo,confParamLen);
		strConfInfo[confParamLen-1]=0;
	}
	*pParam >> eTransportType;

	pRtpRsrcParams->DeSerialize(NATIVE, *pParam);
	pCsRsrcParams->DeSerialize(NATIVE, *pParam);
	pParam->Get((BYTE *) &sUdpAddressesParams, sizeof(UdpAddresses));

	*pParam >> addrLen;
	if (addrLen)
	{
		ALLOCBUFFER(alternativeAddrStr, addrLen);
		pParam->Get((BYTE*)alternativeAddrStr, addrLen);
		DEALLOCBUFFER(alternativeAddrStr);
	}
		
	POBJDELETE(pRtpRsrcParams);
	POBJDELETE(pCsRsrcParams);
	POBJDELETE(pQos);
	PDELETEA(strConfInfo);

	//report on channels connected!!
	m_state = IP_CONNECTING;
//	m_eDialState = kBeforeInvite;
	CIpComMode* pIpComMode = new CIpComMode;
	*pIpComMode = *m_pInitialModeSIP;
	pIpComMode->SetMediaOff(cmCapAudio, cmCapTransmit);
	pIpComMode->SetMediaOff(cmCapVideo, cmCapTransmit);
	OnPartyChannelsConnected(pIpComMode);// only in channels connected before the invite in dial out.
	POBJDELETE(pIpComMode);
}


/////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::OnPartyChannelsConnected(CIpComMode* pIpComMode)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::OnPartyChannelsConnected: Name ",m_partyConfName);
	// nissim
    unsigned int dummy[2];
    dummy[0]=0;
    dummy[1]=0;
    m_pConfApi->SipPartyChannelsConnected(GetPartyId(),pIpComMode,dummy);
    // nissim
	UpdateDbOnChannelsConnected();
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::UpdateDbOnChannelsConnected()
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::UpdateDbOnChannelsConnected: Name ",m_partyConfName);


	// IpV6 - Monitoring
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
void CSipPartyOutSimulation::UpdateDbScm()
{
	CSegment localScmSeg;
	CSegment remoteScmSeg;
	
	m_pInitialModeSIP->Serialize(localScmSeg,cmCapTransmit,YES);
	m_pConfApi->UpdateDB(this,LOCAL323COMMODE,0,1,&localScmSeg);
	m_pInitialModeSIP->Serialize(remoteScmSeg,cmCapReceive,YES);
	m_pConfApi->UpdateDB(this,RMOT323COMMODE,0,1,&remoteScmSeg);   
}


/////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::SetPartyMonitorBaseParamsAndConnectChannel(DWORD channelType,DWORD rate,
												mcTransportAddress* partyAdd,mcTransportAddress* mcuAdd,DWORD protocol,
												DWORD pmIndex,DWORD vendorType)
{
	CCapSetInfo capInfo = (CapEnum)protocol;
	CPrtMontrBaseParams* pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)channelType);
	if(pPrtMonitrParams)
	{
	::SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,rate,partyAdd,mcuAdd,(DWORD)capInfo.GetIpCapCode(),pmIndex);						
	LogicalChannelConnect(pPrtMonitrParams,(DWORD)channelType,vendorType);				
	POBJDELETE(pPrtMonitrParams);				
}
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor, DWORD channelType, DWORD vendorType)
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
void CSipPartyOutSimulation::LogicalChannelUpdate(DWORD channelType, DWORD vendorType)
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
void CSipPartyOutSimulation::DestroyPartyTask()
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	m_pConfApi->PartyEndDisConnect(GetPartyId(), statOK);
	PartySelfKill();
}

/////////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::OnConfPartyReceiveAudBridgeConnected()
{
	ON(m_isAudioBridgeConnected);

	OnPartyReceivedAck(STATUS_OK);// report to the party control on party connected, change m_state to connected.
}

/////////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::OnConfPartyReceiveVidBridgeConnected()
{
	ON(m_isVideoBridgeConnected);

	OnPartyReceivedAck(STATUS_OK);// report to the party control on party connected, change m_state to connected.
}

/////////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::OnPartyReceivedAck(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::OnPartyReceivedAck: Name ",m_partyConfName);
	
	if (status == STATUS_OK)
	{
		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected) ||
			(m_isAudioBridgeConnected && GetIsVoice()))
		{
	//		m_eDialState = kNotInDialState;
			m_state		 = IP_CONNECTED;
			
			// update DB on remote caps
			CSegment remoteCapsSeg;
			m_pLocalCapSIP->SerializeCapArrayOnly(remoteCapsSeg, YES);
			m_pConfApi->UpdateDB(this, RMOT323CAP, (DWORD) 0, 1, &remoteCapsSeg);   

			BYTE bIsEndAddParty = NO;
			m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pInitialModeSIP, FALSE);
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
			StartIvr();
		}
	}
	else
	{
//		m_eDialState = kBadStatusAckArrived;
		DBGPASSERT(status);
		PTRACE2INT(eLevelInfoNormal,"CSipPartyOutSimulation::OnPartyReceivedAck: Ack with bad status %d",status);
		TellConfOnDisconnecting(status);
	}
}


///////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::OnConfCloseConnectingCall(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::OnConfCloseConnectingCall: Name ",m_partyConfName);

	m_state = IP_DISCONNECTING;
	LogicalChannelDisconnect(SDP);

//	if (m_eDialState == kBeforeOk)
//		m_eDialState = kTerminateByConf;
	CleanUp();
}

///////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::LogicalChannelDisconnect(DWORD eChannelType)
{
	CSegment*  pSeg = new CSegment;
	*pSeg << eChannelType;
	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELDISCONNECT,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyOutSimulation::TellConfOnDisconnecting(int reason,const char* alternativeAddrStr)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutSimulation::TellConfOnDisconnecting: Name ",m_partyConfName);
	m_disconnectCause = reason;
	m_state = IP_DISCONNECTING;
	LogicalChannelDisconnect(SDP);
	m_pConfApi->PartyDisConnect(reason,this,alternativeAddrStr); 
	m_pConfApi->UpdateDB(this,DISCAUSE,reason,1); 
}

