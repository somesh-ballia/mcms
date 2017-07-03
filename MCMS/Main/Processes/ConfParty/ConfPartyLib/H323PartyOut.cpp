//+========================================================================+
//                            PARTYOUT.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PARTYOUT.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 3/17/96     |                                                     |
//+========================================================================+


#include "H323PartyOut.h"
#include "ConfApi.h"
#include "ConfPartyOpcodes.h"
#include "CallAndChannelInfo.h"
#include "ConfPartyGlobals.h"
#include "OpcodesMcmsCommon.h"
#include "IpCommon.h"



#ifndef MCMS
#define MCMS 5
#endif

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void PartyH323OutEntryPoint(void* appParam)
{ 
	CH323PartyOut*  pPartyTaskApp = new CH323PartyOut;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CH323PartyOut)

                // conf events
  ONEVENT(H323ESTABLISHCALL  ,PARTYIDLE		,CH323PartyOut::OnConfH323EstablishCallPartyIdle)
//  ONEVENTA(H323ESTABLISHCALL  ,SETUP	,(AFUNC)CH323PartyOut::OnConfEstablishCallIdleH323  ,"H323ESTABLISHCALL","SETUP")
//  ONEVENTA(CONFDISCONNECT	  ,IDLE     ,(AFUNC)CH323PartyOut::OnConfDisconnectIdle     ,"CONFDISCONNECT","IDLE")
  ONEVENT(CONFDISCONNECT	  ,ANYCASE  ,CH323PartyOut::OnConfDisconnect)
//                // party events
//  ONEVENTA(DISCONNECT,IDLE,(AFUNC)CH323PartyOut::OnNetDisconnectConnect,"DISCONNECT","IDLE")
//  ONEVENTA(DISCONNECT,CONNECT,(AFUNC)CH323PartyOut::OnNetDisconnectConnect,"DISCONNECT","CONNECT")
//  ONEVENTA(DISCONNECT,CHANGEMODE ,(AFUNC)CH323PartyOut::OnNetDisconnectChangeMode    ,"DISCONNECT","CHANGEMODE")
  ONEVENT(DISCONNECTH323,PARTYDISCONNECTING       ,CH323PartyOut::NullActionFunction)
  ONEVENT(DISCONNECTH323,ANYCASE      ,CH323PartyOut::OnH323PartyDisconnect)
//  ONEVENTA(DISCONNECTH323,IDLE       ,(AFUNC)CH323PartyOut::OnH323PartyDisconnect    ,"DISCONNECTH323","IDLE")
//  ONEVENTA(DISCONNECTH323,SETUP      ,(AFUNC)CH323PartyOut::OnH323PartyDisconnect    ,"DISCONNECTH323","SETUP")
//  ONEVENTA(DISCONNECTH323,CONNECT    ,(AFUNC)CH323PartyOut::OnH323PartyDisconnect    ,"DISCONNECTH323","CONNECT")
//  ONEVENTA(DISCONNECTH323,CHANGEMODE ,(AFUNC)CH323PartyOut::OnH323PartyDisconnect    ,"DISCONNECTH323","CHANGEMODE")
  ONEVENT(ENDH323DISCONNECT  ,ANYCASE  ,CH323PartyOut::OnEndH323Disconnect)
  ONEVENT(ENDH323DISCONNECT  ,PARTYIDLE     ,CH323PartyOut::OnEndH323Disconnect)
//  ONEVENTA(ENDH323DISCONNECT  ,CONNECT  ,(AFUNC)CH323PartyOut::OnEndH323Disconnect,"ENDH323DISCONNECT","CONNECT")
//  ONEVENTA(ENDH323DISCONNECT  ,DISCONNECTING  ,(AFUNC)CH323PartyOut::OnEndH323Disconnect,"ENDH323DISCONNECT","DISCONNECTING")
//  ONEVENTA(ENDH323DISCONNECT  ,CHANGEMODE     ,(AFUNC)CH323PartyOut::OnEndH323Disconnect,"ENDH323DISCONNECT","CHANGEMODE")

  ONEVENT(TIMER_START_IVR ,ANYCASE	  ,CParty::OnTimerStartIvr)		///anat - temporary - until Sagi fixes timer  	

	// Self timer
  ONEVENT(PARTYCONTOUT  	,PARTYIDLE		,CH323PartyOut::OnH323DisconnectIdle)
  ONEVENT(PARTYCONTOUT	  	,ANYCASE  		,CH323PartyOut::NullActionFunction)

  ONEVENT(PARTY_TRANSLATOR_ARTS_CONNECTED,	    PARTY_ALLOCATE_TRANSLATOR_ARTS,	CH323PartyOut::OnPartyTranslatorArtsConnected)
  ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED,   PARTY_TRANSLATOR_ARTS_DISCONNECTING, CH323PartyOut::OnPartyTranslatorArtsDisconnected)
  ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED,   PARTYCONNECTED, CH323PartyOut::OnPartyTranslatorArtsDisconnectedDeescalating)

