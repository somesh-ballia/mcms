//+========================================================================+
//                            H323PartyControl.CPP                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323PartyControl.CPP                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Sami                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Sami| 7/4/95     | This file contains four class implementation for     |
//     |            | party control in the conference layer i.e. control   |
//     |            | of party layer that implements net and mux control,  |
//     |            | and multipont control i.e. bridges connection and    |
//     |            | disconnections.The four classes are :                |
//     |            | CPartyCntl - This class is used as a base class for  |
//     |            |   all the other classes in is used to control party  |
//     |            |   activities while party is in steady state.         |
//     |            | CAddPartyCntl - This class is used to connect the    |
//     |            |   party to the conference.Its responsiblity to       |
//     |            |   establish h221 connection at full bitrate as       |
//     |            |   defined at the conference initial scm, and to      |
//     |            |   connect the party to the audio controller.         |
//     |            | CChangeModeCntl - This class is used to mantain      |
//     |            |   change mode requestes. Change mode requestes may   |
//     |            |   involve h221 change mode procedure and connection  |
//     |            |   and disconnection from bridge controllers.         |
//     |            |   The class has a role in case of party deletion by  |
//     |            |   "zero" mode change.                                |
//     |            | CDelPartyCntl - This class is used to remove a party |
//     |            |   from the conference.Its responsibilty to disconnect|
//     |            |   the party form the audio controller and from all   |
//     |            |   other controlllers if connected and to disconnect  |
//     |            |   the h221 party controller i.e. mux and net.        |
//     |            |                                                      |
//+========================================================================+
#include "H323PartyControl.h"
#include "ConfPartyOpcodes.h"
#include "PartyApi.h"
#include "H323Caps.h"
#include "CommModeInfo.h"
#include "ConfPartyGlobals.h"
#include "TraceStream.h"

#include "BridgePartyAudioParams.h"
#include "BridgePartyInitParams.h"
#include "FECCBridgePartyInitParams.h"
#include "ContentBridgePartyInitParams.h"
#include "AudioBridgeInterface.h"
#include "FECCBridge.h"
#include "ContentBridge.h"
#include "BridgeMoveParams.h"
#include "Trace.h"
#include "IpCommon.h"
#include  "H263VideoMode.h"
#include "VideoBridgeInterface.h"

/////////////////////////////////////////////////////////////////////////////
//                        CH323PartyCntl
/////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CH323PartyCntl)
  ONEVENT(PARTY_VIDEO_DISCONNECTED   		,ANYCASE	 ,CH323PartyCntl::OnVidBrdgDisconnect)
  ONEVENT(PARTY_AUDIO_DISCONNECTED			,ANYCASE	 ,CH323PartyCntl::OnAudBrdgDisconnect)
  ONEVENT(PARTY_RECEIVE_ECS     			,ANYCASE	 ,CH323PartyCntl::OnPartyReceivedECS)
  ONEVENT(REMOTE_SENT_RE_CAPS   			,ANYCASE	 ,CH323PartyCntl::OnPartyReceivedReCapsAnycase)
  ONEVENT(PARTY_FAULTY_RSRC     			,ANYCASE	 ,CH323PartyCntl::OnPartyReceivedFaultyRsrc)
  ONEVENT(AUDIOMUTE							,ANYCASE	 ,CH323PartyCntl::OnPartyMuteAudioAnycase)
  ONEVENT(PRESENTATION_OUT_STREAM_UPDATED	,ANYCASE	 ,CH323PartyCntl::OnPartyPresentationOutStreamUpdateAnycase)
  ONEVENT(PARTY_CLOSE_CHANNEL				,ANYCASE	 ,CH323PartyCntl::OnPartyChannelDisconnect)
  ONEVENT(H323PARTYCONNECT   				,ANYCASE	 ,CH323PartyCntl::OnPartyH323Connect)
  ONEVENT(UPDATE_VIDEO_RATE  				,ANYCASE	 ,CH323PartyCntl::OnUpdatePartyH323VideoBitRate)
  ONEVENT(CLEAN_VIDEO_RATE_LIMIT			,ANYCASE	 ,CH323PartyCntl::OnCleanVideoRateLimitation)
  // LPR
  ONEVENT(LPR_CHANGE_RATE  					,ANYCASE	 ,CH323PartyCntl::OnUpdatePartyH323LprVideoBitRate)
  ONEVENT(PARTY_LPR_VIDEO_OUT_RATE_UPDATED	,ANYCASE	 ,CH323PartyCntl::OnLprVideoOutBrdgBitRateUpdated)
  ONEVENT(CHANGE_CONTENT_BIT_RATE_BY_LPR    ,ANYCASE	 ,CH323PartyCntl::OnChangeContentBitRateByLprOrDba)// VNGFE-8204

  ONEVENT(REALLOCATE_PARTY_RSRC_IND,	CHANGE_ALL_MEDIA ,CH323PartyCntl::OnRsrcReAllocatePartyRspChangeAll)
  ONEVENT(REALLOCATE_PARTY_RSRC_IND,	REALLOCATE_RSC   ,CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate)
  ONEVENT(UPDATE_CAPS,		            ANYCASE          ,CH323PartyCntl::OnPartyUpdateLocalCaps)

  //Multiple links for ITP in cascaded conference feature:
  ONEVENT(ADDSUBLINKSPARTIES           ,ANYCASE          ,CH323PartyCntl::OnAddSubLinksParties) //error handling...
  ONEVENT(CONNECTLINKTOUT              ,ANYCASE          ,CH323PartyCntl::OnConnectLinkTout)    //error handling...
  ONEVENT(ITPSPEAKERIND                ,ANYCASE          ,CH323PartyCntl::OnMainPartyUpdateITPSpeaker)
  ONEVENT(ITPSPEAKERACKIND             ,ANYCASE          ,CH323PartyCntl::OnMainPartyUpdateITPSpeakerAck)
  ONEVENT(NEW_ITP_SPEAKER_IND          ,ANYCASE          ,CH323PartyCntl::OnMainPartySendNewITPSpeaker)

  ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA ,ANYCASE          ,CH323PartyCntl::OnPartyCntlFirRequestByEP)
  ONEVENT(IP_MUTE_MEDIA                ,ANYCASE          ,CPartyCntl::OnPartyChangeBridgesMuteState)

PEND_MESSAGE_MAP(CH323PartyCntl,CIpPartyCntl);

/////////////////////////////////////////////////////////////////////////////
CH323PartyCntl::CH323PartyCntl()
{
	m_pLocalCapH323						= NULL;
	m_pRmtCapH323						= NULL;
	m_bIsFlowControlParty				= 0;
	m_flowControlRate					= 0;
	m_DbaContentRate					= 0;

	m_bIsCascade						= CASCADE_NONE;
	m_bAdditionalRsrcActivated			= FALSE;
	m_bIsPartyConnectAllWhileMove		= FALSE;
//	m_lsdRequestRate					= 0;
	m_bConfWaitToEndChangeModeForFecc	= FALSE;
	m_bPartyEndChangeVideoInformParty 	= FALSE;
	m_pH323NetSetup						= NULL;
    m_masterSlaveStatus 				= CASCADE_MODE_MASTER;
    m_AddPartyStateBeforeMove 			= IDLE;
    m_bLateReleaseOfResources 			= FALSE;
    m_bContinueChangeModeAfterReAlloc 	= FALSE;
    m_bSuspendVideoUpdates 				= FALSE;
    m_bPendingRemoteCaps   				= FALSE;


    m_roomControl                       = NULL;
    m_linkType                          = eRegularParty;

	VALIDATEMESSAGEMAP;
}

/////////////////////////////////////////////////////////////////////////////
CH323PartyCntl::~CH323PartyCntl() // destructor
{
	POBJDELETE(m_pH323NetSetup);
	POBJDELETE(m_roomControl);
}


/////////////////////////////////////////////////////////////////////////////
CH323PartyCntl& CH323PartyCntl::operator=(const CH323PartyCntl& other)
{
	if(this != &other)
	{
		(CIpPartyCntl&)*this = (CIpPartyCntl&)other;

		m_pLocalCapH323					= other.m_pLocalCapH323;
		m_pRmtCapH323					= other.m_pRmtCapH323;
		m_bIsFlowControlParty			= other.m_bIsFlowControlParty;
		m_flowControlRate				= other.m_flowControlRate;
		m_DbaContentRate				= other.m_DbaContentRate;
		m_bIsCascade					= other.m_bIsCascade;
		m_bAdditionalRsrcActivated		= other.m_bAdditionalRsrcActivated;
        m_masterSlaveStatus             = other.m_masterSlaveStatus;
        m_bIsPartyConnectAllWhileMove	= other.m_bIsPartyConnectAllWhileMove;
        m_AddPartyStateBeforeMove		= other.m_AddPartyStateBeforeMove;
	//	m_lsdRequestRate				= other.m_lsdRequestRate;
		m_bConfWaitToEndChangeModeForFecc	= other.m_bConfWaitToEndChangeModeForFecc;
		m_bPartyEndChangeVideoInformParty	= other.m_bPartyEndChangeVideoInformParty;

		if (CPObject::IsValidPObjectPtr(m_pH323NetSetup))
			POBJDELETE(m_pH323NetSetup);

		m_pH323NetSetup					= new CH323NetSetup;
		m_pH323NetSetup->copy(other.m_pH323NetSetup);
        m_bLateReleaseOfResources 		= other.m_bLateReleaseOfResources;
        m_bContinueChangeModeAfterReAlloc = other.m_bContinueChangeModeAfterReAlloc;
        m_bSuspendVideoUpdates			= other.m_bSuspendVideoUpdates;
        m_bPendingRemoteCaps            = other.m_bPendingRemoteCaps;

        //Multiple links for ITP in cascaded conference feature: CH323PartyCntl::operator=
        if (CPObject::IsValidPObjectPtr(m_roomControl))
            POBJDELETE(m_roomControl);
        m_linkType                      = other.m_linkType;
        if (other.m_roomControl != NULL && m_linkType == eMainLinkParty)
        {
            PTRACE2(eLevelInfoNormal,"CH323PartyCntl::operator= ",m_name);
            m_roomControl = new CRoomCntl();
            *m_roomControl = *(other.m_roomControl);
        }

	}
	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::Destroy()
{
	POBJDELETE(m_pLocalCapH323);
	POBJDELETE(m_pRmtCapH323);
	CIpPartyCntl::Destroy();
}

/////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsRemoteCapNotHaveVideo() const
{
	WORD numOfVideoCaps = m_pRmtCapH323->GetNumOfVideoCap();
	BYTE bIsAudioOnly   = numOfVideoCaps == 0;
	return bIsAudioOnly;
}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsRemoteAndLocalHasHDContent1080() const
{
	BYTE HD1080Mpi = 0;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (m_pRmtCapH323->IsH239() &&   m_pLocalCapH323->IsH239())
		{
			HD1080Mpi = m_pRmtCapH323->IsCapableOfHDContent1080();
		}

	}
	return HD1080Mpi;
}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsRemoteAndLocalHasHDContent720() const
{
	BYTE HD720Mpi = 0;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (m_pRmtCapH323->IsH239() &&   m_pLocalCapH323->IsH239())
		{
			HD720Mpi = m_pRmtCapH323->IsCapableOfHDContent720();
		}

	}
	return HD720Mpi;
}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsRemoteAndLocalHasHighProfileContent() const
{
	BYTE bIsHighProfileContent = FALSE;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (m_pRmtCapH323->IsH239() &&   m_pLocalCapH323->IsH239())
		{
			bIsHighProfileContent = m_pRmtCapH323->IsH264HighProfileContent();
		}

	}
	return bIsHighProfileContent;
}

