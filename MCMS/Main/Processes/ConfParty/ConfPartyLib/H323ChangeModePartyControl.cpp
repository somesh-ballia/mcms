
#include "H323ChangeModePartyControl.h"
#include "ConfPartyOpcodes.h"
#include "Conf.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "H323Caps.h"
#include "ConfPartyGlobals.h"
#include "VideoBridgeInterface.h"
#include "AudioBridgeInterface.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "IpCommon.h"
#include "ContentBridge.h"
#include "AckParams.h"
#include "H264Util.h"


const WORD  OPENDATACHANNEL = 11;
const WORD  CLOSEDATACHANNEL = 14;

const WORD  COMPAREDETAILS = 1001;
const WORD  COMPARECAPCODE = 1002;
const WORD  COP_UPDATE_CASCADE_LINK_LECTURE_MODE_TIMER = 1003; //the time of change mode this is about 2/3 from lecture mode timer
#define PARTYCNTL_CHANGERATE_TIME CHANGERATE_TIME - SECOND

const WORD  LOW_RESOLUTION_REQUIRED = 120;


using namespace std;


PBEGIN_MESSAGE_MAP(CH323ChangeModeCntl)
                                // conf events
  ONEVENT(SCMCHANGEMODE      ,IDLE        			,CH323ChangeModeCntl::OnConfChangeModeIdle)
  ONEVENT(SCMCHANGEMODE      ,CHANGE_ALL_MEDIA    	,CH323ChangeModeCntl::OnConfChangeModeIdle)
  ONEVENT(SCMCHANGEMODE      ,CHANGEVIDEO 		 	,CH323ChangeModeCntl::OnConfChangeModeVideo)
  ONEVENT(SCMCHANGEMODE      ,CHANGECONTENT			,CH323ChangeModeCntl::OnConfChangeModeContent)
  ONEVENT(SCMCHANGEMODE      ,CHANGEOTHER  			,CH323ChangeModeCntl::OnConfChangeModeOther)
  ONEVENT(SCMCHANGEMODE      ,REALLOCATE_RSC		,CH323ChangeModeCntl::OnConfChangeModeRealloc)
  ONEVENT(SCMCHANGEMODE      ,UPDATE_LEGACY_STATUS	,CH323ChangeModeCntl::OnConfChangeModeUpdateLegacyStatus)
/*ONEVENT(CHANGEMODEMSG      ,IDLE			,CH323ChangeModeCntl::OnConfChangeModeMsgIdle)
  ONEVENT(CHANGEMODEMSG      ,CHANGEVIDEO	,CH323ChangeModeCntl::OnConfChangeModeMsgChangeVideo)
  ONEVENT(CHANGEMODEMSG      ,CHANGECONTENT ,CH323ChangeModeCntl::OnConfChangeModeMsgChangeContent)*/

                        // brdg events
  ONEVENT(COP_VIDEO_IN_CHANGE_MODE   ,ANYCASE			,CH323ChangeModeCntl::OnCopVideoBridgeChangeIn)
  ONEVENT(COP_VIDEO_OUT_CHANGE_MODE   ,ANYCASE			,CH323ChangeModeCntl::OnCopVideoBridgeChangeOut)
  ONEVENT(COP_UPDATE_CASCADE_LINK_LECTURE_MODE   ,ANYCASE	,CH323ChangeModeCntl::OnCopCascadeLectureMode)
  ONEVENT(COP_START_CASCADE_LINK_LECTURE_MODE   ,ANYCASE	,CH323ChangeModeCntl::OnCopCascadeStartCopLinkLecturePendingMode)
  ONEVENT(COP_UPDATE_CASCADE_LINK_LECTURE_MODE_TIMER   ,ANYCASE	,CH323ChangeModeCntl::OnCopCascadeCopLinkLecturePendingModeTimerExpire)
  ONEVENT(PARTY_AUDIO_CONNECTED		 ,ANYCASE   	,CH323ChangeModeCntl::OnAudConnectPartyIdleOrAnycase)

  ONEVENT(PARTY_VIDEO_CONNECTED      ,IDLE			,CH323ChangeModeCntl::OnVidBrdgConVideoIdle)
  ONEVENT(PARTY_VIDEO_CONNECTED      ,CHANGEVIDEO	,CH323ChangeModeCntl::OnVidBrdgConVideo)
  ONEVENT(PARTY_VIDEO_CONNECTED      ,CHANGECONTENT	,CH323ChangeModeCntl::OnVidBrdgConContent)
  ONEVENT(PARTY_VIDEO_CONNECTED      ,CHANGE_ALL_MEDIA	,CH323ChangeModeCntl::OnVidBrdgConChangeAll)
  ONEVENT(PARTY_VIDEO_CONNECTED      ,UPDATE_LEGACY_STATUS	,CH323ChangeModeCntl::OnVidBrdgConUpdateLegacyState)

  ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	IDLE,		CH323ChangeModeCntl::OnCamVidBrdgConVideo)
  ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	CHANGEVIDEO,CH323ChangeModeCntl::OnCamVidBrdgConVideo)

  ONEVENT(PARTY_VIDEO_IN_UPDATED	 ,CHANGEVIDEO	,CH323ChangeModeCntl::OnVideoInBrdgUpdatedChangeVideo)
  ONEVENT(PARTY_VIDEO_IN_UPDATED	 ,ANYCASE	    ,CH323ChangeModeCntl::OnVideoInBrdgUpdatedAnycase)
  ONEVENT(PARTY_VIDEO_OUT_UPDATED	 ,CHANGEVIDEO	,CH323ChangeModeCntl::OnVideoOutBrdgUpdatedChangeVideo)
  ONEVENT(PARTY_VIDEO_OUT_UPDATED	 ,ANYCASE	    ,CH323ChangeModeCntl::OnVideoOutBrdgUpdatedAnycase)

  ONEVENT(PARTY_VIDEO_DISCONNECTED   ,UPDATE_LEGACY_STATUS,	CH323ChangeModeCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus)
  ONEVENT(PARTY_VIDEO_DISCONNECTED   ,ANYCASE		,CH323ChangeModeCntl::OnVidBrdgDisConVideo)
  ONEVENT(PARTY_XCODE_DISCONNECTED   ,ANYCASE       ,CH323ChangeModeCntl::OnXCodeBrdgDisconnected)
//  ONEVENT(PARTY_XCODE_DISCONNECTED   ,ANYCASE       ,CH323ChangeModeCntl::NullActionFunction)

  ONEVENT(PARTY_CONTENT_CONNECTED,		CHANGECONTENT,		CH323ChangeModeCntl::OnContentBrdgConnected)
  ONEVENT(PARTY_CONTENT_CONNECTED,		ANYCASE,			CH323ChangeModeCntl::OnContentBrdgConnectedAnycase)
  ONEVENT(PARTY_CONTENT_DISCONNECTED,	ANYCASE,			CH323ChangeModeCntl::OnContentBrdgDisconnected)

 /* ONEVENT(DATCONNECT         ,CHANGEVIDEO   ,CH323ChangeModeCntl::OnDataBrdgConVideo)
  ONEVENT(DATCONNECT         ,CHANGEOTHER   ,CH323ChangeModeCntl::OnDataBrdgConOther)
  ONEVENT(DATCONNECT         ,IDLE          ,CH323ChangeModeCntl::OnDataBrdgConIdle)
  ONEVENT(DATDISCONNECT      ,CHANGEOTHER   ,CH323ChangeModeCntl::OnDataBrdgDisConOther)
  ONEVENT(DATDISCONNECT      ,CHANGEVIDEO   ,CH323ChangeModeCntl::OnDataBrdgDisChangeMode)
  ONEVENT(DATDISCONNECT      ,CHANGECONTENT ,CH323ChangeModeCntl::OnDataBrdgDisChangeMode)*/

  // party events
  ONEVENT(PARTYENDCHANGEMODE ,IDLE          ,CH323ChangeModeCntl::OnPartyEndChangeIdle)
  ONEVENT(PARTYENDCHANGEMODE ,CHANGEOTHER   ,CH323ChangeModeCntl::OnPartyEndChangeOther)
  ONEVENT(PARTYENDCHANGEMODE ,CHANGEVIDEO   ,CH323ChangeModeCntl::OnPartyEndChangeVideo)
  ONEVENT(PARTYENDCHANGEMODE ,CHANGECONTENT ,CH323ChangeModeCntl::OnPartyEndChangeContent)
  ONEVENT(PARTYENDCHANGEMODE ,REALLOCATE_RSC ,CH323ChangeModeCntl::OnPartyEndChangeReAlloc )


//  ONEVENT(H323DISCONNECTMMLP, IDLE		    ,CH323ChangeModeCntl::OnPartyH323DisconnectMmlp)

  ONEVENT(IPPARTYMSECONDARY,IDLE			,CH323ChangeModeCntl::OnPartyMoveToSecondary)
  ONEVENT(IPPARTYMSECONDARY,CHANGEOTHER	    ,CH323ChangeModeCntl::OnPartyMoveToSecondary)
  ONEVENT(IPPARTYMSECONDARY,CHANGECONTENT,   CH323ChangeModeCntl::OnPartyMoveToSecondary)
  ONEVENT(IPPARTYMSECONDARY,CHANGEVIDEO  	,CH323ChangeModeCntl::OnPartyMoveToSecondaryChangeMode)
  ONEVENT(IPPARTYMSECONDARY,REALLOCATE_RSC  ,CH323ChangeModeCntl::OnPartyMoveToSecondaryChangeMode)

/*ONEVENT(END_CONTENT_RECONNECT   ,IDLE		,CH323ChangeModeCntl::OnPartyEndContentReconnectIdle)
  ONEVENT(END_CONTENT_RECONNECT   ,ANYCASE	,CH323ChangeModeCntl::OnPartyEndContentReconnectAnycaseButIdle)*/

  ONEVENT(H323PARTYCONNECTALL     ,IDLE     ,CH323ChangeModeCntl::OnPartyH323ConnectAllIdle)
  ONEVENT(H323PARTYCONNECTALL     ,CHANGEVIDEO     ,CH323ChangeModeCntl::OnPartyH323ConnectAll_other)
  ONEVENT(H323PARTYCONNECTALL     ,REALLOCATE_RSC     ,CH323ChangeModeCntl::OnPartyH323ConnectAll_other)
  ONEVENT(H323PARTYCONNECTALL     ,CHANGECONTENT      ,CH323ChangeModeCntl::OnPartyH323ConnectAll_other)
  ONEVENT(VIDEOMUTE,			   IDLE  	,CH323ChangeModeCntl::OnPartyMuteVideoIdle)
  ONEVENT(VIDEOMUTE,			   ANYCASE	,CH323ChangeModeCntl::OnPartyMuteVideoAnycase)

// self timers
  ONEVENT(CHANGETOUT         ,CHANGECONTENT ,CH323ChangeModeCntl::OnTimerPartyContent)
  ONEVENT(CHANGETOUT         ,CHANGEVIDEO   ,CH323ChangeModeCntl::OnTimerPartyVideo)
  ONEVENT(CHANGETOUT         ,CHANGEOTHER   ,CH323ChangeModeCntl::OnTimerPartyOther)
  ONEVENT(CHANGETOUT         ,REALLOCATE_RSC,CH323ChangeModeCntl::OnTimerPartyReAlloc)

  ONEVENT(VIDEO_UPDATE_TIMEOUT  ,IDLE       ,CH323ChangeModeCntl::OnTimerVideoUpdate)

//ECS:
  ONEVENT(H323PARTYCONNECTALL,		CHANGE_ALL_MEDIA,		CH323ChangeModeCntl::OnPartyH323ConnectAllChangeAll)

  ONEVENT(REMOTE_SENT_RE_CAPS,		CHANGE_ALL_MEDIA,		CH323ChangeModeCntl::OnPartyReceivedReCapsChangeAll)
  ONEVENT(REMOTE_SENT_RE_CAPS,		CHANGEVIDEO, 			CH323ChangeModeCntl::OnPartyReceivedReCapsChangeVideo)
  ONEVENT(REMOTE_SENT_RE_CAPS,		REALLOCATE_RSC, 		CH323ChangeModeCntl::OnPartyReceivedReCapsChangeVideo)

  ONEVENT(PARTY_RECEIVE_ECS,		CHANGEVIDEO,			CH323ChangeModeCntl::OnPartyReceivedECS)
  ONEVENT(PARTY_RECEIVE_ECS,		REALLOCATE_RSC,			CH323ChangeModeCntl::OnPartyReceivedECS)
  ONEVENT(PARTY_RECEIVE_ECS,		CHANGECONTENT,			CH323ChangeModeCntl::OnPartyReceivedECS)

  ONEVENT(H323DBC2COMMAND			,IDLE		   ,CH323ChangeModeCntl::OnH323DBC2Command)
  ONEVENT(H323DBC2COMMAND			,ANYCASE	   ,CH323ChangeModeCntl::OnH323DBC2Command)

  ONEVENT(SECONDARYCAUSEH323		,ANYCASE   ,CH323ChangeModeCntl::OnPartyH323SetSecondaryCause)

  ONEVENT(PARTY_IN_CONF_IND		,ANYCASE	   ,CH323ChangeModeCntl::OnCAMUpdatePartyInConf)

  ONEVENT(PRESENTATION_OUT_STREAM_UPDATED		,IDLE	       ,CH323ChangeModeCntl::OnPartyPresentationOutStreamUpdateIdle)
  ONEVENT(PRESENTATION_OUT_STREAM_UPDATED		,ANYCASE	   ,CH323ChangeModeCntl::OnPartyPresentationOutStreamUpdate)


 // party text on screen messages
  ONEVENT(PARTYLAYOUTCHANGED      ,IDLE			,CH323ChangeModeCntl::OnVideoBridgePartyChangeLayout)
  ONEVENT(PARTYLAYOUTCHANGED      ,CHANGEVIDEO	,CH323ChangeModeCntl::OnVideoBridgePartyChangeLayout)

  ONEVENT(COP_NO_VIDEO_UPDATES_TOUT      ,ANYCASE	,CH323ChangeModeCntl::OnCopNoVideoUpdatesTout)

  // Upgrade from AVConly to AVC->SVC mix mode
  ONEVENT(SET_PARTY_AVC_SVC_MEDIA_STATE,		IDLE,			CIpPartyCntl::OnConfChangeModeUpgradeIdle)
  ONEVENT(AVC_SVC_ADDITIONAL_PARTY_RSRC_IND,	REALLOCATE_RSC,	CH323ChangeModeCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate)
  ONEVENT(UPGRADETOMIXTOUT,						ANYCASE,		CH323ChangeModeCntl::OnPartyUpgradeToMixTout)
  ONEVENT(PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED,CHANGEVIDEO,	CH323ChangeModeCntl::OnUpgradePartyToMixed)
  ONEVENT(END_VIDEO_UPGRADE_TO_MIX_AVC_SVC,		CHANGEVIDEO,	CH323ChangeModeCntl::OnEndVideoUpgradeToMix)
  ONEVENT(END_AUDIO_UPGRADE_TO_MIX_AVC_SVC,		CHANGEVIDEO,	CH323ChangeModeCntl::OnEndAudioUpgradeToMix)
  ONEVENT(END_AVC_TO_SVC_ART_TRANSLATOR_DISCONNECTED,       REALLOCATE_RSC,   CPartyCntl::OnEndAvcToSvcArtTranslatorDisconnected)

PEND_MESSAGE_MAP(CH323ChangeModeCntl,CH323PartyCntl);

/////////////////////////////////////////////////////////////////////////////
CH323ChangeModeCntl::CH323ChangeModeCntl() // constructor
{
	m_pScmSentToParty = new CComModeH323;
	m_lsdChangeModeRequestRate		    = 0;
	m_lsdOldChangeModeRequestRate	    = 0;
	m_bSecondaryLimitation	            = FALSE;
//	m_bChannelsUpdatedWithoutChangeMode = FALSE;
	m_bBridgeChangeToAsymmetricProtocol = FALSE;
	m_isSentH239Out = 0;
	m_eChangeMediaType = eNoChangeMediaType;
	m_lastConnectionRate               = 0;
	m_bChangeFlag =0;


	VALIDATEMESSAGEMAP;
}

/////////////////////////////////////////////////////////////////////////////
CH323ChangeModeCntl::~CH323ChangeModeCntl() // destructor
{
	POBJDELETE(m_pScmSentToParty);
}

/////////////////////////////////////////////////////////////////////////////
const char*  CH323ChangeModeCntl::NameOf() const
{
	return "CH323ChangeModePartyCntl";
}

/////////////////////////////////////////////////////////////////////////////
void*  CH323ChangeModeCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

///////////////////////////////////////////////////////////////////////////////
CH323ChangeModeCntl& CH323ChangeModeCntl::operator =(const CH323PartyCntl& other)
{
	(CH323PartyCntl&)*this = (CH323PartyCntl&)other;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
CH323ChangeModeCntl& CH323ChangeModeCntl::operator= (const CH323ChangeModeCntl& other)
{
	m_lsdChangeModeRequestRate			= other.m_lsdChangeModeRequestRate;
	m_lsdOldChangeModeRequestRate		= other.m_lsdOldChangeModeRequestRate;
	m_bSecondaryLimitation				= other.m_bSecondaryLimitation;
//	m_bChannelsUpdatedWithoutChangeMode = other.m_bChannelsUpdatedWithoutChangeMode;
	m_bBridgeChangeToAsymmetricProtocol = other.m_bBridgeChangeToAsymmetricProtocol;

	m_eChangeMediaType					= other.m_eChangeMediaType;
	m_lastConnectionRate                = other.m_lastConnectionRate;


	if(m_pScmSentToParty)
		POBJDELETE(m_pScmSentToParty);
	if(other.m_pScmSentToParty)
	{
		m_pScmSentToParty = new CComModeH323;
		*m_pScmSentToParty = *other.m_pScmSentToParty;
	}

	m_bChangeFlag = other.m_bChangeFlag;

	(CH323PartyCntl&)*this = (CH323PartyCntl&)other;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// entry point to state machine used by all api functions
/////////////////////////////////////////////////////////////////////////////
//               H323 CHANGE PARTY CONTROL ACTION FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// entry point to change mode cycle : IDLE<-->CHANGEVIDEO<-->CHANGEOTHER
void  CH323ChangeModeCntl::OnConfChangeModeIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeIdle : Name - ",m_partyConfName);
	PASSERTMSG_AND_RETURN(!m_pIpInitialMode, "m_pIpInitialMode not valid");
	PASSERTMSG_AND_RETURN(!m_pIpCurrentMode, "m_pIpCurrentMode not valid");


	//This is only a protection, because the new scm shouldn't have ben sent to the party
	//control, as a result of the code in CConf::ChangeMode
	if (IsConfWaitingToEndChangeModeForMove())
	{//if the conf is waiting for the change mode to ended, in order to start move,
	 //there is no point to start another change mode process
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeIdle - m_confWaitToEndChangeModeForMove is true => disregard from the new scm. Name - ",m_partyConfName);
		EndChangeMode();
		return;
	}

	m_bIsNewScm = FALSE;
	EChangeMediaType eChangeMediaType = m_eChangeMediaType;
	m_eChangeMediaType = eNoChangeMediaType;

	// for audio only  - change mode not active
	if (m_voice)
	{
		/*if (NO == m_pConf->GetAudioConf())
			m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_SECONDARY);
		else
		{*/


		if ((m_pIpInitialMode) && (m_pIpCurrentMode))
			*m_pIpInitialMode = *m_pIpCurrentMode; //in order to end the change mode

		EndChangeMode();
		return;
	}
	// audio mode for H323 party is real audio that was opened by end-point
	m_pIpInitialMode->SetMediaMode(m_pIpCurrentMode->GetMediaMode(cmCapAudio),cmCapAudio);
	m_pIpInitialMode->SetMediaMode(m_pIpCurrentMode->GetMediaMode(cmCapAudio, cmCapTransmit),cmCapAudio, cmCapTransmit); // the default parameters are receive


	//In case an undefine participant that support audio only in video conference - it should be in connect
	//mode with status of "audio only"
	if ((m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople)) &&
		(m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople)) &&
		IsUndefinedParty() && IsRemoteCapNotHaveVideo())
	{
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeIdle: Audio-only Undefined EP = ",m_partyConfName);
		SetPartyStateUpdateDbAndCdrAfterEndConnected();

		if ((m_pIpInitialMode) && (m_pIpCurrentMode))
			*m_pIpInitialMode = *m_pIpCurrentMode; //in order to end the change mode
		EndChangeMode();
		return;
	}

	//set the content params before starting change mode process:
    WORD details = 0;
//	if (IsSecondaryConditionForContent(details))
//		SetPartyToContentSecondaryMode(details);
//	else
		OFF(m_bNoContentChannel);

	//	highest common
	if (eChangeMediaType != eChangeContentOnly)
	{
		//Update initialmode with new scm
		BYTE bChangeVidMode = ChangeVideoModeIfNeeded();

       //if we need to realloc, we need to wait for answer for reallocation before proceeding
       //Or we need to Change video mode - we to return if need to change video mode??
        if (m_state == REALLOCATE_RSC || (bChangeVidMode))
        {
        	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeIdle : Stop change mode. ",m_partyConfName);
            return;
        }
	}

	//in this point media state is connected, so we continue with the checking:
	if ((m_eVidBridgeConnState == eBridgeDisconnected) &&
		(m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople)) &&
		(m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople)))
	{
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeIdle : Video communication wasn't established. ",m_partyConfName);

		SetPartyToSecondaryAndStopChangeMode(SECONDARY_CAUSE_NO_VIDEO_CONNECTION);
		return;
	}

	ChangeOther();
}


///////////////////////////////////////////////////////////////////////////////////////
/*void CH323ChangeModeCntl::HandleDataInMove()
{
	DWORD current_data_rcv	= m_pIpCurrentMode->GetMediaBitRate(cmCapData, cmCapTransmit);
	DWORD initial_data_rcv	= m_pIpInitialMode->GetMediaBitRate(cmCapData, cmCapTransmit);
	CapEnum currDataType	= (CapEnum)m_pIpCurrentMode->GetMediaMode(cmCapData).GetType();
	CapEnum initDataType	= (CapEnum)m_pIpInitialMode->GetMediaMode(cmCapData).GetType();
	BOOL bIsNeedToUpdateTss = FALSE;

	if(current_data_rcv && initial_data_rcv && (current_data_rcv != initial_data_rcv))
	{
		//In fecc in case the rate is different we should send flow control like in daynammic lsd.
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeIdle : moved into conf with fecc different bitrate ",m_partyConfName);
		CComModeInfo CComModeInfo(initDataType,initial_data_rcv*100);
		m_lsdChangeModeRequestRate = CComModeInfo.GetH320ModeType();
		m_lsdOldChangeModeRequestRate = GetOldLsdRate();
		OpenDaynamicLsd();
	}
	else if (current_data_rcv || (current_data_rcv==0 && initial_data_rcv!=0) ) //they are equal so we can check only one of them
	{
		if((currDataType == eAnnexQCapCode) || (currDataType == eRvFeccCapCode) || (initDataType == eAnnexQCapCode) || (initDataType == eRvFeccCapCode))
		{
			BYTE bFeccValid = IsFeccValid();
			if (! bFeccValid)
			{
				PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeIdle : moved into conf with without FECC ",m_partyConfName);
				CloseFecc();
			}
		}
	}
}
*/
///////////////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::ChangeVideoModeIfNeeded()
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeVideoModeIfNeeded : ", GetPartyRsrcId());
	BYTE bChangeVidMode = FALSE;

	eChangeModeState changeModeState = eNotNeeded;
	ePartyMediaState mediaState = eConnect;

	DWORD details = 0;
	BYTE reason = SECONDARY_CAUSE_CONFERENCE_REJECT;
	BYTE direction = cmCapReceive;

	if (m_pIpInitialMode->GetConfType() == kCop)
		mediaState = DecideOnPartyMediaStateForCOP(changeModeState, details, reason, direction);
	else if (m_pIpInitialMode->GetConfType() == kVideoSwitch || m_pIpInitialMode->GetConfType() == kVSW_Fixed) //andrewk debug can be changed later
		mediaState = DecideOnPartyMediaStateForVSW(changeModeState, details, reason, direction);
	else //CP
		mediaState = DecideOnPartyMediaStateForCP(changeModeState, details, reason, direction);
	PTRACE2INT(eLevelInfoNormal, "***CH323ChangeModeCntl::ChangeVideoModeIfNeeded : mediaState", mediaState);
	switch (mediaState)
	{
		case eChangeMode_Must:
		case eChangeMode_BestEffort:
		case eChangeCpMode_FromCaps:
		{
            m_pIpInitialMode->Dump("CH323ChangeModeCntl::ChangeVideoModeIfNeeded - starting the changemode with this initial", eLevelInfoNormal);
			if(mediaState == eChangeMode_Must)
			{// set the party secondary mode. If at the end of the change mode the party will overcome the secondary and return to be in connected mode
				// the setting to connected will "clean" the secondary cause
				SetPartySecondaryCause(reason, details, direction);
			}

			//Need to check if new Rsrc is needed for the new SCM - if needed we will stop
			//change mode and will continue after ReAlloc response from RA.
			AdditionalRsrcHandling (statOK, m_pIpInitialMode);
			if(m_state == REALLOCATE_RSC)
				break;
			else
			{
				PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeVideoModeIfNeeded: Change Patry Mode. ",m_partyConfName);
				ChangeModeFromScm(changeModeState, mediaState);
				bChangeVidMode = TRUE;
				break;
			}
		}
		case eSecondary:
		{
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeVideoModeIfNeeded: Set to secondary.  ",m_partyConfName);

			SetPartyToSecondaryAndStopChangeMode(reason,details,direction);
			bChangeVidMode = TRUE;

			//Need to check if we need to downgrade resources due to secondary (Downgrade to CIF)
			AdditionalRsrcHandling (statOK, m_pIpInitialMode);
			break;
		}
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}

	return bChangeVidMode;
}


