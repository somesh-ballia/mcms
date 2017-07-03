//+========================================================================+
//                            H323PartyIn.CPP                              |
//            Copyright 1995 Polycom Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       H323PartyIn.CPP                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Uri                                                         |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 19/7/05     |                                                     |
//+========================================================================+


#include "H323PartyIn.h"
#include "ConfPartyOpcodes.h"
#include "Lobby.h"
#include "LobbyApi.h"
#include "PartyApi.h"
#include "ConfApi.h"
#include "ConfPartyGlobals.h"
#include "encrAuth.h"
#include "ConfPartyRoutingTable.h"
#include "OpcodesMcmsCommon.h"
#include "TraceStream.h"

#ifndef MCMS
#define MCMS 5
#endif

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void H323PartyInEntryPoint(void* appParam)
{  
	CH323PartyIn*  pPartyTaskApp = new CH323PartyIn;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}



PBEGIN_MESSAGE_MAP(CH323PartyIn)
		// lobby events  
  ONEVENT(LOBBYNETIDENT  ,PARTYIDLE   ,CH323PartyIn::OnLobbyH323IdentIdle)
  ONEVENT(LOBBYTRANS     ,PARTYSETUP  ,CH323PartyIn::OnLobbyTransferSetup)
  ONEVENT(REJECTCALL	 ,PARTYIDLE   ,CH323PartyIn::OnLobbyRejectCallIdle)
  ONEVENT(LOBBYDESTROY	 ,PARTYSETUP  ,CH323PartyIn::OnConfDisconnect) 
  ONEVENT(LOBBYDESTROY	 ,PARTYIDLE   ,CH323PartyIn::OnConfDisconnect) 
  ONEVENT(LOBBYDESTROY   ,PARTYDISCONNECTING,CH323PartyIn::NullActionFunction)
  ONEVENT(LOBBYDESTROY   ,PARTYCHANGEMODE   ,CH323PartyIn::NullActionFunction)
  ONEVENT(LOBBYDESTROY	 ,PARTYCONNECTED  	,CH323PartyIn::NullActionFunction)
		// conf events
  ONEVENT(CONFDISCONNECT ,ANYCASE     ,CH323PartyIn::OnConfDisconnect)

  ONEVENT(H323ESTABLISHCALL  ,PARTYSETUP   		,CH323PartyIn::OnConfEstablishCallSetup)

  ONEVENT(DISCONNECTH323     ,PARTYSETUP  		,CH323PartyIn::OnNetDisconnectSetUp)
  ONEVENT(DISCONNECTH323	 ,PARTYCONNECTED  	,CH323PartyIn::OnNetDisconnectConnect)
  ONEVENT(DISCONNECTH323	 ,PARTYCHANGEMODE	,CH323PartyIn::OnNetDisconnectConnect)
  ONEVENT(DISCONNECTH323     ,PARTYDISCONNECTING,CH323PartyIn::NullActionFunction)  
  ONEVENT(ENDH323DISCONNECT  ,PARTYIDLE  		,CH323PartyIn::OnEndH323Disconnect)
  ONEVENT(ENDH323DISCONNECT  ,PARTYSETUP 		,CH323PartyIn::OnEndH323Disconnect)
  ONEVENT(ENDH323DISCONNECT  ,PARTYDISCONNECTING,CH323PartyIn::OnEndH323Disconnect)
  ONEVENT(ENDH323DISCONNECT  ,PARTYCHANGEMODE   ,CH323PartyIn::OnEndH323Disconnect)

  ONEVENT(PARTY_TRANSLATOR_ARTS_CONNECTED,	    PARTY_ALLOCATE_TRANSLATOR_ARTS,	CH323PartyIn::OnPartyTranslatorArtsConnected)
  ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED,   PARTY_TRANSLATOR_ARTS_DISCONNECTING, CH323PartyIn::OnPartyTranslatorArtsDisconnected)
  ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED,   PARTYCONNECTED, CH323PartyIn::OnPartyTranslatorArtsDisconnectedDeescalating)

  // Self timers
  ONEVENT(PARTYCONTOUT  	 ,PARTYIDLE   		,CH323PartyIn::OnH323DisconnectIdleSetUp)
  ONEVENT(PARTYCONTOUT  	 ,PARTYSETUP 		,CH323PartyIn::OnH323DisconnectIdleSetUp)
  ONEVENT(PARTYCONTOUT  	 ,ANYCASE			,CH323PartyIn::NullActionFunction)
        //party events