//////////////////////////////////////////////////////////////////////////
//Description: This function allocates buffer, which contains the party and the class details.
//NOTE: The function that calls to this function must deallocate this buffer!!
char* CH323PartyCntl::GetPartyAndClassDetails()
{
	const char* className = NameOf();
	DWORD msgLen = SmallPrintLen + strlen(m_partyConfName)+1 + strlen(className)+1;
	ALLOCBUFFER(strDetails,msgLen);
	sprintf(strDetails, "Name - %s, Class - %s", m_partyConfName,className);
	return strDetails;
}
//////////////////////////////////////////////////////////////////////////
/*
BOOL CH323PartyCntl::IsRedialImmediately() const
{
	BYTE res = NO;
	switch (m_disconnectionCause)
	{
		case H323_CALL_CLOSED_REMOTE_BUSY:
		case H323_CALL_CLOSED_REMOTE_UNREACHABLE:
		case H323_CALL_CLOSED_REMOTE_REJECT:
			res = YES;
	}
	return FALSE;
}
*/
/////////////////////////////////////////////////////////////////////////////
/*
BYTE CH323PartyCntl::IsRemoteCapSetHasEpcOrDuoCap(WORD dualVideoMode) const
{
	BYTE res = NO;
	if (m_pRmtCapH323)
	{
		if (dualVideoMode == eDualModeEnterprisePC)
		res = (m_pRmtCapH323->IsEPC())? YES: NO;
		else if (dualVideoMode == eDualModeIpDuoVideo)
			res = (m_pRmtCapH323->IsCapsSupportDuo())? YES: NO;
	}
	return res;
}*/
/////////////////////////////////////////////////////////////////////////////
/*BYTE CH323PartyCntl::IsRemoteCapSetHasContent()
{
	BYTE res = NO;

	if (m_pRmtCapH323)
	{
		res = m_pRmtCapH323->IsH239() ;
	}
	return res;
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsLegacyContentParty()
{
    CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
    PASSERTMSG_AND_RETURN_VALUE(!pCommConf , "pCommConf is NULL", false);

    BYTE isLegacy = !IsRemoteAndLocalCapSetHasContent();
	if (isLegacy)
	{
		PTRACE(eLevelInfoNormal,"CH323PartyCntl::IsLegacyContentParty : No Remote or Local Content Caps");
	}

	if(pCommConf)
	{
		if ((pCommConf->GetIsTipCompatible() == eTipCompatibleVideoAndContent || pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP) )
		{
			DWORD partyContentRate	= GetMinContentPartyRate()/100;
			isLegacy 				= IsPartyLegacyForTipContent(partyContentRate) ;
			TRACEINTO << "IsPartyLegacyForTipContent = " << isLegacy;
			return isLegacy ;
		}

		BYTE presentationProtocol 					= pCommConf->GetPresentationProtocol();
		BYTE doesPartyMeetConfContentHDResolution	= TRUE;
		BOOL bForceH264 							= 0;

		// Content protocol h264 only
		if(presentationProtocol == eH264Fix || presentationProtocol == eH264Dynamic)
		{
			ON(bForceH264);
		}

		if(presentationProtocol == eH264Fix && !isLegacy)  //HP content:
		{
			BYTE isHighProfileContent = pCommConf->GetIsHighProfileContent();
			if(isHighProfileContent)
			{
				isLegacy = !IsRemoteAndLocalHasHighProfileContent();
			}
			else      //BRIDGE-12079
			{
				isLegacy = !m_pRmtCapH323->IsH264BaseProfileContent();
			}
		}

		if(bForceH264 && !isLegacy)
		{
			BYTE isHD720content 		= IsRemoteAndLocalHasHDContent720();
			DWORD possibleContentRate	= GetMinContentPartyRate()/100;

			if(presentationProtocol == eH264Fix)
			{
				doesPartyMeetConfContentHDResolution = m_pConf->DoesPartyMeetConfContentHDResolution(m_name);
			}
			if(presentationProtocol == eH264Dynamic)
			{
				doesPartyMeetConfContentHDResolution = isHD720content;
			}

			DWORD 			actualPartyContentRate = 0;
			DWORD 			confContentRate		   = m_pLocalCapH323->GetMaxContentBitRate();
			BYTE 			lConfRate			   = pCommConf->GetConfTransferRate();
			eEnterpriseMode ContRatelevel		   = (eEnterpriseMode)pCommConf->GetEnterpriseMode();
			CUnifiedComMode localUnifedCommMode(pCommConf->GetEnterpriseModeFixedRate(), lConfRate,pCommConf->GetIsHighProfileContent());

			DWORD H323ConfIPContentRate = localUnifedCommMode.GetContentModeAMCInIPRate(lConfRate, ContRatelevel,
																						(ePresentationProtocol)presentationProtocol,
																						pCommConf->GetCascadeOptimizeResolution(),
																						pCommConf->GetConfMediaType());
			actualPartyContentRate  = min(confContentRate, possibleContentRate);

			DWORD contentThresholdRate 			 = 0; //since we are in an urgent FE, we use the is function instead of a get function
			BOOL isPartyMeetContentRateThreshold = ::isPartyMeetContentRateThreshold(H323ConfIPContentRate/10, actualPartyContentRate/10, pCommConf->GetEnterpriseMode(), pCommConf->GetPresentationProtocol(), contentThresholdRate);
			if( !doesPartyMeetConfContentHDResolution || !isPartyMeetContentRateThreshold)
			{
				CSmallString sstr;
				sstr << "doesPartyMeetConfContentHDResolution - " << doesPartyMeetConfContentHDResolution << ", isPartyMeetContentRateThreshold - " << isPartyMeetContentRateThreshold << " , possibleContentRate = " << possibleContentRate << ", confContentRate = " << confContentRate;
				PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::IsLegacyContentParty : in EP failed threshold in eH264Fix/eH264Dynamic, ",sstr.GetString(), GetPartyRsrcId());
				isLegacy = (!doesPartyMeetConfContentHDResolution)?eContentSecondaryCauseBelowResolution:eContentSecondaryCauseBelowRate;
			}
		}
	}

	//============================================================
	// VNGR-23965 - Lowering log strain, logging only for legacy
	//============================================================
	if (isLegacy)
	{
		CSmallString sstr1;
		sstr1 << isLegacy;
		PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::IsLegacyContentParty : isLegacy=",sstr1.GetString(), GetPartyRsrcId());
	}

	return isLegacy;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CH323PartyCntl::GetPossibleContentRate() const
{
	DWORD  ContentRate = 0;
	ContentRate = m_pRmtCapH323->GetMaxContentBitRate();
	//PTRACE2INT(eLevelInfoNormal,"CH323PartyCntl::GetPossibleContentRate : ContentRate=", ContentRate);
	return ContentRate;
}

/////////////////////////////////////////////////////////////////////////////
CapEnum CH323PartyCntl::GetContentProtocol(BYTE bTakeCurrent) const
{
	if(bTakeCurrent)
		return ((CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation));
	else
		return ((CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation));
}
/////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsOpenContentTxChannel(BYTE bTakeCurrent) const
{
	if(bTakeCurrent)
		return((CapEnum)m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit, kRoleContentOrPresentation));
	else
		return ((CapEnum)m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit, kRoleContentOrPresentation));

}

/////////////////////////////////////////////////////////////////////////////
DWORD CH323PartyCntl::GetMinConfPartyRate() const
{
	DWORD  callRateInOut = 0;
	DWORD  localVidRate = 0;
	DWORD  rmtVideoRate = 0;
	DWORD  minVidRate = 0;
	DWORD  preferedAudRate = 0;
	DWORD  AudPlusVid = 0;
	DWORD  ConfPartyRate = 0;
	DWORD  MinContentRate = 0;
	CLargeString logInfo;

	//	1. Calculate min conf rate

	switch (m_type)
	{
		case DIALIN :
		{
            DWORD isRegard = GetSystemCfgFlagInt<DWORD>(CFG_KEY_CP_REGARD_TO_INCOMING_SETUP_RATE);
            if (isRegard)
            {
                callRateInOut = m_pH323NetSetup->GetRemoteSetupRate();
                logInfo << "In, isRegard callRateInOut, ";
            }
            if(callRateInOut == 0)
            {
                callRateInOut = GetConfRate();
                logInfo << "In, callRateInOut is zero, Take Conf Rate, ";
            }
			break;
		}

	    case DIALOUT:
		{
			callRateInOut = m_pH323NetSetup->GetMaxRate();
            logInfo << "Out, Take Max Setup Rate, ";
			break;
		}
		default :
		{
			PASSERT(m_type);
			return 0;
		}
	}

    logInfo << ", callRateInOut - " << callRateInOut;

	//Anna VNGR-22564
	minVidRate = max(m_pIpCurrentMode->GetMediaBitRate(cmCapVideo,cmCapTransmit), m_pRmtCapH323->GetFirstMaxVideoBitRate());
	minVidRate = min(minVidRate,m_pLocalCapH323->GetFirstMaxVideoBitRate());

	//rmtVideoRate = m_pRmtCapH323->GetFirstMaxVideoBitRate();
	//localVidRate = m_pLocalCapH323->GetFirstMaxVideoBitRate();
	//minVidRate = min(rmtVideoRate, localVidRate);

    if (m_pIpInitialMode->GetConfType() == kCop)
    {
        DWORD currentVideoBitRate = minVidRate;
        // in COP we select according to the actual bit rate of the transmit
        CVidModeH323 * pCurrVideoTx = &(CVidModeH323&)(m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapTransmit));
        if (IsValidPObjectPtr(pCurrVideoTx))
            currentVideoBitRate = pCurrVideoTx->GetBitRate();
        minVidRate = min(minVidRate, currentVideoBitRate);
        CSmallString str;
        str << "CH323PartyCntl::GetMinConfPartyRate rmtVideoRate=" << rmtVideoRate
            << " localVidRate=" << localVidRate
            << " currentVideoBitRate = " << currentVideoBitRate
            << " minVidRate = " << minVidRate;
        PTRACE(eLevelInfoNormal, str.GetString());
    }


	preferedAudRate = (m_pLocalCapH323->GetPreferedAudioRateAccordingToVideo(minVidRate*100))/100;
	AudPlusVid = (minVidRate + preferedAudRate)*100;// possible audio plus video

	// VNGFE-8204, VNGFE-8205
	// in cascade we do NOT change the content rate after LPR indication in any way - according to VNGFE-8204 implementation
	// therefore the code below will only be activated in non cascade conference.
	// (if it is activated in cascade conference the Minimum  Content rate might be below content rate threshold).
	if(m_pIpCurrentMode->GetTotalVideoRate() && m_isLprActive) // in case of LPR audio plus video can be limited by the LPR request from the EP.
	{
		CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
		PASSERT_AND_RETURN_VALUE(!pCommConf, 0);
		if(pCommConf->GetCurrentConfCascadeMode() == CASCADE_MODE_NONE)
		{
			logInfo << "VNGFE-8204: minVidRate - " << minVidRate << "(m_pIpCurrentMode->GetTotalVideoRate() - " << m_pIpCurrentMode->GetTotalVideoRate() << ")\n";
			logInfo << "VNGFE-8204: AudPlusVid - " << AudPlusVid << "preferedAudRate - " << preferedAudRate << "\n";

			AudPlusVid = min(AudPlusVid, (m_pIpCurrentMode->GetTotalVideoRate() + preferedAudRate)*100);// multiple by 100 to convert values from 100 bit/sec to bit/sec
		}
		else
			logInfo << "VNGFE-8204 Cascade Conf. Content rate is not reduced in LPR Indication.";
	}

	ConfPartyRate = min(callRateInOut, AudPlusVid);
	logInfo << "callRateInOut - " << callRateInOut << ", AudPlusVid - " << AudPlusVid << ", ConfPartyRate - " << ConfPartyRate;
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::GetMinConfPartyRate - \n", logInfo.GetString(), GetPartyRsrcId());

    return ConfPartyRate;
	//	2.Calculate presentation rate from remote capabilities
	//DWORD MaxContentCapRate = GetPossibleContentRate()*100;
	//MinContentRate = min(ConfPartyRate,MaxContentCapRate);
	//return MinContentRate;
}

/////////////////////////////////////////
/*
 function name:CH323PartyCntl::GetMinContentPartyRateAMC
 this functions returns the minimum content rate between the party allowed content rate
 resulting from call rate and the remote content rate
 the return value is in AMC units.
 */
//////////////////////////////////////////////////////////////////////////
DWORD CH323PartyCntl::GetMinContentPartyRate(DWORD currContentRate)
{
	DWORD  ConfPartyRate = 0;
	DWORD MinContentRate = 0;
	ConfPartyRate = GetMinConfPartyRate();//returns call rate*100

	//compare to remote caps max rate
	//	2.Calculate presentation rate from remote capabilities
	DWORD MaxContentCapRate = GetPossibleContentRate()*100;//return remote content rate

		// 3. Find the minimum
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);
	CUnifiedComMode unifiedComMode(pCommConf->GetEnterpriseModeFixedRate(),pCommConf->GetConfTransferRate(),pCommConf->GetIsHighProfileContent());
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)pCommConf->GetEnterpriseMode();
	BYTE ConfRateAMC = CUnifiedComMode::TranslateIPRateToXferRate(ConfPartyRate);

	ePresentationProtocol presentaionPrtocol =  (ePresentationProtocol)pCommConf->GetPresentationProtocol();

	if(pCommConf->GetIsTipCompatibleContent())
	{
		TRACEINTO << "Tip content enabled  - for content rate calculation use dynamic h264 rate tables";
		presentaionPrtocol = eH264Dynamic;
	}

	BYTE  MaxContentRateAccordingtoPartyeRateAMC 	 = unifiedComMode.FindAMCContentRateByLevel(ConfRateAMC, pCommConf->GetConfMediaType(), ContRatelevel, presentaionPrtocol, pCommConf->GetCascadeOptimizeResolution());
	DWORD MaxContentRateAccordingToPartyRateInIpRate = unifiedComMode.TranslateAMCRateIPRate(MaxContentRateAccordingtoPartyeRateAMC);

	MinContentRate = min((MaxContentRateAccordingToPartyRateInIpRate*100), MaxContentCapRate);
	CLargeString logInfo;
	logInfo << "MinContentRate = min(MaxContentRateAccordingToPartyRateInIpRate:" << MaxContentRateAccordingToPartyRateInIpRate*100 << ", MaxContentCapRate:" << MaxContentCapRate << ")" << "\n";

	// VNGFE-8204
	DWORD newContentRate = 0;
	if(m_isLprActive || m_isContentDba)
	{
		logInfo << "VNGFE-8204 m_isLprActive - " << (DWORD)m_isLprActive << ", m_isContentDba - " << (DWORD)m_isContentDba << "\n";
		logInfo << "DbaContentRate - "<< m_DbaContentRate << ", current content rate - " << m_pIpCurrentMode->GetContentBitRate(cmCapTransmit) << "\n";
		newContentRate = max(m_DbaContentRate * 100, m_pIpCurrentMode->GetContentBitRate(cmCapTransmit) * 100);
	}

	if (newContentRate)
	{
		logInfo << "VNGFE-8204 MinContentRate:" << MinContentRate << ", newContentRate:" << newContentRate << "\n";
		MinContentRate = min(MinContentRate, newContentRate);
	}

	//===========================================================================
	// VNGR-23965 - Lowering log strain, logging only for when min rate changes
	//===========================================================================
	if (m_lastMinContentRate != MinContentRate)
	{
		logInfo << "Last Content rate - " << m_lastMinContentRate << "\n";
		m_lastMinContentRate = MinContentRate;
	}

	logInfo << "Min Content rate - " << MinContentRate;

	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::GetMinContentPartyRate - \n", logInfo.GetString(), GetPartyRsrcId());
	return MinContentRate;
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnPartyMuteAudioAnycase(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyMuteAudioAnycase: ", m_partyConfName, GetPartyRsrcId());
	PartyMuteAudio(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::PartyMuteAudio(CSegment* pParam)
{
	WORD onOff;
	*pParam >> onOff;
	EOnOff eOnOff = onOff ? eOn : eOff;
	if (IsInDirectionConnectedToAudioBridge())
		m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOnOff, PARTY);
	else
	{
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());

		if (pConfParty == NULL) {
			DBGPASSERT_AND_RETURN(1);
		}

		pConfParty->SetAudioMuteByParty(onOff ? TRUE : FALSE);

	}
	m_pIpCurrentMode->GetMediaMode(cmCapAudio, cmCapReceive).SetMute(onOff ? YES : NO);

	//CMediaModeH323& pNewTransAudio = m_pIpCurrentMode->GetMediaMode(cmCapAudio, cmCapReceive);
	//pNewTransAudio.SetMute(YES);
}

////////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::OnPartyChannelDisconnect(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyChannelDisconnect: partyConfName . ",m_partyConfName, GetPartyRsrcId());

   	WORD tempDataType, tempDirection, tempRoleLabel;
	*pParam >> tempDataType >> tempDirection >> tempRoleLabel;

	cmCapDataType  dataType	 = (cmCapDataType)tempDataType;
	cmCapDirection direction = (cmCapDirection)tempDirection;
	ERoleLabel     roleLabel = (ERoleLabel)tempRoleLabel;
	CapEnum contentProtocol = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo,cmCapTransmit,kRolePresentation);
	CapEnum curContentProtocol = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo,cmCapTransmit,kRolePresentation);

	UpdateCurrentModeMediaOff(dataType, roleLabel, direction);


	if (dataType == cmCapAudio)
	{
		//VNGR-9662
		UpdateInitialModeMediaOff(dataType, roleLabel, direction);
        DisconnectPartyFromAudioBridgeIfNeeded();
	}
//Audio Patch
//return;


	else if (dataType == cmCapVideo)
	{
		if (roleLabel == kRolePeople)
		{
			if (m_bSuspendVideoUpdates)//&& direction == cmCapReceive)
                PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyChannelDisconnect: Suspend video update for incoming and outgoing channel ",m_partyConfName, GetPartyRsrcId());
            else
            {
            	// Escalations (VNG) VNGFE-6939.
            	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyChannelDisconnect: - fix!!! ",m_partyConfName, GetPartyRsrcId());
            	//m_bIsNewScm = TRUE;
            	DisconnectPartyFromVideoBridgeIfNeeded(m_pIpCurrentMode);
            }

		}
		else if ((roleLabel & kRoleContentOrPresentation) && (direction == cmCapTransmit))
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyChannelDisconnect: Secondary content ",m_partyConfName, GetPartyRsrcId());
			ON(m_bNoContentChannel);

			if(m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit, kRoleContentOrPresentation))
			{
			   WORD details = 0;
			   BYTE bIsSpeaker = FALSE;
			   if(m_pContentBridge)
				   bIsSpeaker = m_pContentBridge->IsTokenHolder(m_pParty);
			   if((contentProtocol == curContentProtocol) && (contentProtocol == eH264CapCode) && bIsSpeaker )
			   {
				   PTRACE2(eLevelInfoNormal,"CH323PartyCntl::OnPartyChannelDisconnect: Content protocol remains H264, Do Not  Disconnect content speaker party from content bridge, Name: ",m_partyConfName);
			   }
			   else
			   {
			      if (m_pContentBridge && m_pContentBridge->IsConnected())
			      {
				    DisconnectPartyFromContentBridge();
				    PTRACE2(eLevelInfoNormal,"CH323PartyCntl::OnPartyChannelDisconnect: H264->H263 transition,  Disconnect party from content bridge, Name: ",m_partyConfName);
			      }
			   }
			}
			else
			{
				 if (m_pContentBridge && m_pContentBridge->IsConnected())
			     {
					 DisconnectPartyFromContentBridge();
				     PTRACE2(eLevelInfoNormal,"CH323PartyCntl::OnPartyChannelDisconnect: Content channel will remain now off  Disconnect party from content bridge, Name: ",m_partyConfName);
			     }
			}
			UpdateCurrentModeNoMedia(cmCapVideo, kRoleContentOrPresentation);
			if(m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation)!= 0 || m_pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation)!= 0)//on close out channel only if out content rate was not 0 we need to rest it to p+c else there is no active content anyway rate stay the same
				UpdateVideoOutBridgeH239Case(FALSE);
		}
	}

	else if (dataType == cmCapData)
		DisconnectPartyFromFECCBridge();
}


