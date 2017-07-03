//+========================================================================+
//                            H323PartyIn.CPP                                  |
//            Copyright 1995 Polycom Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323PartyIn.CPP                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Uri                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 19/7/05     |                                                     |
//+========================================================================+


#include "H323PartyInSimulation.h"
#include "ConfPartyOpcodes.h"
#include "Lobby.h"
#include "LobbyApi.h"
#include "PartyApi.h"
#include "ConfApi.h"
#include "H323Caps.h"
#include "H323NetSetup.h"
#include "H323Control.h"
#include "OpcodesMcmsCommon.h"


#ifndef MCMS
#define MCMS 5
#endif

				// party const
//const WORD   TimerForUnreservedCall  = 100;

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void H323PartyInSimulationEntryPoint(void* appParam)
{  
	CH323PartyInSimulation*  pPartyTaskApp = new CH323PartyInSimulation;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}



PBEGIN_MESSAGE_MAP(CH323PartyInSimulation)
		// lobby events  
  ONEVENT(LOBBYNETIDENT  ,PARTYIDLE   	,CH323PartyInSimulation::OnLobbyH323IdentIdle)
  ONEVENT(LOBBYTRANS     ,PARTYSETUP	,CH323PartyInSimulation::OnLobbyTransferSetup)
  ONEVENT(REJECTCALL	 ,PARTYIDLE   	,CH323PartyInSimulation::OnLobbyRejectCallIdle)
		// conf events
  ONEVENT(CONFDISCONNECT ,PARTYIDLE	  	,CH323PartyInSimulation::OnConfDisconnect)
  ONEVENT(CONFDISCONNECT ,ANYCASE     	,CH323PartyInSimulation::OnConfDisconnect)

  ONEVENT(H323ESTABLISHCALL  ,PARTYIDLE ,CH323PartyInSimulation::OnConfEstablishCallSetup)
  ONEVENT(H323ESTABLISHCALL  ,PARTYSETUP,CH323PartyInSimulation::OnConfEstablishCallSetup)

  ONEVENT(DISCONNECTH323     ,PARTYSETUP		,CH323PartyInSimulation::OnNetDisconnectSetUp)
  ONEVENT(DISCONNECTH323	 ,PARTYCONNECT   	,CH323PartyInSimulation::OnNetDisconnectConnect)
  ONEVENT(DISCONNECTH323	 ,PARTYCHANGEMODE	,CH323PartyInSimulation::OnNetDisconnectConnect)
  ONEVENT(ENDH323DISCONNECT  ,PARTYIDLE 		,CH323PartyInSimulation::OnEndH323Disconnect)
  ONEVENT(ENDH323DISCONNECT  ,PARTYSETUP		,CH323PartyInSimulation::OnEndH323Disconnect)
  ONEVENT(ENDH323DISCONNECT  ,PARTYDISCONNECTING,CH323PartyInSimulation::OnEndH323Disconnect)
  ONEVENT(ENDH323DISCONNECT  ,PARTYCHANGEMODE	,CH323PartyInSimulation::OnEndH323Disconnect)
         //party events

PEND_MESSAGE_MAP(CH323PartyInSimulation,CParty);   

/////////////////////////////////////////////////////////////////////////////
CH323PartyInSimulation::CH323PartyInSimulation()
{
	m_pLobbyApi			= new CLobbyApi; 
	m_pInitialModeH323	= new CComModeH323;
	m_pLocalCapH323		= new CCapH323;
//	m_pPartyInApi   = new CPartyApi;
	
	OFF(m_isReject);
	OFF(m_isLobbySetup);
	m_interfaceType = H323_INTERFACE_TYPE;

	m_forwardAlias		= NULL;
	m_rejectCallReason	= -1;
	m_state				= PARTYIDLE;
	VALIDATEMESSAGEMAP;        
}

