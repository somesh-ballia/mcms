#include "CommResApi.h"

#include "NStream.h"
#include "psosxml.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "VideoCellLayout.h"
#include "InternalProcessStatuses.h"
#include "ServicePrefixStr.h"
#include "CdrApiClasses.h"
#include "OperatorConfInfo.h"
#include "AutoScanOrder.h"
#include "TraceStream.h"
#include "DefinesIpService.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "VideoStructs.h"
#include "SysConfigKeys.h"
#include "ConfigManagerApi.h"
#include "EnumsToStrings.h" // TELEPRESENCE_LAYOUTS_DEBUG

#include <stdio.h>
#include <sstream>

////////////////////////////////////////////////////////////////////////////
//                        CCommResApi
////////////////////////////////////////////////////////////////////////////
CCommResApi::CCommResApi()
	: m_cascadedLinksNumber(0)
	, m_cascadedPartiesCounter(0)
	, m_FontType(ftDefault)
{
	memset(m_pParty, 0, sizeof(m_pParty));
	memset(m_pServicePhoneStr, 0, sizeof(m_pServicePhoneStr));
	memset(m_pServicePrefixStr, 0, sizeof(m_pServicePrefixStr));
	memset(m_pRsrvVideoLayout, 0, sizeof(m_pRsrvVideoLayout));

	m_numParties                         = 0;
	m_H243confName[0]                    = '\0';
	m_correlationId[0]                 = '\0';
	m_confDisplayName[0]                 = '\0';
	m_confId                             = 0xFFFFFFFF;
	m_stand_by                           = NO;
	m_automaticTermination               = YES;
	m_confTransferRate                   = Xfer_64;             // Xfer_384 in EMA
	m_audioRate                          = U_Law_OF;            // AUTO in EMA
	m_video                              = YES;
	m_videoPictureFormat                 = V_Qcif;              // AUTO in EMA
	m_CIFframeRate                       = V_4_29_97;           // AUTO in EMA
	m_QCIFframeRate                      = V_4_29_97;
	m_audioTone                          = 0;
	m_alertToneTiming                    = 5;
	m_talkHoldTime                       = 150;                 // 150 in EMA ;//0
	m_audioMixDepth                      = 5;
	m_videoSession                       = CONTINUOUS_PRESENCE; // CONTINUOUS_PRESENCE in EMA //VIDEO_SWITCH
	m_contPresScreenNumber               = ONE_ONE;             // ONE_TWO
	m_pAvMsgStruct                       = new CAvMsgStruct;
	m_videoProtocol                      = AUTO;
	m_meetMePerConf                      = YES;
	m_numServicePhoneStr                 = 0;
	m_numServicePrefixStr                = 0;
	m_H243_password[0]                   = '\0';
	m_numRsrvVideoSource                 = 0;
	m_cascadeMode                        = CASCADE_MODE_NONE;
	m_numUndefParties                    = 0;
	m_pLectureMode                       = new CLectureModeParams;
	m_repSchedulingId                    = 0;
	m_time_beforeFirstJoin               = 5;
	m_time_afterLastQuit                 = 1;
	m_LastQuitType                       = eTerminateAfterLastLeaves;
	m_confLockFlag                       = NO;
	m_confSecureFlag                     = NO;
	m_confOnHoldFlag                     = NO;
	m_Mute_Incoming_Parties_Lecture_Mode = NO;
    m_muteAllPartiesAudioExceptLeader    = NO;
    m_muteAllPartiesVideoExceptLeader    = NO;
	m_FECC_Enabled                       = YES;
	m_max_parties                        = 0xFFFF;
	m_ipVideoRate                        = 768;
	m_isMeetingRoom                      = NO;
	m_isConfTemplate                     = NO;
	m_meetingRoomReoccurrenceNum         = 0xFF;
	m_meetingRoomState                   = MEETING_ROOM_PASSIVE_STATE;
	m_slowUpdateCounter                  = 0;
	m_fastUpdateCounter                  = 0;
	m_dwSummaryUpdateCounter             = 0;
	m_dwFullUpdateCounter                = 0;
	m_SummeryCreationUpdateCounter       = 0;
	m_current_numb_of_party              = 0;
	m_ind_service_phone                  = 0;
	m_ind_rsrv_vid_layout                = 0;
	m_isGateway                          = NO;
	m_isVideoInviteGateway               = NO;
	m_GWDialOutProtocols                 = 0;
	m_partyIdCounter                     = 0;
	m_unrsrvPartiesCounter               = 0;
	m_startConfRequiresLeaderOnOff       = NO;
	m_terminateConfAfterChairDropped     = NO;
	m_entry_password[0]                  = '\0';
	m_isSameLayout                       = NO;
	m_isAutoLayout                       = NO;
	m_confType                           = CONF_TYPE_STANDARD;
	m_media                              = AUDIO_VIDEO_MEDIA;
	m_network                            = NETWORK_H323;
	m_meetMePerEntryQ                    = NO;
	m_IsEntryQ                           = NO;
	m_IsAdHoc                            = NO;
	m_dwAdHocProfileId                   = 0xFFFFFFFF;
	m_baseProfileName[0]                 = '\0';
	m_dualVideoMode                      = eDualModeNone;
	m_EnterpriseMode                     = eGraphics;
	m_EnterpriseModeFixedRate            = ~0;  // the field is meaningfully initialized for m_EnterpriseMode == eFixedRate only
	m_PresentationProtocol               = ePresentationAuto;
	m_cascadeOptimizeResolution          = e_res_dummy;
    m_isHighProfileContent               = NO;
	m_isVideoPlusConf                    = YES; // not serialized
	m_IsConfOnPort                       = NO;  // YES, NO.
	m_pVisualEffectsInfo                 = new CVisualEffectsParams;
	m_pMessageOverlayInfo                = new CMessageOverlayInfo;
	m_pSiteNameInfo                      = new CSiteNameInfo;
	m_SubCPtype                          = eSubCPtypeUnknown;
	m_pConfContactInfo                   = new CConfContactInfo;
	m_pAutoScanOrder                     = new CAutoScanOrder;

	m_NumericConfId[0]                   = '\0';

	m_ServiceNameForMinParties[0]        = '\0';

	m_isTemplate                         = NO;

	m_dwSummaryUpdateCounter             = 0;
	m_dwFullUpdateCounter                = 0;
	m_bChanged                           = FALSE;
	m_encryption                         = NO;
	m_eEncryptionType                    = eEncryptNone;
	m_EnableRecording                    = NO;
	m_EnableRecordingIcon                = YES;
	m_StartRecPolicy                     = START_RECORDING_IMMEDIATELY;
	m_RecordingLinkControl               = eStopRecording;
	m_RecLinkName[0]                     = '\0';
	m_IsEnableRecNotify			=YES;
	m_IsCascadeEQ                        = 0;
	CStructTm tm(0, 0, 0, 0, 0, 0);
	m_duration                           = tm;

	m_isSIPFactory                       = NO;
	m_isAutoConnectSIPFactory            = NO;
	m_HD                                 = NO;

	m_isAdHocConf                        = NO;

	m_webReservUId                       = 0;
	m_webOwnerUId                        = 0;
	m_webDBId                            = 0;
	m_webReserved                        = 0;

	m_videoQuality                       = eVideoQualitySharpness; // Sharpness is the default video quality
	m_IsAudioOnlyRecording               = NO;
	m_isTelePresenceMode                 = NO;                     // eTelePresencePartyNone
	m_telePresenceLayoutMode             = eTelePresenceLayoutContinuousPresence;
	m_telePresenceModeConfiguration      = AUTO;
	m_manageTelepresenceLayoutInternaly  = NO;
	m_isCropping                         = YES;
	m_isLpr                              = NO;
	m_HDResolution                       = eHD720Res; // HD720 is the default HD Resolution for VSW confs
	m_H264VSWHighProfilePreference       = eBaseLineOnly;
	m_isVideoClarityEnabled              = NO;
	m_isSiteNamesEnabled                 = YES;

	m_internalConfStatus                 = STATUS_OK;
	m_minNumVideoParties                 = 0;
	m_minNumAudioParties                 = 0;
	m_EchoSuppression                    = YES;
	m_KeyboardSuppression                = NO;
	m_isAutoMuteNoisyParties             = YES;
	m_operatorConf                       = NO;
	m_pOperatorConfInfo                  = NULL;
	m_AutoScanInterval                   = 10;
	m_ShowContentAsVideo                 = YES;
	m_AutoRedial                         = NO;
	m_bIsExclusiveContentMode            = NO;
	m_isPermanent                        = FALSE;
	m_pCopConfigurationList              = new CCOPConfigurationList;
	m_appoitnmentID[0]                   = '\0';
	m_meetingOrganizer[0]                = '\0';
	m_isStreaming                        = NO;

	// Gathering
	m_bGatheringEnabled                  = NO;
	m_eLanguage                          = eEnglish;
	m_sIpNumberAccess[0]                 = '\0';
	m_sNumberAccess_1[0]                 = '\0';
	m_sNumberAccess_2[0]                 = '\0';
	m_sFreeText_1[0]                     = '\0';
	m_sFreeText_2[0]                     = '\0';
	m_sFreeText_3[0]                     = '\0';

	// This line must be the last one in the constructor
	m_dwConfFlags                        = CalcRsrvFlags();
	m_dwConfFlags2                       = CalcRsrvFlags2();

	m_confMaxResolution                  = eAuto_Res;
	m_ResSts                             = 0;     // Status Field
	m_isAudioClarity                     = FALSE;
	m_isAutoBrightness                   = FALSE;
	m_ivrProviderEQ                      = FALSE;
	m_externalIvrControl                 = FALSE; // AT&T
	for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
	{
		m_ServiceRegistrationContent[i].service_name[0] = '\0';
		m_ServiceRegistrationContent[i].sip_register    = FALSE;
		m_ServiceRegistrationContent[i].accept_call     = TRUE;
		m_ServiceRegistrationContent[i].status          = eSipRegistrationStatusTypeNotConfigured; // sipProxySts
	}

	m_SipRegistrationTotalSts            = eSipRegistrationTotalStatusTypeNotConfigured;         // sipProxySts
	m_TipCompatibility                   = eTipCompatibleNone;
	m_natKeepAlivePeriod                 = 0;                                                    // 30;
	m_speakerChangeThreshold             = eAutoTicks;
	m_confSpeakerChangeMode              = E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;
	m_confMediaType                      = eAvcOnly;                                             // we may need to set a different default to IBM Product type
	m_eOperationPointPreset				 = eOPP_cif;
    m_ContentMultiResolutionEnabled = NO;
    m_ContentXCodeH264Supported = YES;
    m_ContentXCodeH263Supported = YES;
    m_isCascadeOptimized = NO;
    m_isAsSipContent = NO;
    m_mrcMcuId = 1;
    m_FocusUriScheduling[0] = '\0';
    m_FocusUriCurrently[0] = '\0';
    m_avMcuCascadeVideoMode   = eMsSvcResourceOptimize;
    m_SrsPlaybackLayout  = eSrsAutoMode;

	//Indication for audio participants
	m_IconDisplayPosition =             				eLocationTopLeft;
	m_EnableAudioParticipantsIcon =           			YES;
	m_AudioParticipantsIconDisplayMode =             	eIconDisplayOnChange;
	m_AudioParticipantsIconDuration =            		10;
	m_EnableSelfNetworkQualityIcon =             		YES;
    m_overideProfileLayout = false;
}

//--------------------------------------------------------------------------
CCommResApi::CCommResApi(const CCommResApi& other)
	: CSerializeObject(other)
{
	memset(m_pParty, 0, sizeof(m_pParty));
	memset(m_pServicePhoneStr, 0, sizeof(m_pServicePhoneStr));
	memset(m_pServicePrefixStr, 0, sizeof(m_pServicePrefixStr));
	memset(m_pRsrvVideoLayout, 0, sizeof(m_pRsrvVideoLayout));

	m_numParties                         = 0;
	m_numRsrvVideoSource                 = 0;
	m_numServicePhoneStr                 = 0;
	m_numServicePrefixStr                = 0;

	m_pAvMsgStruct                       = new CAvMsgStruct;
	m_pLectureMode                       = new CLectureModeParams;
	m_pVisualEffectsInfo                 = new CVisualEffectsParams;
	m_pMessageOverlayInfo                = new CMessageOverlayInfo;
	m_pSiteNameInfo                      = new CSiteNameInfo;
	m_pConfContactInfo                   = new CConfContactInfo;
	m_pAutoScanOrder                     = new CAutoScanOrder;

	m_manageTelepresenceLayoutInternaly  = other.m_manageTelepresenceLayoutInternaly;
	m_BillingInfo                        = other.m_BillingInfo;
	m_confOnHoldFlag                     = other.m_confOnHoldFlag;
	m_Mute_Incoming_Parties_Lecture_Mode = other.m_Mute_Incoming_Parties_Lecture_Mode;
	m_muteAllPartiesAudioExceptLeader    = other.m_muteAllPartiesAudioExceptLeader;
	m_muteAllPartiesVideoExceptLeader    = other.m_muteAllPartiesVideoExceptLeader;
	m_FECC_Enabled                       = other.m_FECC_Enabled;
	m_isAutoLayout                       = other.m_isAutoLayout;

	// API_ENCRYPTION
	SetEncryptionParameters(other.m_encryption, other.m_eEncryptionType);

	m_operatorConf                       = other.m_operatorConf;

	if (other.m_pOperatorConfInfo != NULL)
		m_pOperatorConfInfo = new COperatorConfInfo(*other.m_pOperatorConfInfo);
	else
		m_pOperatorConfInfo = NULL;

	m_pCopConfigurationList              = new CCOPConfigurationList;

	m_isAudioClarity                     = other.m_isAudioClarity;
	m_isAutoBrightness                   = other.m_isAutoBrightness;

	for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
	{
		if (other.m_ServiceRegistrationContent[i].service_name != '\0')
		{
			strcpy_safe(m_ServiceRegistrationContent[i].service_name, other.m_ServiceRegistrationContent[i].service_name);
			m_ServiceRegistrationContent[i].sip_register = other.m_ServiceRegistrationContent[i].sip_register;
			m_ServiceRegistrationContent[i].accept_call  = other.m_ServiceRegistrationContent[i].accept_call;
			m_ServiceRegistrationContent[i].status       = other.m_ServiceRegistrationContent[i].status; // sipProxySts
		}
	}

	m_ResSts                             = other.m_ResSts;                                           // Status Field
	m_ivrProviderEQ                      = other.m_ivrProviderEQ;
	m_externalIvrControl                 = other.m_externalIvrControl;                               // AT&T
	m_SipRegistrationTotalSts            = other.m_SipRegistrationTotalSts;                          // sipProxySts
	m_TipCompatibility                   = other.m_TipCompatibility;
	m_natKeepAlivePeriod                 = other.m_natKeepAlivePeriod;
	m_speakerChangeThreshold             = other.m_speakerChangeThreshold;
	m_confSpeakerChangeMode              = other.m_confSpeakerChangeMode;
	m_confMediaType                      = other.m_confMediaType;
	m_eOperationPointPreset              = other.m_eOperationPointPreset;
	m_cascadeMode                        = other.m_cascadeMode;
	m_mrcMcuId                           = other.m_mrcMcuId;
	m_avMcuCascadeVideoMode  			 = other.m_avMcuCascadeVideoMode;
	m_overideProfileLayout               = other.m_overideProfileLayout;

	strcpy_safe(m_FocusUriScheduling, other.m_FocusUriScheduling);
	strcpy_safe(m_FocusUriCurrently, other.m_FocusUriCurrently);

	*this                                = other;
}