/////////////////////////////////////////////////////////////////////////////
// video brdg disconnection indication
void CH323PartyCntl::OnVidBrdgDisconnect(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnVidBrdgDisconnect : Name - ",m_partyConfName, GetPartyRsrcId());
	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);
	if (resStat == statVideoInOutResourceProblem)
	{
		BYTE 	mipHwConn = (BYTE)eMipBridge;
		BYTE	mipMedia = (BYTE)eMipVideo;
		BYTE	mipDirect = 0;
		BYTE	mipTimerStat = 0;
		BYTE	mipAction = 0;
		*pParam >>  mipDirect >> mipTimerStat >> mipAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;

		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
		POBJDELETE(pSeg);
	}
    else if (m_state == REALLOCATE_RSC)
    {
        BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_H323_FREE_VIDEO_RESOURCES);
		eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpCurrentMode);
		//m_pIpCurrentMode->Dump ("CH323PartyCntl::OnVidBrdgDisconnect -re allocate state - Current:", eLevelInfoNormal);
		if((bEnableFreeVideoResources == FALSE) || (IsUndefinedParty() == FALSE) || (m_pRmtCapH323->GetNumOfVideoCap()>0))
			eCurrentVideoType = (eCurrentVideoType == eVideo_party_type_none) ? eCP_H264_upto_CIF_video_party_type : eCurrentVideoType;

		DWORD artCapacity = 0;
		artCapacity = CalculateArtCapacityAccordingToScm(m_pIpCurrentMode, TRUE /*add audio + video for current*/);
		m_artCapacity = artCapacity;
		CreateAndSendReAllocatePartyResources(eIP_network_party_type, eCurrentVideoType, eAllocateAllRequestedResources,FALSE/*reAllocRtm*/,FALSE/*isSyncMessage*/,FALSE/*IsEnableSipICE*/,artCapacity);
    }
}


/////////////////////////////////////////////////////////////////////////////
// Audio brdg disconnection indication
void CH323PartyCntl::OnAudBrdgDisconnect(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnAudBrdgDisconnect : Name - ",m_partyConfName, GetPartyRsrcId());
	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam);
	if (bIsDisconnectOk == FALSE)
	{
		BYTE 	mipHwConn = (BYTE)eMipBridge;
		BYTE	mipMedia = (BYTE)eMipAudio;
		BYTE	mipDirect = 0;
		BYTE	mipTimerStat = 0;
		BYTE	mipAction = 0;
		*pParam >>  mipDirect >> mipTimerStat >> mipAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;

		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
		POBJDELETE(pSeg);

	}
}


/////////////////////////////////////////////////////////////////////////////
// content brdg disconnection ack
int  CH323PartyCntl::OnContentBrdgDisconnected(CSegment* pParam)
{
	return CIpPartyCntl::OnContentBrdgDisconnected(pParam);
}

/////////////////////////////////////////////////////////////////////////////
int  CH323PartyCntl::OnContentBrdgConnected(CSegment* pParam)
{
	return CIpPartyCntl::OnContentBrdgConnected(pParam);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsFECC()
{
	/*	if(m_IsMovedParty)
	{
		BYTE bDataValid = IsFeccValid();
		if (! bDataValid)
		{
			PTRACECONFID(eLevelInfoNormal,"CH323PartyCntl::IsFECC - lsd mode OFF or ON after move from none LSD conf", GetPartyRsrcId());
			return FALSE;
		}
	}*/

	if (m_pIpInitialMode->IsMediaOn(cmCapData, cmCapTransmit))
	{
		PTRACEPARTYID(eLevelInfoNormal,"CH323PartyCntl::IsFECC - FECC mode ON", GetPartyRsrcId());
		return TRUE;
	}
	else
		return FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH323PartyCntl::GetRemoteCapsVideoRate(CapEnum protocol) const
{
	DWORD bitrate = m_pRmtCapH323->GetMaxVideoBitRate(protocol, cmCapReceive);
	return (bitrate * 100);
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::SetPartyToSecondaryAndStopChangeMode(BYTE reason,DWORD details,BYTE direction,CSecondaryParams *pSecParams,BYTE bDisconnectChannels)
{
	m_bSuspendVideoUpdates = FALSE;
	CIpPartyCntl::SetPartyToSecondaryAndStopChangeMode(reason,details,direction,pSecParams, bDisconnectChannels);
}


/////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::ImplementSecondaryInPartyLevel()
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323PartyCntl::ImplementSecondaryInPartyLevel", GetPartyRsrcId());
	//video in
	H264VideoModeDetails h264VidModeDetails = GetH264ModeAccordingToVideoPartyType(m_eLastAllocatedVideoPartyType);
	MsSvcVideoModeDetails mssvcmodedetails;
	m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails ,((APIU8)-1)/*CIF4*/,TRUE /*bIsAudioOnly*/,FALSE/*isRTV*/,FALSE/*ms svc*/,0);// update the party about the remote capabilities.

    // if we are in cascade between RMX and MGC, in order to cause MGC moving to secondary,
	// H323Control will operate IpDisconnectMediaChannel instead of InActivateChannel
   	m_pPartyApi->InActivateChannel(cmCapVideo,cmCapReceive,kRolePeople);

	//video out
	m_pPartyApi->IpDisconnectMediaChannel(cmCapVideo,cmCapTransmit,kRolePeople);
	//content out
	m_pPartyApi->IpDisconnectMediaChannel(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation);
	// VNGR-5870 - Fecc out
	m_pPartyApi->IpDisconnectMediaChannel(cmCapData, cmCapTransmit,kRolePeople);
}


/////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::ImplementUpdateSecondaryInPartyControlLevel()
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323PartyCntl::ImplementUpdateSecondaryInPartyControlLevel", GetPartyRsrcId());
    m_pIpCurrentMode->SetMediaOff(cmCapVideo,cmCapTransmit,kRolePeople); //only transmit is closed!!!

    m_pIpCurrentMode->SetVideoBitRate(0, cmCapReceive, kRolePeople); //only transmit is closed!!!

    m_pIpCurrentMode->SetMediaOff(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation);
   // DisconnectPartyFromVideoBridge();
	// VNGR-5870 - Fecc out
    m_pIpCurrentMode->SetMediaOff(cmCapData, cmCapTransmit,kRolePeople);

	UpdateCurrentModeInDB();

	//Inorder to set the resources accordingly
	*m_pIpInitialMode = *m_pIpCurrentMode;

}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::ChangeScmAccordingToH323Scm(CIpComMode* pH323Scm)
{
	TRACECOND_AND_RETURN(!m_pIpCurrentMode, "PartyId:" << GetPartyRsrcId() << " - Failed, invalid 'pIpCurrentMode'");

	if (pH323Scm)
	{
		CLargeString logStr;
		logStr << "Conference types - m_pIpCurrentMode = "<< m_pIpCurrentMode->GetConfType();
		logStr << ", m_pIpInitialMode = "<< m_pIpInitialMode->GetConfType();
		logStr << ", pH323Scm = "<< pH323Scm->GetConfType();
		PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::ChangeScmAccordingToH323Scm - \n", logStr.GetString(), GetPartyRsrcId());

		//audio
		// we set the receive and transmit parameters.
		if (m_pIpCurrentMode->IsMediaOn(cmCapAudio))
			m_pIpInitialMode->SetMediaMode(m_pIpCurrentMode->GetMediaMode(cmCapAudio), cmCapAudio);		// the default is receive
		if (m_pIpCurrentMode->IsMediaOn(cmCapAudio, cmCapTransmit))
			m_pIpInitialMode->SetMediaMode(m_pIpCurrentMode->GetMediaMode(cmCapAudio, cmCapTransmit), cmCapAudio, cmCapTransmit);

		//video
		DWORD videoBitRate = m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapTransmit).GetBitRate();
		m_pIpInitialMode->SetMediaMode(pH323Scm->GetMediaMode(cmCapVideo, cmCapReceive), cmCapVideo, cmCapReceive);
		m_pIpInitialMode->SetMediaMode(pH323Scm->GetMediaMode(cmCapVideo, cmCapTransmit), cmCapVideo, cmCapTransmit);
		if (m_pIpInitialMode->GetConfType() != kCop) // for cop we will update the receive rate update the bridge (in CH323PartyCntl::ChangeMode). for non-cop there is disconnecting of the bridge during reallocation, so there is no update bridge before the party change mode.
		{
			if (m_pIpInitialMode->GetConfType() != kCp) // No Active LPR - Continue as is
				m_pIpInitialMode->SetVideoBitRate(m_videoRate,cmCapReceiveAndTransmit);
			else
			{
				m_pIpInitialMode->SetVideoBitRate(m_videoRate,cmCapReceive);

				// -- VNGFE-8205 - when content rate is 0, xmit video rate stays low so we should take video rate for transmit from original rate
				DWORD xmitContentRateInScm = pH323Scm->GetContentBitRate(cmCapTransmit);
				TRACEINTO << "VNGFE-8205: isLprActive - " << (int)m_isLprActive << ", pH323Scm->xmitContentRateInScm - " << xmitContentRateInScm
						<< ", m_videoRate - " << m_videoRate << ", m_pIpInitialMode->GetTotalVideoRate() - " << m_pIpInitialMode->GetTotalVideoRate();

				if(!m_isLprActive && !xmitContentRateInScm)
					videoBitRate = m_videoRate;
				else if (m_pIpInitialMode->GetTotalVideoRate())
					videoBitRate = min(m_pIpInitialMode->GetTotalVideoRate(), m_videoRate) - xmitContentRateInScm;
				else // protect from not initilized values.
					videoBitRate = m_videoRate - xmitContentRateInScm;

				m_pIpInitialMode->SetVideoBitRate(videoBitRate,cmCapTransmit);
			}
		}

		//content ??
		m_pIpInitialMode->SetMediaMode(pH323Scm->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation), cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		m_pIpInitialMode->SetMediaMode(pH323Scm->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation), cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll(CSegment* pParam)
{
    PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll :  Name - ",m_partyConfName, GetPartyRsrcId());

  	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);

	DWORD videoRate;
	WORD  videoChanged = 0;
	*pParam >> videoRate;

	// VNGFE-787
	*pParam >> m_isCodianVcr;

    *pParam >> m_incomingVideoChannelHandle;
    TRACEINTO << "mix_mode: channel handle set to "  << m_incomingVideoChannelHandle;

	// VNGR-6663
	m_pRmtCapH323->DeSerialize(NATIVE,*pParam);

	SetH323VideoRate(videoRate);
	m_pIpCurrentMode->SetTotalVideoRate(videoRate);

	//RESERVE COP LINK LECTURE level param
	if(m_pIpInitialMode->GetConfType() == kCop)
	{
		SaveLecturerLinkCopLevelAccordingToCurrent();
	}


}

//////////////////////////////////////////////////////////////////////////////////////////////
//void CH323PartyCntl::AdditionalRsrcHandling(WORD status)
//{
//	if (status == statOK) // return status from disTss can be of a failer action
//		m_pTaskApi->EndAddParty(m_pParty, status);
//}

//////////////////////////////////////////////////////////////////////////////////////////////
//New Resource

void CH323PartyCntl::AdditionalRsrcHandling(WORD status, CIpComMode* pScm )
{
    if (!IsValidPObjectPtr (pScm))
    {
        //if we received an SCM it's the initial for change mode
        PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::AdditionalRsrcHandling - NOT Change mode", GetPartyRsrcId());
        m_bContinueChangeModeAfterReAlloc = FALSE;
        pScm = m_pIpCurrentMode;
    }
    else
    {
    	if( pScm->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))
    	{
   	 		PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::AdditionalRsrcHandling - Stop Change mode", GetPartyRsrcId());
    		m_bContinueChangeModeAfterReAlloc = FALSE;
    	}
    	else
		{
			PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::AdditionalRsrcHandling - Change mode", GetPartyRsrcId());
    	 	m_bContinueChangeModeAfterReAlloc = TRUE;
		}

    }


   // pScm->Dump ("CH323PartyCntl::AdditionalRsrcHandling - SCM:", eLevelInfoNormal);
	m_bAdditionalRsrcActivated = TRUE;
	if (status == statOK) // return status from disTss can be of a failer action
	{
		m_pIpCurrentMode->Dump ("CH323PartyCntl::AdditionalRsrcHandling - Current:", eLevelInfoNormal);


		BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_H323_FREE_VIDEO_RESOURCES);
		eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(pScm);

		BYTE isUseFecc = FALSE;
                if( pScm->IsMediaOn(cmCapData,  cmCapTransmit) || pScm->IsMediaOn(cmCapData,  cmCapReceive) )
                     isUseFecc = TRUE;

		if((bEnableFreeVideoResources == FALSE) ||
			((CProcessBase::GetProcess()->GetProductType() != eProductTypeNinja) && (IsUndefinedParty() == FALSE))
			|| (m_pRmtCapH323->GetNumOfVideoCap()>0) || isUseFecc)
			eCurrentVideoType = (eCurrentVideoType == eVideo_party_type_none) ? eCP_H264_upto_CIF_video_party_type : eCurrentVideoType;


        if (m_pIpInitialMode->GetConfType() != kCp && m_pIpInitialMode->GetConfType() != kCop)
        	eCurrentVideoType = min (eCP_H264_upto_CIF_video_party_type, eCurrentVideoType);

        if(eCurrentVideoType == eVideo_party_type_none)
        {
        	// romem
        	if(m_bNoVideRsrcForVideoParty)
        	{
        		PTRACE2(eLevelInfoNormal, "CH323PartyCntl::AdditionalRsrcHandling - Party was already audio only, cancel m_bNoVideRsrcForVideoParty, Name: ",m_partyConfName);
        		m_pPartyApi->UpdateNoResourcesForVideoParty(m_bNoVideRsrcForVideoParty);
        		m_bNoVideRsrcForVideoParty = FALSE;
        	}

        }

		if( (eCurrentVideoType < eCP_H261_CIF_equals_H264_HD1080_video_party_type) &&
		    ( ((CapEnum)pScm->GetMediaType(cmCapVideo, cmCapTransmit,kRolePeople) == eH261CapCode) ||
		      ((CapEnum)pScm->GetMediaType(cmCapVideo, cmCapReceive,kRolePeople) == eH261CapCode) ) )
		{
			eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
			BOOL bH261Support = GetSystemCfgFlagInt<BOOL>("H261_SUPPORT_ALWAYS");
			if( !bH261Support &&  systemCardsBasedMode == eSystemCardsMode_mpmrx)
			{
				eCurrentVideoType = eVideo_party_type_none;
			}
		}

		//KW 1288
		CConfParty* pConfParty = NULL;
		BYTE 		protocol   = 0;
		if(m_pConf && m_pConf->GetCommConf())
		{
			pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
			PASSERTMSG(!pConfParty,"!pConfParty");
			protocol = (pConfParty) ? pConfParty->GetVideoProtocol() : 0;
		}

		eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
		if(protocol == VIDEO_PROTOCOL_H261  && systemCardsBasedMode == eSystemCardsMode_mpmrx && ( ((CapEnum)pScm->GetMediaType(cmCapVideo, cmCapTransmit,kRolePeople) == eH261CapCode ) || ((CapEnum)pScm->GetMediaType(cmCapVideo, cmCapReceive,kRolePeople) == eH261CapCode ) ) )
		{
			PTRACE(eLevelInfoNormal,"CIpPartyCntl::GetLocalVideoPartyType - force H261 resolution on mpmRx- Resource is as HD1080 30");
			eCurrentVideoType = eCP_H261_CIF_equals_H264_HD1080_video_party_type;
		}

		BYTE bChangeResources = IsNeedToChangeVideoResourceAllocation(eCurrentVideoType);

        // if this is an Avaya VIDEO call but video caps did not arrive yet, wait for them
		bChangeResources &= !(m_bLateReleaseOfResources && m_pRmtCapH323->GetNumOfVideoCap()== 0);

        if (bChangeResources == FALSE ||
			((m_pIpInitialMode->GetConfType() != kCp) &&
             (bEnableFreeVideoResources == FALSE)))
		{
			ON(m_isFullBitRateConnect);
            if (!m_bContinueChangeModeAfterReAlloc)
            {
                EndConnectionProcess(status);
            }
		}
		else
		{
            //Disconnect from video bridge if needed
            CIpComMode* pTmpScm = new CIpComMode;
            *pTmpScm = *m_pIpInitialMode;
            pTmpScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
            BYTE bIsDisconnectFromVideoBridge = DisconnectPartyFromVideoBridgeIfNeeded(pTmpScm);
            //if we need to disconnect from the bridge we will handle the ack in this state
            //and continue in the reallocation AFTER the Ack from the bridge

            if (!bIsDisconnectFromVideoBridge)
            {
                //No need to disconnect - continue in ReAllocation
				DWORD artCapacity = 0;
				artCapacity = CalculateArtCapacityAccordingToScm(pScm, TRUE /*add audio + video for current*/);
				m_artCapacity = artCapacity;

				CreateAndSendReAllocatePartyResources(eIP_network_party_type, eCurrentVideoType, eAllocateAllRequestedResources,FALSE/*reAllocRtm*/,FALSE/*isSyncMessage*/,FALSE/*IsEnableSipICE*/,artCapacity);
                if (m_state != REALLOCATE_RSC) // no re-allocation was done for some reason
                {
                	DWORD artCapacity = 0;
					 artCapacity = CalculateArtCapacityAccordingToScm(m_pIpCurrentMode, TRUE /*add audio + video for current*/);
					 if(m_artCapacity != artCapacity)
						 CreateAndSendReAllocateArtForParty(eIP_network_party_type ,eCurrentVideoType, eAllocateAllRequestedResources,FALSE/*ICE*/,artCapacity);

					 EndConnectionProcess(status);
                }
            }
            else
            {
                m_state = REALLOCATE_RSC; //to make us reallocate after the disconnection.
                m_OldState = CHANGEVIDEO; //to reconnect to the video bridge after the reallocation
            }


            POBJDELETE(pTmpScm);
		}
	}
	else
		pScm->Dump ("CH323PartyCntl::AdditionalRsrcHandling -failed status SCM:", eLevelInfoNormal);
}
/////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::CreateAndSendReAllocatePartyResources(eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType, EAllocationPolicy allocationPolicy,WORD reAllocRtm,WORD isSyncMessage,BYTE IsEnableSipICE,DWORD artCapacity )
{
    if (videoPartyType != m_eLastReAllocRequestVideoPartyType)
    {
        if (m_state != REALLOCATE_RSC)
            m_OldState = m_state;
        m_state = REALLOCATE_RSC;
        CPartyCntl::CreateAndSendReAllocatePartyResources(networkPartyType, videoPartyType, allocationPolicy, reAllocRtm,isSyncMessage,IsEnableSipICE,artCapacity);
    }
    else
    {
    	PTRACE2(eLevelInfoNormal, "CH323PartyCntl::CreateAndSendReAllocatePartyResources same re-allocation request", m_partyConfName);
    }

}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::EndConnectionProcess(WORD status)
{
    PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::EndConnectionProcess", GetPartyRsrcId());
	if (status == statOK) // return status from disTss can be of a failer action
		m_pTaskApi->EndAddParty(m_pParty, status);
}

