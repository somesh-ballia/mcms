//                            SIPPartyControl.CPP                          |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControl.CPP                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include "DataTypes.h"
#include "StateMachine.h"
#include "Segment.h"
#include "NStream.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "ConfPartyOpcodes.h"
#include "CommModeInfo.h"
#include "Capabilities.h"
#include "TaskApi.h"
#include "Party.h"
#include "PartyApi.h"
#include "BridgeMoveParams.h"

#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "IpPartyControl.h"
#include "SipNetSetup.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipCall.h"
#include "SIPCommon.h"
#include "SIPPartyControl.h"
//bridges
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "H264Util.h"
#include "ContentBridge.h"
#include "ScpNotificationWrapper.h"
#include "ScpHandler.h"
#include "ScpPipeMappingNotification.h"
#include "ConfPartyProcess.h"
#include "EnumsToStrings.h"

extern CIpServiceListManager* GetIpServiceListMngr();

#define MAX_SCP_RETRANSMISSIONS  5 //according to SCP spec -don't send ack more than 5 times
#define IVR_RETRANSMISSION_TOUT  (SECOND / 2)
#define MAX_IVR_SCP_RETRANSMISSIONS 10

PBEGIN_MESSAGE_MAP(CSipPartyCntl)
	ONEVENT(IP_MUTE_MEDIA,					ANYCASE,		CSipPartyCntl::OnPartyChangeBridgesMuteState)
	ONEVENT(PARTY_VIDEO_CONNECTED,			ANYCASE,		CSipPartyCntl::OnVideoBrdgConnectedIdleOrAnycase)
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	ANYCASE,		CSipPartyCntl::OnVideoBrdgConnectedIdleOrAnycase)
	ONEVENT(PARTY_VIDEO_DISCONNECTED,		ANYCASE,		CSipPartyCntl::OnVideoBrdgDisconnected)
	ONEVENT(PARTY_AUDIO_DISCONNECTED,		ANYCASE,		CSipPartyCntl::OnAudBrdgDisconnect)
	ONEVENT(PARTY_VIDEO_IN_UPDATED,			PARTY_RE_CAPS,	CSipPartyCntl::OnVideoInBrdgUpdatedPartyReCaps) //VSGNINJA-589
	ONEVENT(PARTY_VIDEO_IN_UPDATED,			ANYCASE,		CSipPartyCntl::OnVideoInBrdgUpdatedAnycase)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,		PARTY_RE_CAPS,	CSipPartyCntl::OnVideoOutBrdgUpdatedPartyReCaps) //VSGNINJA-589
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,		ANYCASE,		CSipPartyCntl::OnVideoOutBrdgUpdatedAnycase)
    ONEVENT(UPDATE_VIDEO_RATE,  			ANYCASE,	    CSipPartyCntl::OnUpdatePartyVideoBitRate)
    ONEVENT(UPDATEICEPARAMS,			    ANYCASE,		CSipPartyCntl::OnPartyUpdateICEParams)
    ONEVENT(UPDATE_VIDEO_RESOLUTION,		ANYCASE,		CSipPartyCntl::OnPartyUpdateVideoResolution)
    ONEVENT(UPDATE_VIDEO_AFTER_VSR_MSG,		ANYCASE,		CSipPartyCntl::OnPartyUpdateVideoAfterVsrMsg)

    ONEVENT(SINGLE_UPDATE_PACSI_INFO,		ANYCASE,		CSipPartyCntl::OnPartyReceviedSingleUpdatePacsiInfoAnycase)
    ONEVENT(MSFT_AVMCU2013DETECTED,		    ANYCASE,		CSipPartyCntl::OnPartyAvmcu2013Detected)
    ONEVENT(MSFT_AVMCU2013DETECTED,		    DISCONNECT_BRIDGES,		CSipPartyCntl::NullActionFunction)


// LPR
    ONEVENT(LPR_CHANGE_RATE  				,ANYCASE,       CSipPartyCntl::OnUpdatePartyLprVideoBitRate)

//LYNC2013_FEC_RED:
    ONEVENT(FEC_RED_CHANGE_RATE 		       			,ANYCASE,       CSipPartyCntl::OnPartyCntlUpdatePartyFecOrRedVideoBitRate)
    ONEVENT(SIP_PARTY_CONTROL_SINGLE_FEC_RED_MSG 		,ANYCASE,       CSipPartyCntl::OnPartyCntlSendPartySingleFecOrRedMsg)

    ONEVENT(SIP_SEND_VIDEO_PREFERENCE_TOUT   ,ANYCASE,      CSipPartyCntl::OnSendVideoPreferenceToPartyTout)

    ONEVENT(FALL_BACK_FROM_TIP_TO_REGULAR_SIP  				,ANYCASE,       CSipPartyCntl::OnFallBackFromTip)
    ONEVENT(ADDSLAVEPARTY  				    ,ANYCASE,       CSipPartyCntl::OnAddSlaveParty)
    ONEVENT(PARTY_CONTROL_SLAVE_TO_MASTER_ACK,			    ANYCASE,			CSipPartyCntl::OnSlaveToMasterAckMessage)
    ONEVENT(PARTY_CONTROL_SLAVE_TO_MASTER,				    ANYCASE,			CSipPartyCntl::OnSlaveToMasterMessage)
    ONEVENT(PARTY_PARTYCONTROL_MASTER_TO_SLAVE,			    ANYCASE,			CSipPartyCntl::PartyToPartyCntlMessageMasterToSlave)
    ONEVENT(PARTYCONTROL_PARTY_MASTER_TO_SLAVE,			    ANYCASE,			CSipPartyCntl::PartyCntlToPartyMessageMasterToSlave)
    ONEVENT(PARTY_PARTYCONTROL_SLAVE_TO_MASTER,			    ANYCASE,			CSipPartyCntl::PartyToPartyCntlMessageSlaveToMaster)
    ONEVENT(PARTYCONTROL_PARTY_SLAVE_TO_MASTER,			    ANYCASE,			CSipPartyCntl::PartyCntlToPartyMessageSlaveToMaster)
    ONEVENT(SCP_REQUEST_BY_EP,								ANYCASE,			CSipPartyCntl::OnPartyCntlScpRequestByEP)
    ONEVENT(SCP_NOTIFICATION_BY_EP,							ANYCASE,			CSipPartyCntl::OnPartyCntlScpNotificationByEP)
    ONEVENT(SCP_NOTIFICATION_REQ,							ANYCASE,			CSipPartyCntl::OnPartyCntlScpNotificationFromBridgeReq)
    ONEVENT(SCP_NOTIFICATION_REQ_TOUT,						ANYCASE,			CSipPartyCntl::OnPartyCntlScpNotificationReqTout)
    ONEVENT(CONF_API_SCP_NOTIFICATION_ACK_FROM_EP,			ANYCASE,			CSipPartyCntl::OnConfApiScpNotificationAckFromEP)
    ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,					ANYCASE,			CSipPartyCntl::OnPartyCntlFirRequestByEP)
    ONEVENT(SCP_IVR_SHOW_SLIDE_REQ,							ANYCASE,			CSipPartyCntl::OnPartyCntlScpIvrShowSlideFromBridgeReqANYCASE)
    ONEVENT(SCP_IVR_STOP_SHOW_SLIDE_REQ,					ANYCASE,			CSipPartyCntl::OnPartyCntlScpIvrStopShowSlideFromBridgeReqANYCASE)
    ONEVENT(SCP_IVR_STATE_NOTIFICATION_REQ_TOUT,			ANYCASE,			CSipPartyCntl::OnPartyCntlScpIvrStateNotificationReqToutANYCASE)
    ONEVENT(SCP_PIPES_MAPPING_NOTIFICATION_REQ,				ANYCASE,			CSipPartyCntl::OnPartyCntlScpPipesMappingNotificationFromBridgeReq)
    ONEVENT(SCP_PIPES_MAPPING_NOTIFICATION_REQ_TOUT,		ANYCASE,			CSipPartyCntl::OnPartyCntlScpPipesMappingNotificationReqTout)
    ONEVENT(SIP_PARTY_CONF_PWD_STAUTS_ACK,				ANYCASE,			CSipPartyCntl::OnPartyAuthCompleate)
    ONEVENT(CHANGE_VIDEO_OUT_TIP_POLYCOM,					ANYCASE,			CSipPartyCntl::OnChangeVideoOutTipPolycom) //_t_p_
    ONEVENT(UPDATE_VIDBRDG_TELEPRESENSE_EP_INFO,            ANYCASE,            CSipPartyCntl::OnPartyCntlUpdateVidBrdgTelepresenseEPInfo) //_e_m_
	ONEVENT(UPDATE_ART_WITH_SSRC_REQ,                       ANYCASE,            CSipPartyCntl::OnPartyCntlUpdateArtWithSsrc)
    ONEVENT(UPDATE_REMOTE_CAPS_FROM_PARTY,		ANYCASE,						CSipPartyCntl::NullActionFunction)
    ONEVENT(DISCONNECT_ALL_SLAVES_FROM_MASTER_PARTY_CONTROL,			    ANYCASE,			CSipPartyCntl::SendDisconnectMessageFromMasterToSlaves)
    ONEVENT(PARTY_CONTROL_MS_SLAVE_TO_MAIN_ACK,				ANYCASE,			CSipPartyCntl::OnMsSlaveToMainAckMessage)
    ONEVENT(PARTY_CONTROL_ALL_MS_OUT_SLAVES_CONNECTED,		ANYCASE,			CSipPartyCntl::OnMsftOutSlavesCreated)
    ONEVENT(PARTY_CONTROL_ALL_MS_IN_SLAVES_CONNECTED,		ANYCASE,			CSipPartyCntl::OnMsftInSlavesCreated)
    ONEVENT(SIP_PARTY_SINGLE_VSR_MSG_IND,				    ANYCASE,			CSipPartyCntl::OnSlavesControllerSingleVsrMassageInd)
    //ONEVENT(SLAVES_CONTROLLER_MSG,				    		ANYCASE,			CSipPartyCntl::OnSlavesControllerMsg)
    ONEVENT(FULL_PACSI_INFO_IND,				    		ANYCASE,			CSipPartyCntl::OnSlavesControllerFullPacsiInfoInd)
//eFeatureRssDialin
     ONEVENT(SRS_RECORDING_CONTROL_ACK,			   ANYCASE, 		   CSipPartyCntl::OnPartyRecordingControlAck)
     ONEVENT(SRS_LAYOUT_CONTROL_PARTY_TO_CONF,			   ANYCASE, 		   CSipPartyCntl::OnPartyLayoutControl)

    ONEVENT(PARTY_PARTYCONTROL_MSSLAVE_TO_MAIN,			    ANYCASE,			CSipPartyCntl::PartyToPartyCntlMessageMSSlaveToMain)
	ONEVENT(PARTY_CONTROL_MS_SLAVE_TO_MAIN_MSG, 			ANYCASE,			CSipPartyCntl::OnMsSlaveToMainMsgMessage)
PEND_MESSAGE_MAP(CSipPartyCntl,CIpPartyCntl);

//////////////////////////////////////////////////
CSipPartyCntl::CSipPartyCntl()
{
	m_connectingState 					= IP_DISCONNECTED;
	m_pSipLocalCaps   					= NULL;
	m_pSipRemoteCaps  					= NULL;
	m_pSIPNetSetup						= NULL;
	m_eConfTypeForAdvancedVideoFeatures = kNotAnAdvancedVideoConference;
	m_strAlternativeAddr 				= "";
	m_bIsOfferer	  					= FALSE;
	m_IceParams 						= NULL;
	m_IsIceParty 						= FALSE;

	// TIP
	m_SlaveRightRsrcId 					= 0;
	m_SlaveLeftRsrcId  					= 0;
	m_SlaveAuxRsrcId   					= 0;
	m_TipMasterName 					= NULL;
	m_TipNumOfScreens					= 0;
	m_needToUpdateVisualName 			= TRUE;
	m_pScpHandler						= NULL;
	m_IsWaitForAckForFecUpdate			= FALSE;
	m_IsWaitForAckForRedUpdate          = FALSE;
	m_bIsLync							= FALSE;

	m_IsAsSipContentEnable				= FALSE;
	m_IsFirstContentNegotiation			= TRUE;
	m_bIsBlockContentForLegacy			=FALSE;

	m_TipSlaveRightAddSent				= FALSE;
	m_TipSlaveLeftAddSent				= FALSE;
	m_TipSlaveAuxAddSent				= FALSE;
	m_eMaxVideoPartyTypeForTipFailure	= eVideo_party_type_dummy;
	m_lyncRedialOutAttempt        = 0;
	m_partyContentRate 					= 0;
	m_MsConfReq 						= NULL;
	m_pMsOrganizerMngr				    = NULL;
	m_pMsFocusMngr 						= NULL;
	m_pMsEventPackageMngr				= NULL;

	m_FocusUri 							= NULL;
	m_isDMAAVMCU						= FALSE;


	m_pMsSlavesController 				= NULL;
	m_isWaitingForFullPacsi				= FALSE;
	m_maxResForAvMcu                    = eAuto_Res;

	m_EndSubscriber					= FALSE;
	m_EndFocus						= FALSE;
	m_remoteIdentity					=Regular;
	m_bIsAsSipContentEnable			= FALSE;
	VALIDATEMESSAGEMAP
}

//////////////////////////////////////////////////
CSipPartyCntl::~CSipPartyCntl()
{
    POBJDELETE(m_pSipLocalCaps);
    POBJDELETE(m_pSipRemoteCaps);
    POBJDELETE(m_pSIPNetSetup);
    POBJDELETE(m_IceParams);
    PDELETEA(m_TipMasterName);
    POBJDELETE(m_pScpHandler);
    POBJDELETE(m_pMsSlavesController);


 //   POBJDELETE(m_pMsOrganizerMngr);
 //   POBJDELETE(m_pMsFocusMngr);
 //   POBJDELETE(m_pMsEventPackageMngr);
    PDELETEA(m_FocusUri);
    POBJDELETE(m_MsConfReq);
}

//////////////////////////////////////////////////
void CSipPartyCntl::AllocMemory()
{
	m_pIpInitialMode	= new CSipComMode;
	m_pIpCurrentMode	= new CSipComMode;
	m_pSipLocalCaps		= new CSipCaps;
	m_pSipRemoteCaps	= new CSipCaps;
	m_pSIPNetSetup		= new CSipNetSetup;
	m_pScpHandler		= new CScpHandler;
}

//////////////////////////////////////////////////
void CSipPartyCntl::Destroy()
{
	POBJDELETE(m_pSipLocalCaps);
	POBJDELETE(m_pSipRemoteCaps);
	POBJDELETE(m_pSIPNetSetup);
	POBJDELETE(m_IceParams);
	PDELETEA(m_TipMasterName);
	POBJDELETE(m_pScpHandler);
    POBJDELETE(m_pMsSlavesController);

	CIpPartyCntl::Destroy();
}

//////////////////////////////////////////////////
CSipPartyCntl& CSipPartyCntl::operator=(const CSipPartyCntl& other)
{
	if (this != &other)
	{
		(CIpPartyCntl&)*this = (CIpPartyCntl&)other;

		m_connectingState	= other.m_connectingState;
		m_eConfTypeForAdvancedVideoFeatures = other.m_eConfTypeForAdvancedVideoFeatures;
		m_strAlternativeAddr	= other.m_strAlternativeAddr;
		m_bIsOfferer		= other.m_bIsOfferer;

		POBJDELETE(m_pSIPNetSetup);
		if (other.m_pSIPNetSetup == NULL)
		{
		  m_pSIPNetSetup = NULL;
		}
		else
		{
		  m_pSIPNetSetup = new CSipNetSetup;
		  *m_pSIPNetSetup = *(other.m_pSIPNetSetup);
	    }

		POBJDELETE(m_pSipLocalCaps);
		if (other.m_pSipLocalCaps == NULL)
		{
			m_pSipLocalCaps = NULL;
		}
		else
		{
			m_pSipLocalCaps = new CSipCaps(*(other.m_pSipLocalCaps));
		}

		POBJDELETE(m_pSipRemoteCaps);
		if (other.m_pSipRemoteCaps == NULL)
		{
			m_pSipRemoteCaps = NULL;
		}
		else
		{
			m_pSipRemoteCaps = new CSipCaps(*(other.m_pSipRemoteCaps));
		}

		POBJDELETE(m_IceParams);
		if (other.m_IceParams == NULL)
		{
			m_IceParams = NULL;
		}
		else
		{
			m_IceParams = new CIceParams;
			*m_IceParams = *(other.m_IceParams);
	    }

		m_IsIceParty        = other.m_IsIceParty;
		// m_udpAddresses      = other.m_udpAddresses;
		memcpy(&m_udpAddresses, &(other.m_udpAddresses), sizeof(UdpAddresses));

		m_bIsTipCall		= other.m_bIsTipCall;
		m_TipPartyType   	= other.m_TipPartyType;
		m_MasterRsrcId   		= other.m_MasterRsrcId;
		m_SlaveRightRsrcId 		= other.m_SlaveRightRsrcId;
		m_SlaveLeftRsrcId  		= other.m_SlaveLeftRsrcId;
		m_SlaveAuxRsrcId   		= other.m_SlaveAuxRsrcId;

		m_AvMcuLinkType        = other.m_AvMcuLinkType;

		m_MSSlaveIndex			= other.m_MSSlaveIndex;
		m_MSaudioLocalMsi		= other.m_MSaudioLocalMsi;
		m_TipSlaveRightAddSent	= other.m_TipSlaveRightAddSent;
		m_TipSlaveLeftAddSent	= other.m_TipSlaveLeftAddSent;
		m_TipSlaveAuxAddSent	= other.m_TipSlaveAuxAddSent;
		m_MasterRsrcId          = other.m_MasterRsrcId;
		m_eMaxVideoPartyTypeForTipFailure = other.m_eMaxVideoPartyTypeForTipFailure;
		m_bIsAsSipContentEnable	= other.m_bIsAsSipContentEnable;

		if (other.m_TipMasterName != NULL)
			SetTipMasterName(other.m_TipMasterName);

		m_TipNumOfScreens = other.m_TipNumOfScreens;
		m_needToUpdateVisualName = other.m_needToUpdateVisualName;
		m_IsWaitForAckForFecUpdate = other.m_IsWaitForAckForFecUpdate;
		m_IsWaitForAckForRedUpdate = other.m_IsWaitForAckForRedUpdate;
		m_bIsLync				= other.m_bIsLync;
		m_bIsBlockContentForLegacy	= other.m_bIsBlockContentForLegacy;
		m_partyContentRate = other.m_partyContentRate;
		m_maxResForAvMcu   = other.m_maxResForAvMcu;
		m_isMsftEnv 			= other.m_isMsftEnv;

		m_isWaitingForFullPacsi = other.m_isWaitingForFullPacsi;
		POBJDELETE(m_pScpHandler);
		if (other.m_pScpHandler == NULL)
		{
			m_pScpHandler = NULL;
		}
		else
		{
			m_pScpHandler = new CScpHandler(*(other.m_pScpHandler));
		}

		m_lyncRedialOutAttempt = other.m_lyncRedialOutAttempt;
		POBJDELETE(m_pMsSlavesController);
		if (other.m_pMsSlavesController == NULL)
		{
			m_pMsSlavesController = NULL;
		}
		else
		{
			m_pMsSlavesController = new CMsSlavesController(*(other.m_pMsSlavesController));
		}

		POBJDELETE(m_pMsOrganizerMngr);
		if (other.m_pMsOrganizerMngr == NULL)
		{
			m_pMsOrganizerMngr = NULL;
		}
		else
		{
			m_pMsOrganizerMngr = other.m_pMsOrganizerMngr;
		//	m_pMsOrganizerMngr = new CMSOrganizerMngr(*(other.m_pMsOrganizerMngr));
		}

		POBJDELETE(m_pMsFocusMngr);
		if (other.m_pMsFocusMngr == NULL)
		{
				m_pMsFocusMngr = NULL;
		}
		else
		{
			m_pMsFocusMngr = other.m_pMsFocusMngr;
			//	m_pMsFocusMngr = new CMSFocusMngr(*(other.m_pMsFocusMngr));
		}

		POBJDELETE(m_pMsEventPackageMngr);
		if (other.m_pMsEventPackageMngr == NULL)
		{
			  m_pMsEventPackageMngr = NULL;
		}
		else
		{
			m_pMsEventPackageMngr = other.m_pMsEventPackageMngr;
			//m_pMsEventPackageMngr = new CMSSubscriberMngr(*(other.m_pMsEventPackageMngr));
		}

		m_isDMAAVMCU = other.m_isDMAAVMCU;

		m_EndSubscriber					= other.m_EndSubscriber;
		m_EndFocus						= other.m_EndFocus;
		m_remoteIdentity = other.m_remoteIdentity;

	}

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
void  CSipPartyCntl::SetTipMasterName(char* name)
{
  if ( ! m_TipMasterName ) m_TipMasterName = new char[H243_NAME_LEN];
  strncpy(m_TipMasterName,name,H243_NAME_LEN-1);
  m_TipMasterName[H243_NAME_LEN-1]='\0';
}

//////////////////////////////////////////////////////////
void CSipPartyCntl::SetUdpAddress(UdpAddresses UdpAdd)
{
	memcpy(&m_udpAddresses, &UdpAdd, sizeof(UdpAddresses));
}

//////////////////////////////////////////////////
void CSipPartyCntl::OnPartyUpdateBridges(CSegment* pParam)
{
	// we receive update bridges request from the party
	// Currently happened when the RTP inform on payload different than the one it is set to receive.
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateBridges: Name - ",m_partyConfName, GetPartyRsrcId());
	CSipComMode* pRemoteMode = new CSipComMode;
	pRemoteMode->DeSerialize(NATIVE, *pParam);

	cmCapDataType mediaType = cmCapEmpty;
	cmCapDirection mediaDirection = cmCapReceiveAndTransmit;
	*pParam >> (WORD&)mediaType;
	*pParam >> (WORD&)mediaDirection;
	if (mediaType == cmCapEmpty)
		*m_pIpInitialMode = *pRemoteMode;
	else // copy only specific media
		m_pIpInitialMode->CopyMediaMode(*pRemoteMode, mediaType, mediaDirection);

	int updateBridges = eNoUpdate;
	BYTE bTakeInitial = TRUE;

	updateBridges = UpdateAudioAndVideoBridgesIfNeeded(bTakeInitial);

	if((updateBridges & eUpdateVideoIn) || (updateBridges & eUpdateVideoOut))
	{
		// audio bridge has no response on update, wait for video bridge response
		PTRACEPARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateBridges: (change the video bridge mode)", GetPartyRsrcId());
	}
	else if((updateBridges & eUpdateAudioIn) || (updateBridges & eUpdateAudioOut))
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateBridges, audio bridge was updated. Set current mode", GetPartyRsrcId());
		*m_pIpCurrentMode = *m_pIpInitialMode;
	}

	POBJDELETE(pRemoteMode);
}

////////////////////////////////////////////////////////////
void CSipPartyCntl::OnVideoBrdgConnectedIdleOrAnycase(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnVideoBrdgConnectedIdleOrAnycase: (Do Nothing) Name - ",m_partyConfName, GetPartyRsrcId());

	BYTE bRes = FALSE;

	bRes = CheckIfNeedToSendIntra();

	//In case this is MOC or Lync we will send Intra and Video preference again.
	if(bRes)
	{
		SendIntraToParty();
		SendVideoPreferenceToParty();
	}

}


/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnVideoBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPartyCntl::OnVideoBrdgDisconnected", GetPartyRsrcId());
	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);

	if (resStat == statVideoInOutResourceProblem ||
		resStat == statInvalidPartyInitParams) //BRIDGE-14724
	{
		BYTE 	mipHwConn = (BYTE)eMipBridge;
		BYTE	mipMedia = (BYTE)eMipVideo;
		BYTE	mipDirect = 0;
		BYTE	mipTimerStat = 0;
		BYTE	mipAction = 0;
		*pParam >>  mipDirect >> mipTimerStat >> mipAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;

        // fix bug vngr-4256 : pParam was sent to GetMpiErrorNumber instesd of pSeg caused CSegment asserts
		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty,NULL,MpiErrorNumber);
		POBJDELETE(pSeg);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Audio brdg disconnection indication
