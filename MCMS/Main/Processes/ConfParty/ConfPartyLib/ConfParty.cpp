#include <string.h>
#include "NStream.h"
#include "ConfParty.h"
#include "psosxml.h"
#include "H221StrCap.h"
#include "H323StrCap.h"
#include "PartyMonitor.h"
#include "IpChannelDetails.h"
#include "H323GkStatus.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "ConfApi.h"
#include "ApiStatuses.h"
#include "AttendedStruct.h"
#include "SystemFunctions.h"
#include "MoveInfo.h"
#include "IpServiceListManager.h"
#include "Trace.h"
#include "TraceStream.h"
#include "SipBWStatus.h"
#include "Media.h"

extern CIpServiceListManager* GetIpServiceListMngr();

////////////////////////////////////////////////////////////////////////////
//                        CConfParty
////////////////////////////////////////////////////////////////////////////
CConfParty::CConfParty() : CRsrvParty()
{
	m_party_state           = PARTY_IDLE;
	m_net_channels          = 0;
	m_H221_status           = 0;
	m_disconnect_cause      = NO_DISCONNECTION_CAUSE;
	m_Q931_disconnect_cause = 0;
	m_secondary_cause       = 0xFF;
	m_member_in_media       = 0;
	m_mute_byOperator       = 0;
	m_mute_byParty          = 0;
	m_mute_byMCU            = 0;
	m_blockMedia            = 0;
	m_isHighProfile         = FALSE;
	m_audioActivatedflag    = 0;

       /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
       m_VideoRecoveryStatus = 0;
       /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

	if (!m_voice)
	{
		m_pVideoLayout          = new CVideoLayout;
		m_pPartyConfVideoLayout = new CVideoLayout;
	}
	else
	{
		m_pVideoLayout          = NULL;
		m_pPartyConfVideoLayout = NULL;
	}

	m_pCapabilities          = new CH221Str;
	m_pLocalCommMode         = new CH221Str;
	m_pRemoteCommMode        = new CH221Str;
	m_retriesNumber          = 0;
	m_internalPartyId        = 0; // pointer to CTaskApp
	m_numVideoSource         = 0;
	m_pAttendedStruct        = new CAttendedStruct;
	m_trmBaudRate            = 0;
	m_rcvBaudRate            = 0;
	m_L_syncLostCounter      = 0;
	m_R_syncLostCounter      = 0;
	m_L_videoSyncLostCounter = 0;
	m_R_videoSyncLostCounter = 0;

	int i;
	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
		m_pActualMCUPhoneNumbers[i] = NULL;

	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
		m_pActualPartyPhoneNumbers[i] = NULL;

	m_operatorName[0]                  = '\0';
	m_bondingTmpNumber.phone_number[0] = '\0';

	if (IsIpNetInterfaceType())
	{
		m_pIpCapabilities   = new CH323StrCap;
		m_pIpLocalCommMode  = new CH323StrCap;
		m_pIpRemoteCommMode = new CH323StrCap;
	}
	else
	{
		m_pIpCapabilities   = NULL;
		m_pIpLocalCommMode  = NULL;
		m_pIpRemoteCommMode = NULL;
	}

	m_ipVideoBch            = NO;
	m_ipProtocolSync        = NO;
	m_ipVideoBchCounter     = 0;
	m_ipProtocolSyncCounter = 0;
	m_pParty                = NULL;

	if (IsIpNetInterfaceType())
	{
		for (i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
		{
			m_pIpChannelDetails[i] = new CIpChannelDetails;
			m_pIpChannelDetails[i]->SetChannelType((EIpChannelType)i);
			m_pIpChannelMonitor[i] = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)i);
		}

		m_pH323GatekeeperStatus = new CH323GatekeeperStatus;
		m_pSipBWStatus          = new CSIPBandwidthAllocationStatus;
	}
	else
	{
		for (i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
		{
			m_pIpChannelDetails[i] = NULL;
			m_pIpChannelMonitor[i] = NULL;
		}
		m_pH323GatekeeperStatus = NULL;
		m_pSipBWStatus          = NULL;
	}

	m_h323BackupPartyAlias[0] = '\0';
	// IpV6
	memset(&m_backupIpAddress, 0, sizeof(mcTransportAddress));

	// We need to save it here, because the ip in the reservation might be changed during the call.
	// When the party is disconnected, we change the reservation ip according to this field.
	m_ipVideoIntraSync                        = NO;
	m_pSubject                                = new CSubject;
	m_isLeader                                = 0;
	m_wait_for_oper_assistance                = 0;
	m_visual_partyName[0]                     = '\0';
	m_remoteName[0]                           = '\0';
	m_bReceiveDtmfFromChairperson				= FALSE;
	m_isNoiseDetected                         = NO;
	m_EPCContentProvider                      = NO;
	m_pRes                                    = NULL;
	m_lobbyId                                 = 0;
	m_isPartyCurrentlyEncrypted               = NO;
	m_referredByStr[0]                        = '\0';
	m_bDefinedPartyAssigned                   = FALSE;
	m_mipErrorNumber                          = 0;
	m_lpr_sync_status                         = 0;
	m_bTransparentGw                          = NO;
	m_bRequestToSpeak                         = NO;
	m_GatewayPartyType                        = eRegularPartyNoGateway;
	m_isExclusiveContent                      = FALSE; // Restricted content
	m_isEventModeIntraSuppressed              = FALSE;
	m_eventModeLevel                          = 0;
	m_bIsFirstConnectionAfterHotBackupRestore = FALSE;
	m_HotBackupPartyStateInMaster             = PARTY_IDLE;
	m_lpr_heders_activated                    =  NO;
	m_bFoundInAddrBook                        = FALSE;
	m_bIsTipCall                              = FALSE;
  m_bIsTipHeader                            = FALSE;
	m_remoteIdent                             = Regular;
	m_plcmRequireMask							= 0;
	m_bIsCiscoTagExist							= FALSE;
	m_BfcpTransportType                       = eUnknownTransportType;
	m_MoveType                                = eMoveDummy;
	m_bFlagConnectedOnce                      = FALSE;

	//VNGR-26449 - unencrypted conference message
	m_isUnencryptedParty                      = eNotSet;
	m_isDelPartyWasEncrypted                  = YES;

	m_bIsLyncPlugin						= FALSE;
	m_strFromTag[0] = '\0';
	m_strToTag[0] = '\0';
	
	m_sipUsrName[0]                           = '\0';

	m_partyTypeSubTIP                         = eTipNone;
	m_AvMcuLinkType                           = eAvMcuLinkNone;

	m_IsCountedInAudioIndication              = NO;

	m_bSrcTypeIsEQ                            = FALSE;
	m_tokenRecapCollisionDetection	          = etrcdAvailable;
	m_trcdInPend                              = FALSE;
	
}

//--------------------------------------------------------------------------
CConfParty::CConfParty(const CConfParty& other) : CRsrvParty(*(const CRsrvParty*)&other)
{
	m_party_state           = other.m_party_state;
	m_net_channels          = other.m_net_channels;
	m_H221_status           = other.m_H221_status;
	m_disconnect_cause      = other.m_disconnect_cause;
	m_Q931_disconnect_cause = other.m_Q931_disconnect_cause;
	m_secondary_cause       = other.m_secondary_cause;
	m_member_in_media       = other.m_member_in_media;
	m_mute_byOperator       = other.m_mute_byOperator;
	m_mute_byParty          = other.m_mute_byParty;
	m_mute_byMCU            = other.m_mute_byMCU;
	m_blockMedia            = other.m_blockMedia;
	m_isHighProfile         = other.m_isHighProfile;
       /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
       m_VideoRecoveryStatus = other.m_VideoRecoveryStatus;
       /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

	if (other.m_pVideoLayout == NULL)
		m_pVideoLayout = NULL;
	else
		m_pVideoLayout = new CVideoLayout(*other.m_pVideoLayout);

	if (other.m_pCapabilities == NULL)
		m_pCapabilities = NULL;
	else
		m_pCapabilities = new CH221Str(*other.m_pCapabilities);

	if (other.m_pLocalCommMode == NULL)
		m_pLocalCommMode = NULL;
	else
		m_pLocalCommMode = new CH221Str(*other.m_pLocalCommMode);

	if (other.m_pRemoteCommMode == NULL)
		m_pRemoteCommMode = NULL;
	else
		m_pRemoteCommMode = new CH221Str(*other.m_pRemoteCommMode);

	m_connectTime    = other.m_connectTime;
	m_disconnectTime = other.m_disconnectTime;
	m_retriesNumber  = other.m_retriesNumber;

	strncpy(m_visual_partyName, other.m_visual_partyName, H243_NAME_LEN);
	strncpy(m_remoteName, other.m_remoteName, H243_NAME_LEN);
	m_bReceiveDtmfFromChairperson = other.m_bReceiveDtmfFromChairperson;

	if (other.m_pAttendedStruct == NULL)
		m_pAttendedStruct = NULL;
	else
		m_pAttendedStruct = new CAttendedStruct(*other.m_pAttendedStruct);

	m_trmBaudRate            = other.m_trmBaudRate;
	m_rcvBaudRate            = other.m_rcvBaudRate;
	m_L_syncLostCounter      = other.m_L_syncLostCounter;
	m_R_syncLostCounter      = other.m_R_syncLostCounter;
	m_L_videoSyncLostCounter = other.m_L_videoSyncLostCounter;
	m_R_videoSyncLostCounter = other.m_R_videoSyncLostCounter;

	int i;
	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
	{
		if (other.m_pActualMCUPhoneNumbers[i] == NULL)
			m_pActualMCUPhoneNumbers[i] = NULL;
		else
			m_pActualMCUPhoneNumbers[i] = new Phone(*other.m_pActualMCUPhoneNumbers[i]);
	}

	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
	{
		if (other.m_pActualPartyPhoneNumbers[i] == NULL)
			m_pActualPartyPhoneNumbers[i] = NULL;
		else
			m_pActualPartyPhoneNumbers[i] = new Phone(*other.m_pActualPartyPhoneNumbers[i]);
	}

	m_secondary_cause_params = other.m_secondary_cause_params;
	strncpy(m_operatorName, other.m_operatorName, OPERATOR_NAME_LEN);
	strncpy(m_bondingTmpNumber.phone_number, other.m_bondingTmpNumber.phone_number, PHONE_NUMBER_DIGITS_LEN);

	if (other.m_pIpCapabilities == NULL)
		m_pIpCapabilities = NULL;
	else
		m_pIpCapabilities = new CH323StrCap(*other.m_pIpCapabilities);

	if (other.m_pIpLocalCommMode == NULL)
		m_pIpLocalCommMode = NULL;
	else
		m_pIpLocalCommMode = new CH323StrCap(*other.m_pIpLocalCommMode);

	if (other.m_pIpRemoteCommMode == NULL)
		m_pIpRemoteCommMode = NULL;
	else
		m_pIpRemoteCommMode = new CH323StrCap(*other.m_pIpRemoteCommMode);

	m_ipVideoBch            = other.m_ipVideoBch;
	m_ipProtocolSync        = other.m_ipProtocolSync;
	m_ipVideoBchCounter     = other.m_ipVideoBchCounter;
	m_ipProtocolSyncCounter = other.m_ipProtocolSyncCounter;

	for (i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
	{
		if (other.m_pIpChannelDetails[i] == NULL)
		{
			m_pIpChannelDetails[i] = NULL;
		}
		else
			m_pIpChannelDetails[i] = new CIpChannelDetails(*other.m_pIpChannelDetails[i]);
	}

	for (i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
	{
		if (other.m_pIpChannelMonitor[i] == NULL)
			m_pIpChannelMonitor[i] = NULL;
		else
		{
			// There times other does not contain channelType tet so we can't copy immediately other.
			m_pIpChannelMonitor[i] = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)i);
			if (m_pIpChannelMonitor[i])
				m_pIpChannelMonitor[i]->CopyClass(*other.m_pIpChannelMonitor[i]);
		}
	}

	if (other.m_pH323GatekeeperStatus == NULL)
		m_pH323GatekeeperStatus = NULL;
	else
		m_pH323GatekeeperStatus = new CH323GatekeeperStatus(*other.m_pH323GatekeeperStatus);

	if (other.m_pSipBWStatus == NULL)
		m_pSipBWStatus = NULL;
	else
		m_pSipBWStatus = new CSIPBandwidthAllocationStatus(*other.m_pSipBWStatus);

	// IpV6
	memset(&m_backupIpAddress, 0, sizeof(mcTransportAddress));
	memcpy(&m_backupIpAddress, &other.m_backupIpAddress, sizeof(mcTransportAddress));

	strncpy(m_h323BackupPartyAlias, other.m_h323BackupPartyAlias, IP_STRING_LEN);


	m_ipVideoIntraSync         = other.m_ipVideoIntraSync;
	m_isLeader                 = other.m_isLeader;
	m_wait_for_oper_assistance = other.m_wait_for_oper_assistance;
	m_isNoiseDetected          = other.m_isNoiseDetected;
	m_EPCContentProvider       = other.m_EPCContentProvider;

	if (other.m_pPartyConfVideoLayout == NULL)
		m_pPartyConfVideoLayout = NULL;
	else
		m_pPartyConfVideoLayout = new CVideoLayout(*other.m_pPartyConfVideoLayout);

	m_pRes                      = other.m_pRes;
	m_lobbyId                   = other.m_lobbyId;
	m_isPartyCurrentlyEncrypted = other.m_isPartyCurrentlyEncrypted;

	if (other.m_pSubject == NULL)
		m_pSubject = NULL;
	else
		m_pSubject = new CSubject;

	strncpy(m_referredByStr, other.m_referredByStr, H243_NAME_LEN);

	m_bDefinedPartyAssigned                   = other.m_bDefinedPartyAssigned;
	m_mipErrorNumber                          = other.m_mipErrorNumber;
	m_lpr_sync_status                         =  other.m_lpr_sync_status;
	m_GatewayPartyType                        = other.m_GatewayPartyType;
	m_bTransparentGw                          =  other.m_bTransparentGw;
	m_isExclusiveContent                      = other.m_isExclusiveContent; // Restricted content
	m_bRequestToSpeak                         = other.m_bRequestToSpeak;
	m_isEventModeIntraSuppressed              = other.m_isEventModeIntraSuppressed;
	m_eventModeLevel                          = other.m_eventModeLevel;
	m_bIsFirstConnectionAfterHotBackupRestore = other.m_bIsFirstConnectionAfterHotBackupRestore;
	m_HotBackupPartyStateInMaster             = other.m_HotBackupPartyStateInMaster;
	m_lpr_heders_activated                    = other.m_lpr_heders_activated;
	m_pParty                                  = other.m_pParty;
	m_bIsTipCall                              = other.m_bIsTipCall;
  m_bIsTipHeader                            = other.m_bIsTipHeader;
	m_bFoundInAddrBook                        = other.m_bFoundInAddrBook;
	m_remoteIdent                             = other.m_remoteIdent;
	m_plcmRequireMask                         = other.m_plcmRequireMask;
	m_bIsCiscoTagExist                        = other.m_bIsCiscoTagExist;
	m_BfcpTransportType                       = other.m_BfcpTransportType;
	m_MoveType                                = other.m_MoveType;

	m_mediaList.SetMedia(&other.m_mediaList);
	m_bFlagConnectedOnce                      = other.m_bFlagConnectedOnce;

	//VNGR-26449 - unencrypted conference message
	m_isUnencryptedParty                      = other.m_isUnencryptedParty;
	m_isDelPartyWasEncrypted                  = other.m_isDelPartyWasEncrypted;
	m_bIsLyncPlugin                           = other.m_bIsLyncPlugin;
	m_partyTypeSubTIP                         = other.m_partyTypeSubTIP;
	m_AvMcuLinkType                           = other.m_AvMcuLinkType;
	m_correlationId                           = other.m_correlationId;

	strncpy(m_strFromTag, other.m_strFromTag, H243_NAME_LEN);
	strncpy(m_strToTag, other.m_strToTag, H243_NAME_LEN);
	strncpy(m_sipUsrName, other.m_sipUsrName, H243_NAME_LEN);

	m_IsCountedInAudioIndication = other.m_IsCountedInAudioIndication;

	m_bSrcTypeIsEQ = other.m_bSrcTypeIsEQ;
	m_tokenRecapCollisionDetection	= other.m_tokenRecapCollisionDetection;
	m_trcdInPend					= other.m_trcdInPend;

}