/////////////////////////////////////////////////////////////////////////////
EMediaDirection  CH323PartyCntl::HandleAudioBridgeConnectedInd(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::HandleAudioBridgeConnectedInd", GetPartyRsrcId());
	EMediaDirection eAudBridgeConnState = eNoDirection;
	*pParam >> (WORD&)eAudBridgeConnState;
	UpdateAudioBridgeConnectionState(eAudBridgeConnState);

	if (AreTwoDirectionsConnectedToAudioBridge())
	{
		PTRACE2PARTYID(eLevelInfoNormal," ---> Audio Connection Established ",m_partyConfName, GetPartyRsrcId());
		m_pTaskApi->UpdateDB(m_pParty,AUDCON,TRUE);

		if (m_voice || (IsUndefinedParty() && IsRemoteCapNotHaveVideo()) || IsOutDirectionConnectedToVideoBridge())
		{
			CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
			BYTE isPartyAlreadySetToVoice = NO;
			if(IsValidPObjectPtr(pConfParty))
				isPartyAlreadySetToVoice = pConfParty->GetVoice();

			BYTE bKeepContentSecondaryCause = FALSE;
			if (IsUndefinedParty() && IsRemoteCapNotHaveVideo() && !isPartyAlreadySetToVoice)
				m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_AUDIO_ONLY);
			else
				bKeepContentSecondaryCause = TRUE;
			m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED);
			RemoveSecondaryCause(bKeepContentSecondaryCause);// check if to keep content secondary cause.
			UpdatePartyStateInCdr();
		}
	}

	return eAudBridgeConnState;
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate", GetPartyRsrcId());
    m_state = m_OldState;
    ON(m_isFullBitRateConnect);
    eVideoPartyType eCurrentVideoType;
	BOOL bIsEnableRecap = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SEND_RECAP_ATER_REALLOC_FOR_H323);	
    BOOL bIsResolutionNeededDowngrade 	= FALSE;
    BOOL bIsResolutionWasDowngraded 	= FALSE;
    BYTE bAllocationFailed				= FALSE;

    CIpComMode* pTmpComMode = NULL;
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);

	if (!m_bContinueChangeModeAfterReAlloc)
    {
    	eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpCurrentMode);
    	FixVideoBitRateIfNeeded( pConfParty, m_pIpCurrentMode , m_pH323NetSetup, FALSE, eCurrentVideoType);
    }
    else
    {
    	eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode);
    	FixVideoBitRateIfNeeded( pConfParty, m_pIpInitialMode , m_pH323NetSetup, FALSE, eCurrentVideoType);
    }

    eVideoPartyType requestedVideoPartyType = m_eLastReAllocRequestVideoPartyType;

    //BRIDGE-12084
    TRACEINTO << " Before: m_eLastAllocatedVideoPartyType " << (int)m_eLastAllocatedVideoPartyType
    		  << " m_eLastReAllocRequestVideoPartyType " << (int)m_eLastReAllocRequestVideoPartyType;
    bIsResolutionNeededDowngrade = (m_eLastAllocatedVideoPartyType > m_eLastReAllocRequestVideoPartyType)?TRUE:FALSE;

    bAllocationFailed 		 	 =  HandleReallocateResponse(pParam);

    //BRIDGE-12084
    bIsResolutionWasDowngraded 	 =  bIsResolutionNeededDowngrade && (m_eLastAllocatedVideoPartyType == m_eLastReAllocRequestVideoPartyType)?TRUE:FALSE;
    TRACEINTO << " After: m_eLastAllocatedVideoPartyType " << (int)m_eLastAllocatedVideoPartyType
    		  << " m_eLastReAllocRequestVideoPartyType " << (int)m_eLastReAllocRequestVideoPartyType
    		  << " bIsResolutionNeededDowngrade " << (int)bIsResolutionNeededDowngrade
    		  << " bIsResolutionWasDowngraded " << (int)bIsResolutionWasDowngraded;

	if (!m_bContinueChangeModeAfterReAlloc)
		m_pIpCurrentMode->Dump ("CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate m_bContinueChangeModeAfterReAlloc = false - m_pIpCurrentMode:", eLevelInfoNormal);
	else
		m_pIpInitialMode->Dump ("CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - m_pIpInitialMode:", eLevelInfoNormal);



	if (bAllocationFailed == FALSE)
	{
		// Relevant only to upgrade of resources.
        BYTE bIsAudioOnly = (m_eLastAllocatedVideoPartyType == eVideo_party_type_none) ? 1 : 0;
        if (m_eLastAllocatedVideoPartyType < requestedVideoPartyType || bIsAudioOnly)
        {
        	CSmallString str;
        	str << "AudioOnly = " << bIsAudioOnly;
            PTRACE2PARTYID(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate.Change mode, ", str.GetString(), GetPartyRsrcId());
            H264VideoModeDetails h264VidModeDetails = GetH264ModeAccordingToVideoPartyType(eCurrentVideoType);
            // BRIDGE-1373 - high profile declared twice instead of high+base profile.
            // set H264_Profile_None to prevent run over profile
            h264VidModeDetails.profileValue = H264_Profile_None;
            UpdateH264ModeInLocalCaps(h264VidModeDetails);

            //Change Mode
            if (!bIsAudioOnly)
            {
                BYTE cif4Mpi = (BYTE)-1;
        		DWORD videoRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo,cmCapTransmit, kRolePeople);
        		DWORD audioRate =  (m_pIpInitialMode->GetMediaBitRate(cmCapAudio,cmCapTransmit))*10;
            	if((m_eLastAllocatedVideoPartyType >= eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type))
            	{
			       eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
            		cif4Mpi = CH263VideoMode::GetH263Cif4VideoCardMPI(videoRate, vidQuality);
            	}

            	 h264VidModeDetails.profileValue = ((long)m_pIpCurrentMode->GetH264Profile(cmCapReceive));
            	 MsSvcVideoModeDetails mssvcmodedetails;
				if(bIsEnableRecap)
            		 m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails, cif4Mpi, bIsAudioOnly,FALSE,FALSE,(videoRate+audioRate) );// update the party about the remote capabilities.
            	 PTRACE2INT(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate 1 video rate = ", videoRate);

                CIpComMode* newScm = new CIpComMode(*m_pIpCurrentMode);
                newScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR, cmCapReceiveAndTransmit);
                if (!m_bContinueChangeModeAfterReAlloc)
                    ChangeScm(newScm, eCanChangeVideoAndContent);
                else
                  	m_pIpInitialMode = newScm;


            }
            else if (!m_bContinueChangeModeAfterReAlloc)
            	EndConnectionProcess(statOK);
        }
        else
        {
           	if(m_eLastAllocatedVideoPartyType == requestedVideoPartyType)
        	{
        		 if (!bIsAudioOnly )
        		 {
        			 H264VideoModeDetails h264VidModeDetails = GetH264ModeAccordingToVideoPartyType(eCurrentVideoType);


        			 BYTE IsChangeBestModeNeededForTx = FALSE;
        			 BYTE IsChangeBestModeNeededForRcv = FALSE;
        			 BYTE isCpConf = FALSE;

        			 if (!m_bContinueChangeModeAfterReAlloc)
        				 isCpConf = (m_pIpCurrentMode->GetConfType() == kCp);
        			 else
        				 isCpConf = (m_pIpInitialMode->GetConfType() == kCp);

        			 if (!m_bContinueChangeModeAfterReAlloc)
        			 {
        				 IsChangeBestModeNeededForTx  = checkRsrcLimitationsByPartyType(cmCapTransmit,m_pIpCurrentMode,h264VidModeDetails);
        				 IsChangeBestModeNeededForRcv  = checkRsrcLimitationsByPartyType(cmCapReceive,m_pIpCurrentMode,h264VidModeDetails);
        			 }
        			 else
        			 {
        				 IsChangeBestModeNeededForTx  = checkRsrcLimitationsByPartyType(cmCapTransmit,m_pIpInitialMode,h264VidModeDetails);
        				 IsChangeBestModeNeededForRcv  = checkRsrcLimitationsByPartyType(cmCapReceive,m_pIpInitialMode,h264VidModeDetails);
        			 }

        			 DWORD staticMB = DEFAULT_STATIC_MB;
        			 GetStaticMbForDsp(m_pIpCurrentMode, staticMB);

        			 eVideoPartyType eRemoteVideoPartyTypeTransmit = m_pIpCurrentMode->GetVideoPartyType(cmCapTransmit, staticMB);

        			 DWORD videoRate = 0;
        			 DWORD audioRate = 0;
        			 if (!m_bContinueChangeModeAfterReAlloc)
        			 {
        				 videoRate =  m_pIpCurrentMode->GetMediaBitRate(cmCapVideo,cmCapTransmit, kRolePeople);
        				 audioRate =  (m_pIpCurrentMode->GetMediaBitRate(cmCapAudio,cmCapTransmit))*10;
        			 }
        			 else
        			 {
                		 videoRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo,cmCapTransmit, kRolePeople);
                		 audioRate = (m_pIpInitialMode->GetMediaBitRate(cmCapAudio,cmCapTransmit))*10;
        			 }
                	 PTRACE2INT(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate 2 video rate = ", videoRate);

                	 TRACEINTO << " bIsResolutionWasDowngraded " << (int)bIsResolutionWasDowngraded << " IsChangeBestModeNeededForTx " << (int)IsChangeBestModeNeededForTx
                			   << " IsChangeBestModeNeededForRcv " << (int)IsChangeBestModeNeededForRcv << " isCpConf " << (int)isCpConf
                			   << " eRemoteVideoPartyTypeTransmit " << (int)eRemoteVideoPartyTypeTransmit << " videoRate " << (int)videoRate
                			   << " audioRate " << (int)audioRate;

                	 if((IsChangeBestModeNeededForTx || IsChangeBestModeNeededForRcv) && isCpConf  && eRemoteVideoPartyTypeTransmit != eVideo_party_type_none )
        			 {
        		         // BRIDGE-1373 - high profile declared twice instead of high+base profile.
        		         // set H264_Profile_None to prevent run over profile
        				 h264VidModeDetails.profileValue = H264_Profile_None;
        				 UpdateH264ModeInLocalCaps(h264VidModeDetails);
        			 }

                	 BYTE cif4Mpi = m_pLocalCapH323->Get4CifMpi();
                	 BOOL bNewAllocationCapsValuesSetted = FALSE;

					 if(m_eLastAllocatedVideoPartyType < eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
					 {
						cif4Mpi = (BYTE)-1;
						if(!IsChangeBestModeNeededForTx && !IsChangeBestModeNeededForRcv && isCpConf )
						{
							  // Scm not need to be changed but need to change local caps- VNGFE-6082
							  PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - need recap", GetPartyRsrcId());
							  h264VidModeDetails.profileValue = ((long)m_pIpInitialMode->GetH264Profile(cmCapReceive));
							  PTRACE2INT(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - profile: ",  h264VidModeDetails.profileValue);
							  MsSvcVideoModeDetails mssvcmodedetails;
							  if(bIsEnableRecap)
							  	m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails,cif4Mpi, bIsAudioOnly,FALSE,FALSE, (videoRate+audioRate) );// update the party about the remote capabilities.
							  bNewAllocationCapsValuesSetted = TRUE;
						}
					 }

					 if((IsChangeBestModeNeededForTx || IsChangeBestModeNeededForRcv || (bIsResolutionWasDowngraded && !bNewAllocationCapsValuesSetted)) //BRIDGE-12084 - added bIsResolutionWasDowngraded
					     && isCpConf  && eRemoteVideoPartyTypeTransmit != eVideo_party_type_none )
        			 {
        				PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - New FS < current FS - need recap", GetPartyRsrcId());
        				h264VidModeDetails.profileValue = ((long)m_pIpCurrentMode->GetH264Profile(cmCapReceive));
        				PTRACE2INT(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - profile: ",  h264VidModeDetails.profileValue);
        				MsSvcVideoModeDetails mssvcmodedetails;
        				 if(bIsEnableRecap)
							m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails ,cif4Mpi, bIsAudioOnly,FALSE,FALSE, (videoRate+audioRate) );// update the party about the remote capabilities.
        				CIpComMode* newScm = NULL;
        				PTRACE2INT(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - is continue: ", m_bContinueChangeModeAfterReAlloc);
        				 if (!m_bContinueChangeModeAfterReAlloc)
        					 newScm = new CIpComMode(*m_pIpCurrentMode);
        				 else
        					 newScm = new CIpComMode(*m_pIpInitialMode);

        				//if Rcv changed - need to change Local caps and scm
        				if(IsChangeBestModeNeededForRcv)
        				{

        					newScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR);
        				}

        				//if only tx changed need to update only scm
        				if(IsChangeBestModeNeededForTx)
        					newScm->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR);


        				newScm->Dump ("CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - SCM:", eLevelInfoNormal);

        				if (!m_bContinueChangeModeAfterReAlloc)
        					m_pIpCurrentMode   = newScm;
        				else
        					m_pIpInitialMode   = newScm;

					if (!(m_productId && strstr(m_productId,"VSX 8000") && !m_pRmtCapH323->IsH264CapFound()))  //add condition for BRIDGE-12841
        					EndConnectionProcess(statOK);

        			 }
        			//else if (!m_bContinueChangeModeAfterReAlloc)   //changed to below for BRIDGE-12841
        			else if (!m_bContinueChangeModeAfterReAlloc && !(m_productId && strstr(m_productId,"VSX 8000") && !m_pRmtCapH323->IsH264CapFound() && bNewAllocationCapsValuesSetted))
							EndConnectionProcess(statOK);
        		 }
        		 else if (!m_bContinueChangeModeAfterReAlloc)
        		  		EndConnectionProcess(statOK);

       		}
        	else if (!m_bContinueChangeModeAfterReAlloc)
        		EndConnectionProcess(statOK);

        }

	}

	m_pIpCurrentMode->Dump ("CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - CURRENT NOA DBG:", eLevelInfoNormal);
	m_pIpInitialMode->Dump ("CH323PartyCntl::OnRsrcReAllocatePartyRspReAllocate - m_pIpInitialMode NOA DBG:", eLevelInfoNormal);
	//complete the change mode if needed
    if (m_bContinueChangeModeAfterReAlloc)
    {
        m_bContinueChangeModeAfterReAlloc = FALSE;
        BYTE bChangeVidMode = ChangeVideoModeIfNeeded();
		if (!bChangeVidMode)
        {
            //in this point media state is connected, so we continue with the checking:
            if ((m_eVidBridgeConnState == eBridgeDisconnected) &&
                (m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople)) &&
                (m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople)))
            {
                PTRACE2PARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeIdle : Video communication wasn't established. ",m_partyConfName, GetPartyRsrcId());
                SetPartyToSecondaryAndStopChangeMode(SECONDARY_CAUSE_NO_VIDEO_CONNECTION);
                return;
            }
            ChangeOther();
        }
    }

}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnRsrcReAllocatePartyRspChangeAll(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::OnRsrcReAllocatePartyRspChangeAll", GetPartyRsrcId());
	ON(m_isFullBitRateConnect);
	BYTE bAllocationFailed = HandleReallocateResponse(pParam);
	if (bAllocationFailed == FALSE)
		m_state = m_OldState;
}