///////////////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsRemoteCapsEnableChangeMode(eChangeModeState& changeModeState)
{
	BYTE bRes = TRUE;

	cmCapDirection localDirection;
	if (GetDirectionByChangeModeState(changeModeState, localDirection) == FALSE)
	{
		PTRACE2(eLevelError, "IsRemoteCapsEnableChangeMode - Incorrect change mode state - Name ", m_partyConfName);
		return FALSE;
	}

	DWORD valuesToCompare = kFormat|kFrameRate|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode;
	if (m_pIpInitialMode->GetConfType() == kVideoSwitch || m_pIpInitialMode->GetConfType() == kVSW_Fixed)
		valuesToCompare |= kBitRate;//In CP, the rate isn't a reason for secondary, so we don't check it.

     if (localDirection & cmCapTransmit) // => this is the receive of the remote: need to check receive
		bRes = m_pRmtCapH323->AreRemoteCapsContaining(m_pIpInitialMode, valuesToCompare, cmCapVideo, cmCapReceive, kRolePeople);
	if (bRes && (localDirection & cmCapReceive) )//=> this is the transmit of the remote: need to check transmit or receive
	{
		BYTE bTempRes = m_pRmtCapH323->AreRemoteCapsContaining(m_pIpInitialMode, valuesToCompare, cmCapVideo, cmCapTransmit, kRolePeople);
		if (!bTempRes) //in this case we will also check the receive
			bRes = m_pRmtCapH323->AreRemoteCapsContaining(m_pIpInitialMode, valuesToCompare, cmCapVideo, cmCapReceive, kRolePeople);
	}

	if (!bRes)
		PTRACE2(eLevelError,"CH323ChangeModeCntl::IsRemoteCapsEnableChangeMode: The new scm doesn't supported by remote caps : Name - ", m_partyConfName);
	return bRes;
}
/////////////////////////////////////////////////////////////////////////////
ePartyMediaState CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP(eChangeModeState& changeModeState, DWORD& details, BYTE& reason, BYTE& direction)
{
	ePartyMediaState mediaState = eConnect;
	m_pIpCurrentMode->Dump("CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP - m_pIpCurrentMode", eLevelInfoNormal);
    CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
    if( !pConfParty )
        PTRACE(eLevelError,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP - pConfParty is NULL!!");

	//Check number 1: stay secondary / upgrade from secondary / downgrade to secondary
	mediaState = CheckSecondaryChanges(changeModeState, reason); //can return eSecondary or eChangeMode_Must
	PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP :  mediaState", mediaState);

	if (mediaState == eSecondary)
		return eSecondary;

	// Check change decoder:
	BYTE bCheckChangeDecoder = FALSE;
    if (m_pIpInitialMode->IsMediaOff (cmCapVideo, cmCapReceive, kRolePeople))
        PTRACE (eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP : receive is off, don't disturb the remote with another recap");
    else if (m_eLastCopChangeModeType == eCop_DecoderParams && (ECopDecoderResolution)m_eLastCopChangeModeParam < COP_decoder_resolution_Last)
	{
	//	m_pIpInitialMode->SetVideoRxModeAccordingDecoderResolution((ECopDecoderResolution)m_eLastCopChangeModeParam, m_eFirstRxVideoCapCode);
    	m_pIpInitialMode->SetVideoRxModeAccordingDecoderResolution((ECopDecoderResolution)m_eLastCopChangeModeParam, m_eFirstRxVideoCapCode,m_eFirstRxVideoProfile,m_pConf->GetCommConf()->GetCopConfigurationList(),
    				IsPartyCascadeWithCopMcu(), IsPartyMasterOrSlaveNotLecturer());

        CapEnum capCode = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive));
        DWORD videoRateToSet = m_pLocalCapH323->GetMaxVideoBitRate(capCode, cmCapReceive, kRolePeople);
        m_pIpInitialMode->SetVideoBitRate(videoRateToSet, cmCapReceive, kRolePeople);
        if( !IsPartyCascadeWithCopMcu() )
        	SetInitialRecRateAccordingToRes((ECopDecoderResolution)m_eLastCopChangeModeParam,videoRateToSet);
        if(m_pIpInitialMode->GetCopTxLevel()< NUMBER_OF_COP_LEVELS && m_pIpInitialMode->GetCopTxLevel() != INVALID_COP_LEVEL && m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) && !IsPartyCascadeWithCopMcu() )
        {
               DWORD levelvideoraterate = (m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople) ) / 10;
               DWORD audioRateAccordingToLevel = CalculateAudioRateAccordingToVideoRateOfCopLevel(levelvideoraterate);
               //audioRateAccordingToLevel = audioRateAccordingToLevel *10;
               DWORD maxLevelConfRate = levelvideoraterate + audioRateAccordingToLevel;
               DWORD RecCallRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRolePeople) /10  + m_pIpInitialMode->GetMediaBitRate(cmCapAudio, cmCapReceive, kRolePeople);
               if(RecCallRate > maxLevelConfRate)
               {
            	   PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP: cop rate exceed change in AUDIO is ",m_pIpInitialMode->GetMediaBitRate(cmCapAudio, cmCapReceive, kRolePeople));
                   DWORD newVideoRateForRx = maxLevelConfRate - m_pIpInitialMode->GetMediaBitRate(cmCapAudio, cmCapReceive, kRolePeople);
                   newVideoRateForRx = newVideoRateForRx *10;
                   PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP: cop rate exceed newVideoRateForRx ",newVideoRateForRx);
                   m_pIpInitialMode->SetVideoBitRate(newVideoRateForRx,cmCapReceive);

                }
         }

        bCheckChangeDecoder = TRUE;
	}
    else if (m_eLastCopChangeModeType == eCop_EncoderIndex)
    {
        BYTE encoderIndex = m_eLastCopChangeModeParam;
        CVidModeH323* pVidMode = m_pCopVideoTxModes->GetVideoMode(encoderIndex);
		if (pVidMode && pVidMode->IsMediaOn())
        {
            bCheckChangeDecoder = TRUE;
            if (pVidMode->GetType() == eH264CapCode)
            {
                CCOPConfigurationList* pCOPConfigurationList = m_pConf->GetCommConf()->GetCopConfigurationList();
                CCopVideoParams* pCopLevelParams = pCOPConfigurationList->GetVideoMode(encoderIndex);
                // Change the scm receive video:
                sCopH264VideoMode copH264VideoMode;
                CCopVideoModeTable* pCopTable = new CCopVideoModeTable;
                APIU16 profile = GetProfileAccordingToCopProtocol(pCopLevelParams->GetProtocol());
                DWORD videoRate = pConfParty ? pConfParty->GetVideoRate() : 0;
                pCopTable->GetSignalingH264ModeAccordingToReservationParams(pCopLevelParams, copH264VideoMode, TRUE, videoRate);
                m_pIpInitialMode->SetH264Scm(profile, copH264VideoMode.levelValue, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB, copH264VideoMode.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, copH264VideoMode.maxStaticMbps, cmCapReceive);
                POBJDELETE(pCopTable);
                DWORD videoRateToSet = pVidMode->GetBitRate();
                m_pIpInitialMode->SetVideoBitRate(videoRateToSet, cmCapReceive, kRolePeople);
            }
            else
            	m_pIpInitialMode->SetMediaMode(*pVidMode, cmCapVideo, cmCapReceive, kRolePeople);
            m_pIpInitialMode->SetDirection(cmCapVideo, cmCapReceive, kRolePeople);
        }

        else
            PASSERTMSG(GetPartyRsrcId(),"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP -Failed to set receive mode according to tx level");
    }
	else
		PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP :  m_eLastCopDecoderResolution still hasn't initiated");

    if (bCheckChangeDecoder)
    {
        const CVidModeH323 initRecvVideo = (const CVidModeH323 &)m_pIpInitialMode->GetMediaMode(cmCapVideo,cmCapReceive);
        const CVidModeH323 currRecvVideo = (const CVidModeH323 &)m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapReceive);
        if (!(initRecvVideo == currRecvVideo))
        {
            mediaState = eChangeMode_Must;
            changeModeState = eChangeIncoming;
        }
    }


    //Handle the transmit direction according to the remote caps or conf decision if set in m_lastCopForceEncoderLevel
    CComModeH323* pScm = new CComModeH323(*m_pIpInitialMode);
    BYTE iCopLevel = INVALID_COP_LEVEL;

    //here start regular mode
    if(!m_bCascadeIsLecturer)
    {
		if (m_lastCopForceEncoderLevel == INVALID_COP_LEVEL)
		{

			 DWORD definedMaxRate= pConfParty ? pConfParty->GetVideoRate() : 0;
			 WORD setuprate = GetSetupRate() /1000;
			 if(setuprate != 0 && CASCADE_NONE == m_bIsCascade /*&& strstr(m_productId,"HDX")*/ )
			{
				   DWORD inaudiorate = m_pIpCurrentMode->GetMediaBitRate(cmCapAudio, cmCapReceive, kRolePeople);
				   DWORD AudioRateAccordingToSetupRate = (CalculateAudioRate(setuprate *1000)) /1000;
				   WORD remoteSetupRateonlyforvideo = setuprate - AudioRateAccordingToSetupRate;
				   if(definedMaxRate != 0xFFFFFFFF && inaudiorate != 0 && remoteSetupRateonlyforvideo != 0)
						definedMaxRate = min(definedMaxRate,(DWORD)remoteSetupRateonlyforvideo);
				   else if (inaudiorate != 0 )
						definedMaxRate = remoteSetupRateonlyforvideo;
					PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP - ,remoteSetupRate ",remoteSetupRateonlyforvideo);
			}
			m_pRmtCapH323->FindBestVidTxModeForCop(m_pCopVideoTxModes, pScm, (pConfParty? pConfParty->GetVideoProtocol() : 0), definedMaxRate);
			iCopLevel = pScm->GetCopTxLevel();
			m_pIpInitialMode->SetCopTxLevel(iCopLevel);
		}
		else
		{
			//set specific mode according to conference decision
			iCopLevel = m_lastCopForceEncoderLevel;
			CVidModeH323* pVidMode = m_pCopVideoTxModes->GetVideoMode(iCopLevel);
			if (pVidMode && pVidMode->IsMediaOn())
			{
				 pScm->SetMediaMode(*pVidMode, cmCapVideo, cmCapTransmit, kRolePeople);
				 DWORD valuesToCompare = kBitRate|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS| kH264Additional_FS|kRoleLabel;
				 if (!m_pRmtCapH323->AreRemoteCapsContaining(pScm, valuesToCompare,cmCapVideo, cmCapReceive, kRolePeople))
					 iCopLevel = INVALID_COP_LEVEL;
				 m_pIpInitialMode->SetCopTxLevel(iCopLevel);

			}
			else
				PASSERTMSG(GetPartyRsrcId(),"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP -Failed to set tx mode according to conf decision");
		}

		BYTE currCopLevel = m_pIpCurrentMode->GetCopTxLevel();
		PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP cop level:", iCopLevel);
		if (iCopLevel < NUMBER_OF_COP_LEVELS)
		{
			m_pIpInitialMode->SetMediaMode(pScm->GetMediaMode(cmCapVideo, cmCapTransmit, kRolePeople), cmCapVideo, cmCapTransmit, kRolePeople);
			m_pIpInitialMode->Dump("CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP after setting cop",eLevelInfoNormal);

			if (iCopLevel != currCopLevel || m_pIpInitialMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople) || IsNeedToChangeDueToSwitchFromLecturerToNonLecturer(iCopLevel))
			{
				mediaState = eChangeMode_Must;
				eChangeModeState changeOutState = eChangeOutgoing;
				if (currCopLevel < NUMBER_OF_COP_LEVELS)
				{
					CVidModeH323* pCurVidMode = m_pCopVideoTxModes->GetVideoMode(currCopLevel);
					CVidModeH323* pInitVidMode = m_pCopVideoTxModes->GetVideoMode(iCopLevel);
					if (pCurVidMode && pInitVidMode && (pCurVidMode->GetType() != pInitVidMode->GetType()))
						changeOutState = eReopenOut; // only if protocol is different
					else if (!pCurVidMode)
						changeOutState = eReopenOut;
				}
				else
					changeOutState = eReopenOut; //either current is closed or initial is closed

				changeModeState = CombineChangeModeStates(changeOutState, changeModeState);
			}
		}

		else
		{
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP - Remote caps not contain any Cop level: Name - ",m_partyConfName);
			mediaState = eSecondary;
			changeModeState = CombineChangeModeStates(eChangeOutgoing, changeModeState);
		}
	}
    else  //we are in link lecturer mode
    {
    	const CVidModeH323 lastTransVideo   = (const CVidModeH323 &)m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapTransmit);
    	BYTE isfoundmode = CreateNewModeForCopCascadeLecturerLink(pScm);
    	m_pIpInitialMode->SetMediaMode(pScm->GetMediaMode(cmCapVideo, cmCapTransmit), cmCapVideo, cmCapTransmit);
    	const CVidModeH323 newTransVideo   = (const CVidModeH323 &)m_pIpInitialMode->GetMediaMode(cmCapVideo,cmCapTransmit);
    	if(isfoundmode && ( !(lastTransVideo == newTransVideo) || m_pIpInitialMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople) ) )
    	{
    		mediaState = eChangeMode_Must;
    		eChangeModeState changeOutState = eChangeOutgoing;
    		WORD LastVideoTranType = lastTransVideo.GetType();
    		WORD NewVideoTranType = newTransVideo.GetType();
    		if (lastTransVideo.IsMediaOn() && newTransVideo.IsMediaOn() && LastVideoTranType != NewVideoTranType )
    			changeOutState = eReopenOut; // only if protocol is different
    		else if (!lastTransVideo.IsMediaOn() && newTransVideo.IsMediaOn())
    			changeOutState = eReopenOut;
    		changeModeState = CombineChangeModeStates(changeOutState, changeModeState);


    	}
    	else if(!isfoundmode)
    	{
    		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP - Remote caps not contain lecturer link level: Name - ",m_partyConfName);
    		mediaState = eSecondary;
    		changeModeState = CombineChangeModeStates(eChangeOutgoing, changeModeState);

    	}



    }

    PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForCOP - change mode state = ", GetChangeModeStateStr(changeModeState));
	POBJDELETE (pScm);
	return mediaState;
}
/////////////////////////////////////////////////////////////////////////////
ePartyMediaState CH323ChangeModeCntl::DecideOnPartyMediaStateForCP(eChangeModeState& changeModeState, DWORD& details, BYTE& reason, BYTE& direction)
{
	TRACEINTO <<  "PartyId:" << GetPartyRsrcId();

	ePartyMediaState mediaState = eConnect;
	m_pIpInitialMode->Dump("CH323ChangeModeCntl::DecideOnPartyMediaStateForCP - m_pIpInitialMode", eLevelInfoNormal);

	//Check number 1: stay secondary / upgrade from secondary / downgrade to secondary
	mediaState = CheckSecondaryChanges(changeModeState, reason); //can return eSecondary or eChangeMode_Must
	if (mediaState == eSecondary)
		return eSecondary;

	else if (mediaState == eConnect)
	{
		// Check number 2: Change mode must - in case of re caps from remote to reduce params inside protocol / change protocol:
		BYTE bIsMustToChange = IsMustToChangeModeInCp(reason, details);
		if (bIsMustToChange)
			mediaState = eChangeMode_Must;
		else
		{
			// YAEL: AFTER MERGE - change kH264Additional to only mbps and fs for both following functions!!!
			//Check number 3: Change mode best effore - in case of re caps from remote in order to increase caps
			BYTE bIsEqualToRemote = m_pRmtCapH323->AreRemoteCapsEqual(m_pIpCurrentMode, kCapCode | kFormat | kFrameRate | kAnnexes | kH264Profile | kH264Level | kH264Additional_MBPS | kH264Additional_FS | kMaxFR | kH264Mode, cmCapVideo, cmCapReceive, kRolePeople); //receive remote = transmit scm
			TRACEINTO <<  "PartyId:" << GetPartyRsrcId() << ", IsEqualToRemote:" << (WORD)bIsEqualToRemote;
			if (!bIsEqualToRemote) //we opened less than remote capable
			{ // check if we opened less than local capable
				WORD temp_value = kCapCode | kFormat | kFrameRate | kAnnexes | kH264Profile | kH264Level | kH264Additional_MBPS | kH264Additional_FS | kMaxFR | kH264Mode;

				WORD bIsEqualToLocal = m_pLocalCapH323->AreLocalCapsEqual(m_pIpCurrentMode, temp_value, cmCapVideo, cmCapTransmit, kRolePeople);
				TRACEINTO <<  "PartyId:" << GetPartyRsrcId() << ", IsEqualToLocal:" << (WORD)bIsEqualToLocal;
				if (!bIsEqualToLocal)
					mediaState = eChangeMode_BestEffort;
			}
			BYTE bIsResolution = IsResolutionFeetsToRateInCp(reason, details);
			if (bIsResolution)
				mediaState = eChangeMode_BestEffort;
		}
	}

	//decide on new scm:
	if ((mediaState == eChangeMode_BestEffort) || (mediaState == eChangeMode_Must))
	{
		CBaseVideoCap* pIntersectCap = NULL;
		//intersect scm and remote caps
		if (mediaState == eChangeMode_Must || reason == LOW_RESOLUTION_REQUIRED)
			if (m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit))
				pIntersectCap = m_pRmtCapH323->FindIntersectionBetweenCapsAndVideoScm(m_pIpInitialMode);

		//intersect local and remote caps
		if (pIntersectCap == NULL)
			pIntersectCap = m_pLocalCapH323->FindIntersectionBetweenTwoCaps(m_pRmtCapH323, eUnknownAlgorithemCapCode, cmCapReceiveAndTransmit);

		if (pIntersectCap)
		{
			COstrStream msg; //temp
			pIntersectCap->Dump(msg); //temp
			PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP - pIntersectCap", msg.str().c_str()); //temp

			if (mediaState == eChangeMode_BestEffort)
			{
				BYTE bIsCurrentEqualOrHigher = m_pIpCurrentMode->IsMediaContaining(*pIntersectCap, kCapCode | kFormat | kFrameRate | kH264Profile | kH264Level | kH264Additional_MBPS | kH264Additional_FS | kMaxFR | kH264Mode, &details, cmCapVideo, cmCapTransmit, kRolePeople);
				PTRACE2INT(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: LOW_RESOLUTION_REQUIRED - ", reason);
				if (bIsCurrentEqualOrHigher && reason != LOW_RESOLUTION_REQUIRED)
				{
					PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: best effort and current >= newMode - Do nothing, Name - ", m_partyConfName);
					pIntersectCap->FreeStruct();
					POBJDELETE(pIntersectCap);
					return eConnect;
				}
			}

			if (!(mediaState == eChangeMode_BestEffort && reason == LOW_RESOLUTION_REQUIRED))
			{
				//set new scm in initial
				m_pIpInitialMode->SetMediaMode(pIntersectCap->GetCapCode(), pIntersectCap->SizeOf(), (BYTE*)pIntersectCap->GetStruct(), cmCapVideo, cmCapTransmit, kRolePeople);
				//pIntersectCap->FreeStruct();
			}

			//check change protocol
			DWORD tempDetails = 0x00000000;
			BYTE bRes = m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode, &tempDetails, cmCapVideo, cmCapTransmit, kRolePeople);
			if (!bRes)
			{
				PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: different protocol in transmit - Name ", m_partyConfName);
				details = tempDetails;
				details &= DETAILS_WITHOUT_FORMAT;

				direction = cmCapTransmit;
				//upgrade from secondary:
				if (m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapReceive, kRolePeople))
				{
					m_pIpInitialMode->SetMediaMode(pIntersectCap->GetCapCode(), pIntersectCap->SizeOf(), (BYTE*)pIntersectCap->GetStruct(), cmCapVideo, cmCapReceive, kRolePeople);
					changeModeState = eReopenInAndOut;
				}
				else if (m_pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRolePeople) == 0)
					changeModeState = eFlowControlInAndReopenOut;
				//else - we are responsible to change only transmit
				else
				{
					changeModeState = eReopenOut;
					if (reason == LOW_RESOLUTION_REQUIRED)
					{
						PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: ChangeMode = must, lower resolution ", m_partyConfName);
						changeModeState = eChangeInAndReopenOut;
					}
				}
			}
			else //the change is inside the opened protocol:
			{
				bRes = m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kBitRate | kFormat | kFrameRate | kAnnexes | kH264Level | kH264Additional_MBPS | kH264Additional_FS | kMaxFR | kH264Mode, &tempDetails, cmCapVideo, cmCapTransmit, kRolePeople);
				if (bRes)
				{
					BYTE bResForBestEffort = m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kFormat | kFrameRate | kAnnexes | kH264Level | kH264Additional_MBPS | kH264Additional_FS | kMaxFR | kH264Mode, &tempDetails, cmCapVideo, cmCapTransmit, kRolePeople);
					if (!bResForBestEffort)
					{
						m_pIpInitialMode->Dump("CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: Initial", eLevelInfoNormal);
						m_pIpCurrentMode->Dump("CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: current", eLevelInfoNormal);
						mediaState = eChangeMode_BestEffort;
						bRes = FALSE;
						PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: trying to go up higher in transmit based on best effort ", m_partyConfName);
					}
				}
				if (!bRes)
				{
					PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: different params inside protocol - in transmit ", m_partyConfName);
					details = tempDetails;
					details &= DETAILS_WITHOUT_FORMAT;
					direction = cmCapTransmit;
					changeModeState = eChangeOutgoing;
					if (reason == LOW_RESOLUTION_REQUIRED)
					{
						PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: EP supports lower rate, re_cap needed ", m_partyConfName);
						changeModeState = eReopenInAndOut;
					}

				}
				else
				{
					PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP: No need to change, just connect ", m_partyConfName);
					changeModeState = eNotNeeded;
					mediaState = eConnect;
				}

			}
			pIntersectCap->FreeStruct();
		}

		else
		{
			mediaState = eSecondary;
			reason = SECONDARY_CAUSE_REMOTE_CAPABILITIES;
		}

		POBJDELETE(pIntersectCap);
	}

	PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForCP - change mode state = ", GetChangeModeStateStr(changeModeState)); //temp

	return mediaState;
}