//--------------------------------------------------------------------------
CConfParty::CConfParty(const CRsrvParty& other) : CRsrvParty(other)
{
	if (m_connectionType == DIAL_OUT)
		m_party_state = PARTY_IDLE;
	else
		m_party_state = PARTY_WAITING_FOR_DIAL_IN;

	m_net_channels          = 0;
	m_H221_status           = 0;
	m_disconnect_cause      = NO_DISCONNECTION_CAUSE;
	m_Q931_disconnect_cause = 0;
	m_secondary_cause       = 0xFF;
	m_member_in_media       = 0;
	m_mute_byOperator       = 0;
	m_mute_byParty          = 0;
	m_mute_byMCU            = 0;
	m_blockMedia            = 0;

	m_IsCountedInAudioIndication = NO;

	m_tokenRecapCollisionDetection 	= etrcdAvailable;
	m_trcdInPend					= FALSE;

       /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
       m_VideoRecoveryStatus = 0;
       /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

	if (!m_voice)
	{
		m_pVideoLayout          = new CVideoLayout;
		m_pPartyConfVideoLayout = new CVideoLayout;
	}
	else
	{
		m_pVideoLayout          = NULL;
		m_pPartyConfVideoLayout = NULL;
	}

	m_pCapabilities          = new CH221Str;
	m_pLocalCommMode         = new CH221Str;
	m_pRemoteCommMode        = new CH221Str;
	m_retriesNumber          = 0;
	m_visual_partyName[0]    = '\0';
	m_remoteName[0]          = '\0';
	m_bReceiveDtmfFromChairperson = FALSE;
	m_pAttendedStruct        = new CAttendedStruct;
	m_trmBaudRate            = 0;
	m_rcvBaudRate            = 0;
	m_L_syncLostCounter      = 0;
	m_R_syncLostCounter      = 0;
	m_L_videoSyncLostCounter = 0;
	m_R_videoSyncLostCounter = 0;

	int i;
	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
		m_pActualMCUPhoneNumbers[i] = NULL;

	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
		m_pActualPartyPhoneNumbers[i] = NULL;

	m_operatorName[0]                  = '\0';
	m_bondingTmpNumber.phone_number[0] = '\0';

	if (IsIpNetInterfaceType())
	{
		m_pIpCapabilities   = new CH323StrCap;
		m_pIpLocalCommMode  = new CH323StrCap;
		m_pIpRemoteCommMode = new CH323StrCap;
	}
	else
	{
		m_pIpCapabilities   = NULL;
		m_pIpLocalCommMode  = NULL;
		m_pIpRemoteCommMode = NULL;
	}

	m_ipVideoBch            = NO;
	m_ipProtocolSync        = NO;
	m_ipVideoBchCounter     = 0;
	m_ipProtocolSyncCounter = 0;

	if (IsIpNetInterfaceType())
	{
		for (i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
		{
			m_pIpChannelDetails[i] = new CIpChannelDetails;
			m_pIpChannelMonitor[i] = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)i);
		}
		m_pH323GatekeeperStatus = new CH323GatekeeperStatus;
		m_pSipBWStatus          = new CSIPBandwidthAllocationStatus;
	}
	else
	{
		for (i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
		{
			m_pIpChannelDetails[i] = NULL;
			m_pIpChannelMonitor[i] = NULL;
		}
		m_pH323GatekeeperStatus = NULL;
		m_pSipBWStatus          = NULL;
	}

	memset(&m_backupIpAddress, 0, sizeof(mcTransportAddress));

	m_h323BackupPartyAlias[0]                 = '\0';
	m_ipVideoIntraSync                        = NO;
	m_isLeader                                = 0;
	m_wait_for_oper_assistance                = 0;
	m_isNoiseDetected                         = NO;
	m_EPCContentProvider                      = NO;
	m_pRes                                    = NULL;
	m_lobbyId                                 = 0;
	m_isPartyCurrentlyEncrypted               = NO;
	m_pSubject                                = new CSubject;
	m_referredByStr[0]                        = '\0';
	m_bDefinedPartyAssigned                   = FALSE;
	m_mipErrorNumber                          = 0;
	m_lpr_sync_status                         = 0;
	m_bTransparentGw                          = NO;
	m_GatewayPartyType                        = eRegularPartyNoGateway;
	m_isExclusiveContent                      = FALSE;
	m_bRequestToSpeak                         = NO;
	m_isEventModeIntraSuppressed              = FALSE;
	m_eventModeLevel                          = 0;
	m_bIsFirstConnectionAfterHotBackupRestore = FALSE;
	m_lpr_heders_activated                    = NO;
	m_bFoundInAddrBook                        = FALSE;
	m_bIsTipCall                              = NO;
	m_bIsTipHeader                            = NO;
	m_remoteIdent                             = Regular;
	m_plcmRequireMask                         = 0;
	m_bIsCiscoTagExist                        = FALSE;
	m_BfcpTransportType                       = eUnknownTransportType;
	m_pParty                                  = NULL;
	m_bFlagConnectedOnce                      = FALSE;
	m_MoveType                                = eMoveDummy;

	//VNGR-26449 - unencrypted conference message
	m_isDelPartyWasEncrypted                  = YES;
	m_isUnencryptedParty                      = eNotSet;
	m_bIsLyncPlugin                           = FALSE;
	//_mccf_
	m_strToTag[0]                             = '\0';
	m_strFromTag[0]                           = '\0';
	m_sipUsrName[0]                           = '\0';

	m_partyTypeSubTIP                         = eTipNone;
	m_AvMcuLinkType                           = eAvMcuLinkNone;
	m_bSrcTypeIsEQ							  = FALSE;
}