void CSipPartyCntl::OnAudBrdgDisconnect(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnAudBrdgDisconnect : Name - ",m_partyConfName, GetPartyRsrcId());
	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam);
	if (bIsDisconnectOk == FALSE)
		m_pTaskApi->EndAddParty(m_pParty, statAudioInOutResourceProblem);
}

/////////////////////////////////////////////////////////////////////////////
//VSGNINJA-589
void CSipPartyCntl::OnVideoInBrdgUpdatedPartyReCaps(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnVideoInBrdgUpdatedPartyReCaps - Do Nothing: Name - ",m_partyConfName, GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnVideoInBrdgUpdatedAnycase(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnVideoInBrdgUpdatedAnycase: Name - ",m_partyConfName, GetPartyRsrcId());
	WORD status;
	*pParam >> status;

	*m_pIpCurrentMode = *m_pIpInitialMode;
	if (status)
		PASSERT(status);
}

/////////////////////////////////////////////////////////////////////////////
//VSGNINJA-589
void CSipPartyCntl::OnVideoOutBrdgUpdatedPartyReCaps(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnVideoOutBrdgUpdatedPartyReCaps - Do Nothing: Name - ",m_partyConfName, GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnVideoOutBrdgUpdatedAnycase(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnVideoOutBrdgUpdatedAnycase: Name - ",m_partyConfName, GetPartyRsrcId());
	WORD status;
	BOOL isParams;
	*pParam >> status >> isParams;
	if (status == statOK && isParams)
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnVideoOutBrdgUpdatedAnycase - Handle LPR");
		HandleLprUpdatedIndications(pParam);
		return;
	}
	else if (status == statCCSContentLprSupport  && isParams)
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnVideoOutBrdgUpdatedAnycase - Handle CCS Content LPR");
		HandleLprUpdatedIndications(pParam);
		return;
	}
	else if(!isParams && (m_IsWaitForAckForFecUpdate || m_IsWaitForAckForRedUpdate) && status == statOK)
	{
		if (m_IsWaitForAckForFecUpdate)
		{
			TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - UPDATE_CURRENTMODE - PartyControl got ack from VB regarding FEC change";
			HandleFECUpdatedInd(status);
		}
		if (m_IsWaitForAckForRedUpdate)
		{
			TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - UPDATE_CURRENTMODE - PartyControl got ack from VB regarding RED change";
			HandleREDUpdatedInd(status);
		}
		*m_pIpCurrentMode = *m_pIpInitialMode;
		return;
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnVideoOutBrdgUpdatedAnycase - just copy: Name - ");
		*m_pIpCurrentMode = *m_pIpInitialMode;
	}
	if (status)
		PASSERT(status);
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnVideoInBrdgUpdated(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnVideoInBrdgUpdated: Name - ",m_partyConfName, GetPartyRsrcId());
//    if(m_bIsMrcCall && m_bVideoUpdateCount<VIDEO_UPDATE_COUNT2)
//    {
//        m_bVideoUpdateCount++;
//        TRACEINTO<<"!@# updating m_bVideoUpdateCount to: "<<(int)m_bVideoUpdateCount;
//    }
	WORD status;
	*pParam >> status;

	HandleVideoBridgeUpdate(status, eUpdateVideoIn);
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnVideoOutBrdgUpdated(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnVideoOutBrdgUpdated: Name - ",m_partyConfName, GetPartyRsrcId());
	WORD status;
	*pParam >> status;

	HandleVideoBridgeUpdate(status, eUpdateVideoOut);
}
/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnVideoOutBrdgUpdatedChannelHandle(CSegment* pParam)
{
	WORD status;
	*pParam >> status;
	TRACEINTO << "status:" << status;
	HandleVideoBridgeUpdateForChannelHandle(status, eUpdateVideoOut);
}
/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnVideoInBrdgUpdatedChannelHandle(CSegment* pParam)
{
	WORD status;
	*pParam >> status;
	TRACEINTO << "status:" << status;
	HandleVideoBridgeUpdateForChannelHandle(status, eUpdateVideoIn);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::UpdateBridgesForChannelHandle()
{
    TRACEINTO << "partyId=" << GetPartyRsrcId();

	if (m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
	{
		BYTE bTakeInitial = TRUE;
		m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial);

		if(m_eUpdateState != eNoUpdate)
		{
			TRACEINTO << "Update bridges for channel handle" << " partyId=" << GetPartyRsrcId();
			m_state = UPDATE_BRIDGES_FOR_CHANNEL_HANDLE;
		}

	}
	else if (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc)
    {
        BYTE bTakeInitial = TRUE;
        if (!m_bIsBridgeUpgradedVideo && UpgradeVideoBridgesIfNeeded())
        {
            m_bIsBridgeUpgradedVideo = true;
            int updateState = eUpdateVideoIn + m_eUpdateState;
            m_eUpdateState = (EUpdateBridgeMediaAndDirection)updateState;
            m_state = UPDATE_BRIDGES_FOR_CHANNEL_HANDLE;
            TRACEINTO << "Upgrade bridges for channel handle" << " partyId=" << GetPartyRsrcId() << " m_eUpdateState=" << m_eUpdateState;
            return true;
        }
        return false;
    }
	else
	{
        TRACEINTO << "No update bridges for channel handle" << " partyId=" << GetPartyRsrcId();
	    return false;
	}

    TRACEINTO << "@#@ m_eUpdateState=" << m_eUpdateState << " partyId=" << GetPartyRsrcId();

    return (m_eUpdateState != eNoUpdate);
}

//////////////////////////////////////////////////
BYTE CSipPartyCntl::IsRemoteCapNotHaveVideo() const
{
	BYTE bRes = TRUE;
	int numOfMedia = m_pSipRemoteCaps->GetNumOfMediaCapSets(cmCapVideo);
	if (numOfMedia > 0)
		bRes = FALSE;
	return bRes;
}

//////////////////////////////////////////////////
BOOL CSipPartyCntl::IsDisconnectionBecauseOfNetworkProblems() const
{
	PTRACE2INT(eLevelInfoNormal,"IsDisconnectionBecauseOfNetworkProblems ",m_disconnectionCause);
	BYTE res = NO;
	switch (m_disconnectionCause)
	{
	case SIP_BUSY_HERE:
	case SIP_REQUEST_TIMEOUT:
		res = YES;
	}
	return res;
}

//////////////////////////////////////////////////
BOOL CSipPartyCntl::IsRedialImmediately()
{
	BYTE res = NO;
	if(m_lyncRedialOutAttempt==1)
	{
		TRACEINTO << "LYNC_REDIAL redial immediately on first redial attempt";
		res = YES;
	}else if(m_lyncRedialOutAttempt!=0){
		TRACEINTO << "LYNC_REDIAL redial immediately return NO, if not first redial attempt (Redial from EMA), m_lyncRedialOutAttempt = " << m_lyncRedialOutAttempt;
	}
	switch (m_disconnectionCause)
	{
//	case SIP_TRANS_ERROR_TCP_INVITE:
	case SIP_REDIRECTION_300:
	case SIP_MOVED_PERMANENTLY:
	case SIP_MOVED_TEMPORARILY:
		res = YES;
	}

	return res;
}

//////////////////////////////////////////////////
DWORD CSipPartyCntl::GetRemoteCapsVideoRate(CapEnum protocol) const
{
	DWORD rate = 0;
	CComModeInfo comModeInfo(protocol, StartVideoCap);
	CapEnum h323CapCode = comModeInfo.GetH323ModeType();
	rate = m_pSipRemoteCaps->GetMaxVideoBitRate(h323CapCode);
	return rate * 100;
}

//////////////////////////////////////////////////
WORD CSipPartyCntl::IsSupportErrorCompensation()
{
	BYTE bRes = NO;
	if (m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive))
		bRes = m_pIpCurrentMode->IsSupportErrorCompensation(cmCapReceive);
	return bRes;
}

//////////////////////////////////////////////////
void CSipPartyCntl::SetDataForImportPartyCntl(CPartyCntl* apOtherPartyCntl)
{
    CIpPartyCntl::SetDataForImportPartyCntl(apOtherPartyCntl);

	CSipPartyCntl* pOtherPartyCntl = (CSipPartyCntl*)apOtherPartyCntl;

	POBJDELETE(m_pSipRemoteCaps);
	if (pOtherPartyCntl->m_pSipRemoteCaps == NULL)
	{
		m_pSipRemoteCaps = NULL;
	}
	else
	{
		m_pSipRemoteCaps = new CSipCaps(*(pOtherPartyCntl->m_pSipRemoteCaps));
	}

	if (pOtherPartyCntl->m_pSIPNetSetup == NULL)
	{
	    m_pSIPNetSetup = NULL;
	}
	else
	{
        POBJDELETE(m_pSIPNetSetup);
        m_pSIPNetSetup = new CSipNetSetup;
        *m_pSIPNetSetup = *(pOtherPartyCntl->m_pSIPNetSetup);
    }

	if (pOtherPartyCntl->m_pSipLocalCaps == NULL)
	{
		m_pSipLocalCaps = NULL;
	}
	else
	{
		m_pSipLocalCaps = new CSipCaps(*(pOtherPartyCntl->m_pSipLocalCaps));
	}

	POBJDELETE(m_pScpHandler);
	if (pOtherPartyCntl->m_pScpHandler == NULL)
	{
		m_pScpHandler = NULL;
	}
	else
	{
		m_pScpHandler = new CScpHandler(*(pOtherPartyCntl->m_pScpHandler));
		m_pScpHandler->Init(m_pPartyApi);
	}

	m_bIsTipCall = pOtherPartyCntl->m_bIsTipCall;
	m_TipPartyType = pOtherPartyCntl->m_TipPartyType;
	m_MasterRsrcId = pOtherPartyCntl->m_MasterRsrcId;
	m_SlaveRightRsrcId = pOtherPartyCntl->m_SlaveRightRsrcId;
	m_SlaveLeftRsrcId = pOtherPartyCntl->m_SlaveLeftRsrcId;
	m_SlaveAuxRsrcId = pOtherPartyCntl->m_SlaveAuxRsrcId;
	m_TipSlaveRightAddSent	= pOtherPartyCntl->m_TipSlaveRightAddSent;
	m_TipSlaveLeftAddSent	= pOtherPartyCntl->m_TipSlaveLeftAddSent;
	m_eMaxVideoPartyTypeForTipFailure = pOtherPartyCntl->m_eMaxVideoPartyTypeForTipFailure;
	m_TipSlaveAuxAddSent	= pOtherPartyCntl->m_TipSlaveAuxAddSent;
	m_AvMcuLinkType        = pOtherPartyCntl->m_AvMcuLinkType;
	m_MasterRsrcId         = pOtherPartyCntl->m_MasterRsrcId;
	m_MSSlaveIndex			= pOtherPartyCntl->m_MSSlaveIndex;
	m_bIsAsSipContentEnable = pOtherPartyCntl->m_bIsAsSipContentEnable;

	if (pOtherPartyCntl->m_TipMasterName != NULL)
		SetTipMasterName(pOtherPartyCntl->m_TipMasterName);

	m_TipNumOfScreens = pOtherPartyCntl->m_TipNumOfScreens;
	m_connectingState = pOtherPartyCntl->m_connectingState;
	m_incomingVideoChannelHandle = pOtherPartyCntl->m_incomingVideoChannelHandle;
	m_outgoingVideoChannelHandle = pOtherPartyCntl->m_outgoingVideoChannelHandle;
	m_isSignalingFirstTransactionCompleted = pOtherPartyCntl->m_isSignalingFirstTransactionCompleted;
	m_isMsftEnv 			= pOtherPartyCntl->m_isMsftEnv;
	m_lyncRedialOutAttempt = pOtherPartyCntl->m_lyncRedialOutAttempt;
	m_partyContentRate = pOtherPartyCntl->m_partyContentRate;
	m_maxResForAvMcu  = pOtherPartyCntl->m_maxResForAvMcu;
}


//////////////////////////////////////////////////
void  CSipPartyCntl::OnPartyMoveToSecondary(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyMoveToSecondary : Name - ",m_partyConfName, GetPartyRsrcId());
	WORD reason;
	CSecondaryParams secParams;
	*pParam >> reason;
	secParams.DeSerialize(NATIVE,*pParam);

	SetPartyToSecondaryAndStopChangeMode((BYTE) reason, 0, cmCapTransmit, &secParams, FALSE); //without disconnect the channels, since they are already disconnected
}


//////////////////////////////////////////////////
void  CSipPartyCntl::ImplementSecondaryInPartyLevel()
{
	CSipComMode* pTemp = new CSipComMode;
	*pTemp = *((CSipComMode*)m_pIpCurrentMode);
	pTemp->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit);
	pTemp->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit,kRoleContent);
	m_pPartyApi->SipConfDisconnectChannels(pTemp);
	POBJDELETE(pTemp);
}

//////////////////////////////////////////////////
void  CSipPartyCntl::ImplementUpdateSecondaryInPartyControlLevel()
{
	m_pIpCurrentMode->SetMediaOff(cmCapVideo,cmCapTransmit,kRolePeople); //only transmit is closed!!!
	UpdateCurrentModeInDB();
}

//////////////////////////////////////////////////
CIpComMode*	CSipPartyCntl::GetScmForVideoBridgeConnection(cmCapDirection direction)
{//in SIP: we always connect to video bridge according to initial mode
	return m_pIpInitialMode;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsRemoteAndLocalCapSetHasContent(eToPrint toPrint)const
{
	bool res = false;
	std::ostringstream msg;
	if (m_pSipLocalCaps && m_pSipRemoteCaps)
	{

		int nLocalCaps = m_pSipLocalCaps->GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit, kRolePresentation);
		int nRmtCaps = m_pSipRemoteCaps->GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit, kRolePresentation);


		msg << "\nlocal media caps:"<<(int)nLocalCaps<<", nRmtCaps:" << nRmtCaps << ", m_pSipLocalCaps->IsBfcpSupported:"<<(int)m_pSipLocalCaps->IsBfcpSupported() << "\n";
		msg << "m_pSipRemoteCaps->IsBfcpSupported:"<<(int)m_pSipRemoteCaps->IsBfcpSupported()<<", m_pIpInitialMode->IsMediaOn(cmCapBfcp):"<<(int)m_pIpInitialMode->IsMediaOn(cmCapBfcp) << "\n";

		if ((m_pSipLocalCaps->GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit, kRolePresentation) > 0 )
				&& (m_pSipRemoteCaps->GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit, kRolePresentation) > 0 )
				&& m_pSipLocalCaps->IsBfcpSupported()
				&& m_pSipRemoteCaps->IsBfcpSupported()
				&& m_pIpInitialMode
				&& m_pIpInitialMode->IsMediaOn(cmCapBfcp)
				&& ( m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) || (!m_pIpCurrentMode->IsMediaOn(cmCapBfcp)) ) )
			res = true;
		else if (GetIsTipCall() && m_pSipLocalCaps->IsCapableTipAux5Fps() && m_pSipRemoteCaps->IsCapableTipAux5Fps())
			res = true;

		msg << "pSipRemoteCaps->IsCapableTipAux5Fps:"<<(int)m_pSipRemoteCaps->IsCapableTipAux5Fps() << ", m_pIpCurrentMode->IsMediaOn(cmCapBfcp):"
				  << (int)m_pIpCurrentMode->IsMediaOn(cmCapBfcp) << ", m_pIpInitialMode->IsMediaOn(kRoleContentOrPresentation):" << (int)m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);


	}

	if(eToPrintYes==toPrint || (false==res && eToPrintOnFalseOnly==eToPrintOnFalseOnly))
		TRACEINTO << "return " << (res?"true":"false") << ", partyName: " << m_partyConfName << ", partyId: " << GetPartyRsrcId() << msg.str().c_str();

	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsRemoteAndLocalCapSetHasBfcpUdp()const
{
	if (m_pSipLocalCaps && m_pSipRemoteCaps)
	{
		enTransportType remoteBfcpTransType = m_pSipRemoteCaps->GetBfcpTransportType();
		enTransportType LocalBfcpTransType = m_pSipLocalCaps->GetBfcpTransportType();

		if( remoteBfcpTransType == eTransportTypeUdp && LocalBfcpTransType == eTransportTypeUdp)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsLegacyContentParty()
{
    CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
   	PASSERTMSG_AND_RETURN_VALUE(!pCommConf , "pCommConf is NULL", false);

   	//if (pCommConf->GetConfMediaType() == eSvcOnly || pCommConf->GetConfMediaType() == eMixAvcSvc || pCommConf->GetConfMediaType() == eMixAvcSvcVsw)
	/*if (pCommConf->GetConfMediaType() == eSvcOnly) 
	{
    	PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : soft Mcu - temporary no legacy ");
    	return FALSE;
    }*/

	//BOOL bIsBlock_Content_Legacy_For_Lync = FALSE;
	//string key1 = BLOCK_CONTENT_LEGACY_FOR_LYNC;
	//CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	//sysConfig->GetBOOLDataByKey(key1, bIsBlock_Content_Legacy_For_Lync);  // the default value is "NO"
	if (m_bIsLync && (TRUE == GetIsBlockContentForLegacy()))
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : lync - block content for Lync!! ");
		return FALSE;
	}
	
	if(Microsoft_AV_MCU == GetRemoteIdentity() || Microsoft_AV_MCU2013 == GetRemoteIdentity())
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : AV-MCU - block content for AV-MCU!! ");
		return FALSE;
	}

	BYTE isLegacy = !IsRemoteAndLocalCapSetHasContent(eToPrintOnFalseOnly);
	if (isLegacy)
	{
		  PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : No Remote or Local Content Caps");

	}

	if (IsTipSlavePartyType())
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : no legacy for TIP Slave");
		isLegacy = false;
	}
	else if (m_pIpInitialMode && m_pIpInitialMode->IsTipNegotiated()) // eTipCompatibleVideoOnly
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : no legacy for TIP call");
		isLegacy = false;
	}
	else
	{
		if (pCommConf && (pCommConf->GetIsTipCompatible() == eTipCompatibleVideoAndContent || pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP) )
		{
			DWORD partyContentRate = GetMinContentPartyRate()/100;
			isLegacy = IsPartyLegacyForTipContent(partyContentRate) ;
			TRACEINTO<<"IsPartyLegacyForTipContent= "<< isLegacy;
			return isLegacy ;
		}

		BYTE presentationProtocol = pCommConf->GetPresentationProtocol();
		BYTE doesPartyMeetConfContentHDResolution = TRUE;

		// Content protocol h264 only
		BOOL bForceH264 = 0;
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
		}

		if(bForceH264 && !isLegacy)
		{
		    // if we have h239 caps and H264 content protocol
			// trashold for H264 content protocol is HD720
			BYTE isHD720content = IsRemoteAndLocalHasHDContent720();
			DWORD possibleContentRate = GetMinContentPartyRate()/100;
		    if(presentationProtocol == eH264Fix)
		    {
		    	doesPartyMeetConfContentHDResolution = m_pConf->DoesPartyMeetConfContentHDResolution(m_name);
		    }
		    if(presentationProtocol == eH264Dynamic)
		    {
		    	doesPartyMeetConfContentHDResolution = isHD720content;
		    }
			DWORD confContentRate = m_pSipLocalCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation);
			eEnterpriseMode ContRatelevel = (eEnterpriseMode)pCommConf->GetEnterpriseMode();
			BYTE lConfRate = pCommConf->GetConfTransferRate();
			CUnifiedComMode localUnifedCommMode(pCommConf->GetEnterpriseModeFixedRate(),lConfRate,pCommConf->GetIsHighProfileContent());
			DWORD actualPartyContentRate  = 0;
			DWORD confIPContentRate = localUnifedCommMode.GetContentModeAMCInIPRate(lConfRate,ContRatelevel,
															(ePresentationProtocol)presentationProtocol,
															pCommConf->GetCascadeOptimizeResolution(),
															pCommConf->GetConfMediaType());
			actualPartyContentRate  = min(confContentRate,possibleContentRate);

			DWORD contentThresholdRate = 0; //since we are in an urgent FE, we use the is function instead of a get function
			BOOL isPartyMeetContentRateThreshold = ::isPartyMeetContentRateThreshold(confIPContentRate/10,actualPartyContentRate/10,pCommConf->GetEnterpriseMode(),pCommConf->GetPresentationProtocol(), contentThresholdRate);
			if (!doesPartyMeetConfContentHDResolution || !isPartyMeetContentRateThreshold)
			{
			  CSmallString sstr;
			  sstr << "isHD720content = " << isHD720content << " , possibleContentRate = " << possibleContentRate;
			  PTRACE2(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : in EP failed thrashold in eH264Fix/ , ",sstr.GetString());
		      isLegacy = (!doesPartyMeetConfContentHDResolution)?eContentSecondaryCauseBelowResolution:eContentSecondaryCauseBelowRate;
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : not legacy: possibleContentRate=", possibleContentRate);
			}
		}

	}

	//============================================================
	// VNGR-23965 - Lowering log strain, logging only for legacy
	//============================================================
	if (isLegacy) PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::IsLegacyContentParty : isLegacy=", isLegacy);

	return isLegacy;
}

//isLegacy////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPartyCntl::GetPossibleContentRate() const
{
	DWORD  ContentRate = 0;
	if (GetIsTipCall())
	{
		if (m_pSipRemoteCaps->IsCapableTipAux5Fps())
		{
			BYTE units = 10;
			ContentRate = 512 * units;
		}
	}
	else
	{
		ContentRate = m_pSipRemoteCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation);
		DWORD videoRate = m_pSipRemoteCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePeople);
		if (ContentRate >= videoRate)
		{
			ContentRate = (videoRate > 640) ? videoRate - 640 : 320;
		}
	}

	return ContentRate;
}
/////////////////////////////////////////
/*
 function name: CSipPartyCntl::GetMinContentPartyRateAMC
 this functions returns the minimum content rate between the party allowed content rate
 resulting from call rate and the remote content rate

 */
//////////////////////////////////////////////////////////////////////////
DWORD CSipPartyCntl::GetMinContentPartyRate(DWORD currContentRate)
{
	DWORD MinContentRate = 0;

	// tmp - Eitan:
	DWORD audioVidRate = m_pSipRemoteCaps->GetTotalRate() * 100;
	DWORD remoteRate = GetPossibleContentRate() * 100;//return remote content rate

	if (currContentRate)
	{
		audioVidRate += min(remoteRate,currContentRate);
	}

	TRACEINTO << "N.A. DEBUG CSipPartyCntl::GetMinContentPartyRate currContentRate: " << currContentRate;

	DWORD confRate = GetConfRate();
	DWORD callRate = min(audioVidRate,confRate);

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);

	CUnifiedComMode unifiedComMode(pCommConf->GetEnterpriseModeFixedRate(),pCommConf->GetConfTransferRate(),pCommConf->GetIsHighProfileContent());
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)pCommConf->GetEnterpriseMode();
	ePresentationProtocol presentaionPrtocol =  (ePresentationProtocol)pCommConf->GetPresentationProtocol();

	if (pCommConf->GetIsTipCompatibleContent()) //BRIDGE-14495
	{
		//TRACEINTO << "Tip content enabled  - for content rate calculation use dynamic h264 rate tables";
		TRACEINTO << "Tip video and content enabled  - for content rate calculation use dynamic h264 rate tables";
		presentaionPrtocol = eH264Dynamic;
	}

	DWORD possibleConfContentRate = unifiedComMode.FindIpContentRateByLevel(callRate,ContRatelevel, presentaionPrtocol,
																			pCommConf->GetCascadeOptimizeResolution(), pCommConf->GetConfMediaType());

	TRACEINTO << "N.A. DEBUG CSipPartyCntl::GetMinContentPartyRate possibleConfContentRate: " << possibleConfContentRate << " remoteRate " << remoteRate;
	MinContentRate = min(possibleConfContentRate,remoteRate);

	TRACEINTO << "MinContentRate " << MinContentRate;

	//BRIDGE-13154
	if( callRate < 768000 &&
		pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP &&
		GetBOOLDataByKey(CFG_KEY_ENABLE_CONTENT_IN_PREFER_TIP_FOR_CALL_RATES_LOWER_THAN_768K)
	  )
	{
		TRACEINTO << "ENABLE_CONTENT_IN_PREFER_TIP_FOR_CALL_RATES_LOWER_THAN_768K is YES setting minimum content rate to 512k (tip content rate)";
		MinContentRate = CUnifiedComMode::TranslateAMCRateIPRate(AMC_512k)*100;
	}

	//===========================================================================
	// VNGR-23965 - Lowering log strain, logging only for when min rate changes
	//===========================================================================
	if (m_lastMinContentRate != MinContentRate)
	{
		CLargeString cstr;
		cstr << "remote Content Rate: " << remoteRate << ", conf Content Rate: " << currContentRate  << "\n";
		cstr << "audioVidRate: " << audioVidRate << ", confRate: " << confRate << " , selected call rate is: " << callRate << "\n";
		cstr << "possible content rate in conf (according to party's call rate) is: " << possibleConfContentRate << "\n";
		cstr << "chosen rate is: " << MinContentRate;
		PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::GetMinContentPartyRate - \n", cstr.GetString(), GetPartyRsrcId());
		m_lastMinContentRate = MinContentRate;
	}

	return MinContentRate;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsCapableOfVideo()
{
	return m_pSipLocalCaps->IsMedia(cmCapVideo);
}