/////////////////////////////////////////////////////////////////////////////////////
ePartyMediaState CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW(eChangeModeState& changeModeState,DWORD &details,BYTE &reason,BYTE &direction)
{
	ePartyMediaState mediaState = eConnect;

	//stay secondary / upgrade from secondary / downgrade to secondary
	mediaState = CheckSecondaryChanges(changeModeState, reason);//can return eSecondary or eChangeMode_Must

	TRACESTR(eLevelInfoNormal)<< " CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW,CheckSecondaryChanges:  - mediaState " << (DWORD)mediaState << ",  Name - " << m_partyConfName;

	if (m_pIpInitialMode->GetConfType() == kVSW_Fixed
		&&  mediaState == eChangeMode_Must
		&& !IsRemoteCapsEnableChangeMode(changeModeState))
		mediaState = eSecondary; // can't change
	TRACESTR(eLevelInfoNormal)<< " CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW:  - mediaState "<< (DWORD)mediaState << ",  Name - " << m_partyConfName;

	if (mediaState == eSecondary || mediaState == eChangeMode_Must)
		return mediaState;

	BYTE bRes;
	DWORD tempDetails = 0x00000000;

	if (m_pIpInitialMode->GetConfType() == kVSW_Fixed)
	{		//In case of fixed (current & initial), both modes must be identical
		//check initial >= current - for transmit :
        DWORD valuesToCompare = kCapCode|kFormat|kFrameRate|kAnnexes|kH264Additional_MBPS|kH264Additional_FS| kMaxFR | kH264Mode;
        if (m_bIsCascade != CASCADE_NONE)
            valuesToCompare |= kBitRateForCascade;
        else
            valuesToCompare |= kBitRate;

		bRes =	m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, valuesToCompare, &tempDetails, cmCapVideo,cmCapTransmit,kRolePeople);
		if (!bRes) //initial < current - for transmit : out channel was opened with higher capabilities than we want to transmit
		{
			PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: <VSW_Fixed> initial < current in transmit - Name ", m_partyConfName);
			//this is the only case that means to reopen both directions
			details = tempDetails;
			direction = cmCapTransmit;
			return eSecondary;
		}
		//check current >= initial - for transmit :
		bRes =	m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, valuesToCompare, &tempDetails, cmCapVideo,cmCapTransmit,kRolePeople);
		if (!bRes) //initial < current - for transmit : out channel was opened with higher capabilities than we want to transmit
		{
			PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: <VSW_Fixed> initial > current in transmit - Name ", m_partyConfName);
			m_pIpInitialMode->Dump("CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: secondary, Initial", eLevelInfoNormal);
			m_pIpCurrentMode->Dump("CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: secondary, current", eLevelInfoNormal);
			//this is the only case that means to reopen both directions
			details = tempDetails;
			direction = cmCapTransmit;
			return eSecondary;
		}
	}
	else //VSW auto
	{
	   	if (m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit) &&
			(m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit) == 0) )
		{/* Upgrading from secondary, as a result of re-opening the channels by the remote, and
			not as a result of the mcms.In this case, we need to update the rate in the same
			way as we do it when the mcms initialte the upgrading from secondary. */
			PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: Upgrading from secondary, as a result of re-opening the channels by the remote - Name ", m_partyConfName);
			m_pIpInitialMode->SetVideoBitRate(m_videoRate, cmCapReceiveAndTransmit);
		}

		//1) Must changes:

		//check initial >= current - for transmit :
		bRes =	m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode, kCapCode, &tempDetails, cmCapVideo,cmCapTransmit,kRolePeople);
		if (!bRes) //initial < current - for transmit : out channel was opened with higher capabilities than we want to transmit
		{
			PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: initial < current in transmit - Name ", m_partyConfName);
			//this is the only case that means to reopen both directions
			details = tempDetails;
			details &= DETAILS_WITHOUT_FORMAT;// Gets the details without the format
			direction = cmCapTransmit;

			changeModeState = eReopenOut;
			mediaState = eChangeMode_Must;
		}

		//check current >= initial - for transmit :
	 	bRes = m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode,kBitRate|kFormat|kFrameRate|kAnnexes|kH264Level | kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, &tempDetails, cmCapVideo,cmCapTransmit,kRolePeople);
		if (!bRes) //initial > current - for transmit : out channel was opened with lower capabilities than we want to transmit
		{
			PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: initial > current in transmit - Name ", m_partyConfName);

			eChangeModeState currChangeModeState = DecideOnPartyChangeModeState(tempDetails, cmCapTransmit);
			details = tempDetails;
			details &= DETAILS_WITHOUT_FORMAT;// Gets the details without the format
			direction = cmCapTransmit;

			if (currChangeModeState == eCanNotChange)
				return eSecondary;
			else if (currChangeModeState != eNotNeeded)
			{
				mediaState = eChangeMode_Must;
				if (changeModeState != eReopenOut)
					changeModeState = currChangeModeState; //if we decided to close the out, we can't change this decision
			}
			//else: (changeModeState == eNotNeeded): mediaState isn't changed
		}

		//check initial >= current - for receive :
		bRes =	m_pIpInitialMode->IsMediaContaining(*m_pIpCurrentMode,kBitRate|kFormat|kFrameRate|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, &tempDetails, cmCapVideo,cmCapReceive,kRolePeople);
		if (!bRes) //initial < current - for receive : incoming channel was opened with higher capabilities than we can accept => reduce the channel
		{
			PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: initial < current in receive - Name ", m_partyConfName);
			eChangeModeState incomingState = DecideOnPartyChangeModeState(tempDetails, cmCapReceive);

			details = tempDetails;
			details &= DETAILS_WITHOUT_FORMAT;// Gets the details without the format
			direction = cmCapReceive;

			if (incomingState == eCanNotChange)
				return eSecondary;
			else if (incomingState != eNotNeeded)
			{
				mediaState = eChangeMode_Must;
                changeModeState = CombineChangeModeStates(changeModeState, incomingState);
			}
			//else: (incomingState == eNotNeeded): mediaState isn't changed
		}

		// 2) Best Effort changes:
		//check current >=  initial - for receive :
		/*The bitrate isn't checked here, because of course the incoming rate <= local caps rate,
		because of the % of protection in the card. We can't remove this % here and then compare
		the rates, since not always the % exist, and those considerations are in the card level.*/
		bRes =	m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode,kFormat|kFrameRate|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, &tempDetails, cmCapVideo,cmCapReceive,kRolePeople);
		if (!bRes) //initial > current - for receive : incoming channel was opened with lower capabilities than we can accept => upgrade the channel
		{
			PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: initial > current in receive - best effort - Name ", m_partyConfName);

			eChangeModeState incomingState = DecideOnPartyChangeModeState(tempDetails, cmCapReceive);

			details = tempDetails;
			details &= DETAILS_WITHOUT_FORMAT;// Gets the details without the format
			direction = cmCapReceive;

			if( (incomingState != eCanNotChange) && (incomingState != eNotNeeded) )
			{
				if (IsRemoteCapsEnableChangeMode(incomingState))
				{
					if (mediaState == eConnect) //if it's already eChangeMode_Must, don't change it
					{
						BYTE bFixedMode = (m_pIpInitialMode->IsAutoVideoResolution() == FALSE);
						BOOL bChangeModeVswFixed = GetSystemCfgFlagInt<BOOL>(CFG_KEY_CHANGE_MODE_IN_VSW_FIXED);
						if (bFixedMode && (bChangeModeVswFixed == FALSE) )
							PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW: Fixed mode, but change mode flag is false - Name ", m_partyConfName);
						else

						{
							mediaState = eChangeMode_BestEffort;
				/* If the party gets to this point, that means that the video which we need to
				   change is only best effort.
				   Since change scm from the conf, arrives only with change in 1 media,
				   if we need to change also the content mode, that means that we had already
				   tried to change what we just found that need to be changed, and failed in this
				   trying. So there is no point in trying to do that again (after all, this is
				   only best effort).
				   More import, if we do try to change the video, while it is connected to the
				   video bridge, and while the content rate has also changed, there is a big
				   chance to get asymmetric video TS. The possibility for that is higher in case
				   the ep doesn't have content caps, so the content bridge doesn't wait for the
				   party announcement that it finished the content rate change, in order to
				   reallocate the video TS.	*/
							/*BYTE bNeedToChangeAlsoContent = IsContentRateNeedToBeChanged();
							if (bNeedToChangeAlsoContent && m_isVidConn)
								mediaState = eConnect;*/
						}
					}

					if (mediaState != eConnect)
						changeModeState = CombineChangeModeStates(changeModeState, incomingState);
				}
				//else: we do nothing about this change, because this change is "best effort"
			}
		}
	}

	if (m_pIpInitialMode->GetConfType() == kVideoSwitch) // not for fixed - there is no change moe in case of fixed!!
	{
		if( (mediaState == eChangeMode_Must) || (mediaState == eChangeMode_BestEffort) )
		{
			if (IsRemoteCapsEnableChangeMode(changeModeState) == FALSE)
			{
				if (mediaState == eChangeMode_BestEffort)
				{
					mediaState = eConnect;
					PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW : Change mode state was changed to connect becaues of remote caps, Name - ",m_partyConfName);

					//if we don't change to the new scm, we should put in it the current mode, in order to connect to the bridge correctly.
					m_pIpInitialMode->SetMediaMode(m_pIpCurrentMode->GetMediaMode(cmCapVideo, cmCapReceive), cmCapVideo, cmCapReceive);
					m_pIpInitialMode->SetMediaMode(m_pIpCurrentMode->GetMediaMode(cmCapVideo, cmCapTransmit), cmCapVideo, cmCapTransmit);
				}
				else //(mediaState == eChangeMode_Must)
				{
					mediaState = eSecondary;
					PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::DecideOnPartyMediaStateForVSW : Change mode state was changed to secondary becaues of remote caps, Name - ",m_partyConfName);
				}
			}
		}
	}

	return mediaState;
}

/////////////////////////////////////////////////////////////////////////////////////
eChangeModeState CH323ChangeModeCntl::DecideOnPartyChangeModeState(DWORD details, cmCapDirection direction)
{
	eChangeModeState changeModeState = eNotNeeded;
	details &= DETAILS_WITHOUT_FORMAT;// Gets the details without the format

    //close incoming channel / close outgoing channel:
	if ( (details == DIFFERENT_CAPCODE) || !details/*in case at least one of the modes is off, details is 0*/)
		changeModeState = (direction == cmCapReceive)? eReopenIn: eReopenOut;
	else if(details == NO_H263_PLUS)
		changeModeState = (direction == cmCapReceive)? eChangeIncoming: eReopenOut;
	else
	{
		if (details & ANNEXES_DETAILS)
		{
			for (annexesListEn i = typeAnnexB; i < H263_Annexes_Number; i++)
			{
				if (GetAnnexFromDetails(details) == i)
				{
					changeModeState = (direction == cmCapReceive)? eChangeIncoming: eReopenOut;
					break;
				}
			}
		}
	}

	if (changeModeState == eNotNeeded)
	{	//change incoming channel / close outgoing channel:
		if( (details == HIGHER_FRAME_RATE) || (details == HIGHER_FORMAT) ||
			(details == HIGHER_LEVEL) || (details == HIGHER_MBPS) || (details == HIGHER_FS) ||
			(details == HIGHER_DPB) || (details == HIGHER_BR_AND_CPB) )
		{
			changeModeState = (direction == cmCapReceive)? eChangeIncoming: eReopenOut;
		}

		//flow control:
		else if (details == HIGHER_BIT_RATE)
		{
			DWORD currVidRate = m_pIpCurrentMode->GetMediaBitRate(cmCapVideo, direction);
			DWORD newVidRate  = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, direction);

			//if(currVidRate != newVidRate) should we check before sending flow control?
			changeModeState = (direction == cmCapReceive)? eFlowControlIn: eFlowControlOut;
		}

		//else if (details == DIFFERENT_ROLE)
	}

	return changeModeState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