/////////////////////////////////////////////////////////////////////////////
CH323PartyInSimulation::~CH323PartyInSimulation()
{
	m_pLobbyApi->DestroyOnlyApi();
	POBJDELETE(m_pLobbyApi);
	POBJDELETE(m_pInitialModeH323); 
	POBJDELETE(m_pLocalCapH323); 

	//m_pPartyInApi->DestroyOnlyApi();
	//POBJDELETE(m_pPartyInApi);

	m_pConfApi->DestroyOnlyApi();
	POBJDELETE(m_pConfApi);

	PDELETE(m_forwardAlias); 
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyInSimulation::Create(CSegment& appParam)
{
	CParty::Create(appParam);

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif

	m_pLobbyApi->CreateOnlyApi(*m_pCreatorRcvMbx);
}


/////////////////////////////////////////////////////////////////////////////
//                  PARTY CONTROL ACTION FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnLobbyH323IdentIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyInSimulation::OnLobbyH323IdentIdle : Name - ",PARTYNAME);

#ifdef __AUTHENTICATION__
	DWORD generator;
	DWORD  hkLen;
	WORD hkType;
	BYTE sid[2];
	int indexTblAuth;
	DWORD dIndexTbl;
	DWORD authLen	= 0;
	DWORD encLen	= 0;

	BYTE *pAuthKey	= NULL;
	BYTE *pEncKey	= NULL;
#endif
	WORD  status = statOK;

	m_pH323NetSetup = new CH323NetSetup;
	m_pH323NetSetup->DeSerialize(NATIVE,*pParam);

#ifdef __AUTHENTICATION__
	*pParam >> (DWORD)dIndexTbl >> authLen >> encLen;
	indexTblAuth = (int)dIndexTbl;

	pParam->Get((BYTE *)sid,2);
	if(authLen)
	{
		pAuthKey	= new BYTE[authLen];
		pParam->Get((BYTE *)pAuthKey,authLen);
	}
	if(encLen)
	{
		pEncKey	= new BYTE[encLen];
		pParam->Get((BYTE *)pEncKey,encLen);
	}

	generator	= m_pH323NetSetup->GetGenerator();
	hkLen		= m_pH323NetSetup->GetHkLen();
	hkType		= m_pH323NetSetup->GetHkType();	
	
	m_pH323Cntl->m_pDHKeyManagement->SetGenerator(generator);
#endif
#ifdef __Resources__
	m_pH323Cntl->Create(*m_pIpDesc,this,*m_pH323NetSetup,*m_pTargetComMode,
						*m_pLocalCapH323,NULL, 0,m_isAutoVidBitRate);
#endif

#ifdef __AUTHENTICATION__
	if(hkLen)
	{
		CDHKey *pRmtSharedSecret	= new CDHKey; 
		pRmtSharedSecret->SetArray(m_pH323NetSetup->GetHalfKey(), m_pH323NetSetup->GetHkLen());
		m_pH323Cntl->m_pDHKeyManagement->SetDHRmtSharedSecret(*pRmtSharedSecret);
		m_pH323Cntl->m_pDHKeyManagement->SetRmtHalfKeyAlg(hkType);
		
		if(m_pTargetComMode->GetOtherEncrypMode() == Encryp_On)
		{
			if(hkType == kHalfKeyDH1024)
			{
				m_pH323Cntl->m_pDHKeyManagement->InitLocalHalfKey();
				CalculateSharedSecret(m_pH323Cntl->m_pDHKeyManagement);
				m_pH323Cntl->m_pDHKeyManagement->SetEncCallKey(m_pH323Cntl->m_pDHKeyManagement->GetDHResultSharedSecret()->GetArray(),
															(m_pH323Cntl->m_pDHKeyManagement->GetDHResultSharedSecret()->GetLength() - XMIT_RCV_KEY_LENGTH));
			}
			else
				PTRACE2(eLevelError,"CH323PartyIn::OnLobbyH323IdentIdle - remote opened key different from DH1024: Name - ",PARTYNAME);
		}

		POBJDELETE(pRmtSharedSecret);
	}

	if(pAuthKey)
		m_bIsAuthenticated = TRUE;
	else
		m_bIsAuthenticated = FALSE;
	
	SetAuthEncrParams(m_pH323Cntl->m_pDHKeyManagement,pAuthKey,pEncKey,indexTblAuth);
#endif
//	// if it's a Gateway call set this flag also at the CH323cntl object
//	if(m_IsGateWay)
//		m_pH323Cntl->SetGWcallFlag(TRUE);


	m_pLobbyApi->PartyIdent(this,0,status);  // allways initial channel 
	m_state = PARTYSETUP; 
	ON(m_isLobbySetup);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnLobbyTransferSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyInSimulation::OnLobbyTransferSetup : Party Name - ",PARTYNAME);

	WORD  mode,rc = 0;
    WORD  returnSyncStatus = 0;
    CSegment rspMsg;
      

	*pParam >> mode;

	m_pConfApi->CreateOnlyApi(*m_pConfRcvMbx,NULL,NULL,1);	

	switch( mode )
	{
	case PARTYTRANSFER  :  
	// sync call to conf
	rc = m_pConfApi->AddInH323Party(m_pH323NetSetup,this, *m_pRcvMbx, m_name,
										PARTY_TRANSFER_TOUT, rspMsg);
		if(rc == 0)
		{
            rspMsg >> returnSyncStatus;
		}

		if ( rc )
		{
			PTRACE(eLevelError,"CH323PartyInSimulation::OnLobbyTransferSetup : \'EXPORT PARTY FAILED (TimeOut) !!!\'");
			m_pLobbyApi->PartyTransfer(this,statTout);
//			CleanUp();
		}
		else if ( returnSyncStatus )
		{
			PTRACE(eLevelError,"CH323PartyInSimulation::OnLobbyTransferSetup : \'EXPORT PARTY FAILED (Conf Reject) !!!\'");
			m_pLobbyApi->PartyTransfer(this, returnSyncStatus);
//			CleanUp();
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CH323PartyInSimulation::OnLobbyTransferSetup : \'EXPORT PARTY O.K. !!!\'");
			m_pLobbyApi->PartyTransfer(this,statOK);  
		}  
		break;

	default   :
		break;
	}



	/*//TEMP YOELLA EXCEPTION	rc = m_pConfApi->AddInH323Party(m_pH323NetSetup,this,*m_pRcvMbx, m_name, PARTY_TRANSFER_TOUT);
		//rc=0;
		if ( rc )
		{
			PTRACE(eLevelError,"CH323PartyInSimulation::OnLobbyTransferSetup : \'EXPORT PARTY FAILED !!!\'");
			m_pLobbyApi->PartyTransfer(this,statTout);
			CleanUp();
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CH323PartyInSimulation::OnLobbyTransferSetup : \'EXPORT PARTY O.K. !!!\'");
			m_pLobbyApi->PartyTransfer(this,statOK);  
		}  
		break;

	default   :
		break;
	}
*/
	OFF(m_isLobbySetup); // shout down the flag tha indicates of lobby responsibilities
	OFF(m_isReject); //caution just in case in any senario that I saw if we've got here the flag is off.
	ON(m_isAddPartyOK);// to check if we failed to allocate resources on the bridges (Audio, Video, Data)
}