//--------------------------------------------------------------------------
CCommResApi::~CCommResApi()
{
	POBJDELETE(m_pAvMsgStruct);
	POBJDELETE(m_pLectureMode);
	POBJDELETE(m_pVisualEffectsInfo);
	POBJDELETE(m_pMessageOverlayInfo);
	POBJDELETE(m_pSiteNameInfo);
	POBJDELETE(m_pAutoScanOrder);

	int i;
	for (i = 0; i < MAX_PARTIES_IN_CONF; i++)
		POBJDELETE(m_pParty[i]);

	for (i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
		POBJDELETE(m_pRsrvVideoLayout[i]);

	for (i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
	{
		POBJDELETE(m_pServicePhoneStr[i]);
		POBJDELETE(m_pServicePrefixStr[i]);
	}

	POBJDELETE(m_pConfContactInfo);
	POBJDELETE(m_pOperatorConfInfo);
	POBJDELETE(m_pCopConfigurationList);
}

//--------------------------------------------------------------------------
CCommResApi& CCommResApi::operator =(const CCommResApi& other)
{
	if (&other == this)
		return *this;

	strcpy_safe(m_H243confName, other.m_H243confName);
	strcpy_safe(m_confDisplayName, other.m_confDisplayName);
	strcpy_safe(m_correlationId,other.m_correlationId);
	strcpy_safe(m_H243_password, other.m_H243_password);
	strcpy_safe(m_NumericConfId, other.m_NumericConfId);
	strcpy_safe(m_entry_password, other.m_entry_password);
	strcpy_safe(m_NumericConfId, other.m_NumericConfId);
	strcpy_safe(m_baseProfileName, other.m_baseProfileName);
	strcpy_safe(m_RecLinkName, other.m_RecLinkName);
	strcpy_safe(m_ServiceNameForMinParties, other.m_ServiceNameForMinParties);

	m_numParties              = other.m_numParties;
	m_confId                  = other.m_confId;
	m_startTime               = other.m_startTime;
	m_dtExchangeConfStartTime = other.m_dtExchangeConfStartTime;
	m_duration                = other.m_duration;
	m_stand_by                = other.m_stand_by;
	m_automaticTermination    = other.m_automaticTermination;
	m_confTransferRate        = other.m_confTransferRate;
	m_audioRate               = other.m_audioRate;
	m_video                   = other.m_video;
	m_videoSession            = other.m_videoSession;
	m_contPresScreenNumber    = other.m_contPresScreenNumber;
	int i;
	for (i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
	{
		POBJDELETE(m_pRsrvVideoLayout[i]);
		if (other.m_pRsrvVideoLayout[i])
			m_pRsrvVideoLayout[i] = new CVideoLayout(*other.m_pRsrvVideoLayout[i]);
	}

	if ((m_videoSession != CONTINUOUS_PRESENCE) &&
	    (m_videoSession != SOFTWARE_CONTINUOUS_PRESENCE) &&
	    (m_videoSession != ADVANCED_LAYOUTS))
	{
		SetContinuousPresenceScreenNumber(ONE_ONE);
	}

	m_videoPictureFormat = other.m_videoPictureFormat;
	m_CIFframeRate       = other.m_CIFframeRate;
	m_QCIFframeRate      = other.m_QCIFframeRate;
	m_audioTone          = other.m_audioTone;

	m_alertToneTiming = other.m_alertToneTiming;
	m_talkHoldTime    = other.m_talkHoldTime;
	m_audioMixDepth   = other.m_audioMixDepth;

	for (i = 0; i < MAX_PARTIES_IN_CONF; i++)
	{
		POBJDELETE(m_pParty[i]);
		if (other.m_pParty[i])
		{
			m_pParty[i] = new CRsrvParty(*other.m_pParty[i]);
			m_pParty[i]->SetRes(this);
		}
	}

	m_current_numb_of_party = other.m_current_numb_of_party;
	m_ind_service_phone     = other.m_ind_service_phone;
	m_ind_rsrv_vid_layout   = other.m_ind_rsrv_vid_layout;
	m_isGateway             = other.m_isGateway;
	m_isVideoInviteGateway  = other.m_isVideoInviteGateway;
	m_GWDialOutProtocols    = other.m_GWDialOutProtocols;
	m_videoProtocol         = other.m_videoProtocol;
	m_meetMePerConf         = other.m_meetMePerConf;
	m_numServicePhoneStr    = other.m_numServicePhoneStr;
	m_numServicePrefixStr   = other.m_numServicePrefixStr;

	for (i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
	{
		POBJDELETE(m_pServicePhoneStr[i]);
		if (other.m_pServicePhoneStr[i])
			m_pServicePhoneStr[i] = new CServicePhoneStr(*other.m_pServicePhoneStr[i]);

		POBJDELETE(m_pServicePrefixStr[i]);
		if (other.m_pServicePrefixStr[i])
			m_pServicePrefixStr[i] = new CServicePrefixStr(*other.m_pServicePrefixStr[i]);
	}

	if (other.m_pAvMsgStruct)
		*m_pAvMsgStruct = *other.m_pAvMsgStruct;

	if (other.m_pLectureMode)
		*m_pLectureMode = *other.m_pLectureMode;

	m_cascadeMode                        = other.m_cascadeMode;
	m_cascadedLinksNumber                = other.m_cascadedLinksNumber;
	m_cascadedPartiesCounter             = other.m_cascadedPartiesCounter;
	m_numUndefParties                    = other.m_numUndefParties;
	m_repSchedulingId                    = other.m_repSchedulingId;
	m_time_beforeFirstJoin               = other.m_time_beforeFirstJoin;
	m_time_afterLastQuit                 = other.m_time_afterLastQuit;
	m_LastQuitType                       = other.m_LastQuitType;
	m_confLockFlag                       = other.m_confLockFlag;
	m_confSecureFlag                     = other.m_confSecureFlag;
	m_confOnHoldFlag                     = other.m_confOnHoldFlag;
	m_Mute_Incoming_Parties_Lecture_Mode = other.m_Mute_Incoming_Parties_Lecture_Mode;
    m_muteAllPartiesAudioExceptLeader    = other.m_muteAllPartiesAudioExceptLeader;
    m_muteAllPartiesVideoExceptLeader    = other.m_muteAllPartiesVideoExceptLeader;
	m_FECC_Enabled                       = other.m_FECC_Enabled;
	m_max_parties                        = other.m_max_parties;
	m_numRsrvVideoSource                 = other.m_numRsrvVideoSource;
	m_webReservUId                       = other.m_webReservUId;
	m_webOwnerUId                        = other.m_webOwnerUId;
	m_webDBId                            = other.m_webDBId;
	m_webReserved                        = other.m_webReserved;
	m_isMeetingRoom                      = other.m_isMeetingRoom;
	m_isConfTemplate                     = other.m_isConfTemplate;
	m_meetingRoomReoccurrenceNum         = other.m_meetingRoomReoccurrenceNum;
	m_meetingRoomState                   = other.m_meetingRoomState;
	m_partyIdCounter                     = other.m_partyIdCounter;
	m_unrsrvPartiesCounter               = other.m_unrsrvPartiesCounter;
	m_startConfRequiresLeaderOnOff       = other.m_startConfRequiresLeaderOnOff;
	m_terminateConfAfterChairDropped     = other.m_terminateConfAfterChairDropped;
	m_isAutoLayout                       = other.m_isAutoLayout;
	m_isSameLayout                       = other.m_isSameLayout;
	m_ipVideoRate                        = other.m_ipVideoRate;
	m_confType                           = other.m_confType;
	m_media                              = other.m_media;
	m_network                            = other.m_network;
	m_dwConfFlags                        = other.m_dwConfFlags;
	m_meetMePerEntryQ                    = other.m_meetMePerEntryQ;
	m_IsEntryQ                           = other.m_IsEntryQ;
	m_IsAdHoc                            = other.m_IsAdHoc;
	m_dwAdHocProfileId                   = other.m_dwAdHocProfileId;
	m_dualVideoMode                      = other.m_dualVideoMode;
	m_EnterpriseMode                     = other.m_EnterpriseMode;
	m_EnterpriseModeFixedRate            = other.m_EnterpriseModeFixedRate;
	m_PresentationProtocol               = other.m_PresentationProtocol;
	m_cascadeOptimizeResolution          = other.m_cascadeOptimizeResolution;
	m_isHighProfileContent               = other.m_isHighProfileContent;
	m_isVideoPlusConf                    = other.m_isVideoPlusConf;
	m_IsConfOnPort                       = other.m_IsConfOnPort;
	m_SubCPtype                          = other.m_SubCPtype;

	SetEncryptionParameters(other.m_encryption, other.m_eEncryptionType);

	if (other.m_pVisualEffectsInfo)
		*m_pVisualEffectsInfo = *other.m_pVisualEffectsInfo;

	if (other.m_pMessageOverlayInfo)
		*m_pMessageOverlayInfo = *other.m_pMessageOverlayInfo;

	if (other.m_pSiteNameInfo)
		*m_pSiteNameInfo = *other.m_pSiteNameInfo;

	if (other.m_pConfContactInfo)
		*m_pConfContactInfo = *other.m_pConfContactInfo;

	if (other.m_pAutoScanOrder)
		*m_pAutoScanOrder = *other.m_pAutoScanOrder;

	m_BillingInfo                   = other.m_BillingInfo;
	m_dwSummaryUpdateCounter        = other.m_dwSummaryUpdateCounter;
	m_dwFullUpdateCounter           = other.m_dwFullUpdateCounter;
	m_bChanged                      = other.m_bChanged;
	m_SummeryCreationUpdateCounter  = other.m_SummeryCreationUpdateCounter;
	m_isTemplate                    = other.m_isTemplate;
	m_EnableRecording               = other.m_EnableRecording;
	m_EnableRecordingIcon           = other.m_EnableRecordingIcon;
	m_StartRecPolicy                = other.m_StartRecPolicy;
	m_RecordingLinkControl          = other.m_RecordingLinkControl;
	m_IsEnableRecNotify			= other.m_IsEnableRecNotify;
	m_IsCascadeEQ                   = other.m_IsCascadeEQ;
	m_isSIPFactory                  = other.m_isSIPFactory;
	m_isAutoConnectSIPFactory       = other.m_isAutoConnectSIPFactory;
	m_HD                            = other.m_HD;
	m_isAdHocConf                   = other.m_isAdHocConf;
	m_videoQuality                  = other.m_videoQuality;
	m_IsAudioOnlyRecording          = other.m_IsAudioOnlyRecording;
	m_isTelePresenceMode            = other.m_isTelePresenceMode;
	m_manageTelepresenceLayoutInternaly = other.m_manageTelepresenceLayoutInternaly;
	m_telePresenceModeConfiguration = other.m_telePresenceModeConfiguration;
	m_telePresenceLayoutMode        = other.m_telePresenceLayoutMode;
	m_isCropping                    = other.m_isCropping;
	m_isLpr                         = other.m_isLpr;
	m_HDResolution                  = other.m_HDResolution;
	m_H264VSWHighProfilePreference  = other.m_H264VSWHighProfilePreference;
	m_isVideoClarityEnabled         = other.m_isVideoClarityEnabled;
	m_isSiteNamesEnabled            = other.m_isSiteNamesEnabled;
	m_internalConfStatus            = other.m_internalConfStatus;
	m_minNumVideoParties            = other.m_minNumVideoParties;
	m_minNumAudioParties            = other.m_minNumAudioParties;
	m_EchoSuppression               = other.m_EchoSuppression;
	m_KeyboardSuppression           = other.m_KeyboardSuppression;
	m_isAutoMuteNoisyParties        = other.m_isAutoMuteNoisyParties;
	m_AutoScanInterval              = other.m_AutoScanInterval;
	m_ShowContentAsVideo            = other.m_ShowContentAsVideo;
	m_AutoRedial                    = other.m_AutoRedial;
	m_bIsExclusiveContentMode       = other.m_bIsExclusiveContentMode;
	m_operatorConf                  = other.m_operatorConf;
	m_isPermanent                   = other.m_isPermanent;
	m_confMaxResolution             = other.m_confMaxResolution;

	POBJDELETE(m_pOperatorConfInfo);
	if (other.m_pOperatorConfInfo != NULL)
	{
		m_pOperatorConfInfo = new COperatorConfInfo(*other.m_pOperatorConfInfo);
	}

	if (other.m_pCopConfigurationList)
		*m_pCopConfigurationList = *other.m_pCopConfigurationList;

	strcpy_safe(m_appoitnmentID, other.m_appoitnmentID);
	strcpy_safe(m_meetingOrganizer, other.m_meetingOrganizer);
	strcpy_safe(m_sIpNumberAccess, other.m_sIpNumberAccess);
	strcpy_safe(m_sNumberAccess_1, other.m_sNumberAccess_1);
	strcpy_safe(m_sNumberAccess_2, other.m_sNumberAccess_2);
	strcpy_safe(m_sFreeText_1, other.m_sFreeText_1);
	strcpy_safe(m_sFreeText_2, other.m_sFreeText_2);
	strcpy_safe(m_sFreeText_3, other.m_sFreeText_3);

	m_isStreaming        = other.m_isStreaming;
	m_bGatheringEnabled  = other.m_bGatheringEnabled;
	m_eLanguage          = other.m_eLanguage;
	m_ResSts             = other.m_ResSts;               // Status Field
	m_isAudioClarity     = other.m_isAudioClarity;
	m_isAutoBrightness   = other.m_isAutoBrightness;
	m_ivrProviderEQ      = other.m_ivrProviderEQ;
	m_externalIvrControl = other.m_externalIvrControl;   // AT&T

	for (i = 0; i < NUM_OF_IP_SERVICES; i++)
	{
		strcpy_safe(m_ServiceRegistrationContent[i].service_name, other.m_ServiceRegistrationContent[i].service_name);
		m_ServiceRegistrationContent[i].sip_register = other.m_ServiceRegistrationContent[i].sip_register;
		m_ServiceRegistrationContent[i].accept_call  = other.m_ServiceRegistrationContent[i].accept_call;
		m_ServiceRegistrationContent[i].status       = other.m_ServiceRegistrationContent[i].status; // sipProxySts
	}

	m_TipCompatibility        = other.m_TipCompatibility;
	m_natKeepAlivePeriod      = other.m_natKeepAlivePeriod;
	m_SipRegistrationTotalSts = other.m_SipRegistrationTotalSts;   // sipProxySts
	m_FontType                = other.m_FontType;
	m_speakerChangeThreshold  = other.m_speakerChangeThreshold;
	m_confSpeakerChangeMode   = other.m_confSpeakerChangeMode;
	m_confMediaType           = other.m_confMediaType;
	m_eOperationPointPreset   = other.m_eOperationPointPreset;

	//Content Transcoding
    m_ContentMultiResolutionEnabled = other.m_ContentMultiResolutionEnabled;
    m_ContentXCodeH264Supported = other.m_ContentXCodeH264Supported;
    m_ContentXCodeH263Supported = other.m_ContentXCodeH263Supported;
    m_isCascadeOptimized = other.m_isCascadeOptimized;
    m_isAsSipContent     = other.m_isAsSipContent;
    m_mrcMcuId           = other.m_mrcMcuId;
    m_SrsPlaybackLayout  = other.m_SrsPlaybackLayout;
    m_avMcuCascadeVideoMode     = other.m_avMcuCascadeVideoMode;

    strcpy_safe(m_FocusUriScheduling, other.m_FocusUriScheduling);
    strcpy_safe(m_FocusUriCurrently, other.m_FocusUriCurrently);
	//Indication for audio participants
	m_IconDisplayPosition =  other.m_IconDisplayPosition;
	m_EnableAudioParticipantsIcon =  other.m_EnableAudioParticipantsIcon;
	m_AudioParticipantsIconDisplayMode = other.m_AudioParticipantsIconDisplayMode;
	m_AudioParticipantsIconDuration = other.m_AudioParticipantsIconDuration;
	m_EnableSelfNetworkQualityIcon =  other.m_EnableSelfNetworkQualityIcon;

    m_overideProfileLayout    = other.m_overideProfileLayout;
	return *this;
}

//--------------------------------------------------------------------------
void CCommResApi::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	// !!!! in order to cancel the const on the method
	CCommResApi* pNode = ((CCommResApi*)this);

	if (!pFatherNode)
	{
		pFatherNode = new CXMLDOMElement;
		pFatherNode->set_nodeName("RESERVATION");
	}

	pNode->SerializeXml(pFatherNode, 0xFFFFFFFF);
}

//--------------------------------------------------------------------------
// schema file name:  obj_mcu_memory_state.xsd
int CCommResApi::DeSerializeXml(CXMLDOMElement* pResNode, char* pszError, const char* strAction)
{
	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode, pszError, numAction);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::convertStrActionToNumber(const char* strAction)
{
	int numAction = UNKNOWN_ACTION;
	if (strAction != NULL && !strncmp("START", strAction, 5))
		numAction = ADD_RESERVE;

	return numAction;
}

//--------------------------------------------------------------------------
void CCommResApi::SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken)
{
	CXMLDOMElement* pResNode;
	unsigned char bChanged = FALSE;

	// !!! Udi Add-on for the Profiles Serilaize
	char* nodeName;
	pActionNode->get_nodeName(&nodeName);
	if (strcmp(nodeName, "RESERVATION") != 0)
		pResNode = pActionNode->AddChildNode("RESERVATION");
	else
		pResNode = pActionNode;

	if (ObjToken == 0xFFFFFFFF)
		bChanged = TRUE;
	else
	{
		DWORD ClientCounter = ObjToken;
		if (m_dwFullUpdateCounter > ClientCounter)
		{
			bChanged = TRUE;
		}

		// VNGR-16334 - Cannot delete participant from Conference Template
		// m_dwFullUpdateCounter was reset in CCommResApi::DeserializeXml after Update was sent to template with ObjToken -1
		// but ObjToken is higher at EMA.
		// setting bChanged=TRUE refresh EMA and update ObjToken at EMA
		// This can be a general fix, but because V7.0 will be closed thit week we limit the change to Templates, in order to reduce side-effect risk
		if (m_isConfTemplate && m_dwFullUpdateCounter < ClientCounter)
		{
			TRACEINTO << "ConfName:" << m_H243confName << ", FullUpdateCounter:" << m_dwFullUpdateCounter << ", ObjToken:" << ObjToken << " - Set ObjToken after Counter reset (for template)";
			bChanged = TRUE;
		}
	}

	CXMLDOMElement* pTokenNode = pResNode->AddChildNode("OBJ_TOKEN", m_dwFullUpdateCounter);

	pResNode->AddChildNode("CHANGED", bChanged, _BOOL);

	if (bChanged)
	{
		AddResXmlToResponse(pResNode);
		AddPartiesXmlToResponse(pResNode);
	}
}

//--------------------------------------------------------------------------
void CCommResApi::AddResXmlToResponse(CXMLDOMElement* pResNode, int ConfOpcode /*=CONF_COMPLETE_INFO*/)
{
	CXMLDOMElement* pTempNode  = NULL;
	CXMLDOMElement* pTempNode2 = NULL;

	pResNode->AddChildNode("NAME", m_H243confName);
	pResNode->AddChildNode("ID", m_confId);
	pResNode->AddChildNode("CORRELATION_ID", m_correlationId);

	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("NETWORK", m_network, NETWORK_ENUM);
		pResNode->AddChildNode("MEDIA", m_media, MEDIA_ENUM);
		pResNode->AddChildNode("PASSWORD", m_H243_password);

		BYTE videoSession = m_videoSession;
		pResNode->AddChildNode("VIDEO_SESSION", m_videoSession, VIDEO_SESSION_ENUM);
		pResNode->AddChildNode("VIDEO_PROTOCOL", m_videoProtocol, VIDEO_PROTOCOL_ENUM);
		pResNode->AddChildNode("TRANSFER_RATE", m_confTransferRate, TRANSFER_RATE_ENUM);
		pResNode->AddChildNode("AUDIO_RATE", m_audioRate, AUDIO_RATE_ENUM);
		pResNode->AddChildNode("VIDEO_FORMAT", m_videoPictureFormat, VIDEO_FORMAT_ENUM);
		pResNode->AddChildNode("FRAME_RATE", m_CIFframeRate, FRAME_RATE_ENUM);
		pResNode->AddChildNode("EXTERNAL_IVR_CONTROL", m_externalIvrControl, _BOOL); // AT&T
		m_pAvMsgStruct->SerializeXml(pResNode);
		pResNode->AddChildNode("STAND_BY", m_stand_by, _BOOL);
		pResNode->AddChildNode("OPERATOR_CONF", m_operatorConf, _BOOL);              // create operator conf
		pResNode->AddChildNode("SAME_LAYOUT", m_isSameLayout, _BOOL);
		pTempNode = pResNode->AddChildNode("AUTO_TERMINATE");
		pTempNode->AddChildNode("ON", m_automaticTermination, _BOOL);
		pTempNode->AddChildNode("TIME_BEFORE_FIRST_JOIN", m_time_beforeFirstJoin);
		pTempNode->AddChildNode("TIME_AFTER_LAST_QUIT", m_time_afterLastQuit);
		pTempNode->AddChildNode("LAST_QUIT_TYPE", m_LastQuitType, LAST_QUIT_TYPE_ENUM);
		pResNode->AddChildNode("AUDIO_MIX_DEPTH", m_audioMixDepth);
		pResNode->AddChildNode("TALK_HOLD_TIME", 150);
		pResNode->AddChildNode("MAX_PARTIES", m_max_parties, MAX_PARTIES_ENUM);
		pTempNode = pResNode->AddChildNode("MEET_ME_PER_CONF");

		if (m_meetMePerConf == FALSE && m_meetMePerEntryQ == FALSE && !m_IsEntryQ)
		{
			pTempNode->AddChildNode("ON", FALSE, _BOOL);
			pTempNode->AddChildNode("AUTO_ADD", FALSE, _BOOL);
		}
		else
		{
			pTempNode->AddChildNode("ON", m_meetMePerConf, _BOOL);
			pTempNode->AddChildNode("AUTO_ADD", TRUE, _BOOL);
			pTempNode->AddChildNode("MIN_NUM_OF_PARTIES", m_minNumVideoParties);
			for (int i = 0; i < m_numServicePhoneStr; i++)
				m_pServicePhoneStr[i]->SerializeXml(pTempNode);

			pTempNode->AddChildNode("MIN_NUM_OF_AUDIO_PARTIES", m_minNumAudioParties);
			pTempNode->AddChildNode("SERVICE_NAME_FOR_MIN_PARTIES", m_ServiceNameForMinParties);
		}

		pResNode->AddChildNode("H323_BIT_RATE", m_ipVideoRate);
		pResNode->AddChildNode("LEADER_PASSWORD", m_H243_password);
		pResNode->AddChildNode("START_CONF_LEADER", m_startConfRequiresLeaderOnOff, _BOOL);
		pResNode->AddChildNode("TERMINATE_AFTER_LEADER_EXIT", m_terminateConfAfterChairDropped, _BOOL);
		pResNode->AddChildNode("REPEATED_ID", m_repSchedulingId);
		pTempNode = pResNode->AddChildNode("MEETING_ROOM");
		pTempNode->AddChildNode("ON", m_isMeetingRoom, _BOOL);
		if (m_meetingRoomReoccurrenceNum == 0xff)
			pTempNode->AddChildNode("LIMITED_SEQ", "off");
		else
			pTempNode->AddChildNode("LIMITED_SEQ", m_meetingRoomReoccurrenceNum, LIMITED_SEQ_ENUM);

		if (m_isMeetingRoom)                                                      // /!!!!to check
			pTempNode->AddChildNode("MR_STATE", m_meetingRoomState, MR_STATE_ENUM); // /!!!!to check

		pResNode->AddChildNode("OVERRIDE_PROFILE_LAYOUT", m_overideProfileLayout,_BOOL);
	}
	else
	{
		// patch .form 4.6 the password change(slow) - probably not needed for Carmel
		if (ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO)
			pResNode->AddChildNode("PASSWORD", m_H243_password);
	}

	// /////////////fields relevant for conference ////////////////////////////////////////////////////
	if (ConfOpcode == CONF_COMPLETE_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO)
	{
		if (m_startTime.IsValid())
			pResNode->AddChildNode("START_TIME", m_startTime);

		pTempNode = pResNode->AddChildNode("DURATION");
		pTempNode->AddChildNode("HOUR", m_duration.m_hour);
		pTempNode->AddChildNode("MINUTE", m_duration.m_min);
		pTempNode->AddChildNode("SECOND", m_duration.m_sec);
		pTempNode = pResNode->AddChildNode("CASCADE");
		pTempNode->AddChildNode("CASCADE_ROLE", m_cascadeMode, CASCADE_ROLE_ENUM);
		pTempNode->AddChildNode("CASCADED_LINKS_NUMBER", m_cascadedLinksNumber, _0_TO_4_DECIMAL);
		pResNode->AddChildNode("MUTE_PARTIES_IN_LECTURE", m_Mute_Incoming_Parties_Lecture_Mode, _BOOL);
		pResNode->AddChildNode("MUTE_ALL_PARTIES_AUDIO_EXCEPT_LEADER", m_muteAllPartiesAudioExceptLeader, _BOOL);
		pResNode->AddChildNode("MUTE_ALL_PARTIES_VIDEO_EXCEPT_LEADER", m_muteAllPartiesVideoExceptLeader, _BOOL);

		m_pLectureMode->SerializeXml(pResNode);

		AddVideoLayoutToResResponse(pResNode);
	}

	// 4.6
	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("ENTRY_QUEUE", m_IsEntryQ, _BOOL);
		pResNode->AddChildNode("MEET_ME_PER_ENTRY_QUEUE", m_meetMePerEntryQ, _BOOL);
	}

	if (ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO || ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("ENTRY_PASSWORD", m_entry_password);
	}

	// 5.0
	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("VIDEO_PLUS", m_isVideoPlusConf, _BOOL);
	}

	if (ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO || ConfOpcode == CONF_COMPLETE_INFO)
	{
		m_pVisualEffectsInfo->SerializeXml(pResNode);
		m_pMessageOverlayInfo->SerializeXml(pResNode);
		m_pSiteNameInfo->SerializeXml(pResNode);
		m_pAutoScanOrder->SerializeXml(pResNode);
	}

	// 5.6
	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("COP", m_IsConfOnPort, _BOOL);
	}

	// 6.0
	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("PROFILE", m_isTemplate, _BOOL);
		if (m_numServicePrefixStr > 0)
		{
			pTempNode = pResNode->AddChildNode("DIAL_IN_H323_SRV_PREFIX_LIST");
			for (int i = 0; i < (int)m_numServicePrefixStr; i++)
			{
				if (m_pServicePrefixStr[i])
					m_pServicePrefixStr[i]->SerializeXml(pTempNode);
				else
				{
					CSmallString message = "Service prefix was not found : ";
					message << i << " ; m_numServicePrefixStr = "<<m_numServicePrefixStr;

					PASSERTMSG(TRUE, message.GetString());
				}
			}
		}
	}

	if (ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO || ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("AD_HOC", m_IsAdHoc, _BOOL);
		pResNode->AddChildNode("AD_HOC_PROFILE_ID", m_dwAdHocProfileId);
		pResNode->AddChildNode("BASE_PROFILE_NAME", m_baseProfileName);
		pResNode->AddChildNode("AUTO_LAYOUT", m_isAutoLayout, _BOOL);
		pResNode->AddChildNode("BILLING_DATA", m_BillingInfo);
		if (strcmp(m_NumericConfId, "") != 0)
			pResNode->AddChildNode("NUMERIC_ID", m_NumericConfId);

		if (m_pConfContactInfo)
			m_pConfContactInfo->SerializeXml(pResNode);
	}

	// version 7.0
	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("ENCRYPTION", m_encryption, _BOOL);
		pResNode->AddChildNode("ENCRYPTION_TYPE", m_eEncryptionType, ENCRYPTION_TYPE_ENUM);
		pResNode->AddChildNode("VIDEO", m_video, _BOOL);                                               // not from version 7
		pResNode->AddChildNode("QCIF_FRAME_RATE", m_QCIFframeRate, FRAME_RATE_ENUM);                   // not from version 7
		pResNode->AddChildNode("CONFERENCE_TYPE", CONF_TYPE_STANDARD, CONFERENCE_TYPE_ENUM);           // not from version 7
		pResNode->AddChildNode("ENABLE_RECORDING", m_EnableRecording, _BOOL);
		pResNode->AddChildNode("ENABLE_RECORDING_INDICATION", m_EnableRecordingIcon, _BOOL);
		pResNode->AddChildNode("START_REC_POLICY", m_StartRecPolicy, START_REC_POLICY_ENUM);
		pResNode->AddChildNode("REC_LINK_NAME", m_RecLinkName);

		pResNode->AddChildNode("VIDEO_QUALITY", m_videoQuality, VIDEO_QUALITY_ENUM);
		pResNode->AddChildNode("MAX_RESOLUTION", m_confMaxResolution, RESOLUTION_SLIDER_ENUM);
		pResNode->AddChildNode("ENTRY_QUEUE_TYPE", 0, ENTRY_QUEUE_TYPE_ENUM); // 0: normal (not recording system)
		pResNode->AddChildNode("CASCADE_EQ", m_IsCascadeEQ, _BOOL);
		pResNode->AddChildNode("SIP_FACTORY", m_isSIPFactory, _BOOL);
		pResNode->AddChildNode("SIP_FACTORY_AUTO_CONNECT", m_isAutoConnectSIPFactory, _BOOL);
		pResNode->AddChildNode("ENTERPRISE_MODE", m_EnterpriseMode, ENTERPRISE_MODE_ENUM);

		if (m_EnterpriseMode == eCustomizedRate)
			pResNode->AddChildNode("CUSTOMIZED_CONTENT_RATE_VALUE", m_EnterpriseModeFixedRate, TRANSFER_RATE_ENUM);

		pResNode->AddChildNode("ENTERPRISE_PROTOCOL", m_PresentationProtocol, PRESENTATION_PROTOCOL_ENUM);
		if (m_cascadeOptimizeResolution != 0)
			pResNode->AddChildNode("CASCADE_OPTIMIZE_RESOLUTION", m_cascadeOptimizeResolution, CASCADE_OPTIMIZE_RESOLUTION_ENUM);

		pResNode->AddChildNode("HD", m_HD, _BOOL);
		pResNode->AddChildNode("TIP_COMPATIBILITY", m_TipCompatibility, TIP_COMPATIBILITY_ENUM);
		pResNode->AddChildNode("NAT_KEEP_ALIVE_PERIOD", m_natKeepAlivePeriod, _0_TO_MAXKAVALUE);
		pResNode->AddChildNode("NAT_KEEP_ALIVE_PERIOD", m_natKeepAlivePeriod, _0_TO_MAXKAVALUE);
	//Only For MS

		pResNode->AddChildNode("AV_MCU_FOCUS_URI",m_FocusUriScheduling, ONE_LINE_BUFFER_LENGTH);

		pResNode->AddChildNode("AV_MCU_CASCADE_MODE", m_avMcuCascadeVideoMode, AV_MCU_CASCADE_MODE_ENUM);

		//eFeatureRssDialin
		pResNode->AddChildNode("SEND_RECORDING_ANNOUNCEMENT", m_IsEnableRecNotify, _BOOL);
		pResNode->AddChildNode("RECORDING_SERVER_PLAYBACK_LAYOUT_MODE ", m_SrsPlaybackLayout, SRS_PLAYBACK_LAYOUT_MODE_ENUM);
	}

	if (ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("IS_TELEPRESENCE_MODE", m_isTelePresenceMode, _BOOL);
	}

	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("DISPLAY_NAME", m_confDisplayName);
		pResNode->AddChildNode("AUDIO_ONLY_RECORDING", m_IsAudioOnlyRecording, _BOOL);
	}

	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("LPR", m_isLpr, _BOOL);

		pTempNode = pResNode->AddChildNode("CONFERENCE_TEMPLATE");
		pTempNode->AddChildNode("ON", m_isConfTemplate, _BOOL);
		pResNode->AddChildNode("HD_RESOLUTION", m_HDResolution, HD_RESOLUTION_ENUM);
		pResNode->AddChildNode("H264_VSW_HIGH_PROFILE_PREFERENCE", m_H264VSWHighProfilePreference, H264_VSW_HIGH_PROFILE_PREFERENCE_ENUM);
	}

	if (ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO || ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("VIDEO_CLARITY", m_isVideoClarityEnabled, _BOOL);
		pResNode->AddChildNode("AUTO_REDIAL", m_AutoRedial, _BOOL);
	}

	pResNode->AddChildNode("EXCLUSIVE_CONTENT_MODE", m_bIsExclusiveContentMode, _BOOL); // guy
	pResNode->AddChildNode("ENABLE_FECC", m_FECC_Enabled, _BOOL);                       // guy

	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("PERMANENT", m_isPermanent, _BOOL);
	}

	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("ECHO_SUPPRESSION", m_EchoSuppression, _BOOL);
		pResNode->AddChildNode("KEYBOARD_SUPPRESSION", m_KeyboardSuppression, _BOOL);

		pResNode->AddChildNode("GATEWAY", m_isGateway, _BOOL);
		pTempNode = pResNode->AddChildNode("GW_DIAL_OUT_PROTOCOLS");
		pTempNode->AddChildNode("H323", m_GWDialOutProtocols& GW_H323_OUT, _BOOL);
		pTempNode->AddChildNode("SIP", m_GWDialOutProtocols& GW_SIP_OUT, _BOOL);
		pTempNode->AddChildNode("H320", m_GWDialOutProtocols& GW_H320_OUT, _BOOL);
		pTempNode->AddChildNode("PSTN", m_GWDialOutProtocols& GW_PSTN_OUT, _BOOL);

		pResNode->AddChildNode("CONTENT_TO_LEGACY_EPS", m_ShowContentAsVideo, _BOOL);

		m_pCopConfigurationList->SerializeXml(pResNode);
		pResNode->AddChildNode("APPOINTMENT_ID", m_appoitnmentID);
		pResNode->AddChildNode("SITE_NAMES", m_isSiteNamesEnabled, _BOOL);
	}

	if (ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO || ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("AUTO_SCAN_INTERVAL", m_AutoScanInterval, AUTO_SCAN_INTERVAL_ENUM);
	}

	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pTempNode = pResNode->AddChildNode("GATHERING");
		if (pTempNode)
		{
			pTempNode->AddChildNode("ENABLE_GATHERING", m_bGatheringEnabled, _BOOL);
			pTempNode->AddChildNode("LANGUAGE", m_eLanguage, LANGUAGES_ENUM);
			pTempNode->AddChildNode("IP_NUMBER_ACCESS", m_sIpNumberAccess);
			pTempNode->AddChildNode("ACCESS_NUMBER_1", m_sNumberAccess_1);
			pTempNode->AddChildNode("ACCESS_NUMBER_2", m_sNumberAccess_2);
			pTempNode->AddChildNode("FREE_TEXT_1", m_sFreeText_1);
			pTempNode->AddChildNode("FREE_TEXT_2", m_sFreeText_2);
			pTempNode->AddChildNode("FREE_TEXT_3", m_sFreeText_3);
		}
	}

	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("TELEPRESENCE_MODE_CONFIGURATION", m_telePresenceModeConfiguration, TELEPRESENCE_MODE_CONFIGURATION_ENUM);
		pResNode->AddChildNode("TELEPRESENCE_LAYOUT_MODE", m_telePresenceLayoutMode, TELEPRESENCE_LAYOUT_MODE_ENUM);
		pResNode->AddChildNode("CROPPING", m_isCropping, _BOOL);

		// TELEPRESENCE_LAYOUTS
		if((ETelePresenceLayoutMode)m_telePresenceLayoutMode <= eTelePresenceLayoutCpParticipantsPriority)
			TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG m_telePresenceLayoutMode = " << TelePresenceLayoutModeToString((ETelePresenceLayoutMode)m_telePresenceLayoutMode);
		else
			PASSERT(m_telePresenceLayoutMode);

		pResNode->AddChildNode("RES_STATUS", m_ResSts, RESERVATION_STATUS_ENUM); // Status Field

		pResNode->AddChildNode("AUDIO_CLARITY", m_isAudioClarity, _BOOL);

		pResNode->AddChildNode("SPEAKER_CHANGE_THRESHOLD", m_speakerChangeThreshold, SPEAKER_CHANGE_THRESHOLD_ENUM);

		pResNode->AddChildNode("AUTO_BRIGHTNESS", m_isAutoBrightness, _BOOL);
		pResNode->AddChildNode("IVR_PROVIDER_EQ", m_ivrProviderEQ, _BOOL);
	}

	pResNode->AddChildNode("CHINESE_FONT", m_FontType, FONT_TYPES_ENUM);
	if (ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_COMPLETE_INFO)
	{
		pTempNode = pResNode->AddChildNode("SERVICE_REGISTRATION_LIST");
		if (pTempNode)
		{
			for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
			{
				if (m_ServiceRegistrationContent[i].service_name[0] != '\0')
				{
					pTempNode2 = pTempNode->AddChildNode("SERVICE_REGISTRATION_CONTENT");
					pTempNode2->AddChildNode("SERVICE_NAME", m_ServiceRegistrationContent[i].service_name, ONE_LINE_BUFFER_LENGTH);
					pTempNode2->AddChildNode("SIP_REGISTRATION", m_ServiceRegistrationContent[i].sip_register, _BOOL);
					pTempNode2->AddChildNode("ACCEPT_CALLS", m_ServiceRegistrationContent[i].accept_call, _BOOL);
					pTempNode2->AddChildNode("SIP_REGISTRATION_STATUS", m_ServiceRegistrationContent[i].status, SIP_REG_STATUS_ENUM); // sipProxySts
				}
			}
		}

		pResNode->AddChildNode("SIP_REGISTRATIONS_STATUS", m_SipRegistrationTotalSts, SIP_REG_TOTAL_STATUS_ENUM); // sipProxySts
	}

	// version RMX 7.8
	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("CONF_MEDIA_TYPE", m_confMediaType, CONF_MEDIA_TYPE_ENUM);
	}

	// IBM
	if (ConfOpcode == CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("OPERATION_POINTS_PRESET", m_eOperationPointPreset, OPERATION_POINTS_PRESET_ENUM);
		pResNode->AddChildNode("MRC_MCU_ID", m_mrcMcuId);
	}
	if (ConfOpcode == CONF_COMPLETE_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW_INFO || ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO)
	{
		pResNode->AddChildNode("MULTIPLE_RESOLUTION", m_ContentMultiResolutionEnabled, _BOOL);
		pResNode->AddChildNode("CONTENT_TRANSCODING_H264", m_ContentXCodeH264Supported, _BOOL);
		pResNode->AddChildNode("CONTENT_TRANSCODING_H263", m_ContentXCodeH263Supported, _BOOL);
		pResNode->AddChildNode("H264_CASCADE_OPTIMIZED", m_isCascadeOptimized, _BOOL);
		pResNode->AddChildNode("AS_SIP_CONTENT",m_isAsSipContent, _BOOL);
	}

	//Selective Mixing
	if(ConfOpcode==CONF_COMPLETE_INFO)
	{
		if ((YES == m_isAutoMuteNoisyParties) &&
			(eSvcOnly == m_confMediaType || eMixAvcSvcVsw == m_confMediaType))
		{
			m_isAutoMuteNoisyParties = NO;
		}
		pResNode->AddChildNode("AUTO_MUTE_NOISY_PARTIES", m_isAutoMuteNoisyParties, _BOOL);
	}
	// version RMX 8.3
	if(ConfOpcode==CONF_COMPLETE_INFO)
	{
		pResNode->AddChildNode("HIGH_PROFILE_CONTENT",m_isHighProfileContent, _BOOL);

		pTempNode = pResNode->AddChildNode("LAYOUT_INDICATIONS");
		if (pTempNode)
		{
			pTempNode->AddChildNode("LAYOUT_INDICATIONS_POSITION", m_IconDisplayPosition, ICON_DISPLAY_POSITION_ENUM);
			pTempNode->AddChildNode("LAYOUT_INDICATIONS_AUDIO_PARTICIPANTS", m_EnableAudioParticipantsIcon, _BOOL);
			pTempNode->AddChildNode("LAYOUT_INDICATIONS_AUDIO_PARTICIPANTS_DISPLAY_MODE", m_AudioParticipantsIconDisplayMode, ICON_DISPLAY_MODE_ENUM);
			pTempNode->AddChildNode("LAYOUT_INDICATIONS_AUDIO_PARTICIPANTS_DISPLAY_DURATION", m_AudioParticipantsIconDuration, _3_TO_300_DECIMAL);
			pTempNode->AddChildNode("LAYOUT_SELF_INDICATIONS_NETWORK_QUALITY", m_EnableSelfNetworkQualityIcon, _BOOL);
		}
	}
}