/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsCapableOfHD720()
{
	return m_pSipLocalCaps->IsCapableOfHD720();
}

//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsCapableOfHD1080()
{
	return m_pSipLocalCaps->IsCapableOfHD1080();
}
//////////////////////////////////////////////////////////////////////////////////////////////
 BYTE CSipPartyCntl::IsRemoteAndLocalHasHDContent1080() const
{
	BYTE HD1080Mpi = 0;
    if (m_pSipRemoteCaps && m_pSipLocalCaps)
    {
		if (m_pSipRemoteCaps->IsH239() &&   m_pSipLocalCaps->IsH239())
		{
		  HD1080Mpi = m_pSipRemoteCaps->IsCapableOfHDContent1080();
		}
    }
    return HD1080Mpi;
}
//////////////////////////////////////////////////////////////////////////////////////////////
 BYTE CSipPartyCntl::IsRemoteAndLocalHasHDContent720() const
{
	BYTE HD720Mpi = 0;
    if (m_pSipRemoteCaps && m_pSipLocalCaps)
    {
	   if (m_pSipRemoteCaps->IsH239() &&   m_pSipLocalCaps->IsH239())
	   {
		  HD720Mpi = m_pSipRemoteCaps->IsCapableOfHDContent720();
	   }
    }
    return HD720Mpi;
}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsRemoteAndLocalHasHighProfileContent() const
{
	BYTE bIsHighProfileContent = FALSE;
	if (m_pSipRemoteCaps && m_pSipLocalCaps)
	{
		if (m_pSipRemoteCaps->IsH239() &&   m_pSipLocalCaps->IsH239())
		{
			bIsHighProfileContent = m_pSipRemoteCaps->IsHighProfileContent();
		}

	}
	return bIsHighProfileContent;
}
//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsCapableOf4CIF()
{
	return m_pSipLocalCaps->IsCapableOf4CIF();
}

//////////////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CSipPartyCntl::GetCPVideoPartyTypeAccordingToCapabilities()
{
	return m_pSipLocalCaps->GetCPVideoPartyType();
}

////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CSipPartyCntl::GetVideoPartyTypeForRtvBframe()
{
	bool is_rtv = m_pSipLocalCaps->IsCapSet(eRtvCapCode);
	// for AVMCU always turn the flag
	if (is_rtv && IsRtvBframeEnabled(true))
	{
		CBaseCap* pCap = m_pSipLocalCaps->GetCapSet(eRtvCapCode,0,kRolePeople);
		if (pCap)
		{
		    eVideoPartyType rtvVideoPartyType = ((CRtvVideoCap*)pCap)->GetCPVideoPartyType();
		    rtvVideoPartyType = GetVideoPartyTypeAllocationForRtvBframe(rtvVideoPartyType);
		    POBJDELETE(pCap);
		    return rtvVideoPartyType;
		}
		else
		    PTRACE(eLevelInfoNormal,"CSipPartyCntl::GetVideoPartyTypeForRtvBframe - pCap is NULL");
	}

	return eVideo_party_type_dummy;

}
//////////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPartyCntl::GetConfRate() const
{
	return m_pSIPNetSetup->GetMaxRate();
}

//////////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPartyCntl::GetSetupRate()
{
	return m_pSIPNetSetup->GetRemoteSetupRate();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::UpdateH264ModeInLocalCaps(H264VideoModeDetails h264VidModeDetails,ERoleLabel eRole)
{
	m_pSipLocalCaps->SetLevelAndAdditionals(h264VidModeDetails, kRolePeople);

}
//////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::UpdateRtvModeInLocalCaps(RTVVideoModeDetails rtvVidModeDetails,ERoleLabel eRole,DWORD videoRate)
{
	m_pSipLocalCaps->SetRtvParams(rtvVidModeDetails, kRolePeople,videoRate);

}
//////////////////////////////////////////////////////////////////////////////////////////////
APIS32 CSipPartyCntl::GetLocalCapsMbps(ERoleLabel eRole)
{
	WORD mbps = 0, fs = 0, dpb = 0, brAndCpb = 0, sar = 0, staticMB = 0;
	if(m_pSipLocalCaps)
		m_pSipLocalCaps->GetMaxH264CustomParameters(m_pSipLocalCaps->GetMaxH264Level(kRolePeople), mbps, fs, dpb, brAndCpb, sar, staticMB, eRole);
	if(mbps == 0)
		return INVALID;
	return mbps;
}
//////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::GetRemoteCapsParams( WORD& maxMBPS, WORD& maxFS, WORD& maxDPB, WORD& maxBRandCPB, WORD& maxSAR, WORD& maxStaticMB, ERoleLabel eRole,DWORD profile)
{
	if (m_pSipRemoteCaps)
	{
		CSuperLargeString strCaps1;
		m_pSipRemoteCaps ->DumpToString(strCaps1);
		PTRACE2(eLevelInfoNormal,"CSipPartyCntl::GetRemoteCapsParams   m_pSipRemoteCaps =",strCaps1.GetString());
		m_pSipRemoteCaps->GetMaxH264CustomParameters(m_pSipRemoteCaps->GetMaxH264Level(eRole), maxMBPS, maxFS, maxDPB, maxBRandCPB, maxSAR, maxStaticMB, eRole,profile);
	}
	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::GetRemoteCapsParams -fs is ",maxFS);

}


//////////////////////////////////////////////////////////////////////////////////////////////
void  CSipPartyCntl::Disable4CifInLocalCaps()
{

   if (IsValidPObjectPtr (m_pSipLocalCaps))
   {
    	m_pSipLocalCaps->Set4CifMpi (-1);
        m_pSipLocalCaps->Reomve4cifFromCaps();
    }
    else
        DBGPASSERT (4); // 4 - to signal 4cif :)
}
/////////////////////////////////////////////
void CSipPartyCntl::RemoveH263H261FromLocalCaps()
{
	CCapSetInfo capInfo263 (eH263CapCode);
	CCapSetInfo capInfo261 (eH261CapCode);
	 if (IsValidPObjectPtr (m_pSipLocalCaps))
	 {
		 m_pSipLocalCaps->RemoveCapSet(capInfo263);
	     m_pSipLocalCaps->RemoveCapSet(capInfo261);
	 }
	    else
	        DBGPASSERT (3); // 3 - to signal h263 :)

}

//////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SetPartyToAudioOnly()
{
	// set local caps to audio only

	//save video ssrc for msft
	DWORD  msftSsrcVideo[MaxMsftSvcSdpVideoMlines][2];
	for (int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
	{
	    	msftSsrcVideo[i][0]= m_pSipLocalCaps->getMsftSsrcVideoFirst(i+1);
	    	msftSsrcVideo[i][1]= m_pSipLocalCaps->getMsftSsrcVideoLast(i+1);
	}

    POBJDELETE(m_pSipLocalCaps);


	m_pSipLocalCaps		= new CSipCaps;
	m_pSipLocalCaps->SetAudio(m_pIpInitialMode, m_videoRate, YES, m_name);

	for (int i=0; i<MaxMsftSvcSdpVideoMlines; i++)
	{
		  m_pSipLocalCaps->setMsftSsrcVideo(msftSsrcVideo[i][0],msftSsrcVideo[i][1],i+1);
	}
	PTRACE(eLevelInfoNormal,"CCSipPartyCntl::SetPartyToAudioOnly");




	CIpPartyCntl::SetPartyToAudioOnly();
}

/////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::AreLocalCapsSupportMedia(cmCapDataType eDataType)
{
 	return m_pSipLocalCaps->IsMedia(eDataType);
}


/////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////        Bridge connection               //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void  CSipPartyCntl::InitVideoOutParams(CBridgePartyVideoOutParams *pMediaOutParams, CIpComMode* pScm, BYTE isLprInitiate)
{
	// set the SAR value (not all SIP Eps and EPs versions support it
	if(pScm->IsMediaOn(cmCapVideo, cmCapTransmit))
	{
		CapEnum algorithm = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapTransmit));
		if(algorithm == eH264CapCode)
		{
			long fs = 0;
			long mbps = 0;
			long sar = 0;
			long staticMB = 0;
			long dpb = 0;
			APIU16 profile = 0;
			APIU8 level = 0;
			cmCapDirection direction = cmCapTransmit;

			pScm->GetFSandMBPS(direction, profile, level, fs, mbps, sar, staticMB,dpb);
			CH264Details thisH264Details = level;
			if (fs == -1)
				fs = thisH264Details.GetDefaultFsAsDevision();

			if(fs == 15 || fs == 32)// FS is 15 or 32 (720p or 1080p)
			{
				if(sar == 0 || sar == 255 || sar == -1)// i have tried all the possible combinations. I didn't find any enum for that field (sar)
					pScm->SetSampleAspectRatio(H264_ALL_LEVEL_DEFAULT_SAR);
			}

		}

		// // LYNC 2013/AVMCU party FLOW with mssvc codec
		//if(pScm->IsMediaOn(cmCapVideo, cmCapTransmit)) reach this function also on open_port (encoder)
		{
			CapEnum algorithm = (CapEnum)(pScm->GetMediaType(cmCapVideo, cmCapTransmit));
			if(algorithm == eMsSvcCapCode)
			{
				TRACEINTO << m_pFullPacsi;
				pMediaOutParams->SetFullPacsi(m_pFullPacsi);
			}
		}
	}
	CIpPartyCntl::InitVideoOutParams(pMediaOutParams, pScm, isLprInitiate);
}


/*
//////////////////////////////////////////////////
DWORD CSipPartyCntl::GetLocalCapsVideoRate(CapEnum protocol) const
{
	DWORD rate = 0;
	CComModeInfo comModeInfo(protocol, StartVideoCap);
	CapEnum h323CapCode = comModeInfo.GetH323ModeType();
	rate = m_pSipLocalCaps->GetMaxVideoBitRate(h323CapCode);
	return rate * 100;
}

//////////////////////////////////////////////////
CSipCaps* CSipPartyCntl::GetSipRemoteCap() const
{
	return m_pSipRemoteCaps;
}

//////////////////////////////////////////////////
void CSipPartyCntl::DisconnectVideoChannels()
{
	CSipComMode* pTemp = new CSipComMode;
	*pTemp = *((CSipComMode*)m_pIpCurrentMode);
	pTemp->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit);
	pTemp->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit,kRoleContent);
	m_pPartyApi->SipConfDisconnectChannels(pTemp);
	POBJDELETE(pTemp);
}

*/

///////////////////////////////////////////////////////////
void CSipPartyCntl::OnUpdatePartyVideoBitRate(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyVideoBitRate : Name - ",m_partyConfName, GetPartyRsrcId());


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
            m_pIpInitialMode->GetConfType() == kCop)
		{
			// VSW or COP:
			PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyVideoBitRate : VSW or COP conf. Name - ",m_partyConfName, GetPartyRsrcId());
			m_pIpCurrentMode->SetFlowControlRateConstraint(newBitRate);
			UpdateBridgeFlowControlRateIfNeeded();
		}
		else
		{
			// CP:
			m_eUpdateState = eNoUpdate;
			m_pIpInitialMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyVideoBitRate : cp conf -rate - ",newBitRate);
			BYTE bTakeInitial = TRUE;

			//m_eUpdateState = UpdateAudioAndVideoBridgesIfNeeded(bTakeInitial);
			m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial);

			if(m_eUpdateState != eNoUpdate)
			{
				PTRACEPARTYID(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyVideoBitRate Update Video Out Bit Rate", GetPartyRsrcId());
				m_pIpCurrentMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			}
		}
	}
}
///////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyUpdateVideoResolution(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyVideoResolution : Name - ",m_partyConfName, GetPartyRsrcId());

	if (m_bIsMrcCall || m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw)
	{
	    TRACEINTO << "MRC call or MFW VSW: do nothing";
	    return;
	}

	DWORD Width;
	DWORD Height;
	DWORD FrameRate;
	DWORD Rate;

	*pParam >> Width;
	*pParam >> Height;
	*pParam >> FrameRate;
	*pParam >> Rate;

	if (m_pIpInitialMode->GetConfType() == kCp)
	{
		// CP:
		m_eUpdateState = eNoUpdate;
		m_pIpInitialMode->SetRtvScm(Width,Height,FrameRate,cmCapTransmit,Rate);
		BYTE bTakeInitial = TRUE;

		//m_eUpdateState = UpdateAudioAndVideoBridgesIfNeeded(bTakeInitial);
		m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial);

		if(m_eUpdateState != eNoUpdate)
		{

			PTRACEPARTYID(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyVideoBitRate Update Video Out Bit Rate", GetPartyRsrcId());
			m_pIpCurrentMode->SetRtvScm(Width,Height,FrameRate,cmCapTransmit,Rate);
		}
	}

	if (m_pPartyApi)
		m_pPartyApi->VideoBridgeUpdatedWithNewResolution(STATUS_OK);

}

//////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyUpdateVideoAfterVsrMsg(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateVideoAfterVsrMsg : Name - ",m_partyConfName, GetPartyRsrcId());

	CSipComMode BestMode;
	BestMode.DeSerialize(NATIVE,*pParam);

	COstrStream msg2;
	BestMode.Dump(msg2);
	PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateVideoAfterVsrMsg, bestMode after receive VSR:", msg2.str().c_str());
	COstrStream msg3;
	m_pIpCurrentMode->Dump(msg3);
	PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateVideoAfterVsrMsg, m_pIpCurrentMode after receive VSR:", msg3.str().c_str());


	if (m_pIpInitialMode->GetConfType() == kCp)
	{
		// CP:
		m_pIpInitialMode->CopyMediaMode(BestMode, cmCapVideo, cmCapTransmit, kRolePeople);

		COstrStream msg4;
		m_pIpInitialMode->Dump(msg4);
		PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateVideoAfterVsrMsg, m_pTargetMode after receive VSR4:", msg4.str().c_str());


		// ? build full pacsi for lync 2013 client:
		CapEnum algorithm = (CapEnum)(BestMode.GetMediaType(cmCapVideo, cmCapTransmit));
		BuildSingleUpdatePacsiInfo(m_pFullPacsi.pacsiInfo[0], &BestMode, algorithm, FALSE);

		TRACEINTO << m_pFullPacsi;

		m_eUpdateState = eNoUpdate;
		BYTE bTakeInitial = TRUE;

		BYTE bForceUpdateOut = (eMsSvcCapCode == algorithm) ? TRUE : FALSE;
		m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial,FALSE, bForceUpdateOut);

		if(m_eUpdateState != eNoUpdate)
		{
			TRACEINTO << " ConfName:" << m_partyConfName << ", PartyID:" << GetPartyRsrcId() << " - no update to vb";
			m_pIpCurrentMode->CopyMediaMode(BestMode, cmCapVideo, cmCapTransmit, kRolePeople); // bestMode is m_pIpInitialMode
		}
	}

	if (m_pPartyApi)
		m_pPartyApi->VideoBridgeUpdatedWithNewResolution(STATUS_OK); // back to transaction, send muteifneededtoparty + end transaction

}