/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnLobbyRejectCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyInSimulation::OnLobbyRejectCallIdle : Name - ",PARTYNAME);
	m_pH323NetSetup = new CH323NetSetup;
	m_pH323NetSetup->DeSerialize(NATIVE,*pParam); 
  
	WORD reason;
	*pParam >> /*(WORD)*/reason;  //YOELLA-YAEL

	if(reason == TimerForUnreservedCall)
	{// if it's a reject because of a timer popout of unreserved call
		// we identify the case so we could not return answer to the
		// Lobby and remove resources and return the correct reject reason
		m_isReject = 2;
		reason	   = cmReasonTypeNoPermision;
	}
	else
		ON(m_isReject);

	/*DWORD aliasLen;
	*pParam >> aliasLen;
	if(aliasLen)		
	{  //in case of forwarding
		ALLOCBUFFER(buf, aliasLen);
		pParam->Get((unsigned char*)buf, aliasLen);
		m_forwardAlias = buf;
	}*/

	m_rejectCallReason = (int)reason;
#ifdef __Resources__
	m_pH323Cntl->Create(*m_pIpDesc,this,*m_pH323NetSetup,*m_pTargetComMode,
						*m_pLocalCapH323,NULL,0,m_isAutoVidBitRate);  //dialin
#endif 
	m_state = PARTYSETUP;

	CSegment sParam;
	sParam << (WORD)statOK;
	OnEndH323Disconnect(&sParam);
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyInSimulation::OnEndH323Disconnect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	WORD status;
	*pParam >> status;

	DeleteAllTimers();

	if (!m_isReject && !m_isLobbySetup)
		m_pConfApi->PartyEndDisConnect(GetPartyId(), status); // notify conference
	else
		OFF(m_isReject);

	PartySelfKill();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnConfEstablishCallSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyInSimulation::OnConfEstablishCallIdleH323 : Name - ",PARTYNAME);

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

	CH323NetSetup* pH323NetSetup = new CH323NetSetup;
	pH323NetSetup->DeSerialize(NATIVE,*pParam);
	POBJDELETE(pH323NetSetup);

	m_pInitialModeH323->DeSerialize(NATIVE,*pParam);
	m_pLocalCapH323->DeSerialize(NATIVE,*pParam);

	CQoS* pQos = new CQoS;
	pQos->DeSerialize(NATIVE,*pParam);