eChangeModeState CH323ChangeModeCntl::CombineChangeModeStates(eChangeModeState outgoingState, eChangeModeState incomingState)
{
	eChangeModeState changeModeState = outgoingState;
	if (outgoingState == eNotNeeded)
		changeModeState = incomingState;
	else if(incomingState == eChangeIncoming && outgoingState == eReopenOut)
		changeModeState = eChangeInAndReopenOut;
    else if (incomingState == eChangeIncoming && outgoingState == eChangeOutgoing)
        changeModeState = eChangeInAndOut;
	else if( (incomingState == eReopenIn) && ((outgoingState == eReopenOut) || (outgoingState == eChangeInAndReopenOut) ))
		changeModeState = eReopenInAndOut;
	//flow control:
	else if( (changeModeState == eFlowControlIn) && (outgoingState == eFlowControlOut) )
		changeModeState = eFlowControlInAndOut;
	else if( (incomingState == eChangeIncoming) && (outgoingState == eFlowControlOut))
		changeModeState = eFlowControlOutAndChangeIn;
	else if( (incomingState == eReopenIn) && (outgoingState == eFlowControlOut))
		changeModeState = eFlowControlOutAndReopenIn;

	return changeModeState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
ePartyMediaState CH323ChangeModeCntl::DecideOnUpgradingFromSecondary(eChangeModeState& rChangeModeState)
{
	if (m_bSecondaryLimitation)
		return eSecondary; // can't change

	else if (m_pRmtCapH323->OnType(cmCapVideo) == FALSE)
		return eSecondary; // can't change
	else if (m_pLocalCapH323->OnType(cmCapVideo) == FALSE)
		return eSecondary; // can't change
	else
	{
		m_pIpInitialMode->SetVideoBitRate(m_videoRate, cmCapReceiveAndTransmit);
		rChangeModeState = eReopenInAndOut;
		return eChangeMode_Must;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
//stay secondary / upgrade from secondary / downgrade to secondary
ePartyMediaState CH323ChangeModeCntl::CheckSecondaryChanges(eChangeModeState& rChangeModeState, BYTE &reason)
{
	ePartyMediaState mediaState = eConnect;
	if (IsSecondary()) //in case are in secondary and will stay in secondary - the disconnect cause was already initialized
		mediaState = DecideOnUpgradingFromSecondary(rChangeModeState);

	else if (m_pRmtCapH323->OnType(cmCapVideo) == FALSE)
	{
		mediaState = eSecondary;
		reason = SECONDARY_CAUSE_REMOTE_CAPABILITIES;
	}
	TRACEINTO << m_partyConfName << ", MediaState:" << (DWORD)mediaState;
	return mediaState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO-URI:
// the check should include
// a party should be in secondary of content if it can't receive the target content video mode.
// Question: should we check remote support of the protocol only or all the inter
//			protocol parameters (like: resolution)?
//	Answer: all the inter protocol parameters as well!!
BYTE CH323ChangeModeCntl::IsSecondaryConditionForContent(DWORD &details)
{
	BYTE bSecondaryCondition = FALSE;
	BYTE bRes = TRUE;
    BOOL bStrictPolicyForH239HighestCommon = GetSystemCfgFlagInt<BOOL>(CFG_KEY_H239_FORCE_CAPABILITIES);
    DWORD valuesToCompare = 0;
    if (bStrictPolicyForH239HighestCommon)
        valuesToCompare = kCapCode|kFormat|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode;
    else
        valuesToCompare = kCapCode;

	BYTE isLegacy = IsLegacyContentParty();
	if (isLegacy == eContentSecondaryCauseBelowRate)
	{
		details = LOWER_FRAME_RATE;
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsSecondaryConditionForContent: Secondary content: Content rate is below minimum required threshold, ",m_partyConfName);
		return TRUE;
	}
	else if (isLegacy == eContentSecondaryCauseBelowResolution)
	{
		details = DIFFERENT_H264MODE;
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl:IsSecondaryConditionForContent:: Secondary content: Content resolution is below minimum required threshold, ",m_partyConfName);
		return TRUE;
	}

//    m_pRmtCapH323->Dump("CH323ChangeModeCntl::IsSecondaryConditionForContent: remote caps:", eLevelInfoNormal);
//    m_pIpInitialMode->Dump("CH323ChangeModeCntl::IsSecondaryConditionForContent: Initial", eLevelInfoNormal);

    // if the remote doesn't have content its secondary only if the initial has content
    // if the initial doesn't have content we only need that the remote caps will
    // support content in order not to get into secondary
    if(((m_pRmtCapH323->IsH239() == FALSE) && (!m_pRmtCapH323->IsEPC() || !m_pLocalCapH323->IsEPC()))
    		&& (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation)))
    {
        PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::IsSecondaryConditionForContent: Remote doesn't support H239 or call doesn't support EPC caps");
        bRes = FALSE;//secondary
    }
    else if(((m_pRmtCapH323->IsH239() == FALSE) || (!m_pRmtCapH323->IsEPC() || !m_pLocalCapH323->IsEPC()))
    		&& (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation) == FALSE))
    {// no content in target mode, only in remote caps and in current mode
        PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::IsSecondaryConditionForContent: No outgoing content channel");
    	bRes = FALSE;//secondary
        details = DIFFERENT_CAPCODE;// remote support different content protocol
    }
    else
    {
    	//    	CapEnum commonContentMode = eUnknownAlgorithemCapCode;
    	//   		commonContentMode = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
    	//   		bRes = m_pRmtCapH323->AreCapsSupportProtocol(commonContentMode, cmCapVideo, kRoleContentOrPresentation);
    	bRes = m_pRmtCapH323->AreRemoteCapsContaining(m_pIpInitialMode, valuesToCompare, cmCapVideo, cmCapReceive, kRoleContentOrPresentation);

	    if (!bRes)// secondary cause is the mismatch between capabilities.
	    {
	        PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::IsSecondaryConditionForContent: Content caps is not contained.");
	        details = DIFFERENT_PARAMETERS;// it may be same cap code but with different parameters
	    }
	    else if (isConfH239Cascade() && bRes)
	    {
	        //In H239 cascade the content rate is fixed, so the remote caps must support the conference content rate
	        CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
			BYTE presentationProtocol = pCommConf->GetPresentationProtocol();
			if(presentationProtocol == eH264Fix)
			{
				valuesToCompare = kBitRate;
		        //m_pIpInitialMode->Dump("CH323ChangeModeCntl::IsSecondaryConditionForContent: initial", eLevelInfoNormal);
		        bRes = m_pRmtCapH323->AreRemoteCapsContaining(m_pIpInitialMode, valuesToCompare, cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		        if (!bRes)
		        {
		            PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::IsSecondaryConditionForContent: Different cascade rate");
				    details = HIGHER_BIT_RATE;
		        }
				PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::IsSecondaryConditionForContent: Find caps");
			}
			else
				PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::IsSecondaryConditionForContent: not H264Fix");
	    }
    }

	if(details)
	{
		CSmallString msg;
		DumpDetailsToStream(cmCapVideo,details,msg);
		TRACESTR(eLevelInfoNormal)<< " CH323ChangeModeCntl::IsSecondaryConditionForContent, Name -  " << m_partyConfName << ": " << msg.GetString();
	}

	bSecondaryCondition = (!bRes);
	return bSecondaryCondition;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::SetPartyToContentSecondaryMode(DWORD &details)
{
	ON(m_bNoContentChannel);
	if(m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))
	{
        //m_pPartyApi->InActivateChannel(cmCapVideo,cmCapReceive,kRoleContentOrPresentation);
		m_pPartyApi->IpDisconnectMediaChannel(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation);
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: Secondary content ",m_partyConfName);
	}
	else
	{
		//shmulik todo add recap m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails, cif4Mpi, bIsAudioOnly);// update the party about the remote capabilities.
	}

	UpdateCurrentModeNoMedia(cmCapVideo,kRoleContentOrPresentation);

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "ENABLE_H239";
	BOOL bIsH239;
	BOOL bIsRateValid;
	sysConfig->GetBOOLDataByKey(key, bIsH239);
	if(!bIsH239)
	{
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: ENABLE_H239 sysflag is disabled. ",m_partyConfName);
		return;
	}

    if (details & HIGHER_BIT_RATE)
    {
        PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: Setting Cause (bitrate)!");
        SetPartySecondaryCause(SECONDARY_CAUSE_H239_BW_MISMATCH);
    }
    else if (details & DIFFERENT_PARAMETERS)
    {
        PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: Setting Cause (Caps don't match)!");
        SetPartySecondaryCause(SECONDARY_CAUSE_H239_INCOMPATIBLE_CAPS);
    }
    else if (details & DIFFERENT_CAPCODE)
    {
        PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: Setting Cause (Caps don't match)!");
        SetPartySecondaryCause(SECONDARY_CAUSE_H239_RMT_DIFF_CAPCODE);
    }
    else if(details & DIFFERENT_ROLE )
    {
    	 PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: Setting Cause (Content bridge rejected party connection)!");
    	 SetPartySecondaryCause(SECONDARY_CAUSE_H239_CONFERENCE_REJECT);
    }
    else if (details & LOWER_FRAME_RATE)
    {
		 PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: Setting Cause (Content rate is below minimum required threshold)!");
		 SetPartySecondaryCause(SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD);
    }
    else if(details & DIFFERENT_H264MODE)
    {
		 PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: Setting Cause (Content resolution is below minimum required threshold");
		 SetPartySecondaryCause(SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD);
    }
    else
        PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::SetPartyToContentSecondaryMode: details", details);
}

/////////////////////////////////////////////////////////////////////////////
// we need to change content if the intial and the current (transmit) are not the same codec
// the conference identify if we need to change between H264 to H263 therefore the party control only need to compare codec and not inter codec values.
BYTE CH323ChangeModeCntl::IsContentOutProtocolNeedToBeChanged(BYTE isSecondaryCondition) const
{
	if (isSecondaryCondition)    	// in case of secondary we can't change content mode, just continue to be with close
		return FALSE;		// outgoing channels and disconnected (or not connected) to the content bridge.

	BYTE bNeedToChangeContent = FALSE;
	CapEnum initialContentType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum currentContentType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

	TRACEINTO << "InitialContentType:" << initialContentType << ", CurrentContentType:" << currentContentType;
	bNeedToChangeContent = (initialContentType != currentContentType);

	ALLOCBUFFER(str, MediumPrintLen);
	sprintf(str,"need to change content %d, initialContType %d, currentContentType %d, ", bNeedToChangeContent, initialContentType, currentContentType);
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsContentOutProtocolNeedToBeChanged: The details: ",str);
	DEALLOCBUFFER(str);

	return bNeedToChangeContent;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsContentInProtocolNeedToBeChanged(BYTE isSecondaryCondition) const
{
	if (isSecondaryCondition)		// in case of secondary we can't change content mode, just continue to be with close
		return FALSE;		// outgoing channels and disconnected (or not connected) to the content bridge.

	BYTE  bNeedToChangeContent = FALSE;
	CapEnum initialContentType = eUnknownAlgorithemCapCode;
	CapEnum currentContentType = eUnknownAlgorithemCapCode;

	if( m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && !m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) )
	{
		m_pIpInitialMode->Dump("CH323ChangeModeCntl::IsContentInProtocolNeedToBeChanged: initial rx off but on current on, Initial", eLevelInfoNormal);
		m_pIpCurrentMode->Dump("CH323ChangeModeCntl::IsContentInProtocolNeedToBeChanged: initial rx off but on current on, Current", eLevelInfoNormal);
	}
	if (m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && 
		m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && 
		(CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) != eUnknownAlgorithemCapCode )
	{
	   initialContentType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	   currentContentType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);

		TRACEINTO << "InitialContentType:" << initialContentType << ", CurrentContentType:" << currentContentType;
		bNeedToChangeContent = (initialContentType != currentContentType);
	}

   ALLOCBUFFER(str, MediumPrintLen);
   sprintf(str,"need to change content %d, initialContType %d, currentContentType %d, ", bNeedToChangeContent, initialContentType, currentContentType);
   PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsContentInProtocolNeedToBeChanged: The details: ",str);
   DEALLOCBUFFER(str);

	return bNeedToChangeContent;
}
// Romem 1.11.10

/////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsContentOutHDResOrMpiNeedToBeChanged(BYTE isSecondaryCondition) const
{
	if(isSecondaryCondition)// in case of secondary we can't change content mode, just continue to be with close
		return FALSE;		// outgoing channels and disconnected (or not connected) to the content bridge.

	CapEnum initialContentOutType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum currentContentOutType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	if(initialContentOutType != eH264CapCode || currentContentOutType != eH264CapCode)
	   return FALSE;

	BYTE  bNeedToChangeContent = FALSE;
	BYTE  isInitialContentHD1080Supported = m_pIpInitialMode->isHDContent1080Supported(cmCapTransmit);
	BYTE  isCurrentContentHD1080Supported = m_pIpCurrentMode->isHDContent1080Supported(cmCapTransmit);

	bNeedToChangeContent =  (isInitialContentHD1080Supported != isCurrentContentHD1080Supported);
	if(!bNeedToChangeContent && !isCurrentContentHD1080Supported)
	{
	    BYTE  initialTxContentHD720Mpi = m_pIpInitialMode->isHDContent720Supported(cmCapTransmit);
	    BYTE  currentTxContentHD720Mpi = m_pIpCurrentMode->isHDContent720Supported(cmCapTransmit);
	    if(/*BRIDGE-15567: do not assume 720p resolution as XGA is used in TIP. - initialTxContentHD720Mpi && currentTxContentHD720Mpi &&*/ (initialTxContentHD720Mpi != currentTxContentHD720Mpi))
	    {
	       CMedString msg2;
	       msg2 << " Tx MPI of HD 720 Content: Initial=" << (DWORD)initialTxContentHD720Mpi << ", Current=" << (DWORD)currentTxContentHD720Mpi;
	       PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsContentOutHDResOrMpiNeedToBeChanged: ",msg2.GetString());
	       bNeedToChangeContent = TRUE;
	    }
	 }

	ALLOCBUFFER(str, MediumPrintLen);
	sprintf(str,"need to change content %d, is Content HD 1080 supported in: Initial SCM:  %d, Current SCM %d, ", bNeedToChangeContent, isInitialContentHD1080Supported, isCurrentContentHD1080Supported);
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsContentOutHDResOrMpiNeedToBeChanged: The details: ",str);
	DEALLOCBUFFER(str);

	return bNeedToChangeContent;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsContentInHDResOrMpiNeedToBeChanged(BYTE isSecondaryCondition) const
{
	if(isSecondaryCondition)// in case of secondary we can't change content mode, just continue to be with close
		return FALSE;		// outgoing channels and disconnected (or not connected) to the content bridge.

	BYTE  bNeedToChangeContent 				= FALSE;
	BYTE  isInitialContentHD1080Supported	= FALSE;
	BYTE  isCurrentContentHD1080Supported	= FALSE;

	if( m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && !m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) )
	{
		m_pIpInitialMode->Dump("CH323ChangeModeCntl::IsContentInHDResOrMpiNeedToBeChanged: initial rx off but on current on, Initial", eLevelInfoNormal);
		m_pIpCurrentMode->Dump("CH323ChangeModeCntl::IsContentInHDResOrMpiNeedToBeChanged: initial rx off but on current on, Current", eLevelInfoNormal);
	}
	if(m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation)
			&& (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) != eUnknownAlgorithemCapCode )
	{
		CapEnum initialContentInType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		CapEnum currentContentInType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		if(initialContentInType != eH264CapCode || currentContentInType != eH264CapCode)
		  return FALSE;

		isInitialContentHD1080Supported = m_pIpInitialMode->isHDContent1080Supported(cmCapReceive);
		isCurrentContentHD1080Supported = m_pIpCurrentMode->isHDContent1080Supported(cmCapReceive);

		bNeedToChangeContent =  (isInitialContentHD1080Supported != isCurrentContentHD1080Supported);
	    if(!bNeedToChangeContent && !isCurrentContentHD1080Supported )
		{
			BYTE  initialRcvContentHD720Mpi = m_pIpInitialMode->isHDContent720Supported(cmCapReceive);
			BYTE  currentRcvContentHD720Mpi = m_pIpCurrentMode->isHDContent720Supported(cmCapReceive);
			if(/*BRIDGE-15567: do not assume 720p resolution as XGA is used in TIP. - initialRcvContentHD720Mpi && currentRcvContentHD720Mpi &&*/ (initialRcvContentHD720Mpi != currentRcvContentHD720Mpi))
			{
				CMedString msg1;
				msg1 << " RCV MPI of HD 720 Content: Initial=" << (DWORD)initialRcvContentHD720Mpi << ", Current=" << (DWORD)currentRcvContentHD720Mpi;
				PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsContentInHDResOrMpiNeedToBeChanged: ",msg1.GetString());
				bNeedToChangeContent = TRUE;
			}
		}
	}

	ALLOCBUFFER(str, MediumPrintLen);
	sprintf(str,"need to change content %d, is Content HD 1080 supported in: Initial SCM:  %d, Current SCM %d, ", bNeedToChangeContent, isInitialContentHD1080Supported, isCurrentContentHD1080Supported);
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsContentInHDResOrMpiNeedToBeChanged: The details: ",str);
	DEALLOCBUFFER(str);

	return bNeedToChangeContent;
}

//HP content:
BYTE CH323ChangeModeCntl::IsContentOutProfileNeedToBeChanged(BYTE isSecondaryCondition) const
{
	if(isSecondaryCondition)// in case of secondary we can't change content mode, just continue to be with close
		return FALSE;		// outgoing channels and disconnected (or not connected) to the content bridge.

	BYTE  bNeedToChangeProfile = FALSE;
	CapEnum initialContentType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum currentContentType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

	if(initialContentType != eH264CapCode || currentContentType != eH264CapCode)
	   return FALSE;

	BYTE initialContentProfile = m_pIpInitialMode->IsH264HighProfileContent(cmCapTransmit);
	BYTE currentContentProfile = m_pIpCurrentMode->IsH264HighProfileContent(cmCapTransmit);

	ALLOCBUFFER(str, MediumPrintLen);
	sprintf(str,"initialContentProfile %d, currentContentProfile %d, ", initialContentProfile, currentContentProfile);
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsContentOutProfileNeedToBeChanged: The details: ",str);
	DEALLOCBUFFER(str);

	bNeedToChangeProfile =  (initialContentProfile != currentContentProfile);

	return bNeedToChangeProfile;
}

BYTE CH323ChangeModeCntl::IsContentInProfileNeedToBeChanged(BYTE isSecondaryCondition) const
{
	if(isSecondaryCondition)// in case of secondary we can't change content mode, just continue to be with close
		return FALSE;		// outgoing channels and disconnected (or not connected) to the content bridge.

	BYTE  bNeedToChangeProfile = FALSE;
	if( m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && !m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) )
	{
		m_pIpInitialMode->Dump("CH323ChangeModeCntl::IsContentInProtocolNeedToBeChanged: initial rx off but on current on, Initial", eLevelInfoNormal);
		m_pIpCurrentMode->Dump("CH323ChangeModeCntl::IsContentInProtocolNeedToBeChanged: initial rx off but on current on, Current", eLevelInfoNormal);
	}
	if(m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation)
			&& (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) == eH264CapCode )
	{
	   	BYTE initialContentProfile = m_pIpInitialMode->IsH264HighProfileContent(cmCapReceive);
		BYTE currentContentProfile = m_pIpCurrentMode->IsH264HighProfileContent(cmCapReceive);

	   ALLOCBUFFER(str, MediumPrintLen);
	   sprintf(str,"initialContentProfile %d, currentContentProfile %d, ", initialContentProfile, currentContentProfile);
	   PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::IsContentInProfileNeedToBeChanged: The details: ",str);
	   DEALLOCBUFFER(str);
	   bNeedToChangeProfile =  (initialContentProfile != currentContentProfile);
	}

	return bNeedToChangeProfile;
}











// Romem 1.11.10
/////////////////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::ChangeModeFromScm(eChangeModeState changeModeState, ePartyMediaState mediaState, cmCapDataType dataType, ERoleLabel role, CRsrcParams** avcToSvcTranslatorRsrcParams,CRsrcParams* pMrmpRsrcParams)
{
	cmCapDirection direction;
   /* if (changeModeState == eNotNeeded)
    {
        PTRACE2(eLevelError, "ChangeModeFromScm - change mode not needed - try to Connect to video bridge - Name ", m_partyConfName);
        ChangeVideoBridgeState();
        //ChangeVideoBridgeStateAccordingToNewMode();
        return;
    }
	else */

    if (GetDirectionByChangeModeState(changeModeState, direction) == FALSE)
	{
        PTRACE2(eLevelError, "CH323ChangeModeCntl::ChangeModeFromScm - Incorrect change mode state - Name ", m_partyConfName);
        return;
	}

	PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::ChangeModeFromScm  - Name ", m_partyConfName);

	if (m_pIpInitialMode->GetConfType() == kCop && !m_bCascadeIsLecturer)
	{
		BYTE iCopLevel = m_pIpInitialMode->GetCopTxLevel();
		if (iCopLevel < NUMBER_OF_COP_LEVELS)
		{
			DWORD transmitRate = (m_pCopVideoTxModes->GetVideoMode(iCopLevel))->GetBitRate();
			m_pIpInitialMode->SetVideoBitRate(transmitRate, cmCapTransmit, kRolePeople);
		}
	}
	else
	{
	/*In case the change mode is after the party was connected and the incoming rate is lower than the transmit rate
	  we want to update the receive rate. We did *m_pIpInitialMode = *m_pIpCurrentMode,
	  So we need to update the initial in the receive, otherwise, the change mode might fail
	  in the check in the function AreAllMustChangesInModeSucceeded.*/
	DWORD transmitRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);
	DWORD receiveRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapReceive);
	if(receiveRate < transmitRate)
		m_pIpInitialMode->SetVideoBitRate(transmitRate, cmCapReceive);
	}

	ChangeMode(changeModeState, dataType, direction, role, mediaState, avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);
	*m_pScmSentToParty = *m_pIpInitialMode;
	m_state = CHANGEVIDEO;
//	m_bChannelsUpdatedWithoutChangeMode = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// if any change mode occurred during the change mode process
// the new scm will be recorded at m_pInitialScm.
void  CH323ChangeModeCntl::OnConfChangeModeContent(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeContent : Name - ",m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
// if any change mode occured during the change mode process
// the new scm will be recorded at m_pInitialScm.
void  CH323ChangeModeCntl::OnConfChangeModeVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeVideo : Name - ",m_partyConfName);
	m_bIsNewScm = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnConfChangeModeRealloc(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeRealloc : Name - ",m_partyConfName);
}
/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnConfChangeModeUpdateLegacyStatus(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeUpdateLegacyStatus : Name - ",m_partyConfName);
}
/////////////////////////////////////////////////////////////////////////////
// if any change mode occured during the change mode process
// the new scm will be recorded at m_pInitialScm.
void  CH323ChangeModeCntl::OnConfChangeModeOther(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeOther : Name - ",m_partyConfName);
}

/////////////////////////////////////////////////////////////////////////////
//At this point, CHANGEMODEMSG arrives without a new scm, so we can't deserialize to m_pIpInitialMode from pParam
/*
void CH323ChangeModeCntl::OnConfChangeModeMsgChangeVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeMsgChangeVideo : Name - ",m_partyConfName);
	//Since this party won't handle the lsd until he finished the change mode process, we
	//need to avoid a situation, in which the gets asymetric time slot, while he is in this
	//state. Therefore, we disconnect it from the bridge now
	if (m_pIpInitialMode->GetConfType() == kVideoSwitch) //We need to do changes only in VSW
	{
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeMsgChangeVideo:VSW conf - openLsd Name - ",m_partyConfName);

		if(m_pVidConnection)
			if (m_pVidConnection->IsConnected())
			{
				ON(m_isVidConn);
				m_pVidConnection->DisConnect();
				m_bConfWaitToEndChangeModeForFecc = TRUE;
			}
	}
	else
	{
		m_bConfWaitToEndChangeModeForFecc = TRUE;
		m_lsdChangeModeRequestRate = 0;
		m_lsdOldChangeModeRequestRate = 0;
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeMsgChangeVideo:CP conf - openLsd Name - ",m_partyConfName);
	}
}*/

/////////////////////////////////////////////////////////////////////////////
/*void CH323ChangeModeCntl::OnConfChangeModeMsgChangeContent(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeMsgChangeContent : Name - ",m_partyConfName);
	m_lsdOldChangeModeRequestRate = 0;
	m_bConfWaitToEndChangeModeForFecc = TRUE;
}*/

/////////////////////////////////////////////////////////////////////////////
//we got here when in 320 open LSD dynamic channel so in 323 we need to send flowControl on the video and send new TSS
/*void CH323ChangeModeCntl::OnConfChangeModeMsgIdle(CSegment* pParam)
{
    WORD type_call;

    *pParam >> type_call;

	if (m_pIpInitialMode->GetConfType() == kVideoSwitch) //We need to do changes only in VSW
	{
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeMsgIdle:VSW conf - openLsd Name - ",m_partyConfName);
		switch (type_call)
		{
		case OPENDATACHANNEL:
			OpenDaynamicLsd();
			break;
		case CLOSEDATACHANNEL:
			CloseDaynamicLsd();
			break;
		default:
			{
				PTRACE(eLevelError,"CH323ChangeModeCntl::OnConfChangeModeMsgIdle: no case found");
				PASSERT(1);
				return;
			}
		}
	}
	else
	{
		//In case of CP we shouldn't do anything to the video channels but be should inform to the conf about the
		//endChangeMode
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnConfChangeModeMsgIdle :CP conf -  Name - ",m_partyConfName);
		m_lsdChangeModeRequestRate = 0;
		m_lsdOldChangeModeRequestRate = 0;
		m_pTaskApi->EndChangeModeParty(m_pParty);
	}
}*/

//////////////////////////////////////////////////////////////////////////////
/*void CH323ChangeModeCntl::OpenDaynamicLsd()
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OpenDaynamicLsd : Name - ",m_partyConfName);

	WORD lsdBitRateIfNeeded = 0;
	if (m_lsdChangeModeRequestRate)
	{
		BYTE isIpOnlyConf		= (m_pConf->GetCommConf()->GetNetwork() == NETWORK_H323);
		DWORD partyTdmRate = 0;
		DWORD partyRate	= 0;
		WORD  partyVidContentTdmRate = 0;
		DWORD lsdBitRate		= 0;
	    CLsdMlpMode  lsdMode	= m_pInitialScm->m_lsdMlpMode;

		WORD  contentAmscRateType = m_pScm->GetContentModeContentRate();
		Get_Content_command_Bitrate(contentAmscRateType,&partyVidContentTdmRate);


        lsdMode.SetLsdMode(m_lsdChangeModeRequestRate);
        m_pScm->SetLsdMlpMode(lsdMode);
		*m_pInitialScm = *m_pScm;

		if(!isIpOnlyConf)
		{
			Get_Lsd_Mlp_Command_BitRate((BYTE)m_lsdChangeModeRequestRate, (WORD *)&lsdBitRate);

			if(IsLSD() && m_pIpCurrentMode->GetH320ModeType(cmCapData) != m_lsdChangeModeRequestRate)
			{
				m_pIpInitialMode->SetDataBitRate(lsdBitRate, cmCapReceiveAndTransmit);
				m_pIpCurrentMode->SetDataBitRate(lsdBitRate, cmCapReceiveAndTransmit);
				lsdBitRateIfNeeded = lsdBitRate;
			}

			//We should return to the original m_tdmVideoRate before reducing in the last change, for prevent the reduce
			//m_tdmVideoRate every time.
			//Get_Lsd_Mlp_Command_BitRate((BYTE)oldLsdMode, (WORD *)&oldLsdBitRate);

			m_tdmVideoRate	= m_tdmVideoRate + m_lsdOldChangeModeRequestRate - lsdBitRate; //for 100 bit per second
			m_videoRate		= (m_tdmVideoRate * 96) / 100;	//reducing BCH

			//In case there is no content partyTdmRate == m_tdmVideoRate
			partyTdmRate	= m_tdmVideoRate - partyVidContentTdmRate;
			partyRate		= (partyTdmRate * 96) / 100;	//reducing BCH
			//even though it's VSW, we can change the initial SCM here, because the conf sent this message
			// In case the content rate is > 0 (Content ON) we do not need to remove the content rate from the TR bit rate,
			// Only from  the RCV one.
			if ( partyVidContentTdmRate > 0 )
			{
				m_pIpInitialMode->SetVideoBitRate(m_videoRate, cmCapTransmit);
				m_pIpCurrentMode->SetVideoBitRate(m_videoRate, cmCapTransmit);
				m_pIpInitialMode->SetVideoBitRate(partyRate, cmCapReceive);
				m_pIpCurrentMode->SetVideoBitRate(partyRate, cmCapReceive);
			}
			else
			{
				m_pIpInitialMode->SetVideoBitRate(partyRate, cmCapReceiveAndTransmit);
				m_pIpCurrentMode->SetVideoBitRate(partyRate, cmCapReceiveAndTransmit);
			}

			if(m_pVidConnection)
				if (m_pVidConnection->IsConnected())
				{
					ON(m_isVidConn);
					m_pVidConnection->DisConnect();
					m_bConfWaitToEndChangeModeForFecc = TRUE;
				}

			m_pMuxDesc->SetMuxUserType(H323_PARTY);
			DistributeIpTimeSlots();
			ConvertIpTss2MuxTss(m_pIpRsrcDesc);
			// Sending the correct rates both for the party and the H323Cntl
			m_pPartyApi->HandleOpenDaynamicLsdChannel(m_videoRate,lsdBitRateIfNeeded, partyRate);
		}

		m_lsdChangeModeRequestRate = 0;
		m_lsdOldChangeModeRequestRate = 0;
	}

	else
		PTRACE(eLevelError,"CH323ChangeModeCntl::OpenDaynamicLsd: m_lsdChangeModeRequestRate is zero");
}*/

//////////////////////////////////////////////////////////////////////////////
//When we in mix conference dynamic lsd and the isdn endpoint released and free it's lsd channel
//we should increase video channel and remove lsd from the scm.
/*void CH323ChangeModeCntl::CloseDaynamicLsd()
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::CloseDaynamicLsd : Name - ",m_partyConfName);

	WORD lsdBitRateIfNeeded = 0;
	if (!m_lsdChangeModeRequestRate)
	{
		BYTE isIpOnlyConf		= (m_pConf->GetCommConf()->GetNetwork() == NETWORK_H323);
		DWORD lsdBitRate		= 0;
	    CLsdMlpMode  lsdMode	= m_pInitialScm->m_lsdMlpMode;

        lsdMode.SetLsdMode(m_lsdChangeModeRequestRate);
        m_pInitialScm->SetLsdMlpMode(lsdMode);
		*m_pScm = *m_pInitialScm;

		if(!isIpOnlyConf)
		{
			WORD currH320ModeType((BYTE)m_pIpCurrentMode->GetH320ModeType(cmCapData));
			Get_Lsd_Mlp_Command_BitRate(currH320ModeType, (WORD *)&lsdBitRate);

			m_tdmVideoRate = m_tdmVideoRate + lsdBitRate; //for 100 bit per second
			m_videoRate = (m_tdmVideoRate * 96) / 100;
			//even though it's VSW, we can change the initial SCM here, because the conf sent this message
			m_pIpInitialMode->SetVideoBitRate(m_videoRate, cmCapReceiveAndTransmit);
			m_pIpCurrentMode->SetVideoBitRate(m_videoRate, cmCapReceiveAndTransmit);

			m_pMuxDesc->SetMuxUserType(H323_PARTY);
			DistributeIpTimeSlots();
			ConvertIpTss2MuxTss(m_pIpRsrcDesc);

			m_pPartyApi->HandleOpenDaynamicLsdChannel(m_videoRate,lsdBitRateIfNeeded);

			if(m_pVidConnection && m_pVidConnection->IsConnected())
			{
				ON(m_isVidConn);
				m_pVidConnection->DisConnect();
				m_bConfWaitToEndChangeModeForFecc = TRUE;
			}
		}
	}
	else
		PTRACE(eLevelError,"CH323ChangeModeCntl::ChangeLsd: m_lsdChangeModeRequestRate is not zero");
}*/

//////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnAudConnectPartyIdleOrAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnAudConnectPartyIdleOrAnycase : Name - ",m_partyConfName);
	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2(eLevelInfoNormal,"CH323AddPartyCntl::OnAudConnectPartyIdleOrAnycase : Connect has received after disconnect. Name - ",m_partyConfName);
	}

	else
	{
		EMediaDirection eMediaDirection = HandleAudioBridgeConnectedInd(pParam);
		if (eMediaDirection == eMediaIn || eMediaDirection == eMediaInAndOut)
		{
			//BOOL bMuteStatus = m_pIpCurrentMode->GetMediaMode(cmCapAudio, cmCapReceive).GetMute();
            CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
            EOnOff eOnOff = eOff;
            if (CPObject::IsValidPObjectPtr(pConfParty))
            {
                BOOL bMuteStatus = pConfParty->IsAudioMutedByParty();
                eOnOff = bMuteStatus ? eOn : eOff;
            }
            else
                DBGPASSERT(GetPartyRsrcId());
			m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOnOff, PARTY);
			//fix for VNGR-24401
			if (pConfParty->IsAudioMutedByOperator())
			{
					PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnAudConnectPartyIdleOrAnycase: Audio muted by Operateor", GetPartyRsrcId());
					eOnOff = eOn;
					m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOnOff, OPERATOR);
			}
			if (eMediaDirection == eMediaOut || eMediaDirection == eMediaInAndOut)
			{
				if (pConfParty->IsAudioBlocked())
				{
					PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnAudConnectPartyIdleOrAnycase: Audio blocked by Operateor", GetPartyRsrcId());
					eOnOff = eOn;
					m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaOut, eOnOff, OPERATOR);
				}
			}
		}

	/*	if (AreTwoDirectionsConnectedToAudioBridge())
		{
			if (m_voice || (IsUndefinedParty() && IsRemoteCapNotHaveVideo()) || IsOutDirectionConnectedToVideoBridge())
			{
				if (IsUndefinedParty() && IsRemoteCapNotHaveVideo())
					m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_AUDIO_ONLY);
				m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED);
				// In GL1 we update the CDR only regarding connect/disconnect state
				// Meaning that if we updated the CDR regarding the connection of the party -
				// There is no need to update again.
				//UpdatePartyStateInCdr();
			}
		}
*/	}
}

//////////////////////////////////////////////////////////////////////////////
// video brdg connection ack
void  CH323ChangeModeCntl::OnVidBrdgConVideo(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnVidBrdgConVideo");
	OnCamVidBrdgConVideo(pParam);
//	StartPartyMsgOnScreen();
}

//////////////////////////////////////////////////////////////////////////////
// CAM and video brdg connection ack
void  CH323ChangeModeCntl::OnCamVidBrdgConVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnCamVidBrdgConVideo : Name - ",m_partyConfName);

	UpdateVideoBrigdeStateAfterBridgeConnected(pParam);

	if (AreTwoDirectionsConnectedToVideoBridge() || (IsOutDirectionConnectedToVideoBridge() && m_isCodianVcr))
	{
		m_state = IDLE;
		if (!m_bIsNewScm)
			ChangeOther();
		else
		{
			m_bIsNewScm = FALSE;
			DispatchChangeModeEvent();  // redo the new scm
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnVidBrdgConContent(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVidBrdgConContent : Name - ",m_partyConfName);
    UpdateVideoBrigdeStateAfterBridgeConnected(pParam);

}

///////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnVidBrdgConChangeAll(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVidBrdgConChangeAll : Name - ",m_partyConfName);
	UpdateVideoBrigdeStateAfterBridgeConnected(pParam);
}
///////////////////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnVidBrdgConUpdateLegacyState(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVidBrdgConUpdateLegacyState : Name - ",m_partyConfName);

	 UpdateVideoBrigdeStateAfterBridgeConnected(pParam);

	 ContinueToChangeContent();
}
///////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::UpdateVideoBrigdeStateAfterBridgeConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::UpdateVideoBrigdeStateAfterBridgeConnected : Name - ",m_partyConfName);
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::UpdateVideoBrigdeStateAfterBridgeConnected : Connect has received after disconnect. Name - ",m_partyConfName);
	}
	else
	{
		HandleVideoBridgeConnectedInd(pParam);
		UpdatePartyStateAfterVideoBridgeConnected();
	}
}

//////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnVidBrdgConVideoIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVidBrdgConVideoIdle - Ignored : Name - ",m_partyConfName);
	// Escalations (VNG) VNGFE-6939.
	UpdateVideoBrigdeStateAfterBridgeConnected(pParam);

    UpdatePartyStateAfterVideoBridgeConnected();

//    UpdatePartyStateInCdr();
//    StartPartyMsgOnScreen();
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnVideoInBrdgUpdatedChangeVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVideoInBrdgUpdatedChangeVideo: Name - ",m_partyConfName);
	WORD status;
	*pParam >> status;

	if (IsInDirectionConnectedToVideoBridge())
		HandleVideoBridgeUpdate(status, eUpdateVideoIn);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnVideoInBrdgUpdatedAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVideoInBrdgUpdatedAnycase: Name - ",m_partyConfName);
	WORD status;
	*pParam >> status;
	if (status)
		m_eUpdateState = (EUpdateBridgeMediaAndDirection)(m_eUpdateState - eUpdateVideoIn);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnVideoOutBrdgUpdatedChangeVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVideoOutBrdgUpdatedChangeVideo: Name - ",m_partyConfName);
	WORD status;
	BOOL isParams;
	*pParam >> status >> isParams;
	if (status == statOK && isParams)
		HandleLprUpdatedIndications(pParam);

	if (IsOutDirectionConnectedToVideoBridge())
		HandleVideoBridgeUpdate(status, eUpdateVideoOut);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnVideoOutBrdgUpdatedAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVideoOutBrdgUpdatedAnycase: Name - ",m_partyConfName);
	WORD status;
	BOOL isParams;
	*pParam >> status >> isParams;

	if (status == statOK)
	{
		m_eUpdateState = (EUpdateBridgeMediaAndDirection)(m_eUpdateState - eUpdateVideoOut);
		if(isParams)
		{
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVideoOutBrdgUpdatedAnycase - LPR params in ack msg, Name - ",m_partyConfName);
			HandleLprUpdatedIndications(pParam);
		}
		else
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVideoOutBrdgUpdatedAnycase - no isParams, Name - ",m_partyConfName);
	}
	else
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVideoOutBrdgUpdatedAnycase - status not OK, Name - ",m_partyConfName);
}

////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::HandleLprUpdatedIndications(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::HandleLprUpdatedIndications: Name - ",m_partyConfName);

	CAckParams* pAckParams = new CAckParams;

	pAckParams->DeSerialize(NATIVE,*pParam);

	if(CPObject::IsValidPObjectPtr(pAckParams))
	{
		PTRACE (eLevelInfoNormal,"CH323ChangeModeCntl::HandleLprUpdatedIndications : LPR Request ");

		CLPRParams*	pLPRParams = pAckParams->GetLPRParams();

		if(CPObject::IsValidPObjectPtr(pLPRParams))
		{
			CSegment* pSeg = new CSegment;
			*pSeg << pLPRParams->GetLossProtection() << pLPRParams->GetMTBF() << pLPRParams->GetCongestionCeiling()
				<< pLPRParams->GetFill() << pLPRParams->GetModeTimeout();

			DispatchEvent(PARTY_LPR_VIDEO_OUT_RATE_UPDATED,pSeg);
			POBJDELETE(pSeg);
		}
		else
			PTRACE (eLevelInfoNormal,"CH323ChangeModeCntl::HandleLprUpdatedIndications : No LPR Params ");

	}
	POBJDELETE(pAckParams);
}

////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::HandleVideoBridgeUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::HandleVideoBridgeUpdate: Name - ",m_partyConfName);
	BYTE bVideoBridgeEndUpdate = CheckVideoBridgeEndUpdate(status, eUpdatedBridges);

	if (bVideoBridgeEndUpdate && !m_bPartyEndChangeVideoInformParty)
	{
		DeleteTimer(CHANGETOUT);
		m_state = IDLE;

		if (m_bIsNewScm)
		{
			m_bIsNewScm = FALSE;
			DispatchChangeModeEvent();  // redo the new scm
		}
		else
			ChangeOther();
	}
}


/////////////////////////////////////////////////////////////////////////////
// video brdg disconnection indication
void  CH323ChangeModeCntl::OnVidBrdgDisConVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVidBrdgDisConVideo : Name - ",m_partyConfName);

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
		return;
	}
	else if(resStat != statOK)
	{// if we receive disconnect from the bridge with status that is not OK we should change call the to secondary
		SetPartyToSecondaryAndStopChangeMode(SECONDARY_CAUSE_NO_VIDEO_CONNECTION);
	}

    if (m_state == REALLOCATE_RSC)
    {
        BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_H323_FREE_VIDEO_RESOURCES);
        eVideoPartyType eCurrentVideoType;
		//eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpCurrentMode); VNGR-9495
        if(m_bContinueChangeModeAfterReAlloc)
        	 eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode);
        else
        {
        	PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnVidBrdgDisConVideo - the reallocation did not arraive from change mode - continue according to current");
            eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpCurrentMode);
        }
		if((bEnableFreeVideoResources == FALSE) || (IsUndefinedParty() == FALSE) || (m_pRmtCapH323->GetNumOfVideoCap()>0))
			eCurrentVideoType = (eCurrentVideoType == eVideo_party_type_none) ? eCP_H264_upto_CIF_video_party_type : eCurrentVideoType;

		DWORD artCapacity = 0;
		artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, TRUE /*add audio + video for current*/);
		m_artCapacity = artCapacity;
        CreateAndSendReAllocatePartyResources(eIP_network_party_type, eCurrentVideoType, eAllocateAllRequestedResources,FALSE/*reAllocRtm*/,FALSE/*isSyncMessage*/,FALSE/*IsEnableSipICE*/,artCapacity);
        return;
    }

	if (m_pIpInitialMode && (m_pIpInitialMode->GetConfType() != kCp) && (m_pIpInitialMode->GetConfType() != kCop))
	{
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
		if (pConfParty)
		{
		    DWORD partyState = pConfParty->GetPartyState();
		    if (partyState == PARTY_SECONDARY)
		        m_pTaskApi->PartyChangeVidMode(m_pParty,TRUE); //in order to remove from the sub-group, which can be done only after the video bridge is disconnected.
		}
		else
		    PTRACE(eLevelError,"CH323ChangeModeCntl::OnVidBrdgDisConVideo - pConfParty is NULL");
	}

	//in case we arrive here after the party end change video, so state is CHANGEVIDEO
	if (!m_bPartyEndChangeVideoInformParty)
		m_state = IDLE;

    if( (m_state != CHANGE_ALL_MEDIA) &&
		((m_state != CHANGEVIDEO) || ((m_state == CHANGEVIDEO) && (m_changeModeInitiator == eNoInitiator))))
	{  //1) In case of CHANGE_ALL_MEDIA, we wait for the party in order to continue the process
	   //2) In case of CHANGEVIDEO, we wait for the "OnPartyEndChangeVideo", in order to continue the process
	  // 3) In case of CHANGEVIDEO not as a result of sending change mode to party but as
	  // a result of connecting to the video bridge: And before getting its response on the
	  // connecting, we disconnected it: So now we want to reconnect it again.
		if (!m_bIsNewScm)
			if (m_pIpInitialMode && m_pIpCurrentMode)
			{
				CComModeH323* pTmpModeH323 = NULL;
				if(!IsInitialContentModeEqualToCurContentMode())
				{
					pTmpModeH323 = new CComModeH323;
					*pTmpModeH323 = *m_pIpInitialMode;
				}


				*m_pIpInitialMode = *m_pIpCurrentMode;


				if(pTmpModeH323)
				{
					UpdateInitialWithNewContentMode(pTmpModeH323);
					POBJDELETE (pTmpModeH323);
				}


			}



		EndChangeMode();
	}
}