//--------------------------------------------------------------------------
CConfParty::~CConfParty()
{
	POBJDELETE(m_pVideoLayout);
	POBJDELETE(m_pCapabilities);
	POBJDELETE(m_pLocalCommMode);
	POBJDELETE(m_pRemoteCommMode);
	POBJDELETE(m_pAttendedStruct);

	int i;
	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
		PDELETE(m_pActualMCUPhoneNumbers[i]);

	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
		PDELETE(m_pActualPartyPhoneNumbers[i]);

	POBJDELETE(m_pIpCapabilities);
	POBJDELETE(m_pIpLocalCommMode);
	POBJDELETE(m_pIpRemoteCommMode);

	for (i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
	{
		POBJDELETE(m_pIpChannelDetails[i]);
		POBJDELETE(m_pIpChannelMonitor[i]);
	}

	POBJDELETE(m_pH323GatekeeperStatus);
	POBJDELETE(m_pSipBWStatus);
	POBJDELETE(m_pPartyConfVideoLayout);
	POBJDELETE(m_pUserDefinedInfo);
	POBJDELETE(m_pSubject);
}

//--------------------------------------------------------------------------
CConfParty& CConfParty::operator =(const CConfParty& other)
{
	if (&other == this)
		return *this;

	CRsrvParty::operator =(other);

	m_party_state           = other.m_party_state;
	m_net_channels          = other.m_net_channels;
	m_H221_status           = other.m_H221_status;
	m_disconnect_cause      = other.m_disconnect_cause;
	m_Q931_disconnect_cause = other.m_Q931_disconnect_cause;
	m_secondary_cause       = other.m_secondary_cause;
	m_member_in_media       = other.m_member_in_media;
	m_mute_byOperator       = other.m_mute_byOperator;
	m_mute_byParty          = other.m_mute_byParty;
	m_mute_byMCU            = other.m_mute_byMCU;
	m_audioActivatedflag    = other.m_audioActivatedflag;
	int i;

       /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
       m_VideoRecoveryStatus = other.m_VideoRecoveryStatus;
       /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
       
	for (i = 0; i < MAX_VIDEO_SOURCE_PARTY; i++)
		m_video_source_id[i] = other.m_video_source_id[i];

	POBJDELETE(m_pCapabilities);
	POBJDELETE(m_pLocalCommMode);
	POBJDELETE(m_pRemoteCommMode);

	if (m_pVideoLayout != NULL)
		POBJDELETE(m_pVideoLayout);

	if (other.m_pVideoLayout != NULL)
		m_pVideoLayout = new CVideoLayout(*other.m_pVideoLayout);

	for (i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
	{
		if (m_pPrivateVideoLayout[i] != NULL)
			POBJDELETE(m_pPrivateVideoLayout[i]);

		if (other.m_pPrivateVideoLayout[i] != NULL)
			m_pPrivateVideoLayout[i] = new CVideoLayout(*(other.m_pPrivateVideoLayout[i]));
	}

	m_isPrivate             = other.m_isPrivate; // Private layout
	m_numPrivateVideoLayout = other.m_numPrivateVideoLayout;

	m_isVip                 = other.m_isVip;

	if (other.m_pCapabilities == NULL)
		m_pCapabilities = NULL;
	else
		m_pCapabilities = new CH221Str(*other.m_pCapabilities);

	if (other.m_pLocalCommMode == NULL)
		m_pLocalCommMode = NULL;
	else
		m_pLocalCommMode = new CH221Str(*other.m_pLocalCommMode);

	if (other.m_pRemoteCommMode == NULL)
		m_pRemoteCommMode = NULL;
	else
		m_pRemoteCommMode = new CH221Str(*other.m_pRemoteCommMode);

	m_disconnectTime  = other.m_disconnectTime;
	m_retriesNumber   = other.m_retriesNumber;
	m_internalPartyId = other.m_internalPartyId;
	m_numVideoSource  = other.m_numVideoSource;

	strncpy(m_H243_partyName, other.m_H243_partyName, H243_NAME_LEN);

	POBJDELETE(m_pAttendedStruct);

	if (other.m_pAttendedStruct == NULL)
		m_pAttendedStruct = NULL;
	else
		m_pAttendedStruct = new CAttendedStruct(*other.m_pAttendedStruct);

	m_trmBaudRate            = other.m_trmBaudRate;
	m_rcvBaudRate            = other.m_rcvBaudRate;
	m_L_syncLostCounter      = other.m_L_syncLostCounter;
	m_R_syncLostCounter      = other.m_R_syncLostCounter;
	m_L_videoSyncLostCounter = other.m_L_videoSyncLostCounter;
	m_R_videoSyncLostCounter = other.m_R_videoSyncLostCounter;

	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
	{
		PDELETE(m_pActualMCUPhoneNumbers[i]);
		if (other.m_pActualMCUPhoneNumbers[i] == NULL)
			m_pActualMCUPhoneNumbers[i] = NULL;
		else
			m_pActualMCUPhoneNumbers[i] = new Phone(*other.m_pActualMCUPhoneNumbers[i]);
	}

	for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
	{
		PDELETE(m_pActualPartyPhoneNumbers[i]);
		if (other.m_pActualPartyPhoneNumbers[i] == NULL)
			m_pActualPartyPhoneNumbers[i] = NULL;
		else
			m_pActualPartyPhoneNumbers[i] = new Phone(*other.m_pActualPartyPhoneNumbers[i]);
	}

	m_secondary_cause_params = other.m_secondary_cause_params;
	strncpy(m_operatorName, other.m_operatorName, OPERATOR_NAME_LEN);
	strncpy(m_bondingTmpNumber.phone_number, other.m_bondingTmpNumber.phone_number, PHONE_NUMBER_DIGITS_LEN);

	PDELETE(m_pIpCapabilities);
	PDELETE(m_pIpLocalCommMode);
	PDELETE(m_pIpRemoteCommMode);


	if (other.m_pIpCapabilities == NULL)
		m_pIpCapabilities = NULL;
	else
		m_pIpCapabilities = new CH323StrCap(*other.m_pIpCapabilities);

	if (other.m_pIpLocalCommMode == NULL)
		m_pIpLocalCommMode = NULL;
	else
		m_pIpLocalCommMode = new CH323StrCap(*other.m_pIpLocalCommMode);

	if (other.m_pIpRemoteCommMode == NULL)
		m_pIpRemoteCommMode = NULL;
	else
		m_pIpRemoteCommMode = new CH323StrCap(*other.m_pIpRemoteCommMode);

	m_ipVideoBch            = other.m_ipVideoBch;
	m_ipProtocolSync        = other.m_ipProtocolSync;
	m_ipVideoBchCounter     = other.m_ipVideoBchCounter;
	m_ipProtocolSyncCounter = other.m_ipProtocolSyncCounter;


	for (i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
	{
		POBJDELETE(m_pIpChannelDetails[i]);
		if (other.m_pIpChannelDetails[i] == NULL)
		{
			m_pIpChannelDetails[i] = NULL;
		}
		else
		{
			m_pIpChannelDetails[i] = new CIpChannelDetails(*other.m_pIpChannelDetails[i]);
		}

		POBJDELETE(m_pIpChannelMonitor[i]);
		if (other.m_pIpChannelMonitor[i] == NULL)
			m_pIpChannelMonitor[i] = NULL;
		else
		{
			// There times other does not contain channelType tet so we can't copy immediately other.
			m_pIpChannelMonitor[i] = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)i);
			if (m_pIpChannelMonitor[i])
				m_pIpChannelMonitor[i]->CopyClass(*other.m_pIpChannelMonitor[i]);
		}
	}

	POBJDELETE(m_pH323GatekeeperStatus);
	if (other.m_pH323GatekeeperStatus == NULL)
		m_pH323GatekeeperStatus = NULL;
	else
		m_pH323GatekeeperStatus = new CH323GatekeeperStatus(*other.m_pH323GatekeeperStatus);

	POBJDELETE(m_pSipBWStatus);
	if (other.m_pSipBWStatus == NULL)
		m_pSipBWStatus = NULL;
	else
		m_pSipBWStatus = new CSIPBandwidthAllocationStatus(*other.m_pSipBWStatus);

	memset(&m_backupIpAddress, 0, sizeof(mcTransportAddress));
	memcpy(&m_backupIpAddress, &other.m_backupIpAddress, sizeof(mcTransportAddress));

	strncpy(m_h323BackupPartyAlias, other.m_h323BackupPartyAlias, IP_STRING_LEN);

	m_blockMedia               = other.m_blockMedia;
	m_ipVideoIntraSync         = other.m_ipVideoIntraSync;
	m_netChannelNumber         = other.m_netChannelNumber;
	m_audioVolume              = other.m_audioVolume;
	m_isLeader                 = other.m_isLeader;
	m_wait_for_oper_assistance = other.m_wait_for_oper_assistance;

	strncpy(m_visual_partyName, other.m_visual_partyName, H243_NAME_LEN);
	strncpy(m_remoteName, other.m_remoteName, H243_NAME_LEN);
	strncpy(m_PreDefinedIvrString, other.m_PreDefinedIvrString, H243_NAME_LEN);
	m_bReceiveDtmfFromChairperson = other.m_bReceiveDtmfFromChairperson;

	m_isNoiseDetected    = other.m_isNoiseDetected;
	m_AGC                = other.m_AGC;
	m_EPCContentProvider = other.m_EPCContentProvider;

	if (m_pPartyConfVideoLayout != NULL)
		POBJDELETE(m_pPartyConfVideoLayout);

	if (other.m_pPartyConfVideoLayout != NULL)
		m_pPartyConfVideoLayout = new CVideoLayout(*other.m_pPartyConfVideoLayout);

	POBJDELETE(m_pUserDefinedInfo);
	if (other.m_pUserDefinedInfo == NULL)
		m_pUserDefinedInfo = NULL;
	else
		m_pUserDefinedInfo = new CUserDefinedInfo(*other.m_pUserDefinedInfo);

	m_listening_audioVolume      = other.m_listening_audioVolume;
	m_pRes                       = other.m_pRes;
	m_lobbyId                    = other.m_lobbyId;
	m_isPartyCurrentlyEncrypted  = other.m_isPartyCurrentlyEncrypted;
	m_mipErrorNumber             = other.m_mipErrorNumber;
	m_lpr_sync_status            = other.m_lpr_sync_status;
	m_GatewayPartyType           = other.m_GatewayPartyType;
	m_bTransparentGw             = other.m_bTransparentGw;
	m_isExclusiveContent         = other.m_isExclusiveContent; // Restricted content
	m_bRequestToSpeak            = other.m_bRequestToSpeak;
	m_isEventModeIntraSuppressed = other.m_isEventModeIntraSuppressed;
	m_eventModeLevel             = other.m_eventModeLevel;
	m_lpr_heders_activated       = other.m_lpr_heders_activated;
	m_bFoundInAddrBook           = other.m_bFoundInAddrBook;
	m_bIsTipCall                 = other.m_bIsTipCall;
	m_bIsTipHeader               = other.m_bIsTipHeader;
	m_remoteIdent                = other.m_remoteIdent;
	m_plcmRequireMask            = other.m_plcmRequireMask;
	m_bIsCiscoTagExist           = other.m_bIsCiscoTagExist;
	m_BfcpTransportType          = other.m_BfcpTransportType;
	m_pParty                     = other.m_pParty;
	m_bFlagConnectedOnce         = other.m_bFlagConnectedOnce;

	//VNGR-26449 - unencrypted conference message
	m_isDelPartyWasEncrypted     = other.m_isDelPartyWasEncrypted;
	m_isUnencryptedParty         = other.m_isUnencryptedParty;
	m_bIsLyncPlugin              = other.m_bIsLyncPlugin;
	m_bDefinedPartyAssigned      = other.m_bDefinedPartyAssigned;
	m_MoveType                   = other.m_MoveType;
	m_correlationId              = other.m_correlationId;
	m_partyTypeSubTIP            = other.m_partyTypeSubTIP;
	m_AvMcuLinkType              = other.m_AvMcuLinkType;

	//_mccf_
	strncpy(m_strFromTag, other.m_strFromTag, H243_NAME_LEN);
	strncpy(m_strToTag, other.m_strToTag, H243_NAME_LEN);

	strncpy(m_referredByStr, other.m_referredByStr, H243_NAME_LEN);
	strncpy(m_sipUsrName, other.m_sipUsrName, H243_NAME_LEN);
	
	m_IsCountedInAudioIndication = other.m_IsCountedInAudioIndication;

	m_tokenRecapCollisionDetection	= other.m_tokenRecapCollisionDetection;
	m_trcdInPend					= other.m_trcdInPend;

	m_bSrcTypeIsEQ = other.m_bSrcTypeIsEQ;

	return *this;
}

//--------------------------------------------------------------------------
int CConfParty::DeSerializeXml(CXMLDOMElement* pConfPartyNode, char* pszError)
{
  int             nStatus = STATUS_OK;
  char            str[128];
  CXMLDOMElement* pNode;
  WORD            nTmp;
  CXMLDOMElement* pRsrvPartyNode;
  m_infoOpcode = PARTY_NOT_CHANGED;


  GET_VALIDATE_MANDATORY_CHILD(pConfPartyNode, "PARTY_CHANGE_TYPE", &m_infoOpcode, PARTY_CHANGE_STATE_ENUM);

  GET_MANDATORY_CHILD_NODE(pConfPartyNode, "PARTY", pRsrvPartyNode);

  if (m_infoOpcode == PARTY_NEW_INFO
      || m_infoOpcode == PARTY_COMPLETE_INFO
      || m_infoOpcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_FAST_PLUS_SLOW_INFO)
  {
    nStatus = CRsrvParty::DeSerializeXml(pRsrvPartyNode, pszError, UPDATE_RESERVE);

    if (nStatus != STATUS_OK)
      return nStatus;
  }
  else if (m_infoOpcode == PARTY_FAST_PLUS_SLOW1_INFO || m_infoOpcode == PARTY_FAST_INFO ||
           m_infoOpcode == PARTY_NOT_CHANGED)
  {
    GET_VALIDATE_MANDATORY_CHILD(pRsrvPartyNode, "ID", &m_partyId, _0_TO_DWORD);
  }

  if (m_infoOpcode == PARTY_NOT_CHANGED)
    return STATUS_OK;

  GET_VALIDATE_CHILD(pConfPartyNode, "L_SYNC_LOSS", &m_L_syncLostCounter, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pConfPartyNode, "R_SYNC_LOSS", &m_R_syncLostCounter, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pConfPartyNode, "L_VIDEO_SYNC_LOSS", &m_L_videoSyncLostCounter, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pConfPartyNode, "R_VIDEO_SYNC_LOSS", &m_R_videoSyncLostCounter, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pConfPartyNode, "H323_VIDEO_INTRA_SYNC", &m_ipVideoIntraSync, _BOOL);
  GET_VALIDATE_CHILD(pConfPartyNode, "H323_SYNC", &m_ipProtocolSyncCounter, _0_TO_DWORD);

  if (m_infoOpcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_FAST_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_NEW_INFO
      || m_infoOpcode == PARTY_COMPLETE_INFO)
  {
    pNode = NULL;
    GET_CHILD_NODE(pConfPartyNode, "ONGOING_PARTY_STATUS", pNode);

    if (pNode)
      GET_VALIDATE_CHILD(pNode, "ID", &m_party_state, _0_TO_DWORD);

    pNode = NULL;
    GET_CHILD_NODE(pConfPartyNode, "SECONDARY_CAUSE", pNode);

    if (pNode)
      GET_VALIDATE_CHILD(pNode, "ID", &m_secondary_cause, _0_TO_DWORD);

    pNode = NULL;
    GET_CHILD_NODE(pConfPartyNode, "DISCONNECTION_CAUSE", pNode);

    if (pNode)
      GET_VALIDATE_CHILD(pNode, "ID", &m_disconnect_cause, _0_TO_DWORD);

    CXMLDOMElement* pQ931DisconCauseNode = NULL;

    GET_CHILD_NODE(pConfPartyNode, "Q931_DISCONNECTION_CAUSE", pQ931DisconCauseNode);

    if (pQ931DisconCauseNode)
      GET_VALIDATE_CHILD(pQ931DisconCauseNode, "ID", &m_Q931_disconnect_cause, _0_TO_DWORD);

    GET_VALIDATE_CHILD(pConfPartyNode, "DISCONNECTING_OPERATOR", m_operatorName, _0_TO_OPERATOR_NAME_LENGTH);
    GET_VALIDATE_CHILD(pConfPartyNode, "CONNECT_RETRY", &m_retriesNumber, _0_TO_DWORD);
    GET_VALIDATE_CHILD(pConfPartyNode, "CONNECT_TIME", &m_connectTime, DATE_TIME);
    GET_VALIDATE_CHILD(pConfPartyNode, "DISCONNECT_TIME", &m_disconnectTime, DATE_TIME);
    GET_VALIDATE_CHILD(pConfPartyNode, "AUDIO_MUTE", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetAudioMuteByOperator(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "AUDIO_SELF_MUTE", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetAudioMuteByParty(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "AUDIO_MCU_MUTE", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetAudioMuteByMCU(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "AUDIO_BLOCK", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetAudioBlocked(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "VIDEO_MUTE", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetVideoMuteByOperator(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "VIDEO_SELF_MUTE", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetVideoMuteByParty(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "VIDEO_MCU_MUTE", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetVideoMuteByMCU(nTmp);

    if (!m_pAttendedStruct)
      m_pAttendedStruct = new CAttendedStruct;

    GET_VALIDATE_CHILD(pConfPartyNode, "ATTENDING_STATE", &(m_pAttendedStruct->m_ordinary_party), ATTENDING_STATE_ENUM);
    GET_VALIDATE_CHILD(pConfPartyNode, "OPERATOR_PARTY", &(m_isOperatorParty), _BOOL);
    GET_VALIDATE_CHILD(pConfPartyNode, "AUTO_ADD", &m_undefinedType, _BOOL);

    CXMLDOMElement* pChannelsNode, * pActMCUPhonesNode, * pActPartyPhonesNode, * pNode, * pTempNode;
    char            szPhoneNumber[PHONE_NUMBER_DIGITS_LEN];
    szPhoneNumber[0] = '\0';

    GET_CHILD_NODE(pConfPartyNode, "CHANNELS", pChannelsNode);

    if (pChannelsNode)
    {
      for (int i = 1; i < 7; i++)
      {
        sprintf(str, "CH_%d", i);
        GET_VALIDATE_CHILD(pChannelsNode, str, &nTmp, _BOOL);

        if (nStatus == STATUS_OK)
          SetNet_channels(nTmp, i, 1, 0);
      }

      GET_CHILD_NODE(pChannelsNode, "CHANNEL_LIST_EX", pNode);
      if (pNode)
      {
        GET_FIRST_CHILD_NODE(pNode, "CHANNEL_EX", pTempNode);
        for (int i = 0; pTempNode; ++i)
        {
          GET_VALIDATE(pTempNode, &nTmp, _BOOL);

          if (nStatus == STATUS_OK)
            SetNet_channels(nTmp, i, 1, 0);

          GET_NEXT_CHILD_NODE(pNode, "CHANNEL_EX", pTempNode);
        }
      }
    }

    GET_CHILD_NODE(pConfPartyNode, "ACTUAL_MCU_PHONES", pActMCUPhonesNode);

    if (pActMCUPhonesNode)
    {
      for (int i = 0; i < 6; i++) // i<GetNetChannelNumber()
      {
        szPhoneNumber[0] = '\0';
        sprintf(str, "PHONE%d", i+1);
        GET_VALIDATE_CHILD(pActMCUPhonesNode, str, szPhoneNumber, PHONE_NUMBER_DIGITS_LENGTH);

        if (nStatus == STATUS_OK && strlen(szPhoneNumber) > 0)
          SetActualMCUPhoneNumber(i, szPhoneNumber);
      }

      GET_CHILD_NODE(pActMCUPhonesNode, "PHONE_LIST_EX", pNode);
      if (pNode)
      {
        GET_FIRST_CHILD_NODE(pNode, "PHONE", pTempNode);
        for (int i = 0; pTempNode; ++i)
        {
          szPhoneNumber[0] = '\0';
          GET_VALIDATE(pTempNode, szPhoneNumber, PHONE_NUMBER_DIGITS_LENGTH);

          if (nStatus == STATUS_OK && strlen(szPhoneNumber) > 0)
            SetActualMCUPhoneNumber(i, szPhoneNumber);

          GET_NEXT_CHILD_NODE(pNode, "PHONE", pTempNode);
        }
      }
    }

    GET_CHILD_NODE(pConfPartyNode, "ACTUAL_PARTY_PHONES", pActPartyPhonesNode);

    if (pActPartyPhonesNode)
    {
      for (int i = 0; i < 6; i++) // i<GetNetChannelNumber()
      {
        szPhoneNumber[0] = '\0';
        sprintf(str, "PHONE%d", i+1);
        GET_VALIDATE_CHILD(pActPartyPhonesNode, str, szPhoneNumber, PHONE_NUMBER_DIGITS_LENGTH);

        if (nStatus == STATUS_OK && strlen(szPhoneNumber) > 0)
          SetActualPartyPhoneNumber(i, szPhoneNumber);
      }

      GET_CHILD_NODE(pActPartyPhonesNode, "PHONE_LIST_EX", pNode);
      if (pNode)
      {
        GET_FIRST_CHILD_NODE(pNode, "PHONE", pTempNode);
        for (int i = 0; pTempNode; ++i)
        {
          szPhoneNumber[0] = '\0';
          GET_VALIDATE(pTempNode, szPhoneNumber, PHONE_NUMBER_DIGITS_LENGTH);

          if (nStatus == STATUS_OK && strlen(szPhoneNumber) > 0)
            SetActualPartyPhoneNumber(i, szPhoneNumber);

          GET_NEXT_CHILD_NODE(pNode, "PHONE", pTempNode);
        }
      }
    }

    GET_VALIDATE_CHILD(pConfPartyNode, "AUDIO_MEMBER", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetAudio_Member(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "VIDEO_MEMBER", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetVideo_Member(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "LEADER", &m_isLeader, _BOOL);
    GET_VALIDATE_CHILD(pConfPartyNode, "WAIT_FOR_ASSISTANCE", &m_wait_for_oper_assistance, WAIT_FOR_ASSISTANCE_ENUM);
    GET_VALIDATE_CHILD(pConfPartyNode, "SRC_TYPE_IS_EQ", &m_bSrcTypeIsEQ, _BOOL);


    if (!m_pVideoLayout)
      m_pVideoLayout = new CVideoLayout;

    GET_CHILD_NODE(pConfPartyNode, "FORCE", pNode);

    if (pNode)
    {
      nStatus = m_pVideoLayout->DeSerializeXml(pNode, pszError, m_isPrivate);

      if (nStatus != STATUS_OK)
      {
        POBJDELETE(m_pVideoLayout);
        return nStatus;
      }
    }

    if (IsIpNetInterfaceType())
    {
      GET_CHILD_NODE(pConfPartyNode, "GK_STATUS", pNode);

      if (pNode)
      {
        CH323GatekeeperStatus* pH323GatekeeperStatus = new CH323GatekeeperStatus;
        nStatus = pH323GatekeeperStatus->DeSerializeXml(pNode, pszError);
        if (nStatus != STATUS_OK)
        {
          POBJDELETE(pH323GatekeeperStatus);
          return nStatus;
        }
        POBJDELETE(m_pH323GatekeeperStatus);
        m_pH323GatekeeperStatus = pH323GatekeeperStatus;
      }

      GET_CHILD_NODE(pConfPartyNode, "CAC_STATUS", pNode);

      if (pNode)
      {
        CSIPBandwidthAllocationStatus* pSipBWStatus = new CSIPBandwidthAllocationStatus;
        nStatus = pSipBWStatus->DeSerializeXml(pNode, pszError);
        if (nStatus != STATUS_OK)
        {
          POBJDELETE(pSipBWStatus);
          return nStatus;
        }

        POBJDELETE(m_pSipBWStatus);
        m_pSipBWStatus = pSipBWStatus;
      }
    }
  }

  GET_VALIDATE_CHILD(pConfPartyNode, "TX_BAUD_RATE", &m_trmBaudRate, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pConfPartyNode, "RX_BAUD_RATE", &m_rcvBaudRate, _0_TO_DWORD);

  if (m_infoOpcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_FAST_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_NEW_INFO
      || m_infoOpcode == PARTY_COMPLETE_INFO)
  {
    GET_VALIDATE_CHILD(pConfPartyNode, "NOISY", &m_isNoiseDetected, _BOOL);
  }

  if (m_infoOpcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_FAST_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_NEW_INFO
      || m_infoOpcode == PARTY_COMPLETE_INFO)
  {
    GET_VALIDATE_CHILD(pConfPartyNode, "CONTENT_MEMBER", &nTmp, _BOOL);

    if (nStatus == STATUS_OK)
      SetContent_Member(nTmp);

    GET_VALIDATE_CHILD(pConfPartyNode, "CONTENT_PROVIDER", &m_EPCContentProvider, _BOOL);
  }

  if (m_infoOpcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_FAST_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_NEW_INFO
      || m_infoOpcode == PARTY_COMPLETE_INFO)
  {
    GET_VALIDATE_CHILD(pConfPartyNode, "VISUAL_NAME", m_visual_partyName, _0_TO_H243_NAME_LENGTH);
    if ((m_visual_partyName != NULL) && strcmp(m_visual_partyName, ""))
    {
      GET_VALIDATE_CHILD(pConfPartyNode, "VISUAL_NAME", m_H243_partyName, _0_TO_H243_NAME_LENGTH);
    }
  }

  if (m_infoOpcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_FAST_PLUS_SLOW_INFO
      || m_infoOpcode == PARTY_NEW_INFO
      || m_infoOpcode == PARTY_COMPLETE_INFO)
  {
    pNode = NULL;
    GET_CHILD_NODE(pConfPartyNode, "SECONDARY_CAUSE_PARAMS", pNode);
    if (pNode)
    {
      nStatus = m_secondary_cause_params.DeSerializeXml(pNode, pszError);
      if (nStatus != STATUS_OK)
        return nStatus;
    }
  }

  GET_VALIDATE_CHILD(pConfPartyNode, "IS_SYNC_LOSS", &nTmp, _BOOL);
  if (nStatus == STATUS_OK)
    SetSyncLoss(nTmp);

  GET_VALIDATE_CHILD(pConfPartyNode, "IS_RV_SYNC_LOSS", &nTmp, _BOOL);
  if (nStatus == STATUS_OK)
    Set_M_SyncLoss(nTmp);

  GET_VALIDATE_CHILD(pConfPartyNode, "IS_R_SYNC_LOSS", &nTmp, _BOOL);
  if (nStatus == STATUS_OK)
    Set_R_SyncLoss(nTmp);

  GET_VALIDATE_CHILD(pConfPartyNode, "IS_LV_SYNC_LOSS", &nTmp, _BOOL);
  if (nStatus == STATUS_OK)
    Set_L_Video_SyncLoss(nTmp);

  if (!m_pPartyConfVideoLayout)
    m_pPartyConfVideoLayout = new CVideoLayout;

  GET_CHILD_NODE(pConfPartyNode, "PARTY_CONF_FORCE", pNode);

  if (pNode)
  {
    nStatus = m_pPartyConfVideoLayout->DeSerializeXml(pNode, pszError, FALSE);

    if (nStatus != STATUS_OK)
    {
      POBJDELETE(m_pPartyConfVideoLayout);
      return nStatus;
    }
  }

  GET_VALIDATE_CHILD(pConfPartyNode, "IS_CURRENTLY_ENCRYPTED", &m_isPartyCurrentlyEncrypted, _BOOL);
  GET_VALIDATE_CHILD(pConfPartyNode, "R_LPR_ACTIVATION", &nTmp, _BOOL);
  if (nStatus == STATUS_OK)
    Set_Lpr_Rmt_SyncLoss(nTmp);

  GET_VALIDATE_CHILD(pConfPartyNode, "L_LPR_ACTIVATION", &nTmp, _BOOL);
  if (nStatus == STATUS_OK)
    Set_Lpr_Local_SyncLoss(nTmp);

  GET_VALIDATE_CHILD(pConfPartyNode, "IS_EXCLUSIVE_CONTENT", &m_isExclusiveContent, _BOOL); // Restricted content
  GET_VALIDATE_CHILD(pConfPartyNode, "LPR_HEADERS_ACTIVATION", &m_lpr_heders_activated, _BOOL);
  GET_VALIDATE_CHILD(pConfPartyNode, "TIP_MODE", &m_partyTypeTIP, TIP_PARTY_TYPE_ENUM);


  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CConfParty::SerializeDetailsPartyXml(CXMLDOMElement* pConfPartyNode)
{
  // Capabilities and communication modes serializing
  SerializeDetailsPartyXml_H221CommAndCap(pConfPartyNode);
  SerializeDetailsPartyXml_IpCommAndCap(pConfPartyNode);
  SerializeDetailsPartyXml_IpMonitoring(pConfPartyNode);
}

//--------------------------------------------------------------------------
void CConfParty::SerializeDetailsPartyXml_IpCommAndCap(CXMLDOMElement* pConfPartyNode)
{
  if (m_pIpLocalCommMode)
  {
    COstrStream  Ostr;
    CH323strCom* pIpLocalCommMode = new CH323strCom((CH323strCom&)*m_pIpLocalCommMode);
    pIpLocalCommMode->Dump(Ostr); // convert H221 format(array of bytes) into a string
    pConfPartyNode->AddChildNode("H323_LOCAL_COMM_MODE", Ostr);
    POBJDELETE(pIpLocalCommMode);
  }

  if (m_pIpRemoteCommMode)
  {
    COstrStream  Ostr;
    CH323strCom* pIpRemoteCommMode = new CH323strCom((CH323strCom&)*m_pIpRemoteCommMode);

    pIpRemoteCommMode->Dump(Ostr);
    pConfPartyNode->AddChildNode("H323_REMOTE_COMM_MODE", Ostr);

    POBJDELETE(pIpRemoteCommMode);
  }

  if (m_pIpCapabilities)
  {
    COstrStream  Ostr;
    CH323StrCap* pIpCapabilities = new CH323StrCap((CH323StrCap&)*m_pIpCapabilities);

    pIpCapabilities->Dump(Ostr);
    pConfPartyNode->AddChildNode("H323_REMOTE_CAP", Ostr);

    POBJDELETE(pIpCapabilities);
  }
}

//--------------------------------------------------------------------------
void CConfParty::SerializeDetailsPartyXml_H221CommAndCap(CXMLDOMElement* pConfPartyNode)
{
  if (m_pLocalCommMode)
  {
    COstrStream  Ostr;
    CH221strCom* pLocalCommMode = new CH221strCom((CH221strCom&)*m_pLocalCommMode);

    pLocalCommMode->Dump(Ostr);
    pConfPartyNode->AddChildNode("LOCAL_COMM_MODE", Ostr);

    POBJDELETE(pLocalCommMode);
  }

  if (m_pRemoteCommMode)
  {
    COstrStream  Ostr;
    CH221strCom* pRemoteCommMode = new CH221strCom((CH221strCom&)*m_pRemoteCommMode);

    pRemoteCommMode->Dump(Ostr);
    pConfPartyNode->AddChildNode("REMOTE_COMM_MODE", Ostr);

    POBJDELETE(pRemoteCommMode);
  }

  if (m_pCapabilities)
  {
    COstrStream  Ostr;
    CH221strCap* pCapabilities = new CH221strCap((CH221strCap&)*m_pCapabilities);

    pCapabilities->Dump(Ostr);
    pConfPartyNode->AddChildNode("REMOTE_CAP", Ostr);

    POBJDELETE(pCapabilities);
  }
}

//--------------------------------------------------------------------------
void CConfParty::SerializeDetailsPartyXml_IpMonitoring(CXMLDOMElement* pConfPartyNode)
{
  CXMLDOMElement* pParentNode;
  pParentNode = pConfPartyNode->AddChildNode("IP_MONITOR_CHANNELS");
  for (int i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
  {
    if (m_pIpChannelMonitor[i])
    {
      if (i != 0 && (m_pIpChannelMonitor[i]->GetChannelType() == H225 || m_pIpChannelMonitor[i]->GetChannelType() == BFCP_IN || m_pIpChannelMonitor[i]->GetChannelType() == BFCP_OUT))
        continue;

      // Anna - we send only relevant bfcp channel (according to transport type)
      if ((m_BfcpTransportType == eTransportTypeUdp && m_pIpChannelMonitor[i]->GetChannelType() == BFCP) ||
          (m_BfcpTransportType == eUnknownTransportType && (m_pIpChannelMonitor[i]->GetChannelType() == BFCP_UDP || m_pIpChannelMonitor[i]->GetChannelType() == BFCP)) ||
          ((m_BfcpTransportType == eTransportTypeTcp || m_BfcpTransportType == eTransportTypeTls) && m_pIpChannelMonitor[i]->GetChannelType() == BFCP_UDP))
        continue;

      CXMLDOMElement* pNode = pParentNode->AddChildNode("IP_MONITOR_CHANNEL");
      m_pIpChannelMonitor[i]->SerializeXml(pNode);
    }
  }
}

//--------------------------------------------------------------------------
void CConfParty::SerializeSpecPartyXml(CXMLDOMElement* pActionNode)
{
// SpecParty contains H221, H243 and RsrcDetails info
  CXMLDOMElement* pTempNode;
  CXMLDOMElement* pConfPartyNode = pActionNode->AddChildNode("ONGOING_PARTY");
  // OnGoing_party contains the relevant info

  CXMLDOMElement* pPartyNode = pConfPartyNode->AddChildNode("PARTY");
  pPartyNode->AddChildNode("NAME", m_H243_partyName);
  pPartyNode->AddChildNode("ID", m_partyId);
  pPartyNode->AddChildNode("INTERFACE", m_netInterfaceType, INTERFACE_ENUM);

  //VNGR-25242, map '0' to 'AUTO'
  BYTE	ucNetChannelNumber 	= AUTO; 
  if(0 != m_netChannelNumber)
  {
  	ucNetChannelNumber = m_netChannelNumber;
  }
  pPartyNode->AddChildNode("NET_CHANNEL_NUMBER", ucNetChannelNumber, NET_CHANNEL_NUMBER_ENUM);
  pPartyNode->AddChildNode("ENDPOINT_MEDIA_TYPE", m_ePartyMediaType, PARTY_MEDIA_TYPE_ENUM);
  SerializeDetailsPartyXml(pConfPartyNode);
}

//--------------------------------------------------------------------------
void CConfParty::SetVideoLayout(const CVideoLayout& other)
{
  POBJDELETE(m_pVideoLayout);
  m_pVideoLayout = new CVideoLayout(other);

  SLOWCHANGE;

  if (CPObject::IsValidPObjectPtr(m_pSubject))
    m_pSubject->Notify(CPPARTYLAYOUT, 0);
}

//--------------------------------------------------------------------------
void CConfParty::SetPartyState(const DWORD partyState, DWORD confId)
{
  m_party_state = partyState;
  SLOWCHANGE;

  if (m_party_state == PARTY_DISCONNECTED) // dirtel bug
  {
    m_net_channels                            = 0;
    m_bIsFirstConnectionAfterHotBackupRestore = FALSE;
  }

  m_pSubject->Notify(PARTYSTATE, m_party_state);
}

//--------------------------------------------------------------------------
// in order to activate PCAS event when the CONTACT_INFO arrive from the external DB
void CConfParty::UpdateExtDBUserInfo(const char* UserInfo, int InfoNumber)
{
  //if (0 == InfoNumber)
    m_pSubject->Notify(CONTACT_INFO, InfoNumber);
}


//--------------------------------------------------------------------------
// Function name: IsH323paramInclude                    written by: Uri Avni
// Variables:     partyIPaddress: The IP address of the dial in setup message.
// pH323AliasArray: The source aliases.
// wNumAlias: Source number of aliases.
// Description:   Find if there a match between the party from DB (this) and the incoming call
// Return value: 0 - for not finding any match
// 1 - for finding only alias match
// 2 - for finding Ip only match
// 3 - for finding Ip and Alias match
WORD CConfParty::IsH323paramInclude(mcTransportAddress* pPartyIPaddress, CH323Alias* pH323AliasArray, WORD wNumAlias)
{
  if (IsIpNetInterfaceType() == NO)
    return 0;

  WORD result         = 0;
  WORD isCompareAlias = 0;

  // IpV6
  // check if alias is an IP address
  if (isApiTaNull(pPartyIPaddress) || isIpTaNonValid(pPartyIPaddress))
    if (pH323AliasArray)
      stringToIp(pPartyIPaddress, (char*)pH323AliasArray[0].GetAliasName());

  if (!isApiTaNull(pPartyIPaddress) && !isIpTaNonValid(pPartyIPaddress))
  {
    mcTransportAddress rsrvIpAddress = GetIpAddress();
    if (isIpAddressEqual(&rsrvIpAddress, pPartyIPaddress))
      result = 2;
    else if (!isApiTaNull(&rsrvIpAddress))  // if we defined the party with IP and the IP doesn't match
    { // the remote IP we return 'no match'
      PTRACE2(eLevelInfoNormal, "CConfParty::IsH323paramInclude : \'No Ip Match, Name: \'", GetName());
      return 0;
    }
  }

	if (pH323AliasArray)
  {
    for (WORD i = 0; i < wNumAlias; i++)
    {
      if (CPObject::IsValidPObjectPtr(&pH323AliasArray[i]))
      {
        if (pH323AliasArray[i].GetAliasName())
        {        	
        	char strTmp[H243_NAME_LEN+4];
			//memset(strTmp,0,sizeof(strTmp));
			snprintf(strTmp,sizeof(strTmp)-1,"\"%s\"",GetName());
			strTmp[sizeof(strTmp ) -1] = '\0' ;
         	if (!strcmp(GetH323PartyAlias(), pH323AliasArray[i].GetAliasName())
              || !strcmp(GetSipPartyAddress(), pH323AliasArray[i].GetAliasName())
              || !strcmp(strTmp, pH323AliasArray[i].GetAliasName()))
          	{
            	result++;
           	 	break;
          	}
        
        }

        isCompareAlias = 1;
      }
      else
        break;
    }

    if ((GetH323PartyAlias()[0] != '\0' || GetSipPartyAddress()[0] != '\0') && isCompareAlias && (result != 1) && (result != 3))
    {
      PTRACE2(eLevelInfoNormal, "CCConfParty::IsH323paramInclude : \'No Alias Match, Name: \'", GetName());
      return 0; // bouth remote and party has aliases but the aliases are different
    }
  }
  else if (GetH323PartyAlias()[0] != '\0' || GetSipPartyAddress()[0] != '\0')
  {
    PTRACE2(eLevelInfoNormal, "CCConfParty::IsH323paramInclude : \'Alias for remote, though EP is configured with alias at EMA,  Name: \'", GetName());
    return 0;   // if we defined alias at the party then an incoming call has to come with an alias.
  }

	return result;
}
/////////////////////////////////////////////////////////////////////////////
// Function name: IsH323AliasInclude 
// Variables:     pH323AliasArray: The source aliases.
//                wNumAlias: Source number of aliases.
// Description:	  Find if there a match between the party from DB (this) and the incoming call
//                      
//			without checking for IP address.
//			  
// Return value: 0 - for not finding any match
//				 1 - for finding only alias match
/////////////////////////////////////////////////////////////////////////////
WORD  CConfParty::IsH323AliasInclude(CH323Alias* pH323AliasArray, WORD wNumAlias)           
{
	
	if (IsIpNetInterfaceType() == NO)
		return 0;
	
	WORD  result		 = 0;
	WORD  isCompareAlias = 0;	
	

	if (pH323AliasArray)
	{
		for (WORD i=0; i<wNumAlias; i++)
		{
			if (CPObject::IsValidPObjectPtr(&pH323AliasArray[i]))
			{
				if(pH323AliasArray[i].GetAliasName())
					if (!strcmp(GetH323PartyAlias(), pH323AliasArray[i].GetAliasName())
						|| !strcmp(GetSipPartyAddress(), pH323AliasArray[i].GetAliasName()))
					{
						result++;
						break;
					}
				isCompareAlias = 1;
			}
			else
				break;
		}
		if((GetH323PartyAlias()[0] != '\0' || GetSipPartyAddress()[0] != '\0') && isCompareAlias && (result != 1) && (result != 3))
		{
			PTRACE2(eLevelInfoNormal,"CCConfParty::IsH323paramInclude : \'No Alias Match, Name: \'", GetName());
			return 0; // bouth remote and party has aliases but the aliases are different
		}
	}
	else if( GetH323PartyAlias()[0] != '\0'|| GetSipPartyAddress()[0] != '\0')
	{
		PTRACE2(eLevelInfoNormal,"CCConfParty::IsH323paramInclude : \'Alias for remote, though EP is configured with alias at EMA,  Name: \'", GetName());
		return 0; // if we defined alias at the party then an incoming call has to come with an alias.	
	}
  return result;
}

//--------------------------------------------------------------------------
void CConfParty::SetSyncLoss(WORD bl)
{
  DWORD temp = m_H221_status;

  if (bl == FALSE)
  {
    m_H221_status &= 0xFFFFFFFE;
  }
  else
  {
    m_H221_status |= 0x00000001;
    L_syncLostCounterIncrease();
  }

  if (temp != m_H221_status)
    SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::Set_M_SyncLoss(WORD bl)
{
  DWORD temp = m_H221_status;

  if (bl == FALSE)
  {
    m_H221_status &= 0xFFFFFFFD;
  }
  else
  {
    m_H221_status |= 0x00000002;
    R_videoSyncLostCounterIncrease();
  }

  if (temp != m_H221_status)
    SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::Set_R_SyncLoss(WORD bl)
{
  DWORD temp = m_H221_status;

  if (bl == FALSE)
    m_H221_status &= 0xFFFFFFFB;
  else
  {
    m_H221_status |= 0x00000004;
    R_syncLostCounterIncrease();
  }

  if (temp != m_H221_status)
    SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::Set_Lpr_Rmt_SyncLoss(WORD bl)
{
  DWORD temp = m_lpr_sync_status;

  if (bl == FALSE)
  {
    m_lpr_sync_status &= 0xFFFFFFFD;
  }
  else
  {
    m_lpr_sync_status |= 0x00000002;
  }

  if (temp != m_lpr_sync_status)
    SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::Set_L_Video_SyncLoss(WORD bl)
{
  DWORD temp = m_H221_status;

  if (bl == FALSE)
    m_H221_status &= 0xFFFFFFF7;
  else
  {
    m_H221_status |= 0x00000008;
    L_videoSyncLostCounterIncrease();
  }

  if (temp != m_H221_status)
    SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::Set_Lpr_Local_SyncLoss(WORD bl)
{
  DWORD temp = m_lpr_sync_status;

  if (bl == FALSE)
  {
    m_lpr_sync_status &= 0xFFFFFFF7;
  }
  else
  {
    m_lpr_sync_status |= 0x00000008;
  }

  if (temp != m_lpr_sync_status)
    SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::Set_Is_Lpr_Headers_Activated(BOOL bl)
{
  BOOL temp = m_lpr_heders_activated;

  if (bl == NO)
  {
    m_lpr_heders_activated = NO;
  }
  else
  {
    PTRACE2(eLevelInfoNormal, "CCConfParty::Set_Is_Lpr_Headers_Activated :YES' ", GetName());
    m_lpr_heders_activated = YES;
  }

  if (temp != m_lpr_heders_activated)
    SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetDisconnectCause(const DWORD disconnectCause, DWORD MipErrorNumber)
{
  m_disconnect_cause = disconnectCause;
  if (m_mipErrorNumber == 0)
    m_mipErrorNumber = MipErrorNumber;

  // Reset the m_mipErrorNumber incase of no disconnection cause
  if (m_disconnect_cause == NO_DISCONNECTION_CAUSE)
    m_mipErrorNumber = 0;

  m_pSubject->Notify(DISCAUSE, m_disconnect_cause);

  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetQ931DisconnectCause(const DWORD Q931disconnectCause)
{
  m_Q931_disconnect_cause = Q931disconnectCause;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetMipErrorNumber(const DWORD mipErrorNumber)
{
  m_mipErrorNumber = mipErrorNumber;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetSecondaryCause(const BYTE secondaryCause)
{
  m_secondary_cause = secondaryCause;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetSecondaryCauseParams(CSecondaryParams secParam)
{
  m_secondary_cause_params = secParam;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetAudio_Member(WORD bl)
{
  if (bl == FALSE)
    m_member_in_media &= 0xFFFFFFFE;
  else
    m_member_in_media |= 0x00000001;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetVideo_Member(WORD bl)
{
  if (bl == FALSE)
    m_member_in_media &= 0xFFFFFFFD;
  else
    m_member_in_media |= 0x00000002;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetContent_Member(WORD bl)
{
  if (bl == FALSE)
    m_member_in_media &= 0xFFFFFFDF;
  else
    m_member_in_media |= 0x00000020;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetAudioMuteByOperator(WORD bl)
{
  if (bl == FALSE)
    m_mute_byOperator &= 0xFFFFFFFE;
  else
    m_mute_byOperator |= 0x00000001;

  m_pSubject->Notify(MUTE_STATE, 0); // talya to check
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetVideoMuteByOperator(WORD bl)
{
  if (bl == FALSE)
    m_mute_byOperator &= 0xFFFFFFFD;
  else
    m_mute_byOperator |= 0x00000002;

  m_pSubject->Notify(MUTE_STATE, 2); // talya to check
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetNet_channels(WORD onOff, WORD firstPort, WORD numPorts, WORD call_type)
{
  for (WORD i = firstPort; i < firstPort + numPorts; i++)
  {
    DWORD ch = (DWORD)1 << (i-1);

    if (onOff == TRUE)
      m_net_channels |= ch;
    else
      m_net_channels &= (DWORD)(0xFFFFFFFF - ch);
  }
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
WORD CConfParty::IsChannelConnected(const DWORD channel) const
{
  DWORD ch = (DWORD)1 << (channel-1);
  if ((m_net_channels & ch) == ch)
    return TRUE;
  else
    return FALSE;
}

//--------------------------------------------------------------------------
WORD CConfParty::GetNumChannelConnected() const
{
  DWORD m_net_channelsCopy = m_net_channels;
  WORD  count = 0;
  while (m_net_channelsCopy)
  {
    count++;
    // sets the rightmost 1 bit in m_net_channelsCopy to 0
    m_net_channelsCopy &= (m_net_channelsCopy - 1);
  }
  return count;
}

//--------------------------------------------------------------------------
void CConfParty::SetAudioMuteByParty(WORD bl)
{
  if (bl == FALSE)
    m_mute_byParty &= 0xFFFFFFFE;
  else
    m_mute_byParty |= 0x00000001;

  m_pSubject->Notify(MUTE_STATE, 0); // talya to check
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetVideoMuteByParty(WORD bl)
{
  if (bl == FALSE)
    m_mute_byParty &= 0xFFFFFFFD;
  else
    m_mute_byParty |= 0x00000002;

  m_pSubject->Notify(MUTE_STATE, 2); // talya to check
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetAudioMuteByMCU(const WORD bl)
{
  if (bl == FALSE)
    m_mute_byMCU &= 0xFFFFFFFE;
  else
    m_mute_byMCU |= 0x00000001;

  m_pSubject->Notify(MUTE_STATE, 0); // talya to check
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetVideoMuteByMCU(const WORD bl)
{
  if (bl == FALSE)
    m_mute_byMCU &= 0xFFFFFFFD;
  else
    m_mute_byMCU |= 0x00000002;

  m_pSubject->Notify(MUTE_STATE, 2); // talya to check
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetCapabilities(const CH221Str& other)
{
  POBJDELETE(m_pCapabilities);
  m_pCapabilities = new CH221Str(other);
}

//--------------------------------------------------------------------------
void CConfParty::SetLocalCommMode(const CH221Str& other)
{
  POBJDELETE(m_pLocalCommMode);
  m_pLocalCommMode = new CH221Str(other);
}

//--------------------------------------------------------------------------
void CConfParty::SetRemoteCommMode(const CH221Str& other)
{
  POBJDELETE(m_pRemoteCommMode);
  m_pRemoteCommMode = new CH221Str(other);
}

//--------------------------------------------------------------------------
void CConfParty::SetConnectTime(const CStructTm& other)
{
  m_connectTime = other;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetDisconnectTime(const CStructTm& other)
{
  m_disconnectTime = other;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetRetriesNumber(const WORD retNumber)
{
  m_retriesNumber = retNumber;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetOrdinaryParty(const BYTE ordinary_party)
{
  m_pAttendedStruct->SetOrdinaryParty(ordinary_party);
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
WORD CConfParty::IsProblem() const
{
	if (IsVideo_Member() && Is_M_SyncLoss())
		return 1; // M sync loss

	if (IsVideo_Member() && Is_L_Video_SyncLoss() && !(GetRecordingPort() || GetRecordingLinkParty()))
		return 2; // L sync loss

	/*  MSSlave-Flora Yao-2013/11/05- MSSlave Flora Comment: For MSSlave Party, There is no audio */
	if (GetAvMcuLinkType() == eAvMcuLinkNone || GetAvMcuLinkType() == eAvMcuLinkMain)
	{
		if (!IsAudio_Member())
			return 3; // No audio
	}

	if (!IsVideo_Member() && !m_voice && !IsContent_Member())
		return 4; // Not audio only and no video

	return 0;
}

//--------------------------------------------------------------------------
void CConfParty::SetTransmitBaudRate(const DWORD baudRate)
{
  m_trmBaudRate = baudRate;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetReceiveBaudRate(const DWORD baudRate)
{
  m_rcvBaudRate = baudRate;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::L_syncLostCounterIncrease()
{
  m_L_syncLostCounter++;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetL_syncLostCounter(const WORD num)
{
  m_L_syncLostCounter = num;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::R_syncLostCounterIncrease()
{
  m_R_syncLostCounter++;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetR_syncLostCounter(const WORD num)
{
  m_R_syncLostCounter = num;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::L_videoSyncLostCounterIncrease()
{
  m_L_videoSyncLostCounter++;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetL_videoSyncLostCounter(const WORD num)
{
  m_L_videoSyncLostCounter = num;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SerializeXml(CXMLDOMElement* pConfPartyNode, int Opcode, ePartyData party_data_amount)
{
  CXMLDOMElement* pNode, * pParentNode;
  char            str[128];

  if (party_data_amount == FULL_DATA)
  {
    pConfPartyNode->AddChildNode("L_SYNC_LOSS", m_L_syncLostCounter);
    pConfPartyNode->AddChildNode("R_SYNC_LOSS", m_R_syncLostCounter);
    pConfPartyNode->AddChildNode("L_VIDEO_SYNC_LOSS", m_L_videoSyncLostCounter);
    pConfPartyNode->AddChildNode("R_VIDEO_SYNC_LOSS", m_R_videoSyncLostCounter);
    pConfPartyNode->AddChildNode("H323_VIDEO_INTRA_SYNC", m_ipVideoIntraSync, _BOOL);
    pConfPartyNode->AddChildNode("H323_SYNC", m_ipProtocolSyncCounter);
  }

  if (Opcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO || Opcode == PARTY_FAST_PLUS_SLOW_INFO || Opcode == PARTY_NEW_INFO)
  {
    pNode = pConfPartyNode->AddChildNode("ONGOING_PARTY_STATUS");
    pNode->AddChildNode("ID", m_party_state);
    
    pNode->AddChildNode("DESCRIPTION", m_party_state, ONGOING_PARTY_STATUS_ENUM);

    if (party_data_amount == FULL_DATA)
    {
      if ((m_secondary_cause != 0xFF && (m_party_state == PARTY_SECONDARY || m_party_state == PARTY_CONNECTED_PARTIALY)) ||
          (m_secondary_cause == SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD || m_secondary_cause == SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD))
      {
        pNode = pConfPartyNode->AddChildNode("SECONDARY_CAUSE");
        pNode->AddChildNode("ID", m_secondary_cause);
        pNode->AddChildNode("DESCRIPTION", m_secondary_cause, SECONDARY_CAUSE_ENUM);
      }

      {
        pNode = pConfPartyNode->AddChildNode("DISCONNECTION_CAUSE");
        

        pNode->AddChildNode("ID", m_disconnect_cause);
        pNode->AddChildNode("DESCRIPTION", m_disconnect_cause, DISCONNECTION_CAUSE_ENUM);
        pNode->AddChildNode("DESCRIPTION_EX", m_mipErrorNumber);
        
      }
      pConfPartyNode->AddChildNode("DISCONNECTING_OPERATOR", m_operatorName);
      pConfPartyNode->AddChildNode("CONNECT_RETRY", m_retriesNumber);

      if (m_connectTime.m_mon > 0)
        pConfPartyNode->AddChildNode("CONNECT_TIME", m_connectTime);

      if (m_disconnectTime.m_mon > 0)
        pConfPartyNode->AddChildNode("DISCONNECT_TIME", m_disconnectTime);
    }

    pConfPartyNode->AddChildNode("AUDIO_MUTE", IsAudioMutedByOperator(), _BOOL);
    pConfPartyNode->AddChildNode("AUDIO_SELF_MUTE", IsAudioMutedByParty(), _BOOL);
    pConfPartyNode->AddChildNode("AUDIO_MCU_MUTE", IsAudioMutedByMCU(), _BOOL);

    if (party_data_amount == FULL_DATA)
      pConfPartyNode->AddChildNode("AUDIO_BLOCK", IsAudioBlocked(), _BOOL);

    pConfPartyNode->AddChildNode("VIDEO_MUTE", IsVideoMutedByOperator(), _BOOL);
    pConfPartyNode->AddChildNode("VIDEO_SELF_MUTE", IsVideoMutedByParty(), _BOOL);
    pConfPartyNode->AddChildNode("VIDEO_MCU_MUTE", IsVideoMutedByMCU(), _BOOL);

    if (m_pAttendedStruct)
    {
      pConfPartyNode->AddChildNode("ATTENDING_STATE", m_pAttendedStruct->m_ordinary_party, ATTENDING_STATE_ENUM);
    }

    pConfPartyNode->AddChildNode("OPERATOR_PARTY", m_isOperatorParty, _BOOL);

    if (party_data_amount == FULL_DATA)
    {
      pConfPartyNode->AddChildNode("AUTO_ADD", m_undefinedType, _BOOL);

      pNode = pConfPartyNode->AddChildNode("CHANNELS");
      for (int i = 1; i < GetNetChannelNumber()+1; i++)
      {
        if (i < 7)
        {
          sprintf(str, "CH_%d", i);
          pNode->AddChildNode(str, IsChannelConnected(i), _BOOL);
        }
        else
        {
          if (i == 7)
            pNode = pNode->AddChildNode("CHANNEL_LIST_EX");

          pNode->AddChildNode("CHANNEL_EX", IsChannelConnected(i), _BOOL);
        }
      }

      pNode = pConfPartyNode->AddChildNode("ACTUAL_MCU_PHONES");
      for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
      {
        if (m_pActualMCUPhoneNumbers[i])
        {
          if (i < 6)
          {
            sprintf(str, "PHONE%d", i+1);
            pNode->AddChildNode(str, m_pActualMCUPhoneNumbers[i]->phone_number);
          }
          else
          {
            if (i == 6)
              pNode = pNode->AddChildNode("PHONE_LIST_EX");

            pNode->AddChildNode("PHONE", m_pActualMCUPhoneNumbers[i]->phone_number);
          }
        }
      }

      pNode = pConfPartyNode->AddChildNode("ACTUAL_PARTY_PHONES");
      for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
      {
        if (m_pActualPartyPhoneNumbers[i])
        {
          if (i < 6)
          {
            sprintf(str, "PHONE%d", i+1);
            pNode->AddChildNode(str, m_pActualPartyPhoneNumbers[i]->phone_number);
          }
          else
          {
            if (i == 6)
              pNode = pNode->AddChildNode("PHONE_LIST_EX");

            pNode->AddChildNode("PHONE", m_pActualPartyPhoneNumbers[i]->phone_number);
          }
        }
      }

      pConfPartyNode->AddChildNode("AUDIO_MEMBER", IsAudio_Member(), _BOOL);
      pConfPartyNode->AddChildNode("VIDEO_MEMBER", IsVideo_Member(), _BOOL);
      pConfPartyNode->AddChildNode("LEADER", m_isLeader, _BOOL);
      pConfPartyNode->AddChildNode("WAIT_FOR_ASSISTANCE", m_wait_for_oper_assistance, WAIT_FOR_ASSISTANCE_ENUM);
      pConfPartyNode->AddChildNode("SRC_TYPE_IS_EQ", m_bSrcTypeIsEQ, _BOOL);


      if (m_pVideoLayout != NULL)
      {
        pParentNode = pConfPartyNode->AddChildNode("FORCE");
        m_pVideoLayout->SerializeXml(pParentNode);
      }

      if (GetH323GatekeeperStatus() != NULL && IsIpNetInterfaceType())
        GetH323GatekeeperStatus()->SerializeXml(pConfPartyNode);

      if (GetSipBWStatus() != NULL && IsIpNetInterfaceType())
        GetSipBWStatus()->SerializeXml(pConfPartyNode);

      pConfPartyNode->AddChildNode("TX_BAUD_RATE", m_trmBaudRate);
      pConfPartyNode->AddChildNode("RX_BAUD_RATE", m_rcvBaudRate);
    }
  }

  if (party_data_amount == FULL_DATA)
  {
    if (Opcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
        || Opcode == PARTY_FAST_PLUS_SLOW_INFO
        || Opcode == PARTY_NEW_INFO
        || Opcode == PARTY_COMPLETE_INFO)
    {
      pConfPartyNode->AddChildNode("NOISY", m_isNoiseDetected, _BOOL);
    }

    if (Opcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
        || Opcode == PARTY_FAST_PLUS_SLOW_INFO
        || Opcode == PARTY_NEW_INFO
        || Opcode == PARTY_COMPLETE_INFO)
      pConfPartyNode->AddChildNode("CONTENT_MEMBER", IsContent_Member(), _BOOL);

    pConfPartyNode->AddChildNode("CONTENT_PROVIDER", m_EPCContentProvider, _BOOL);

    if (Opcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
        || Opcode == PARTY_FAST_PLUS_SLOW_INFO
        || Opcode == PARTY_NEW_INFO
        || Opcode == PARTY_COMPLETE_INFO)
    {
      pConfPartyNode->AddChildNode("VISUAL_NAME", m_visual_partyName);
    }

    if (Opcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
        || Opcode == PARTY_FAST_PLUS_SLOW_INFO
        || Opcode == PARTY_NEW_INFO
        || Opcode == PARTY_COMPLETE_INFO)
    {
      pParentNode = pConfPartyNode->AddChildNode("SECONDARY_CAUSE_PARAMS");
      m_secondary_cause_params.SerializeXml(pParentNode);
    }

    pConfPartyNode->AddChildNode("PARTY_CHANGE_TYPE", Opcode, PARTY_CHANGE_STATE_ENUM);
    pConfPartyNode->AddChildNode("IS_RV_SYNC_LOSS", Is_M_SyncLoss(), _BOOL);                // not part from version 7
    pConfPartyNode->AddChildNode("IS_LV_SYNC_LOSS", Is_L_Video_SyncLoss(), _BOOL);          // not part from version 7

    if (!m_voice && m_pPartyConfVideoLayout)                                                // not part from version 7
    {
      pNode = pConfPartyNode->AddChildNode("PARTY_CONF_FORCE");
      m_pPartyConfVideoLayout->SerializeXml(pNode);                                         // not part from version 7
    }

    pConfPartyNode->AddChildNode("IS_CURRENTLY_ENCRYPTED", m_isPartyCurrentlyEncrypted, _BOOL);
    pConfPartyNode->AddChildNode("R_LPR_ACTIVATION", Is_Lpr_Rmt_SyncLoss(), _BOOL);         // not part from version 7
    pConfPartyNode->AddChildNode("L_LPR_ACTIVATION", Is_Lpr_Local_SyncLoss(), _BOOL);       // not part from version 7

    // only serialize xml to operator - that EMA will know to remove "to home conf" option
    BYTE is_valid_home_conf = 0;
    if (m_pMoveInfo && m_pMoveInfo->IsValidHomeConf())
    {
      is_valid_home_conf = 1;
    }

    pConfPartyNode->AddChildNode("IS_VALID_HOME_CONF", is_valid_home_conf, _BOOL);

    pConfPartyNode->AddChildNode("IS_EXCLUSIVE_CONTENT", m_isExclusiveContent, _BOOL);      // Restricted content
    pConfPartyNode->AddChildNode("REQUEST_TO_SPEAK", m_bRequestToSpeak, _BOOL);
    pConfPartyNode->AddChildNode("IS_EVENT_MODE_INTRA_SUPPRESSED", m_isEventModeIntraSuppressed, _BOOL);
    pConfPartyNode->AddChildNode("EVENT_MODE_LEVEL", m_eventModeLevel);
    if (Opcode == PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
        || Opcode == PARTY_FAST_PLUS_SLOW_INFO
        || Opcode == PARTY_NEW_INFO
        || Opcode == PARTY_COMPLETE_INFO)
    {
      pConfPartyNode->AddChildNode("LPR_HEADERS_ACTIVATION", m_lpr_heders_activated, _BOOL);
    }

    pConfPartyNode->AddChildNode("TIP_MODE", m_partyTypeTIP, TIP_PARTY_TYPE_ENUM);
    if (Opcode==PARTY_FAST_PLUS_SLOW_INFO || Opcode==PARTY_NEW_INFO || Opcode==PARTY_FAST_INFO || Opcode==PARTY_COMPLETE_INFO)
	{
	// RelayMedia Info
		bool bFull = false;
		if (party_data_amount==FULL_DATA)
			bFull = true;
		m_mediaList.SerializeXmlApi( pConfPartyNode, bFull );
	}
  }
}

//--------------------------------------------------------------------------
void CConfParty::R_videoSyncLostCounterIncrease()
{
  m_R_videoSyncLostCounter++;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetR_videoSyncLostCounter(const WORD num)
{
  m_R_videoSyncLostCounter = num;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetVisualPartyName(const char* visualname)
{
  if (visualname && strcmp(visualname, ""))
  {
    strncpy(m_visual_partyName, visualname, sizeof(m_visual_partyName) - 1);
    m_visual_partyName[sizeof(m_visual_partyName) - 1] = '\0';
    SLOWCHANGE;

    m_pSubject->Notify(UPDATEVISUALNAME, 0); // inform pcm
  }

  else
  {
    PTRACE(eLevelInfoNormal, "CConfParty::SetVisualPartyName NULL pointer\'");
    DBGPASSERT(1);
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetRemoteName(const char* remoteName)
{
  if (strcmp(remoteName, ""))
  {
    strncpy(m_remoteName, remoteName, sizeof(m_remoteName) - 1);
    m_remoteName[sizeof(m_remoteName) - 1] = '\0';
    SLOWCHANGE;
  }

  else
  {
    PTRACE(eLevelInfoNormal, "CConfParty::SetRemoteName NULL pointer\'");
    DBGPASSERT(1);
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetActualMCUPhoneNumber(WORD ind, const char* MCUPhoneNumber)
{
  if (ind >= MAX_CHANNEL_NUMBER)
    return;

  SLOWCHANGE;

  PDELETE(m_pActualMCUPhoneNumbers[ind]);
  m_pActualMCUPhoneNumbers[ind] = new Phone;

  strncpy(m_pActualMCUPhoneNumbers[ind]->phone_number, MCUPhoneNumber, sizeof(m_pActualMCUPhoneNumbers[ind]->phone_number) - 1);
  m_pActualMCUPhoneNumbers[ind]->phone_number[sizeof(m_pActualMCUPhoneNumbers[ind]->phone_number) - 1] = '\0';
}

//--------------------------------------------------------------------------
Phone* CConfParty::GetActualMCUPhoneNumber(WORD ind)
{
  if (ind >= MAX_CHANNEL_NUMBER)
    return NULL;

  return m_pActualMCUPhoneNumbers[ind];
}

//--------------------------------------------------------------------------
int CConfParty::DeleteActualMCUPhoneNumber(WORD ind)
{
  if (ind >= MAX_CHANNEL_NUMBER)
    return STATUS_ILLEGAL;

  SLOWCHANGE;

  if (m_pActualMCUPhoneNumbers[ind] == NULL)
    return STATUS_PHONE_NUMBER_NOT_EXISTS;

  PDELETE(m_pActualMCUPhoneNumbers[ind]);

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CConfParty::FindActualMCUPhoneNumber(const char* phoneNumber)
{
  for (int i = 0; i < (int)MAX_CHANNEL_NUMBER; i++)
  {
    if (m_pActualMCUPhoneNumbers[i] != NULL)
    {
      if (!strcmp(m_pActualMCUPhoneNumbers[i]->phone_number, phoneNumber))
        return i;
    }
  }

  return NOT_FIND;
}

//--------------------------------------------------------------------------
void CConfParty::SetActualPartyPhoneNumber(WORD ind, const char* PartyPhoneNumber)
{
  if (ind >= MAX_CHANNEL_NUMBER)
    return;

  SLOWCHANGE;

  PDELETE(m_pActualPartyPhoneNumbers[ind]);
  m_pActualPartyPhoneNumbers[ind] = new Phone;

  strncpy(m_pActualPartyPhoneNumbers[ind]->phone_number, PartyPhoneNumber, sizeof(m_pActualPartyPhoneNumbers[ind]->phone_number) - 1);
  m_pActualPartyPhoneNumbers[ind]->phone_number[sizeof(m_pActualPartyPhoneNumbers[ind]->phone_number) - 1] = '\0';
}

//--------------------------------------------------------------------------
void CConfParty::SetBackupH323PartyAlias(const char* name)
{
  strncpy(m_h323BackupPartyAlias, name, sizeof(m_h323BackupPartyAlias) - 1);
  m_h323BackupPartyAlias[sizeof(m_h323BackupPartyAlias) - 1] = '\0';
}

//--------------------------------------------------------------------------
Phone* CConfParty::GetActualPartyPhoneNumber(WORD ind)
{
  if (ind >= MAX_CHANNEL_NUMBER)
    return NULL;

  return m_pActualPartyPhoneNumbers[ind];
}

//--------------------------------------------------------------------------
int CConfParty::DeleteActualPartyPhoneNumber(WORD ind)
{
  if (ind >= MAX_CHANNEL_NUMBER)
    return STATUS_ILLEGAL;

  SLOWCHANGE;

  if (m_pActualPartyPhoneNumbers[ind] == NULL)
    return STATUS_PHONE_NUMBER_NOT_EXISTS;

  PDELETE(m_pActualPartyPhoneNumbers[ind]);

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CConfParty::FindActualPartyPhoneNumber(const char* phoneNumber)
{
  for (int i = 0; i < (int)MAX_CHANNEL_NUMBER; i++)
  {
    if (m_pActualPartyPhoneNumbers[i] != NULL)
    {
      if (!strcmp(m_pActualPartyPhoneNumbers[i]->phone_number, phoneNumber))
        return i;
    }
  }

  return NOT_FIND;
}

//--------------------------------------------------------------------------
void CConfParty::SetActualMCUandPartyPhoneNumbers(BYTE channel, const char* MCUPhoneNumber, const char* partyPhoneNumber)
{
  WORD ind = channel;

  SetActualMCUPhoneNumber(ind, MCUPhoneNumber);
  SetActualPartyPhoneNumber(ind, partyPhoneNumber);
}

//--------------------------------------------------------------------------
void CConfParty::SetOperatorName(const char* name)
{
  if (name)
  {
    strncpy(m_operatorName, name, sizeof(m_operatorName) - 1);
    m_operatorName[sizeof(m_operatorName) - 1] = '\0';
    SLOWCHANGE;
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetBondingTmpNumber(const char* phone)
{
  PTRACE2(eLevelInfoNormal, "CConfParty::SetBondingTmpNumber - Phone:", phone);
  strncpy(m_bondingTmpNumber.phone_number, phone, sizeof(m_bondingTmpNumber.phone_number) - 1);
  m_bondingTmpNumber.phone_number[sizeof(m_bondingTmpNumber.phone_number) - 1] = '\0';
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIpCapabilities(const CH221Str& other)
{
  POBJDELETE(m_pIpCapabilities);
  m_pIpCapabilities = new CH323StrCap(other);
}

//--------------------------------------------------------------------------
void CConfParty::SetIpLocalCommMode(const CH221Str& other)
{
  POBJDELETE(m_pIpLocalCommMode);
  m_pIpLocalCommMode = new CH323StrCap(other);
}

//--------------------------------------------------------------------------
void CConfParty::SetIpRemoteCommMode(const CH221Str& other)
{
  POBJDELETE(m_pIpRemoteCommMode);
  m_pIpRemoteCommMode = new CH323StrCap(other);
}

//--------------------------------------------------------------------------
void CConfParty::SetIpVideoBch(BYTE ipVideoBch)
{
  m_ipVideoBch = ipVideoBch;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIpProtocolSync(BYTE protocolSync)
{
  m_ipProtocolSync = protocolSync;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIpChannelDetails(CPrtMontrBaseParams* pPrtMonitrParams, BYTE isUpdateChannelMonitor)
{
  if (!pPrtMonitrParams)
    return;

  EIpChannelType     channelType      = (EIpChannelType)pPrtMonitrParams->GetChannelType();
  BYTE               connectionStatus = (BYTE)pPrtMonitrParams->GetConnectionStatus();
  DWORD              acualRate        = pPrtMonitrParams->GetBitRate();

  mcTransportAddress partyAddr;
  memset(&partyAddr, 0, sizeof(mcTransportAddress));
  memcpy(&partyAddr, pPrtMonitrParams->GetPartyAddr(), sizeof(mcTransportAddress));

  WORD               partyPort = pPrtMonitrParams->GetPartyPort();

  mcTransportAddress mcuAddr;
  memset(&mcuAddr, 0, sizeof(mcTransportAddress));
  memcpy(&mcuAddr, pPrtMonitrParams->GetMcuAddr(), sizeof(mcTransportAddress));

  WORD               mcuPort           = pPrtMonitrParams->GetMcuPort();
  WORD               frameRate         = pPrtMonitrParams->GetFrameRate();
  int                resolution        = pPrtMonitrParams->GetResolution();
  BYTE               IsIce             = pPrtMonitrParams->GetIsIce();
  EIceConnectionType IceConnectionType = pPrtMonitrParams->GetIceConnectionType();

  mcTransportAddress IcePartyAddr;
  memset(&IcePartyAddr, 0, sizeof(mcTransportAddress));

  mcTransportAddress IceMcuAddr;
  memset(&IceMcuAddr, 0, sizeof(mcTransportAddress));

  if (pPrtMonitrParams->GetIcePartyAddr() && pPrtMonitrParams->GetIceMcuAddr())
  {
    memcpy(&IcePartyAddr, pPrtMonitrParams->GetIcePartyAddr(), sizeof(mcTransportAddress));
    memcpy(&IceMcuAddr, pPrtMonitrParams->GetIceMcuAddr(), sizeof(mcTransportAddress));
  }

  if (channelType < IP_CHANNEL_TYPES_NUMBER)
  {
    if (m_pIpChannelMonitor[channelType] && isUpdateChannelMonitor)
      m_pIpChannelMonitor[channelType]->CopyClass(*pPrtMonitrParams);

    if (m_pIpChannelDetails[channelType])
    {
      SetIpChannelDetails(channelType, connectionStatus, acualRate, &partyAddr, &mcuAddr, IsIce, &IcePartyAddr, &IceMcuAddr, IceConnectionType, 0xFFFFFFFF, 0xFFFFFFFF, frameRate, resolution);
    }
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetIpChannelDetails(EIpChannelType channelType)
{
  if (channelType < IP_CHANNEL_TYPES_NUMBER)
  {
    CIpChannelDetails pH323channelDetails;
    pH323channelDetails.SetChannelType(channelType);

    POBJDELETE(m_pIpChannelDetails[channelType]);
    m_pIpChannelDetails[channelType] = new CIpChannelDetails(pH323channelDetails);

    POBJDELETE(m_pIpChannelMonitor[channelType]);
    m_pIpChannelMonitor[channelType] = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)channelType);
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetIpChannelDetails(EIpChannelType channelType, BYTE connectionStatus,
                                     DWORD actualRate, mcTransportAddress* partyAddr, mcTransportAddress* mcuAddr, BYTE IsIce,
                                     mcTransportAddress* IcePartyAddr, mcTransportAddress* IceMcuAddr, EIceConnectionType IceConnectionType,
                                     DWORD packetsCounterIn, DWORD packetsCounterUse, WORD frameRate, int videoResolution)
{
  if (channelType < IP_CHANNEL_TYPES_NUMBER)
  {
    if (m_pIpChannelDetails[channelType])
    {
      if (channelType != m_pIpChannelDetails[channelType]->GetChannelType())
        m_pIpChannelDetails[channelType]->SetChannelType(channelType);

      if (connectionStatus != 0xFF)
        m_pIpChannelDetails[channelType]->SetConnectionStatus(connectionStatus);

      if (actualRate != 0xFFFFFFFF)
        m_pIpChannelDetails[channelType]->SetActualRate(actualRate);

      if (::isApiTaNull(partyAddr) != TRUE || partyAddr->port != 0xFFFF)
        m_pIpChannelDetails[channelType]->SetPartyAddrPort(partyAddr);

      if (::isApiTaNull(mcuAddr) != TRUE || mcuAddr->port != 0xFFFF)
        m_pIpChannelDetails[channelType]->SetMcuAddrPort(mcuAddr);

      if (packetsCounterIn != 0xFFFFFFFF)
        m_pIpChannelDetails[channelType]->SetPacketsCounterIn(packetsCounterIn);

      if (packetsCounterUse != 0xFFFFFFFF)
        m_pIpChannelDetails[channelType]->SetPacketsCounterUse(packetsCounterUse);

      if (frameRate != 0xFFFF)
        m_pIpChannelDetails[channelType]->SetFrameRate(frameRate);

      if (videoResolution != 0xFFFF)
      {
        m_pIpChannelDetails[channelType]->SetVideoResolution(videoResolution);
      }

      m_pIpChannelDetails[channelType]->SetIsIce(IsIce);

      if (::isApiTaNull(IcePartyAddr) != TRUE || IcePartyAddr->port != 0xFFFF)
        m_pIpChannelDetails[channelType]->SetIcePartyAddrPort(IcePartyAddr);

      if (::isApiTaNull(IceMcuAddr) != TRUE || IceMcuAddr->port != 0xFFFF)
        m_pIpChannelDetails[channelType]->SetIceMcuAddrPort(IceMcuAddr);

      if (IceConnectionType)
        m_pIpChannelDetails[channelType]->SetIceConnectionType(IceConnectionType);
    }
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetH323GatekeeperStatus(const CH323GatekeeperStatus& other)
{
  SLOWCHANGE;

  POBJDELETE(m_pH323GatekeeperStatus);
  m_pH323GatekeeperStatus = new CH323GatekeeperStatus(other);
}

//--------------------------------------------------------------------------
void CConfParty::SetH323GatekeeperStatus(BYTE gkState, DWORD reqBandwidth, DWORD allocBandwidth, WORD requestIntoInterval, BYTE gkRouted)
{
  SLOWCHANGE;

  if (m_pH323GatekeeperStatus)
  {
    if (gkState != 0xFF)
      m_pH323GatekeeperStatus->SetGkState(gkState);

    if (reqBandwidth != 0xFFFFFFFF)
      m_pH323GatekeeperStatus->SetReqBandwidth(reqBandwidth);

    if (allocBandwidth != 0xFFFFFFFF)
      m_pH323GatekeeperStatus->SetAllocBandwidth(allocBandwidth);

    if (requestIntoInterval != 0xFFFF)
      m_pH323GatekeeperStatus->SetRequestIntoInterval(requestIntoInterval);

    if (gkRouted != 0xFF)
      m_pH323GatekeeperStatus->SetGkRouted(gkRouted);
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetH323GatekeeperStatus()
{
  CH323GatekeeperStatus pH323GatekeeperStatus;

  POBJDELETE(m_pH323GatekeeperStatus);
  m_pH323GatekeeperStatus = new CH323GatekeeperStatus(pH323GatekeeperStatus);
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetSipBWStatus(const CSIPBandwidthAllocationStatus& other)
{
  SLOWCHANGE;

  POBJDELETE(m_pSipBWStatus);
  m_pSipBWStatus = new CSIPBandwidthAllocationStatus(other);
}

//--------------------------------------------------------------------------
void CConfParty::SetSipBWStatus(DWORD reqBandwidth, DWORD allocBandwidth)
{
  SLOWCHANGE;

  if (m_pSipBWStatus)
  {
    if (reqBandwidth != 0xFFFFFFFF)
      m_pSipBWStatus->SetReqBandwidth(reqBandwidth);

    if (allocBandwidth != 0xFFFFFFFF)
      m_pSipBWStatus->SetAllocBandwidth(allocBandwidth);
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetSipBWStatus()
{
  CSIPBandwidthAllocationStatus pSipBWStatus;

  POBJDELETE(m_pSipBWStatus);
  m_pSipBWStatus = new CSIPBandwidthAllocationStatus(pSipBWStatus);

  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::IpVideoBchCounterIncrease()
{
  m_ipVideoBchCounter++;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIpVideoBchCounter(const WORD num)
{
  m_ipVideoBchCounter = num;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::IpProtocolSyncCounterIncrease()
{
  m_ipProtocolSyncCounter++;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIpProtocolSyncCounter(const WORD num)
{
  m_ipProtocolSyncCounter = num;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetAudioBlocked(WORD bl)
{
  if (bl == FALSE)
    m_blockMedia &= 0xFFFFFFFE;
  else
    m_blockMedia |= 0x00000001;

  m_pSubject->Notify(MUTE_STATE, 0); // inform pcm
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetVideoBlocked(WORD bl)
{
  if (bl == FALSE)
    m_blockMedia &= 0xFFFFFFFD;
  else
    m_blockMedia |= 0x00000002;

  m_pSubject->Notify(MUTE_STATE, 0); // inform pcm
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIpVideoIntraSync(BYTE videoIntraSync)
{
  m_ipVideoIntraSync = videoIntraSync;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetEPCContentProvider(const BYTE contentProvider)
{
  m_EPCContentProvider = contentProvider;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIsPartyCurrentlyEncrypted(const BYTE isPartyCurrentlyEncrypted)
{
  m_isPartyCurrentlyEncrypted = isPartyCurrentlyEncrypted;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIsLeader(const WORD leader)
{
  m_isLeader = leader;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetIsNoiseDetected(BYTE wIsNoiseDetected)
{
  m_isNoiseDetected = wIsNoiseDetected;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetWaitForOperAssistance(BYTE wait_for_oper_assistance)
{
  if (wait_for_oper_assistance == WAIT_FOR_OPER_NONE)
  {
    PTRACE2(eLevelInfoNormal, "CConfParty::SetWaitForOperAssistance [operator_assistance_trace] WAIT_FOR_OPER_NONE : party_name = ", GetName());
  }
  else if (wait_for_oper_assistance == WAIT_FOR_OPER_ON_REQ_PRIVATE)
  {
    PTRACE2(eLevelInfoNormal, "CConfParty::SetWaitForOperAssistance [operator_assistance_trace] WAIT_FOR_OPER_ON_REQ_PRIVATE : party_name = ", GetName());
  }
  else if (wait_for_oper_assistance == WAIT_FOR_OPER_ON_REQ_PUBLIC)
  {
    PTRACE2(eLevelInfoNormal, "CConfParty::SetWaitForOperAssistance [operator_assistance_trace] WAIT_FOR_OPER_ON_REQ_PUBLIC : party_name = ", GetName());
  }

  m_wait_for_oper_assistance = wait_for_oper_assistance;

  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetPartyConfVideoLayout()
{
  POBJDELETE(m_pPartyConfVideoLayout);
  m_pPartyConfVideoLayout = new CVideoLayout(*m_pVideoLayout);
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
DWORD CConfParty::AttachObserver(COsQueue* pObserver, WORD event, WORD type, DWORD observerInfo1)
{
  m_pSubject->AttachObserver(pObserver, event, type, observerInfo1);
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CConfParty::DetachObserver(COsQueue* pObserver)
{
  m_pSubject->DetachObserver(pObserver);
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CConfParty::DetachObserver(COsQueue* pObserver, WORD event, WORD type, DWORD observerInfo1)
{
  m_pSubject->DetachObserver(pObserver, event, type, observerInfo1);
  return STATUS_OK;
}

//--------------------------------------------------------------------------
BOOL CConfParty::IsActiveVideoParty() const
{
  if (m_voice)
    return FALSE;

  switch (m_party_state)
  {
    case PARTY_SECONDARY:
    case PARTY_DISCONNECTED:
    case PARTY_WAITING_FOR_DIAL_IN:
    case PARTY_STAND_BY:
      return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------------------
void CConfParty::SetReferredBy(const char* pReferredBy)
{
  if (pReferredBy)
  {
    strncpy(m_referredByStr, pReferredBy, sizeof(m_referredByStr) - 1);
    m_referredByStr[sizeof(m_referredByStr) - 1] = '\0';
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetBackupIpAddress(mcTransportAddress ipAddress)
{
  // IpV6
  memset(&m_backupIpAddress, 0, sizeof(mcTransportAddress));
  memcpy(&m_backupIpAddress, &ipAddress, sizeof(mcTransportAddress));
}

//--------------------------------------------------------------------------
void CConfParty::UpdateHotBackupFields()
{
  m_HotBackupPartyStateInMaster = m_party_state;
  m_party_state                 = PARTY_DISCONNECTED;
}

//--------------------------------------------------------------------------
void CConfParty::RestoreHotBackupFields()
{
  m_bIsFirstConnectionAfterHotBackupRestore = TRUE;
  m_party_state                             = m_HotBackupPartyStateInMaster;

  // change params
  if (m_connectionType == DIAL_IN)
  {
	  TRACEINTO << "disconnect cause: " << m_disconnect_cause << ", party state: " << m_party_state;
	  if (m_disconnect_cause == SIP_INTERNAL_MCU_PROBLEM //the disconnection is the result of master card crash
            ||DISCONNECTED_BY_VIDEO_RECOVERY == m_disconnect_cause)//added by Richer for BRIDGE-13006,2014.04.29
	  {
		  m_party_state = PARTY_CONNECTED;
		  m_disconnect_cause = NO_DISCONNECTION_CAUSE;
	  }


	  if ((m_party_state != PARTY_WAITING_FOR_DIAL_IN) && (m_party_state != PARTY_DISCONNECTING) && (m_party_state != PARTY_DISCONNECTED))
	  {
		  m_connectionType = DIAL_OUT;
		  const char*            serviceName           = GetServiceProviderName();
		  CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
		  CConfIpParameters*     pServiceParams        = pIpServiceListManager->GetRelevantService(serviceName, GetNetInterfaceType());
		  if (pServiceParams == NULL)
		  {
			PTRACE2INT(eLevelInfoNormal, "CConfParty::RestoreHotBackupFields - IP Service does not exist!!!", m_partyId);
		  }
		  else
		  {
			BYTE interfaceType = GetNetInterfaceType();
			mcTransportAddress IpAddr;
			memset(&IpAddr, 0, sizeof(mcTransportAddress));

			if (interfaceType == H323_INTERFACE_TYPE) // H323: In case of GK - connect with alias and not IP
			{
			  BOOL bIsGkExternal = pServiceParams->isGKExternal();
			  if (bIsGkExternal)
			  {
				if (strlen(GetH323PartyAlias()) != 0)
				  SetIpAddress(IpAddr);
			  }
			}

			else if (interfaceType == SIP_INTERFACE_TYPE) // SIP: In case of Proxy - connect with URI and not IP
			{
			  BYTE bIsProxy = (pServiceParams->GetSipProxyStatus() != eServerStatusOff);
			  if (bIsProxy)
			  {
				if (strlen(GetSipPartyAddress()) != 0)
				  SetIpAddress(IpAddr);
			  }
			}
		  }
		}
	  }

  if (m_party_state == PARTY_DISCONNECTING)
    m_party_state = PARTY_DISCONNECTED;
}

//--------------------------------------------------------------------------
BOOL CConfParty::CanConnectParty()
{
  if (m_connectionType == DIAL_IN)
    return FALSE;

  BOOL bCanConnect = TRUE;
  if (m_bIsFirstConnectionAfterHotBackupRestore)
  {
    if (((m_party_state == PARTY_DISCONNECTED) && (DISCONNECTED_BY_VIDEO_RECOVERY != m_disconnect_cause))//added by Richer for BRIDGE-13006,2014.04.29
        || (m_party_state == PARTY_STAND_BY) || (m_party_state == PARTY_IDLE))
    {
      bCanConnect                               = FALSE;
    }
  }

  return bCanConnect;
}

//--------------------------------------------------------------------------
void CConfParty::SetExclusiveContentOwner(BOOL i_isExclusiveContent)  // Restricted content
{
  m_isExclusiveContent = i_isExclusiveContent;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetRequestToSpeak(BOOL isRequestToSpeak)
{
  m_bRequestToSpeak = isRequestToSpeak;
  FASTCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetEventModeIntraSuppressed(BOOL isEventModeIntraSuppresed)
{
  m_isEventModeIntraSuppressed = isEventModeIntraSuppresed;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::SetEventModeLevel(WORD eventModeLevel)
{
  if (m_eventModeLevel != eventModeLevel)
  {
    m_eventModeLevel = eventModeLevel;
    SLOWCHANGE;
  }
}

//--------------------------------------------------------------------------
void CConfParty::SetIsHighProfile(BYTE isHighProfile)
{
  m_isHighProfile = isHighProfile;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CConfParty::AllocateIpChannels()
{
  for (int i = 0; i < IP_CHANNEL_TYPES_NUMBER; i++)
  {
    m_pIpChannelDetails[i] = new CIpChannelDetails;
    m_pIpChannelDetails[i]->SetChannelType((EIpChannelType)i);
    m_pIpChannelMonitor[i] = CPrtMontrBaseParams::AllocNewClass((EIpChannelType)i);
  }
}

//--------------------------------------------------------------------------
BOOL CConfParty::IsAllowReconnect() const
{
  return m_party_state == PARTY_DISCONNECTED || m_party_state == PARTY_STAND_BY;
}

//--------------------------------------------------------------------------
BOOL CConfParty::IsAllowDisconnect() const
{
  switch (m_party_state)
  {

    case PARTY_CONNECTED:
    case PARTY_CONNECTING:
    case PARTY_CONNECTED_PARTIALY:
    case PARTY_SECONDARY:
    case PARTY_CONNECTED_WITH_PROBLEM:
    case PARTY_REDIALING:
      return TRUE;
  }
  return FALSE;
}
EMoveType	CConfParty::GetMoveType()
{
	return m_MoveType;
}
void	   CConfParty::SetMoveType(EMoveType  enMoveType)
{
	if(eMoveDummy > enMoveType || eMoveBackIntoIvr< enMoveType)
	{
		PTRACE2INT(eLevelInfoNormal,"CConfParty::SetMoveType out of range! - ", enMoveType);
		return;
	}
	m_MoveType = enMoveType;
	return;
}

// Media ------------------------------------------------------------
void CConfParty::SetMediaList(CMediaList& mediaList)
{
	FASTCHANGE;

	m_mediaList.ReplaceMedia(mediaList);
	PTRACE2(eLevelInfoNormal, "CConfParty::SetMediaList - ", m_mediaList.ToString().c_str());
	m_pSubject->Notify(MEDIA, 0);
}

void CConfParty::RemoveMedia(std::list<unsigned int> listMediaID, bool bUrgent)
{
	m_mediaList.SetMediaDeleted(listMediaID, bUrgent);
	TRACEINTO << "CConfParty::RemoveMedia - bUrgent = " << (bUrgent ? "true" : "false") << "\n" << m_mediaList.ToString().c_str();
	m_pSubject->Notify(MEDIA, 0);
}

const CMediaList* CConfParty::GetMediaList() const
{
	return &m_mediaList;
}

void CConfParty::SetMediaListUpdated()
{
	m_mediaList.RemoveDeletedMedia();
}

////////////////////////////////////////////////////////////////////////////
bool CConfParty::IsRelayInfoExists()
{
	return (m_mediaList.GetListMediaSize() > 0);
}

void CConfParty::SLOW_FAST_CHANGE_Terminal( int slowFastAction )
{
	if (0 == slowFastAction)
		{SLOWCHANGE;}
	else
		{FASTCHANGE;}
}

//_mccf_--------------------------------------------------------------------------
void CConfParty::SetToTag(const char* tag)
{
  strncpy(m_strToTag, tag, sizeof(m_strToTag) - 1);
  m_strToTag[sizeof(m_strToTag) - 1] = '\0';
}

//_mccf_--------------------------------------------------------------------------
void CConfParty::SetFromTag(const char* tag)
{
  strncpy(m_strFromTag, tag, sizeof(m_strFromTag) - 1);
  m_strFromTag[sizeof(m_strFromTag) - 1] = '\0';
}
//_mccf_--------------------------------------------------------------------------
std::string CConfParty::GetPartyTag() const
{
	std::string tag(m_strFromTag);
	tag += ':';
	tag += m_strToTag;

	return tag;
}
///////////////////////////////////
void CConfParty::SetSipUsrName(const char* usrName)
{
	if (strcmp(usrName, ""))
	{
		strncpy(m_sipUsrName, usrName, sizeof(m_sipUsrName) - 1);
		m_sipUsrName[sizeof(m_sipUsrName) - 1] = '\0';
		SLOWCHANGE;
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CConfParty::SetSipUsrName NULL pointer\'");
		DBGPASSERT(1);
	}
}
///////////////////////////////////
void CConfParty::SetCorrelationId(std::string correlationId)
{
	m_correlationId = correlationId;
}