void CSipPartyCntl::BuildSingleUpdatePacsiInfo(MsSvcParamsStruct& pacsiInfo, CIpComMode* bestMode, CapEnum algorithm, BYTE isMute)
{
	PASSERTMSG_AND_RETURN(!bestMode, "bestMode == NULL");

	memset(&pacsiInfo, 0, sizeof(MsSvcParamsStruct) );

	pacsiInfo.pr_id = ~0;//DUMMY_PRID; // if isMute or RTV

	if (!isMute && algorithm == eMsSvcCapCode)
	{
		pacsiInfo.pr_id = GetPrIdLync2013();
		TRACEINTO << "MSSVC, NOT MUTED, PR_ID = " << pacsiInfo.pr_id;

		APIS32 width=0, height=0, aspectRatio, maxFrameRate=0;
		cmCapDirection direction = cmCapTransmit;
		bestMode->GetMSSvcSpecificParams(direction, width, height, aspectRatio, maxFrameRate);

		DWORD ssrc = GetFirs2013Ssrc(1); //1 ?
		pacsiInfo.ssrc = ssrc;

		pacsiInfo.nWidth = width;
		pacsiInfo.nHeight = height;
		pacsiInfo.aspectRatio = aspectRatio;
		pacsiInfo.maxFrameRate = maxFrameRate;
		pacsiInfo.maxBitRate = bestMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendSingleUpdatePacsiInfoToSlavesController(const MsSvcParamsStruct& pacsiInfo, BYTE isReasonFecOrRed)
{
	TRACEINTO << " ConfName:" << m_partyConfName << ", PartyID:" << GetPartyRsrcId() << ", isReasonFecOrRed:" << (DWORD)isReasonFecOrRed;

	   // main party
	if(NULL != m_pMsSlavesController)
	{
		CSegment seg;
		seg << GetPartyRsrcId() << isReasonFecOrRed;
		seg.Put((BYTE*)(& pacsiInfo),sizeof(MsSvcParamsStruct));

		m_pMsSlavesController->DispatchEvent(SINGLE_PACSI_INFO_IND, &seg);
	}
	else
	{
		TRACEINTO << " ConfName:" << m_partyConfName << ", PartyID:" << GetPartyRsrcId() << " - m_pMsSlavesController is NULL !!!!!!!!!!!!!!!!!!!!!!!!!";
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyReceviedSingleUpdatePacsiInfoAnycase(CSegment* pParam)
{
	PASSERTMSG_AND_RETURN(!pParam, "pParam == NULL");
	CIpComMode* bestMode = new CIpComMode;
	PASSERTMSG_AND_RETURN(!bestMode, "bestMode  = NULL") ;
	BYTE isMute = FALSE;
		*pParam >> isMute;

	bestMode->DeSerialize(NATIVE,*pParam);

	// build pacsi info
	MsSvcParamsStruct pacsiInfo;
	memset(&pacsiInfo, 0, sizeof(MsSvcParamsStruct) );

	CapEnum algorithm = (CapEnum)(bestMode->GetMediaType(cmCapVideo, cmCapTransmit));
	BuildSingleUpdatePacsiInfo(pacsiInfo, bestMode, algorithm, isMute);

	TRACEINTO << " ConfName:" << m_partyConfName << ", PartyID:" << GetPartyRsrcId() << " - algorithm:" << algorithm << ", isMute:" << (DWORD)isMute;

	if (!isMute && eMsSvcCapCode == algorithm)
	{
		// WAIT FOR FULL PACSI
		m_isWaitingForFullPacsi = TRUE;
	}

	SendSingleUpdatePacsiInfoToSlavesController(pacsiInfo);

	if( !isMute )
	{
		if (m_pIpInitialMode->GetConfType() == kCp && bestMode) // update video bridge only after receiving full pacsi
		{
			m_pIpInitialMode->CopyMediaMode(*bestMode, cmCapVideo, cmCapTransmit, kRolePeople);
		}

		if (eRtvCapCode == algorithm) // do not wait for full pacsi, update video bridges
		{
			if (m_pIpInitialMode->GetConfType() == kCp && bestMode)
			{
				m_eUpdateState = eNoUpdate;
				BYTE bTakeInitial = TRUE;
				BYTE bForceUpdateOut = FALSE; // for rtv
				m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial,FALSE, bForceUpdateOut);

				if(m_eUpdateState != eNoUpdate)
				{
					PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateVideoAfterVsrMsg no update to vb: Name - ",m_partyConfName, GetPartyRsrcId());
					m_pIpCurrentMode->CopyMediaMode(*bestMode, cmCapVideo, cmCapTransmit, kRolePeople); // bestMode is m_pIpInitialMode
				}
			}

			if (m_pPartyApi)
				m_pPartyApi->VideoBridgeUpdatedWithNewResolution(STATUS_OK);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//  on FULL_PACSI_INFO_IND from slaves controller - main and slave out
void CSipPartyCntl::OnSlavesControllerFullPacsiInfoInd(CSegment* pParam)
{
	PASSERTMSG_AND_RETURN(!pParam, "pParam == NULL");

	BYTE isReasonFecOrRed=FALSE;

	*pParam >> isReasonFecOrRed;

	MsFullPacsiInfoStruct fullPACSI;
	pParam->Get(reinterpret_cast<BYTE*>(&fullPACSI), sizeof(MsFullPacsiInfoStruct));

	TRACEINTO << fullPACSI;

	CapEnum algorithm = (CapEnum)(m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit));
	BOOL isMsSvcFecEnabled = FALSE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("ENABLE_SIP_LYNC2013_FEC", isMsSvcFecEnabled);

	if(m_isWaitingForFullPacsi || (isMsSvcFecEnabled && algorithm!=eRtvCapCode) )
	{
		TRACEINTO << " ConfName:" << m_partyConfName << ",LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " ,isReasonFecOrRed:"
				  << (DWORD)isReasonFecOrRed << " - received full pacsi";

		m_pFullPacsi = fullPACSI;

		m_isWaitingForFullPacsi = FALSE;

		if (m_pIpInitialMode->GetConfType() == kCp )
		{
			m_eUpdateState = eNoUpdate;
			BYTE bTakeInitial = TRUE;

			BYTE bForceUpdateOut = TRUE; // for pacsi

			m_eUpdateState = UpdateVideoBridgesIfNeeded(bTakeInitial, FALSE, bForceUpdateOut);

			if(m_eUpdateState != eNoUpdate)
			{
				TRACEINTO << " ConfName:" << m_partyConfName << ", PartyID:" << GetPartyRsrcId() << " - updated";
				m_pIpCurrentMode->CopyMediaMode(*m_pIpInitialMode, cmCapVideo, cmCapTransmit, kRolePeople);
			}
		}

		if (m_pPartyApi && isReasonFecOrRed==FALSE)
			m_pPartyApi->VideoBridgeUpdatedWithNewResolution(STATUS_OK);
	}
	else
	{
		TRACEINTO << " ConfName:" << m_partyConfName << ", PartyID:" << GetPartyRsrcId() << " - This party is muted or RTV";
	}
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::UpdateBridgeFlowControlRateIfNeeded(CLPRParams* pLPRParams)
{
	DWORD flowControlRate = m_pIpCurrentMode->GetFlowControlRateConstraint();

	if (((m_pIpInitialMode->GetConfType() == kVideoSwitch) ||
         (m_pIpInitialMode->GetConfType() == kVSW_Fixed) ||
         (m_pIpInitialMode->GetConfType() == kCop))
		&& IsOutDirectionConnectedToVideoBridge()
		&& flowControlRate)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::UpdateBridgeFlowControlRateIfNeeded - ", flowControlRate);
		CSegment* pSeg = NULL;
		if (pLPRParams != NULL)
		{
			pSeg = new CSegment;
			*pSeg << (DWORD)pLPRParams;
		}

		if (m_pParty)
			m_pTaskApi->UpdateVideoBridgeFlowControlRate(((CParty*)m_pParty)->GetPartyRsrcID(), flowControlRate, cmCapTransmit, kRolePeople, FALSE /*IS CASCADE*/,  pSeg);
	}
}
///////////////////////////////////////////////////////////
BOOL CSipPartyCntl::IsNeedToChangeResAccordingToRemoteRevision()
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsNeedToChangeResAccordingToRemoteRevision");

	BYTE res = FALSE;
	APIU16 profile;
	APIU8 level;
	long dpb,Mbps,fs,brAndCpb, sar, staticMB;
	m_pIpCurrentMode->GetH264Scm(profile, level, Mbps, fs, dpb, brAndCpb, sar, (long&)staticMB, cmCapTransmit);

	DWORD details	= 0;
	BYTE values		= 0;
	int  arrIndex	= 0;
	DWORD ValuesToCompare = kCapCode|kBitRate|kFormat|kAnnexes|kH264Profile|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode | kPacketizationMode;

	//To identify :
	//HDX 8000 Rev B - Cap supported HD1080p30 or HD720p60
	//HDX 7000 Rev C - Cap supported HD1080p30 or HD720p60
	if((strstr(m_productId,"HDX 800"))||(strstr(m_productId,"HDX 700")))
	{
		CComModeH323* pScmWithHD720At60 =  new CComModeH323;
		*pScmWithHD720At60 = *m_pIpCurrentMode;
		pScmWithHD720At60->SetScmToCpHD720At60(cmCapReceiveAndTransmit);
		const CBaseCap* pModeHD720At60 = pScmWithHD720At60->GetMediaAsCapClass(cmCapVideo,cmCapTransmit);
		BYTE IsSupportH720At60 = m_pSipRemoteCaps->IsContainingCapSet(cmCapReceive, *pModeHD720At60, ValuesToCompare, &details, &arrIndex);


		CComModeH323* pScmWithHD1080=  new CComModeH323;
		*pScmWithHD1080 = *m_pIpCurrentMode;
		pScmWithHD1080->SetScmToHdCp(eHD1080Res, cmCapReceiveAndTransmit);
		const CBaseCap* pModeHD1080 = pScmWithHD1080->GetMediaAsCapClass(cmCapVideo,cmCapTransmit);
		BYTE IsSupportHD1080 = m_pSipRemoteCaps->IsContainingCapSet(cmCapReceive, *pModeHD1080, ValuesToCompare, &details, &arrIndex);

		if(IsSupportHD1080 ||IsSupportH720At60)
		{
			PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsNeedToChangeResAccordingToRemoteRevision - HDX 8000 Rev B or HDX 7000 Rev C- Need to change");
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
			CComModeH323* pScmWithHD720At30=  new CComModeH323;
			*pScmWithHD720At30 = *m_pIpCurrentMode;
			pScmWithHD720At30->SetScmToHdCp(eHD720Res, cmCapReceiveAndTransmit);
			const CBaseCap* pModeHD720At30 = pScmWithHD720At30->GetMediaAsCapClass(cmCapVideo,cmCapTransmit);
			BYTE IsSupportHD720At30 = m_pSipRemoteCaps->IsContainingCapSet(cmCapReceive, *pModeHD720At30, ValuesToCompare, &details, &arrIndex);

			if(!IsSupportHD720At30)
			{
				PTRACE(eLevelInfoNormal,"CSipPartyCntl::IsNeedToChangeResAccordingToRemoteRevision - HDX 4000/7000/9000 SD - Need to change");
				res = TRUE;
			}

			POBJDELETE(pScmWithHD720At30);
		}
	}

	return res;
}

//////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnUpdatePartyLprVideoBitRate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyLprVideoBitRate : Name - ",m_partyConfName);
	WORD status;
	DWORD newBitRate;
	WORD channelDirection;
	DWORD lossProtection;
	DWORD mtbf;
	DWORD congestionCeiling;
	DWORD fill;
	DWORD modeTimeout;
	DWORD totalVideoRate;
	DWORD RemoteIdent;

	*pParam >> newBitRate;
	*pParam >> channelDirection;
	*pParam >> lossProtection;
	*pParam >> mtbf;
	*pParam >> congestionCeiling;
	*pParam >> fill;
	*pParam >> modeTimeout;
	*pParam >> totalVideoRate;
	*pParam >> RemoteIdent;

	if (MicrosoftEP_Lync_CCS == RemoteIdent)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::UpdateBridgeFlowControlRateIfNeeded -IsOutDirectionConnectedToVideoBridge = ", IsOutDirectionConnectedToVideoBridge());
	}
	else if( !IsOutDirectionConnectedToVideoBridge() )
	{
	    PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyLprVideoBitRate: video bridge is not connected. Name - ",m_partyConfName);
	    return;
	}

	if ( (cmCapDirection)channelDirection == cmCapTransmit )
	{
		DWORD preBitRate = 0;
		if (MicrosoftEP_Lync_CCS == RemoteIdent)
		{
			preBitRate = m_pIpCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePresentation);
		}
		else
		{
			preBitRate = m_pIpCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
		}
		CMedString str;
	    str << "CSipPartyCntl::UpdateBridgeFlowControlRateIfNeeded: preBitRate = " << preBitRate
	        << " newBitRate = " << (DWORD)newBitRate
			<< " totalVideoRate = " << (DWORD)totalVideoRate;
		PTRACE (eLevelError, str.GetString());

		if (newBitRate ==  preBitRate)
			m_isLprActive = 0;
		else
			m_isLprActive = 1;

		m_pIpCurrentMode->SetTotalVideoRate(totalVideoRate);
		m_pIpInitialMode->SetTotalVideoRate(totalVideoRate);

		if ((m_pIpInitialMode->GetConfType() == kVideoSwitch) ||
            (m_pIpInitialMode->GetConfType() == kVSW_Fixed) ||
            (m_pIpInitialMode->GetConfType() == kCop))
		{
			// or cop:
			PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyLprVideoBitRate : VSW or Cop conf. Name - ",m_partyConfName);
			m_pIpCurrentMode->SetFlowControlRateConstraint(newBitRate);

			CLPRParams* pLprParams = new CLPRParams(lossProtection,mtbf,congestionCeiling*10,fill,modeTimeout);
			UpdateBridgeFlowControlRateIfNeeded(pLprParams);

			POBJDELETE(pLprParams);

		}
		else
		{
			// CP:
			m_eUpdateState = eNoUpdate;
			if (MicrosoftEP_Lync_CCS == RemoteIdent)
			{
				m_pIpInitialMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePresentation);
			}
			else
			{
				m_pIpInitialMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			}
			BYTE bTakeInitial = TRUE;
			m_eUpdateState = UpdateLprVideoOutBridgeRate(lossProtection, mtbf, congestionCeiling*10, fill, modeTimeout, bTakeInitial,RemoteIdent);
			if(m_eUpdateState != eNoUpdate)
			{
				PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnUpdatePartyLprVideoBitRate Update Video Out Bit Rate");
				if (MicrosoftEP_Lync_CCS == RemoteIdent)
				{
					m_pIpCurrentMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePresentation);
				}
				else
				{
					m_pIpCurrentMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
				}

			}

		}
	}
}
//////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED: update the initial mode and the VB if needed.
void CSipPartyCntl::OnPartyCntlUpdatePartyFecOrRedVideoBitRate(CSegment* pParam)
{

	DWORD newBitRate = m_pIpCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople); //default value
	DWORD type = cmCapEmpty;     //default value.
	BYTE isVidoOutMuted = FALSE; //default value
	BYTE isThereNewMode = FALSE; //default value
	CSipComMode newMode;

	*pParam >> newBitRate >> type >> isVidoOutMuted >> isThereNewMode;

	if(!IsOutDirectionConnectedToVideoBridge())
	{
	    TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - video bridge is not connected. Name - " << m_partyConfName << ", type:" << type;
	    m_pPartyApi->VideoOutChannelsUpdatedForFecOrRed( (WORD)statIllegal, type);
	    return;
	}

	if (isThereNewMode)
	{
		newMode.DeSerialize(NATIVE,*pParam);

		COstrStream msg2;
		newMode.Dump(msg2);
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - newBitRate:" << newBitRate << " ,type:" << type << " ,isVidoOutMuted:" << (DWORD)isVidoOutMuted << " ,isThereNewMode:" << (DWORD)isThereNewMode;
		PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnPartyCntlUpdatePartyFecOrRedVideoBitRate - LYNC2013_FEC_RED: UPDATE_INITIALMODE with new mode:", msg2.str().c_str());
	}
	else
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - UPDATE_INITIALMODE with newBitRate:" << newBitRate << " ,type:" << type
		          << " ,isVidoOutMuted:" << (DWORD)isVidoOutMuted << " ,isThereNewMode:" << (DWORD)isThereNewMode << ", Name:" << m_partyConfName;


	if (newBitRate == m_pIpCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople) )
	{
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - same rate";
		m_pPartyApi->VideoOutChannelsUpdatedForFecOrRed( (WORD)statOK, type);
	}
	else
	{

		if (m_pIpInitialMode->GetConfType() == kCp)
		{
			CapEnum algorithm = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit));

			///////////////////////////////////update transmit details//////////////////////////////////////////////////////
			//SVC: update new mode (copy the Target to the Initial)
			m_pIpInitialMode->CopyMediaMode(newMode, cmCapVideo, cmCapTransmit, kRolePeople);

			//RTV: update only video bit rate
			if (algorithm == eRtvCapCode)
				m_pIpInitialMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////


			BYTE isNeedToUpdateTheVideoBridge = FALSE;
			if ( (m_eVidBridgeConnState & eSendOpenOut) &&
				 (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit) && m_pIpCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit)) )
				isNeedToUpdateTheVideoBridge = TRUE;

			if (isNeedToUpdateTheVideoBridge == TRUE)
			{
				CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_name);
				DWORD   msftAvmcuState = eMsftAvmcuNone;
				DWORD   avMcuLinkType  = eAvMcuLinkNone;

				if (pConfParty)
				{
					msftAvmcuState = pConfParty->GetMsftAvmcuState();
					avMcuLinkType  = pConfParty->GetAvMcuLinkType();
				}

				TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - wait to update the VB (check if need to update pacsi before). MsftAvmcuState:" << msftAvmcuState;

				if (type == cmCapVideo)
					m_IsWaitForAckForFecUpdate = TRUE;
				else if (type == cmCapAudio)
					m_IsWaitForAckForRedUpdate = TRUE;

				/*
				if ( (msftAvmcuState == eMsftAvmcu2013 || avMcuLinkType == eAvMcuLinkSlaveOut) && algorithm == eMsSvcCapCode )
				{
					MsSvcParamsStruct pacsiInfo;
					memset(&pacsiInfo, 0, sizeof(MsSvcParamsStruct) );
					BuildSingleUpdatePacsiInfo(pacsiInfo, m_pIpInitialMode, algorithm , isVidoOutMuted);

					TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - av-mcu 2013 - send Pacsi Message, ConfName:" << m_partyConfName
							  << ", algorithm:" << algorithm << ", isMute:" << (DWORD)isVidoOutMuted;

					SendSingleUpdatePacsiInfoToSlavesController(pacsiInfo,1);
				}
				else   // rtv or svc with no avmcu
				{ */
					TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - No need to update pacsi (rtv/no avmcu), algorithm:"
							  << (DWORD)algorithm << ", isAvMCU:" << msftAvmcuState;
					m_eUpdateState = UpdateFecOrRedVideoOutBridgeRate();
				//}
			}
			else
			{
				TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - Update Video Out Bit Rate";
				m_pIpCurrentMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
				m_pPartyApi->VideoOutChannelsUpdatedForFecOrRed( (WORD)statOK , type);
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipPartyCntl::OnPartyCntlSendPartySingleFecOrRedMsg(CSegment* pParam)
{
	DWORD  mediaType = cmCapEmpty;
	DWORD  newFecRedPercent = 0;

	*pParam >> mediaType >> newFecRedPercent;

	TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - mediaType:" << mediaType << ", newFecRedPercent:" << newFecRedPercent
			  << ", name:" << m_partyConfName;

	if(!m_pPartyApi)
		PASSERT_AND_RETURN(1);
	m_pPartyApi->SendSingleFecOrRedMsgToSipParty(mediaType,newFecRedPercent);
}
//////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::HandleLprUpdatedIndications(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyCntl::HandleLprUpdatedIndications: Name - ",m_partyConfName);

	CAckParams* pAckParams = new CAckParams;

	pAckParams->DeSerialize(NATIVE,*pParam);

	if(CPObject::IsValidPObjectPtr(pAckParams))
	{
		PTRACE (eLevelInfoNormal,"CSipPartyCntl::HandleLprUpdatedIndications : LPR Request ");

		CLPRParams*	pLPRParams = pAckParams->GetLPRParams();

		if(CPObject::IsValidPObjectPtr(pLPRParams))
		{
			CSegment* pSeg = new CSegment;
			*pSeg << pLPRParams->GetLossProtection() << pLPRParams->GetMTBF() << pLPRParams->GetCongestionCeiling()
				<< pLPRParams->GetFill() << pLPRParams->GetModeTimeout();

			//DispatchEvent(PARTY_LPR_VIDEO_OUT_RATE_UPDATED,pSeg);//instead we shout call function OnLprVideoOutBrdgBitRateUpdated
			OnLprVideoOutBrdgBitRateUpdated(pSeg);
			POBJDELETE(pSeg);
		}
		else
			PTRACE (eLevelInfoNormal,"CSipPartyCntl::HandleLprUpdatedIndications : No LPR Params ");

	}
	POBJDELETE(pAckParams);
}
////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnLprVideoOutBrdgBitRateUpdated(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnLprVideoOutBrdgBitRateUpdated : Name - ",m_partyConfName, GetPartyRsrcId());

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

	if (m_pPartyApi)
		m_pPartyApi->SetLprModeForVideoOutChannels(status, lossProtection, mtbf, congestionCeiling, fill, modeTimeout);
}
////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipPartyCntl::HandleFECUpdatedInd(WORD status)
{
	TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - need to update the current mode of party. ConfName:" << m_partyConfName;

	m_IsWaitForAckForFecUpdate = FALSE;
	m_pPartyApi->VideoOutChannelsUpdatedForFecOrRed( (WORD)status, cmCapVideo);
}
////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipPartyCntl::HandleREDUpdatedInd(WORD status)
{
	TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyRsrcId() << " - need to update the current mode of party. ConfName:" << m_partyConfName;

	m_IsWaitForAckForRedUpdate = FALSE;
	m_pPartyApi->VideoOutChannelsUpdatedForFecOrRed( (WORD)status, cmCapAudio);
}
////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyUpdateICEParams(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::OnPartyUpdateICEParams",GetPartyRsrcId() );
	BYTE IsIceParams = 0;
	*pParam >> m_IsIceParty >> IsIceParams;

	PTRACE2INT(eLevelInfoNormal,"CSipAddPartyCntl::OnPartyUpdateICEParams -  m_IsIceParty: ",m_IsIceParty);

    if (IsIceParams)
    {
    	m_IceParams	= new CIceParams;
    	m_IceParams->DeSerialize(NATIVE, *pParam);
    	m_IceParams->Dump("SetIceParams");
    }
}

/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::UpdateCurrentModeInDB()
{
    CSegment* pCurComSeg = new CSegment;
    m_pIpCurrentMode->Serialize(*pCurComSeg,cmCapTransmit,YES, const_cast<CSipCaps*>(m_pSipLocalCaps));
    m_pTaskApi->UpdateDB(m_pParty,LOCAL323COMMODE,(DWORD) 0,1,pCurComSeg);
    POBJDELETE(pCurComSeg);
}
////////////////////////////////////////////////////////////////////////////////
DWORD CSipPartyCntl::GetMaxFsAccordingtoProfile(APIU16 profile)
{
	DWORD maxFs = m_pSipRemoteCaps->GetMaxFsAccordingToProfile(profile);
	return maxFs;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::DumpUdpAddresses()
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPartyCntl::DumpUdpAddresses",GetPartyRsrcId() );

	char szIP[IPV6_ADDRESS_LEN];
	CLargeString *pStr = new CLargeString;

	*pStr	<< "AudioChannelPort	 = " << m_udpAddresses.AudioChannelPort  <<'\n'
			<< "VideoChannelPort	 = " << m_udpAddresses.VideoChannelPort  <<'\n'
			<< "ContentChannelPort 	 = " << m_udpAddresses.ContentChannelPort  <<'\n'
			<< "FeccChannelPort		 = " << m_udpAddresses.FeccChannelPort  <<'\n'
			<< "BfcpChannelPort		 = " << m_udpAddresses.BfcpChannelPort  <<'\n'
			<< "UdpAddressType		 = " << IpTypeToString(m_udpAddresses.IpType)  <<'\n';

	SystemDWORDToIpString(m_udpAddresses.IpV4Addr.ip, szIP);
	*pStr << "IP v4	  = " << szIP <<'\n';

	*pStr << "Ip V6:";

	mcTransportAddress tempAddr;

	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(&tempAddr,0,sizeof(mcTransportAddress));
		memset(szIP,0,IPV6_ADDRESS_LEN);

		tempAddr.ipVersion = (DWORD)eIpVersion6;
		tempAddr.addr.v6.scopeId = m_udpAddresses.IpV6AddrArray[i].scopeId;
		memcpy(tempAddr.addr.v6.ip,m_udpAddresses.IpV6AddrArray[i].ip,IPV6_ADDRESS_BYTES_LEN);
		::ipToString(tempAddr,szIP,1);

		*pStr << "\nIp V6 " << i << " ="
		      << "\nScope Id: " << m_udpAddresses.IpV6AddrArray[i].scopeId
		      << "\nIp: " << szIP << "\n";
	}

	PTRACEPARTYID(eLevelInfoNormal, pStr->GetString(), GetPartyRsrcId());
	POBJDELETE(pStr);
}

////////////////////////////////
void CSipPartyCntl::DumpNewUdpAddresses(UdpAddresses& newudp)
{
	CLargeString *pStr = new CLargeString;
	PTRACEPARTYID(eLevelInfoNormal,"CSipAddPartyCntl::DumpNewUdpAddresses",GetPartyRsrcId() );
	*pStr	<< "AudioChannelPort	 = " << newudp.AudioChannelPort  <<'\n'
			<< "VideoChannelPort	 = " << newudp.VideoChannelPort  <<'\n'
			<< "ContentChannelPort 	 = " << newudp.ContentChannelPort  <<'\n'
			<< "FeccChannelPort		 = " << newudp.FeccChannelPort  <<'\n'
			<< "BfcpChannelPort		 = " << newudp.BfcpChannelPort  <<'\n'
			<< "UdpAddressType		 = " << newudp.IpType  <<'\n';


	*pStr << "IP v4	  = " << newudp.IpV4Addr.ip  <<'\n';

	*pStr << "Ip V6:";
	mcTransportAddress tempAddr;
	char szIP[IPV6_ADDRESS_LEN];
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(&tempAddr,0,sizeof(mcTransportAddress));
		memset(szIP,0,IPV6_ADDRESS_LEN);

		tempAddr.ipVersion = (DWORD)eIpVersion6;
		tempAddr.addr.v6.scopeId = newudp.IpV6AddrArray[i].scopeId;
		memcpy(tempAddr.addr.v6.ip,newudp.IpV6AddrArray[i].ip,IPV6_ADDRESS_BYTES_LEN);
		::ipToString(tempAddr,szIP,1);

		*pStr << "\nIp V6 " << i << " ="
		      << "\nScope Id: " << newudp.IpV6AddrArray[i].scopeId
		      << "\nIp: " << szIP << "\n";
	}


	//		*pStr << "IP v6	  = " << (WORD)m_udpAddresses.IpAddr.v6.ip  <<'\n';
	PTRACEPARTYID(eLevelInfoNormal, pStr->GetString(), GetPartyRsrcId());
	POBJDELETE(pStr);
}


//////////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsPartyCapsContainsH264SCM(const CMediaModeH323* H323ContentMode,ERoleLabel role)
{
	DWORD details = 0;
	int  arrIndex = 0;
	DWORD valuesToCompare = kCapCode|kBitRate|kFormat|kAnnexes|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode;
	CBaseCap* pCapClass = H323ContentMode->GetAsCapClass();
	if (pCapClass && m_pSipRemoteCaps->IsContainingCapSet(cmCapReceive, *pCapClass, valuesToCompare, &details, &arrIndex))
	{
		POBJDELETE(pCapClass);
		return TRUE;
	}
	POBJDELETE(pCapClass);
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::CheckIfNeedToSendIntra()
{
	const char* MOC_PRODUCT_NAME = "Microsoft Office Communicator";
	const char* MOC_LYNC_PRODUCT_NAME = "Microsoft Lync 2010";
	const char* MOC_MAC_PRODUCT_NAME = "Microsoft Communicator for Mac";
	const char* LYNC_MAC_PRODUCT_NAME = "Microsoft Lync for Mac";

	char* bIsMsMoc = strstr(m_productId, MOC_PRODUCT_NAME);
	char* bIsMsLyncMoc = strstr(m_productId, MOC_LYNC_PRODUCT_NAME);
	char* bIsMsMacMoc = strstr(m_productId, MOC_MAC_PRODUCT_NAME);
	char* bIsMsMacLync = strstr(m_productId, LYNC_MAC_PRODUCT_NAME);

	if(bIsMsMoc || bIsMsLyncMoc || bIsMsMacMoc || bIsMsMacLync)
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::CheckIfNeedToSendIntra Sending Intra to MS client");
		return TRUE;
	}
	else
		return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendVideoPreferenceToParty()
{
	StartTimer(SIP_SEND_VIDEO_PREFERENCE_TOUT, 1*SECOND);
}

////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnSendVideoPreferenceToPartyTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::OnSendVideoPreferenceToPartyTout Send video preference to party");

	if (m_pPartyApi)
		m_pPartyApi->SendVideoPreferenceToSipParty();

}

////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpRequestByEP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"_scp_flow_ CSipPartyCntl::OnPartyCntlScpRequestByEP");
	printf("_scp_flow_ CSipPartyCntl::OnPartyCntlScpRequestByEP\n");

	CMrmpScpStreamsRequestStructWrap scpStreamReq;
	scpStreamReq.DeSerialize(NATIVE,*pParam);

	if (m_pScpHandler)
	{
		bool bOk = m_pScpHandler->HandleScpRequest(scpStreamReq, m_pIpInitialMode, m_pSipLocalCaps);
		
		if (true == bOk)
		{
			if (IsValidTimer(SCP_NOTIFICATION_REQ_TOUT))
				DeleteTimer(SCP_NOTIFICATION_REQ_TOUT);
		}

		if ((IP_CONNECTED == m_connectingState) && (true == bOk))
		{
			if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit))
			{
				// @#@ update Party with new SCM streams
				m_pPartyApi->UpdateScmStreams(m_pIpInitialMode);

				m_pSipLocalCaps->Dump("CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded localCaps ",eLevelInfoNormal);
				UpdateVideoRelayOutBridgeIfNeeded(TRUE,TRUE); //
				*m_pIpCurrentMode = *m_pIpInitialMode;
				//UpdateVideoBridgeIfNeeded();
			}
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"_scp_flow_ CSipPartyCntl::OnPartyCntlScpRequestByEP m_pScpHandler is NULL");
	}
}

////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpNotificationByEP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipPartyCntl::OnPartyCntlScpNotificationByEP");
	printf("CSipPartyCntl::OnPartyCntlScpNotificationByEP\n");

	CScpNotificationWrapper scpNotificationInd;
	scpNotificationInd.DeSerialize(NATIVE,*pParam);
	scpNotificationInd.Dump("CSipPartyCntl::OnPartyCntlScpNotificationByEP");

	if (m_pScpHandler)
	{
		bool bOk = m_pScpHandler->HandleScpNotificationInd(scpNotificationInd, m_pIpInitialMode);

		if ( (IP_CONNECTED == m_connectingState) && (true == bOk) )
		{
			if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapReceive))
			{
				m_pSipLocalCaps->Dump("CIpPartyCntl::OnPartyCntlScpNotificationByEP localCaps ",eLevelInfoNormal);
				UpdateVideoRelayInBridgeIfNeeded(TRUE, TRUE); //
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlFirRequestByEP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"OnPartyCntlFirRequestByEP");

	m_pVideoBridgeInterface->RelayEpAskForIntra(pParam);
}
////////////////////////////////////////////////////////////////////////////////
void  CSipPartyCntl::OnPartyAuthCompleate(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnPartyAuthCompleate: ");

	BYTE status = 0;
	*pParam >> status;

	if (status == 1)
		m_isPartyInConf = TRUE;
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnPartyAuthCompleate: password auth failed status," ,status);
		m_pTaskApi->PartyDisConnect(PASSWORD_FAILURE,m_pParty);
	}

}
// TIP
////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SetIsTipCall(BYTE bIsTipCall)
{
	m_bIsTipCall = bIsTipCall;
}

////////////////////////////////////////////////////////////////////////////////
BOOL CSipPartyCntl::GetIsTipCall() const
{
	return m_bIsTipCall;
}