//////////////////////////////////////////////////////////////////////////////
// content brdg connection ack
int  CH323ChangeModeCntl::OnContentBrdgConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnContentBrdgConnected : Name - ",m_partyConfName);
	CH323PartyCntl::OnContentBrdgConnected(pParam);
	if (!m_isContentConn)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnContentBrdgConnected : Problems while connecting. Name - ",m_partyConfName);
		if (m_isFaulty)
		{
			m_pTaskApi->PartyDisConnect(H323_CALL_CLOSED_PROBLEM_WITH_CONTENT_CONNECTION_TO_MCU,m_pParty);
			return -1;
		}

	}

	if (!m_bIsNewScm)
		ChangeOther();
	else
	{
		m_state = IDLE;
		m_bIsNewScm = FALSE;
		DispatchChangeModeEvent();  // redo the new scm
	}

    return 0;
}


//////////////////////////////////////////////////////////////////////////////
// content brdg connection ack
void  CH323ChangeModeCntl::OnContentBrdgConnectedAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnContentBrdgConnectedAnycase : Name - ",m_partyConfName);
	PASSERTMSG(1, "worng state");
}
//////////////////////////////////////////////////////////////////////////////
// content brdg connection ack
void CH323ChangeModeCntl::OnXCodeBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnXCodeBrdgDisconnected", GetPartyRsrcId());
	CPartyCntl::OnXCodeBrdgDisconnected(pParam);

}
//////////////////////////////////////////////////////////////////////////////
int CH323ChangeModeCntl::OnContentBrdgDisconnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnContentBrdgDisconnected : Name - ",m_partyConfName);

	CSegment* localSeg = new CSegment(*pParam);
	WORD status;
	*localSeg >> status;
	POBJDELETE(localSeg);

	int isErr = CH323PartyCntl::OnContentBrdgDisconnected(pParam);

	if (isErr != 0)
	{
		if (statInconsistent == status)
		    {
		    	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnContentBrdgDisconnected : Name - ",m_partyConfName);
		    	m_pTaskApi->PartyDisConnect(H323_CALL_CLOSED_PROBLEM_WITH_ACTIVE_CONTENT_SLAVE,m_pParty);
		    }
		else
			if(status == statInvalidPartyInitParams)
			{
				DBGPASSERT(status);
				PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnContentBrdgDisconnected Invalid content party params : Name - ",m_partyConfName);

				DWORD details = 0;
				details = DIFFERENT_ROLE;
				SetPartyToContentSecondaryMode(details);
			}
			else
				m_pTaskApi->PartyDisConnect(H323_CALL_CLOSED_PROBLEM_WITH_CONTENT_CONNECTION_TO_MCU,m_pParty);

		return -1;
	}
	/*EndChangeMode();*/
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
/*void CH323ChangeModeCntl::OnDataBrdgConVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnDataBrdgConVideo : Name - ",m_partyConfName);
	WORD  status;
	WORD  mlpType;
	*pParam >> mlpType >> status;
	if (status)
	{
		PTRACE2(eLevelError,"CH323ChangeModeCntl::OnDataBrdgConVideo failure : Name - ",m_partyConfName);
		EndChangeMode();
	}
	else
	{
		ON(m_isMlpConn);
		if (m_isVidConn)     // if video connected already to brdg
			EndChangeMode();
		m_pTaskApi->UpdateDB(m_pParty,T120CON,TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnDataBrdgConOther(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnDataBrdgConOther : Name - ",m_partyConfName);
	WORD  status;
	WORD  mlpType;
	*pParam >> mlpType >> status;
	if ( status )
	{
	  PTRACE2(eLevelError,"CH323ChangeModeCntl::OnDataBrdgConOther failure : Name - ",m_partyConfName);
	  EndChangeMode();
	}
	else
	{
		ON(m_isMlpConn);
		EndChangeMode();
		m_pTaskApi->UpdateDB(m_pParty,T120CON,TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnDataBrdgConIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnDataBrdgConIdle : Name - ",m_partyConfName);
	WORD  status;
	WORD  mlpType;
	*pParam >> mlpType >> status;
	if ( status )
	{
	  	PTRACE2(eLevelError,"CH323ChangeModeCntl::OnDataBrdgConIdle failure : Name - ",m_partyConfName);
	    return;
	}
	else
	{
		ON(m_isMlpConn);
		m_pTaskApi->UpdateDB(m_pParty,T120CON,TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnDataBrdgDisConOther(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnDataBrdgDisConOther : Name - ",m_partyConfName);

	WORD  status;
	*pParam >> status;
	DBGPASSERT(status);
	OFF(m_isMlpConn);
	OFF(m_isDatConnReq); // Now it's possible to send another connect request if necessary.

	if (!m_bIsNewScm)
		if ((m_pIpInitialMode) && (m_pIpCurrentMode))
			*m_pIpInitialMode = *m_pIpCurrentMode; //in order to end the change mode

	EndChangeMode();
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnDataBrdgDisChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnDataBrdgDisChangeMode : Name - ",m_partyConfName);
	WORD  status;
	*pParam >> status;
	DBGPASSERT(status);
	OFF(m_isMlpConn);
	OFF(m_isDatConnReq); // Now it's possible to send another connect request if necessary.

	//we don't call to the EndChangeMode, since the party is in change mode process
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyH323DisconnectMmlp(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyH323DisconnectMmlp : Name - ",m_partyConfName);

	if (m_pMlpCntl)
	{
		if (m_pMlpCntl->IsConnected())
		{
			ON(m_isMlpConn);
			m_pMlpCntl->DisConnect(m_pParty);
			m_pTaskApi->UpdateDB(m_pParty,T120CON,FALSE);
		}
	}
	else
		return;
}*/
/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyEndChangeReAlloc(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeReAlloc : Name - ",m_partyConfName);
    switch (m_OldState)
    {
        case CHANGECONTENT:
            OnPartyEndChangeContent(pParam);
            break;
        case CHANGEVIDEO:
            OnPartyEndChangeVideo(pParam);
            break;
        case CHANGEOTHER:
            OnPartyEndChangeOther(pParam);
            break;
        default:
            DBGPASSERT(m_OldState);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyEndChangeIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeIdle : Name - ",m_partyConfName);
	if (IsValidTimer(CHANGETOUT))
		DeleteTimer(CHANGETOUT);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyEndChangeOther(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeOther : Name - ",m_partyConfName);
	DeleteTimer(CHANGETOUT);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyEndChangeContent(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeContent");
	DeleteTimer(CHANGETOUT);

	CComModeH323* pH323Mode = new CComModeH323;
	DWORD           status;

	// party sends the current mode in case of success and the target mode in case of failure.
	pH323Mode->DeSerialize(NATIVE,*pParam);
	*pParam >> status;

	// Since the close of the outgoing content channel has set the flag to TRUE and disconnect
	// the party from the content bridge, a solution for that case need to be found.
	BYTE bIsSecondaryCondition = FALSE;
	DWORD details = 0;
	bIsSecondaryCondition	= IsSecondaryConditionForContent(details);
	if(pH323Mode->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
	{
		PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeContent: content out open.");
		if (m_bNoContentChannel)
		{//if content is reopened, we need to check the secondary condition again, because it might be not valid now.
			PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeContent: close-content flag is ON.");
			if(bIsSecondaryCondition == FALSE)
			{
				PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeContent: Upgrade content from secondary.");
				OFF(m_bNoContentChannel);
			}
		}
	}

	if (m_bNoContentChannel)
	{
		PTRACE2(eLevelError,
			"CH323ChangeModeCntl::OnPartyEndChangeContent: Party doesn't have content caps or already closed it's content channels ",m_partyConfName);
		m_state = CHANGEOTHER;
		ChangeOther();
	}

	else //content channel is open
	{
		if (status)
		{
			// if the scm changed in the middle of the change content process, try to send the new scm.
			// if content has failed and the scm is the same, continue with the change mode process.
			if (m_bIsNewScm)
			{
				PTRACE2(eLevelError,
					"CH323ChangeModeCntl::OnPartyEndChangeContent: Failed to change mode content. Redo new scm ",m_partyConfName);
				m_state = IDLE;
				m_bIsNewScm = FALSE;
				DispatchChangeModeEvent();  // redo the new scm
			}
			else
			{
				PTRACE2(eLevelError,
					"CH323ChangeModeCntl::OnPartyEndChangeContent: Failed to change mode content. Continue change other ",m_partyConfName);
				*m_pIpCurrentMode = *pH323Mode;
				m_state = CHANGEOTHER;
				ChangeOther();
			}
		}
		else
		{
			PTRACE2(eLevelInfoNormal,"***CH323ChangeModeCntl::OnPartyEndChangeContent: Success. Name - ",m_partyConfName);
			*m_pIpCurrentMode = *pH323Mode;

			if (m_bIsNewScm)
			{
				//first we need to inform content bridge if it waits for party response and there is no change in content scm:
				BYTE bContentBridgeConnected = m_pContentBridge && m_pContentBridge->IsConnected() && m_isContentConn;
				BYTE bContentBridgeWaiting   = FALSE;
				if (bContentBridgeConnected)
				{
					bContentBridgeWaiting = !m_bNoContentChannel && m_pContentBridge->IsPartyWaitForRateChange(m_pParty);
					if (bContentBridgeWaiting == FALSE)
						PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeContent - new scm arrived + content bridge is connected, but isn't waiting for party. Name -",m_partyConfName);
				}
				else
					PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeContent - new scm arrived + content bridge isn't connected. Name - ",m_partyConfName);

				if (bContentBridgeConnected && bContentBridgeWaiting)
				{
					BYTE bNeedToChangeContent = IsContentRateNeedToBeChanged();
					if (!bNeedToChangeContent)
					{
						BYTE bIsContentConnected  = TRUE;

						if (m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))
								bIsContentConnected = FALSE;//in case the outgoing channel isn't opened => it always a problem
/*							else //only in case of H239, the incoming channel must be opened!
							{
								if ( !m_pRmtCapH323->IsH239() && m_pRmtCapH323->IsEPC() && m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapReceive,kRoleContent))
									bIsContentConnected = FALSE;
							}*/
//						}

//						else if (dualVideoMode == eDualModeIpDuoVideo)
//						res = (m_pRmtCapH323->IsCapsSupportDuo())? YES: NO;
						if (bIsContentConnected)
						{	 // initial content rate is equals the current content rate
//							WORD  currentAmscRateType = m_pScm->GetContentModeContentRate();
							DWORD currentContRate     = m_pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);

							TRACESTR(eLevelInfoNormal)<< " CH323ChangeModeCntl::OnPartyEndChangeContent - Content rate changed to "<< currentContRate << ",  Name: " << m_partyConfName;

							BYTE parameterID = 0; // Used only in ISDN
							m_pTaskApi->PartyContentRateChanged(m_pParty,parameterID,currentContRate);

							if (IsOutDirectionConnectedToVideoBridge())
							{
								// In this case we need to inform the video bridge to change its rate towards the ep (Encoder - Video out).
								BYTE bTakeInitial = FALSE;
								if( m_pIpInitialMode->GetConfType() == kCp ||  m_pIpInitialMode->GetConfType() == kCop)
									UpdateVideoOutBridgeH239Case(bTakeInitial);
							}
						}
						else
							PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeContent - new scm arrived + content channels are not connected,  Name: ",m_partyConfName);
					}
					else
						PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeContent - new scm arrived + need to change content again,  Name: ",m_partyConfName);
				}

				m_state = IDLE;
				m_bIsNewScm = FALSE;
				DispatchChangeModeEvent();  // redo the new scm
			}
			else
			{
				ChangeOther(); //with m_state = CHANGECONTENT
			}
		}

	}
	POBJDELETE(pH323Mode);
}

/////////////////////////////////////////////////////////////////////////////
WORD  CH323ChangeModeCntl::HandleCpRate()
{
	WORD status = statOK;
	DWORD vidOutRate = m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapTransmit).GetBitRate();
	DWORD vidInRate = m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapReceive).GetBitRate();
	if(vidInRate > vidOutRate)
	{
		if(vidInRate != m_videoRate)
		{
	    	vidInRate = vidInRate % 8 ? vidInRate / 8 + 1 : vidInRate / 8;
			vidInRate = vidInRate  * 8;
			status = SetH323VideoRate(vidInRate, FALSE, FALSE);
		}
	}
	else //vidInRate <= vidOutRate
	{
		if (vidOutRate != m_videoRate)
		{
			status = SetH323VideoRate(vidOutRate, FALSE, FALSE);
		}
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyEndChangeVideo(CSegment* pParam)
{
	DWORD details = 0;
	BOOL bForceBridgeIncomingUpdate = NO;

	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeVideo : Name - ",m_partyConfName);
	DeleteTimer(CHANGETOUT);
	m_bPartyEndChangeVideoInformParty = FALSE;

	BYTE bWasSecondaryBeforeChangeMode = IsSecondary();
	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam); // this is the mode the party opened

	CComModeH323* pTmpModeH323 = NULL;

	BOOL bNeedToUpdateContentMode = FALSE;

	//Incase there was change mode of both video and content
	//When we start change mode in party level we update only video mode no
	if(!IsInitialContentModeEqualToCurContentMode())
	{
		pTmpModeH323 = new CComModeH323;
		*pTmpModeH323 = *m_pIpInitialMode;
	}

	bForceBridgeIncomingUpdate = !m_pIpCurrentMode->IsMediaContaining(*m_pIpInitialMode, kCapCode|kFormat|kFrameRate|kAnnexes|kH264Profile|kH264Level|kH264Additional|kBitRate,&details,cmCapVideo,cmCapReceive);

	*m_pIpInitialMode = *m_pIpCurrentMode;


	if(pTmpModeH323)
	{
		UpdateInitialWithNewContentMode(pTmpModeH323);
		POBJDELETE (pTmpModeH323);
	}

	BYTE reason = SECONDARY_CAUSE_DEFAULT;
	BYTE direction = cmCapReceive;
	CSecondaryParams secParams;

	DWORD status;
	*pParam >> status >> reason;

	BYTE bSuccess = TRUE;
	details = 0x00000000;

	// The endpoint sent a video stream with violation => change it to secondary.
	//Only in case of stream violation we did serialize for secParams
	if (status == statIncopatibleVideo)
	{
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeVideo : statIncopatibleVideo. Name - ",m_partyConfName);
		bSuccess  = FALSE;
		direction = cmCapReceive;
		secParams.DeSerialize(NATIVE,*pParam);
	}

	// Check the new current vs. ScmSentToParty
	else
	{
		bSuccess = AreAllMustChangesInModeSucceeded(reason, details, direction) || m_bIsNewScm;
		if (!bSuccess && (reason == SECONDARY_CAUSE_DEFAULT) )
			reason = SECONDARY_CAUSE_CHANGE_MODE;
	}

	if (bSuccess)
	{
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndChangeVideo: Success. Name - ",m_partyConfName);

	/*	if (bWasSecondaryBeforeChangeMode) //upgrade from secondary
			SetFlowControlParamsForMove(TRUE);*/

		if (m_pIpInitialMode->GetConfType() == kCp)
		{
			//1)
			// In case of change CP mode, the scm that was selected in function ChangeScmAndCapsInCpConf,
			// isn't have to be the one that the remote selected. In this case, we should update the scm, for connecting to the bridge.
			if (! m_bIsNewScm)
			{
				if (m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive) != m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapReceive) )
					PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::OnPartyEndChangeVideo: Initial in receive is different than current ",m_partyConfName);
			}

			//2)
		    //status = HandleCpRate(); (vngfe-5524):
			status = statOK;
			if(status != statOK)
			{
				PTRACE2(eLevelError, "CH323ChangeModeCntl::OnPartyEndChangeVideo: Failed to change video rate. Set to secondary ",m_partyConfName);
				SetPartyToSecondaryAndStopChangeMode(SECONDARY_CAUSE_VIDEO_PROBLEM,details,direction,&secParams);
				return;
			}
		}//end CP

		BYTE bWaitForBridgeInd = ChangeVideoBridgeState(bForceBridgeIncomingUpdate); // true in case of connect / disconnect / update video bridge

		if (! bWaitForBridgeInd)
		{
			m_state = IDLE;

			if (m_bIsNewScm)
			{
				m_bIsNewScm = FALSE;
				DispatchChangeModeEvent();  // redo the new scm
			}
			else
				ChangeOther();
		}
	}

	else
	{
		if(m_bIsNewScm)
		{
			PTRACE2(eLevelError, "CH323ChangeModeCntl::OnPartyEndChangeVideo: Failed to change video mode. try to recover with new scm ",m_partyConfName);
			m_bIsNewScm = FALSE;
			DispatchChangeModeEvent();  // redo the new scm

		}
		else
		{
			PTRACE2(eLevelError, "CH323ChangeModeCntl::OnPartyEndChangeVideo: Failed to change video mode. Set to secondary ",m_partyConfName);
			SetPartyToSecondaryAndStopChangeMode(reason,details,direction,&secParams);
		}

		m_pIpInitialMode->Dump("CH323ChangeModeCntl::OnPartyEndChangeVideo: secondary, Initial", eLevelInfoNormal);
		m_pIpCurrentMode->Dump("CH323ChangeModeCntl::OnPartyEndChangeVideo: secondary, current", eLevelInfoNormal);
	}
}
/////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsInitialContentModeEqualToCurContentMode()
{
	BYTE bIsEqualToCurMode = TRUE;
	if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation) &&
	        m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))
	{
		const CMediaModeH323& transCurMode = m_pIpCurrentMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
		const CMediaModeH323& tansInitMode  = m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
		bIsEqualToCurMode &= (tansInitMode == transCurMode);
	}
	//(vngef-5524):
	if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation) &&
	    !(m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation)))
	{
	   PTRACE(eLevelError, "CH323ChangeModeCntl::IsInitialContentModeEqualToCurContentMode currentMode with no receive content");
	   bIsEqualToCurMode = FALSE;
	}

	PTRACE2INT(eLevelError, "CH323ChangeModeCntl::IsInitialContentModeEqualToCurContentMode -bIsEqualToCurMode:  ",bIsEqualToCurMode);
	return bIsEqualToCurMode;
}
/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::UpdateInitialWithNewContentMode(CComModeH323* pNewModeH323)
{

	m_pIpInitialMode->Dump("CH323ChangeModeCntl::UpdateInitialWithNewContentMode: , Initial",eLevelInfoNormal);
	m_pIpCurrentMode->Dump("CH323ChangeModeCntl::UpdateInitialWithNewContentMode: , current",eLevelInfoNormal);
	pNewModeH323->Dump("CH323ChangeModeCntl::UpdateInitialWithNewContentMode: , pNewModeH323",eLevelInfoNormal);


	if (pNewModeH323->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))
			m_pIpInitialMode->SetMediaMode(pNewModeH323->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation), cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

	if (pNewModeH323->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation))
	{
	    PTRACE(eLevelInfoNormal, "CH323ChangeModeCntl::UpdateInitialWithNewContentMode m_pIpInitialMode->IsMediaOn-cmCapReceive");
	    if (!m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation))
	        PTRACE(eLevelInfoNormal, "CH323ChangeModeCntl::UpdateInitialWithNewContentMode !m_pIpCurrentMode->IsMediaOn-cmCapReceive");

	    m_pIpInitialMode->SetMediaMode(pNewModeH323->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation), cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	    //m_pIpInitialMode->SetMediaMode(pNewModeH323->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation), cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	}
	//m_pIpInitialMode->Dump("CH323ChangeModeCntl::UpdateInitialWithNewContentMode, Initial after setting:", eLevelError);


}


/////////////////////////////////////////////////////////////////////////////
//Description: Compare the new current vs. the scm which was sent to the party by
//checking that all the "must change mode"s are ok
// the return mode of the function is: if we succeed to change the mode of the party connection to the EP return TRUE, else we failed to change the mode return FALSE.
BYTE CH323ChangeModeCntl::AreAllMustChangesInModeSucceeded(BYTE& reason, DWORD& details, BYTE& direction)
{
	BYTE bRes = TRUE;
	WORD compareNotEqual = 0;
	if (IsSecondary())
	{ //if the party is secondary, we try to reconnect its video channels
		PTRACE2(eLevelError, "CH323ChangeModeCntl::AreAllMustChangesInModeSucceeded: Secondary. ",m_partyConfName);
		if(m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople))
			direction = cmCapReceive;
		else
			direction = cmCapTransmit;

		reason = SECONDARY_CAUSE_RMT_NOT_OPEN_AFTER_CHANGE_MODE;
		bRes = FALSE;
		return bRes;
	}

	/* Case 1:  */
	if (m_pIpInitialMode->GetConfType() == kCp)
	{//After change CP mode, we don't compare to the scm. We only check the current mode
		BYTE tempReason = 0;
		bRes = !(IsMustToChangeModeInCp(tempReason, details));
	/*	if (tempReason == SECONDARY_CAUSE_MOVE_PARTY)
			reason = SECONDARY_CAUSE_MOVE_PARTY; //in the rest of the cases, we want to leave the reason SECONDARY_CAUSE_CHANGE_MODE*/
		direction = cmCapTransmit;
	}
	/* Case 2: Change VSW Mode  */
    else if (m_pIpInitialMode->GetConfType() != kCop)
	{
		bRes = m_pScmSentToParty->IsMediaContaining(*m_pIpCurrentMode, kCapCode, &details, cmCapVideo,cmCapTransmit,kRolePeople);
		direction = cmCapTransmit;
		if (bRes)
		{
			//check that we don't receive more than we can
			bRes = m_pScmSentToParty->IsMediaContaining(*m_pIpCurrentMode, kBitRate|kFormat|kFrameRate|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, &details, cmCapVideo,cmCapReceive,kRolePeople);

			if (!bRes)
				direction = cmCapReceive;
		}

		if (bRes)
		{//If VSW, we also need to check that we don't transmit less than we should
			bRes = m_pIpCurrentMode->IsMediaContaining(*m_pScmSentToParty, kBitRate|kFormat|kFrameRate|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, &details, cmCapVideo,cmCapTransmit,kRolePeople);
			direction = cmCapTransmit;
		}
	}

	return bRes;
}


