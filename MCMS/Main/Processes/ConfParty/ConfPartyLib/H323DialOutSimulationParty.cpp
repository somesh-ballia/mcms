
/*$Header: /MCMS/MAIN/subsys/mcms/PRT323OU.H 18    26/03/01 20:49 Matvey $*/
//+========================================================================+
//                            PRT323OU.CPP                                  |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323DialOutSimulationParty.cpp                                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Uri A.                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 05/07/05     |                                                     |
//+========================================================================+

//#ifndef __H323OUTPARTYSIMULATION__
#include "H323DialOutSimulationParty.h"
//#endif

//#include "TraceStream.h"
//#include "Trace.h"
#include "StateMachine.h"
#include "ConfPartyOpcodes.h"
#include "H323Scm.h"
#include "H323Caps.h"
#include "RsrcParams.h"
#include "AllocateStructs.h"
#include "H323NetSetup.h"
#include "OpcodesMcmsCommon.h"
#include "IpCommon.h"



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CH323DialOutSimulationPartyEntryPoint(void* appParam)
{ 
  CH323DialOutSimulationParty*  pPartyTaskApp = new CH323DialOutSimulationParty;
  pPartyTaskApp->Create(*(CSegment*)appParam);
}
////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CH323DialOutSimulationParty)
                // conf events
  ONEVENT(H323ESTABLISHCALL  ,PARTYIDLE			,CH323DialOutSimulationParty::OnConfEstablishCallIdleH323)
  ONEVENT(H323ESTABLISHCALL  ,PARTYSETUP		,CH323DialOutSimulationParty::OnConfEstablishCallIdleH323)
  ONEVENT(CONFCHANGEMODE	 ,PARTYCONNECTED	,CH323DialOutSimulationParty::OnConfChangeModeConnect)
  ONEVENT(CONFDISCONNECT	 ,PARTYIDLE     	,CH323DialOutSimulationParty::OnConfDisconnect)
  ONEVENT(CONFDISCONNECT	 ,ANYCASE  			,CH323DialOutSimulationParty::OnConfDisconnect)
//                // party events	
  
PEND_MESSAGE_MAP(CH323DialOutSimulationParty,CParty);   

/////////////////////////////////////////////////////////////////////////////
CH323DialOutSimulationParty::CH323DialOutSimulationParty() 
{
	
	VALIDATEMESSAGEMAP;  
	
	m_pInitiateScm	= new CComModeH323;
	m_pLocalCap		= new CCapH323;
	m_state			= IDLE;
	m_ipVersion		= eIpVersion4;
	
//  m_pTpktTask						= NULL;
//  m_pRsrcTbl						= NULL;
//	m_pReqCallSetup				= NULL;
//	m_pReqCallAnswer			= NULL;
//	m_pReqCreateCntl			= NULL;
//	m_pReqOutChnl					= NULL;
//	m_pReqInChnlResponse	= NULL;
//	m_pReqStreamOn				= NULL;
//	m_pReqVideoUpdatePic	= NULL;
//	m_interfaceType = H323_INTERFACE_TYPE;
//
//#ifndef WINNT
//
//        retval=make_dir(dirname,0);
//        if(retval != 0x2011 && retval !=0)
//        {
//                        PASSERT(1);
//                        PTRACE(eLevelError,"CH323PartyOut::CH323PartyOut : make_dir() failure");
//        }       
//        retval = remove_f(fullname);
//	
//
//#endif
//
//	m_pMuxCntl->Suspend(1); //Meanwhile The H323 parties don't need a MuXCntl        

}