///End New Reource handling

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsCapableOfVideo()
{
	return m_pLocalCapH323->OnType(cmCapVideo);
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsCapableOfHD720()
{
	return m_pLocalCapH323->IsCapableOfHD720();
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsCapableOfHD1080()
{
	return m_pLocalCapH323->IsCapableOfHD1080();
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsCapableOf4CIF()
{
	return m_pLocalCapH323->IsCapableOf4CIF();
}

//////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CH323PartyCntl::GetCPVideoPartyTypeAccordingToCapabilities()
{
	return m_pLocalCapH323->GetCPVideoPartyType();
}


//////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH323PartyCntl::GetConfRate() const
{
	return m_pH323NetSetup->GetMaxRate();
}

//////////////////////////////////////////////////////////////////////////////////////////////
DWORD CH323PartyCntl::GetSetupRate()
{
	return m_pH323NetSetup->GetRemoteSetupRate();
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::PartyConnectedAudio(CSegment* pParam, WORD status)
{
	PASSERT_AND_RETURN(NULL == m_pIpCurrentMode);

	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::PartyConnectedAudio : Name - ",m_partyConfName, GetPartyRsrcId());

	if ( (status != statOK) && (status != statSecondary) && (status != statVideoBeforeAudio) )
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::PartyConnectedAudio: Failed to connect the audio!!!",m_partyConfName, GetPartyRsrcId());
		m_pTaskApi->PartyDisConnect(H323_CALL_CLOSED_BY_MCU, m_pParty);
		m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,status);
		return;
	}

	BYTE bIsRmtCap;
	*pParam >> bIsRmtCap;
	m_pRmtCapH323->DeSerialize(NATIVE,*pParam);

	BYTE bIsCurrentScm;
	*pParam >> bIsCurrentScm;
	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pIpCurrentMode->Dump ("CH323PartyCntl::PartyConnectedAudio - mix_mode: Current:", eLevelInfoNormal);

	ON(m_isRemoteCapReady);
	if (m_pRmtCapH323->IsDBC2())
		ON(m_isRemoteDBC2);

	*pParam >>  m_bLateReleaseOfResources;

    //Audio Patch
    //if (m_eAudBridgeConnState ==  eInAndOutConnected)
    //	UpdateAudioBridgesIfNeeded();
    //else

	//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::PartyConnectedAudio - mute audio rec
	if ( (GetPartyCascadeType() == CASCADE_MCU) && (m_linkType == eSubLinkParty))
	{
	    //PTRACE2PARTYID(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::PartyConnectedAudio - mute audio rec - Name - ",m_partyConfName, GetPartyRsrcId());
	    if (m_pIpCurrentMode)
	        (m_pIpCurrentMode->GetMediaMode(cmCapAudio, cmCapReceive)).SetMute(TRUE);
	    else
	    {
	        PTRACE(eLevelError, "ITP_CASCADE: CH323PartyCntl::PartyConnectedAudio ERROR - m_pIpCurrentMode is NULL - cann't set mute");
	        //PASSERTMSG(1,"ITP_CASCADE: CH323PartyCntl::PartyConnectedAudio ERROR - m_pIpCurrentMode is NULL - cann't set mute");
	    }
	}

	ConnectPartyToAudioBridge(m_pIpCurrentMode);
}

//YOELLA - YAELS` patch
/////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::ConnectPartyToContentBridge()
{
	PTRACE(eLevelInfoNormal,"CH323PartyCntl::ConnectPartyToContentBridge() - begin");
	if (m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation))
		return;

	if (m_pContentBridge)
	{
		if (!m_isContentConn)
		{
			ON(m_isContentConn);
			//VNGFE-6111 Set the fix as implementation for ISDN (no need to ask regarding the content holder)
            //const CTaskApp* GetTokenHolder(BYTE &mcuNum,BYTE &terminalNum);
            //BYTE mcuNum;
            //BYTE termNum;
            //const CTaskApp* tokenHolder = m_pContentBridge->GetTokenHolder(mcuNum, termNum);


            WORD partyContentRate = 0;
			 //fix for HW problem on ChinaUnicom HW EP disconnected when active content on conference
			if( !IsPartyCascade() || ( IsPartyCascade() &&(MASTER == m_masterSlaveStatus) ) ) //Master Link or EP keep the original behavior
			{
				 //if (IsValidPObjectPtr(tokenHolder))
					 partyContentRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);
			}
			else //Slave Link
				 PTRACE(eLevelInfoNormal,"CH323PartyCntl::ConnectPartyToContentBridge() : SLAVE Link connected, connect it with Zero K for China Unicom usages!!!!!");

            PTRACE2INT(eLevelInfoNormal,"CH323PartyCntl::ConnectPartyToContentBridge partyContentRate , ",partyContentRate);
            CapEnum H323partyProtocol = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);
            CComModeInfo cmInfo = H323partyProtocol;
		    WORD partyH239Protocol = cmInfo.GetH320ModeType();
            BYTE bCascade = FALSE;
            if (m_bIsCascade == CASCADE_MCU)
                bCascade = TRUE;

			//HP content
			BYTE isContentH264HighProfile = FALSE;
			if (partyH239Protocol == H264)
				isContentH264HighProfile = m_pIpInitialMode->IsH264HighProfileContent(cmCapTransmit);

			CContentBridgePartyInitParams* pBrdgPartyInitParams = 	new CContentBridgePartyInitParams(m_name, m_pParty, GetPartyRsrcId(), GetInterfaceType(),partyContentRate,partyH239Protocol,isContentH264HighProfile,NULL, NULL,NULL,NULL,bCascade,m_masterSlaveStatus);  //HP content

			CSmallString str;
			str << "CH323PartyCntl::ConnectPartyToContentBridge. pBrdgPartyInitParams - " << ((CContentBridgePartyInitParams*)pBrdgPartyInitParams)->GetByCurrentContentRate()
			    << ", H264Profile:  " << (int)(((CContentBridgePartyInitParams*)pBrdgPartyInitParams)->GetByCurrentContentH264HighProfile());   //HP content
	     	if (TRUE == bCascade/*and (CASCADE_MODE_SLAVE == m_masterSlaveStatus) || (CASCADE_MODE_MASTER == m_masterSlaveStatus)*/)
			{
	      		PTRACE2PARTYID(eLevelInfoNormal,"Inform Conf On LinkTryToConnect. ", str.GetString(), GetPartyRsrcId());


	      		CSegment* pParam = new CSegment;
	      		pBrdgPartyInitParams->Serialize(NATIVE, *pParam);
	      		m_pTaskApi->LinkTryToConnect(pParam);
	      		POBJDELETE(pParam);
			}

	     	else
	      	{
				ALLOCBUFFER(tmp,2*H243_NAME_LEN+150); //max size of m_partyConfName is (2*H243_NAME_LEN+50)
				sprintf(tmp, "%s\n ---> [%s] - Establishing Content Party Connection ", str.GetString(), m_partyConfName);
				PTRACE2PARTYID(eLevelInfoNormal,"",tmp, GetPartyRsrcId());
				DEALLOCBUFFER(tmp);

			 	m_pContentBridge->ConnectParty(pBrdgPartyInitParams);
			 	if(m_pXCodeBridgeInterface && !m_isXCodeConn)
			 	{
			 		m_pConf->ConnectPartyToXcodeBridge(m_pParty);
			 		ON(m_isXCodeConn);
			 	}
			}

	      	POBJDELETE(pBrdgPartyInitParams);
		}
	}
	PTRACE(eLevelInfoNormal,"CH323PartyCntl::ConnectPartyToContentBridge() - begin");
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnPartyReceivedECS(CSegment* pParam)
{
	char* strDetails = GetPartyAndClassDetails();
	PTRACE2PARTYID(eLevelInfoNormal, "CH323PartyCntl::OnPartyReceivedECS - ", strDetails, GetPartyRsrcId());
	DEALLOCBUFFER(strDetails);
	PartyReceivedECS();
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::PartyReceivedECS()
{
	m_state = CHANGE_ALL_MEDIA;
	OFF(m_isFullBitRateConnect);
	m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED_PARTIALY);
	RemoveSecondaryCause(FALSE);// check if to keep content secondary cause.
}


/////////////////////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::PartyReceivedReCapsChangeAll(CSegment* pParam)
{
	PartyReceivedReCaps(pParam);

	//Check upgrade from voice only (ECS case) - no need to update DB status, since this action is alreay happening in PartyReceivedReCaps function.
}

////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnPartyReceivedReCapsAnycase(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::OnPartyReceivedReCapsAnycase", GetPartyRsrcId());
	PartyReceivedReCaps(pParam);
	if(!m_bPendingRemoteCaps)
		HandleRemoteReCap();
	else
		PTRACEPARTYID(eLevelInfoNormal, "CH323PartyCntl::OnPartyReceivedReCapsAnycase - pending remote cap due to link lecturer", GetPartyRsrcId());

	/*if (m_pRmtCapH323->GetNumOfVideoCap() > 0 && IsUndefinedParty() && m_pLocalCapH323->GetNumOfVideoCap()>0)
		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_RESET_STATUS);*/
}

////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::HandleRemoteReCap()
{
	cmCapDirection direction = cmCapReceive;
//	cmCapDirection direction = cmCapTransmit;
	if (m_pIpInitialMode->GetConfType() != kCp)
        direction = cmCapReceiveAndTransmit;
	/* v4.1C <--> v6 merge toDo: decide which version is correct:
	   v4.1c
if (m_pIpInitialMode->GetConfType() == kCop)
		m_pTaskApi->PartyChangeVidMode(m_pParty, FALSE);
	else if (!m_pRmtCapH323->AreRemoteCapsContaining(m_pIpInitialMode, kCapCode|kBitRate|kFormat|kFrameRate|kH264Level|kH264Additional_FS|kH264Additional_MBPS, cmCapVideo, direction, kRolePeople))
        m_pTaskApi->PartyChangeVidMode(m_pParty, FALSE);
		if(!m_pRmtCapH323->AreRemoteCapsContaining(m_pIpInitialMode, kCapCode|kBitRate|kFormat|kFrameRate|kH264Level|kH264Additional_FS|kH264Additional_MBPS,
								cmCapVideo, direction, kRoleContentOrPresentation))
	        m_pTaskApi->PartyChangeVidMode(m_pParty, FALSE);
	}
	End of v4.1C*/
	//V6:
	m_pTaskApi->PartyChangeVidMode(m_pParty, FALSE);
}

////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::PartyReceivedReCaps(CSegment* pParam)
{
	char* strDetails = GetPartyAndClassDetails();
	PTRACE2PARTYID(eLevelInfoNormal, "CH323PartyCntl::OnPartyReceivedReCaps - ", strDetails, GetPartyRsrcId());
	DEALLOCBUFFER(strDetails);

	m_pRmtCapH323->DeSerialize(NATIVE,*pParam);

	//Party might no longer be audio only after recap
    CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
    if (pConfParty)
    {
       BYTE isCurrentlyAudioOnly = pConfParty->GetVoice();
	   if (m_pRmtCapH323->GetNumOfVideoCap() && isCurrentlyAudioOnly)
	       m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_RESET_STATUS);
	}
}