/////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsMustToChangeModeInCp(BYTE& reason, DWORD& details)
{
	//re-caps from remote to reduce params inside protocol / change protocol:
	const CMediaModeH323& rVideoCurrentScm = m_pIpCurrentMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);
	//kH264Level|kH264Additional_FS|kH264Additional_MBPS
	BYTE bRes = !(m_pRmtCapH323->IsContaining(rVideoCurrentScm, kCapCode | kBitRate | kFormat | kFrameRate | kAnnexes | kH264Profile | kH264Level | kH264Additional_FS | kH264Additional_MBPS | kMaxFR | kH264Mode, cmCapReceiveAndTransmit, kRolePeople));
	TRACEINTO << m_partyConfName << ", Res:" << (WORD)bRes;
	if (bRes) //!bRes
	{
		CapEnum capCode = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive));
		if (capCode == eH264CapCode) //check if capCode is 264 otherwise do nothing
		{
			DWORD newBitRate = m_pRmtCapH323->GetMaxVideoBitRate(capCode, cmCapReceive, kRolePeople);
			DWORD oldBitRate = m_pLocalCapH323->GetMaxVideoBitRate(capCode, cmCapReceive, kRolePeople);
			DWORD audioRate = m_pIpInitialMode->GetMediaBitRate(cmCapAudio) * 1000;

			TRACEINTO
				<< "NewBitRate:" << (WORD)newBitRate
				<< ", OldBitRate:" << oldBitRate
				<< ", AudioRate:" << (WORD)audioRate
				<< ", ConfBitRate:" << (DWORD)((oldBitRate * 100) + audioRate);

			DWORD threshold = 5;
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("VIDEO_RESOLUTION_DECREASE_THRESHOLD_PERCENT", threshold);

			if (((oldBitRate * 100) + audioRate) > ((newBitRate * 100) + (newBitRate * threshold)))
			{
				DWORD partyRate = (newBitRate * 100) + audioRate;
				eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
				H264VideoModeDetails h264VidModeDetails;
				BYTE isHighprofie = (m_pIpCurrentMode->IsH264HighProfile(cmCapReceive) || m_pIpCurrentMode->IsH264HighProfile(cmCapTransmit));

				Eh264VideoModeType resourceMaxVideoMode = TranslateCPVideoPartyTypeToMaxH264VideoModeType(m_eLastAllocatedVideoPartyType);

				if (resourceMaxVideoMode == eHD720Asymmetric || resourceMaxVideoMode == eHD720At60Asymmetric || resourceMaxVideoMode == eHD1080Asymmetric || resourceMaxVideoMode == eHD1080At60Asymmetric)
					::GetH264AssymetricVideoParams(h264VidModeDetails, partyRate, vidQuality, resourceMaxVideoMode, isHighprofie);
				else
					::GetH264VideoParams(h264VidModeDetails, partyRate, vidQuality, resourceMaxVideoMode, isHighprofie);

				long newFs = h264VidModeDetails.maxFS;
				if (newFs == (long)INVALID)
				{
					CH264Details thisH264Details = h264VidModeDetails.levelValue;
					newFs = thisH264Details.GetDefaultFsAsDevision();
				}
				long newMBPS = h264VidModeDetails.maxMBPS;

				if (newMBPS == (long)INVALID)
				{
					CH264Details thisH264Details = h264VidModeDetails.levelValue;
					newMBPS = thisH264Details.GetDefaultMbpsAsDevision();
				}
				long currentFs = 0;
				long currentFs1 = 0;
				long currentFs2 = 0;
				long currentMBPS = 0;
				APIU16 currentProfile = 0;
				long currentDPB = 0;
				long currentBRandCPB = 0;
				long currentSAR = 0;
				long currentStaticMB = 0;
				APIU8 currentLevel1 = 0;
				APIU8 currentLevel2 = 0;

				m_pIpCurrentMode->GetH264Scm(currentProfile, currentLevel1, currentMBPS, currentFs1, currentDPB, currentBRandCPB, currentSAR, currentStaticMB, cmCapReceive);
				m_pIpCurrentMode->GetH264Scm(currentProfile, currentLevel2, currentMBPS, currentFs2, currentDPB, currentBRandCPB, currentSAR, currentStaticMB, cmCapTransmit);
				if (currentFs1 == (long)INVALID)
				{
					CH264Details thisH264Details = currentLevel1;
					currentFs1 = thisH264Details.GetDefaultFsAsDevision();
				}
				if (currentFs2 == (long)INVALID)
				{
					CH264Details thisH264Details = currentLevel2;
					currentFs2 = thisH264Details.GetDefaultFsAsDevision();
				}
				currentFs = max(currentFs1, currentFs2);
				if (!bRes && (newFs < currentFs))
				{
					TRACESTR(eLevelInfoNormal) << " CH323ChangeModeCntl::IsMustToChangeModeInCp - !bRes - newFs = " << (WORD)newFs << ",  currentFs: " << (WORD)currentFs;
					bRes = TRUE;
					m_pIpInitialMode->SetH264Scm(currentProfile, h264VidModeDetails.levelValue, newMBPS, newFs, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, h264VidModeDetails.maxStaticMbps, cmCapReceive);
					m_pIpInitialMode->SetH264Scm(currentProfile, h264VidModeDetails.levelValue, newMBPS, newFs, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, h264VidModeDetails.maxStaticMbps, cmCapTransmit);
					reason = LOW_RESOLUTION_REQUIRED;
				}
				if (bRes && newFs != currentFs)
				{
					TRACESTR(eLevelError) << " CH323ChangeModeCntl::IsMustToChangeModeInCp - bRes - newFs = " << (WORD)newFs << ",  currentFs: " << (WORD)currentFs;
					bRes = TRUE;
					m_pIpInitialMode->SetH264Scm(currentProfile, h264VidModeDetails.levelValue, newMBPS, newFs, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, h264VidModeDetails.maxStaticMbps, cmCapTransmit);
				}
			}
		}
	}

	return bRes;
}

/////////////////////////////////////////////////////////////////////////////
/*There are cases we move party to secondary because it has lower line rate but the conf level doesn't regard to the
//rate in the highest common's calculation, so we remove this protocol. So the actual reason this party moved to secondary is it's lower line rate
//and not what we have in the details. If this is the case I need to update the details to the actual reason*/
void  CH323ChangeModeCntl::CheckDetails(DWORD* pDetails, BYTE* direction) const
{
	if (m_pIpInitialMode->GetConfType() == kVideoSwitch) //VSW
	{
		if (m_pIpInitialMode->IsAutoVideoResolution())//VSW auto
		{
			CapEnum protocol = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit));
			DWORD scmRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);
			BYTE  bRateOk = m_pRmtCapH323->AreCapsSupportVideoProtocolAndRate(protocol, scmRate);
			if (!bRateOk)
			{
				if(*pDetails != HIGHER_BIT_RATE)
				{
					PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::CheckDetails - change the reason to HIGHER_BIT_RATE : Name - ",m_partyConfName);
					*pDetails = 0;
					*pDetails |= HIGHER_BIT_RATE;
					*direction = cmCapReceive;//we want to show the low bit rate of the remote
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyMoveToSecondary(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyMoveToSecondary : Name - ",m_partyConfName);
	WORD reason = SECONDARY_CAUSE_DEFAULT;
	CSecondaryParams secParams;
	*pParam >> reason;
	secParams.DeSerialize(NATIVE,*pParam);
	if (reason == SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH || reason == SECONDARY_CAUSE_AVF_INSUFFICIENT_BANDWIDTH)
	{
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyMoveToSecondary : Secondary limitation ",m_partyConfName);
		m_bSecondaryLimitation = TRUE;
	}

	SetPartyToSecondaryAndStopChangeMode(reason,0,cmCapTransmit,&secParams,FALSE); //without disconnect the channels, since they are already disconnected
}

//////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyMoveToSecondaryChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyMoveToSecondaryChangeMode: Name - ",m_partyConfName);

	if (IsValidTimer(CHANGETOUT))
	{/*In case that the conf sends new scm to the party, in the same time that the party
	 becames secondary as a result from getting violation from the card (and therefore
	 becames secondary), we have a conflict between this 2 processes. Therefore, we stay
	 here in state of CHANGEVIDEO*/
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyMoveToSecondaryChangeMode : Start again the change mode timer,  Name - ",m_partyConfName);
        m_pIpCurrentMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit);
		DeleteTimer(CHANGETOUT);
		StartTimer(CHANGETOUT,PARTYCNTL_CHANGEVIDEO_TIME); //give the party an opportunity to change the secondary to connected
	}

	else //that's mean that the party isn't in change mode process, but in bridge connection process.
		OnPartyMoveToSecondary(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::SetPartyToSecondaryAndStopChangeMode(BYTE reason,DWORD details,BYTE direction,CSecondaryParams *pSecParams,BYTE bDisconnectChannels)
{
	m_bBridgeChangeToAsymmetricProtocol = FALSE;
	CH323PartyCntl::SetPartyToSecondaryAndStopChangeMode(reason,details,direction,pSecParams,bDisconnectChannels);
}
//////////////////////////////////////////////////////////////////////////////
// no ack from party task
void  CH323ChangeModeCntl::OnTimerPartyReAlloc(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnTimerPartyReAlloc : Name - ",m_partyConfName);
    switch (m_OldState)
    {
        case CHANGECONTENT:
            OnTimerPartyContent(pParam);
            break;
        case CHANGEVIDEO:
            OnTimerPartyVideo(pParam);
            break;
        case CHANGEOTHER:
            OnTimerPartyOther(pParam);
            break;
        default:
            DBGPASSERT(m_OldState);
            break;
    }

}

//////////////////////////////////////////////////////////////////////////////
// no ack from party task
void  CH323ChangeModeCntl::OnTimerPartyContent(CSegment* pParam)
{
	DBGPASSERT(1);
    PTRACE2(eLevelError,
            "CH323ChangeModeCntl::OnTimerPartyContent: Failed to change mode content. Continue change other ",m_partyConfName);
    m_state = CHANGEOTHER;
    ChangeOther();
  // don't try to continue with change mode process because something must have happened to the party
  // if it didn't even send "end change mode" with "failure" state.
}

/////////////////////////////////////////////////////////////////////////////
// no ack from party task
void  CH323ChangeModeCntl::OnTimerPartyVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnTimerPartyVideo : Name - ",m_partyConfName);
	DBGPASSERT(GetPartyRsrcId());
	CSegment* pSeg = new CSegment;
	m_pIpCurrentMode->Serialize(NATIVE,*pSeg);

	WORD reason = SECONDARY_CAUSE_DEFAULT;
	DWORD status = statIllegal;

	*pSeg << status << reason;
	DispatchEvent(PARTYENDCHANGEMODE,pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
// no ack from party task
void  CH323ChangeModeCntl::OnTimerPartyOther(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnTimerPartyOther : Name - ",m_partyConfName);
	DBGPASSERT(GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////
// derives the specific mode change requests
void  CH323ChangeModeCntl::ChangeOther()
{
	// There is no Other changes in H323 (except from content). NOW!!!
	TRACEINTO << m_partyConfName << ", State:" << m_state;
	// enter only if we are in the beginning of the change mode process or if we are in "change content"
	// and we need to chcek if it was changed ok
	if (m_state == IDLE)
	{
		// if the call has content
		if (m_pContentBridge && m_pContentBridge->IsConnected())
		{
			ChangeContent();
			if (m_state != IDLE)
				return; //the content is now been changed as a result of the function ChangeContent
		}
	}

	if (m_state == REALLOCATE_RSC)  // BRIDGE-13402
	{
		   PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeOther - not doing anything while resources are reallocated");
	}
	else
	{
		if (m_state == CHANGECONTENT)
		{
			if (IsAtLeastOneDirectionConnectedToVideoBridge())
			{
				if (m_pContentBridge && m_pContentBridge->IsConnected())
				{
					ChangeContent();
					return;
				}
				else
				   PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeOther not connected to CB: Name - ", (DWORD)m_pContentBridge);
			}
			else
			   PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeOther not connect to v-b: Name - ",m_partyConfName);
		}

		ConnectPartyToFECCBridge(m_pIpCurrentMode);

	//	OnPartyConnectDataBrdg();

		// Initial and Current scm must be equal for change mode to be finished!!!!
		if (!m_bIsNewScm)
		{
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeOther saving current content mode: Name - ",m_partyConfName);
			CComModeH323* pTmpModeH323 = NULL;


				//Incase there was change mode of both video and content
				//When we start change mode in party level we update only video mode no
			if(!IsInitialContentModeEqualToCurContentMode())
			{
				pTmpModeH323 = new CComModeH323;
				*pTmpModeH323 = *m_pIpInitialMode;
			}
			*m_pIpInitialMode = *m_pIpCurrentMode;
			if(pTmpModeH323)
			{
				UpdateInitialWithNewContentMode(pTmpModeH323);
				POBJDELETE (pTmpModeH323);
			}
		}

		if (m_state == CHANGE_ALL_MEDIA)
		{
			// bug fix #19034 : when attend party , and party is in change mode apdate DB to CONNECTED state on source conf,
			// will remove the party from attended queue , after the move will continue and the party will be stack at operator conf (Ron)
	//		if(! (IsConfWaitingToEndChangeModeForMove() && GetMoveWaitingToEndChangeModeAttendType()==MOVE_ATTEND) ){
				m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED);
				RemoveSecondaryCause(TRUE);// check if to keep content secondary cause.

				// In GL1 we update the CDR only regarding connect/disconnect state
				// Meaning that if we updated the CDR regarding the connection of the party -
				// There is no need to update again.
				UpdatePartyStateInCdr();

	//		else
	//			PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeOther : not updating party state to connected because move wait to end change mode");
	//		UpdateVideoStreamIfNeeded();
		}

		m_state = IDLE;
		EndChangeMode();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::ChangeContent()
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeContent - ",m_partyConfName);

	// Need check secondary condition for content
	// if equal TRUE => if the content open, set it to secondary. If the content close do nothing (keep it in secondary mode)
	// if equal FALSE => if the content open, check for change mode because of rate or protocol.
	// If the content close try to upgrade it to connect.
	DWORD details = 0;
	BYTE bIsSecondaryCondition	= FALSE;
//	BYTE bNeedToChangeRate		= FALSE;
//	BYTE bNeedToChangeProtocol	= FALSE;
//	BYTE bNeedToChangeProtocolOut	= FALSE;
//	BYTE bNeedToChangeProtocolIn	= FALSE;
//	BYTE bNeedToChangeHDResOut = FALSE;
//	BYTE bNeedToChangeHDResIn  = FALSE;
//	BYTE bNeedToChangeHDRes	= FALSE;
//	BYTE bNeedToChangeContent	= FALSE;

	bIsSecondaryCondition	= IsSecondaryConditionForContent(details);
//	bNeedToChangeRate		= IsContentRateNeedToBeChanged();
//	bNeedToChangeProtocolOut	= IsContentOutProtocolNeedToBeChanged(bIsSecondaryCondition);
//	bNeedToChangeProtocolIn	        = IsContentInProtocolNeedToBeChanged(bIsSecondaryCondition);
//	bNeedToChangeProtocol   = bNeedToChangeProtocolOut || bNeedToChangeProtocolIn;
//	bNeedToChangeHDResOut  = IsContentOutHDResOrMpiNeedToBeChanged(bIsSecondaryCondition);
//	bNeedToChangeHDResIn   = IsContentInHDResOrMpiNeedToBeChanged(bIsSecondaryCondition);
//	bNeedToChangeHDRes     = bNeedToChangeHDResOut || bNeedToChangeHDResIn;
//	bNeedToChangeContent	= bNeedToChangeRate || bNeedToChangeProtocol || bNeedToChangeHDRes;
	BYTE bIsNeedToChangeVideoLegacy = FALSE;

	if(bIsSecondaryCondition)
	{//check content open and set it to secondary.
//		SystemSleep(5 * SECOND);
//    	m_pRmtCapH323->Dump("CH323ChangeModeCntl::ChangeContent: secondary, remote caps:", eLevelInfoNormal);
//    	m_pIpInitialMode->Dump("CH323ChangeModeCntl::ChangeContent: secondary, Initial", eLevelInfoNormal);
		SetPartyToContentSecondaryMode(details);

		m_pIpCurrentMode->Dump("CH323ChangeModeCntl::ChangeContent: secondary, m_pIpCurrentMode", eLevelInfoNormal);
		m_pIpInitialMode->Dump("CH323ChangeModeCntl::ChangeContent: secondary, m_pIpInitialMode", eLevelInfoNormal);


		//Incase of fall back from Content to Legacy
		CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();


		BYTE Multy = pCommConf->GetContentMultiResolutionEnabled();
		PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeContent : Multy - ",Multy);
		PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeContent : m_ConnectedToVideoAsLegacy - ",m_ConnectedToVideoAsLegacy);

		//if(pCommConf->GetContentMultiResolutionEnabled() )// To check with romem
		if(!m_ConnectedToVideoAsLegacy)
		{
			//If this is also the token holder we need to send withdraw
			BYTE bIsTokenHolder = m_pContentBridge->IsTokenHolder(m_pParty);
			if(bIsTokenHolder)
				m_pTaskApi->ContentTokenWithdraw();

			PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeContent - fall back to Legacy.");

			//DisconnectPartyFromContentBridge();
			bIsNeedToChangeVideoLegacy = DisconnectForUpdateLegacyStatusIfNeeded();

		}
	}
	else //if (bIsSecondaryCondition == FALSE)
	{
		if(m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
		{
			if (m_bNoContentChannel)
			{//if content is reopened, we need to check the secondary condition again, because it might be not valid now.
				if(bIsSecondaryCondition == FALSE)//TODO-URI
				{
					PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeContent : Upgrade content from secondary. Name - ",m_partyConfName);
					OFF(m_bNoContentChannel);
				}
			}
		}
	}


	if(!bIsNeedToChangeVideoLegacy)
	{
		ContinueToChangeContent();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::ContinueToChangeContent()
{
	DWORD details = 0;
	BYTE bIsSecondaryCondition	= FALSE;
		BYTE bNeedToChangeRate		= FALSE;
		BYTE bNeedToChangeProtocol	= FALSE;
		BYTE bNeedToChangeProtocolOut	= FALSE;
		BYTE bNeedToChangeProtocolIn	= FALSE;
		BYTE bNeedToChangeHDResOut = FALSE;
		BYTE bNeedToChangeHDResIn  = FALSE;
		BYTE bNeedToChangeHDRes	= FALSE;
		BYTE bNeedToChangeContent	= FALSE;
		//HP content:
		BYTE bNeedToChangeH264Profile		= FALSE;
		BYTE bNeedToChangeH264ProfileOut	= FALSE;
		BYTE bNeedToChangeH264ProfileIn	= FALSE;

		bIsSecondaryCondition	= IsSecondaryConditionForContent(details);
		bNeedToChangeRate		= IsContentRateNeedToBeChanged();
		bNeedToChangeProtocolOut	= IsContentOutProtocolNeedToBeChanged(bIsSecondaryCondition);
		bNeedToChangeProtocolIn	        = IsContentInProtocolNeedToBeChanged(bIsSecondaryCondition);
		bNeedToChangeProtocol   = bNeedToChangeProtocolOut || bNeedToChangeProtocolIn;
		bNeedToChangeHDResOut  = IsContentOutHDResOrMpiNeedToBeChanged(bIsSecondaryCondition);
		bNeedToChangeHDResIn   = IsContentInHDResOrMpiNeedToBeChanged(bIsSecondaryCondition);
		bNeedToChangeHDRes     = bNeedToChangeHDResOut || bNeedToChangeHDResIn;
		//HP content:
		bNeedToChangeH264ProfileOut	= IsContentOutProfileNeedToBeChanged(bIsSecondaryCondition);
		bNeedToChangeH264ProfileIn	= IsContentInProfileNeedToBeChanged(bIsSecondaryCondition);
		bNeedToChangeH264Profile   	= bNeedToChangeH264ProfileOut || bNeedToChangeH264ProfileIn;

		bNeedToChangeContent	= bNeedToChangeRate || bNeedToChangeProtocol || bNeedToChangeHDRes || bNeedToChangeH264Profile;


	PTRACE2INT(eLevelInfoNormal,"***CH323ChangeModeCntl::ContinueToChangeContent :  Is Content change process - ",bNeedToChangeContent);
	BYTE bIsContentConnected  = TRUE;

	if (bNeedToChangeContent)
		m_conferenceContentRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation); //store the last content rate in the class member, for further comparisons
	DWORD initialContRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	DWORD currentContRate = m_pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

	if ( !m_bNoContentChannel) //party can have content!
	{//check if the content channels are not opened
		if (m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))
			bIsContentConnected = FALSE;//in case the outgoing channel isn't opened => it always a problem
	}

	eChangeModeState changeModeState = eNotNeeded;
	if(bNeedToChangeProtocol)
	{
	    if(bNeedToChangeProtocolOut)
		changeModeState = eChangeContentOut;
	else
	        changeModeState = eChangeContentIn;
	}
	else if(bNeedToChangeHDRes)
	{
		if(bNeedToChangeHDResOut)
				changeModeState = eChangeContentOut;
			else
			     changeModeState = eChangeContentIn;
	}
	//HP content:
	else if(bNeedToChangeH264Profile)
	{
		if(bNeedToChangeH264ProfileOut)
				changeModeState = eChangeContentOut;
			else
			     changeModeState = eChangeContentIn;
	}
	else
		changeModeState = eChangeContentRate;



	if (m_bNoContentChannel || ! bIsContentConnected)
	{
		if (bNeedToChangeContent)
		{
			m_state = CHANGECONTENT;
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ContinueToChangeContent: Party has NO content: ", m_partyConfName);
			DWORD numOfChangeModes = GetNumOfChangeModesInSec();
			TICKS curTicks = SystemGetTickCount();
			TICKS diff;
			diff = curTicks - m_TimeOfChangeModeInTicks;
			//PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeContent: Party has content:noa dbg numOfChangeModes  ", numOfChangeModes);
			if(numOfChangeModes == 0 || diff.GetIntegerPartForTrace() > 100)
			{
				PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::ContinueToChangeContent: Party has no content: numOfChangeModes starting timer  ", numOfChangeModes);
				//StartTimer(CHANGE_MODE_LOOP,1 * SECOND);
				m_TimeOfChangeModeInTicks = curTicks;
				SetNumOfChangeModesInSec(0);
			}
			else if (numOfChangeModes > NUM_OF_CHANGE_MODES_PER_SECOND)
			{
				PTRACE2INT(eLevelError,"CH323ChangeModeCntl::ContinueToChangeContent -no content loop of change modes numOfChangeModes, ",numOfChangeModes);
				SetNumOfChangeModesInSec( (numOfChangeModes + 1) );
				DBGPASSERT(7);
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
			m_pPartyApi->ChangeContentMode(m_pIpInitialMode, (WORD)changeModeState, 0);
			StartTimer(CHANGETOUT,PARTYCNTL_CHANGERATE_TIME);
			return;
		}
		else
		{
			//VNGFE-8958
			TRACEINTO <<"state: " << (int)m_state <<". Content closed and no need to change content. set state to IDLE. " << "Name: " << m_partyConfName;
			m_state = IDLE;
			return;
		}
	}
	else
	{
		// Check if there is a need to change the content rate or protocol
		if (bNeedToChangeContent)
		{
			m_state = CHANGECONTENT;
			BYTE bIsSpeaker = m_pContentBridge->IsTokenHolder(m_pParty);
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ContinueToChangeContent: Party has content: ", m_partyConfName);
			DWORD numOfChangeModes = GetNumOfChangeModesInSec();
			TICKS curTicks = SystemGetTickCount();
			TICKS diff;
            diff = curTicks - m_TimeOfChangeModeInTicks;
		//	PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeContent: Party has content:noa dbg numOfChangeModes  ", numOfChangeModes);
			if(numOfChangeModes == 0 || diff.GetIntegerPartForTrace() > 100)
			{
				PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::ContinueToChangeContent: Party has content:noa dbg numOfChangeModes starting timer  ", numOfChangeModes);
				//StartTimer(CHANGE_MODE_LOOP,1 * SECOND);
				m_TimeOfChangeModeInTicks = curTicks;
				SetNumOfChangeModesInSec(0);
			}
			else if (numOfChangeModes > NUM_OF_CHANGE_MODES_PER_SECOND)
			{
				PTRACE2INT(eLevelError,"CH323ChangeModeCntl::ContinueToChangeContent - loop of change modes numOfChangeModes, ",numOfChangeModes);
				SetNumOfChangeModesInSec( (numOfChangeModes + 1) );
				DBGPASSERT(5);
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
			m_pPartyApi->ChangeContentMode(m_pIpInitialMode, (WORD)changeModeState, bIsSpeaker);
			StartTimer(CHANGETOUT,PARTYCNTL_CHANGERATE_TIME);
			return;
		}
		else // initial content rate and protocol is equals the current content rate/protocol
		{
			DWORD currentRateType = m_pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);;
			TRACESTR(eLevelInfoNormal)<< " CH323ChangeModeCntl::ContinueToChangeContent :  currentRateType - " << currentRateType << ",  Name: " << m_partyConfName;
			// connect the content bridge or inform a new rate (if changed)
			if (m_isContentConn == FALSE)
			{
				m_state = IDLE; //if we will connect content bridge then the state will be changed to CHANGECONTENT
				ConnectToContentBridgeIfPossible();

				return;
			}
			else if (m_state == CHANGECONTENT)
			{
				// the content rate was changed or current contained initial and party sent re-cap
			    TRACESTR(eLevelInfoNormal)<< " CH323ChangeModeCntl::ContinueToChangeContent - Current contained initial or Content rate changed to " << currentContRate << ",  Name: " << m_partyConfName;

				if(m_pContentBridge->IsPartyWaitForRateChange(m_pParty))
				{
					BYTE parameterID = 0; // Used only in ISDN calls
					m_pTaskApi->PartyContentRateChanged(m_pParty,parameterID,currentRateType);
					if (IsOutDirectionConnectedToVideoBridge())
					{
						// In this case we need to inform the video bridge to change its rate towards the ep (Encoder - Video out).
						BYTE bTakeInitial = FALSE;
						if( m_pIpInitialMode->GetConfType() == kCp ||  m_pIpInitialMode->GetConfType() == kCop)
							UpdateVideoOutBridgeH239Case(bTakeInitial);
					}
				}
				else
				{
					if( m_pIpInitialMode->GetConfType() == kCp ||  m_pIpInitialMode->GetConfType() == kCop)
						UpdateVideoOutBridgeH239Case(FALSE);
				}
				m_pPartyApi->Rmt323CommModeUpdateDB(m_pIpCurrentMode);
				m_state = IDLE; // change content has been done!!
			}
			else
			{
			    TRACESTR(eLevelInfoNormal)<< " CH323ChangeModeCntl::ContinueToChangeContent: m_state = " << m_state << ",  Name: " << m_partyConfName;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// exit point from state machine
void  CH323ChangeModeCntl::EndChangeMode()
{
	CConfParty* pConfParty = GetConfParty();
	PASSERT_AND_RETURN(!pConfParty || !m_pPartyApi);

	CMedString str;
	str << " Name - " << m_partyConfName << ", state - " << m_state;
	PTRACE2(eLevelInfoNormal,"***CH323ChangeModeCntl::EndChangeMode:",str.GetString());

	//this part is for CDR purppose
	UpdateNewRateForCdrIfNeeded();

	CComModeH323* pTmpModeH323 = NULL;
	if(m_pIpInitialMode)
	{
		pTmpModeH323 = new CComModeH323;
		*pTmpModeH323  = *m_pIpInitialMode;
	}
    if(!m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation) && m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation) )
    {
        PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::EndChangeMode - no Rx content. Name - ",m_partyConfName);
        pTmpModeH323->SetMediaOff(cmCapVideo,cmCapReceive,kRoleContentOrPresentation);
    }
	//
	if (*m_pIpCurrentMode != *pTmpModeH323)   // change mode requests received while in process.
	{
        POBJDELETE (pTmpModeH323);
		m_bIsNewScm = FALSE;
		m_state = IDLE;// first change m_state and then dispatch the change mode event

		//if the conf is waiting for the change mode to ended, in order to start move,
		//there is no point to start another change mode process
		if (IsConfWaitingToEndChangeModeForMove() || m_bConfWaitToEndChangeModeForFecc)
		{
			m_changeModeInitiator = eNoInitiator; //init for next time
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::EndChangeMode - m_confWaitToEndChangeModeForMove is true => ending change mode. Name - ",m_partyConfName);
			m_bConfWaitToEndChangeModeForFecc = FALSE;
//			m_pTaskApi->EndChangeModeParty(m_pParty);
		}
		else
			DispatchChangeModeEvent();  // redo the new scm
	}

	// upgrade to mix_mode only if the VB is connected, or it's AudioOnly
	else if (m_deferUpgrade && ( m_eVidBridgeConnState != eBridgeDisconnected  ||
			(m_pIpInitialMode->IsMediaOff(cmCapVideo,cmCapReceive,kRolePeople) &&
			m_pIpInitialMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))))
	{
		m_deferUpgrade = false;
		m_bPartyInUpgradeProcess = true;
		TRACEINTO << "mix_mode: generating deferred upgrade to mixed mode";
		CSegment* pSeg = new CSegment;
		DWORD confID = -1; // EY_20866
		DWORD confState=0;
		*pSeg << (DWORD)confState << confID << GetPartyRsrcId();
		DispatchEvent(SET_PARTY_AVC_SVC_MEDIA_STATE, pSeg);
		POBJDELETE(pSeg);
	}
	else
	{
		POBJDELETE (pTmpModeH323);
		m_changeModeInitiator = eNoInitiator;//init for next time
		if(m_deferUpgrade && m_eVidBridgeConnState == eBridgeDisconnected )
			TRACEINTO << "didn't start upgrade to mix as party is not connected to vb!";

		BYTE bWaitForVideoBridgeInd = ChangeVideoBridgeState();

		//disconnect content bridge only after the video bridge is connected/disconnected:
		BYTE bWaitForContentBridgeInd = FALSE;
/*		if (bWaitForVideoBridgeInd == FALSE)
		{
			if ((m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapReceive)) &&
				(m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit)))
			{
				if(m_pContentCntl && m_pContentCntl->IsConnected() && m_isContentConn)
				{
					PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::EndChangeMode - Disconnect content bridge. Name - ",m_partyConfName);
					OFF(m_isContentConn);
					m_pContentCntl->DisConnect(m_pParty);
					bWaitForContentBridgeInd = TRUE;
				}
				ON(m_bNoContentChannel);
			}
		}*/


		if (!bWaitForVideoBridgeInd && !bWaitForContentBridgeInd)
		{
			m_state = IDLE;
			if (IsConfWaitingToEndChangeModeForMove() || m_bConfWaitToEndChangeModeForFecc)
			{
				m_bConfWaitToEndChangeModeForFecc = FALSE;
//				m_pTaskApi->EndChangeModeParty(m_pParty);
			}
		}
	}
	//for CDR event: PARTICIPANT_DISCONNECT_INFORMATION
	UpdateDetailsForParticipantDisconnectInfoCDREvent(m_pIpCurrentMode);
	//PTRACE2INT(eLevelInfoNormal, "CH323ChangeModeCntl::EndChangeMode - m_maxConnectionRateCurrently: ",m_maxConnectionRateCurrently );
	//PTRACE2INT(eLevelInfoNormal,"CH323AddPartyCntl::EndConnectionProcess: m_maxFormatCurrently - ",m_maxFormatCurrently);
	//PTRACE2INT(eLevelInfoNormal, "CH323ChangeModeCntl::EndChangeMode - m_maxFrameRateCurrently: ",m_maxFrameRateCurrently );

	//=============================================================================================================================
	// This may be the end of token handling on which a recap got pended, or an end of a recap on which token handling got pended
	//=============================================================================================================================
	eTokenRecapCollisionDetectionType	eTokenRecapCollisionDetection	= pConfParty -> GetTokenRecapCollisionDetection();
	BOOL 								bTokenRecapPended				= pConfParty -> IsTokenRecapPendedDueToCollisionDetection();
	std::ostringstream strTRCD;
	strTRCD << "Check token/recap handling status: ";
	if (eTokenRecapCollisionDetection != etrcdAvailable)
	{
		strTRCD << "token/recap handling was in progress, ";
		if (bTokenRecapPended)
		{
			//===============================
			// Reinvoking the pended action
			//===============================
			strTRCD << "there is pended action.";
			TRACEINTO << "there is pended action - Reinvoking it.";
			m_pPartyApi -> TokenRecapCollisionEnded();
		}
		else
		{
			//=============================================================================
			// No pends happened, still need to reset the flags as this change-mode ended
			//=============================================================================
			strTRCD << "nothing is pending.";
			pConfParty -> SetTokenRecapCollisionDetection(etrcdAvailable);
		}
	}
	else if (bTokenRecapPended)
	{
		//====================================================
		// Not expecting a pend while nothing is in progress
		//====================================================
		pConfParty -> UnpendTokenRecapDueToCollisionDetection();
		strTRCD << "a trcd pend got noted, with nothing in progress";
	}
	TRACEINTO << strTRCD.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
//returned value: wait for bridge indication (yes/no)
BYTE  CH323ChangeModeCntl::ChangeVideoBridgeState(BOOL bForceBridgeIncomingUpdate)
{
	//(1) If the party isn't connected to the video bridge but has video:
	if (IsNeedToConnectToVideoBridge(m_pIpCurrentMode))
	{  // party connected with video
		if (m_isFaulty)
		{
			PTRACE2(eLevelError,"CH323ChangeModeCntl::ChangeVideoBridgeState - Can't connect to video bridge, because of faulty flag - ",m_partyConfName);
			return FALSE;
		}
		else
		{
			PTRACE2(eLevelInfoNormal," CH323ChangeModeCntl::ChangeVideoBridgeState - Establishing Video Connection... ",m_partyConfName);
			//at this point, m_pIpCurrentMode should be equal to m_pIpInitialMode, so we can connect to the bridege
			ConnectPartyToVideoBridge(m_pIpCurrentMode);
			m_state = CHANGEVIDEO;
			return TRUE;
		}
	}

	//(2) If the party is connected to the video bridge
	if ((m_eVidBridgeConnState & eSendOpenIn) || (m_eVidBridgeConnState & eSendOpenOut))
	{
		//Set the party to secondary as a result of a new scm, after it has already been connected
		if (m_state == IDLE)
		{
		    BYTE bIsDisconnectFromVideoBridge = DisconnectPartyFromVideoBridgeIfNeeded(m_pIpCurrentMode);
		    if (bIsDisconnectFromVideoBridge)
		    {
		    	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ChangeVideoBridgeState - disconnect from the video bridge: Name - ",m_partyConfName);
				return TRUE;
		    }
		}

		//(3) update bridge
		else
		{
			BYTE bTakeInitial = FALSE;
			m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial, bForceBridgeIncomingUpdate);
			if (m_eUpdateState != eNoUpdate)
				return TRUE; //need to wait
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::HandlePendingScmIfNeeded()
{
	if(!m_pendingScmForUpgrade)
		return;

	//since scm is not updated with video streams and operation points, update initial mode with pending scm content only

	m_pIpInitialMode->SetMediaMode(m_pendingScmForUpgrade->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation), cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	m_pIpInitialMode->SetMediaMode(m_pendingScmForUpgrade->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation), cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

	POBJDELETE(m_pendingScmForUpgrade);
	ChangeScm(m_pIpInitialMode, eCanChangeVideoAndContent);
}
/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::ChangeScm(CIpComMode* pH323Scm, EChangeMediaType eChangeMediaType, BYTE bOnlyUdateBridge)
{
	#undef TRACE_LOG
	#define TRACE_LOG "PartyId:" << GetPartyRsrcId() << ", MediaType:" << eChangeMediaType << ", IsUpdateBridgeOnly:" << (int)bOnlyUdateBridge

	if (bOnlyUdateBridge == FORCE_UPDATE_IN)
	{
		TRACEINTO << TRACE_LOG << " - Only update the in-bridge";
		UpdateVideoInBridgeIfNeeded(TRUE, TRUE);
		return;

	}
	else if (bOnlyUdateBridge == FORCE_UPDATE_OUT)
	{
		TRACEINTO << TRACE_LOG << " - Only update the out-bridge";
		UpdateVideoOutBridgeIfNeeded(TRUE, TRUE);
		return;
	}

	TRACEINTO << TRACE_LOG;

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetMonitorPartyId());
	PASSERT(!pConfParty);

	const CVidModeH323 lastTransVideo   = (const CVidModeH323&)m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapTransmit);
	const CVidModeH323 lastTransContent = (const CVidModeH323&)m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	const CVidModeH323 lastRecvVideo    = (const CVidModeH323&)m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapReceive);
	BYTE bCopBestModeChangedInRmtCaps   = FALSE;

	if (m_pIpInitialMode->GetConfType() == kCop)
	{
		if (!m_bCascadeIsLecturer)
		{
			if (m_lastCopForceEncoderLevel == INVALID_COP_LEVEL)
			{
				//Not forced to specific level by the conference
				CComModeH323* pScm = new CComModeH323(*m_pIpInitialMode);
				pScm->SetCopTxLevel(INVALID_COP_LEVEL);
				DWORD definedMaxRate = pConfParty ? pConfParty->GetVideoRate() : 0;
				DWORD setupRate = GetSetupRate() / 1000;

				if (setupRate != 0 && CASCADE_NONE == m_bIsCascade)
				{
					DWORD inAudioRate = m_pIpCurrentMode->GetMediaBitRate(cmCapAudio, cmCapReceive, kRolePeople);
					DWORD audioRateAccordingToSetupRate = (CalculateAudioRate(setupRate * 1000)) / 1000;
					DWORD remoteSetupRateonlyforvideo = setupRate - audioRateAccordingToSetupRate;

					if (definedMaxRate != 0xFFFFFFFF && inAudioRate != 0 && remoteSetupRateonlyforvideo != 0)
						definedMaxRate = min(definedMaxRate, (DWORD)remoteSetupRateonlyforvideo);
					else if (inAudioRate != 0)
						definedMaxRate = remoteSetupRateonlyforvideo;
				}
				m_pRmtCapH323->FindBestVidTxModeForCop(m_pCopVideoTxModes, pScm, (pConfParty ? pConfParty->GetVideoProtocol() : 0), definedMaxRate);
				if (pScm->GetCopTxLevel() != m_pIpInitialMode->GetCopTxLevel())
				{
					m_pIpInitialMode->SetCopTxLevel(pScm->GetCopTxLevel());
					bCopBestModeChangedInRmtCaps = TRUE;
				}
				POBJDELETE(pScm);
			}
			else
			{
				m_pIpInitialMode->SetCopTxLevel(m_lastCopForceEncoderLevel);
				bCopBestModeChangedInRmtCaps = TRUE;
			}
		}
		else
		{
			CreateNewModeForCopCascadeLecturerLink(pH323Scm);
			m_pIpInitialMode->SetMediaMode(pH323Scm->GetMediaMode(cmCapVideo, cmCapTransmit), cmCapVideo, cmCapTransmit);

			//in case m_bCascadeIsLecturer is active we ignore it
			//We need to find the best cap according to remote caps which will not exceed m_copResourceIndexOfCascadeLinkLecturer
		}
	}
	if(m_bPartyInUpgradeProcess)
	{
		//we can't start change scm while upgrade
		TRACEINTO << "Change Scm is not allowed while upgrading. move Scm to pending";
		MoveScmToPendingWhileUpgrading(pH323Scm);
		return;
	}

	if (pH323Scm)
		ChangeScmAccordingToH323Scm(pH323Scm);
	else
		PASSERT(1);

	const CVidModeH323 newTransVideo   = (const CVidModeH323&)m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapTransmit);
	const CVidModeH323 newTransContent = (const CVidModeH323&)m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	const CVidModeH323 newRecvVideo    = (const CVidModeH323&)m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapReceive);

	if (!(lastTransVideo == newTransVideo) || !(lastTransContent == newTransContent) || !(lastRecvVideo == newRecvVideo) || bCopBestModeChangedInRmtCaps)
		m_bIsNewScm = TRUE;

	m_changeModeInitiator = eConfInitiator;
	m_eChangeMediaType |= eChangeMediaType;

	DispatchChangeModeEvent();
}

/////////////////////////////////////////////////////////////////////////////
// this function is used for the Video_not_decoded message of H.263 annex N
// it uses the m_pCap which is CH221 class and if we change the CH323ChangeModeCntl class
// to use only H.323 classes we will need to replace it.
WORD  CH323ChangeModeCntl::IsSupportErrorCompensation()
{
	BYTE bRes = m_pIpCurrentMode->IsSupportErrorCompensation();
	return bRes;
}

//////////////////////////////////////////////////////////////////////////////////
/* Added for FLOW CONTROL*/
void CH323ChangeModeCntl::SendFlowControlFromConf(DWORD newVidRate)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::SendFlowControlFromConf : Name - ",m_partyConfName);
	m_pPartyApi->SendFlowControlToCs(newVidRate,FALSE);
}


/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OpenDataChannel(WORD bitRate,WORD type)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OpenDataChannel : Name - ",m_partyConfName);
	m_lsdChangeModeRequestRate = bitRate;
	if (type == LSD)
	{
		CSegment* pParam = new CSegment;
		*pParam << OPENDATACHANNEL;
		DispatchEvent(CHANGEMODEMSG,pParam);
		POBJDELETE(pParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::CloseDataChannel(WORD type)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::CloseDataChannel : Name - ",m_partyConfName);
	m_lsdChangeModeRequestRate = 0;
	m_lsdOldChangeModeRequestRate = 0;
	if (type == LSD)
	{
		CSegment* pParam = new CSegment;
		*pParam << CLOSEDATACHANNEL;
		DispatchEvent(CHANGEMODEMSG,pParam);
		POBJDELETE(pParam);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*oid  CH323ChangeModeCntl::UpdateVideoStreamIfNeeded()
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::UpdateVideoStreamIfNeeded : Name - ",m_partyConfName);
	if (m_isVidConn && ((m_state == CHANGEVIDEO) || (m_state == CHANGE_ALL_MEDIA) || m_bChannelsUpdatedWithoutChangeMode) )
	{
		if( (m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive)) &&
			(m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapTransmit)) )
		{
			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::UpdateVideoStreamIfNeeded : update. Name - ",m_partyConfName);
			m_pPartyApi->ReConnectStream(cmCapVideo); // we didn't disconnected the TS, so only new stream on is needed
			if (m_state == CHANGEVIDEO)//we changed the video without disconnecting from the bridge
			{
				StartTimer(VIDEO_UPDATE_TIMEOUT, 5); //50 mili-seconds - enable the 323 card to prepare the stream on and all the other staff...
_
				if (m_pIpInitialMode->GetConfType() == kVideoSwitch)
					m_pTaskApi->VideoActive(m_pParty,TRUE,MCMS);//unMute = on
			}
		}
	}
}*/

///////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyH323ConnectAllChangeAll(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyH323ConnectAllChangeAll : Name - ",m_partyConfName);
	PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll : Name - ",m_partyConfName);

	BYTE bOldVideoRxOn = m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople);

	CH323PartyCntl::PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);

	BYTE bNewVideoRxOn = m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople);
	if (!bOldVideoRxOn && bNewVideoRxOn)
		m_bSuspendVideoUpdates = FALSE; // if video receive channel is connected after it was disconnected, we can set the no updates flag to false.
	AdditionalRsrcHandling(statOK);
	ConnectPartyToFECCBridge(m_pIpCurrentMode);
}

///////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyReceivedReCapsChangeAll(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyReceivedReCapsChangeAll : Name - ",m_partyConfName);
 	PartyReceivedReCapsChangeAll(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyReceivedReCapsChangeVideo(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyReceivedReCapsChangeVideo : Name - ",m_partyConfName);
	PartyReceivedReCaps(pParam);

	if (m_pRmtCapH323->GetNumOfVideoCap() == 0)
	{
		PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::OnPartyReceivedReCapsChangeVideo - end change mode - remote doesn't have video caps , Name - ",m_partyConfName);
		//End change mode
		if (m_pIpInitialMode && m_pIpCurrentMode)
		{
			CComModeH323* pTmpModeH323 = NULL;
			if(!IsInitialContentModeEqualToCurContentMode())
			{
					pTmpModeH323 = new CComModeH323;
					*pTmpModeH323 = *m_pIpInitialMode;
			}


			*m_pIpInitialMode = *m_pIpCurrentMode;


			if(pTmpModeH323)
			{
					UpdateInitialWithNewContentMode(pTmpModeH323);
					POBJDELETE (pTmpModeH323);
			}
		}
		m_state = IDLE; // in order to end the change mode, and to disconnect from the video bridge, in case it is needed
		EndChangeMode();
	}
	if(!m_bPendingRemoteCaps)
		HandleRemoteReCap();
	else
		PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyReceivedReCapsChangeVideo -pending remote caps due to link cascade: Name - ",m_partyConfName);
}


///////////////////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyReceivedECS(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::OnPartyReceivedECS - Party is in ChangeMode - End it first , Name - ",m_partyConfName);
	//End change mode
	if (m_pIpInitialMode && m_pIpCurrentMode)
		*m_pIpInitialMode = *m_pIpCurrentMode; //in order to end the change mode
	m_state = IDLE; // in order to end the change mode, and to disconnect from the bridge, in case it's needed.
	if (IsValidTimer(CHANGETOUT))
		DeleteTimer(CHANGETOUT);
	EndChangeMode();

	PartyReceivedECS();
}

///////////////////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnTimerVideoUpdate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnTimerVideoUpdate : Name - ",m_partyConfName);
	m_pVideoBridgeInterface->VideoRefresh(GetPartyRsrcId());
}
///////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyH323ConnectAll_other(CSegment* pParam)
{
    CSmallString str;
    str << "CH323ChangeModeCntl::OnPartyH323ConnectAll_other " << m_partyConfName << " state:" << m_state;
    // Escalations (VNG) VNGFE-6939.
	m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);
	m_bIsNewScm = TRUE;

    PTRACE(eLevelInfoNormal,str.GetString());
}