/////////////////////////////////////////////////////////////////////////////
CH323DialOutSimulationParty::~CH323DialOutSimulationParty() 
{
  POBJDELETE(m_pInitiateScm); 
  POBJDELETE(m_pLocalCap); 
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DialOutSimulationParty::Create(CSegment& appParam)     
{      
	CParty::Create(appParam);   
	
	m_pConfApi = new CConfApi; 
	
	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx,NULL,NULL,1); 
	// update initial number of retrials
	DWORD redialnum = GetSystemCfgFlagInt<DWORD>(CFG_KEY_NUMBER_OF_REDIAL);//::GetpConfCfg()->GetNumRedials();
	m_pConfApi->UpdateDB(this,NUMRETRY,(DWORD) redialnum,1); 
//	Run(PARTY_TASK_PRIORITY); 
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DialOutSimulationParty::OnConfEstablishCallIdleH323(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323DialOutSimulationParty::OnConfEstablishCallIdleH323 : Name - ",PARTYNAME);

	DWORD  tempWord;

	*pParam >> tempWord;
//	*pParam >> tempWord;//videoPlusType
	*pParam >> m_PartyRsrcID;
	*pParam >> tempWord;//cascadeMode
	*pParam >> tempWord;//nodeType
	*pParam >> m_videoRate;//vidRate
//	*pParam >> tempByte;//bIsIpOnlyConf
	*pParam >> tempWord;//encAlg
	*pParam >> tempWord;//halfKeyType

	CH323NetSetup* pTempH323NetSetup;
	pTempH323NetSetup = new CH323NetSetup; AUTO_DELETE(pTempH323NetSetup);
	pTempH323NetSetup->DeSerialize(NATIVE,*pParam);

	m_ipVersion = pTempH323NetSetup->GetIpVersion();
	m_pInitiateScm->DeSerialize(NATIVE,*pParam);
	m_pLocalCap->DeSerialize(NATIVE,*pParam);

	//debug - Uri A.
	PTRACE(eLevelInfoNormal,"CH323DialOutSimulationParty::OnConfEstablishCallIdleH323, Local Caps: ");
	m_pLocalCap->Dump(NULL, eLevelInfoNormal);
	
	//debug - Uri A.
	PTRACE(eLevelInfoNormal,"CH323DialOutSimulationParty::OnConfEstablishCallIdleH323, Local Initial: ");
	m_pInitiateScm->Dump(NULL, eLevelInfoNormal);
	
	CQoS* pTempQos;
	pTempQos = new CQoS;
	pTempQos->DeSerialize(NATIVE,*pParam);
	POBJDELETE(pTempQos);

	//temp: just to check deserialize
	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	pRtpRsrcParams->DeSerialize(NATIVE,*pParam);  
    pCsRsrcParams->DeSerialize(NATIVE,*pParam);  
    m_ConfRsrcId = pCsRsrcParams->GetConfRsrcId();
	POBJDELETE(pRtpRsrcParams);
	POBJDELETE(pCsRsrcParams);

	UdpAddresses sUdpAddressesParams;
	int sizeOfUdps = sizeof(UdpAddresses);
	pParam->Get((BYTE*)(&sUdpAddressesParams),sizeOfUdps);
	//end of temp: just to check deserialize

	// 1. update DB on open N x channels
	OnH323LogicalChannelConnect(H225);
	OnH323LogicalChannelConnect(H245);
	OnH323LogicalChannelConnect(AUDIO_IN);
	OnH323LogicalChannelConnect(AUDIO_OUT);
	OnH323LogicalChannelConnect(VIDEO_IN);
	OnH323LogicalChannelConnect(VIDEO_OUT);

	// 2. Update Rmt capabilities, Rmt SCM and local SCM 
	// 3. connect
	// 4. connect All
	OnH323EndChannelConnectSetupOrConnect();

	m_state = PARTYCONNECTED;
}


////////////////////////////////////////////////////////////////////////////////
void CH323DialOutSimulationParty::OnH323LogicalChannelConnect(DWORD channelType)
{
	CPrtMontrBaseParams *pPrtMonitrParams	= CPrtMontrBaseParams::AllocNewClass((EIpChannelType)channelType);
	CSegment* pSeg = new CSegment;  

	InitAndSetPartyMonitor(pPrtMonitrParams, channelType);

	*pSeg << (DWORD)0;//Regular;//*vendorType
	*pSeg << channelType;//of type EIpChannelType

	if (pPrtMonitrParams)
		pPrtMonitrParams->Serialize(NATIVE,*pSeg);

	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELCONNECT,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);
	POBJDELETE(pPrtMonitrParams);
}