//    m_pH323Cntl->SetQualityOfService(*pQos);
	POBJDELETE(pQos);

	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
	pRtpRsrcParams->DeSerialize(NATIVE,*pParam);
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	pCsRsrcParams->DeSerialize(NATIVE,*pParam);
	m_ConfRsrcId = pCsRsrcParams->GetConfRsrcId();
	UdpAddresses sUdpAddressesParams;
	pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));
//    m_pH323Cntl->SetControllerResource(pRtpRsrcParams, pCsRsrcParams, sUdpAddressesParams);
	POBJDELETE(pRtpRsrcParams);
	POBJDELETE(pCsRsrcParams);


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
void CH323PartyInSimulation::OnH323LogicalChannelConnect(DWORD channelType)
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
void CH323PartyInSimulation::InitAndSetPartyMonitor(CPrtMontrBaseParams *pPrtMonitrParams, DWORD channelType)
{
	cmCapDirection direction;
	cmCapDataType dataType;

	DWORD actualRate=0xFFFFFFFF;
	CCapSetInfo capInfo;
	mcTransportAddress partyAddr;
	mcTransportAddress mcuAddr;
	
	memset(&partyAddr,0,sizeof(mcTransportAddress));
	memset(&mcuAddr,0,sizeof(mcTransportAddress));
	if (m_pH323NetSetup->GetIpVersion() == eIpVersion4)
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
		capInfo = (CapEnum)m_pInitialModeH323->GetMediaType(dataType, direction);
	}
	else
	{
		partyAddr.port	= 5;
		mcuAddr.port		= 6;
	}

	SetPartyMonitorBaseParams(pPrtMonitrParams,channelType,actualRate,&partyAddr,&mcuAddr,(DWORD)capInfo.GetIpCapCode());		
}


/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnH323EndChannelConnectSetupOrConnect()
{
	PTRACE2(eLevelInfoNormal,"CH323DialOutSimulationParty::OnH323EndChannelConnectSetupOrConnect : Name - ",PARTYNAME);
	
	H323EndChannelConnect();

	BYTE bIsCascade = FALSE;// can be change to TRUE for testing

	m_status = statOK;

	if ( (m_status != statOK) && (m_status != statSecondary) && (m_status != statVideoBeforeAudio) )
	{	
		m_pConfApi->H323PartyConnect(GetPartyId(), m_status, bIsCascade, cmMSMaster, m_pH323Cntl->GetMrmpChannelHandle(cmCapVideo));
		return; // failed to establish connection
	}
	
	PTRACE2(eLevelInfoNormal,"CH323DialOutSimulationParty::OnH323EndChannelConnectSetupOrConnect : \'PARTY CONNECTED !!!\' - ",PARTYNAME);
	
	CCapH323 *pTmpRmtCaps = new CCapH323;
	*pTmpRmtCaps = *m_pLocalCapH323;
	m_status = statOK;
	
	{// send the connect (RMT caps and audio mode) and connect (other media) all to the party control
		m_pConfApi->H323PartyConnect(GetPartyId(), m_status, bIsCascade, cmMSMaster, m_pH323Cntl->GetMrmpChannelHandle(cmCapVideo), pTmpRmtCaps, m_pInitialModeH323);
		m_pConfApi->H323PartyConnectAll(GetPartyId(), m_pInitialModeH323, m_videoRate,  pTmpRmtCaps,0, 0);
	}
	
	POBJDELETE(pTmpRmtCaps);		
}



/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::H323EndChannelConnect()
{
	PTRACE2(eLevelInfoNormal,"CH323DialOutSimulationParty::H323EndChannelConnect : Name  - ",PARTYNAME);

	CSegment rmtCapSeg;
	m_pLocalCapH323->SerializeCapArrayOnly(rmtCapSeg,TRUE);
	m_pConfApi->UpdateDB(this,RMOT323CAP,(DWORD) 0,1,&rmtCapSeg);   

	
	// Updates the video in channel.
	CSegment      rmtCurComSeg;
	CComModeH323 *pRmtCurComH323 = new CComModeH323;
	
	// In case of video in channel frame rate or resolution is differ from 
	// video out channel, we get the real values from the segment and update
	// correspondingly.
	*pRmtCurComH323 = *m_pInitialModeH323;
	// Adding LPR capabilities for monitoring
	if (m_pLocalCapH323->IsLPR())
	{
		pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES,m_pLocalCapH323);
	}
	else
		pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES);

	