///////////////////////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::OnPartyH323ConnectAllIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyH323ConnectAllIdle : Name - ",m_partyConfName);

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());

	if (!pConfParty)
	{
	    PTRACE(eLevelError,"CH323ChangeModeCntl::OnPartyH323ConnectAllIdle - pConfParty is NULL");
	    DBGPASSERT(1202);
	    return;
	}

	DWORD partyState = pConfParty->GetPartyState();
	if (partyState == PARTY_SECONDARY)
		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED_PARTIALY); //in order not to ignore this party in calculation the highest common

//	PTRACE2INT2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyH323ConnectAllIdle : partyState =  %d", partyState);
	// Bug 18961 - It is possible that during disconnection and reconnection of the video in channel, the video bridge will
	// change party state to PARTY_CONNECTED_WITH_PROBLEM due to video sync loss. The additional condition was added
	// because there is an old bug in the move process where in the begining of move procedure, the party state is changed
	// to PARTY_CONNECTED_WITH_PROBLEM instead of PARTY_CONNECTED_PARTIALY and we do not want to risk the overwriting of
	// the CExportH323PartyCntl methods.

	if (partyState == PARTY_CONNECTED || (partyState == PARTY_CONNECTED_WITH_PROBLEM && ( ( !( strcmp(GetRTType(),"CH323ChangeModeCntl" )) ) || !( strcmp(NameOf(),"CH323ChangeModePartyCntl" )))))
	{/* In VSW, the SCM is determined according to remote capabilities. Since in this case
	 the change is in the channel, we don't need to recalculate the common denominator.
	 We just need to check if the new mode is suite to the conf settings.
	 But in CP, the SCM is determined according to channels, so we do need to update conf. */
		//m_bChannelsUpdatedWithoutChangeMode = TRUE;

		if (m_pIpInitialMode->GetConfType() == kCp || m_pIpInitialMode->GetConfType() == kCop)
			PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);

		else //VSW
		{
			m_pIpCurrentMode->DeSerialize(NATIVE,*pParam);

			DWORD videoRate;
			WORD  videoChanged = 0;
			*pParam >> videoRate;

			WORD status;
			*pParam >> status;
			if(status != statOK)
			{
				DBGPASSERT(status);
				PTRACE2(eLevelError,"CH323ChangeModeCntl::OnPartyH323ConnectAllIconnectOrChangeAll : \'Status isn't o.k. !!! \' Name - ",m_partyConfName);
				m_pTaskApi->UpdateDB(m_pParty,DISCAUSE, H323_FAILURE);
				return;
			}
			*pParam >> m_isCodianVcr;

		    *pParam >> m_incomingVideoChannelHandle;
		    TRACEINTO << "mix_mode: channel handle set to "  << m_incomingVideoChannelHandle;

			// VNGR-6663
			m_pRmtCapH323->DeSerialize(NATIVE,*pParam);
			DispatchChangeModeEvent(); //in order to call the function OnConfChangeModeIdle and check if the new mode is suite to the conf settings.
		}
	}
	else
		PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);
}


///////////////////////////////////////////////////////////////////////////////////////////
/*void  CH323ChangeModeCntl::OnPartyEndContentReconnectIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndContentReconnectIdle : Name - ",m_partyConfName);

	PartyEndContentReconnect(pParam);

	if (m_pContentCntl && m_pContentCntl->IsConnected())
		ChangeContent();
}*/

/////////////////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyH323SetSecondaryCause(CSegment* pParam)
{
	CMedString trace_str;
	trace_str << "Name = " << m_partyConfName;
	trace_str << "We do not initialize secondary cause, we will wait till the conference decide on seconadry cause. " << "\n";

	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyH323SetSecondaryCause - ",trace_str.GetString());
}