////////////////////////////////////////////////////////////////////////////////
void CH323DialOutSimulationParty::InitAndSetPartyMonitor(CPrtMontrBaseParams *pPrtMonitrParams, DWORD channelType)
{
	cmCapDirection direction;
	cmCapDataType dataType;

	DWORD actualRate=0xFFFFFFFF;
	mcTransportAddress partyAddr;
//	DWORD partyPort;
	mcTransportAddress mcuAddr;
//	DWORD mcuPort;
	CCapSetInfo capInfo;
	
	memset(&partyAddr,0,sizeof(mcTransportAddress));
	memset(&mcuAddr,0,sizeof(mcTransportAddress));
	if (m_ipVersion == eIpVersion4)
	{
		partyAddr.addr.v4.ip	= 0xAC16BC0A;
		mcuAddr.addr.v4.ip		= 0xAC16BC14;
	}
	else
	{
		::stringToIp(&partyAddr,"2001:db8:0:1:213:21ff:feae:4aab");
		::stringToIp(&mcuAddr,"2001:db8:0:1:213:21ff:fe11:4bbc");
	}

	if((channelType != H225) && (channelType != H245))
	{
		if(AUDIO_IN)
		{
			direction	= cmCapReceive;
			dataType	= cmCapAudio;
			partyAddr.port	= 30;
		}
		if(AUDIO_OUT)
		{
			direction	= cmCapTransmit;
			dataType	= cmCapAudio;
			partyAddr.port	= 32;
		}
		if(VIDEO_IN)
		{
			direction	= cmCapReceive;
			dataType	= cmCapVideo;
			partyAddr.port	= 34;
		}
		if(VIDEO_OUT)
		{
			direction	= cmCapTransmit;
			dataType	= cmCapVideo;
			partyAddr.port	= 36;
		}
		mcuAddr.port = partyAddr.port + 10;
		capInfo = (CapEnum)m_pInitiateScm->GetMediaType(dataType, direction);
	}
	else
	{
		partyAddr.port	= 5;
		mcuAddr.port		= 6;
	}

	SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,actualRate,&partyAddr,&mcuAddr,(DWORD)capInfo.GetIpCapCode());		
}


/////////////////////////////////////////////////////////////////////////////
void  CH323DialOutSimulationParty::OnH323EndChannelConnectSetupOrConnect()
{
	PTRACE2(eLevelInfoNormal,"CH323DialOutSimulationParty::OnH323EndChannelConnectSetupOrConnect : Name - ",PARTYNAME);
	
	H323EndChannelConnect();

	BYTE bIsCascade = FALSE;// can be change to TRUE for testing

	m_status = statOK;

/* 	if ( (m_status != statOK) && (m_status != statSecondary) && (m_status != statVideoBeforeAudio) )
	{	
		m_pConfApi->H323PartyConnect(this, m_status, bIsCascade);
		return; // failed to establish connection
	}*/
	
	PTRACE2(eLevelInfoNormal,"CH323DialOutSimulationParty::OnH323EndChannelConnectSetupOrConnect : \'PARTY CONNECTED !!!\' - ",PARTYNAME);
	
	CCapH323 *pTmpRmtCaps = new CCapH323;
	*pTmpRmtCaps = *m_pLocalCap;
	m_status = statOK;
	
	{// send the connect (RMT caps and audio mode) and connect (other media) all to the party control
		m_pConfApi->H323PartyConnect(GetPartyId(), m_status, bIsCascade, cmMSMaster, 0, pTmpRmtCaps, m_pInitiateScm);
		m_pConfApi->H323PartyConnectAll(GetPartyId(), m_pInitiateScm, m_videoRate,pTmpRmtCaps,0, 0);
	}
	
	POBJDELETE(pTmpRmtCaps);		
}



/////////////////////////////////////////////////////////////////////////////
void  CH323DialOutSimulationParty::H323EndChannelConnect()
{
	PTRACE2(eLevelInfoNormal,"CH323DialOutSimulationParty::H323EndChannelConnect : Name  - ",PARTYNAME);

	CSegment rmtCapSeg;
	m_pLocalCap->SerializeCapArrayOnly(rmtCapSeg,TRUE);
	m_pConfApi->UpdateDB(this,RMOT323CAP,(DWORD) 0,1,&rmtCapSeg);   

	
	// Updates the video in channel.
	CSegment      rmtCurComSeg;
	CComModeH323 *pRmtCurComH323 = new CComModeH323;
	
	// In case of video in channel frame rate or resolution is differ from 
	// video out channel, we get the real values from the segment and update
	// correspondingly.
	*pRmtCurComH323 = *m_pInitiateScm;
	// Adding LPR capabilities for monitoring
	if (m_pLocalCap->IsLPR())
	{
		pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES,m_pLocalCap);
	}
	else
		pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES);
	
//	pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES);
	m_pConfApi->UpdateDB(this,RMOT323COMMODE,(DWORD) 0,1,&rmtCurComSeg);   
	
	POBJDELETE(pRmtCurComH323);


	CSegment localComSeg;  
	CComModeH323* pLocalScmH323 = new CComModeH323;
	*pLocalScmH323 = *m_pInitiateScm;
	if (m_pLocalCap->IsLPR())
	{
		pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES,m_pLocalCap);
	}
	else
		pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES);

//	pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES);
	m_pConfApi->UpdateDB(this,LOCAL323COMMODE,(DWORD) 0,1,&localComSeg);
	H323UpdateBaudRate();
	POBJDELETE(pLocalScmH323);	
}

