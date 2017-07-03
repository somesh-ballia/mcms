//+========================================================================+
//                            IPPRTCTL.CPP                                 |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IPPRTCTL.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date       | Description                                         |
//-------------------------------------------------------------------------|
// Atara| 04/03/03   | Ip party control include h323 and SIP party controls|
//+========================================================================+

#include "IpPartyControl.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
#include "ConfPartyGlobals.h"

#include "ConfApi.h"
#include "CommModeInfo.h"
#include "BridgePartyVideoParams.h"
#include "BridgePartyAudioParams.h"
#include "BridgePartyInitParams.h"
#include "FECCBridgePartyInitParams.h"
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "FECCBridge.h"
#include "BridgeMoveParams.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgePartyVideoRelayMediaParams.h"

#include "Conf.h"
#include "Trace.h"
#include "IpCommon.h"

#include "OpcodesMcmsInternal.h"
#include "H264Util.h"
#include "PartyApi.h"
#include "VideoBridge.h"
#include "VideoRelayInOutMediaStream.h"
#include "AvcToSvcParams.h"
#include "IpServiceListManager.h"
#include "MsSvcMode.h"
#include "vp8VideoMode.h"

#define REASON_CONNECT 1
#define REASON_UPDATE  2
#include "SysConfig.h"
#include "SysConfigKeys.h"

#include "EnumsToStrings.h"
#include "string.h"
#include "ConfigHelper.h"
#include "Party.h"
#ifdef PERFORMANCE_TEST
	#include "Stopper.h"
	#include "PrettyTable.h"
#endif

extern CIpServiceListManager* GetIpServiceListMngr();
extern const char* MediaStateToString(eConfMediaState confState);
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

PBEGIN_MESSAGE_MAP(CIpPartyCntl)
  ONEVENT(VIDREFRESH						,ANYCASE	 ,CIpPartyCntl::OnPartyRefreshVideoAnycase)
  ONEVENT(REQUEST_PREVIEW_INTRA             ,ANYCASE	 ,CIpPartyCntl::OnPartyEventModeVideoPreviewRefreshVideoAnycase)
  ONEVENT(PARTY_IN_CONF_IND					,ANYCASE 	 ,CIpPartyCntl::OnCAMUpdatePartyInConf)
  ONEVENT(FECC_PARTY_BRIDGE_CONNECTED  		,ANYCASE	 ,CIpPartyCntl::OnFeccBrdgCon)
  ONEVENT(FECC_PARTY_BRIDGE_DISCONNECTED	,ANYCASE 	 ,CIpPartyCntl::OnFeccBridgeDisConnect)

  ONEVENT(IP_SEND_INTRA_TO_PARTY_TOUT,      ANYCASE	 ,CIpPartyCntl::OnSendIntraToPartyTout)

  ONEVENT(CHANGE_MODE_LOOP                 ,ANYCASE           ,CIpPartyCntl::OnTimerPartyChangeModeLoop)


PEND_MESSAGE_MAP(CIpPartyCntl,CPartyCntl);

#define isCapInCapRange(_lowRange, _highRange, _cap) ((_cap <= _highRange) && (_cap >= _lowRange))
#define isSirenLPRCap(_cap) isCapInCapRange(eSirenLPR_32kCapCode, eSirenLPRStereo_128kCapCode, _cap)

/////////////////////////////////////////////////////////////////////////////

CIpPartyCntl::CIpPartyCntl()
{
	//PTRACE(eLevelInfoNormal,"CIpPartyCntl::CIpPartyCntl()");
	m_pIpInitialMode	= NULL;
	m_pIpCurrentMode	= NULL;
	m_pCopVideoTxModes	= NULL;
//	m_pVerParam			= NULL;
	ON(m_bNoContentChannel);
	m_videoRate			= 0;
//	m_newVideoContRate	= 0;
	m_conferenceContentRate = 0;


	m_avChangeModeRequest = NO;
	m_avNewModeRequest	  = 0;
	m_bIsNewScm	   		  = NO;
	m_isTipFallbackFlow   = NO;
	m_eUpdateState		  = eNoUpdate;

	m_pOriginalIpScm= NULL;
	m_pQos			= NULL;

	m_pSecondaryParams = NULL;
	m_SecondaryCause   = SECONDARY_CAUSE_DEFAULT;

	m_strConfParamInfo	 = "";

	m_changeModeInitiator = eNoInitiator;

	m_isFaulty = FALSE;
	m_isRecovery = FALSE;
	m_currentVideoEncRate = 0;
	OFF(m_presentationStreamOutIsUpdated);

    m_entryPoint = NULL;
    m_McuNumber =0;
    m_isRecording = 0;
    m_isStreaming = 0;
    m_pExchangeServerConfId = NULL;
    m_password = NULL;
    m_confName = NULL;

	// VNGFE-787
	m_isCodianVcr = 0;

    m_isLprActive	= 0;
    m_isContentDba	= 0;
    m_isHDVSWEnabled = 0;

    m_numOfHotbackupRedial = GetSystemCfgFlagInt<DWORD>(CFG_KEY_HOT_BACKUP_NUMBER_OF_REDIAL_ATTEMPTS);
    m_bIsFirstConnectionAfterHotBackupRestore = FALSE;

    m_eLastCopChangeModeParam = COP_decoder_resolution_Last;
    m_eLastCopChangeModeType = eCop_DecoderParams;
    m_lastCopForceEncoderLevel = INVALID_COP_LEVEL;
    m_eFirstRxVideoCapCode = eUnknownAlgorithemCapCode;
    m_isSentH239Out	= 0;

    m_lastConnectionRate = 0;

    m_eFirstRxVideoProfile  = H264_Profile_BaseLine;
    m_copResourceIndexOfCascadeLinkLecturer = INVALID_COP_LEVEL;
    m_bCascadeIsLecturer = FALSE;

    m_incomingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
    m_outgoingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
    m_isSignalingFirstTransactionCompleted = false;

    m_lastMinContentRate = DISPLAY_MIN_CONTENT_RATE_FLAG;
	m_isBframeRscEnabled = TRUE;
	m_NumOfChangeModesInSec = 0;
	m_bVideoRelayOutReady = FALSE;
	m_bVideoRelayInReady = FALSE;
	m_bVideoUpdateCount=VIDEO_UPDATE_COUNT0;
	m_pMrmpRsrcParams = NULL;
    	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    	{
    		m_avcToSvcTranslatorRsrcParams[i]=NULL;
    	}
    m_ConnectedToVideoAsLegacy = FALSE;
	m_bIsNeedToChangeVideoOutTipPolycom=FALSE; //_t_p_
	m_bIsBridgeUpgradedAudio = true;
	m_bIsBridgeUpgradedVideo = true;
	m_isAudioDecoderUpdateNeeded = eMediaTypeUpdateNotNeeded;
	m_pendingScmForUpgrade = NULL;
	m_pendingScmForChangeMode = NULL;
	memset(&m_udpAddresses, 0, sizeof(UdpAddresses));

	VALIDATEMESSAGEMAP
}

/////////////////////////////////////////////////////////////////////////////
CIpPartyCntl::~CIpPartyCntl()
{
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::Destroy()
{
	POBJDELETE(m_pIpInitialMode);
	POBJDELETE(m_pIpCurrentMode);
	POBJDELETE(m_pCopVideoTxModes);
	POBJDELETE(m_pOriginalIpScm);
	POBJDELETE(m_pQos);
	POBJDELETE(m_pSecondaryParams);
	PDELETEA(m_pExchangeServerConfId);
	POBJDELETE(m_pMrmpRsrcParams);
	POBJDELETE(m_pendingScmForUpgrade);
	POBJDELETE(m_pendingScmForChangeMode);
    	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    	{
	  POBJDELETE(m_avcToSvcTranslatorRsrcParams[i]);
    	}

	CPartyCntl::Destroy();
}

/////////////////////////////////////////////////////////////////////////////
CIpPartyCntl& CIpPartyCntl::operator=(const CIpPartyCntl& other)
{
	if(this != &other)
	{
		(CPartyCntl&)*this		= (CPartyCntl&)other;

		m_pIpInitialMode		= other.m_pIpInitialMode;
		m_pIpCurrentMode		= other.m_pIpCurrentMode;
		m_pCopVideoTxModes		= other.m_pCopVideoTxModes;
//		m_pVerParam				= other.m_pVerParam;
		m_bNoContentChannel		= other.m_bNoContentChannel;
     	m_videoRate				= other.m_videoRate;
//		m_newVideoContRate		= other.m_newVideoContRate;
		m_originalConfRate		= other.m_originalConfRate;
		m_avChangeModeRequest	= other.m_avChangeModeRequest;
		m_avNewModeRequest		= other.m_avNewModeRequest;
		m_bIsNewScm 			= other.m_bIsNewScm;
		m_isTipFallbackFlow		= other.m_isTipFallbackFlow;
		m_pOriginalIpScm        = other.m_pOriginalIpScm;
		m_pQos					= other.m_pQos;
		m_pSecondaryParams		= other.m_pSecondaryParams;
		m_SecondaryCause		= other.m_SecondaryCause;

		m_strConfParamInfo		= other.m_strConfParamInfo;

		m_presentationStreamOutIsUpdated = other.m_presentationStreamOutIsUpdated;

		//Multiple links for ITP in cascaded conference feature:
		m_eTelePresenceMode      = other.m_eTelePresenceMode;
		if (CPObject::IsValidPObjectPtr(m_telepresenseEPInfo))
		    POBJDELETE(m_telepresenseEPInfo);
		if (other.m_telepresenseEPInfo != NULL)
		{
		    PTRACE2(eLevelInfoNormal,"CPartyCntl::operator= ",m_name);
		    m_telepresenseEPInfo  = new CTelepresenseEPInfo();
		    *m_telepresenseEPInfo = *(other.m_telepresenseEPInfo);
		}

		// VNGFE-787
		m_isCodianVcr 			= other.m_isCodianVcr;
		m_isLprActive			= other.m_isLprActive;
		m_isContentDba			= other.m_isContentDba;
		m_isHDVSWEnabled		= other.m_isHDVSWEnabled;
		m_pExchangeServerConfId = other.m_pExchangeServerConfId;
		m_numOfHotbackupRedial 					  = other.m_numOfHotbackupRedial;
		m_bIsFirstConnectionAfterHotBackupRestore = other.m_bIsFirstConnectionAfterHotBackupRestore;

		m_eLastCopChangeModeParam	= other.m_eLastCopChangeModeParam;
		m_eLastCopChangeModeType 	= other.m_eLastCopChangeModeType;
		m_lastCopForceEncoderLevel 	= other.m_lastCopForceEncoderLevel;
		m_eFirstRxVideoCapCode 		= other.m_eFirstRxVideoCapCode;

		m_isSentH239Out				= other.m_isSentH239Out;
		m_lastConnectionRate    	= other.m_lastConnectionRate;

		m_eFirstRxVideoProfile      = other.m_eFirstRxVideoProfile;
		m_copResourceIndexOfCascadeLinkLecturer = other.m_copResourceIndexOfCascadeLinkLecturer;
		m_bCascadeIsLecturer = other.m_bCascadeIsLecturer;
		m_isBframeRscEnabled = other.m_isBframeRscEnabled;
		m_NumOfChangeModesInSec  =  other.m_NumOfChangeModesInSec;

	    m_incomingVideoChannelHandle = other.m_incomingVideoChannelHandle;
	    m_outgoingVideoChannelHandle = other.m_outgoingVideoChannelHandle;
	    m_isSignalingFirstTransactionCompleted = other.m_isSignalingFirstTransactionCompleted;
	    m_operationPointsSet         = other.m_operationPointsSet;
        m_SsrcIdsForAvcParty         = other.m_SsrcIdsForAvcParty;


		//==================================
        // Not copying, forcing re-logging
        //==================================
        m_lastMinContentRate			= DISPLAY_MIN_CONTENT_RATE_FLAG;
        m_bVideoRelayOutReady = other.m_bVideoRelayOutReady;
        m_bVideoRelayInReady = other.m_bVideoRelayInReady;
        m_bVideoUpdateCount=other.m_bVideoUpdateCount;

        m_pMrmpRsrcParams = other.m_pMrmpRsrcParams;
    	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    	{
	  m_avcToSvcTranslatorRsrcParams[i]=other.m_avcToSvcTranslatorRsrcParams[i];
    	}

        m_ConnectedToVideoAsLegacy = other.m_ConnectedToVideoAsLegacy;
        m_bIsNeedToChangeVideoOutTipPolycom = other.m_bIsNeedToChangeVideoOutTipPolycom; //_t_p_

        m_bIsUseSpeakerSsrcForTx_TRUE_sent = other.m_bIsUseSpeakerSsrcForTx_TRUE_sent;

        m_isAudioDecoderUpdateNeeded = other.m_isAudioDecoderUpdateNeeded;

        POBJDELETE(m_pendingScmForUpgrade);
        if (other.m_pendingScmForUpgrade)
        	MoveScmToPendingWhileUpgrading(other.m_pendingScmForUpgrade);
        POBJDELETE(m_pendingScmForChangeMode);
        if (other.m_pendingScmForChangeMode)
        	MoveScmToPendingWhileChangeMode(other.m_pendingScmForChangeMode);
	}
	return *this;
}

void CIpPartyCntl::MoveScmToPendingWhileUpgrading(CIpComMode* incomingScm)
{
		if (!m_pendingScmForUpgrade)
			m_pendingScmForUpgrade = new CIpComMode(*incomingScm);
		else
			*m_pendingScmForUpgrade = *incomingScm;
}

void CIpPartyCntl::MoveScmToPendingWhileChangeMode(CIpComMode* incomingScm)
{
		if (!m_pendingScmForChangeMode)
			m_pendingScmForChangeMode = new CIpComMode(*incomingScm);
		else
			*m_pendingScmForChangeMode = *incomingScm;
}

/////////////////////////////////////////////////////////////////////////////
/*BYTE CIpPartyCntl::IsQuad() const
{
	if(m_videoPlusType && (GetSubCP_ConfType() == eSubCPtypeAdvanced))
		return YES;
	else
		return NO;
}*/
/////////////////////////////////////////////////////////////////////////////
BYTE  CIpPartyCntl::IsSecondary() const
{
	BYTE bIsSecondary = NO;
	// not audio only and there are open video channels
	bIsSecondary = ((m_voice == NO) &&
		((m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople)) ||
		(m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))));

	return bIsSecondary;
}
/////////////////////////////////////////////////////////////////////////////////////////////
/*BYTE CIpPartyCntl::IsCOP() const
{
	if (m_pScm->IsFreeVideoRate()) //CP
		return((m_videoPlusType == eSINGLE_PORT) || (m_videoPlusType == eDUAL_PORT));
	else //VSW
		return FALSE;
}*/


///////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetNewVideoRates(DWORD videoRate)
{
    SetVideoRate(videoRate);
	m_pIpInitialMode->SetVideoBitRate(videoRate,cmCapReceiveAndTransmit);
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdateCurrentModeNoMedia(WORD channelType, ERoleLabel eRole)
{
    m_pIpCurrentMode->SetMediaOff((cmCapDataType)channelType,cmCapReceiveAndTransmit,eRole);
    UpdateCurrentModeInDB();
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::UpdateCurrentModeNoVideo()
{
    m_pIpCurrentMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
    m_pIpCurrentMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
	UpdateCurrentModeInDB();
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::SetPartyToAudioOnly()
{

	// set SCM to audio only.
    m_pIpInitialMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit, kRolePeople);
    m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
    m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

    // set DB to audio only
    // When updating the party status to audio only in DB, we should call UpdateDB because we need to update the number of video participants in CommConf as well
    //CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
    //pConfParty->SetVoice(YES);
    CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
    BYTE isPartyAlreadySetToVoice = NO;
    if(IsValidPObjectPtr(pConfParty))
    	isPartyAlreadySetToVoice = pConfParty->GetVoice();
    if (!isPartyAlreadySetToVoice)
    {
    	m_isAudioDecoderUpdateNeeded = eMediaTypeUpdateVideoToAudio;
    	m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_AUDIO_ONLY);
    }

    // set voice member
    m_voice = YES;
}


/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdateCurrentModeInDB()
{
    CSegment* pCurComSeg = new CSegment;
    m_pIpCurrentMode->Serialize(*pCurComSeg,cmCapTransmit,YES);
    m_pTaskApi->UpdateDB(m_pParty,LOCAL323COMMODE,(DWORD) 0,1,pCurComSeg);
    POBJDELETE(pCurComSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdateCurrentModeMediaOff(WORD channelType, ERoleLabel eRole,cmCapDirection direction)
{
	m_pIpCurrentMode->SetMediaOff((cmCapDataType)channelType,direction,eRole);
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdateInitialModeMediaOff(WORD channelType, ERoleLabel eRole,cmCapDirection direction)
{
	m_pIpInitialMode->SetMediaOff((cmCapDataType)channelType,direction,eRole);
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetPartyToSecondaryAndStopChangeMode(BYTE reason,DWORD details,BYTE direction,CSecondaryParams *pSecParams, BYTE bDisconnectChannels)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::SetPartyToSecondaryAndStopChangeMode: Name - ",m_partyConfName, GetPartyRsrcId());
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());

	if (pConfParty)
	{
	    DWORD partyState = pConfParty->GetPartyState();
	    if (partyState != PARTY_SECONDARY)
	    {
	        //since this function is the only place that changes the state in the operator to secondary,
	        //for ip party, we can ask this question
	        //1) Update party
	        SetPartySecondaryCause(reason,details,direction,pSecParams);

	        //2) Conf level (Monitoring + SetCapCommonDenominator) + update m_pIpCurrentMode
	        m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_SECONDARY);
	        UpdatePartyStateInCdr();
	        TRACEINTO << "after updateDB" ;
	        if (m_pIpInitialMode->GetConfType() != kCp && m_pIpInitialMode->GetConfType() != kCop)
	        {
	            //In COP: remove from the sub-group can be done only after disconnect from the bridge.
	            //In VSW: recalculate the scm will cause to send a new scm to this party, which
	            //		  should be done only after the party ends the disconnection from the bridge.
	            //		  Otherwise, there might be a contradiction between the change mode and secondary processes.
	            BYTE bUpdateConfLevel = TRUE;

	            /* After party ends the change mode, it reports to the conf EndChangeModeParty,
			which causes the conf to proceed with the move process,	in case the party is still
			not in change mode.
			BUT: The problem is when the party ends the change mode in secondary state.
			In this	case it also reports on the secondary to the conf in order that the conf will
			recalculate the scm => the party gets new scm and starts change mode.
			Therefore, the move will be actually done while the party is in change mode process
			(the new scm arrives to the party after the move process has continued.
			To prevent this conflict, we won't report here to conf.*/
	            if (IsConfWaitingToEndChangeModeForMove())
	                bUpdateConfLevel = FALSE;

	            if (bUpdateConfLevel)
	                m_pTaskApi->PartyChangeVidMode(m_pParty,TRUE);
	        }
	    }
	    else
	    {
	        PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::SetPartyToSecondaryAndStopChangeMode: Party is in secondary state in DB", GetPartyRsrcId());
	    }
	}
	else
	    DBGPASSERT(1111);

	if (bDisconnectChannels)
		ImplementSecondaryInPartyLevel();

	ImplementUpdateSecondaryInPartyControlLevel();

	ON(m_bNoContentChannel);

	//3) Save the rate in order to have the opportunity to change the party to connected in the future.
	DWORD initialRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);
	if (initialRate)
		m_videoRate = initialRate;

/*	//If for some reason we decided to close the video but the conference has T120 we should continue to open it.
	if (m_pIpCurrentMode->IsMediaOn(cmCapData))
		HandleData();*/

	// End change mode
	if (!m_bIsNewScm)
		if ((m_pIpInitialMode) && (m_pIpCurrentMode))
			*m_pIpInitialMode = *m_pIpCurrentMode; //in order to end the change mode
	m_state = IDLE; // in order to end the change mode, and to disconnect from the bridge, in case it's needed.
	// VNGR-6720 - In case the party was forced in VB - Remove the force
	m_pVideoBridgeInterface->DeletePartyFromConf(GetName());
	EndChangeMode();

	//Multiple links for ITP in cascaded conference feature: if one of the links was changed to secondary state -> disconnect all links.
	if ( (GetInterfaceType() == H323_INTERFACE_TYPE) && pConfParty && (pConfParty->GetPartyType() == eMainLinkParty|| pConfParty->GetPartyType() == eSubLinkParty) )
	{
	    PTRACE2INT(eLevelError,"ITP_CASCADE: CIpPartyCntl::SetPartyToSecondaryAndStopChangeMode disconnect linkType:",pConfParty->GetPartyType());
	    PTRACE2(eLevelError,"ITP_CASCADE: CIpPartyCntl::SetPartyToSecondaryAndStopChangeMode disconnect linkType:",pConfParty->GetName());

	    m_pTaskApi->PartyDisConnect(SUB_OR_MAIN_LINK_IS_SECONDARY,m_pParty);

	    if (pConfParty->GetPartyType() == eMainLinkParty)
	        m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,RESOURCES_DEFICIENCY,1); // update disconnnect cause
	}
}


/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::DispatchChangeModeEvent()
{
	CSegment* pParam = NULL;
	DispatchEvent(SCMCHANGEMODE, pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::EndChangeMode()
{
	PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::EndChangeMode - Error. Should call to derived class's function.", GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::RemoveSecondaryCause(BYTE bKeepH239SecondaryCause)
{
	BYTE bRemoveSecondaryCause = TRUE;
	if(bKeepH239SecondaryCause && (m_SecondaryCause != SECONDARY_CAUSE_DEFAULT))
	{
		if((m_SecondaryCause >= SECONDARY_CAUSE_H239_FIRST_OPCODE) && (m_SecondaryCause <= SECONDARY_CAUSE_H239_LAST_OPCODE))
		{//send update DB
			PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::RemoveSecondaryCause : Keep secondary Cause. Inform Conf.", GetPartyRsrcId());
	//		CSecondaryParams		secParams;
			CSegment				seg;
			if(m_pSecondaryParams != NULL)
				m_pSecondaryParams->Serialize(NATIVE,seg);
			m_pTaskApi->UpdateDB(m_pParty,SECONDARYCAUSE,m_SecondaryCause,1,&seg); // Disconnect cause
			bRemoveSecondaryCause = FALSE;
		}
	}

	if(bRemoveSecondaryCause)
	{
		PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::RemoveSecondaryCause : Reset Secondary Cause", GetPartyRsrcId());
		m_SecondaryCause   = SECONDARY_CAUSE_DEFAULT;
		POBJDELETE(m_pSecondaryParams);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::SetPartySecondaryCause(BYTE reason,WORD details,BYTE direction,CSecondaryParams *pSecParams)
{
	CMedString strBase;
	strBase<<"m_partyConfName = "<<m_partyConfName <<", reason = "<<reason<<", details = "<<details<<", direction = "<<direction
			<< ", m_SecondaryCause = "<< m_SecondaryCause;
	strBase<<", m_pSecondaryParams = "<< (DWORD)m_pSecondaryParams;

	CSegment*				seg = new CSegment;
	CSecondaryParams		secParams;
	// if the party level set the content into secondary and the conf level set the people into secondary, we preffered the people.
	BYTE bPrefferedPeopleOverContent = ((reason < SECONDARY_CAUSE_H239_FIRST_OPCODE) && (m_SecondaryCause >= SECONDARY_CAUSE_H239_FIRST_OPCODE));

	//There are cases we send reason from h323cntl before the party get to secondary and we want this reason to be print.
	if(m_SecondaryCause != SECONDARY_CAUSE_DEFAULT && !bPrefferedPeopleOverContent)
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::SetPartySecondaryCause, member secondary cause: ", strBase.GetString(), GetPartyRsrcId());
		reason = (BYTE)m_SecondaryCause;
		if (m_pSecondaryParams) //ask this second!!
			secParams = *m_pSecondaryParams;
	}

	else
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::SetPartySecondaryCause, create the secondary cause: ", strBase.GetString(), GetPartyRsrcId());
	switch(reason)
	{
	case SECONDARY_CAUSE_CONFERENCE_REJECT:
		m_pIpInitialMode->GetMediaParams(cmCapVideo,(cmCapDirection)direction,kRolePeople,secParams,details);
		CheckDetails(&details, &direction);
		if(details == 0)
		{
//			PTRACE2INT2(eLevelInfoNormal,"CIpPartyCntl::SetPartySecondaryCause: no details - reason = %d",reason);
			//secParams.m_problemParam		= UnKnown; //UnKnown
			reason = SECONDARY_CAUSE_NO_VIDEO_CONNECTION;
		}
		else
		{
			m_pIpCurrentMode->GetDiffFromDetails(cmCapVideo,(cmCapDirection)direction,kRolePeople,secParams,details);
			POBJDELETE(m_pSecondaryParams);
			m_pSecondaryParams = new CSecondaryParams;
			*m_pSecondaryParams = secParams;
		}
		break;
	case SECONDARY_CAUSE_CHANGE_MODE:
		m_pIpInitialMode->GetMediaParams(cmCapVideo,(cmCapDirection)direction,kRolePeople,secParams,details);
		if(details == 0)
		{
		//	PTRACE2INT2(eLevelInfoNormal,"CIpPartyCntl::SetPartySecondaryCause: no details - reason = %d",reason);
			secParams.m_problemParam		= UnKnown; //Unknown
		}
		else
			m_pIpCurrentMode->GetDiffFromDetails(cmCapVideo,(cmCapDirection)direction,kRolePeople,secParams,details);
		break;
	case SECONDARY_CAUSE_REMOTE_CAPABILITIES:
	case SECONDARY_CAUSE_H239_RMT_DIFF_CAPCODE:
//	case SECONDARY_CAUSE_SWCP_PROBLEM:
		if (pSecParams)//ask this first!!!
			secParams = *pSecParams;
		else if (m_pSecondaryParams) //ask this second!!
			secParams = *m_pSecondaryParams;
		break;
	case SECONDARY_CAUSE_CONFERENCING_LIMITATION:
	case SECONDARY_CAUSE_STREAM_VIOLATION:
		secParams = *pSecParams;
		break;
	case SECONDARY_CAUSE_VIDEO_PROBLEM:
	case SECONDARY_CAUSE_MOVE_PARTY:
//	case SECONDARY_CAUSE_COP_PROBLEM:
	case SECONDARY_CAUSE_NO_VIDEO_CONNECTION:
//	case SECONDARY_CAUSE_MOVE_DIFF_BITRATE:
	case SECONDARY_CAUSE_RMT_CLOSE_CHAN:
//	case SECONDARY_CAUSE_COP_REOPEN_OUT:
	case SECONDARY_CAUSE_RMT_DIFF_CAPCODE:
	case SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH:
	case SECONDARY_CAUSE_AVF_INSUFFICIENT_BANDWIDTH:
    case SECONDARY_CAUSE_H239_BW_MISMATCH:
    case SECONDARY_CAUSE_H239_INCOMPATIBLE_CAPS:
    case SECONDARY_CAUSE_H239_CONFERENCE_REJECT:
		//We only want to send the reason to operator.
		break;
    case SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD:
    case SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD:
    	secParams.m_problemParam = reason;
    	break;
    case SECONDARY_CAUSE_RMT_NOT_OPEN_AFTER_CHANGE_MODE:
		m_pIpInitialMode->GetMediaParams(cmCapVideo,(cmCapDirection)direction,kRolePeople,secParams,details);
		break;
	case SECONDARY_CAUSE_DEFAULT:
	  POBJDELETE(seg);
		return; //we already send the operator the reason of this action.
	default:
	//	PTRACE2INT2(eLevelInfoNormal,"CIpPartyCntl::SetPartySecondaryCause: reason %d not handle - ",reason);
	  POBJDELETE(seg);
		return;
	}
	}

	secParams.Serialize(NATIVE,*seg);

	m_pTaskApi->UpdateDB(m_pParty,SECONDARYCAUSE,reason,1,seg); // Disconnect cause

	if(m_SecondaryCause == SECONDARY_CAUSE_DEFAULT || bPrefferedPeopleOverContent)
	{
		m_SecondaryCause = reason;
		POBJDELETE(m_pSecondaryParams);
		m_pSecondaryParams = new CSecondaryParams;
		*m_pSecondaryParams = secParams;
	}

//	m_SecondaryCause   = SECONDARY_CAUSE_DEFAULT;
	POBJDELETE(seg);
//	POBJDELETE(m_pSecondaryParams);
}

/////////////////////////////////////////////////////////////////////////////
/*There are cases we move party to secondary because it has lower line rate but the conf level doesn't regard to the
//rate in the highest common's calculation, so we remove this protocol. So the actual reason this party moved to secondary is it's lower line rate
//and not what we have in the details. If this is the case I need to update the details to the actual reason*/
void  CIpPartyCntl::CheckDetails(WORD* pDetails, BYTE* direction) const
{
#ifdef __VSW_FIX_MODE__
	CapEnum protocol		= (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo,cmCapTransmit);
	CComModeInfo cmInfo		= protocol;
	WORD modeType			= cmInfo.GetH320ModeType();
	DWORD remoteCapsRate	= GetRemoteCapsVideoRate(modeType);

	BOOL bIsConfHasProtocol = FALSE;

	if(protocol == eH261CapCode)
		bIsConfHasProtocol = (m_pCap->OnDataVidCap(V_Cif) && m_pCap->OnDataVidCap(V_Qcif));
	else if (protocol == eH263CapCode)
		bIsConfHasProtocol = (m_pCap->IsH263());
	else
		bIsConfHasProtocol = TRUE;

	//If the conf does not have the protocol but the remote cap has the protocol, it means that we removed the protocol
	//from the conf because of the line rate.
	if ((!bIsConfHasProtocol) && (remoteCapsRate))
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckDetails - change the reason to HIGHER_BIT_RATE : Name - ",m_partyConfName, GetPartyRsrcId());
		*pDetails = 0;
		*pDetails |= HIGHER_BIT_RATE;
		*direction = cmCapReceive;//we want to show the low bit rate of the remote
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
/*void  CIpPartyCntl::HandleData()
{
	CapEnum currDataType	= (CapEnum)m_pIpCurrentMode->GetMediaMode(cmCapData).GetType();

	if (m_pIpCurrentMode->IsMediaOn(cmCapData))
	{
		if((currDataType == eAnnexQCapCode) || (currDataType == eRvFeccCapCode))
		{
			// To prevent from sending Connect request more than once.
			// We can not use the flag m_isMlpConn because it indicates only on bridge connect and then we could have
			// sent connect twice or more before we got the bridge connect.
			if (m_isDatConnReq == FALSE)
			{
				PTRACEPARTYID(FULL_TRACE,"CIpPartyCntl::HandleData - continue to open FECC", GetPartyRsrcId());
				WORD feccBitRate = m_pIpCurrentMode->GetMediaBitRate(cmCapData);
				m_pLsdCntl->Connect(m_name,m_pParty,m_pPartyApi->GetRcvMbx(),m_pMuxDesc);
				ON(m_isDatConnReq);
			}
			else
				PTRACEPARTYID(FULL_TRACE,"CIpPartyCntl::HandleData: Connect fecc was already sent.", GetPartyRsrcId());
		}
	}
}*/
/////////////////////////////////////////////////////////////////////////////
/*void  CIpPartyCntl::DisconnectDataChannels()
{
	if(m_pIpCurrentMode->IsMediaOn(cmCapData,cmCapReceiveAndTransmit,kRolePeople))
	{
		PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::DisconnectDataChannels - close data channels", GetPartyRsrcId());
		m_pPartyApi->IpDisconnectMediaChannel(cmCapData,cmCapReceiveAndTransmit,kRolePeople);
		m_pPartyApi->ChangeH323DataMode();
		UpdateCurrentModeNoMedia(cmCapData);
	}
}*/

/////////////////////////////////////////////////////////////////////////////
/*void  CIpPartyCntl::CloseFecc(BYTE bDisconnectDataChannels)
{
	PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::CloseFecc", GetPartyRsrcId());
	if (m_pIpCurrentMode->IsMediaOn(cmCapData))
	{
		if (m_pLsdCntl){
			if ( m_pLsdCntl->IsConnected() )
			{
				PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::CloseFecc - disconnect party from bridge", GetPartyRsrcId());
				OFF(m_isDatConnReq);
				m_pLsdCntl->DisConnect(m_pParty);        // sync call
				m_pTaskApi->UpdateDB(m_pParty,LSDCON,FALSE);
			}
		}

		if(bDisconnectDataChannels)
			DisconnectDataChannels();
	}

	PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::CloseFecc - update selected comunication mode", GetPartyRsrcId());
	m_pScm->m_lsdMlpMode.SetLsdMode(LSD_Off);
	m_pScm->m_lsdMlpMode.SetlsdCap(LSD_NONE);
	m_pInitialScm->m_lsdMlpMode.SetLsdMode(LSD_Off);
	m_pInitialScm->m_lsdMlpMode.SetlsdCap(LSD_NONE);
	m_pIpInitialMode->SetMediaOff(cmCapData,cmCapReceiveAndTransmit);
	m_pLocalCap->RemoveDataVidCap(Dxfer_Cap_6400);
	m_pLocalCap->RemoveDataVidCap(Dxfer_Cap_4800);
}*/

/////////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdatePartyAsAudioOnly()
{

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if (pConfParty)
	{
	    BYTE isPartyCurrentlyAudioOnlyInDB = pConfParty->GetVoice();
	    if(!isPartyCurrentlyAudioOnlyInDB)
	        m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_AUDIO_ONLY);
	    else
	        PTRACEPARTYID(eLevelInfoNormal, "CIpPartyCntl::UpdatePartyAsAudioOnly party already set as audio only in DB", GetPartyRsrcId());
	}
	else
	{
	    PTRACE(eLevelError, "CIpPartyCntl::UpdatePartyAsAudioOnly - pConfParty is NULL!!");
	    DBGPASSERT(1112);
	    return;
	}
	m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED);
	RemoveSecondaryCause(FALSE);// check if to keep content secondary cause.

	// In GL1 we update the CDR only regarding connect/disconnect state (we also update GK identifier (H323) and special Header (SIP)
	// Meaning that if we updated the CDR regarding the connection of the party -
	// There is no need to update again.
	UpdatePartyStateInCdr();
	UpdateCurrentModeNoVideo();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::DeleteBothOriginalScm()
{
	POBJDELETE(m_pOriginalIpScm);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//in used fro sip, so need to move to sip file
void CIpPartyCntl::SetConfParamInfo()
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();

	// supported-network
	if (pCommConf)
	{
		BYTE network = pCommConf->GetNetwork();
		switch (network)
		{
		case NETWORK_H323:
			m_strConfParamInfo << "i";
			break;
		case NETWORK_H320_H323:
			m_strConfParamInfo << "m";
			break;
		default:
			m_strConfParamInfo << (WORD)network;
		}
	}

	m_strConfParamInfo << ",";

	// conf-type
	if (m_voice)
		m_strConfParamInfo << "a-on";

//	else if (IsSoftwareCp())
//		m_strConfParamInfo << "sc";

//	else if (m_SubCPtype == eSubCPtypeAdvanced)
//		m_strConfParamInfo << "q";

//	else if (IsCOP())
//		m_strConfParamInfo << "co";

	else if (m_pIpInitialMode->GetConfType() == kCp)
		m_strConfParamInfo << "c";
	else if(m_pIpInitialMode->GetConfType() == kCop)
		m_strConfParamInfo << "e";  //e means event mode
	else // VSW
	{
		m_strConfParamInfo << "v=";
//		if (m_pScm->m_vidMode.IsAutoVidScm())//VSW auto
//			m_strConfParamInfo << "a";
//		else
//			m_strConfParamInfo << "f";
	}

	m_strConfParamInfo << ",";

	// conf-rate
	DWORD confRate = GetConfRate();//the conference rate shoud be the intersect call rate. m_pIpInitialMode->GetTotalBitRate(); // maybe this value is not true (need to check
	m_strConfParamInfo << confRate << ",";

	// aud-mode
	CComModeInfo cmInfo((WORD)m_pIpInitialMode->GetMediaBitRate(cmCapAudio),StartAudioCap);
	m_strConfParamInfo << cmInfo.GetH323ModeType() << ",";

	// video-type
	CCommConf* pCurrentConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	if (pCurrentConf)
	{
		CConfParty*	pConfParty = pCurrentConf->GetCurrentParty(m_monitorPartyId);
		if (pConfParty)
		{
		    BYTE protocol = pConfParty->GetVideoProtocol();
		    switch (protocol)
		    {
		    case H264 :
		        m_strConfParamInfo << "h4";
		        break;
		    case Video_ISO:
		        m_strConfParamInfo << "hL";
		        break;
		    case H263 :
		        m_strConfParamInfo << "h3";
		        break;
		    case H261 :
		        m_strConfParamInfo << "h1";
		        break;
		    case AUTO :
		        m_strConfParamInfo << "a";
		        break;
		    default:
		        m_strConfParamInfo << (WORD)protocol;
		    }
		}
		else
		    PTRACE(eLevelError, "CIpPartyCntl::SetConfParamInfo - pConfParty is NULL!!");
	}

	//add-param
//	if (m_pLocalCap->IsEnterprisePeopleContent())
//		m_strConfParamInfo << ",e";
//	else if (m_pLocalCap->IsH239Cap())
//		m_strConfParamInfo << ",h9";

	// feel free to add any param you want (fecc, t120, etc).
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::UpdateConfEndDisconnect(WORD  status)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateConfEndDisconnect : Name - ",m_partyConfName, GetPartyRsrcId());

	// Delete all the valid timer in case the party is only disconnected (dial out) and not deleted.
	if (IsValidTimer(BRIDGEDISCONNECT))
		DeleteTimer(BRIDGEDISCONNECT);
	if (IsValidTimer(PARTYDISCONNECTTOUT))
		DeleteTimer(PARTYDISCONNECTTOUT);

	m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_DISCONNECTED);
	UpdatePartyStateInCdr(); //m_pTaskApi->UpdatePartyStateInCdr(m_pParty)
	m_pTaskApi->EndDelParty(m_pParty, status);
}


/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::InitiateRedialIfNeeded()
{
	BOOL bIsHotBackupRedial = FALSE;
	if (m_bIsFirstConnectionAfterHotBackupRestore)
	{//check salve that become master:
		if ((m_numOfHotbackupRedial > 0) && IsRedialForHotBackup())
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::InitiateRedialIfNeeded: Redialing case After Hot Backup. Name - ",m_partyConfName, GetPartyRsrcId());
			m_numOfHotbackupRedial--;
			m_pTaskApi->UpdateDB(m_pParty,NUMRETRY,(DWORD) m_numOfHotbackupRedial,0);
			m_pTaskApi->RedailParty(m_pParty, TRUE);
			bIsHotBackupRedial = TRUE;
		}
		else //the meaning is only for consequence redialing
		{
			m_numOfHotbackupRedial = 0;
			m_bIsFirstConnectionAfterHotBackupRestore = FALSE;
		}
	}

	if (bIsHotBackupRedial == FALSE)
	{
	// redial if its dial out and the disconnection is because network problems.
		DWORD bIsEnableIpRedial; // = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_IP_REDIAL);
		bIsEnableIpRedial = m_pConf->GetCommConf()->GetIsAutoRedial();
	DWORD numOfRedials = GetSystemCfgFlagInt<DWORD>(CFG_KEY_NUMBER_OF_REDIAL);
	if(GetDialType() == DIALOUT)
	{
		if (IsRedialImmediately())
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::InitiateRedialIfNeeded: Redial immediatly. Name - ",m_partyConfName, GetPartyRsrcId());
			m_pTaskApi->RedailParty(m_pParty);
		}
		else if(bIsEnableIpRedial)
		{
				if ((m_redial > 0) &&
					(IsDisconnectionBecauseOfNetworkProblems() || IsDisconnectionBecauseOfInternalProblems()))
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::InitiateRedialIfNeeded: Redialing case . Name - ",m_partyConfName, GetPartyRsrcId());
				m_redial--;
				m_pTaskApi->UpdateDB(m_pParty,NUMRETRY,(DWORD) m_redial,0);
				m_pTaskApi->RedailParty(m_pParty);
			}
			else
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::InitiateRedialIfNeeded : Not a redial scenario, Name - ",m_partyConfName, GetPartyRsrcId());
				m_redial = numOfRedials;
				m_pTaskApi->UpdateDB(m_pParty,NUMRETRY,(DWORD) m_redial,1);
			}
		}
		else
		{
 			CMedString str;
			str << "dial type " << GetDialType();
			str << " disconnect cause " << m_disconnectionCause;
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::InitiateRedialIfNeeded: redial is not enabled, ", str.GetString(), GetPartyRsrcId());
		}
	}
	}

	if((GetDialType() != DIALOUT) || (IsRedialImmediately() == 0))// we set the disconnect cause for SIP parties only after we initiate the call.
		m_disconnectionCause = 0;
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::EndIpPartyDisconnect()
{
	PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::EndIpPartyDisconnect : Name - ",GetFullName(), GetPartyRsrcId());
	if (GetDisconnectMode())
		m_disconnectState = DISCONNECTED;

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
	m_pParty = (CTaskApp*)(m_monitorPartyId + 100);  // just to get unique id for party list
//m_pTaskApi->UpdateDB(m_pParty,PARTY_ENCRYPTION_STATE,NO);
//m_pTaskApi->UpdateDB(m_pParty,MUTE_STATE, 0x00000000); //indicate party audio is not muted by operator
//m_pTaskApi->UpdateDB(m_pParty,MUTE_STATE, 0x0000000E); //indicate party video is not muted by operator
//m_pTaskApi->UpdateDB(m_pParty,MUTE_STATE, 0xF0000000); //indicate party is not audio self muted
//m_pTaskApi->UpdateDB(m_pParty,MUTE_STATE, 0xF000000E); //indicate party is not video self muted
//m_pTaskApi->UpdateDB(m_pParty,MUTE_STATE, 0x0F000000); //indicate party audio is not muted by MCU
//m_pTaskApi->UpdateDB(m_pParty,MUTE_STATE, 0x0F00000E); //indicate party video is not muted by MCU
//m_pTaskApi->UpdateDB(m_pParty,BLOCK_STATE,0x00000000); //indicate party video is not blocked
//m_pTaskApi->UpdateDB(m_pParty, UPDATE_EXCLUSIVE_CONTENT, FALSE); // set off Restricted content
	CSegment* pSpeakerRequestSeg = new CSegment;
	BYTE onOff = 0;
	*pSpeakerRequestSeg << m_name << onOff;
	m_pTaskApi->UpdateDB(NULL,PARTY_REQUEST_TO_SPEAK,(DWORD) 0,1,pSpeakerRequestSeg);
	POBJDELETE(pSpeakerRequestSeg);
	WORD  status = statOK;
	if (m_isRecover)
	{
		status = m_disconnectMode;
		OFF(m_isRecover);
	}

	UpdateConfEndDisconnect(status);
	//for CDR event: PARTICIPANT_DISCONNECT_INFORMATION
	//	PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::EndIpPartyDisconnect:m_maxConnectionRateCurrently- ",m_maxConnectionRateCurrently );
	//	PTRACE2INT(eLevelInfoNormal,"CH323AddPartyCntl::EndConnectionProcess: m_maxFormatCurrently - ",m_maxFormatCurrently);
	//	PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::EndIpPartyDisconnect: m_maxFrameRateCurrently- ",m_maxFrameRateCurrently );

	/*char tempName[64];
	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	ipToString(pConfParty->GetIpAddress(),tempName,1);
	if (pConfParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE)
		strcpy(tempName, pConfParty->GetSipPartyAddress() );*/

	//Don't send for audio-only participant for VNGR-24111
	if ( !(IsRemoteCapNotHaveVideo() || m_voice))
	{
		m_pTaskApi->UpdateCallInfoInCdr(m_pParty,m_maxConnectionRateCurrently,m_maxFormatCurrently,
				                     m_maxFrameRateCurrently);
	}
	InitiateRedialIfNeeded();
}

void  CIpPartyCntl::InitVideoInParams(CBridgePartyVideoInParams *pMediaInParams, CIpComMode* pScm)
{
	InitVideoParams(cmCapReceive, pMediaInParams, pScm);
}

/////////////////////////////////////////////////////////////////////////////
// in that function we also adjust between the MCMS and CS default value for static MB (-1)
// to the DSP default value for static MB (0)
void  CIpPartyCntl::InitVideoOutParams(CBridgePartyVideoOutParams *pMediaOutParams, CIpComMode* pScm, BYTE isLprInitiate)
{
	if (pScm->GetConfType() == kCop)
	{
		// Set Cop Level:
		pScm->Dump("CIpPartyCntl::InitVideoOutParams scm ",eLevelInfoNormal);
		WORD  indxCopVideoLevel = 0;
		indxCopVideoLevel = pScm->GetCopTxLevel();
		pMediaOutParams->SetCopResourceIndex(indxCopVideoLevel);
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitVideoOutParams: indxCopVideoLevel = ",indxCopVideoLevel);
	}

	pMediaOutParams->SetIsCascadeLink(GetPartyCascadeType() == CASCADE_MCU);
	InitVideoParams(cmCapTransmit, pMediaOutParams, pScm, isLprInitiate);

	// Set static MB according to system flag.
	DWORD staticMB = DEFAULT_STATIC_MB;
	GetStaticMbForDsp(pScm, staticMB);
	pMediaOutParams->SetStaticMB(staticMB);

	if (m_pIpInitialMode->GetConfType() == kCp)
		UpdateAudioDelay(pMediaOutParams);

	InitVideoOutTipPolycomIfNeeded(pMediaOutParams, pScm); //_t_p_
}

void CIpPartyCntl::UpdateAudioDelay(CBridgePartyVideoOutParams *pMediaOutParams)
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioDelay - begin");

	DWORD dwFS = pMediaOutParams->GetFS();
	DWORD dwMBPS = pMediaOutParams->GetMBPS();

	if (m_iAudioDelayUpdated < 2 && ((dwFS >= 15 && dwMBPS >= 216)  ||
			IsConfTelePresenceMode()))
	{
		DWORD audioDecoderCompressedDelay = 0; // default value
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		std::string key = "AUDIO_DECODER_COMPRESSED_DELAY";
		sysConfig->GetDWORDDataByKey(key, audioDecoderCompressedDelay);

		//Set---decoder
		TAudioUpdateCompressedAudioDelayReq st;

		st.bnCompressedAudioDelay = 1;
		st.nCompressedAudioDelayValue = audioDecoderCompressedDelay;

		PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioDelay - (dwFS >= 15 && dwMBPS >= 216 || m_eTelePresenceMode != eTelePresencePartyNone) && m_iAudioDelayUpdated < 2");

		if (m_pAudioInterface->UpdateAudioDelay(GetPartyRsrcId(), &st))
		{
			m_iAudioDelayUpdated = 2;
			PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioDelay - OK");
		}

	}
	else if (m_iAudioDelayUpdated == 2 && (dwFS < 15 || dwMBPS < 216) && !IsConfTelePresenceMode() )
	{
		TAudioUpdateCompressedAudioDelayReq st;

		st.bnCompressedAudioDelay = 0;
		st.nCompressedAudioDelayValue = 0;

		PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioDelay - (dwFS < 15 || dwMBPS < 216) && m_eTelePresenceMode != eTelePresencePartyNone && m_iAudioDelayUpdated == 2");

		if (m_pAudioInterface->UpdateAudioDelay(GetPartyRsrcId(), &st))
		{
			m_iAudioDelayUpdated = 1;
			PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioDelay - OK");
		}
	}

	PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioDelay - end");
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::InitVideoParams(cmCapDirection direction, CBridgePartyVideoParams *pMediaParams, CIpComMode* pScm, BYTE isLprInitiate)
{
	WORD  videoBridgeAlgorithm = 0;
	DWORD videoBridgeBitRate   = 0;

	WORD  videoBridgeQcifFormat    = 0;
	int   videoBridgeQcifFrameRate = 0;
	WORD  videoBridgeCifFormat     = 0;
	int   videoBridgeCifFrameRate  = 0;
	WORD  videoBridge4CifFormat    = 0;
	int   videoBridge4CifFrameRate = 0;

	WORD videoBridgeFormat = 0;

	eVideoProfile videoBridgeProfile = eVideoProfileBaseline;

	DWORD videoBridgeFs       = 0;
	DWORD videoBridgeMbps     = 0;
	DWORD videoBridgeSar      = 0;
	DWORD videoBridgeStaticMB = 0;
	DWORD videoBridgeDPB      = 0;

	eVideoPacketPayloadFormat videoBridgePacketFormat =  eVideoPacketPayloadFormatSingleUnit;

	pMediaParams->MsSvcParams().Clear();

	DWORD msPartyRsrcId = 0;
	DWORD msAudioLocalMsi = 0;
	DWORD msAvMcuIndex =0;
	CCommConf* pCurrentConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);

	CapEnum algorithm = (CapEnum)(pScm->GetMediaType(cmCapVideo, direction));

	CMedString strBase;
	strBase<<"algorithm = "<< algorithm <<", direction = "<< direction;
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams :: ", strBase.GetString(), GetPartyRsrcId());
	// TIP
	BYTE bIsTipMode = pScm->GetIsTipMode();

	std::ostringstream ostrin;
	ostrin << "CIpPartyCntl::InitVideoParams:   m_conferenceContentRate = : " << m_conferenceContentRate << " Video MediaBitRate:" << (DWORD)m_pIpInitialMode->GetMediaBitRate(cmCapVideo, direction);
	PTRACE(eLevelInfoHigh,ostrin.str().c_str());

	CLargeString string1;
    string1 << "CIpPartyCntl::InitVideoParams:   m_conferenceContentRate = " <<  m_conferenceContentRate << " "
    		<< ", m_pIpInitialMode->GetMediaBitRate(cmCapVideo, direction) = " << (DWORD)m_pIpInitialMode->GetMediaBitRate(cmCapVideo, direction)
    		<< ", Name - " << m_partyConfName;
    PTRACE (eLevelInfoNormal, string1.GetString());

	if (direction == cmCapTransmit) // If content is already opened - open the video encoder in correct rate.
	{
		if (GetInterfaceType() == H323_INTERFACE_TYPE)
		{
			CMedString str;

			if (m_isLprActive == 0)
			{// LPR not active

				PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams :  (NO LPR) ", GetPartyRsrcId());
			}
			else
			{
				PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams :  (LPR Case) ", GetPartyRsrcId());
			}

            DWORD totalVideoRate	= m_pIpInitialMode->GetTotalVideoRate();
            DWORD currentVideoRate	= m_pIpInitialMode->GetMediaBitRate(cmCapVideo, direction);

			TRACESTR(eLevelInfoNormal) << " CIpPartyCntl::InitVideoParams :  isLprInitiate - " << (DWORD)isLprInitiate << ", totalVideoRate - " << totalVideoRate;
            if(isLprInitiate)
            	videoBridgeBitRate 	= currentVideoRate;// LPR already set the the people rate, if conference will re-calculate content rate, let it change the encoder again.
            else
            {
            	if(totalVideoRate < m_conferenceContentRate + 640)
            		videoBridgeBitRate = 640; // 64000 100bit/sec is the minumum rate is people.
            	else
            		videoBridgeBitRate = totalVideoRate - m_conferenceContentRate;

           		videoBridgeBitRate 	= min(videoBridgeBitRate, currentVideoRate);
            }
            videoBridgeBitRate 	= videoBridgeBitRate * 100;
			TRACESTR(eLevelInfoNormal) << " CIpPartyCntl::InitVideoParams :  chosen video rate - " << videoBridgeBitRate << ", Name - " << m_partyConfName;

			if ( !strncmp(m_productId, "RADVision ViaIP GW", strlen("RADVision ViaIP GW")) )
			{
				DWORD bitRateReductionPrc = 0; // default value
				CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				std::string key = "RV_GW_VIDEO_RATE_REDUCTION_PERCENTAGE";
				sysConfig->GetDWORDDataByKey(key, bitRateReductionPrc);
				if(bitRateReductionPrc)
				{
					bitRateReductionPrc = 100 - bitRateReductionPrc;
					videoBridgeBitRate = ((bitRateReductionPrc * videoBridgeBitRate) / 100 );
				}

				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams - only for RV GW lowring transmit rate to decoder!",videoBridgeBitRate);
			}
		}
		else
		{
			videoBridgeBitRate = 100 * (m_pIpInitialMode->GetMediaBitRate(cmCapVideo, direction) - m_conferenceContentRate);
		}

		TRACESTR(eLevelInfoNormal) << " Flora debug CIpPartyCntl::InitVideoParams :  Init rate - " << m_pIpInitialMode->GetMediaBitRate(cmCapVideo, direction) << ", m_conferenceContentRate - " << m_conferenceContentRate << ", videoBridgeBitRate -"<< videoBridgeBitRate;
	}
	else
	{
		videoBridgeBitRate = 100 * m_pIpInitialMode->GetMediaBitRate(cmCapVideo, direction);
	}

	APIU16 profile = 0;
	long fs = 0;
	long mbps = 0;
	APIU8 level = 0;
	long sar = 0;
	long staticMB = 0;
	long dpb = 0;
	EFormat eFormat = kUnknownFormat;
	APIS16  qcifFrameRate = 0;
	APIS16  cifFrameRate = 0;
	APIS16  cif4FrameRate = 0;
	DWORD ForcedFR = 0;

	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();

	//  set default...
	if (vidQuality == eVideoQualitySharpness)
		ForcedFR = 15;
	else
		ForcedFR = 60;

	if (algorithm == eH264CapCode)
	{	//mbps, fs
		//APIU8 level = 0;

		pScm->GetFSandMBPS(direction, profile, level, fs, mbps, sar, staticMB,dpb);

		CH264Details thisH264Details = level;

		if (fs == -1)
			fs = thisH264Details.GetDefaultFsAsDevision();

		if (mbps == -1)
			mbps = thisH264Details.GetDefaultMbpsAsDevision();

		if (dpb == -1)
			dpb = thisH264Details.GetDefaultDpbAsProduct();

		if (sar == -1)
			sar = 0;

		if (staticMB == -1)
			staticMB = DEFAULT_STATIC_MB;

		if (profile == H264_Profile_High)
		{
			videoBridgeProfile = eVideoProfileHigh;
			m_pTaskApi->UpdateDB(m_pParty,PARTYHIGHPROFILE,TRUE);
		}
		else if (profile == H264_Profile_Main)// TIP
		{
			videoBridgeProfile = eVideoProfileMain;
			m_pTaskApi->UpdateDB(m_pParty,PARTYMAINPROFILE,TRUE);
			bIsTipMode = 1;
		}
		else
			m_pTaskApi->UpdateDB(m_pParty,PARTYHIGHPROFILE,FALSE);

		BOOL IsRsrcByFs = IsSetNewFRTresholdForParty();
		long ForcedMbps = 0;

		if (IsRsrcByFs &&  direction== cmCapTransmit) //For MXP patch for transmit only
		{
			ForcedFR = GetForcedMbpsAndFR(fs,mbps,ForcedMbps);
		}

		if (ForcedMbps)
			videoBridgeMbps   = (DWORD)ForcedMbps;
		else
			videoBridgeMbps = (DWORD)mbps;

		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams - videoBridgeMbps: ",videoBridgeMbps);
		videoBridgeFs = (DWORD)fs;
		videoBridgeSar = (DWORD)sar;
		videoBridgeStaticMB = (DWORD)staticMB;

		if (videoBridgeProfile == eVideoProfileHigh || videoBridgeProfile == eVideoProfileMain)// TIP - add videoBridgeProfile == eVideoProfileMain
			videoBridgePacketFormat = eVideoPacketPayloadFormatFragmentationUnit;

		videoBridgeDPB = (DWORD)dpb;
	}
	else if (algorithm == eRtvCapCode)
	{
		pScm->GetRtvFSandMBPS(direction,fs, mbps);

		videoBridgeFs   = (DWORD)fs;
		videoBridgeMbps = (DWORD)mbps;

		bool isAvmcu = strstr(m_productId, "AV-MCU");
		bool isRtvBframeEnabled = m_isBframeRscEnabled && IsRtvBframeEnabled(isAvmcu);
		pMediaParams->SetIsEncodeRTVBFrame(isRtvBframeEnabled);
	}

	APIS32 Width = 0,Height = 0, aspectRatio = 0, maxFrameRate = 0;

	if (algorithm == eVP8CapCode)
	{
		pMediaParams->SetVideoAlgorithm(VP8);
		pMediaParams->SetIsEncodeRTVBFrame(FALSE);
		pScm->GetVp8Scm(direction, fs, maxFrameRate);
		mbps = (fs * maxFrameRate)/500;

		videoBridgeMbps = (DWORD)mbps;
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams - videoBridgeMbps: ",videoBridgeMbps);
		videoBridgeFs = (DWORD)(fs/256);

		CMedString str;
		str << "N.A. DEBUG CIpPartyCntl::InitVideoParams VP8 case "  << "videoBridgeMbps = " << videoBridgeMbps << "videoBridgeFs = "  << videoBridgeFs << "maxFrameRate = " << maxFrameRate ;
		//VP8ParamsStruct& vp8Params = pMediaParams->VP8Params();
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams: ",str.GetString(), GetPartyRsrcId());

	}

	DWORD ssrc = 0;
	DWORD enable_msft_cap = 0;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("ENABLE_MSFT_CAP", enable_msft_cap);
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());

	if(IsValidPObjectPtr(pConfParty) && (strstr(pConfParty->GetName() , "##FORCE_MS_SVC")!=NULL))
	{
		algorithm = eMsSvcCapCode;
	}
	else if (2 == enable_msft_cap)
	{
		algorithm = eRtvCapCode;
	}

	if (algorithm == eMsSvcCapCode)
	{
		videoBridgeProfile = eVideoProfileHigh;
		staticMB = DEFAULT_STATIC_MB;
		pMediaParams->SetVideoAlgorithm(MS_SVC);
		videoBridgePacketFormat = eVideoPacketPayloadFormatFragmentationUnit;
		pMediaParams->SetIsEncodeRTVBFrame(FALSE);

		pScm->GetMSSvcSpecificParams(direction, Width, Height, aspectRatio, maxFrameRate);
		videoBridgeSar = (DWORD)aspectRatio;

		ssrc = GetFirs2013Ssrc(1);//for link cliet it is 1 in AV-MCU it will b accoring to index.

		MsSvcParamsStruct& msSvcParams = pMediaParams->MsSvcParams();

		// TODO: add aspectratio and maxframe rate
		msSvcParams.ssrc = ssrc;
		msSvcParams.pr_id = GetPrIdLync2013();
		msSvcParams.nHeight = Height;
		msSvcParams.nWidth = Width;

		eVideoFrameRate frameRateSvc = TranslateIntegerFrameRateToVideoBridgeFrameRate(maxFrameRate);
		pMediaParams->SetVidFrameRate(frameRateSvc);
		pMediaParams->SetVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_REGULAR);
		pScm->GetMsSvcScm(mbps, fs, direction);
		videoBridgeFs   = (DWORD)fs;
		videoBridgeMbps = (DWORD)mbps;
	}
	else
	{	//get the highest video format

		eFormat   			= pScm->GetVideoFormat(direction);
		videoBridgeFormat   = TranslateIpFormatToVideoBridgeFormat(eFormat);

		// H263 frame rates (qcif,cif,4cif)
		qcifFrameRate = pScm->GetFrameRate(kQCif, direction);
		cifFrameRate  = pScm->GetFrameRate(kCif, direction);
		cif4FrameRate = pScm->GetFrameRate(k4Cif, direction);

		videoBridgeQcifFormat    = TranslateIpFormatToVideoBridgeFormat(kQCif);
		videoBridgeQcifFrameRate = TranslateIpMpiToVideoBridgeFrameRate(qcifFrameRate);

		videoBridgeCifFormat     = TranslateIpFormatToVideoBridgeFormat(kCif);
		videoBridgeCifFrameRate  = TranslateIpMpiToVideoBridgeFrameRate(cifFrameRate);

		videoBridge4CifFormat    = TranslateIpFormatToVideoBridgeFormat(k4Cif);
		videoBridge4CifFrameRate = TranslateIpMpiToVideoBridgeFrameRate(cif4FrameRate);
	}

	pMediaParams->SetCopLinkLecturerMode(m_bCascadeIsLecturer);
	pMediaParams->SetCopResourceOfLecturerLinkIndex(m_copResourceIndexOfCascadeLinkLecturer);
	CComModeInfo cmInfo = algorithm;
	videoBridgeAlgorithm = cmInfo.GetH320ModeType();
	CCapSetInfo capInfo;

	CMedString str;
	str << " Direction[" << ::GetDirectionStr(direction) << "], Video Alg[" << cmInfo.GetH323CapName()
		<< "], eVideoFrameRate[" << (DWORD)pMediaParams->GetVidFrameRate() << "], Format[" << capInfo.GetFormatStr(kQCif) << "]:Frame rate qcif[" << qcifFrameRate
		<< "], Format[" << capInfo.GetFormatStr(kCif) << "]:Frame rate cif[" << cifFrameRate
		<< "], Format[" << capInfo.GetFormatStr(k4Cif) << "]:Frame rate 4cif[" << cif4FrameRate
		<< "], Bit rate[" << videoBridgeBitRate << "], MBPS["<< videoBridgeMbps << "], FS["<< videoBridgeFs << "],ForcedFR[" << ForcedFR <<"] , SAR[" << sar << "], StaticMB["<< videoBridgeStaticMB << "], Profile[" << videoBridgeProfile << "], PacketFormat[" << videoBridgePacketFormat << "],DPB" << videoBridgeDPB
		<< "], TIP mode[" << bIsTipMode << "]"
		<< "], MS SVC Width mode[" << Width << "]"
		<< "], MS SVC Height mode[" << Height << "]"
		<< "], MS SVC Max frame rate mode[" << maxFrameRate << "]"
		<< "], MS SVC ssrc [" << ssrc << "]";

	if (pScm->GetConfType() == kCop && direction == cmCapTransmit)
		str << ", Cop level[" << pMediaParams->GetCopResourceIndex() << "]";
		str << ", Cop Lecturer link level[" << pMediaParams->GetCopResourceOfLecturerLinkIndex() << "]";
		str << ", Cop Lecturer link mode[" << pMediaParams->GetIsCopLinkLecturer() << "]";

	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams: ",str.GetString(), GetPartyRsrcId());

	pMediaParams->SetVideoAlgorithm(videoBridgeAlgorithm);
	pMediaParams->SetVideoBitRate(videoBridgeBitRate);

	pMediaParams->SetVideoFrameRate(eVideoResolutionQCIF, eVideoFrameRate(videoBridgeQcifFrameRate));
	pMediaParams->SetVideoFrameRate(eVideoResolutionCIF, eVideoFrameRate(videoBridgeCifFrameRate));
	pMediaParams->SetVideoFrameRate(eVideoResolution4CIF, eVideoFrameRate(videoBridge4CifFrameRate));

	pMediaParams->SetVideoResolution(eVideoResolution(videoBridgeFormat));

	pMediaParams->SetProfile(videoBridgeProfile);
	pMediaParams->SetPacketFormat(videoBridgePacketFormat);
	pMediaParams->SetMBPS(videoBridgeMbps);
	pMediaParams->SetFS(videoBridgeFs);
	pMediaParams->SetSampleAspectRatio(videoBridgeSar);
	pMediaParams->SetStaticMB(videoBridgeStaticMB);
	pMediaParams->SetMaxDPB(videoBridgeDPB);
	pMediaParams->SetIsTipMode(bIsTipMode);// TIP
	pMediaParams->SetFrThreshold(ForcedFR);

	if ((algorithm == eH264CapCode) || (algorithm == eRtvCapCode))
		SetResolutionTable(pMediaParams);
	else if (algorithm != eMsSvcCapCode)
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams scm video protocol is not H264 or RTV");
		pMediaParams->SetVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_REGULAR);
		BOOL isH263Plus = FALSE;
		CVidModeH323& videoMode = (CVidModeH323&)pScm->GetMediaMode(cmCapVideo,direction,kRolePeople);
		if ((videoMode.GetDataType() == cmCapVideo) && (videoMode.GetType() == eH263CapCode) && (direction == cmCapReceive/*encoder not use H.263+ in current version*/))
		{
			if (videoMode.GetPayloadType() >= 96 && videoMode.GetPayloadType() <= 127)//dynamic payload type in H263 means H263 plus.
				isH263Plus = TRUE;
		}
		pMediaParams->SetIsH263Plus(isH263Plus);
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams ,H263+ is set to ",isH263Plus);
	}

	ECascadePartyType CascadeType = GetPartyCascadeTypeAndVendor();
	pMediaParams->SetPartyCascadeMode(CascadeType);
	PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams ,this is cascade mode,",CascadeType);

	BYTE useIntermediateSDResolution = 0;

	if(GetInterfaceType() == SIP_INTERFACE_TYPE)
		useIntermediateSDResolution = TRUE;
	else if(GetInterfaceType() == H323_INTERFACE_TYPE)
		useIntermediateSDResolution = IsIntermediateSDRes(m_productId);
	else
	{
		PASSERT(1);
	}

	pMediaParams->SetUseIntermediateSDResolution(useIntermediateSDResolution);

}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::ConnectPartyToAudioBridge(CIpComMode* pScm)
{
	if ((m_eAudBridgeConnState & eSendOpenIn) && (m_eAudBridgeConnState & eSendOpenOut))
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToAudioBridge - Not needed. Name - ", m_partyConfName, GetPartyRsrcId());
		return;
	}

	CMedString str;
	str << " Name - " << m_partyConfName;

	BOOL isVideoParticipant = GetVoice() ? FALSE : TRUE;
	BOOL isVtxSupport = FALSE;
	BOOL bIsRelayToMix = false;
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if (pConfParty && ((pConfParty->GetCascadeMode() == CASCADE_MODE_MASTER) || (pConfParty->GetCascadeMode() == CASCADE_MODE_SLAVE)))
	{
		bIsRelayToMix = true;
		TRACEINTO << "set bIsRelayToMix to true";
	}
	CBridgePartyAudioInParams* audioDecoderParams = NULL;
	CBridgePartyAudioOutParams* audioEncoderParams = NULL;
	if (pScm->IsMediaOn(cmCapAudio, cmCapReceive) && !(m_eAudBridgeConnState & eSendOpenIn) )
	{
		audioDecoderParams = new CBridgePartyAudioInParams();

		PASSERTMSG_AND_RETURN(NULL == audioDecoderParams, "new audioDecoderParams failed");

		m_eAudBridgeConnState |= eSendOpenIn;
		CapEnum inAudioAlg  = (CapEnum)pScm->GetMediaType(cmCapAudio, cmCapReceive);

		CComModeInfo inInfo = inAudioAlg;
		WORD inAlgForAB     = inInfo.GetH320ModeType();
		str << ", In Audio Alg - " << inInfo.GetH323CapName();
		DWORD audioDecoderCompressedDelay = 0; // default value

		//if(m_telepresenseEPInfo->GetEPtype() != eTelePresencePartyNone)
		if(m_eTelePresenceMode != eTelePresencePartyNone)
		{
				CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				std::string key = "AUDIO_DECODER_COMPRESSED_DELAY";
				sysConfig->GetDWORDDataByKey(key, audioDecoderCompressedDelay);
		}

		CDwordBitMask muteMask;
		SetPartyAudioMute(muteMask);
		BYTE bInNumberOfChannels = inInfo.GetCodecNumberOfChannels();

		APIU32 ssrc = 0;
		CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
		if (m_bIsMrcCall || pScm->GetConfMediaType()==eMixAvcSvcVsw || pScm->GetConfMediaType()==eMixAvcSvc )
		{
			int aNumOfSsrcIds=0;
			APIU32* aSsrcIds=NULL;
			// to do - add role to this func in scm
			m_pIpInitialMode->GetSsrcIds(cmCapAudio,cmCapReceive,aSsrcIds,&aNumOfSsrcIds);
			if (0<aNumOfSsrcIds)
				ssrc = aSsrcIds[0];
			TRACEINTO << "Receive ssrc=" << ssrc << " aNumOfSsrcIds=" << aNumOfSsrcIds;
			delete[] aSsrcIds;
		}

		BOOL isSupportLegacyToSac=FALSE;
		if(pCommConf->GetConfMediaType() == eMixAvcSvc && !IsSoftMcu())
		{
			if(m_pIpInitialMode->GetConfMediaType()==eMixAvcSvc)
			{
				TRACEINTO<<"!@# setting LegacyToSacTranslate true";
				isSupportLegacyToSac=TRUE;
				audioDecoderParams->SetIsSupportLegacyToSacTranslate(isSupportLegacyToSac);
			}
			else
			{
				TRACEINTO<<"!@# setting LegacyToSacTranslate to false";
				isSupportLegacyToSac=FALSE;
				audioDecoderParams->SetIsSupportLegacyToSacTranslate(isSupportLegacyToSac);
			}
		}
		else
		{
			TRACEINTO<<"!@# either conf configuration is other than eMixAvcSvc or softMcu is running no need to set LegacyToSacTranslate";
		}

		TRACEINTO << "audio ssrc = " << ssrc;
		CCommConf* pCurrentConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
		MSFT_CLIENT_ENUM eMsftClientType = MSFT_CLIENT_NONE_MSFT;
		if (pCurrentConf)
			{
				CConfParty*	pConfParty = pCurrentConf->GetCurrentParty(m_monitorPartyId);
				if (pConfParty && (pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013))
				{
					eMsftClientType = MSFT_CLIENT_AVMCU2013;

				}
			}


		DWORD maxAverageBitrate = pScm->GetMediaBitRate(cmCapAudio, cmCapReceive);

		audioDecoderParams->InitParams(inAlgForAB, muteMask, GetAudioVolume(), bInNumberOfChannels,
										GetConfAudioSampleRate(),isVideoParticipant, IsAgcOn(),  isVtxSupport, GetDialType(), audioDecoderCompressedDelay,
										GetEchoSuppression(),GetKeyboardSuppression(), GetAutoMuteNoisyParties(), GetAudioClarity(),
										ssrc, GetConfSpeakerChangeMode(),isSupportLegacyToSac, bIsRelayToMix, eMsftClientType, maxAverageBitrate);

		if (pScm->GetMediaMode(cmCapAudio, cmCapReceive).GetMute() == YES)
			audioDecoderParams->SetMute(eOff, PARTY);


	}

   	if (pScm->IsMediaOn(cmCapAudio, cmCapTransmit) && !(m_eAudBridgeConnState & eSendOpenOut))
	{
		audioEncoderParams = new CBridgePartyAudioOutParams();
		if(!audioEncoderParams)
		{
			POBJDELETE(audioDecoderParams); // memory leak fix
			PASSERTMSG_AND_RETURN(NULL == audioEncoderParams, "new audioEncoderParams failed");
		}

		m_eAudBridgeConnState |= eSendOpenOut;
		CapEnum outAudioAlg  = (CapEnum)pScm->GetMediaType(cmCapAudio, cmCapTransmit);

		CComModeInfo outInfo = outAudioAlg;
		WORD outAlgForAB     = outInfo.GetH320ModeType();
		str << ", Out Audio Alg - " << outInfo.GetH323CapName();

		CDwordBitMask blockMask;
		SetPartyAudioBlock(blockMask);
		BYTE bOutNumberOfChannels = outInfo.GetCodecNumberOfChannels();

		// temp temp temp
		APIU32* aSsrcIds = NULL;
		int aNumOfSsrcIds=0;
		DWORD ivrSsrc = 0;
		if (m_bIsMrcCall || pScm->GetConfMediaType()==eMixAvcSvcVsw)
		{
			// to do - add role to this func in scm
			m_pIpInitialMode->GetSsrcIds(cmCapAudio,cmCapTransmit,aSsrcIds,&aNumOfSsrcIds);
			PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::ConnectPartyToAudioBridge transmit aNumOfSsrcIds=",aNumOfSsrcIds);
			if (0<aNumOfSsrcIds)
				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::ConnectPartyToAudioBridge transmit aSsrcIds[0]=",aSsrcIds[0]);

			ivrSsrc = m_pIpInitialMode->GetIvrSsrc(cmCapAudio);
			TRACEINTO << " KEREN, ivrSsrc: " << ivrSsrc;

		}

		DWORD maxAverageBitrate = pScm->GetMediaBitRate(cmCapAudio, cmCapTransmit);

		audioEncoderParams->InitParams(outAlgForAB, blockMask, GetListeningAudioVolume(), bOutNumberOfChannels,
										GetConfAudioSampleRate(), isVideoParticipant, aNumOfSsrcIds, aSsrcIds, ivrSsrc, FALSE, bIsRelayToMix, MSFT_CLIENT_NONE_MSFT, maxAverageBitrate);

		delete[] aSsrcIds;
	}
	TRACEINTO
		<< m_partyConfName
		<< ", PartyId:" << GetPartyId()
		<< ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eAudBridgeConnState);

	if (!audioDecoderParams && !audioEncoderParams)
	{
		TRACEINTO << "audioDecoderParams and audioEncoderParams are NULL! Channel is not open. Not connecting to audio bridge.";
		return;
	}
	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(
										m_name, m_pParty, GetPartyId(), m_RoomId, GetInterfaceType(),
										audioDecoderParams, audioEncoderParams);

	if(pBrdgPartyInitParams)
	{
		pBrdgPartyInitParams->SetIsVideoRelay(m_bIsMrcCall || pScm->GetConfMediaType()==eMixAvcSvcVsw);

		// speakerIndication
		TRACEINTO << "speakerIndication - fill BrdgPartyInitParams - decision point- IbmSametimeEp: " << ( ((IbmSametimeEp == m_eRemoteVendorIdent) || (IbmSametimeEp_Legacy == m_eRemoteVendorIdent)) ? "yes" : "no" );
		if ((IbmSametimeEp == m_eRemoteVendorIdent) || (IbmSametimeEp_Legacy == m_eRemoteVendorIdent))
		{
			pBrdgPartyInitParams->SetUseSpeakerSsrcForTx(TRUE);
			m_bIsUseSpeakerSsrcForTx_TRUE_sent = true;
		}

		m_pConfAppMngrInterface->ConnectPartyAudio(pBrdgPartyInitParams);
		ON(m_bIsMemberInAudBridge);
		POBJDELETE(pBrdgPartyInitParams);
	}


	POBJDELETE(audioDecoderParams);
	POBJDELETE(audioEncoderParams);

}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::ConnectPartyToVideoBridge(CIpComMode* pScm)
{
	PTRACE(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge - begin");
	if ((m_eVidBridgeConnState & eSendOpenIn) && (m_eVidBridgeConnState & eSendOpenOut))
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge - Not needed. Name - ", m_partyConfName, GetPartyRsrcId());
		return;
	}

	SetSiteName(m_name);
	if (m_bIsMrcCall || pScm->GetConfMediaType()==eMixAvcSvcVsw)
	{
		PTRACE2INT(eLevelDebug, "CIpPartyCntl::ConnectPartyToVideoBridge - m_bIsMrcCall=", m_bIsMrcCall);
		ConnectPartyToVideoBridge_videoRelay(pScm);
	}
	else
	{
		ConnectPartyToVideoBridge_noVideoRelay(pScm);
	}

	ON(m_bIsMemberInVidBridge);

}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::ConnectPartyToVideoBridge_noVideoRelay(CIpComMode* pScm)
{
	PTRACE(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge_noVideoRelay - begin");

	if ((m_eVidBridgeConnState & eSendOpenIn) && (m_eVidBridgeConnState & eSendOpenOut))
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge_noVideoRelay - Not needed. Name - ", m_partyConfName, GetPartyRsrcId());
		return;
	}

	CBridgePartyVideoInParams*  pInVideoParams  = NULL;
	CBridgePartyVideoOutParams* pOutVideoParams = NULL;

	// VNGFE-787
	if (!m_isCodianVcr)
	{
		DWORD vidInRate = pScm->GetMediaBitRate(cmCapVideo, cmCapReceive); // another check since video rate can be 0 in receive because of secondary

		if (pScm->IsMediaOn(cmCapVideo, cmCapReceive) && !(m_eVidBridgeConnState & eSendOpenIn) && vidInRate)
		{
			pInVideoParams = new CBridgePartyVideoInParams();
			m_eVidBridgeConnState |= eSendOpenIn;
			InitVideoInParams(pInVideoParams, pScm);
			pInVideoParams->SetSiteName(m_siteName);

			//_e_m_
			CMedString str = "";

			if (m_telepresenseEPInfo)
			{
				str << " EPtype " 		 << (int)m_telepresenseEPInfo->GetEPtype()
					<< " LinkNum " 	 	 << (int)m_telepresenseEPInfo->GetLinkNum()
					<< " LinkRole " 	 << (int)m_telepresenseEPInfo->GetLinkRole()
					<< " NumOfLinks " 	 << (int)m_telepresenseEPInfo->GetNumOfLinks()
					<< " PartyMonitorID "<< (int)m_telepresenseEPInfo->GetPartyMonitorID()
					<< " RTType " 		 << (int)m_telepresenseEPInfo->GetRTType()
					<< " RoomID " 		 << (int)m_telepresenseEPInfo->GetRoomID()
					<< " WaitForUpdate " << (int)m_telepresenseEPInfo->GetWaitForUpdate();
				m_eTelePresenceMode = m_telepresenseEPInfo->GetEPtype();
			}

			PTRACE2(eLevelInfoNormal, "EMB_MLA : CIpPartyCntl::ConnectPartyToVideoBridge_noVideoRelay - m_telepresenseEPInfo: ", str.GetString());

			pInVideoParams->SetTelePresenceMode(m_eTelePresenceMode);
			pInVideoParams->SetTelePresenceEPInfo(m_telepresenseEPInfo);

			pInVideoParams->SetIsAutoBrightness(bool (GetAutoBrightness()));
			pInVideoParams->SetVideoPartyType(m_eLastAllocatedVideoPartyType);
			CDwordBitMask muteMask;
			SetPartyVideoMute(muteMask);
			pInVideoParams->SetMuteMask(muteMask);
			m_eFirstRxVideoCapCode = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople));
			if(m_eFirstRxVideoCapCode == eH264CapCode)
				m_eFirstRxVideoProfile = pScm->GetH264Profile(cmCapReceive);

			TRACEINTOFUNC
				<< "m_eFirstRxVideoCapCode: " << CapEnumToString(m_eFirstRxVideoCapCode)
				<< ", m_eFirstRxVideoProfile: " << m_eFirstRxVideoProfile;

			BYTE IsSetSmartSwitchAccordingToVendoer = IsSetSmartSwitchForUser(m_productId);

			if(IsSetSmartSwitchAccordingToVendoer )
			{
				pInVideoParams->SetIsRemoteNeedSmartSwitchAccordingToVendor(TRUE);
				PTRACE(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge_noVideoRelay remote is  HDX -8000 or vendor needing smart switch ");
			}

			// if AvcToSvc - @#@ which flag needs to be checked
			CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

			if (pCommConf->GetConfMediaType() == eMixAvcSvc)
			{
				pInVideoParams->InitAvcToSvcParams();
				CAvcToSvcParams *pAvcToSvcParams = pInVideoParams->GetAvcToSvcParams();
				TRACEINTO << "mix_mode: m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle;
				pAvcToSvcParams->SetChannelHandle(m_incomingVideoChannelHandle);

				if (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvc
					&& m_incomingVideoChannelHandle!=INVALID_CHANNEL_HANDLE)
				{
					pAvcToSvcParams->SetIsSupportAvcSvcTranslate(true);
					TRACEINTO<<"!@# setting SupportAvcSvcTranslate to true";
				}
				else
				{
					pAvcToSvcParams->SetIsSupportAvcSvcTranslate(false);
					TRACEINTO<<"!@# setting SupportAvcSvcTranslate to false";
				}

				pAvcToSvcParams->SetVideoOperationPointsSet(&m_operationPointsSet);

				if (m_operationPointsSet.m_numberOfOperationPoints==0)
					TRACEINTOFUNC << "vops is empty!";

				bool result = FillStreamSSRCForMedia_noRelay(pAvcToSvcParams);
			}
			else
				TRACEINTO << "!@# conf is other than eMixAvcSvc, no need to set IsSupportAvcSvcTran";
		}
	}

    CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
    BOOL isCallFromGW = (pConfParty) ? pConfParty->GetIsCallFromGW(): FALSE;
	if (pScm->IsMediaOn(cmCapVideo, cmCapTransmit) && !(m_eVidBridgeConnState & eSendOpenOut))
	{
		pOutVideoParams = new CBridgePartyVideoOutParams();

		if(m_telepresenseEPInfo)
			m_eTelePresenceMode = m_telepresenseEPInfo->GetEPtype();

		pOutVideoParams->SetTelePresenceMode(m_eTelePresenceMode);
		pOutVideoParams->SetTelePresenceEPInfo(m_telepresenseEPInfo);


        if(isCallFromGW){
			pOutVideoParams->SetIsCascadeLink(0);
			TRACEINTO << "SetIsCascadeLink(0) in pOutVideoParams for call from GW";
        }else{
			pOutVideoParams->SetIsCascadeLink(GetPartyCascadeType() == CASCADE_MCU);
        }

		pOutVideoParams->SetVideoPartyType(m_eLastAllocatedVideoPartyType);

		// Init Party Video Layout Params
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
		InitVideoLayoutParams(pOutVideoParams, pConfParty);

		m_eVidBridgeConnState |= eSendOpenOut;
		InitVideoOutParams(pOutVideoParams, pScm);
	}
	DWORD msPartyRsrcId = 0;
	DWORD msAudioLocalMsi = 0;
	DWORD msAvMcuIndex =0;
	CCommConf* pCurrentConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	if (pCurrentConf)
	{
		CConfParty*	pConfParty = pCurrentConf->GetCurrentParty(m_monitorPartyId);
		if (pConfParty && pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013)
		{
			PTRACE(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams - AV-MCU main");
			msAvMcuIndex =0;//main is laways index 1;
			msPartyRsrcId =  GetPartyRsrcId();
			//pOutVideoParams->SetIsCascadeLink(TRUE);
		}
	}

	if(IsMsSlaveOut() || IsMsSlaveIn())
	{
		msAvMcuIndex = m_MSSlaveIndex;
		msPartyRsrcId =  GetMainPartyId();
		TRACEINTO << "msSlaveIndex:" << msAvMcuIndex << ", msPartyRsrcId:" << msPartyRsrcId;
		//pOutVideoParams->SetIsCascadeLink(TRUE);
	}

    BYTE cascadePartyType = GetPartyCascadeType();
    if(isCallFromGW){
		cascadePartyType = CASCADE_NONE;
		TRACEINTO << "cascadePartyType = eCascadeNone for call from GW";
    }

	TRACEINTO
		<< m_partyConfName
		<< ", PartyId:" << GetPartyId()
		<< ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState)
		<< ", CascadeMode:" << (WORD)GetPartyCascadeType()
		<< ", isMsSlaveOut " << (WORD)IsMsSlaveOut()
		<< ", isMsSlaveIn " << (WORD)IsMsSlaveIn()
		<< ", msAudioLocalMsi " << (DWORD)msAudioLocalMsi
		<< ", msAvMcuIndex "    << (WORD)msAvMcuIndex
		<< ", MainPartyId " << (DWORD)msPartyRsrcId;

	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(m_name,
																			  m_pParty,
																			  GetPartyId(),
																			  m_RoomId,
																			  GetInterfaceType(),
																			  pInVideoParams,
																			  pOutVideoParams,
																			  m_pBridgeMoveParams->GetAndResetVideoBridgePartyCntlOnImport(),
																			  m_siteName,
																			  cascadePartyType,
																			  false/*bIsVideoRelay*/,
																			  FALSE,/*isUseSpeakerSsrcForTx*/
																			  msPartyRsrcId,
																			  msAudioLocalMsi,
																		      msAvMcuIndex);

	m_pConfAppMngrInterface->ConnectPartyVideo(pBrdgPartyInitParams);

	//To Move to another location...
	BYTE isLegacy = IsLegacyContentParty();

	if(isLegacy)
		m_ConnectedToVideoAsLegacy = TRUE;
	else
		m_ConnectedToVideoAsLegacy = FALSE;

	POBJDELETE(pInVideoParams);
	POBJDELETE(pOutVideoParams);
	POBJDELETE(pBrdgPartyInitParams);

	PTRACE(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge_noVideoRelay - end");
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::ConnectPartyToVideoBridge_videoRelay(CIpComMode* pScm)
{
	PTRACE(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge_VideoRelay - begin");
	if ((m_eVidBridgeConnState & eSendOpenIn) && (m_eVidBridgeConnState & eSendOpenOut))
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge_videoRelay - Not needed. Name - ", m_partyConfName, GetPartyRsrcId());
		return;
	}

	CBridgePartyVideoRelayMediaParams* pInVideoParams  = NULL;
	CBridgePartyVideoRelayMediaParams* pOutVideoParams = NULL;

	bool m_bVideoRecvOn = false, m_bVideoTransOn = false;

	// VNGFE-787
	if (!m_isCodianVcr)
	{
		DWORD vidInRate = pScm->GetMediaBitRate(cmCapVideo, cmCapReceive); // another check since video rate can be 0 in receive because of secondary
		if (pScm->IsMediaOn(cmCapVideo, cmCapReceive) && !(m_eVidBridgeConnState & eSendOpenIn) && vidInRate)
		{
			m_bVideoRecvOn = true;

			pInVideoParams = new CBridgePartyVideoRelayMediaParams();
			m_eVidBridgeConnState |= eSendOpenIn;
//			InitVideoInParams(pInVideoParams, pScm);

			m_bVideoRelayInReady = IsRelayChannelReadyForMedia(cmCapReceive);

			if (m_bVideoRelayInReady)
			{
				TRACEINTO << "m_incomingVideoChannelHandle: " << m_incomingVideoChannelHandle;
				FillStreamSSRCForMedia(pInVideoParams,cmCapVideo,cmCapReceive,REASON_CONNECT);
				pInVideoParams->SetChannelHandle(m_incomingVideoChannelHandle);

			}
			pInVideoParams->SetIsReady(m_bVideoRelayInReady);

			CMedString str = "";
			if (m_telepresenseEPInfo)
			{
				str << " EPtype " 		 << (int)m_telepresenseEPInfo->GetEPtype()
					<< " LinkNum " 	 	 << (int)m_telepresenseEPInfo->GetLinkNum()
					<< " LinkRole " 	 << (int)m_telepresenseEPInfo->GetLinkRole()
					<< " NumOfLinks " 	 << (int)m_telepresenseEPInfo->GetNumOfLinks()
					<< " PartyMonitorID "<< (int)m_telepresenseEPInfo->GetPartyMonitorID()
					<< " RTType " 		 << (int)m_telepresenseEPInfo->GetRTType()
					<< " RoomID " 		 << (int)m_telepresenseEPInfo->GetRoomID()
					<< " WaitForUpdate " << (int)m_telepresenseEPInfo->GetWaitForUpdate();

				m_eTelePresenceMode = m_telepresenseEPInfo->GetEPtype();
			}

			TRACEINTO << "EMB_MLA_OLGA : m_telepresenseEPInfo: " << str.GetString();

			pInVideoParams->SetTelePresenceMode(m_eTelePresenceMode);
			pInVideoParams->SetTelePresenceEPInfo(m_telepresenseEPInfo);

			CDwordBitMask muteMask;
			SetPartyVideoMute(muteMask);
			pInVideoParams->SetMuteMask(muteMask);

			m_eFirstRxVideoCapCode = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople));
			if(m_eFirstRxVideoCapCode == eH264CapCode)
				m_eFirstRxVideoProfile = pScm->GetH264Profile(cmCapReceive);

			TRACEINTOFUNC << "m_eFirstRxVideoCapCode: " << m_eFirstRxVideoCapCode << ", m_eFirstRxVideoProfile :" << m_eFirstRxVideoProfile;

			CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
			if(pCommConf->GetConfMediaType() == eMixAvcSvc )
			{
			    TRACEINTOFUNC << "mix_mode: m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle;
			    if(m_pIpInitialMode->GetConfMediaType()==eMixAvcSvc)
			    {
			    	pInVideoParams->SetSupportSvcAvcTranslate(true);
			    	TRACEINTO<<"!@# setting SupportAvcSvcTranslate to true";
			    }
			    else
			    {
			    	pInVideoParams->SetSupportSvcAvcTranslate(false);
			    	TRACEINTO<<"!@# setting SupportAvcSvcTranslate to false";
			    }
			}
			else
			{
				TRACEINTO<<"!@# conf is other than eMixAvcSvc, no need to set IsSupportAvcSvcTran";
			}
			pInVideoParams->SetIsCascadeLink(false);
			CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
			if (pConfParty && ((pConfParty->GetCascadeMode() == CASCADE_MODE_MASTER) || (pConfParty->GetCascadeMode() == CASCADE_MODE_SLAVE)))
			{
				pInVideoParams->SetIsCascadeLink(true);
				TRACEINTO << "set IsCascadeLink to true";
			}
		}
	}

	if (pScm->IsMediaOn(cmCapVideo, cmCapTransmit) && !(m_eVidBridgeConnState & eSendOpenOut))
	{
		m_bVideoTransOn = true;

		m_eVidBridgeConnState |= eSendOpenOut;

		pOutVideoParams = new CBridgePartyVideoRelayMediaParams();

		m_bVideoRelayOutReady = IsRelayChannelReadyForMedia(cmCapTransmit);

		if (m_bVideoRelayOutReady)
		{
			TRACEINTO << "m_outgoingVideoChannelHandle: " << m_outgoingVideoChannelHandle;
			pOutVideoParams->SetChannelHandle(m_outgoingVideoChannelHandle);
			FillStreamSSRCForMedia(pOutVideoParams,cmCapVideo,cmCapTransmit,REASON_CONNECT);
			pOutVideoParams->SetChannelHandle(m_outgoingVideoChannelHandle);
			pOutVideoParams->SetScpRequestSequenceNumber( GetScpNotificationRemoteSeqNumber() );
		}
		pOutVideoParams->SetIsReady(m_bVideoRelayOutReady);

		if (m_telepresenseEPInfo)
			m_eTelePresenceMode = m_telepresenseEPInfo->GetEPtype();
		pOutVideoParams->SetTelePresenceMode(m_eTelePresenceMode);
		pOutVideoParams->SetTelePresenceEPInfo(m_telepresenseEPInfo);

		DWORD ivrSlideSsrc= pScm->GetIvrSsrc(cmCapVideo);
		pOutVideoParams->SetPipeIdForIvrSlide(ivrSlideSsrc);
//		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
//		InitVideoLayoutParams(pOutVideoParams, pConfParty);

//		InitVideoOutParams(pOutVideoParams, pScm);
		pOutVideoParams->SetIsCascadeLink(false);
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
		if (pConfParty && ((pConfParty->GetCascadeMode() == CASCADE_MODE_MASTER) || (pConfParty->GetCascadeMode() == CASCADE_MODE_SLAVE)))
		{
			TRACEINTO << "set IsCascadeLink to true";
			pOutVideoParams->SetIsCascadeLink(true);
		}
	}

	TRACEINTO
		<< m_partyConfName
		<< ", PartyId:" << GetPartyId()
		<< ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState)
		<< ", CascadeMode:" << (WORD)GetPartyCascadeType();

	if (m_bVideoRecvOn || m_bVideoTransOn)
	{
		TRACEINTO << "m_bVideoRecvOn: " << (m_bVideoRecvOn ? "yes" : "no") << ", m_bVideoTransOn: " << (m_bVideoTransOn ? "yes" : "no") << " - calling ConnectPartyVideo (" << m_partyConfName << ")";

		CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(m_name,
																				  m_pParty,
																				  GetPartyId(),
																				  m_RoomId,
																				  GetInterfaceType(),
																				  pInVideoParams,
																				  pOutVideoParams,
																				  m_pBridgeMoveParams->GetAndResetVideoBridgePartyCntlOnImport(),
																				  m_siteName,
																				  GetPartyCascadeType(),
																				  true/*bIsVideoRelay*/);

		m_pConfAppMngrInterface->ConnectPartyVideo(pBrdgPartyInitParams);

		POBJDELETE(pBrdgPartyInitParams);
	}

	else
	{
		TRACEINTO << "m_bVideoRecvOn: no, m_bVideoTransOn: no - ConnectPartyVideo is not called! (" << m_partyConfName << ")";
	}

	POBJDELETE(pInVideoParams);
	POBJDELETE(pOutVideoParams);
	PTRACE(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridge_videoRelay - end");
}

///////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::ConnectPartyToFECCBridge(CIpComMode* pScm)
{
	if (pScm->IsMediaOff(cmCapData))
		return;

	if (m_bIsMrcCall) // hg and Shmuel - currently MRC does not support FECC
		return;

	if (m_pFECCBridge)
	{
		if (!m_isFeccConn)
		{
			ON(m_isFeccConn);

			CBridgePartyInitParams* pBrdgPartyInitParams = 	new CFECCBridgePartyInitParams(m_name, m_pParty, GetPartyRsrcId(), GetInterfaceType(), NULL, NULL);

			ALLOCBUFFER(tmp,2*H243_NAME_LEN+150); //max size of m_partyConfName is (2*H243_NAME_LEN+50)
			sprintf(tmp, "[%s] - Establishing FECC Connection ", m_partyConfName);
			PTRACE2PARTYID(eLevelInfoNormal," ---> ",tmp, GetPartyRsrcId());
			DEALLOCBUFFER(tmp);

			//for debug FEEC problem - will remove
			TRACEINTO << "pParty: " << pBrdgPartyInitParams->GetParty() << " , partyID: " << pBrdgPartyInitParams->GetPartyRsrcID() << " , m_pParty: " << m_pParty;;

		 	m_pFECCBridge->ConnectParty(pBrdgPartyInitParams);
			POBJDELETE(pBrdgPartyInitParams);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::OnFeccBrdgCon(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::OnFeccBrdgCon : Name - ",m_partyConfName, GetPartyRsrcId());
	WORD  status;
	*pParam >> status;
	if (status)
	  	PTRACE2PARTYID(eLevelError,"CIpPartyCntl::OnFeccBrdgCon - status is not OK : Name - ",m_partyConfName, GetPartyRsrcId());

	else if (!m_isFeccConn)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::OnFeccBrdgCon : Connect has received after disconnect. Name - ",m_partyConfName, GetPartyRsrcId());
	}
	else
	{
		m_pTaskApi->UpdateDB(m_pParty,T120CON,TRUE); //??
		m_pPartyApi->InformArtFeccPartyType(); //update parties RTP
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::OnFeccBridgeDisConnect(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::OnFeccBridgeDisConnect : Name - ",m_partyConfName, GetPartyRsrcId());
	WORD  status;
	*pParam >> status;
	if(TRUE == m_isFeccConn) // BRIDGE - 1100
		DBGPASSERT(status);
	OFF(m_isFeccConn);
//	OFF(m_isDatConnReq);
}

/////////////////////////////////////////////////////////////////////////////
EUpdateBridgeMediaAndDirection  CIpPartyCntl::UpdateAudioAndVideoBridgesIfNeeded(BYTE bTakeInitial)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioAndVideoBridgesIfNeeded : Name - ",m_partyConfName, GetPartyRsrcId());

	int updateBridges = eNoUpdate;

	updateBridges += UpdateAudioBridgesIfNeeded();
	updateBridges += UpdateVideoBridgesIfNeeded(bTakeInitial);

	PTRACE2INT(eLevelInfoNormal,"***CIpPartyCntl::UpdateAudioAndVideoBridgesIfNeeded - updateBridges: ",updateBridges);

	return (EUpdateBridgeMediaAndDirection)updateBridges;
}

/////////////////////////////////////////////////////////////////////////////
EUpdateBridgeMediaAndDirection  CIpPartyCntl::UpdateAudioBridgesIfNeeded()
{
	PTRACE2PARTYID(eLevelInfoNormal,"***CIpPartyCntl::UpdateAudioBridgesIfNeeded : Name - ",m_partyConfName, GetPartyRsrcId());
	int updateBridges = eNoUpdate;

	if (m_eAudBridgeConnState & eSendOpenIn)
	{
		if(UpdateAudioInBridgeIfNeeded())
			updateBridges += eUpdateAudioIn;
	}
	else
		PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioBridgesIfNeeded : No IN Connection", GetPartyRsrcId());

	if (m_eAudBridgeConnState & eSendOpenOut)
	{
		if(UpdateAudioOutBridgeIfNeeded())
			updateBridges += eUpdateAudioOut;
	}
	else
		PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioBridgesIfNeeded : No OUT Connection", GetPartyRsrcId());

	if (m_isTipFallbackFlow && updateBridges)
		m_isTipFallbackFlow = false;

	return (EUpdateBridgeMediaAndDirection)updateBridges;
}

/////////////////////////////////////////////////////////////////////////////
EUpdateBridgeMediaAndDirection  CIpPartyCntl::UpdateVideoBridgesIfNeeded(BYTE bTakeInitial, BYTE bForceUpdateIn, BYTE bForceUpdateOut)
{
	PTRACE2PARTYID(eLevelInfoNormal,"***CIpPartyCntl::UpdateVideoBridgesIfNeeded : Name - ",m_partyConfName, GetPartyRsrcId());
	int updateBridges = eNoUpdate;

	if (m_eVidBridgeConnState & eSendOpenIn)
	{
		if(UpdateVideoInBridgeIfNeeded(bTakeInitial, bForceUpdateIn))
			updateBridges += eUpdateVideoIn;
	}
	else
		PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoBridgesIfNeeded : No IN Connection", GetPartyRsrcId());


    	if (m_eVidBridgeConnState & eSendOpenOut)
    	{
    		TRACEINTOFUNC<<"###! inside m_eVidBridgeConnState & eSendOpenOut";
    		if(UpdateVideoOutBridgeIfNeeded(bTakeInitial, bForceUpdateOut))
    			updateBridges += eUpdateVideoOut;
    	}
    	else
    		PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoBridgesIfNeeded : No OUT Connection", GetPartyRsrcId());

	return (EUpdateBridgeMediaAndDirection)updateBridges;
}

/////////////////////////////////////////////////////////////////////////////
int  CIpPartyCntl::UpdateVideoInBridgeIfNeeded(BYTE bTakeInitial, BYTE bForceUpdate)
{
    PTRACE(eLevelInfoNormal, "CIpPartyCntl::UpdateVideoInBridgeIfNeeded");

    if(m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw /*|| m_pIpInitialMode->GetConfMediaType()==eMixAvcSvc*/)
        return UpdateVideoRelayInBridgeIfNeeded(bTakeInitial, bForceUpdate);

	DWORD details = 0;
	BYTE bVideoInDiff = NO;
	if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapReceive) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive))
	{
		bVideoInDiff  = !m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapReceive);
		if (bVideoInDiff == FALSE)
			bVideoInDiff = !m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapReceive);

		if (bVideoInDiff == FALSE && bForceUpdate)
		{
			PTRACE2PARTYID(eLevelError,"CIpPartyCntl::UpdateVideoInBridgeIfNeeded - No Update is needed but Force update is true : Name - ",m_partyConfName, GetPartyRsrcId());
			bVideoInDiff = TRUE;
		}

		if(bVideoInDiff)
		{
			if (m_pIpInitialMode->GetConfType() == kVSW_Fixed)
			{
				PTRACE2PARTYID(eLevelError,"CIpPartyCntl::UpdateVideoInBridgeIfNeeded - Can not send update in case of vsw fixed : Name - ",m_partyConfName, GetPartyRsrcId());
				DBGPASSERT(GetPartyRsrcId());
				return NO;
			}

			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoInBridgeIfNeeded (Update Bridge): Name - ",m_partyConfName, GetPartyRsrcId());
			CBridgePartyVideoInParams *inVideoParams = new CBridgePartyVideoInParams;
			if(bTakeInitial)
				InitVideoInParams(inVideoParams, m_pIpInitialMode);
			else
				InitVideoInParams(inVideoParams, m_pIpCurrentMode);

			// if AvcToSvc - @#@ which flag needs to be checked
			CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
			if(pCommConf->GetConfMediaType() == eMixAvcSvc )
			{
				inVideoParams->InitAvcToSvcParams();
			    CAvcToSvcParams *pAvcToSvcParams = inVideoParams->GetAvcToSvcParams();
			    TRACEINTOFUNC << "mix_mode: m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle;
			    pAvcToSvcParams->SetChannelHandle(m_incomingVideoChannelHandle);


			    if(m_pIpInitialMode->GetConfMediaType()==eMixAvcSvc && m_incomingVideoChannelHandle!=INVALID_CHANNEL_HANDLE)
			    {
			    	pAvcToSvcParams->SetIsSupportAvcSvcTranslate(true);
			    	TRACEINTO<<"!@# setting SupportAvcSvcTranslate to true";
			    }
			    else
			    {
			    	pAvcToSvcParams->SetIsSupportAvcSvcTranslate(false);
			    	TRACEINTO<<"!@# setting SupportAvcSvcTranslate to false";
			    }

		        pAvcToSvcParams->SetVideoOperationPointsSet(m_pIpInitialMode->GetOperationPoints());
		        if (m_pIpInitialMode->GetOperationPoints()->m_numberOfOperationPoints==0)
		        {
		            TRACEINTOFUNC << "vops is empty!";
		        }
		        bool result=FillStreamSSRCForMedia_noRelay(pAvcToSvcParams);
			if(m_pIpInitialMode->GetConfMediaType()== eMixAvcSvc)
			{
			  if(result==true)
			  {
			     TRACEINTO<<"dynMixedPosAck update VB for nonRelay";
			  }
			  else
			  {
			     TRACEINTO<<"dynMixedErr either no streams or layer id is invalid";
			  }

			}

			}
			else
			{
			    TRACEINTO<<"!@# conf is other than eMixAvcSvc, no need to set IsSupportAvcSvcTran";
			}

			///Update the in params
			inVideoParams->SetSiteName(m_siteName);
			CDwordBitMask muteMask;
			SetPartyVideoMute(muteMask);
			inVideoParams->SetMuteMask(muteMask);
			BYTE IsSetSmartSwitchAccordingToVendoer = IsSetSmartSwitchForUser(m_productId);
			if(IsSetSmartSwitchAccordingToVendoer )
			{
				inVideoParams->SetIsRemoteNeedSmartSwitchAccordingToVendor(TRUE);
				//PTRACE(eLevelInfoNormal, "CIpPartyCntl::ConnectPartyToVideoBridgeREMOTE IS hdx-8000 ");
			}

			m_pVideoBridgeInterface->UpdateVideoInParams(m_pParty, inVideoParams);

			POBJDELETE(inVideoParams);
		}
		else
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoInBridgeIfNeeded (No Update): Name - ",m_partyConfName, GetPartyRsrcId());
	}
	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoInBridgeIfNeeded, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());

	return bVideoInDiff;
}

/////////////////////////////////////////////////////////////////////////////
int  CIpPartyCntl::UpdateVideoOutBridgeIfNeeded(BYTE bTakeInitial,BYTE bForceUpdate)
{
    PTRACE(eLevelInfoNormal, "CIpPartyCntl::UpdateVideoOutBridgeIfNeeded");

	if(m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
        return UpdateVideoRelayOutBridgeIfNeeded(bTakeInitial, bForceUpdate);

	DWORD details = 0;
	BYTE bVideoOutDiff = NO;

    DWORD iniContentRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
    DWORD iniPeopleRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);

	if(m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit))
	{
		bVideoOutDiff  = !m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate |kMsSvcParams |kBitRateWithoutTolerance,&details,cmCapVideo,cmCapTransmit);
		if (bVideoOutDiff == FALSE)
			bVideoOutDiff = !m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate | kBitRateWithoutTolerance |kMsSvcParams,&details,cmCapVideo,cmCapTransmit);

		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded -this is  m_currentVideoEncRate - ",m_currentVideoEncRate);
		if(m_currentVideoEncRate != 0 && !bVideoOutDiff && ((m_currentVideoEncRate/100 ) != iniPeopleRate) && m_pIpInitialMode->GetConfType() == kCp && bTakeInitial)
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded -UPDATING ALTOUGH NO DIFF: Name - ",m_partyConfName, GetPartyRsrcId());
			bVideoOutDiff = TRUE;
		}

		//update for PACSI
		CapEnum algorithmInitial = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit));
		if (bForceUpdate && ( /*m_AvMcuLinkType != eAvMcuLinkNone ||*/  eMsSvcCapCode == algorithmInitial)  )
		{
			TRACEINTO << "PartyName:" << m_partyConfName << ", PartyID: " << GetPartyRsrcId() << " - force UPDATE for pacsi";
			bVideoOutDiff = true;
		}

		if(bVideoOutDiff)
		{
			if (m_pIpInitialMode->GetConfType() == kVSW_Fixed)
			{
				PTRACE2PARTYID(eLevelError,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded - Can not send update in case of vsw fixed: Name - ",m_partyConfName, GetPartyRsrcId());
				DBGPASSERT(GetPartyRsrcId());
				return NO;
			}

			if ((m_pIpInitialMode->GetConfType() == kCop) && (m_pIpInitialMode->GetCopTxLevel() == m_pIpCurrentMode->GetCopTxLevel() && !m_bCascadeIsLecturer && !IsNeedToChangeDueToSwitchFromLecturerToNonLecturer(m_pIpCurrentMode->GetCopTxLevel(),FALSE/*do not take current*/) ))
			{
				// This is an error case, because if cop levels are equal, there should not be any differences in video out parameters.
				// Either the video out parameters are wrong or the cop level is wrong.
				// We don't sent update to Bridge in such case because Bridge check only the cop level, and there is no change in the cop level.
				bVideoOutDiff  = !m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS|kH264Additional_FS|kBitRate| kMaxFR | kH264Mode |kMsSvcParams |kPacketizationMode,&details,cmCapVideo,cmCapTransmit);
				if (bVideoOutDiff == FALSE)
					bVideoOutDiff = !m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS|kH264Additional_FS|kBitRate| kMaxFR | kH264Mode|kMsSvcParams | kPacketizationMode,&details,cmCapVideo,cmCapTransmit);
				if (bVideoOutDiff)
				{
					m_pIpCurrentMode->Dump("CIpPartyCntl::UpdateVideoOutBridgeIfNeeded Current:", eLevelInfoNormal);
					m_pIpInitialMode->Dump("CIpPartyCntl::UpdateVideoOutBridgeIfNeeded Initial:", eLevelInfoNormal);
					//DBGPASSERT(GetPartyRsrcId());
				}
				else
					PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded, modes are different in non significant parameters. Name - ",m_partyConfName, GetPartyRsrcId());
				if(!bForceUpdate)
					return NO;
			}

			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded (Update Bridge): Name - ",m_partyConfName, GetPartyRsrcId());
			CBridgePartyVideoOutParams *outVideoParams = new CBridgePartyVideoOutParams;

			CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
			BYTE partyResolution = eAuto_Res;
			if (pConfParty)
			    partyResolution =  pConfParty->GetMaxResolution();
			else
			{
			    PTRACE(eLevelError,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded - pConfParty is NULL - set eAuto_Res");
			    DBGPASSERT(1113);
			}
			BYTE maxConfResolution = m_pConf->GetCommConf()->GetConfMaxResolution();

			if(bTakeInitial)
			{
				InitVideoOutParams(outVideoParams, m_pIpInitialMode);
				if(partyResolution == eAuto_Res && maxConfResolution == eAuto_Res)
					updateResolutionAccordingToNewVideoRate( outVideoParams, m_pIpInitialMode);
			}
			else
			{
				InitVideoOutParams(outVideoParams, m_pIpCurrentMode);
				if(partyResolution == eAuto_Res && maxConfResolution == eAuto_Res)
					updateResolutionAccordingToNewVideoRate( outVideoParams, m_pIpCurrentMode);
			}

			outVideoParams->SetIsCascadeLink(GetPartyCascadeType() == CASCADE_MCU);

			std::ostringstream msg;
			msg << outVideoParams->PACSI();
			TRACEINTO << msg.str().c_str();
			m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);

			POBJDELETE(outVideoParams);
		}
		else
		{
			//_t_p_
			if (m_bIsNeedToChangeVideoOutTipPolycom && (GetIsTipCall() || IsTipSlavePartyType()))
			{
				PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded - need to change video out for TIP polycom");

				CIpComMode* pCommMode;
				CBridgePartyVideoOutParams *outVideoParams = new CBridgePartyVideoOutParams;

				if (bTakeInitial)
					pCommMode = m_pIpInitialMode;
				else
					pCommMode = m_pIpCurrentMode;

				InitVideoOutParams(outVideoParams, pCommMode);

				m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);

				POBJDELETE(outVideoParams);
			}
			else
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded (No Update): Name - ",m_partyConfName, GetPartyRsrcId());
	}
	}
	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());

	return bVideoOutDiff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::GetIsForce720Pand2MForPLCMFlag(BYTE& bForce720PFlag , BYTE& bForce2MFlag ) const
{
	bForce720PFlag = FALSE;
	bForce2MFlag = FALSE;

	CProcessBase* pProccess = CProcessBase::GetProcess();
	if (!pProccess)
		return;

	CSysConfig *sysConfig = pProccess->GetSysConfig();
	if (!sysConfig)
		return;

	std::string strKey;
    sysConfig->GetDataByKey(CFG_KEY_FORCE_720P_2048_FOR_PLCM_TIP, strKey);

    if(strKey.compare("FORCE_2048_FOR_PLCM_TIP ")==0)
    {
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetIsForce720Pand2MForPLCMFlag : FORCE_720P_2048_FOR_PLCM_TIP is FORCE_2048_FOR_PLCM_TIP");
    	bForce720PFlag = FALSE;
    	bForce2MFlag = TRUE;
    }
    else if(strKey.compare("FORCE_720P_2048_FOR_PLCM_TIP")==0)
    {
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetIsForce720Pand2MForPLCMFlag : FORCE_720P_2048_FOR_PLCM_TIP is FORCE_720P_2048_FOR_PLCM_TIP");
    	bForce720PFlag = TRUE;
    	bForce2MFlag = TRUE;
    }
    else if(strKey.compare("NO_FORCE")==0)
    {
    	bForce720PFlag = FALSE;
    	bForce2MFlag = FALSE;
    }
}

//_t_p_
/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::InitVideoOutTipPolycomIfNeeded(CBridgePartyVideoOutParams* pOutVideoParams, CIpComMode* pScm)
{
	if (!GetIsTipCall() && !IsTipSlavePartyType())
		return;

	if (!m_bIsNeedToChangeVideoOutTipPolycom)
		return;

	PTRACE(eLevelInfoNormal,"CIpPartyCntl::InitVideoOutTipPolycomIfNeeded");

	DBGPASSERT_AND_RETURN(!pOutVideoParams);

	CIpComMode* pCommMode;
	DWORD currentBitRate 			= pScm->GetVideoBitRate(cmCapTransmit,kRolePeople);
	DWORD newBitRate 				= 2048;
	BYTE bForce720PFlag 			= FALSE;
	BYTE bForce2MFlag				= FALSE;

	if(currentBitRate <= newBitRate*10)
		return;

	GetIsForce720Pand2MForPLCMFlag(bForce720PFlag, bForce2MFlag);

	if (bForce720PFlag)
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::InitVideoOutTipPolycomIfNeeded - FORCE_720P_FOR_TIP_POLYCOM = YES ");

		pOutVideoParams->SetFS(15);
		pOutVideoParams->SetMBPS(216);
		pOutVideoParams->SetVideoResolution(eVideoResolutionHD720);
	}

	if (bForce2MFlag)
	{
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitVideoOutTipPolycomIfNeeded - Set video out bit rate to ", newBitRate);

		pOutVideoParams->SetVideoBitRate(newBitRate * 1000);
	}
}

/////////////////////////////////////////////////////////////////////////////
int  CIpPartyCntl::UpdateVideoRelayInBridgeIfNeeded(BYTE bTakeInitial, BYTE bForceUpdate)
{
	DWORD details = 0;
	BYTE bVideoInDiff = NO;
	BYTE bRet = FALSE;

	if (m_pIpInitialMode->GetConfType() == kVSW_Fixed)
	{
		PTRACE2PARTYID(eLevelError,"CIpPartyCntl::UpdateVideoRelayInBridgeIfNeeded - Can not send update in case of vsw fixed : Name - ",m_partyConfName, GetPartyRsrcId());
		DBGPASSERT(GetPartyRsrcId());
		return NO;
	}

	if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapReceive) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive))
	{
		bVideoInDiff  = !m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapReceive);
		if (bVideoInDiff == FALSE)
			bVideoInDiff = !m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapReceive);

		if (bVideoInDiff == FALSE && bForceUpdate)
		{
			PTRACE2PARTYID(eLevelError,"CIpPartyCntl::UpdateVideoRelayInBridgeIfNeeded - No Update is needed but Force update is true : Name - ",m_partyConfName, GetPartyRsrcId());
			bVideoInDiff = TRUE;
		}
		// temp - Always send 1st update to VideoRelayBridge because:
		// 1) bVideoInDiff is not always accurate for SVC
		// 2) There might be cases that the scm has not changed, but the channel handle is new / changed

		BYTE bVideoRelayInReady = IsRelayChannelReadyForMedia(cmCapReceive);

		if(bVideoInDiff || (!m_bVideoRelayInReady && bVideoRelayInReady))
		{
			m_bVideoRelayInReady = bVideoRelayInReady;
			if (m_bVideoRelayInReady || bForceUpdate)
			{
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayInBridgeIfNeeded (Update Bridge): Name - ",m_partyConfName, GetPartyRsrcId());
			CBridgePartyVideoRelayMediaParams *pInVideoParams = new CBridgePartyVideoRelayMediaParams;

				CDwordBitMask muteMask;
				SetPartyVideoMute(muteMask);
				pInVideoParams->SetMuteMask(muteMask);

			FillStreamSSRCForMedia(pInVideoParams,cmCapVideo,cmCapReceive,REASON_UPDATE);
			PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayInBridgeIfNeeded (Update Bridge): m_incomingVideoChannelHandle: ", m_incomingVideoChannelHandle);
				pInVideoParams->SetChannelHandle(m_incomingVideoChannelHandle);
				pInVideoParams->SetIsReady(m_bVideoRelayInReady);


				CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
				if(pCommConf->GetConfMediaType() == eMixAvcSvc )
				{
				    TRACEINTOFUNC << "mix_mode: m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle;
				    if(m_pIpInitialMode->GetConfMediaType()==eMixAvcSvc)
				    {
				    	pInVideoParams->SetSupportSvcAvcTranslate(true);
				    	TRACEINTO<<"!@# setting SupportAvcSvcTranslate to true";
				    }
				    else
				    {
				    	pInVideoParams->SetSupportSvcAvcTranslate(false);
				    	TRACEINTO<<"!@# setting SupportAvcSvcTranslate to false";
				    }
				}
				else
				{
					TRACEINTO<<"!@# conf is other than eMixAvcSvc, no need to set IsSupportAvcSvcTran";
				}
				pInVideoParams->SetIsCascadeLink(false);
				CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
				if (pConfParty && ((pConfParty->GetCascadeMode() == CASCADE_MODE_MASTER) || (pConfParty->GetCascadeMode() == CASCADE_MODE_SLAVE)))
				{
					TRACEINTO << "set IsCascadeLink to true";
					pInVideoParams->SetIsCascadeLink(true);
				}

				m_pVideoBridgeInterface->UpdateVideoRelayInParams(m_pParty, pInVideoParams);
				bRet = TRUE;

				POBJDELETE(pInVideoParams);

			}
			else
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayInBridgeIfNeeded Video In Relay channel is not ready! -->(No Update): Name - ",m_partyConfName, GetPartyRsrcId());
			}

		}
		else
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayInBridgeIfNeeded (No Update): Name - ",m_partyConfName, GetPartyRsrcId());
	}

	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayInBridgeIfNeeded, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
int  CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded(BYTE bTakeInitial,BYTE bForceUpdate)
{
	DWORD details = 0;
	BYTE bVideoOutDiff = NO;
	BYTE bRet = FALSE;

	if (m_pIpInitialMode->GetConfType() == kVSW_Fixed || m_pIpInitialMode->GetConfType() == kCop)
	{
		PTRACE2PARTYID(eLevelError,"CIpPartyCntl::UpdateVideoOutBridgeIfNeeded - VideoRelay parties are not supported in COP / VSW-Fixed conf - should not get here! Name - ",m_partyConfName, GetPartyRsrcId());
		DBGPASSERT(GetPartyRsrcId());
		return NO;
	}

    DWORD iniContentRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
    DWORD iniPeopleRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);

	if(m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit))
	{
		bVideoOutDiff  = !m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate|kBitRateWithoutTolerance|kNumOfStreamDesc,&details,cmCapVideo,cmCapTransmit);
		if (bVideoOutDiff == FALSE)
			bVideoOutDiff = !m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate|kBitRateWithoutTolerance|kNumOfStreamDesc,&details,cmCapVideo,cmCapTransmit);

		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded -this is  m_currentVideoEncRate - ",m_currentVideoEncRate);
		if (m_currentVideoEncRate != 0 && !bVideoOutDiff && ((m_currentVideoEncRate/100 ) != iniPeopleRate) && m_pIpInitialMode->GetConfType() == kCp && bTakeInitial)
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded -UPDATING ALTOUGH NO DIFF: Name - ",m_partyConfName, GetPartyRsrcId());
			bVideoOutDiff = TRUE;
		}
		// temp - Always send 1st update to VideoRelayBridge because:
		// 1) bVideoOutDiff is not always accurate for SVC
		// 2) There might be cases that the scm has not changed, but the channel handle is new / changed
		BYTE bVideoRelayOutReady = IsRelayChannelReadyForMedia(cmCapTransmit);

		if (bForceUpdate || bVideoOutDiff || (!m_bVideoRelayOutReady && bVideoRelayOutReady))
		{
			m_bVideoRelayOutReady = bVideoRelayOutReady;

			if (m_bVideoRelayOutReady || bForceUpdate)
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded (Update Bridge): Name - ",m_partyConfName, GetPartyRsrcId());
				CBridgePartyVideoRelayMediaParams *pOutVideoParams = new CBridgePartyVideoRelayMediaParams;

				FillStreamSSRCForMedia(pOutVideoParams,cmCapVideo,cmCapTransmit,REASON_UPDATE);
				
				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded (Update Bridge): m_outgoingVideoChannelHandle: ", m_outgoingVideoChannelHandle);
				pOutVideoParams->SetChannelHandle(m_outgoingVideoChannelHandle);
				pOutVideoParams->SetIsReady(m_bVideoRelayOutReady);
				pOutVideoParams->SetScpRequestSequenceNumber( GetScpNotificationRemoteSeqNumber() );

				DWORD ivrSlideSsrc= m_pIpInitialMode->GetIvrSsrc(cmCapVideo);
				pOutVideoParams->SetPipeIdForIvrSlide(ivrSlideSsrc);

				pOutVideoParams->SetIsCascadeLink(false);
				CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
				if (pConfParty && ((pConfParty->GetCascadeMode() == CASCADE_MODE_MASTER) || (pConfParty->GetCascadeMode() == CASCADE_MODE_SLAVE)))
				{
					TRACEINTO << "set IsCascadeLink to true";
					pOutVideoParams->SetIsCascadeLink(true);
				}

				m_pVideoBridgeInterface->UpdateVideoRelayOutParams(m_pParty, pOutVideoParams);
				bRet = TRUE;
				POBJDELETE(pOutVideoParams);
			}
			else
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded Video Out Relay channel is not ready! -->(No Update): Name - ",m_partyConfName, GetPartyRsrcId());
			}

//							TRACEINTO<<"CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded GetScpNotificationRemoteSeqNumber()"<<GetScpNotificationRemoteSeqNumber();
		}
		else
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded (No Update): Name - ",m_partyConfName, GetPartyRsrcId());
	}
	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::FillStreamSSRCForMedia(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoParams,cmCapDataType aMediaType,cmCapDirection direction,int reason,ERoleLabel eRole)
{
	std::ostringstream ostr;
	static int callIdx = 0;
	const std::list <StreamDesc> streamsDescList = m_pIpInitialMode->GetStreamsListForMediaMode(aMediaType,direction,eRole);
	std::list <StreamDesc>::const_iterator itr_streams;
	VIDEO_OPERATION_POINT_SET_S* vopS = m_pIpInitialMode->GetOperationPoints(cmCapVideo,cmCapReceive,eRole);
	DWORD partyId=m_pIpInitialMode->GetPartyId();

	PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::FillStreamSSRCForMedia direction = ", direction);
	
	if (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw && !m_bIsMrcCall)
	{
	    vopS = new VIDEO_OPERATION_POINT_SET_S;
	    ::FillVideoOperationSetParams (*vopS, m_pIpInitialMode->GetOperationPoints());
	}

	if (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvcVsw)
	{
		ostr << "avc_vsw_relay for partyId: " << partyId;
	}
	else
	{
		ostr << "(psuedo) avc_vsw_relay for partyId: " << partyId;
	}
	
	if (reason == REASON_CONNECT)
	{
		ostr << ", bridge connecting";
	}
	else
	{
		ostr << ", bridge updating";
	}
	
	if (direction == cmCapTransmit)
	{
		ostr << ", transmit";
	}
	else
	{
		ostr << ", receive";
	}

	if (vopS)
	{
		int streamIdx=0;
		
		for (itr_streams = streamsDescList.begin(); itr_streams != streamsDescList.end(); itr_streams++)
		{
			CVideoRelayMediaStream* pVideoMediaStream = NULL; AUTO_DELETE(pVideoMediaStream);

			if (direction == cmCapTransmit)
			{
				CVideoRelayOutMediaStream *pVideoOutMediaStream = new CVideoRelayOutMediaStream;
				
				pVideoOutMediaStream->SetIsSpecificSourceCsrc(itr_streams->m_specificSourceSsrc);
				
				if (itr_streams->m_specificSourceSsrc)
				{
					TRACEINTO << " m_specificSourceSsrc, m_sourceIdSsrc=" << (DWORD)(itr_streams->m_sourceIdSsrc);
					pVideoOutMediaStream->SetCsrc(itr_streams->m_sourceIdSsrc);
				}

				pVideoOutMediaStream->SetPriority(itr_streams->m_priority);

				if (!m_bIsMrcCall && m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
				{
					TRACEINTOFUNC << "avc_vsw_relay pVideoOutMediaStream->SetTemporalScalabilitySupported(false)";
					pVideoOutMediaStream->SetTemporalScalabilitySupported(false);
				}
				else
				{
					TRACEINTOFUNC << "avc_vsw_relaypVideoOutMediaStream->SetTemporalScalabilitySupported(true)";

					pVideoOutMediaStream->SetTemporalScalabilitySupported(true);
				}

				pVideoMediaStream = pVideoOutMediaStream;
			}
			else if (direction == cmCapReceive)
			{
				CVideoRelayInMediaStream *pVideoInMediaStream = new CVideoRelayInMediaStream;
				pVideoInMediaStream->m_scpPipe = itr_streams->m_scpNotificationParams;

				pVideoMediaStream = pVideoInMediaStream;
			}

			else
				pVideoMediaStream = new CVideoRelayMediaStream;

			if (pVideoMediaStream)
			{
				pVideoMediaStream->SetSsrc(itr_streams->m_pipeIdSsrc);

				if (aMediaType == cmCapVideo)
				{
					int layerId = ILLEGAL_LAYER_ID;
					
					if (itr_streams->m_isLegal)
						layerId = ::GetLayerId(*vopS, itr_streams->m_width, itr_streams->m_height, itr_streams->m_frameRate, itr_streams->m_bitRate, direction);
			
					if (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
					{
						ostr<<" layerId:"<<layerId<<" itr_streams->m_width:"<<itr_streams->m_width<<" itr_streams->m_height: "<<itr_streams->m_height<<" itr_streams->m_frameRate:"<<itr_streams->m_frameRate;
					}

					if (layerId == ILLEGAL_LAYER_ID && itr_streams->m_isLegal)
					{
						TRACEINTOFUNC << "PartId: " << this->GetPartyId() << "can't find a valid operation point for stream no. " << streamIdx << " (call index: " << callIdx << "): Height: " << itr_streams->m_height << ", width: " << itr_streams->m_width << ", layerId: " << layerId << ", ssrc: " << pVideoMediaStream->GetSsrc();
					}
					else
					{
						 std::ostringstream ostrin;
						 
						 ostrin << "CIpPartyCntl::FillStreamSSRCForMedia PartId: " << this->GetPartyId() << " ---- stream no. " << streamIdx << " (call index: " << callIdx << "): Height: " << itr_streams->m_height << ", width: " << itr_streams->m_width << ", layerId: " << layerId << ", ssrc: " << pVideoMediaStream->GetSsrc();
						 
						 if (direction == cmCapTransmit)
						 {
							 ostrin << ", for out stream priority=" <<  itr_streams->m_priority;
						 }
						 
						 PTRACE(eLevelInfoHigh,ostrin.str().c_str());
					}

					pVideoMediaStream->SetResolutionHeight(itr_streams->m_height);
					pVideoMediaStream->SetResolutionWidth(itr_streams->m_width);
					pVideoMediaStream->SetLayerId(layerId);
					pVideoMediaStream->SetIsVswStream(itr_streams->m_isAvcToSvcVsw);

					pBridgePartyVideoParams->m_pStreamsList.push_back(pVideoMediaStream);
					pVideoMediaStream = NULL;

					++streamIdx;
				} // end if (aMediaType == cmCapVideo)

			}
		} // end loop over streamsDescList

		if (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
		{
			PTRACE(eLevelInfoNormal, ostr.str().c_str());
		}

	    if (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw && !m_bIsMrcCall)
	    {
	        delete vopS;
	    }
	} // end if (vopS)
	else
	{
		PTRACE(eLevelInfoNormal,"pStreamGroupStruct is not valid!");
	}

	++callIdx;
}

/////////////////////////////////////////////////////////////////////////////
bool CIpPartyCntl::FillStreamSSRCForMedia_noRelay(CAvcToSvcParams* pAvcToSvcParams)
{
  bool result=true;

    std::ostringstream ostr;

    TRACEINTOFUNC << "CIpPartyCntl::FillStreamSSRCForMedia_noRelay";

    m_pIpInitialMode->Dump("CIpPartyCntl::FillStreamSSRCForMedia_noRelay m_pIpInitialMode",eLevelInfoNormal);

	const std::list <StreamDesc> streamsDescList =
		m_pIpInitialMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
    std::list <StreamDesc>::const_iterator itr_streams;

    int streamIdx=0;

	for (itr_streams = streamsDescList.begin(); itr_streams != streamsDescList.end() ; itr_streams++)
    {
    	TRACEINTOFUNC << "mix_mode: stream #" << streamIdx;

        CVideoRelayMediaStream *pVideoMediaStream = NULL;
        CVideoRelayInMediaStream *pVideoInMediaStream = new CVideoRelayInMediaStream;
        pVideoInMediaStream->m_scpPipe = itr_streams->m_scpNotificationParams;

        pVideoMediaStream = pVideoInMediaStream;
        pVideoMediaStream->SetSsrc(itr_streams->m_pipeIdSsrc);

		int layerId = pAvcToSvcParams->GetVideoOperationPointsSet()->GetLayerId(itr_streams->m_width,
			itr_streams->m_height, itr_streams->m_frameRate, itr_streams->m_bitRate, cmCapReceive);

        if (layerId == -1)
        {
	  result=false;
			TRACEINTOFUNC << "mix_mode: can't find a valid operation point for stream no. "
				<< streamIdx <<  ": Height: " << itr_streams->m_height
				<< ", width: " << itr_streams->m_width << ", layerId: " << layerId
				<< ", ssrc: " << pVideoMediaStream->GetSsrc();
		}
		else
		{
			TRACEINTOFUNC << "mix_mode: ---- stream no. "
				<< streamIdx << ": Height: " << itr_streams->m_height
				<< ", width: " << itr_streams->m_width << ", layerId: " << layerId
				<< ", ssrc: " << pVideoMediaStream->GetSsrc();
        }

        pVideoMediaStream->SetResolutionHeight(itr_streams->m_height);
        pVideoMediaStream->SetResolutionWidth(itr_streams->m_width);
        pVideoMediaStream->SetLayerId(layerId);
		pVideoMediaStream->SetIsVswStream(itr_streams->m_isAvcToSvcVsw);

        pAvcToSvcParams->m_listVideoRelayMediaStreams.push_back(pVideoMediaStream);

        ++streamIdx;
	} // end loop over streamsDescList

	if (streamIdx==0)
    {
      result=false;
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////
EUpdateBridgeMediaAndDirection CIpPartyCntl::UpdateLprVideoOutBridgeRate(DWORD lossProtection, DWORD mtbf, DWORD congestionCeiling,
				DWORD fill, DWORD modeTimeout, BYTE bTakeInitial, DWORD RemoteIdent )
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateLprVideoOutBridgeRate : Name - ",m_partyConfName, GetPartyRsrcId());
	int updateBridges = eNoUpdate;

	if (m_eVidBridgeConnState & eSendOpenOut || ( MicrosoftEP_Lync_CCS == RemoteIdent))
	{
		DWORD details = 0;
		BYTE bVideoOutDiff = NO;
		ERoleLabel eRole = kRolePeople;
		if ( MicrosoftEP_Lync_CCS == RemoteIdent)
		{
			eRole = kRolePresentation;
		}
		if(m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,eRole) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit,eRole))
		{
			bVideoOutDiff  = !m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kCapCode|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapTransmit);
			if (bVideoOutDiff == FALSE)
				bVideoOutDiff = !m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapTransmit);

			if(bVideoOutDiff || ( MicrosoftEP_Lync_CCS == RemoteIdent))
			{
				if (m_pIpInitialMode->GetConfType() == kVSW_Fixed)
				{
					PTRACE2PARTYID(eLevelError,"CIpPartyCntl::UpdateLprVideoOutBridgeRate - Can not send update in case of vsw fixed: Name - ",m_partyConfName, GetPartyRsrcId());
					DBGPASSERT(GetPartyRsrcId());
					return (EUpdateBridgeMediaAndDirection)updateBridges;
				}

				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateLprVideoOutBridgeRate (Update Bridge): Name - ",m_partyConfName, GetPartyRsrcId());
				CBridgePartyVideoOutParams *outVideoParams = new CBridgePartyVideoOutParams;

				CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
				BYTE partyResolution = eAuto_Res;
				if (pConfParty)
				    partyResolution =  pConfParty->GetMaxResolution();
				else
				{
				    PTRACE(eLevelError,"CIpPartyCntl::UpdateLprVideoOutBridgeRate - pConfParty is NULL - set eAuto_Res");
				    DBGPASSERT(1114);
				}
				BYTE maxConfResolution = m_pConf->GetCommConf()->GetConfMaxResolution();
				if ( MicrosoftEP_Lync_CCS != RemoteIdent)
				{
					if(bTakeInitial)
					{
						InitVideoOutParams(outVideoParams, m_pIpInitialMode, m_isLprActive);
						if(partyResolution == eAuto_Res && maxConfResolution == eAuto_Res)
							updateResolutionAccordingToNewVideoRate( outVideoParams, m_pIpInitialMode);
					}
					else
					{
						InitVideoOutParams(outVideoParams, m_pIpCurrentMode);
						if(partyResolution == eAuto_Res && maxConfResolution == eAuto_Res)
							updateResolutionAccordingToNewVideoRate( outVideoParams, m_pIpCurrentMode);
					}
				}

				outVideoParams->SetRemoteIdent(RemoteIdent);
				//Ack params are add to update msg when needed
				//for acknowledge msg from bridge
				outVideoParams->SetLPRParams(lossProtection,mtbf,congestionCeiling,fill,modeTimeout);

				outVideoParams->SetIsCascadeLink(GetPartyCascadeType() == CASCADE_MCU);
				m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);

				POBJDELETE(outVideoParams);
			}
			else
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateLprVideoOutBridgeRate (No Update): Name - ",m_partyConfName, GetPartyRsrcId());
		}
		else
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateLprVideoOutBridgeRate, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());

		if(bVideoOutDiff)
			updateBridges += eUpdateVideoOut;
	}
	else
		PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateLprVideoOutBridgeRate : No OUT Connection", GetPartyRsrcId());

	return (EUpdateBridgeMediaAndDirection)updateBridges;
}
/////////////////////////////////////////////////////////////////////////////
EUpdateBridgeMediaAndDirection CIpPartyCntl::UpdateFecOrRedVideoOutBridgeRate()
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateFecOrRedVideoOutBridgeRate : Name - ",m_partyConfName, GetPartyRsrcId());

	int updateBridges = eNoUpdate;
	if (m_eVidBridgeConnState & eSendOpenOut)
		{

			if(m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit))
			{
				CBridgePartyVideoOutParams *outVideoParams = new CBridgePartyVideoOutParams;
				InitVideoOutParams(outVideoParams, m_pIpInitialMode);

				m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);

				POBJDELETE(outVideoParams);

				updateBridges += eUpdateVideoOut;

			}
			else
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateFecOrRedVideoOutBridgeRate, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());
		}
		else
			PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateFecOrRedVideoOutBridgeRate : No OUT Connection", GetPartyRsrcId());

	return (EUpdateBridgeMediaAndDirection)updateBridges;

}
/////////////////////////////////////////////////////////////////////////////
void  CIpPartyCntl::UpdateVideoOutBridgeH239Case(BYTE bTakeInitial)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case (Update Bridge): Name - ",m_partyConfName, GetPartyRsrcId());
	
	if (m_pIpInitialMode->GetIsTipMode())
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case : Tip call");
		return;
	}
	
    if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit))
	{
		CBridgePartyVideoOutParams *outVideoParams = new CBridgePartyVideoOutParams;
		
		if (bTakeInitial)
			InitVideoOutParams(outVideoParams, m_pIpInitialMode);
		else
			InitVideoOutParams(outVideoParams, m_pIpCurrentMode);
		
		DWORD currentVideoRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit,(ERoleLabel)kRolePeople);
		DWORD iniContentRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
		DWORD newVideoEncRate = 0;

		CSmallString str;
		DWORD iniVideoOutRate = 0;
		
		if (m_pIpInitialMode->GetConfType() == kCop)
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case (Update Bridge): cop Name - ",m_partyConfName, GetPartyRsrcId());
			if (m_isLprActive == 0)
			{
				str <<  "(NO LPR) ";
				iniVideoOutRate = m_pIpInitialMode->GetVideoBitRate(cmCapTransmit, (ERoleLabel)kRolePeople);
			}
			else
			{
				iniVideoOutRate =m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);
				str <<  "(LPR Case) ";
			}
		}
		else
		{
			if (m_isLprActive == 0)
			{// LPR not active
				iniVideoOutRate = m_pIpInitialMode->GetTotalVideoRate();
				str <<  "(NO LPR) ";
			}
			else
			{
				iniVideoOutRate = currentVideoRate;
				str <<  "(LPR Case) ";
			}
		}

		str <<  "chosen video rate - " << iniVideoOutRate;
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case: ",str.GetString(), GetPartyRsrcId());
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case (Update Bridge): h323 rate - ",iniVideoOutRate);
		
		newVideoEncRate = (iniVideoOutRate - iniContentRate); //* 100;
		
		if (newVideoEncRate < 640)
			newVideoEncRate = 640; // LPR constraint

		//BRIDGE-13154
		DWORD currentCallRate  = m_pIpInitialMode->GetCallRate();
		CCommConf* pCommConf   = (CCommConf*)m_pConf->GetCommConf();
		
		if(pCommConf == NULL)
		{
			PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case - pCommConf is NULL ");
			POBJDELETE(outVideoParams);
			DBGPASSERT_AND_RETURN(pCommConf == NULL);
		}
		
		TRACEINTO  << " currentCallRate " << (int)currentCallRate 
			<< " iniContentRate " << (int)iniContentRate  
			<< " newVideoEncRate " << (int)newVideoEncRate;

		if (iniContentRate &&
			currentCallRate < 768 &&
			((newVideoEncRate + iniContentRate) > (currentCallRate*10)) &&
			pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP &&
			GetBOOLDataByKey(CFG_KEY_ENABLE_CONTENT_IN_PREFER_TIP_FOR_CALL_RATES_LOWER_THAN_768K)
		  )
		{
			TRACEINTO  << " ENABLE_CONTENT_IN_PREFER_TIP_FOR_CALL_RATES_LOWER_THAN_768K is YES - setting the new video rate to 64k";
			newVideoEncRate = 640;
		}

		if(m_pIpInitialMode->GetConfType() == kCp && GetInterfaceType() == H323_INTERFACE_TYPE && m_isLprActive)// in case of LPR H323 CP we already remove the content rate from the people in ChangeScmAccordingToH323Scm
		{
			newVideoEncRate = iniVideoOutRate;
		}
		else if((newVideoEncRate < 640) || (iniVideoOutRate < iniContentRate))
			newVideoEncRate = 640; // LPR constraint

		if ((currentVideoRate < newVideoEncRate) && iniContentRate) //content is active but current is lower than required rate
		{
			CSmallString str;
			str <<  "newVideoEncRate - " << newVideoEncRate << ", currentVideoRate - " << currentVideoRate << ", iniContentRate - " << iniContentRate << ", iniVideoOutRate - " << iniVideoOutRate;
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case video rate is low enough in order to stay in the same rate when there is content. ",str.GetString(), GetPartyRsrcId());
			newVideoEncRate = currentVideoRate;
			POBJDELETE(outVideoParams);

			return;
		}

		newVideoEncRate = newVideoEncRate * 100;

		CConfParty* pConfParty = NULL;

		pConfParty = pCommConf->GetCurrentParty(GetName());
		BYTE partyResolution = eAuto_Res;
		
		if (pConfParty)
		    partyResolution =  pConfParty->GetMaxResolution();
		else
		{
		    PTRACE(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case - pConfParty is NULL - set eAuto_Res");
		    DBGPASSERT(1115);
		}

		BYTE maxConfResolution = pCommConf->GetConfMaxResolution();
		outVideoParams->SetIsCascadeLink(GetPartyCascadeType() == CASCADE_MCU);

		if (newVideoEncRate != outVideoParams->GetVideoBitRate() || ( newVideoEncRate != currentVideoRate && m_pIpInitialMode->GetConfType() == kCop))
		{ // For downgrading rate
			TRACEINTO << "downgrading rate";

			m_currentVideoEncRate = newVideoEncRate;
			
			if ( !strncmp(m_productId, "RADVision ViaIP GW", strlen("RADVision ViaIP GW")) )
			{
				DWORD bitRateReductionPrc = 0; // default value
				CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				std::string key = "RV_GW_VIDEO_RATE_REDUCTION_PERCENTAGE";
				sysConfig->GetDWORDDataByKey(key, bitRateReductionPrc);
				if(bitRateReductionPrc)
				{
					bitRateReductionPrc = 100 - bitRateReductionPrc;
					newVideoEncRate = ((bitRateReductionPrc * newVideoEncRate) / 100 );
				}

				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case downgrade - only for RV GW lowring transmit rate to decoder!",newVideoEncRate);
			}
			
			outVideoParams->SetVideoBitRate(newVideoEncRate);
			
			if (partyResolution == eAuto_Res && maxConfResolution == eAuto_Res)
				updateResolutionAccordingToNewVideoRate(outVideoParams,m_pIpCurrentMode);
			
			if ( m_pIpInitialMode->GetConfType() == kCp )
				m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);
			else if(m_pIpInitialMode->GetConfType() == kCop)
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case (Update Bridge):update using flow control downgrade Name - ",m_partyConfName, GetPartyRsrcId());
				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case (Update Bridge):update using flow control downgrade rate - ",newVideoEncRate);
				DWORD flowControlRate = m_pIpCurrentMode->GetFlowControlRateConstraint();
				BYTE isupdateToLowerRate = TRUE;
				if(flowControlRate && (flowControlRate * 100) < newVideoEncRate)
				{
					PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case downgrade -do not update there is flow control limit - ",newVideoEncRate);
					isupdateToLowerRate = FALSE;
				}
				if(isupdateToLowerRate && m_pParty)
					m_pTaskApi->UpdateVideoBridgeFlowControlRate(((CParty*)m_pParty)->GetPartyRsrcID(), newVideoEncRate, cmCapTransmit, kRolePeople, FALSE,  NULL);
			}
		}
		else
		{ // For upgrading rate
			if  (m_currentVideoEncRate != newVideoEncRate)
			{
				TRACEINTO << "m_currentVideoEncRate=" << m_currentVideoEncRate << " newVideoEncRate=" << newVideoEncRate;

				m_currentVideoEncRate = newVideoEncRate;
				if ( !strncmp(m_productId, "RADVision ViaIP GW", strlen("RADVision ViaIP GW")) )
				{
					DWORD bitRateReductionPrc = 0; // default value
					CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
					std::string key = "RV_GW_VIDEO_RATE_REDUCTION_PERCENTAGE";
					sysConfig->GetDWORDDataByKey(key, bitRateReductionPrc);
					if(bitRateReductionPrc)
					{
						bitRateReductionPrc = 100 - bitRateReductionPrc;
						newVideoEncRate = ((bitRateReductionPrc * newVideoEncRate) / 100 );
					}

					PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case - only for RV GW lowring transmit rate to decoder!",newVideoEncRate);
				}
				outVideoParams->SetVideoBitRate(newVideoEncRate);
				if(partyResolution == eAuto_Res && maxConfResolution == eAuto_Res)
					updateResolutionAccordingToNewVideoRate(outVideoParams,m_pIpCurrentMode);
				if( m_pIpInitialMode->GetConfType() == kCp )
					m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);
				else if(m_pIpInitialMode->GetConfType() == kCop)
				{
					PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case (Update Bridge):update using flow control upgrade Name - ",m_partyConfName, GetPartyRsrcId());
					PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case (Update Bridge):update using flow control upgrade rate - ",newVideoEncRate);
					BYTE isupdateToLowerRate = TRUE;
					DWORD flowControlRate = m_pIpCurrentMode->GetFlowControlRateConstraint();
					if(flowControlRate && (flowControlRate * 100) < newVideoEncRate)
					{
						PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case ugrade -do not update there is flow control limit - ",flowControlRate);
						isupdateToLowerRate = FALSE;
					}
					if(isupdateToLowerRate && m_pParty)
						m_pTaskApi->UpdateVideoBridgeFlowControlRate(((CParty*)m_pParty)->GetPartyRsrcID(), newVideoEncRate, cmCapTransmit, kRolePeople, FALSE,  NULL);
				}
			}
			else
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case (No Update): Name - ",m_partyConfName, GetPartyRsrcId());
		}
		POBJDELETE(outVideoParams);
	}
	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoOutBridgeH239Case, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////
int  CIpPartyCntl::UpdateAudioInBridgeIfNeeded()
{
	CMedString str;
	DWORD details = 0;
	BYTE bAudioInDiff = NO;
    //Audio Patch
    //if(m_pIpCurrentMode->IsMediaOn(cmCapAudio,cmCapReceive))
	if (m_pIpInitialMode->IsMediaOn(cmCapAudio,cmCapReceive) && m_pIpCurrentMode->IsMediaOn(cmCapAudio,cmCapReceive))
    {
		bAudioInDiff  = !m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode,&details,cmCapAudio,cmCapReceive);

		/* In case of fallback update the encoder/decoder anyway */
		if (m_isTipFallbackFlow)
			bAudioInDiff = true;

		if(bAudioInDiff)
		{
			str << " Name - " << m_partyConfName;
			CapEnum inAudioAlg  = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapAudio, cmCapReceive);
			DWORD bitRate = m_pIpInitialMode->GetMediaBitRate(cmCapAudio, cmCapReceive);
			CComModeInfo inInfo = inAudioAlg;
			WORD inAlgForAB     = inInfo.GetH320ModeType();
			str << ", In update Alg - " << inInfo.GetH323CapName();
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioInBridgeIfNeeded (Update Bridge):",str.GetString(), GetPartyRsrcId());

			m_pAudioInterface->UpdateAudioAlgorithm(GetPartyRsrcId(), eMediaIn, inAlgForAB, bitRate);
			m_pIpCurrentMode->SetAudioAlg( inInfo.GetH323ModeType(),cmCapReceive);
		}
		else
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioInBridgeIfNeeded (No Update): Name - ",m_partyConfName, GetPartyRsrcId());


		if (m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw/*temp - should check if ssrc changed*/)
		{
			// temp - should use Use FillStreamSSRCForMedia for Audio (as for Video) ??? need Bridge support for that
			DWORD ssrc = 0;
			const std::list <StreamDesc> streamsDescList = m_pIpInitialMode->GetStreamsListForMediaMode(cmCapAudio, cmCapReceive, kRolePeople);
			std::list <StreamDesc>::const_iterator itr_streams;
			if (streamsDescList.size()>0)
			{
				itr_streams = streamsDescList.begin();
				ssrc = itr_streams->m_pipeIdSsrc;
			}
			//APIU32* aSsrcIds = NULL;
		//	m_pIpInitialMode->GetSsrcIds(cmCapAudio, cmCapReceive, aSsrcIds, &aNumOfSsrcIds);
			//
		//	PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioInBridgeIfNeeded receive aNumOfSsrcIds=",aNumOfSsrcIds);
			if (ssrc)
			{
				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioInBridgeIfNeeded receive ssrc=",ssrc);
				m_pAudioInterface->UpdateAudioRelayParamsIn(GetPartyRsrcId(), ssrc);
			}
            // delete[] aSsrcIds;
		}

		else if( eMediaTypeUpdateNotNeeded != m_isAudioDecoderUpdateNeeded )
		{
			if( eMediaTypeUpdateAudioToVideo == m_isAudioDecoderUpdateNeeded )
		        m_pAudioInterface->UpdateMediaType(GetPartyRsrcId(), eMediaTypeUpdateAudioToVideo);
		    else
		    {
		    	if( eMediaTypeUpdateVideoToAudio == m_isAudioDecoderUpdateNeeded )
		    		m_pAudioInterface->UpdateMediaType(GetPartyRsrcId(), eMediaTypeUpdateVideoToAudio);
			}

			m_isAudioDecoderUpdateNeeded = eMediaTypeUpdateNotNeeded; //Reset
		 }

	}
	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioInBridgeIfNeeded, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());

	return bAudioInDiff;
}

/////////////////////////////////////////////////////////////////////////////
int  CIpPartyCntl::UpdateAudioOutBridgeIfNeeded()
{
	CMedString str;
	DWORD details = 0;
	BYTE bAudioOutDiff = NO;
    //Audio Patch
    //if (m_pIpCurrentMode->IsMediaOn(cmCapAudio,cmCapTransmit))
	if(m_pIpInitialMode->IsMediaOn(cmCapAudio,cmCapTransmit) && m_pIpCurrentMode->IsMediaOn(cmCapAudio,cmCapTransmit))
	{
		bAudioOutDiff  = !m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kCapCode,&details,cmCapAudio,cmCapTransmit);

		/* In case of fallback update the encoder/decoder anyway */
		if (m_isTipFallbackFlow)
			bAudioOutDiff = true;

		if(bAudioOutDiff)
		{
			str << " Name - " << m_partyConfName;
			CapEnum outAudioAlg  = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapAudio, cmCapTransmit);
			DWORD bitRate = m_pIpInitialMode->GetMediaBitRate(cmCapAudio, cmCapTransmit);
			CComModeInfo outInfo = outAudioAlg;
			WORD outAlgForAB     = outInfo.GetH320ModeType();
			str << ", Out update Alg - " << outInfo.GetH323CapName();
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioOutBridgeIfNeeded (Update Bridge):",str.GetString(), GetPartyRsrcId());

			m_pAudioInterface->UpdateAudioAlgorithm(GetPartyRsrcId(), eMediaOut, outAlgForAB, bitRate);
		}
		else
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioOutBridgeIfNeeded (No Update): Name - ",m_partyConfName, GetPartyRsrcId());
		if (m_bIsMrcCall/*temp - should check if ssrc changed*/)
		{
			// temp - should use Use FillStreamSSRCForMedia for Audio (as for Video) ??? need Bridge support for that
			APIU32* aSsrcIds = NULL;
			const std::list <StreamDesc> streamsDescList = m_pIpInitialMode->GetStreamsListForMediaMode(cmCapAudio, cmCapTransmit, kRolePeople);
			std::list <StreamDesc>::const_iterator itr_streams;
			int aNumOfSsrcIds=streamsDescList.size();

			if (aNumOfSsrcIds>0)
			{
				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioOutBridgeIfNeeded transmit aNumOfSsrcIds=",aNumOfSsrcIds);

				aSsrcIds = new APIU32[aNumOfSsrcIds];
				int streamIdx=0;
				for(itr_streams = streamsDescList.begin();itr_streams != streamsDescList.end() ; itr_streams++)
				{
					aSsrcIds[streamIdx] = itr_streams->m_pipeIdSsrc;
					PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioOutBridgeIfNeeded transmit ssrc=",aSsrcIds[streamIdx]);
					streamIdx++;
				}
				DWORD ivrSsrc = 0;
				ivrSsrc = m_pIpInitialMode->GetIvrSsrc(cmCapAudio);
				m_pAudioInterface->UpdateAudioRelayParamsOut(GetPartyRsrcId(), aNumOfSsrcIds, aSsrcIds, ivrSsrc);
			}
            delete[] aSsrcIds;
		}


		// speakerIndication - update
		if ( ((IbmSametimeEp == m_eRemoteVendorIdent) || (IbmSametimeEp_Legacy == m_eRemoteVendorIdent)) && (false == m_bIsUseSpeakerSsrcForTx_TRUE_sent) )
		{
			m_pAudioInterface->UpdateUseSpeakerSsrcForTx(GetPartyRsrcId(), TRUE);
			m_bIsUseSpeakerSsrcForTx_TRUE_sent = true;

			TRACEINTO << "speakerIndication - updating UseSpeakerSsrcForTx (TRUE)";
		}
	}

	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateAudioOutBridgeIfNeeded, One SCM (Init or Current) media is OFF: Name - ",m_partyConfName, GetPartyRsrcId());

	return bAudioOutDiff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::CheckVideoBridgeEndUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges)
{
	BYTE bVideoBridgeEndUpdate = FALSE;

	if (status)
		PASSERT(status);
	else if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckVideoBridgeEndUpdate : Update has received after disconnect. Name - ",m_partyConfName, GetPartyRsrcId());
	}
	else
	{
		if (m_eUpdateState >= eUpdatedBridges)
		{
			m_eUpdateState = (EUpdateBridgeMediaAndDirection)(m_eUpdateState - eUpdatedBridges);
			if (!((m_eUpdateState & eUpdateVideoIn) || (m_eUpdateState & eUpdateVideoOut)))
			{
				bVideoBridgeEndUpdate = TRUE;
			}
		}
		else
		{
			DBGPASSERT(m_eUpdateState);
			PASSERT(m_eUpdateState);
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckVideoBridgeEndUpdate : m_eUpdateState is illegal. Name - ",m_partyConfName, GetPartyRsrcId());
			TRACEINTO << "m_eUpdateState=" << (int)m_eUpdateState << " eUpdatedBridges=" << eUpdatedBridges;
		}
	}

	return bVideoBridgeEndUpdate;
}


//////////////////////////////////////////////////
BYTE CIpPartyCntl::ChangeAudioBridgeStateAccordingToNewMode()
{
	BYTE bIsConnectToAudioBridge = FALSE;

	//Check Disconnect
	BYTE bIsDisconnectFromAudioBridge = DisconnectPartyFromAudioBridgeIfNeeded();

	//Check Connect
	if (!bIsDisconnectFromAudioBridge)
	{
		BYTE bNeedToConnectIn  = !(m_eAudBridgeConnState & eSendOpenIn)  && m_pIpInitialMode->IsMediaOn(cmCapAudio,cmCapReceive);
		BYTE bNeedToConnectOut = !(m_eAudBridgeConnState & eSendOpenOut) && m_pIpInitialMode->IsMediaOn(cmCapAudio,cmCapTransmit);
		bIsConnectToAudioBridge = (bNeedToConnectIn || bNeedToConnectOut);
		if (bIsConnectToAudioBridge)
		 	ConnectPartyToAudioBridge(m_pIpInitialMode);
	}

	return (bIsDisconnectFromAudioBridge || bIsConnectToAudioBridge);
}

//////////////////////////////////////////////////
BYTE CIpPartyCntl::DisconnectPartyFromAudioBridgeIfNeeded()
{
	EMediaDirection eDisconnectedDirection = eNoDirection;
	if ((m_eAudBridgeConnState & eSendOpenIn) && m_pIpInitialMode->IsMediaOff(cmCapAudio, cmCapReceive))
	{
		eDisconnectedDirection |= eMediaIn;
		int eInDisconnecting = ~eInConnected;
		m_eAudBridgeConnState &= (EBridgeConnectionState)eInDisconnecting;
	}

	if ((m_eAudBridgeConnState & eSendOpenOut) && m_pIpInitialMode->IsMediaOff(cmCapAudio, cmCapTransmit))
	{
		eDisconnectedDirection |= eMediaOut;
		int eOutDisconnecting = ~eOutConnected;
		m_eAudBridgeConnState &= (EBridgeConnectionState)eOutDisconnecting;
	}

	if (eDisconnectedDirection == eNoDirection)
		return FALSE;

	TRACEINTO
		<< m_partyConfName
		<< ", PartyId:" << GetPartyId()
		<< ", MediaDirection:" << eDisconnectedDirection
		<< ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eAudBridgeConnState);

	CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId(), eDisconnectedDirection);
	m_pConfAppMngrInterface->DisconnectPartyAudio(&bridgePartyDisconnectParams);
	return TRUE;
}

//////////////////////////////////////////////////
BYTE CIpPartyCntl::ChangeVideoBridgeStateAccordingToNewMode()
{
	BYTE bIsConnectToVideoBridge = FALSE;

	//Check Disconnect
	BYTE bIsDisconnectFromVideoBridge = DisconnectPartyFromVideoBridgeIfNeeded(m_pIpInitialMode);

	//Check Connect
	if (!bIsDisconnectFromVideoBridge && !m_isFaulty)
	{
		bIsConnectToVideoBridge = IsNeedToConnectToVideoBridge(m_pIpInitialMode);
		if (bIsConnectToVideoBridge)
		 	ConnectPartyToVideoBridge(m_pIpInitialMode);
	}

	return (bIsDisconnectFromVideoBridge || bIsConnectToVideoBridge);
}

//////////////////////////////////////////////////
BYTE CIpPartyCntl::IsNeedToDisconnectVideoIn(CIpComMode* pScm)
{
	if (pScm->IsMediaOff(cmCapVideo,cmCapReceive))
		return (m_eVidBridgeConnState & eSendOpenIn);
	else if (pScm->GetMediaBitRate(cmCapVideo, cmCapReceive) == 0) // another check since video rate can be 0 in receive because of secondary
	{
        if (m_bIsMrcCall && (m_pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation) > 0))
        {
            TRACEINTO << "CIpPartyCntl::IsNeedToDisconnectVideoIn - do not disconnect @@@ ";
            return FALSE;
        }
        return (m_eVidBridgeConnState & eSendOpenIn);
	}
	return FALSE;
}

//////////////////////////////////////////////////
BYTE CIpPartyCntl::IsNeedToDisconnectVideoOut(CIpComMode* pScm)
{
	if (pScm->IsMediaOff(cmCapVideo,cmCapTransmit))
		return (m_eVidBridgeConnState & eSendOpenOut);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::DisconnectPartyFromVideoBridgeIfNeeded(CIpComMode* pScm)
{
	EMediaDirection eDisconnectedDirection = eNoDirection;
	// VNGFE-787
	if (IsNeedToDisconnectVideoIn(pScm) && m_isCodianVcr == 0)
	{
		eDisconnectedDirection |= eMediaIn;
		int eInDisconnecting = ~eInConnected;
		m_eVidBridgeConnState &= (EBridgeConnectionState)eInDisconnecting;
		m_incomingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
	    m_bVideoRelayInReady = FALSE;
	    m_bVideoUpdateCount=VIDEO_UPDATE_COUNT0;
	}

	if (IsNeedToDisconnectVideoOut(pScm))
	{
		eDisconnectedDirection |= eMediaOut;
		int eOutDisconnecting = ~eOutConnected;
		m_eVidBridgeConnState &= (EBridgeConnectionState)eOutDisconnecting;
		m_outgoingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
	    m_bVideoRelayOutReady = FALSE;
	}

	if (eDisconnectedDirection == eNoDirection)
		return FALSE;

	TRACEINTO
		<< m_partyConfName
		<< ", PartyId:" << GetPartyId()
		<< ", MediaDirection:" << eDisconnectedDirection
		<< ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState)
		<< ", CascadeMode:" << (WORD)GetPartyCascadeType();

	m_pTaskApi->UpdateDB(m_pParty, PARTYHIGHPROFILE, FALSE);

	CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId(), eDisconnectedDirection);
	m_pConfAppMngrInterface->DisconnectPartyVideo(&bridgePartyDisconnectParams);
	return TRUE;
}

//////////////////////////////////////////////////
BYTE CIpPartyCntl::IsNeedToConnectToVideoBridge(CIpComMode* pScm)
{
	DWORD vidInRate = pScm->GetMediaBitRate(cmCapVideo, cmCapReceive); // another check since video rate can be 0 in receive because of secondary
	BYTE bNeedToConnectIn  = !(m_eVidBridgeConnState & eSendOpenIn)  && pScm->IsMediaOn(cmCapVideo,cmCapReceive) && vidInRate;
	BYTE bNeedToConnectOut = !(m_eVidBridgeConnState & eSendOpenOut) && pScm->IsMediaOn(cmCapVideo,cmCapTransmit);
	if ((bNeedToConnectIn && !m_isCodianVcr)|| bNeedToConnectOut)
		return TRUE;
	else
		return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::IsContentRateNeedToBeChanged()
{
	BYTE  bNeedToChangeContent = FALSE;
	DWORD initialContRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	DWORD currentContRate = m_pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

	if (m_bNoContentChannel || m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
	{	//note:
		//1- for not content eps currentContRate is always 0.
		//   This is true also in case of H239
		//2- (m_ConferenceContentRate != m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapReceive,kRoleContentOrPresentation);) means that we got a new scm with a change in the content
		bNeedToChangeContent =  (m_conferenceContentRate != initialContRate);//m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapReceive,kRoleContentOrPresentation));
	}
	else
		bNeedToChangeContent =  (initialContRate != currentContRate);

	ALLOCBUFFER(str, LargePrintLen);
	sprintf(str,"need to change content %d, initialContRate %d, currentContRate %d, lastContentRate %d, newContentRate %d", bNeedToChangeContent, initialContRate, currentContRate, m_conferenceContentRate, m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation));
	TRACESTR(eLevelInfoNormal)<< " CIpPartyCntl::IsContentRateNeedToBeChanged: The details: " << str << ",  Name: " << m_partyConfName;
	DEALLOCBUFFER(str);

	return bNeedToChangeContent;
}

//////////////////////////////////////////////////
BYTE CIpPartyCntl::ChangeDataBridgeStateAccordingToNewMode()
{
	BYTE bIsConnectToDataBridge = FALSE;

	//Check Disconnect
	BYTE bIsDisconnectFromDataBridge = DisconnectPartyFromDataBridgeIfNeeded();

	//Check Connect
	if (!bIsDisconnectFromDataBridge)
	{
		bIsConnectToDataBridge = m_pIpInitialMode->IsMediaOn(cmCapData) && !m_isFeccConn;
		if (bIsConnectToDataBridge)
		 	ConnectPartyToFECCBridge(m_pIpInitialMode);
	}

	return (bIsDisconnectFromDataBridge || bIsConnectToDataBridge);
}

//////////////////////////////////////////////////
BYTE CIpPartyCntl::DisconnectPartyFromDataBridgeIfNeeded()
{
	BYTE bDisconnected = FALSE;;
	if (m_pIpInitialMode->IsMediaOff(cmCapData,cmCapTransmit) || m_pIpInitialMode->IsMediaOff(cmCapData,cmCapReceive))
	{
		bDisconnected = TRUE;
		DisconnectPartyFromFECCBridge();
	}

	return bDisconnected;
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::HandleVideoBridgeConnectedInd(CSegment* pParam)
{
	EMediaDirection eVidBridgeConnState = eNoDirection;
	*pParam >> (WORD&)eVidBridgeConnState;
	UpdateVideoBridgeConnectionState(eVidBridgeConnState);

	if (AreTwoDirectionsConnectedToVideoBridge() || IsOutDirectionConnectedToVideoBridge()
		|| (IsMsSlaveIn() && IsInDirectionConnectedToVideoBridge()))
	{
		PTRACE2PARTYID(eLevelInfoNormal," ---> Video Connection Established ",m_partyConfName, GetPartyRsrcId());
		m_pTaskApi->UpdateDB(m_pParty,VIDCON,TRUE);
	}

	if(m_pIpInitialMode->GetConfType() == kVSW_Fixed || m_pIpInitialMode->GetConfType() == kVideoSwitch)
	{
		CapEnum protocol = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo);
		TRACESTR(eLevelInfoNormal) << " CIpPartyCntl::HandleVideoBridgeConnectedInd :  GetRemoteCapsVideoRate - " << GetRemoteCapsVideoRate(protocol);
		DWORD videoBridgeBitRate = m_pIpCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
		if( GetRemoteCapsVideoRate(protocol)< videoBridgeBitRate )
			m_pIpCurrentMode->SetFlowControlRateConstraint(GetRemoteCapsVideoRate(protocol));
	}

	UpdateBridgeFlowControlRateIfNeeded();
	UpdateVidBrdgTelepresenceEPInfoIfNeeded(m_eTelePresenceMode); //_e_m_
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdateBridgeFlowControlRateIfNeeded(CLPRParams* lprParams)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateBridgeFlowControlRateIfNeeded - Error. Should call to derived class's function. ",m_partyConfName, GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdatePartyStateAfterVideoBridgeConnected()
{

	if (IsOutDirectionConnectedToVideoBridge())
	{
		SetPartyStateUpdateDbAndCdrAfterEndConnected();
	}
}

/////////////////////////////////////////////////////////////////////////////
// content brdg disconnection ack
int  CIpPartyCntl::OnContentBrdgDisconnected(CSegment* pParam)
{
	return CPartyCntl::OnContentBrdgDisconnected(pParam);
}

/////////////////////////////////////////////////////////////////////////////
int  CIpPartyCntl::OnContentBrdgConnected(CSegment* pParam)
{
	return CPartyCntl::OnContentBrdgConnected(pParam);
}

//////////////////////////////////////////////////
void CIpPartyCntl::SetPartyStateUpdateDbAndCdrAfterEndConnected(BYTE reason)
{
	BYTE bSecondary = FALSE;
	BYTE bPartially = FALSE;
	BYTE bConnected = FALSE;
	BYTE bAudioOnly = FALSE;
#if 0
	CMedString msg1;
	msg1 << "Name -:" << m_partyConfName
		<< ", reason = " << IsMsSlaveIn()
		<< ", m_eVidBridgeConnState = " << GetBridgeConnectionStateStr(m_eVidBridgeConnState)
		<< ", IsInDirectionConnectedToVideoBridge() = " << IsInDirectionConnectedToVideoBridge()
		<< ", IsMsSlaveIn() " << IsMsSlaveIn()
		<< ", IsMsSlaveOut(): = " << IsMsSlaveOut()
		<< ", IsOutDirectionConnectedToVideoBridge() = " << IsOutDirectionConnectedToVideoBridge();
	PTRACE2(eLevelInfoNormal, "CIpPartyCntl::SetPartyStateUpdateDbAndCdrAfterEndConnected: ", msg1.GetString());
#endif
	//1 - Determinate the state
	if (!AreTwoDirectionsConnectedToAudioBridge() && (!IsMsSlaveIn()) && (!IsMsSlaveOut())) // (no audio) => partialy connected
	{
		bPartially = TRUE;
	}

	else // audio is connected
	{
		if (m_eVidBridgeConnState == eBridgeDisconnected) // no video connection
		{
			if (IsUndefinedParty())
			{
				if (IsRemoteCapNotHaveVideo() || m_voice)// (undefined + A connected + remote don't have V) => audio only connected - or allocation received only Audio
					bAudioOnly = TRUE;
				else						 // (undefined + A connected + remote have V) => secondary
					bSecondary = TRUE;
			}
			else //defined + video not connected
			{
				if (m_voice)// audio only connected
					bAudioOnly = TRUE;
				else // (defined + only audio connected) => secondary
					bSecondary = TRUE;
			}
		}

		else //video is partially connected
		{
			if (IsOutDirectionConnectedToVideoBridge() ||
				(IsMsSlaveIn() && IsInDirectionConnectedToVideoBridge())) // and of course if both video bridge directions are connectred
			{
				bConnected = TRUE;
			}
			else
			{
				bPartially = TRUE;
			}
		}
	}

	//2 - Set the in DB
	if (bSecondary)
	{
	    CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());

		if (pConfParty)
		{
		    DWORD partyState = pConfParty->GetPartyState();
		    if (partyState != PARTY_SECONDARY)
		    {	//since this function is the only place that changes the state in the operator to secondary,
		        //for ip party, we can ask this question
		        //1) Update party (all the parameters accept for reason are the default ones
		        SetPartySecondaryCause(reason,0,cmCapTransmit,NULL);

		        //2) Conf level (Monitoring + SetCapCommonDenominator) + update m_pIpCurrentMode
		        m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_SECONDARY);
		    }
		}
		else
		{
		    PTRACE(eLevelError,"CIpPartyCntl::SetPartyStateUpdateDbAndCdrAfterEndConnected - pConfParty is NULL!!");
		    DBGPASSERT(1116);
		}
	}

	else if (bPartially)
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::SetPartyStateUpdateDbAndCdrAfterEndConnected PARTY_CONNECTED_PARTIALY");
		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED_PARTIALY);
	}

	else if (bAudioOnly)
		UpdatePartyAsAudioOnly();

	else if (bConnected)
	{
	#ifdef PERFORMANCE_TEST
		m_Stopper.AddTime();
		static PerformanceStatistics performanceStatistics;
		performanceStatistics.insert(std::make_pair(GetPartyRsrcId(), m_Stopper));
		if (performanceStatistics.size() == 10)
		{
			std::ostringstream msg;

			for (PerformanceStatistics::iterator _ii = performanceStatistics.begin(); _ii != performanceStatistics.end(); ++_ii)
			{
				msg << "\nPartyId:" << _ii->first << " ";
				_ii->second.Dump(msg);
			}
			TRACEINTOLVLERR << "PERFORMANCE_TEST\n" << msg.str().c_str();
			performanceStatistics.clear();
		}
	#endif //PERFORMANCE_TEST

		TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", State:PARTY_CONNECTED";

		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED);
		RemoveSecondaryCause(TRUE);// check if to keep content secondary cause.
		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_RESET_STATUS);
	}

	//3 - Set the state in CDR
	if (bAudioOnly || bConnected)//we do not update partially
		UpdatePartyStateInCdr();

	/*to fix VNGR-26326*/
	/*after party connected, we don't need to redial for hotbackup anymore*/
	m_bIsFirstConnectionAfterHotBackupRestore = FALSE;
	m_numOfHotbackupRedial = 0;

}

////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::IsInitialModeAndCurrentModeNotContaining()
{
	BYTE bIsContaining  = NO;
	DWORD details = 0;
	bIsContaining = m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFormat|kFrameRate|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapReceive);
	bIsContaining &= m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFormat|kFrameRate|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapTransmit);
	bIsContaining &= m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFrameRate, &details,cmCapAudio,cmCapReceive);
	bIsContaining &= m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode|kFrameRate, &details,cmCapAudio,cmCapTransmit);
	return !bIsContaining;
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::OnPartyRefreshVideoAnycase(CSegment* pParam)
{
	if (m_eVidBridgeConnState != eBridgeDisconnected && (m_eVidBridgeConnState & eOutConnected) )
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::OnPartyRefreshVideoAnycase : Send to bridge. Name - ",m_partyConfName, GetPartyRsrcId());
		m_pVideoBridgeInterface->VideoRefresh(GetPartyRsrcId());
	}
	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::OnPartyRefreshVideoAnycase : Bridge is not connected. Name - ",m_partyConfName, GetPartyRsrcId());
}
/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::OnPartyEventModeVideoPreviewRefreshVideoAnycase(CSegment* pParam)
{
	if (m_eVidBridgeConnState != eBridgeDisconnected && (m_eVidBridgeConnState & eOutConnected) )
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::OnPartyEventModeVideoPreviewRefreshVideoAnycase : Send to bridge. Name - ",m_partyConfName, GetPartyRsrcId());
		m_pVideoBridgeInterface->EventModeIntraPreviewReq(GetPartyRsrcId());
	}
	else
		PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::OnPartyEventModeVideoPreviewRefreshVideoAnycase : Bridge is not connected. Name - ",m_partyConfName, GetPartyRsrcId());
}

//////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::OnCAMUpdatePartyInConf(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::OnCAMUpdatePartyInConf : Name - ",m_partyConfName, GetPartyRsrcId());
	ON(m_isPartyInConf);

	BYTE bRes = FALSE;
	bRes = CheckIfNeedToSendIntra();

	if(bRes)
		SendIntraToParty();
}

//////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetRedailCounterToZero()
{
	m_redial = 0;
}

//////////////////////////////////////////////////////////////////////////////
eVideoPartyType CIpPartyCntl::GetLocalVideoPartyType(BYTE partyTypeWithH263)
{
	eVideoPartyType videoPartyType = eVideo_party_type_none;

	if (!m_voice)
	{
	    if (m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType() == eMixAvcSvcVsw)
		{
			if (m_bIsMrcCall)
				videoPartyType = GetRelayResourceLevel(false, m_pIpInitialMode);
			else
				videoPartyType = GetRelayResourceLevel(true, m_pIpInitialMode);  // eVSW_relay_party_type;

			TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType];
			return videoPartyType;
		}

		if (m_pIpInitialMode->GetConfType() == kCp)
		{
			videoPartyType = GetCPVideoPartyTypeAccordingToCapabilities();
			eVideoPartyType rtvVideoPartyType = GetVideoPartyTypeForRtvBframe();
			videoPartyType = max(videoPartyType, rtvVideoPartyType);

			TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << " - According to local cap";

			// /ASYMMETRIC MODES
			if (IsCapableOf4CIF() && partyTypeWithH263 && (videoPartyType < eCP_H264_upto_SD30_video_party_type))
			{
				videoPartyType = eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type;
				TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << " - Change to 4Cif";
			}

			CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
			PASSERT_AND_RETURN_VALUE(!pConfParty, videoPartyType);

			BYTE protocol = pConfParty->GetVideoProtocol();
			eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
			if (protocol == VIDEO_PROTOCOL_H261 && videoPartyType != eVideo_party_type_none && systemCardsBasedMode == eSystemCardsMode_mpmrx)
			{
				videoPartyType = eCP_H261_CIF_equals_H264_HD1080_video_party_type;
				TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << " - Force H261 resolution on mpmRx";
			}

			BYTE isHD1080Enabled = m_pIpInitialMode->IsHd1080Enabled();

			if (isHD1080Enabled && videoPartyType == eCP_H264_upto_HD720_30FS_Symmetric_video_party_type)
			{
				//videoPartyType = eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
				//PTRACEPARTYID(eLevelInfoNormal, "CIpPartyCntl::GetLocalVideoPartyType : HD1080 enabled but we chose HD720", GetPartyRsrcId());
			}
			else
			{
				BYTE isHD720At60Enabled = m_pIpInitialMode->IsHd720At60Enabled();

				if (isHD720At60Enabled && (videoPartyType == eCP_H264_upto_HD720_30FS_Symmetric_video_party_type))
				{
					videoPartyType = eCP_H264_upto_HD720_60FS_Symmetric_video_party_type;
					TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << " - Breeze mode symetric HD720 60fps";
				}
				else // 1080_60
				{
					BYTE isHD1080At60Enabled = m_pIpInitialMode->IsHd1080At60Enabled();
					if (isHD1080At60Enabled && (videoPartyType == eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type || videoPartyType == eCP_H264_upto_HD720_60FS_Symmetric_video_party_type))
					{
						videoPartyType = IsFeatureSupportedBySystem(eFeatureHD1080p60Asymmetric) ? eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type : eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type;
						TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << " - HD1080p60 is enabled, change party to 1080p60 from 1080p30/720p60";
					}
				}
			}
		}
		else if (m_pIpInitialMode->GetConfType() == kCop)
		{
			videoPartyType = eCOP_party_type;
		}
		else // VSW
		{
			if (IsCapableOfVideo())
			{
				videoPartyType = eVSW_video_party_type;
			}
		}
	}

	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType];
	return videoPartyType;
}

//////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CIpPartyCntl::GetMaxCurrentCallVideoPartyTypeForAllocation(CIpComMode* pScm,BYTE isSip)
{
	eVideoPartyType eRemoteVideoPartyType;

	eConfMediaType confMediaType = m_pIpInitialMode->GetConfMediaType();
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	PASSERTMSG_AND_RETURN_VALUE(!pCommConf, "!pCommConf return Dummy - NO need to Continue", eVideo_party_type_dummy);

	TRACEINTO << "pCommConf->GetConfMediaType(): " << ConfMediaTypeToString(pCommConf->GetConfMediaType())
		<< ", confMediaType: " << ConfMediaTypeToString(confMediaType);



	if (pScm->GetConfType() == kCop)
	{
		eRemoteVideoPartyType = eCOP_party_type;
	}
	else if (m_bIsMrcCall)
	{
		if ( eVOICE_session == m_pConf->GetSessionTypeForResourceAllocator() )
			eRemoteVideoPartyType = eVoice_relay_party_type;
		else
			eRemoteVideoPartyType = GetRelayResourceLevel(false, pScm);
	}
	else if (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvcVsw)
	{
        if ( eVOICE_session == m_pConf->GetSessionTypeForResourceAllocator() )
            eRemoteVideoPartyType = eVoice_relay_party_type;
        else
            eRemoteVideoPartyType = GetRelayResourceLevel(true, pScm);//eVSW_relay_party_type;
	}
	else
	{
		// this is going to be a long story for short lines.
		// 1. CM allocate resources (bandwidth) according to the FS it receive in the open port command
		// 2. DSP receive MBPS value for the amount of process it need to do in each port it open.
		// 3. In case of HD that is created with static MB, the DSP calculation power is of SD, so it can do more ports, but the CD handle it
		// 		as HD bandwidth so it do lower amount of calls. In order to avoid the mismatch we calculate the total of MB =  static MB + MBPS.

		DWORD staticMB = DEFAULT_STATIC_MB;
		GetStaticMbForDsp(pScm, staticMB);

		BOOL IsRsrcByFs = IsSetNewFRTresholdForParty();

		if ((pCommConf->GetConfMediaType() == eMixAvcSvc) &&
			(pCommConf->GetEnableHighVideoResInAvcToSvcMixMode() || pCommConf->GetEnableHighVideoResInSvcToAvcMixMode()))
		{
			const VideoOperationPoint* pVideoOperationPoint = m_pIpInitialMode->GetHighestOperationPoint(m_pIpInitialMode->GetPartyId());

			if (pVideoOperationPoint && (pVideoOperationPoint->m_frameHeight == 720) && (pVideoOperationPoint->m_frameRate == 3840)) //BRIDGE-15286 -  fix resource allocation for 720@15
				IsRsrcByFs = true;
		}

		eVideoPartyType eRemoteVideoPartyTypeTransmit = pScm->GetVideoPartyType(cmCapTransmit, staticMB,IsRsrcByFs);
		eVideoPartyType eRemoteVideoPartyTypeReceive  = pScm->GetVideoPartyType(cmCapReceive);

		eRemoteVideoPartyType = max (eRemoteVideoPartyTypeTransmit, eRemoteVideoPartyTypeReceive);

		//BRIDGE-12596
		eRemoteVideoPartyType 	 = GetMaxVideoPartyTypeByVideoQuality(eRemoteVideoPartyType, pCommConf->GetVideoQuality());

		if((eRemoteVideoPartyTypeTransmit == eVideo_party_type_none) && (eRemoteVideoPartyTypeReceive == eVideo_party_type_none) && (IsUndefinedParty() == FALSE)
			&& (CProcessBase::GetProcess()->GetProductType() != eProductTypeNinja))
		{
			PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetMaxCurrentCallVideoPartyTypeForAllocation -maybe change to cif on type none");
			// in case of secondary - we leave video resources, but the basic ones.
			BYTE AllowChangeToCif = TRUE;
			BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_FREE_VIDEO_RESOURCES);
			//if(pScm->GetConfType() == kCp && isSip && bEnableFreeVideoResources) //DPA
			//	AllowChangeToCif = FALSE;
			if (AreLocalCapsSupportMedia(cmCapVideo) && AllowChangeToCif)
				eRemoteVideoPartyType = eCP_H264_upto_CIF_video_party_type;
			TRACEINTO << "eRemoteVideoPartyType = " << eVideoPartyTypeNames[eRemoteVideoPartyType];
		}
		if (eRemoteVideoPartyType == eVideo_party_type_none && confMediaType == eMixAvcSvc && m_eLastAllocatedVideoPartyType != eVideo_party_type_none)
			if (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
				eRemoteVideoPartyType = eCP_H264_upto_CIF_video_party_type;
		//RTV Bframes
		if (m_isBframeRscEnabled)
		{
			bool isAvmcu = strstr(m_productId, "AV-MCU");
			CapEnum algorithm = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapTransmit));
			if ((algorithm == eRtvCapCode) && IsRtvBframeEnabled(isAvmcu))
			{
				eRemoteVideoPartyType = GetVideoPartyTypeAllocationForRtvBframe(eRemoteVideoPartyType);
			}
		}
	}

	PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::GetMaxCurrentCallVideoPartyTypeForAllocation - eRemoteVideoPartyType ", eVideoPartyTypeNames[eRemoteVideoPartyType], GetPartyRsrcId());

	return eRemoteVideoPartyType;
}
//////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::IsNeedToChangeVideoResourceAllocation(eVideoPartyType eCurrentVideoPartyType)
{
	CLargeString str;

	TRACEINTO
		<< "Name: " << m_partyConfName
		<< "\n\t eCurrentVideoPartyType:" << (WORD)eCurrentVideoPartyType
		<< ", m_eLastAllocatedVideoPartyType=" << m_eLastAllocatedVideoPartyType
		<< ", m_pIpInitialMode->GetConfType()=" << m_pIpInitialMode->GetConfType();

	if (strstr(GetName(), "FORCE_108060_AS"))
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::IsNeedToChangeVideoResourceAllocation  FORCE_108060_AS- retrun false");
		return false;
	}

	if (m_voice)
		return FALSE;
	else if (m_pIpInitialMode->GetConfType() == kCop)
		return FALSE;

	if (m_bIsMrcCall && CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw)
		return false; // realloc for SVC not supported
	else if (m_pIpInitialMode->GetConfType() != kCp)
	{// if its VSW and the remote types audio only (current mode) and its undefined we will free video resources
		if( (eCurrentVideoPartyType == eVideo_party_type_none) && (IsUndefinedParty()) && IsRemoteCapNotHaveVideo() )
			return TRUE;
		else
			return FALSE;
	}

	//CP video cases:
	BYTE bResourcesMatchAllocation = TRUE;

	eVideoPartyType eLocalVideoPartyType = m_eLastAllocatedVideoPartyType;

	if (m_eLastAllocatedVideoPartyType == eVideo_party_type_dummy) //not allocated  - take from local caps
	{
		eLocalVideoPartyType = GetLocalVideoPartyType();
	}


// 1080_60
	if (eLocalVideoPartyType == eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type)
		bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type);
	else if (eLocalVideoPartyType == eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type)
		bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type);
	else if (eLocalVideoPartyType == eCP_H264_upto_HD720_30FS_Symmetric_video_party_type)
		bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H264_upto_HD720_30FS_Symmetric_video_party_type);
	else if (eLocalVideoPartyType == eCP_H264_upto_SD30_video_party_type)
		bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H264_upto_SD30_video_party_type);
	else if (eLocalVideoPartyType == eCP_H264_upto_CIF_video_party_type)
		bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H264_upto_CIF_video_party_type);
	else if (eLocalVideoPartyType == eCP_H261_H263_upto_CIF_video_party_type)
		bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H261_H263_upto_CIF_video_party_type);
	else if (eLocalVideoPartyType == eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
			bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type);
	else if (eLocalVideoPartyType == eCP_H261_CIF_equals_H264_HD1080_video_party_type)
			bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H261_CIF_equals_H264_HD1080_video_party_type);
	else if (eLocalVideoPartyType == eCP_H264_upto_HD720_60FS_Symmetric_video_party_type)
		bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H264_upto_HD720_60FS_Symmetric_video_party_type);
	else if (eLocalVideoPartyType == eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type)
		bResourcesMatchAllocation = (eCurrentVideoPartyType == eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type);
	else if (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw || m_bIsMrcCall)
    {// for SVC call or VSW call, check if the party type changed
        bResourcesMatchAllocation = (eCurrentVideoPartyType == eLocalVideoPartyType);
        TRACEINTO << "bResourcesMatchAllocation=" << (int)bResourcesMatchAllocation;
    }

	if (!bResourcesMatchAllocation)

		TRACEINTO
			<< "eLocalVideoPartyType: " << (WORD)eLocalVideoPartyType
			<< ", eCurrentVideoPartyType: " << (WORD)eCurrentVideoPartyType
			<< " , need to change resources";

	return !bResourcesMatchAllocation;
}
////////////////////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::IsRemoteHasContentXGA()
{ // check if remote supports in TIP content (fix with 512k XGA).
	 WORD remotemaxMBPS = 0;
	 WORD remotemaxFS = 0;
	 WORD remotemaxDPB = 0;
	 WORD remotemaxBRandCPB = 0;
	 WORD remotemaxSAR = 0;
	 WORD remotemaxStaticMB=0;
	 ERoleLabel eRole = kRoleContentOrPresentation;
	 DWORD profile = 0 ;
	 GetRemoteCapsParams(remotemaxMBPS,remotemaxFS,remotemaxDPB,remotemaxBRandCPB,remotemaxSAR,remotemaxStaticMB,eRole,profile);

	// check if EP is supported in XGA (XGA =>  3072FS / 256FACTOR = 12)
	 TRACEINTO << "remotemaxFS= " <<remotemaxFS ;
	return (remotemaxFS >= H264_XGA_FS_AS_DEVISION) ? true : false ;
}
////////////////////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::IsPartyLegacyForTipContent (DWORD partyContentRate)
{
	DWORD tipContentRate = CUnifiedComMode::TranslateAMCRateIPRate(AMC_512k);

	BOOL bIsRemoteHasContentXGA = IsRemoteHasContentXGA() ;
	TRACEINTO << "partyContentRate= " << partyContentRate << " IsRemoteHasContentXGA= " << (DWORD)bIsRemoteHasContentXGA  ;

	if(partyContentRate < tipContentRate  || !bIsRemoteHasContentXGA )
	{
		TRACEINTO << "Remote doesn't meet TIP content treshold. Setting to content legacy";
		if(!bIsRemoteHasContentXGA)
		{
			return eContentSecondaryCauseBelowResolution ;
		}
		return eContentSecondaryCauseBelowRate;

	}
	return NO ;
}

////////////////////////////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::GetPartyRate()
{
	DWORD confRate = GetConfRate();

	DWORD partyReservationRate = 0xFFFFFFFF;
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
	if (pConfParty)
	{
		partyReservationRate =  pConfParty->GetVideoRate();
		if (partyReservationRate != 0xFFFFFFFF)
			partyReservationRate *= 1000;
	}
	else
		PASSERTMSG(GetPartyRsrcId(),"CIpPartyCntl::GetPartyRate - pConfParty not found");

    DWORD isRegard = GetSystemCfgFlagInt<DWORD>(CFG_KEY_CP_REGARD_TO_INCOMING_SETUP_RATE);
    DWORD setupRate = 0;
    if (isRegard)
        setupRate	= GetSetupRate();
    else
        setupRate	= confRate;

	if (setupRate > confRate)
		setupRate = 0; // we do not regard to incoming setup rate which is higher than conf rate.

	DWORD partyRate = 0;

	if ( (partyReservationRate == 0xFFFFFFFF) && (setupRate == 0) )
		partyRate = confRate;
	else if ( (partyReservationRate != 0xFFFFFFFF) && (setupRate != 0) )
		partyRate = min (partyReservationRate, setupRate);
	else
		partyRate = (setupRate ? setupRate : partyReservationRate);

	TRACEINTO << "confRate= " <<confRate << " partyReservationRate= " <<partyReservationRate << " setupRate= " <<setupRate << " partyRate= " <<partyRate;

	return partyRate;
}

//////////////////////////////////////////////////////////////////////////////
H264VideoModeDetails CIpPartyCntl::GetH264ModeAccordingToVideoPartyType(eVideoPartyType videoPartyType)
{
	DWORD partyRate = GetPartyRate();

	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
	Eh264VideoModeType videoModeType = ::TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	BYTE partyResolution =  eAuto_Res;

	if (pConfParty)
	    partyResolution =  pConfParty->GetMaxResolution();
	else
	{
       PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetH264ModeAccordingToVideoPartyType - pConfParty is NULL - set eAuto_Res");
       DBGPASSERT(1117);
	}

	BYTE maxConfResolution = m_pConf->GetCommConf()->GetConfMaxResolution();
	H264VideoModeDetails h264VidModeDetails;
	h264VidModeDetails.videoModeType =videoModeType;
	CH264VideoMode* pH264VidMode = new CH264VideoMode();

	if (partyResolution == eAuto_Res && maxConfResolution == eAuto_Res)
		pH264VidMode->GetH264VideoParams(h264VidModeDetails, partyRate, vidQuality, (Eh264VideoModeType)videoModeType);
	else if (partyResolution != eAuto_Res)
	{
	  Eh264VideoModeType h264VidMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)partyResolution,vidQuality);
	  h264VidMode = min(h264VidMode,videoModeType);
		pH264VidMode->GetH264VideoModeDetailsAccordingToType(h264VidModeDetails, h264VidMode);
	}
	else
	{
	  Eh264VideoModeType h264VidMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxConfResolution,vidQuality);
	  h264VidMode = min(h264VidMode,videoModeType);
		pH264VidMode->GetH264VideoModeDetailsAccordingToType(h264VidModeDetails, h264VidMode);
	}

	POBJDELETE(pH264VidMode);
	h264VidModeDetails.profileValue = H264_Profile_None;

	return h264VidModeDetails;
}
///////////////////////////////////////////////////////////////////////////////
MsSvcVideoModeDetails CIpPartyCntl::GetMsSvcModeAccordingToVideoPartyType(eVideoPartyType videoPartyType)
{
	DWORD partyRate = GetPartyRate();

	eVideoQuality vidQuality = eVideoQualitySharpness;
	Eh264VideoModeType videoModeType = ::TranslateCPVideoPartyTypeToMaxH264VideoModeType(videoPartyType);
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	BYTE partyResolution =  eAuto_Res;
	if (pConfParty)
	    partyResolution =  pConfParty->GetMaxResolution();
	else
	{
       PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetH264ModeAccordingToVideoPartyType - pConfParty is NULL - set eAuto_Res");
       DBGPASSERT(1117);
	}
	BYTE maxConfResolution = m_pConf->GetCommConf()->GetConfMaxResolution();
	MsSvcVideoModeDetails MsSvcVidModeDetails;
	MsSvcVidModeDetails.videoModeType =videoModeType;
	CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();

	if(partyResolution == eAuto_Res && maxConfResolution == eAuto_Res)
		MsSvcVidMode->GetMsSvcVideoParamsByRate(MsSvcVidModeDetails, partyRate, (Eh264VideoModeType)videoModeType,E_VIDEO_RES_ASPECT_RATIO_DUMMY);
	else if(partyResolution != eAuto_Res)
	{
	  Eh264VideoModeType h264VidMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)partyResolution,vidQuality);
	  h264VidMode = min(h264VidMode,videoModeType);
	  MsSvcVidMode->GetMsSvcVideoParamsByRate(MsSvcVidModeDetails,partyRate, h264VidMode);
	}
	else
	{
	  Eh264VideoModeType h264VidMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxConfResolution,vidQuality);
	  h264VidMode = min(h264VidMode,videoModeType);
	  MsSvcVidMode->GetMsSvcVideoParamsByRate(MsSvcVidModeDetails,partyRate ,h264VidMode);

	}

	POBJDELETE(MsSvcVidMode);


	return MsSvcVidModeDetails;
}

/////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::CheckResourceAllocationTowardsRequest(BYTE isSip)//Check towards requested allocation and update if needed:
{
	eVideoPartyType requestedVideoPartyType = m_eLastAllocRequestVideoPartyType;

	eVideoPartyType allocatedVideoPartyType = m_pPartyAllocatedRsrc->GetVideoPartyType();
	if (requestedVideoPartyType != allocatedVideoPartyType)
	{
		if ((allocatedVideoPartyType == eCOP_party_type) || (requestedVideoPartyType == eCOP_party_type))
		{
			DBGPASSERT(1);
		}
		else if(allocatedVideoPartyType == eVideo_party_type_none)
		{// if its undefined call make it audio only one (remove all none audio media from SCM, set audio only caps, set m_voice member, set ConfParty DB).
			// the only case we can get allocated different then request is in audio only udefined, therefore there is no need for the undefined check.
			BOOL bEnableNoVideoResourcesMessage = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_NO_VIDEO_RESOURCES);
			if(bEnableNoVideoResourcesMessage)
			   m_bNoVideRsrcForVideoParty = TRUE;
			PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckResourceAllocationTowardsRequest : No video Resources for Video party - Name = ",m_partyConfName, GetPartyRsrcId());
			SetPartyToAudioOnly();
		}
		else
		{
			CapEnum currentProtocol = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit));
			if (currentProtocol == eH264CapCode)
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckResourceAllocationTowardsRequest : H264 Protocol - Different - Name = ",m_partyConfName, GetPartyRsrcId());
				H264VideoModeDetails h264VidModeDetails = GetH264ModeAccordingToVideoPartyType(allocatedVideoPartyType);
				//if (isSip)
				//	m_pIpInitialMode->SetH264VideoParams(h264VidModeDetails);
				//else

				APIU8 profile = m_pIpInitialMode->GetH264Profile(cmCapReceive);
				m_pIpInitialMode->SetH264VideoParams(h264VidModeDetails,H264_ALL_LEVEL_DEFAULT_SAR, cmCapReceiveAndTransmit);
				h264VidModeDetails.profileValue = profile;
				m_pIpInitialMode->SetH264Profile(profile,cmCapReceiveAndTransmit);

				// 1080_60
				//If we got resources just for HD1080p30 we wont create a HD1080p60 asymmetric call
				if(requestedVideoPartyType == eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type)
				{
					m_pIpInitialMode->SetHd1080At60Enabled(FALSE);
					PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckResourceAllocationTowardsRequest : didn't get HD1080 60fps Asymmetric resources",m_partyConfName, GetPartyRsrcId());
				}

				if (allocatedVideoPartyType <= eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
					Disable4CifInLocalCaps();
		        // BRIDGE-1373 - high profile declared twice instead of high+base profile.
		        // set H264_Profile_None to prevent run over profile
				h264VidModeDetails.profileValue = H264_Profile_None;
				UpdateH264ModeInLocalCaps(h264VidModeDetails);
			}
			if (currentProtocol == eH263CapCode)
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckResourceAllocationTowardsRequest : H263 Protocol - Different - Name = ",m_partyConfName, GetPartyRsrcId());
				if (allocatedVideoPartyType <= eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
				{
                    Disable4CifInLocalCaps();

				}
			}
			if(allocatedVideoPartyType <= eCP_H264_upto_CIF_video_party_type)
			{
				RemoveH263H261FromLocalCaps();
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckResourceAllocationTowardsRequest : remove H263 on mpmx mode and H264 cif party - Name = ",m_partyConfName, GetPartyRsrcId());

			}

			eVideoPartyType rtvVideoPartyType = GetVideoPartyTypeForRtvBframe();
			if (rtvVideoPartyType > allocatedVideoPartyType)
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CheckResourceAllocationTowardsRequest : Not enough resources, disabling RTV Bframe - Name = ",m_partyConfName, GetPartyRsrcId());
				m_isBframeRscEnabled = FALSE;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::HandleReallocateResponse(CSegment* pParam)
{
	CPartyRsrcDesc* pTempPartyAllocatedRsrc = new CPartyRsrcDesc;
	pTempPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	DWORD status = pTempPartyAllocatedRsrc->GetStatus();
	pTempPartyAllocatedRsrc->DumpToTrace();
	BYTE bAllocationFailed = FALSE;

	eNetworkPartyType networkPartyType =  pTempPartyAllocatedRsrc->GetNetworkPartyType();

	if ((status != STATUS_OK)||(networkPartyType!=eIP_network_party_type))
	{
		if(networkPartyType!=eIP_network_party_type)
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::HandleReallocateResponse eNetworkPartyType!= eIP_network_party_type, eNetworkPartyType = ",eNetworkPartyTypeNames[networkPartyType], GetPartyRsrcId());
			PASSERT(1);
		}
		PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::HandleReallocateResponse : REALLOCATION FAILED!!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
		bAllocationFailed = TRUE;
	}

	else
	{
		eVideoPartyType requestedVideoPartyType = m_eLastReAllocRequestVideoPartyType;
		eVideoPartyType allocatedVideoPartyType = pTempPartyAllocatedRsrc->GetVideoPartyType();

		UpdateResourceTableAfterRealloc(pTempPartyAllocatedRsrc);

		if(requestedVideoPartyType >= allocatedVideoPartyType)
		{
			if((allocatedVideoPartyType == eVideo_party_type_none)&&(!AreLocalCapsSupportMedia(cmCapVideo)))
			{// if its undefined call make it audio only one (remove all none audio media from SCM, set audio only caps, set m_voice member, set ConfParty DB).
				PTRACE(eLevelInfoNormal,"CIpPartyCntl::HandleReallocateResponse1");
				SetPartyToAudioOnly();
                SetPartyStateUpdateDbAndCdrAfterEndConnected();
			}
			m_eLastAllocatedVideoPartyType = allocatedVideoPartyType;
	        UpdateScmWithResources(m_SsrcIdsForAvcParty.m_SsrcIds, allocatedVideoPartyType, pTempPartyAllocatedRsrc->GetIsAvcVswInMixedMode());
		}
		else if (requestedVideoPartyType < allocatedVideoPartyType)
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CIpPartyCntl::HandleReallocateResponse : Higher allocation than requested !!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
			bAllocationFailed = TRUE;
			if(m_pIpInitialMode && m_pIpInitialMode->GetConfType() == kVSW_Fixed && requestedVideoPartyType == eVideo_party_type_none && (allocatedVideoPartyType == eVSW_video_party_type) && IsUndefinedParty() )
			{
				PTRACE(eLevelInfoNormal, "CIpPartyCntl::HandleReallocateResponse -VSW Undefined staying connected altough this is an audio only call");
				bAllocationFailed = FALSE;
				SetPartyToAudioOnly();
				SetPartyStateUpdateDbAndCdrAfterEndConnected();

			}
		}
	}


	POBJDELETE(pTempPartyAllocatedRsrc);

	if (bAllocationFailed)
	{
		m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,RESOURCES_DEFICIENCY,1); // Disconnnect cause
		m_pTaskApi->EndAddParty(m_pParty,statIllegal);
		//m_isFaulty = 1;
	}
	else
		PTRACEPARTYID(eLevelInfoNormal, "CIpPartyCntl::HandleReallocateResponse : REAllocation is OK ", GetPartyRsrcId());

	return bAllocationFailed;
}


//////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::GetStaticMbForDsp(CIpComMode* pScm, DWORD& staticMB)
{
	// Tandberg MXP support static MB which are not supported in MPMx/MPM-Rx
	staticMB = DEFAULT_STATIC_MB;

	return;
}

//////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetMbpsForStaticMbCalculation(CIpComMode* pScm, DWORD& initialMBPS, long& currentScmMbps, long& fs, DWORD& staticMB)
{// set the MBPS values (call initial MBPS and current MBPS) for static MB calculation
	DWORD initialFS    = 0;
	DWORD localCapMbps = 0;
	// get max MBPS
	DWORD partyRate = GetPartyRate();
	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
	H264VideoModeDetails h264VidModeDetails;
	::GetH264VideoParams(h264VidModeDetails, partyRate, vidQuality);

	// get current transmit MBPS
	APIU16 profile;
	APIU8 level;
	long dpb, brAndCpb, sar, tempStaticMB;
	pScm->GetH264Scm(profile, level, currentScmMbps, fs, dpb, brAndCpb, sar, (long&)staticMB, cmCapTransmit);
//	staticMB = (DWORD)tempStaticMB;
	initialFS = h264VidModeDetails.maxFS;
	if(initialFS == INVALID)
	{
		CH264Details thisH264Details = h264VidModeDetails.levelValue;
		initialFS = thisH264Details.GetDefaultFsAsDevision();
	}

	if(initialFS == (DWORD)fs)
	{
		initialMBPS = h264VidModeDetails.maxMBPS;
		if(initialMBPS == INVALID)
		{
			CH264Details thisH264Details = h264VidModeDetails.levelValue;
			initialMBPS = thisH264Details.GetDefaultMbpsAsDevision();
		}
	}
	else
	{// get the local MBPS since the FS is different (we made re-allocation process)
		initialMBPS = GetLocalCapsMbps(kRolePeople);
	}

	CMedString str;
	str <<  "partyRate - " << partyRate <<  ", initialFS - " << initialFS <<  ", fs - " << fs <<  ", initialMBPS - " << initialMBPS
						<<  ", currentScmMbps - " << currentScmMbps <<  ", staticMB - " << staticMB;
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::SetMbpsForStaticMbCalculation : ",str.GetString(), GetPartyRsrcId());
}
//////////////////////////////////////////////////////////////////////////////
eVideoPartyType CIpPartyCntl::SetNewRemoteTransPartyTypeAccordngToVendorIfNeeded(CIpComMode* pScm,eVideoPartyType currentpartytype)
{
	CapEnum algorithm = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapTransmit));
	PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::SetNewRemotePartyTypeAccordngToVendorIfNeeded - party current type is: ",currentpartytype);

	if(  algorithm == eH264CapCode )
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::SetNewRemotePartyTypeAccordngToVendorIfNeeded - H264");
		BYTE isFlagSetForThisUser = ::IsSetStaticMbForUser(m_productId);//the same flag for static mb is used to reduce to SD in breeze mode -because breeze does not support static mb's.
		if(isFlagSetForThisUser)
		{
			APIU16 profile;
			APIU8 level;
			long Mbps,fs,dpb,brAndCpb, sar;
			long staticMB = 0;
			pScm->GetH264Scm(profile, level, Mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
			CMedString str;
			if (Mbps == -1)
			{
				CH264Details thisH264Details = level;
				Mbps = thisH264Details.GetDefaultMbpsAsProduct();
			}
			else
				Mbps *= CUSTOM_MAX_MBPS_FACTOR;
			if (fs == -1)
			{
				CH264Details thisH264Details = level;
				fs = thisH264Details.GetDefaultFsAsProduct();
				if(0 == fs)
					PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::SetNewRemoteTransPartyTypeAccordngToVendorIfNeeded, level value is:", level);
			}
			else
				fs *= CUSTOM_MAX_FS_FACTOR;

			PASSERTMSG(fs == 0, "FrameSize equals to 0 - this is unexpected");

			int fps;
			if(fs)
				fps = ( (int)(Mbps/fs) );
			else
				fps = ( (int)(Mbps/1) );

			str << "this is Ep mbps: " <<  Mbps << "this  is Ep fs: "<<fs<< "this is fps: "<< fps;

		    PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::SetNewRemotePartyTypeAccordngToVendorIfNeeded : ",str.GetString(), GetPartyRsrcId());
			if (fps <= 10)
			{
				long maxfs = H264_W4CIF_FS;
				PTRACE(eLevelInfoNormal,"CIpPartyCntl::SetNewRemotePartyTypeAccordngToVendorIfNeeded - changing party type because vendor has HD in fps less than 10");
				return ::GetCPH264ResourceVideoPartyType(maxfs,Mbps);
			}
			else
				return currentpartytype;
		}
		else
			return currentpartytype;
	}

	return currentpartytype;
}
////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetResolutionTable(CBridgePartyVideoParams *pMediaParams)
{
	BOOL bEnableAutoResForIP = TRUE;
	BOOL bSendAltResForAllSIPDialOut = FALSE;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "SEND_WIDE_RES_TO_IP";
	sysConfig->GetBOOLDataByKey(key, bEnableAutoResForIP);

	if (bEnableAutoResForIP)
	{
		//If this is sip dial out and the flag "DISABLE_WIDE_RES_TO_SIP_DIAL_OUT "is ON
		if((GetDialType() == DIALOUT) && (GetInterfaceType() == SIP_INTERFACE_TYPE))
		{
			std::string key = "DISABLE_WIDE_RES_TO_SIP_DIAL_OUT";
			sysConfig->GetBOOLDataByKey(key, bSendAltResForAllSIPDialOut);

			if(bSendAltResForAllSIPDialOut)
			{
				PTRACE(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams - send Alternaive table to sip dial out parties");
				pMediaParams->SetVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_ALTERNATIVE);
			}
		}

		if(!bSendAltResForAllSIPDialOut)
		{
			//if enabled than set according to party parameters
			if(IsDisableWideResolution())
			{
				PTRACE(eLevelInfoNormal,"CIpPartyCntl::InitVideoParams - send Alternaive table");
				pMediaParams->SetVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_ALTERNATIVE);
			}
			else
				pMediaParams->SetVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_REGULAR);
		}
	}//if disabled use alternate table.
	else
		pMediaParams->SetVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_ALTERNATIVE);

}

////////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::IsDisableWideResolution()
{
	BYTE res = FALSE;

	PTRACE2(eLevelInfoNormal,"CIpPartyCntl::IsDisableWideResolution product Id ",m_productId);

	CapEnum currentProtocol = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit));

	//If ep product id appear in flag than disable WideRes and use alternative table.
	if(IsForceResolutionForParty(m_productId))
	{
		res  = TRUE;
	}
	else
	{
		if(strstr(m_productId, "HDX"))
		{
			if(IsNeedToChangeResAccordingToRemoteRevision())
				res = HandleSpecialEps();
		}
		else
			if(strstr(m_productId, "VSX 8000"))
				res = HandleSpecialEps();
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::HandleSpecialEps()
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::HandleSpecialHDXEps ");
	BYTE res = 0;
	BOOL isVersionIdFull = FALSE;
	BOOL isPivotVersionFULL = FALSE;

	char* RemoteVersionIdToCheck = new char[100];
	memset(RemoteVersionIdToCheck,'\0',100);

	SetProblematicVersionId(m_productId,&RemoteVersionIdToCheck);

	TRACEINTOFUNC << "PartId: " << this->GetPartyId() << "remote version Id: " << m_VersionId << ", Pivote Version Id: " << RemoteVersionIdToCheck;

	if(m_VersionId[0]!='\0')
		isVersionIdFull = TRUE;

	if(RemoteVersionIdToCheck[0]!='\0')
	isPivotVersionFULL = TRUE;

	if(isVersionIdFull && isPivotVersionFULL)
		res = CompareTwoVersionId(RemoteVersionIdToCheck, m_VersionId);

	//return 1 - meaning need to send alternitive resolution table from the encoder.
	//return 0 - meaning need to send regular resolution table from the encoder

	if(res)
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::HandleSpecialHDXEps - Need to send alternative resolution");

	else
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::HandleSpecialHDXEps - Need to send regular resolution");

	PDELETEA(RemoteVersionIdToCheck);

	return res;
}

void CIpPartyCntl::updateResolutionAccordingToNewVideoRate( CBridgePartyVideoOutParams *pMediaOutParams, CIpComMode* pScm )
{
	DWORD videoBridgeFs = pMediaOutParams->GetFS();
	DWORD origVideoBridgeFs = videoBridgeFs;
	DWORD videoBridgeMbps; // BRIDGE-10972
	DWORD newBitRate = pMediaOutParams->GetVideoBitRate();
    CLargeString string1;
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if(pScm->GetConfType() != kCp || pScm->GetIsTipMode())
		return;

    if (m_bIsMrcCall || pScm->GetConfMediaType()==eMixAvcSvcVsw)
        return;

    BYTE isCacEnabled = FALSE;
	std::string sKey;
	std::string sCacEnable;
	sKey = "CAC_ENABLE";
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetDataByKey(sKey, sCacEnable);
	if( strcmp("NO", sCacEnable.c_str()) != 0 )
	{
		isCacEnabled = TRUE;
	}

	Eh264VideoModeType resourceMaxVideoMode = TranslateCPVideoPartyTypeToMaxH264VideoModeType(m_eLastAllocatedVideoPartyType);
	string1 << "CIpPartyCntl::updateResolutionAccordingToNewVideoRate:   newVideoRate = " <<  newBitRate  << "   Name - " << m_partyConfName;
	PTRACE (eLevelInfoNormal, string1.GetString());
	CapEnum outVideoAlg = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapTransmit));

	DWORD audioRate = pScm->GetMediaBitRate(cmCapAudio, cmCapTransmit)*1000;
	DWORD confBitRate = GetPartyRate(); //(pScm->GetCallRate()) * 1000;
	DWORD confAudioBitRate = ::CalculateAudioRate(confBitRate);
	confAudioBitRate = max( confAudioBitRate, audioRate );
	DWORD partyRate = newBitRate + confAudioBitRate;

	if( (CapEnum)outVideoAlg == eH264CapCode )
	{
		/*if( newBitRate <= rate128K )
            videoBridgeFs = min(videoBridgeFs, (DWORD)H264_CIF_FS_AS_DEVISION);
          else if ( newBitRate <= rate768K )
            videoBridgeFs = min(videoBridgeFs, (DWORD)H264_WCIF60_FS_AS_DEVISION);
          else if ( newBitRate <= rate1920K )
            videoBridgeFs = min(videoBridgeFs, (DWORD)H264_HD720_FS_AS_DEVISION);*/

		CSmallString string2;
		string2 << "CIpPartyCntl::updateResolutionAccordingToNewVideoRate: audioRate: " << audioRate << ", confBitRate: " <<
			confBitRate << ", partyRate: " << partyRate;
		PTRACE (eLevelInfoNormal, string2.GetString());

		// AN: for IBM vngfe-4195
		if (partyRate > confBitRate)
			partyRate = confBitRate;

		eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();

		H264VideoModeDetails h264VidModeDetails;
		// CH264VideoMode* pH264VidModeForAssymetric = new CH264VideoMode();
		BYTE isHighprofie = FALSE;
		eVideoProfile eMediaProfile = pMediaOutParams->GetProfile();
		if (eMediaProfile == eVideoProfileHigh)
			isHighprofie = TRUE;
		if(resourceMaxVideoMode == eHD720Asymmetric || resourceMaxVideoMode == eHD720At60Asymmetric || resourceMaxVideoMode == eHD1080Asymmetric || resourceMaxVideoMode == eHD1080At60Asymmetric)
		{
			::GetH264AssymetricVideoParams(h264VidModeDetails, partyRate, vidQuality,resourceMaxVideoMode,isHighprofie);
		}
		else
			::GetH264VideoParams(h264VidModeDetails, partyRate, vidQuality,resourceMaxVideoMode,isHighprofie);

		videoBridgeFs = h264VidModeDetails.maxFS;

		if( videoBridgeFs == INVALID )
		{
			CH264Details thisH264Details = h264VidModeDetails.levelValue;
			videoBridgeFs = thisH264Details.GetDefaultFsAsDevision();
		}

		// BRIDGE-10972
		videoBridgeMbps = h264VidModeDetails.maxMBPS;
		if( videoBridgeMbps == INVALID )
		{
			CH264Details thisH264Details = h264VidModeDetails.levelValue;
			videoBridgeMbps = thisH264Details.GetDefaultMbpsAsDevision();
		}

		//VNGR-18500: 1080 -> the resolution will drop to HD720 only in case that the bitrate is lower than 1728kb
		if ((H264_HD1080_FS_AS_DEVISION == origVideoBridgeFs) && (H264_HD720_FS_AS_DEVISION == videoBridgeFs) &&
			(newBitRate >= 1728000))
		{
			PTRACE(eLevelInfoNormal, "CIpPartyCntl::updateResolutionAccordingToNewVideoRate: NOT updating 1080 Resolution as bitrate >= 17280 ");
			return;
		}

		//FSN-613: Dynamic Content for SVC/Mix Conf
		bool isEnableHdVsw = false;
		CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
		if (pCommConf && pCommConf->GetConfMediaType() == eMixAvcSvc && pCommConf->GetEnableHighVideoResInAvcToSvcMixMode())
		{
			isEnableHdVsw = true;
		}
		if ((H264_HD1080_FS_AS_DEVISION == origVideoBridgeFs) && (H264_HD720_FS_AS_DEVISION == videoBridgeFs) &&
			isEnableHdVsw && (partyRate >= 1232000))
		{
			PTRACE(eLevelInfoNormal, "CIpPartyCntl::updateResolutionAccordingToNewVideoRate: NOT updating 1080 Resolution as EnableHighVideoResInAvcToSvcMixMode ");
			return;
		}
		//DWORD remotemaxFs = GetMaxFsAccordingtoProfile(pMediaOutParams->GetProfile() );
		WORD remotemaxMBPS = 0;
		WORD remotemaxFS = 0;
		WORD remotemaxDPB = 0;
		WORD remotemaxBRandCPB = 0;
		WORD remotemaxSAR = 0;
		WORD remotemaxStaticMB=0;
		ERoleLabel eRole = kRolePeople;
		DWORD profile = 0;
		if (eMediaProfile == eVideoProfileHigh) 
			profile = H264_Profile_High;
		else if(eMediaProfile == eVideoProfileBaseline)
			profile = H264_Profile_BaseLine;
		GetRemoteCapsParams(remotemaxMBPS,remotemaxFS,remotemaxDPB,remotemaxBRandCPB,remotemaxSAR,remotemaxStaticMB,eRole,profile);

		//PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::updateResolutionAccordingToNewVideoRate - remote fs is,",remotemaxFS);
		//PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::updateResolutionAccordingToNewVideoRate - video bridge fs is,",videoBridgeFs);
		if(remotemaxFS)
			videoBridgeFs = min((WORD)videoBridgeFs, remotemaxFS);
		else
			PTRACE(eLevelInfoNormal,"CIpPartyCntl::updateResolutionAccordingToNewVideoRate - remote fs is zero!!");

		PTRACE2INT(eLevelInfoNormal,"***CIpPartyCntl::updateResolutionAccordingToNewVideoRate - chosen fs is,",videoBridgeFs);
		pMediaOutParams->SetFS(videoBridgeFs);

		// BRIDGE-10972
		if(remotemaxMBPS)
		{
			videoBridgeMbps = min((WORD)videoBridgeMbps, remotemaxMBPS);
		}
		PTRACE2INT(eLevelInfoNormal,"***CIpPartyCntl::updateResolutionAccordingToNewVideoRate - chosen mbps is,",videoBridgeMbps);
		pMediaOutParams->SetMBPS(videoBridgeMbps);

	}
	else if( (CapEnum)outVideoAlg == eH263CapCode )
	{
		APIS16  cif4FrameRate = 0;
		int   videoBridge4CifFrameRate  = 0;

		if( newBitRate <= rate256K )
		{
			cif4FrameRate  = -1;
			videoBridge4CifFrameRate  = TranslateIpMpiToVideoBridgeFrameRate(cif4FrameRate);

			pMediaOutParams->SetVideoFrameRate(eVideoResolution4CIF, eVideoFrameRate(videoBridge4CifFrameRate));
		}
	}
	else if(isCacEnabled && (CapEnum)outVideoAlg == eMsSvcCapCode && pConfParty &&  pConfParty->GetAvMcuLinkType() == eAvMcuLinkNone && GetAvMcuLinkType() == eAvMcuLinkNone)
	{
		APIS32 Width = 0,Height = 0, aspectRatio = 0, maxFrameRate = 0;
		cmCapDirection direction = cmCapTransmit;
		pScm->GetMSSvcSpecificParams(direction, Width, Height, aspectRatio, maxFrameRate);
		MsSvcVideoModeDetails MsSvcVidModeDetails;
		CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
		MsSvcVidMode->GetMsSvcVideoParamsByRate(MsSvcVidModeDetails, partyRate, (Eh264VideoModeType)resourceMaxVideoMode,aspectRatio);

		if(MsSvcVidModeDetails.videoModeType != eInvalidModeType && ( ((APIS32)MsSvcVidModeDetails.maxWidth) < Width || ((APIS32)MsSvcVidModeDetails.maxHeight) <  Height) )
		{
			MsSvcParamsStruct& msSvcParams = pMediaOutParams->MsSvcParams();
			msSvcParams.nHeight =  MsSvcVidModeDetails.maxHeight;
			msSvcParams.nWidth =  MsSvcVidModeDetails.maxWidth;
			TRACEINTO <<"lowering MS SVC width and height as a result of low rate- usually CAC flow - for 8.4 open only for cac"<< "msSvcParams.nHeight: " << (DWORD)msSvcParams.nHeight  <<"msSvcParams.nWidth "<<(DWORD)msSvcParams.nWidth;
		}
	}

}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::IsDisconnectionBecauseOfInternalProblems() const
{
    // for RMX 2000C
    BOOL rval = TRUE;

    switch (m_disconnectionCause)
    {
        case H323_CALL_CLOSED_UNKNOWN_REASON:
        case H323_CALL_CLOSED_REMOTE_STOP_RESPONDING:
        case H323_CALL_CLOSED_MEDIA_DISCONNECTED:
        case SIP_REMOTE_STOP_RESPONDING:
        case SIP_INTERNAL_MCU_PROBLEM:
        case MCU_INTERNAL_PROBLEM:
            rval = TRUE;
            break;
        default:
            rval = FALSE;
    }
	return rval;
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::IsRedialForHotBackup() const
{
    // for RMX 2000C
    BOOL rval = TRUE;

    switch (m_disconnectionCause)
    {
		case H323_CALL_CLOSED_REMOTE_BUSY:
		case H323_CALL_CLOSED_REMOTE_REJECT:
		case SIP_BUSY_HERE:
		case SIP_NOT_FOUND:
		case NO_DISCONNECTION_CAUSE:
		case SIP_GLOBAL_FAILURE_603:
		case SIP_GLOBAL_FAILURE_604:
		case SIP_GLOBAL_FAILURE_606:
            rval = TRUE;
            break;
        default:
            rval = FALSE;
    }
	return rval;
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::IsDisconnectionBecauseOfRemoteBusy() const
{
    // for RMX 2000C
    BOOL rval = TRUE;

    switch (m_disconnectionCause)
    {
		case H323_CALL_CLOSED_REMOTE_BUSY:
		case H323_CALL_CLOSED_REMOTE_REJECT:
		case SIP_BUSY_HERE:
            rval = TRUE;
            break;
        default:
            rval = FALSE;
    }
	return rval;
}
//////////////////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::IsChangeIncomingMode (eChangeModeState changeMdoeState)
{
    BOOL rval = FALSE;

    switch (changeMdoeState)
    {
        case eChangeIncoming:
        case eChangeInAndReopenOut:
        case eReopenIn:
        case eReopenInAndOut:
        case eFlowControlOutAndChangeIn:
        case eFlowControlOutAndReopenIn:
            rval = TRUE;
            break;
        default:
            rval = FALSE;
    }
    return rval;
}
//////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::CopVideoBridgeChangeIn(CSegment* pParam, CIpComMode* pNewScm)
{
	BYTE changeModeParam = 0, changeModeType = 0;

	*pParam >> (BYTE &) changeModeParam;
    *pParam >> (BYTE &) changeModeType;

    if (m_eLastCopChangeModeParam == changeModeParam && m_eLastCopChangeModeType==changeModeType)
    	PTRACEPARTYID(eLevelError,"CIpPartyCntl::CopVideoBridgeChangeIn : Parameters weren't changed", GetPartyRsrcId());

    m_eLastCopChangeModeParam = changeModeParam;
    m_eLastCopChangeModeType = (ECopChangeModeType)changeModeType;

	CSmallString str;
    if(m_eLastCopChangeModeType == eCop_DecoderParams)
    {
        ECopDecoderResolution  decoderResolution = (ECopDecoderResolution)m_eLastCopChangeModeParam;
        CCopLayoutResolutionTable table;
        str << "New mode = " << table.GetCopDecoderResolutionStr(decoderResolution) << ", state = " << m_state;
        PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CopVideoBridgeChangeIn : ", str.GetString(), GetPartyRsrcId());
        DBGPASSERT_AND_RETURN(decoderResolution >= COP_decoder_resolution_Last);
        pNewScm->Dump("CIpPartyCntl::CopVideoBridgeChangeIn- pNewScm Before change:",eLevelInfoNormal);
        pNewScm->SetVideoRxModeAccordingDecoderResolution(decoderResolution, m_eFirstRxVideoCapCode,m_eFirstRxVideoProfile,m_pConf->GetCommConf()->GetCopConfigurationList(),
        		IsPartyCascadeWithCopMcu(), IsPartyMasterOrSlaveNotLecturer());

    }
    else if (m_eLastCopChangeModeType == eCop_EncoderIndex)
    {
        //encoder index
        BYTE encoderIndex = m_eLastCopChangeModeParam;
        str << "New mode (encoder index)= " << encoderIndex << ", state = " << m_state;
        PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::CopVideoBridgeChangeIn : ", str.GetString(), GetPartyRsrcId());
        pNewScm->Dump("CIpPartyCntl::CopVideoBridgeChangeIn- pNewScm Before change:",eLevelInfoNormal);
        CVidModeH323* pVidMode = m_pCopVideoTxModes->GetVideoMode(encoderIndex);
		if (pVidMode && pVidMode->IsMediaOn())
		{
			if (pVidMode->GetType() == eH264CapCode)
			{
				CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
				if (!pConfParty)
				{
				    PTRACE(eLevelError,"CIpPartyCntl::CopVideoBridgeChangeIn - pConfParty is NULL");
				    DBGPASSERT(1118);
				    return;
				}
				CCOPConfigurationList* pCOPConfigurationList = m_pConf->GetCommConf()->GetCopConfigurationList();
				CCopVideoParams* pCopLevelParams = pCOPConfigurationList->GetVideoMode(encoderIndex);
				// Change the scm receive video:
				sCopH264VideoMode copH264VideoMode;
				CCopVideoModeTable* pCopTable = new CCopVideoModeTable;
				APIU16 profile =GetProfileAccordingToCopProtocol(pCopLevelParams->GetProtocol());
				pCopTable->GetSignalingH264ModeAccordingToReservationParams(pCopLevelParams, copH264VideoMode, TRUE, pConfParty->GetVideoRate());
				pNewScm->SetH264Scm(profile, copH264VideoMode.levelValue, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB, copH264VideoMode.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, copH264VideoMode.maxStaticMbps, cmCapReceive);
				POBJDELETE(pCopTable);
				DWORD videoRateToSet = pVidMode->GetBitRate();
				pNewScm->SetVideoBitRate(videoRateToSet, cmCapReceive, kRolePeople);
			}
			else
				pNewScm->SetMediaMode(*pVidMode, cmCapVideo, cmCapReceive, kRolePeople);
			pNewScm->SetDirection(cmCapVideo, cmCapReceive, kRolePeople);
		}
        else
            PASSERTMSG(GetPartyRsrcId(),"CIpPartyCntl::CopVideoBridgeChangeIn -Failed to set receive mode according to tx level");
    }
    else
        PASSERTMSG(GetPartyRsrcId(),"CIpPartyCntl::CopVideoBridgeChangeIn - UNKNOWN COP CHANGE MODE TYPE!");

    pNewScm->Dump("CIpPartyCntl::CopVideoBridgeChangeIn- pNewScm After change:",eLevelInfoNormal);
}

//////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::CopVideoBridgeChangeOut(CSegment* pParam)
{
    BYTE encoderIndex = INVALID_COP_LEVEL;
    *pParam >> (BYTE &) encoderIndex;
    PTRACE2INT (eLevelInfoNormal, "CIpPartyCntl::CopVideoBridgeChangeOut - conf force encoder index = ", encoderIndex);
    m_lastCopForceEncoderLevel = encoderIndex;
}
///////////////////////////////////////////////////////
EUpdateBridgeMediaAndDirection  CIpPartyCntl::UpdateVideoBridgeOutIfNeeded(BYTE bTakeInitial)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoBridgesOutIfNeeded : Name - ",m_partyConfName, GetPartyRsrcId());
	int updateBridges = eNoUpdate;
	if (m_eVidBridgeConnState & eSendOpenOut)
	{
		if(UpdateVideoOutBridgeIfNeeded(bTakeInitial))
			updateBridges += eUpdateVideoOut;
	}
	else
		PTRACEPARTYID(eLevelInfoNormal,"CIpPartyCntl::UpdateVideoBridgesOutIfNeeded : No OUT Connection", GetPartyRsrcId());

	return (EUpdateBridgeMediaAndDirection)updateBridges;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::IsOpenContentTxChannel(BYTE bTakeCurrent) const
{
	if(bTakeCurrent)
		return((CapEnum)m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit, kRoleContentOrPresentation));
	else
		return ((CapEnum)m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit, kRoleContentOrPresentation));
}
////////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::CheckIfNeedToSendIntra()
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::CheckIfNeedToSendIntra -- False");
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SendIntraToParty()
{
	StartTimer(IP_SEND_INTRA_TO_PARTY_TOUT, 1*SECOND);
}
////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::OnSendIntraToPartyTout()
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::SendIntraToParty Send Intra to party");

	CSegment* pParam = NULL;
	DispatchEvent(VIDREFRESH, pParam);
}
//////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::OnTimerPartyChangeModeLoop()
{
	DWORD numOfChangeModes = GetNumOfChangeModesInSec();
	if(numOfChangeModes > NUM_OF_CHANGE_MODES_PER_SECOND)
	{

		PTRACE2INT(eLevelError,"CIpPartyCntl::OnTimerPartyChangeModeLoop - loop of change modes numOfChangeModes, ",numOfChangeModes);
		BYTE 	mipHwConn = (BYTE)eMipNoneHw;
		BYTE	mipMedia = (BYTE)eMipNoneMedia;
		BYTE	mipDirect = 0;
		BYTE	mipTimerStat = 0;
		BYTE	mipAction = 0;
		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL/*,MpiErrorNumber*/);
		POBJDELETE(pSeg);
		return;
	}
	else
		PTRACE2INT(eLevelError,"CIpPartyCntl::OnTimerPartyChangeModeLoop -  numOfChangeModes, ",numOfChangeModes);


	SetNumOfChangeModesInSec( 0 );
}
///////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdateNewRateForCdrIfNeeded()
{
	DWORD connectionrate = GetConnectionRate(m_pIpCurrentMode);
	if(connectionrate != m_lastConnectionRate)
	{
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::UpdateNewRateForCdrIfNeeded - different rate. Name - ",connectionrate);
		m_pIpCurrentMode->Dump("CIpPartyCntl::UpdateNewRateForCdrIfNeeded : current - ",eLevelInfoNormal);
		m_pTaskApi->UpdateNewRateInfoInCdr(m_pParty,connectionrate);
		m_lastConnectionRate = connectionrate;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetInitialRecRateAccordingToRes(ECopDecoderResolution eLastCopChangeModeParam,DWORD videoRate)
{
	//DWORD videoRateToSet = m_pLocalCapH323->GetMaxVideoBitRate(capCode, cmCapReceive, kRolePeople);
	if(eLastCopChangeModeParam == COP_decoder_resolution_CIF25 && videoRate > COP_MAX_RATE_FOR_CIF_RESOLUTION )
	{
		PTRACE2INT (eLevelInfoNormal, "CComModeH323::SetInitialRecRateAccordingToRes -, new decoder COP_decoder_resolution_CIF25:", eLastCopChangeModeParam);
		m_pIpInitialMode->SetVideoBitRate(COP_MAX_RATE_FOR_CIF_RESOLUTION,cmCapReceive);
	}
	if( ( eLastCopChangeModeParam == COP_decoder_resolution_4CIF25 || eLastCopChangeModeParam == COP_decoder_resolution_4CIF50 || eLastCopChangeModeParam == COP_decoder_resolution_W4CIF25 ) && videoRate > COP_MAX_RATE_FOR_SD_RESOLUTION)
	{

		m_pIpInitialMode->SetVideoBitRate(COP_MAX_RATE_FOR_SD_RESOLUTION,cmCapReceive);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::CalculateArtCapacityAccordingToScm(CIpComMode* pScm, BOOL isAddVideoAndAudio)
{
	DWORD artCapacity = 0;

	// For MPM-Rx card
	if (IsFeatureSupportedBySystem(eFeatureWeightedArtMpmRx))
	{
		if( pScm->GetPartyMediaType() == eAvcPartyType )
		{
			DWORD audioOut        = 0;
			DWORD videoPeopleOut  = 0;
			DWORD videoContentOut = 0;

			if (!pScm->IsMediaOff(cmCapAudio, cmCapTransmit))
			{
				if (pScm->GetConfType() == kVSW_Fixed)
				{
					DWORD callRate = m_pIpInitialMode->GetCallRate();
					audioOut = CalculateAudioRate(callRate * 1000) / 1000;
				}
				else
					audioOut = pScm->GetMediaBitRate(cmCapAudio, cmCapTransmit);

				// audio rate of one ART
				if (GetIsTipCall())
					audioOut = 64;
			}

			if (!pScm->IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople))
			{
				videoPeopleOut = (DWORD)((pScm->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople))/10);
			}

			if (!m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
			{
				videoContentOut = (DWORD)((pScm->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation)) /10);
			}

			// When initiating the call, the Video Out rate is equal to the conf rate and it includes the Audio Out rate inside it.
			// Therefore we use isAddVideoAndAudio to determine which case is it.
			if (isAddVideoAndAudio || (pScm->GetConfType() == kVSW_Fixed))
				artCapacity = audioOut + videoPeopleOut + videoContentOut;
			else
				artCapacity = videoPeopleOut + videoContentOut;

			TRACEINTO << "isAddVideoAndAudio:" << (WORD)isAddVideoAndAudio << ", audioOut:" << audioOut << ", videoPeopleOut:" << videoPeopleOut << ", videoContentOut:" << videoContentOut << ", artCapacity (Connection Rate):" << artCapacity;
		}
		else
		{
			TRACEINTO << "not eAvcPartyType, setting artCapacity to zero";
			artCapacity = 0;
		}
	}
	// For MPMx card - if COP or VSW
	// Tsahi - Disable this feature for MPMx VSW since Resources are not handling it right now.
	//         If this will be supported in CP/VSW on MPMx then this should be enabled here.
	else if (::GetIsCOPdongleSysMode() /*|| (pScm->GetConfType() == kVSW_Fixed)*/)
	{
		DWORD inRate = 0;
		DWORD outRate = 0;
		BOOL  isLpr = 0;
		BOOL  isEncryption = 0;
		BOOL  isStereo = 0;
		BOOL  isSirenFamily = 0;
		bool  isVideo = false;

		CalculateArtCapacityAccordingToScmCop(pScm, outRate, inRate, isVideo, isSirenFamily, FALSE /* isAddVideoAndAudio is FALSE for VSW and COP */);

		if (pScm->GetIsLpr() && isVideo)
			isLpr = 1;

		if (pScm->GetIsEncrypted() != Encryp_Off || m_pIpInitialMode->GetIsEncrypted() != Encryp_Off)
			isEncryption = 1;

		if (pScm->isModeIncludeStereo())
			isStereo = 1;

		TRACEINTO << "inRate:" << inRate << ", outRate:" << outRate << ", isLpr:" << (WORD)isLpr << ", isEncryption:" << (WORD)isEncryption << ", isStereo:" << (WORD)isStereo << ", isSirenFamily:" << (WORD)isSirenFamily;
		artCapacity = GetPartyArtWeight(inRate, outRate, isLpr, isEncryption, isStereo, isSirenFamily);
	}

	return artCapacity;
}

void CIpPartyCntl::CalculateArtCapacityAccordingToScmCop(CIpComMode* pScm, DWORD& outRate, DWORD& inRate, bool& isVideo, BOOL& isSirenFamily, BOOL isAddvideoAndAudio)
{
	DWORD audioOut=0;
	DWORD audioIn = 0;
	DWORD videoPeopleOut=0;
	DWORD videoPeopleIn= 0;

	if (!pScm->IsMediaOff(cmCapAudio,cmCapTransmit))
	{
			audioOut = pScm->GetMediaBitRate(cmCapAudio ,cmCapTransmit);
			CapEnum algorithm = (CapEnum)(pScm->GetMediaType(cmCapAudio, cmCapTransmit));
			isSirenFamily = isSirenLPRCap(algorithm);
	}
	if (!pScm->IsMediaOff(cmCapAudio,cmCapReceive))
	{
		audioIn = pScm->GetMediaBitRate(cmCapAudio ,cmCapReceive);
		CapEnum algorithm = (CapEnum)(pScm->GetMediaType(cmCapAudio, cmCapReceive));
		isSirenFamily = isSirenLPRCap(algorithm);
	}
	if (!pScm->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople))
	{
		videoPeopleIn = (DWORD)((pScm->GetMediaBitRate(cmCapVideo ,cmCapReceive,kRolePeople)) / 10);
	}
	if (!pScm->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))
	{
		videoPeopleOut = (DWORD)((pScm->GetMediaBitRate(cmCapVideo ,cmCapTransmit,kRolePeople))/10);
	}
	DWORD callRate = m_pIpInitialMode->GetCallRate();
	DWORD audioAccordingToConfRate = CalculateAudioRate((callRate * 1000));
	audioAccordingToConfRate = (audioAccordingToConfRate /1000);

	//BECAUSE FROM 7.2(4.7.2 IS ALSO REBASED TO 7.2) WE PUT CALL RATE IN VIDEO RATE WE DON'T NEED TO ADD UNLESS THIS IS AUDIO ONLY CALL
	if (videoPeopleIn)
	{
		if(pScm->GetConfType() == kVSW_Fixed)
		{
			inRate = videoPeopleIn + audioAccordingToConfRate;
		}
		else
		{
			if(isAddvideoAndAudio)
				inRate = videoPeopleIn + audioIn;
			else
				inRate = videoPeopleIn;
		}
	}
	else
	{
		inRate = audioIn;
	}

	if (videoPeopleOut)
	{
		if(pScm->GetConfType() == kVSW_Fixed)
		{
			outRate = videoPeopleOut + audioAccordingToConfRate;
		}
		else
		{
			if(isAddvideoAndAudio)
			{
				if(audioOut > audioAccordingToConfRate)
				{
					audioOut = audioAccordingToConfRate;
				}

				outRate = videoPeopleOut + audioOut;

			}
			else
				outRate = videoPeopleOut;
		}
	}
	else
	{
		outRate = audioOut;
	}

	isVideo = videoPeopleIn || videoPeopleOut;

	TRACEINTO << "callRate:" << callRate << ", outRate:" << outRate << ", inRate:" << inRate << ", audioAccordingToConfRate:" << audioAccordingToConfRate << ", isVideo:" << (WORD)isVideo << ", isSirenFamily:" << (WORD)isSirenFamily << ", isAddvideoAndAudio:" << (WORD)isAddvideoAndAudio;
}

///////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::GetScpNotificationRemoteSeqNumber()
{
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////
// receive user product ID and check if Need to change FR. - Relevant only to H323 MXP
// return the FR Trashold
BOOL CIpPartyCntl::IsSetNewFRTresholdForParty()
{

	if(m_productId == NULL || strlen(m_productId) == 0)
		return FALSE;

	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();


	if((GetInterfaceType() == H323_INTERFACE_TYPE) && strstr(m_productId, "Tandberg MXP")&& vidQuality == eVideoQualitySharpness)
	{
		DWORD dwMXP_FrameRate = 0;

		CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		std::string key = "MXP_FORCE_HD_FR";
		sysConfig->GetDWORDDataByKey(key, dwMXP_FrameRate);

		FPTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::IsSetNewFRTresholdForParty", dwMXP_FrameRate);

		if(dwMXP_FrameRate != 0) //Default
		{
			return TRUE;
		}
		else
			return FALSE;


	}
	else
		return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::GetForcedMbpsAndFR(long fs,long mbps,long& ForcedMbps)
{

	DWORD dwMXP_FrameRate = 0;
	long TmpMbps =0;
	DWORD ForcedFR = 0;
	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "MXP_FORCE_HD_FR";
	sysConfig->GetDWORDDataByKey(key, dwMXP_FrameRate);

	FPTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::GetForcedMbpsAndFR - dwMXP_FrameRate:", dwMXP_FrameRate);

	if(dwMXP_FrameRate == 0) // remain the old behavior like in V7.6.1 --> use defaults
	{
		if(vidQuality == eVideoQualitySharpness)
			ForcedFR = 15;
		else
			ForcedFR = 60;
	}
	else
	{
		mbps *= CUSTOM_MAX_MBPS_FACTOR;
		fs *= CUSTOM_MAX_FS_FACTOR;

		DWORD CurrentFR = mbps/fs;

		TRACEINTO << "Mbps:" << mbps << ", Fs:" << fs << ", CurrentFr:" << CurrentFR << ", MaxFr:" << dwMXP_FrameRate;

		if(vidQuality == eVideoQualitySharpness )
		{
			if((dwMXP_FrameRate>=5) && (dwMXP_FrameRate <= 20))//Forced FR should be set from 5 to 20
			{
				PTRACE(eLevelInfoNormal, "CH264VideoCap::GetVideoPartyTypeMBPSandFS - use Foreced FR for Sharpness");

				if(CurrentFR < dwMXP_FrameRate) //If current FR is lower than the Forced FR -> we prefer the Forced FR
				{
					TmpMbps = ((long)fs*dwMXP_FrameRate);

					PTRACE2INT(eLevelInfoNormal, "CH264VideoCap::GetVideoPartyTypeMBPSandFS - Need to calc new MBPS",TmpMbps);

				}
				ForcedFR = dwMXP_FrameRate;
			}
			else
				ForcedFR = 15;
		}
		else //Motion
		{
			PTRACE(eLevelInfoNormal, "CH264VideoCap::GetVideoPartyTypeMBPSandFS - use default FR for Motion");
			ForcedFR = 60; //No exception currently for Motion
		}

	}

	if(TmpMbps)
		ForcedMbps = GetMaxMbpsAsDevision(TmpMbps);
	return ForcedFR;

}
///////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::IsRelayChannelReadyForMedia(cmCapDirection direction)
{
	BOOL retVal = FALSE;
	if (m_isSignalingFirstTransactionCompleted)
	{
		if (direction == cmCapReceive)
		{
			if(m_incomingVideoChannelHandle != INVALID_CHANNEL_HANDLE)
			{
				retVal = TRUE;

/*				   if(m_bIsMrcCall && m_bVideoUpdateCount<VIDEO_UPDATE_COUNT2)
				   {
				    m_bVideoUpdateCount++;
				    TRACEINTO<<"!@# updating m_bVideoUpdateCount to: "<<(int)m_bVideoUpdateCount;
				    }*/

			}
			else
				retVal = FALSE;
		}
		else if (direction == cmCapTransmit)
		{
			if(m_outgoingVideoChannelHandle != INVALID_CHANNEL_HANDLE)
				retVal = TRUE;
			else
				retVal = FALSE;
		}
	}
	TRACEINTOFUNC << "direction = " << direction << " m_incomingVideoChannelHandle:" << m_incomingVideoChannelHandle << " m_outgoingVideoChannelHandle:" << m_outgoingVideoChannelHandle << ", m_isSignalingFirstTransactionCompleted: " << (m_isSignalingFirstTransactionCompleted? "true": "false") << " retVal: " << (retVal? "TRUE": "FALSE") ;
	return retVal;
}

///////////////////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::UpdateScmWithResources(SVC_PARTY_IND_PARAMS_S  &aSvcParams,eVideoPartyType allocatedVideoPartyType, BOOL isAvcVswInMixedMode)
{
    SVC_PARTY_IND_PARAMS_S  svcParams;
    int cnt=0;
    DWORD status = STATUS_OK;

    m_pIpInitialMode->SetPartyId(GetPartyRsrcId());
    CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

    if (m_pIpInitialMode->GetOperationPoints()->m_numberOfOperationPoints == 0)
    {
    	if (m_pConf->GetVideoOperationPointsSet())
    	{
    		m_pIpInitialMode->SetOperationPoints(m_pConf->GetVideoOperationPointsSet());
            TRACEINTOFUNC << "Set the op. points for the initial SCM";
    	}
    	else
    	{
    		TRACEINTO << "mix_mode: no operation points at conference level.";
    	}
    }

    m_operationPointsSet = *(m_pIpInitialMode->GetOperationPoints());
    m_operationPointsSet.Trace("mix_mode: CIpPartyCntl::UpdateScmWithResources - Setting operation points.");

    if (m_bIsMrcCall)
    {
        TRACEINTOFUNC << "mix_mode: ERROR!!! Should not arrive here";
        status=STATUS_FAIL;
        return status;
    }

    if (m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
    {
        // @@@ - Do I need to update payload type in SCM?
        m_SsrcIdsForAvcParty = aSvcParams;

        // for update streams, we need SSRC, payload type, bit rate, frame rate, height, width
        StreamDesc streamDesc;
        std::list <StreamDesc> streamDescList;

        // audio Rx
        streamDesc.InitDefaults();
        streamDesc.m_pipeIdSsrc = aSvcParams.m_ssrcAudio;
        TRACEINTO<<"!@# audio SSRC:"<<streamDesc.m_pipeIdSsrc;
        streamDesc.m_payloadType = m_pIpInitialMode->GetPayloadType(cmCapAudio, cmCapReceive, kRolePeople);
        CBaseCap *pCap = m_pIpInitialMode->GetMediaAsCapClass(cmCapAudio, cmCapReceive, kRolePeople);
        if (!pCap)
        {
            //@@@ - NO AUDIO
            PASSERT(1);

        }
        else
        {
            CBaseAudioCap *pAudioCap = (CBaseAudioCap *)pCap;
            streamDesc.m_bitRate = pAudioCap->GetBitRate();
            streamDesc.m_frameRate = pCap->GetFrameRate(pCap->GetFormat());
            POBJDELETE(pCap);
        }

        streamDesc.m_isLegal = true;
        streamDesc.m_isAvcToSvcVsw = false;

        // add audio stream descriptor
        streamDescList.push_back(streamDesc);
        m_pIpInitialMode->SetStreamsListForMediaMode(streamDescList, cmCapAudio, cmCapReceive, kRolePeople);
        m_pIpInitialMode->SetStreamsListForMediaMode(streamDescList, cmCapAudio, cmCapTransmit, kRolePeople);
        streamDescList.clear();


        // video RX - the stream we want to receive from EP
        // For VSW, we will declare the lowest operation point in H.264 syntax
        // This should be updated both in SIP caps and SCM StreamDesc

        // update SCM stream descriptor


        streamDesc.InitDefaults();
        streamDescList.clear();
        streamDesc.m_pipeIdSsrc = aSvcParams.m_ssrcVideo[0];
        streamDesc.m_payloadType = m_pIpInitialMode->GetPayloadType(cmCapVideo, cmCapReceive, kRolePeople);

        // get lowest operation-point data

        const VideoOperationPoint* videoOperationPoint=m_pIpInitialMode->GetLowestOperationPoint(m_pIpInitialMode->GetPartyId()); //vopList.begin();        VideoOperationPoint videoOperationPoint=*itr;

        if(videoOperationPoint!=NULL)
        {
            streamDesc.m_bitRate = videoOperationPoint->m_maxBitRate;
            streamDesc.m_frameRate = videoOperationPoint->m_frameRate;
            streamDesc.m_height = videoOperationPoint->m_frameHeight;
            streamDesc.m_width = videoOperationPoint->m_frameWidth;
            streamDesc.m_isLegal = true;
            streamDesc.m_isAvcToSvcVsw = false;
        }
        // add video stream descriptor
        streamDescList.push_back(streamDesc);
        m_pIpInitialMode->SetStreamsListForMediaMode(streamDescList, cmCapVideo, cmCapReceive, kRolePeople);
        streamDescList.clear();



        streamDesc.InitDefaults();
        streamDescList.clear();
        streamDesc.m_pipeIdSsrc = (MASK_FOR_VSW_AVC_PIPEID/*0x000fffff*/ & 210001);
        streamDesc.m_payloadType = m_pIpInitialMode->GetPayloadType(cmCapVideo, cmCapTransmit, kRolePeople);

        // get highest operation-point data
        videoOperationPoint=m_pIpInitialMode->GetHighestOperationPoint(m_pIpInitialMode->GetPartyId());
		PASSERT_AND_RETURN_VALUE(NULL == videoOperationPoint,STATUS_FAIL);

        if(videoOperationPoint!=NULL)
        {
			streamDesc.m_bitRate = videoOperationPoint->m_maxBitRate;
			streamDesc.m_frameRate = videoOperationPoint->m_frameRate;
			streamDesc.m_height = videoOperationPoint->m_frameHeight;
			streamDesc.m_width = videoOperationPoint->m_frameWidth;
			streamDesc.m_isLegal = true;
			streamDesc.m_isAvcToSvcVsw = false;
        }
        // add video stream descriptor
        streamDescList.push_back(streamDesc);
        m_pIpInitialMode->SetStreamsListForMediaMode(streamDescList, cmCapVideo, cmCapTransmit, kRolePeople);
        streamDescList.clear();

        TRACEINTO << "m_pIpInitialMode->GetConfType()==kVswRelayAvc svcParams.m_ssrcVideo[0]:"<<svcParams.m_ssrcVideo[0];

        m_SsrcIdsForAvcParty = aSvcParams;
        m_pIpInitialMode->Dump("VSW: CIpPartyCntl::UpdateScmWithResources initial",eLevelInfoNormal);

    }

	if (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc)
	{
		TRACEINTO << "mix_mode: CIpPartyCntl::UpdateScmWithResources bIsActiveAvcToSvc==TRUE";

		CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
		eLogicalResourceTypes logicalType[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS]=
			{eLogical_relay_avc_to_svc_rtp_with_audio_encoder,eLogical_relay_avc_to_svc_rtp};

		for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
		{
			WORD itemNum=1;
			WORD found;
			avcToSvcTranslatorRsrcParams[i]=new CRsrcParams;
			found=m_pPartyAllocatedRsrc->GetRsrcParams(*avcToSvcTranslatorRsrcParams[i],logicalType[i],itemNum);

			if(found)
			{
				cnt++;
			}
		}

		TRACEINTO << "!@# number of Internal Arts should be: " << cnt;

		if (allocatedVideoPartyType==eVideo_party_type_none)
		{
			if (IsSoftMcu())
			{
				if (cnt!=0)
				{
					TRACEINTO<<"dynMixedErr AudioOnly SoftMcu, cnt should be 0 while in practice cnt="<<cnt;
					status=STATUS_FAIL;
					return status;
				}
			}
			else
			{
				if (cnt!=1)
				{
					TRACEINTO<<"dynMixedErr AudioOnly RMX, cnt should be 1 while in practice cnt="<<cnt;
					status=STATUS_FAIL;
					return status;
				}
			}
		}
		else
		{
			if(cnt==0)
			{
				TRACEINTO<<"dynMixedErr Video party type, cnt should be at least 1 while in practice cnt="<<cnt;
				status=STATUS_FAIL;
				return status;
			}
		}

		for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
		{
			POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
		}

		m_SsrcIdsForAvcParty = aSvcParams;

		// @#@ update SCM with streams

		// for update streams, we need SSRC, payload type, bit rate, frame rate, height, width
		StreamDesc streamDesc;
		std::list <StreamDesc> streamDescList;

		streamDescList.clear();

		CVideoOperationPointsSet* pOperationPointsSet = m_pIpInitialMode->GetOperationPoints();
		const VideoOperationPoint *pVideoOperationPoint;

		// add video stream descriptor
		if (cnt>MAX_NUM_RECV_STREAMS_FOR_MIX_AVC_VIDEO)
		{
			TRACEINTO<<"!@# number of internal arts is too high - truncating:"<<cnt;
			cnt=MAX_NUM_RECV_STREAMS_FOR_MIX_AVC_VIDEO;
		}

		for (int i=0;i<cnt/*MAX_NUM_RECV_STREAMS_FOR_MIX_AVC_VIDEO*/;++i)
		{
			TRACEINTO << "!@# aSvcParams.m_ssrcVideo[" << i << "]=" << aSvcParams.m_ssrcVideo[i];
			pVideoOperationPoint = pOperationPointsSet->GetHighestOperationPointForStream(i);

			if (pVideoOperationPoint)
			{
				streamDesc.InitDefaults();
				streamDesc.m_bitRate = pVideoOperationPoint->m_maxBitRate;
				streamDesc.m_frameRate = pVideoOperationPoint->m_frameRate;
				streamDesc.m_height = pVideoOperationPoint->m_frameHeight;
				streamDesc.m_width = pVideoOperationPoint->m_frameWidth;
				streamDesc.m_pipeIdSsrc = aSvcParams.m_ssrcVideo[i];
				streamDesc.m_payloadType = m_pIpInitialMode->GetPayloadType(cmCapVideo, cmCapReceive, kRolePeople);
				streamDesc.m_isLegal = true;
				streamDesc.m_isAvcToSvcVsw = false;

				if(streamDesc.m_pipeIdSsrc==0)
				{
					TRACEINTO<<"dynMixedErr video SSRC is 0";
					status=STATUS_FAIL;
				}

				streamDescList.push_back(streamDesc);
			}
		}

		// add VSW stream if needed
		bool isEnableHdVsw = false;
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
		if (pCommConf)
		{
			if (pCommConf->GetEnableHighVideoResInAvcToSvcMixMode() || pCommConf->GetEnableHighVideoResInSvcToAvcMixMode())
				isEnableHdVsw = true;
		}

        if (isEnableHdVsw)
        {
            // get the highest operation point and check if it is HD
            TRACEINTO << "mix_mode: Get the highest operation point and check if it is HD";
            pVideoOperationPoint = pOperationPointsSet->GetHighestOperationPoint(this->GetPartyId());

            if (pVideoOperationPoint && (pVideoOperationPoint->m_rsrcLevel >= eResourceLevel_HD720)) //JasonTest
            {
                streamDesc.InitDefaults();
                streamDesc.m_bitRate = pVideoOperationPoint->m_maxBitRate;
                streamDesc.m_frameRate = pVideoOperationPoint->m_frameRate;
                streamDesc.m_height = pVideoOperationPoint->m_frameHeight;
                streamDesc.m_width = pVideoOperationPoint->m_frameWidth;
                streamDesc.m_pipeIdSsrc = aSvcParams.m_ssrcVideo[MAX_NUM_RECV_STREAMS_FOR_VIDEO-1];
                streamDesc.m_payloadType = m_pIpInitialMode->GetPayloadType(cmCapVideo, cmCapReceive, kRolePeople);
                streamDesc.m_isLegal = true;
                streamDesc.m_isAvcToSvcVsw = true;


                if(streamDesc.m_pipeIdSsrc==0)
                {
                    TRACEINTO<<"dynMixedErr video SSRC is 0";
                    status=STATUS_FAIL;
                }

                if (isAvcVswInMixedMode && cnt > 1)
                {
                	streamDescList.push_back(streamDesc);
                }

                // update the H.264 m-line to be as the VSW stream
                SetScmForAvcMixMode(m_pIpInitialMode, GetVoice());

                // update the local caps accordingly too
                UpdateLocalCapsForHdVswInMixMode(pVideoOperationPoint);

            }
        }

		if(cnt>0)
		{
			m_pIpInitialMode->SetStreamsListForMediaMode(streamDescList, cmCapVideo, cmCapReceive, kRolePeople);
		}

		streamDescList.clear();

		// audio Rx
		if((!IsSoftMcu()||(eProductTypeNinja == CProcessBase::GetProcess()->GetProductType())) && cnt>0)
		{
			streamDesc.InitDefaults();
			streamDesc.m_pipeIdSsrc = aSvcParams.m_ssrcAudio;

			if(streamDesc.m_pipeIdSsrc==0)
			{
				TRACEINTO << "dynMixedErr audio SSRC is 0";
				status=STATUS_FAIL;
			}

			streamDesc.m_payloadType = m_pIpInitialMode->GetPayloadType(cmCapAudio, cmCapReceive, kRolePeople);
			CBaseCap *pCap = m_pIpInitialMode->GetMediaAsCapClass(cmCapAudio, cmCapReceive, kRolePeople);
			if (!pCap)
			{
				//@@@ - NO AUDIO
				PASSERT_AND_RETURN_VALUE(pCap==NULL,STATUS_FAIL);
			}
			else
			{
				CBaseAudioCap *pAudioCap = (CBaseAudioCap *)pCap;
				streamDesc.m_bitRate = pAudioCap->GetBitRate();
				streamDesc.m_frameRate = pCap->GetFrameRate(pCap->GetFormat());
			}


			streamDesc.m_isLegal = true;
			streamDesc.m_isAvcToSvcVsw = false;

			// add audio stream descriptor
			streamDescList.push_back(streamDesc);
			m_pIpInitialMode->SetStreamsListForMediaMode(streamDescList, cmCapAudio, cmCapReceive, kRolePeople);

			POBJDELETE(pCap);
		}

		streamDescList.clear();

		m_pIpInitialMode->Dump("***mix_mode: CIpPartyCntl::UpdateScmWithResources initial",eLevelInfoNormal);
	}
	else if (m_pIpInitialMode->GetConfMediaType() == eAvcOnly)
	{
		// update local caps if needed in case the conference is mix mode but AvcOnly de-facto
		bool isEnableHdVsw = false;
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
		if (pCommConf)
		{
			if ((pCommConf->GetConfMediaType() == eMixAvcSvc) &&
					(pCommConf->GetEnableHighVideoResInAvcToSvcMixMode() || pCommConf->GetEnableHighVideoResInSvcToAvcMixMode()))
				isEnableHdVsw = true;
		}

	    if (isEnableHdVsw)
	    {
	        // get the highest operation point and check if it is HD
	        TRACEINTO << "mix_mode: Get the highest operation point and check if it is HD";
			CVideoOperationPointsSet* pOperationPointsSet = m_pIpInitialMode->GetOperationPoints();
			const VideoOperationPoint *pVideoOperationPoint;
	        pVideoOperationPoint = pOperationPointsSet->GetHighestOperationPoint(this->GetPartyId());

	        if (pVideoOperationPoint && pVideoOperationPoint->m_rsrcLevel >= eResourceLevel_HD720)
	        {
	            // update the H.264 m-line to be as the VSW stream
	            SetScmForAvcMixMode(m_pIpInitialMode, GetVoice());

	            // update the local caps accordingly too
	            UpdateLocalCapsForHdVswInMixMode(pVideoOperationPoint);
	        }
	    }
	}
	else
	{
		TRACEINTOFUNC << "m_pIpInitialMode->GetConfType(): " <<
			ConfTypeToString(m_pIpInitialMode->GetConfType()) << ", m_pIpInitialMode->GetConfMediaType(): "
			<< ConfMediaTypeToString(m_pIpInitialMode->GetConfMediaType());
	}

	return status;
}

//////////////////////////////////////////////////
void CIpPartyCntl::SetDataForImportPartyCntl(CPartyCntl *apOtherPartyCntl)
{
    CPartyCntl::SetDataForImportPartyCntl(apOtherPartyCntl);

    CIpPartyCntl *pOtherPartyCntl = (CIpPartyCntl *)apOtherPartyCntl;

    m_videoRate = pOtherPartyCntl->m_videoRate;
    m_originalConfRate          = pOtherPartyCntl->m_originalConfRate;

    POBJDELETE(m_pOriginalIpScm);
//  if(pOtherPartyCntl->m_pOriginalIpScm == NULL){m_pOriginalIpScm = NULL;}
//  else{m_pOriginalIpScm = new CIpComMode(*(pOtherPartyCntl->m_pOriginalIpScm));}

    if(pOtherPartyCntl->m_pIpInitialMode == NULL){m_pIpInitialMode = NULL;}
    else{m_pIpInitialMode = new CComModeH323(*(pOtherPartyCntl->m_pIpInitialMode));}

    if(pOtherPartyCntl->m_pIpCurrentMode == NULL){m_pIpCurrentMode = NULL;}
    else{m_pIpCurrentMode = new CComModeH323(*(pOtherPartyCntl->m_pIpCurrentMode));}

    m_presentationStreamOutIsUpdated =  pOtherPartyCntl->m_presentationStreamOutIsUpdated;

    m_SecondaryCause        = pOtherPartyCntl->m_SecondaryCause;
    if(pOtherPartyCntl->m_pSecondaryParams == NULL)
        {m_pSecondaryParams = NULL;}
    else
    {
        POBJDELETE(m_pSecondaryParams);
        m_pSecondaryParams = new CSecondaryParams;
        *m_pSecondaryParams = *(pOtherPartyCntl->m_pSecondaryParams);
    }

    m_incomingVideoChannelHandle = pOtherPartyCntl->m_incomingVideoChannelHandle;
    m_outgoingVideoChannelHandle = pOtherPartyCntl->m_outgoingVideoChannelHandle;
    m_bVideoUpdateCount=VIDEO_UPDATE_COUNT0;
    m_isSignalingFirstTransactionCompleted = pOtherPartyCntl->m_isSignalingFirstTransactionCompleted;
    m_operationPointsSet        = pOtherPartyCntl->m_operationPointsSet;
    m_SsrcIdsForAvcParty        = pOtherPartyCntl->m_SsrcIdsForAvcParty;
}


//////////////////////////////////////////////////
bool CIpPartyCntl::IsPortConnected(eLogicalResourceTypes lrt)
{
	// check if port is allocated in resources
	// if it is allocated, party has opened channels to it.
	// if there is a failure in open channels, then party is disconnecting

	if (!m_pPartyAllocatedRsrc->GetConnectionId(lrt))
		return false;
	return true;
}

//////////////////////////////////////////////////
// This function is called when an EP needs to be upgraded from
// pure AVC only or SVC only mode to Mix mode
// It sends a request to the resource allocator to receive the updated resources
// needed for the upgrade
void CIpPartyCntl::OnConfChangeModeUpgradeIdle(CSegment* pParam)
{
    if (GetInitialMode()->GetConfMediaType() == eMixAvcSvc)
    {// already in mix mode - no need to upgrade
        TRACEINTO << "mix_mode: Party in mix mode already. Do not upgrade!";
        return;
    }

	DWORD monitorPartyId = (DWORD)(-1);
	DWORD confID = (DWORD)(-1);
	eConfMediaState confMediaState = eMediaStateEmpty;
	DWORD tmp=(DWORD)confMediaState;
	*pParam >> tmp  >> confID  >> monitorPartyId;

	TRACEINTO << "mix_mode: conference type: "<< MediaStateToString((eConfMediaState)tmp ) << " conference id: " << confID << " monitor party id:  "<< monitorPartyId << " m_state:" << m_state;;

	DWORD artCapacity = 0;
	m_state 		= REALLOCATE_RSC;
	eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode);

	if(m_bIsMrcCall)
	{
		TRACEINTO << "mix_mode: !@# dynMixedPosAck svc party: request additional resources from RA";
	}
	else
	{
		TRACEINTO << "mix_mode: !@# dynMixedPosAck avc party: request additional resources from RA";
	}
    CreateAndSendReAllocatePartyResources(eIP_network_party_type, eCurrentVideoType, eAllocateAllRequestedResources,FALSE,0,GetEnableICE(),artCapacity,eTipNone,m_RoomId,AVC_SVC_ADDITIONAL_PARTY_RSRC_REQ);

	// todo:  future use for Eyal and Keren

}

/////////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate(CSegment* pParam, CRsrcParams** avcToSvcTranslatorRsrcParams, CRsrcParams* &pMrmpRsrcParams,int allocationType)
{
	if (allocationType==ALLOCATION_TYPE_UPGRADE)
	{
		if (m_bIsMrcCall)
		{
			TRACEINTO<<"mix_mode: dynMixedPosAck svc party: upgrade reallocate response start";
		}
		else
		{
			TRACEINTO<<"mix_mode: dynMixedPosAck avc party: upgrade reallocate response start";
		}
	}
	else
	{
		if (m_bIsMrcCall)
		{
			TRACEINTO<<"mix_mode: dynMixedPosAck svc party: reallocate response start";
		}
		else
		{
			TRACEINTO<<"mix_mode: dynMixedPosAck avc party: reallocate response start";
		}
	}

	BYTE bAllocationFailed = FALSE;

	// deserialize the updated resources
	CPartyRsrcDesc* pTempPartyAllocatedRsrc = new CPartyRsrcDesc;
	pTempPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	DWORD status = pTempPartyAllocatedRsrc->GetStatus();
	pTempPartyAllocatedRsrc->DumpToTrace();

	if (status != STATUS_OK)
	{
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE,PARTY_CONNECTED_PARTIALY);
		TRACEINTO << "mix_mode: dynMixedErr Reallocation while upgrading to mixed mode failed";
		PASSERT(101);
		POBJDELETE(pTempPartyAllocatedRsrc);
		return status;
	}

	for (int i=0; i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++i)
	{
		avcToSvcTranslatorRsrcParams[i] = NULL;
	}

	CRsrcParams additionalRsrcParam;

	MixedLogicalResourceInfo addtionalRmxAvcPartylogicalTypes[]={
			{eLogical_relay_avc_to_svc_rtp_with_audio_encoder,true,true},
			{eLogical_relay_avc_to_svc_rtp,false,false},
			{eLogical_relay_rtp,true,true},
			{eLogical_relay_avc_to_svc_video_encoder_1,true,false},
			{eLogical_relay_avc_to_svc_video_encoder_2,false,false},
			{eLogical_legacy_to_SAC_audio_encoder,true,true} /* true for RMX */
	};

	MixedLogicalResourceInfo addtionalSoftMcuAvcPartylogicalTypes[]={
			{eLogical_relay_avc_to_svc_rtp_with_audio_encoder,true,false},
			{eLogical_relay_avc_to_svc_rtp,false,false},
			{eLogical_relay_rtp,true,false},
			{eLogical_relay_avc_to_svc_video_encoder_1,true,false},
			{eLogical_relay_avc_to_svc_video_encoder_2,false,false}
	};

	MixedLogicalResourceInfo addtionalSvcPartylogicalTypes[]={
			{eLogical_video_decoder,true,false}
	};

	MixedLogicalResourceInfo* logicalTypeInfo = NULL;

	int max = 0;
	CRsrcDesc tmpRsrcDesc;
	WORD found = FALSE;
	WORD itemNum = 0;

	// @#@ - verify this block
	eConfMediaType confMediaType=pTempPartyAllocatedRsrc->GetConfMediaType();
	bool mandatory;
	TRACEINTO<<"!@# conference media type confMediaType: " << ConfMediaTypeToString(confMediaType);
	m_pPartyAllocatedRsrc->SetConfMediaType(confMediaType);
	m_pIpInitialMode->SetConfMediaType(confMediaType);
	if(confMediaType != eMixAvcSvc)
	{
		TRACEINTO << "mix_mode: dynMixedErr conference media type confMediaType is not mixed";
		status=STATUS_FAIL;
		PASSERT(101);
		POBJDELETE(pTempPartyAllocatedRsrc);
		return status;
	}

	if (m_bIsMrcCall)
	{
		logicalTypeInfo = addtionalSvcPartylogicalTypes;
		max = sizeof(addtionalSvcPartylogicalTypes) / sizeof(MixedLogicalResourceInfo);
	}
	else
	{
		if (IsSoftMcu())
		{
			logicalTypeInfo = addtionalSoftMcuAvcPartylogicalTypes;
			max = sizeof(addtionalSoftMcuAvcPartylogicalTypes) / sizeof(MixedLogicalResourceInfo);
		}
		else
		{
			logicalTypeInfo = addtionalRmxAvcPartylogicalTypes;
			max = sizeof(addtionalRmxAvcPartylogicalTypes) / sizeof(MixedLogicalResourceInfo);
		}
	}

	eVideoPartyType allocatedVideoPartyType = pTempPartyAllocatedRsrc->GetVideoPartyType();
	TRACEINTO << "!@# allocatedVideoPartyType " << VideoPartyTypeToString(allocatedVideoPartyType);

	for (int i=0; i<max; ++i)
	{
		itemNum = 1;
		m_pPartyAllocatedRsrc->DeleteRsrcDescAccordingToLogicalResourceType(logicalTypeInfo[i].lrt); // ey_20866
		found=pTempPartyAllocatedRsrc->GetRsrcParams(additionalRsrcParam,logicalTypeInfo[i].lrt,itemNum);

		if (found)
		{
			tmpRsrcDesc=pTempPartyAllocatedRsrc->GetRsrcDesc(logicalTypeInfo[i].lrt);
			m_pPartyAllocatedRsrc->AddNewRsrcDesc(&tmpRsrcDesc);
		}
		else
		{
			if (allocatedVideoPartyType==eVideo_party_type_none || allocatedVideoPartyType== eVSW_Content_for_CCS_Plugin_party_type)
			{
				mandatory=logicalTypeInfo[i].mandatoryAud;
			}
			else
			{
				mandatory=logicalTypeInfo[i].mandatoryVid;
			}

			if (mandatory==true)
			{
				TRACEINTO << "!@# dynMixedErr reallocation for dynamic mixed mode failed missing lrt: "
					<< LogicalResourceTypeToString(logicalTypeInfo[i].lrt);  // ey_20866
				PASSERT(101);
				status=STATUS_FAIL;
			}
		}
	}

	ISDN_PARTY_IND_PARAMS_S tempIsdnParams;
	SVC_PARTY_IND_PARAMS_S  svcParams;

	if (!m_bIsMrcCall)
	{
		pMrmpRsrcParams = new CRsrcParams;
		TRACEINTO << "mix_mode: pMrmpRsrcParams= " << pMrmpRsrcParams;

		WORD found = m_pPartyAllocatedRsrc->GetRsrcParams(*pMrmpRsrcParams, eLogical_relay_rtp); // eyaln
		if (found && pMrmpRsrcParams->GetRsrcDesc())
		{
			if (pMrmpRsrcParams->GetRsrcDesc()->GetConnectionId() == 0)
			{
				PTRACE(eLevelInfoNormal, " dynMixedErr CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate eLogical_relay_rtp wasn't allocated1");
				POBJDELETE(pMrmpRsrcParams);
				pMrmpRsrcParams=NULL;
				status=STATUS_FAIL;
				PASSERT(101);
				POBJDELETE(pTempPartyAllocatedRsrc);
				return status;
			}

			CRsrcDesc* tmp;
			tmp=pMrmpRsrcParams->GetRsrcDesc();
			TRACEINTO<<"!@# mrmp ConnectionId: " << tmp->GetConnectionId()
				<< " LogicalRsrcType: " << LogicalResourceTypeToString(tmp->GetLogicalRsrcType());
		}
		else
		{
			TRACEINTO << "mix_mode: eLogical_relay_rtp wasn't allocated";
			POBJDELETE(pMrmpRsrcParams);
			pMrmpRsrcParams=NULL;
		}


		for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
		{
			WORD itemNum=1;
			WORD found;
			avcToSvcTranslatorRsrcParams[i]=new CRsrcParams;
			found=m_pPartyAllocatedRsrc->GetRsrcParams(*avcToSvcTranslatorRsrcParams[i],logicalTypeInfo[i].lrt,itemNum);

			if (found)
			{
				TRACEINTO << "!@# good:  translator: " << i << " will be opened";
				CRsrcDesc* tmp = avcToSvcTranslatorRsrcParams[i]->GetRsrcDesc();
				TRACEINTO << "!@# avcToSvcTranslatorRsrcParams[" << i << "] ConnectionId: " << tmp->GetConnectionId()
					<< " LogicalRsrcType: " << LogicalResourceTypeToString(tmp->GetLogicalRsrcType());
			}
			else
			{
				POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
				avcToSvcTranslatorRsrcParams[i]=NULL;

				if(allocatedVideoPartyType==eVideo_party_type_none || allocatedVideoPartyType== eVSW_Content_for_CCS_Plugin_party_type)
				{
					mandatory=logicalTypeInfo[i].mandatoryAud;
				}
				else
				{
					mandatory=logicalTypeInfo[i].mandatoryVid;
				}

				if(mandatory==true)
				{
					TRACEINTO << "!@# dynMixedErr :  translator: " << i << " will not be opened"; // ey_20866
					status=STATUS_FAIL;
					PASSERT(101);
				}
			}
		}

		pParam->Get((BYTE*)(&m_udpAddresses), sizeof(UdpAddresses));
		pParam->Get((BYTE*)(&tempIsdnParams), sizeof(ISDN_PARTY_IND_PARAMS_S));
		pParam->Get((BYTE*)(&svcParams), sizeof(SVC_PARTY_IND_PARAMS_S));

		UpdateScmWithResources(svcParams, allocatedVideoPartyType, pTempPartyAllocatedRsrc->GetIsAvcVswInMixedMode());
	}

	m_pPartyAllocatedRsrc->DumpToTrace();

	if (status==STATUS_OK && allocationType==ALLOCATION_TYPE_UPGRADE)
	{
		if(m_bIsMrcCall)
		{
			TRACEINTO<<"!@# dynMixedPosAck svc party: upgrade starting timer";
		}
		else
		{
			TRACEINTO<<"!@# dynMixedPosAck avc party: upgrade starting timer";
		}
		
		if(IsSoftMcu() && (allocatedVideoPartyType==eVideo_party_type_none))
			TRACEINTO << "mix_mode: For softMCU, audio only doesn't need start the timer";
		else	
			StartTimer(UPGRADETOMIXTOUT, 25*SECOND); // ASK HOW MUCH TIME   EY_20866
	}

	POBJDELETE(pTempPartyAllocatedRsrc);
	return status;
}

void CIpPartyCntl::OnUpgradePartyToMixed(CSegment* pParam)
{
	TRACEINTO << "mix_mode: started";
	bool result;

	m_bIsBridgeUpgradedAudio=true;
	m_bIsBridgeUpgradedVideo=true;
	// bridge  14289
	m_pIpCurrentMode->SetStreamsListForMediaMode(m_pIpInitialMode->GetStreamsListForMediaMode(cmCapAudio, cmCapReceive, kRolePeople), cmCapAudio, cmCapReceive, kRolePeople);
	m_pIpCurrentMode->SetStreamsListForMediaMode(m_pIpInitialMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople), cmCapVideo, cmCapReceive, kRolePeople);

	if(m_bIsMrcCall)
	{
		if(m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) ||  m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))
		{
			TRACEINTO<<"mix_mode: requesting VB to commence  upgrade";
			m_pVideoBridgeInterface->UpgradeToMixAvcSvcForRelayParty(GetPartyRsrcId());
		}
		else
		{
			TRACEINTO<<"mix_mode: !@# audio participant !@#";
			FinishUpgradeToMix();
		}
	}
	else
	{

	  if(m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) ||  m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))
	  {
	    TRACEINTO<<"!@# video participant";
	  }
	  else
	  {
	    TRACEINTO<<"!@# audio participant";
	  }


	   if(m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) ||  (m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))  )
	    {
		*pParam>>m_incomingVideoChannelHandle ;
		CAvcToSvcParams *pAvcToSvcParams=new CAvcToSvcParams;
		TRACEINTO << "mix_mode: m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle;
		pAvcToSvcParams->SetChannelHandle(m_incomingVideoChannelHandle);
		pAvcToSvcParams->SetIsSupportAvcSvcTranslate(true);
		pAvcToSvcParams->SetVideoOperationPointsSet(m_pIpInitialMode->GetOperationPoints());
		if (m_pIpInitialMode->GetOperationPoints()->m_numberOfOperationPoints==0)
		{
		    TRACEINTOFUNC << "mix_mode: dynMixedErr vops is empty!"; //  ey_20866
		}


		result=FillStreamSSRCForMedia_noRelay(pAvcToSvcParams);
		if(result==false)
		{
		  TRACEINTO<<"dynMixedErr either no streams or layerid is invalid";
		}
		TRACEINTO<<"mix_mode: dynMixedPosAck requesting VB to commence  upgrade";
		m_bIsBridgeUpgradedVideo=false;
		m_pVideoBridgeInterface->UpgradeToMixAvcSvcForNonRelayParty(GetPartyRsrcId(),pAvcToSvcParams); // ey_20866


		POBJDELETE(pAvcToSvcParams);
	    }

	    if(!IsSoftMcu())
	    {
	       TRACEINTO<<"mix_mode: dynMixedPosAck requesting Audio bridge  to commence  upgrade";
		m_bIsBridgeUpgradedAudio=false;
		m_pAudioInterface->UpgradeToMixAvcSvc(GetPartyRsrcId());

	    }
	    if(m_bIsBridgeUpgradedAudio==true && m_bIsBridgeUpgradedVideo==true)
	    {
	       TRACEINTO<<" dynMixedPosAck SoftMcu audio only no need to upgrade bridge";
	       FinishUpgradeToMix();
	    }


	}
//	m_pTaskApi->ReplayUpgradeSvcAvcTranslate(this,VIDEO,statOK); // ey_20866 simulate a response from conference
}

void CIpPartyCntl::OnEscalateUpgradePartyToMixed(CSegment* pParam)
{
	TRACEINTO << "mix_mode: started";
	bool result;

	m_bIsBridgeUpgradedAudio=true;
	m_bIsBridgeUpgradedVideo=true;

	if(m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) ||  m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))
	{
		TRACEINTO<<"!@# video participant";
	}
	else
	{
		TRACEINTO<<"!@# audio participant";
	}


	if(m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) ||  (m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))  )
	{
		CAvcToSvcParams *pAvcToSvcParams=new CAvcToSvcParams;
		TRACEINTO << "mix_mode: m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle;
		pAvcToSvcParams->SetChannelHandle(m_incomingVideoChannelHandle);
		pAvcToSvcParams->SetIsSupportAvcSvcTranslate(true);
		pAvcToSvcParams->SetVideoOperationPointsSet(m_pIpInitialMode->GetOperationPoints());
		if (m_pIpInitialMode->GetOperationPoints()->m_numberOfOperationPoints==0)
		{
			TRACEINTOFUNC << "mix_mode: dynMixedErr vops is empty!"; //  ey_20866
		}

		result=FillStreamSSRCForMedia_noRelay(pAvcToSvcParams);
		if(result==false)
		{
			TRACEINTO<<"dynMixedErr either no streams or layerid is invalid";
		}
		TRACEINTO<<"mix_mode: dynMixedPosAck requesting VB to commence  upgrade";
		m_bIsBridgeUpgradedVideo=false;
		m_pVideoBridgeInterface->UpgradeToMixAvcSvcForNonRelayParty(GetPartyRsrcId(),pAvcToSvcParams); // ey_20866


		POBJDELETE(pAvcToSvcParams);
	}
}

bool CIpPartyCntl::UpgradeVideoBridgesIfNeeded()
{
	TRACEINTO << "mix_mode: started";
	bool result = false;

	if (m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) ||  m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))
	{
		TRACEINTO << "video participant";
	}
	else
	{
		TRACEINTO << "audio participant";
	}

	if (m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) ||  (m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople)))
	{
		CAvcToSvcParams* pAvcToSvcParams = new CAvcToSvcParams;
		TRACEINTO << "mix_mode: m_incomingVideoChannelHandle=" << m_incomingVideoChannelHandle;

		pAvcToSvcParams->SetChannelHandle(m_incomingVideoChannelHandle);
		pAvcToSvcParams->SetIsSupportAvcSvcTranslate(true);
		pAvcToSvcParams->SetVideoOperationPointsSet(m_pIpInitialMode->GetOperationPoints());

		if (m_pIpInitialMode->GetOperationPoints()->m_numberOfOperationPoints == 0)
			TRACEINTO << "mix_mode: dynMixedErr vops is empty!";

		result = FillStreamSSRCForMedia_noRelay(pAvcToSvcParams);
		if (!result)
			TRACEINTO << "dynMixedErr either no streams or layerid is invalid";

		TRACEINTO << "mix_mode: dynMixedPosAck requesting VB to commence  upgrade";
		m_pVideoBridgeInterface->UpgradeToMixAvcSvcForNonRelayParty(GetPartyRsrcId(), pAvcToSvcParams); // ey_20866

		POBJDELETE(pAvcToSvcParams);
	}

	return result;
}


void CIpPartyCntl::OnEndAudioUpgradeToMix(CSegment* pParam)
{
	DWORD status = STATUS_OK;
	*pParam >> status;
	if (status != STATUS_OK)
	{
		TRACEINTO << "mix_mode: dynMixedErr failed to connect audio bridge status = " << status;
		// disconnect party
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, SIP_TOUT_DURING_UPGRADE_TO_MIXED);
		return;
	}
	bool isComplete;
	m_bIsBridgeUpgradedAudio = true;
	TRACEINTO << "mix_mode: dynMixedPosAck audio bridge finished upgrade m_bIsBridgeUpgradedAudio:"<<(int)m_bIsBridgeUpgradedAudio;
	isComplete = CheckBridgesUpgradeCompletion();
	if(isComplete)
	{
		FinishUpgradeToMix();
	}
}
void CIpPartyCntl::OnEndVideoUpgradeToMix(CSegment* pParam)
{
	DWORD status = STATUS_OK;
	*pParam >> status;
	if (status != STATUS_OK)
	{
		TRACEINTO << "mix_mode: dynMixedErr failed to connect video bridge status = " << status;
		// disconnect party
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, SIP_TOUT_DURING_UPGRADE_TO_MIXED);
		return;
	}

	bool isComplete;
	m_bIsBridgeUpgradedVideo=true;
	if(m_bIsMrcCall)
	{
		TRACEINTO<<"mix_mode: dynMixedPosAck svc party: video bridge finished upgrade m_bIsBridgeUpgradedVideo:"<<(int)m_bIsBridgeUpgradedVideo;
	}
	else
	{
		TRACEINTO<<"mix_mode: dynMixedPosAck avc party: video bridge finished upgrade m_bIsBridgeUpgradedVideo:"<<(int)m_bIsBridgeUpgradedVideo;
	}

	isComplete = CheckBridgesUpgradeCompletion();
	if(isComplete)
	{
		FinishUpgradeToMix();
	}
}

void CIpPartyCntl::FinishUpgradeToMix()
{
	if(m_bIsMrcCall)
	{
		TRACEINTO<<"mix_mode: dynMixedPosAck svc party: bridge(s) finished upgrade to mixed";
	}
	else
	{
		TRACEINTO<<"mix_mode: dynMixedPosAck avc party: bridge(s) finished upgrade to mixed";
	}
	if (IsValidTimer(UPGRADETOMIXTOUT))
	{
	    DeleteTimer(UPGRADETOMIXTOUT);
	    TRACEINTO<<"mix_mode: DeleteTimer(UPGRADETOMIXTOUT)";
	}
	m_bPartyInUpgradeProcess = false;
	m_state = IDLE;
	m_pPartyApi->SendEndVideoUpgradeToMix();
	HandlePendingScmIfNeeded();

}

bool  CIpPartyCntl::CheckBridgesUpgradeCompletion()
{
	bool isComplete = false;

	// @#@ add audio only here !
	if (m_bIsMrcCall || IsSoftMcu())
	{
		if (m_bIsBridgeUpgradedVideo == true)
		{
			isComplete=true;
		}
	}
	else
	{
		if (m_bIsBridgeUpgradedVideo == true && m_bIsBridgeUpgradedAudio == true)
		{
			isComplete=true;
		}
	}

	return isComplete;
}

////////////////////////////////////////////////////////////////////////////
CConfIpParameters* CIpPartyCntl::GetConfIpParameters(CConfParty* pConfParty)
{
	CConfIpParameters* pServiceParams = NULL;
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	const char* serviceName  = pConfParty->GetServiceProviderName();
	BYTE interfaceType = pConfParty->GetNetInterfaceType();
	pServiceParams = pIpServiceListManager->GetRelevantService(serviceName, interfaceType);
	if (pServiceParams == NULL)
	{
		PASSERTMSG(1,"CIpPartyCntl::GetConfIpParameters - IP ServiceList is empty, can't configure Default service!!!");
		return NULL;
	}

	return pServiceParams;
}
////////////////////////////////////////////////////////////////////////////
char* CIpPartyCntl::GetUniqueGuid(char* strConfGUID,CConfParty* pConfParty)
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetUniqueGuid ");
	//char* strConfGUID = (char*)pConf->GetConfGuid();
	WORD isFirst = 1;
	for (int b = 0 ; b < 16 ; b++)
	{
		if (strConfGUID[b] != '\0')
		{
			isFirst = 0;
			break;
		}
	}


	if (isFirst)
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetUniqueGuid 2 ");

	    	CConfIpParameters* pServiceParams = GetConfIpParameters(pConfParty);
		if (pServiceParams == NULL)
		{
		   PASSERTMSG(1,"CIpPartyCntl::GetUniqueGuid - IP ServiceList is empty, can't configure Default service!!!");
		   //return;
		}
		else
		{
			  if(!pServiceParams->GetServiceName())
			  {
			    	PASSERTMSG(1,"CIpPartyCntl::GetUniqueGuid - Service Name in list is NULL!!!");
			    	return NULL;
			  }

			  ipAddressIf localIpAddr;
			  DWORD serIpAddr = 0;
			  if (pServiceParams->GetIPAddressTypesInService() == eIpType_IpV4 || pServiceParams->GetIPAddressTypesInService() == eIpType_Both)
			  {
			  	localIpAddr = pServiceParams->GetIpV4Address();
			  	serIpAddr = localIpAddr.v4.ip;
			  }
			  else
			  {
			  	localIpAddr = pServiceParams->GetIpV6Address(0);
			  	memcpy(&serIpAddr, localIpAddr.v6.ip,4);
			  }
			  // allocate unique Conf GUID (currently used only in H323 dial out calls.
			  ::CalculateUniqueNumber((char*)strConfGUID,serIpAddr);
		}
	}

	return strConfGUID;
}

////////////////////////////////////////////////////////////////////////////
WORD CIpPartyCntl::TranslateToConfPartyConnectionType(WORD dialType)
{
	if (dialType == DIAL_OUT)
		return DIALOUT;
	else if (dialType == DIAL_IN)
		return DIALIN;
	else
	{
		PASSERTMSG(dialType,"CIpPartyCntl::TranslateToConfPartyConnectionType");
		return  dialType;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::GetEncryptKeyExchangeModeFlag(BYTE& bAllowDtlsFlag , BYTE& bAllowSdesFlag ) const
{
	bAllowDtlsFlag = FALSE;
	bAllowSdesFlag = FALSE;

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
    std::string strEncryptionMode;
    sysConfig->GetDataByKey(CFG_KEY_SIP_ENCRYPTION_KEY_EXCHANGE_MODE, strEncryptionMode);

    if(strEncryptionMode.compare("AUTO")==0)
    {
    	bAllowDtlsFlag = TRUE;
    	bAllowSdesFlag = TRUE;
    }
    else if(strEncryptionMode.compare("DTLS")==0)
    {
		PTRACE(eLevelInfoNormal,"CConf::GetEncrypKeyExchangeModeFlag : SIP_ENCRYPTION_KEY_EXCHANGE_MODE is DTLS");
    	bAllowDtlsFlag = TRUE;
    }
    else if(strEncryptionMode.compare("SDES")==0)
    {
		PTRACE(eLevelInfoNormal,"CConf::GetEncrypKeyExchangeModeFlag : SIP_ENCRYPTION_KEY_EXCHANGE_MODE is SDES");
    	bAllowSdesFlag = TRUE;
    }
    else if(strEncryptionMode.compare("NONE")==0)
    {
		PTRACE(eLevelInfoNormal,"CConf::GetEncrypKeyExchangeModeFlag : SIP_ENCRYPTION_KEY_EXCHANGE_MODE is NONE");
    	bAllowDtlsFlag = FALSE;
    	bAllowSdesFlag = FALSE;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdatePartyEncryptionMode(CConfParty* pConfParty, CIpComMode * pPartyScm, BYTE confEncType) const
{
	BYTE bShouldDtlsEncrypt = FALSE;
	BYTE bShouldSdesEncrypt = FALSE;
	BYTE bAllowDtlsFlag 	= FALSE;
	BYTE bAllowSdesFlag 	= FALSE;
	BYTE eConfPartyEncryptionSetting = pConfParty->GetIsEncrypted();
	//BYTE confEncryptionType = m_pConf->GetCommConf()->GetEncryptionType();
	BYTE isTipCompatible 	= pConfParty->GetIsTipCall(); //GetCommConf()->GetIsTipCompatible(); //BRIDGE-4095
	BYTE bShouldEncryptProfile = FALSE;
	BYTE bShouldDisconnectIfEncryptionFails = FALSE;
	BYTE bIsSip = (pConfParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE);
	BYTE bIsSVC = (pConfParty->GetPartyMediaType() == eSvcPartyType);
	//check whether the connection should be encrypted, and if a non-encryptred connection is allowed in case the EP doesn't support encryption.
	::ResolveEncryptionParameters( confEncType, pConfParty, bShouldEncryptProfile, bShouldDisconnectIfEncryptionFails);
	// update selected communication mode with the encryption mode


	GetEncryptKeyExchangeModeFlag(bAllowDtlsFlag , bAllowSdesFlag );
	bShouldSdesEncrypt = bShouldEncryptProfile && bAllowSdesFlag && bIsSip;
	pPartyScm->SetEncryption(bShouldEncryptProfile, bShouldDisconnectIfEncryptionFails);

	if(bShouldEncryptProfile)
	{
		if(bIsSip)
		{
			if(bShouldSdesEncrypt)
				pPartyScm->CreateLocalSipComModeSdes(TRUE, (bIsSVC? TRUE: isTipCompatible));
			else
				pPartyScm->SetEncryption(FALSE, bShouldDisconnectIfEncryptionFails);
		}
		else
		{
			pPartyScm->CreateLocalComModeECS(kAES_CBC,kHalfKeyDH1024);
		}
	}
	else
	{
		if(bIsSip)
			pPartyScm->CreateLocalSipComModeSdes(FALSE);
		else
			pPartyScm->CreateLocalComModeECS(kUnKnownMediaType,kHalfKeyUnKnownType);
	}

	//DTLS
	bShouldDtlsEncrypt = bShouldEncryptProfile && bIsSip && isTipCompatible && bAllowDtlsFlag;
	pPartyScm->SetDtlsEncryption(bShouldDtlsEncrypt);
	pPartyScm->CreateLocalSipComModeDtls(bShouldDtlsEncrypt, isTipCompatible);
}
////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetScmForMrcCall(CIpComMode* pPartyScm, CVideoOperationPointsSet*pConfOperationPointsSet, BYTE cascadeMode)
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::SetScmForMrcCall");

	  //Disable LPR for SVC
    	pPartyScm->SetIsLpr(FALSE);

	// Audio:
//	DWORD maxSendSsrc = AUDIO_MAX_SEND_SSRC_DEFAULT;
//	DWORD maxRecvSsrc = 0;
//	if (cascadeMode == CASCADE_MODE_SLAVE)
//	{
//		maxSendSsrc = 0;
//		maxRecvSsrc = AUDIO_MAX_SEND_SSRC_LINK;
//	}
//	else if (cascadeMode == CASCADE_MODE_MASTER)
//	{
//		DWORD maxSendSsrc = AUDIO_MAX_SEND_SSRC_LINK;
//	}

	pPartyScm->SetMediaOff(cmCapAudio, cmCapReceiveAndTransmit);
	pPartyScm->SetSacScm(cmCapReceiveAndTransmit);


	// Video:
	if (pPartyScm->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople)) //BG Audio only conference: check IsVideo
	{
		APIU16 profile;
		APIU8 level;
		long mbps, fs, dpb, brAndCpb, sar, staticMB;

		DWORD bitRate = pPartyScm->GetMediaBitRate(cmCapVideo, cmCapReceive);
		pPartyScm->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
		pPartyScm->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit);
		pPartyScm->SetSvcScm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceiveAndTransmit);
		pPartyScm->SetVideoBitRate(bitRate,cmCapReceive);
		pPartyScm->SetVideoBitRate(bitRate,cmCapTransmit);

		pConfOperationPointsSet->Dump();
		pPartyScm->SetOperationPointsAndRecvStreamsGroup(pConfOperationPointsSet);
	}

	// SCP:
	CComModeInfo comModeInfo(LSD_6400, (WORD)INDEX_START_OF_LSD);
	CapEnum ipDataCapCode = comModeInfo.GetH323ModeType();
	DWORD   ipFeccRate    = 64; //NOTE: only 6.4 now!!
	pPartyScm->SetFECCMode(ipDataCapCode, ipFeccRate, cmCapReceiveAndTransmit);

	pPartyScm->SetIsSupportScp(TRUE);

	pPartyScm->Dump("Mrc scm",eLevelInfoNormal);
}
////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetScmForVswRelay(CIpComMode* pPartyScm, WORD bIsAudioOnly, CVideoOperationPointsSet* pConfOperationPointsSet)
{
    PTRACE(eLevelInfoNormal,"avc_vsw_relay: CIpPartyCntl::SetScmForVswRelay");

	// Video:
	APIU16 profile;
	APIU8 level;
	long mbps, fs, dpb, brAndCpb, sar, staticMB,bitRate;

	if (bIsAudioOnly)
	{
		return;
	}

	pPartyScm->SetToOperationPointsOnly(pConfOperationPointsSet);

	std::list <VideoOperationPoint> vopList = pConfOperationPointsSet->m_videoOperationPoints;
	const VideoOperationPoint*	pOperationPoint =pPartyScm->GetLowestOperationPoint(0); //vopList.begin();

	PASSERTMSG_AND_RETURN(pOperationPoint==NULL,"pOperationPoint is NULL");

	// for receive set the lowest operation point
	pPartyScm->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
	//	  TRACEINTOFUNC<<"@@@!original profile: "<<profile<<" level: "<<(int)level<<" mbps: "<<mbps<<" fs: "<<fs;
	//	  mbps = (::CalcOperationPointMBPS(*pOperationPoint)/500);
	//fs = (pOperationPoint->m_frameWidth * pOperationPoint->m_frameHeight)>>16;
	//fs = CalcOperationPointFS(*pOperationPoint);

	//	  if(fs==0)
	//	  {
	//		fs=-1;
	//		mbps=-1;
	//	  }
	//  level = ProfileToLevelTranslator::ConvertResoltionAndRateToLevel(itr->m_frameHeight, itr->m_frameWidth, itr->m_frameRate);

	EOperationPointPreset eOPPreset = eOPP_cif;
	const CCommConf* pCommConf = m_pConf->GetCommConf();
	if (pCommConf)
		eOPPreset = pCommConf->GetOperationPointPreset();

	if ( SetPredefinedH264ParamsForVswRelayIfNeeded(eOPPreset, level, fs, mbps, pPartyScm->GetIsUseOperationPointesPresets(), *pOperationPoint) == false )
	{
		mbps = CalcMBPSforVswRelay(*pOperationPoint);
		fs = CalcFSforVswRelay(*pOperationPoint);
		ProfileToLevelTranslator plt;
		level = plt.ConvertResolutionAndRateToLevelEx(fs, mbps);
	}

	profile = ProfileToLevelTranslator::SvcProfileToH264(pOperationPoint->m_videoProfile);

	TRACEINTOFUNC<<"@@@!after profile:(middle o.p.) "<<profile<<" level: "<<((int)level)<<" mbps: "<<mbps<<" fs: "<<fs;

	pPartyScm->SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive,H264_SINGLE_NAL_PACKETIZATION_MODE);
	bitRate=pOperationPoint->m_maxBitRate*10;
	//	  TRACEINTOFUNC<<"@@@! bitRate"<<bitRate;
	pPartyScm->SetVideoBitRate(bitRate,cmCapReceive,kRolePeople);

	pOperationPoint =pPartyScm->GetHighestOperationPoint(0); //vopList.begin();

	PASSERTMSG_AND_RETURN(pOperationPoint==NULL,"pOperationPoint is NULL");

	// for receive set the lowest operation point
	pPartyScm->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit);
	//	  mbps = ::CalcOperationPointMBPS(*pOperationPoint)/500;
	//	 // fs = (pOperationPoint->m_frameWidth * pOperationPoint->m_frameHeight)>>16;
	//	  fs = CalcOperationPointFS(*pOperationPoint);
	//	  fs = GetMaxFsAsDevision(fs); // roundup
	//	  if(fs==0)
	//	  {
	//		fs=-1;
	//		mbps=-1;
	//	  }
	//	  level = ProfileToLevelTranslator::ConvertResoltionAndRateToLevel(itr->m_frameHeight, itr->m_frameWidth, itr->m_frameRate);

	if (SetPredefinedH264ParamsForVswRelayIfNeeded( eOPPreset, level, fs, mbps, pPartyScm->GetIsUseOperationPointesPresets(), *pOperationPoint) == false )
	{
		mbps = CalcMBPSforVswRelay(*pOperationPoint);
		fs = CalcFSforVswRelay(*pOperationPoint);
		ProfileToLevelTranslator plt;
		level = plt.ConvertResolutionAndRateToLevelEx(fs, mbps);
	}
	profile = ProfileToLevelTranslator::SvcProfileToH264(pOperationPoint->m_videoProfile);
	//	  TRACEINTOFUNC<<"@@@!after profile (highest o.p.): "<<profile<<" level: "<<(int)level<<" mbps: "<<mbps<<" fs: "<<fs;

	pPartyScm->SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit,H264_SINGLE_NAL_PACKETIZATION_MODE);
	bitRate=pOperationPoint->m_maxBitRate*10;
	//	  TRACEINTOFUNC<<"@@@! bitRate"<<bitRate;
	pPartyScm->SetVideoBitRate(bitRate,cmCapTransmit,kRolePeople);
 	pConfOperationPointsSet->Dump(); //@@@ - fix this to print correctly op. points
	pPartyScm->Dump("avc_vsw_relay:initial SCM",eLevelInfoNormal);
}

////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetScmForAvcMixMode(CIpComMode* pPartyScm, WORD bIsAudioOnly)
{
    TRACEINTO << "@#@";

    // Video:
    APIU16 profile;
    APIU8 level;
    long mbps, fs, dpb, brAndCpb, sar, staticMB;
    DWORD bitRate, AVCThrshldRate;

    if (bIsAudioOnly)
    {
        TRACEINTO << "Audio only!!!";
        return;
    }

    const VideoOperationPoint* pOperationPoint = pPartyScm->GetHighestOperationPoint(0);
    PASSERTMSG_AND_RETURN(pOperationPoint==NULL, "pOperationPoint is NULL");

    CH264VideoCap* pH264VideoCap = (CH264VideoCap *)CBaseCap::AllocNewCap(eH264CapCode, NULL);
    PASSERTMSG_AND_RETURN(pH264VideoCap==NULL, "pH264VideoCap is NULL");

    pH264VideoCap->SetAccordingToOperationPoint(*pOperationPoint);
    COstrStream msg;
    pH264VideoCap->Dump(msg);
    TRACEINTO << "Operation Point Cap \n" << msg.str().c_str();
    DWORD details = 0;
    bool bShouldUpdateProfile = true;
    if (!pPartyScm->IsMediaContaining(*pH264VideoCap, kCapCode|kH264Profile|kH264Additional|kBitRate,&details,cmCapVideo,cmCapReceive))
    {
        if (!pPartyScm->IsMediaContaining(*pH264VideoCap, kCapCode|kH264Additional|kBitRate,&details,cmCapVideo,cmCapReceive))
        {
            TRACEINTO << "Initial cap is lower than the highest operation point. Do not update SCM.";
            pH264VideoCap->FreeStruct();
            POBJDELETE(pH264VideoCap);

	     //FSN-613: Dynamic Content for SVC/Mix Conf: check if need to limit videorate
	     bitRate = pOperationPoint->m_maxBitRate * 10;
	     /*pPartyScm->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);
		 
	     if (pOperationPoint->m_rsrcLevel == eResourceLevel_HD1080)
	    {
	    	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
	       BOOL isHighProfile = (profile == H264_Profile_High);
		AVCThrshldRate = CResRsrcCalculator::GetRateThrshldBasedOnVideoModeType(GetSystemCardsBasedMode(), vidQuality, isHighProfile, eHD1080Symmetric); 
		TRACEINTO << "bitRate = " << bitRate <<", AVCThrshldRate = " <<AVCThrshldRate;
		//DWORD audioRate = pPartyScm->GetMediaBitRate(cmCapAudio, cmCapReceive)*10;
	       bitRate = max(bitRate, AVCThrshldRate);
	    }*/

	     DWORD curPartyVideoRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);
	     if (curPartyVideoRate > bitRate)
    	    	pPartyScm->SetVideoBitRate(bitRate, cmCapReceive, kRolePeople);
            return;
        }

        TRACEINTO << "Base profile - do not update the profile";
        bShouldUpdateProfile = false;
//        bitRate = pOperationPoint->m_maxBitRate * 10;
//        TRACEINTOFUNC << "bitRate = " << bitRate;
//        pPartyScm->SetVideoBitRate(bitRate, cmCapReceive, kRolePeople);
//        pH264VideoCap->FreeStruct();
//        POBJDELETE(pH264VideoCap);
//        return;
    }

    pH264VideoCap->FreeStruct();
    POBJDELETE(pH264VideoCap);

    // for receive set the lowest operation point
    pPartyScm->GetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive);

     //FSN-613: Dynamic Content for SVC/Mix Conf
    APIU8  packetizationMode = 0;
    packetizationMode = pPartyScm->GetH264PacketizationMode(cmCapReceive);
    mbps = CalcMBPSforVswRelay(*pOperationPoint);
    fs = CalcFSforVswRelay(*pOperationPoint);
    ProfileToLevelTranslator plt;
    level = plt.ConvertResolutionAndRateToLevelEx(fs, mbps);
    if (bShouldUpdateProfile)
    	profile = ProfileToLevelTranslator::SvcProfileToH264(pOperationPoint->m_videoProfile);

    TRACEINTO << "after profile:(middle o.p.) "<<profile<<" level: "<<((int)level)<<" mbps: "<<mbps<<" fs: "<<fs;
    pPartyScm->SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapReceive, packetizationMode);
   	pPartyScm->SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit, packetizationMode);

    bitRate = pOperationPoint->m_maxBitRate * 10;
    TRACEINTOFUNC << "bitRate = " << bitRate;
    //FSN-613: Dynamic Content for SVC/Mix Conf, since bitRate = 1232 in pOperationPoint for HD1080, while it is different from AVC requirement for HD1080. (1536)
    /*if (pOperationPoint->m_rsrcLevel == eResourceLevel_HD1080)
    {
    	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
       BOOL isHighProfile = (profile == H264_Profile_High);
	AVCThrshldRate = CResRsrcCalculator::GetRateThrshldBasedOnVideoModeType(GetSystemCardsBasedMode(), vidQuality, isHighProfile, eHD1080Symmetric); 
	TRACEINTO << "bitRate = " << bitRate <<", AVCThrshldRate = " <<AVCThrshldRate;
	//DWORD audioRate = pPartyScm->GetMediaBitRate(cmCapAudio, cmCapReceive)*10;
       bitRate = max(bitRate, AVCThrshldRate);
    }*/
 
    pPartyScm->SetVideoBitRate(bitRate, cmCapReceive, kRolePeople);
    pPartyScm->SetVideoBitRate(bitRate, cmCapTransmit, kRolePeople);

    //if (pOperationPoint->m_rsrcLevel == eResourceLevel_HD1080)
    //{
    	//since there is no 1080p60 source in mix conf, sync Tx 
    	//pPartyScm->SetH264Scm(profile, level, mbps, fs, dpb, brAndCpb, sar, staticMB, cmCapTransmit, packetizationMode);
	//pPartyScm->SetVideoBitRate(bitRate, cmCapTransmit, kRolePeople);
    //}

    pPartyScm->Dump("***initial SCM", eLevelInfoNormal);
}

////////////////////////////////////////////////////////////////////////////
CIpComMode* CIpPartyCntl::NewAndGetPartyCntlScm(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams )
{
	CIpComMode*	pConfIpScm						= partyControInitParam.pConfIpScm;
	BYTE			bIsEncryption					= partyControInitParam.bIsEncript;
	BYTE			confEncType						= partyControInitParam.encryptionType;
	eConfMediaType confMediaType					= partyControlDataParams.confMediaType;
	CConfParty		*pConfParty						= partyControInitParam.pConfParty;
	CVideoOperationPointsSet*pConfOperationPointsSet 	= partyControlDataParams.pConfOperationPointsSet;
	BYTE			bIsMrcHeader					= partyControlDataParams.bIsMrcHeader;
	BYTE			bIsMrcCall						= partyControlDataParams.bIsMrcCall;
	BYTE            bIsLync2013Call                 = FALSE;

	if (pConfParty == NULL)
	{
		PTRACE2(eLevelInfoNormal,"CIpPartyCntl::NewAndGetPartyCntlScm: ConfParty is NULL! Name - ",m_partyConfName);
		DBGPASSERT(GetPartyRsrcId());
	}

/* MSSlave Flora comment: add for the MSSlaveOut */
	PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::NewAndGetPartyCntlScm - Setting MS to true ",partyControlDataParams.lyncEpType );
	if (pConfParty && (((partyControlDataParams.lyncEpType == Lync2013 || partyControlDataParams.lyncEpType == AvMcuLync2013Slave) && IsFeatureSupportedBySystem(eFeatureMs2013SVC)) ||
		(pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 ||	pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown)))
	{
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::NewAndGetPartyCntlScm: Lync 2013, ICE=", pConfParty->GetEnableICE());
		WORD serviceId = pConfParty->GetServiceId();

		CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(serviceId);
				if ( m_AvMcuLinkType != eAvMcuLinkNone || (( pConfParty && pConfParty->GetEnableICE())) )
				{
					TRACEINTO << "Setting MS to true";
					bIsLync2013Call = true;
				}
	}



	pConfIpScm->Dump("N.A. DEBUG pConfIpScm->Dump"); //N.A. DEBUG VP8


	CIpComMode*	pPartyScm  = new CIpComMode(*pConfIpScm);

	pPartyScm->Dump("N.A. DEBUG pPartyScm->Dump"); //N.A. DEBUG VP8

	if (pConfParty)
		pPartyScm->ChangeH264ScmForForceParty(pConfParty->GetName());

	pPartyScm->SetIsUseOperationPointesPresets(partyControlDataParams.isUseOperationPointesPresets);

	/* In case we're in VEQ flow and Video-main was not enabled in the initial call setup - do not add the video-main caps. */
	BOOL bVeqFlowEnableVideo = pConfParty && (!pConfParty->GetPlcmRequireMask() || (pConfParty->GetPlcmRequireMask() && (pConfParty->GetPlcmRequireMask() & m_plcmRequireVideoMain)));

	if (partyControlDataParams.bIsOffer && !bVeqFlowEnableVideo)
	{
		BOOL enableTagsCfg = NO;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("X_PLCM_REQUIRE_VIDEO_MAIN_ENABLE_TAG", enableTagsCfg);

		if (enableTagsCfg)
		{
			if (pConfParty)
			TRACEINTO << "VEQ scenario without video-main, disabling video plcmRequireMask = " << pConfParty->GetPlcmRequireMask() << " bVeqFlowEnableVideo = " << (bVeqFlowEnableVideo ? "YES" : "NO") << " IsOfferer = " << (partyControlDataParams.bIsOffer ? "YES" : "NO") << " enablevideoTagsCfg = " << (enableTagsCfg ? "YES" : "NO");
			pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
		}
	}


	/* In case we're in VEQ flow and Video-main was not enabled in the initial call setup - do not add the video-main caps. */
	BOOL bVeqFlowEnableData = (pConfParty && (!pConfParty->GetPlcmRequireMask() ||
							   ((pConfParty->GetPlcmRequireMask() && (pConfParty->GetPlcmRequireMask() & m_plcmRequireFecc )))));

	if (partyControlDataParams.bIsOffer && !bVeqFlowEnableData)
	{
		BOOL enableTagsCfg = NO;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("X_PLCM_REQUIRE_FECC_ENABLE_TAG", enableTagsCfg);

		if (enableTagsCfg)
		{
			if (pConfParty)
			TRACEINTO << "VEQ scenario without FECC, disabling FECC data plcmRequireMask = " << pConfParty->GetPlcmRequireMask() << " bVeqFlowEnableData = " << (bVeqFlowEnableData ? "YES" : "NO") << " IsOfferer = " << (partyControlDataParams.bIsOffer ? "YES" : "NO") << " enableDataTagsCfg = " << (enableTagsCfg ? "YES" : "NO");
			pPartyScm->SetMediaOff(cmCapData, cmCapReceiveAndTransmit, kRolePeople);
		}
	}

	BOOL bVeqFlowEnableBFCP = (pConfParty && (!pConfParty->GetPlcmRequireMask() ||
							   ((pConfParty->GetPlcmRequireMask() && (pConfParty->GetPlcmRequireMask() & m_plcmRequireBfcpUdp || pConfParty->GetPlcmRequireMask() & m_plcmRequireBfcpTcp)))));

	if (partyControlDataParams.bIsOffer && !bVeqFlowEnableBFCP)
	{
		if (pConfParty)
		TRACEINTO << "VEQ scenario without BFCP, disabling bfcp plcmRequireMask = " << pConfParty->GetPlcmRequireMask() << " bVeqFlowEnableBFCP = " << (bVeqFlowEnableBFCP ? "YES" : "NO") << "IsOfferer = " << (partyControlDataParams.bIsOffer ? "YES" : "NO");
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	}

	BYTE bIsAudioOnly = FALSE;
	if (pConfParty)
		bIsAudioOnly = pConfParty->GetVoice();

	if (bIsMrcCall)
	{
		BYTE cascadeMode = CASCADE_MODE_NONE;
		if (pConfParty)
			cascadeMode = pConfParty->GetCascadeMode();
		SetScmForMrcCall(pPartyScm,partyControlDataParams.pConfOperationPointsSet, cascadeMode);

		if (cascadeMode == CASCADE_MODE_SLAVE)
			partyControlDataParams.bIsOffer = FALSE;
		else
		partyControlDataParams.bIsOffer = TRUE;
		if (pConfParty)
		{
		pConfParty->SetPartyMediaType(eSvcPartyType);
		TRACEINTOFUNC << "PartyMediaType = " << (WORD)pConfParty->GetPartyMediaType();
	}
	}
	else if (confMediaType == eMixAvcSvcVsw)
	{
	    SetScmForVswRelay(pPartyScm,bIsAudioOnly,pConfOperationPointsSet);
	}
	else if (confMediaType == eMixAvcSvc)
	{
	    pPartyScm->SetOperationPoints(pConfOperationPointsSet);
//		CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
//		if (pCommConf && pCommConf->GetEnableHighVideoResInAvcToSvcMixMode()) //@#@ - add HD in conf too
//			SetScmForAvcMixMode(pPartyScm,pConfParty->GetVoice());

	}

	// set conf media type
	TRACEINTOFUNC << "confMediaType = " << confMediaType;
	pPartyScm->SetConfMediaType(confMediaType);

	if (bIsAudioOnly)
	{
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
		 pPartyScm->SetMediaOff(cmCapData, cmCapReceiveAndTransmit, kRolePeople);
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
	}

	//TIP DIAL OUT
	if (pConfParty)
	{
	WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());

	if (partyControInitParam.bIsTipCompatibleVideo && confPartyConnectionType==DIALOUT && pConfParty->GetMsftAvmcuState() == eMsftAvmcuNone && partyControlDataParams.lyncEpType != AvMcuLync2013Slave)
			pConfParty->SetIsTipCall(TRUE);

	PTRACE2INT(eLevelError,"CIpPartyCntl::NewAndGetPartyCntlScm : pConfParty->GetIsTipCall() ", pConfParty->GetIsTipCall());

	if (pConfParty->GetIsTipCall())
		pPartyScm->SetTipMode(eTipModePossible);
	else
		pPartyScm->SetTipMode(eTipModeNone);
	}

	if (pConfParty)
	UpdatePartyEncryptionMode(pConfParty,pPartyScm,confEncType);

	//N.A. DEBUG VP8
	pPartyScm->Dump("N.A. DEBUG pPartyScm->Dump after add encrytption mode");

	DWORD confRate = 0;
	DWORD vidBitrate = 0;
	int val = -1;
	if (pConfParty)
	val = ::CalculateRateForIpCalls(pConfParty, pPartyScm, confRate, vidBitrate, bIsEncryption);

	PASSERT((val == -1));

	if (pConfParty)
	{
	CSmallString str;
	str << "CIpPartyCntl::NewAndGetPartyCntlScm vidBitrate = " << vidBitrate
		<< ",confRate = "<< confRate
		<< ",isMutebyOperator "<<pConfParty->IsAudioMutedByOperator()
		<< ",isMutebyMCU " << pConfParty->IsAudioMutedByMCU();
	PTRACE (eLevelInfoNormal, str.GetString());
	}

	vidBitrate = vidBitrate / 100;// divided by 100 because is used in system in
	if (confMediaType != eMixAvcSvcVsw || m_bIsMrcCall) // do not update AVC VSW
	    pPartyScm->SetVideoBitRate(vidBitrate, cmCapReceiveAndTransmit); // SLTODO - maybe isn't needed for cop


	if(pConfParty && GetIsWebRtcCall() /*YES == GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_WEBRTC_SUPPORT)*/) // N.A. DEBUG if identify the party as Webrtc type can change the scm to VP8 for this specific party
	{
		PTRACE(eLevelError,"N.A. DEBUG WebRTC Call - set SCM to VP8 instead of H264 in funcCIpPartyCntl::NewAndGetPartyCntlScm");
		SetScmForWebRTCCall(pPartyScm, pConfParty);
		//N.A. DEBUG VP8
		pPartyScm->Dump("N.A. DEBUG pPartyScm->Dump after SetScmForWebRTCCall");
	}

	//N.A. DEBUG VP8


    BYTE protocol = AUTO;
    if (pConfParty)
    	protocol = pConfParty->GetVideoProtocol();
	PTRACE2INT(eLevelError,"CIpPartyCntl::NewAndGetPartyCntlScm : protocol= ", protocol);

	if (pConfParty && bIsLync2013Call && (protocol != VIDEO_PROTOCOL_RTV && protocol != VIDEO_PROTOCOL_H264))
	{
		/* MSSlave Flora comment: call this function to set proper MS-Svc caps according to the max resolution */
		SetScmForLync2013Call(pPartyScm,pConfParty);
		pPartyScm->SetTipMode(eTipModeNone);

		//LYNC2013_FEC_RED:
		BOOL isSipFecEnabled = TRUE;
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		pSysConfig->GetBOOLDataByKey("ENABLE_SIP_LYNC2013_FEC", isSipFecEnabled);
		if	(isSipFecEnabled == TRUE)
			pPartyScm->SetIsFec(TRUE);

		BOOL isSipRedEnabled = TRUE;
		pSysConfig->GetBOOLDataByKey("ENABLE_SIP_LYNC2013_RED", isSipRedEnabled);
		if	(isSipRedEnabled == TRUE)
			pPartyScm->SetIsRed(TRUE);
	}

	if (pConfParty && (pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 || pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown))
	{
		PTRACE2(eLevelError,"CIpPartyCntl::NewAndGetPartyCntlScm : MsftAvmcuState= ", enMsftAvmcuStateNames[pConfParty->GetMsftAvmcuState()]);
		pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
	    pPartyScm->SetMediaOff(cmCapData, cmCapReceiveAndTransmit, kRolePeople);
	 //   pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
	 //   pPartyScm->SetAudioAlg(eG7221_24kCapCode,cmCapReceiveAndTransmit);

	}

	if ((pPartyScm->GetConfMediaType() == eMixAvcSvc) && (!bIsMrcCall))
		{
			PASSERT(partyControInitParam.pConf == 0);
			CCommConf* pCommConf = (CCommConf*)partyControInitParam.pConf->GetCommConf();

			if (pCommConf->GetEnableHighVideoResInAvcToSvcMixMode() && pCommConf->GetEnableHighVideoResInSvcToAvcMixMode())
			{
				m_isHDVSWEnabled = 1;
				const VideoOperationPoint* pVideoOperationPoint =  pConfOperationPointsSet->GetHighestOperationPoint(this->GetPartyId());
				CConfParty* pConfParty = partyControInitParam.pConfParty;

				PTRACE2INT(eLevelError,"CIpPartyCntl::NewAndGetPartyCntlScm : video rate =  ", pPartyScm->GetMediaBitRate(cmCapVideo, cmCapReceive));

                PASSERTMSG_AND_RETURN_VALUE(!pVideoOperationPoint, "!pVideoOperationPoint", pPartyScm);
				if (pPartyScm->GetMediaBitRate(cmCapVideo, cmCapReceive)/10 >= pVideoOperationPoint->m_maxBitRate)
				{
					if (pVideoOperationPoint->m_frameHeight > 720) {
					    if(pConfParty) pConfParty->SetMaxResolution(eHD1080_Res);
                    } else {
						if(pConfParty) pConfParty->SetMaxResolution(eHD720_Res);
                    }
				}
			}
		}

	return pPartyScm;
}
////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::InitDisplayNameForNetSetup(CIpNetSetup* pIpNetSetup)
{

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string productTypeStr;
	std::string key = "MCU_DISPLAY_NAME";
	sysConfig->GetDataByKey(key, productTypeStr);
	// VNFE-2507the display name flag default value changed to "" ,
	// if it is empty change according to product type, otherwise according to system flag
	if (productTypeStr.size() == 0)
	{
		// VNGR-10405 set the display name according to product type and not according to system flag
//		productTypeStr = "Polycom RMX 2000";
//		eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
//		if(eProductTypeRMX4000 == curProductType)
//			productTypeStr = "Polycom RMX 4000";
//		if(eProductTypeCallGenerator == curProductType)
//			productTypeStr = "Polycom Call Generator";
//		else if (eProductTypeRMX1500 == curProductType)
//			productTypeStr = "Polycom RMX 1500";

		// BRIDGE-5459 - EP receive display as connected to "RMX 2000" when in actual it is connected to cloud Axis
		productTypeStr = m_pConf->GetProductTypeAsString();
	}
	pIpNetSetup->SetLocalDisplayName(productTypeStr.c_str());
}
////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetSeviceIdForConfPartyByConnectionType(CConfParty* pConfParty,CIpNetSetup* pIpNetSetup)
{
	// Set Service Name if empty
	if (pConfParty->GetConnectionType() == DIAL_OUT)
	{
		SetSeviceIdForConfParty(pConfParty);
	}
	else
	{
		WORD serviceId  = (WORD)pIpNetSetup->GetCsServiceid();
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::SetSeviceIdForConfParty :  service Id - ",serviceId);
		pConfParty->SetServiceId(serviceId);
	}

}
////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetSeviceIdForConfParty(CConfParty* pConfParty)
{
 	CConfIpParameters* pServiceParams = GetConfIpParameters(pConfParty);
	if (pServiceParams)
	{
		pConfParty->SetServiceProviderName((const char*)(pServiceParams->GetServiceName ()));
		WORD serviceId  = (WORD)pServiceParams->GetServiceId();
		PTRACE2INT (eLevelInfoNormal, "CIpPartyCntl::SetSeviceIdForConfPartyOfDialOut - serviceId ", serviceId );
		pConfParty->SetServiceId(serviceId);
	}
}
////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::InitSetupTaAddr(CIpNetSetup* pIpNetSetup,char* conf_name,CConfParty* pConfParty,BYTE bIsH323)
{
    PASSERTMSG_AND_RETURN(!pConfParty, "CIpPartyCntl::InitSetupTaAddr - !pConfParty");

	CConfIpParameters* pServiceParams = GetConfIpParameters(pConfParty);
	if (pServiceParams == NULL)
	{
		PASSERTMSG(1,"CIpPartyCntl::InitSetupTaAddr - IP ServiceList is empty, can't configure Default service!!!");
		return;
	}

	mcTransportAddress partyAddr = pConfParty->GetIpAddress();
	// if party ip address not null and there is no matched(same type) ip within service, cannot continue.
	if( !::isApiTaNull(&partyAddr) && !::isIpVersionMatchBetweenPartyAndService(&partyAddr, pServiceParams) )
	{
		PASSERTMSG(3,"CIpPartyCntl::InitSetupTaAddr - No match between service and EP ip types!!!");
	   	return;
	}

	ipAddressIf localIpAddr;
	mcTransportAddress srcServiceIpAddr;
	memset(&srcServiceIpAddr,0,sizeof(mcTransportAddress));

	if( !::isApiTaNull(&partyAddr) && !bIsH323)
	{
	    if (partyAddr.ipVersion == (APIU32)eIpVersion4)
	    {
	  	  localIpAddr = pServiceParams->GetIpV4Address();
	  	  srcServiceIpAddr.addr.v4.ip = localIpAddr.v4.ip;

	    }
	    else
	    {

		BYTE place = ::FindIpVersionScopeIdMatchBetweenPartyAndService(&partyAddr, pServiceParams);
		TRACEINTO << "CIpPartyCntl::InitSetupTaAddr - place = " << (DWORD)place;

		if (place == 0xFF)
		{
		  PASSERTMSG(4,"CIpPartyCntl::InitSetupTaAddr - No IpV6 in Service");
		  return;
		}

		localIpAddr = pServiceParams->GetIpV6Address((int)place);
		memcpy(&(srcServiceIpAddr.addr.v6),(void*)&localIpAddr.v6,sizeof(ipAddressV6If));
		srcServiceIpAddr.ipVersion = partyAddr.ipVersion;
	    }
	    pIpNetSetup->SetIpVersion((enIpVersion)partyAddr.ipVersion);
	    if (partyAddr.ipVersion == (APIU32)eIpVersion6)
	    {
	    	((CSipNetSetup*)pIpNetSetup)->SetPerferedIpV6ScopeAddr((enScopeId)(srcServiceIpAddr.addr.v6.scopeId));
	    	PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitSetupTaAddr  -SCOPE PREFFERED : ",((enScopeId)(srcServiceIpAddr.addr.v6.scopeId)));
	    }
	}
	//add for BRIDGE-15392
	else if((::isApiTaNull(&partyAddr))&&(bIsH323) )
	{
		  if (pServiceParams == NULL)
		  {
		 TRACEINTO << " Ip Service doesn't exist ";
		  }
		  else
		  {
			eIpType  ipType = pServiceParams->GetIPAddressTypesInService();
			if(ipType == eIpType_IpV6)
			{
				TRACEINTO << "Set IpType to IPv6";
				pIpNetSetup->SetIpVersion(eIpVersion6);
				//Set Ip Version in Src and Dst address
				memset(&srcServiceIpAddr,0,sizeof(mcTransportAddress));
				srcServiceIpAddr.ipVersion = eIpVersion6;
				pIpNetSetup->SetTaDestPartyAddr(&srcServiceIpAddr);
			}
		  }
	}
    	else
    	{
		// In this case we fill the Src Addr with the correct CS address
		BYTE bIsProxy = TRUE;
		char* pProxyAddress = new char[MaxLengthOfSingleUrl];
		memset(pProxyAddress,0, MaxLengthOfSingleUrl);
		CSmallString outboundProxyName = pServiceParams->GetSipProxyName();
		mcTransportAddress proxyAddr;
		memset(&proxyAddr,0,sizeof(mcTransportAddress));
		if(outboundProxyName.IsEmpty() == NO && pServiceParams->GetSipProxyStatus() != eServerStatusOff)
		{
			memcpy(pProxyAddress,outboundProxyName.GetString(),MaxLengthOfSingleUrl - 1);
			pProxyAddress[MaxLengthOfSingleUrl - 1] = '\0';
			TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - pProxyAddress = " << pProxyAddress;
			stringToIp(&proxyAddr,pProxyAddress);
		}
		else
			bIsProxy = FALSE;

	  	localIpAddr = pServiceParams->GetIpV4Address();
	  	srcServiceIpAddr.addr.v4.ip = localIpAddr.v4.ip;
	  	srcServiceIpAddr.ipVersion = eIpVersion4 ;

		if(bIsH323)
			pIpNetSetup->SetIpVersion((enIpVersion)partyAddr.ipVersion);
		else if (bIsProxy && !(((CSipNetSetup*)pIpNetSetup)->isUriWithDomainInIpFormat()) && proxyAddr.ipVersion == (APIU32)eIpVersion6)
		{
			BYTE place = ::FindIpVersionScopeIdMatchBetweenPartyAndService(&proxyAddr, pServiceParams);
			TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - place (Proxy case) = " << place;
			srcServiceIpAddr.ipVersion = eIpVersion6 ;
			localIpAddr = pServiceParams->GetIpV6Address((int)place);
			memcpy(&(srcServiceIpAddr.addr.v6),(void*)&localIpAddr.v6,sizeof(ipAddressV6If));
			pIpNetSetup->SetIpVersion((enIpVersion)proxyAddr.ipVersion);
			((CSipNetSetup*)pIpNetSetup)->SetPerferedIpV6ScopeAddr((enScopeId)(proxyAddr.addr.v6.scopeId));
			PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitSetupTaAddr  -SCOPE PREFFERED : ",((enScopeId)(proxyAddr.addr.v6.scopeId)));
		}
		else
		{
			int scopeId = -1;

			// added for ms ice
			DWORD serviceIpType = eIpType_IpV4;

			WORD serviceId = pConfParty->GetServiceId();
			CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(serviceId);
			if(pService)
			{
				serviceIpType = pService->GetIPAddressTypesInService();
			}

			scopeId = ((CSipNetSetup*)pIpNetSetup)->SetIpVersionAccordingToUri(serviceIpType);

			if(scopeId != -1)
			{
				BYTE place = 0;
				place = ::FindPlaceAccordingtoScopeType((enScopeId)scopeId, pServiceParams);
				localIpAddr = pServiceParams->GetIpV6Address((int)place);
				memcpy(&(srcServiceIpAddr.addr.v6),(void*)&localIpAddr.v6,sizeof(ipAddressV6If));
				srcServiceIpAddr.ipVersion = eIpVersion6 ;
				((CSipNetSetup*)pIpNetSetup)->SetPerferedIpV6ScopeAddr((enScopeId)(srcServiceIpAddr.addr.v6.scopeId));
				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::InitSetupTaAddr  -SCOPE PREFFERED : ",((enScopeId)(srcServiceIpAddr.addr.v6.scopeId)));
				pIpNetSetup->SetIpVersion(eIpVersion6);
			}

		}

		delete [] pProxyAddress;
    	}


	char tempName[64];
	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	ipToString(pConfParty->GetIpAddress(),tempName,1);
	TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - pConfParty->GetIpAddress() " << tempName << "\n";

	//char tempName[64];
	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	ipToString(pConfParty->GetIpAddress(),tempName,1);
	TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - pConfParty->GetSipPartyAddress() (1) " << pConfParty->GetSipPartyAddress() << "\n";
	TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - pConfParty->GetIpAddress() (1) " << tempName << "\n";
	//For H323 calls with GK, this field will be left as empty 
	pIpNetSetup->SetTaSrcPartyAddr(&srcServiceIpAddr);


	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	mcTransportAddress ttt;
	memcpy(&ttt,pIpNetSetup->GetTaSrcPartyAddr(),sizeof(mcTransportAddress));
	ipToString(ttt,tempName,1);
	TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - pIpNetSetup->GetTaSrcPartyAddr()  " << tempName << "\n";

	if (bIsH323 == NO)
	{
	    memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	    mcTransportAddress tttDest;
	    memcpy(&tttDest,pIpNetSetup->GetTaDestPartyAddr(),sizeof(mcTransportAddress));
	    ipToString(tttDest,tempName,1);
	    pIpNetSetup->SetDestPartyAddress(tempName);
	}

	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	memcpy(&ttt,pIpNetSetup->GetTaDestPartyAddr(),sizeof(mcTransportAddress));
	ipToString(ttt,tempName,1);

	TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - pIpNetSetup->GetTaDestPartyAddr()  " << tempName << "\n";

	// Get Quality of Service from reservation
	if (bIsH323 == NO ) //sip
	{
		char localIp[IPV6_ADDRESS_LEN];
		memset (&localIp,'\0',IPV6_ADDRESS_LEN);
		mcTransportAddress localSipAddr;
		memcpy(&localSipAddr,pIpNetSetup->GetTaSrcPartyAddr(),sizeof(mcTransportAddress));
		ipToString(localSipAddr,localIp,1);

		char confName[H243_NAME_LEN+8];
		memset(confName, '\0', H243_NAME_LEN+8);
		CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
		if (pCommConf && pCommConf->GetIsCallGeneratorConference())   //bridge-8006
			snprintf(confName, sizeof(confName) -1, "%s_(%03d)", conf_name, pCommConf->NextCGPartiesCounter());
		else
			memcpy(confName, conf_name, sizeof(confName));

		confName[sizeof(confName)-1]='\0';
		int nameLen = strlen(confName);
		TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - setting sip from address  name len is  " << (DWORD)nameLen;
		((CSipNetSetup *)pIpNetSetup)->SetLocalSipAddress(confName,nameLen,localIp);
		TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - setting sip from address  name ip is   " <<localIp;
		TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - setting sip from address  name  is   " << confName;
	}

	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	//mcTransportAddress ttt;
	memcpy(&ttt,pIpNetSetup->GetTaSrcPartyAddr(),sizeof(mcTransportAddress));
	ipToString(ttt,tempName,1);
	TRACEINTO << "CIpPartyCntl::InitSetupTaAddr - pIpNetSetup->GetTaSrcPartyAddr()  " << tempName << "\n";
	// remove the difference between RSS and other calls vngr-13232
	partyAddr.port = pConfParty->GetCallSignallingPort();
	//BRIDGE-15392: set dst address only when it's not NULL
	if(::isApiTaNull(&partyAddr) == FALSE)
	{
		pIpNetSetup->SetTaDestPartyAddr(&partyAddr);
	}

	// IpV6
	if(pConfParty->GetRecordingLinkParty())
	{
	  memset(&partyAddr,0,sizeof(mcTransportAddress));
	  pIpNetSetup->SetIpVersion((enIpVersion)partyAddr.ipVersion);
	}
//Already SET!!!!	
//	else
//	{
//	  partyAddr.port = pConfParty->GetCallSignallingPort();
//	  pIpNetSetup->SetTaDestPartyAddr(&partyAddr);
//	}

	if (bIsH323 == NO)
	{
	    memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	    mcTransportAddress tttDest;
	    memcpy(&tttDest,pIpNetSetup->GetTaDestPartyAddr(),sizeof(mcTransportAddress));
	    ipToString(tttDest,tempName,1);
	    pIpNetSetup->SetDestPartyAddress(tempName);
	}

	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	memcpy(&ttt,pIpNetSetup->GetTaDestPartyAddr(),sizeof(mcTransportAddress));
	ipToString(ttt,tempName,1);

	TRACEINTO << "CIpPartyCntl::InitSetupTaAddr  - pIpNetSetup->GetTaDestPartyAddr()  " << tempName << "\n";

	if (bIsH323 == NO && ::isApiTaNull(pIpNetSetup->GetTaDestPartyAddr()) == TRUE) //sip
	{
		const char* strSipAddress = ((CSipNetSetup*)pIpNetSetup)->GetRemoteSipAddress();
		((CSipNetSetup *)pIpNetSetup)->SetDestPartyAddress(strSipAddress);
	}
}


////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetPartyScmForTip(CIpComMode* pPartyScm, CConfParty* pConfParty, CSipNetSetup* pNetSetup, DWORD serviceId,PartyControlDataParameters &partyControlDataParams)
{
	PASSERTMSG_AND_RETURN(!pConfParty, "!pConfParty");
	PASSERTMSG_AND_RETURN(!pPartyScm, "!pPartyScm");

	pPartyScm->SetTipMode(eTipModePossible);
	BYTE IsTIPContentEnable = partyControlDataParams.bIsTIPContentEnable;

	int videoBitRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::SetPartyScmForTip, DEBUG TIP - rate: ", videoBitRate);

	//BRIDGE-5753
	// Change PartyScm resolution according to TIP definitions
	//Eh264VideoModeType h264VidMode = GetTipResolutionTypeAccordingToVideoRate(videoBitRate);

	//const CCommConf* pCommConf = m_pConf->GetCommConf();
	//PASSERTMSG_AND_RETURN(!pCommConf ,"!pCommConf");

	//EVideoResolutionType maxResolution  = (EVideoResolutionType)(pCommConf->GetConfMaxResolution());
	EVideoResolutionType maxResolution  = (EVideoResolutionType)partyControlDataParams.maxConfResolution;
	Eh264VideoModeType   h264VidMode 	= GetTipResolutionTypeAccordingToMaxResolutionAndVidRate(maxResolution, videoBitRate, pConfParty);

	FixTipScmVideoBitRateIfNeeded(pConfParty , pPartyScm, pNetSetup, h264VidMode);

	pPartyScm->SetAudioAlg(eAAC_LDCapCode,cmCapReceiveAndTransmit);



		pPartyScm->SetIsLpr(FALSE);

		//pPartyScm->SetMediaOff(cmCapData, cmCapReceiveAndTransmit, kRolePeople);

		//pPartyScm->SetIsBfcp(FALSE);//Bfcp still disable in TIP

	PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::SetPartyScmForTip : TIP partyResolution ==> chosen h264VidMode: ", (WORD)h264VidMode);

	pConfParty->SetMaxResolution((BYTE)h264VidMode); /* bridge-10010 */

	H264VideoModeDetails h264VidModeDetails;

	CH264VideoMode* pH264VidMode = new CH264VideoMode();
	pH264VidMode->GetH264VideoModeDetailsAccordingToTypeForTIP(h264VidModeDetails, h264VidMode);
	POBJDELETE(pH264VidMode);

	PTRACE(eLevelInfoNormal, "CIpPartyCntl::SetPartyScmForTip, might be a TIP call, set main profile for TIP !!!");
	pPartyScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR, cmCapReceiveAndTransmit);

	if (IsTIPContentEnable && pPartyScm->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation))
		pPartyScm->SetTipAuxFPS(eTipAux5FPS);

	if( partyControlDataParams.pSipRmtCaps && (partyControlDataParams.pSipRmtCaps->GetNumOfCapSets()!=0) && (pConfParty->GetConnectionType() == DIAL_IN))
	{
		PTRACE(eLevelInfoNormal, "CIpPartyCntl::SetPartyScmForTip, disabling content and BFCP  !!!");
		pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::CheckIfTipEnableByParams(CIpComMode* pPartyScm, CConfParty* pConfParty, DWORD confRate, DWORD partyRate)
{
	CapEnum scmVideoProtocol 	= (CapEnum)pPartyScm->GetMediaType(cmCapVideo, cmCapTransmit);

	DWORD	minRate 			= min(confRate, partyRate);

	if (!IsFeatureSupportedBySystem(eFeatureTip))
	{
		PTRACE(eLevelInfoNormal, "CIpPartyCntl::CheckIfTipEnableByParams, NOT TIP - not supported system card !!!");
		return FALSE;
	}

	if (!pConfParty->GetIsTipCall())
	{
		PTRACE(eLevelInfoNormal, "CIpPartyCntl::CheckIfTipEnableByParams, NOT TIP - not a TIP call !!!");
		return FALSE;
	}

	if (scmVideoProtocol != eH264CapCode)
	{
		PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::CheckIfTipEnableByParams, NOT TIP - scmVideoProtocol: ", scmVideoProtocol);
		return FALSE;
	}

	int videoBitRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::CheckIfTipEnableByParams, DEBUG TIP - rate: ", videoBitRate);

	if (videoBitRate < 9360)
	{
		PTRACE2INT(eLevelInfoNormal, "CIpPartyCntl::CheckIfTipEnableByParams, NOT TIP - conf rate: ", videoBitRate);
		return FALSE;
	}

	return TRUE;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//LYNC_AVMCU_1080p30:
/*void CIpPartyCntl::SetScmForLync2013AvMcuMainCall(CIpComMode* pPartyScm, CConfParty* pConfParty)
{
	PASSERTMSG_AND_RETURN(!pPartyScm, "LYNC_AVMCU_1080p30: CIpPartyCntl::SetScmForLync2013AvMcuMainCall - pPartyScm is NULL");
	PASSERTMSG_AND_RETURN(!pConfParty, "LYNC_AVMCU_1080p30: CIpPartyCntl::SetScmForLync2013AvMcuMainCall - pConfParty is NULL");

	BOOL isSLynvAvMcu1080p30Enabled = FALSE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("LYNC_AVMCU_1080p30_ENCODE_RESOLUTION", isSLynvAvMcu1080p30Enabled);

	DWORD videoBitRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);
	Eh264VideoModeType maxH264Mode = GetMaxH264VideoModeForMsSvcAccordingToSettings(pConfParty);
	CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
	MsSvcVideoModeDetails MsSvcDetails;

	if (isSLynvAvMcu1080p30Enabled == FALSE)
		maxH264Mode = min(eHD720Symmetric,maxH264Mode);

	TRACEINTO << "LYNC_AVMCU_1080p30 - maxH264Mode:" << maxH264Mode << ", videoBitRate:" << videoBitRate*100;

	MsSvcVidMode->GetMsSvcVideoParamsByRate(MsSvcDetails,(videoBitRate*100),maxH264Mode,E_VIDEO_RES_ASPECT_RATIO_DUMMY);
	pPartyScm->SetMsSvcScm(MsSvcDetails,cmCapReceiveAndTransmit,videoBitRate);
	pPartyScm->SetAudioAlg(eG722Stereo_128kCapCode,cmCapReceiveAndTransmit);  //Noa - do we need this?
	pPartyScm->Dump("CIpPartyCntl::SetScmForLync2013AvMcuMainCall - LYNC_AVMCU_1080p30: scm:" ,eLevelInfoNormal);

	POBJDELETE(MsSvcVidMode);
}*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetScmForLync2013Call(CIpComMode* pPartyScm, CConfParty* pConfParty)
{
	PASSERTMSG_AND_RETURN(!pPartyScm, "CIpPartyCntl::SetScmForLync2013Call - pPartyScm is NULL");
	PASSERTMSG_AND_RETURN(!pConfParty, "CIpPartyCntl::SetScmForLync2013Call - pConfParty is NULL");

	DWORD videoBitRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);
	if(pConfParty->GetAvMcuLinkType() != eAvMcuLinkNone || pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 ||	pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown )
	{
		videoBitRate = videoBitRate -640; //for av-mcu reduce 64k for audio in advanced
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::SetScmForLync2013Call -av-mcu reduce audio as 64k videoBitRate ",videoBitRate);
	}

	//Calculate max resolution mode for MS SVC accoring to party/conf/system setting(need to add max cp mode)
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::SetScmForLync2013Call");

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

	Eh264VideoModeType maxH264Mode = GetMaxH264VideoModeForMsSvcAccordingToSettings(pConfParty, pCommConf);

	//LYNC_AVMCU_1080p30:
	BOOL isSLynvAvMcu1080p30Enabled = FALSE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("LYNC_AVMCU_1080p30_ENCODE_RESOLUTION", isSLynvAvMcu1080p30Enabled);
	if ( isSLynvAvMcu1080p30Enabled == FALSE && (pConfParty->GetAvMcuLinkType() != eAvMcuLinkNone || pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 || pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown) )
		maxH264Mode = min(eHD720Symmetric,maxH264Mode);

	TRACEINTO << "LYNC_AVMCU_1080p30 - maxH264Mode:" << maxH264Mode << ", videoBitRate:" << videoBitRate*100;

	MsSvcVideoModeDetails MsSvcDetails;
	CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
	MsSvcVidMode->GetMsSvcVideoParamsByRate(MsSvcDetails,(videoBitRate*100),maxH264Mode,E_VIDEO_RES_ASPECT_RATIO_DUMMY);
	pPartyScm->SetMsSvcScm(MsSvcDetails,cmCapReceiveAndTransmit,videoBitRate);
	BOOL bDisableAvMcuLowRate = GetSystemCfgFlag<BOOL>("DISABLE_LYNC_AV_MCU_128_192_KBPS");
	if(!bDisableAvMcuLowRate && (videoBitRate == 640|| videoBitRate == 1280) && ( pConfParty->GetAvMcuLinkType() != eAvMcuLinkNone || pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 ||	pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown ) )
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::SetScmForLync2013Call - patch for AV-MCU not sending video in 128k call rate! changing to 256k call rate");

		videoBitRate = 1920;

		//videoBitRate = 1120;
		CMsSvcVideoMode* MsSvcVidModeTemp = new CMsSvcVideoMode();
		MsSvcVidModeTemp->GetMsSvcVideoParamsByRate(MsSvcDetails,(videoBitRate*100),maxH264Mode,E_VIDEO_RES_ASPECT_RATIO_DUMMY);
		pPartyScm->SetMsSvcScm(MsSvcDetails,cmCapReceive,videoBitRate);

		POBJDELETE(MsSvcVidModeTemp);

	}

	DWORD confBitRate = pPartyScm->GetCallRate() * 1000;
	DWORD confAudioBitRate = ::CalculateAudioRate(confBitRate);
	DWORD calculatedConfRate = videoBitRate * 100;
	if((confBitRate - confAudioBitRate) != calculatedConfRate)
	{//because of the new rates of 96K and above the round up result is different then the conference video rate
		// to make the calculated rate a multiple of 64k
		calculatedConfRate = (calculatedConfRate % rate64K) ? (calculatedConfRate/rate64K + 1) * rate64K : (calculatedConfRate/rate64K) * rate64K;
		if(calculatedConfRate)
			confBitRate = calculatedConfRate;
		TRACEINTO << "LYNC_G722Stereo128k - confBitRate:" << (DWORD)confBitRate << ", confAudioBitRate:" << (DWORD)confAudioBitRate << ", calculatedConfRate:" << (DWORD)calculatedConfRate;
	}
	// default value is rate1024K
	pPartyScm->DecideOnConfBitRateForAudioSelection(confBitRate);

	DWORD audioBitRate = pPartyScm->GetMediaMode(cmCapAudio).GetBitRate() * _K_;
	BOOL isSLyncEnableG722Stereo = FALSE;
	pSysConfig->GetBOOLDataByKey("LYNC2013_ENABLE_G722Stereo128k", isSLyncEnableG722Stereo);
	TRACEINTO << "LYNC_G722Stereo128k - isSLyncEnableG722Stereo:" << (DWORD)isSLyncEnableG722Stereo << ", audioBitRate:" << (DWORD)audioBitRate
			  << ", confBitRate:" << (DWORD)confBitRate << ", confAudioBitRate:" << (DWORD)confAudioBitRate
			  << ", calculatedConfRate:" << (DWORD)calculatedConfRate << ", rate128K:" << rate128K;
	if ( (isSLyncEnableG722Stereo == TRUE) && (confBitRate == rate1024K) )
		pPartyScm->SetAudioAlg(eG722Stereo_128kCapCode,cmCapReceiveAndTransmit);

	pPartyScm->SetAudioWithDSHforAvMcu();

	pPartyScm->Dump("CIpPartyCntl::SetScmForLync2013Call - scm " ,eLevelInfoNormal);

	POBJDELETE(MsSvcVidMode);

}
void CIpPartyCntl::SetScmForWebRTCCall(CIpComMode* pPartyScm, CConfParty* pConfParty)
{//N.A. DEBUG VP8
	DWORD videoBitRate = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf(); // Amir in Rebase 28-4-2014


	PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::SetScmForWebRTCCall videoBitRate", videoBitRate);
	Eh264VideoModeType maxH264Mode = GetMaxH264VideoModeForMsSvcAccordingToSettings(pConfParty, pCommConf); // N.A. DEBUG Change the function name!!!
	VP8VideoModeDetails VP8VideoDetails;
	CVP8VideoMode* VP8VidMode = new CVP8VideoMode();
	VP8VidMode->GetVp8VideoParamsByRate(VP8VideoDetails, (videoBitRate*100),maxH264Mode, E_VIDEO_RES_ASPECT_RATIO_DUMMY);
	pPartyScm->SetVP8Scm(VP8VideoDetails,cmCapReceiveAndTransmit, videoBitRate);

	POBJDELETE(VP8VidMode);
}
/*
Eh264VideoModeType CIpPartyCntl::GetMaxH264VideoModeForMsSvcAccordingToSettings(CConfParty* pConfParty)
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();
//	PASSERTMSG_AND_RETURN(!pCommConf ,"!pCommConf");
	if(!pCommConf)
	{
		TRACEINTO << "CIpPartyCntl:: CIpPartyCntl::GetMaxH264VideoModeForMsSvcAccordingToSettings : error no comconf";
		return eInvalidModeType;
	}
	EVideoResolutionType maxConfResolution  = (EVideoResolutionType)(pCommConf->GetConfMaxResolution());
	BYTE partyResolution =  eAuto_Res;
	if (pConfParty)
		partyResolution =  pConfParty->GetMaxResolution();

	BYTE maxPartyResolution = eAuto_Res;
	if(partyResolution != eAuto_Res)
		maxPartyResolution = partyResolution;
	else if (maxConfResolution != eAuto_Res)
		maxPartyResolution = maxConfResolution;

	Eh264VideoModeType partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxPartyResolution,eVideoQualitySharpness );
	partyMaxVideoMode = GetMaxMsSvcVideoModeByFlag(partyMaxVideoMode);

	//========================================================================================
	// Adding consideration for the maximum resolution specified above the resolution slider
	//========================================================================================
	Eh264VideoModeType systemMaxVideoMode = GetMaxVideoModeBySysCfg();
	if (eSD30 == systemMaxVideoMode) systemMaxVideoMode = eW4CIF30;
	Eh264VideoModeType chosenMaxVideoMode = min(partyMaxVideoMode, systemMaxVideoMode);
	CSmallString log;
	log << "partyMaxVideoMode[" << partyMaxVideoMode << "], systemMaxVideoMode[" << systemMaxVideoMode << "], chosenMaxVideoMode[" << chosenMaxVideoMode << "]";

	PTRACE2(eLevelInfoNormal,"CIpPartyCntl::GetMaxH264VideoModeForMsSvcAccordingToSettings - ", log.GetString());

//need to check we only consider max cp mode and not actual slider - also what about RTV flag=TBD
	return chosenMaxVideoMode;
}*/

////////////////////////////////////////////////////////////////////////////
/*Eh264VideoModeType CIpPartyCntl::GetMaxMsSvcVideoModeByFlag(Eh264VideoModeType partyVideoMode)
{
	std::string max_mssvc_protocol_str;
    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL res = pSysConfig->GetDataByKey(CFG_MAX_MSSVC_RESOLUTION, max_mssvc_protocol_str);
	PASSERTSTREAM(!res, "CSysConfig::GetDataByKey: " << CFG_MAX_MSSVC_RESOLUTION);
	PTRACE2(eLevelInfoNormal,"CIpPartyCntl::GetMaxMsSvcVideoModeByFlag - MAX_MS_SVC_RESOLUTION flag is: ", (max_mssvc_protocol_str.c_str()));

	BOOL bIsAuto      = ("AUTO" == max_mssvc_protocol_str);
	BOOL bForceHd1080 = ("HD1080" == max_mssvc_protocol_str);
	BOOL bForceHd720  = ("HD720" == max_mssvc_protocol_str);
	BOOL bForceVga    = ("VGA" == max_mssvc_protocol_str);
//	BOOL bForceCif    = ("CIF" == max_mssvc_protocol_str);
//	BOOL bForceQCif   = ("QCIF" == max_mssvc_protocol_str);


	if (bIsAuto || bForceHd1080)
	{
		return partyVideoMode;
	}
	if (bForceHd720)
	{
		return (partyVideoMode >= eHD720At60Asymmetric) ? eHD720Symmetric : partyVideoMode;
	}
	if (bForceVga)
	{
		return (partyVideoMode >= eSD60) ? eW4CIF30 : partyVideoMode;
	}

	//bForceCif || bForceQCif
	return eCIF30;
}*/

////////////////////////////////////////////////////////////////////////////
// Resolution Slider: a new 'Resolution' field is added in the participant's properties.
//This field (together with the video bit rate field) will override any general definition in the conference properties.
BOOL CIpPartyCntl::IsNeedRecalcH264ParamsAccordingToPartySettings( CConfParty* pConfParty ,BYTE maxConfResolution,eVideoQuality vidQuality)
{
	BYTE partyResolution =  pConfParty->GetMaxResolution();
	//BYTE maxConfResolution = m_pCommConf->GetConfMaxResolution();
	BYTE maxPartyResolution = ( partyResolution == eAuto_Res && maxConfResolution != eAuto_Res) ? maxConfResolution : partyResolution;
	if( maxPartyResolution != maxConfResolution && maxPartyResolution != eAuto_Res )
		return TRUE;

	return FALSE;

}
////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::GetH264VideoParamsAccordingToPartySettings( H264VideoModeDetails& returnH264VidModeDetails, CConfParty* pConfParty, DWORD decisionRate, BOOL isHighProfile,eVideoQuality vidQuality,BYTE maxConfResolution)
{
	BYTE partyResolution =  pConfParty->GetMaxResolution();
	//BYTE maxConfResolution = m_pCommConf->GetConfMaxResolution();
	BYTE maxPartyResolution = ( partyResolution == eAuto_Res && maxConfResolution != eAuto_Res) ? maxConfResolution : partyResolution;

	if( partyResolution == eAuto_Res )
	{
		Eh264VideoModeType partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxPartyResolution, vidQuality );
		TRACEINTO << "CIpPartyCntl::GetH264VideoParamsAccordingToPartySettings : partyResolution is Auto_Res ==> chosen partyMaxVideoMode = " << (WORD)partyMaxVideoMode;
		GetH264VideoParams( returnH264VidModeDetails, decisionRate, vidQuality, partyMaxVideoMode, isHighProfile );
	}
	else
	{
		Eh264VideoModeType h264VidMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)partyResolution, vidQuality, decisionRate);
		TRACEINTO << "CIpPartyCntl::GetH264VideoParamsAccordingToPartySettings : partyResolution = " << (WORD)partyResolution << " ==> chosen h264VidMode = " << (WORD)h264VidMode;
		CH264VideoMode* pH264VidMode = new CH264VideoMode();
		pH264VidMode->GetH264VideoModeDetailsAccordingToType( returnH264VidModeDetails, h264VidMode );
		POBJDELETE(pH264VidMode);
	}

}

////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf(CIpComMode* pPartyScm, CConfParty* pConfParty, BYTE partyVidProtocol, DWORD setupRate, BYTE bCreateNewScm,DWORD confRate,WORD dialType,DWORD serviceId,PartyControlDataParameters& partyControlDataParams, CSipNetSetup* pNetSetup)
{
	PASSERT_AND_RETURN(NULL == pPartyScm);

	if (pPartyScm->GetConfType() != kCp && pPartyScm->GetConfType() != kCop)
		return;

	// do not change SCM for AVC VSW
	if (!m_bIsMrcCall && pPartyScm->GetConfMediaType() == eMixAvcSvcVsw)
	    return;

	PASSERT_AND_RETURN(NULL == pConfParty);

	PASSERTMSG_AND_RETURN(NULL == pConfParty, "CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf - pConfParty is NULL!");

	if ((pPartyScm->GetConfMediaType() == eMixAvcSvc) && (!m_bIsMrcCall))
	{
		if (m_isHDVSWEnabled)
		{
			CVideoOperationPointsSet* pConfOperationPointsSet = pPartyScm->GetOperationPoints();
			const VideoOperationPoint* pVideoOperationPoint =  pConfOperationPointsSet->GetHighestOperationPoint(this->GetPartyId());
			PTRACE2INT(eLevelError,"CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf : Video rate =  ", pPartyScm->GetMediaBitRate(cmCapVideo, cmCapReceive));

            PASSERTMSG_AND_RETURN(!pVideoOperationPoint, "CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf : !pVideoOperationPoint");
			if (pPartyScm->GetMediaBitRate(cmCapVideo, cmCapReceive)/10 >= pVideoOperationPoint->m_maxBitRate)
			{
				if (pVideoOperationPoint->m_frameHeight > 720)
					pConfParty->SetMaxResolution(eHD1080_Res);
				else
					pConfParty->SetMaxResolution(eHD720_Res);
			}
			else
				pConfParty->SetMaxResolution(eAuto_Res);
		}
	}

	if (partyControlDataParams.bIsEnableH239)
	{
		DWORD H323AMCRate = partyControlDataParams.h323MaxContentAMCRate;
		BYTE isHDContent1080Supported = partyControlDataParams.bIsHDContent1080Supported;
		BYTE HDResMpi = partyControlDataParams.hdResMpi;
		BYTE isHighProfileContent = partyControlDataParams.bIsHighProfileContent;

		if (H323AMCRate == 0)
		{
            		PTRACE(eLevelInfoNormal,"CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf - disabling BFCP");
			pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		}
		else
		{
		    CapEnum contentProtocol = (CapEnum)partyControlDataParams.contentProtocol;

			if (!partyControlDataParams.bIsTIPContentEnable)
			{
				pPartyScm->SetContent(H323AMCRate,cmCapReceiveAndTransmit,contentProtocol,isHDContent1080Supported,HDResMpi,isHighProfileContent);
			}
			else if (partyControlDataParams.bIsPreferTIP)
			{
				pPartyScm->SetTIPContent(H323AMCRate,cmCapReceiveAndTransmit,FALSE);
			}
			else //eTipCompatibleVideoAndContent
			{
				pPartyScm->SetTIPContent(H323AMCRate,cmCapReceiveAndTransmit);
			}

			if (pConfParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE)
			{
				SetBfcpInSipPartyScm(pPartyScm);
			}
		}
	}

	CapEnum scmProtocol = (CapEnum)pPartyScm->GetMediaType(cmCapVideo, cmCapTransmit);
	CComModeInfo cmInfo = scmProtocol;

	WORD scmProtocolInReservationTerms = cmInfo.GetH320ModeType();

	CCapSetInfo capInfo(scmProtocol);

	if ((scmProtocol == eH264CapCode) && (capInfo.IsSupporedCap() == NO))
	{
		capInfo = eH263CapCode;

		if(capInfo.IsSupporedCap())
			partyVidProtocol = H263;
		else
		{
			capInfo = eH261CapCode;

			if(capInfo.IsSupporedCap())
				partyVidProtocol = H261;
		}
	}
	else if ((scmProtocol == eH263CapCode) && (capInfo.IsSupporedCap() == NO))
		return;

	if(pPartyScm->GetConfType() == kCp)
	{
		//SD changes - use decision matrix regard to conf rate / setup rate / party reservation rate
		DWORD decisionRate 			= 0;
		DWORD callRate 				= partyControlDataParams.callRate;
		DWORD partyReservationRate 	= pConfParty->GetVideoRate();

		if (partyReservationRate != 0xFFFFFFFF)
		{
			DWORD audioRate = pPartyScm->GetMediaBitRate(cmCapAudio);
			partyReservationRate += audioRate;
			partyReservationRate *= 1000;
		}
/*
		if ((partyReservationRate == 0xFFFFFFFF) && (setupRate == 0))
			decisionRate = callRate;
		else if ((partyReservationRate != 0xFFFFFFFF) && (setupRate != 0))
			decisionRate = min (partyReservationRate, setupRate);
		else
			decisionRate = (setupRate ? setupRate : partyReservationRate);
*/
		decisionRate = callRate;
		if (partyReservationRate != 0xFFFFFFFF)
			decisionRate = min (partyReservationRate, decisionRate);
		if (setupRate != 0)
			decisionRate = min (setupRate, decisionRate);

		//Update call rate: Use the new line rate
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf - callRate ", callRate);
		PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf - decisionRate ", decisionRate);
		pPartyScm->SetCallRate(decisionRate/1000);
		partyControlDataParams.callRate = decisionRate;

		BYTE bRecalcH264 = FALSE;
		BOOL useHpDescionMatrix =TRUE;

		if ((partyVidProtocol == H264) && (scmProtocolInReservationTerms != H264))
			bRecalcH264 = TRUE; // conf is H263 and party is H264 => re-calc scm

		if (partyVidProtocol == VIDEO_PROTOCOL_H264 && pPartyScm->GetH264Profile(cmCapReceiveAndTransmit) == H264_Profile_High)
		{
			PTRACE(eLevelInfoNormal,"CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf - FORCE h264 BASELINE");
			bRecalcH264 = TRUE;//change from HP to baseline
			useHpDescionMatrix = FALSE;
			pPartyScm->SetH264Profile(H264_Profile_BaseLine,cmCapReceiveAndTransmit);

		}
		else if (((partyVidProtocol == AUTO) || (partyVidProtocol == H264))
				&& (scmProtocolInReservationTerms == H264))
		{
			//BRIDGE-12263
			BYTE bIsLowerRate = (decisionRate < confRate);
			if(!bIsLowerRate)
				bIsLowerRate = (decisionRate < callRate);

			PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf - bIsLowerRate ",bIsLowerRate);

			BYTE bIsHigherConfiguraedRate = ((decisionRate > callRate) && (partyReservationRate != 0xFFFFFFFF && decisionRate <= partyReservationRate)); //if it is because of a higher setup rate - we won't change scm
			if (bIsLowerRate || bIsHigherConfiguraedRate)
				bRecalcH264 = TRUE;
		}

		//Olga - Resolution Slider: a new 'Resolution' field is added in the participant's properties.
		//This field (together with the video bit rate field) will override any general definition in the conference properties.
		if (!bRecalcH264)
			bRecalcH264 = IsNeedRecalcH264ParamsAccordingToPartySettings( pConfParty,partyControlDataParams.maxConfResolution,partyControlDataParams.vidQuality );

		eVideoQuality vidQuality = partyControlDataParams.vidQuality;
		Eh264VideoModeType partyMaxVideoMode;

		if (bRecalcH264)
		{
			APIU16 lastprofile = 0;

			if (scmProtocol == eH264CapCode)
			{
				lastprofile = pPartyScm->GetH264Profile(cmCapReceiveAndTransmit);
				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf - THIS IS THE PROFILE ",lastprofile);
			}

			BOOL bIsSharpness = NO;

			H264VideoModeDetails h264VidModeDetails;
			GetH264VideoParamsAccordingToPartySettings(h264VidModeDetails, pConfParty, decisionRate,useHpDescionMatrix,partyControlDataParams.vidQuality,partyControlDataParams.maxConfResolution);
			pPartyScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR, cmCapReceiveAndTransmit);

			if (scmProtocol == eH264CapCode)
			{
				pPartyScm->SetH264Profile(lastprofile,cmCapReceiveAndTransmit);
			}
		}
		else
		{
			BOOL bMsEnviroment = FALSE;

			if (pConfParty->GetNetInterfaceType() != H323_INTERFACE_TYPE)
			{
				CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(serviceId);

				if ( pService != NULL && pService->GetConfigurationOfSipServers() )
				{
					if(pService->GetSipServerType() == eSipServer_ms)
						bMsEnviroment = TRUE;
				}

			}

			if	(partyVidProtocol == RTV && bMsEnviroment && (pConfParty->GetNetInterfaceType() != H323_INTERFACE_TYPE) && IsFeatureSupportedBySystem(eFeatureRtv) )
			{
				RTVVideoModeDetails rtvVidModeDetails;
				BYTE partyResolution =  pConfParty->GetMaxResolution();
				BYTE maxConfResolution = partyControlDataParams.maxConfResolution;
				BYTE maxPartyResolution = ( partyResolution == eAuto_Res && maxConfResolution != eAuto_Res) ? maxConfResolution : partyResolution;

				if( partyResolution == eAuto_Res )
				{
					partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxPartyResolution,vidQuality );
					TRACEINTO << "RTV -  partyResolution is Auto_Res ==> chosen partyMaxVideoMode = " << (WORD)partyMaxVideoMode;
					GetRtvVideoParams(rtvVidModeDetails, decisionRate, vidQuality, partyMaxVideoMode);
				}
				else
				{
					partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)partyResolution,vidQuality);
					TRACEINTO << "RTV -  partyResolution = " << (WORD)partyResolution << " ==> chosen h264VidMode = " << (WORD)partyMaxVideoMode;
					GetRtvVideoParams(rtvVidModeDetails, decisionRate, vidQuality, partyMaxVideoMode);
				}

				pPartyScm->SetRtvVideoParams(rtvVidModeDetails,cmCapReceiveAndTransmit);
			}
		}

		BYTE bIsTipCall = (pConfParty && pConfParty->GetIsTipCall());

		if (bIsTipCall)
		{
			SetPartyScmForTip(pPartyScm, pConfParty, pNetSetup , serviceId,partyControlDataParams); //shira-TIP
		}
		else
		{
			pConfParty->SetIsTipCall(FALSE);
			pPartyScm->SetTipMode(eTipModeNone);
			pPartyScm->SetTipAuxFPS(eTipAuxNone);
			BOOL rateChanged = FixVideoBitRateIfNeeded( pConfParty, pPartyScm , pNetSetup, TRUE);

			if( rateChanged )
			{
				partyControlDataParams.callRate = pPartyScm->GetCallRate()*1000;
			}
		}

		pPartyScm->Dump("***CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf - scm " , eLevelInfoNormal);
	}
	else if (pPartyScm->GetConfType() == kCop)
	{
		if (pConfParty->GetVideoRate() !=0xFFFFFFFF)
		{
			// In this stage, the scm receive is the unified mode, that was built according to highest cop level. We need to check if the Party rate is fit for this level.
			CCOPConfigurationList* pCOPConfigurationList = partyControlDataParams.pCOPConfigurationList;
			CCopVideoParams* pCopHighestLevelParams = pCOPConfigurationList->GetVideoMode(0);

			if (pConfParty->GetVideoRate() < GetMinBitRateForCopLevel(pCopHighestLevelParams->GetFormat(),pCopHighestLevelParams->GetFrameRate(),pCopHighestLevelParams->GetProtocol()))
			{
				PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::ChangeScmOfIpPartyInCpOrCopConf : Change scm receive video according to party rate - ", pConfParty->GetVideoRate());
				// Change the scm receive video:
				sCopH264VideoMode copH264VideoMode;
				CCopVideoModeTable* pCopTable = new CCopVideoModeTable;
				BOOL isUseHD1080 = FALSE;
				APIU16 profile = H264_Profile_BaseLine;
				eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

				if (IsFeatureSupportedBySystem(eFeatureH264HighProfile))
				{
					if( pCOPConfigurationList->IsHighProfileFoundInOneLevelAtLeast() )
						profile = H264_Profile_High;

				    isUseHD1080 = TRUE;
				}

				pCopTable->GetSignalingH264ModeAccordingToReservationParams(pCopHighestLevelParams, copH264VideoMode, isUseHD1080, pConfParty->GetVideoRate());
				pPartyScm->SetH264Scm(profile, copH264VideoMode.levelValue, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB, copH264VideoMode.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, copH264VideoMode.maxStaticMbps, cmCapReceive);
				POBJDELETE(pCopTable);
			}
		}
	}

	Eh264VideoModeType vidMode = GetMaxVideoModeBySysCfg();
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	if (partyVidProtocol == H263)
	{
		pPartyScm->SetHighestH263ScmForCP(cmCapReceiveAndTransmit, partyControlDataParams.vidQuality);
	}
	else if (partyVidProtocol == H261)
		pPartyScm->SetHighestH261ScmForCP(cmCapReceiveAndTransmit, partyControlDataParams.vidQuality);
}

////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetupConnParameters(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	CConfParty* pConfParty 		= partyControInitParam.pConfParty;
	DWORD connectDelay 			= partyControInitParam.connectDelay;
	m_pConf						= partyControInitParam.pConf;
	m_pParty					= partyControlDataParams.pParty;

	m_type						= TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
	m_connectDelay				= connectDelay;

	POBJDELETE(m_pCopVideoTxModes);
	if (partyControInitParam.pCopVideoTxModes)
		m_pCopVideoTxModes		= new CCopVideoTxModes(*partyControInitParam.pCopVideoTxModes);

	m_pAudioInterface			= partyControInitParam.pAudioBridgeInterface;
	m_pVideoBridgeInterface		= partyControInitParam.pVideoBridgeInterface;
	m_pFECCBridge				= partyControInitParam.pFECCBridge;
	m_pContentBridge			= partyControInitParam.pContentBridge;
	m_pConfAppMngrInterface		= partyControInitParam.pConfAppMngrInterface;
	m_pTerminalNumberingManager = partyControInitParam.pTerminalNumberingManager;
	m_TcMode					= partyControInitParam.TcMode;
	m_bIsMrcCall				= partyControlDataParams.bIsMrcCall;
	m_bIsWebRtcCall				= partyControlDataParams.bIsWebRtcCall;

	POBJDELETE(m_pBridgeMoveParams);
	m_pBridgeMoveParams 		= new CBridgeMoveParams;
}

//////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::SetBfcpInSipPartyScm(CIpComMode* pPartyScm)
{
	enTransportType transportType = eTransportTypeUdp;

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string dialOutMode;

	sysConfig->GetDataByKey(CFG_KEY_SIP_BFCP_DIAL_OUT_MODE, dialOutMode);

	if(dialOutMode.compare("UDP")==0)
		transportType = eTransportTypeUdp;
	else if(dialOutMode.compare("TCP")==0)
		transportType = eTransportTypeTcp;

	PTRACE2INT(eLevelInfoNormal,"CIpPartyCntl::SetBfcpInSipPartyScm - new transportType according to flag:", transportType);

	pPartyScm->SetBfcp(transportType);

}

//////////////////////////////////////////////////////////////////////////////
BYTE CIpPartyCntl::GetIsMsftEnv()
{
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if  (!pConfParty)
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetIsMsftEnv ERROR pConfParty is NULL !!!!!!!!!!!!");
		return FALSE;
	}


	if(pConfParty->GetNetInterfaceType() != H323_INTERFACE_TYPE)
	{
		CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(m_serviceId);
		if( pService != NULL && pService->GetConfigurationOfSipServers() )
		{
			if(pService->GetSipServerType() == eSipServer_ms)
				PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetIsMsftEnv Working on MSFT ENV");
				return TRUE;
		}
	}

	PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetIsMsftEnv NOT Working on MSFT ENV");
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::GetIsNeedToChangeTipResAccordingToConfVidQuality(CConfParty* pConfParty)
{//BRIDGE-5753
	if(!pConfParty)
	{
		PASSERTMSG(!pConfParty, "!pConfParty");
		return FALSE;
	}

	BOOL bIsFlagOn 		= GetFlagTipResAccordingToConfVidQuality();
	BOOL bIsTipCall		= pConfParty->GetIsTipCall();
	BOOL bIsTipHeader  	= pConfParty->GetIsTipHeader();

	if(!bIsFlagOn)
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetIsNeedToChangeTipResAccordingToConfVidQuality - TIP_RESOLUTION_ACCORDING_TO_CONF_VID_QUALITY is NO ");
		return FALSE;
	}

	if(!bIsTipCall || !bIsTipHeader)
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetIsNeedToChangeTipResAccordingToConfVidQuality - not TIP! ");
		return FALSE;
	}

	return TRUE;;
}


////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::GetFlagTipResAccordingToConfVidQuality()
{//BRIDGE-5753
	BOOL          bRetVal 	 = YES; //Default
	CSysConfig*   sysConfig  = NULL;
	CProcessBase* pProcess   = CProcessBase::GetProcess();

	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();

	if(sysConfig)
		sysConfig->GetBOOLDataByKey(CFG_KEY_TIP_RESOLUTION_ACCORDING_TO_CONF_VID_QUALITY, bRetVal);  // the default value is "NO"

	return bRetVal;
}

////////////////////////////////////////////////////////////////////////////
//Note: for Tip only
DWORD CIpPartyCntl::GetMaxVidBitrateForHD720()
{//BRIDGE-5753
	return 2560;
}

////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::IsNeedToChangeTipVidRate(Eh264VideoModeType selectedVideoMode, DWORD vidBitrate)
{//BRIDGE-5753
	BOOL       bNeedToChange 		  = FALSE;
	DWORD      nVidBitRateFor720HD 	  = GetMaxVidBitrateForHD720() ;
	CMedString str	   		 		  = "";

	if(selectedVideoMode == eHD720Symmetric && vidBitrate > nVidBitRateFor720HD * 10)
	{
		str << "CIpPartyCntl::IsNeedToChangeTipVidRate - TIP call - res HD720 - need to change bit rate from "<< (int)vidBitrate <<" to " << (int)nVidBitRateFor720HD;
		bNeedToChange = TRUE;
	}
	else
	{
		str << "CIpPartyCntl::IsNeedToChangeTipVidRate - no need to change bit rate";
		bNeedToChange = FALSE;
	}

	PTRACE(eLevelInfoNormal,str.GetString());
	return bNeedToChange;
}

////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::ChangeVideoBitrateForTipIfNeeded(CConfParty* pConfParty, CIpComMode* pPartyScm , DWORD& vidBitrate)
{//BRIDGE-5753
	PASSERTMSG_AND_RETURN(!pConfParty || !pPartyScm, "!pConfParty || !pPartyScm");

	if(!GetIsNeedToChangeTipResAccordingToConfVidQuality(pConfParty))
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::ChangeVideoBitrateForTipIfNeeded - no need");
		return;
	}

	int videoBitRateFromScm = pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	vidBitrate = videoBitRateFromScm;
}


////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::FixTipScmVideoBitRateIfNeeded(CConfParty* pConfParty, CIpComMode* pPartyScm , CSipNetSetup* pNetSetup,  Eh264VideoModeType selectedVideoMode)
{//BRIDGE-5753
	PASSERTMSG_AND_RETURN(!pConfParty || !pPartyScm , "!pConfParty || !pPartyScm");

	if(!GetIsNeedToChangeTipResAccordingToConfVidQuality(pConfParty))
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::FixTipScmVideoBitRateIfNeeded - no need ");
		return;
	}

	int nVidBitRateFor720HD = GetMaxVidBitrateForHD720() ;
	int videoBitRate 		= pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	CMedString str = "";
	str << "CIpPartyCntl::FixTipScmVideoBitRateIfNeeded - (Eh264VideoModeType)selectedVideoMode " << (int)selectedVideoMode << " videoBitRate " << videoBitRate;
	PTRACE(eLevelInfoNormal,str.GetString());

	if(IsNeedToChangeTipVidRate(selectedVideoMode, videoBitRate))
	{
		pPartyScm->SetVideoBitRate(nVidBitRateFor720HD * 10, cmCapReceiveAndTransmit);
		pPartyScm->SetTotalVideoRate(nVidBitRateFor720HD * 10);
		pPartyScm->SetCallRate(nVidBitRateFor720HD);
		pConfParty->SetVideoRate(nVidBitRateFor720HD);

		if(pNetSetup)
		{
			TRACEINTO << "pNetSetup->SetMaxRate: " << (int)(nVidBitRateFor720HD * 1000);
			pNetSetup->SetMaxRate(nVidBitRateFor720HD * 1000);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType CIpPartyCntl::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate(EVideoResolutionType maxResolution, DWORD videoBitRate, CConfParty* pConfParty)
{//BRIDGE-5753
	CMedString str = "";
	str << "CIpPartyCntl::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate - (EVideoResolutionType)maxResolution " << (int)maxResolution << " videoBitRate " << videoBitRate ;
	PTRACE(eLevelInfoNormal,str.GetString());

	BOOL bResAccordingToVidQuality = GetIsNeedToChangeTipResAccordingToConfVidQuality(pConfParty);

	if( bResAccordingToVidQuality && maxResolution <= eHD720_Res )
	{
		PTRACE(eLevelInfoNormal, "CIpPartyCntl::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate,  TIP call, force eHD720_Res");
		return eHD720Symmetric;
	}
	else
	{
		if (videoBitRate >= 30000)
		{
			PTRACE(eLevelInfoNormal, "CIpPartyCntl::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate, set eHD1080Symmetric");
			return eHD1080Symmetric;// when we'll support 1080 for TIP we need to set it to: eHD1080Symmetric (or maybe even eHD1080At60Asymmetric)
		}

		else if (videoBitRate >= 936 && videoBitRate < 30000)
		{
			PTRACE(eLevelInfoNormal, "CIpPartyCntl::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate, set eHD720Symmetric");
			return eHD720Symmetric;
		}

		PASSERTMSG(1,"CIpPartyCntl::GetTipResolutionTypeAccordingToMaxResolutionAndVidRate - TIP video rate is lower than 936 kbps!!!");
	}

	return eInvalidModeType;
}

////////////////////////////////////////////////////////////////////////////
Eh264VideoModeType CIpPartyCntl::GetTipResolutionTypeAccordingToVideoRate(DWORD videoBitRate)
{
	if (videoBitRate >= 30000)
    	return eHD1080Symmetric;// when we'll support 1080 for TIP we need to set it to: eHD1080Symmetric (or maybe even eHD1080At60Asymmetric)

    else if (videoBitRate >= 936 && videoBitRate < 30000)
    	return eHD720Symmetric;

	PASSERTMSG(1,"CIpPartyCntl::GetTipResolutionTypeAccordingToVideoRate - TIP video rate is lower than 936 kbps!!!");

	return eInvalidModeType;
}

////////////////////////////////////////////////////////////////////////////
EVideoResolutionType CIpPartyCntl::GetMaxResolutionAccordingToVideoModeType(Eh264VideoModeType videoModeType)
{
	if (eHD720Symmetric == videoModeType)
    	return eHD720_Res;

    else if (eHD1080Symmetric == videoModeType)
    	return eHD1080_Res;

	PASSERTMSG(1,"CIpPartyCntl::GetMaxResolutionAccordingToVideoModeType - wrong video mode!!!");

	return eAuto_Res;
}

////////////////////////////////////////////////////////////////////////////
eVideoPartyType CIpPartyCntl::GetRelayResourceLevel(bool isVsw, CIpComMode* apScm) const
{
	if ((CProcessBase::GetProcess()->GetProductType()==eProductTypeSoftMCUMfw) && IsRemoteCapNotHaveVideo() && (m_eRemoteVendorIdent == IbmSametime9_Q42015_AndLater))
				return eVoice_relay_party_type;

	eVideoPartyType localVideoPartyType = eVideo_relay_CIF_party_type;

    RelayResourceLevelUsage rsrcLevel;
	if (apScm)
	{
	    const std::list <StreamDesc> videoRxStreamsDescList = apScm->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
        const std::list <StreamDesc> videoTxStreamsDescList = apScm->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);
	    CVideoOperationPointsSet* videoOperPointSet = apScm->GetOperationPoints();
	    PASSERT_AND_RETURN_VALUE(!videoOperPointSet, localVideoPartyType);

        if (apScm->IsMediaOff(cmCapVideo, cmCapReceive, kRolePeople))
        {
            TRACEINTO << "Video is off. This is audio only.";
            return eVoice_relay_party_type;
        }

        if (!videoRxStreamsDescList.size() && !videoTxStreamsDescList.size())
        {
            TRACEINTO << "No video streams. This is audio only.";
            return eVoice_relay_party_type;
        }

        RelayResourceLevelUsage rsrcLevelTx, rsrcLevelRx;
        rsrcLevelTx = videoOperPointSet->GetRelayResourceLevelUsage(videoTxStreamsDescList);
        rsrcLevelRx = videoOperPointSet->GetRelayResourceLevelUsage(videoRxStreamsDescList);
        if (rsrcLevelTx > rsrcLevelRx)
            rsrcLevel = rsrcLevelTx;
        else
            rsrcLevel = rsrcLevelRx;
	}
	else
	{
	CVideoOperationPointsSet* videoOperPointSet = m_pConf ? m_pConf->GetVideoOperationPointsSet() : NULL;
	PASSERT_AND_RETURN_VALUE(!videoOperPointSet, localVideoPartyType);

        rsrcLevel = videoOperPointSet->GetRelayResourceLevelUsage();
	}

	switch (rsrcLevel)
	{
		case eResourceLevel_CIF:
			localVideoPartyType = (isVsw ? eVSW_relay_CIF_party_type : eVideo_relay_CIF_party_type);
			break;
		case eResourceLevel_SD:
			localVideoPartyType = (isVsw ? eVSW_relay_SD_party_type : eVideo_relay_SD_party_type);
			break;
		case eResourceLevel_HD720:
			localVideoPartyType = (isVsw ? eVSW_relay_HD720_party_type : eVideo_relay_HD720_party_type);
			break;
		case eResourceLevel_HD1080:
			localVideoPartyType = (isVsw ? eVSW_relay_HD1080_party_type : eVideo_relay_HD1080_party_type);
			break;
	}
	TRACEINTO << "localVideoPartyType="  << eVideoPartyTypeNames[localVideoPartyType];
	return localVideoPartyType;
}

////////////////////////////////////////////////////////////////////////////
BOOL CIpPartyCntl::FixVideoBitRateIfNeeded(CConfParty* pConfParty, CIpComMode* pPartyScm , CIpNetSetup* pNetSetup,  BOOL initParty, eVideoPartyType videoPartyType)
{

	BOOL rateChanged = FALSE;
	PASSERTMSG_AND_RETURN_VALUE(!pConfParty || !pPartyScm , "!pConfParty || !pPartyScm", FALSE);

    // do not change bit rate for AVC VSW
    if (!m_bIsMrcCall && pPartyScm->GetConfMediaType() == eMixAvcSvcVsw)
        return rateChanged;

	DWORD  videoBitRate 		= pPartyScm->GetVideoBitRate(cmCapReceive, kRolePeople);

	if(eVideo_party_type_dummy == videoPartyType)
	{
		videoPartyType = max(pPartyScm->GetVideoPartyType(cmCapTransmit), pPartyScm->GetVideoPartyType(cmCapReceive));
	}
	CMedString str = "";
	str << "CIpPartyCntl::FixVideoBitRateIfNeeded - currentVideoPartyType " <<  VideoPartyTypeToString(videoPartyType) << " videoBitRate " << videoBitRate;
	PTRACE(eLevelInfoNormal,str.GetString());

	EConfType confType = pPartyScm->GetConfType();

	DWORD changedVidRate = 0;

	if( kCp == confType )
	{
		changedVidRate = ::ChangeVideoRateAccordingToPartyTypeIfNeeded( (eVideoPartyType)videoPartyType, videoBitRate);
	}

	if(0 != changedVidRate && videoBitRate != changedVidRate)   
	{
		pPartyScm->SetVideoBitRate(changedVidRate, cmCapReceiveAndTransmit);
		pPartyScm->SetTotalVideoRate(changedVidRate);
		pPartyScm->SetCallRate(changedVidRate/10);
		//pConfParty->SetVideoRate(changedVidRate/10); BRIDGE-14654- pConfParty save data from DataBase. mustn't change values!!

		if(pNetSetup)
		{
			TRACEINTO << "pNetSetup->SetMaxRate: " << (int)(changedVidRate * 100);
			pNetSetup->SetMaxRate(changedVidRate * 100);
		}
		rateChanged = TRUE;
	}
	return rateChanged;
}

////////////////////////////////////////////////////////////////////////////
int CIpPartyCntl::IsNeedToCloseInternalArt(eVideoPartyType newVideoPartyType) const
{
    // relevant only for mix mode
    if (m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType() != eMixAvcSvc)
        return false;

    // check how many ARTs are needed according to videoPartyType
    int numOfArtsNew = GetNumOfInternalArts(newVideoPartyType);
    int numOfArtsCurrent = GetNumOfInternalArts(m_eLastAllocatedVideoPartyType);
    TRACEINTO <<  "mix_mode: m_eLastReAllocRequestVideoPartyType = " << m_eLastAllocatedVideoPartyType << " newVideoPartyType = " << newVideoPartyType;
    TRACEINTO <<  "mix_mode: numOfArtsCurrent = " << numOfArtsCurrent << " numOfArtsNew = " << numOfArtsNew;

    if (numOfArtsNew < numOfArtsCurrent)
        return (numOfArtsCurrent-numOfArtsNew);

    return 0;
}

////////////////////////////////////////////////////////////////////////////
int CIpPartyCntl::GetNumOfInternalArts(eVideoPartyType aVideoPartyType) const
{
    // relevant only for mix mode
    if (m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType() != eMixAvcSvc)
        return 0;

    if (eVideo_party_type_none != aVideoPartyType || eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
    {
        if (aVideoPartyType > eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
        {// allocate two video encoders & ARTs when video resources > HD720
            return 2;
        }
        else
        {
            return 1;
        }
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::Deescalate(int numOfArtsToClose)
{
    // relevant only for mix mode
    if (m_pIpCurrentMode->GetConfMediaType() != eMixAvcSvc)
        return;

    if (!m_pPartyApi)
        return;

    TRACEINTO << "numOfArtsToClose=" << numOfArtsToClose;

    // remove the relevant streams
    std::list <StreamDesc> videoStreams = m_pIpCurrentMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
    if (videoStreams.size() && videoStreams.back().m_isAvcToSvcVsw)
    {// remove the VSW stream if it exists
        videoStreams.pop_back();
    }

    for (int i = 0; i < numOfArtsToClose && videoStreams.size(); ++i)
    {
        videoStreams.pop_back();
    }

    m_pIpInitialMode->SetStreamsListForMediaMode(videoStreams, cmCapVideo, cmCapReceive, kRolePeople);
    m_pIpCurrentMode->SetStreamsListForMediaMode(videoStreams, cmCapVideo, cmCapReceive, kRolePeople);

    m_pIpCurrentMode->Dump("IpPartyCntl::Deescalate   m_pIpCurrentMode");
    m_pIpInitialMode->Dump("IpPartyCntl::Deescalate   m_pIpInitialMode");

    m_pPartyApi->DisconnectAvcToSvcArtTranslator(m_pIpInitialMode);
}
////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdateStreamsListOfCurMode()
{
    // update streams in the current mode
    m_pIpCurrentMode->SetStreamsListForMediaMode(m_pIpInitialMode->GetStreamsListForMediaMode(cmCapAudio, cmCapReceive, kRolePeople), cmCapAudio, cmCapReceive, kRolePeople);
    m_pIpCurrentMode->SetStreamsListForMediaMode(m_pIpInitialMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople), cmCapVideo, cmCapReceive, kRolePeople);
}
///////////////////////////////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::GetFirs2013Ssrc(BYTE index)
{
        return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::CheckResourceAllocationForMixMode()
{
    if( m_pIpInitialMode->GetConfMediaType() != eMixAvcSvc)
        return TRUE;

    CRsrcParams additionalRsrcParam;
    MixedLogicalResourceInfo addtionalRmxAvcPartylogicalTypes[]={
          {eLogical_relay_avc_to_svc_rtp_with_audio_encoder,true,true},
          {eLogical_relay_avc_to_svc_rtp,false,false},
          {eLogical_relay_rtp,true,true},
          {eLogical_relay_avc_to_svc_video_encoder_1,true,false},
          {eLogical_relay_avc_to_svc_video_encoder_2,false,false},
          {eLogical_legacy_to_SAC_audio_encoder,true,true} /* true for RMX */
    };

    MixedLogicalResourceInfo addtionalSoftMcuAvcPartylogicalTypes[]={
          {eLogical_relay_avc_to_svc_rtp_with_audio_encoder,true,false},
          {eLogical_relay_avc_to_svc_rtp,false,false},
          {eLogical_relay_rtp,true,false},
          {eLogical_relay_avc_to_svc_video_encoder_1,true,false},
          {eLogical_relay_avc_to_svc_video_encoder_2,false,false}

    };

    MixedLogicalResourceInfo addtionalSvcPartylogicalTypes[]={
        {eLogical_video_decoder,true,false}
    };

    MixedLogicalResourceInfo* logicalTypeInfo;
    int max;
    CRsrcDesc tmpRsrcDesc;
    WORD found;
    WORD itemNum;
    bool mandatory = false;
    DWORD status = STATUS_OK;
    eVideoPartyType allocatedVideoPartyType = m_pPartyAllocatedRsrc->GetVideoPartyType();

    if (m_bIsMrcCall)
    {
        logicalTypeInfo = addtionalSvcPartylogicalTypes;
        max = sizeof(addtionalSvcPartylogicalTypes)/sizeof(MixedLogicalResourceInfo);
    }
    else
    {
        if (IsSoftMcu())
        {
            logicalTypeInfo = addtionalSoftMcuAvcPartylogicalTypes;
            max = sizeof(addtionalSoftMcuAvcPartylogicalTypes)/sizeof(MixedLogicalResourceInfo);
        }
        else
        {
            logicalTypeInfo = addtionalRmxAvcPartylogicalTypes;
            max = sizeof(addtionalRmxAvcPartylogicalTypes)/sizeof(MixedLogicalResourceInfo);
        }
    }
    for(int i=0; i<max; ++i)
    {
        itemNum=1;
        found = m_pPartyAllocatedRsrc->GetRsrcParams(additionalRsrcParam,logicalTypeInfo[i].lrt,itemNum);
        if(allocatedVideoPartyType==eVideo_party_type_none || allocatedVideoPartyType== eVSW_Content_for_CCS_Plugin_party_type
          ||allocatedVideoPartyType== eVoice_relay_party_type)
        {
            mandatory=logicalTypeInfo[i].mandatoryAud;
        }
        else
        {
            mandatory=logicalTypeInfo[i].mandatoryVid;
        }


        if(!found && mandatory==true)
        {
            TRACEINTO<<"!@# dynMixedErr reallocation for dynamic mixed mode failed missing lrt: "<<logicalTypeInfo[i].lrt;  // ey_20866
            PASSERT(101);
            status=STATUS_FAIL;
        }
    }

    if( m_pMrmpRsrcParams == NULL)
    {
        m_pMrmpRsrcParams = new CRsrcParams;
    }
    *m_pMrmpRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_relay_rtp); // eyaln

    if ( m_pMrmpRsrcParams->GetRsrcDesc())
    {
        if (m_pMrmpRsrcParams->GetRsrcDesc()->GetConnectionId() == 0)
        {
            TRACEINTO<<" dynMixedErr eLogical_relay_rtp wasn't allocated1";
            POBJDELETE( m_pMrmpRsrcParams);
            m_pMrmpRsrcParams=NULL;
            if (!m_bIsMrcCall && !IsSoftMcu())
            {// mandatory only if translator art is allocated - should improve the check
                if (!IsSoftMcu())
                {// mandatory only if translator art is allocated - should improve the check
                    PASSERT(101);
                    status=STATUS_FAIL;
                }
            }
        }
        else
        {
            CRsrcDesc* tmp;
            tmp= m_pMrmpRsrcParams->GetRsrcDesc();
            TRACEINTO<<"!@# mrmp ConnectionId:"<<tmp->GetConnectionId()<<" LogicalRsrcType:"<<tmp->GetLogicalRsrcType();
        }
    }
    else
    {
        TRACEINTO<<" dynMixedErr  eLogical_relay_rtp wasn't allocated2";
        PASSERT(101);
        status=STATUS_FAIL;
        //          SystemCoreDump(FALSE);
    }

    if(!m_bIsMrcCall)
    {
        for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
        {
            WORD itemNum=1;
            WORD found;
            if(m_avcToSvcTranslatorRsrcParams[i]==NULL)
            {
                m_avcToSvcTranslatorRsrcParams[i]=new CRsrcParams;
            }
            found=m_pPartyAllocatedRsrc->GetRsrcParams(*m_avcToSvcTranslatorRsrcParams[i],logicalTypeInfo[i].lrt,itemNum);

            if(found)
            {
                TRACEINTO<<"!@# good:  translator:"<<i<<" will be allocated";
                CRsrcDesc* tmp;
                tmp=m_avcToSvcTranslatorRsrcParams[i]->GetRsrcDesc();
                TRACEINTO<<"!@# m_avcToSvcTranslatorRsrcParams["<<i<<"] ConnectionId:"<<tmp->GetConnectionId()<<" LogicalRsrcType:"<<tmp->GetLogicalRsrcType();
            }
            else
            {
                POBJDELETE(m_avcToSvcTranslatorRsrcParams[i]);
                m_avcToSvcTranslatorRsrcParams[i]=NULL;

                if(allocatedVideoPartyType==eVideo_party_type_none || allocatedVideoPartyType== eVSW_Content_for_CCS_Plugin_party_type)
                {
                    mandatory=logicalTypeInfo[i].mandatoryAud;
                }
                else
                {
                    mandatory=logicalTypeInfo[i].mandatoryVid;
                }

                if(mandatory==true)
                {
                    TRACEINTO<<"!@# dynMixedErr :  translator:"<<i<<" will not be allocated"; // ey_20866
                    PASSERT(101);
                    status=STATUS_FAIL;
                }
            }
        }
    }
    if(status!=STATUS_OK)
    {
        CSegment *pSeg = new CSegment;
        *pSeg << m_name;
        m_pTaskApi->UpdateDB(NULL,DISCAUSE,RESOURCES_DEFICIENCY,1,pSeg); // Disconnnect cause
        m_pTaskApi->EndAddParty(m_pParty,statIllegal);
        m_pPartyAllocatedRsrc->DumpToTrace();
        POBJDELETE(pSeg);
        return FALSE;
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////
DWORD CIpPartyCntl::GetPrIdLync2013()
{
	DWORD prID = 0;

	if(m_pConf && m_pConf->GetCommConf())
	{
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_name);

		if (pConfParty)
		{
			EVideoResolutionType vidResolutionType = eAuto_Res;
			//TRACEINTO << "GetAvMcuLinkType() = " << (DWORD)GetAvMcuLinkType();

			if(GetAvMcuLinkType() == eAvMcuLinkMain)
			{
				vidResolutionType = convertVideoTypeToResType(m_eLastAllocatedVideoPartyType) ;
				//TRACEINTO << "vidResolutionType = " << (DWORD)vidResolutionType;
				prID = GetPrIdByResolutionType(vidResolutionType);
			} else if(GetAvMcuLinkType() == eAvMcuLinkSlaveOut || GetAvMcuLinkType() == eAvMcuLinkSlaveIn)
			{
				vidResolutionType = (EVideoResolutionType)pConfParty->GetMaxResolution();
				//TRACEINTO << "vidResolutionType = " << (DWORD)vidResolutionType;
				prID = GetPrIdByResolutionType(vidResolutionType);
			}
		}
	}
	return prID;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void CIpPartyCntl::UpdateResourceTableAfterRealloc(CPartyRsrcDesc* apPartyAllocatedRsrc)
{

    eVideoPartyType requestedVideoPartyType = m_eLastReAllocRequestVideoPartyType;
    eVideoPartyType allocatedVideoPartyType = apPartyAllocatedRsrc->GetVideoPartyType();

    if(requestedVideoPartyType < allocatedVideoPartyType)
        return;

    TRACEINTO << "requestedVideoPartyType=" << requestedVideoPartyType << " allocatedVideoPartyType=" << allocatedVideoPartyType;

    if (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc)
    {
        TRACEINTO << "Handle Mix mode resources - START";
        ::GetpConfPartyRoutingTable()->DumpTable();

        m_pPartyAllocatedRsrc->DeleteAllRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_video_encoder_1);
        apPartyAllocatedRsrc->AddAllRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_video_encoder_1);

        m_pPartyAllocatedRsrc->DeleteAllRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_video_encoder_2);
        apPartyAllocatedRsrc->AddAllRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_video_encoder_2);

        // sync eLogical_relay_avc_to_svc_rtp_with_audio_encoder
        CRsrcDesc newRsrcDesc, oldRsrcDesc;
        WORD existsInNewAllocation = apPartyAllocatedRsrc->GetRsrcDesc(newRsrcDesc, eLogical_relay_avc_to_svc_rtp_with_audio_encoder, (WORD)1);
        WORD existsInOldAllocation = m_pPartyAllocatedRsrc->GetRsrcDesc(oldRsrcDesc, eLogical_relay_avc_to_svc_rtp_with_audio_encoder, (WORD)1);
        if (existsInNewAllocation && !existsInOldAllocation)
        {
            TRACEINTO << "Adding eLogical_relay_avc_to_svc_rtp_with_audio_encoder connId=" << newRsrcDesc.GetConnectionId();
            CRsrcDesc* pTempDesc = new CRsrcDesc(newRsrcDesc);
            if(pTempDesc)
            {
            	m_pPartyAllocatedRsrc->AddNewRsrcDesc(pTempDesc);
            	POBJDELETE(pTempDesc);
            }
        }
        else if (!existsInNewAllocation && existsInOldAllocation)
        {
            m_pPartyAllocatedRsrc->DeleteRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
        }

        // sync eLogical_relay_avc_to_svc_rtp
        existsInNewAllocation = apPartyAllocatedRsrc->GetRsrcDesc(newRsrcDesc, eLogical_relay_avc_to_svc_rtp, (WORD)1);
        existsInOldAllocation = m_pPartyAllocatedRsrc->GetRsrcDesc(oldRsrcDesc, eLogical_relay_avc_to_svc_rtp, (WORD)1);
        if (existsInNewAllocation && !existsInOldAllocation)
        {
            TRACEINTO << "Adding eLogical_relay_avc_to_svc_rtp connId=" << newRsrcDesc.GetConnectionId();
            CRsrcDesc* pTempDesc = new CRsrcDesc(newRsrcDesc);
            if(pTempDesc)
            {
            	m_pPartyAllocatedRsrc->AddNewRsrcDesc(pTempDesc);
            	POBJDELETE(pTempDesc);
            }
        }
        else if (!existsInNewAllocation && existsInOldAllocation)
        {
            m_pPartyAllocatedRsrc->DeleteRsrcDescAccordingToLogicalResourceType(eLogical_relay_avc_to_svc_rtp);
        }

        TRACEINTO << "Handle Mix mode resources - END";
        ::GetpConfPartyRoutingTable()->DumpTable();
    }

    if (allocatedVideoPartyType != eVideo_party_type_none && m_eLastAllocatedVideoPartyType == eVideo_party_type_none)   //forDPA A to V in SIP
    {
        TRACEINTO << "REALLOCATION A to V new resources!!!";

        CRsrcDesc Decoder= apPartyAllocatedRsrc->GetRsrcDesc(eLogical_video_decoder);
        CRsrcDesc Encoder= apPartyAllocatedRsrc->GetRsrcDesc(eLogical_video_encoder);
        CRsrcDesc* pTempDec = new CRsrcDesc(Decoder);
        CRsrcDesc* pTempEnc = new CRsrcDesc(Encoder);
        m_pPartyAllocatedRsrc->DeleteRsrcDescAccordingToLogicalResourceType(eLogical_video_decoder);
        m_pPartyAllocatedRsrc->DeleteRsrcDescAccordingToLogicalResourceType(eLogical_video_encoder);
        if(pTempDec)
        {
        	m_pPartyAllocatedRsrc->AddNewRsrcDesc(pTempDec);
        	POBJDELETE(pTempDec);
        }
        if(pTempEnc)
        {
        	  m_pPartyAllocatedRsrc->AddNewRsrcDesc(pTempEnc);
        	  POBJDELETE(pTempEnc);

        }
    }
}

//BRIDGE-13154
BOOL CIpPartyCntl::GetBOOLDataByKey(const std::string& key)
{
	BOOL bRetVal = FALSE;

	CSysConfig* sysConfig = NULL;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();
	else
		PASSERTMSG(!pProcess,"!pProcess");

	if(sysConfig)
		sysConfig->GetBOOLDataByKey(key, bRetVal);
	else
		PASSERTMSG(!sysConfig,"!sysConfig");

	return bRetVal;
}