///////////////////////////////////////////////////////////////////////////////////////////
/*void  CH323ChangeModeCntl::OnPartyEndContentReconnectAnycaseButIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyEndContentReconnectAnycaseButIdle : Name - ",m_partyConfName);
	PartyEndContentReconnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::PartyEndContentReconnect(CSegment* pParam)
{
	OFF(m_bNoContentChannel);

	CVidModeH323* pTempContentReceiveMode = new CVidModeH323;
	pTempContentReceiveMode->DeSerialize(NATIVE,*pParam);
	m_pIpCurrentMode->SetMediaMode(*pTempContentReceiveMode, cmCapVideo, cmCapReceive, kRoleContent);
	POBJDELETE(pTempContentReceiveMode);
}*/

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyMuteVideoIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyMuteVideoIdle : Name - ",m_partyConfName);
	PartyMuteVideo(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyMuteVideoAnycase(CSegment* pParam)
{
	CMedString str;
	str << "Name = " << m_partyConfName;
	switch (m_state)
	{
		case CHANGECONTENT:
			str << "State = CHANGECONTENT";
			break;
		case CHANGEOTHER:
			str << "State = CHANGEOTHER";
			break;
		case CHANGEVIDEO:
			str << "State = CHANGEVIDEO";
			break;
	}
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyMuteVideoAnycase: ", str.GetString());
	PartyMuteVideo(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::PartyMuteVideo(CSegment* pParam)
{
	WORD onOff;
	*pParam >> onOff;
	EOnOff eOnOff = onOff ? eOn : eOff;
	m_pVideoBridgeInterface->UpdateMute(GetPartyRsrcId(), eOnOff, PARTY);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnH323DBC2Command(CSegment* pParam)
{
    PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnH323DBC2Command : Name - ",m_partyConfName);
	//m_pVidConnection->DBC2Command(pParam);
}

/////////////////////////////////////////////////////////////////////////////
WORD CH323ChangeModeCntl::IsFeccChanOpened() const
{
	//The cases the function should return TRUE:
	//1. In case the party has capabilities of FECC and one of it's channel are opened.
	//2. In case the party has capabilities of FECC but he did not open/close it's fecc channels, and he in secondary mode
	//3. In case on of the sides does not support fecc

	BOOL bIsPartySupportFecc = FALSE;
	if(m_pRmtCapH323->IsFECC() && m_pLocalCapH323->IsFECC())
	{
		if(m_pIpCurrentMode->IsMediaOn(cmCapData,cmCapReceive) || m_pIpCurrentMode->IsMediaOn(cmCapData,cmCapTransmit))
			bIsPartySupportFecc = TRUE;
		else if(IsSecondary())
			bIsPartySupportFecc = TRUE;
		else
			bIsPartySupportFecc = FALSE;
	}
	else
		bIsPartySupportFecc = TRUE;

	if(bIsPartySupportFecc && !m_bConfWaitToEndChangeModeForFecc)
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
/*WORD CH323ChangeModeCntl::GetOldLsdRate()
{
	DWORD oldLsdBitRate		= 0;
	WORD  oldLsdMode		= m_pScm->GetLsdMode();

	Get_Lsd_Mlp_Command_BitRate((BYTE)oldLsdMode, (WORD *)&oldLsdBitRate);

	return oldLsdBitRate;
}*/

////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnVideoBridgePartyChangeLayout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnVideoBridgePartyChangeLayout. Name - ", m_partyConfName);
	StartPartyMsgOnScreen();
}

////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::StartPartyMsgOnScreen()
{
	//until decision of SRE, I disable the feature.
	//return;

	char**	displayStringArr = new char*[MAX_TEXT_LEN]; //800
	DWORD displayStringArrNo = 0;
	BYTE rVal = FALSE;

	//all the message on screen party should be added here
	rVal = SetNoContentMsg(displayStringArr, displayStringArrNo);

	if(rVal)
	{
		m_pVideoBridgeInterface->DisplayPartyMsgOnScreen(GetPartyRsrcId(), displayStringArr, displayStringArrNo);
		PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::StartPartyMsgOnScreen, number of messages to display - ", displayStringArrNo);
	}
	else
		PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::StartPartyMsgOnScreen, no message to display");

	if(displayStringArr)
	{
		for (DWORD j = 0  ; j < displayStringArrNo ; j++ )
			PDELETEA(displayStringArr[j]);
	}

	PDELETEA(displayStringArr);
}

////////////////////////////////////////////////////////////////////////////
// the function return true if it set the string with valid data
BYTE CH323ChangeModeCntl::SetNoContentMsg(char** displayStringArr, DWORD &displayStringArrNo)
{
	BYTE rVal = FALSE;
	CapEnum commonContentMode = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
    if(m_pRmtCapH323->IsH239() && (commonContentMode == eUnknownAlgorithemCapCode))
    {// remote has H239 but different mode than the conference setting. We assume local always has content (this is the MCU defualt setting)
    	CapEnum contentOptionMode = eH264CapCode;
    	APIS32  H264mode          = H264_standard;
    	if (m_pIpInitialMode->IsTIPContentEnableInH264Scm() == TRUE)
    		H264mode = H264_tipContent;
		BYTE rValRemote = m_pRmtCapH323->AreCapsSupportProtocol(contentOptionMode, cmCapVideo, kRoleContentOrPresentation, H264mode);
		BYTE rValLocal = m_pLocalCapH323->AreCapsSupportProtocol(contentOptionMode, cmCapVideo, kRoleContentOrPresentation, H264mode);

		// add "No Content.
		displayStringArr[displayStringArrNo] = new char[MAX_SITE_NAME_ARR_SIZE];
		memset(displayStringArr[displayStringArrNo],'\0', MAX_SITE_NAME_ARR_SIZE);
		strncpy(displayStringArr[displayStringArrNo], "No Content.", MAX_SITE_NAME_ARR_SIZE);
		displayStringArrNo++;

		displayStringArr[displayStringArrNo] = new char[MAX_SITE_NAME_ARR_SIZE];
		memset(displayStringArr[displayStringArrNo],'\0', MAX_SITE_NAME_ARR_SIZE);

		if(!rValRemote && rValLocal)
		{
			// add "Conference content mode is H264"
			strncpy(displayStringArr[displayStringArrNo], "Conference content mode is H264", MAX_SITE_NAME_ARR_SIZE);
		}
		else if(rValRemote && !rValLocal)
		{
			// add "Conference content mode is H264"
			strncpy(displayStringArr[displayStringArrNo], "Conference content mode is H263", MAX_SITE_NAME_ARR_SIZE);
		}

		displayStringArrNo++;
		rVal = TRUE;
    }
    return rVal;
}

////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCAMUpdatePartyInConf(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnCAMUpdatePartyInConf : Name - ",m_partyConfName);
	ON(m_isPartyInConf);

	// Need to check if presentation chennles are opened
	if ( !m_bNoContentChannel) //party can have content!
		{//check if the content channels were opened
			if (!m_isSentH239Out)
			{
				if (m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))
				{
						ON(m_isSentH239Out);
						m_pPartyApi->UpdatePresentationOutStream();
				}
			}

		}
}
///////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyPresentationOutStreamUpdate(CSegment* pParam)
{
	//if we receive this opcode, we already received PARTY_IN_CONF from CAM.
    TRACESTR(eLevelInfoNormal)<< " CH323ChangeModeCntl::OnPartyPresentationOutStreamUpdate : state - " << m_state << ",  Name - " << m_partyConfName;
	ON(m_presentationStreamOutIsUpdated);

}


/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCGSendStartContent()
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnCGSendStartContent - ERROR - system is not CG!!");
		return;
	}

	m_pPartyApi->CGSendContent();
}


////////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCGSendStopContent()
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnCGSendStopContent - ERROR - system is not CG!!");
		return;
	}

	m_pPartyApi->CGStopContent();
}


///////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyPresentationOutStreamUpdateIdle(CSegment* pParam)
{
	//if we receive this opcode we also received PARTY_IN_CONF from CAM, so we can
	// connect to the content bridge
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnPartyPresentationOutStreamUpdateIdle : Name - ",m_partyConfName);
	OnPartyPresentationOutStreamUpdate(pParam);

	ConnectToContentBridgeIfPossible();
}

////////////////////////////////////////////////////////////////////////////
//Inorder to connect to the content bridge we need to receive 2 opcodes:
//1) From CAM - PARTY_IN_CONF
//2) From PARTY - PRESENTATION_OUT_STR_UPDATE
//This function check both flags and connect to content bridge or
// send UpdatePresentationOutStream to party if necessary...
void CH323ChangeModeCntl::ConnectToContentBridgeIfPossible()
{
	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ConnectToContentBridgeIfPossible : Name - ",m_partyConfName);
	if(m_isPartyInConf)
	{
		if(m_presentationStreamOutIsUpdated)
		{

			PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ConnectToContentBridgeIfPossible: Connect the content bridge, Name - ",m_partyConfName);
			if(!m_isContentConn)
			{
				m_state	= CHANGECONTENT;
			   ConnectPartyToContentBridge();
			}
			else
				PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ConnectToContentBridgeIfPossible: content bridge already connected - ignore, Name - ",m_partyConfName);
		}
		else
		{	// Need to check if presentation chennles are opened
			if ( !m_bNoContentChannel) //party can have content!
			{//check if the content channels were opened
				if (!m_isSentH239Out)
				{
					if (m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation))
					{
						PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ConnectToContentBridgeIfPossible: Send update presentation stream out to party, Name - ",m_partyConfName);
						m_pPartyApi->UpdatePresentationOutStream();
						ON(m_isSentH239Out);
					}
				}
				else
					PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::ConnectToContentBridgeIfPossible: content channels aren't open yet... Name - ",m_partyConfName);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsRemoteAndLocalCapSetHasContent(eToPrint toPrint)const
{
	BYTE res = NO;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (m_pRmtCapH323->IsH239() && m_pLocalCapH323->IsH239())
			res = YES;
		else if (m_pRmtCapH323->IsEPC() && m_pLocalCapH323->IsEPC())
			res = YES;
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsRemoteAndLocalHasEPCContentOnly()
{
	BYTE res = NO;
	if (m_pRmtCapH323 && m_pLocalCapH323)
	{
		if (!m_pRmtCapH323->IsH239() || !m_pLocalCapH323->IsH239())
			if (m_pRmtCapH323->IsEPC() && m_pLocalCapH323->IsEPC())
				res = YES;

	}
	return res;
}
///////////////////////////////////////////////////////////////////////////
void  CH323ChangeModeCntl::EndConnectionProcess(WORD status)
{
        if (IsNeedToConnectToVideoBridge(m_pIpCurrentMode))
        {
            PTRACE2(eLevelInfoNormal," CH323ChangeModeCntl::EndConnectionProcess - Establishing Video Connection... ",m_partyConfName);

            if (m_pIpInitialMode && m_pIpCurrentMode)
            {
                CComModeH323* pTmpModeH323 = NULL;
                if(!IsInitialContentModeEqualToCurContentMode())
                {
                        pTmpModeH323 = new CComModeH323;
                        *pTmpModeH323 = *m_pIpInitialMode;
                }

                *m_pIpInitialMode = *m_pIpCurrentMode;

                if(pTmpModeH323)
                {
                        UpdateInitialWithNewContentMode(pTmpModeH323);
                        POBJDELETE (pTmpModeH323);
                }
            }

            ConnectPartyToVideoBridge(m_pIpCurrentMode);
        }
        else
            PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::EndConnectionProcess - Do nothing");
}
///////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCopVideoBridgeChangeIn(CSegment* pParam)
{
	CIpComMode* pNewScm = new CIpComMode(*m_pIpInitialMode);
	CopVideoBridgeChangeIn(pParam, pNewScm);
    BYTE bModesAreTheSame = FALSE;
    const CVidModeH323 InitRecvVideo = (const CVidModeH323 &)m_pIpInitialMode->GetMediaMode(cmCapVideo,cmCapReceive);
    const CVidModeH323 newRecvVideo = (const CVidModeH323 &)pNewScm->GetMediaMode(cmCapVideo,cmCapReceive);
    if (InitRecvVideo == newRecvVideo)
        bModesAreTheSame = FORCE_UPDATE_IN;
	ChangeScm(pNewScm, eCanChangeVideoAndContent, bModesAreTheSame);
	POBJDELETE(pNewScm);
}
////////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCopVideoBridgeChangeOut(CSegment* pParam)
{
	CopVideoBridgeChangeOut(pParam);
	ChangeScm(m_pIpInitialMode, eCanChangeVideoAndContent);
}
//////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCopCascadeLectureMode(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeLectureMode");
	BYTE isLinkLecturer = FALSE;
	*pParam >> isLinkLecturer;
	if (IsValidTimer(COP_UPDATE_CASCADE_LINK_LECTURE_MODE_TIMER))
			DeleteTimer(COP_UPDATE_CASCADE_LINK_LECTURE_MODE_TIMER);
	if(m_bCascadeIsLecturer == isLinkLecturer && isLinkLecturer == FALSE)
	{
		PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeLectureMode - got update lecture mode with the same mode as before.this is the mode", m_bCascadeIsLecturer);
		DBGPASSERT(102);
	}
	m_bCascadeIsLecturer = isLinkLecturer;
	if(!isLinkLecturer && m_bPendingRemoteCaps)
	{
		PTRACE2INT(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeLectureMode - pending on but got lecture of ", m_bCascadeIsLecturer);
		DBGPASSERT(103);

	}
	m_bPendingRemoteCaps = FALSE;
	CIpComMode* pNewScm = new CIpComMode(*m_pIpInitialMode);
	const CVidModeH323 InitTxVideo = (const CVidModeH323 &)m_pIpInitialMode->GetMediaMode(cmCapVideo,cmCapTransmit);
//	COstrStream msg;//temp
//	InitTxVideo.Dump(msg); //temp
//	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeLectureMode - initial tx", msg.str().c_str());//temp
	CopVideoBridgeChangeLinkLectureModeOut(pNewScm);
	BYTE bModesAreTheSame = FALSE;
	const CVidModeH323 newTxVideo = (const CVidModeH323 &)pNewScm->GetMediaMode(cmCapVideo,cmCapTransmit);
	//COstrStream msg2;//temp
//	newTxVideo.Dump(msg2); //temp
//	PTRACE2(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeLectureMode - new tx", msg2.str().c_str());//temp
	if (InitTxVideo == newTxVideo)
	   bModesAreTheSame = FORCE_UPDATE_OUT;
	ChangeScm(pNewScm, eCanChangeVideoAndContent,bModesAreTheSame);//maybe we can improve our out mode


}
////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCopCascadeStartCopLinkLecturePendingMode(CSegment* pParam)
{
	if(m_pIpInitialMode->GetConfType() == kCop)
	{
		if( m_copResourceIndexOfCascadeLinkLecturer != INVALID_COP_LEVEL)
		{
			PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeStartCopLinkLecturePendingMode - set pending mode", GetPartyRsrcId());
			m_bPendingRemoteCaps = TRUE;
			m_bCascadeIsLecturer = TRUE;
			//StartTimer(COP_UPDATE_CASCADE_LINK_LECTURE_MODE_TIMER,PARTYCNTL_CHANGERATE_TIME);

		}
		else
		{
			PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeStartCopLinkLecturePendingMode - invalid level", GetPartyRsrcId());
		}
	}
	else
		PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeStartCopLinkLecturePendingMode - this in not cop conf", GetPartyRsrcId());
	m_pVideoBridgeInterface->UpdateEMPartyStartCascadeLinkAsLecturerPendingMode(m_pParty, m_copResourceIndexOfCascadeLinkLecturer);

}
//////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCopCascadeCopLinkLecturePendingModeTimerExpire(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopCascadeCopLinkLecturePendingModeTimerExpire", GetPartyRsrcId());
	DBGPASSERT(109);
	m_bPendingRemoteCaps = FALSE;
	m_bCascadeIsLecturer = FALSE;
}
///////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnCopNoVideoUpdatesTout(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::OnCopNoVideoUpdatesTout", GetPartyRsrcId());
	if (m_bSuspendVideoUpdates)
	{
		m_bSuspendVideoUpdates = FALSE;
		m_pIpCurrentMode->Dump("CH323ChangeModeCntl::OnCopNoVideoUpdatesTout : current",eLevelInfoNormal);
		DisconnectPartyFromVideoBridgeIfNeeded(m_pIpCurrentMode);
	}
}

BYTE CH323ChangeModeCntl::IsResolutionFeetsToRateInCp(BYTE& reason, DWORD& details)
{
	BYTE bRes = FALSE;
	CapEnum capCodeRec = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive));
	CapEnum capCodeTx = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit));
	if (m_productId && strstr(m_productId, "VVX"))
	{
		PTRACE2(eLevelInfoNormal, "CH323ChangeModeCntl::IsResolutionFeetsToRateInCp remote VVX has problems with recap  : Name - ", m_partyConfName);
		return FALSE;
	}
	if ((capCodeRec == eH264CapCode) && (capCodeTx == eH264CapCode)) //check if capCode is 264 otherwise do nothing
	{
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetMonitorPartyId());
		if (CPObject::IsValidPObjectPtr(pConfParty))
		{
			BYTE partyResolution = pConfParty->GetMaxResolution();
			if (partyResolution != eAuto_Res)
				return eConnect;
		}
		else
			DBGPASSERT(GetPartyRsrcId());

		DWORD newBitRate = m_pRmtCapH323->GetMaxVideoBitRate(capCodeRec, cmCapReceive, kRolePeople);
		DWORD oldBitRate = m_pLocalCapH323->GetMaxVideoBitRate(capCodeRec, cmCapReceive, kRolePeople);
		DWORD audioRate = m_pIpInitialMode->GetMediaBitRate(cmCapAudio) * 1000;

		TRACEINTO
			<< "NewBitRate:" << (WORD)newBitRate
			<< ", OldBitRate:" << oldBitRate
			<< ", AudioRate:" << (WORD)audioRate
			<< ", ConfBitRate:" << (DWORD)((oldBitRate * 100) + audioRate);

		static DWORD threshold = 0xFFFFFFFF;
		if (threshold == 0xFFFFFFFF)
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("VIDEO_RESOLUTION_DECREASE_THRESHOLD_PERCENT", threshold);

		if (((oldBitRate * 100) + audioRate) > ((newBitRate * 100) + (newBitRate * threshold)))
		{
			DWORD partyRate = (newBitRate * 100) + audioRate;
			eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
			H264VideoModeDetails h264VidModeDetails;
			BYTE isHighprofie = (m_pIpCurrentMode->IsH264HighProfile(cmCapReceive) || m_pIpCurrentMode->IsH264HighProfile(cmCapTransmit));

			Eh264VideoModeType resourceMaxVideoMode = TranslateCPVideoPartyTypeToMaxH264VideoModeType(m_eLastAllocatedVideoPartyType);

			if (resourceMaxVideoMode == eHD720Asymmetric || resourceMaxVideoMode == eHD720At60Asymmetric || resourceMaxVideoMode == eHD1080Asymmetric || resourceMaxVideoMode == eHD1080At60Asymmetric)
				::GetH264AssymetricVideoParams(h264VidModeDetails, partyRate, vidQuality, resourceMaxVideoMode, isHighprofie);
			else
				::GetH264VideoParams(h264VidModeDetails, partyRate, vidQuality, resourceMaxVideoMode, isHighprofie);

			long newFs = h264VidModeDetails.maxFS;
			if (newFs == (long)INVALID)
			{
				CH264Details thisH264Details = h264VidModeDetails.levelValue;
				newFs = thisH264Details.GetDefaultFsAsDevision();
			}

			DWORD FsInRmtCapH323;
			long MbpsInRmtCapH323;

			if (isHighprofie)
			{
				FsInRmtCapH323 = m_pRmtCapH323->GetMaxFsAccordingToProfile(H264_Profile_High);
				MbpsInRmtCapH323 = m_pRmtCapH323->GetMaxMbpsAccordingToProfile(H264_Profile_High);
			}
			else
			{
				FsInRmtCapH323 = m_pRmtCapH323->GetMaxFsAccordingToProfile(H264_Profile_BaseLine);
				MbpsInRmtCapH323 = m_pRmtCapH323->GetMaxMbpsAccordingToProfile(H264_Profile_BaseLine);
			}

			DWORD FsInLocalCapH323;
			if (isHighprofie)
				FsInLocalCapH323 = m_pLocalCapH323->GetMaxFsAccordingToProfile(H264_Profile_High);
			else
				FsInLocalCapH323 = m_pLocalCapH323->GetMaxFsAccordingToProfile(H264_Profile_BaseLine);

			long newMBPS = h264VidModeDetails.maxMBPS;
			long MinMBPS;

			if (newMBPS == (long)INVALID)
			{
				CH264Details thisH264Details = h264VidModeDetails.levelValue;
				newMBPS = thisH264Details.GetDefaultMbpsAsDevision();

			}
			// We need to check if the rmt mbps is lower than the resource mbps
			TRACEINTO << "MbpsNew:" << (WORD)newMBPS << ",  MbpsInRmtCapH323:" << (WORD)MbpsInRmtCapH323;
			if (MbpsInRmtCapH323 != (long)INVALID)
			{
				newMBPS = min(newMBPS, MbpsInRmtCapH323);
			}

			long currentFs = 0;
			long currentFs1 = 0;
			long currentFs2 = 0;
			long currentMBPS = 0;
			APIU16 currentProfile = 0;
			long currentDPB = 0;
			long currentBRandCPB = 0;
			long currentSAR = 0;
			long currentStaticMB = 0;
			APIU8 currentLevel1 = 0;
			APIU8 currentLevel2 = 0;

			m_pIpCurrentMode->GetH264Scm(currentProfile, currentLevel1, currentMBPS, currentFs1, currentDPB, currentBRandCPB, currentSAR, currentStaticMB, cmCapReceive);
			m_pIpCurrentMode->GetH264Scm(currentProfile, currentLevel2, currentMBPS, currentFs2, currentDPB, currentBRandCPB, currentSAR, currentStaticMB, cmCapTransmit);
			if (currentFs1 == (long)INVALID)
			{
				CH264Details thisH264Details = currentLevel1;
				currentFs1 = thisH264Details.GetDefaultFsAsDevision();
			}
			if (currentFs2 == (long)INVALID)
			{
				CH264Details thisH264Details = currentLevel2;
				currentFs2 = thisH264Details.GetDefaultFsAsDevision();
			}
			currentFs = max(currentFs1, currentFs2);

			if (newFs < currentFs)
			{
				TRACEINTO << "FsNew:" << (WORD)newFs << ",  FsCurrent:" << (WORD)currentFs;
				bRes = TRUE;

				if ((DWORD)newFs < FsInLocalCapH323)
					m_pIpInitialMode->SetH264Scm(currentProfile, h264VidModeDetails.levelValue, newMBPS, newFs, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, h264VidModeDetails.maxStaticMbps, cmCapReceive);

				if ((DWORD)newFs < FsInRmtCapH323)
					m_pIpInitialMode->SetH264Scm(currentProfile, h264VidModeDetails.levelValue, newMBPS, newFs, h264VidModeDetails.maxDPB, h264VidModeDetails.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, h264VidModeDetails.maxStaticMbps, cmCapTransmit);
				reason = LOW_RESOLUTION_REQUIRED;
			}
		}
	}
	return bRes;
}

/////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::IsNeedToChangeDueToSwitchFromLecturerToNonLecturer(BYTE copLevel,BYTE takeInitial)
{
	 if(CASCADE_MODE_SLAVE == m_masterSlaveStatus)
	{
	   if(strstr(m_VersionId, "V4.6") || m_IsCascadeToCopMcu)  //tbd RMX 1000
	   {
		   if(!m_bCascadeIsLecturer && copLevel)
		   {
			   CVidModeH323* pVidMode = m_pCopVideoTxModes->GetVideoMode(copLevel);
			   CVidModeH323 *pVidModeAccordingToInitialParams = NULL;
			   if(takeInitial)
				   pVidModeAccordingToInitialParams	= &(CVidModeH323 &)m_pIpInitialMode->GetMediaMode(cmCapVideo,cmCapTransmit);
			   else
				   pVidModeAccordingToInitialParams	= &(CVidModeH323 &)m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapTransmit);
			   if(pVidMode!= pVidModeAccordingToInitialParams)
				   return TRUE;

		   }

	   }

	}
	 return FALSE;

}
/////////////////////////////////////////////////////////////////////////////
BYTE CH323ChangeModeCntl::DisconnectForUpdateLegacyStatusIfNeeded()
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ChangeModeCntl::DisconnectForUpdateLegacyStatusIfNeeded", GetPartyRsrcId());

	if (m_bIsMrcCall)
	{
		return FALSE;
	}
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

	if (pCommConf && pCommConf->IsLegacyShowContentAsVideo())
	{
		CIpComMode* pTmpScm = new CIpComMode;

		*pTmpScm = *m_pIpInitialMode;

		pTmpScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);

		unsigned int  tmp_incomingVideoChannelHandle = m_incomingVideoChannelHandle;

		BYTE bIsDisconnectFromVideoBridge = DisconnectPartyFromVideoBridgeIfNeeded(pTmpScm);

		//BRIDGE-14310, in case update legacy status, the incoming video channel handler should unchange since 
		//video receive port doesn't need to close/reopen.
		m_incomingVideoChannelHandle = tmp_incomingVideoChannelHandle;

		POBJDELETE(pTmpScm);


		if (bIsDisconnectFromVideoBridge)
		{
				PTRACE2PARTYID(eLevelInfoNormal, "CH323ChangeModeCntl::DisconnectForUpdateLegacyStatusIfNeeded : updating video brdige, content to legacy Name - ", m_partyConfName, GetPartyRsrcId());
				m_state = UPDATE_LEGACY_STATUS;
				return TRUE;
		}

	}
	return FALSE;

}
/////////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CH323ChangeModePartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus : Name - ", m_partyConfName, GetPartyRsrcId());
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
		return;
	}
	 ConnectPartyToVideoBridge(m_pIpCurrentMode);

	//ContinueToChangeContent();
}
/////////////////////////////////////////////////////////////////////////
DWORD CH323ChangeModeCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate(CSegment* pParam)
{

	TRACEINTO << "mix_mode: AVC_SVC_ADDITIONAL_PARTY_RSRC_IND accepted";
	CRsrcParams* pMrmpRsrcParams = NULL;
	CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
	DWORD status = CIpPartyCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate(pParam, avcToSvcTranslatorRsrcParams, pMrmpRsrcParams,ALLOCATION_TYPE_UPGRADE);
	if (status != STATUS_OK)
	{	// disconnect party
//		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, RESOURCES_DEFICIENCY);
		return status;
	}

	if(IsSoftMcu() && (m_pPartyAllocatedRsrc->GetVideoPartyType()==eVideo_party_type_none))
	{
		TRACEINTO << "mix_mode: For softMCU,Audio only participant needn't change mode";
		return status;
	}
		
//	m_eChangeModeState = eConfRequestMoveToMixed;

	TRACEINTO << "mix_mode: before ChangeModeFromScm pMrmpRsrcParams=" << pMrmpRsrcParams
			<< " avcToSvcTranslatorRsrcParams[0]=" << avcToSvcTranslatorRsrcParams[0] << " avcToSvcTranslatorRsrcParams[1]="  << avcToSvcTranslatorRsrcParams[1];


	// @#@ - ask Noa regarding states
	m_changeModeInitiator = eConfInitiator;
	ChangeModeFromScm(eConfRequestMoveToMixed, eChangeMode_Must, cmCapVideo, kRolePeople, avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);
	TRACEINTO << "mix_mode: after ChangeModeFromScm";
	return STATUS_OK;

	//
//	m_pPartyApi->ChangeModeIp(m_pIpInitialMode,m_eChangeModeState, NO/*content speaker*/, m_pSipLocalCaps,avcToSvcTranslatorRsrcParams,pMrmpRsrcParams);

}

/////////////////////////////////////////////////////////////////////////
void CH323ChangeModeCntl::OnPartyUpgradeToMixTout(CSegment* pParam)
{
	TRACEINTO << "mix_mode: dynMixedErr got timeout on H.323 party upgrade to mixed";

	// Set state to Connected with problem and check party state too - need to reduce counter at h323 control level
	m_state = IDLE;
	m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, SIP_TOUT_DURING_UPGRADE_TO_MIXED);
//	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATUS, PARTY_RESET_STATUS);
}
void CH323ChangeModeCntl::FinishUpgradeToMix()
{
	DeleteTimer(CHANGETOUT);
	CIpPartyCntl::FinishUpgradeToMix();
}