/////////////////////////////////////////////////////////////////////////////
void   CH323DialOutSimulationParty::H323UpdateBaudRate()
{  
	DWORD totalRate = 0;
	CSegment* pSeg1 = new CSegment; 
	CSegment* pSeg2 = new CSegment;
    totalRate = m_pInitiateScm->GetTotalBitRate(cmCapReceive);
	*pSeg1 << totalRate;
	m_pConfApi->UpdateDB(this,RECEIVEBAUDRATE,(DWORD) 0,1,pSeg1);
    totalRate = m_pInitiateScm->GetTotalBitRate(cmCapTransmit);
	*pSeg2 << totalRate;
	m_pConfApi->UpdateDB(this,TRANSMITBAUDRATE,(DWORD) 0,1,pSeg2);
	POBJDELETE(pSeg1);
	POBJDELETE(pSeg2);
}


/////////////////////////////////////////////////////////////////////////////
void  CH323DialOutSimulationParty::OnConfChangeModeConnect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	BYTE  tempByte;
	WORD  tempWord;

	*pParam >> tempWord;//changeModeState
	*pParam >> tempByte;//bIsLocalCapH323

	m_pInitiateScm->DeSerialize(NATIVE,*pParam);
	if(tempByte)
		m_pLocalCap->DeSerialize(NATIVE,*pParam);

	// response only with successfull Change Mode
	m_pConfApi->PartyEndChangeModeIp(GetPartyId(), *m_pInitiateScm, statOK);
}


/////////////////////////////////////////////////////////////////////////////
void  CH323DialOutSimulationParty::OnConfDisconnect()
{
	CSmallString msg;
	msg << PARTYNAME << ". m_state = " << m_state;
	PTRACE2(eLevelInfoNormal, "CH323DialOutSimulationParty::OnConfDisconnect : Name - ", msg.GetString());

	m_state = PARTYDISCONNECTING;
	//1. Update DB on disconnecting of the channels.
	H323LogicalChannelDisConnect(VIDEO_OUT, cmCapVideo, 1, kRolePeople, FALSE);
	H323LogicalChannelDisConnect(VIDEO_IN,  cmCapVideo, 0, kRolePeople, FALSE);
	H323LogicalChannelDisConnect(AUDIO_OUT, cmCapAudio, 1, kRolePeople, FALSE);
	H323LogicalChannelDisConnect(AUDIO_IN,  cmCapAudio, 0, kRolePeople, FALSE);
	H323LogicalChannelDisConnect(H245,  cmCapEmpty, 0, kRolePeople, FALSE);
	H323LogicalChannelDisConnect(H225,  cmCapEmpty, 0, kRolePeople, FALSE);

	//2. Send End Disconnect to conf level.
	OnEndH323Disconnect();
}

/////////////////////////////////////////////////////////////////////////////
void CH323DialOutSimulationParty::OnEndH323Disconnect()
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	DeleteAllTimers();

	m_pConfApi->PartyEndDisConnect(GetPartyId(), statOK);   // notify conference
	PartySelfKill();
}

/////////////////////////////////////////////////////////////////////////////////
void CH323DialOutSimulationParty::H323LogicalChannelDisConnect(DWORD channelType, WORD dataType, WORD bTransmitting, BYTE roleLabel, BYTE bUpdateCommMode)
{
	//1: Update Monitoring:
	//1.1 Update channel monitoring:
	CSegment*  pSeg = new CSegment;
	*pSeg   << channelType;
	
	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELDISCONNECT,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);

	//1.2 Update communication mode monitoring:
	// only in case we will want to create partially sets
	if (bUpdateCommMode)
	{
		CComModeH323* pCurComH323 = new CComModeH323;
		*pCurComH323 = *m_pInitiateScm;
		CSegment* pSeg = new CSegment;
		
		cmCapDirection direction = CalcCmCapDirection(bTransmitting);
		
		pCurComH323->SetMediaOff((cmCapDataType)dataType ,direction, (ERoleLabel)roleLabel); 
		if (m_pLocalCap->IsLPR())
		{
			pCurComH323->Serialize(*pSeg,direction,YES,m_pLocalCap);
		}
		else
			pCurComH323->Serialize(*pSeg,direction,YES);
		
//		pCurComH323->Serialize(*pSeg,direction,YES);
		
		if (direction == cmCapReceive)
			m_pConfApi->UpdateDB(this,RMOT323COMMODE,(DWORD) 0,1,pSeg);
		
		else if (direction == cmCapTransmit)
			m_pConfApi->UpdateDB(this,LOCAL323COMMODE,(DWORD) 0,1,pSeg);
		
		POBJDELETE(pSeg);
		POBJDELETE(pCurComH323);
	}
}