PEND_MESSAGE_MAP(CH323PartyOut,CH323Party);   


/////////////////////////////////////////////////////////////////////////////
CH323PartyOut::CH323PartyOut() 
{
//	char fullname[30]	="7.256/mcu/h323/logfile.log";
//	char dirname[15]	="7.256/mcu/h323";
//	WORD retval;
	
	VALIDATEMESSAGEMAP;      
//	m_pTpktTask						= NULL;
//	m_pRsrcTbl						= NULL;
//	m_pReqCallSetup				= NULL;
//	m_pReqCallAnswer			= NULL;
//	m_pReqCreateCntl			= NULL;
//	m_pReqOutChnl					= NULL;
//	m_pReqInChnlResponse	= NULL;
//	m_pReqStreamOn				= NULL;
//	m_pReqVideoUpdatePic	= NULL;
	m_interfaceType = H323_INTERFACE_TYPE;
	m_state			= PARTYIDLE;
	m_disconnectionCause = 0;
	
//#ifndef WINNT
//	
//	retval=make_dir(dirname,0);
//	if(retval != 0x2011 && retval !=0)
//	{
//		PASSERT(1);
//		PTRACE(eLevelError,"CH323PartyOut::CH323PartyOut : make_dir() failure");
//	}       
//	retval = remove_f(fullname);
//	
//	
//#endif
	
//	m_pMuxCntl->Suspend(1); //Meanwhile The H323 parties don't need a MuXCntl        	
}

/////////////////////////////////////////////////////////////////////////////
CH323PartyOut::~CH323PartyOut() 
{
}


/////////////////////////////////////////////////////////////////////////////
void CH323PartyOut::Create(CSegment& appParam)
{
	CH323Party::Create(appParam);

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif
	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx, NULL, NULL, 1);

	// update initial number of retries
	DWORD redialnum = GetSystemCfgFlagInt<DWORD> (CFG_KEY_NUMBER_OF_REDIAL);
	m_pConfApi->UpdateDB(this, NUMRETRY, (DWORD) redialnum, 1);
}

/////////////////////////////////////////////////////////////////////////////
void*  CH323PartyOut::GetMessageMap()                                        
{
	return (void*)m_msgEntries;    
}

/*
/////////////////////////////////////////////////////////////////////////////
TCall*  CH323PartyOut::GetMcCallPtr()                                        
{
	return m_pmcCallPtr;    
}
*/        
/////////////////////////////////////////////////////////////////////////////
void CH323PartyOut::SetCallId(CH323NetSetup* pH323NetSetup)
{
	  char aCallId[16];
	  const mcTransportAddress* pSrcPartyAddr = pH323NetSetup->GetTaSrcPartyAddr();
	  DWORD serIpAddr = 0;
	  if ( pH323NetSetup->GetIpVersion() == eIpVersion4)
	  {
	  	serIpAddr = pSrcPartyAddr->addr.v4.ip; 
	  }
	  else
	  {
	  	memcpy(&serIpAddr, pSrcPartyAddr->addr.v6.ip,4);
	  }
	
	  ::CalculateUniqueNumber(aCallId, serIpAddr);
	  pH323NetSetup->SetCallId(aCallId);
}
/*

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnConfDisconnectIdle(CSegment* pParam)
{
	DWORD	msgLen = MediumPrintLen + strlen(PARTYNAME) + 1;

	ALLOCBUFFER(str,msgLen);

	sprintf(str, "CH323PartyOut::OnConfDisconnectIdle : Name=%s, state=%d", PARTYNAME,m_state);
	PTRACE(eLevelInfoNormal, str);

    DEALLOCBUFFER(str);

    CleanUp();
}
*/

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnConfDisconnect(CSegment* pParam)
{
	CMedString msg;
	msg << PARTYNAME << ". m_state = " << GetPartyStateAsString(PartyStateNo);
	PTRACE2(eLevelInfoNormal, "CH323PartyOut::OnConfDisconnect : Name - ", msg.GetString());

	m_state = PARTYDISCONNECTING;
	ON(m_isCleanup);
	H323CallDrop();
//	DisconnectH323Cntl();                
//     CleanUp();
}