void CSipPartyCntl::OnSlaveToMasterMessage(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnSlaveToMasterMessage");
	if (m_pPartyApi)
		m_pPartyApi->PartyCntlToPartyMessageSlaveToMaster(pParam);
}

void CSipPartyCntl::OnSlaveToMasterAckMessage(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnSlaveToMasterAckMessage");

	DWORD status;
	DWORD peerRsrcId;
	WORD tipPartyType;
	WORD allocated_resolution;
	DWORD m_monitorSlavePartyId;
	WORD reason;

	*pParam >> status
		    >> peerRsrcId
		    >> tipPartyType
		    >> allocated_resolution;

	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnSlaveToMasterAckMessage status ", status);
	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnSlaveToMasterAckMessage peerRsrcId ", peerRsrcId);
	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnSlaveToMasterAckMessage tipPartyType ", tipPartyType);
	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnSlaveToMasterAckMessage allocated_resolution ", allocated_resolution);


	switch (tipPartyType)
	{
		case eTipSlaveLeft:

			m_TipSlaveLeftAddSent = FALSE;
			reason = SLAVE_PARTY_LEFT_ALLOCATION_FAILURE;
			break;
		case eTipSlaveRigth:
			m_TipSlaveRightAddSent = FALSE;
			reason = SLAVE_PARTY_RIGHT_ALLOCATION_FAILURE;
			break;
		case eTipSlaveAux:
			m_TipSlaveAuxAddSent = FALSE;
			reason = SLAVE_PARTY_AUX_ALLOCATION_FAILURE;
			break;
		default:
			PASSERTSTREAM(TRUE, "tipPartyType " << tipPartyType << " is invalid??");
			reason = 0;
			break;
	}
	if( status != statOK)
	{
		//slave failed, so disconnect master.
		PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnSlaveToMasterAckMessage - failure status ", status);
		m_pTaskApi->PartyDisConnect(reason,m_pParty);
		return;
	}
	else
	{
		switch (tipPartyType)
		{
			case eTipSlaveLeft:
				m_SlaveLeftRsrcId = peerRsrcId;
				break;
			case eTipSlaveRigth:
				m_SlaveRightRsrcId = peerRsrcId;
				break;
			case eTipSlaveAux:
				m_SlaveAuxRsrcId = peerRsrcId;
				break;
			default:
				PASSERTSTREAM(TRUE, "tipPartyType " << tipPartyType << " is invalid??");
				reason = 0;
		}
	}
	if( tipPartyType != eTipSlaveAux && allocated_resolution<eCP_H264_upto_HD720_30FS_Symmetric_video_party_type)
	{
		//resolution of slave less than HD720, so disconnect master.
		PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnSlaveToMasterAckMessage allocated_resolution is less than HD720 ", allocated_resolution);
		m_pTaskApi->PartyDisConnect(reason,m_pParty);
		return;
	}
	/*if(allocated_resolution != m_eLastAllocatedVideoPartyType) There will be no realloc: party have to be HD1080
	{
		//reallocate resources and send ReInvite with new resolution
		H264VideoModeDetails h264VidModeDetails = GetH264ModeAccordingToVideoPartyType((eVideoPartyType)allocated_resolution);
		UpdateH264ModeInLocalCaps(h264VidModeDetails);
		m_pIpInitialMode->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR,cmCapReceive);
		m_pIpInitialMode->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR,cmCapTransmit);
		m_state = CONF_REQUEST_PARTY_CHANGE_MODE;
		BYTE bIsContentSpeaker = (m_pContentBridge && m_pContentBridge->IsTokenHolder(m_pParty)) ? TRUE : FALSE;
		m_pPartyApi->ChangeModeIp(m_pIpInitialMode, eChangeIncoming, bIsContentSpeaker);
	}*/

	if (m_pPartyApi)
		m_pPartyApi->SetSlavePartyRsrcId(tipPartyType, peerRsrcId);

}

void CSipPartyCntl::PartyToPartyCntlMessageMasterToSlave(CSegment* pParam)
{
	DWORD destTipPartyType;
	DWORD temp;
	DWORD opcode;
	DWORD peerRsrcId;

	*pParam >> destTipPartyType >> opcode;

	switch (destTipPartyType)
	{
	case eTipSlaveLeft:
		peerRsrcId = m_SlaveLeftRsrcId;
		break;
	case eTipSlaveRigth:
		peerRsrcId = m_SlaveRightRsrcId;
		break;
	case eTipSlaveAux:
		peerRsrcId = m_SlaveAuxRsrcId;
		break;
	default:
		PASSERTSTREAM(TRUE, "destTipPartyType " << destTipPartyType << " is invalid??");
		peerRsrcId = 0xFFFFFFFF;
	}

	TRACEINTO << " CSipPartyCntl::PartyToPartyCntlMessageMasterToSlave : destTipPartyType=" << destTipPartyType << ", opcode=" << opcode << ", peerRsrcId=" << peerRsrcId;

	if(peerRsrcId != 0 )
		m_pTaskApi->PartyCntlToPartyMsgFromMasterToSlave(peerRsrcId, opcode, pParam);
	else
		PASSERTSTREAM(TRUE, "peerRsrcId " << peerRsrcId << " is invalid ");

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::PartyCntlToPartyMessageMasterToSlave(CSegment* pParam)
{
	if (m_pPartyApi)
		m_pPartyApi->PartyCntlToPartyMessageMasterToSlave(pParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::PartyToPartyCntlMessageSlaveToMaster(CSegment* pParam)
{
//	PTRACE(eLevelInfoNormal,"CSipPartyCntl::PartyToPartyCntlMessageSlaveToMaster ");
	DWORD tipPartyType,	opcode;
	*pParam >> tipPartyType >> opcode;
	TRACEINTO << " CSipPartyCntl::PartyToPartyCntlMessageSlaveToMaster : tipPartyType=" << tipPartyType << ", opcode=" << opcode;
	SendMessageFromSlaveToMaster(tipPartyType, opcode, pParam);//m_pPartyApi->PartyCntlToPartyMessageSlaveToMaster(pParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::PartyCntlToPartyMessageSlaveToMaster(CSegment* pParam)
{
	if (m_pPartyApi)
		m_pPartyApi->PartyCntlToPartyMessageSlaveToMaster(pParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnAddSlaveParty(CSegment* pParam)
{

	DWORD tipPartyType=0;

	*pParam >> tipPartyType;

	CRsrvParty* slaveParty = new CRsrvParty;
	slaveParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);

	slaveParty->SetUndefinedType(UNRESERVED_PARTY);

	TRACEINTO << " tipPartyType=" << tipPartyType << ", GetIsTipCall = " <<  (int)GetIsTipCall() << ", m_RoomId = " <<  (int)m_RoomId ;

	if(m_TipMasterName == NULL)
	{
			AdjustPartynameToTip(m_RoomId);
			//if( !IsTipCompatibleContent() )
			//	UpdateCurrentModeNoMedia(cmCapVideo,kRoleContentOrPresentation);
	}

	char *p_name = new char[H243_NAME_LEN];
	memset(p_name, '\0', H243_NAME_LEN);
	strncpy(p_name, m_TipMasterName, H243_NAME_LEN  - sizeof("_aux") - 1);

	switch(	tipPartyType)
	{
		case eTipSlaveAux:
			strcat(p_name, "_aux");
			slaveParty->SetVoice(YES);
			m_TipSlaveAuxAddSent = TRUE;
			break;
		case eTipSlaveLeft:
			strcat(p_name, "_2");
			m_TipSlaveLeftAddSent = TRUE;
			break;
		case eTipSlaveRigth:
			strcat(p_name, "_3");
			m_TipSlaveRightAddSent = TRUE;
			break;
	}
	slaveParty->SetName(p_name);
	slaveParty->SetTIPPartyType( eTipPartySlave );//Olga
	slaveParty->SetRoomId(m_RoomId);//Olga

	PDELETEA(p_name);

	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnAddSlaveParty m_RoomId is ", m_RoomId);

	slaveParty->SetPartyId(0xFFFFFFFF);

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_name);
	if (pConfParty)
	{
	    slaveParty->SetServiceProviderName(pConfParty->GetServiceProviderName());
	    slaveParty->SetServiceId(pConfParty->GetServiceId());
	    slaveParty->SetConnectionType(pConfParty->GetConnectionTypeOper());
	    slaveParty->SetIpAddress(pConfParty->GetIpAddress());
	    slaveParty->SetH323PartyAliasType(pConfParty->GetH323PartyAliasType());
	    slaveParty->SetH323PartyAlias(pConfParty->GetH323PartyAlias());
	    slaveParty->SetSipPartyAddress(pConfParty->GetSipPartyAddress());
	    slaveParty->SetSipPartyAddressType(pConfParty->GetSipPartyAddressType());
	    if(eTipSlaveLeft == tipPartyType || eTipSlaveRigth == tipPartyType)
	    	slaveParty->SetTelePresenceMode( (BYTE)eTelePresencePartyCTS );
	    slaveParty->SetPreDefinedIvrString(pConfParty->GetPreDefinedIvrString()); //BRIDGE-4486
	    //Add party to the conference
	    m_pTaskApi->SendAddSlavePartyToConf(*slaveParty, 0, tipPartyType, m_RoomId, m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_eLastAllocatedVideoPartyType, m_pIpInitialMode);
	}
	else
	    PTRACE(eLevelError,"CSipPartyCntl::OnAddSlaveParty - pConfParty is NULL - can not SendAddSlavePartyToConf");

	POBJDELETE(slaveParty);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendMessageFromMasterToSlave(WORD destTipPartyType, DWORD opcode, CSegment *pMsg)
{
	PartyRsrcID destPartyRsrcId = 0;
	switch(	destTipPartyType)
	{
		case eTipSlaveAux:
			destPartyRsrcId = m_SlaveAuxRsrcId;
			break;
		case eTipSlaveLeft:
			destPartyRsrcId = m_SlaveLeftRsrcId;
			break;
		case eTipSlaveRigth:
			destPartyRsrcId = m_SlaveRightRsrcId;
			break;
	}
	m_pTaskApi->SendMessageFromMasterToSlave(destPartyRsrcId, opcode, pMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendMessageFromSlaveToMaster(WORD srcTipPartyType, DWORD opcode, CSegment *pMsg)
{
	m_pTaskApi->SendMessageFromSlaveToMaster( m_MasterRsrcId, srcTipPartyType, opcode, pMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendMessageFromMSSlaveToMain(DWORD opcode, CSegment *pMsg)
{
	m_pTaskApi->SendMsSlaveToMainMsg( m_MasterRsrcId, opcode, pMsg);
}


//////////////////////////////////////////////////////////////////////////////
BOOL CSipPartyCntl::IsNeedToUpdateVisualName()
{
	return m_needToUpdateVisualName;
}
//////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::AdjustPartynameToTip(DWORD roomId)
{
	   char temp[H243_NAME_LEN];
	   char tempVisualName[H243_NAME_LEN];
	   memset(temp, '\0', H243_NAME_LEN);
	   memset(tempVisualName, '\0', H243_NAME_LEN);

	   CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_name);
	   if (!pConfParty)
	   {
	       PTRACE(eLevelError,"CSipPartyCntl::AdjustPartynameToTip - pConfParty is NULL");
	       DBGPASSERT(1123);
	       return;
	   }

//	   strncpy(temp, m_name, sizeof(temp)-1);
//	   temp[sizeof(temp)-1] = '\0';
//	   char *last_brekat = strstr(temp, "000)");
//	   if ( last_brekat != NULL )
//	   {
//			*last_brekat = '\0';
//		    m_TipMasterName = new char[H243_NAME_LEN];
//		    memset(m_TipMasterName, '\0', H243_NAME_LEN);
//			sprintf(m_TipMasterName, "%s%03d)", temp, roomId);
//			PTRACE2(eLevelInfoNormal,"CSipPartyCntl::AdjustPartynameToTip m_TipMasterName is ", m_TipMasterName);
//		    memset(tempVisualName, '\0', H243_NAME_LEN);
//			snprintf(tempVisualName, sizeof(tempVisualName), "%s_1", m_TipMasterName);
//			PTRACE2(eLevelInfoNormal,"CSipPartyCntl::AdjustPartynameToTip m_name is ", m_name);
//	   }
//	   else
//	   {

	   m_TipMasterName = new char[H243_NAME_LEN];
	   memset(m_TipMasterName, '\0', H243_NAME_LEN);
	   strncpy(m_TipMasterName,m_name ,H243_NAME_LEN-1);
	   PTRACE2(eLevelInfoNormal,"CSipPartyCntl::AdjustPartynameToTip m_TipMasterName is ", m_TipMasterName);
	   memset(tempVisualName, '\0', H243_NAME_LEN);
	   snprintf(tempVisualName, sizeof(tempVisualName), "%s_1", m_TipMasterName);
	   PTRACE2(eLevelInfoNormal,"CSipPartyCntl::AdjustPartynameToTip m_name is ", m_name);

	   //	   }

	   const char* strConfName = m_pConf->GetName();
	   //SetFullName(m_name, strConfName);
	   //if (DIALOUT != m_type)
	   //    m_pPartyApi->SendNewNameWithRoomId(m_name, strConfName);
	   //pConfParty->SetName(m_name);
	   pConfParty->SetRemoteName(tempVisualName);
	   pConfParty->SetVisualPartyName(tempVisualName);//michael
	   if( m_TipNumOfScreens>1 )
	   {
		   PTRACE(eLevelInfoNormal,"CSipPartyCntl::AdjustPartynameToTip -  setting  eTelePresencePartyCTS ");
		   pConfParty->SetTelePresenceMode( (BYTE)eTelePresencePartyCTS );
	   }
	   //	m_pTaskApi->UpdateDB(m_pParty, PARTYTELEPRESENCEMODE, (DWORD)eTelePresencePartyCTS);
	   DWORD len = strlen(tempVisualName);
	   SetSiteName(tempVisualName);
	   CSegment* pSeg = new CSegment;
	   *pSeg << len;
	   pSeg->Put((unsigned char*)tempVisualName, len);
	   m_pTaskApi->UpdateDB(m_pParty, UPDATEVISUALNAME, (DWORD)0, 1, pSeg);
	   POBJDELETE(pSeg);

	   m_needToUpdateVisualName = FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnFallBackFromTip(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnFallBackFromTip - from partycontrol to conf");
	//CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	//m_pTaskApi-> call to fall back

	CIpComMode* pScm = new CIpComMode;

	pScm->DeSerialize(NATIVE,*pParam);

	m_bIsTipCall = FALSE;
	m_TipPartyType = eTipNone;
	m_pTaskApi->SendFallBckToReglarPartyToConf(GetName(), pScm);

	POBJDELETE(pScm);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::ChangeSipfromTipToNonTip(CIpComMode* pScm,PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::ChangeSipfromTipToNonTip from conf to party in order to start a new reinvite");

	CConfParty *pConfParty =  partyControInitParam.pConfParty;
	CIpComMode*	pPartyScm = NewAndGetPartyCntlScmForFallback(partyControInitParam, partyControlDataParams);

 	SetSeviceIdForConfParty(pConfParty);

	CSipCaps* pSipCaps = new CSipCaps;
	DWORD vidBitrate;
	SetSIPPartyCapsAndVideoParam(pPartyScm, pSipCaps, pConfParty, vidBitrate,NO /*((CSipNetSetup*)pIpNetSetup)->GetEnableSipICE()*/,NULL,  0, FALSE, pConfParty->GetServiceId(), partyControInitParam, partyControlDataParams);

	if (pScm->GetIsEncrypted())
	{
		CopyNoneTipEncryptionParams(pPartyScm, pScm);
	}

	*m_pIpInitialMode 	= *pPartyScm;
	*m_pSipLocalCaps 	= *pSipCaps;

	if(!m_bIsOfferer)
	{
	CSipComMode* pBestMode = NULL;
	BYTE bWithinProtocolLimitation = FALSE;
	pBestMode = m_pSipRemoteCaps->FindBestMode(cmCapReceiveAndTransmit, ((const CSipComMode&)*m_pIpInitialMode), ((const CSipCaps&)*m_pSipLocalCaps), bWithinProtocolLimitation/*fix protocol*/, m_bIsOfferer, FALSE);

	*m_pIpInitialMode = *pBestMode;
	POBJDELETE(pBestMode);
	}

	if (m_pPartyApi)
		m_pPartyApi->ChangeModeIp(m_pIpInitialMode, eFallBackFromTipToSip, NO/*content speaker*/, m_pSipLocalCaps);

	POBJDELETE(pPartyScm);
	POBJDELETE(pSipCaps);

}

BYTE CSipPartyCntl::IsNeedToChangeVideoResourceAllocation(eVideoPartyType eCurrentVideoPartyType)
{
    BOOL bSplitTIPVideoResources = GetSystemCfgFlagInt<BOOL>( CFG_KEY_SPLIT_TIP_VIDEO_RESOURCES );

    if( bSplitTIPVideoResources && IsTipVideoPartyType() )
		return FALSE;

    return CIpPartyCntl::IsNeedToChangeVideoResourceAllocation(eCurrentVideoPartyType);
}
//////////////////////////////////////////////////////////////////////////////////////
eVideoPartyType CSipPartyCntl::GetLocalVideoPartyType(BYTE partyTypeWithH263)
{
    BOOL bSplitTIPVideoResources = GetSystemCfgFlagInt<BOOL>( CFG_KEY_SPLIT_TIP_VIDEO_RESOURCES );

	//TIP HD720p30 call needs port HD1080 (requested by Algo)
	if( GetIsTipCall() && IsTipVideoPartyType() && bSplitTIPVideoResources )
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipPartyCntl::GetLocalVideoPartyType tip party is HD1080.", GetPartyRsrcId());
		return eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type;
	}
	else
		return CIpPartyCntl::GetLocalVideoPartyType(partyTypeWithH263);
}
//////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsTipCompatibleContent()
{
	BYTE bIsTipCompatibleContent = FALSE;
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	bIsTipCompatibleContent = pCommConf->GetIsTipCompatibleContent();

	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::IsTipCompatibleContent : ", bIsTipCompatibleContent);
	return bIsTipCompatibleContent;
}
////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPartyCntl::IsTipVideoPartyType() const
{
	if( (eTipMasterCenter == m_TipPartyType || eTipSlaveLeft == m_TipPartyType || eTipSlaveRigth == m_TipPartyType) && GetIsTipCall() )
		return TRUE;
	return FALSE;

}
////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
ECascadePartyType  CSipPartyCntl::GetPartyCascadeTypeAndVendor()
{
	if (strstr(m_productId, "AV-MCU") || m_AvMcuLinkType != eAvMcuLinkNone )
		return eCascadeMasterToRmx;

	return eCascadeNone;
}

BYTE CSipPartyCntl::GetPartyCascadeType() const
{
	if (strstr(m_productId, "AV-MCU") || m_AvMcuLinkType != eAvMcuLinkNone  )
		return CASCADE_MCU;

	return CASCADE_NONE;
}



/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpNotificationFromBridgeReq(CSegment* pParam)//(const ScpStreamsNotificationParams& notificationParams,cmCapDataType aMediaType, ERoleLabel eRole)
{
	PTRACE(eLevelInfoNormal, "CSipPartyCntl::OnPartyCntlScpNotificationFromBridgeReq");

	CScpNotificationWrapper notifyReq;
	notifyReq.DeSerialize(NATIVE,*pParam);

	if ( notifyReq.m_remoteSequenceNumber != m_pScpHandler->GetLastScpRequestSeqNumber() )
	{
		char dumpStr[500];
		memset(dumpStr, 0, 500);

		sprintf( dumpStr,
				 "CSipPartyCntl::OnPartyCntlScpNotificationFromBridgeReq\nnotificationParams.m_remoteSeq (%d) != LastScpRequestSeqNumber (%d)",
				 notifyReq.m_remoteSequenceNumber,
				 m_pScpHandler->GetLastScpRequestSeqNumber() );

		PASSERT(1);
		PTRACE(eLevelInfoNormal, dumpStr);
		return;
	}

	m_pScpHandler->UpdateNotificationsMap(notifyReq);

	// send the notification
	m_pScpHandler->UpdateLastScpStreamsNotificationLocalSeqNumber();
	SendScpNotificationToParty(notifyReq.m_channelHandle);

	// start timer
	DWORD remoteSeqNum = m_pScpHandler->m_pScpStreamNewReq->GetSequenceNumber();
	StartScpNotificationTimer(0, notifyReq.m_channelHandle, remoteSeqNum);
}


/////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendScpNotificationToParty(unsigned int channelHandle)
{
	CScpNotificationWrapper notifyStruct;
	notifyStruct.m_channelHandle		= channelHandle;
	notifyStruct.m_remoteSequenceNumber	= m_pScpHandler->m_pScpStreamNewReq->GetSequenceNumber();
	notifyStruct.m_sequenceNumber		= m_pScpHandler->GetLastScpStreamsNotificationLocalSeqNumber();

	SCP_NOTIFICATIONS_TO_PARTY_MAP::const_iterator itr;
	std::list <CScpPipeWrapper>::const_iterator pipe_list_itr;
	for (itr = m_pScpHandler->m_notificationsMap.begin();itr != m_pScpHandler->m_notificationsMap.end();itr++)
	{
		std::list <CScpPipeWrapper>  pipes = itr->second;
		for(pipe_list_itr = pipes.begin();pipe_list_itr != pipes.end();pipe_list_itr++)
		{
			notifyStruct.AddPipe(*pipe_list_itr);
		}
	}

	PTRACE(eLevelInfoNormal,"_scp_flow_ CSipPartyCntl::SendScpNotificationToParty");
	notifyStruct.Dump("CSipPartyCntl::SendScpNotificationToParty");

	// send notification to party
	if (NULL == m_pPartyApi)
	{
		PTRACE(eLevelInfoNormal,"_scp_flow_ CSipPartyCntl::SendScpNotificationToParty - m_pPartyApi is NULL");
		PASSERT(1);
		return;
	}

	m_pPartyApi->SendScpNotificationToMrmpReq(&notifyStruct);
}

/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpNotificationReqTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::OnPartyCntlScpNotificationReqTout");

	DWORD iterationNum=0, channelHandle=0, remoteSeqNum=0;

	*pParam >> iterationNum;
	*pParam >> channelHandle;
	*pParam >> remoteSeqNum;


	if (MAX_SCP_RETRANSMISSIONS <= iterationNum)
	{
		TRACEINTOFUNC << "MAX_SCP_RETRANSMISSIONS (" << (std::dec) << MAX_SCP_RETRANSMISSIONS << ") has reached!";
		PASSERT(1);
		return;
	}

	DWORD lastSeqNum = m_pScpHandler->m_pScpStreamNewReq->GetSequenceNumber();
	if (remoteSeqNum != lastSeqNum)
	{
		TRACEINTOFUNC << "notification remote seq num (from timer): " << remoteSeqNum << ", while last Scp Request remote seq num is " << lastSeqNum;
		PASSERT(1);
		return;
	}

	// resend the notification
	SendScpNotificationToParty(channelHandle);

	// start timer
	++iterationNum;
	StartScpNotificationTimer(iterationNum, channelHandle, remoteSeqNum);
}

/////////////////////////////////////////////////////////////////
void CSipPartyCntl::StartScpNotificationTimer(DWORD iterationNum, DWORD channelHandle, DWORD remoteSeqNum)
{
	CSegment *pNotifySeg = new CSegment;
	*pNotifySeg << iterationNum;
	*pNotifySeg << channelHandle;
	*pNotifySeg << remoteSeqNum;

	StartTimer(SCP_NOTIFICATION_REQ_TOUT, 1*SECOND, pNotifySeg);
}

/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnConfApiScpNotificationAckFromEP(CSegment* pParam)
{
	DWORD channelHandle		= 0;
	DWORD remoteSequenseNumber	= 0;
	DWORD bIsAck				= FALSE;

	*pParam >> channelHandle
	>> remoteSequenseNumber
	>> bIsAck;

	// find the notification that this ACK belongs to according to seq number
	if (remoteSequenseNumber == m_pScpHandler->GetLastScpStreamsNotificationLocalSeqNumber())
	{
		//ACK for last streams notification request
		TRACEINTOFUNC << " - received ACK for last streams notification request, remoteSeqNum: " << remoteSequenseNumber << ", IsAck: " << (bIsAck? "TRUE" : "FALSE") << "( ==> DeleteTimer(SCP_NOTIFICATION_REQ_TOUT))";
		if ( IsValidTimer(SCP_NOTIFICATION_REQ_TOUT) )
			DeleteTimer(SCP_NOTIFICATION_REQ_TOUT);
	}
	else if (remoteSequenseNumber == m_pScpHandler->GetLastScpIvrStateNotificationLocalSeqNumber())
	{	//ACK for last ivr state notification request
		TRACEINTOFUNC << " - received ACK for last ivr state notification request, remoteSeqNum: " << remoteSequenseNumber << ", IsAck: " << (bIsAck? "TRUE" : "FALSE") << "( ==> DeleteTimer(SCP_IVR_STATE_NOTIFICATION_REQ_TOUT))";
		if ( IsValidTimer(SCP_IVR_STATE_NOTIFICATION_REQ_TOUT) )
			DeleteTimer(SCP_IVR_STATE_NOTIFICATION_REQ_TOUT);
		ScpIvrStateNotification lastIvr = m_pScpHandler->GetLastScpIvrStateNotification();
		BYTE isAck = (bIsAck)?1:0;
		if (lastIvr.m_ivrState == eIvrStateSlideOn)
		{
			m_pVideoBridgeInterface->AckOnIvrScpShowSlide(m_pParty->GetPartyId(),isAck );
		}
		else if(lastIvr.m_ivrState == eIvrStateSlideOff)
		{
			m_pVideoBridgeInterface->AckOnIvrScpStopShowSlide(m_pParty->GetPartyId(),isAck);
		}
	}
	else if (remoteSequenseNumber == m_pScpHandler->GetLastScpPipesMappingNotificationLocalSeqNumber())
	{	//ACK for last pipes mapping notification request
		TRACEINTOFUNC << " - received ACK for last pipes mapping notification request, remoteSeqNum: " << remoteSequenseNumber << ", IsAck: " << (bIsAck? "TRUE" : "FALSE") << "( ==> DeleteTimer(SCP_PIPES_MAPPING_NOTIFICATION_REQ_TOUT))";
		if ( IsValidTimer(SCP_PIPES_MAPPING_NOTIFICATION_REQ_TOUT) )
			DeleteTimer(SCP_PIPES_MAPPING_NOTIFICATION_REQ_TOUT);
	}
	else
	{
		TRACEINTOFUNC << " - received ACK for unknown notification request (might be old one), remoteSeqNum: " << remoteSequenseNumber << ", IsAck: " << (bIsAck? "TRUE" : "FALSE") << "( ==> Do Nothing)";
	}
}

/////////////////////////////////////////////////////////////////
DWORD CSipPartyCntl::GetScpNotificationRemoteSeqNumber()
{
	DWORD retVal = 0;

	if ( (m_pScpHandler) && (m_pScpHandler->m_pScpStreamNewReq) )
	{
		retVal = m_pScpHandler->m_pScpStreamNewReq->GetSequenceNumber();
	}

	return retVal;
}
/////////////////////////////////////////////////////////////////
void  CSipPartyCntl::UpdateVideoOutBridgeH239Case(BYTE bTakeInitial)
{
    if (!m_bIsMrcCall)
    {
        CIpPartyCntl::UpdateVideoOutBridgeH239Case(bTakeInitial);
        return;
    }

    TRACEINTOFUNC << "PartId: " << this->GetPartyId() << " update video out rate for mrc.";
    m_pIpInitialMode->Dump("mix_mode: Before ScpHandler", eLevelInfoNormal);

    DWORD contentRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
    const std::list <StreamDesc> streamsDescList = m_pIpInitialMode->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);

    std::list <StreamDesc>::const_iterator itr_streams;
    int numberOfStreams = streamsDescList.size();
    CMrmpStreamDescWrap* pMediaStreams = new CMrmpStreamDescWrap[numberOfStreams];
    int i = 0;
    for(itr_streams = streamsDescList.begin(); itr_streams != streamsDescList.end() ;itr_streams++, i++)
    {
        pMediaStreams[i] = *itr_streams;
    }

    m_pScpHandler->CalculateDesiredBitRate(pMediaStreams, numberOfStreams, m_pIpInitialMode, m_pSipLocalCaps);
    m_pScpHandler->UpdateScm(pMediaStreams, numberOfStreams, m_pIpInitialMode, cmCapTransmit);
    delete []pMediaStreams;
    if (IP_CONNECTED == m_connectingState)
    {
        if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit))
        {
            // @#@ update Party with new SCM streams
            m_pPartyApi->UpdateScmStreams(m_pIpInitialMode);

            m_pSipLocalCaps->Dump("CIpPartyCntl::UpdateVideoRelayOutBridgeIfNeeded localCaps ",eLevelInfoNormal);
            UpdateVideoRelayOutBridgeIfNeeded(TRUE,TRUE); //
            *m_pIpCurrentMode = *m_pIpInitialMode;
            //UpdateVideoBridgeIfNeeded();
        }

    }
}

/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpIvrShowSlideFromBridgeReqANYCASE(CSegment* pParam)
{
	TRACEINTOFUNC << "SCP_IVR_SHOW_SLIDE_REQ arrived";
	OnPartyCntlScpIvrMessageFromBridge(eIvrStateSlideOn);


}
/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpIvrStopShowSlideFromBridgeReqANYCASE(CSegment* pParam)
{
	TRACEINTOFUNC << "SCP_IVR_STOP_SHOW_SLIDE_REQ arrived";
	OnPartyCntlScpIvrMessageFromBridge(eIvrStateSlideOff);
}
/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpIvrMessageFromBridge(eIvrState newState)
{
	m_pScpHandler->UpdateLastScpIvrStateNotification(newState);

	const ScpIvrStateNotification lastScpIvrStateNotification = m_pScpHandler->GetLastScpIvrStateNotification();
	SendScpIvrStateNotificationToParty(lastScpIvrStateNotification);

	StartScpIvrStateNotificationTimer(0, lastScpIvrStateNotification.m_localSeqNum);
}
/////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendScpIvrStateNotificationToParty(const ScpIvrStateNotification& rScpIvrStateNotification)
{
	if (NULL == m_pPartyApi)
	{
		PTRACE(eLevelInfoNormal,"_scp_flow_ CSipPartyCntl::SendScpIvrStateNotificationToParty - m_pPartyApi is NULL");
		PASSERT(1);
		return;
	}

	m_pPartyApi->SendScpIvrStateNotificationReqToParty(rScpIvrStateNotification.m_ivrState, rScpIvrStateNotification.m_localSeqNum);
}
/////////////////////////////////////////////////////////////////
void CSipPartyCntl::StartScpIvrStateNotificationTimer(DWORD iterationNum, DWORD notification_seq_num)
{
	CSegment *pIvrStateNotifySeg = new CSegment;
	*pIvrStateNotifySeg << iterationNum;
	*pIvrStateNotifySeg << notification_seq_num;

	StartTimer(SCP_IVR_STATE_NOTIFICATION_REQ_TOUT, IVR_RETRANSMISSION_TOUT, pIvrStateNotifySeg);
}
/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpIvrStateNotificationReqToutANYCASE(CSegment* pParam)
{
	DWORD iterationNum=0, remoteSeqNum=0;

	*pParam >> iterationNum;
	*pParam >> remoteSeqNum;

	TRACEINTOFUNC << "iterationNum: " << iterationNum << ", remoteSeqNum:" << remoteSeqNum;

	if (MAX_IVR_SCP_RETRANSMISSIONS <= iterationNum)
	{
		std::string errStr = "CSipPartyCntl::OnPartyCntlScpIvrStateNotificationReqToutANYCASE - MAX_IVR_SCP_RETRANSMISSIONS (";
		errStr += MAX_IVR_SCP_RETRANSMISSIONS;
		errStr += ") has reached!";

		PASSERT(1);
		PTRACE(eLevelInfoNormal, errStr.c_str());
		return;
	}
	const ScpIvrStateNotification lastScpIvrStateNotification = m_pScpHandler->GetLastScpIvrStateNotification();
	// resend the notification
	SendScpIvrStateNotificationToParty(lastScpIvrStateNotification);

	// start timer
	++iterationNum;
	StartScpIvrStateNotificationTimer(iterationNum, lastScpIvrStateNotification.m_localSeqNum);

}

////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpPipesMappingNotificationFromBridgeReq(CSegment* pParam)
{
	if (!m_bIsMrcCall)
	{
		// CONF_PARTY_MRMP_SCP_PIPES_MAPPING_NOTIFICATION_REQ should not be sent to AVC participant
		return;
	}

	// stop previous timer, if exists
	if ( IsValidTimer(SCP_PIPES_MAPPING_NOTIFICATION_REQ_TOUT) )
		DeleteTimer(SCP_PIPES_MAPPING_NOTIFICATION_REQ_TOUT);

	CScpPipeMappingNotification pipeMappingNotifyReq;
	pipeMappingNotifyReq.DeSerialize(NATIVE,*pParam);

	PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnPartyCntlScpPipesMappingNotificationFromBridgeReq");
//	pipeMappingNotifyReq.Dump();

	unsigned int remoteSeqNum  = (unsigned int)( pipeMappingNotifyReq.GetRemoteSeuqenceNumber() );
	unsigned int lastReqSeqNum = m_pScpHandler->GetLastScpRequestSeqNumber();
	if (remoteSeqNum != lastReqSeqNum)
	{
		TRACEINTOFUNC << "pipeMappingNotifyReq.m_remoteSeq (" << remoteSeqNum << ")"
					  << " != LastScpRequestSeqNumber (" << lastReqSeqNum << ")";

		PASSERT(1);
		return;
	}

//	m_pScpHandler->UpdateNotificationsMap(notifyReq);

	// send the notification
	m_pScpHandler->UpdateLastScpPipesMappingNotificationLocalSeqNumber();
	pipeMappingNotifyReq.SetSequenceNumber( m_pScpHandler->GetLastScpPipesMappingNotificationLocalSeqNumber() );
	SendScpPipesMappingNotificationToParty(&pipeMappingNotifyReq);

	// start timer
	StartScpPipesMappingNotificationTimer(0, &pipeMappingNotifyReq);
}

/////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendScpPipesMappingNotificationToParty(CScpPipeMappingNotification* pPipesMapStruct)
{
/*
//	CScpNotificationWrapper notifyStruct;
//	notifyStruct.m_channelHandle		= channelHandle;
//	notifyStruct.m_remoteSequenceNumber	= m_pScpHandler->m_pScpStreamNewReq->GetSequenceNumber();
//	notifyStruct.m_sequenceNumber		= updatedLocalSeqNum;
//
//	SCP_NOTIFICATIONS_TO_PARTY_MAP::const_iterator itr;
//	std::list <CScpPipeWrapper>::const_iterator pipe_list_itr;
//	for (itr = m_pScpHandler->m_notificationsMap.begin();itr != m_pScpHandler->m_notificationsMap.end();itr++)
//	{
//		std::list <CScpPipeWrapper>  pipes = itr->second;
//		for(pipe_list_itr = pipes.begin();pipe_list_itr != pipes.end();pipe_list_itr++)
//		{
//			notifyStruct.AddPipe(*pipe_list_itr);
//		}
//	}
//
//	PTRACE(eLevelInfoNormal,"_scp_flow_ CSipPartyCntl::SendScpPipesMappingNotificationToParty");
*/

	PTRACE(eLevelInfoNormal,"CIpPartyCntl::SendScpPipesMappingNotificationToParty");
	pPipesMapStruct->Dump();

	// send notification to party
	if (NULL == m_pPartyApi)
	{
		TRACEINTOFUNC << "m_pPartyApi is NULL";
		PASSERT(1);
		return;
	}

	m_pPartyApi->SendScpPipesMappingNotificationToMrmpReq(pPipesMapStruct);
}

/////////////////////////////////////////////////////////////////
void CSipPartyCntl::StartScpPipesMappingNotificationTimer(DWORD iterationNum, CScpPipeMappingNotification* pPipesMapStruct)
{
	CSegment *pNotifySeg = new CSegment;
	*pNotifySeg << iterationNum;
	pPipesMapStruct->Serialize(NATIVE,*pNotifySeg);

	StartTimer(SCP_PIPES_MAPPING_NOTIFICATION_REQ_TOUT, 1*SECOND, pNotifySeg);
}

/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlScpPipesMappingNotificationReqTout(CSegment* pParam)
{
	if (!m_bIsMrcCall)
	{
		// CONF_PARTY_MRMP_SCP_PIPES_MAPPING_NOTIFICATION_REQ should not be sent to AVC participant
		return;
	}

	PTRACE(eLevelInfoNormal,"CIpPartyCntl::OnPartyCntlScpPipesMappingNotificationReqTout");

	DWORD iterationNum = 0;
	*pParam >> iterationNum;

	if (MAX_SCP_RETRANSMISSIONS <= iterationNum)
	{
		// Old EPs(or MARS), which do not support PipesMappingNotification, will reach this timeout a lot of times (on each videosources request)
		//    so it decided to remove the ASSERT.
//		DBGPASSERT(1);
		TRACEINTOFUNC << "MAX_SCP_RETRANSMISSIONS (" << MAX_SCP_RETRANSMISSIONS << ") has reached!";
		return;
	}

	CScpPipeMappingNotification pipeMappingNotifyReq;
	pipeMappingNotifyReq.DeSerialize(NATIVE,*pParam);

	DWORD lastSeqNum	= m_pScpHandler->m_pScpStreamNewReq->GetSequenceNumber();
	DWORD remoteSeqNum	= pipeMappingNotifyReq.GetRemoteSeuqenceNumber();
	if (remoteSeqNum != lastSeqNum)
	{
		TRACEINTOFUNC << "Nnotification remote seq num (from timer): " << remoteSeqNum
					  << ", while last Scp Request remote seq num is " << lastSeqNum;

        // ASSERT is removed since Bridge may send a new StreamsRequest without sending PipesMapping,
		//     so it's a valid scenario although the sequence numbers are not equal
		// PASSERT(1);

		return;
	}

	// resend the notification
	SendScpPipesMappingNotificationToParty(&pipeMappingNotifyReq);

	// start timer
	++iterationNum;
	StartScpPipesMappingNotificationTimer(iterationNum, &pipeMappingNotifyReq);
}

/////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SendDisconnectMessageFromMasterToSlaves(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CIpPartyCntl::SendDisconnectMessageFromMasterToSlaves - this function can only be called in del state do nothing");
}
/////////////////////////////////////////////////////////////////
void CSipPartyCntl::UpdateSvcSipPartyConnectedInCdr(CIpComMode* pIpCurrentMode)
{
    const std::list <StreamDesc> streamsDescList = pIpCurrentMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
 //   TRACEINTO << "BG_CDR  streamsDescList size = " << (int)streamsDescList.size();
    CapEnum algorithm = (CapEnum)(pIpCurrentMode->GetMediaType(cmCapAudio, cmCapReceive));
    ECodecSubType eAudioCodec = CapEnum2ECodecSubType(algorithm);

	DWORD dwBitRateOut = 0;
	DWORD dwBitRateIn = 0;

	DWORD audioOut        = 0;
	DWORD audioIn         = 0;
	DWORD videoPeopleout  = 0;
	DWORD videoPeopleIn   = 0;
	DWORD videoContentout = 0;
	DWORD videoContentIn  = 0;
	if (!pIpCurrentMode->IsMediaOff(cmCapAudio, cmCapTransmit))
		audioOut = pIpCurrentMode->GetMediaBitRate(cmCapAudio, cmCapTransmit);
	if (!pIpCurrentMode->IsMediaOff(cmCapAudio, cmCapReceive))
		audioIn = pIpCurrentMode->GetMediaBitRate(cmCapAudio, cmCapReceive);
	if (!pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapReceive, kRolePeople))
		videoPeopleIn = (DWORD)((pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRolePeople)) / 10);
	if (!pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople))
		videoPeopleout = (DWORD)((pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople))/10);
	if (!pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapReceive, kRoleContentOrPresentation))
		videoContentIn = (DWORD)((pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRoleContentOrPresentation)) /10);
	if (!pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
		videoContentout = (DWORD)((pIpCurrentMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation)) /10);

	dwBitRateOut = audioOut + videoPeopleout + videoContentout;
	dwBitRateIn = audioIn + videoPeopleIn + videoContentIn;

	m_pTaskApi->UpdateSvcSipPartyInfoInCdr(m_pParty, &streamsDescList, eAudioCodec, dwBitRateOut, dwBitRateIn);
}
/////////////////////////////////////////////////////////////////
ECodecSubType CSipPartyCntl::CapEnum2ECodecSubType(CapEnum algorithm)
{
	if(algorithm == eH264CapCode)
		return eH264;
	else if(algorithm == eSvcCapCode)
		return eH264SVC;
	else if(algorithm == eSirenLPR_Scalable_32kCapCode || algorithm == eSirenLPR_Scalable_48kCapCode || algorithm == eSirenLPR_Scalable_64kCapCode
			|| algorithm == eSirenLPRStereo_Scalable_64kCapCode || algorithm == eSirenLPRStereo_Scalable_96kCapCode || algorithm == eSirenLPRStereo_Scalable_128kCapCode)
		return eSAC;
	else if(algorithm == eG711Alaw64kCapCode || algorithm == eG711Alaw56kCapCode || algorithm == eG711Ulaw64kCapCode || algorithm == eG711Ulaw56kCapCode)
		return eG711;
	else if(algorithm == eG722_64kCapCode || algorithm == eG722_56kCapCode || algorithm == eG722_48kCapCode)
		return eG722;
	else if(algorithm == eG7221_32kCapCode || algorithm == eG7221_24kCapCode || algorithm == eG7221_16kCapCode
			|| algorithm == eG7221C_48kCapCode || algorithm == eG7221C_32kCapCode || algorithm == eG7221C_24kCapCode || algorithm == eG7221C_CapCode)
		return eG722;
	return eCodecSubTypeUndefined;
}