//--------------------------------------------------------------------------
void CCommResApi::AddVideoLayoutToResResponse(CXMLDOMElement* pResNode)
{
	CXMLDOMElement* pForceNode;

	CVideoLayout* pVideoLayout = GetActiveRsrvVidLayout();
	if (pVideoLayout)
		pResNode->AddChildNode("LAYOUT", pVideoLayout->GetScreenLayoutITP(), LAYOUT_ENUM);
	else
		pResNode->AddChildNode("LAYOUT", "1x1");

	CXMLDOMElement* pForceListNode = pResNode->AddChildNode("FORCE_LIST");
	pVideoLayout = GetFirstRsrvVidLayout();
	while (pVideoLayout)
	{
		pForceNode = pForceListNode->AddChildNode("FORCE");
		pVideoLayout->SerializeXml(pForceNode);
		pVideoLayout = GetNextRsrvVidLayout();
	}
}

//--------------------------------------------------------------------------
void CCommResApi::AddPartiesXmlToResponse(CXMLDOMElement* pResNode)
{
	if (!(m_dwConfFlags & SECURED))
	{
		CXMLDOMElement* pPartyListNode = pResNode->AddChildNode("PARTY_LIST");
		for (int i = 0; i < (int)m_numParties; i++)
			m_pParty[i]->SerializeXml(pPartyListNode, FULL_DATA);
	}
}