/////////////////////////////////////////////////////////////////////////////
// after getting the new capabilities from the Conference update the Tss distribution
WORD CH323PartyCntl::SetH323VideoRate(DWORD videoRate, BYTE bIsSpecialCaseForChangeRate,
													  BYTE bIsDisconnectParty)
{
	//in case of VSW, we can't change the rates (only in GW smart VSW we can do that)!!
	if ( (m_pIpInitialMode->GetConfType() == kCp) || bIsSpecialCaseForChangeRate)
	{
		m_videoRate    = videoRate;
		m_pIpInitialMode->SetVideoBitRate(videoRate, cmCapReceiveAndTransmit);
	}

	WORD status = statOK;
	return status;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
CIpComMode*	CH323PartyCntl::GetScmForVideoBridgeConnection(cmCapDirection direction)
{
	if ((direction == cmCapReceive) && (m_pIpInitialMode->GetConfType() == kCp))
		return m_pIpCurrentMode;
	else
		return m_pIpInitialMode;
}


//////////////////////////////////////////////////////////////////////////////
/* Description:
 The function change a specific media in m_pIpCurrentMode
 In case it is off, it might be that the channel is in connecting process or has already
 been connected, but m_pIpCurrentMode hasn't been updated yet. In that case, we only close
 the channels.
 Regarding the channels: Only if the initiator is partyControl, we close the channels.
   If the initiator is party, that means that the remote has already closed the channels.
   If the initiator is conf, that means thate channels will be close as part of the change mode mechanism.
 Regarding the bridges: The function change the bridge states according to direction, m_pIpCurrentMode, and changeModeState. */

void CH323PartyCntl::ChangeMode(eChangeModeState changeModeState, cmCapDataType dataType, cmCapDirection direction, ERoleLabel role, ePartyMediaState mediaState, CRsrcParams** avcToSvcTranslatorRsrcParams,CRsrcParams* pMrmpRsrcParams)
{
	TRACEINTO << "mix_mode: ";

	/*BYTE bBridgeChangeAudioReceiveMode  = FALSE;
	BYTE bBridgeChangeAudioTransmitMode = FALSE;

	if( (dataType == cmCapAudio) && m_isAudConn)
	{
		bBridgeChangeAudioReceiveMode = ( (direction & cmCapReceive) &&
						m_pIpCurrentMode->IsMediaOn(cmCapAudio, cmCapReceive) );

		bBridgeChangeAudioTransmitMode = ( (direction & cmCapTransmit) &&
						m_pIpCurrentMode->IsMediaOn(cmCapAudio, cmCapTransmit) );
	}*/

/*	BYTE bBridgeChangeVideoReceiveMode  = FALSE;
	BYTE bBridgeChangeVideoTransmitMode = FALSE;

	if ( (dataType == cmCapVideo) && (role == kRolePeople) && m_isVidConn)
	{
		bBridgeChangeVideoReceiveMode = (direction & cmCapReceive);

		bBridgeChangeVideoTransmitMode = ( (direction & cmCapTransmit) &&
						m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapTransmit) );
	}*/

/*	BYTE bBridgeChangeContentReceiveMode  = FALSE;
	BYTE bBridgeChangeContentTransmitMode = FALSE;

	if( (dataType == cmCapVideo) && (role & kRoleContentOrPresentation) )
	{
		if (m_isContentConn)
		{
			bBridgeChangeContentReceiveMode = ( (direction & cmCapReceive) &&
												m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) );

			bBridgeChangeContentTransmitMode = ( (direction & cmCapTransmit) &&
												m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation) );
		}
		else
			PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::ChangeMode : - the mode is content but m_isContentConn is off. Name - ",m_partyConfName);
	}*/

	/* Treatment towards the conf level (bridges): */
	//AUDIO:
/*	if (bBridgeChangeAudioReceiveMode)
	{
		m_pIpCurrentMode->SetMediaOff(cmCapAudio, cmCapReceive);//in audio the change is only disconnect this direction
		//mute the decoder
		if (m_isAudConn)
			m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOn, MCMS);
	}

	else if (bBridgeChangeAudioTransmitMode)
	{	//block the encoder:
		if(m_changeModeInitiator == ePartyInitiator)
			m_pIpCurrentMode->SetMediaOff(cmCapAudio, cmCapTransmit);//in audio the change is only disconnect this direction

		if (m_isAudConn)
			m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaOut, eOn, MCMS);
	}*/

	//VIDEO:

	if ((dataType == cmCapVideo) && (role == kRolePeople))
	{
		m_eUpdateState = eNoUpdate;
		BYTE bTakeInitial = TRUE;
		BYTE bForceUpdateIn = FALSE;
		if ((m_pIpInitialMode->GetConfType() == kCop) && IsChangeModeOfIncomingState(changeModeState) )
			bForceUpdateIn = TRUE; // even if there is no real change of the mode, we must return update video in as a response to change decoder
		m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial, bForceUpdateIn);
		DisconnectPartyFromVideoBridgeIfNeeded(m_pIpInitialMode);
	}

	//if (m_pIpInitialMode->GetConfType() == kCop && m_videoRate && m_eLastCopChangeModeType != eCop_EncoderIndex)
   // 	m_pIpInitialMode->SetVideoBitRate(m_videoRate,cmCapReceive);

	/*if (bBridgeChangeVideoReceiveMode)
	{
		if (bBridgeChangeVideoTransmitMode || m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit))
		//change both of the directions -  in COP, the bridge is responsible for the update
			DisconnectPartyFromVideoBridge();
		else //change only the receive
		{
			//if (m_pIpInitialMode->GetConfType() == kVideoSwitch) //VSW
			//	m_pVidConnection->VideoActive(FALSE,MCMS);//mute
		}
	}
	//if only bBridgeChangeVideoTransmitMode we do nothing!!*/

	// CONTENT:

	/*if (bBridgeChangeContentReceiveMode || bBridgeChangeContentTransmitMode )
	{	// disconnect content
		PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::ChangeMode : Set no content channel. Name - ",m_partyConfName);
		if (m_pContentBridge)
		{
			if (m_pContentBridge->IsConnected())
			{
				PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::ChangeMode : Disconnect content bridge. Name - ",m_partyConfName);
				OFF(m_isContentConn);
				m_pContentBridge->DisconnectParty(m_pParty);
			}
		}
		ON(m_bNoContentChannel);
	}*/

	// else if - when content will be added
	TRACEINTO << "m_changeModeInitiator=" << m_changeModeInitiator;
	if (m_changeModeInitiator == eConfInitiator) //result of change scm
	{

		DWORD numOfChangeModes = GetNumOfChangeModesInSec();
		TICKS curTicks = SystemGetTickCount();
		TICKS diff;
		diff = curTicks - m_TimeOfChangeModeInTicks;
	//	PTRACE2INT(eLevelInfoNormal,"CH323PartyCntl::ChangeMode: Party has content:noa dbg numOfChangeModes  ", numOfChangeModes);
		if(numOfChangeModes == 0 || diff.GetIntegerPartForTrace() > 100)
		{
			PTRACE2INT(eLevelInfoNormal,"CH323PartyCntl::ChangeMode: Party has content:noa dbg numOfChangeModes starting timer  ", numOfChangeModes);
			//StartTimer(CHANGE_MODE_LOOP,1 * SECOND);
			m_TimeOfChangeModeInTicks = curTicks;
			SetNumOfChangeModesInSec(0);
		}
		else if (numOfChangeModes > NUM_OF_CHANGE_MODES_PER_SECOND)
		{
			PTRACE2INT(eLevelError,"CH323PartyCntl::ChangeMode - loop of change modes numOfChangeModes, ",numOfChangeModes);
			SetNumOfChangeModesInSec( (numOfChangeModes + 1) );
			DBGPASSERT(6);
			BYTE 	mipHwConn = (BYTE)eMipNoneHw;
			BYTE	mipMedia = (BYTE)eMipNoneMedia;
			BYTE	mipDirect = 0;
			BYTE	mipTimerStat = 0;
			BYTE	mipAction = 0;
			CSegment* pSeg = new CSegment;
			*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
			DWORD MpiErrorNumber = 1;//GetMpiErrorNumber(pSeg);
			m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
			POBJDELETE(pSeg);
			return;
		}

		SetNumOfChangeModesInSec( (GetNumOfChangeModesInSec() + 1) );


		if ((m_pIpInitialMode->GetConfType() == kCop) /*&& (IsChangeIncomingMode(changeModeState))*/) // change decoder
		{
			if (IsValidTimer(COP_NO_VIDEO_UPDATES_TOUT))
				DeleteTimer(COP_NO_VIDEO_UPDATES_TOUT);
			StartTimer(COP_NO_VIDEO_UPDATES_TOUT, COP_NO_VIDEO_UPDATES_TIME);
			m_bSuspendVideoUpdates = TRUE;
			//m_pLocalCapH323->SetVideoCapsExactlyAccordingToScm(m_pIpInitialMode);
			m_pPartyApi->ChangeModeH323(m_pIpInitialMode, changeModeState, NULL, avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);
		}
		else if((m_pIpInitialMode->GetConfType() == kCp) && (mediaState == eChangeCpMode_FromCaps))  		// change CP mode from caps
			m_pPartyApi->ChangeModeH323(m_pIpInitialMode, changeModeState, m_pLocalCapH323, avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);
		else //vsw / CP with a specific scm
			m_pPartyApi->ChangeModeH323(m_pIpInitialMode, changeModeState, NULL, avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);
		m_bPartyEndChangeVideoInformParty = TRUE;

		StartTimer(CHANGETOUT,PARTYCNTL_CHANGEVIDEO_TIME);
		CSmallString str;
		str << "Send Change Mode To Party : Name - " << m_partyConfName << " Change mode state - " << GetChangeModeStateStr(changeModeState);
	    PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::ChangeMode - ",str.GetString(), GetPartyRsrcId());
	}

	else if (m_changeModeInitiator == ePartyControlInitiator) //result of empty capabilities set
		DisconnectChannel(dataType, direction, role);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::DisconnectChannel(cmCapDataType dataType, cmCapDirection direction, ERoleLabel role)
{
	if(m_pIpCurrentMode->IsMediaOn(dataType,direction,role))
	{
		m_pPartyApi->IpDisconnectMediaChannel(dataType,direction,role);
		m_pIpCurrentMode->SetMediaOff(dataType,direction,role);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsConfHasSameLineRateAsSourceConf()
{
	WORD origionalRate = GetOriginalConfRate();
	WORD newRate       = m_pIpInitialMode->GetCallRate();
	BYTE bRes = (origionalRate == newRate);
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CH323PartyCntl::IsDisconnectionBecauseOfNetworkProblems() const
{
	BOOL rval = FALSE;
	// ARQ GK failure
	if(m_disconnectionCause == H323_CALL_CLOSED_ARQTIMEOUT)
		rval = TRUE;

	// response to call setup
	else if((m_disconnectionCause == H323_CALL_CLOSED_REMOTE_BUSY) || 	/*(m_disconnectionCause == H323_CALL_CLOSED_REMOTE_REJECT) ||*/
			(m_disconnectionCause == H323_CALL_CLOSED_REMOTE_UNREACHABLE) ||
			(m_disconnectionCause == H323_CALL_CLOSED_REMOTE_REJECT))// when setup is not response by the EP
		rval = TRUE;
	// in case of failure with the connection to DH task.
	else if(m_disconnectionCause == A_COMMON_KEY_EXCHANGE_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE)
		rval = TRUE;

	BOOL bEnableredialForNotCloseCapEx = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_IP_REDIAL_FOR_NOT_FINISH_CAPS_EXCHANGE);

	if(bEnableredialForNotCloseCapEx && m_disconnectionCause == H323_CALL_CLOSED_REMOTE_HAS_NOT_SENT_CAPABILITY)
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::IsDisconnectionBecauseOfNetworkProblems - caps didn't finish redial! : Name - ",m_partyConfName, GetPartyRsrcId());
		rval = TRUE;
	}


	return rval;
}

/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323PartyCntl::IsVidModeIncludedInCaps() const
{
	BYTE bRes = TRUE;
	if (m_pIpInitialMode->GetConfType() == kVideoSwitch) //VSW
	{
		CapEnum protocol = eUnknownAlgorithemCapCode;
		if (m_pIpInitialMode->IsAutoVideoProtocol() == FALSE) //fixed protocol
			protocol = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit));

		DWORD rate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);

		bRes = m_pRmtCapH323->AreCapsSupportVideoProtocolAndRate(protocol, rate);
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323PartyCntl::IsPartyEncrypted() const
{
	BYTE bRes = TRUE;
	if(m_pLocalCapH323)
		bRes = m_pLocalCapH323->IsPartyEncrypted();
	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::SetDataForImportPartyCntl(CPartyCntl *apOtherPartyCntl)
{
    CIpPartyCntl::SetDataForImportPartyCntl(apOtherPartyCntl);

    CH323PartyCntl *pOtherPartyCntl = (CH323PartyCntl *)apOtherPartyCntl;

	m_bIsCascade				= pOtherPartyCntl->m_bIsCascade;
	m_masterSlaveStatus			= pOtherPartyCntl->m_masterSlaveStatus;
	m_bAdditionalRsrcActivated	= pOtherPartyCntl->m_bAdditionalRsrcActivated;
	m_bIsPartyConnectAllWhileMove = pOtherPartyCntl->m_bIsPartyConnectAllWhileMove;
	m_AddPartyStateBeforeMove	= pOtherPartyCntl->m_AddPartyStateBeforeMove;

	PTRACE2INT(eLevelInfoNormal, "CH323PartyCntl::OnPartyReceivedFaultyRsrc m_bIsVideoMuted:", m_bIsVideoMuted);
	if(m_bIsCascade != CASCADE_NONE)
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyReceivedFaultyRsrc: CASCADE_NONE copy m_bIsVideoMuted Name - ",m_partyConfName, GetPartyRsrcId());
		m_bIsVideoMuted = pOtherPartyCntl->m_bIsVideoMuted;
	}

	POBJDELETE(m_pRmtCapH323);
	if(pOtherPartyCntl->m_pRmtCapH323 == NULL){m_pRmtCapH323 = NULL;}
	else{m_pRmtCapH323 = new CCapH323(*(pOtherPartyCntl->m_pRmtCapH323));}

	if(pOtherPartyCntl->m_pH323NetSetup == NULL){m_pH323NetSetup = NULL;}
	else{
		POBJDELETE(m_pH323NetSetup);
		m_pH323NetSetup = new CH323NetSetup;
		*m_pH323NetSetup = *(pOtherPartyCntl->m_pH323NetSetup);
	}

	if(pOtherPartyCntl->m_pLocalCapH323 == NULL){m_pLocalCapH323 = NULL;}
	else{m_pLocalCapH323 = new CCapH323(*(pOtherPartyCntl->m_pLocalCapH323));}

	//Multiple links for ITP in cascaded conference feature:
	if (CPObject::IsValidPObjectPtr(m_telepresenseEPInfo))
	    POBJDELETE(m_telepresenseEPInfo);
	if (pOtherPartyCntl->m_telepresenseEPInfo != NULL)
	{
	    PTRACE2(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::SetDataForImportH323PartyCntl name:",m_name);
	    m_telepresenseEPInfo  = new CTelepresenseEPInfo();
	    *m_telepresenseEPInfo = *(pOtherPartyCntl->m_telepresenseEPInfo);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnPartyReceivedFaultyRsrc(CSegment* pSeg)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyReceivedFaultyRsrc: Name - ",m_partyConfName, GetPartyRsrcId());
	m_isFaulty = 1;
	DWORD reason;
	*pSeg >> reason;
	if (reason == STATUS_FAIL)
	{
		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
        CSmallString str;
        str << "CH323PartyCntl::OnPartyReceivedFaultyRsrc party:" << GetPartyRsrcId();
        PASSERTMSG(MpiErrorNumber, str.GetString());
	}
	else
		m_pTaskApi->PartyDisConnect(reason,m_pParty);
}

////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnPartyPresentationOutStreamUpdateAnycase(CSegment* pSeg)
{
    PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyPresentationOutStreamUpdateAnycase: Name - ",m_partyConfName, GetPartyRsrcId());
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::OnPartyH323Connect(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnPartyH323Connect : Name - ",m_partyConfName, GetPartyRsrcId());

	WORD status;
	WORD dummyMasterSlaveStatus;  // We deserialize this parameter but don't use it to update the m_masterSlaveStatus member in middle of call.

	*pParam >> status>> m_bIsCascade >> dummyMasterSlaveStatus >> m_incomingVideoChannelHandle;
	TRACEINTO << "mix_mode: channel handle set to "  << m_incomingVideoChannelHandle;

	PartyConnectedAudio(pParam, status);

	if(m_pIpInitialMode->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople) && !m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople))
	{
		m_pIpInitialMode->SetMediaMode(m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapReceive,kRolePeople), cmCapVideo,cmCapReceive,kRolePeople);
		ConnectPartyToVideoBridge(m_pIpCurrentMode);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::OnUpdatePartyH323VideoBitRate(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnUpdatePartyH323VideoBitRate : Name - ",m_partyConfName, GetPartyRsrcId());

	WORD status;
	DWORD newBitRate;
	WORD channelDirection;
	WORD roleLabel;

	*pParam >> newBitRate;
	*pParam >> channelDirection;
	*pParam >> roleLabel;

	if ( ((cmCapDirection)channelDirection == cmCapTransmit) && ((ERoleLabel)roleLabel == kRolePeople) )
	{
		if ((m_pIpInitialMode->GetConfType() == kVideoSwitch) ||
            (m_pIpInitialMode->GetConfType() == kVSW_Fixed) ||
            (m_pIpInitialMode->GetConfType() == kCop) )
		{
			// VSW or Cop:
			PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnUpdatePartyH323VideoBitRate : Name - ",m_partyConfName, GetPartyRsrcId());
			m_pIpCurrentMode->SetFlowControlRateConstraint(newBitRate);
			UpdateBridgeFlowControlRateIfNeeded();
		}
		else if (m_pIpInitialMode->GetConfType() == kCp)
		{
			// CP:
			CSmallString str;
			str << "cp conf, rate - " << newBitRate;
			m_eUpdateState = eNoUpdate;
			m_pIpInitialMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnUpdatePartyH323VideoBitRate: ",str.GetString(), GetPartyRsrcId());
			//*m_pIpInitialMode = *m_pIpCurrentMode;
			//	m_pIpInitialMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			//*m_pInitialModeH323 = *m_pCurrentModeH323;
			BYTE bTakeInitial = TRUE;
			//m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial);
			m_eUpdateState = UpdateVideoBridgeOutIfNeeded(bTakeInitial);

			if(m_eUpdateState != eNoUpdate)
			{

				PTRACEPARTYID(eLevelInfoNormal,"CH323PartyCntl::OnUpdatePartyH323VideoBitRate Update Video Out Bit Rate", GetPartyRsrcId());
				m_pIpCurrentMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);

			}
		}
		else
		{	// COP:
			PTRACE(eLevelError,"CH323PartyCntl::OnUpdatePartyH323VideoBitRate: Need implementation for COP");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnUpdatePartyH323LprVideoBitRate(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnUpdatePartyH323LprVideoBitRate : Name - ",m_partyConfName, GetPartyRsrcId());

	WORD status;
	DWORD newBitRate;
	WORD channelDirection;
	DWORD lossProtection;
	DWORD mtbf;
	DWORD congestionCeiling;
	DWORD fill;
	DWORD modeTimeout;
    DWORD totalVideoRate;

	*pParam >> newBitRate;
	*pParam >> channelDirection;
	*pParam >> lossProtection;
	*pParam >> mtbf;
	*pParam >> congestionCeiling;
	*pParam >> fill;
	*pParam >> modeTimeout;
    *pParam >> totalVideoRate;

	if ( (cmCapDirection)channelDirection == cmCapTransmit )
	{
		if (newBitRate == m_videoRate)
			m_isLprActive = 0;
		else
			m_isLprActive = 1;


		if ((m_pIpInitialMode->GetConfType() == kVideoSwitch) ||
            (m_pIpInitialMode->GetConfType() == kVSW_Fixed) ||
            (m_pIpInitialMode->GetConfType() == kCop))
		{
			// VSW or Cop:
			PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnUpdatePartyH323LprVideoBitRate : VSW conf. Name - ",m_partyConfName, GetPartyRsrcId());
			m_pIpCurrentMode->SetFlowControlRateConstraint(newBitRate);

		/*	lPRModeChangeParams lprModeChangeData;
			lprModeChangeData.lossProtection = lossProtection;
			lprModeChangeData.mtbf = mtbf;
			lprModeChangeData.congestionCeiling = congestionCeiling;
			lprModeChangeData.fill = fill;
			lprModeChangeData.modeTimeout = modeTimeout;
		*/
			CLPRParams* pLprParams = new CLPRParams(lossProtection,mtbf,congestionCeiling,fill,modeTimeout);
			UpdateBridgeFlowControlRateIfNeeded(pLprParams);

			POBJDELETE(pLprParams);

		}
		else
		{
			// CP:
            m_pIpCurrentMode->SetTotalVideoRate(totalVideoRate);
        	m_pIpInitialMode->SetTotalVideoRate(totalVideoRate);
			m_eUpdateState = eNoUpdate;
			m_pIpInitialMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			//*m_pIpInitialMode = *m_pIpCurrentMode;
			//	m_pIpInitialMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			//*m_pInitialModeH323 = *m_pCurrentModeH323;
			BYTE bTakeInitial = TRUE;
			m_eUpdateState = UpdateLprVideoOutBridgeRate(lossProtection, mtbf, congestionCeiling, fill, modeTimeout, bTakeInitial);

			if(m_eUpdateState != eNoUpdate)
			{
				PTRACEPARTYID(eLevelInfoNormal,"CH323PartyCntl::OnUpdatePartyH323LprVideoBitRate Update Video Out Bit Rate", GetPartyRsrcId());
				m_pIpCurrentMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			}
			else
			{
				PTRACE (eLevelInfoNormal,"CH323PartyCntl::OnUpdatePartyH323LprVideoBitRate : LPR not updated on bridge, continuing");

				CSegment* pSeg = new CSegment;
				*pSeg << lossProtection << mtbf << congestionCeiling << fill << modeTimeout;

				DispatchEvent(PARTY_LPR_VIDEO_OUT_RATE_UPDATED,pSeg);
				POBJDELETE(pSeg);

			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::UpdateBridgeFlowControlRateIfNeeded(CLPRParams* pLPRParams)
{
	DWORD flowControlRate = m_pIpCurrentMode->GetFlowControlRateConstraint();

	if (((m_pIpInitialMode->GetConfType() == kVideoSwitch) ||
         (m_pIpInitialMode->GetConfType() == kVSW_Fixed) ||
         (m_pIpInitialMode->GetConfType() == kCop))
		&& IsOutDirectionConnectedToVideoBridge()
		&& flowControlRate)
	{
		CSmallString str;
		str << flowControlRate;
		PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::UpdateBridgeFlowControlRateIfNeeded - ", str.GetString(), GetPartyRsrcId());
		CSegment* pSeg = NULL;
		if (pLPRParams != NULL)
		{
			pSeg = new CSegment;
			*pSeg << (DWORD)pLPRParams;
		}

		if (m_pParty)
			m_pTaskApi->UpdateVideoBridgeFlowControlRate(((CParty*)m_pParty)->GetPartyRsrcID(), flowControlRate, cmCapTransmit, kRolePeople, m_bIsCascade == CASCADE_MCU,  pSeg);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnCleanVideoRateLimitation()
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnCleanVideoRateLimitation. Name - ",m_partyConfName, GetPartyRsrcId());
	m_pIpCurrentMode->SetFlowControlRateConstraint(0);
	// Since this function is called while video out channel was closed,
	// there in not needed to update the bridge because the party has disconnected already from bridge.
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::UpdateH264ModeInLocalCaps(H264VideoModeDetails h264VidModeDetails,ERoleLabel eRole)
{
	m_pLocalCapH323->SetLevelAndAdditionals(h264VidModeDetails,-1,eRole);

	COstrStream msg;//temp
	m_pLocalCapH323->Dump(msg); //temp
	PTRACEPARTYID(eLevelInfoNormal, msg.str().c_str(), GetPartyRsrcId()); //temp
}

//////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CH323PartyCntl::GetLocalCapsMbps(ERoleLabel eRole)
{
	WORD mbps = 0, fs = 0, dpb = 0, brAndCpb = 0, sar = 0, staticMB = 0;
	mbps = fs = dpb = brAndCpb = sar = staticMB = 0;
	if(m_pLocalCapH323)
		m_pLocalCapH323->GetMaxH264CustomParameters(m_pLocalCapH323->GetMaxH264Level(eRole), mbps, fs, dpb, brAndCpb, sar, staticMB, eRole);
	if(mbps == 0)
		return INVALID;
	return mbps;
}
//////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::GetRemoteCapsParams( WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile )
{
	if(m_pRmtCapH323)
	{
		//m_pRmtCapH323->Dump("CH323PartyCntl::GetRemoteCapsParams m_pRmtCapH323 TMP ",eLevelInfoNormal);
		m_pRmtCapH323->GetMaxH264CustomParameters(m_pRmtCapH323->GetMaxH264Level(eRole), maxMBPS, maxFS, maxDPB, maxBRandCPB, maxSAR, maxStaticMB, eRole,profile);
	}

}


//////////////////////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::Disable4CifInLocalCaps()
{
	PTRACE(eLevelInfoNormal,"CH323PartyCntl::Disable4CifInLocalCaps  ");
    if (CPObject::IsValidPObjectPtr (m_pLocalCapH323))
    {
       // m_pLocalCapH323->Set4CifMpi ((APIS8)-1);
    //	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
    //	 if(systemCardsBasedMode != eSystemCardsMode_mpm)//only in mpm we allow asymetric h2634cif-only transmit
    //	 {
       m_pLocalCapH323->Set4CifMpi ((APIS8)-1);
    //	 }
        m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePeople);
    //    m_pLocalCapH323->SetH263FormatMpi(k4Cif, -1, kRolePresentation);
    }
    else
        DBGPASSERT (4); // 4 - to signal 4cif :)

  //  COstrStream msg;
  //  m_pLocalCapH323->Dump(msg);
   // PTRACE2(eLevelInfoNormal,"CH323PartyCntl::Disable4CifInLocalCaps ", msg.str().c_str());
}
/////////////////////////////////////////////////////
void CH323PartyCntl::RemoveH263H261FromLocalCaps()
{
	PTRACE(eLevelInfoNormal,"CH323PartyCntl::RemoveH263H261FromLocalCaps  ");
	if (CPObject::IsValidPObjectPtr (m_pLocalCapH323))
	 {
	     m_pLocalCapH323->RemovePeopleCapSet(eH263CapCode);
	     m_pLocalCapH323->RemovePeopleCapSet(eH261CapCode);

	 }
   else
	    DBGPASSERT (3); // 3 - to signal h263 :)

}


//////////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::SetPartyToAudioOnly()
{
	// set local caps to audio only
	POBJDELETE(m_pLocalCapH323);
	m_pLocalCapH323		= new CCapH323;
	m_pLocalCapH323->CreateAudioOnlyCap(m_videoRate, m_pIpInitialMode, m_name);

	// set local caps to audio only
	CIpPartyCntl::SetPartyToAudioOnly();
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::AreLocalCapsSupportMedia(cmCapDataType eDataType)
{
 	return m_pLocalCapH323->OnType(eDataType);
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323PartyCntl::IsPartyMasterOrSlaveNotLecturer()
{
   if(CASCADE_MODE_MASTER == m_masterSlaveStatus)
   		return TRUE;
   else
   {
	   CLectureModeParams* pLectureModeParams =( (CCommConf*) m_pConf->GetCommConf() )->GetLectureMode();
	   BYTE lectureModeOnOff = pLectureModeParams->GetLectureModeType();
	   if(lectureModeOnOff)
	   {
		   const char* lecturerName =pLectureModeParams->GetLecturerName();
		   if(lecturerName!=NULL &&  strcmp(lecturerName,""))
		   	{
			   if( !strncmp(m_name,lecturerName,H243_NAME_LEN) )
			   {
					   PTRACE2(eLevelInfoNormal,"IsPartyMasterorSlaveNotLecturer -slave but lecturer",m_name);
					   return FALSE;
			   }
		   	}

	   }
	  if (CASCADE_NONE == m_bIsCascade)
		  return FALSE;

   	  return TRUE;
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323PartyCntl::IsPartyCascadeWithCopMcu() const
{
    if (CASCADE_NONE == m_bIsCascade)
        return FALSE;
    else
    {
    	if(( strstr(m_VersionId, "V4.6") && strstr(m_productId, "RMX") ) || m_IsCascadeToCopMcu || (strstr(m_productId, "RMX1000") &&  !strstr(m_productId, "ACCORD MGC / Polycom RMX1000_2000")) )
    	{
    		PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::IsPartyCascadeWithCopMcu - TRUE : Name - ",m_partyConfName, GetPartyRsrcId());
    		return TRUE;
    	}
    	else
    		return FALSE;
    }

}
/////////////////////////////////////////////////////////////////
BYTE  CH323PartyCntl::IsPartyCascade() const
{
    if (CASCADE_NONE == m_bIsCascade)
        return FALSE;
    else
        return TRUE;
}
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
ECascadePartyType  CH323PartyCntl::GetPartyCascadeTypeAndVendor()
{
	if (CASCADE_MODE_MASTER == m_masterSlaveStatus)
	{
		if(strstr(m_productId, "RMX1000") && !strstr(m_productId, "ACCORD MGC / Polycom RMX1000_2000") )
			return eCascadeMasterToRmx1000;
		else if(strstr(m_productId, "RMX"))
			return eCascadeMasterToRmx;
		else if(strstr(m_productId, "MGC"))
			return eCascadeMasterToMgc;
		else
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::GetPartyCascadeType -party is master to non polycom mcu or to an EP setting as not cascade : Name - ",m_partyConfName, GetPartyRsrcId());
			return eCascadeNone;
		}
	}
	else if(CASCADE_MODE_SLAVE == m_masterSlaveStatus)
	{
		//if(strstr(m_productId, "RMX1000") && !strstr(m_productId, "ACCORD MGC / Polycom RMX1000_2000") )
			//return eCascadeSlaveToRmx1000NotSupportSmartCascade;
		if(strstr(m_productId, "RMX"))
		{
			if(( strstr(m_productId, "RMX1000") || strstr(m_productId, "RMX500")) && !strstr(m_productId, "ACCORD MGC / Polycom RMX1000_2000") && m_IsCascadeToCopMcu )
				return eCascadeSlaveToRmx1000SupportSmartCascade;
			else if(( strstr(m_productId, "RMX1000") || strstr(m_productId, "RMX500")) && !strstr(m_productId, "ACCORD MGC / Polycom RMX1000_2000") && !m_IsCascadeToCopMcu )
				return eCascadeSlaveToRmx1000NotSupportSmartCascade;
			else if(strstr(m_VersionId, "V4.6") || m_IsCascadeToCopMcu)
				return eCascadeSlaveToRmxSupportSmartCascade;
			else
				return eCascadeSlaveToRmxNotSupportSmartCascade;
		}
		else if(strstr(m_productId, "MGC"))
			return eCascadeSlaveToMgcNotSupportSmartCascade;
		else
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::GetPartyCascadeType -party is slave to non polycom mcu setting as not cascade : Name - ",m_partyConfName, GetPartyRsrcId());
			return eCascadeNone;
		}

	}
	else
	{
		DBGPASSERT(m_masterSlaveStatus);
		return eCascadeNone;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnLprVideoOutBrdgBitRateUpdated(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnLprVideoOutBrdgBitRateUpdated : Name - ",m_partyConfName, GetPartyRsrcId());

	DWORD lossProtection;
	DWORD mtbf;
	DWORD congestionCeiling;
	DWORD fill;
	DWORD modeTimeout;
	WORD status = statOK;

	*pParam >> lossProtection;
	*pParam >> mtbf;
	*pParam >> congestionCeiling;
	*pParam >> fill;
	*pParam >> modeTimeout;

	m_pPartyApi->SetLprModeForVideoOutChannels(status, lossProtection, mtbf, congestionCeiling, fill, modeTimeout);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::OnAddSubLinksParties
void  CH323PartyCntl::OnAddSubLinksParties(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnAddSubLinksParties - ERROR - we should get to CH323AddPartyCntl");
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::DisconnectMainLink (then disconnect the other sub links)
void  CH323PartyCntl::DisconnectMainLink()
{
    //disconnect main link -> then disconnect the other sub links:
    if (m_linkType == eMainLinkParty)
    {
        PTRACEPARTYID(eLevelError,"ITP_CASCADE: CH323PartyCntl::DisconnectMainLink - ERROR! -> disconnect MainLink:", GetPartyRsrcId());
        //PASSERTMSG(GetPartyRsrcId(),"ITP_CASCADE: CH323PartyCntl::DisconnectMainLink - ERROR! -> disconnect MainLink:");

        BYTE    mipHwConn    = (BYTE)eMipNoneHw;
        BYTE    mipMedia     = (BYTE)eMipNoneMedia;
        BYTE    mipDirect    = (BYTE)eMipNoneDirction;
        BYTE    mipTimerStat = (BYTE)eMpiNoTimerAndStatus;
        BYTE    mipAction    = (BYTE)eMipNoAction;

        CSegment* pSeg = new CSegment;
        *pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
        DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
        POBJDELETE(pSeg);

        if (m_pTaskApi && m_pParty)
            m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
        else
        {
            PTRACE(eLevelError, "ITP_CASCADE: CH323PartyCntl::DisconnectMainLink ERROR - m_pTaskApi or m_pParty is NULL - cann't disconnect the call");
            //PASSERTMSG(1,"ITP_CASCADE: CH323PartyCntl::DisconnectMainLink ERROR - m_pTaskApi or m_pParty is NULL - cann't disconnect the call");
        }
    }
    else
    {
        PTRACEPARTYID(eLevelError,"ITP_CASCADE: CH323PartyCntl::DisconnectMainLink - ERROR! not MainLink! PartyRsrcId:", GetPartyRsrcId());
        PASSERTMSG(GetPartyRsrcId(),"ITP_CASCADE: CH323PartyCntl::DisconnectMainLink - ERROR! not MainLink! PartyRsrcId:");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::OnConnectLinkTout
void  CH323PartyCntl::OnConnectLinkTout(CSegment* pParam)
{
    DisconnectMainLink();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::OnMainPartyUpdateITPSpeaker //shiraITP - 93
void CH323PartyCntl::OnMainPartyUpdateITPSpeaker(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker");

    DWORD numOfActiveLinks;
    BYTE  itpType;
    BOOL  isFinishedToUpdateITPSpeaker = FALSE;

    *pParam >> numOfActiveLinks;
    *pParam >> itpType;

    PTRACE2INT(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker -numOfActiveLinks:",numOfActiveLinks);
    PTRACE2INT(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker -itpType:",itpType);

    if (m_roomControl != NULL)
    {
        isFinishedToUpdateITPSpeaker = m_roomControl->SetTelepresenceTypeAndIsActiveField((eTelePresencePartyType) itpType,numOfActiveLinks);
    }
    else
    {
        //PTRACE(eLevelError,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker -ERROR- m_roomControl is NULL!");
        PASSERTMSG(1,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker -ERROR- m_roomControl is NULL!");
    }

    if (isFinishedToUpdateITPSpeaker == TRUE)
    {
        PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker -isFinishedToUpdateITPSpeaker == TRUE");

        //need to update VB:
        if ( m_pVideoBridgeInterface && (m_eVidBridgeConnState != eBridgeDisconnected) && (m_eVidBridgeConnState & eOutConnected) )
        {
            PTRACE2PARTYID(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker : Send to bridge. Name - ",m_partyConfName, GetPartyRsrcId());
            m_pVideoBridgeInterface->UpdateNewSpeakerIndReceivedFromRemoteMCU(GetPartyRsrcId(),numOfActiveLinks,itpType);
        }
        else
            PTRACE2PARTYID(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker : Bridge is not connected. Name - ",m_partyConfName, GetPartyRsrcId());

        //create and send ACK:
        if (m_pPartyApi)
            m_pPartyApi->CreateNewITPSpeakerAckReq(); //shiraITP - 94
        else
        {
            PTRACE(eLevelError, "ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker ERROR - m_pPartyApi is NULL - DONT CreateNewITPSpeakerAckReq");
            //PASSERTMSG(1,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker ERROR - m_pPartyApi is NULL - DONT CreateNewITPSpeakerAckReq");
        }
    }
    else
    {
        PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeaker -ERROR- DONT SEND ACK isFinishedToUpdateITPSpeaker == FALSE");
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::OnMainPartyUpdateITPSpeakerAck //shiraITP - 114
void CH323PartyCntl::OnMainPartyUpdateITPSpeakerAck(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeakerAck");

    //need to update VB:
    if ( m_pVideoBridgeInterface && (m_eVidBridgeConnState != eBridgeDisconnected) && (m_eVidBridgeConnState & eOutConnected) )
    {
        PTRACE2PARTYID(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeakerAck : Send to bridge. Name - ",m_partyConfName, GetPartyRsrcId());
        m_pVideoBridgeInterface->UpdateNewSpeakerAckIndReceivedFromRemoteMCU(GetPartyRsrcId());
    }
    else
        PTRACE2PARTYID(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartyUpdateITPSpeakerAck : Bridge is not connected. Name - ",m_partyConfName, GetPartyRsrcId());

}
//////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::OnMainPartySendNewITPSpeaker  //shiraITP - 100
void  CH323PartyCntl::OnMainPartySendNewITPSpeaker(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl::OnMainPartySendNewITPSpeaker");

    WORD status;
	BOOL isParams;
    DWORD numOfActiveLinks = 0;
    BYTE  itpType = 0;

	*pParam >> status >> isParams;
	if ((status == STATUS_OK) && isParams)
	{
		*pParam >> (DWORD&)numOfActiveLinks;
		*pParam >> (BYTE&)itpType;
	}
	else
		PASSERTMSG_AND_RETURN(TRUE, "invalid params for NEW_ITP_SPEAKER_IND");

    TRACESTR(eLevelInfoNormal) << "ITP_CASCADE: CH323PartyCntl::OnMainPartySendNewITPSpeaker - numOfActiveLinks: " << numOfActiveLinks << " itpType:" << (WORD) itpType;

    if (m_pPartyApi)
        m_pPartyApi->CreateNewITPSpeakerReq(numOfActiveLinks,itpType);   //shiraITP - 101
    else
    {
        PTRACE(eLevelError, "ITP_CASCADE: CH323PartyCntl::OnMainPartySendNewITPSpeaker ERROR - m_pPartyApi is NULL - DONT CreateNewITPSpeakerReq");
        //PASSERTMSG(1,"ITP_CASCADE: CH323PartyCntl::OnMainPartySendNewITPSpeaker ERROR - m_pPartyApi is NULL - DONT CreateNewITPSpeakerReq");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void  CH323PartyCntl::OnPartyUpdateLocalCaps(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323PartyCntl::UpdateLocalCaps!!!", GetPartyRsrcId());
	m_pLocalCapH323->DeSerialize(NATIVE,*pParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::IsPartyCapsContainsH264SCM(const CMediaModeH323* H323ContentMode,ERoleLabel role)
{
	//Old code
	//BYTE bRes = (m_pRmtCapH323->IsContaining(*H323ContentMode, kCapCode|kBitRate|kFormat|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS, cmCapReceive, role));
	DWORD valuesToCompare = 0;

	if(strstr(GetProductId(), "CMA Desktop")/* &&
	   strstr(GetVersionId(), "5.0")*/) {

		valuesToCompare = kCapCode;
	}
	else
	{
		valuesToCompare = kCapCode|kBitRate|kFormat|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode;
	}

	BYTE bRes = (m_pRmtCapH323->IsContaining(*H323ContentMode, valuesToCompare, cmCapReceive, role));

	return bRes;
}
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL CH323PartyCntl::IsNeedToChangeResAccordingToRemoteRevision()
{
	BYTE res = FALSE;

	PTRACE(eLevelInfoNormal,"CH323PartyCntl::IsNeedToChangeResAccordingToRemoteRevision ");
	APIU16 profile;
	APIU8 level;
	long dpb,Mbps,fs,brAndCpb, sar, staticMB;
	m_pIpCurrentMode->GetH264Scm(profile, level, Mbps, fs, dpb, brAndCpb, sar, (long&)staticMB, cmCapTransmit);

	//To identify :
	//HDX 8000 Rev B - Cap supported HD1080p30 or HD720p60
	//HDX 7000 Rev C - Cap supported HD1080p30 or HD720p60
	if(strstr(m_productId,"HDX 800") || (strstr(m_productId,"HDX 700")))
	{
		CComModeH323* pScmWithHD1080= new CComModeH323;
		*pScmWithHD1080 = *m_pIpCurrentMode;
		pScmWithHD1080->SetScmToHdCp(eHD1080Res, cmCapReceiveAndTransmit);
		const CMediaModeH323& rH264ModeHD1080 = pScmWithHD1080->GetMediaMode(cmCapVideo,cmCapTransmit,kRolePeople);
		BOOL IsSupportHD1080 = m_pRmtCapH323->IsContaining(rH264ModeHD1080, kCapCode|kBitRate|kFormat|kAnnexes|kH264Profile|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, cmCapReceive, kRolePeople);

		CComModeH323* pScmWithHD720At60 = new CComModeH323;
		*pScmWithHD720At60 = *m_pIpCurrentMode;
		pScmWithHD720At60->SetScmToCpHD720At60(cmCapReceiveAndTransmit);
		const CMediaModeH323& rH264ModeHD720At60 = pScmWithHD720At60->GetMediaMode(cmCapVideo,cmCapTransmit,kRolePeople);
		BOOL IsSupportHD720At60 = m_pRmtCapH323->IsContaining(rH264ModeHD720At60, kCapCode|kBitRate|kFormat|kAnnexes|kH264Profile|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, cmCapReceive, kRolePeople);

		if(IsSupportHD1080 ||IsSupportHD720At60)
		{
			PTRACE(eLevelInfoNormal,"CH323PartyCntl::IsNeedToChangeResAccordingToRemoteRevision - HDX 8000 Rev B or HDX 7000 Rev C- Need to change");
			res = TRUE;
		}

		POBJDELETE(pScmWithHD720At60);
		POBJDELETE(pScmWithHD1080);
	}
	if(!res)
	{
		//to identify HDX 4000/7000/9000 SD -
		// HD - cap support HD720p30
		// SD - cap support 4CIF30
		if((strstr(m_productId,"HDX 400"))||(strstr(m_productId,"HDX 700"))||(strstr(m_productId,"HDX 900")))
		{
			CComModeH323* pScmWithHD720At30= new CComModeH323;
			*pScmWithHD720At30 = *m_pIpCurrentMode;
			pScmWithHD720At30->SetScmToHdCp(eHD720Res, cmCapReceiveAndTransmit);
			const CMediaModeH323& rH264ModeHD720At30 = pScmWithHD720At30->GetMediaMode(cmCapVideo,cmCapTransmit,kRolePeople);
			BOOL IsSupportHD720At30 = m_pRmtCapH323->IsContaining(rH264ModeHD720At30, kCapCode|kBitRate|kFormat|kAnnexes|kH264Profile|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, cmCapReceive, kRolePeople);

			if(!IsSupportHD720At30)
			{
				PTRACE(eLevelInfoNormal,"CH323PartyCntl::IsNeedToChangeResAccordingToRemoteRevision - HDX 4000/7000/9000 SD - Need to change");
				res = TRUE;
			}
			POBJDELETE(pScmWithHD720At30);
		}
	}

	return res;

}


/////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CH323PartyCntl::GetMaxFsAccordingtoProfile(APIU16 profile)
{
	DWORD maxFs = m_pRmtCapH323->GetMaxFsAccordingToProfile(profile);
	return maxFs;

}
//////////////////////////////////////////////////////////////////
BYTE CH323PartyCntl::CreateNewModeForCopCascadeLecturerLink(CIpComMode* pH323Scm)
{

	//TBD what to do in case level is not valid
	BYTE isfoundMatch = FALSE;
	CComModeH323* pScm = new CComModeH323(*m_pIpInitialMode); AUTO_DELETE(pScm);
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if (!pConfParty)
	{
	    PTRACE(eLevelError,"CH323PartyCntl::CreateNewModeForCopCascadeLecturerLink - pConfParty is NULL!!!");
	    return isfoundMatch;
	}
	DWORD definedMaxRate= pConfParty->GetVideoRate();
	isfoundMatch = m_pRmtCapH323->FindBestVidTxModeForCopLecturerLink(m_pCopVideoTxModes, pScm,m_copResourceIndexOfCascadeLinkLecturer ,pConfParty->GetVideoProtocol(), definedMaxRate);
	if(isfoundMatch)
	{
		PTRACE(eLevelInfoNormal,"CH323PartyCntl::CreateNewModeForCopCascadeLecturerLink");
		//m_pIpInitialMode->SetMediaMode(pScm->GetMediaMode(cmCapVideo, cmCapTransmit), cmCapVideo, cmCapTransmit);
		pH323Scm->SetMediaMode(pScm->GetMediaMode(cmCapVideo, cmCapTransmit), cmCapVideo, cmCapTransmit);
	}
	return isfoundMatch;
}

//////////////////////////////////////////////////////////////////////////
//in this function we want to check if  there is a need in change mode out if not still need to update out
//////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::CopVideoBridgeChangeLinkLectureModeOut(CIpComMode* pNewScm)
{
	if(m_bCascadeIsLecturer)
	{
		CreateNewModeForCopCascadeLecturerLink(pNewScm);
	}
	else
	{
		pNewScm->SetCopTxLevel(INVALID_COP_LEVEL);
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
		if (pConfParty)
		{
		    DWORD definedMaxRate= pConfParty->GetVideoRate();
		    m_pRmtCapH323->FindBestVidTxModeForCop(m_pCopVideoTxModes, pNewScm, pConfParty->GetVideoProtocol(), definedMaxRate);
		}
		else
		      PTRACE(eLevelError,"CH323PartyCntl::CopVideoBridgeChangeLinkLectureModeOut - pConfParty is NULL!!!");
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::SaveLecturerLinkCopLevelAccordingToCurrent()
{
	if(m_pIpInitialMode->GetConfType() != kCop)
	{
		PTRACE(eLevelError,"CH323PartyCntl::SaveLecturerLinkCopLevelAccordingToCurrent not in cop mode");
		return;
	}
	ECascadePartyType CascadeType = GetPartyCascadeTypeAndVendor();
	if(CascadeType == eCascadeSlaveToRmx1000SupportSmartCascade || CascadeType == eCascadeSlaveToRmxSupportSmartCascade)
	{
		BYTE copLevel = m_pIpCurrentMode->GetCopTxLevel();
		if(copLevel != INVALID_COP_LEVEL && m_copResourceIndexOfCascadeLinkLecturer == INVALID_COP_LEVEL )
		{
			m_copResourceIndexOfCascadeLinkLecturer = copLevel;
			PTRACE2INT(eLevelInfoNormal,"CH323PartyCntl::SaveLecturerLinkCopLevelAccordingToCurrent this is lecturer link level, ",m_copResourceIndexOfCascadeLinkLecturer);
		}
		else
		{
			PTRACE(eLevelError,"CH323PartyCntl::SaveLecturerLinkCopLevelAccordingToCurrent TX LEVEL IS NOT VALID");
		}

	}

}
void CH323PartyCntl::PartySendMuteVideo(BYTE isActive)
{
	if( isActive == NO )
	   SetIsVideoMuted(YES);
	else
	   SetIsVideoMuted(NO);

	m_pPartyApi->PartySendMuteVideo(isActive);
}
////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::OnAddSubLinkToRoomControl
void  CH323PartyCntl::OnAddSubLinkToRoomControl(const char* name, DWORD indexOfLink, eTypeOfLinkParty type, DWORD mailbox)
{
    PTRACE2(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl:OnAddSubLinkToRoomControl main nameOfParty:",m_name);
    BOOL fullyConnected = FALSE;
    if (m_roomControl != NULL)
    {
        BOOL fullyConnected =  m_roomControl->AddLink(name,indexOfLink,type,mailbox);
        if (fullyConnected == TRUE)
        {
            PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl:OnAddSubLinkToRoomControl - room is fullyConnected");
            DeleteTimer(CONNECTLINKTOUT);
        }
    }
    else
    {
        PTRACE(eLevelError,"ITP_CASCADE: CH323PartyCntl:OnAddSubLinkToRoomControl -ERROR- m_roomControl is NULL!");
        DisconnectMainLink();
    }
}
/////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::OnRemoveSubLinkFromRoomControl
void  CH323PartyCntl::OnRemoveSubLinkFromRoomControl(const char* name, DWORD indexOfLink)
{
    //PTRACE2(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl:OnRemoveSubLinkFromRoomControl nameOfParty:",m_name);
    if (m_roomControl != NULL)
    {
        PTRACE(eLevelError,"ITP_CASCADE: CH323PartyCntl:OnAddSubLinkToRoomControl - m_roomControl is not NULL!");
        m_roomControl->RemoveLink(name,indexOfLink);
    }
    else
    {
        PTRACE(eLevelError,"ITP_CASCADE: CH323PartyCntl:OnRemoveSubLinkFromRoomControl -ERROR- m_roomControl is NULL!");
        //PASSERTMSG(1,"ITP_CASCADE: CH323PartyCntl:OnRemoveSubLinkFromRoomControl -ERROR- m_roomControl is NULL!");
    }
}
////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323PartyCntl::IsSubPartyConnected
BOOL CH323PartyCntl::IsSubPartyConnected(const char* name, DWORD indexOfLink)
{
    PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl:IsSubPartyConnected");

    BOOL isSubPartyConnected = FALSE;

    if (m_roomControl != NULL)
    {
        isSubPartyConnected = m_roomControl->IsSubPartyConnected(name,indexOfLink);
    }
    else
    {
        PTRACE(eLevelInfoNormal,"ITP_CASCADE: CH323PartyCntl:IsSubPartyConnected - m_roomControl is NULL!");
    }

    return isSubPartyConnected;
}

void CH323PartyCntl::OnPartyCntlFirRequestByEP(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal,"OnPartyCntlFirRequestByEP");

    m_pVideoBridgeInterface->RelayEpAskForIntra(pParam);
}

////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::UpdateLocalCapsForHdVswInMixMode(const VideoOperationPoint *pVideoOperationPoint)
{
    m_pLocalCapH323->UpdateCapsForHdVswInMixedMode(m_pIpInitialMode, pVideoOperationPoint);      //FSN-613: Dynamic Content for SVC/Mix Conf
}


////////////////////////////////////////////////////////////////////////////////////////
void CH323PartyCntl::OnChangeContentBitRateByLprOrDba(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323PartyCntl::OnChangeContentBitRateByLprOrDba - VNGFE-8204 : Name - ",m_partyConfName, GetPartyRsrcId());

	PASSERTMSG_AND_RETURN(!m_pIpCurrentMode , "m_pIpCurrentMode is NULL");

	DWORD newContentRate;
	DWORD isDba;

	*pParam >> newContentRate;
	*pParam >> isDba;
	m_isContentDba = isDba;

	TRACEINTO << "VNGFE-8204 - newContentRate:" << newContentRate << ", currentContentRate:" << m_pIpCurrentMode->GetContentBitRate(cmCapTransmit) ;
	m_pIpCurrentMode->SetContentBitRate(newContentRate, cmCapTransmit);
	m_DbaContentRate = newContentRate;

	if (m_pTaskApi && m_pParty)
	{
		m_pTaskApi->InformConfOnPartyReCap(m_pParty);
	}
	else
	{
		PTRACE(eLevelError, "CH323PartyCntl::OnChangeContentBitRateByLprOrDba - m_pTaskApi or m_pParty is NULL - cann't change content rate");
	}
}