//_t_p_
/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnChangeVideoOutTipPolycom(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnChangeVideoOutTipPolycom");
	m_bIsNeedToChangeVideoOutTipPolycom = TRUE;

	if (m_eVidBridgeConnState & eSendOpenOut)
	{
		if (UpdateVideoOutBridgeIfNeeded(TRUE, TRUE))
		{
			TRACEINTO<<"###! updateBridges eUpdateVideoOut ";
		}
		else
			TRACEINTO<<"###! updateBridges NOT eUpdateVideoOut ";
	}
}

void CSipPartyCntl::OnPartyCntlUpdateArtWithSsrc(CSegment* pParam)
{
    TRACEINTOFUNC << "mix_mode: UPDATE_ART_WITH_SSRC_REQ arrived";
    DWORD ssrc = 0;
    *pParam >> ssrc;

    // send message to ART
    m_pPartyApi->UpdateArtOnTranslateVideoSSRC(ssrc);
}

/////////////////////////////////////////////////////////////////
BOOL CSipPartyCntl::GetEnableICE()
{
	if (!m_pSIPNetSetup)
		return FALSE;
	return m_pSIPNetSetup->GetEnableSipICE();
}
void CSipPartyCntl::OnMrmpStreamIsMustReq(CSegment* pParam)
{
	TRACEINTO<<"got req CONF_PARTY_MRMP_STREAM_IS_MUST_REQ";
	CSegment* pSeg=new CSegment(*pParam);
	m_pPartyApi->MrmpStreamIsMustReq(pSeg);
	POBJDELETE(pSeg);
}


//_e_m_
/////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyCntlUpdateVidBrdgTelepresenseEPInfo(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnPartyCntlUpdateVidBrdgTelepresenseEPInfo");

	PASSERTMSG_AND_RETURN(!m_telepresenseEPInfo, "m_telepresenseEPInfo == NULL");

	CTelepresenseEPInfo telepresenseEPInfo;

	telepresenseEPInfo.DeSerialize(NATIVE, *pParam);

	CMedString str = "";
	str << " EPtype " 			<< (int)telepresenseEPInfo.GetEPtype()
		<< " LinkNum " 			<< (int)telepresenseEPInfo.GetLinkNum()
		<< " LinkRole " 		<< (int)telepresenseEPInfo.GetLinkRole()
		<< " NumOfLinks " 		<< (int)telepresenseEPInfo.GetNumOfLinks()
		<< " PartyMonitorID " 	<< (int)telepresenseEPInfo.GetPartyMonitorID()
		<< " RoomID " 			<< (int)telepresenseEPInfo.GetRoomID();
	PTRACE2(eLevelInfoNormal, "EMB_MLA : CCSipPartyCntl::OnPartyCntlUpdateVidBrdgTelepresenseEPInfo - telepresenseEPInfo: ", str.GetString());

	m_telepresenseEPInfo->SetLinkNum(telepresenseEPInfo.GetLinkNum());
	m_telepresenseEPInfo->SetLinkRole(telepresenseEPInfo.GetLinkRole());
	m_telepresenseEPInfo->SetNumOfLinks(telepresenseEPInfo.GetNumOfLinks());
	m_telepresenseEPInfo->SetPartyMonitorID(telepresenseEPInfo.GetPartyMonitorID());
	m_telepresenseEPInfo->SetRoomID(telepresenseEPInfo.GetRoomID());

	UpdateVidBrdgTelepresenceEPInfoIfNeeded(telepresenseEPInfo.GetEPtype());

	m_pTaskApi->UpdateDB(m_pParty, PARTYTELEPRESENCEMODE, (DWORD)telepresenseEPInfo.GetEPtype(), 1, NULL);
}
///////////////////////////////////////////////////////////////////////
DWORD CSipPartyCntl::GetFirs2013Ssrc(BYTE index)
{
	return m_pSipLocalCaps->getMsftSsrcVideoFirst(index);
}

//BRIDGE-10123
///////////////////////////////////////////////////////////////////////////////BRIDGE-11708
DWORD CSipPartyCntl::GetUseMkiEncrytion(CConfParty* pConfParty, BYTE IsOfferer)
{
	DWORD encryptionKeyToUse = eUseBothEncryptionKeys;

	PASSERTMSG_AND_RETURN_VALUE(!pConfParty, "!pConfParty", encryptionKeyToUse)

	RemoteIdent remoteIdent 	= pConfParty->GetRemoteIdent();
	BOOL bIsMrcCall 			= m_bIsMrcCall;
	BOOL bIsCiscoTagExist 		= pConfParty->GetIsCiscoTagExist();
	BOOL bIsTandbergEp			= remoteIdent == TandbergEp;
	BOOL bIsRvEp           		= remoteIdent == RvEp;
	BOOL bWithMki				= FALSE;
	EEncryptionKeyToUse mkiFlag = CSipCaps::GetUseMkiEncrytionFlag();


	if(IsOfferer && mkiFlag != eUseBothEncryptionKeys)
		encryptionKeyToUse = mkiFlag;

	else if(m_lyncRedialOutAttempt || pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone){
		encryptionKeyToUse = eUseMkiKeyOnly;
		if(m_lyncRedialOutAttempt){
			TRACEINTO << "LYNC_REDIAL setting encryptionKeyToUse = eUseMkiKeyOnly for lync redial";
		}
	}

	else if(bIsCiscoTagExist || bIsTandbergEp || bIsRvEp || bIsMrcCall)
		encryptionKeyToUse = eUseNonMkiKeyOnly;

	else if(!IsOfferer && remoteIdent == Regular)
		encryptionKeyToUse = mkiFlag;

	bWithMki = (encryptionKeyToUse != eUseNonMkiKeyOnly);

	TRACEINTO << " encryptionKeyToUse " << (int)encryptionKeyToUse << " bWithMki " << (int)bWithMki <<" bIsCiscoTagExist " << (int)bIsCiscoTagExist << " bIsTandbergEp " << (int)bIsTandbergEp
			  << " bIsRvEp " << (int)bIsRvEp << " bIsMrcCall " << (int)bIsMrcCall << " m_lyncRedialOutAttempt " << (int)m_lyncRedialOutAttempt
			  << " remoteIdent " << (int)remoteIdent << " IsOfferer " << (int)IsOfferer << " mkiFlag " << (int)mkiFlag ;

	return encryptionKeyToUse;
}