//	pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES);
	m_pConfApi->UpdateDB(this,RMOT323COMMODE,(DWORD) 0,1,&rmtCurComSeg);   
	
	POBJDELETE(pRmtCurComH323);


	CSegment localComSeg;  
	CComModeH323* pLocalScmH323 = new CComModeH323;
	*pLocalScmH323 = *m_pInitialModeH323;
	// Adding LPR capabilities for monitoring
	if (m_pLocalCapH323->IsLPR())
	{
		pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES,m_pLocalCapH323);
	}
	else
		pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES);

	pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES);
	m_pConfApi->UpdateDB(this,LOCAL323COMMODE,(DWORD) 0,1,&localComSeg);
	H323UpdateBaudRate();
	POBJDELETE(pLocalScmH323);	
}

/////////////////////////////////////////////////////////////////////////////
void   CH323PartyInSimulation::H323UpdateBaudRate()
{  
	DWORD totalRate = 0;
	CSegment* pSeg1 = new CSegment; 
	CSegment* pSeg2 = new CSegment;
    totalRate = m_pInitialModeH323->GetTotalBitRate(cmCapReceive);
	*pSeg1 << totalRate;
	m_pConfApi->UpdateDB(this,RECEIVEBAUDRATE,(DWORD) 0,1,pSeg1);
    totalRate = m_pInitialModeH323->GetTotalBitRate(cmCapTransmit);
	*pSeg2 << totalRate;
	m_pConfApi->UpdateDB(this,TRANSMITBAUDRATE,(DWORD) 0,1,pSeg2);
	POBJDELETE(pSeg1);
	POBJDELETE(pSeg2);
}



/////////////////////////////////////////////////////////////////////////////
//void CH323PartyIn::SetMcCallStruct()
//{
//	CH323Party::SetMcCallStruct();
//#ifdef __TCall__
//	m_pmcCallPtr->bIsOrigin	= FALSE;
//#endif
//}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnNetDisconnectSetUp(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyInSimulation::OnNetDisconnectSetUp : Name - ",PARTYNAME);
	CH323PartyInSimulation::OnNetDisconnect(pParam);
//	WORD  cause;
//	*pParam >> cause;
//	m_pConfApi->PartyDisConnect(PARTY_HANG_UP,this);
//	m_pConfApi->UpdateDB(this,DISCAUSE,cause,1); // Disconnnect cause
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnConfDisconnect(CSegment* pParam)
{

	CSmallString msg;

            msg << PARTYNAME << ". m_state = " << m_state;

            PTRACE2(eLevelInfoNormal, "CH323PartyInSimulation::OnConfDisconnect : Name - ", msg.GetString());

 

	
	//	CMedString msg;
//	msg << PARTYNAME << ". m_state = " << GetPartyStateAsString(PartyStateNo);
//	PTRACE2(eLevelInfoNormal, "CH323PartyInSimulation::OnConfDisconnect : Name - ", msg.GetString());

    CleanUp();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::CleanUp()
{
	m_state = PARTYDISCONNECTING;
	ON(m_isCleanup);
	H323CallDrop();
}


/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::H323CallDrop()
{
	PTRACE(eLevelInfoNormal,"CH323PartyInSimulation::H323CallDrop");

	CSegment sParam;
	sParam << (WORD)statOK;
	OnEndH323Disconnect(&sParam);
 }
   
/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnNetDisconnectConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyInSimulation::OnNetDisconnectConnect : Name - ",PARTYNAME);

#ifdef __Sinai__
    // for Sinai
	if (m_recordingManager && m_ivrCtrl) 
	{
		m_savepParam = new CSegment;
		*m_savepParam << (void*)pParam;		// save parameters
		m_ivrCtrl->PartyNetDisconnected(STOP_INITIATOR_NET);	// block activities
		NetStopRecording();	// initiator is NET
	}
	else
#endif
		CH323PartyInSimulation::OnNetDisconnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyInSimulation::OnNetDisconnect(CSegment* pParam)
{
	WORD  cause;
	*pParam >> cause;
	m_pConfApi->PartyDisConnect(PARTY_HANG_UP,this); 
	m_pConfApi->UpdateDB(this,DISCAUSE,cause,1); // Disconnnect cause
}