PEND_MESSAGE_MAP(CH323PartyIn,CH323Party);   



//void AllocateConnectionID(DWORD &connectionID) {}
void DeAllocateConnectionID(DWORD &connectionID){}

/////////////////////////////////////////////////////////////////////////////
CH323PartyIn::CH323PartyIn()
{
	m_pLobbyApi		= new CLobbyApi; 
	m_pPartyInApi   = new CPartyApi;
	
	OFF(m_isReject);
	OFF(m_isLobbySetup);
	m_interfaceType = H323_INTERFACE_TYPE;

	m_rejectCallReason	= -1;
	m_state				= PARTYIDLE;
	m_DialInRejectConnectionId = 0xFFFFFFFF;
	OFF(m_isAddPartyOK);
	VALIDATEMESSAGEMAP;        
}

/////////////////////////////////////////////////////////////////////////////
CH323PartyIn::~CH323PartyIn()
{
	m_pLobbyApi->DestroyOnlyApi();
	POBJDELETE(m_pLobbyApi);

	m_pPartyInApi->DestroyOnlyApi();
	POBJDELETE(m_pPartyInApi);

	m_pConfApi->DestroyOnlyApi();
	POBJDELETE(m_pConfApi);
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyIn::Create(CSegment& appParam)
{
	CH323Party::Create(appParam);

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
void  CH323PartyIn::OnLobbyTransferSetup(CSegment* pParam)//shiraITP - 58
{
	PTRACE2(eLevelInfoNormal,"CH323PartyIn::OnLobbyTransferSetup : Party Name - ",PARTYNAME);

	WORD  mode,rc = 0;
	WORD  returnSyncStatus = 0;
	*pParam >> mode;

	m_pConfApi->CreateOnlyApi(*m_pConfRcvMbx,NULL,NULL,1);	
	CSegment rspMsg;
	
	switch (mode)
	{
	case PARTYTRANSFER  :  
		// sync call to conf		
		rc = m_pConfApi->AddInH323Party(m_pH323NetSetup,this, *m_pRcvMbx, m_name,  //shiraITP - 59
										PARTY_TRANSFER_TOUT, rspMsg);
		if (rc == 0)
		{
            rspMsg >> returnSyncStatus;
		}
		if (rc)
		{
			PTRACE(eLevelError,"CH323PartyIn::OnLobbyTransferSetup : \'EXPORT PARTY FAILED (TimeOut) !!!\'");
			m_pLobbyApi->PartyTransfer(this,statTout);
			OFF(m_isLobbySetup);
			ON(m_isReject);
			if (IsValidTimer(PARTYCONTOUT))
				DeleteTimer(PARTYCONTOUT);
			return;
		}
		
		else if (returnSyncStatus)
		{
			PTRACE(eLevelError,"CH323PartyIn::OnLobbyTransferSetup : \'EXPORT PARTY FAILED (Conf Reject) !!!\'");
			m_pLobbyApi->PartyTransfer(this, returnSyncStatus);
			OFF(m_isLobbySetup);
			ON(m_isReject);
			if (IsValidTimer(PARTYCONTOUT))
				DeleteTimer(PARTYCONTOUT);
			return;
		}
		
		else
		{
			PTRACE(eLevelInfoNormal,"CH323PartyIn::OnLobbyTransferSetup : \'EXPORT PARTY O.K. !!!\'");
			m_pLobbyApi->PartyTransfer(this,statOK);  
		}  
		break;

	default   :
		DBGPASSERT((DWORD)this);
		break;
	}
	
	OFF(m_isLobbySetup); // shout down the flag tha indicates of lobby responsibilities
	OFF(m_isReject); //caution just in case in any senario that I saw if we've got here the flag is off.
	ON(m_isAddPartyOK);// to check if we failed to allocate resources on the bridges (Audio, Video, Data)
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyIn::OnLobbyH323IdentIdle(CSegment* pParam) //shiraITP - 54
{
	PTRACE2(eLevelInfoNormal,"CH323PartyIn::OnLobbyH323IdentIdle : Name - ",PARTYNAME);

	DWORD generator;
	DWORD  hkLen;
	WORD hkType;
	DWORD bIsEncrypted = 0;
#ifdef __AUTHENTICATION__	
	BYTE sid[2];
	int indexTblAuth;
	DWORD dIndexTbl;
	DWORD authLen	= 0;
	DWORD encLen	= 0;

	BYTE *pAuthKey	= NULL;
	BYTE *pEncKey	= NULL;
#endif
	WORD  status = statOK;

	m_pH323NetSetup->DeSerialize(NATIVE,*pParam);
	*pParam >> bIsEncrypted;
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
#endif
	generator	= m_pH323NetSetup->GetGenerator();
	hkLen		= m_pH323NetSetup->GetHkLen();
	hkType		= m_pH323NetSetup->GetHkType();	
	
	m_pH323Cntl->m_pDHKeyManagement->SetGenerator(generator);

	m_pH323Cntl->CreateForAccept(this, *m_pH323NetSetup, *m_pLocalCapH323,NULL, 0, NULL, m_serviceId, m_isAutoVidBitRate, m_RoomId,m_linkType);

	if(hkLen)
	{
		CDHKey *pRmtSharedSecret	= new CDHKey; 
		pRmtSharedSecret->SetArray(m_pH323NetSetup->GetHalfKey(), m_pH323NetSetup->GetHkLen());
		m_pH323Cntl->m_pDHKeyManagement->SetDHRmtSharedSecret(*pRmtSharedSecret);
		m_pH323Cntl->m_pDHKeyManagement->SetRmtHalfKeyAlg(hkType);
		
		if( bIsEncrypted)
		{
			if(hkType == kHalfKeyDH1024)
			{
				// Getting the halfKey and RandomNumber from the EncryptionKeyServer process
				BYTE isAllocOk = statOK;
				isAllocOk = AllocateLocalHalfKey(NO);
				if (isAllocOk != statOK) // Failed to allocate the HalfKey for some reason
				{
					m_pLobbyApi->PartyIdent(this,0,statIllegal);
					ON(m_isLobbySetup);
					ON(m_isReject);
					POBJDELETE(pRmtSharedSecret);
					if (IsValidTimer(PARTYCONTOUT))
						DeleteTimer(PARTYCONTOUT);
					//m_pConfApi->CreateOnlyApi(*m_pConfRcvMbx,NULL,NULL,1);
					//m_pConfApi->DropParty(m_name, 0, NO_DISCONNECTION_CAUSE);
					return;	
				}
				CalculateSharedSecret(m_pH323Cntl->m_pDHKeyManagement);
				m_pH323Cntl->m_pDHKeyManagement->SetEncCallKey(m_pH323Cntl->m_pDHKeyManagement->GetDHResultSharedSecret()->GetArray(),
														(m_pH323Cntl->m_pDHKeyManagement->GetDHResultSharedSecret()->GetLength() - XMIT_RCV_KEY_LENGTH));
			}
			else
				PTRACE2(eLevelError,"CH323PartyIn::OnLobbyH323IdentIdle - remote opened key different from DH1024: Name - ",PARTYNAME);
		}

		POBJDELETE(pRmtSharedSecret);
	}
#ifdef __AUTHENTICATION__
	if(pAuthKey)
		m_bIsAuthenticated = TRUE;
	elsehkLen
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
void  CH323PartyIn::OnLobbyRejectCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyIn::OnLobbyRejectCallIdle : Name - ",PARTYNAME);
	m_pH323NetSetup->DeSerialize(NATIVE,*pParam); 
  
	WORD reason;
	*pParam >> reason;

	if(reason == TimerForUnreservedCall)
	{
		reason = cmReasonTypeNoPermision;
	}

	ON(m_isReject);

	m_rejectCallReason = (int)reason;

	m_pH323Cntl->CreateForReject(this, *m_pH323NetSetup, *m_pLocalCapH323, NULL, 0, NULL, m_serviceId, m_isAutoVidBitRate, m_RoomId);  //dialin
	m_pH323Cntl->SetOrign(FALSE);
	m_state = PARTYSETUP;
	ON(m_isLobbySetup);
	if (IsValidTimer(PARTYCONTOUT))
	{
		PTRACE(eLevelInfoNormal,"CH323PartyIn::OnLobbyRejectCallIdle - Delete PARTYCONTOUT");
		DeleteTimer(PARTYCONTOUT);
	}
	
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyIn::OnEndH323Disconnect(CSegment* pParam)
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

    if(m_pH323Cntl->IsAtLeastOneInternalTranslatorArtConnected())
    {
        TRACEINTO << "mix_mode: Disconnect internal ARTs";
        m_pDisconnectParams = new CSegment(*pParam);
        m_state = PARTY_TRANSLATOR_ARTS_DISCONNECTING;
        m_pH323Cntl->CloseTranslatorArts();
    }
    else
    {
        WORD status;
        *pParam >> status;

        WORD isRejected = 0;
        m_pPartyInApi->DestroyOnlyApi();

        if (!m_isReject && !m_isLobbySetup)
            m_pConfApi->PartyEndDisConnect(GetPartyId(), status); // notify conference
        else
            isRejected = 1;

        EndH323Disconnect(isRejected);
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyIn::OnConfEstablishCallSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyIn::OnConfEstablishCallSetup : Name - ",PARTYNAME);
	OFF(m_isAddPartyOK);
	WORD halfKeyAlg, encAlg;
	// catalog self party api in resource table
	m_pPartyInApi->CreateOnlyApi(GetRcvMbx(),this);
	m_pPartyInApi->SetLocalMbx(GetLocalQueue());
	CH323NetSetup* pH323NetSetup = new CH323NetSetup;

	 CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
     for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	 {
    	  avcToSvcTranslatorRsrcParams[i]=NULL;
	 }

	OnConfEstablishH323Call(pParam, encAlg, halfKeyAlg, pH323NetSetup,avcToSvcTranslatorRsrcParams);

    // class CH323Cntl was already created at CH323PartyIn::OnLobbyH323IdentIdle
	// different from CH323PartyOut
	m_pH323Cntl->AddToRoutingTable();
	m_pH323Cntl->UpdateLocalCapH323(*m_pLocalCapH323);
	m_pH323Cntl->UpdateTargetMode(m_pInitialModeH323);
	m_pH323Cntl->UpdateNetSetUp(*pH323NetSetup);

	m_pH323Cntl->SetEncrAlgType((EenMediaType)encAlg);
	m_pH323Cntl->m_pDHKeyManagement->SetLocalHalfKeyAlg(halfKeyAlg);

	if (IsValidTimer(PARTYCONTOUT))
	{
		PTRACE(eLevelInfoNormal,"CH323PartyIn::OnConfEstablishCallSetup - Delete PARTYCONTOUT");
		DeleteTimer(PARTYCONTOUT);
	}
	POBJDELETE(pH323NetSetup);	
//	m_pH323Cntl->SetState(SETUP);

	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    {
	   POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
    }

	if(m_pInitialModeH323->GetConfMediaType() == eMixAvcSvc)
	{
	    m_state = PARTY_ALLOCATE_TRANSLATOR_ARTS;
		PTRACE2(eLevelInfoNormal,"CH323PartyIn::OnConfEstablishCallSetup - Need to open new ARTS : Name - ",PARTYNAME);
		int numOfArtsToOpen = m_pH323Cntl->OpenInternalArts(E_NETWORK_TYPE_IP);
		if (numOfArtsToOpen == 0)
		{
			OnPartyTranslatorArtsConnected();
		}

		return;
	}

	// IF YOU NEED TO ADD MORE LOGIC, ADD IT INSIDE THIS FUNCTION!!!
	ContinueEstablishCall();
}
/////////////////////////////////////////////////////////////////////////////
void CH323PartyIn::OnPartyTranslatorArtsConnected()
{
    TRACEINTO << "@#@";
	m_state = PARTYSETUP;
	ContinueEstablishCall();
}


/////////////////////////////////////////////////////////////////////////////
void CH323PartyIn::ContinueEstablishCall()
{
    TRACEINTO << "@#@";
	m_pH323Cntl->OnPartyCallAnswerDialIn(); //get logic port from stack
}

/////////////////////////////////////////////////////////////////////////////
//void CH323PartyIn::SetMcCallStruct()
//{
//	CH323Party::SetMcCallStruct();
//#ifdef __TCall__
//	m_pmcCallPtr->bIsOrigin	= FALSE;
//#endif
//}

/////////////////////////PARTYCHANGEMODE////////////////////////////////////////////////////
void  CH323PartyIn::OnNetDisconnectSetUp(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyIn::OnNetDisconnectSetUp : Name - ",PARTYNAME);
	CH323PartyIn::OnNetDisconnect(pParam);
//	WORD  cause;
//	*pParam >> cause;
//	m_pConfApi->PartyDisConnect(PARTY_HANG_UP,this);
//	m_pConfApi->UpdateDB(this,DISCAUSE,cause,1); // Disconnnect cause
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyIn::OnConfDisconnect(CSegment* pParam)
{
	CMedString msg;
	msg << PARTYNAME << ". m_state = " << GetPartyStateAsString(PartyStateNo);
	PTRACE2(eLevelInfoNormal, "CH323PartyIn::OnConfDisconnect : Name - ", msg.GetString());

    CleanUp();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyIn::CleanUp()
{
	
	m_state = PARTYDISCONNECTING;
	ON(m_isCleanup);
	H323CallDrop();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyIn::H323CallDrop()
{
	PTRACE(eLevelInfoNormal,"CH323PartyIn::H323CallDrop");

	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);

	if( m_isLobbySetup || m_isReject )
	{
		if (m_DialInRejectConnectionId == 0xFFFFFFFF)
		{
			::AllocateRejectID(m_DialInRejectConnectionId);
			m_pH323Cntl->SetConnectionIdForReject(m_DialInRejectConnectionId);
		}
		
		m_pH323Cntl->OnPartyCallAnswerDialInFailure(m_rejectCallReason);
	}
	else if(m_isAddPartyOK)//if there no resources on the bridges
	{
		PTRACE(eLevelInfoNormal,"CH323PartyIn::H323CallDrop - Failed to Resources or connect to the MPL_API");

		::AllocateRejectID(m_DialInRejectConnectionId);
		m_pH323Cntl->SetConnectionIdForReject(m_DialInRejectConnectionId);

		m_pH323Cntl->OnPartyCallAnswerDialInFailure(int(cmReasonTypeNoBandwidth));
	}
	else
		m_pH323Cntl->OnPartyCallDropDialIn();
}

   
/////////////////////////////////////////////////////////////////////////////
void  CH323PartyIn::OnNetDisconnectConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323PartyIn::OnNetDisconnectConnect : Name - ",PARTYNAME);

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
		CH323PartyIn::OnNetDisconnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyIn::OnNetDisconnect(CSegment* pParam)
{
	WORD  cause;
	*pParam >> cause;
	TRACESTR(eLevelInfoNormal) << "CH323PartyIn::OnNetDisconnect : disconnect cause :  - " << cause;
	m_pConfApi->PartyDisConnect(PARTY_HANG_UP,this); 
	m_pConfApi->UpdateDB(this,DISCAUSE,cause,1); // Disconnnect cause
//	m_pConfApi->UpdateDB(this,DISCAUSE,PARTY_HANG_UP,1);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyIn::OnH323DisconnectIdleSetUp(CSegment* pParam)
{
	PASSERTMSG(m_PartyRsrcID,"CH323PartyIn::OnH323DisconnectIdleSetUp : ");

	// The m_isPreSignalingFlowProb indicates that we are disconnecting the call before the signaling
	m_isPreSignalingFlowProb = 1;
	// In case we already allocated and registered the CS and MFA we need to remove them from the 
	// routing table to avoid duplicate party entrence in the routing table
	m_pH323Cntl->RemoveFromRsrcTbl();
	::AllocateRejectID(m_DialInRejectConnectionId);
	m_pH323Cntl->SetConnectionIdForReject(m_DialInRejectConnectionId);

	BYTE 	mipHwConn = (BYTE)eMipConnectionProcess;
	BYTE	mipMedia = (BYTE)eMipNoneMedia;
	BYTE	mipDirect = (BYTE)eMipIn;
	BYTE	mipTimerStat = (BYTE)eMipTimer;
	BYTE	mipAction = (BYTE)eMipConnect;
	
	if( m_isLobbySetup || m_isReject)// the responsibility is still under the lobby.
	{
		PTRACE(eLevelInfoNormal,"CH323PartyIn::OnH323DisconnectIdleSetUp : party is under lobby responsibility");
		CleanUp();
	}
	else if(m_isAddPartyOK)//GetRcvMbx
	{
		PTRACE(eLevelInfoNormal,"CH323PartyIn::OnH323DisconnectIdleSetUp : party is under conf responsibility");
		m_pConfApi->SendFaultyMfaNoticeToPartyCntl(GetPartyId(), STATUS_FAIL,mipHwConn,mipMedia,mipDirect,mipTimerStat,mipAction);
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CH323PartyIn::OnH323DisconnectIdleSetUp : self kill");
		WORD isReject = 1;
		EndH323Disconnect(isReject);
	}
}	
	