///////////////////////////////////////////////////////////////////////////////BRIDGE-11708
BOOL CSipPartyCntl::GetUseNonMkiEncryptOrderFirst(CConfParty* pConfParty, BYTE IsOfferer)
{
	PASSERTMSG_AND_RETURN_VALUE(!pConfParty, "!pConfParty", FALSE);

	BOOL bUseNonMkiEncryptOrderFirst = FALSE;
	WORD confPartyConnectionType 	 = -1;
	BOOL bIsInviteNoSdp 			 = FALSE;
	BOOL bPlcmReqMaskAvailable		 = FALSE;
	BOOL bVeqFlow 					 = FALSE;


	confPartyConnectionType 	= TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
	bIsInviteNoSdp 		 		= (confPartyConnectionType == DIALIN && IsOfferer);
	bPlcmReqMaskAvailable	 	= (pConfParty->GetPlcmRequireMask())?TRUE:FALSE;
	bVeqFlow 				 	= bIsInviteNoSdp && bPlcmReqMaskAvailable;
	bUseNonMkiEncryptOrderFirst = bVeqFlow;

	TRACEINTO << " bUseNonMkiEncryptOrderFirst " << (int)bUseNonMkiEncryptOrderFirst << " bVeqFlow " << (int)bVeqFlow << " bIsInviteNoSdp " << (int)bIsInviteNoSdp << " IsOfferer " << (int)IsOfferer << " bPlcmReqMaskAvailable " << (int)bPlcmReqMaskAvailable;

	return bUseNonMkiEncryptOrderFirst;
}
/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SetSIPPartyCapsAndVideoParam(CIpComMode* pPartyScm, CSipCaps* pCap, CConfParty* pConfParty, DWORD& vidBitrate, BOOL bEnableIce,  CSipNetSetup* pNetSetup , DWORD setupRate,BYTE IsOfferer, DWORD serviceId,PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
    PASSERTMSG_AND_RETURN(NULL == pConfParty, "NULL == pConfParty");
    PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::SetSIPPartyCapsAndVideoParam - IsTipCall:", pConfParty->GetIsTipCall());
    WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());

	// BRIDGE-13499 - CESC / Microsoft / June 2014 Train / RMX: RMX not able to dial out lync mobility client
	// remove FECC caps on lync redial (added at start of function before CalculateRateForIpCalls
	if(m_lyncRedialOutAttempt){
		pPartyScm->SetMediaOff(cmCapData,cmCapReceiveAndTransmit);
		TRACEINTO << " LYNC_REDIAL remove FECC capabilities on lync redial";
	}

	BOOL bReduceAudioCodecs = FALSE;  // Temporary solution for reducing audio codecs. In future will be implemented by Re-Invite.

	if ((confPartyConnectionType == DIALOUT || IsOfferer)
		&& GetSystemCfgFlagInt<BOOL>(serviceId, CFG_KEY_SIP_REDUCE_AUDIO_CODECS_DECLARATION))
	{
		bReduceAudioCodecs = TRUE;
	}

	//Shmulik Y: patch to fix VNGR-18913
	DWORD callRate = pPartyScm->GetCallRate() * 1000;

	if ((confPartyConnectionType == DIALOUT || IsOfferer) && (rate128K == callRate))
	{
		PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::SetSIPPartyCapsAndVideoParam setting Audio cap to eG7221_32k on 128K conf rate (if it's not SAC already)");
		pPartyScm->SetAudioAlg(eG7221_32kCapCode, cmCapReceiveAndTransmit);
	}
	
	pPartyScm->SetAudioAlg(TRUE, vidBitrate, pConfParty->GetName(), NO, bReduceAudioCodecs);// if the audio algorithm is auto, we set it to the preffered one.
	
	if (pConfParty->GetVoice())
	{
		pCap->SetAudio(pPartyScm, vidBitrate, YES, pConfParty->GetName());

		if (bReduceAudioCodecs)
			pCap->RemoveAudioCapsAccordingToList();

		return;
	}

	if (bReduceAudioCodecs) // should recalc the video rate (part of the temporary solution).
	{
		DWORD confRate = 0;
		int val;
		val = ::CalculateRateForIpCalls(pConfParty, pPartyScm, confRate, vidBitrate, partyControInitParam.bIsEncript);

		if (val == -1)
			PASSERT(1);

		vidBitrate = vidBitrate / 100;// divided by 100 because is used in system in

		BOOL isPreventOverflow = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_PREVENT_OVERFLOW);

		if (!isPreventOverflow)
		{
			 DWORD audioRate = pPartyScm->GetMediaBitRate(cmCapAudio);
			 DWORD callRate = partyControlDataParams.callRate;

			 if (audioRate > 48)
				 vidBitrate = (callRate - 48*_K_)/100;
		}

		if (pPartyScm->GetConfType() != kVideoSwitch && kVSW_Fixed != pPartyScm->GetConfType() &&
		    !m_bIsMrcCall && pPartyScm->GetConfMediaType()!=eMixAvcSvcVsw)
			pPartyScm->SetVideoBitRate(vidBitrate, cmCapReceiveAndTransmit);
	}

	// set scm with conf video quality
	eVideoQuality vidQuality = partyControlDataParams.vidQuality;
	ePresentationProtocol contentProtocolMode = (ePresentationProtocol)partyControlDataParams.presentationProtocol;

	BYTE LinkCascadeMode = pConfParty->GetCascadeMode();

	if (partyControlDataParams.bContentMultiResolutionEnabled)
	{
		PTRACE(eLevelInfoNormal,
				"CSipPartyCntl::SetSIPPartyCapsAndVideoParam This is XCode Conf need to change content protocol mode");

		//If Link
		if (LinkCascadeMode != CASCADE_MODE_NONE)
		{
			PTRACE(eLevelInfoNormal,
					"CSipPartyCntl::SetSIPPartyCapsAndVideoParam  this is Link - set to eH264Fix ");
			contentProtocolMode = eH264Fix;
		}
		else // Not link!
		{
			if (partyControlDataParams.bContentXCodeH263Supported)
			{
				PTRACE(eLevelInfoNormal,
						"CSipPartyCntl::SetSIPPartyCapsAndVideoParam set to ePresentationAuto ");
				contentProtocolMode = ePresentationAuto;
			}
			else
			{
				PTRACE(eLevelInfoNormal,
						"CSipPartyCntl::SetSIPPartyCapsAndVideoParam set to eH264Dynamic ");
				contentProtocolMode = eH264Dynamic;
			}
		}
	}

	pPartyScm->SetContentProtocolMode(contentProtocolMode);

	BYTE bCreateNewScm = 0;
	DWORD confRate = 0;

	if (pPartyScm->GetConfType() == kCp || pPartyScm->GetConfType() == kCop)
	{
		ChangeScmOfIpPartyInCpOrCopConf(pPartyScm, pConfParty, pConfParty->GetVideoProtocol(), setupRate, bCreateNewScm,confRate,confPartyConnectionType,serviceId,partyControlDataParams, pNetSetup);
		ChangeVideoBitrateForTipIfNeeded(pConfParty, pPartyScm,  vidBitrate); //BRIDGE-5753
	}
	else
	{// vsw / hd conf
		// srt h.239 rate from cosetupRatenf rate
		if (partyControlDataParams.bIsEnableH239)
		{
			// Find the Max content rate of the conf
		   	DWORD H323AMCRate = partyControlDataParams.h323MaxContentAMCRate;

			if (partyControlDataParams.h323MaxContentAMCRate == 0)
			{
                PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::SetSIPPartyCapsAndVideoParam - BFCP is disabled");
				pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
				pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::SetSIPPartyCapsAndVideoParam");
				if (!partyControlDataParams.bIsTIPContentEnable)
				{
					pPartyScm->SetContent(H323AMCRate,cmCapReceiveAndTransmit,(CapEnum)partyControlDataParams.contentCap,partyControlDataParams.bIsHDContent1080Supported,partyControlDataParams.hdResMpi,partyControlDataParams.bIsHighProfileContent);
				}
				else if (partyControlDataParams.bIsPreferTIP)
				{
					pPartyScm->SetTIPContent(H323AMCRate,cmCapReceiveAndTransmit,FALSE);
				}
				else //eTipCompatibleVideoAndContent
				{
					pPartyScm->SetTIPContent(H323AMCRate,cmCapReceiveAndTransmit);
				}

				SetBfcpInSipPartyScm(pPartyScm);
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::SetSIPPartyCapsAndVideoParam - H.239 is disabled");
			pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		}
	}

	// Disable content if needed:
	BOOL bEnableSipPPC = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_SIP_PEOPLE_PLUS_CONTENT);

	BOOL bVeqFlowEnableBFCP = (!pConfParty->GetPlcmRequireMask() ||
							   ((pConfParty->GetPlcmRequireMask() && (pConfParty->GetPlcmRequireMask() & m_plcmRequireBfcpUdp || pConfParty->GetPlcmRequireMask() & m_plcmRequireBfcpTcp))));

	if (!bVeqFlowEnableBFCP || !bEnableSipPPC || bEnableIce)
		TRACEINTO << "ENABLE_SIP_PEOPLE_PLUS_CONTENT = " << int(bEnableSipPPC) << " bEnableIce = " << int(bEnableIce) << " bVeqFlowEnableBFCP = " << int(bVeqFlowEnableBFCP);

	// if ICE - remove bfcp and content for dial out only, so if remote fallback to none ICE, we'll have bfcp and content in scm and caps.
	if (!bEnableSipPPC || !bVeqFlowEnableBFCP ||  (bEnableIce && (confPartyConnectionType == DIALOUT || IsOfferer)))
	{
        PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::SetSIPPartyCapsAndVideoParam - disabling BFCP");
        m_partyContentRate = pPartyScm->GetContentBitRate(cmCapReceive);
		//pPartyScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);  //shira-debug
		pPartyScm->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	}

	// create with default caps and enable H263 4cif according to video parameters
	BYTE highestframerate= (BYTE)eCopVideoFrameRate_None;

	if (pPartyScm->GetConfType() == kCop)
	{
		CCOPConfigurationList* pCOPConfigurationList = partyControlDataParams.pCOPConfigurationList;
		CCopVideoParams* pCopHighestLevelParams = pCOPConfigurationList->GetVideoMode(0);
		highestframerate= pCopHighestLevelParams->GetFrameRate();
	}

	CSuperLargeString strCaps1;
	/*//BRIDGE-11708
	DWORD encryptionKeyToUse=eUseBothEncryptionKeys;
	if(m_lyncRedialOutAttempt==1)
	{
		encryptionKeyToUse=eUseMkiKeyOnly;
	}
	TRACEINTO<<"lyncCryptoLinesDebugBridge8497: encryptionKeyToUse: "<<(int)encryptionKeyToUse;
	 */

	//BRIDGE-11708
	DWORD encryptionKeyToUse 		  = eUseBothEncryptionKeys;
	BOOL  bUseNonMkiEncryptOrderFirst = FALSE;
	encryptionKeyToUse 				  = GetUseMkiEncrytion(pConfParty, IsOfferer); //BRIDGE-10123
	bUseNonMkiEncryptOrderFirst 	  = GetUseNonMkiEncryptOrderFirst(pConfParty, IsOfferer);

	BYTE isLync2013Call = FALSE;
	if ((((partyControlDataParams.lyncEpType == Lync2013 || partyControlDataParams.lyncEpType == AvMcuLync2013Slave) && IsFeatureSupportedBySystem(eFeatureMs2013SVC)) ||
			(pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 ||	pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown)))
	{
		isLync2013Call = TRUE;
	}

	//BRIDGE-11708
	pCap->Create(pPartyScm, vidBitrate, pConfParty->GetName(), vidQuality,serviceId,encryptionKeyToUse, ((ECopVideoFrameRate)highestframerate ), pConfParty->GetMaxResolution(), bUseNonMkiEncryptOrderFirst,isLync2013Call); //eyalnmic
	pCap->DumpToString(strCaps1);

	if (pPartyScm->GetConfMediaType()==eMixAvcSvcVsw)
	{
		PTRACE2(eLevelInfoNormal,"avc_vsw_relay: initialization local caps:",strCaps1.GetString());
	}

	strCaps1.Clear();

	if (bReduceAudioCodecs)
		pCap->RemoveAudioCapsAccordingToList();

    CCapSetInfo capInfo261 (eH261CapCode);
    CCapSetInfo capInfo263 (eH263CapCode);
    CCapSetInfo capInfo264 (eH264CapCode);
    CCapSetInfo capInfoVP8 (eVP8CapCode); //N.A. DEBUG VP8
    CCapSetInfo capInfoRtv (eRtvCapCode);
    CCapSetInfo capInfoMsSvc (eMsSvcCapCode);

    BYTE protocol = pConfParty->GetVideoProtocol();

    switch (protocol)
    {
		case(VIDEO_PROTOCOL_H261):
        {
            pCap->RemoveCapSet (capInfo263);
            pCap->RemoveCapSet (capInfo264);
            pCap->RemoveCapSet (capInfoVP8); //N.A. DEBUG VP8
            pCap->RemoveCapSet (capInfoRtv);
            pCap->RemoveCapSet (capInfoMsSvc);
            pCap->Set4CifMpi(-1);
            break;
        }
        case(VIDEO_PROTOCOL_H263):
        {
            pCap->RemoveCapSet (capInfo264);
            pCap->RemoveCapSet (capInfoRtv);
            pCap->RemoveCapSet (capInfo261);
            pCap->RemoveCapSet (capInfoVP8); //N.A. DEBUG VP8
            pCap->RemoveCapSet (capInfoMsSvc);
            break;
        }
        case(VIDEO_PROTOCOL_H264):
        {
        	pCap->RemoveCapSet (capInfoRtv);
            pCap->RemoveCapSet (capInfo263);
            pCap->RemoveCapSet (capInfoVP8); //N.A. DEBUG VP8
            pCap->RemoveCapSet (capInfo261);
            pCap->RemoveCapSet (capInfoMsSvc);
            pCap->Set4CifMpi(-1);
            break;
        }
        case(VIDEO_PROTOCOL_RTV):
        {
        	 pCap->RemoveCapSet (capInfo264);
        	 pCap->RemoveCapSet (capInfo263);
        	 pCap->RemoveCapSet (capInfoVP8); //N.A. DEBUG VP8
        	 pCap->RemoveCapSet (capInfo261);
             pCap->RemoveCapSet (capInfoMsSvc);
        	 pCap->Set4CifMpi(-1);
        	 break;
        }

		case(VIDEO_PROTOCOL_MS_SVC):
        {
        	 pCap->RemoveCapSet (capInfoRtv);
        	 pCap->RemoveCapSet (capInfo264);
        	 pCap->RemoveCapSet (capInfoVP8); //N.A. DEBUG VP8
        	 pCap->RemoveCapSet (capInfo263);
        	 pCap->RemoveCapSet (capInfo261);
        	 pCap->Set4CifMpi(-1);
        	 break;
        }
		case(VIDEO_PROTOCOL_VP8): //N.A. DEBUG VP8
		{
			 pCap->RemoveCapSet (capInfoRtv);
			 pCap->RemoveCapSet (capInfo264);
			 pCap->RemoveCapSet (capInfoMsSvc);
			 pCap->RemoveCapSet (capInfo263);
			 pCap->RemoveCapSet (capInfo261);
			 pCap->Set4CifMpi(-1);
			 break;
		}
    }

    if( pConfParty->GetRemoteIdent() == CiscoCucm )
        pCap->RemoveCapSet (capInfoRtv);

    // if we want to reduce video resolution for H263 calls when system.cfg flag is set (in the caps function)
	if (pPartyScm->GetConfType() == kCp)
	{
		DWORD dwRate_KB = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_CIF_IN_CP_FROM_RATE);
		if ( dwRate_KB > 0 )
		{
			if(dwRate_KB >= vidBitrate/10 )
			{
				pCap->FixCIFDeclarationForCP();
				pPartyScm->SetFormatMpi(kCif, -1, cmCapReceiveAndTransmit);
			}
		}
		Eh264VideoModeType vidMode = GetMaxVideoModeBySysCfg();
		eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
		Eh264VideoModeType vidModeAccordingtoRate = eHD1080At60Symmetric;
		BYTE partyResolution =  pConfParty->GetMaxResolution();
		BYTE maxConfResolution = partyControlDataParams.maxConfResolution;

		DWORD partyBitrate;
		if (pConfParty->GetVideoRate() == 0xFFFFFFFF)
			partyBitrate = pConfParty->GetVideoRate();
		else
			partyBitrate = pConfParty->GetVideoRate() * 1000;

		DWORD confBitrate = partyControlDataParams.callRate;

		if ((confBitrate!=0xFFFFFFFF && partyBitrate != confBitrate && partyResolution == eAuto_Res ) || pPartyScm->GetIsTipMode())
		{
			BYTE maxPartyResolution = ( partyResolution == eAuto_Res && maxConfResolution != eAuto_Res) ?
											maxConfResolution : partyResolution;
			Eh264VideoModeType partyMaxVideoMode = GetMaxVideoModeByResolutionType( (EVideoResolutionType)maxPartyResolution,vidQuality );
			if( partyBitrate != 0xFFFFFFFF )
			   	vidModeAccordingtoRate = CResRsrcCalculator::GetVideoMode( GetSystemCardsBasedMode(), partyBitrate, vidQuality, partyMaxVideoMode, FALSE); //False=BaseProfile
			else
			    vidModeAccordingtoRate = CResRsrcCalculator::GetVideoMode( GetSystemCardsBasedMode(), confBitrate, vidQuality, partyMaxVideoMode, FALSE);  //False=BaseProfile
			TRACEINTO << "partyBitrate:" << partyBitrate << ", confBitrate:" << confBitrate << ", videoMode:" << vidMode;
		}
		if(pPartyScm->GetConfType() == kCp && (vidMode == eCIF30 || (maxConfResolution == eCIF_Res && partyResolution == eAuto_Res) || partyResolution ==  eCIF_Res || vidModeAccordingtoRate == eCIF30) )
		{
			PTRACE(eLevelInfoNormal,"CSipAddPartyCntl::SetSIPPartyCapsAndVideoParam -  with max res = cif -remove 4cif fromh263 cap  ");
			pCap->Set4CifMpi (-1);
			pCap->Reomve4cifFromCaps();

			if(pPartyScm->GetMediaType(cmCapVideo,cmCapReceive) == eH263CapCode)
			{
				   pPartyScm->SetFormatMpi (k4Cif, -1, cmCapReceive, kRolePeople);
			}
		    if(pPartyScm->GetMediaType(cmCapVideo,cmCapTransmit) == eH263CapCode)
		    {
				 pPartyScm->SetFormatMpi (k4Cif, -1, cmCapTransmit, kRolePeople);
		    }
		}

		//Bridge-10151: considering fallback from TIP to SIP
		if (pPartyScm->GetIsTipMode() && confPartyConnectionType == DIALIN && partyControlDataParams.epType == PolycomEp)
		{
			eVideoPartyType eRmtVideoPartyType = eVideo_party_type_dummy;
			if (pNetSetup)
				eRmtVideoPartyType = pNetSetup->GetRemoteVideoPartyType();
			eVideoPartyType eLocalVideoPartyType =  CResRsrcCalculator::TranslateVideoTypeToResourceType(GetSystemCardsBasedMode(), vidModeAccordingtoRate);

			if (eLocalVideoPartyType >= eCP_H264_upto_CIF_video_party_type && eRmtVideoPartyType >= eCP_H264_upto_CIF_video_party_type)
				m_eMaxVideoPartyTypeForTipFailure = min(eLocalVideoPartyType, eRmtVideoPartyType);
			CLargeString cstr;
			cstr << "CSipAddPartyCntl::SetSIPPartyCapsAndVideoParam ";
			cstr << "Local Max VideoPartyType: " <<eVideoPartyTypeNames[eLocalVideoPartyType] << ", Remote Max VideoPartyType: " << eVideoPartyTypeNames[eRmtVideoPartyType];
			PTRACE(eLevelInfoNormal,cstr.GetString());
		}

	}

}

/////////////////////////////////////////////////////////////////////////////
CIpComMode* CSipPartyCntl::NewAndGetPartyCntlScmForFallback(
		PartyControlInitParameters &partyControInitParam,
		PartyControlDataParameters &partyControlDataParams) {
	CIpComMode*	pPartyScm = new CIpComMode(*(partyControInitParam.pConfIpScm));
	DWORD confRate = 0;
	DWORD vidBitrate;
	int val;
	val = ::CalculateRateForIpCalls(partyControInitParam.pConfParty, pPartyScm, confRate, vidBitrate, partyControInitParam.bIsEncript);
	if(val == -1)
		PASSERT(1);

	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::NewAndGetPartyCntlScmForFallback vidBitrate:",vidBitrate);
	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::NewAndGetPartyCntlScmForFallback confRate:",confRate);

	vidBitrate = vidBitrate / 100;// divided by 100 because is used in system in
	pPartyScm->SetVideoBitRate(vidBitrate, cmCapReceiveAndTransmit);

	return pPartyScm;

}