/*
/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::CleanUp()
{
	PTRACE2(eLevelInfoNormal,"CH323PartyOut::CleanUp : Name - ",PARTYNAME);
	m_state = DISCONNECTING;
	ON(m_isCleanup);
	DisconnectH323Cntl();                
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::DisconnectH323Cntl()
{
	PTRACE2(eLevelInfoNormal,"CH323PartyOut::DisconnectH323Cntl : Name - ",PARTYNAME);
	H323CallDrop();
}
*/

/////////////////////////////////////////////////////////////////////////////
void CH323PartyOut::OnEndH323Disconnect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

    if(m_pH323Cntl->IsAtLeastOneInternalTranslatorArtConnected())
    {
        m_pDisconnectParams = new CSegment(*pParam);
        m_state = PARTY_TRANSLATOR_ARTS_DISCONNECTING;
        m_pH323Cntl->CloseTranslatorArts();
    }
    else
    {
        m_pConfApi->PartyEndDisConnect(GetPartyId(), m_disconnectionCause);  // notify conference

        EndH323Disconnect();
    }
}

///////////////////////////////////////////////////////////////////////////////shiraITP - 14 - CH323PartyOut::OnConfH323EstablishCallPartyIdle
void  CH323PartyOut::OnConfH323EstablishCallPartyIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyOut::OnConfH323EstablishCallPartyIdle : Name - ",PARTYNAME);
//	InitTimer(GetRcvMbx());
	WORD halfKeyAlg, encAlg;
	CH323NetSetup* pH323NetSetup = new CH323NetSetup;

	CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		avcToSvcTranslatorRsrcParams[i]=NULL;
    }


	OnConfEstablishH323Call(pParam, encAlg, halfKeyAlg, pH323NetSetup,avcToSvcTranslatorRsrcParams);

#ifdef __NETSETUP_AT_H323CNTL__
	//the netsetup initialization will be done in the signaling level (currently what used to be H323Cntl) 
/*	const char * srcIpAddress = pH323NetSetup->GetSrcPartyAddress();
	if(srcIpAddress[0] == 0)
	{
		char srcAddr[300]="TA:";
		//sagia , memory leak.
		//strncat(srcAddr,GetH323IPaddress(m_pIpDesc->GetBoardId()),20);

		char *tmp = (char*) GetH323IPaddress(m_pIpDesc->GetBoardId());
		strncat(srcAddr,tmp,20);
		PDELETE(tmp);

		strncat(srcAddr,":",2);
		pH323NetSetup->SetSrcPartyAddress(srcAddr);
	}
*/
#endif

// IpV6

    SetCallId(pH323NetSetup);
    // create class CH323Cntl
	m_pH323Cntl->CreateForAccept(this, *pH323NetSetup, *m_pLocalCapH323, m_pInitialModeH323,1,NULL, m_serviceId, m_isAutoVidBitRate, m_RoomId,m_linkType);
	m_pH323Cntl->AddToRoutingTable();

	