//--------------------------------------------------------------------------
int CCommResApi::DeSerializePartyListXml(CXMLDOMElement* pListNode, char* pszError, int nAction)
{
	CXMLDOMElement* pPartyNode = NULL;

	GET_FIRST_CHILD_NODE(pListNode, "PARTY", pPartyNode);
	DWORD maxPartyId = 0;

	while (pPartyNode)
	{
		CRsrvParty* pParty = new CRsrvParty;
		int nStatus = pParty->DeSerializeXml(pPartyNode, pszError, nAction);

		GET_NEXT_CHILD_NODE(pListNode, "PARTY", pPartyNode);

		// MCC: Special handling for the Save Conference to Template
		if (STATUS_OK == nStatus && m_isConfTemplate)
		{
			switch (pParty->GetPartyType())
			{
				case eSubLinkParty:  // do NOT save to template sub-link parties
					POBJDELETE(pParty);
					continue;

				case eMainLinkParty: // remove the trailing '_1' from the party name
				{
					char name[H243_NAME_LEN]={0};
					strcpy_safe(name, pParty->GetName());

					size_t size = strlen(name) - 2;
					if (name[size] == '_' && name[size+1] == '1')
						name[size] = 0;

					pParty->SetName(name);
					break;
				}
				default:
					// Note: some enumeration value are not handled in switch. Add default to suppress warning.
					break;
			} // switch
		}

		if (nStatus != STATUS_OK)
		{
			POBJDELETE(pParty);
			return nStatus;
		}

		Add(*pParty);

		maxPartyId = std::max(maxPartyId, pParty->GetPartyId());
		POBJDELETE(pParty);
	}

	// Setting the party Id counter if we find a party
	if (m_numParties > 0)
		m_partyIdCounter = ++maxPartyId;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::DeSerializeXml(CXMLDOMElement* pResNode, char* pszError, const int nAction)
{
	CXMLDOMElement* pChildNode, * pVisualEffectsNode, * pMessageOverlatNode, * pServiceNode, * pSiteNamelatNode;
	DWORD           nVal;
	BYTE            nByteVal;
	int             nStatus = STATUS_OK;
	char*           pszVal  = NULL;

	m_bChanged = TRUE;
	BYTE            bIsProfileLoadedFromDb = (nAction == UNKNOWN_ACTION) ? TRUE : FALSE;

	GET_VALIDATE_CHILD(pResNode, "OBJ_TOKEN", &m_dwFullUpdateCounter, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pResNode, "CHANGED", &m_bChanged, _BOOL);

	if (nAction != ADD_REPEATED_EX && nAction != ADD_RESERVE)
	{
		if (m_bChanged == FALSE)
			return STATUS_OK;
	}

	GET_VALIDATE_CHILD(pResNode, "NAME", m_H243confName, _0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pResNode, "ID", &m_confId, _0_TO_DWORD);

	if (nAction != AUTHENTICATED_START && nAction != ADD_RESERVE && nStatus == STATUS_NODE_MISSING) // Action update
		return nStatus;

	GET_VALIDATE_CHILD(pResNode, "NETWORK", &m_network, NETWORK_ENUM);
	GET_VALIDATE_CHILD(pResNode, "MEDIA", &m_media, MEDIA_ENUM);
	// TODO BG Audio only conference
	// m_media = AUDIO_MEDIA;
	if (m_media == AUDIO_MEDIA &&
		(CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw) &&
		(CProcessBase::GetProcess()->GetProductType() != eProductTypeNinja))
		m_media = AUDIO_VIDEO_MEDIA;

	GET_VALIDATE_CHILD(pResNode, "PASSWORD", m_H243_password, _0_TO_H243_NAME_LENGTH);
	if (m_H243_password == NULL)
		GET_VALIDATE_CHILD(pResNode, "LEADER_PASSWORD", m_H243_password, _0_TO_H243_NAME_LENGTH);

	GET_VALIDATE_CHILD(pResNode, "VIDEO_SESSION", &m_videoSession, VIDEO_SESSION_ENUM);
	GET_VALIDATE_CHILD(pResNode, "VIDEO_PROTOCOL", &m_videoProtocol, VIDEO_PROTOCOL_ENUM);
	GET_VALIDATE_CHILD(pResNode, "TRANSFER_RATE", &m_confTransferRate, TRANSFER_RATE_ENUM);
	GET_VALIDATE_CHILD(pResNode, "AUDIO_RATE", &m_audioRate, AUDIO_RATE_ENUM);
	GET_VALIDATE_CHILD(pResNode, "VIDEO_FORMAT", &m_videoPictureFormat, VIDEO_FORMAT_ENUM);
	GET_VALIDATE_CHILD(pResNode, "FRAME_RATE", &m_CIFframeRate, FRAME_RATE_ENUM);
	// AT&T - EXTERNAL_IVR_CONTROL should come before we do m_pAvMsgStruct->DeSerializeXml(pResNode, pszError)!!
	GET_VALIDATE_CHILD(pResNode, "EXTERNAL_IVR_CONTROL", &m_externalIvrControl, _BOOL);
	if (m_pAvMsgStruct)
	{
		if (m_externalIvrControl == NO)
		{
			nStatus = m_pAvMsgStruct->DeSerializeXml(pResNode, pszError);
			PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
		}
	}

	GET_CHILD_NODE(pResNode, "MEETING_ROOM", pChildNode);
	if (pChildNode)
	{
		pChildNode->ResetChildList();
		GET_VALIDATE_CHILD(pChildNode, "ON", &nVal, _BOOL);
		if (nVal)
			m_isMeetingRoom = YES;

		GET_VALIDATE_CHILD(pChildNode, "LIMITED_SEQ", &m_meetingRoomReoccurrenceNum, LIMITED_SEQ_ENUM);
		GET_VALIDATE_CHILD(pChildNode, "MR_STATE", &m_meetingRoomState, MR_STATE_ENUM);
	}

	// if the START_TIME node is missing, this is start immediately
	m_startTime.m_day  = 0;
	m_startTime.m_year = 0;

	GET_VALIDATE_CHILD(pResNode, "START_TIME", &m_startTime, DATE_TIME);

	GET_CHILD_NODE(pResNode, "DURATION", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode, "HOUR", &m_duration.m_hour, _0_TO_DWORD);
		GET_VALIDATE_CHILD(pChildNode, "MINUTE", &m_duration.m_min, _0_TO_59_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode, "SECOND", &m_duration.m_sec, _0_TO_59_DECIMAL);
		SetEndTime();
	}
	else
	{
		// default duration is 2 hours
		if (nAction == AUTHENTICATED_START || nAction == ADD_RESERVE)
		{
			CStructTm TempTime(0, 0, 0, 2, 0, 0);
			m_duration = TempTime;
			SetEndTime();
		}
	}

	SetVideoLayoutParams(pResNode, pszError);

	GET_VALIDATE_CHILD(pResNode, "STAND_BY", &m_stand_by, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "OPERATOR_CONF", &m_operatorConf, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "SAME_LAYOUT", &m_isSameLayout, _BOOL);
	GET_CHILD_NODE(pResNode, "AUTO_TERMINATE", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode, "ON", &m_automaticTermination, _BOOL);
		GET_VALIDATE_CHILD(pChildNode, "TIME_BEFORE_FIRST_JOIN", &m_time_beforeFirstJoin, _1_TO_60_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode, "TIME_AFTER_LAST_QUIT", &m_time_afterLastQuit, _0_TO_60_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode, "LAST_QUIT_TYPE", &m_LastQuitType, LAST_QUIT_TYPE_ENUM);
	}

	GET_VALIDATE_CHILD(pResNode, "AUDIO_MIX_DEPTH", &m_audioMixDepth, _1_TO_5_DECIMAL);
	GET_VALIDATE_CHILD(pResNode, "MAX_PARTIES", &m_max_parties, MAX_PARTIES_ENUM);
	GET_CHILD_NODE(pResNode, "MEET_ME_PER_CONF", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode, "ON", &m_meetMePerConf, _BOOL);
		GET_VALIDATE_CHILD(pChildNode, "MIN_NUM_OF_AUDIO_PARTIES", &m_minNumAudioParties, _0_TO_3600_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode, "MIN_NUM_OF_PARTIES", &m_minNumVideoParties, _0_TO_3600_DECIMAL);
		GET_FIRST_CHILD_NODE(pChildNode, "SERVICE", pServiceNode);
		while (pServiceNode)
		{
			CServicePhoneStr PhoneService;
			nStatus = PhoneService.DeSerializeXml(pServiceNode, pszError);
			PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);

			AddServicePhone(PhoneService);
			GET_NEXT_CHILD_NODE(pChildNode, "SERVICE", pServiceNode);
		}

		GET_VALIDATE_CHILD(pChildNode, "SERVICE_NAME_FOR_MIN_PARTIES", m_ServiceNameForMinParties, ONE_LINE_BUFFER_LENGTH);
	}

	GET_CHILD_NODE(pResNode, "CASCADE", pChildNode);
	if (pChildNode)
	{
		pChildNode->ResetChildList();
		GET_VALIDATE_CHILD(pChildNode, "CASCADE_ROLE", &m_cascadeMode, CASCADE_ROLE_ENUM);
		// do NOT deserialize the value of "CASCADED_LNIKS_NUMBER" into m_cascadedLinksNumber
	}

	GET_VALIDATE_CHILD(pResNode, "MUTE_PARTIES_IN_LECTURE", &m_Mute_Incoming_Parties_Lecture_Mode, _BOOL);
	GET_CHILD_NODE(pResNode, "LECTURE_MODE", pChildNode);
	if (pChildNode && m_pLectureMode)
	{
		nStatus = m_pLectureMode->DeSerializeXml(pChildNode, pszError);
		PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
	}

	GET_VALIDATE_CHILD(pResNode, "H323_BIT_RATE", &m_ipVideoRate, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pResNode, "TERMINATE_AFTER_LEADER_EXIT", &m_terminateConfAfterChairDropped, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "START_CONF_LEADER", &m_startConfRequiresLeaderOnOff, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "MEET_ME_PER_ENTRY_QUEUE", &m_meetMePerEntryQ, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "ENTRY_QUEUE", &m_IsEntryQ, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "ENTRY_PASSWORD", m_entry_password, _0_TO_CONFERENCE_ENTRY_PASSWORD_LENGTH);
	GET_CHILD_NODE(pResNode, "VISUAL_EFFECTS", pVisualEffectsNode);
	if (pVisualEffectsNode)
	{
		nStatus = m_pVisualEffectsInfo->DeSerializeXml(pVisualEffectsNode, pszError);
		PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
	}

	GET_CHILD_NODE(pResNode, "MESSAGE_OVERLAY", pMessageOverlatNode);
	if (pMessageOverlatNode)
	{
		nStatus = m_pMessageOverlayInfo->DeSerializeXml(pMessageOverlatNode, pszError);
		PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
	}

	GET_CHILD_NODE(pResNode, "SITE_NAME", pSiteNamelatNode);
	if (pSiteNamelatNode)
	{
		nStatus = m_pSiteNameInfo->DeSerializeXml(pSiteNamelatNode, pszError);

		PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
	}

	GET_VALIDATE_CHILD(pResNode, "COP", &m_IsConfOnPort, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "PROFILE", &m_isTemplate, _BOOL);

	GET_CHILD_NODE(pResNode, "DIAL_IN_H323_SRV_PREFIX_LIST", pChildNode);
	if (pChildNode)
	{
		CleanAllServicePrefix();
		GET_FIRST_CHILD_NODE(pChildNode, "DIAL_IN_H323_SRV_PREFIX", pServiceNode);
		while (pServiceNode)
		{
			CServicePrefixStr servicePrefixStr;
			nStatus = servicePrefixStr.DeSerializeXml(pServiceNode, pszError);
			PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);

			AddServicePrefix(servicePrefixStr);
			GET_NEXT_CHILD_NODE(pChildNode, "DIAL_IN_H323_SRV_PREFIX", pServiceNode);
		}
	}

	GET_VALIDATE_CHILD(pResNode, "AD_HOC", &m_IsAdHoc, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "AD_HOC_PROFILE_ID", &m_dwAdHocProfileId, _0_TO_DWORD);
	int tmpStatus = nStatus;
	GET_VALIDATE_CHILD(pResNode, "BASE_PROFILE_NAME", m_baseProfileName, _0_TO_H243_NAME_LENGTH);
	GET_VALIDATE_CHILD(pResNode, "AUTO_LAYOUT", &m_isAutoLayout, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "BILLING_DATA", m_BillingInfo, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pResNode, "NUMERIC_ID", m_NumericConfId, _0_TO_NUMERIC_CONFERENCE_ID_LENGTH);

	GET_CHILD_NODE(pResNode, "CONTACT_INFO_LIST", pChildNode);
	if (pChildNode)
	{
		nStatus = m_pConfContactInfo->DeSerializeXml(pChildNode, pszError);
		PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
	}

	BYTE xmlEncryptionFlag, xmlEncryptionType;
	GET_VALIDATE_CHILD(pResNode, "ENCRYPTION", &xmlEncryptionFlag, _BOOL);
	BYTE isOldEncryptionParamPresent = nStatus == STATUS_OK;
	xmlEncryptionType = eEncryptNone;
	GET_VALIDATE_CHILD(pResNode, "ENCRYPTION_TYPE", &xmlEncryptionType, ENCRYPTION_TYPE_ENUM);
	BYTE isNewEncryptionParamPresent = nStatus == STATUS_OK;

	// verify new encryption type value using the old parameter + the old configuration flag
	if (isOldEncryptionParamPresent)
	{
		if (xmlEncryptionFlag == YES)
		{
			if (!isNewEncryptionParamPresent)
				xmlEncryptionType = eEncryptAll;
		}
		else	// m_encryption == NO
		{
			if (xmlEncryptionType != eEncryptNone)
				xmlEncryptionFlag = YES;
			else
				xmlEncryptionFlag = NO;
		}
	}
	else
	{
		if (xmlEncryptionType != eEncryptNone)
			xmlEncryptionFlag = YES;
		else
			xmlEncryptionFlag = NO;
	}

	SetEncryptionParameters(xmlEncryptionFlag, xmlEncryptionType);

	GET_VALIDATE_CHILD(pResNode, "VIDEO", &m_video, _BOOL);                             // not from version 7
	GET_VALIDATE_CHILD(pResNode, "QCIF_FRAME_RATE", &m_QCIFframeRate, FRAME_RATE_ENUM); // not from version 7
	GET_VALIDATE_CHILD(pResNode, "REPEATED_ID", &m_repSchedulingId, _0_TO_DWORD);       // not from version 7
	GET_VALIDATE_CHILD(pResNode, "ENABLE_RECORDING", &m_EnableRecording, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "ENABLE_RECORDING_INDICATION", &m_EnableRecordingIcon, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "START_REC_POLICY", &m_StartRecPolicy, START_REC_POLICY_ENUM);
	GET_VALIDATE_CHILD(pResNode, "REC_LINK_NAME", m_RecLinkName, _0_TO_H243_NAME_LENGTH);

	GET_VALIDATE_CHILD(pResNode, "CASCADE_EQ", &m_IsCascadeEQ, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "SIP_FACTORY", &m_isSIPFactory, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "SIP_FACTORY_AUTO_CONNECT", &m_isAutoConnectSIPFactory, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "ENTERPRISE_MODE", &m_EnterpriseMode, ENTERPRISE_MODE_ENUM);

	if (m_EnterpriseMode == eCustomizedRate)
		GET_VALIDATE_CHILD(pResNode, "CUSTOMIZED_CONTENT_RATE_VALUE", &m_EnterpriseModeFixedRate, TRANSFER_RATE_ENUM);

	GET_VALIDATE_CHILD(pResNode, "HD", &m_HD, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "DISPLAY_NAME", m_confDisplayName, _0_TO_H243_NAME_LENGTH);

	if (strstr(m_confDisplayName, "_FORCE_H263_CONTENT") != NULL) // temporary code until the field will be supported by API (EMA + MCMS integration)
		m_PresentationProtocol = eH263Fix;

	GET_VALIDATE_CHILD(pResNode, "ENTERPRISE_PROTOCOL", &m_PresentationProtocol, PRESENTATION_PROTOCOL_ENUM);
	GET_VALIDATE_CHILD(pResNode, "CASCADE_OPTIMIZE_RESOLUTION", &m_cascadeOptimizeResolution, CASCADE_OPTIMIZE_RESOLUTION_ENUM);
	GET_VALIDATE_CHILD(pResNode, "HIGH_PROFILE_CONTENT", &m_isHighProfileContent, _BOOL);

	if (m_H243confName[0] == '\0' && m_confDisplayName[0] == '\0' && (nAction == AUTHENTICATED_START || nAction == ADD_RESERVE))
	{
		// in case both name empty re-test display name with 1 to 80 name length
		// to fail the test with right status
		GET_VALIDATE_CHILD(pResNode, "DISPLAY_NAME", m_confDisplayName, _1_TO_H243_NAME_LENGTH);
		//PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
  		if (nStatus != STATUS_OK)
		{
			TRACEWARN << "both DISPLAY_NAME and NAME are empty!!!!" ;
			return nStatus;
		}
	}

	GET_VALIDATE_CHILD(pResNode, "VIDEO_QUALITY", &nVal, VIDEO_QUALITY_ENUM);
	m_videoQuality = (eVideoQuality)nVal;

	GET_VALIDATE_CHILD(pResNode, "MAX_RESOLUTION", &m_confMaxResolution, RESOLUTION_SLIDER_ENUM);
	GET_VALIDATE_CHILD(pResNode, "IS_TELEPRESENCE_MODE", &m_isTelePresenceMode, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "AUDIO_ONLY_RECORDING", &m_IsAudioOnlyRecording, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "LPR", &m_isLpr, _BOOL);

	GET_CHILD_NODE(pResNode, "CONFERENCE_TEMPLATE", pChildNode);
	if (pChildNode)
	{
		pChildNode->ResetChildList();
		GET_VALIDATE_CHILD(pChildNode, "ON", &nVal, _BOOL);
		if (nVal)
			m_isConfTemplate = YES;
	}

	GET_VALIDATE_CHILD(pResNode, "HD_RESOLUTION", &nVal, HD_RESOLUTION_ENUM);
	m_HDResolution = (EHDResolution)nVal;

	GET_VALIDATE_CHILD(pResNode, "H264_VSW_HIGH_PROFILE_PREFERENCE", &m_H264VSWHighProfilePreference, H264_VSW_HIGH_PROFILE_PREFERENCE_ENUM);
	GET_VALIDATE_CHILD(pResNode, "VIDEO_CLARITY", &m_isVideoClarityEnabled, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "SITE_NAMES", &m_isSiteNamesEnabled, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "AUTO_REDIAL", &m_AutoRedial, _BOOL);
	BOOL exclusiveContentMode = FALSE;
	GET_VALIDATE_CHILD(pResNode, "EXCLUSIVE_CONTENT_MODE", &exclusiveContentMode, _BOOL); // guy
	SetExclusiveContentMode(exclusiveContentMode);
	GET_VALIDATE_CHILD(pResNode, "ENABLE_FECC", &m_FECC_Enabled, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "PERMANENT", &m_isPermanent, _BOOL);                     // Olga
	GET_VALIDATE_CHILD(pResNode, "TIP_COMPATIBILITY", &m_TipCompatibility, TIP_COMPATIBILITY_ENUM);
	//EE-631
	if(m_TipCompatibility == eTipCompatibleVideoOnly || m_TipCompatibility == eTipCompatibleVideoAndContent)
	{
		TRACEINTO << "fixing m_TipCompatibility to prefer TIP";
		m_TipCompatibility = eTipCompatiblePreferTIP;
	}
	GET_VALIDATE_CHILD(pResNode, "AV_MCU_CASCADE_MODE", &m_avMcuCascadeVideoMode, AV_MCU_CASCADE_MODE_ENUM);

	//eFeatureRssDialin
	GET_VALIDATE_CHILD(pResNode, "SEND_RECORDING_ANNOUNCEMENT", &m_IsEnableRecNotify, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "RECORDING_SERVER_PLAYBACK_LAYOUT_MODE", &m_SrsPlaybackLayout, SRS_PLAYBACK_LAYOUT_MODE_ENUM);

	GET_VALIDATE_CHILD(pResNode, "AV_MCU_FOCUS_URI", m_FocusUriScheduling, ONE_LINE_BUFFER_LENGTH);

	GET_VALIDATE_CHILD(pResNode, "NAT_KEEP_ALIVE_PERIOD", &m_natKeepAlivePeriod, _0_TO_MAXKAVALUE);
	GET_VALIDATE_CHILD(pResNode, "ECHO_SUPPRESSION", &m_EchoSuppression, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "KEYBOARD_SUPPRESSION", &m_KeyboardSuppression, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "AUTO_MUTE_NOISY_PARTIES", &m_isAutoMuteNoisyParties, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "APPOINTMENT_ID", m_appoitnmentID, _0_TO_APPOINTMENT_ID_LENGTH);

	GET_CHILD_NODE(pResNode, "GATHERING", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode, "ENABLE_GATHERING", &m_bGatheringEnabled, _BOOL);
		GET_VALIDATE_CHILD(pChildNode, "LANGUAGE", &nVal, LANGUAGES_ENUM);
		if (nVal < LAST_LANGUAGE)
			m_eLanguage = (ELanguges)nVal;

		GET_VALIDATE_CHILD(pChildNode, "IP_NUMBER_ACCESS", m_sIpNumberAccess, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pChildNode, "ACCESS_NUMBER_1", m_sNumberAccess_1, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pChildNode, "ACCESS_NUMBER_2", m_sNumberAccess_2, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pChildNode, "FREE_TEXT_1", m_sFreeText_1, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pChildNode, "FREE_TEXT_2", m_sFreeText_2, ONE_LINE_BUFFER_LENGTH);
		GET_VALIDATE_CHILD(pChildNode, "FREE_TEXT_3", m_sFreeText_3, ONE_LINE_BUFFER_LENGTH);
	}

	GET_VALIDATE_CHILD(pResNode, "GATEWAY", &m_isGateway, _BOOL);

	GET_CHILD_NODE(pResNode, "GW_DIAL_OUT_PROTOCOLS", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode, "H323", &nVal, _BOOL);
		SetIsGWDialOutToH323(nVal);
		GET_VALIDATE_CHILD(pChildNode, "SIP", &nVal, _BOOL);
		SetIsGWDialOutToSIP(nVal);
		GET_VALIDATE_CHILD(pChildNode, "H320", &nVal, _BOOL);
		SetIsGWDialOutToH320(nVal);
		GET_VALIDATE_CHILD(pChildNode, "PSTN", &nVal, _BOOL);
		SetIsGWDialOutToPSTN(nVal);
	}

	GET_VALIDATE_CHILD(pResNode, "CONTENT_TO_LEGACY_EPS", &m_ShowContentAsVideo, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "TELEPRESENCE_MODE_CONFIGURATION", &m_telePresenceModeConfiguration, TELEPRESENCE_MODE_CONFIGURATION_ENUM);
	GET_VALIDATE_CHILD(pResNode, "TELEPRESENCE_LAYOUT_MODE", &m_telePresenceLayoutMode, TELEPRESENCE_LAYOUT_MODE_ENUM);
	GET_VALIDATE_CHILD(pResNode, "CROPPING", &m_isCropping, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "IVR_PROVIDER_EQ", &m_ivrProviderEQ, _BOOL);

	// TELEPRESENCE_LAYOUTS
	if((ETelePresenceLayoutMode)m_telePresenceLayoutMode <= eTelePresenceLayoutCpParticipantsPriority)
		TRACEINTO << "TELEPRESENCE_LAYOUTS_DEBUG m_telePresenceLayoutMode = " << TelePresenceLayoutModeToString((ETelePresenceLayoutMode)m_telePresenceLayoutMode);
	else
		PASSERT(m_telePresenceLayoutMode);

	GET_VALIDATE_CHILD(pResNode, "CHINESE_FONT", &m_FontType, FONT_TYPES_ENUM);

	GET_CHILD_NODE(pResNode, "PARTY_LIST", pChildNode);
	if (pChildNode)
	{
		int nPartyAction = (nAction == AUTHENTICATED_START || nAction == ADD_RESERVE) ? NEW_PARTY : UPDATE_PARTY;
		nStatus = DeSerializePartyListXml(pChildNode, pszError, nPartyAction);
		PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
	}

	GET_VALIDATE_CHILD(pResNode, "AUTO_SCAN_INTERVAL", &m_AutoScanInterval, AUTO_SCAN_INTERVAL_ENUM);

	GET_CHILD_NODE(pResNode, "COP_CONFIGURATION_LIST", pChildNode);
	if (pChildNode)
	{
		nStatus = m_pCopConfigurationList->DeSerializeXml(pChildNode, pszError);
		PASSERT_AND_RETURN_VALUE(nStatus != STATUS_OK, nStatus);
	}

	GET_VALIDATE_CHILD(pResNode, "RES_STATUS", &m_ResSts, RESERVATION_STATUS_ENUM); // Status Field
	GET_VALIDATE_CHILD(pResNode, "AUDIO_CLARITY", &m_isAudioClarity, _BOOL);

	GET_VALIDATE_CHILD(pResNode, "SPEAKER_CHANGE_THRESHOLD", &m_speakerChangeThreshold, SPEAKER_CHANGE_THRESHOLD_ENUM);
	SetSpeakChangeParam(m_speakerChangeThreshold);

	GET_VALIDATE_CHILD(pResNode, "MUTE_PARTIES_IN_LECTURE", &m_Mute_Incoming_Parties_Lecture_Mode, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "MUTE_ALL_PARTIES_AUDIO_EXCEPT_LEADER", &m_muteAllPartiesAudioExceptLeader, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "MUTE_ALL_PARTIES_VIDEO_EXCEPT_LEADER", &m_muteAllPartiesVideoExceptLeader, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "AUTO_BRIGHTNESS", &m_isAutoBrightness, _BOOL);

	GET_CHILD_NODE(pResNode, "SERVICE_REGISTRATION_LIST", pChildNode);
	if (pChildNode)
	{
		int idx = 0;
		GET_FIRST_CHILD_NODE(pChildNode, "SERVICE_REGISTRATION_CONTENT", pServiceNode);
		while (pServiceNode)
		{
			GET_VALIDATE_CHILD(pServiceNode, "SERVICE_NAME", m_ServiceRegistrationContent[idx].service_name, ONE_LINE_BUFFER_LENGTH);
			if (m_ServiceRegistrationContent[idx].service_name[0] != '\0')
			{
				GET_VALIDATE_CHILD(pServiceNode, "SIP_REGISTRATION", &m_ServiceRegistrationContent[idx].sip_register, _BOOL);
				GET_VALIDATE_CHILD(pServiceNode, "ACCEPT_CALLS", &m_ServiceRegistrationContent[idx].accept_call, _BOOL);
				GET_VALIDATE_CHILD(pServiceNode, "SIP_REGISTRATION_STATUS", &m_ServiceRegistrationContent[idx].status, SIP_REG_STATUS_ENUM); // sipProxySts
			}

			idx++;
			GET_NEXT_CHILD_NODE(pChildNode, "SERVICE_REGISTRATION_CONTENT", pServiceNode);
		}
	}

	nVal = eConfMediaType_last;
	GET_VALIDATE_CHILD(pResNode, "SIP_REGISTRATIONS_STATUS", &m_SipRegistrationTotalSts, SIP_REG_TOTAL_STATUS_ENUM); // sipProxySts
	GET_VALIDATE_CHILD(pResNode, "CONF_MEDIA_TYPE", &nVal, CONF_MEDIA_TYPE_ENUM);
	if (nVal < eConfMediaType_last && nVal > 0)
		m_confMediaType = (eConfMediaType)nVal;
	nVal = eOPP_cif;
	GET_VALIDATE_CHILD(pResNode, "OPERATION_POINTS_PRESET", &nVal, OPERATION_POINTS_PRESET_ENUM);
	if (nVal > eOPP_dummy &&  nVal < eOPP_last )
		m_eOperationPointPreset = (EOperationPointPreset)nVal;

	//Content Transcoding
	GET_VALIDATE_CHILD(pResNode, "MULTIPLE_RESOLUTION", &m_ContentMultiResolutionEnabled, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "CONTENT_TRANSCODING_H264", &m_ContentXCodeH264Supported, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "CONTENT_TRANSCODING_H263", &m_ContentXCodeH263Supported, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "H264_CASCADE_OPTIMIZED", &m_isCascadeOptimized, _BOOL);
	GET_VALIDATE_CHILD(pResNode, "AS_SIP_CONTENT", &m_isAsSipContent, _BOOL);


	GET_CHILD_NODE(pResNode, "LAYOUT_INDICATIONS", pChildNode);
	if (pChildNode)
	{
		GET_VALIDATE_CHILD(pChildNode, "LAYOUT_INDICATIONS_POSITION", &m_IconDisplayPosition, ICON_DISPLAY_POSITION_ENUM);
		GET_VALIDATE_CHILD(pChildNode, "LAYOUT_INDICATIONS_AUDIO_PARTICIPANTS", &m_EnableAudioParticipantsIcon, _BOOL);
		GET_VALIDATE_CHILD(pChildNode, "LAYOUT_INDICATIONS_AUDIO_PARTICIPANTS_DISPLAY_MODE", &m_AudioParticipantsIconDisplayMode, ICON_DISPLAY_MODE_ENUM);
		GET_VALIDATE_CHILD(pChildNode, "LAYOUT_INDICATIONS_AUDIO_PARTICIPANTS_DISPLAY_DURATION", &m_AudioParticipantsIconDuration, _3_TO_300_DECIMAL);
		GET_VALIDATE_CHILD(pChildNode, "LAYOUT_SELF_INDICATIONS_NETWORK_QUALITY", &m_EnableSelfNetworkQualityIcon, _BOOL);
	}

	//If it is conference and the m_correlationId is not updated yet and m_confDisplayName is already updated and time is updated
	if ( IsTemplate() == FALSE && IsConfTemplate() == FALSE && m_correlationId[0] == '\0' && m_confDisplayName[0]!='\0' && m_startTime.m_year!=0)
	{
		SetCorrelationId();
		TRACEINTO << "Correlation Id: " << m_correlationId;
	}
	WORD mrcMcuId = 1;
	GET_VALIDATE_CHILD(pResNode, "MRC_MCU_ID", &mrcMcuId, _0_TO_WORD);
	SetMrcMcuId(mrcMcuId);
	GET_VALIDATE_CHILD(pResNode, "OVERRIDE_PROFILE_LAYOUT", &m_overideProfileLayout, _BOOL);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::SetVideoLayoutParams(CXMLDOMElement* pResNode, char* pszError)
{
	int             nStatus = STATUS_OK;
	CVideoLayout    VideoLayout;
	CXMLDOMElement* pChildNode = NULL;
	CXMLDOMElement* pForceNode = NULL;
	BYTE            nByteVal   = 0;

	GET_VALIDATE_CHILD(pResNode, "LAYOUT", &m_contPresScreenNumber, LAYOUT_ENUM);

	if (nStatus == STATUS_OK)
	{
		CVideoLayout* pVideoLayout = GetCurrentRsrvVidLayout(m_contPresScreenNumber);

		if (pVideoLayout)
		{
			VideoLayout = *pVideoLayout;
			VideoLayout.SetActive(YES);
			UpdateRsrvVidLayout(VideoLayout);
		}
		else
			AddNewVideoLayout(m_contPresScreenNumber, YES);

		CVideoLayout vLayout;
		int          nPos;

		pVideoLayout = GetFirstRsrvVidLayout(nPos);

		while (pVideoLayout)
		{
			if (pVideoLayout->GetScreenLayout() != m_contPresScreenNumber)
			{
				vLayout = *pVideoLayout;
				vLayout.SetActive(NO);
				UpdateRsrvVidLayout(vLayout);
			}

			pVideoLayout = GetNextRsrvVidLayout(nPos);
		}
	}

	GET_CHILD_NODE(pResNode, "FORCE_LIST", pChildNode);

	if (pChildNode)
	{
		GET_FIRST_CHILD_NODE(pChildNode, "FORCE", pForceNode);

		while (pForceNode)
		{
			GET_VALIDATE_CHILD(pForceNode, "LAYOUT", &nByteVal, LAYOUT_ENUM);
			CVideoLayout* pVideoLayout = GetCurrentRsrvVidLayout(nByteVal);

			if (pVideoLayout == NULL)
				AddNewVideoLayout(nByteVal, NO, &VideoLayout);
			else
				VideoLayout = *pVideoLayout;

			nStatus = VideoLayout.DeSerializeXml(pForceNode, pszError, FALSE);

			if (nStatus != STATUS_OK)
				return nStatus;

			UpdateRsrvVidLayout(VideoLayout);

			GET_NEXT_CHILD_NODE(pChildNode, "FORCE", pForceNode);
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCommResApi::Serialize(WORD format, CSegment& seg)
{
	if (format != NATIVE) PASSERT(1);

	COstrStream pOstr;
	Serialize(format, pOstr);
	pOstr.Serialize(seg);
}

//--------------------------------------------------------------------------
void CCommResApi::Serialize(WORD format, std::ostream& ostr)
{
// Note: When adding a new field to this function to be serialized, with format == CONFIG,
// don't forget to add it to function CCommResApi::SerializeToFile(COstrStream &m_ostr, DWORD apiNum) as well!!
	WORD removeFlag = 0;

	ostr << m_H243confName << "\n";
	ostr << m_confDisplayName << "\n";
	ostr << m_correlationId << "\n";
	ostr << m_confId   << "\n";

	m_startTime.Serialize(ostr);
	m_duration.Serialize(ostr);
	ostr << (WORD)m_stand_by   << "\n";
	ostr << (WORD)m_automaticTermination  << "\n";
	ostr << (WORD)m_confTransferRate  << "\n";
	ostr << (WORD)m_audioRate  << "\n";
	ostr << (WORD)m_video  << "\n";

	BYTE localVideoSession = m_videoSession;

	if (format != NATIVE)
	{
		if ((m_videoSession == CONTINUOUS_PRESENCE) && (m_SubCPtype == eSubCPtypeAdvanced))
		{
			localVideoSession = ADVANCED_LAYOUTS;
		}
	}

	ostr << (WORD)localVideoSession  << "\n";
	ostr << (WORD)m_contPresScreenNumber  << "\n";
	ostr << (WORD)m_videoPictureFormat  << "\n";
	ostr << (WORD)m_CIFframeRate  << "\n";
	ostr << (WORD)m_QCIFframeRate  << "\n";
	ostr << m_audioTone  << "\n";
	ostr << (WORD)m_alertToneTiming  << "\n";
	ostr << m_talkHoldTime  << "\n";
	ostr << (WORD)m_audioMixDepth  << "\n";
	ostr << (WORD)m_externalIvrControl << "\n";  // AT&T - should come before m_pAvMsgStruct->Serialize(format, ostr)!!
	if (m_externalIvrControl == NO)
		m_pAvMsgStruct->Serialize(format, ostr);

	WORD numTempParties = m_numParties;          // Local Number of parties in conference.
	if (m_numParties > MAX_PARTIES_IN_CONF)
		numTempParties = MAX_PARTIES_IN_CONF;

	if (m_dwConfFlags & SECURED)
		ostr <<  (WORD)0 << "\n";          // PARTIES_NUMBER
	else
		ostr <<  numTempParties << "\n";   // PARTIES_NUMBER

	if (!(m_dwConfFlags & SECURED))
	{
		for (int i = 0; i < (int)numTempParties; i++)
			m_pParty[i]->Serialize(format, ostr);
	}

	ostr << (WORD)m_videoProtocol<< "\n";
	ostr << (WORD)m_meetMePerConf<< "\n";
	ostr << m_numServicePhoneStr << "\n";
	for (int i = 0; i < (int)m_numServicePhoneStr; i++)
		m_pServicePhoneStr[i]->Serialize(format, ostr);

	ostr << m_H243_password  << "\n";
	ostr << (WORD)m_cascadeMode<< "\n";
	ostr << m_numUndefParties << "\n";

	m_pLectureMode->Serialize(format, ostr);

	ostr << m_repSchedulingId << "\n";
	ostr << (WORD)m_time_beforeFirstJoin << "\n";
	ostr << (WORD)m_time_afterLastQuit << "\n";
	ostr << (WORD)m_LastQuitType << "\n";
	ostr << (WORD)m_confLockFlag << "\n";
	ostr << m_max_parties << "\n";
	ostr <<  m_numRsrvVideoSource << "\n";

	for (int i = 0; i < (int)m_numRsrvVideoSource; i++)
		m_pRsrvVideoLayout[i]->Serialize(format, ostr);

	ostr << (WORD)m_isMeetingRoom << "\n";
	ostr << (WORD)m_meetingRoomReoccurrenceNum << "\n";
	ostr << (WORD)m_meetingRoomState << "\n";
	ostr << m_webReservUId << "\n";
	ostr << m_webOwnerUId << "\n";
	ostr << m_webDBId << "\n";
	ostr << (WORD)m_webReserved << "\n";
	ostr << m_partyIdCounter << "\n";
	ostr << m_unrsrvPartiesCounter << "\n";
	ostr << (WORD)m_startConfRequiresLeaderOnOff << "\n";
	ostr << (WORD)m_terminateConfAfterChairDropped <<"\n";
	ostr << m_entry_password << "\n";
	ostr << (WORD)m_isSameLayout << "\n";
	ostr << m_ipVideoRate << "\n";
	ostr << (WORD)m_media << "\n";
	ostr << (WORD)m_network << "\n";
	ostr << (WORD)m_confOnHoldFlag << "\n";
	ostr << (WORD)m_Mute_Incoming_Parties_Lecture_Mode << "\n";
	ostr << (WORD)m_muteAllPartiesAudioExceptLeader << "\n";
	ostr << (WORD)m_muteAllPartiesVideoExceptLeader << "\n";
	ostr << (WORD)m_FECC_Enabled << "\n";
	ostr << (WORD)m_meetMePerEntryQ << "\n";
	ostr << (WORD)m_IsEntryQ << "\n";
	ostr << (WORD)m_dualVideoMode << "\n";

	m_pVisualEffectsInfo->Serialize(format, ostr);

	ostr << (WORD)m_IsConfOnPort << "\n";
	ostr << (WORD)m_isAutoLayout << "\n";
	ostr << m_NumericConfId << "\n";

	m_pConfContactInfo->Serialize(format, ostr);
	m_BillingInfo.Serialize(format, ostr);
	ostr << (WORD)m_IsAdHoc << "\n";
	ostr << m_dwAdHocProfileId << "\n";
	ostr << m_baseProfileName << "\n";
	ostr << (WORD)m_isTemplate << "\n";
	ostr << m_dwFullUpdateCounter << "\n";
	ostr << (WORD)m_encryption << "\n";
	ostr << (WORD)m_eEncryptionType << "\n";
	ostr << (WORD)m_EnableRecording << "\n";
	ostr << (WORD)m_EnableRecordingIcon << "\n";
	ostr << (WORD)m_IsEnableRecNotify << "\n";
	ostr << (WORD)m_StartRecPolicy << "\n";
	ostr << (WORD)m_RecordingLinkControl << "\n";
	ostr << m_RecLinkName << "\n";
	ostr << (WORD)m_IsCascadeEQ << "\n";
	ostr << (WORD)m_isSIPFactory << "\n";
	ostr << (WORD)m_isAutoConnectSIPFactory << "\n";
	ostr << (WORD)m_HD << "\n";
	ostr << (WORD)m_isAdHocConf << "\n";
	ostr << (WORD)m_EnterpriseMode << "\n";

	if (m_EnterpriseMode == eCustomizedRate)
		ostr << (WORD)m_EnterpriseModeFixedRate << "\n";

	ostr << (WORD)m_PresentationProtocol << "\n";
	ostr << (WORD)m_cascadeOptimizeResolution << "\n";
	ostr << (WORD)m_isHighProfileContent << "\n";
	ostr << (WORD)m_videoQuality << "\n";
	ostr << (WORD)m_isTelePresenceMode << "\n";
	ostr << (WORD)m_IsAudioOnlyRecording << "\n";
	ostr << (WORD)m_HDResolution << "\n";
	ostr << (WORD)m_H264VSWHighProfilePreference << "\n";
	ostr << (WORD)m_isLpr << "\n";
	ostr << (WORD)m_isConfTemplate << "\n";
	ostr << (WORD)m_minNumAudioParties << "\n";
	ostr << (WORD)m_minNumVideoParties << "\n";
	ostr << m_ServiceNameForMinParties << "\n";
	ostr << (WORD)m_isGateway << "\n";
	ostr << (WORD)m_isVideoInviteGateway << "\n";
	ostr << (WORD)m_GWDialOutProtocols << "\n";
	ostr << (WORD)m_AutoScanInterval << "\n";
	ostr << (WORD)m_isVideoClarityEnabled << "\n";
	ostr << (WORD)m_ShowContentAsVideo << "\n";
	ostr << (WORD)m_EchoSuppression << "\n";
	ostr << (WORD)m_KeyboardSuppression << "\n";
	ostr << (WORD)m_isAutoMuteNoisyParties << "\n";
	ostr << (WORD)m_isPermanent << "\n";
	ostr << (WORD)m_confMaxResolution << "\n";
	ostr << (WORD)m_AutoRedial << "\n";
	ostr << (WORD)m_bIsExclusiveContentMode << "\n";
	ostr << (WORD)m_operatorConf << "\n";

	WORD add_operator_conf_info = 0;

	if (m_pOperatorConfInfo != NULL)
	{
		add_operator_conf_info = 1;
		ostr << add_operator_conf_info << "\n";
		m_pOperatorConfInfo->Serialize(format, ostr);
	}
	else
	{
		ostr << add_operator_conf_info << "\n";
	}

	ostr << m_appoitnmentID << "\n";
	ostr << m_meetingOrganizer << "\n";
	ostr << (WORD)m_isStreaming << "\n";
	ostr << (WORD)m_bGatheringEnabled << "\n";
	ostr << (WORD)m_eLanguage << "\n";
	ostr << m_sIpNumberAccess << "\n";
	ostr << m_sNumberAccess_1 << "\n";
	ostr << m_sNumberAccess_2 << "\n";
	ostr << m_sFreeText_1 << "\n";
	ostr << m_sFreeText_2 << "\n";
	ostr << m_sFreeText_3 << "\n";
	ostr << m_FocusUriScheduling << "\n";
	ostr << m_FocusUriCurrently << "\n";
	m_dtExchangeConfStartTime.Serialize(ostr);

	m_pMessageOverlayInfo->Serialize(format, ostr);
	m_pSiteNameInfo->Serialize(format, ostr);
	m_pCopConfigurationList->Serialize(format, ostr);
	ostr << (WORD)m_telePresenceModeConfiguration << "\n";
	ostr << (WORD)m_telePresenceLayoutMode << "\n";
	ostr << (WORD)m_isCropping << "\n";
	ostr << (WORD)m_ResSts << "\n"; // Status Field
	ostr << (WORD)m_isAudioClarity << "\n";
	ostr << (WORD)m_isAutoBrightness << "\n";
	ostr << (WORD)m_ivrProviderEQ << "\n";
	ostr << (WORD)m_isSiteNamesEnabled << "\n";

	for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
	{
		ostr << m_ServiceRegistrationContent[i].service_name << "\n";
		ostr << (WORD)m_ServiceRegistrationContent[i].sip_register << "\n";
		ostr << (WORD)m_ServiceRegistrationContent[i].accept_call << "\n";
		ostr << (WORD)m_ServiceRegistrationContent[i].status << "\n"; // sipProxySts
	}

	ostr << (WORD)m_SipRegistrationTotalSts << "\n"; // sipProxySts
	ostr << (WORD)m_TipCompatibility << "\n";
	ostr << (WORD)m_avMcuCascadeVideoMode << "\n";

	ostr << (DWORD)m_natKeepAlivePeriod << "\n";

	ostr << (WORD)m_FontType <<'\n';
	ostr << (WORD)m_speakerChangeThreshold << "\n";
	ostr << (WORD)m_confSpeakerChangeMode << "\n";
	ostr << (WORD)m_confMediaType << "\n";
	ostr << (WORD)m_manageTelepresenceLayoutInternaly << "\n";
	ostr << (WORD)m_eOperationPointPreset << "\n";

	//Content Transcoding
	ostr << (WORD)m_ContentMultiResolutionEnabled << "\n";
	ostr << (WORD)m_ContentXCodeH264Supported << "\n";
	ostr << (WORD)m_ContentXCodeH263Supported << "\n";
	ostr << (WORD)m_isCascadeOptimized << "\n";
	ostr << (WORD)m_isAsSipContent << "\n";
	ostr << m_mrcMcuId << "\n";
	ostr << (WORD)m_SrsPlaybackLayout << "\n";

	//Indication for audio participants
	ostr << m_IconDisplayPosition << "\n";
	ostr << (WORD)m_EnableAudioParticipantsIcon << "\n";
	ostr << (WORD)m_AudioParticipantsIconDisplayMode << "\n";
	ostr << (WORD)m_AudioParticipantsIconDuration << "\n";
	ostr << (WORD)m_EnableSelfNetworkQualityIcon << "\n";
	ostr << (WORD)m_overideProfileLayout << "\n";
}

//--------------------------------------------------------------------------
void CCommResApi::DeSerialize(WORD format, CSegment& seg)
{
	if (format != NATIVE) PASSERT(1);

	CIstrStream istr(seg);
	DeSerialize(format, istr);
}

//--------------------------------------------------------------------------
void CCommResApi::DeSerialize(WORD format, std::istream& istr)
{
	int i;

	istr.getline(m_H243confName, H243_NAME_LEN);
	istr.getline(m_confDisplayName, H243_NAME_LEN);
	istr.getline(m_correlationId, CORRELATION_ID_LENGTH);

	istr >> m_confId;

	m_startTime.DeSerialize(istr);
	m_duration.DeSerialize(istr);

	SetEndTime();

	WORD tmp;
	WORD tmp3;
	istr >> tmp;
	m_stand_by = (BYTE)tmp;
	istr >> tmp;
	m_automaticTermination = (BYTE)tmp;
	istr >> tmp;
	m_confTransferRate = (BYTE)tmp;
	istr >> tmp;
	m_audioRate = (BYTE)tmp;
	istr >> tmp;
	m_video = (BYTE)tmp;
	istr >> tmp;
	m_videoSession = (BYTE)tmp;

	istr >> tmp;
	m_contPresScreenNumber = (BYTE)tmp;

	istr >> tmp;
	m_videoPictureFormat = (BYTE)tmp;
	istr >> tmp;
	m_CIFframeRate = (BYTE)tmp;
	istr >> tmp;
	m_QCIFframeRate = (BYTE)tmp;

	istr >> m_audioTone;
	istr >> tmp;
	m_alertToneTiming = (BYTE)tmp;
	istr >> m_talkHoldTime;
	istr >> tmp;
	m_audioMixDepth = (BYTE)tmp;
	istr >> tmp;
	m_externalIvrControl = (BYTE)tmp; // AT&T - should come before m_pAvMsgStruct->DeSerialize(format, istr)!!
	if (m_externalIvrControl == NO)
		m_pAvMsgStruct->DeSerialize(format, istr);

	istr >> m_numParties;
	istr.ignore(1);
	for (i = 0; i < (int)m_numParties; i++)
	{
		m_pParty[i] = new CRsrvParty;
		m_pParty[i]->DeSerialize(format, istr);
		m_pParty[i]->SetRes(this);
	}

	istr >> tmp;
	m_videoProtocol = (BYTE)tmp;
	istr >> tmp;
	m_meetMePerConf = (BYTE)tmp;

	istr >> m_numServicePhoneStr;
	istr.ignore(1);
	for (i = 0; i < (int)m_numServicePhoneStr; i++)
	{
		m_pServicePhoneStr[i] = new CServicePhoneStr;
		m_pServicePhoneStr[i]->DeSerialize(format, istr);
	}

	istr.getline(m_H243_password, H243_NAME_LEN+1, '\n');
	istr >> tmp;
	m_cascadeMode = (BYTE)tmp;
	istr >> m_numUndefParties;

	m_pLectureMode->DeSerialize(format, istr);
	istr >> m_repSchedulingId;
	istr >> tmp;
	m_time_beforeFirstJoin = (BYTE)tmp;

	istr >> tmp;
	m_time_afterLastQuit = (BYTE)tmp;

	istr >> tmp;
	m_LastQuitType = (BYTE)tmp;

	istr >> tmp;
	m_confLockFlag = (BYTE)tmp;
	istr >> m_max_parties;
	istr >> m_numRsrvVideoSource;
	for (i = 0; i < (int)m_numRsrvVideoSource; i++)
	{
		if (m_pRsrvVideoLayout[i] != NULL)
		{
			PDELETE(m_pRsrvVideoLayout[i]);
		}

		m_pRsrvVideoLayout[i] = new CVideoLayout;
		m_pRsrvVideoLayout[i]->DeSerialize(format, istr);
	}

	istr >> tmp;
	m_isMeetingRoom = (BYTE)tmp;
	istr >> tmp;
	m_meetingRoomReoccurrenceNum = (BYTE)tmp;

	istr >> tmp;
	m_meetingRoomState = (BYTE)tmp;
	istr >> m_webReservUId;
	istr >> m_webOwnerUId;
	istr >> m_webDBId;
	istr >> tmp;
	m_webReserved = (BYTE)tmp;
	istr >> m_partyIdCounter;
	istr >> m_unrsrvPartiesCounter;
	istr >> tmp;
	m_startConfRequiresLeaderOnOff = (BYTE)tmp;
	istr >> tmp;
	m_terminateConfAfterChairDropped = (BYTE)tmp;
	istr.ignore(1);
	istr.getline(m_entry_password, CONFERENCE_ENTRY_PASSWORD_LEN+1, '\n');
	istr >> tmp;
	m_isSameLayout = (BYTE)tmp;
	istr >> m_ipVideoRate;
	istr >> tmp;
	m_media = (BYTE)tmp;
	istr >> tmp;
	m_network = (BYTE)tmp;

	istr >> tmp;
	m_confOnHoldFlag = (BYTE)tmp;

	istr >> tmp;
	m_Mute_Incoming_Parties_Lecture_Mode = (BYTE)tmp;
	istr >> tmp;
	m_muteAllPartiesAudioExceptLeader = (BYTE)tmp;
	istr >> tmp;
	m_muteAllPartiesVideoExceptLeader = (BYTE)tmp;

	istr >> tmp;
	m_FECC_Enabled = (BYTE)tmp;
	istr >> tmp;
	m_meetMePerEntryQ = (BYTE)tmp;
	istr >> tmp;
	m_IsEntryQ = (BYTE)tmp;

	istr >> tmp;
	m_dualVideoMode = (BYTE)tmp;

	m_pVisualEffectsInfo->DeSerialize(format, istr);

	istr >> tmp;
	m_IsConfOnPort = (BYTE)tmp;

	istr >> tmp;
	m_isAutoLayout = (BYTE)tmp;
	istr.ignore(1);
	istr.getline(m_NumericConfId, NUMERIC_CONFERENCE_ID_LEN+1, '\n');

	m_pConfContactInfo->DeSerialize(format, istr);
	m_BillingInfo.DeSerialize(format, istr);
	istr >> tmp;
	m_IsAdHoc = (BYTE)tmp;
	istr >> m_dwAdHocProfileId;
	istr.ignore(1);
	istr.getline(m_baseProfileName, H243_NAME_LEN+1, '\n');

	istr >> tmp;
	m_isTemplate = (BYTE)tmp;
	istr >> m_dwFullUpdateCounter;
	BYTE xmlEncryptionFlag, xmlEncryptionType;
	xmlEncryptionFlag = NO;
	istr >> tmp;
	xmlEncryptionFlag = (BYTE)tmp;
	istr >> tmp;
	xmlEncryptionType = (BYTE)tmp;
	SetEncryptionParameters(xmlEncryptionFlag, xmlEncryptionType);
	istr >> tmp;
	m_EnableRecording = (BYTE)tmp;

	istr >> tmp;
	m_EnableRecordingIcon = (BYTE)tmp;

	istr >> tmp;
	m_IsEnableRecNotify = (BYTE)tmp;

	istr >> tmp;
	m_StartRecPolicy = (BYTE)tmp;
	istr >> tmp;
	m_RecordingLinkControl = (BYTE)tmp;

	istr.ignore(1);
	istr.getline(m_RecLinkName, H243_NAME_LEN + 1, '\n');

	istr >> tmp;
	m_IsCascadeEQ = (BYTE)tmp;

	istr >> tmp;
	m_isSIPFactory = (BYTE)tmp;
	istr >> tmp;
	m_isAutoConnectSIPFactory = (BYTE)tmp;

	istr >> tmp;
	m_HD = (BYTE)tmp;

	istr >> tmp;
	m_isAdHocConf = (BYTE)tmp;

	istr >> tmp;
	m_EnterpriseMode = (BYTE)tmp;

	if (m_EnterpriseMode == eCustomizedRate)
	{
		istr >> tmp;
		m_EnterpriseModeFixedRate = (BYTE)tmp;
	}

	istr >> tmp;
	m_PresentationProtocol = (BYTE)tmp;


	istr >> tmp;
	m_cascadeOptimizeResolution = (BYTE)tmp;

	istr >> tmp;
	m_isHighProfileContent = (BYTE)tmp;

	istr >> tmp;
	m_videoQuality = (eVideoQuality)tmp;

	istr >> tmp;
	m_isTelePresenceMode = (BYTE)tmp;

	istr >> tmp;
	m_IsAudioOnlyRecording = (BYTE)tmp;

	istr >> tmp;
	m_HDResolution = (EHDResolution)tmp;

	istr >> tmp;
	m_H264VSWHighProfilePreference = (BYTE)tmp;

	istr >> tmp;
	m_isLpr = (BYTE)tmp;
	istr >> tmp;
	m_isConfTemplate = (BYTE)tmp;
	istr >> tmp;
	m_minNumAudioParties = tmp;
	istr >> tmp;
	m_minNumVideoParties = tmp;
	istr.ignore(1);
	istr.getline(m_ServiceNameForMinParties, ONE_LINE_BUFFER_LEN, '\n');

	istr >> tmp;
	m_isGateway = (BYTE)tmp;
	istr >> tmp;
	m_isVideoInviteGateway = (BYTE)tmp;
	istr >> tmp;
	m_GWDialOutProtocols = tmp;
	istr >> tmp;
	m_AutoScanInterval = tmp;
	istr >> tmp;
	m_isVideoClarityEnabled = (BYTE)tmp;
	istr >> tmp;
	m_ShowContentAsVideo = (BYTE)tmp;
	istr >> tmp;
	m_EchoSuppression = (BYTE)tmp;
	istr >> tmp;
	m_KeyboardSuppression = (BYTE)tmp;
	istr >> tmp;
	m_isAutoMuteNoisyParties = (BYTE)tmp;

	istr >> tmp;
	m_isPermanent = (BYTE)tmp;

	istr >> tmp;
	m_confMaxResolution = (BYTE)tmp;  // olga

	istr >> tmp;
	m_AutoRedial = (BYTE)tmp;

	istr >> tmp;
	SetExclusiveContentMode((BOOL)tmp != 0);

	istr >> tmp;
	m_operatorConf = (BYTE)tmp;
	istr.ignore(1);

	WORD add_operator_conf_info = 0;
	istr >> add_operator_conf_info;
	istr.ignore(1);

	if (add_operator_conf_info)
	{
		if (m_pOperatorConfInfo == NULL)
			m_pOperatorConfInfo = new COperatorConfInfo();

		m_pOperatorConfInfo->DeSerialize(format, istr);
	}

	istr.getline(m_appoitnmentID, APPOITNMENT_ID_LEN + 1, '\n');
	istr.getline(m_meetingOrganizer, H243_NAME_LEN + 1, '\n');
	istr >> tmp;
	m_isStreaming = (BYTE)tmp;

// Gathering
	istr >> tmp;
	m_bGatheringEnabled = (BYTE)tmp;
	istr >> tmp;
	m_eLanguage = (ELanguges)tmp;
	istr.ignore(1);
	istr.getline(m_sIpNumberAccess, ONE_LINE_BUFFER_LEN, '\n');
	istr.getline(m_sNumberAccess_1, ONE_LINE_BUFFER_LEN, '\n');
	istr.getline(m_sNumberAccess_2, ONE_LINE_BUFFER_LEN, '\n');
	istr.getline(m_sFreeText_1, ONE_LINE_BUFFER_LEN, '\n');
	istr.getline(m_sFreeText_2, ONE_LINE_BUFFER_LEN, '\n');
	istr.getline(m_sFreeText_3, ONE_LINE_BUFFER_LEN, '\n');
	istr.getline(m_FocusUriScheduling, ONE_LINE_BUFFER_LEN, '\n');
	istr.getline(m_FocusUriCurrently, ONE_LINE_BUFFER_LEN, '\n');
	m_dtExchangeConfStartTime.DeSerialize(istr);

	m_pMessageOverlayInfo->DeSerialize(format, istr);
	m_pSiteNameInfo->DeSerialize(format, istr);
	m_pCopConfigurationList->DeSerialize(format, istr);

	istr >> tmp;
	m_telePresenceModeConfiguration = (BYTE)tmp;
	istr >> tmp;
	m_telePresenceLayoutMode = (BYTE)tmp;
	istr >> tmp;
	m_isCropping = (BYTE)tmp;

	istr >> tmp;
	m_ResSts = (BYTE)tmp; // Status Field
	istr >> tmp;
	m_isAudioClarity = (BYTE)tmp;
	istr >> tmp;
	m_isAutoBrightness = (BYTE)tmp;
	istr >> tmp;
	m_ivrProviderEQ = (BYTE)tmp;
	istr >> tmp;
	m_isSiteNamesEnabled = (BYTE)tmp;


	if (istr.eof())
	{
		for (int i = 0; i < NUM_OF_IP_SERVICES; i++) // for Multiple services fix it
		{
			istr.ignore(1);
			m_ServiceRegistrationContent[i].service_name[0] = '\0';
			m_ServiceRegistrationContent[i].sip_register    = 0;
			m_ServiceRegistrationContent[i].accept_call     = 1;
			m_ServiceRegistrationContent[i].status          = eSipRegistrationStatusTypeNotConfigured; // sipProxySts
		}
	}
	else
	{
		for (int i = 0; i < NUM_OF_IP_SERVICES; i++) // for Multiple services fix it
		{
			istr.ignore(1);
			istr.getline(m_ServiceRegistrationContent[i].service_name, ONE_LINE_BUFFER_LEN, '\n');
			istr >> tmp;
			m_ServiceRegistrationContent[i].sip_register = (BYTE)tmp;
			istr >> tmp;
			m_ServiceRegistrationContent[i].accept_call = (BYTE)tmp;
			istr >> tmp; // sipProxySts
			m_ServiceRegistrationContent[i].status = (BYTE)tmp;
		}
	}

	istr >> tmp; // sipProxySts
	m_SipRegistrationTotalSts = (BYTE)tmp;
	istr >> tmp;
	m_TipCompatibility = (BYTE)tmp;
	//EE-631
	if(m_TipCompatibility == eTipCompatibleVideoOnly || m_TipCompatibility == eTipCompatibleVideoAndContent)
	{
		TRACEINTO << "fixing m_TipCompatibility to prefer TIP";
		m_TipCompatibility = (BYTE)eTipCompatiblePreferTIP;
	}

	istr >> tmp3;
	m_avMcuCascadeVideoMode = (BYTE)tmp3;

	TRACEINTO<< m_avMcuCascadeVideoMode;
	DWORD tmp2=0;
	istr >> tmp2;
	m_natKeepAlivePeriod = (DWORD)tmp2;
	istr >> tmp;
	m_FontType = (BYTE)tmp;
	istr >>tmp;
	m_speakerChangeThreshold = (BYTE)tmp;
	istr >>tmp;
	m_confSpeakerChangeMode = (WORD)tmp;

	istr >> tmp;
	m_confMediaType = (eConfMediaType)tmp;
	istr >> tmp;
	m_manageTelepresenceLayoutInternaly = (BYTE)tmp;
	istr >> tmp;
	m_eOperationPointPreset = (EOperationPointPreset)tmp;
	//Content Transcoding
	istr >> tmp;
	m_ContentMultiResolutionEnabled = (BYTE)tmp;
	istr >> tmp;
	m_ContentXCodeH264Supported = (BYTE)tmp;
	istr >> tmp;
	m_ContentXCodeH263Supported = (BYTE)tmp;
	istr >> tmp;
	m_isCascadeOptimized = (BYTE)tmp;
	istr >> tmp;
	m_isAsSipContent = (BYTE)tmp;

	WORD mrcMcuId = 1;
	istr >> mrcMcuId;
	SetMrcMcuId(mrcMcuId);

	istr >> tmp;
	m_SrsPlaybackLayout = (BYTE)tmp;

	//Indication for audio participants
	istr >> tmp;
	m_IconDisplayPosition = tmp;
	istr >> tmp;
	m_EnableAudioParticipantsIcon = (BYTE)tmp;
	istr >> tmp;
	m_AudioParticipantsIconDisplayMode = (BYTE)tmp;
	istr >> tmp;
	m_AudioParticipantsIconDuration = tmp;
	istr >> tmp;
	m_EnableSelfNetworkQualityIcon = (BYTE)tmp;
	istr >> tmp;
	m_overideProfileLayout = (BYTE)tmp;
}

//--------------------------------------------------------------------------
int CCommResApi::Add(const CRsrvParty& other)
{
	if (m_numParties >= MAX_PARTIES_IN_CONF)
		return STATUS_MAX_PARTIES_EXCEEDED;

	if (FindName(other) != NOT_FIND)
		return STATUS_PARTY_NAME_EXISTS;

	m_pParty[m_numParties] = new CRsrvParty(other);

	m_pParty[m_numParties]->SetRes(this);

	m_numParties++;
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::FindName(const CRsrvParty& other)
{
	for (int i = 0; i < (int)m_numParties; i++)
		if (!strncmp(m_pParty[i]->GetName(), other.GetName(), H243_NAME_LEN))
			return i;

	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CCommResApi::FindName(const char* name)
{
	for (int i = 0; i < (int)m_numParties; i++)
		if (!strncmp(m_pParty[i]->GetName(), name, H243_NAME_LEN))
			return i;

	return NOT_FIND;
}

//--------------------------------------------------------------------------
void CCommResApi::SetEndTime(const CStructTm& other)
{
	m_endTime = other;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetMessageOverlay(CMessageOverlayInfo* pMessageOverlay)
{
	if (m_pMessageOverlayInfo == NULL)
		m_pMessageOverlayInfo = new CMessageOverlayInfo();

	*m_pMessageOverlayInfo = *pMessageOverlay;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetSiteName(CSiteNameInfo* pSiteName)
{
	if (m_pSiteNameInfo == NULL)
		m_pSiteNameInfo = new CSiteNameInfo();

	*m_pSiteNameInfo = *pSiteName;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetAutoScanOrder(CAutoScanOrder* pAutoScanOrder)
{
	if (m_pAutoScanOrder == NULL)
		m_pAutoScanOrder = new CAutoScanOrder();

	*m_pAutoScanOrder = *pAutoScanOrder;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetStartTime(const CStructTm& other)
{
	m_startTime = other;
}

//--------------------------------------------------------------------------
void CCommResApi::SetEndTime()
{
	CStructTm endTime = m_startTime + m_duration;
	SetEndTime(endTime);
}

//--------------------------------------------------------------------------
DWORD CCommResApi::ExtendDurationIfNeed(BOOL IsEnableAutoExtension, DWORD ExtensionTimeInterval)
{
	// set timer for conference duration
	const CStructTm* pEndTime = GetEndTime();
	DWORD durationTime = *pEndTime - m_startTime;

	if (IsEnableAutoExtension)
	{
		if (durationTime <= ExtensionTimeInterval*60)
		{
			TranslateExtensionTimeInterval((durationTime/60) + ExtensionTimeInterval);
			SetEndTime();
			durationTime = *pEndTime - m_startTime;
		}
	}

	return durationTime;
}

//--------------------------------------------------------------------------
void CCommResApi::TranslateExtensionTimeInterval(DWORD ExtensionTimeInterval)
{
	m_duration.m_day  = 0;
	m_duration.m_mon  = 0;
	m_duration.m_year = 0;
	m_duration.m_hour = ExtensionTimeInterval / 60;
	m_duration.m_min  = ExtensionTimeInterval % 60;
}

//--------------------------------------------------------------------------
const CStructTm* CCommResApi::GetEndTime()
{
	CStructTm endTime = m_startTime + m_duration;
	m_endTime = endTime;
	return &m_endTime;
}

//--------------------------------------------------------------------------
const CStructTm* CCommResApi::GetDuration() const
{
	return &m_duration;
}

//--------------------------------------------------------------------------
void CCommResApi::SetDuration(const CStructTm& duration)
{
	m_duration = duration;
	SetEndTime();
}

//--------------------------------------------------------------------------
WORD CCommResApi::IsRollCall() const
{
	if ((m_audioTone & 0x00000008) == 8)
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetRollCall(WORD bl)
{
	if (bl == FALSE)
		m_audioTone &= 0xFFFFFFF7;
	else
		m_audioTone |= 0x00000008;

	UPDATECONFLAGSNEGATIV(bl, NO, ROLL_CALL_FLAG);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
int CCommResApi::AddRsrvVidLayout(const CVideoLayout& other)
{
	PASSERTMSG_AND_RETURN_VALUE(m_numRsrvVideoSource >= MAX_VIDEO_LAYOUT_NUMBER, "Failed, maximum video layouts exceeded", STATUS_MAX_VIDEO_LAYOUTS_EXCEEDED);

	if (FindRsrvVidLayout(other) != NOT_FIND)
		return STATUS_VIDEO_SOURCE_EXISTS;

	POBJDELETE(m_pRsrvVideoLayout[m_numRsrvVideoSource]);
	m_pRsrvVideoLayout[m_numRsrvVideoSource] = new CVideoLayout(other);

	m_numRsrvVideoSource++;
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::UpdateRsrvVidLayout(const CVideoLayout& other)
{
	PASSERTMSG_AND_RETURN_VALUE(m_numRsrvVideoSource > MAX_VIDEO_LAYOUT_NUMBER, "Failed, maximum video layouts exceeded", STATUS_MAX_VIDEO_LAYOUTS_EXCEEDED);

	int ind = FindRsrvVidLayout(other);
	if (ind == NOT_FIND)
		return STATUS_VIDEO_SOURCE_NOT_EXISTS;

	POBJDELETE(m_pRsrvVideoLayout[ind]);
	m_pRsrvVideoLayout[ind] = new CVideoLayout(other);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::CancelRsrvVidLayout(BYTE screenLayout)
{
	int ind = FindRsrvVidLayout(screenLayout);
	if (ind == NOT_FIND)
		return STATUS_VIDEO_SOURCE_NOT_EXISTS;

	if (ind > 0 && ind < MAX_VIDEO_LAYOUT_NUMBER)
		POBJDELETE(m_pRsrvVideoLayout[ind]);

	WORD i;
	for (i = 0; i < m_numRsrvVideoSource && i < MAX_VIDEO_LAYOUT_NUMBER; i++)
		if (m_pRsrvVideoLayout[i] == NULL)
			break;

	for (WORD j = i; j < m_numRsrvVideoSource - 1 && j < MAX_VIDEO_LAYOUT_NUMBER-1; j++)
		m_pRsrvVideoLayout[j] = m_pRsrvVideoLayout[j + 1];
	if( m_numRsrvVideoSource <= MAX_VIDEO_LAYOUT_NUMBER)
	{
		m_pRsrvVideoLayout[m_numRsrvVideoSource - 1] = NULL;
		--m_numRsrvVideoSource;
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::FindRsrvVidLayout(const CVideoLayout& other)
{
	for (int i = 0; i < (int)m_numRsrvVideoSource; i++)
	{
		if (m_pRsrvVideoLayout[i] != NULL)
		{
			if (m_pRsrvVideoLayout[i]->GetScreenLayout() == other.GetScreenLayout())
				return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
int CCommResApi::FindRsrvVidLayout(BYTE screenLayout)
{
	for (int i = 0; i < (int)m_numRsrvVideoSource; i++)
	{
		if (m_pRsrvVideoLayout[i] != NULL)
		{
			if (m_pRsrvVideoLayout[i]->GetScreenLayout() == screenLayout)
				return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
CVideoLayout* CCommResApi::GetFirstRsrvVidLayout()
{
	m_ind_rsrv_vid_layout = 1;
	return m_pRsrvVideoLayout[0];
}

//--------------------------------------------------------------------------
CVideoLayout* CCommResApi::GetNextRsrvVidLayout()
{
	PASSERTMSG_AND_RETURN_VALUE(m_numRsrvVideoSource > MAX_VIDEO_LAYOUT_NUMBER, "Failed, maximum video layouts exceeded", NULL);

	if (m_ind_rsrv_vid_layout >= m_numRsrvVideoSource) return NULL;

	return m_pRsrvVideoLayout[m_ind_rsrv_vid_layout++];
}

//--------------------------------------------------------------------------
CVideoLayout* CCommResApi::GetFirstRsrvVidLayout(int& nPos)
{
	CVideoLayout* pRsrvVideoLayout = GetFirstRsrvVidLayout();
	nPos = m_ind_rsrv_vid_layout;
	return pRsrvVideoLayout;
}

//--------------------------------------------------------------------------
CVideoLayout* CCommResApi::GetNextRsrvVidLayout(int& nPos)
{
	m_ind_rsrv_vid_layout = nPos;
	CVideoLayout* pRsrvVideoLayout = GetNextRsrvVidLayout();
	nPos = m_ind_rsrv_vid_layout;
	return pRsrvVideoLayout;
}

//--------------------------------------------------------------------------
CVideoLayout* CCommResApi::GetCurrentRsrvVidLayout(BYTE screenLayout)
{
	int ind = FindRsrvVidLayout(screenLayout);
	if (ind == NOT_FIND) return NULL;

	PASSERTMSG_AND_RETURN_VALUE(ind >= MAX_VIDEO_LAYOUT_NUMBER, "Failed, index exceed maximum video layouts", NULL);

	return m_pRsrvVideoLayout[ind];
}

//--------------------------------------------------------------------------
CVideoLayout* CCommResApi::GetCurrentRsrvVidLayout()
{
	int ind = FindRsrvVidLayout(m_contPresScreenNumber);
	if (ind == NOT_FIND) return NULL;

	PASSERTMSG_AND_RETURN_VALUE(ind >= MAX_VIDEO_LAYOUT_NUMBER, "Failed, index exceed maximum video layouts", NULL);

	return m_pRsrvVideoLayout[ind];
}

//--------------------------------------------------------------------------
CVideoLayout* CCommResApi::GetActiveRsrvVidLayout()
{
	for (WORD i = 0; i < (WORD)m_numRsrvVideoSource; i++)
		if (m_pRsrvVideoLayout[i]->IsActive())
			return m_pRsrvVideoLayout[i];

	return NULL;
}

//--------------------------------------------------------------------------
BYTE CCommResApi::IsAudioConf() const
{
	return (AUDIO_MEDIA == m_media);
}

//--------------------------------------------------------------------------
void CCommResApi::IncreaseFullUpdateCounter()
{
	m_dwFullUpdateCounter++;
}

//--------------------------------------------------------------------------
DWORD CCommResApi::CalcRsrvFlags()
{
	DWORD localFlag = 0x0;

	UPDATERESFLAGSNEGATIV(m_meetMePerConf, NO, ACTIVE_PERMANEMT, localFlag);
	UPDATERESFLAGSNEGATIV(m_isMeetingRoom, NO, ACTIVE_PERMANEMT, localFlag);
	UPDATERESFLAGSNEGATIV(IsAudioConf(), NO, AUDIO_ONLY, localFlag);
	UPDATERESFLAGSNEGATIV(m_confLockFlag, NO, LOCKED, localFlag);
	UPDATERESFLAGSPOSITIV(m_network, NETWORK_H323, H323_ONLY, localFlag);
	UPDATERESFLAGSNEGATIV((m_pLectureMode->GetLectureModeType()), NO, LECTURE_MODE, localFlag);
	UPDATERESFLAGSPOSITIV((m_pLectureMode->GetLectureModeType()), LECTURE_SHOW, LECTURE_MODE_SHOW, localFlag);
	UPDATERESFLAGSNEGATIV(m_cascadeMode, CASCADE_MODE_NONE, CASCADING, localFlag);
	UPDATERESFLAGSPOSITIV(m_cascadeMode, CASCADE_MODE_AUTO, CASCADING_AUTO, localFlag);
	UPDATERESFLAGSNEGATIV(m_confOnHoldFlag, NO, ON_HOLD, localFlag);
	UPDATERESFLAGSNEGATIV(m_IsEntryQ, NO, ENTRY_QUEUE, localFlag);
	UPDATERESFLAGSNEGATIV(m_confSecureFlag, NO, SECURED, localFlag);
	UPDATERESFLAGSNEGATIV((m_audioTone && 0x8), NO, ROLL_CALL_FLAG, localFlag);
	UPDATERESFLAGSNEGATIV(m_isAutoLayout, NO, AUTO_LAYOUT_FLAG, localFlag);
	UPDATERESFLAGSPOSITIV((m_pLectureMode->GetLectureModeType()), PRESENTATION_LECTURE_MODE, LECTURE_MODE_PRESENTATION, localFlag);
	UPDATERESFLAGSNEGATIV(m_encryption, NO, ENCRYPTION, localFlag);
	UPDATERESFLAGSNEGATIV(m_isSIPFactory, NO, SIP_FACTORY, localFlag);
	UPDATERESFLAGSNEGATIV(m_isTelePresenceMode, NO, TELEPRESENCE_MODE, localFlag);
	UPDATERESFLAGSNEGATIV(m_isVideoClarityEnabled, NO, VIDEO_CLARITY_FLAG, localFlag);
	UPDATERESFLAGSNEGATIV(m_ShowContentAsVideo, NO, VIDEO_AS_CONTENT_FLAG, localFlag);
	UPDATERESFLAGSNEGATIV(m_isGateway, NO, RMX_GATEWAY, localFlag);
	return localFlag;
}

//--------------------------------------------------------------------------
DWORD CCommResApi::CalcRsrvFlags2()
{
    DWORD localFlag=0x0;
	return localFlag;
}

//--------------------------------------------------------------------------
void CCommResApi::AddNewVideoLayout(int nLayout, int bActive, CVideoLayout* pRetLayout)
{
	CVideoLayout vLayout;

	vLayout.SetScreenLayout(nLayout);

	int nCells = vLayout.Layout2SrcNum(nLayout);

	for (int i = 0; i < nCells; i++)
	{
		CVideoCellLayout cell;
		cell.SetCellId(i+1);
		cell.SetCurrentPartyId(0xffffffff);
		cell.SetAudioActivated();
		vLayout.AddCell(cell);
	}

	vLayout.SetActive(bActive);
	AddRsrvVidLayout(vLayout);

	if (pRetLayout)
		*pRetLayout = vLayout;
}

//--------------------------------------------------------------------------
bool CCommResApi::TestMultiCascadeValidity(CRsrvParty& party)
{
	if (party.GetCascadeMode() != CASCADE_MODE_NONE)
	{
		TRACEINTO
			<< "\n  PartyName              :" << party.GetName()
			<< "\n  PartyType              :" << party.GetPartyType()
			<< "\n  ConnectionType         :" << (int)party.GetConnectionType()
			<< "\n  CascadedLinks          :" << (int)party.GetCascadedLinksNumber()
			<< "\n  ConfCascadedLinks      :" << (int)m_cascadedLinksNumber
			<< "\n  CascadedPartiesCounter :" << m_cascadedPartiesCounter;

		if (party.GetConnectionType() == DIAL_IN)
		{
			// *** A DIAL-IN connection is received:
			// reject connection if its links # is different from conf's links #
			if (party.GetPartyType() != eRegularParty) // TODO: verify the condition is correct
				return !m_cascadedLinksNumber || party.GetCascadedLinksNumber() == m_cascadedLinksNumber;

			// *** DIAL-IN party is added via GUI:
			// Reset its links #
			party.SetCascadedLinksNumber(0);
		}
		else
		{
			// *** DIAL-OUT party is added via GUI:
			// Force links # for DIAL-OUT only when conference has already one or more cascaded parties
			if (m_cascadedLinksNumber)
				party.SetCascadedLinksNumber(m_cascadedLinksNumber);
		}
	}

	return true;
}

//--------------------------------------------------------------------------
void CCommResApi::OnPartyAdd(const CRsrvParty& party)
{
	if (party.GetCascadeMode() != CASCADE_MODE_NONE)
	{
		TRACEINTO
			<< "\n  PartyName              :" << party.GetName()
			<< "\n  PartyType              :" << party.GetPartyType()
			<< "\n  ConnectionType         :" << (int)party.GetConnectionType()
			<< "\n  CascadedLinks          :" << (int)party.GetCascadedLinksNumber()
			<< "\n  ConfCascadedLinks      :" << (int)m_cascadedLinksNumber
			<< "\n  CascadedPartiesCounter :" << m_cascadedPartiesCounter;

		if (!m_cascadedLinksNumber)
			m_cascadedLinksNumber = party.GetCascadedLinksNumber();

		if (party.GetPartyType() == eMainLinkParty)
			++m_cascadedPartiesCounter;
	}
}

//--------------------------------------------------------------------------
void CCommResApi::OnPartyDelete(const CRsrvParty& party)
{
	if (party.GetCascadeMode() != CASCADE_MODE_NONE)
	{
		TRACEINTO
			<< "\n  PartyName              :" << party.GetName()
			<< "\n  PartyType              :" << party.GetPartyType()
			<< "\n  ConnectionType         :" << (int)party.GetConnectionType()
			<< "\n  CascadedLinks          :" << (int)party.GetCascadedLinksNumber()
			<< "\n  ConfCascadedLinks      :" << (int)m_cascadedLinksNumber
			<< "\n  CascadedPartiesCounter :" << m_cascadedPartiesCounter;

		if (party.GetPartyType() != eSubLinkParty && party.GetCascadedLinksNumber())
		{
			PASSERT(!m_cascadedPartiesCounter);
			--m_cascadedPartiesCounter;

			if (!m_cascadedPartiesCounter)
				m_cascadedLinksNumber = 0;
		}
	}
}

//--------------------------------------------------------------------------
BOOL CCommResApi::IsNormalRes() const
{
	if (IsTemplate() || GetEntryQ() || IsMeetingRoom())
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------------
void CCommResApi::DecreaseReoccurrenceNum()
{
	if (m_meetingRoomReoccurrenceNum)
		m_meetingRoomReoccurrenceNum--;
}

//--------------------------------------------------------------------------
void CCommResApi::IncreaseReoccurrenceNum()
{
	m_meetingRoomReoccurrenceNum++;
}

//--------------------------------------------------------------------------
void CCommResApi::SetMeetingRoomState(BYTE value)
{
	// Entry-queue can be multiple while meetingroomstate is activated and bigger than 1 .
	// regular meeting-room can be only passive or active (1).
	int oldState = m_meetingRoomState;

	if (value == MEETING_ROOM_INITIALIZE_STATE)
		m_meetingRoomState = MEETING_ROOM_PASSIVE_STATE;
	else
		m_meetingRoomState = value;

	switch (value)
	{
		case MEETING_ROOM_ACTIVE_STATE:
		{
			TRACEINTO << "Name:" << GetName() << ", OldState:" << oldState << ", NewState:" << (int)m_meetingRoomState << " - MEETING_ROOM_ACTIVE_STATE";
			break;
		}

		case MEETING_ROOM_PASSIVE_STATE:
		{
			TRACEINTO << "Name:" << GetName() << ", OldState:" << oldState << ", NewState:" << (int)m_meetingRoomState << " - MEETING_ROOM_PASSIVE_STATE";
			break;
		}

		case MEETING_ROOM_INITIALIZE_STATE:
		{
			TRACEINTO << "Name:" << GetName() << ", OldState:" << oldState << ", NewState:" << (int)m_meetingRoomState << " - MEETING_ROOM_INITIALIZE_STATE";
			break;
		}

		default:
		{
			TRACEWARN << "Name:" << GetName() << ", State:" << (int)value << " - Failed, unknown state";
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CCommResApi::SetBillingData(const char* billingData)
{
	m_BillingInfo = CSmallString(billingData);
	if (NULL != billingData)
	{
		m_BillingInfo = CSmallString(billingData);
		SLOWCHANGE;
	}
	else
	{
		DBGPASSERT(1);
	}
}

//--------------------------------------------------------------------------
void CCommResApi::SetContactInfo(const char* confContactInfo, int ContactNumber)
{
	if (NULL != confContactInfo)
	{
		m_pConfContactInfo->SetContactInfo(confContactInfo, ContactNumber);
		SLOWCHANGE;
	}
}

//--------------------------------------------------------------------------
BYTE CCommResApi::IsConfFromProfile(DWORD& profileID)
{
	if (m_dwAdHocProfileId != 0xFFFFFFFF)
	{
		profileID = m_dwAdHocProfileId;
		return YES;
	}
	return NO;
}

//--------------------------------------------------------------------------
void CCommResApi::SetMyProfileBasedParams(const CCommResApi* pProfile, BOOL updateServiceRegistrationContent)
{
	CCommResApi profile = *pProfile;

	m_media                   = profile.m_media;
	m_videoSession            = profile.m_videoSession; // Default in profile should be CP
	m_videoProtocol           = profile.m_videoProtocol;
	m_confTransferRate        = profile.m_confTransferRate;
	m_audioRate               = profile.m_audioRate;
	m_videoPictureFormat      = profile.m_videoPictureFormat;
	m_CIFframeRate            = profile.m_CIFframeRate;
	m_isSameLayout            = profile.m_isSameLayout;
	m_automaticTermination    = profile.m_automaticTermination;
	m_audioMixDepth           = profile.m_audioMixDepth;
	m_talkHoldTime            = profile.m_talkHoldTime;
	m_numUndefParties         = profile.m_numUndefParties;
	m_audioTone               = profile.m_audioTone;
	m_dualVideoMode           = profile.m_dualVideoMode;
	m_EnterpriseMode          = profile.m_EnterpriseMode;
	m_EnterpriseModeFixedRate = profile.m_EnterpriseModeFixedRate;

	m_PresentationProtocol = GetEntryQ() ? static_cast<BYTE>(ePresentationAuto) : profile.m_PresentationProtocol;

	m_cascadeOptimizeResolution  = profile.m_cascadeOptimizeResolution;
	m_isHighProfileContent       = profile.m_isHighProfileContent;
	m_operatorConf               = profile.m_operatorConf;
	m_time_beforeFirstJoin       = profile.m_time_beforeFirstJoin;
	m_time_afterLastQuit         = profile.m_time_afterLastQuit;
	m_LastQuitType               = profile.m_LastQuitType;
	m_ipVideoRate                = profile.m_ipVideoRate;
	m_meetingRoomReoccurrenceNum = profile.m_meetingRoomReoccurrenceNum;
	m_confLockFlag               = profile.m_confLockFlag;
	m_LSDRate                    = profile.m_LSDRate;

	if (profile.m_pAvMsgStruct && (GetEntryQ() == FALSE))
		*m_pAvMsgStruct = *profile.m_pAvMsgStruct;

	if (m_pLectureMode && profile.m_pLectureMode)
	{
		m_pLectureMode->SetTimerOnOff(profile.m_pLectureMode->GetTimerOnOff());
		// in cop mode the type is setting the lecture mode params
		m_pLectureMode->SetLectureModeType(profile.m_pLectureMode->GetLectureModeType());

		if (m_pLectureMode->GetLectureModeType() != 0)
		{
			m_isSameLayout = FALSE;
		}
	}


	int i;

	//Predefined forced layout for KAF - FSN 433
	if (!m_overideProfileLayout || profile.m_telePresenceModeConfiguration == YES)
	{
		for (i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
		{
			POBJDELETE(m_pRsrvVideoLayout[i]);

			if (profile.m_pRsrvVideoLayout[i] == NULL)
				m_pRsrvVideoLayout[i] = NULL;
			else
				m_pRsrvVideoLayout[i] = new CVideoLayout(*profile.m_pRsrvVideoLayout[i]);
		}
		m_numRsrvVideoSource = profile.m_numRsrvVideoSource;
		m_isAutoLayout       = profile.m_isAutoLayout;
	}

	if (profile.m_pVisualEffectsInfo)
	{
		*m_pVisualEffectsInfo = *profile.m_pVisualEffectsInfo;
		m_pVisualEffectsInfo->SetSiteNamesEnable(profile.m_isSiteNamesEnabled);
	}

	if (profile.m_pMessageOverlayInfo)
	{
		m_pMessageOverlayInfo->Dump("CCommResApi::SetMyProfileBasedParams (old)");

		*m_pMessageOverlayInfo = *profile.m_pMessageOverlayInfo;

		m_pMessageOverlayInfo->Dump("CCommResApi::SetMyProfileBasedParams (new)");
	}

	if (profile.m_pSiteNameInfo)
	{
		m_pSiteNameInfo->Dump("CCommResApi::SetMyProfileBasedParams (old)");

		*m_pSiteNameInfo = *profile.m_pSiteNameInfo;

		m_pSiteNameInfo->Dump("CCommResApi::SetMyProfileBasedParams (new)");

		if (m_pVisualEffectsInfo->GetBackgroundImageID() == 2 || m_pVisualEffectsInfo->GetBackgroundImageID() == 1)
			m_pSiteNameInfo->EnableBorder(TRUE);
	}

	SetEncryptionParameters(profile.m_encryption, profile.m_eEncryptionType);
	m_video         = profile.m_video;
	m_QCIFframeRate = profile.m_QCIFframeRate;
	m_confType      = profile.m_confType;
	strncpy(m_RecLinkName, profile.m_RecLinkName, H243_NAME_LEN);
	m_alertToneTiming                    = profile.m_alertToneTiming;
	m_IsCascadeEQ                        = profile.m_IsCascadeEQ;
	m_meetMePerEntryQ                    = profile.m_meetMePerEntryQ;
	m_confOnHoldFlag                     = profile.m_confOnHoldFlag;
	m_Mute_Incoming_Parties_Lecture_Mode = profile.m_Mute_Incoming_Parties_Lecture_Mode;
	m_muteAllPartiesAudioExceptLeader    = profile.m_muteAllPartiesAudioExceptLeader;
	m_muteAllPartiesVideoExceptLeader    = profile.m_muteAllPartiesVideoExceptLeader;
	m_FECC_Enabled                       = profile.m_FECC_Enabled;
	m_isVideoPlusConf                    = profile.m_isVideoPlusConf;
	m_IsConfOnPort                       = profile.m_IsConfOnPort;
	m_HD                                 = profile.m_HD;
	m_videoQuality                       = profile.m_videoQuality;
	m_confMaxResolution                  = profile.m_confMaxResolution; // olga
	m_isTelePresenceMode                 = profile.m_isTelePresenceMode;
	m_IsAudioOnlyRecording               = profile.m_IsAudioOnlyRecording;
	m_HDResolution                       = profile.m_HDResolution;
	m_H264VSWHighProfilePreference       = profile.m_H264VSWHighProfilePreference;
	m_isLpr                              = profile.m_isLpr;
	m_isVideoClarityEnabled              = profile.m_isVideoClarityEnabled;
	m_ShowContentAsVideo                 = profile.m_ShowContentAsVideo;
	m_AutoRedial                         = profile.m_AutoRedial;
	SetExclusiveContentMode(profile.m_bIsExclusiveContentMode);
	m_EchoSuppression     = profile.m_EchoSuppression;
	m_KeyboardSuppression = profile.m_KeyboardSuppression;
	m_isAutoMuteNoisyParties = profile.m_isAutoMuteNoisyParties;
	m_AutoScanInterval    = profile.m_AutoScanInterval;
	m_isSiteNamesEnabled  = profile.m_isSiteNamesEnabled;

	if (profile.m_pCopConfigurationList)
		*m_pCopConfigurationList = *profile.m_pCopConfigurationList;

	// Gathering
	// if AppointmentId not empty, recording field will be set according to Appointment and not from profile
	if (strlen(m_appoitnmentID) == 0)
	{
		m_EnableRecording                = profile.m_EnableRecording;
		m_EnableRecordingIcon            = profile.m_EnableRecordingIcon;
		m_IsEnableRecNotify		     =profile.m_IsEnableRecNotify;
		m_StartRecPolicy                 = profile.m_StartRecPolicy;
		m_startConfRequiresLeaderOnOff   = profile.m_startConfRequiresLeaderOnOff;
		m_terminateConfAfterChairDropped = profile.m_terminateConfAfterChairDropped;
		m_bGatheringEnabled              = profile.m_bGatheringEnabled;
		m_eLanguage                      = profile.m_eLanguage;
		strncpy(m_sNumberAccess_1, profile.m_sNumberAccess_1, ONE_LINE_BUFFER_LEN);
		strncpy(m_sNumberAccess_2, profile.m_sNumberAccess_2, ONE_LINE_BUFFER_LEN);
		strncpy(m_sIpNumberAccess, profile.m_sIpNumberAccess, ONE_LINE_BUFFER_LEN);
	}

	strncpy(m_sFreeText_1, profile.m_sFreeText_1, ONE_LINE_BUFFER_LEN);
	strncpy(m_sFreeText_2, profile.m_sFreeText_2, ONE_LINE_BUFFER_LEN);
	strncpy(m_sFreeText_3, profile.m_sFreeText_3, ONE_LINE_BUFFER_LEN);

	if (strlen(profile.m_FocusUriScheduling))
		strncpy(m_FocusUriScheduling, profile.m_FocusUriScheduling, ONE_LINE_BUFFER_LEN);

	if (strlen(profile.m_FocusUriCurrently))
		strncpy(m_FocusUriCurrently, profile.m_FocusUriCurrently, ONE_LINE_BUFFER_LEN);

	m_telePresenceModeConfiguration = profile.m_telePresenceModeConfiguration;
	m_telePresenceLayoutMode        = profile.m_telePresenceLayoutMode;
	SetManageTelepresenceLayoutInternaly(IsLayoutManagedInternally());
	m_isCropping                    = profile.m_isCropping;
	m_ResSts                        = profile.m_ResSts; // Status Field
	m_isAudioClarity                = profile.m_isAudioClarity;
	m_isAutoBrightness              = profile.m_isAutoBrightness;
	m_TipCompatibility              = profile.m_TipCompatibility;
	m_avMcuCascadeVideoMode           = profile.m_avMcuCascadeVideoMode;
	m_natKeepAlivePeriod            = profile.m_natKeepAlivePeriod;

	if (updateServiceRegistrationContent)
	{
		for (i = 0; i < NUM_OF_IP_SERVICES; i++)
		{
			if (profile.m_ServiceRegistrationContent[i].service_name != '\0')
			{
				strncpy(m_ServiceRegistrationContent[i].service_name, profile.m_ServiceRegistrationContent[i].service_name, ONE_LINE_BUFFER_LEN);
				m_ServiceRegistrationContent[i].sip_register = profile.m_ServiceRegistrationContent[i].sip_register;
				m_ServiceRegistrationContent[i].accept_call  = profile.m_ServiceRegistrationContent[i].accept_call;

				// VSGNINJA-964: Ninja>>SIP>>v8.2.0.58>>Conference SIP Registration always display as Not configured
				if (m_ServiceRegistrationContent[i].sip_register)
				{
					m_ServiceRegistrationContent[i].status       = eSipRegistrationStatusTypeFailed; // sipProxySts
				}
				else
				{
					m_ServiceRegistrationContent[i].status       = eSipRegistrationStatusTypeNotConfigured;
				}
			}
		}

		m_SipRegistrationTotalSts = profile.m_SipRegistrationTotalSts; // sipProxySts
	}

	m_FontType = profile.m_FontType;

	m_speakerChangeThreshold = profile.m_speakerChangeThreshold;
	SetSpeakChangeParam(m_speakerChangeThreshold);

	m_confMediaType = profile.m_confMediaType;
	m_eOperationPointPreset = profile.m_eOperationPointPreset;

	/* In previous versions, this line does not exist or is a comment (m_cascadeMode = profile.m_cascadeMode;)
	We do not know why this line appears again, and in entryQ it overrides the value
	We give solution for entryQ, for AT&T project   */
	if (!GetEntryQ())
	{
		m_cascadeMode = profile.m_cascadeMode;
	}

	//Content Transcoding
    m_ContentMultiResolutionEnabled      = profile.m_ContentMultiResolutionEnabled;
    m_ContentXCodeH264Supported      = profile.m_ContentXCodeH264Supported;
    m_ContentXCodeH263Supported      = profile.m_ContentXCodeH263Supported;
    m_isCascadeOptimized      = profile.m_isCascadeOptimized;
    m_isAsSipContent          = profile.m_isAsSipContent;
    m_SrsPlaybackLayout	= profile.m_SrsPlaybackLayout;

	//Indication for audio participants
	m_IconDisplayPosition =  profile.m_IconDisplayPosition;
	m_EnableAudioParticipantsIcon =  profile.m_EnableAudioParticipantsIcon;
	m_AudioParticipantsIconDisplayMode = profile.m_AudioParticipantsIconDisplayMode;
	m_AudioParticipantsIconDuration = profile.m_AudioParticipantsIconDuration;
	m_EnableSelfNetworkQualityIcon =  profile.m_EnableSelfNetworkQualityIcon;

	// ee-462 VEQ support for AVC and SVC calls
	if (GetEntryQ() && (m_ivrProviderEQ || m_externalIvrControl) && m_confMediaType != eAvcOnly)
	{
		TRACEINTO << "EE-462 - EQ conf media type mismatech- changed to eAvcOnly, was " << ConfMediaTypeToString(m_confMediaType) << "";
		m_confMediaType = eAvcOnly;
	}

}

//--------------------------------------------------------------------------
void CCommResApi::SetSIPFactory(BYTE value)
{
	m_isSIPFactory = value;
	UPDATECONFLAGSNEGATIV(m_isSIPFactory, NO, SIP_FACTORY);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetEncryptionParameters(BYTE newIsEncryption, BYTE newEncryptionType)
{
	if (newIsEncryption)
		m_eEncryptionType = ((EEncyptionType)newEncryptionType != eEncryptNone) ? (EEncyptionType)newEncryptionType : eEncryptAll;
	else
		m_eEncryptionType = eEncryptNone;

	m_encryption = (newIsEncryption) ? YES : NO;
	UPDATECONFLAGSNEGATIV(m_encryption, NO, ENCRYPTION);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetIsTelePresenceMode(BYTE value)
{
	m_isTelePresenceMode = (value) ? YES : NO;
	UPDATECONFLAGSNEGATIV(m_isTelePresenceMode, NO, TELEPRESENCE_MODE);
	FASTCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetManageTelepresenceLayoutInternaly(const BYTE yesNo)
{
	m_manageTelepresenceLayoutInternaly = yesNo;
}

//--------------------------------------------------------------------------
int CCommResApi::CleanAllServicePrefix()
{
	for (int i = 0; i < (int)m_numServicePrefixStr; i++)
	{
		if (m_pServicePrefixStr[i] != NULL)
		{
			POBJDELETE(m_pServicePrefixStr[i]);
			m_pServicePrefixStr[i] = NULL;
		}
	}

	m_numServicePrefixStr = 0;
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::AddServicePrefix(const CServicePrefixStr& other)
{
	if (m_numServicePrefixStr >= MAX_NET_SERV_PROVIDERS_IN_LIST)
		return STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED;

	m_pServicePrefixStr[m_numServicePrefixStr] = new CServicePrefixStr(other);
	m_numServicePrefixStr++;
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::AddServicePhone(const CServicePhoneStr& other)
{
	if (m_numServicePhoneStr >= MAX_NET_SERV_PROVIDERS_IN_LIST)
		return STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED;

	if (FindServicePhone(other) != NOT_FIND)
		return STATUS_SERVICE_PROVIDER_NAME_EXISTS;

	POBJDELETE(m_pServicePhoneStr[m_numServicePhoneStr]);
	m_pServicePhoneStr[m_numServicePhoneStr] = new CServicePhoneStr(other);

	m_numServicePhoneStr++;
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResApi::FindServicePhone(const CServicePhoneStr& other)
{
	for (int i = 0; i < (int)m_numServicePhoneStr; i++)
	{
		// find a service that "larger(have the same and all phones us "other"
		// but can be more phones ) or equal to the given
		if (m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) >= other)
			return i;

		if (m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) < other)
		{
			*m_pServicePhoneStr[i] = other;
			return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
CServicePhoneStr* CCommResApi::GetFirstServicePhone()
{
	m_ind_service_phone = 0;
	return m_pServicePhoneStr[0];
}

//--------------------------------------------------------------------------
CServicePhoneStr* CCommResApi::GetNextServicePhone()
{
	m_ind_service_phone++;
	if (m_ind_service_phone >= m_numServicePhoneStr) return NULL;

	return m_pServicePhoneStr[m_ind_service_phone];
}

//--------------------------------------------------------------------------
void CCommResApi::SetIsGateway(BYTE value)
{
	m_isGateway = (value) ? YES : NO;
	UPDATECONFLAGSNEGATIV(m_isGateway, NO, RMX_GATEWAY);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetIsLPR(BYTE value)
{
	m_isLpr = (value) ? YES : NO;
	UPDATECONFLAGSNEGATIV(m_isLpr, NO, LPR);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetEchoSuppression(BYTE value)
{
	m_EchoSuppression = (value) ? YES : NO;
	UPDATECONFLAGSNEGATIV(m_EchoSuppression, NO, ECHO_SUPPRESSION);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetKeyboardSuppression(BYTE value)
{
	m_KeyboardSuppression = (value) ? YES : NO;
	UPDATECONFLAGSNEGATIV(m_KeyboardSuppression, NO, KEYBOARD_SUPPRESSION);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
std::string CCommResApi::GetFileUniqueName(const std::string& dbPrefix) const
{
	unsigned int ID_LENGTH_ON_FILE_NAME = 5; // the length of the id in the file name
	string fileName = dbPrefix;
	char pConfId[33];
	sprintf(pConfId, "%d", GetMonitorConfId());
	string confIdStr = pConfId;

	// Padding the file anme with zeros
	if (confIdStr.size() < ID_LENGTH_ON_FILE_NAME)
		for (unsigned int i = 0; i < (ID_LENGTH_ON_FILE_NAME - confIdStr.size()); ++i)
			fileName += "0";

	fileName += pConfId;
	fileName += ".xml";
	return fileName;
}

//--------------------------------------------------------------------------
CRsrvParty* CCommResApi::GetFirstParty()
{
	m_ind = 1;
	return m_pParty[0];
}

//--------------------------------------------------------------------------
CRsrvParty* CCommResApi::GetNextParty()
{
	if (m_ind >= m_numParties) return NULL;

	return m_pParty[m_ind++];
}

//--------------------------------------------------------------------------
void CCommResApi::SetCopConfigurationList(CCOPConfigurationList* pCOPConfigurationList)
{
	POBJDELETE(m_pCopConfigurationList);
	m_pCopConfigurationList = pCOPConfigurationList;
}

//--------------------------------------------------------------------------
void CCommResApi::SetLanguageFromString(const char* pszLanguage)
{
	if (!strcmp("de_DE", pszLanguage))
		m_eLanguage = eGerman;
	else if (!strcmp("es_ES", pszLanguage))
		m_eLanguage = eSpanishSA;
	else if (!strcmp("fr_FR", pszLanguage))
		m_eLanguage = eFrench;
	else if (!strcmp("ja_JP", pszLanguage))
		m_eLanguage = eJapanese;
	else if (!strcmp("ko_KR", pszLanguage))
		m_eLanguage = eKorean;
	else if (!strcmp("zh_CN", pszLanguage))
		m_eLanguage = eChineseSimpl;
	else
		m_eLanguage = eEnglish;

	SLOWCHANGE;
}

//--------------------------------------------------------------------------
const char* CCommResApi::GetServiceRegistrationContentServiceName(int serviceId)
{
	PASSERT_AND_RETURN_VALUE(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES, NULL);
	return m_ServiceRegistrationContent[serviceId].service_name;
}

//--------------------------------------------------------------------------
int CCommResApi::GetServiceRegistrationContentServiceIndexByName(const char* name) // sipProxySts
{
	int serviceIndex = -1;
	for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
	{
		if (strcmp(m_ServiceRegistrationContent[i].service_name, name) == 0)
			serviceIndex = i;
	}

	return serviceIndex;
}

//--------------------------------------------------------------------------
BOOL CCommResApi::GetServiceRegistrationContentRegister(int serviceId)
{
	PASSERT_AND_RETURN_VALUE(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES, FALSE);
	return m_ServiceRegistrationContent[serviceId].sip_register;
}

//--------------------------------------------------------------------------
BOOL CCommResApi::GetServiceRegistrationContentAcceptCall(int serviceId)
{
	PASSERT_AND_RETURN_VALUE(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES, FALSE);
	return m_ServiceRegistrationContent[serviceId].accept_call;
}

//--------------------------------------------------------------------------
void CCommResApi::SetServiceRegistrationContentServiceName(int serviceId, const char* name)
{
	PASSERT_AND_RETURN(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES);
	strcpy_safe(m_ServiceRegistrationContent[serviceId].service_name, name);
}

//--------------------------------------------------------------------------
void CCommResApi::SetServiceRegistrationContentRegister(int serviceId, BOOL reg)
{
	PASSERT_AND_RETURN(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES);
	m_ServiceRegistrationContent[serviceId].sip_register = reg;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::SetServiceRegistrationContentAcceptCall(int serviceId, BOOL accept)
{
	PASSERT_AND_RETURN(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES);
	m_ServiceRegistrationContent[serviceId].accept_call = accept;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
BYTE CCommResApi::GetServiceRegistrationContentStatus(int serviceId) // sipProxySts
{
	PASSERT_AND_RETURN_VALUE(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES, 0);
	return m_ServiceRegistrationContent[serviceId].status;
}

//--------------------------------------------------------------------------
void CCommResApi::SetServiceRegistrationContentStatus(int serviceId, BYTE status) // sipProxySts
{
	PASSERT_AND_RETURN(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES);

	TRACEINTO << "ServiceId:" << serviceId << ", Status:" << (int)status;
	m_ServiceRegistrationContent[serviceId].status = status;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommResApi::UpdateServiceRegistrationTotalStatus(int serviceId, BYTE status) // sipProxySts
{
	PASSERT_AND_RETURN(serviceId < 0 || serviceId >= NUM_OF_IP_SERVICES);

	TRACEINTO << "OldSipRegistrationTotalStatus:" << (int)m_SipRegistrationTotalSts << ", ServiceId:" << serviceId << ", Status:" << (int)status;

	BYTE localServiceRegistrationTotalStatus = 0x00;

	m_ServiceRegistrationContent[serviceId].status = status;

	for (int indx_serviceId = 0; indx_serviceId < NUM_OF_IP_SERVICES; indx_serviceId++)
	{
		switch (m_ServiceRegistrationContent[indx_serviceId].status)
		{
			case eSipRegistrationStatusTypeNotConfigured:
			{
				localServiceRegistrationTotalStatus |= SIP_REG_NOT_CONFIGURED;
				break;
			}

			case eSipRegistrationStatusTypeRegistered:
			{
				localServiceRegistrationTotalStatus |= SIP_REG_REGISTERED;
				break;
			}

			case eSipRegistrationStatusTypeFailed:
			{
				localServiceRegistrationTotalStatus |= SIP_REG_FAILED;
				break;
			}

			default:
			{
				TRACEWARN << "ServiceId:" << serviceId << " - Failed, illegal status";
				return;
			}
		}
	}

	TRACEINTO << "LocalServiceRegistrationTotalStatus:" << (int)localServiceRegistrationTotalStatus;

	// STATUS  localServiceRegistrationTotalStatus
	// ======  ===================================
	// 0001    eSipRegistrationTotalStatusNotConfigured
	// 0010    eSipRegistrationTotalStatusRegistered
	// 0100    eSipRegistrationTotalStatusFailed
	// 0011    eSipRegistrationTotalStatusRegistered
	// 0110    eSipRegistrationTotalStatusPartiallyRegistered
	// 0111    eSipRegistrationTotalStatusPartiallyRegistered
	// 0101    eSipRegistrationTotalStatusFailed

	switch (localServiceRegistrationTotalStatus)
	{
		case SIP_REG_NOT_CONFIGURED:
		{
			SetSipRegistrationTotalSts(eSipRegistrationTotalStatusTypeNotConfigured);
			break;
		}

		case SIP_REG_FAILED:
		case (SIP_REG_FAILED | SIP_REG_NOT_CONFIGURED):
		{
			SetSipRegistrationTotalSts(eSipRegistrationTotalStatusTypeFailed);
			break;
		}

		case (SIP_REG_REGISTERED | SIP_REG_FAILED):
		// or register or here???  case (SIP_REG_REGISTERED & SIP_REG_NOT_CONFIGURED):
		{
			SetSipRegistrationTotalSts(eSipRegistrationTotalStatusTypePartiallyRegistered);
			break;
		}

		case SIP_REG_REGISTERED:
		case (SIP_REG_REGISTERED | SIP_REG_NOT_CONFIGURED):
		{
			SetSipRegistrationTotalSts(eSipRegistrationTotalStatusTypeRegistered);
			break;
		}

		default:
		{
			TRACEWARN << "Illegal calculated status";
			return;
		}
	}

	TRACEINTO << "NewSipRegistrationTotalStatus:" << (int)m_SipRegistrationTotalSts;
}

//--------------------------------------------------------------------------
void CCommResApi::SetSpeakChangeParam(BYTE speakerChangeThreshold)
{
	switch (speakerChangeThreshold)
	{
		case eAutoTicks:
			m_talkHoldTime = 300;
			m_confSpeakerChangeMode = E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;
			break;

		case e150Ticks:
			m_talkHoldTime = 150;
			m_confSpeakerChangeMode = E_CONF_FAST_SPEAKER_CHANGE_MODE;
			break;

		case e300Ticks:
			m_talkHoldTime = 300;
			m_confSpeakerChangeMode = E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;
			break;

		case e500Ticks:
			m_talkHoldTime = 500;
			m_confSpeakerChangeMode = E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;
			break;

		default:
		{
			m_talkHoldTime = 300;
			m_confSpeakerChangeMode = E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;
			TRACEWARN << "speakerChangeThreshold:" << speakerChangeThreshold << " - Illegal";
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CCommResApi::SetConfMediaType(eConfMediaType value)
{
	if ((value > eConfMediaType_dummy) && (value < eConfMediaType_last))
	{
		m_confMediaType = value;
		TRACEINTO << "ConfMediaType: " << m_confMediaType;
	}
}
void CCommResApi::SetOperationPointPreset(EOperationPointPreset value)
{
	if ((value > eOPP_dummy) && (value < eOPP_last))
	{
		m_eOperationPointPreset = value;
		TRACEINTO << "m_eOperationPointPreset: " << m_eOperationPointPreset;
	}
	else
		PASSERT(value);
}
////////////////////////////////////////////////////////////////////////////
void CCommResApi::SetContentMultiResolutionEnabled(const BYTE ContentMultiResolutionEnabled)
{
	m_ContentMultiResolutionEnabled = ContentMultiResolutionEnabled;
	if(!ContentMultiResolutionEnabled)
		SetIsAsSipContent(NO);
	FASTCHANGE;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCommResApi::GetContentMultiResolutionEnabled() const
{
	return m_ContentMultiResolutionEnabled;
}

/////////////////////////////////////////////////////////////////////////////
void CCommResApi::SetContentXCodeH264Supported(const BYTE ContentXCodeH264Supported)
{
	m_ContentXCodeH264Supported = ContentXCodeH264Supported;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCommResApi::GetContentXCodeH264Supported() const
{
	return m_ContentXCodeH264Supported;
}

/////////////////////////////////////////////////////////////////////////////
void CCommResApi::SetContentXCodeH263Supported(const BYTE ContentXCodeH263Supported)
{
	m_ContentXCodeH263Supported = ContentXCodeH263Supported;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCommResApi::GetContentXCodeH263Supported() const
{
	return m_ContentXCodeH263Supported;
}

/////////////////////////////////////////////////////////////////////////////
void CCommResApi::SetIsCascadeOptimized(const BYTE isCascadeOptimized)
{
	m_isCascadeOptimized = isCascadeOptimized;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCommResApi::GetIsCascadeOptimized() const
{
	return m_isCascadeOptimized;
}
/////////////////////////////////////////////////////////////////////////////
void CCommResApi::SetIsAsSipContent(const BYTE isAsSipContent)
{
	m_isAsSipContent = isAsSipContent;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCommResApi::GetIsAsSipContent() const
{
	return m_isAsSipContent;
}
/////////////////////////////////////////////////////////////////////////////
void CCommResApi::SetIsHighProfileContent(const BYTE isHighProfileContent)
{
	m_isHighProfileContent = isHighProfileContent;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CCommResApi::GetIsHighProfileContent() const
{
	return m_isHighProfileContent;
}
/////////////////////////////////////////////////////////////////////////////
void CCommResApi::SetCorrelationId()
{
	// *** Get the MAC address by the network card type ***

	CProcessBase* proc = CProcessBase::GetProcess();


	std::string MAC = proc->m_NetSettings.m_MacAddress;


	PASSERTMSG(MAC.size() == 0 || MAC.size() > 18, "Wrong MAC Address");

	// *** combine the Correlation ID out of components ***

	std::ostringstream os;

	os
		<< m_confDisplayName << '-'
		<< m_startTime.m_year << '-'
		<< std::setfill('0') << std::setw(2) << m_startTime.m_mon << '-'
		<< std::setfill('0') << std::setw(2) << m_startTime.m_day
		<< 'T'
		<< std::setfill('0') << std::setw(2) << m_startTime.m_hour << ':'
		<< std::setfill('0') << std::setw(2) << m_startTime.m_min << ':'
		<< std::setfill('0') << std::setw(2) << m_startTime.m_sec << ".0Z"
		<< '-' << MAC;

	strncpy(m_correlationId, os.str().c_str(), sizeof(m_correlationId) - 1);
	m_correlationId[sizeof(m_correlationId) - 1] = 0;

	TRACEINTO << m_correlationId;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CCommResApi::IsLayoutManagedInternally() const
{
	BYTE isInternaly = NO;

	switch(m_telePresenceLayoutMode){
	case eTelePresenceLayoutManual:
	case eTelePresenceLayoutContinuousPresence:
	{
		break;
	}
	case eTelePresenceLayoutRoomSwitch:
	{
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		PASSERT_AND_RETURN_VALUE(!sysConfig , FALSE);
		BOOL isManageTelepresenceRoomSwitchLayoutsEnabled;
		sysConfig->GetBOOLDataByKey("MANAGE_TELEPRESENCE_ROOM_SWITCH_LAYOUTS", isManageTelepresenceRoomSwitchLayoutsEnabled);

		if(isManageTelepresenceRoomSwitchLayoutsEnabled == YES){
			isInternaly = YES;
		}
		break;
	}
	case eTelePresenceLayoutCpSpeakerPriority:
	case eTelePresenceLayoutCpParticipantsPriority:
	{
		isInternaly = YES;
		break;
	}
	default:
	{
		DBGPASSERT(m_telePresenceLayoutMode);
		break;
	}
	}
	return isInternaly;
}
/////////////////////////////////////////////////////////////////////////////

void CCommResApi::SetMrcMcuId(WORD value)
{
	if (value >= MAX_MRC_MCU_ID-1)
	{
		TRACESTRFUNC(eLevelError) << " wrong Mcu Id = " << value << " will be set to max 1024";
		m_mrcMcuId = MAX_MRC_MCU_ID;
	} else
		m_mrcMcuId = value;
}
///////////////////////////////////////////////////
void   CCommResApi::SetFocusUriCurrently(const char* value) 	
{
	if(!value)
	{
		TRACEINTO<<" invalid current-Focus Uri! ";
	}
	TRACEINTO<<" new current-Focus Uri - "<<value;
	strcpy_safe(m_FocusUriCurrently, value); 
}