////////////////////////////////////////////////////////////////////////////////////////////
CSipNetSetup* CSipPartyCntl::NewAndSetupSipNetSetup(CConfParty* pConfParty,PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	CSipNetSetup* pIpNetSetup = NULL;

	if (pConfParty->GetConnectionType() == DIAL_OUT)
	{
		pIpNetSetup = new CSipNetSetup;
		//noa until here ttemp
		const char* strSipAddress = pConfParty->GetSipPartyAddress();
		WORD type = pConfParty->GetSipPartyAddressType();
		WORD port = pConfParty->GetCallSignallingPort();//5060 - ema bug.
		pIpNetSetup->SetRemoteDisplayName(pConfParty->GetName());
		pIpNetSetup->SetRemoteSipAddress(strSipAddress);
		pIpNetSetup->SetRemoteSipAddressType(type);
		pIpNetSetup->SetRemoteSignallingPort(port);

		if ((pConfParty->GetNodeType()) == 0)
		{
			pIpNetSetup->SetEndpointType(2);
		}
		else
		{
			if ((pConfParty->GetNodeType()) == 1)
			{
				pIpNetSetup->SetEndpointType(0);
			}
		}

		pIpNetSetup->SetMinRate(0);
		pIpNetSetup->SetConfId(partyControInitParam.monitorConfId);
	}
	else
		pIpNetSetup = (CSipNetSetup*)partyControlDataParams.pIpNetSetup;

	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();

	SetSeviceIdForConfPartyByConnectionType(pConfParty,pIpNetSetup);
    BOOL bMsEnviroment = FALSE;
	CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(pConfParty->GetServiceId());

	if (pService != NULL && pService->GetConfigurationOfSipServers())
	{
		if(pService->GetSipServerType() == eSipServer_ms)
			bMsEnviroment = TRUE;
	}

    TRACEINTO<<"m_bIsMrcCall: "<<(int)m_bIsMrcCall<<"bMsEnviroment: "<<(int)bMsEnviroment;


    if( (pConfParty->GetConnectionType() == DIAL_OUT || (pConfParty->GetConnectionType() == DIAL_IN && pIpNetSetup->GetEnableSipICE() == TRUE) ) && pConfPartyProcess->m_IceInitializationStatus == eIceStatusON && !(bMsEnviroment && m_bIsMrcCall) && !(pConfParty->GetIsCiscoTagExist()))
	{
		PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::NewAndSetupSipNetSetup WIth ICE!!");
		pConfParty->SetEnableICE(TRUE);

		// if ICE is enabled and MS env. and service IP type is ipv6 only
		if(pService && pService->GetSipServerType() == eSipServer_ms  )
		{
			if(pService->GetIPAddressTypesInService() == eIpType_IpV6)
			{
				TRACEINTO << "ICE is enabled and MS env. and service IP type is ipv6 only";
				pIpNetSetup->SetIpVersion(eIpVersion6);
			}
			else if(pService->GetIPAddressTypesInService() == eIpType_IpV4)
			{
				TRACEINTO << "ICE is enabled and MS env. and service IP type is ipv4 only";
				pIpNetSetup->SetIpVersion(eIpVersion4);
			}

		}
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::NewAndSetupSipNetSetup Without ICE!! -",
			pConfPartyProcess->m_IceInitializationStatus);
		pConfParty->SetEnableICE(FALSE);
	}

	pIpNetSetup->SetEnableSipICE(pConfParty->GetEnableICE());

	InitDisplayNameForNetSetup(pIpNetSetup);

	if (pConfParty->GetConnectionType() == DIAL_OUT)
		InitSetupTaAddr(pIpNetSetup,partyControInitParam.pStrConfName,pConfParty, NO);

	return pIpNetSetup;
}
///////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::SetMaxBitRateInNetSetup(CSipNetSetup* pIpNetSetup, CConfParty* pConfParty, CIpComMode*pPartyScm, PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	DWORD confRate = 0;
	DWORD vidBitrate;
	int val;
	val = ::CalculateRateForIpCalls(pConfParty, pPartyScm, confRate, vidBitrate,partyControInitParam.bIsEncript);
	PASSERT((val == -1));
	vidBitrate = vidBitrate / 100;

	if (pConfParty->GetVideoRate() == 0XFFFFFFFF) {
		pIpNetSetup->SetMaxRate(confRate);
		PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::SetMaxBitRateInNetSetup conf rate -",confRate);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CSipAddPartyCntl::SetMaxBitRateInNetSetup pConfParty->GetVideoRate() * 1000 -",pConfParty->GetVideoRate() * 1000);
		pIpNetSetup->SetMaxRate(pConfParty->GetVideoRate() * 1000);
	}
	BOOL bDisableAvMcuLowRate = GetSystemCfgFlag<BOOL>("DISABLE_LYNC_AV_MCU_128_192_KBPS");
	if(!bDisableAvMcuLowRate &&  (confRate == 128000 || confRate == 192000)  && ( pConfParty->GetAvMcuLinkType() != eAvMcuLinkNone || pConfParty->GetMsftAvmcuState() == eMsftAvmcu2013 ||	pConfParty->GetMsftAvmcuState() == eMsftAvmcuUnkown ) && ( eAvMcuLinkMain == GetAvMcuLinkType() || eAvMcuLinkSlaveIn == GetAvMcuLinkType()  )  )
	{
		PTRACE(eLevelInfoNormal, "CSipAddPartyCntl::SetMaxBitRateInNetSetup av-mcu patch for 128k call rate changing to 256k");
		pIpNetSetup->SetMaxRate(256000);

	//	pIpNetSetup->SetMaxRate(176000);
	}
}
///////////////////////////////////////////////////////////////////////////
CSipCaps* CSipPartyCntl::NewAndGetLocalSipCaps(CIpComMode* pPartyScm,CConfParty* pConfParty,CSipNetSetup* pIpNetSetup,DWORD& vidBitrate,PartyControlInitParameters& partyControInitParam, PartyControlDataParameters& partyControlDataParams)
{
	DWORD confRate = 0;
	int val;
	val = ::CalculateRateForIpCalls(pConfParty, pPartyScm, confRate, vidBitrate,partyControInitParam.bIsEncript);
	PASSERT((val == -1));
	vidBitrate = vidBitrate / 100;

	CSipCaps* pSIPCaps = new CSipCaps;

	WORD confPartyConnectionType = TranslateToConfPartyConnectionType(pConfParty->GetConnectionType());
	if (confPartyConnectionType == DIALOUT)
	{
		//BRIDGE-10123 IsOfferer needs to be TRUE and not FALSE!!!!!!
		SetSIPPartyCapsAndVideoParam(pPartyScm, pSIPCaps, pConfParty, vidBitrate, ((CSipNetSetup*)pIpNetSetup)->GetEnableSipICE(), pIpNetSetup, 0, TRUE, pConfParty->GetServiceId(),partyControInitParam,partyControlDataParams);
	}
	else
	{
		//BYTE networkType = m_pCommConf->GetNetwork();
		DWORD setupRate = pIpNetSetup->GetRemoteSetupRate();
		BOOL bIsCpRegardToIncomingSetupRate = 0;
		if ( (pPartyScm->GetConfType() == kCp) && (setupRate != 0) && (setupRate < confRate) )
		{
			CSmallString str;
			str << "CSipAddPartyCntl::NewAndGetLocalSipCaps : setup rate is less than the conf rate "
			   << " setup rate = " << setupRate
			   << " conf rate = " << confRate;
			PTRACE (eLevelInfoNormal, str.GetString());

			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			std::string key = "CP_REGARD_TO_INCOMING_SETUP_RATE"; //yael!!!
			pSysConfig->GetBOOLDataByKey(key, bIsCpRegardToIncomingSetupRate);

			if (bIsCpRegardToIncomingSetupRate)	//in order not to allocate too high resources:
			{

				::ReCalculateRateForIpCpDialInCalls (pConfParty, pPartyScm, partyControlDataParams.networkType, setupRate, confRate, vidBitrate);
				PTRACE2INT(eLevelInfoNormal,"CConf::NewAndGetLocalSipCaps : Recalculating the rate",vidBitrate);
				vidBitrate = vidBitrate / 100;// divided by 100 because is used in system in
			}
		}

	    if (pPartyScm->GetConfMediaType()!=eMixAvcSvcVsw || m_bIsMrcCall)
	    {
	        pPartyScm->SetVideoBitRate(vidBitrate, cmCapReceiveAndTransmit);
	        pPartyScm->SetTotalVideoRate(vidBitrate);
	    }

#if 0
		//BRIDGE-6001 -- LPR is not supported as desinged for CCS-plugin
		if (MicrosoftEP_Lync_CCS == partyControlDataParams.epType)
 			pPartyScm->SetIsLpr(FALSE);
#endif

		SetSIPPartyCapsAndVideoParam(pPartyScm, pSIPCaps, pConfParty, vidBitrate,
										((CSipNetSetup*)pIpNetSetup)->GetEnableSipICE(),pIpNetSetup,
										setupRate,partyControlDataParams.bIsOffer,pIpNetSetup->GetCsServiceid(),
										partyControInitParam,partyControlDataParams);

		UpdatePartyEncryptionMode(pConfParty,pPartyScm,partyControInitParam.encryptionType);

	}

	return pSIPCaps;
}
///////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::CopyNoneTipEncryptionParams(CIpComMode* pPartyScm, CIpComMode* pTempScm)
{
	PTRACE(eLevelInfoNormal, "CConf::CopyNoneTipEncryptionParams");

	CSdesCap *pSdesCap = NULL;

	cmCapDataType 	mediaType;
	ERoleLabel 		eRole;

	for(int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		pSdesCap = NULL;

		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		for (int j = 0; j < 2; j++)// 2 = number of direction (receive, transmit).
		{
			if (pTempScm->IsMediaOn(mediaType, globalDirectionArr[j], eRole) )
			{
				if (mediaType != cmCapBfcp) {

					pSdesCap =  pTempScm->GetSipSdes(mediaType,globalDirectionArr[j],eRole);

					if (pSdesCap)
						pPartyScm->SetSipSdes(mediaType, globalDirectionArr[j], eRole, pSdesCap);
					else
					{
						PTRACE2INT(eLevelInfoNormal, "CConf::CopyNoneTipEncryptionParams, no sdes cap for media type:", mediaType);

						if (mediaType == cmCapVideo && eRole == kRolePresentation)
						{
							pPartyScm->CreateLocalSipComModeSdesForSpecficMedia(mediaType, globalDirectionArr[j], eRole);

							pPartyScm->SetSdesMkiDefaultParams(mediaType, globalDirectionArr[j], eRole, FALSE);
							pPartyScm->SetSdesLifeTimeDefaultParams(mediaType, globalDirectionArr[j], eRole, FALSE);
							pPartyScm->SetSdesFecDefaultParams(mediaType, globalDirectionArr[j], eRole, FALSE);
						}
					}
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
/*void CSipPartyCntl::SetTimerForAsSipAddContentIfNeeded()
{
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

	if(pCommConf->GetIsAsSipContent() && m_IsAsSipContentEnable)
	{
		CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();

		DWORD ContentDelay;
		sysConfig->GetDWORDDataByKey(CFG_KEY_AS_SIP_CONTENT_ADD_DELAY, ContentDelay);

		PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::SetTimerForAsSipAddContentIfNeeded - Starttimer  for : ",ContentDelay);

		StartTimer(ADDASSIPCONTENTTOUT, ContentDelay*SECOND);
	}


}
////////////////////////////////////////////////////////////////
*/
/*
void CSipPartyCntl::SetCSConnectionsId()
{
//	POBJDELETE(m_pCsOrganizerRsrcDesc);
//	POBJDELETE(m_pCsFocusRsrcDesc);

//	m_pCsOrganizerRsrcDesc = new CRsrcParams;
//	*m_pCsOrganizerRsrcDesc = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_sigOrganizer);

	m_pCsOrganizerInterface = new CCsInterface;
	m_pCsOrganizerInterface->Create(m_pCsOrganizerRsrcDesc);

	m_pCsFocusRsrcDesc = new CRsrcParams;
	*m_pCsFocusRsrcDesc = m_pPartyAllocatedRsrc->GetRsrcParams(eLogical_ip_sigFocus);

	m_pCsFocusInterface = new CCsInterface;
	m_pCsFocusInterface->Create(m_pCsFocusRsrcDesc);

	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if ( pRoutingTbl== NULL )
	{
		PASSERT_AND_RETURN(101);
	}

	WORD status = pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pCsOrganizerRsrcDesc, m_pPartyApi);
	if (status != STATUS_OK)
		DBGPASSERT(status);

	status = pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pCsFocusRsrcDesc, m_pPartyApi);
	if (status != STATUS_OK)
		DBGPASSERT(status);

}
*/

//////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyAvmcu2013Detected(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyAvmcu2013Detected : Name - ",m_partyConfName, GetPartyRsrcId());
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_name);
	if (pConfParty && pConfParty->GetMsftAvmcuState() != eMsftAvmcu2013)
	{
		PTRACE(eLevelError,"CSipPartyCntl::OnPartyAvmcu2013Detected : error not 2013 av-mcu returning ");
		return;
	}

	DWORD msBwLimitation = m_pSipRemoteCaps->getMsftRxVideoBwLimitation();
	//m_pSipRemoteCaps->Dump("CSipPartyCntl::OnPartyAvmcu2013Detected m_pSipRemoteCaps ",eLevelInfoNormal);
	CSuperLargeString strCaps1;
	m_pSipRemoteCaps ->DumpToString(strCaps1);
	PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnPartyAvmcu2013Detected   m_pSipRemoteCaps =",strCaps1.GetString());

	DWORD scmBitrate = m_pIpCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
	DWORD bw = min(msBwLimitation, 3*scmBitrate);
	EVideoResolutionType vidResolutionType = convertVideoTypeToResType(m_eLastAllocatedVideoPartyType) ;
	DWORD localSsrc = m_pSipLocalCaps->getMsftSsrcVideoFirst(1);
	CSmallString sstr;
	sstr << "msBwLimitation = " << msBwLimitation << " , scmBitrate = " << scmBitrate <<" ,bw="<<bw<<" , vidResolutionType="<<vidResolutionType<<" ,localSsrc="<<localSsrc << "m_maxResForAvMcu = "<< m_maxResForAvMcu;
	PTRACE2(eLevelInfoNormal,"CSipPartyCntl::OnPartyAvmcu2013Detected: ",sstr.GetString());


	//create slave controller
	m_pMsSlavesController = new CMsSlavesController();
	PASSERT_AND_RETURN(!m_pMsSlavesController);


//create out slave
	CBaseCap *xmitEncryptionCommonOutCap = m_pIpCurrentMode->GetSipSdes(cmCapVideo ,cmCapTransmit, kRolePeople);
	CVidModeH323 *tmp_pSipLocalCaps		= &(CVidModeH323 &)m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapTransmit);
	BOOL isEncrypt  = FALSE;
	if (xmitEncryptionCommonOutCap)
	{
		isEncrypt  = TRUE;
		COstrStream ostr;
		xmitEncryptionCommonOutCap->Dump(ostr);

		PTRACE2(eLevelInfoNormal,"Flora debug 0102 CSipPartyCntl::OnPartyAvmcu2013Detected,Send Local Sdes cap: ",ostr.str().c_str());
	}


	m_pMsSlavesController->Create(GetPartyRsrcId(), GetMonitorPartyId(), m_pConf,m_pSipRemoteCaps,isEncrypt);


	//


	m_pMsSlavesController->ConnectOutSlaves(vidResolutionType,m_maxResForAvMcu ,localSsrc, bw, tmp_pSipLocalCaps);

	//change state and wait for out slave
	m_OldState = m_state;
	m_state = WAITING_FOR_MSFT_OUT_SLAVES;
}

//////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnMsftOutSlavesCreated(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnMsftOutSlavesCreated : Name - ",m_partyConfName, GetPartyRsrcId());

	m_state = m_OldState;

	//send to party (reinvite+mux)
	if (m_pPartyApi)
		m_pPartyApi->SendMsftOutSlaveCreatedToSipParty(pParam);

	LyncMsi audioMsi = m_pSipRemoteCaps->GetMsftMsiAudio();
	LyncMsi videoMsi = m_pSipRemoteCaps->getMsftMsiVideo(1);
	m_pTaskApi->SendAvMcuLocalRMXMsi(GetPartyRsrcId(), audioMsi, videoMsi);
	m_pMsSlavesController->SetLocalAudioMsi(audioMsi);

	//start in slaves creation
	EVideoResolutionType vidResolutionType = convertVideoTypeToResType(m_eLastAllocatedVideoPartyType) ;
	m_pMsSlavesController->ConnectInSlaves(vidResolutionType);
}

//////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnMsftInSlavesCreated(CSegment* pParam)
{
    PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnMsftInSlavesCreated : Name - ",m_partyConfName, GetPartyRsrcId());

    PASSERT_AND_RETURN(!pParam);
    DWORD status = statOK;
    DWORD numOfConnectedInSlaves = 0;
    *pParam >> status
    		>> numOfConnectedInSlaves;

    m_pTaskApi->SendAvMcuAllMsInSlavesConnected(GetPartyRsrcId(), numOfConnectedInSlaves);
}

//////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnMsSlaveToMainAckMessage(CSegment* pMsg)
{
	TRACEINTO;
	PASSERT_AND_RETURN(!pMsg);
	PASSERT_AND_RETURN (!m_pMsSlavesController);

	m_pMsSlavesController->HandleEvent(pMsg, PARTY_CONTROL_MS_SLAVE_TO_MAIN_ACK);
}

//////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnMsSlaveToMainMsgMessage(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	OPCODE opcode = (DWORD)(-1);

	*pMsg >> opcode;

	TRACEINTO << "MsSlaveToMainMsg's details:"
			  << "\n opcode: " << opcode;

	switch (opcode)
	{
		case MS_SLAVE_VIDEO_IN_SYNCED:
			if(m_pPartyApi)
			{
				m_pPartyApi->SendMsSlaveVideoInSyncedToSipParty(pMsg);
			}
			break;
		case MS_SLAVE_VIDREFRESH:
			// Delete slave from list + Send all slaves disconnected if needed
			if(m_pPartyApi)
		    {
		    	m_pPartyApi->SendMsSlaveVidRefreshToSipParty(pMsg);
		    }
			break;

		default:
			TRACEINTO << "incorrect opcode - ack was received for opcode: " << opcode;
			PASSERT(1);
	}
}

//////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnMsSlaveToMainVideoInSyncMessage(CSegment* pMsg)
{
	TRACEINTO;
	PASSERT_AND_RETURN(!pMsg);
	PASSERT_AND_RETURN(!m_pPartyApi);

	m_pMsSlavesController->HandleEvent(pMsg, PARTY_CONTROL_MS_SLAVE_TO_MAIN_ACK);
}

//////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyVsrMsgInd(CSegment* pMsg)
{
	// TRACEINTO << " VSR_FROM_AV_MCU_DEBUG ";
	if(!m_pMsSlavesController){
		TRACEINTO << " m_pMsSlavesController is NULL";
		return;
	}
	if(!pMsg){
		TRACEINTO << " pMsg is NULL";
		return;
	}
	m_pMsSlavesController->HandleEvent(pMsg, SIP_PARTY_VSR_MSG_IND);
}
//////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnSlavesControllerSingleVsrMassageInd(CSegment* pMsg)
{
	//TRACEINTO << " TODO - implement slaves flow :)";
	TRACEINTO;
	if(!m_pPartyApi)
	{
		//PASSERT_AND_RETURN(1);
		TRACEINTO << "m_pPartyApi is NULL - could be ERROR (if it not first VSR message). will not SendSingleVsrMsgIndToSipParty";
		return;
	}

	m_pPartyApi->SendSingleVsrMsgIndToSipParty(pMsg);
}
//////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnSlavesControllerMsg(CSegment* pMsg)
{
	if(NULL != m_pMsSlavesController){
		DWORD opcode = (DWORD)(-1);
		if(NULL != pMsg){
			*pMsg >> opcode;
		}
//		DWORD sendPartyId = (DWORD)(-1);
//		DWORD sendOpcode = (DWORD)(-1);
//		*pMsg >> sendPartyId;
//		*pMsg >> sendOpcode;
		TRACEINTO << " opcode = " << CProcessBase::GetProcess()->GetOpcodeAsString(opcode); // << " , sendOpcode = " << sendOpcode << " , sendPartyId = " << sendPartyId;
		// pMsg->DumpHex();
		m_pMsSlavesController->DispatchEvent(opcode,pMsg);
	}else{
		TRACEINTO << " m_pMsSlavesController is NULL";
	}
}

//////////////////////////////////////////////////////////////////////////////
// Handle Msg from MSSlaveParty -> MSSlavePartyCntl
// target: pass msg from MSSlavePartyCntl -> MainPartyCntl
void CSipPartyCntl::PartyToPartyCntlMessageMSSlaveToMain(CSegment* pParam)
{
//	PTRACE(eLevelInfoNormal,"CSipPartyCntl::PartyToPartyCntlMessageSlaveToMaster ");
	DWORD	opcode;
	*pParam >> opcode;
	TRACEINTO << " CSipPartyCntl::PartyToPartyCntlMessageMSSlaveToMain : " << ", opcode=" << opcode;
	SendMessageFromMSSlaveToMain(opcode, pParam);//m_pPartyApi->PartyCntlToPartyMessageSlaveToMaster(pParam);
}

//////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
void  CSipPartyCntl::OnPartyRecordingControlAck(CSegment* pParam)
{
	BYTE status = 0;
	*pParam >> status;
	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnPartyRecordingControlAck: statu - ", status);

	if(m_pConfAppMngrInterface)
	m_pConfAppMngrInterface->HandleRecordingControlAck(status);
}
////////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
void  CSipPartyCntl::OnPartyLayoutControl(CSegment* pParam)
{
	BYTE layout = 0;

	*pParam >> layout;
	PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnPartyLayoutControl: layout - ", (int)layout);

	enSrsVideoLayoutType videoLayout = static_cast<enSrsVideoLayoutType>(layout);

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());

	if(NULL == pCommConf ||  NULL == pConfParty)
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnPartyLayoutControl: not a recording link! ");
		return;
	}

	//only used for recording link now, can remove this limitation later for other parties.
	if(YES == pConfParty->GetRecordingLinkParty())
	{

	BYTE  isHDVSW = pCommConf->GetIsHDVSW();
	BYTE isCOP = pCommConf->GetIsCOP();
          if(!isCOP)
          {
              const char* pPartyName = pConfParty->GetName();
              CConfApi confApi;
              confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

              if(!isHDVSW)
              {
              		if(eSrsVideoLayoutAuto == videoLayout)
              		{
				pConfParty->SetLastLayoutForRL((BYTE)videoLayout);
              			BYTE  curConfLayout = pCommConf->GetCurConfVideoLayout();
				CVideoLayout* pVideoLayout = pCommConf->GetVideoLayout(curConfLayout);

				confApi.SetVideoPrivateLayout(pPartyName, *pVideoLayout);

			}
			else
			{
				pConfParty->SetLastLayoutForRL((BYTE)videoLayout);
				CVideoLayout*	 pVideoLayout =	   new CVideoLayout;
				pVideoLayout->SetLayoutForRecording(videoLayout);

				confApi.SetVideoPrivateLayout(pPartyName, *pVideoLayout);

				delete pVideoLayout;
			}

	}
    	    else
       	    {
                    PTRACE(eLevelInfoNormal, "CConf::OnPartyLayoutControl, cant set private layout in VSW conference");
             }
            confApi.DestroyOnlyApi();
            }
            else
            {
                PTRACE(eLevelInfoNormal, "CConf::OnPartyLayoutControl,No personal layouts in COP conf");
            }
	}
	//Added for Playback link
	else if(YES == pConfParty->GetPlaybackLinkParty())
	{		
		BYTE  isHDVSW = pCommConf->GetIsHDVSW();
		BYTE isCOP = pCommConf->GetIsCOP();
		
		if(!isCOP && !isHDVSW)
		{
			const char* pPartyName = pConfParty->GetName();
			CConfApi confApi;
			confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
			CLectureModeParams* pLectureMode = new CLectureModeParams;
			if(eSrsVideoLayoutAuto == videoLayout )
			{
				pLectureMode->SetLectureModeType(NO);
				pLectureMode->SetTimerOnOff(NO);
				pLectureMode->SetLectureTimeInterval(15);
				pLectureMode->SetAudioActivated(NO);
				//pLectureMode->SetLecturerName(pPartyName);
				pLectureMode->SetLecturerId(-1);
				
				confApi.UpdateLectureMode(pLectureMode);
				confApi.UpdateContentLectureMode(pCommConf->IsExclusiveContentMode());
			}
			else if(eSrsVideoLayoutLecture == videoLayout )
			{
				pLectureMode->SetLectureModeType(YES);
				pLectureMode->SetTimerOnOff(YES);
				pLectureMode->SetLectureTimeInterval(15);
				pLectureMode->SetAudioActivated(NO);
				pLectureMode->SetLecturerName(pPartyName);
				pLectureMode->SetLecturerId(-1);
				
				confApi.UpdateLectureMode(pLectureMode);
				confApi.UpdateContentLectureMode(pCommConf->IsExclusiveContentMode());
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnPartyLayoutControl: invalid layout for playback link! ");
			}
			confApi.DestroyOnlyApi();
			delete pLectureMode;
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::OnPartyLayoutControl: not a recording/playback link! ");
	}
}

////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::UpdateLocalCapsForHdVswInMixMode(const VideoOperationPoint *pVideoOperationPoint)
{
    m_pSipLocalCaps->UpdateCapsForHdVswInMixedMode(eH264CapCode, m_pIpInitialMode, pVideoOperationPoint);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyChangeBridgesMuteState(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyChangeBridgesMuteState: Name - ",m_partyConfName, GetPartyRsrcId());

	BYTE bMuteAudioIn;
	BYTE bMuteAudioOut;
	BYTE bMuteVideoIn;
	BYTE bMuteVideoOut;
	BYTE bMuteContentIn;
	BYTE bMuteContentOut;
	BYTE bMuteFeccIn;
	BYTE bMuteFeccOut;

	*pParam >> bMuteAudioIn
			>> bMuteAudioOut
			>> bMuteVideoIn
			>> bMuteVideoOut
			>> bMuteContentIn
			>> bMuteContentOut
			>> bMuteFeccIn
			>> bMuteFeccOut;

	TRACEINTO << "bMuteAudioIn: " << (DWORD)bMuteAudioIn <<", bMuteAudioOut: " << (DWORD)bMuteAudioOut
			  << ", bMuteVideoIn: " << (DWORD)bMuteVideoIn << ", bMuteVideoOut: "<<(DWORD)bMuteVideoOut
			  << ", bMuteContentIn: " << (DWORD)bMuteContentIn << ", bMuteContentOut: " <<(DWORD)bMuteContentOut
			  << ", bMuteContentOut: " << (DWORD)bMuteFeccIn << ", bMuteFeccOut: " << (DWORD)bMuteFeccOut;

	if (IsInDirectionConnectedToAudioBridge())
	{
		if (bMuteAudioIn != AUTO)
			m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, (EOnOff)bMuteAudioIn, PARTY);
	}

	if (IsOutDirectionConnectedToAudioBridge())
	{
		if(bMuteAudioOut != AUTO)
			m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaOut, (EOnOff)bMuteAudioOut, PARTY);
	}

	if (IsInDirectionConnectedToVideoBridge())
	{
		if (bMuteVideoIn != AUTO)
			m_pVideoBridgeInterface->UpdateMute(GetPartyRsrcId(), (EOnOff)bMuteVideoIn, PARTY);
	}

	if (IsOutDirectionConnectedToVideoBridge())
	{
		TRACEINTO<<"error there is not mute out to VB";

	}

	if (m_isFeccConn)
	{
		if (bMuteFeccIn != AUTO || bMuteFeccOut != AUTO)
			PTRACEPARTYID(eLevelInfoNormal, "Mute Fecc - not implemented", GetPartyRsrcId());
	}
}