// if the parameters need to be at the H323cntl the init of it should be in the H323cntl as well.
	m_pH323Cntl->m_pDHKeyManagement->SetLocalHalfKeyAlg(halfKeyAlg);
	m_pH323Cntl->SetEncrAlgType((EenMediaType)encAlg);

	if(halfKeyAlg == kHalfKeyDH1024)
	{
		BYTE isAllocOk = statOK;
		// Getting the halfKey and RandomNumber from the EncryptionKeyServer process
		isAllocOk = AllocateLocalHalfKey(YES);
		if (isAllocOk != statOK)
			return;
	}
	POBJDELETE(pH323NetSetup);
	if (IsValidTimer(PARTYCONTOUT))
	{
		PTRACE2(eLevelInfoNormal,"CH323PartyOut::OnConfH323EstablishCallPartyIdle - Delete PARTYCONTOUT: Name - ",PARTYNAME);
		DeleteTimer(PARTYCONTOUT);
	}
	
	m_pH323Cntl->SetOrign(TRUE);

	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
	}
	if(m_pInitialModeH323->GetConfMediaType() == eMixAvcSvc)
	{
		m_state = PARTY_ALLOCATE_TRANSLATOR_ARTS;
		PTRACE2(eLevelInfoNormal,"CH323PartyOut::OnConfH323EstablishCallPartyIdle - Need to open new ARTS : Name - ",PARTYNAME);
		int numOfArtsToOpen = m_pH323Cntl->OpenInternalArts(E_NETWORK_TYPE_IP);
		if (numOfArtsToOpen == 0)
		{
			OnPartyTranslatorArtsConnected(NULL);
		}

		return;
	}

    m_state = PARTYSETUP;

 	// IF YOU NEED TO ADD MORE LOGIC, ADD IT INSIDE THIS FUNCTION!!!
    ContinueEstablishCall();
}
/////////////////////////////////////////////////////////////////////////////
void CH323PartyOut::OnPartyTranslatorArtsConnected(CSegment* pParam)
{
	 m_state = PARTYSETUP;
	 ContinueEstablishCall();
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyOut::ContinueEstablishCall()
{
    TRACEINTO << "@#@";
    m_pH323Cntl->OnPartyGetPortReq(); //get logic port from stack  //shiraITP - 15  //????
}


/*
/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnPartyProceeding(CSegment* pParam)
{
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnPartyConnected(CSegment* pParam)
{
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnPartyCallConnected(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CH323PartyOut::OnPartyCallConnected : Name - ",PARTYNAME);
}
*/

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::H323CallDrop()
{
	PTRACE(eLevelInfoNormal,"CH323PartyOut::H323CallDrop");
	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);
	
	if (m_pH323Cntl->IsCreated() && m_isPreSignalingFlowProb == 0)
       	m_pH323Cntl->OnPartyCallDropReq();
	else
	{
		WORD status = 0;
		CSegment* pSeg = new CSegment;
		*pSeg << status;
		DispatchEvent(ENDH323DISCONNECT, pSeg);
		POBJDELETE(pSeg);	
	}
}

/*
/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnNetDisconnectConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyOut::OnNetDisconnectConnect : Name - ",PARTYNAME);
// 	CH323P// artyOut::OnNetDisconnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnNetDisconnect(CSegment* pParam)
{
	BYTE  cause;
	*pParam >> cause;
	m_pConfApi->PartyDisConnect(PARTY_HANG_UP,this);
	m_pConfApi->UpdateDB(this,DISCAUSE,cause,1); // Disconnnect cause	
}


/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnNetDisconnectChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyOut::OnNetDisconnectChangeMode : Name - ",PARTYNAME);
	WORD  seqNum;
	BYTE  cause;
	*pParam >> seqNum >> cause;
	if ( seqNum > 0 )  // addtional channel disconnected
		
		m_pConfApi->PartyEndChangeMode(this,m_pTargetComMode,statIllegal);
	else
	{             // initial channel disconnected
		m_pConfApi->AudioActive(this,0,MCMS);
		m_pConfApi->PartyDisConnect(0,this);    
	} 
}
*/

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::OnH323PartyDisconnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyOut::OnH323PartyDisconnect : Name - ",PARTYNAME);
	WORD  cause;
	*pParam >> cause;
	m_disconnectionCause = cause;
	
	m_pConfApi->PartyDisConnect(cause,this); 
	m_pConfApi->UpdateDB(this,DISCAUSE,cause,1); // Disconnnect cause  
}


/////////////////////////////////////////////////////////////////////////////
void  CH323PartyOut::SetVideoRate(DWORD vidRate)
{
	m_videoRate = vidRate; 
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyOut::OnH323DisconnectIdle(CSegment* pParam)
{
	PASSERTMSG(m_PartyRsrcID,"CH323PartyOut::OnH323DisconnectIdle : ");
	// The m_isPreSignalingFlowProb indicates that we are disconnecting the call before the signaling
	m_isPreSignalingFlowProb = 1;
	WORD disconnectionCause = STATUS_FAIL;

	BYTE 	mipHwConn = (BYTE)eMipConnectionProcess;
	BYTE	mipMedia = (BYTE)eMipNoneMedia;
	BYTE	mipDirect = (BYTE)eMipOut;
	BYTE	mipTimerStat = (BYTE)eMipTimer;
	BYTE	mipAction = (BYTE)eMipConnect;
	
	m_pConfApi->SendFaultyMfaNoticeToPartyCntl(GetPartyId(), STATUS_FAIL,mipHwConn,mipMedia,mipDirect,mipTimerStat,mipAction);
}



