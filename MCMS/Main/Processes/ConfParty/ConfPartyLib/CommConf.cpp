#include <string.h>
#include "NStream.h"
#include "psosxml.h"
#include "CommConf.h"
#include "CommConfDB.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "ConfApi.h"
#include "CommRes.h"
#include "CDRShort.h"
#include "CDRLogApi.h"
#include "CDRDetal.h"
#include "ConfStart.h"
#include "CdrApiClasses.h"
#include "SystemFunctions.h"
#include "OperEvent.h"
#include "BilParty.h"
#include "NetChannConn.h"
#include "NetChannelDisco.h"
#include "SipNetSetup.h"
#include "SysConfigKeys.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "ServicePrefixStr.h"
#include "ConfPartyProcess.h"
#include "ConfPartyOpcodes.h"
#include "OngoingConfStore.h"
#include "MccfIvrPackageResponse.h"
#include "ConfigManagerApi.h"
#include "CdrPersistHelper.h"
#include "PlcmCdrEventConfEnd.h"

////////////////////////////////////////////////////////////////////////////
//                        CCommConf
////////////////////////////////////////////////////////////////////////////
CCommConf::CCommConf() : CCommRes()
{
	for (int i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
		m_pVideoLayout[i] = NULL;

	for (int i = 0; i < 1000; i++)
	{
		m_DeletedCounterHistory[i] = 0;
		m_DeletedIdHistory[i]      = 0;
	}

	m_conf_status                      = 0;
	m_number_of_connected_parties      = 0;
	m_source_audio_Id                  = 0xFFFFFFFF;
	m_source_LSD_Id                    = 0xFFFFFFFF;
	m_source_HSD_Id                    = 0xFFFFFFFF;
	m_chair_Id                         = 0xFFFFFFFF;
	m_force_video_source_Id            = 0xFFFFFFFF;
	m_downspeed                        = NO;
	m_LSDRate                          = NO;
	m_curConfVideoLayout               = ONE_ONE;
	m_numVideoLayout                   = 0;
	m_ind_conf                         = 0;
	m_ind_vid_layout                   = 0;
	m_number_of_active_recording_ports = 0;
	m_meetingRoomIsUp                  = FALSE;
	m_MuteAllButX_Id                   = 0xFFFFFFFF;
	m_IsCandidateForTerminationMR      = FALSE;
	m_EPC_Content_source_Id            = 0xFFFFFFFF;
	m_COPChairInVC                     = FALSE;
	m_dwSummaryUpdateCounter           = 0;
	m_dwFullUpdateCounter              = 0;
	m_LastDeletedIndex                 = 0;
	m_pSubject                         = new CSubject();
	m_numVideoParties                  = 0;
	m_SipRegTotalSts                   = eSipRegistrationTotalStatusTypeNotConfigured;
	m_msConversationId[0]              = '\0';

	m_pSavedLectureMode                = NULL;
	m_pSavedLecturerVideoLayout        = NULL;

	m_mainPartiesCounter               = 0;
	m_currentConfCascadeMode		   = CASCADE_MODE_NONE;
	m_isVideoRecoveryStatus            = false;

	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
		m_activeSpeakerList[i] = 0xFFFFFFFF;

	m_bIsCallGeneratorConf             = FALSE;
	m_CGPartiesCounter                 = 0;
	m_isEnableHighVideoResInAvcToSvcMixMode = FALSE;
	m_isEnableHighVideoResInSvcToAvcMixMode = FALSE;
	m_isNotifyAvMcuUri 		=0;
}

////////////////////////////////////////////////////////////////////////////
CCommConf::CCommConf(const CCommConf& other) : CCommRes(*(const CCommRes*)&other)
{
	m_conf_status                 = other.m_conf_status;
	m_number_of_connected_parties = other.m_number_of_connected_parties;
	m_source_audio_Id             = other.m_source_audio_Id;
	m_source_LSD_Id               = other.m_source_LSD_Id;
	m_source_HSD_Id               = other.m_source_HSD_Id;
	m_chair_Id                    = other.m_chair_Id;
	m_force_video_source_Id       = other.m_force_video_source_Id;
	m_MuteAllButX_Id              = other.m_MuteAllButX_Id; //VNGFE-6720
	m_mainPartiesCounter          = other.m_mainPartiesCounter;
	m_currentConfCascadeMode	  = other.m_currentConfCascadeMode;
	m_isVideoRecoveryStatus       = other.m_isVideoRecoveryStatus;
	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

	CConfPartyProcess* pConfPartyProcess                  = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	WORD               maxPartiesInConfPerSystemMode      = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
	WORD               maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
	m_numParties      = 0;
	m_numVideoParties = 0;

	for (int i = 0; i < MAX_PARTIES_IN_CONF; i++)
	{
		if (m_pParty[i] != NULL)
		{
			delete   m_pParty[i];
			m_pParty[i] = NULL;
		}

		if (m_numParties < maxPartiesInConfPerSystemMode)
		{
			if (other.m_pParty[i] != NULL)
			{
				BYTE isAudioOnlyParty = other.m_pParty[i]->GetVoice();
				if (isAudioOnlyParty || (!isAudioOnlyParty && (m_numVideoParties < maxVideoPartiesInConfPerSystemMode)))
				{
					m_pParty[m_numParties] = new CConfParty(*(CConfParty*)other.m_pParty[i]);
					m_pParty[m_numParties]->SetRes(this);
					m_numParties++;
					if (!isAudioOnlyParty)
						m_numVideoParties++;
				}
			}
		}
	}

	m_downspeed          = other.m_downspeed;
	m_LSDRate            = other.m_LSDRate;
	m_curConfVideoLayout = other.m_curConfVideoLayout;
	m_numVideoLayout     = other.m_numVideoLayout;
	m_MuteAllButX_Id = other.m_MuteAllButX_Id;

	for (int i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
	{
		if (other.m_pVideoLayout[i] == NULL)
			m_pVideoLayout[i] = NULL;
		else
			m_pVideoLayout[i] = new CVideoLayout(*other.m_pVideoLayout[i]);
	}

	if ((m_videoSession != CONTINUOUS_PRESENCE) && (m_videoSession != SOFTWARE_CONTINUOUS_PRESENCE))
		m_curConfVideoLayout = ONE_ONE;

	m_ind_conf               = other.m_ind_conf;
	m_ind_vid_layout         = other.m_ind_vid_layout;
	m_dwConfFlags            = other.m_dwConfFlags;
	m_meetingRoomIsUp        = other.m_meetingRoomIsUp;
	m_EPC_Content_source_Id  = other.m_EPC_Content_source_Id;
	m_dwSummaryUpdateCounter = other.m_dwSummaryUpdateCounter;
	m_dwFullUpdateCounter    = other.m_dwFullUpdateCounter;
	m_fastUpdateCounter      = other.m_fastUpdateCounter;
	m_slowUpdateCounter      = other.m_slowUpdateCounter;
	m_LastDeletedIndex       = other.m_LastDeletedIndex;

	for (int i = 0; i < 1000; i++)
	{
		m_DeletedCounterHistory[i] = other.m_DeletedCounterHistory[i];
		m_DeletedIdHistory[i]      = other.m_DeletedIdHistory[i];
	}

	if (other.m_pSubject == NULL)
		m_pSubject = NULL;
	else
		m_pSubject = new CSubject(*other.m_pSubject);

	m_SipRegTotalSts = other.m_SipRegTotalSts; // sipProxySts
	strncpy(m_msConversationId, other.m_msConversationId, MS_CONVERSATION_ID_LEN);

	m_pSavedLectureMode = NULL;
	m_pSavedLecturerVideoLayout = NULL;

	if (other.m_pSavedLectureMode)
	{
			m_pSavedLectureMode = new CLectureModeParams(*other.m_pSavedLectureMode);
	}

	if (other.m_pSavedLecturerVideoLayout)
	{
			m_pSavedLecturerVideoLayout = new CVideoLayout(*other.m_pSavedLecturerVideoLayout);
	}
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
		m_activeSpeakerList[i] = other.m_activeSpeakerList[i];

	m_bIsCallGeneratorConf = other.m_bIsCallGeneratorConf;
	m_CGPartiesCounter = other.m_CGPartiesCounter;
	m_isEnableHighVideoResInAvcToSvcMixMode = other.m_isEnableHighVideoResInAvcToSvcMixMode;
	m_isEnableHighVideoResInSvcToAvcMixMode = other.m_isEnableHighVideoResInSvcToAvcMixMode;
	m_isNotifyAvMcuUri = other.m_isNotifyAvMcuUri;

}

////////////////////////////////////////////////////////////////////////////
CCommConf& CCommConf::operator =(const CCommConf& other)
{
	if (&other == this)
		return *this;

	CCommRes::operator =(other);

	m_conf_status                 = other.m_conf_status;
	m_number_of_connected_parties = other.m_number_of_connected_parties;
	m_source_audio_Id             = other.m_source_audio_Id;
	m_source_LSD_Id               = other.m_source_LSD_Id;
	m_source_HSD_Id               = other.m_source_HSD_Id;
	m_chair_Id                    = other.m_chair_Id;
	m_force_video_source_Id       = other.m_force_video_source_Id;
	m_MuteAllButX_Id              = other.m_MuteAllButX_Id; //VNGFE-6720
	m_mainPartiesCounter          = other.m_mainPartiesCounter;
    m_currentConfCascadeMode	  = other.m_currentConfCascadeMode;
	m_isVideoRecoveryStatus       = false;

	CConfPartyProcess* pConfPartyProcess                  = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	WORD               maxPartiesInConfPerSystemMode      = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
	WORD               maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
	m_numParties      = 0;
	m_numVideoParties = 0;
	for (int i = 0; i < MAX_PARTIES_IN_CONF; i++)
	{
		if (m_pParty[i] != NULL)
		{
			delete   m_pParty[i];
			m_pParty[i] = NULL;
		}

		if (m_numParties < maxPartiesInConfPerSystemMode)
		{
			if (other.m_pParty[i] != NULL)
			{
				BYTE isAudioOnlyParty = other.m_pParty[i]->GetVoice();
				if (isAudioOnlyParty || (!isAudioOnlyParty && (m_numVideoParties < maxVideoPartiesInConfPerSystemMode)))
				{
					m_pParty[m_numParties] = new CConfParty(*(CConfParty*)other.m_pParty[i]);
					m_pParty[m_numParties]->SetRes(this);
					m_numParties++;
					if (!isAudioOnlyParty)
						m_numVideoParties++;
				}
			}
		}
	}

	m_downspeed          = other.m_downspeed;
	m_LSDRate            = other.m_LSDRate;
	m_curConfVideoLayout = other.m_curConfVideoLayout;
	m_numVideoLayout     = other.m_numVideoLayout;
	m_MuteAllButX_Id = other.m_MuteAllButX_Id;


	for (int i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
	{
		POBJDELETE(m_pVideoLayout[i]);

		if (other.m_pVideoLayout[i] == NULL)
			m_pVideoLayout[i] = NULL;
		else
			m_pVideoLayout[i] = new CVideoLayout(*other.m_pVideoLayout[i]);
	}

	m_ind_conf               = other.m_ind_conf;
	m_ind_vid_layout         = other.m_ind_vid_layout;

	m_dwConfFlags            = other.m_dwConfFlags;
	m_meetingRoomIsUp        = other.m_meetingRoomIsUp;
	m_EPC_Content_source_Id  = other.m_EPC_Content_source_Id;
	m_dwSummaryUpdateCounter = other.m_dwSummaryUpdateCounter;
	m_dwFullUpdateCounter    = other.m_dwFullUpdateCounter;
	m_fastUpdateCounter      = other.m_fastUpdateCounter;
	m_slowUpdateCounter      = other.m_slowUpdateCounter;
	m_LastDeletedIndex       = other.m_LastDeletedIndex;
	for (int i = 0; i < 1000; i++)
	{
		m_DeletedCounterHistory[i] = other.m_DeletedCounterHistory[i];
		m_DeletedIdHistory[i]      = other.m_DeletedIdHistory[i];
	}

	if (other.m_pSubject == NULL)
		m_pSubject = NULL;
	else
	{
		POBJDELETE(m_pSubject);
		m_pSubject = new CSubject(*other.m_pSubject);
	}

	m_SipRegTotalSts = other.m_SipRegTotalSts;  // sipProxySts
	strncpy(m_msConversationId, other.m_msConversationId, MS_CONVERSATION_ID_LEN);
		POBJDELETE(m_pSavedLectureMode);
	POBJDELETE(m_pSavedLecturerVideoLayout);

	if (other.m_pSavedLectureMode)
	{
		m_pSavedLectureMode = new CLectureModeParams(*other.m_pSavedLectureMode);
	}

	if (other.m_pSavedLecturerVideoLayout)
	{
		m_pSavedLecturerVideoLayout = new CVideoLayout(*other.m_pSavedLecturerVideoLayout);
	}
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
		m_activeSpeakerList[i] = other.m_activeSpeakerList[i];

	m_bIsCallGeneratorConf = other.m_bIsCallGeneratorConf;
	m_CGPartiesCounter = other.m_CGPartiesCounter;
	m_isEnableHighVideoResInAvcToSvcMixMode = other.m_isEnableHighVideoResInAvcToSvcMixMode;
	m_isEnableHighVideoResInSvcToAvcMixMode = other.m_isEnableHighVideoResInSvcToAvcMixMode;
	m_isNotifyAvMcuUri = other.m_isNotifyAvMcuUri;

	//SetDefaultParamsAccordingToProductType();
	return *this;
}

////////////////////////////////////////////////////////////////////////////
CCommConf::CCommConf(const CCommRes& other) : CCommRes(other)
{
	m_conf_status                 = 0;
	m_number_of_connected_parties = 0;
	m_chair_Id                    = 0xFFFFFFFF;
	m_force_video_source_Id       = 0xFFFFFFFF;
	m_source_audio_Id             = 0xFFFFFFFF;
	m_source_LSD_Id               = 0xFFFFFFFF;
	m_source_HSD_Id               = 0xFFFFFFFF;
	m_MuteAllButX_Id              = 0xFFFFFFFF; //VNGFE-6720
	m_ind_conf                    = 0;
	m_ind_vid_layout              = 0;
	m_pSubject                    = new CSubject;
	m_mainPartiesCounter          = 0;
	m_currentConfCascadeMode	  = CASCADE_MODE_NONE;
	m_isVideoRecoveryStatus       = false;

	CConfPartyProcess* pConfPartyProcess    = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	WORD maxPartiesInConfPerSystemMode      = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
	WORD maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
	m_numParties      = 0;
	m_numVideoParties = 0;
	for (int i = 0; i < MAX_PARTIES_IN_CONF; i++)
	{
		if (m_pParty[i] != NULL)
		{
			delete m_pParty[i];
			m_pParty[i] = NULL;
		}

		if (m_numParties < maxPartiesInConfPerSystemMode)
		{
			CRsrvParty* pParty = (*(CCommConf*)&other).m_pParty[i];
			if (pParty != NULL)
			{
				BYTE isAudioOnlyParty = pParty->GetVoice();
				if (isAudioOnlyParty || (!isAudioOnlyParty && (m_numVideoParties < maxVideoPartiesInConfPerSystemMode)))
				{
					m_pParty[m_numParties] = new CConfParty(*pParty);
					m_pParty[m_numParties]->SetRes(this);
					m_numParties++;
					if (!isAudioOnlyParty)
						m_numVideoParties++;

					CVideoLayout* pVideoLayout = ((CConfParty*)m_pParty[i])->GetVideoLayout();
					if (pVideoLayout)
						pVideoLayout->SetScreenLayout(m_contPresScreenNumber);
				}
			}
		}
	}

	m_downspeed          = NO;
	m_LSDRate            = NO;
	m_curConfVideoLayout = m_contPresScreenNumber;
	m_numVideoLayout     = 0;
	m_MuteAllButX_Id     = 0xFFFFFFFF;

	for (int i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
		m_pVideoLayout[i] = NULL;

	if (m_videoSession != CONTINUOUS_PRESENCE)
		SetCurConfVideoLayout(ONE_ONE);

	m_meetingRoomIsUp        = 0;
	m_EPC_Content_source_Id  = 0xFFFFFFFF;
	m_dwSummaryUpdateCounter = 0;
	m_dwFullUpdateCounter    = 0;
	for (int i = 0; i < 1000; i++)
	{
		m_DeletedCounterHistory[i] = 0;
		m_DeletedIdHistory[i]      = 0;
	}

	m_LastDeletedIndex    = 0;
	m_SipRegTotalSts      = eSipRegistrationTotalStatusTypeNotConfigured;

	m_msConversationId[0] = '\0';
	m_pSavedLectureMode = NULL;
	m_pSavedLecturerVideoLayout = NULL;
	m_pConfRcvMbx = NULL;
	m_COPChairInVC = FALSE;
	m_number_of_active_recording_ports = 0;
	m_bIsTipEnable = FALSE;
	m_IsCandidateForTerminationMR = FALSE;
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
		m_activeSpeakerList[i] = 0xFFFFFFFF;

	m_bIsCallGeneratorConf = FALSE;
	m_CGPartiesCounter = 0;
  m_isEnableHighVideoResInAvcToSvcMixMode = FALSE;
  m_isEnableHighVideoResInSvcToAvcMixMode = FALSE;
  m_isNotifyAvMcuUri = 0;
}

////////////////////////////////////////////////////////////////////////////
CCommConf::~CCommConf()
{
  for (int i = 0; i < MAX_PARTIES_IN_CONF; i++)
    POBJDELETE(m_pParty[i]);

  for (int i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
    POBJDELETE(m_pVideoLayout[i]);

  POBJDELETE(m_pSubject);
  POBJDELETE(m_pSavedLectureMode);
  POBJDELETE(m_pSavedLecturerVideoLayout);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::StartConference(BYTE GMTOffset, BYTE GMTOffsetSign)
{
	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	m_actualStartTime = curTime;
	m_startTime = curTime;
	CCdrShort* cdrShort = new CCdrShort;
	cdrShort->SetH243ConfName(m_H243confName);
	cdrShort->SetConfId(m_confId);
	cdrShort->SetRsrvStrtTime(m_startTime);
	cdrShort->SetRsrvDuration(m_duration);

	cdrShort->SetActualStrtTime(curTime);
	cdrShort->SetActualDuration(m_duration);
	cdrShort->SetStatus(ONGOING_CONFERENCE);

	cdrShort->SetRsrvAudioPartiesNum(m_minNumAudioParties); //VNGR-9263
	cdrShort->SetRsrvVideoPartiesNum(m_minNumVideoParties);
	cdrShort->SetGMTOffset(GMTOffset);
	cdrShort->SetGMTOffsetSign(GMTOffsetSign);

	/*set gmt*/
	CCdrLogApi cdrApi;
	STATUS status = cdrApi.StartConference(*cdrShort);

	CConfStart confStart;
	confStart.SetStandBy(m_stand_by);
	confStart.SetAutoTerminate(m_automaticTermination);
	confStart.SetConfTransfRate(m_confTransferRate);
	confStart.SetRestrictMode(28);
	confStart.SetAudioRate(m_audioRate);
	confStart.SetVideoSession(m_videoSession);
	confStart.SetVideoPicFormat(m_videoPictureFormat);
	confStart.SetCifFrameRate(m_CIFframeRate);
	confStart.SetQcifFrameRate(m_QCIFframeRate);
	confStart.SetLsdRate(m_LSDRate);

	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(CONFERENCE_START);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetConferenceStart(&confStart);
	cdrApi.ConferenceEvent(m_confId, cdrEvent);

	CConfStartCont1* confStartCont1 = new CConfStartCont1;
	confStartCont1->SetAudioTone(GetAudioTone());
	confStartCont1->SetTalkHoldTime(GetTalkHoldTime());
	confStartCont1->SetAudioMixDepth(GetAudioMixDepth());
	confStartCont1->SetOperatorConf(GetOperatorConf());
	confStartCont1->SetVideoProtocol(GetVideoProtocol());
	confStartCont1->SetMeetMePerConf(GetMeetMePerConf());
	confStartCont1->SetConf_password(GetH243Password());
	confStartCont1->SetCascadeMode(GetCascadeMode());
	confStartCont1->SetNumUndefParties(GetNumUndefParties());
	confStartCont1->SetTime_beforeFirstJoin(GetTimeBeforeFirstJoin());
	confStartCont1->SetTime_afterLastQuit(GetTimeAfterLastQuit());
	confStartCont1->SetConfLockFlag(GetConfLockFlag());
	confStartCont1->SetMax_parties(GetMaxParties());
	confStartCont1->SetAvMsgStruct(*GetpAvMsgStruct());
	confStartCont1->SetLectureMode(*GetLectureMode());

	//CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(CONFERENCE_START_CONTINUE_1);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetConfStartCont1(confStartCont1);

	cdrApi.ConferenceEvent(m_confId, cdrEvent);

	CConfStartCont4* confStartCont4 = new CConfStartCont4;
	confStartCont4->SetNumericConfId(GetNumericConfId());
	confStartCont4->SetUser_password(GetEntryPassword());
	confStartCont4->SetChair_password(GetH243Password());
	confStartCont4->SetConfBillingInfo(GetBillingInfo());
	const char* pStrContactInfo;
	for (int j = 0; j < MAX_CONF_INFO_ITEMS; j++)
	{
		pStrContactInfo = NULL;
		pStrContactInfo = GetConfContactInfo(j);
		if (pStrContactInfo)
			confStartCont4->SetContactInfo(pStrContactInfo, j);
	}

	cdrEvent.SetCdrEventType(CONFERENCE_START_CONTINUE_4);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetConfStartCont4(confStartCont4);
	POBJDELETE(confStartCont4);

	cdrApi.ConferenceEvent(m_confId, cdrEvent);

	/////////////////////////////////////////////////
	//CONFERENCE_START_CONTINUE_5
	CConfStartCont5* confStartCont5 = new CConfStartCont5;
	confStartCont5->SetIsEncryptedConf(GetIsEncryption());

	cdrEvent.SetCdrEventType(CONFERENCE_START_CONTINUE_5);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetConfStartCont5(confStartCont5);
	POBJDELETE(confStartCont5);

	cdrApi.ConferenceEvent(m_confId, cdrEvent);

	/////////////////////////////////////////////////
	//CONFERENCE_START_CONTINUE_10
	CConfStartCont10* confStartCont10 = new CConfStartCont10;
	confStartCont10->SetConfDisplayName(GetDisplayName());
	confStartCont10->SetAvcSvc(m_confMediaType);

	cdrEvent.SetCdrEventType(CONFERENCE_START_CONTINUE_10);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetConfStartCont10(confStartCont10);
	POBJDELETE(confStartCont10);

	cdrApi.ConferenceEvent(m_confId, cdrEvent);

	for (int i = 0; i < (int)m_numParties; i++)
	{
		COperAddParty* rsrvStart = new COperAddParty;
		CCdrEvent* pCdrEventParty = new CCdrEvent;
		rsrvStart->SetPartyName(m_pParty[i]->GetName());
		rsrvStart->SetPartyId(m_pParty[i]->GetPartyId());
		rsrvStart->SetConnectionType(m_pParty[i]->GetConnectionTypeOper());
		rsrvStart->SetNetNumberChannel(m_pParty[i]->GetNetChannelNumber());
		rsrvStart->SetNetServiceName((char*)(m_pParty[i]->GetServiceProviderName()));
		rsrvStart->SetVoice(m_pParty[i]->GetVoice());
		rsrvStart->SetNumType(m_pParty[i]->GetNumType());
		rsrvStart->SetNetSubServiceName((char*)(m_pParty[i]->GetSubServiceName()));
		rsrvStart->SetIdentMethod(m_pParty[i]->GetIdentificationMethod());
		rsrvStart->SetMeetMeMethod(m_pParty[i]->GetMeet_me_method());
		Phone* pPhoneNum = m_pParty[i]->GetFirstCallingPhoneNumber();
		while (pPhoneNum != NULL)
		{
			rsrvStart->AddPartyPhoneNumber(pPhoneNum->phone_number);
			pPhoneNum = m_pParty[i]->GetNextCallingPhoneNumberOper();
		}
		pPhoneNum = m_pParty[i]->GetFirstCalledPhoneNumber();
		while (pPhoneNum != NULL)
		{
			rsrvStart->AddMcuPhoneNumber(pPhoneNum->phone_number);
			pPhoneNum = m_pParty[i]->GetNextCalledPhoneNumber();
		}

		pCdrEventParty->SetCdrEventType(RESERVED_PARTY);
		pCdrEventParty->SetTimeStamp(curTime);
		pCdrEventParty->SetAddReservUpdatParty(rsrvStart);
		cdrApi.ConferenceEvent(m_confId, *pCdrEventParty);
		POBJDELETE(pCdrEventParty);

		OperatorAddPartyCont1(m_pParty[i], (m_pParty[i]->GetServiceProviderName()), RESERVED_PARTY_CONTINUE_1);
		// IpV6
		OperatorIpV6PartyCont1(m_pParty[i], (m_pParty[i]->GetServiceProviderName()), RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS);

		OperatorAddPartyCont2(m_pParty[i], RESERVED_PARTY_CONTINUE_2);

		UpdateUserDefinedInformation(m_pParty[i]);
		OperatorAddPartyEventToCdr(m_pParty[i], (m_pParty[i]->GetServiceProviderName()), eOperatorAddPartyAction_OperatorAddPartyToReservedConference);
		POBJDELETE(rsrvStart);
	}

	/////////////////////////////////////////////////
	//CONF_CORRELATION_DATA
	CConfCorrelationData* pCConfCorrelationData = new CConfCorrelationData;
	pCConfCorrelationData->SetSigUuid(GetCorrelationId());

	cdrEvent.SetCdrEventType(CONF_CORRELATION_DATA);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetCConfCorrelationData(pCConfCorrelationData);

	cdrApi.ConferenceEvent(m_confId, cdrEvent);
	POBJDELETE(pCConfCorrelationData);

	PlcmCdrEventConfBeginExtended confBeginExtended;
	std::ostringstream confIdStr;
	confIdStr << m_confId;
	confBeginExtended.m_confId = confIdStr.str();
	confBeginExtended.m_standBy = m_stand_by;
	confBeginExtended.m_autoTerminate.m_on = m_automaticTermination;
	confBeginExtended.m_transferRate = ConvertTransferRateType(m_confTransferRate);
	confBeginExtended.m_restrictMode = eRestrictModeType_Derestricted;
	confBeginExtended.m_videoSession = ConvertVideoSessionType(m_videoSession);
	confBeginExtended.m_videoFormat = ConvertVideoFormatType(m_videoPictureFormat);
	confBeginExtended.m_qcifFrameRate = ConvertVideoFormatRateType(m_QCIFframeRate);
	confBeginExtended.m_talkHoldTime = GetTalkHoldTime();
	confBeginExtended.m_audioMixDepth = GetAudioMixDepth();
	confBeginExtended.m_operatorConf = GetOperatorConf();
	confBeginExtended.m_videoProtocol = ConvertVideoProtocolType(GetVideoProtocol());
	confBeginExtended.m_meetMePerConf.m_on = GetMeetMePerConf();
	confBeginExtended.m_password = GetH243Password();
	confBeginExtended.m_cascade.m_cascadeRole = (CascadeRole)GetCascadeMode();
	confBeginExtended.m_cascade.m_cascadeLinksNumber = 0;
	confBeginExtended.m_cascade.m_linkType = eLinkType_Regular;
	confBeginExtended.m_timeBeforeFirstJoin = GetTimeBeforeFirstJoin();
	confBeginExtended.m_timeAfterLastQuit = GetTimeAfterLastQuit();
	WORD maxParties = GetMaxParties();
	if (maxParties == 0xFFFF)
	{
		confBeginExtended.m_maxParties = "automatic";
	}
	else
	{
		std::ostringstream maxPartiesStr;
		maxPartiesStr << maxParties;
		confBeginExtended.m_maxParties = maxPartiesStr.str();
	}

	confBeginExtended.m_avMessage = GetpAvMsgStruct()->GetAvMsgServiceName();
	CLectureModeParams* lectureMod = GetLectureMode();
	confBeginExtended.m_lectureMode.m_audioActivated = lectureMod->m_audioActivated;
	confBeginExtended.m_lectureMode.m_interval = lectureMod->m_TimeInterval;
	confBeginExtended.m_lectureMode.m_lectureName = lectureMod->m_LecturerName;
	confBeginExtended.m_lectureMode.m_on = lectureMod->m_LectureModeOnOff;
	confBeginExtended.m_lectureMode.m_timer = lectureMod->m_timerOnOff;
	confBeginExtended.m_numericId = GetNumericConfId();
	confBeginExtended.m_entryPassword = GetEntryPassword();
	confBeginExtended.m_chairPassword = GetH243Password();
	confBeginExtended.m_billingData = GetBillingInfo();
	//confBeginExtended.m_contactInfoList.m_additionalInfo = NULL;
	pStrContactInfo = NULL;
	pStrContactInfo = GetConfContactInfo(0);
	if (pStrContactInfo)
		confBeginExtended.m_contactInfoList.m_contactInfo = pStrContactInfo;
	pStrContactInfo = NULL;
	pStrContactInfo = GetConfContactInfo(1);
	if (pStrContactInfo)
		confBeginExtended.m_contactInfoList.m_contactInfo2 = pStrContactInfo;
	pStrContactInfo = NULL;
	pStrContactInfo = GetConfContactInfo(2);
	if (pStrContactInfo)
		confBeginExtended.m_contactInfoList.m_contactInfo3 = pStrContactInfo;
	pStrContactInfo = NULL;
	pStrContactInfo = GetConfContactInfo(3);
	if (pStrContactInfo)
		confBeginExtended.m_contactInfoList.m_contactInfo4 = pStrContactInfo;

	confBeginExtended.m_encryption = (Encryption)GetEncryptionType();
	confBeginExtended.m_displayName = GetDisplayName();

	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&confBeginExtended);
	////////////////////////
	POBJDELETE(cdrShort);
	POBJDELETE(confStartCont1);
	return;
}
Connection CCommConf::ConvertConnectionType(BYTE ConnectionType)
{
	switch (ConnectionType)
	{
		case DIAL_IN:
		{
			return 	eConnection_DialIn;
			break;
		}
		case DIAL_OUT:
		{
			return 	eConnection_DialOut;
			break;
		}

		default:
		{
			return eConnection_DEFAULT ;
			break;
		}
	}
}

Bonding CCommConf::ConvertBondingType(BYTE BondingType)
{
	switch (BondingType)
	{
		case 0xFF:
		{
			return 	eBonding_Auto;
			break;
		}
		case 1:
		{
			return 	eBonding_Enabled;
			break;
		}
		case 0:
		{
			return 	eBonding_Disabled;
			break;
		}

		default:
		{
			return eBonding_DEFAULT;
			break;
		}
	}
}
NetworkLineType CCommConf::ConvertNumType(BYTE numType)
{
	switch (numType)
	{
		case UNKNOWN:
		{
			return 	eNetworkLineType_Unknown;
			break;
		}
		case INTERNATIONAL_TYPE:
		{
			return 	eNetworkLineType_International;
			break;
		}
		case NATIONAL_TYPE:
		{
			return 	eNetworkLineType_National;
			break;
		}
		case NETWORK_SPECIFIC_TYPE:
		{
			return 	eNetworkLineType_NetworkSpecific;
			break;
		}
		case SUBSCRIBER_TYPE:
		{
			return 	eNetworkLineType_Subscriber;
			break;
		}
		case ABBREVIATED_TYPE:
		{
			return 	eNetworkLineType_Abbreviated;
			break;
		}
		case 0xFF:
		{
			return 	eNetworkLineType_TakenFromService;
			break;
		}
		default:
		{
			return eNetworkLineType_DEFAULT;
			break;
		}
	}
}


VideoProtocolType CCommConf::ConvertVideoProtocolType(BYTE videoProtocolType)
{
	switch(videoProtocolType)
	{
		case VIDEO_PROTOCOL_H261:
		{
			return 	eVideoProtocolType_H261;
			break;
		}
		case VIDEO_PROTOCOL_H263:
		{
			return 	eVideoProtocolType_H263;
			break;
		}
		case VIDEO_PROTOCOL_H26L:
		{
			return 	eVideoProtocolType_H26L;
			break;
		}
		case VIDEO_PROTOCOL_H264:
		{
			return 	eVideoProtocolType_H264;
			break;
		}
		case VIDEO_PROTOCOL_RTV:
		{
			return 	eVideoProtocolType_Rtv;
			break;
		}
		case AUTO:
		{
			return 	eVideoProtocolType_Auto;
			break;
		}
		case VIDEO_PROTOCOL_H264_HIGH_PROFILE:
		{
			return 	eVideoProtocolType_H264HighProfile;
			break;
		}
		default:
		{
			return eVideoProtocolType_DEFAULT;
			break;
		}
	}
}
FrameRateType CCommConf::ConvertVideoFormatRateType(BYTE formatRateType)
{
	switch(formatRateType)
	{
		case V_1_29_97:
		{
			return 	eFrameRateType_30;
			break;
		}
		case V_2_29_97:
		{
			return 	eFrameRateType_15;
			break;
		}
		case V_3_29_97:
		{
			return 	eFrameRateType_10;
			break;
		}
		case V_4_29_97:
		{
			return 	eFrameRateType_75;
			break;
		}
		case 255:
		{
			return 	eFrameRateType_Auto;
			break;
		}
		default:
		{
			return eFrameRateType_DEFAULT;
			break;
		}
	}
}
VideoFormatType CCommConf::ConvertVideoFormatType(BYTE formatType)
{
	switch(formatType)
	{
		case V_Qcif:
		{
			return 	eVideoFormatType_Qcif;
			break;
		}
		case V_Cif:
		{
			return 	eVideoFormatType_Cif;
			break;
		}
		case H263_CIF_4:
		{
			return 	eVideoFormatType_4cif;
			break;
		}
		case H263_CIF_16:
		{
			return 	eVideoFormatType_16cif;
			break;
		}
		case kVGA:
		{
			return 	eVideoFormatType_Vga;
			break;
		}
		case kSVGA:
		{
			return 	eVideoFormatType_Svga;
			break;
		}
		case kXGA:
		{
			return 	eVideoFormatType_Xga;
			break;
		}
		case kNTSC:
		{
			return 	eVideoFormatType_Ntsc;
			break;
		}
		case 255:
		{
			return 	eVideoFormatType_Auto;
			break;
		}
		default:
		{
			return eVideoFormatType_DEFAULT;
			break;
		}
	}
}
MeetMeMethodeType CCommConf::ConvertMeetMeMethod(BYTE meetMeType)
{
	switch(meetMeType)
	{
		case 1:
		{
			return 	eMeetMeMethodeType_McuConference;
			break;
		}
		case 3:
		{
			return  eMeetMeMethodeType_Party;
			break;
		}
		case 4:
		{
			return 	eMeetMeMethodeType_Channel;
			break;
		}
		default:
		{
			return eMeetMeMethodeType_Unknown;
			break;
		}
	}
}
VideoSessionType CCommConf::ConvertVideoSessionType(BYTE sessionType)
{
	switch(sessionType)
	{
		case VIDEO_SWITCH:
		{
			return 	eVideoSessionType_Switching;
			break;
		}
		case CONTINUOUS_PRESENCE:
		{
			return 	eVideoSessionType_ContinuousPresence;
			break;
		}
		case VIDEO_SESSION_COP:
		{
			return 	eVideoSessionType_Cop;
			break;
		}
		default:
		{
			return eVideoSessionType_DEFAULT;
			break;
		}
	}
}

AliasType CCommConf::ConvertAliasType(BYTE AliasType)
{
	switch(AliasType)
		{
			case ALIAS_TYPE_ENUM:
			{
				return 	eAliasType_None;
				break;
			}
			case PARTY_H323_ALIAS_H323_ID_TYPE:
			{
				return 	eAliasType_323Id;
				break;
			}
			case PARTY_H323_ALIAS_E164_TYPE:
			{
				return 	eAliasType_E164;
				break;
			}
			case PARTY_H323_ALIAS_URL_ID_TYPE:
			{
				return 	eAliasType_UrlId;
				break;
			}

			case PARTY_H323_ALIAS_TRANSPORT_ID_TYPE:
			{
				return 	eAliasType_TransportId;
				break;
			}
			case PARTY_H323_ALIAS_EMAIL_ID_TYPE:
			{
				return 	eAliasType_EmailId;
				break;
			}
			case PARTY_H323_ALIAS_PARTY_NUMBER_TYPE:
			{
				return 	eAliasType_PartyNumber;
				break;
			}
			default:
			{
				return eAliasType_DEFAULT;
				break;
			}
		}
}
TransferRateType CCommConf::ConvertTransferRateType(BYTE transferType)
{
	switch(transferType)
	{
		case Xfer_64:
		{
			return 	eTransferRateType_64;
			break;
		}
		case Xfer_2x64:
		{
			return 	eTransferRateType_128;
			break;
		}
		case Xfer_3x64:
		{
			return 	eTransferRateType_192;
			break;
		}
		case Xfer_4x64:
		{
			return 	eTransferRateType_256;
			break;
		}
		case  Xfer_5x64:
		{
			return 	eTransferRateType_320;
			break;
		}
		case Xfer_6x64:
		{
			return 	eTransferRateType_384;
			break;
		}
		case Xfer_384:
		{
			return 	eTransferRateType_384;
			break;
		}
		case Xfer_2x384:
		{
			return 	eTransferRateType_768;
			break;
		}
		case Xfer_3x384:
		{
			return 	eTransferRateType_1152;
			break;
		}
		case Xfer_4x384:
		{
			return 	eTransferRateType_1536;
			break;
		}
		case Xfer_5x384:
		{
			return 	eTransferRateType_1920;
			break;
		}
		case Xfer_1536:
		{
			return 	eTransferRateType_1536;
			break;
		}
		case Xfer_1920:
		{
			return 	eTransferRateType_1920;
			break;
		}
		case Xfer_128:
		{
			return 	eTransferRateType_128;
			break;
		}
		case Xfer_192:
		{
			return 	eTransferRateType_192;
			break;
		}
		case Xfer_256:
		{
			return 	eTransferRateType_256;
			break;
		}
		case Xfer_320:
		{
			return 	eTransferRateType_320;
			break;
		}
		case Xfer_512:
		{
			return 	eTransferRateType_512;
			break;
		}
		case Xfer_768:
		{
			return 	eTransferRateType_768;
			break;
		}
		case Xfer_1152:
		{
			return 	eTransferRateType_1152;
			break;
		}
		case Xfer_1472:
		{
			return 	eTransferRateType_1472;
			break;
		}
		case Xfer_96:
		{
			return 	eTransferRateType_96;
			break;
		}
		case Xfer_1024:
		{
			return 	eTransferRateType_1024;
			break;
		}
		case Xfer_4096:
		{
			return 	eTransferRateType_4096;
			break;
		}
		case Xfer_6144:
		{
			return 	eTransferRateType_6144;
			break;
		}
		case Xfer_832:
		{
			return 	eTransferRateType_832;
			break;
		}
		case Xfer_1728:
		{
			return 	eTransferRateType_1728;
			break;
		}
		case Xfer_2048:
		{
			return 	eTransferRateType_2048;
			break;
		}
		case Xfer_1280:
		{
			return 	eTransferRateType_1280;
			break;
		}
		case Xfer_2560:
		{
			return 	eTransferRateType_2560;
			break;
		}
		case Xfer_3072:
		{
			return 	eTransferRateType_3072;
			break;
		}
		case Xfer_3584:
		{
			return 	eTransferRateType_3584;
			break;
		}
		case Xfer_8192:
		{
			return 	eTransferRateType_8192;
			break;
		}
		default:
		{
			return eTransferRateType_DEFAULT;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetCurrentConfCascadeMode(BYTE currentConfCascadeMode)
{
	// valid cascade mode types
	if(currentConfCascadeMode == CASCADE_MODE_NONE || currentConfCascadeMode == CASCADE_MODE_MASTER || currentConfCascadeMode == CASCADE_MODE_SLAVE)
		m_currentConfCascadeMode = currentConfCascadeMode;
}


////////////////////////////////////////////////
void  CCommConf::EndConference(BYTE cause)
{
	  TRACEINTO << "CCommConf::EndConference";
  CStructTm curTime;
  PASSERT(SystemGetTime(curTime));

  DWORD diffTime;
  diffTime = curTime - m_actualStartTime;
  int hour = diffTime/3600;
  int min = (diffTime%3600)/60;
  int sec = (diffTime%3600)%60;
  CStructTm actualDuration(0,0,0,hour,min,sec);

  CCdrEvent cdrEvent;
  cdrEvent.SetCdrEventType(CONFERENCES_END);
  cdrEvent.SetTimeStamp(curTime);
  cdrEvent.SetConfEndCause((eConfCdrStatus)cause);

  CCdrLogApi cdrApi;
  STATUS status = cdrApi.EndConference(m_confId, (eConfCdrStatus)cause, m_actualStartTime, actualDuration, cdrEvent);

  //send new event to CDR Manger
  PlcmCdrEventConfEnd ConfEndEvent;
  std::ostringstream confIdStr;
  confIdStr << m_confId;
  ConfEndEvent.m_confId = confIdStr.str();//m_confId;
  ConfEndEvent.m_bridgeConference = true;
  ConfEndEvent.m_confEndCause =(ConfEndCauseEnum)cause ;
  SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfEndEvent);

  return;
}

STATUS  CCommConf::SendCdrEvendToCdrManager(ApiBaseObjectPtr ConfEndEvent, bool isConference, DWORD monitoryPartyId, std::string pCorrelationId) const
{
	EventData eventData;
	eventData.m_pEventData = ConfEndEvent;
	PlcmCdrEvent  plcmCdrEvent;
	CStructTm curTime;
	CConfParty* pConfParty = GetCurrentParty(monitoryPartyId);
	STATUS res = STATUS_OK;

	if(isConference == true)
	{
		plcmCdrEvent.m_entityType = "Conference";
		plcmCdrEvent.m_correlationId = GetCorrelationId();
		// TRACEINTO << "SendCdrEvendToCdrManager::GetCorrelationId conf = " <<GetCorrelationId();
	}
	else
	{
		plcmCdrEvent.m_entityType = "Call";
		//CConfParty* pConfParty = GetCurrentParty(monitoryPartyId);
		if (pConfParty)
		{
			if(pConfParty->IsTIPSlaveParty() || pConfParty->IsTIPAuxParty())
			{
					return res;
		    }
			plcmCdrEvent.m_correlationId = pConfParty->GetCorrelationId();
			PASSERT(SystemGetTime(curTime));
		}
		else if(pCorrelationId.length() != 0)
		{
			plcmCdrEvent.m_correlationId = pCorrelationId;
		}
		else
		{
			TRACEINTO << "SendCdrEvendToCdrManager:: else pCorrelationId is empty";
		}
	}
	plcmCdrEvent.m_eventData = eventData;
	plcmCdrEvent.m_eventTimestamp = GetGmtTime().c_str();
	CManagerApi cdrMngrApi(eProcessCDR);
	CSegment* seg = new CSegment;
						*seg << plcmCdrEvent;
	res  = cdrMngrApi.SendMsg(seg, CDR_PERSISTENCE_QUEUE_ADD_CDR);

	return res;
}
////////////////////////////////////////////////////////////////////////////
std::string CCommConf::GetGmtTime() const
{
	CStructTm curTime;
	SystemGetTime(curTime);
	time_t currTime = curTime.GetAbsTime(true);
	tm* pCurrGmtTimeTm;

	// time(&currTime);
	pCurrGmtTimeTm = gmtime(&currTime);
	char yearSt[16];
	char monthSt[16];
	char daySt[16];
	char hourSt[16];
	char minSt[16];
	char secSt[16];
	sprintf(yearSt,"%2d",curTime.m_year);
	sprintf(monthSt,"%02d",(pCurrGmtTimeTm->tm_mon + 1));
	sprintf(daySt,"%02d",pCurrGmtTimeTm->tm_mday);

	sprintf(hourSt,"%02d",pCurrGmtTimeTm->tm_hour);
	sprintf(minSt,"%02d",pCurrGmtTimeTm->tm_min);
	sprintf(secSt,"%02d",pCurrGmtTimeTm->tm_sec);

	std::string timeRet = std::string(yearSt) + "-" + std::string(monthSt) + "-" + std::string(daySt) + "T" + std::string(hourSt) + ":" + std::string(minSt) + ":" + std::string(secSt) + ".0Z";
	return timeRet;

}
std::string CCommConf::GetTimeStr(CStructTm curTime)
{

	char yearSt[16];
	char monthSt[16];
	char daySt[16];
	char hourSt[16];
	char minSt[16];
	char secSt[16];
	sprintf(yearSt,"%2d",curTime.m_year);
	sprintf(monthSt,"%02d",(curTime.m_mon));
	sprintf(daySt,"%02d",curTime.m_day);

	sprintf(hourSt,"%02d",curTime.m_hour);
	sprintf(minSt,"%02d",curTime.m_min);
	sprintf(secSt,"%02d",curTime.m_sec);

	std::string timeRet = std::string(yearSt) + "-" + std::string(monthSt) + "-" + std::string(daySt) + "T" + std::string(hourSt) + ":" + std::string(minSt) + ":" + std::string(secSt) + ".0Z";
	return timeRet;

}
////////////////////////////////////////////////////////////////////////////
void CCommConf::NetChnnelConnToCDR(char* party_name,DWORD party_id,BYTE channel_id,WORD numChan,WORD dialType,CIsdnNetSetup* NetSetUp,char* PhoneNum)
{
  ALLOCBUFFER(callingPhone, PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);
  ALLOCBUFFER(calledPhone, PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER);

  CStructTm curTime;
  PASSERT(SystemGetTime(curTime));

  CCallingParty*  callingParty= new CCallingParty  ;
  callingParty->SetCallingNumType(NetSetUp->m_calling.m_numType);
  callingParty->SetCallingNumPlan(NetSetUp->m_calling.m_numPlan);
  callingParty->SetCallingPresentInd(NetSetUp->m_calling.m_presentationInd);
  callingParty->SetCallingScreenInd(NetSetUp->m_calling.m_screeningInd);
  memcpy ((char *)callingPhone,
	  (char *)(NetSetUp->m_calling.m_digits),
	  NetSetUp->m_calling.m_numDigits);
  if (NetSetUp->m_calling.m_numDigits <= PRI_LIMIT_PHONE_DIGITS_LEN)
	  callingPhone[NetSetUp->m_calling.m_numDigits] = '\0';
  else
	  callingPhone[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';

  callingParty->SetCallingPhoneNum(callingPhone);

  CCalledParty*  calledParty= new CCalledParty  ;
  calledParty->SetCalledNumType(NetSetUp->m_called.m_numType);
  calledParty->SetCalledNumPlan(NetSetUp->m_called.m_numPlan);

  memcpy ((char *)calledPhone,
	  (char *)(NetSetUp->m_called.m_digits),
	  NetSetUp->m_called.m_numDigits);
  if (NetSetUp->m_called.m_numDigits <= PRI_LIMIT_PHONE_DIGITS_LEN)
	  calledPhone[NetSetUp->m_called.m_numDigits] = '\0';
  else
	  calledPhone[PRI_LIMIT_PHONE_DIGITS_LEN] = '\0';

  calledParty->SetCalledPhoneNum(calledPhone);

  CNetChanlCon* netChanlCon = new CNetChanlCon;
  netChanlCon->SetPartyName(party_name);
  netChanlCon->SetPartyId(party_id);
  netChanlCon-> SetChanlNum(numChan);
  netChanlCon->SetChanlId(channel_id);
  netChanlCon->SetConctIniator(netChanlCon->ConvertConnectionTypeToConnectInitiatorType(dialType));
  netChanlCon->SetNetSpec(NetSetUp->m_netSpcf);
  netChanlCon->SetCallingParty(*callingParty);
  netChanlCon->SetCalledParty(*calledParty);

  CCdrEvent cdrEvent;
  cdrEvent.SetCdrEventType(NET_CHANNEL_CONNECTED);
  cdrEvent.SetTimeStamp(curTime);
  cdrEvent.SetNetChanlConnect(netChanlCon);
  CCdrLogApi cdrApi;
  STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

  PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
  ConfUserDataUpdateEvent.m_netChannelConnect.m_name = party_name;
  ConfUserDataUpdateEvent.m_netChannelConnect.m_partyId = party_id;
  ConfUserDataUpdateEvent.m_netChannelConnect.m_channelNum = numChan;
  ConfUserDataUpdateEvent.m_netChannelConnect.m_channelId = channel_id;
  ConfUserDataUpdateEvent.m_netChannelConnect.m_initiator = (InitiatorType)netChanlCon->ConvertConnectionTypeToConnectInitiatorType(dialType);
  ConfUserDataUpdateEvent.m_netChannelConnect.m_callType = eCallType_Voice;//(CallType)NetSetUp->m_callType
  ConfUserDataUpdateEvent.m_netChannelConnect.m_netSpecific = (NetSpecific)NetSetUp->m_netSpcf;
  ConfUserDataUpdateEvent.m_netChannelConnect.m_calledParty.m_numType = ConvertNumType(calledParty->GetCalledNumType());
  ConfUserDataUpdateEvent.m_netChannelConnect.m_calledParty.m_phone1 = calledParty->GetCalledPhoneNum();
  ConfUserDataUpdateEvent.m_netChannelConnect.m_calledParty.m_planType = (CdrNumPlanType)calledParty->GetCalledNumPlan();

  ConfUserDataUpdateEvent.m_netChannelConnect.m_callingParty.m_numType = ConvertNumType(callingParty->GetCallingNumType());
  ConfUserDataUpdateEvent.m_netChannelConnect.m_callingParty.m_phone1 = callingParty->GetCallingPhoneNum();
  ConfUserDataUpdateEvent.m_netChannelConnect.m_callingParty.m_planType = (CdrNumPlanType)callingParty->GetCallingNumPlan();
  ConfUserDataUpdateEvent.m_netChannelConnect.m_callingParty.m_presentationIndicator = (PresentationIndicator)callingParty->GetCallingPresentInd();
  ConfUserDataUpdateEvent.m_netChannelConnect.m_callingParty.m_screenIndicator = (ScreenIndicator)callingParty->GetCallingScreenInd();

  SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, party_id);

  POBJDELETE(calledParty);
  POBJDELETE(callingParty);
  POBJDELETE(netChanlCon);
  DEALLOCBUFFER(callingPhone);
  DEALLOCBUFFER(calledPhone);
  return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::NetChnnelDisconnToCDR(char* party_name,DWORD party_id,BYTE channelNumber,BYTE discoInitiator,BYTE code_stndrd,BYTE location,BYTE cause_value,WORD event)
{
		  CStructTm curTime;
		  PASSERT(SystemGetTime(curTime));

		  ACCCDREventDisconnectCause* p931_cause = new ACCCDREventDisconnectCause;

		  p931_cause->SetDisconctCauseCodeStandrd(code_stndrd);
		  p931_cause->SetDisconctCauseLocation(location);
		  p931_cause->SetDisconctCauseValue(cause_value);

		  CNetChannelDisco* pNetChannelDisco =new CNetChannelDisco;

		   pNetChannelDisco->SetNetDiscoPartyName(party_name);
		   pNetChannelDisco->SetNetDiscoPartyId( party_id);
		   pNetChannelDisco->SetNetDiscoChannelId(channelNumber);
		   pNetChannelDisco->SetNetDiscoInitiator(discoInitiator);
		   pNetChannelDisco->SetNetDiscoQ931(*(p931_cause));

		  CCdrEvent cdrEvent;
		  cdrEvent.SetCdrEventType(event);
		  cdrEvent.SetTimeStamp(curTime);
		  cdrEvent.SetNetChanlDisconnect(pNetChannelDisco);
		  CCdrLogApi cdrApi;
  		  STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);


  		  PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
  		  ConfUserDataUpdateEvent.m_netChannelDisconnect.m_name = party_name;
  		  ConfUserDataUpdateEvent.m_netChannelDisconnect.m_partyId = party_id;
  		  ConfUserDataUpdateEvent.m_netChannelDisconnect.m_channelId = channelNumber;
  		  ConfUserDataUpdateEvent.m_netChannelDisconnect.m_initiator = (InitiatorType)discoInitiator;
  		  SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, party_id);

		  POBJDELETE(p931_cause);
		  POBJDELETE(pNetChannelDisco);
		return;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::OperatorAddParty(CRsrvParty* pConfParty, const char* operName, WORD event)
{
		  CStructTm curTime;
		  PASSERT(SystemGetTime(curTime));

		  CCdrEvent cdrEvent;
		  COperAddParty* rsrvStart = new COperAddParty;
		  rsrvStart->SetOperatorName(operName);
		  rsrvStart->SetPartyName(pConfParty->GetName());
		  rsrvStart->SetPartyId(pConfParty->GetPartyId());
		  rsrvStart->SetConnectionType(pConfParty->GetConnectionTypeOper());
		  rsrvStart->SetNetNumberChannel(pConfParty->GetNetChannelNumber());
		  rsrvStart->SetNetServiceName((char*)(pConfParty->GetServiceProviderName()));
		  rsrvStart->SetVoice(pConfParty->GetVoice());
		  rsrvStart->SetNumType(pConfParty->GetNumType());
		  rsrvStart->SetNetSubServiceName((char*)(pConfParty->GetSubServiceName()));
		  rsrvStart->SetIdentMethod(pConfParty->GetIdentificationMethod());
		  rsrvStart->SetMeetMeMethod(pConfParty->GetMeet_me_method());
		  Phone* pPhoneNum = pConfParty->GetFirstCallingPhoneNumber();
		  while (pPhoneNum != NULL) {
		     rsrvStart->AddPartyPhoneNumber(pPhoneNum->phone_number);
		     pPhoneNum = pConfParty->GetNextCallingPhoneNumberOper();
		  }
		  pPhoneNum = pConfParty->GetFirstCalledPhoneNumber();
		  while (pPhoneNum != NULL) {
		     rsrvStart->AddMcuPhoneNumber(pPhoneNum->phone_number);
		     pPhoneNum = pConfParty->GetNextCalledPhoneNumber();
		  }

		  cdrEvent.SetCdrEventType(event);
		  cdrEvent.SetTimeStamp(curTime);
		  cdrEvent.SetAddReservUpdatParty( rsrvStart );

		  CCdrLogApi cdrApi;
		  cdrApi.ConferenceEvent(m_confId, cdrEvent);
		  POBJDELETE(rsrvStart);
	return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::OperatorAddPartyCont1(CRsrvParty* pConfParty, const char* operName, WORD event)
{
	   CStructTm curTime;
 	   PASSERT(SystemGetTime(curTime));

		CCdrEvent cdrEvent;
		COperAddPartyCont1* rsrvStartCont1 = new COperAddPartyCont1;
		rsrvStartCont1->SetNodeType(pConfParty->GetNodeType ());
		rsrvStartCont1->SetPassword(pConfParty->GetPassword ());
		// IpV6
		if ((pConfParty->GetIpAddress ()).ipVersion == eIpVersion4)
			rsrvStartCont1->SetIpAddress((pConfParty->GetIpAddress ()).addr.v4.ip);
		else
			rsrvStartCont1->SetIpAddress(0xFFFFFFFF);
		rsrvStartCont1->SetCallSignallingPort(pConfParty->GetCallSignallingPort ());
		rsrvStartCont1->SetVideoProtocol(pConfParty->GetVideoProtocol ());
		rsrvStartCont1->SetVideoRate(pConfParty->GetVideoRate ());
		ACCCdrPhone cdrPhone;
		cdrPhone.phone_number = pConfParty->GetPhoneNumber();
		rsrvStartCont1->SetBondingPhoneNumber(&cdrPhone);

		BYTE interfaceType = pConfParty->GetNetInterfaceType();
		if (H323_INTERFACE_TYPE == interfaceType)
		{
			rsrvStartCont1->SetIpPartyAliasType(pConfParty->GetH323PartyAliasType ());
			rsrvStartCont1->SetIpPartyAlias(pConfParty->GetH323PartyAlias ());
		}
		else if (SIP_INTERFACE_TYPE == interfaceType)
		{
			rsrvStartCont1->SetSipPartyAddressType(pConfParty->GetSipPartyAddressType());
			rsrvStartCont1->SetSipPartyAddress(pConfParty->GetSipPartyAddress());
		}

		rsrvStartCont1->SetAudioVolume(pConfParty->GetAudioVolume ());
		rsrvStartCont1->SetUndefinedType(pConfParty->GetUndefinedType ());
		rsrvStartCont1->SetNetInterfaceType(pConfParty->GetNetInterfaceType ());

		cdrEvent.SetCdrEventType(event);
		cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetOperAddPartyCont1(rsrvStartCont1);

 		CCdrLogApi cdrApi;
  		STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);
		POBJDELETE(rsrvStartCont1);
	return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::OperatorAddPartyCont2(CRsrvParty* pConfParty, WORD event, PlcmCdrEventCallStartExtendedHelper* pCdrEventCallStartExtendedHelper)
{
	  CStructTm curTime;
	  PASSERT(SystemGetTime(curTime));

	  COperAddPartyCont2* rsrvStartCont2 = new COperAddPartyCont2;
	  rsrvStartCont2->SetIsEncryptedParty(pConfParty->GetIsEncrypted());
	  rsrvStartCont2->SetPartyName(pConfParty->GetName());
	  rsrvStartCont2->SetPartyId(pConfParty->GetPartyId());

	  CCdrEvent cdrEvent;
	  cdrEvent.SetCdrEventType(event);
	  cdrEvent.SetTimeStamp(curTime);
	  cdrEvent.SetOperAddPartyCont2(rsrvStartCont2);

	  CCdrLogApi cdrApi;
	  STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

	  bool toDelAndSendHelper = false;

	  POBJDELETE(rsrvStartCont2);
}

void  CCommConf::OperatorAddPartyEventToCdr(CRsrvParty* pConfParty, const char* operName, OperatorAddPartyAction pAction)
{
	PlcmCdrEventConfOperatorAddParty eventOperatorAddParty;
	eventOperatorAddParty.m_action = pAction;
	std::ostringstream confIdStr;
	confIdStr << m_confId;
	eventOperatorAddParty.m_confId = confIdStr.str();
	eventOperatorAddParty.m_operatorName = operName;
	eventOperatorAddParty.m_name = pConfParty->GetName();
	eventOperatorAddParty.m_id = pConfParty->GetPartyId();
	eventOperatorAddParty.m_connection=   (ConnectionType)pConfParty->GetConnectionTypeOper();
	eventOperatorAddParty.m_netChannelNumber=  (NetChannelNumType)pConfParty->GetNetChannelNumber();
	eventOperatorAddParty.m_serviceName = (char*)(pConfParty->GetServiceProviderName());
	eventOperatorAddParty.m_numType = ConvertNumType(pConfParty->GetNumType());
	eventOperatorAddParty.m_subServiceName =  (char*)pConfParty->GetSubServiceName();
	eventOperatorAddParty.m_identificationMethod = (IdentMethodType)pConfParty->GetIdentificationMethod();
	eventOperatorAddParty.m_meetMeMethod = ConvertMeetMeMethod(pConfParty->GetMeet_me_method());
	Phone* pPhoneNum = pConfParty->GetFirstCallingPhoneNumber();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_phoneList.m_phone1 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetNextCallingPhoneNumberOper();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_phoneList.m_phone2 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetNextCallingPhoneNumberOper();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_phoneList.m_phone3 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetNextCallingPhoneNumberOper();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_phoneList.m_phone4 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetNextCallingPhoneNumberOper();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_phoneList.m_phone5 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetFirstCalledPhoneNumber();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_phoneList.m_phone6 = pPhoneNum->phone_number;

		/////////////
		pPhoneNum = pConfParty->GetNextCalledPhoneNumber();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_mcuPhoneList.m_phone1 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetNextCalledPhoneNumber();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_mcuPhoneList.m_phone2 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetNextCalledPhoneNumber();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_mcuPhoneList.m_phone3 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetNextCalledPhoneNumber();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_mcuPhoneList.m_phone4 = pPhoneNum->phone_number;

		pPhoneNum = pConfParty->GetNextCalledPhoneNumber();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_mcuPhoneList.m_phone5 = pPhoneNum->phone_number;

		pPhoneNum =pConfParty->GetNextCalledPhoneNumber();
		if (pPhoneNum != NULL)
			eventOperatorAddParty.m_mcuPhoneList.m_phone6 = pPhoneNum->phone_number;

		eventOperatorAddParty.m_nodeType = (NodeType)pConfParty->GetNodeType ();
		eventOperatorAddParty.m_password = pConfParty->GetPassword ();
		// IpV6
		if ((pConfParty->GetIpAddress ()).ipVersion == eIpVersion4)
		{
			char tempName[64];
			memset (&tempName,'\0',IPV6_ADDRESS_LEN);
			ipToString(pConfParty->GetIpAddress(),tempName,1);

			eventOperatorAddParty.m_ipAddress = tempName;
		}
		else
		{
			eventOperatorAddParty.m_ipAddress = '\0';
		}

		eventOperatorAddParty.m_signallingPort = pConfParty->GetCallSignallingPort ();
		eventOperatorAddParty.m_videoProtocol = ConvertVideoProtocolType(GetVideoProtocol());
		eventOperatorAddParty.m_videoBitRate = CCdrPersistConverter::ConvertBitRate(pConfParty->GetVideoRate ());
		eventOperatorAddParty.m_bondingPhone = pConfParty->GetPhoneNumber();

		BYTE interfaceType = pConfParty->GetNetInterfaceType();

		if (H323_INTERFACE_TYPE == interfaceType)
		{
			eventOperatorAddParty.m_interface = eInterface_H323;
			eventOperatorAddParty.m_alias.m_aliasType =  ConvertAliasType(pConfParty->GetH323PartyAliasType ());
			eventOperatorAddParty.m_alias.m_name  = pConfParty->GetH323PartyAlias ();

		}
		else if (SIP_INTERFACE_TYPE == interfaceType)
		{
			eventOperatorAddParty.m_interface = eInterface_SIP;
			eventOperatorAddParty.m_alias.m_aliasType = ConvertAliasType(pConfParty->GetSipPartyAddressType());
			eventOperatorAddParty.m_alias.m_name = pConfParty->GetSipPartyAddress();
		}

		eventOperatorAddParty.m_volume = pConfParty->GetAudioVolume ();
		eventOperatorAddParty.m_undefined = pConfParty->GetUndefinedType ();
		eventOperatorAddParty.m_encryption = (BoolAutoType)GetIsEncryption();

		if (pConfParty->GetIpAddress().ipVersion == eIpVersion6)
		{
			mcTransportAddress trAddr;
			memset(&trAddr,0,sizeof(mcTransportAddress));
			memcpy(&(trAddr.addr.v6),(void*)&pConfParty->GetIpAddress().addr.v6,sizeof(ipAddressV6If));
			trAddr.ipVersion = eIpVersion6;
			char tempName[64];
			memset (&tempName,'\0',64);
			ipToString(trAddr,tempName,1);
			eventOperatorAddParty.m_ipv6Address = tempName;

		}

		SendCdrEvendToCdrManager((ApiBaseObjectPtr)&eventOperatorAddParty);

}
////////////////////////////////////////////////////////////////////////////
void CCommConf::OperatorIpV6PartyCont1(CRsrvParty* pConfParty, const char* operName, WORD event)
{
	   CStructTm curTime;
 	   PASSERT(SystemGetTime(curTime));

		CCdrEvent cdrEvent;
		COperIpV6PartyCont1* rsrvStartCont1 = new COperIpV6PartyCont1;

		mcTransportAddress trAddr;
		memset(&trAddr,0,sizeof(mcTransportAddress));
		if ((pConfParty->GetIpAddress().ipVersion) == eIpVersion6)
		{
			  memcpy(&(trAddr.addr.v6),(void*)&pConfParty->GetIpAddress().addr.v6,sizeof(ipAddressV6If));
			  trAddr.ipVersion = eIpVersion6;
		}
		char tempName[64];
		memset (&tempName,'\0',64);
		ipToString(trAddr,tempName,1);
		rsrvStartCont1->SetIpV6Address(tempName);

		cdrEvent.SetCdrEventType(event);
		cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetOperIpV6PartyCont1(rsrvStartCont1);

 		CCdrLogApi cdrApi;
  		STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);
		POBJDELETE(rsrvStartCont1);
	return;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::UpdateUserDefinedInformation(CRsrvParty* pConfParty)
{
	   CStructTm curTime;
 	   PASSERT(SystemGetTime(curTime));

		CUpdateUserDefinedInfo* pUpdateUserDefinedInfo = new CUpdateUserDefinedInfo;
		const char*  pStrUserInfo;

		for(int i = 0 ; i < MAX_USER_INFO_ITEMS ; i++)
		{
			pStrUserInfo = NULL;

			pStrUserInfo = pConfParty->GetUserDefinedInfo(i);
			if(pStrUserInfo)
			{
			   pUpdateUserDefinedInfo->SetUserDefinedInfo(pStrUserInfo,i);
			}
		}

		CCdrEvent cdrEvent;
		cdrEvent.SetCdrEventType(USER_DEFINED_INFORMATION);
		cdrEvent.SetTimeStamp(curTime);

		cdrEvent.SetUserDefinedInfo(pUpdateUserDefinedInfo);
		POBJDELETE(pUpdateUserDefinedInfo);

 		CCdrLogApi cdrApi;
 		STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

	return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::IpChnnelConnToCDR(char* party_name,DWORD party_id,BYTE connectionStatus,CIpNetSetup* pNetSetup,DWORD callType)
{
	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	CCdrLogApi cdrApi;
	PlcmCdrEventCallStart evnetCallStart;

	//evnetCallStart.m_nativeCallid;
	evnetCallStart.m_partyId = party_id;
	evnetCallStart.m_name = party_name;

	CIpChanlCon* pIpChanlCon = new CIpChanlCon;
	pIpChanlCon->SetPartyName(party_name);
	pIpChanlCon->SetPartyId(party_id);

	if (callType == DIAL_OUT)
	{
		pIpChanlCon->SetConctIniator(MCU_INITIATOR);
		evnetCallStart.m_direction = eDirection_OUTBOUND;
	}
	else
	{
		pIpChanlCon->SetConctIniator(REMOTE_PARTY_INITIATOR);
		evnetCallStart.m_direction = eDirection_INBOUND;
	}

	pIpChanlCon->SetIpMaxRate(pNetSetup->GetMaxRate());
	pIpChanlCon->SetIpMinRate(pNetSetup->GetMinRate());
	pIpChanlCon->SetIpEndPointType(pNetSetup->GetEndpointType());

	evnetCallStart.m_negotiatedBandwidth = pNetSetup->GetMaxRate();
	evnetCallStart.m_hostAppServer = "MCU";
	evnetCallStart.m_endPointType = (EndPointType)pNetSetup->GetEndpointType();

	WORD event;

	if (SIP_INTERFACE_TYPE == connectionStatus)
	{
		if (callType == DIAL_OUT)
		{
			pIpChanlCon->SetSrcPartyAddress(((CSipNetSetup*)pNetSetup)->GetLocalSipAddress());
			pIpChanlCon->SetDstPartyAddress(pNetSetup->GetDestPartyAddress());
			evnetCallStart.m_sourceUri = ((CSipNetSetup*)pNetSetup)->GetLocalSipAddress();
			evnetCallStart.m_destinationUri = pNetSetup->GetDestPartyAddress();
		}
		else
		{
			pIpChanlCon->SetSrcPartyAddress(((CSipNetSetup*)pNetSetup)->GetRemoteSipAddress());
			pIpChanlCon->SetDstPartyAddress(((CSipNetSetup*)pNetSetup)->GetLocalSipAddress());
			evnetCallStart.m_sourceUri = ((CSipNetSetup*)pNetSetup)->GetRemoteSipAddress();
			evnetCallStart.m_destinationUri = ((CSipNetSetup*)pNetSetup)->GetLocalSipAddress();
		}

		event = SIP_CALL_SETUP;
		evnetCallStart.m_signallingType = eSignallingType_SIP;
	}
	else
	{
		char 	tempSrcPartyAddress[IP_LIMIT_ADDRESS_CHAR_LEN];
		char 	tempDestPartyAddress[IP_LIMIT_ADDRESS_CHAR_LEN];

		strncpy(tempSrcPartyAddress,pNetSetup->GetSrcPartyAddress(),sizeof(tempSrcPartyAddress) - 1);
		tempSrcPartyAddress[sizeof(tempSrcPartyAddress) - 1] = '\0';

		strncpy(tempDestPartyAddress,pNetSetup->GetDestPartyAddress(),sizeof(tempDestPartyAddress) - 1);
		tempDestPartyAddress[sizeof(tempDestPartyAddress) - 1] = '\0';

		char * pSrcAdd = strtok(tempSrcPartyAddress,",");
		char * pDstAdd = strtok(tempDestPartyAddress,",");

		if (pSrcAdd)
		{
			pIpChanlCon->SetSrcPartyAddress(pSrcAdd);
			evnetCallStart.m_sourceUri = pSrcAdd;
		}
		else
		{
			pIpChanlCon->SetSrcPartyAddress("");
			evnetCallStart.m_sourceUri = "";
		}

		if (pDstAdd)
		{
			pIpChanlCon->SetDstPartyAddress(pDstAdd);
			evnetCallStart.m_destinationUri = pDstAdd;
		}
		else
		{
			pIpChanlCon->SetDstPartyAddress("");
			evnetCallStart.m_destinationUri = "";
		}

		event = H323_CALL_SETUP;
		evnetCallStart.m_signallingType = eSignallingType_H323;
	}

	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(event);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetIpChanlConnect(pIpChanlCon);

	STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

	CConfParty* pConfParty = GetCurrentParty(party_id);

	if (pConfParty)
	{
		evnetCallStart.m_nativeCallid = pConfParty->GetCorrelationId();

	}

	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&evnetCallStart, false, party_id);

	POBJDELETE(pIpChanlCon);

	return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::PartyCorrelationDataToCDR(const char* party_name,DWORD party_id, std::string sig_uuid)
{
		CStructTm curTime;
 	    PASSERT(SystemGetTime(curTime));

 	   TRACEINTO << "CPartyCorrelationData::PartyCorrelationDataToCDR sig_uuid= " << sig_uuid;
		CCdrLogApi cdrApi;
		WORD event = PARTY_CORRELATION_DATA;
		CPartyCorrelationData* pCPartyCorrelationData = new CPartyCorrelationData;
		pCPartyCorrelationData->SetPartyName(party_name);
		pCPartyCorrelationData->SetPartyId(party_id);
		pCPartyCorrelationData->SetSigUuid(sig_uuid.c_str());
		TRACEINTO << " party_name= " <<pCPartyCorrelationData->GetPartyName()<< " party_id=" << pCPartyCorrelationData->GetPartyId()<< " sig_uuid=" << pCPartyCorrelationData->GetSigUid();

		CCdrEvent cdrEvent;
		cdrEvent.SetCdrEventType(event);
		cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetCPartyCorrelationData(pCPartyCorrelationData);

		STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

		POBJDELETE(pCPartyCorrelationData);
	return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::BillingCodeToCDR( DWORD confID, const char* partyName, DWORD partyRsrcID, const char* pBillingCode )
{
		CStructTm curTime;
 	    PASSERT(SystemGetTime(curTime));

		CPartyAddBillingCode* pPartyAddBillingCode = new CPartyAddBillingCode;
		pPartyAddBillingCode->SetPartyName(partyName);
		pPartyAddBillingCode->SetPartyId( partyRsrcID);
		pPartyAddBillingCode->SetBillingData(pBillingCode);

		CCdrEvent cdrEvent;
		cdrEvent.SetCdrEventType(EVENT_PARTY_ADD_BILLING_CODE);
		cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetPartyBillingCode(pPartyAddBillingCode);

		CCdrLogApi cdrApi;
		STATUS status = cdrApi.ConferenceEvent(confID, cdrEvent);

		//Send BILLING to CdrManager
		PlcmCdrEventConfDataUpdate confDataUpdate;
		confDataUpdate.m_id = partyRsrcID;
		confDataUpdate.m_name = partyName;
		confDataUpdate.m_billingData = pBillingCode;
		SendCdrEvendToCdrManager((ApiBaseObjectPtr)&confDataUpdate);

		POBJDELETE(pPartyAddBillingCode);
	return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::PartyDtmfFailureIndicationToCDR( DWORD confID, const char* partyName,
												 DWORD partyMonitorID, const char* dtmfCode,
												 const char* rightData, DWORD failureState )
{
		CStructTm curTime;
 	    PASSERT(SystemGetTime(curTime));

 	  	CCDRPartyDTMFfailureIndication* pPartyDTMFfailureIndication = new CCDRPartyDTMFfailureIndication;
	   	pPartyDTMFfailureIndication->SetPartyName(partyName);
	   	pPartyDTMFfailureIndication->SetPartyId( partyMonitorID);
	   	pPartyDTMFfailureIndication->SetDTMFcode(dtmfCode);
	   	pPartyDTMFfailureIndication->SetRightData(rightData);
	   	pPartyDTMFfailureIndication->SetFailureStatus(failureState);

	  	CCdrEvent cdrEvent;
	  	cdrEvent.SetCdrEventType(DTMF_CODE_FAILURE);
	  	cdrEvent.SetTimeStamp(curTime);
	  	cdrEvent.SetDTMFfailureInd(pPartyDTMFfailureIndication);

	  	CCdrLogApi cdrApi;
	  	STATUS status = cdrApi.ConferenceEvent(confID, cdrEvent);
	  	PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;

	  	ConfUserDataUpdateEvent.m_dtmfCodeFailure.m_name = partyName;
	  	ConfUserDataUpdateEvent.m_dtmfCodeFailure.m_id = partyMonitorID;
	  	ConfUserDataUpdateEvent.m_dtmfCodeFailure.m_dtmfCode = dtmfCode;
	  	ConfUserDataUpdateEvent.m_dtmfCodeFailure.m_rightData = rightData;
	  	ConfUserDataUpdateEvent.m_dtmfCodeFailure.m_failureState = (FailureState)failureState;

	  	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, partyMonitorID);
	  	POBJDELETE(pPartyDTMFfailureIndication);
	return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::ControlRecordingCDR( WORD recordingControl, CRsrvParty *pConfParty )
{
	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	CCDRPartyRecording* pPartyRecording = new CCDRPartyRecording();

	pPartyRecording->SetPartyName(pConfParty->GetName());
	pPartyRecording->SetOpType( recordingControl );
	pPartyRecording->SetPartyId( pConfParty->GetPartyId());
	pPartyRecording->SetOpInitiator(0);
	pPartyRecording->SetRecordingLinkName( GetRecLinkName() );
	pPartyRecording->SetStartRecPolicy( GetStartRecPolicy() );

	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(RECORDING_LINK_EVENT);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetRecording(pPartyRecording);

	CCdrLogApi cdrApi;
	STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

	//Send RECORDING_LINK_EVENT to CdrManager
	PlcmCdrEventConfRecord eventConfRecord;
	std::ostringstream confIdStr;
	confIdStr << m_confId;
	eventConfRecord.m_confId = confIdStr.str();
	eventConfRecord.m_name = pConfParty->GetName();
	eventConfRecord.m_partyId = pConfParty->GetPartyId();
	eventConfRecord.m_startRecordingPolicy = (StartRecordingPolicy)GetStartRecPolicy();
	eventConfRecord.m_recLinkId = 0;
	eventConfRecord.m_recLinkName = GetRecLinkName();
	eventConfRecord.m_recordingStatus = (RecordingStatus)recordingControl;
	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&eventConfRecord);

	POBJDELETE(pPartyRecording);
}

////////////////////////////////////////////////////////////////////////////


void CCommConf::PartyConnectDisconnectToCDR(CConfParty *pConfParty, PlcmCdrEventDisconnectedExtendedHelper* cdrEventDisconnectedExtendedHelper)
{
		CStructTm curTime;
 	    PASSERT(SystemGetTime(curTime));
 	    DWORD partyState = pConfParty->GetPartyState();
 	    if ((partyState == PARTY_DISCONNECTED)||(partyState == PARTY_REDIALING))//VNGFE-2014 in case the party disconnected and now we start to redial, in the CDR we need to reflect that the party disconnected
		{
			CPartyDisconnected partyDisconnected;
			partyDisconnected.SetPartyName(pConfParty->GetName());
			partyDisconnected.SetPartyId(pConfParty->GetPartyId());
			if (pConfParty->GetDisconnectCause() != 0xFFFFFFFF)
				partyDisconnected.SetDisconctCause(pConfParty->GetDisconnectCause());
			else
				partyDisconnected.SetDisconctCause(0);

			// MCU_INTERNAL_PROBLEM
			// In case of mcu internal problem we will add the error number instead of Q931.
			if (pConfParty->GetDisconnectCause() == MCU_INTERNAL_PROBLEM)
				partyDisconnected.SetQ931DisonctCause(pConfParty->GetMipErrorNumber());
			else
				partyDisconnected.SetQ931DisonctCause(pConfParty->GetQ931DisconnectCause());
		  	CCdrEvent cdrEvent;
		  	cdrEvent.SetCdrEventType(EVENT_PARTY_DISCONNECTED);
		  	cdrEvent.SetTimeStamp(curTime);
		  	cdrEvent.SetPartyDisconnect(&partyDisconnected);

		  	CCdrLogApi cdrApi;
		  	STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

		  	if (cdrEventDisconnectedExtendedHelper )
		  	{
		  		 cdrEventDisconnectedExtendedHelper->SetDataFromPartyDisconnected(partyDisconnected);
		  	}

		}
		else
		{
			CCdrEvent cdrEvent;
			CPartyConnected partyConnected;
			partyConnected.SetPartyName(pConfParty->GetName());
			partyConnected.SetPartyId(pConfParty->GetPartyId());
			partyConnected.SetSecondaryCause(0);
			BYTE bIpType = pConfParty->GetNetInterfaceType();
			// VNGR-6981
			if (pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM)
				partyConnected.SetPartyState(PARTY_CONNECTED);
			else
				partyConnected.SetPartyState(pConfParty->GetPartyState());

			PlcmEventConnectedHelper cdrEventConnectedHelper;

			// When SIP will be added we can move this code down
			switch(bIpType)
			{
				case H323_INTERFACE_TYPE:
				{
				   cdrEvent.SetCdrEventType(IP_PARTY_CONNECTED);
				   cdrEvent.SetTimeStamp(curTime);
				   cdrEvent.SetPartyConnect(&partyConnected);

				   cdrEventConnectedHelper.SetDataPartyConnected(bIpType, partyConnected);
				   SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventConnectedHelper.GetCdrObject(), false, cdrEventConnectedHelper.GetCdrObject().m_partyId, pConfParty->GetCorrelationId());

				   break;
				}
				case SIP_INTERFACE_TYPE:
				{

			  		cdrEvent.SetCdrEventType(SIP_PARTY_CONNECTED);
			  		cdrEvent.SetTimeStamp(curTime);
				    cdrEvent.SetPartyConnect(&partyConnected);

					cdrEventConnectedHelper.SetDataPartyConnected(bIpType, partyConnected);

				   SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventConnectedHelper.GetCdrObject(), false, cdrEventConnectedHelper.GetCdrObject().m_partyId, pConfParty->GetCorrelationId());


					break;
				}
				case ISDN_INTERFACE_TYPE:
				{
					cdrEvent.SetCdrEventType(EVENT_PARTY_CONNECTED);
					cdrEvent.SetTimeStamp(curTime);
				    cdrEvent.SetPartyConnect(&partyConnected);
					cdrEventConnectedHelper.SetDataPartyConnected(bIpType, partyConnected);
				    SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventConnectedHelper.GetCdrObject(), false, cdrEventConnectedHelper.GetCdrObject().m_partyId, pConfParty->GetCorrelationId());

					break;
				}
				default:
				{
				    PASSERTMSG(bIpType,"CCommConf::PartyConnectDisconnectToCDR - Wrong network interface ");
					break;
				}
			}
			CCdrLogApi cdrApi;
			STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);
		}
}


void CCommConf::SvcSipPartyConnectCDR(CConfParty *pConfParty, std::list<SvcStreamDesc>* pStreams, ECodecSubType eAudioCodec, DWORD dwBitRateOut, DWORD dwBitRateIn)
{
	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));
	DWORD partyState = pConfParty->GetPartyState();
//	TRACEINTO << "BG_CDR  partyState = " << partyState << "  MediaType = " << (DWORD)(pConfParty->GetPartyMediaType());
	if (partyState != PARTY_DISCONNECTED && partyState != PARTY_REDIALING)
	{
		if (pConfParty->GetPartyMediaType() == eSvcPartyType)
		{
			CCdrEvent cdrEvent;
			CSvcSipPartyConnected partyConnected;
			partyConnected.SetPartyName(pConfParty->GetName());
			partyConnected.SetPartyId(pConfParty->GetPartyId());
			partyConnected.SetPartyState(pConfParty->GetPartyState());
			partyConnected.SetStreams(pStreams);
			partyConnected.SetAudioCodec((DWORD)eAudioCodec);
			partyConnected.SetBitRateOut(dwBitRateOut);
			partyConnected.SetBitRateIn(dwBitRateIn);
//			TRACEINTO << "BG_CDR\n\tPartyMonitorID = " << partyConnected.GetPartyId()
//					<< "\n\tPartyState = " << partyConnected.GetPartyState()
//					<< "\n\tStreams count = " << (int)(partyConnected.GetStreams()->size())
//					<< "\n\tAudioCodec = " << (DWORD)eAudioCodec
//					<< "\n\tBitRateOut = " << dwBitRateOut << "\n\tBitRateIn = " << dwBitRateIn;

			cdrEvent.SetCdrEventType(SVC_SIP_PARTY_CONNECTED);
			cdrEvent.SetTimeStamp(curTime);
			cdrEvent.SetSvcSipPartyConnect(&partyConnected);

			CCdrLogApi cdrApi;
			STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

			PlcmEventConnectedHelper cdrEventConnectedHelper;
			cdrEventConnectedHelper.SetDataSvcPartyConnected(SIP_INTERFACE_TYPE, partyConnected);
			SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventConnectedHelper.GetCdrObject(), false, cdrEventConnectedHelper.GetCdrObject().m_partyId);

		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::GkInfoToCDR(const char *pPartyName, const DWORD partyId, BYTE* gkCallId)
{
		CStructTm curTime;
 	    PASSERT(SystemGetTime(curTime));

		CGkInfo gkInfo;
		gkInfo.SetPartyName(pPartyName);
		gkInfo.SetPartyId(partyId);
		gkInfo.SetGkCallId(gkCallId);
	  	CCdrEvent cdrEvent;
	  	cdrEvent.SetCdrEventType(GK_INFO);
	  	cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetGkInfo(&gkInfo);

	  	CCdrLogApi cdrApi;
	  	STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

	  	PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
	  	ConfUserDataUpdateEvent.m_gatekeeperInformation.m_name = pPartyName;
	  	ConfUserDataUpdateEvent.m_gatekeeperInformation.m_id = partyId;
	  //	ConfUserDataUpdateEvent.m_gatekeeperInformation.m_gkCallId = *gkCallId;
	  	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, partyId);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::NewRateInfoToCdr(const char *pPartyName, const DWORD partyId, const DWORD currentRate)
{
	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	CNewRateInfo NewRateInfo;
	NewRateInfo.SetPartyName(pPartyName);
	NewRateInfo.SetPartyId(partyId);
	NewRateInfo.SetPartyCurrentRate(currentRate);
	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(PARTY_NEW_RATE);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetNewRateInfo(&NewRateInfo);

	PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
	ConfUserDataUpdateEvent.m_participantConnectionRate.m_name = pPartyName;
	ConfUserDataUpdateEvent.m_participantConnectionRate.m_partyId = partyId;
	ConfUserDataUpdateEvent.m_participantConnectionRate.m_participantCurrentRate = currentRate;

	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, partyId);

	CCdrLogApi cdrApi;
	STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::CallInfoPerPartyAfterDisconnectionToCdr(const char *pPartyName,
		const DWORD partyId, const DWORD maxBitRate,
		const char *maxResolution, const char *maxFrameRate,const char *address)
		//const char *maxResolution, const WORD maxFrameRate,const char *address)
{
	CStructTm curTime;
	CCdrEvent cdrEvent;
	PASSERT(SystemGetTime(curTime));
	CCallInfo CallInfo;
	CallInfo.SetPartyName(pPartyName);
	CallInfo.SetPartyId(partyId);
	CallInfo.SetPartyMaxBitRate(maxBitRate);
	CallInfo.SetPartyMaxFrameRate(maxFrameRate);
	CallInfo.SetPartyMaxResolution(maxResolution);
	CallInfo.SetPartyAddress(address);
	cdrEvent.SetCdrEventType(PARTICIPANT_MAX_USAGE_INFO);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetCallInfo(&CallInfo);
	CCdrLogApi cdrApi;
	STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

	 PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
	 ConfUserDataUpdateEvent.m_participantMaxUsage.m_name = pPartyName;
	 ConfUserDataUpdateEvent.m_participantMaxUsage.m_partyId = partyId;
	 ConfUserDataUpdateEvent.m_participantMaxUsage.m_maxRate = maxBitRate;
	 ConfUserDataUpdateEvent.m_participantMaxUsage.m_maxFrameRate = maxFrameRate;
	 ConfUserDataUpdateEvent.m_participantMaxUsage.m_maxResolution = maxResolution;
	 ConfUserDataUpdateEvent.m_participantMaxUsage.m_address = address;

  	 SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, partyId);
}

void CCommConf::SipPrivateExtensionsToCDR( const char *pPartyName,	const DWORD	partyId, const char *pCalledPartyID,  const char *pAssertedIdentity, const char *pChargingVector, const char *pPreferredIdentity )
{
	CStructTm curTime;
    PASSERT(SystemGetTime(curTime));

	// set of the private extention object
	CCDRSipPrivateExtensions* pSipPrivateExtensions = new CCDRSipPrivateExtensions();
	pSipPrivateExtensions->SetPartyName(pPartyName);
	pSipPrivateExtensions->SetPartyId(partyId);
	pSipPrivateExtensions->SetPCalledPartyId( pCalledPartyID );
	pSipPrivateExtensions->SetPAssertedIdentity( pAssertedIdentity );
	pSipPrivateExtensions->SetPChargingVector( pChargingVector );
	pSipPrivateExtensions->SetPPreferredIdentity( pPreferredIdentity );

	// set CDR event
	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(SIP_PRIVATE_EXTENSIONS_EVENT);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetSipPrivateExtensions(pSipPrivateExtensions);
  	CCdrLogApi cdrApi;
  	STATUS status = cdrApi.ConferenceEvent(m_confId, cdrEvent);

	POBJDELETE(pSipPrivateExtensions);
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::OperatorTerminate(const char* operName)
{
		  CStructTm curTime;
	 	  PASSERT(SystemGetTime(curTime));

		  CCdrEvent cdrEvent;
		  cdrEvent.SetCdrEventType(OPERATOR_TERMINATE);
		  cdrEvent.SetTimeStamp(curTime);
		  cdrEvent.SetOperatorName(operName);

		  CCdrLogApi cdrApi;
		  cdrApi.ConferenceEvent(m_confId, cdrEvent);

		  PlcmCdrEventOperatorConfActivity eventOperatorConfActivity;
		  std::ostringstream confIdStr;
		  confIdStr << m_confId;
		  eventOperatorConfActivity.m_confId = confIdStr.str();
		  eventOperatorConfActivity.m_operatorAction = eOperatorConfActivityAction_TerminateConference;
		  eventOperatorConfActivity.m_operatorName = operName;
		  SendCdrEvendToCdrManager((ApiBaseObjectPtr)&eventOperatorConfActivity);


	  return;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::OperatorPartyAction(const char* pPartyName, DWORD partyId, const char* operName,
									 WORD event)
{
		  CStructTm curTime;
	 	  PASSERT(SystemGetTime(curTime));

		  CCdrEvent cdrEvent;
		  COperDelParty partyAct;
		  partyAct.SetOperatorName(operName);
		  partyAct.SetPartyName(pPartyName);
		  partyAct.SetPartyId(partyId);
		  cdrEvent.SetCdrEventType(event);
		  cdrEvent.SetTimeStamp(curTime);
		  cdrEvent.SetDelDisconctReconctParty(&partyAct);

		  CCdrLogApi cdrApi;
		  cdrApi.ConferenceEvent(m_confId, cdrEvent);

		  if(event == OPERATOR_DISCONNECTE_PARTY || event == OPERATOR_RECONNECT_PARTY)
			  OperatorPartyActionEventToCdr(pPartyName,  partyId, operName, event);

	  return;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::OperatorPartyActionEventToCdr(const char* pPartyName, DWORD partyId, const char* operName,
		WORD event)
{
	PlcmCdrEventOperatorPartyActivity eventOperatorPartyActivity;

	eventOperatorPartyActivity.m_operatorName = operName;
	eventOperatorPartyActivity.m_partyName = pPartyName;
	eventOperatorPartyActivity.m_partyId = partyId;
	switch (event)
	{
		case OPERATOR_DISCONNECTE_PARTY:
		{
			eventOperatorPartyActivity.m_operatorAction = eOperatorPartyActivityAction_DisconnectParty;

			break;
		}
		case OPERATOR_RECONNECT_PARTY:
		{
			eventOperatorPartyActivity.m_operatorAction = eOperatorPartyActivityAction_ReconnectParty;
			break;
		}
	}
	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&eventOperatorPartyActivity, false, partyId);

	return;
}
void CCommConf::OperatorSetEndTime(const CStructTm &newTime, const char* operName)
{
		  CStructTm curTime;
	 	  PASSERT(SystemGetTime(curTime));

		  CCdrEvent cdrEvent;
		  COperSetEndTime operEndTime;
		  operEndTime.SetOperatorName(operName);
		  operEndTime.SetNewEndTime(newTime);
		  cdrEvent.SetCdrEventType(OPERATOR_SET_END_TIME);
		  cdrEvent.SetTimeStamp(curTime);
		  cdrEvent.SetEndTimeEvent(&operEndTime);

		  CCdrLogApi cdrApi;
		  cdrApi.ConferenceEvent(m_confId, cdrEvent);

		  PlcmCdrEventOperatorConfActivity eventOperatorConfActivity;
		  std::ostringstream confIdStr;
		  confIdStr << m_confId;
		  eventOperatorConfActivity.m_confId = confIdStr.str();
		  eventOperatorConfActivity.m_operatorAction = eOperatorConfActivityAction_SetEndTime;
		  eventOperatorConfActivity.m_operatorName = operName;
		  eventOperatorConfActivity.m_endTime = GetTimeStr(newTime);
		  SendCdrEvendToCdrManager((ApiBaseObjectPtr)&eventOperatorConfActivity);


		  return;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::OperatorSetVisualName(const char* pPartyCurName, DWORD partyId, const char* pPartyPrevName, std::string pCorrelationId)
{
		  CStructTm curTime;
	 	  PASSERT(SystemGetTime(curTime));

		  CCdrEvent cdrEvent;
		  CPartySetVisualName partyVisualName;
		  partyVisualName.SetPartyName(pPartyPrevName);
		  partyVisualName.SetPartyId(partyId);
		  partyVisualName.SetVisualName(pPartyCurName);

		  cdrEvent.SetCdrEventType(EVENT_SET_PARTY_VISUAL_NAME);
		  cdrEvent.SetTimeStamp(curTime);
		  cdrEvent.SetPartyVisualName(&partyVisualName);

		  CCdrLogApi cdrApi;
		  cdrApi.ConferenceEvent(m_confId, cdrEvent);


		  if(pCorrelationId.length() != 0)
		  {
			  PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
			  ConfUserDataUpdateEvent.m_visualName.m_name = pPartyPrevName;
			  ConfUserDataUpdateEvent.m_visualName.m_id = partyId;
			  ConfUserDataUpdateEvent.m_visualName.m_visualName = pPartyCurName;
			  SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, partyId, pCorrelationId);
		  }



	  return;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::PartyDisconnectCont1(WORD sumL_syncLost, WORD sumR_syncLost,
									  WORD sumL_videoSyncLost, WORD sumR_videoSyncLost, PlcmCdrEventDisconnectedExtendedHelper* cdrEventDisconnectedExtendedHelper)
{
		    CStructTm curTime;
	 	    PASSERT(SystemGetTime(curTime));

		    CCdrEvent cdrEvent;
		    cdrEvent.SetTimeStamp(curTime);
		    cdrEvent.SetCdrEventType(PARTY_DISCONNECTED_CONTINUE_1);

		    CPartyDisconnectedCont1 partyDisconnectedCont1;

		    partyDisconnectedCont1.SetL_syncLostCounter(sumL_syncLost);
		    partyDisconnectedCont1.SetR_syncLostCounter(sumR_syncLost);
		    partyDisconnectedCont1.SetL_videoSyncLostCounter(sumL_videoSyncLost);
		    partyDisconnectedCont1.SetR_videoSyncLostCounter(sumR_videoSyncLost);

		    cdrEvent.SetPartyDisconectCont1(&partyDisconnectedCont1);

		 	CCdrLogApi cdrApi;
 	        cdrApi.ConferenceEvent(m_confId, cdrEvent);

 	        if (cdrEventDisconnectedExtendedHelper)
 	        {
 	        	cdrEventDisconnectedExtendedHelper->SetDataFromPartyDisconnected1(partyDisconnectedCont1);

 	        }
	return;

}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::NewUndefinedParty(CConfParty* pConfParty, WORD event)
{
		CStructTm curTime;
	 	PASSERT(SystemGetTime(curTime));

		CCdrEvent cdrEvent;
		CAddPartyDetailed* rsrvStart = new CAddPartyDetailed;

		rsrvStart->SetPartyName(pConfParty->GetName());
		rsrvStart->SetPartyId(pConfParty->GetPartyId());
		rsrvStart->SetConnectionType(pConfParty->GetConnectionTypeOper());
		rsrvStart->SetNetNumberChannel(pConfParty->GetNetChannelNumber());
		rsrvStart->SetNetServiceName((char*)(pConfParty->GetServiceProviderName()));
		rsrvStart->SetVoice(pConfParty->GetVoice());
		rsrvStart->SetNumType(pConfParty->GetNumType());
		rsrvStart->SetNetSubServiceName((char*)(pConfParty->GetSubServiceName()));
		rsrvStart->SetIdentMethod(pConfParty->GetIdentificationMethod());
		rsrvStart->SetMeetMeMethod(pConfParty->GetMeet_me_method());

		WORD indPhone = 0;
		Phone* pPhoneNum = pConfParty->GetActualPartyPhoneNumber(indPhone);
		while (pPhoneNum != NULL) {
					rsrvStart->AddPartyPhoneNumber(pPhoneNum->phone_number);
					indPhone++;
					pPhoneNum = pConfParty->GetActualPartyPhoneNumber(indPhone);
		}
		indPhone = 0;
		pPhoneNum = pConfParty->GetActualMCUPhoneNumber(indPhone);
		while (pPhoneNum != NULL) {
			        rsrvStart->AddMcuPhoneNumber(pPhoneNum->phone_number);
				    indPhone++;
					pPhoneNum = pConfParty->GetActualMCUPhoneNumber(indPhone);
		}
		rsrvStart->SetNodeType(pConfParty->GetNodeType ());
		rsrvStart->SetChair(pConfParty->GetIsLeader());
		if ((pConfParty->GetIpAddress ()).ipVersion == eIpVersion4)
			rsrvStart->SetIpAddress((pConfParty->GetIpAddress ()).addr.v4.ip);
		else
			rsrvStart->SetIpAddress(0xFFFFFFFF);
		rsrvStart->SetCallSignallingPort(pConfParty->GetCallSignallingPort ());
		rsrvStart->SetVideoProtocol(pConfParty->GetVideoProtocol ());
		rsrvStart->SetVideoRate(pConfParty->GetVideoRate ());

		BYTE interfaceType = pConfParty->GetNetInterfaceType();
		if (H323_INTERFACE_TYPE == interfaceType)
		{
			rsrvStart->SetIpPartyAliasType(pConfParty->GetH323PartyAliasType ());
			rsrvStart->SetIpPartyAlias(pConfParty->GetH323PartyAlias ());
		}
		else if (SIP_INTERFACE_TYPE == interfaceType)
		{
			rsrvStart->SetSipPartyAddressType(pConfParty->GetSipPartyAddressType());
			rsrvStart->SetSipPartyAddress(pConfParty->GetSipPartyAddress());
		}

		rsrvStart->SetAudioVolume(pConfParty->GetAudioVolume ());
		rsrvStart->SetUndefinedType(pConfParty->GetUndefinedType ());
		rsrvStart->SetNetInterfaceType(pConfParty->GetNetInterfaceType ());

		cdrEvent.SetCdrEventType(event);
		cdrEvent.SetTimeStamp(curTime);
		cdrEvent.SetAddUnReservUpdatParty( rsrvStart );

		CCdrLogApi cdrApi;
		cdrApi.ConferenceEvent(m_confId, cdrEvent);

		POBJDELETE(rsrvStart);

	return;
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError, const char * action)
{
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SerializeXml(CXMLDOMElement *pActionNode, DWORD ObjToken, ePartyData party_data_amount/*= FULL_DATA*/)
{
	CXMLDOMElement *pConfNode,*pResNode,*pDeletedParitesNode;
	BYTE bChanged=FALSE;
	int ConfOpcode=CONF_COMPLETE_INFO;
	DWORD ResCounter=0xFFFFFFFF;

 	pConfNode=pActionNode->AddChildNode("CONFERENCE");
	pConfNode->AddChildNode("OBJ_TOKEN",m_dwFullUpdateCounter);

	if (ObjToken==0xFFFFFFFF)
		bChanged=TRUE;
	else
	{
		//in case the string can not be converted ResSummeryCounter=0
		//the conferences will be added as if the user sent -1 in the object token .
		ResCounter=ObjToken;
		if(m_dwFullUpdateCounter>ResCounter)
			bChanged=TRUE;

		WORD fastUpdateCounter = m_fastUpdateCounter;

		if (m_fastUpdateCounter>ResCounter)
			ConfOpcode=CONF_FAST_INFO;

		if (m_slowUpdateCounter>ResCounter)
			ConfOpcode=CONF_FAST_PLUS_SLOW_INFO;
		else if (ConfOpcode!=CONF_FAST_INFO)
			ConfOpcode=CONF_NOT_CHANGED;
	}

	pConfNode->AddChildNode("CHANGED",bChanged,_BOOL);

	if (!IsConfSecured())
	{
		pResNode=pConfNode->AddChildNode("RESERVATION");

		AddResXmlToResponse(pResNode,ConfOpcode);
    }
	AddConfXmlToResponse(pConfNode,ConfOpcode);

	AddConfPartiesToResponse(pConfNode,ResCounter, party_data_amount);

	pDeletedParitesNode=pConfNode->AddChildNode("DELETED_PARTIES");

	for (int i=0 ;i<1000;i++)
	{
		if(m_DeletedCounterHistory[i]>ResCounter&&ResCounter!=0xFFFFFFFF)
			pDeletedParitesNode->AddChildNode("PARTY_ID",m_DeletedIdHistory[i]);
	}
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::SerializeShortXml(CXMLDOMElement *pNode,int opcode)
{
	CXMLDOMElement *pResNode,*pChildNode;

	pResNode=pNode->AddChildNode("CONF_SUMMARY");

	if (opcode!=CONF_NOT_CHANGED)
		pResNode->AddChildNode("NAME",m_H243confName);

	pResNode->AddChildNode("ID",m_confId);
	pResNode->AddChildNode("CONF_CHANGE",opcode,CHANGE_STATUS_ENUM);

	if (opcode!=CONF_NOT_CHANGED)
	{
		AddConfStatusToResponse(pResNode);
        GetEndTime();

		pResNode->AddChildNode("START_TIME",m_startTime);
		if( !IsPermanent() ) //from SRS: A permanent conference is a conference that does not have end date and hour
			pResNode->AddChildNode("END_TIME",m_endTime);
		pResNode->AddChildNode("OPERATOR_CONF",m_operatorConf,_BOOL);
		pResNode->AddChildNode("LOCK",m_dwConfFlags&LOCKED,_BOOL);
		pResNode->AddChildNode("LECTURE_CONF",m_dwConfFlags&LECTURE_MODE_SHOW,_BOOL);

		BYTE videoSession = m_videoSession;
        // VIDEO_SESSION vngr-4078 add fields to serialize with fixed value for web-comander/mgc-manager monitoring
		pResNode->AddChildNode("VIDEO_SESSION",m_videoSession,VIDEO_SESSION_ENUM);
		pResNode->AddChildNode("NUM_PARTIES",m_numParties);
		pResNode->AddChildNode("NUM_CONNECTED_PARTIES",m_number_of_connected_parties);
		pResNode->AddChildNode("INVITE_PARTY",m_dwConfFlags&INVITE_PARTICPANT,_BOOL);
		pResNode->AddChildNode("ENTRY_QUEUE",m_dwConfFlags& ENTRY_QUEUE,_BOOL);
		pResNode->AddChildNode("ROLL_CALL",m_dwConfFlags& ROLL_CALL_FLAG,_BOOL);
		pResNode->AddChildNode("SECURE",m_dwConfFlags& SECURED,_BOOL);
		pResNode->AddChildNode("ENTRY_PASSWORD",m_entry_password);
		pResNode->AddChildNode("PASSWORD",m_H243_password);
		if (m_pServicePhoneStr[0])
		{
			Phone* pFirstPhone = m_pServicePhoneStr[0]->GetFirstPhoneNumber();

			if (pFirstPhone)
				pResNode->AddChildNode("MEET_ME_PHONE",pFirstPhone->phone_number);
			else
				pResNode->AddChildNode("MEET_ME_PHONE","");
		}
		else
			pResNode->AddChildNode("MEET_ME_PHONE","");

		if(strcmp(m_NumericConfId,"")!=0)
		  pResNode->AddChildNode("NUMERIC_ID",m_NumericConfId);

		  if(m_pConfContactInfo)
			m_pConfContactInfo->SerializeXml(pResNode);

		pResNode->AddChildNode("AUTO_LAYOUT", m_dwConfFlags&AUTO_LAYOUT_FLAG,_BOOL);

		if( !IsPermanent() )
		{
			pChildNode=pResNode->AddChildNode("DURATION");
			pChildNode->AddChildNode("HOUR",m_duration.m_hour);
			pChildNode->AddChildNode("MINUTE",m_duration.m_min);
			pChildNode->AddChildNode("SECOND",m_duration.m_sec);
		}
		pResNode->AddChildNode("NUM_UNDEFINED_PARTIES",m_numUndefParties);

		if(m_numServicePrefixStr>0)
		  {
		    CXMLDOMElement *pTempNode=pResNode->AddChildNode("DIAL_IN_H323_SRV_PREFIX_LIST");
		    for (int i=0;i<(int)m_numServicePrefixStr;i++)
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
		pResNode->AddChildNode("ENCRYPTION",m_dwConfFlags&ENCRYPTION,_BOOL);
		pResNode->AddChildNode("ENCRYPTION_TYPE",GetEncryptionType(),ENCRYPTION_TYPE_ENUM);
		pResNode->AddChildNode("GATEWAY",m_dwConfFlags&RMX_GATEWAY,_BOOL);
		pResNode->AddChildNode("RECORDING_STATUS",m_RecordingLinkControl,RECORDING_STATUS_ENUM);
		pResNode->AddChildNode("TRANSFER_RATE",m_confTransferRate,TRANSFER_RATE_ENUM);
		pResNode->AddChildNode("HD",m_HD,_BOOL);
		pResNode->AddChildNode("DISPLAY_NAME",m_confDisplayName);
		pResNode->AddChildNode("EPC_CONTENT_SOURCE_ID",m_EPC_Content_source_Id);
		pResNode->AddChildNode("IS_TELEPRESENCE_MODE",m_isTelePresenceMode,_BOOL);
		pResNode->AddChildNode("SIP_REGISTRATIONS_STATUS",m_SipRegistrationTotalSts,SIP_REG_TOTAL_STATUS_ENUM);//sipProxySts
		pResNode->AddChildNode("CONF_MEDIA_TYPE", m_confMediaType, CONF_MEDIA_TYPE_ENUM);	// Conf Media Type (AVC/SVC/Mix/etc.)
		AddConfActiveSpeakersListToXmlResponse( pResNode );	// Active Speaker List
		if(GetisNotifyAvMcuUri())
		{
			pResNode->AddChildNode("AV_MCU_FOCUS_URI",m_FocusUriCurrently, ONE_LINE_BUFFER_LENGTH);
		}
	}
}


////////////////////////////////////////////////////////////////////////////
void  CCommConf::SerializeXmlRelayInfo(CXMLDOMElement *pActionNode, DWORD ObjToken, ePartyData party_data_amount)
{
	CXMLDOMElement *pConfNode,*pResNode,*pDeletedParitesNode;
	BYTE bChanged=FALSE;
	int ConfOpcode=CONF_COMPLETE_INFO;
	DWORD ResCounter=0xFFFFFFFF;

 	pConfNode=pActionNode->AddChildNode("CONFERENCE");

	if (ObjToken==0xFFFFFFFF)
		bChanged=TRUE;
	else
	{
		ResCounter=ObjToken;
		if(m_dwFullUpdateCounter > ResCounter)
			bChanged=TRUE;
	}

	// addinf Conf parameters
	AddConfRelayInfoToResponse( pConfNode );
	// adding all parties parameters
	AddConfRelayInfoPartiesToResponse( pConfNode, ResCounter, party_data_amount );
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddConfRelayInfoToResponse(CXMLDOMElement *pConfNode)
{
	// pConfNode->AddChildNode("OBJ_TOKEN",m_dwFullUpdateCounter);
	pConfNode->AddChildNode("CONF_ID",m_confId);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddConfRelayInfoPartiesToResponse(CXMLDOMElement *pConfNode, DWORD ResCounter, ePartyData party_data_amount)
{
	CXMLDOMElement *pPartyListNode,*pConfPartyNode;
	int Opcode=PARTY_NOT_CHANGED;

	pPartyListNode=pConfNode->AddChildNode("ONGOING_PARTY_LIST");

	// add on-going parties Relay Info
	CConfParty* pConfParty = GetFirstParty();
	for (int i=0; i<(int)m_numParties; i++)
	{
			PASSERT(! pConfParty);
			if (! pConfParty) {
				continue;
			}
			// check if this is a relay party with media to report
			if (!pConfParty->IsRelayInfoExists())
				continue;

			pConfPartyNode = pPartyListNode->AddChildNode("ONGOING_PARTY");
			bool bChanged = AddPartyRelayMediaInfoToResponse( pConfPartyNode, pConfParty, party_data_amount );
			AddPartyRelayGeneralInfoToResponse( pConfPartyNode, pConfParty, party_data_amount, bChanged );

			if ( i+1 < m_numParties )  {
				pConfParty = NULL;
				pConfParty = GetNextParty();
				PASSERT(!pConfParty);
				if (! pConfParty)
					break;
			}
	}

	// add deleted participants to the list
	for (int i=0 ;i<1000; i++)
	{
		if((m_DeletedCounterHistory[i] > ResCounter) && (ResCounter != 0xFFFFFFFF))
		{
			pConfPartyNode=pPartyListNode->AddChildNode("ONGOING_PARTY");
			pConfPartyNode->AddChildNode("PARTY_ID",m_DeletedIdHistory[i]);
			pConfPartyNode->AddChildNode("STATE","deleted");
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddPartyRelayGeneralInfoToResponse( CXMLDOMElement *pConfPartyNode, CConfParty* pConfParty, ePartyData party_data_amount, bool bChanged )
{
	// get all needed info for this relay info transaction
	// party_data_amount means full or partial
	// bChanged - if the previous Party data was changed (so the party details bmust be inserted as well)
//	if (bChanged || (party_data_amount == FULL_DATA))
//	{
		pConfPartyNode->AddChildNode("PARTY_ID",pConfParty->GetPartyId());
		if (party_data_amount == FULL_DATA)
			pConfPartyNode->AddChildNode("STATE","full");
		else
			pConfPartyNode->AddChildNode("STATE","partial");
		pConfPartyNode->AddChildNode("MUTE","no");
//	}
}

////////////////////////////////////////////////////////////////////////////
bool CCommConf::AddPartyRelayMediaInfoToResponse( CXMLDOMElement *pConfPartyNode, CConfParty* pConfParty, ePartyData party_data_amount)
{
	// get all MEDIA info for this party
	// party_data_amount means full or partial

	// serialize party data (partyMonitorId)
	// pConfParty->SerializeXmlRelayInfo(CXMLDOMElement *pActionNode, char *pszError);



	return true;
}


////////////////////////////////////////////////////////////////////////////
int CCommConf::DeSerializeConfStatus(CXMLDOMElement* pStatusNode, char* pszError)
{
	int nStatus=STATUS_OK;
	BYTE bEmpty, bSingleParty, bNotFull, bResDeficiency, bBadRes, bProblemParty , bPartyRequiredOperatorAssist, bContentResDeficiency;

	PASSERTMSG_AND_RETURN_VALUE(pStatusNode == NULL, "pStatusNode shouldn't be NULL!!.", STATUS_FAIL);

	GET_VALIDATE_CHILD(pStatusNode,"CONF_EMPTY",&bEmpty,_BOOL);

	if(nStatus == STATUS_OK)
	{
		if(bEmpty)
			m_conf_status |= CONFERENCE_EMPTY;
		else
			m_conf_status &= (0xffffffff ^ CONFERENCE_EMPTY);
	}

	GET_VALIDATE_CHILD(pStatusNode,"SINGLE_PARTY",&bSingleParty,_BOOL);

	if(nStatus == STATUS_OK)
	{
		if(bSingleParty)
			m_conf_status |= CONFERENCE_SINGLE_PARTY;
		else
			m_conf_status &= (0xffffffff ^ CONFERENCE_SINGLE_PARTY);
	}

	GET_VALIDATE_CHILD(pStatusNode,"NOT_FULL",&bNotFull,_BOOL);

	if(nStatus == STATUS_OK)
	{
		if(bNotFull)
			m_conf_status |= CONFERENCE_NOT_FULL;
		else
			m_conf_status &= (0xffffffff ^ CONFERENCE_NOT_FULL);
	}

	GET_VALIDATE_CHILD(pStatusNode,"RESOURCES_DEFICIENCY",&bResDeficiency,_BOOL);

	if(nStatus == STATUS_OK)
	{
		if(bResDeficiency)
			m_conf_status |= CONFERENCE_RESOURCES_DEFICIENCY;
		else
			m_conf_status &= (0xffffffff ^ CONFERENCE_RESOURCES_DEFICIENCY);
	}

	GET_VALIDATE_CHILD(pStatusNode,"BAD_RESOURCES",&bBadRes,_BOOL);

	if(nStatus == STATUS_OK)
	{
		if(bBadRes)
			m_conf_status |= CONFERENCE_BAD_RESOURCES;
		else
			m_conf_status &= (0xffffffff ^ CONFERENCE_BAD_RESOURCES);
	}

	GET_VALIDATE_CHILD(pStatusNode,"PROBLEM_PARTY",&bProblemParty,_BOOL);

	if(nStatus == STATUS_OK)
	{
		if(bProblemParty)
			m_conf_status |= PROBLEM_PARTY;
		else
			m_conf_status &= (0xffffffff ^ PROBLEM_PARTY);

	}

	GET_VALIDATE_CHILD(pStatusNode,"PARTY_REQUIRES_OPERATOR_ASSIST",&bPartyRequiredOperatorAssist,_BOOL);

	if(nStatus == STATUS_OK)
	{
		if(bPartyRequiredOperatorAssist)
			m_conf_status |= PARTY_REQUIRES_OPERATOR_ASSIST;
		else
			m_conf_status &= (0xffffffff ^ PARTY_REQUIRES_OPERATOR_ASSIST);
	}

	GET_VALIDATE_CHILD(pStatusNode,"CONTENT_RESOURCES_DEFICIENCY",&bContentResDeficiency,_BOOL);

	if(nStatus == STATUS_OK)
	{
		if(bContentResDeficiency)
			m_conf_status |= CONFERENCE_CONTENT_RESOURCES_DEFICIENCY;
		else
			m_conf_status &= (0xffffffff ^ CONFERENCE_CONTENT_RESOURCES_DEFICIENCY);

	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::DeSerializeShortXml(CXMLDOMElement* pSummaryNode, char* pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pSummaryNode, "CONF_CHANGE", &m_infoOpcode, CHANGE_STATUS_ENUM);

	if (nStatus != STATUS_OK)
		return nStatus;

	if (m_infoOpcode != CONF_NOT_CHANGED)
		GET_VALIDATE_CHILD(pSummaryNode, "NAME", m_H243confName, _0_TO_H243_NAME_LENGTH);

	GET_VALIDATE_CHILD(pSummaryNode, "ID", &m_confId, _0_TO_DWORD);

	if (nStatus != STATUS_OK)
		return nStatus;

	if (m_infoOpcode != CONF_NOT_CHANGED)
	{
		CXMLDOMElement* pStatusNode, * pContactInfoListNode, * pChildNode;
		BYTE bTmp = NO;
		DWORD dwJoinConfID;

		GET_CHILD_NODE(pSummaryNode, "CONF_STATUS", pStatusNode);

		if (pStatusNode)
		{
			nStatus = DeSerializeConfStatus(pStatusNode, pszError);
			if (nStatus != STATUS_OK)
				return nStatus;
		}

		GET_VALIDATE_CHILD(pSummaryNode, "START_TIME", &m_startTime, DATE_TIME);
		GET_VALIDATE_CHILD(pSummaryNode, "END_TIME", &m_endTime, DATE_TIME);
		GET_VALIDATE_CHILD(pSummaryNode, "OPERATOR_CONF", &m_operatorConf, _BOOL);

		GET_VALIDATE_CHILD(pSummaryNode, "LOCK", &bTmp, _BOOL);

		if (nStatus == STATUS_OK)
		{
			if (bTmp)
				m_dwConfFlags |= LOCKED;
			else
				m_dwConfFlags &= (0xffffffff ^ LOCKED);
		}

		GET_VALIDATE_CHILD(pSummaryNode, "LECTURE_CONF", &bTmp, _BOOL);

		if (nStatus == STATUS_OK)
		{
			if (bTmp)
				m_dwConfFlags |= LECTURE_MODE_SHOW;
			else
				m_dwConfFlags &= (0xffffffff ^ LECTURE_MODE_SHOW);
		}

		GET_VALIDATE_CHILD(pSummaryNode, "NUM_PARTIES", &m_numParties, _0_TO_DWORD);
		GET_VALIDATE_CHILD(pSummaryNode, "NUM_CONNECTED_PARTIES", &m_number_of_connected_parties, _0_TO_DWORD);

		PTRACE2INT(eLevelInfoNormal, "CCommConf::DeSerializeShortXml : m_numParties = ", m_numParties);

		GET_VALIDATE_CHILD(pSummaryNode, "SECURE", &bTmp, _BOOL);

		if (nStatus == STATUS_OK)
		{
			if (bTmp)
				m_dwConfFlags |= SECURED;
			else
				m_dwConfFlags &= (0xffffffff ^ SECURED);
		}

		GET_VALIDATE_CHILD(pSummaryNode, "NUMERIC_ID", m_NumericConfId, _0_TO_NUMERIC_CONFERENCE_ID_LENGTH);

		GET_CHILD_NODE(pSummaryNode, "CONTACT_INFO_LIST", pContactInfoListNode);

		if (pContactInfoListNode)
		{
			if (!m_pConfContactInfo)
				m_pConfContactInfo = new CConfContactInfo;

			nStatus = m_pConfContactInfo->DeSerializeXml(pContactInfoListNode, pszError);

			if (nStatus != STATUS_OK)
			{
				POBJDELETE(m_pConfContactInfo);
				return nStatus;
			}
		}

		GET_VALIDATE_CHILD(pSummaryNode, "AUTO_LAYOUT", &m_isAutoLayout, _BOOL);
		GET_CHILD_NODE(pSummaryNode, "DURATION", pChildNode);

		if (pChildNode)
		{
			GET_VALIDATE_CHILD(pChildNode, "HOUR", &m_duration.m_hour, _0_TO_DWORD);
			GET_VALIDATE_CHILD(pChildNode, "MINUTE", &m_duration.m_min, _0_TO_59_DECIMAL);
			GET_VALIDATE_CHILD(pChildNode, "SECOND", &m_duration.m_sec, _0_TO_59_DECIMAL);
		}

		GET_VALIDATE_CHILD(pSummaryNode, "NUM_UNDEFINED_PARTIES", &m_numUndefParties, _0_TO_DWORD);

		GET_CHILD_NODE(pSummaryNode, "DIAL_IN_H323_SRV_PREFIX_LIST", pChildNode);
		if (pChildNode)
		{
			CXMLDOMElement* pServiceNode;
			CleanAllServicePrefix();

			GET_FIRST_CHILD_NODE(pChildNode, "DIAL_IN_H323_SRV_PREFIX", pServiceNode);

			while (pServiceNode)
			{
				CServicePrefixStr servicePrefixStr;
				nStatus = servicePrefixStr.DeSerializeXml(pServiceNode, pszError);

				if (nStatus != STATUS_OK)
					return nStatus;

				AddServicePrefix(servicePrefixStr);

				GET_NEXT_CHILD_NODE(pChildNode, "DIAL_IN_H323_SRV_PREFIX", pServiceNode);
			}
		}

		GET_VALIDATE_CHILD(pSummaryNode, "ENCRYPTION", &bTmp, _BOOL);

		if (nStatus == STATUS_OK)
		{
			if (bTmp)
				m_dwConfFlags |= ENCRYPTION;
			else
				m_dwConfFlags &= (0xffffffff ^ ENCRYPTION);
		}

		BYTE encryptionType = eEncryptNone;
		GET_VALIDATE_CHILD(pSummaryNode, "ENCRYPTION_TYPE", &encryptionType, ENCRYPTION_TYPE_ENUM)
		if (bTmp && (nStatus == STATUS_OK))
			m_eEncryptionType = encryptionType;
		else
			m_eEncryptionType = eEncryptNone;

		GET_VALIDATE_CHILD(pSummaryNode, "RECORDING_STATUS", &m_RecordingLinkControl, RECORDING_STATUS_ENUM);
		GET_VALIDATE_CHILD(pSummaryNode, "TRANSFER_RATE", &m_confTransferRate, TRANSFER_RATE_ENUM);
		GET_VALIDATE_CHILD(pSummaryNode, "HD", &m_HD, _BOOL);
		GET_VALIDATE_CHILD(pSummaryNode, "DISPLAY_NAME", m_confDisplayName, _0_TO_H243_NAME_LENGTH);
	}

	GET_VALIDATE_CHILD(pSummaryNode, "SIP_REGISTRATIONS_STATUS", &m_SipRegTotalSts, SIP_REG_TOTAL_STATUS_ENUM); // sipProxySts

	if ( IsTemplate() == FALSE && IsConfTemplate() == FALSE && m_correlationId[0] == '\0' && m_confDisplayName[0]!='\0' && m_startTime.m_year!=0)
	{
		SetCorrelationId();
		TRACEINTO << "Correlation Id: " << m_correlationId;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::DeSerializeOngoingPartiesList(CXMLDOMElement *pPartiesListNode, char *pszError)
{
	CXMLDOMElement *pOngoPartyNode;
	int nStatus;

	GET_FIRST_CHILD_NODE(pPartiesListNode, "ONGOING_PARTY", pOngoPartyNode);

	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
    WORD maxPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
	WORD maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
	m_numParties =0;
	m_numVideoParties = 0;
	while (pOngoPartyNode && m_numParties < maxPartiesInConfPerSystemMode
		&& m_numParties < MAX_PARTIES_IN_CONF)
	{
		CConfParty *pConfParty = new CConfParty;
		nStatus = pConfParty->DeSerializeXml(pOngoPartyNode,pszError);
		BYTE networkType = pConfParty->GetNetInterfaceType();

		if (networkType==H323_INTERFACE_TYPE||networkType==SIP_INTERFACE_TYPE)
		 {
			pConfParty->AllocateIpChannels();
		 }

		if(nStatus != STATUS_OK)
		{
			POBJDELETE(pConfParty);
			return nStatus;
		}
		BYTE isAudioOnlyParty = pConfParty->GetVoice();
		// Romem klocwork
		if(isAudioOnlyParty||
				((!isAudioOnlyParty && m_numVideoParties<maxVideoPartiesInConfPerSystemMode) && m_numParties < MAX_PARTIES_IN_CONF))
		{
			m_pParty[m_numParties] = pConfParty;
			m_pParty[m_numParties]->SetRes(this);
			m_numParties++;
                        PTRACE2INT(eLevelInfoNormal,"CCommConf::DeSerializeOngoingPartiesList : m_numParties(After add) = ",m_numParties);
			GET_NEXT_CHILD_NODE(pPartiesListNode, "ONGOING_PARTY", pOngoPartyNode);
			if(!isAudioOnlyParty)
				m_numVideoParties++;
		}
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::DeSerializeFullXml(CXMLDOMElement *pConferenceNode, char *pszError)
{
	int nStatus=STATUS_OK;

	CXMLDOMElement *pStatusNode, *pChildNode;

	GET_CHILD_NODE(pConferenceNode, "RESERVATION", pChildNode);
	if (pChildNode)
	{
		nStatus = CCommResApi::DeSerializeXml(pChildNode, pszError, ADD_RESERVE); // yael: is ADD_RESERVE the correct one?
		if (nStatus != STATUS_OK)
			return nStatus;
		CopyVideoLayoutToReservationLayout();
	}

	GET_CHILD_NODE(pConferenceNode, "CONF_STATUS", pStatusNode);
	if (pStatusNode)
	{
		nStatus = DeSerializeConfStatus(pStatusNode,pszError);
		if(nStatus != STATUS_OK)
			return nStatus;
	}

	GET_VALIDATE_CHILD(pConferenceNode, "AUDIO_SOURCE_ID", &m_source_audio_Id, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pConferenceNode, "LSD_SOURCE_ID", &m_source_LSD_Id, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pConferenceNode,"NUM_CONNECTED_PARTIES",&m_number_of_connected_parties, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pConferenceNode,"SECURE",&m_confSecureFlag,_BOOL);
	GET_VALIDATE_CHILD(pConferenceNode, "EPC_CONTENT_SOURCE_ID", &m_EPC_Content_source_Id, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pConferenceNode, "RECORDING_STATUS", &m_RecordingLinkControl, RECORDING_STATUS_ENUM);
	GET_VALIDATE_CHILD(pConferenceNode, "MANAGE_TELEPRESENCE_LAYOUTS_INTERNALLY", &m_manageTelepresenceLayoutInternaly, _BOOL);
	GET_VALIDATE_CHILD(pConferenceNode, "GATEWAY", &m_isGateway, _BOOL);
	GET_VALIDATE_CHILD(pConferenceNode,"VIDEO_SESSION",&m_videoSession,VIDEO_SESSION_ENUM);

	GET_CHILD_NODE(pConferenceNode, "ONGOING_PARTY_LIST", pChildNode);
	if (pChildNode)
	{
		nStatus = DeSerializeOngoingPartiesList(pChildNode, pszError);
		if (nStatus != STATUS_OK)
			return nStatus;
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddConfPartiesToResponse(CXMLDOMElement *pConfNode,int ResCounter, ePartyData party_data_amount /* = FULL_DATA*/)
{

	CXMLDOMElement *pPartyListNode,*pConfPartyNode,*pResPartyNode;
	int Opcode=PARTY_NOT_CHANGED;

	pPartyListNode=pConfNode->AddChildNode("ONGOING_PARTY_LIST");

	if (!IsConfSecured())
	{
		//BRIDGE-1621,BRIDGE-1657
		//Sort party list in the order that first connect party at the beginning and last disconnect party at the end
		std::list<CConfParty *> pConfPartyList;


		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

		std::string sMsMonitor = "MAIN_AND_IN_SLAVE";
		if (sysConfig)
		{
			sysConfig->GetDataByKey("MS_AV_MCU_MONITORING", sMsMonitor);
		}
		TRACEINTO << "MS_AV_MCU_MONITORING value: " << sMsMonitor;

		for (int i=0;i<(int)m_numParties;i++)
		{

			if (m_pParty[i]!=NULL)
			{
				CConfParty* tmp_pParty = (CConfParty *)m_pParty[i];

				const eAvMcuLinkType linkType = tmp_pParty->GetAvMcuLinkType();

				if (sMsMonitor.compare("MAIN_ONLY") == 0)
				{
					if ((linkType == eAvMcuLinkSlaveOut) || (linkType == eAvMcuLinkSlaveIn))
					{
						continue;
					}
				}

				else if (sMsMonitor.compare("MAIN_AND_IN_SLAVE") == 0)
				{
					if (linkType == eAvMcuLinkSlaveOut)
					{
						continue;
					}
				}


				if( pConfPartyList.empty() )
				{
					pConfPartyList.push_back((CConfParty *)m_pParty[i]);
					continue;
				}

				const CStructTm *pTm = NULL;
				time_t conntt = 0;
				time_t disctt = 0;
				time_t curconntt = 0;
				time_t curdisctt = 0;
				pTm = ((CConfParty *)m_pParty[i])->GetConnectTime();
				curconntt = pTm->GetAbsTime(TRUE);
				pTm = ((CConfParty *)m_pParty[i])->GetDisconnectTime();
				curdisctt = pTm->GetAbsTime(TRUE);

				std::list<CConfParty *>::iterator it = pConfPartyList.begin();
				for(; it != pConfPartyList.end(); it++)
				{
					pTm = (*it)->GetConnectTime();
					conntt = pTm->GetAbsTime(TRUE);
					pTm = (*it)->GetDisconnectTime();
					disctt = pTm->GetAbsTime(TRUE);

					if( curconntt > curdisctt ) //connected party
					{
						if( (curconntt >= conntt) && (conntt > disctt) )
							continue;
						else
						{
							pConfPartyList.insert( it, (CConfParty *)m_pParty[i]);
							break;
						}
					}
					else //disconnected party
					{
						if( (curdisctt >= disctt) || (conntt > disctt) )
							continue;
						else
						{
							pConfPartyList.insert( it, (CConfParty *)m_pParty[i]);
							break;
						}
					}
				}

				if(pConfPartyList.end() == it)
				{
					pConfPartyList.push_back((CConfParty *)m_pParty[i]);
				}
			}
		}

		for(std::list<CConfParty *>::iterator it = pConfPartyList.begin(); it != pConfPartyList.end(); it++)
		{
			WORD bChange=TRUE;

			pConfPartyNode=pPartyListNode->AddChildNode("ONGOING_PARTY");

			if((int)(*it)->GetCreationUpdateCounter()>(int)ResCounter)
				Opcode=PARTY_NEW_INFO;
			else if((int)(*it)->GetCompleteUpdateCounter()>(int)ResCounter)
				Opcode=PARTY_COMPLETE_INFO;
			else if ((int)(*it)->GetSlow1UpdateCounter()>(int)ResCounter&&(int)(*it)->GetSlowUpdateCounter()>ResCounter)
				Opcode=PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO;
			else if ((int)(*it)->GetSlow1UpdateCounter()>(int)ResCounter)
				Opcode=PARTY_FAST_PLUS_SLOW1_INFO;
			else if ((int)(*it)->GetSlowUpdateCounter()>(int)ResCounter)
				Opcode=PARTY_FAST_PLUS_SLOW_INFO;
			else if ((int)(*it)->GetFastUpdateCounter()>(int)ResCounter)
				Opcode=PARTY_FAST_INFO;
			else
			{
				Opcode=PARTY_NOT_CHANGED;
				pConfPartyNode->AddChildNode("ONGOING_PARTY_CHANGE","none");
				pResPartyNode=pConfPartyNode->AddChildNode("PARTY");
				pResPartyNode->AddChildNode("NAME",(*it)->GetName());
				pResPartyNode->AddChildNode("ID",(*it)->GetPartyId());
				pConfPartyNode->AddChildNode("PARTY_CHANGE_TYPE",Opcode,PARTY_CHANGE_STATE_ENUM);
				bChange=FALSE;
			}

			if(bChange==TRUE)
				AddOnGoingPartyToResResponse(pConfPartyNode,(*it),Opcode, party_data_amount);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
const char* CCommConf::PartyOpcodeToString(int opcode)
{
	switch (opcode)
	{
	case PARTY_NOT_CHANGED: 					return "PARTY_NOT_CHANGED";
	case PARTY_COMPLETE_INFO: 					return "PARTY_COMPLETE_INFO";
	case PARTY_FAST_INFO: 						return "PARTY_FAST_INFO";
	case PARTY_FAST_PLUS_SLOW_INFO: 			return "PARTY_FAST_PLUS_SLOW_INFO";
	case PARTY_NEW_INFO: 						return "PARTY_NEW_INFO";
	case PARTY_FAST_PLUS_SLOW1_INFO: 			return "PARTY_FAST_PLUS_SLOW1_INFO";
	case PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO: 	return "PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO";
	default:									return "opcode not defined";
	}
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddConfStatusToResponse(CXMLDOMElement *pResNode)
{
	CXMLDOMElement *pStatusNode;
	pStatusNode=pResNode->AddChildNode("CONF_STATUS");

	pStatusNode->AddChildNode("CONF_OK",!m_conf_status,_BOOL);
	pStatusNode->AddChildNode("CONF_EMPTY",(m_conf_status & CONFERENCE_EMPTY) > 0,_BOOL);
	pStatusNode->AddChildNode("SINGLE_PARTY",(m_conf_status & CONFERENCE_SINGLE_PARTY) > 0,_BOOL);
	pStatusNode->AddChildNode("NOT_FULL",(m_conf_status & CONFERENCE_NOT_FULL) > 0,_BOOL);
	pStatusNode->AddChildNode("RESOURCES_DEFICIENCY",(m_conf_status & CONFERENCE_RESOURCES_DEFICIENCY) > 0,_BOOL);
	pStatusNode->AddChildNode("BAD_RESOURCES",(m_conf_status & CONFERENCE_BAD_RESOURCES) > 0,_BOOL);
	pStatusNode->AddChildNode("PROBLEM_PARTY",(m_conf_status & PROBLEM_PARTY) > 0,_BOOL);
	pStatusNode->AddChildNode("PARTY_REQUIRES_OPERATOR_ASSIST",(m_conf_status & PARTY_REQUIRES_OPERATOR_ASSIST) > 0,_BOOL);
	pStatusNode->AddChildNode("CONTENT_RESOURCES_DEFICIENCY",(m_conf_status & CONFERENCE_CONTENT_RESOURCES_DEFICIENCY) > 0,_BOOL);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddOnGoingPartyToResResponse(CXMLDOMElement *pConfPartyNode,CRsrvParty* pParty,int Opcode, ePartyData party_data_amount)
{
	CXMLDOMElement *pResPartyNode;

	if (Opcode==PARTY_NEW_INFO)
	{
		pConfPartyNode->AddChildNode("ONGOING_PARTY_CHANGE","new");
		pParty->SerializeXml(pConfPartyNode, party_data_amount);
	}
	else if (Opcode==PARTY_COMPLETE_INFO)
	{
		pConfPartyNode->AddChildNode("ONGOING_PARTY_CHANGE","update");
		pParty->SerializeXml(pConfPartyNode, party_data_amount);
	}
	else if (Opcode==PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO||Opcode==PARTY_FAST_PLUS_SLOW1_INFO||Opcode==PARTY_FAST_PLUS_SLOW_INFO||Opcode==PARTY_FAST_INFO)
	{
		pConfPartyNode->AddChildNode("ONGOING_PARTY_CHANGE","update");
		if (Opcode==PARTY_FAST_PLUS_SLOW1_PLUS_SLOW_INFO||Opcode==PARTY_FAST_PLUS_SLOW_INFO)
		{
			pParty->SerializeXml(pConfPartyNode, party_data_amount);
		}
		else
		{
			pResPartyNode=pConfPartyNode->AddChildNode("PARTY");
			pResPartyNode->AddChildNode("ID",pParty->GetPartyId());
		}
	}

	((CConfParty*)pParty)->SerializeXml(pConfPartyNode,Opcode, party_data_amount);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddConfXmlToResponse(CXMLDOMElement *pConfNode,int ConfOpcode)
{
	if (ConfOpcode==CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
		||ConfOpcode==CONF_FAST_PLUS_SLOW_INFO
		||ConfOpcode==CONF_FAST_INFO
		||ConfOpcode==CONF_COMPLETE_INFO)
	{
		//fast changes
		AddConfStatusToResponse(pConfNode);
		pConfNode->AddChildNode("AUDIO_SOURCE_ID",m_source_audio_Id);
    	AddConfActiveSpeakersListToXmlResponse(pConfNode);
    	pConfNode->AddChildNode("LSD_SOURCE_ID",m_source_LSD_Id);
	}
	//4.6
	if (ConfOpcode==CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
		||ConfOpcode==CONF_FAST_PLUS_SLOW_INFO
		||ConfOpcode==CONF_FAST_PLUS_SLOW1_INFO
		||ConfOpcode==CONF_FAST_INFO
		||ConfOpcode==CONF_COMPLETE_INFO)
	{
		//fast changes
		pConfNode->AddChildNode("NUM_CONNECTED_PARTIES",m_number_of_connected_parties);
	}

	if (ConfOpcode==CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
		||ConfOpcode==CONF_FAST_PLUS_SLOW_INFO
		||ConfOpcode==CONF_COMPLETE_INFO)
	{
	  //slow changes
		pConfNode->AddChildNode("SECURE",m_confSecureFlag,_BOOL);
	}

	if (ConfOpcode==CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO
		||ConfOpcode==CONF_FAST_PLUS_SLOW_INFO
		||ConfOpcode==CONF_FAST_INFO
		||ConfOpcode==CONF_COMPLETE_INFO
		||ConfOpcode==CONF_FAST_PLUS_SLOW1_INFO)
	{
		//fast chages
		pConfNode->AddChildNode("EPC_CONTENT_SOURCE_ID",m_EPC_Content_source_Id);
	}

	//7.0
	if (ConfOpcode == CONF_COMPLETE_INFO||
		ConfOpcode == CONF_FAST_PLUS_SLOW_INFO ||
		ConfOpcode == CONF_FAST_PLUS_SLOW1_PLUS_SLOW_INFO )
	{
		//slow changes
		pConfNode->AddChildNode("RECORDING_STATUS", m_RecordingLinkControl, RECORDING_STATUS_ENUM);
		pConfNode->AddChildNode("SIP_REGISTRATIONS_STATUS",m_SipRegistrationTotalSts,SIP_REG_TOTAL_STATUS_ENUM);//sipProxySts
		pConfNode->AddChildNode("MANAGE_TELEPRESENCE_LAYOUTS_INTERNALLY", m_manageTelepresenceLayoutInternaly, _BOOL);
	}
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::Add(const CConfParty&  other)
{
  WORD confMaxParts = GetMaxParties();
  WORD confNumParts = GetNumParties();
  if (IncludeRecordingParty()) //recording party isn't counted in the max limitation
  	confNumParts = confNumParts -1;

  if ((confMaxParts <= confNumParts) && (confMaxParts != 255))
    {
      //Reject only if we do not have IVR service or we got a defiend party (*recording party is undefined)
      if (!IsDefinedIVRService() || !other.IsUndefinedParty())
      {
      	TRACESTR (eLevelError) <<"CCommConf::Add Conference has already : "
      	<< GetNumParties()<< ", max parties allowed is: "<< confMaxParts;
      	return STATUS_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM_FROM_PROFILE;
      }
    }

	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	WORD maxPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
	WORD maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
    BYTE isAudioOnlyParty = other.GetVoice();
    if (m_numParties>=maxPartiesInConfPerSystemMode)
    	 return STATUS_MAX_PARTIES_EXCEEDED;
	if(!isAudioOnlyParty)
	{
		if(m_numVideoParties>=maxVideoPartiesInConfPerSystemMode)
    	{
    		return STATUS_MAX_VIDEO_PARTIES_EXCEEDED;
    	}
	}

  if (FindName(other)!=NOT_FIND)
	return STATUS_PARTY_NAME_EXISTS;

  // Romeme klocwork
  int undefFlag = 0;	// for later use, when accessing the file
  if(m_numParties < MAX_PARTIES_IN_CONF)
  {
	  m_pParty[m_numParties] = new CConfParty(other);
	  m_pParty[m_numParties]->SetRes(this);

	  if ( m_pParty[m_numParties]->IsUndefinedParty() == TRUE )
		  undefFlag = 1;
  }

  SLOWCHANGE;

  // Romem klocwork
  if(m_numParties < MAX_PARTIES_IN_CONF)
  {
	  m_pParty[m_numParties]->SetCreationUpdateCounter();

	  m_numParties++;
  }
  PTRACE2INT(eLevelInfoNormal,"CCommConf::Add : m_numParties(After add) = ",m_numParties);

  if(!isAudioOnlyParty)
  {
	 m_numVideoParties++;
	 PTRACE2INT(eLevelInfoNormal,"CCommConf::Add  m_numVideoParties(After add) = ",m_numVideoParties);

  }
  int status=STATUS_OK;

  IncreaseSummaryUpdateCounter();

  if ( !undefFlag )	// because undefParties should not be written to the file
 {
	  ONGOING_CONF_STORE->Add((CCommResApi*) this);
 }
m_pSubject->Notify(PARTY_ADDED, m_pParty[m_numParties-1]->GetPartyId());
  return status;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::IncConnectedPartiesNum(){
	++m_number_of_connected_parties;
	IncreaseSummaryUpdateCounter();
	FASTCHANGE;
	if(m_number_of_connected_parties > 0)
		m_pSubject->Notify(CONF_ACTIVE, TRUE);
	else
		m_pSubject->Notify(CONF_ACTIVE, FALSE);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::DecConnectedPartiesNum(){

	if(m_number_of_connected_parties > 0)
		--m_number_of_connected_parties;
	IncreaseSummaryUpdateCounter();
	FASTCHANGE;
	if(m_number_of_connected_parties > 0)
		m_pSubject->Notify(CONF_ACTIVE, TRUE);
	else
		m_pSubject->Notify(CONF_ACTIVE, FALSE);
}

////////////////////////////////////////////////////////////////////////////
WORD CCommConf::GetConnectedPartiesNumber(){
	return m_number_of_connected_parties;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::SetConnectedPartiesNumber(WORD PartiesNum)
{
	if(m_number_of_connected_parties != PartiesNum)
		IncreaseSummaryUpdateCounter();

	m_number_of_connected_parties=PartiesNum;
	FASTCHANGE;
	if(m_number_of_connected_parties > 0)
		m_pSubject->Notify(CONF_ACTIVE, TRUE);
	else
		m_pSubject->Notify(CONF_ACTIVE, FALSE);
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::Cancel(const DWORD id)
{
	int ind = FindId(id);
	if (ind==NOT_FIND)
		return STATUS_PARTY_DOES_NOT_EXIST;

	// Romem klocwork
	if (ind >= MAX_PARTIES_IN_CONF)
		return STATUS_PARTY_DOES_NOT_EXIST;

	UpdateActiveSpeakersListUponPartyLeftTheConf( id );
	int undefFlag = 0; // for later use, when accessing the file
	if (m_pParty[ind]->IsUndefinedParty())
		undefFlag = 1;

	TRACEINTOFUNC << "id = " << id;
	if (m_pConfRcvMbx)
	{
		((CConfParty*)m_pParty[ind])->DetachObserver(m_pConfRcvMbx); //This DetachObserver exist for the possibility that there is a Subscriber that observe this participant
	}

	CConfParty* pConfParty = (CConfParty*)m_pParty[ind];
	BYTE isAudioOnlyParty = pConfParty->GetVoice();

	OnPartyDelete(*pConfParty);

	POBJDELETE(m_pParty[ind]);

	int i = 0;
	for ( ; i < (int)m_numParties; ++i)
	{
		if (!m_pParty[i])
			break;
	}

	for (int j = i; j < (int)m_numParties-1; ++j)
	{
		m_pParty[j] = m_pParty[j+1];
	}

	m_pParty[m_numParties-1] = NULL;

	--m_numParties;
	TRACEINTOFUNC << "(by id) (After cancel) m_numParties = " << m_numParties;

	if (!isAudioOnlyParty)
		--m_numVideoParties;

	SLOWCHANGE;

	if (m_LastDeletedIndex >= 1000)
		m_LastDeletedIndex = 0;

	m_DeletedIdHistory[m_LastDeletedIndex] = id;
	m_DeletedCounterHistory[m_LastDeletedIndex] = m_dwFullUpdateCounter;

	++m_LastDeletedIndex;

	int status = STATUS_OK;

	IncreaseSummaryUpdateCounter();

	if (!Is_TerminatingState()) {
#ifdef __HIGHC__
		if (!undefFlag) // because undefParties have never been written to the file
		{
			::WaitForReservationDBSemaphore();
			CLargeConfCfgFile  LargeCfgFile(OBJECT_ID, SIZE_RESERV_RECORD);
			LargeCfgFile.Del(status, this->GetName(), this->GetMonitorConfId(), FILE_LARGE_CONF_DB_IND);
			if (status==STATUS_OK)
			{
				char* msg;
				CCommRes* pResConf = new CCommRes((const CCommRes&)*this);
				msg=pResConf->SerializeToFile();
				POBJDELETE(pResConf);
				LargeCfgFile.Put(status, msg, this->GetName(), this->GetMonitorConfId(), FILE_LARGE_CONF_DB_IND);
				delete []msg;
			}
			::ReleaseReservationDBSemaphore();   // semaphore
		}
		m_pSubject->Notify(PARTY_DELETED, id);
#endif
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::Cancel(const char*  name)
{
	int ind = FindName(name);
	if (ind == NOT_FIND)
		return STATUS_PARTY_DOES_NOT_EXIST;

	// Romem klocwork
	if (ind >= MAX_PARTIES_IN_CONF)
		return STATUS_PARTY_DOES_NOT_EXIST;

	PASSERTSTREAM_AND_RETURN_VALUE(m_numParties > MAX_PARTIES_IN_CONF,
		"m_numParties has invalid value " << m_numParties, STATUS_FAIL);

	int undefFlag = 0;	// for later use, when accessing the file
	if (m_pParty[ind]->IsUndefinedParty())
		undefFlag = 1;

	DWORD id = m_pParty[ind]->GetPartyId();

	UpdateActiveSpeakersListUponPartyLeftTheConf( id );
	TRACEINTOFUNC << "name = " << name;
	if (m_pConfRcvMbx)
	{
		((CConfParty*)m_pParty[ind])->DetachObserver(m_pConfRcvMbx); //This DetachObserver exist for the possibility that there is a Subscriber that observe this participant
	}

	CConfParty* pConfParty = (CConfParty*)m_pParty[ind];
	BYTE isAudioOnlyParty = pConfParty->GetVoice();

	OnPartyDelete(*pConfParty);

	POBJDELETE(m_pParty[ind]);

	int i = 0;
	for ( ; i < (int)m_numParties; ++i)
	{
		if (!m_pParty[i])
			break;
	}

	for (int j = i; j < (int)m_numParties-1; ++j)
	{
		m_pParty[j] = m_pParty[j+1];
	}

	m_pParty[m_numParties-1] = NULL;
	--m_numParties;

	TRACEINTOFUNC << "(by name) (After cancel) m_numParties = " << m_numParties;

	if (!isAudioOnlyParty)
		--m_numVideoParties;

	SLOWCHANGE;

	if (m_LastDeletedIndex >= 1000)
		m_LastDeletedIndex = 0;

	m_DeletedIdHistory[m_LastDeletedIndex] = id;
	m_DeletedCounterHistory[m_LastDeletedIndex] = m_dwFullUpdateCounter;

	++m_LastDeletedIndex;

	int status = STATUS_OK;

	IncreaseSummaryUpdateCounter();

	if (!Is_TerminatingState())
	{
		if (!undefFlag) // because undefParties have never been written to the file
		{
			if (status == STATUS_OK)
			{
				CCommRes* pResConf = new CCommRes((const CCommRes&)*this);
				POBJDELETE(pResConf);
			}
		}

		m_pSubject->Notify(PARTY_DELETED, id);
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::IncreaseFullUpdateCounter()
{
	CCommConfDB* pConfDB = ::GetpConfDB();
	if (pConfDB)
	{
		pConfDB->IncreaseFullUpdateCounter();
		m_dwFullUpdateCounter = pConfDB->GetFullUpdateCounter();
	}
}

////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetFirstParty(int& nPos)
{
   CConfParty* pParty = GetFirstParty();
   nPos = m_ind_conf;
   return pParty;
}

////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetFirstParty()
{
   m_ind_conf=1;
   return (CConfParty*)m_pParty[0];
}

////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetNextParty(int& nPos)
{
   m_ind_conf = nPos;

   CConfParty* pParty = GetNextParty();

   nPos = m_ind_conf;

   return pParty;
}

////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetNextParty()
{
   if (m_ind_conf>=m_numParties) return NULL;

   PASSERTSTREAM_AND_RETURN_VALUE(m_ind_conf >= MAX_PARTIES_IN_CONF,
   	"m_ind_conf has invalid value " << m_ind_conf, NULL);

   return (CConfParty*)m_pParty[m_ind_conf++];
}

////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetCurrentParty(const DWORD id) const
{
  for (int i=0;i<(int)m_numParties;i++)
  {
	 if (m_pParty[i]!=NULL) {
		  if (m_pParty[i]->GetPartyId() == id)
			 return (CConfParty*)m_pParty[i];
	 }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetCurrentParty(const char*  name) const
{
  for (int i=0;i<(int)m_numParties;i++)
  {
	 if (m_pParty[i]!=NULL) {
		  if (! strcmp(m_pParty[i]->GetName(), name))
			 return (CConfParty*)m_pParty[i];
	 }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetCurrentPartyAccordingToVisualName(const char*  visual_name) const
{
	for (int i=0;i<(int)m_numParties;i++)
	  {
		 if (m_pParty[i]!=NULL) {
			  if (! strcmp(((CConfParty*)m_pParty[i])->GetVisualPartyName(), visual_name))
				 return (CConfParty*)m_pParty[i];
		 }
	  }
	  return NULL;
}

////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetRecordLinkCurrentParty(void) const
{
  for (int i=0;i<(int)m_numParties;i++)
  {
	 if (m_pParty[i]!=NULL) {
		  if (m_pParty[i]->GetRecordingLinkParty() == YES)
			 return (CConfParty*)m_pParty[i];
	 }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////
//Finds unreserved party according to Id given by lobby.
CConfParty*  CCommConf::GetUnreservedParty(const DWORD lobbyId) const
{
	for (int i=0;i<(int)m_numParties;i++)
	{
		if (m_pParty[i]!=NULL) {
			if (((CConfParty*)m_pParty[i])->GetLobbyId() == lobbyId)
				if(((CConfParty*)m_pParty[i])->IsUndefinedParty())
					return (CConfParty*)m_pParty[i];
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CCommConf::GetMainLinkNameAccordingToUnrsrvPartiesCounter
//Finds unreserved mainlink party (ITP cascaded feature) name (dial in) according to Id given by ConfPartyManager.
//void CCommConf::GetMainLinkNameAccordingToMainPartiesCounter(const DWORD mainPartiesCounter, char *mainPartyName)
BOOL CCommConf::GetMainLinkNameAccordingToMainPartiesCounterAndReturnIsMainLinkDefined(const DWORD mainPartiesCounter, char *mainPartyName)
{
	std::ostringstream msg;
	msg << "mainPartiesCounter:" << mainPartiesCounter;

    BOOL        isMainLinkDefined = FALSE;
    CConfParty* pTempConfParty;
    int numParties = GetNumParties();

    for (int i=0;i<numParties;i++)
    {
        if (i==0)
            pTempConfParty = GetFirstParty();
        else
            pTempConfParty = GetNextParty();

        if ( (pTempConfParty != NULL) && (pTempConfParty->GetMainPartyNumber() == mainPartiesCounter)
                                      && (pTempConfParty->GetPartyType() == eMainLinkParty) )
        {
            memset(mainPartyName, '\0', H243_NAME_LEN);
            strncpy(mainPartyName, pTempConfParty->GetName(), H243_NAME_LEN - 1);

            if (pTempConfParty->IsUndefinedParty())
            	msg << ", undefined:" << mainPartyName;
            else
            	msg << ", defined:" << mainPartyName;

            isMainLinkDefined = !(pTempConfParty->IsUndefinedParty());

        }
    }

    TRACEINTO << msg.str().c_str() << " - ITP_CASCADE";

    return isMainLinkDefined;
}

////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
//Multiple links for ITP in cascaded conference feature:
DWORD CCommConf::NextMainPartiesCounter()
{
    m_mainPartiesCounter++;
    return m_mainPartiesCounter;
}

////////////////////////////////////////////////////////////////////////////
DWORD  CCommConf::GetSourceAudioId() const
{
	return m_source_audio_Id;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetSourceAudioId(const DWORD source_audio_Id)
{
  PTRACE(eLevelInfoNormal , "CCommConf::SetSourceAudioId , AUDIOSRC");
  m_source_audio_Id=source_audio_Id;

  FASTCHANGE;
	m_pSubject->Notify(AUDIOSRC, m_source_audio_Id);
}

////////////////////////////////////////////////////////////////////////////
DWORD  CCommConf::GetLSDSourceId() const
{
	return m_source_LSD_Id;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetLSDSourceId(const DWORD source_LSD_Id)
{
   m_source_LSD_Id=source_LSD_Id;
  FASTCHANGE;
}

////////////////////////////////////////////////////////////////////////////
DWORD  CCommConf::GetEPCContentSourceId() const
{
	return m_EPC_Content_source_Id;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetEPCContentSourceId(const DWORD EPC_Content_source_Id)
{
  m_EPC_Content_source_Id=EPC_Content_source_Id;
  IncreaseSummaryUpdateCounter();
  FASTCHANGE;
}

////////////////////////////////////////////////////////////////////////////
DWORD  CCommConf::GetStatus() //const
{
	return m_conf_status;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetStatus(const DWORD status)
{
	if(m_conf_status != status){
        IncreaseSummaryUpdateCounter();
		FASTCHANGE;
	}
	m_conf_status=status;
}

////////////////////////////////////////////////////////////////////////////
WORD  CCommConf::Is_TerminatingState() const
{
	  if ((m_conf_status & 0x80000000)==0x80000000)
	  return TRUE;
	else
	  return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD  CCommConf::FindPartyByBondingTelNum(const char* calledPhoneNumber, CConfParty** ppConfParty)
{
	CConfParty* pTempConfParty;
	*ppConfParty = NULL;
	for (int i=0;i<GetNumParties();i++)
	{
		if (i==0)
			pTempConfParty = GetFirstParty();
		else
			pTempConfParty = GetNextParty();

		if ((pTempConfParty != NULL )
			&& (pTempConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE)
			&& (pTempConfParty->GetConnectionType() == DIAL_IN))
			{
				if(ComparePhones(calledPhoneNumber, pTempConfParty->GetBondingTmpNumber()) == 0)
				{
					*ppConfParty = pTempConfParty;
					return 1;
				}
			}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
int  CCommConf::ComparePhones(const char* incomming_telNumber,const char* db_telNumber) const
{
  //return 0 if suffix of the given incomming phone numbers is the same
  //as the suffix of the phone number stored in database
  int index=0;

  if (strlen(incomming_telNumber) > strlen(db_telNumber))
    index = (strlen(incomming_telNumber)-strlen(db_telNumber));

  const char *suffix = &incomming_telNumber[index];

  // If one of the phones is an empty string then result = UNEQUAL.
  if (strlen(suffix)==0 || strlen(db_telNumber)==0)
    return 1;

  return strcmp(suffix,db_telNumber);
}

/////////////////////////////////////////////////////////////////////////////
WORD  CCommConf::IsConfPhone(const char* telNumber)
{
  Phone*  phone = NULL;
  WORD	result = FALSE;
  WORD	numPhones = 0;

  for (int i=0;i<(int) m_numServicePhoneStr; i++){
    numPhones = m_pServicePhoneStr[i]->GetNumPhoneNumbers();

    for (int j=0; j<numPhones; j++){
      // Romem klocwork
      if(j >= MAX_PHONE_NUMBERS_IN_CONFERENCE)
    	  continue;
      phone = m_pServicePhoneStr[i]->m_pPhoneNumberList[j];
      if (phone!=NULL && !ComparePhones(telNumber,phone->phone_number))
	result = TRUE;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////
// in cases the isdn operator add prefix to the dial in number
// we want to allocate temporery bonding number from the same span
// of the conf dial in number (Eitan, 8/08)
Phone* CCommConf::GetActualConfPhone(char* dialingTelNumber,const char* party_service_name)
{
	Phone*  phone = NULL;
	WORD	result = FALSE;
	WORD	numPhones = 0;

	 for (int i=0;i<(int) m_numServicePhoneStr; i++)
	 {
		 if(!strncmp(m_pServicePhoneStr[i]->GetNetServiceName(),party_service_name,NET_SERVICE_PROVIDER_NAME_LEN))
		 {
			 numPhones = m_pServicePhoneStr[i]->GetNumPhoneNumbers();

			 for (int j=0; j<numPhones; j++)
			 {
				 // Romem klockwork
				 if(j>= MAX_PHONE_NUMBERS_IN_CONFERENCE)
					 continue;
				 phone = m_pServicePhoneStr[i]->m_pPhoneNumberList[j];
				 if (phone!=NULL && !ComparePhones(dialingTelNumber,phone->phone_number))
					 return phone;
			 }
		 }
	  }
	  return NULL;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CCommConf::GetLSDRate() const
{
	return m_LSDRate;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetLSDRate(BYTE LSDRate)
{
	m_LSDRate = LSDRate;
	SLOWCHANGE;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CCommConf::GetCurConfVideoLayout() const
{
  return m_curConfVideoLayout;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetCurConfVideoLayout(BYTE curConfVideoLayout)
{
	m_curConfVideoLayout = curConfVideoLayout;
	SLOWCHANGE;

	if (CPObject::IsValidPObjectPtr(m_pSubject))
		m_pSubject->Notify(CPCONFLAYOUT, 0);
}

////////////////////////////////////////////////////////////////////////////
WORD   CCommConf::GetNumVideoLayout() const
{
	return m_numVideoLayout;
}

////////////////////////////////////////////////////////////////////////////
void   CCommConf::SetNumVideoLayout(const WORD num)
{
	m_numVideoLayout = num;
	SLOWCHANGE;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddNewVideoLayout(int nLayout, int bActive, CVideoLayout* pRetLayout)
{
	CVideoLayout vLayout;

	vLayout.SetScreenLayout(nLayout);

	int nCells = vLayout.Layout2SrcNum(nLayout);

	for(int i=0;i<nCells;i++)
	{
		CVideoCellLayout cell;
		cell.SetCellId(i+1);
		cell.SetCurrentPartyId(0xffffffff);
		cell.SetAudioActivated();
		vLayout.AddCell(cell);
	}

	vLayout.SetActive(bActive);
	AddVideoLayout(vLayout);

	if(pRetLayout)
		*pRetLayout = vLayout;
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::AddVideoLayout(const CVideoLayout &other)
{
  if (m_numVideoLayout >= MAX_VIDEO_LAYOUT_NUMBER) {
    PASSERTMSG_AND_RETURN_VALUE(1, "Failed, maximum video layouts exceeded", STATUS_MAX_VIDEO_LAYOUTS_EXCEEDED);
  }
  if (FindVideoLayout(other) != NOT_FIND)
    return UpdateVideoLayout(other);

  m_pVideoLayout[m_numVideoLayout] = new CVideoLayout(other);
  m_numVideoLayout++;

  SLOWCHANGE;

  //Set other layouts to unactive state
  if (other.IsActive())
  {
    for (int i = 0; i < m_numVideoLayout - 1; i++)
      m_pVideoLayout[i]->SetActive(NO);

    SetCurConfVideoLayout(other.GetScreenLayout());
  }
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int	CCommConf::UpdateVideoLayout(const CVideoLayout &other)
{
  int ind;
  ind=FindVideoLayout(other);
  if (ind==NOT_FIND) return STATUS_VIDEO_SOURCE_NOT_EXISTS;
  // Romem klocwork
  if (ind>=MAX_VIDEO_LAYOUT_NUMBER) return STATUS_VIDEO_SOURCE_NOT_EXISTS;

  PASSERTSTREAM_AND_RETURN_VALUE(m_numVideoLayout > MAX_VIDEO_LAYOUT_NUMBER,
  	"m_numVideoLayout has invalid value " << m_numVideoLayout, STATUS_FAIL);

  delete m_pVideoLayout[ind];
  m_pVideoLayout[ind] = new CVideoLayout(other);

  //Set other layouts to unactive state
  if (other.IsActive()){
		for (int i=0; i<m_numVideoLayout; i++)
			if (i!=ind)
				m_pVideoLayout[i]->SetActive(NO);

		SetCurConfVideoLayout(other.GetScreenLayout());
  }
  SLOWCHANGE;
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int	CCommConf::CancelVideoLayout(const BYTE screenLayout)
{
  int ind;
  ind=FindVideoLayout(screenLayout);
  if (ind==NOT_FIND) return STATUS_VIDEO_SOURCE_NOT_EXISTS;
  // Romem klocwork
   if (ind>=MAX_VIDEO_LAYOUT_NUMBER) return STATUS_VIDEO_SOURCE_NOT_EXISTS;

  PASSERTSTREAM_AND_RETURN_VALUE(m_numVideoLayout > MAX_VIDEO_LAYOUT_NUMBER,
    "m_numVideoLayout has invalid value " << m_numVideoLayout, STATUS_FAIL);

  POBJDELETE(m_pVideoLayout[ind]);
  int i;
  for (i=0;i<(int)m_numVideoLayout;i++)
  {
	  if (m_pVideoLayout[i]==NULL)
		 break;
  }
  for (int j=i;j<(int)m_numVideoLayout-1;j++)
  {
	 m_pVideoLayout[j]=m_pVideoLayout[j+1] ;
  }
  m_pVideoLayout[m_numVideoLayout-1] = NULL;
  m_numVideoLayout--;

  SLOWCHANGE;
  return   STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int	CCommConf::FindVideoLayout(const CVideoLayout &other)
{
  for (int i=0;i<(int)m_numVideoLayout;i++)
  {
	 if (m_pVideoLayout[i]!=NULL) {
		 if (m_pVideoLayout[i]->GetScreenLayout()==other.GetScreenLayout())
			return i;
	 }
  }
  return NOT_FIND;
}

////////////////////////////////////////////////////////////////////////////
int	CCommConf::FindVideoLayout(const BYTE screenLayout)
{
  for (int i=0;i<(int)m_numVideoLayout;i++)
  {
	 if (m_pVideoLayout[i]!=NULL) {
		 if (m_pVideoLayout[i]->GetScreenLayout()==screenLayout)
			return i;
	 }
  }
  return NOT_FIND;
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout* CCommConf::GetFirstVideoLayout()
{
	m_ind_vid_layout = 1;
	return m_pVideoLayout[0];
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout* CCommConf::GetNextVideoLayout()
{
	if (m_ind_vid_layout>=m_numVideoLayout) return NULL;
	return m_pVideoLayout[m_ind_vid_layout++];
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout* CCommConf::GetFirstVideoLayout(int& nPos)
{
   CVideoLayout* pVideoLayout = GetFirstVideoLayout();
   nPos = m_ind_vid_layout;
   return pVideoLayout;
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout* CCommConf::GetNextVideoLayout(int& nPos)
{
   m_ind_vid_layout = nPos;
   CVideoLayout* pVideoLayout = GetNextVideoLayout();
   nPos = m_ind_vid_layout;
   return pVideoLayout;
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout* CCommConf::GetVideoLayout(const BYTE screenLayout)
{
	int ind = FindVideoLayout(screenLayout);
	if (ind==NOT_FIND) return NULL;
	// Romem klocwork
    if(ind >= MAX_VIDEO_LAYOUT_NUMBER)
    	return NULL;
	return m_pVideoLayout[ind];
}

////////////////////////////////////////////////////////////////////////////
CVideoLayout* CCommConf::GetVideoLayout()
{
	int ind = FindVideoLayout(m_curConfVideoLayout);
	if (ind==NOT_FIND) return NULL;
	// Romem klocwork
	if(ind >= MAX_VIDEO_LAYOUT_NUMBER)
		return NULL;
	return m_pVideoLayout[ind];
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetVideoLayout(const CVideoLayout& other)
{
	int ind = FindVideoLayout(m_curConfVideoLayout);
	// Romem klocwork
	if (ind!=NOT_FIND && ind < MAX_VIDEO_LAYOUT_NUMBER){
		POBJDELETE(m_pVideoLayout[ind]);
		m_pVideoLayout[ind] = new CVideoLayout(other);
	}
	   SLOWCHANGE;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::RemoveAllLayouts( )
{
  for( int i=0; i<(int)m_numVideoLayout; i++ )
	  POBJDELETE(m_pVideoLayout[i]);
  m_numVideoLayout=0;
  m_ind_vid_layout=0;

  SLOWCHANGE;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::CopyVideoLayoutToReservationLayout()
{
	for (int i=0;i<MAX_VIDEO_LAYOUT_NUMBER;i++)
	{
		POBJDELETE(m_pRsrvVideoLayout[i]);

		if( m_pVideoLayout[i]!=NULL)
			m_pRsrvVideoLayout[i]= new CVideoLayout(*m_pVideoLayout[i]);
	}
	m_numRsrvVideoSource = m_numVideoLayout;
	m_ind_rsrv_vid_layout = m_ind_vid_layout;
	m_contPresScreenNumber = m_curConfVideoLayout;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetEndTime(const CStructTm &other)
{
	m_endTime=other;

	DWORD diffTime;
	diffTime = m_endTime - m_startTime;
	int hour = diffTime/3600;
	int min = (diffTime%3600)/60;
	int sec = (diffTime%3600)%60;
	CStructTm TempTime(0,0,0,hour,min,sec);
	m_duration = TempTime;

	IncreaseSummaryUpdateCounter();
	SLOWCHANGE;
}

////////////////////////////////////////////////////////////////////////////
void   CCommConf::SetIsMuteAllButX(DWORD MuteAllButX_Id)
{
	m_MuteAllButX_Id = MuteAllButX_Id;
}

////////////////////////////////////////////////////////////////////////////
WORD   CCommConf::IsMuteAllButX(DWORD PartyId)
{
	WORD result=FALSE;

	if((m_MuteAllButX_Id !=(DWORD)-1) && (m_MuteAllButX_Id == PartyId))
		result=TRUE;

	return result;
}

////////////////////////////////////////////////////////////////////////////
DWORD  CCommConf::GetMuteAllButX()const
{
	return m_MuteAllButX_Id;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::SetSummeryCreationUpdateCounter()
{
	m_SummeryCreationUpdateCounter=::GetpConfDB()->GetSummaryUpdateCounter();
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::AddVideoLayoutToResResponse(CXMLDOMElement* pResNode)
{
	CXMLDOMElement *pForceNode;
	CXMLDOMElement* pTempNode;
	CVideoLayout*  pVideoLayout;

	pVideoLayout=GetFirstVideoLayout();
	while (pVideoLayout)
	{
		if (pVideoLayout->IsActive())
		{

			pResNode->AddChildNode("LAYOUT",pVideoLayout->GetScreenLayoutITP(),LAYOUT_ENUM);
			break;
		}
		pVideoLayout=GetNextVideoLayout();
	}


	CXMLDOMElement *pForceListNode=pResNode->AddChildNode("FORCE_LIST");
	pVideoLayout=GetFirstVideoLayout();

	while (pVideoLayout)
	{

		pForceNode=pForceListNode->AddChildNode("FORCE");//checked
		pForceNode->AddChildNode("LAYOUT",pVideoLayout->GetScreenLayoutITP(),LAYOUT_ENUM);//checked
		CVideoCellLayout* pCell;
		WORD numb_of_cells =  pVideoLayout->m_numb_of_cell;

		if (pVideoLayout->GetScreenLayoutITP() == ONE_PLUS_TWO_OVERLAY_ITP)
			numb_of_cells = 3;
		for (int i=0;i<numb_of_cells;i++)
		{
			pCell=pVideoLayout->GetCurrentCell(i+1);
			if (pCell)
			{
				pTempNode=pForceNode->AddChildNode("CELL");//checked

				pTempNode->AddChildNode("ID",pCell->GetCellId());

				if (pCell->IsBlank())
				{
					PTRACE(eLevelInfoNormal,"CCommConf::AddVideoLayoutToResResponse, Blank screen");
					pTempNode->AddChildNode("FORCE_STATE", "blank");//checked
				}
				else if (pCell->IsAudioActivated())
				{
					pTempNode->AddChildNode("FORCE_STATE", "auto");//checked
				}
				else if (pCell->IsAutoScan())
				{
					pTempNode->AddChildNode("FORCE_STATE", "auto_scan");
				}
				else
				{
					pTempNode->AddChildNode("FORCE_STATE", "forced");//checked
					pTempNode->AddChildNode("FORCE_ID", pCell->GetForcedPartyId());//checked
					pTempNode->AddChildNode("FORCE_NAME", pCell->GetName());//checked
					pTempNode->AddChildNode("SOURCE_ID", pCell->GetCurrentPartyId());
				}
			}
		}
		pVideoLayout=GetNextVideoLayout();
	}
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::SetVideoLayoutParams(CXMLDOMElement* pResNode, char* pszError)
{
	int nStatus=STATUS_OK;
	CVideoLayout VideoLayout;
	CXMLDOMElement *pChildNode, *pForceNode;
	BYTE nByteVal;

	GET_VALIDATE_CHILD(pResNode,"LAYOUT",&m_curConfVideoLayout,LAYOUT_ENUM);

	if(nStatus == STATUS_OK)
	{
		CVideoLayout* pVideoLayout = GetVideoLayout(m_curConfVideoLayout);

		if(pVideoLayout)
		{
			VideoLayout = *pVideoLayout;
			VideoLayout.SetActive(YES);
			UpdateVideoLayout(VideoLayout);
		}
		else
			AddNewVideoLayout(m_curConfVideoLayout, YES);

		CVideoLayout vLayout;
		int nPos;

		pVideoLayout = GetFirstVideoLayout(nPos);

		while(pVideoLayout)
		{
			if(pVideoLayout->GetScreenLayout() != m_curConfVideoLayout)
			{
				vLayout = *pVideoLayout;
				vLayout.SetActive(NO);
				UpdateVideoLayout(vLayout);
			}
			pVideoLayout = GetNextVideoLayout(nPos);
		}
	}

	GET_CHILD_NODE(pResNode, "FORCE_LIST", pChildNode);

	if(pChildNode)
	{
		GET_FIRST_CHILD_NODE(pChildNode, "FORCE", pForceNode);

		while(pForceNode)
		{
			CVideoLayout vLayout;
			GET_VALIDATE_CHILD(pForceNode,"LAYOUT",&nByteVal,LAYOUT_ENUM);
			CVideoLayout* pVideoLayout = GetVideoLayout(nByteVal);

			if(pVideoLayout == NULL)
			{
				AddNewVideoLayout(nByteVal, NO);
				vLayout.SetActive(NO);
			}
			else
				vLayout = *pVideoLayout;

			nStatus = vLayout.DeSerializeXml(pForceNode,pszError,FALSE);

			if(nStatus != STATUS_OK)
				return nStatus;

			UpdateVideoLayout(vLayout);

			GET_NEXT_CHILD_NODE(pChildNode, "FORCE", pForceNode);
		}
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
DWORD CCommConf::AttachObserver(COsQueue* pObserver, WORD event, WORD type, DWORD observerInfo1)
{
	DWORD status = STATUS_OK;
	m_pSubject->AttachObserver(pObserver, event, type, observerInfo1);
	return status;
}

////////////////////////////////////////////////////////////////////////////
int CCommConf::DetachObserver(COsQueue* pObserver)
{
	int status = STATUS_OK;
	m_pSubject->DetachObserver(pObserver);
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCommConf::SearchPartyByIPOrAlias(mcTransportAddress ipAddress,const std::string partyAlias,WORD PartyAliasType) const
{
  const CRsrvParty * pCurrParty =0;
  char tempName[IPV6_ADDRESS_LEN];
  memset (&tempName,'\0',IPV6_ADDRESS_LEN);

  for (int i=0;i<(int)m_numParties;i++)
  {
      pCurrParty = m_pParty[i];

      //Make sure IP is not the smae and different from 0 !
      // IpV6
      mcTransportAddress currPartyIpAddr = pCurrParty->GetIpAddress();
      if (!isApiTaNull(&ipAddress))
      {
      		if (ipAddress.ipVersion == currPartyIpAddr.ipVersion)
      		{
      			if (ipAddress.ipVersion == (APIU32)eIpVersion4)
      			{
      				if (ipAddress.addr.v4.ip == currPartyIpAddr.addr.v4.ip)
      			    {
      			    	PTRACE(eLevelInfoNormal,"CCommConf::SearchPartyByIPOrAlias ,IPV4 is not the smae and different from 0");
      			    	TRACESTR (eLevelInfoNormal) <<"CCommConf::SearchPartyByIPOrAlias , ipAddress = " << ::ipToString(ipAddress,tempName,1);
      			    	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
      			    	TRACESTR (eLevelInfoNormal) <<"CCommConf::SearchPartyByIPOrAlias , pCurrParty->GetIpAddress () = " << ::ipToString(pCurrParty->GetIpAddress(),tempName,1);
      					return STATUS_PARTY_IP_ALIAS_ALREADY_EXISTS | WARNING_MASK;
      			    }

      			}
      			else if (ipAddress.ipVersion == (APIU32)eIpVersion6)
      			{
      				if (!memcmp((ipAddress.addr.v6.ip),(currPartyIpAddr.addr.v6.ip),IPV6_ADDRESS_BYTES_LEN))
      			    {
      			    	PTRACE(eLevelInfoNormal,"CCommConf::SearchPartyByIPOrAlias ,IPV6 is not the smae and different from 0");
      			    	TRACESTR (eLevelInfoNormal) <<"CCommConf::SearchPartyByIPOrAlias , ipAddress = " << ::ipToString(ipAddress,tempName,1);
      			    	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
      			    	TRACESTR (eLevelInfoNormal) <<"CCommConf::SearchPartyByIPOrAlias , pCurrParty->GetIpAddress () = " << ::ipToString(pCurrParty->GetIpAddress(),tempName,1);
      					return STATUS_PARTY_IP_ALIAS_ALREADY_EXISTS | WARNING_MASK;
      			    }
      			}
      		}
      }

      //Make sure the Alias and the alias type are different
      if ( partyAlias.size() > 0
	   && partyAlias == pCurrParty->GetH323PartyAlias ()
	   && PartyAliasType == pCurrParty->GetH323PartyAliasType ())
	   {
	   		PTRACE(eLevelInfoNormal,"CCommConf::SearchPartyByIPOrAlias ,Alias and the alias type are different");
	  		return STATUS_PARTY_IP_ALIAS_ALREADY_EXISTS | WARNING_MASK ;
	   }
   }
  return STATUS_PARTY_DOES_NOT_EXIST;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCommConf::SearchPartyByIP(mcTransportAddress* ipAddress)
{
  CRsrvParty * pCurrParty =0;

  for (int i=0;i<(int)m_numParties;i++)
    {
      pCurrParty = m_pParty[i];
      mcTransportAddress ip = pCurrParty->GetIpAddress();

      //Make sure IP is not the same and different from 0 !
    if (!isApiTaNull(ipAddress) && isIpAddressEqual(ipAddress,&ip) == TRUE)
		return STATUS_PARTY_IP_ALREADY_EXISTS | WARNING_MASK;
    }
  return STATUS_PARTY_DOES_NOT_EXIST;
}

////////////////////////////////////////////////////////////////////////////
STATUS CCommConf::SearchPartyByAlias(const std::string partyAlias,WORD PartyAliasType)
{
  CRsrvParty * pCurrParty =0;

  for (int i=0;i<(int)m_numParties;i++)
    {
      pCurrParty = m_pParty[i];

      //Make sure the Alias and the alias type are different
      if ( partyAlias.size() > 0
	   && partyAlias == pCurrParty->GetH323PartyAlias ()
	   && PartyAliasType == pCurrParty->GetH323PartyAliasType ())
	return STATUS_PARTY_ALIAS_ALREADY_EXISTS | WARNING_MASK;
    }
  return STATUS_PARTY_DOES_NOT_EXIST;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCommConf::IsActiveSlaveConf()
{
  CConfParty* pCurrParty =0;
  for (int i=0;i<(int)m_numParties;i++)
  {
    pCurrParty = (CConfParty*)m_pParty[i];

   //Party is CASCADE_MODE_SLAVE and connected
   if ( CASCADE_MODE_SLAVE == pCurrParty->GetCascadeMode() && (pCurrParty->IsAllowDisconnect()))
   	return YES;
  }
  return NO;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCommConf::IncludeRecordingParty()
{
	CRsrvParty * pCurrParty =0;
    for (int i=0;i<(int)m_numParties;i++)
    {
    	pCurrParty = m_pParty[i];
    	if(pCurrParty && pCurrParty->GetRecordingLinkParty())
    	{
    		return YES;
    	}
    }
    return NO;
}
////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
BYTE CCommConf::IncludePlaybackParty()
{
	CRsrvParty * pCurrParty =0;
    for (int i=0;i<(int)m_numParties;i++)
    {
    	pCurrParty = m_pParty[i];
    	if(pCurrParty && pCurrParty->GetPlaybackLinkParty())
    	{
    		return YES;
    	}
    }
    return NO;
}


////////////////////////////////////////////////////////////////////////////
WORD CCommConf::GetNumVideoParties() const
{
	return m_numVideoParties;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::SetNumVideoParties(WORD numVideoParties)
{
	PTRACE2INT(eLevelInfoNormal,"CCommConf::SetNumVideoParties",numVideoParties);
	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	WORD maxVideoPartiesInConfPerSystemMode = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
	if(numVideoParties <= maxVideoPartiesInConfPerSystemMode)
	{
		m_numVideoParties = numVideoParties;
	}
	else
	{
		PASSERT(numVideoParties);
	}
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::OperatorMovePartyFromConf(const char* operName,const char* partyName,DWORD sourcePartyId,WORD event,char* destConfName,DWORD targetConfId/*,DWORD sourceConfId*/)
{
  CStructTm curTime;
  PASSERT(SystemGetTime(curTime));

  COperMoveParty* pOperMoveParty = new COperMoveParty;

  pOperMoveParty->SetOperatorName(operName);
  pOperMoveParty->SetPartyName(partyName);
  pOperMoveParty->SetPartyId(sourcePartyId);
  pOperMoveParty->SetDestConfName(destConfName);
  pOperMoveParty->SetDestConfId(targetConfId);

  CCdrEvent cdrEvent;
  cdrEvent.SetCdrEventType( event);
  cdrEvent.SetTimeStamp(curTime);
  cdrEvent.SetOperMoveParty(pOperMoveParty);
  CCdrLogApi cdrApi;
  cdrApi.ConferenceEvent(m_confId, cdrEvent);

  PlcmCdrEventConfOperatorMoveParty movePartyEventFromCdr;
  movePartyEventFromCdr.m_operatorName = operName;
  movePartyEventFromCdr.m_partyName = partyName;
  movePartyEventFromCdr.m_partyId = sourcePartyId;
  movePartyEventFromCdr.m_destConfName = destConfName;
  movePartyEventFromCdr.m_destConfId = targetConfId;
  movePartyEventFromCdr.m_action = eOperatorMovePartyAction_MovePartyFromConference;

  SendCdrEvendToCdrManager((ApiBaseObjectPtr)&movePartyEventFromCdr, false, sourcePartyId);
  POBJDELETE(pOperMoveParty);
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::OperatorMovePartyToConfEventToCdr(CConfParty* pConfParty,const char* operName,char* sourceConfName,DWORD sourceConfId, Phone * CallingNum, Phone * CalledNum)
{
	PlcmCdrEventConfOperatorMoveParty movePartyEventToCdr;
	movePartyEventToCdr.m_action = eOperatorMovePartyAction_MovePartyToConference;

	movePartyEventToCdr.m_operatorName = operName;
	movePartyEventToCdr.m_srcConfName = sourceConfName;
	movePartyEventToCdr.m_srcConfId = sourceConfId;
	movePartyEventToCdr.m_partyDetails.m_name = pConfParty->GetName();
	movePartyEventToCdr.m_partyDetails.m_id = pConfParty->GetPartyId();
	movePartyEventToCdr.m_partyDetails.m_connection = ConvertConnectionType(pConfParty->GetConnectionTypeOper());
	movePartyEventToCdr.m_partyDetails.m_bonding = ConvertBondingType(pConfParty->GetBondingMode1());
	movePartyEventToCdr.m_partyDetails.m_netChannelNumber = (NetChannelNumType)pConfParty->GetNetChannelNumber();
	movePartyEventToCdr.m_partyDetails.m_serviceName = (char*)(pConfParty->GetServiceProviderName());
	movePartyEventToCdr.m_partyDetails.m_resrtictMode = eRestrictModeType_Derestricted;
	//movePartyEventToCdr.m_partyDetails.m_ = pConfParty->GetVoice();voice?
	movePartyEventToCdr.m_partyDetails.m_numType = ConvertNumType(pConfParty->GetNumType());
	movePartyEventToCdr.m_partyDetails.m_subServiceName = (char*)pConfParty->GetSubServiceName();
	movePartyEventToCdr.m_partyDetails.m_identificationMethod = (IdentMethodType)pConfParty->GetIdentificationMethod();
	movePartyEventToCdr.m_partyDetails.m_meetMeMethod = ConvertMeetMeMethod(pConfParty->GetMeet_me_method());

	Phone* pPhoneNum = pConfParty->GetActualPartyPhoneNumber(0);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_phoneList.m_phone1 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualPartyPhoneNumber(1);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_phoneList.m_phone2 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualPartyPhoneNumber(2);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_phoneList.m_phone3 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualPartyPhoneNumber(3);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_phoneList.m_phone4 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualPartyPhoneNumber(4);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_phoneList.m_phone5 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualPartyPhoneNumber(5);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_phoneList.m_phone6 = pPhoneNum->phone_number;

	/////////////
	pPhoneNum = pConfParty->GetActualMCUPhoneNumber(0);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_mcuPhoneList.m_phone1 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualMCUPhoneNumber(1);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_mcuPhoneList.m_phone2 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualMCUPhoneNumber(2);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_mcuPhoneList.m_phone3 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualMCUPhoneNumber(3);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_mcuPhoneList.m_phone4 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualMCUPhoneNumber(4);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_mcuPhoneList.m_phone5 = pPhoneNum->phone_number;

	pPhoneNum = pConfParty->GetActualMCUPhoneNumber(5);
	if (pPhoneNum != NULL)
		movePartyEventToCdr.m_partyDetails.m_mcuPhoneList.m_phone6 = pPhoneNum->phone_number;

	movePartyEventToCdr.m_partyDetails.m_nodeType = (NodeType)pConfParty->GetNodeType ();
	movePartyEventToCdr.m_partyDetails.m_chair = pConfParty->GetIsLeader();
	movePartyEventToCdr.m_partyDetails.m_password = pConfParty->GetPassword ();
	char tempName[64];
	memset (&tempName,'\0',IPV6_ADDRESS_LEN);
	ipToString(pConfParty->GetIpAddress(),tempName,1);
	movePartyEventToCdr.m_partyDetails.m_ipAddress = tempName;//ipStr;
	movePartyEventToCdr.m_partyDetails.m_signallingPort = pConfParty->GetCallSignallingPort ();
	movePartyEventToCdr.m_partyDetails.m_videoProtocol = ConvertVideoProtocolType(GetVideoProtocol());
	movePartyEventToCdr.m_partyDetails.m_videoBitRate = CCdrPersistConverter::ConvertBitRate(pConfParty->GetVideoRate ());

	BYTE bIpType = pConfParty->GetNetInterfaceType();

	if (H323_INTERFACE_TYPE == bIpType)
	{
		movePartyEventToCdr.m_partyDetails.m_alias.m_aliasType =  ConvertAliasType(pConfParty->GetH323PartyAliasType ());
		movePartyEventToCdr.m_partyDetails.m_alias.m_name = pConfParty->GetH323PartyAlias ();
	}
	else
	{
		if (SIP_INTERFACE_TYPE == bIpType)
		{
			movePartyEventToCdr.m_partyDetails.m_alias.m_aliasType = ConvertAliasType(pConfParty->GetSipPartyAddressType());
			movePartyEventToCdr.m_partyDetails.m_alias.m_name = pConfParty->GetSipPartyAddress();
		}
	}
	movePartyEventToCdr.m_partyDetails.m_volume = pConfParty->GetAudioVolume () ;
	movePartyEventToCdr.m_partyDetails.m_undefined = pConfParty->GetUndefinedType ();

	movePartyEventToCdr.m_partyName = pConfParty->GetName();
	movePartyEventToCdr.m_partyId = pConfParty->GetPartyId();

	if (CallingNum!= NULL)
	{
		const char* pCallingNum = CallingNum->phone_number;
		if(strcmp(pCallingNum,""))
			movePartyEventToCdr.m_phone1 = pCallingNum;
	}
	// send move party called number CDR event
	if (CalledNum!= NULL)
	{
		const char* pCalledNum = CalledNum->phone_number;
		if(strcmp(pCalledNum,""))
			movePartyEventToCdr.m_phone1 = pCalledNum;
	}
	movePartyEventToCdr.m_partyDetails.m_conferenceCorrelationId = GetCorrelationId();

	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&movePartyEventToCdr, false, pConfParty->GetPartyId(), pConfParty->GetCorrelationId());

}
////////////////////////////////////////////////////////////////////////////
void  CCommConf::OperatorMovePartyToConf(CConfParty* pConfParty,const char* operName,char* sourceConfName,DWORD sourceConfId/*,DWORD sourcePartyId*/, WORD event)
{
	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	COperMoveToConf* pOperMoveToConf = new COperMoveToConf;

	pOperMoveToConf->SetOperatorName(operName);
	pOperMoveToConf->SetSourceConfName(sourceConfName);
	pOperMoveToConf->SetSourceConfId(sourceConfId);
	pOperMoveToConf->GetAddPartyDetail()->SetPartyName(pConfParty->GetName());
	pOperMoveToConf->GetAddPartyDetail()->SetPartyId(pConfParty->GetPartyId());
	pOperMoveToConf->GetAddPartyDetail()->SetConnectionType(pConfParty->GetConnectionTypeOper());
	pOperMoveToConf->GetAddPartyDetail()->SetBondingMode(pConfParty->GetBondingMode1());
	pOperMoveToConf->GetAddPartyDetail()->SetNetNumberChannel(pConfParty->GetNetChannelNumber());
	pOperMoveToConf->GetAddPartyDetail()->SetNetServiceName((char*)(pConfParty->GetServiceProviderName()));
	pOperMoveToConf->GetAddPartyDetail()->SetRestrict(pConfParty->GetRestrict());
	pOperMoveToConf->GetAddPartyDetail()->SetVoice(pConfParty->GetVoice());
	pOperMoveToConf->GetAddPartyDetail()->SetNumType(pConfParty->GetNumType());
	pOperMoveToConf->GetAddPartyDetail()->SetNetSubServiceName((char*)(pConfParty->GetSubServiceName()));
	pOperMoveToConf->GetAddPartyDetail()->SetIdentMethod(pConfParty->GetIdentificationMethod());
	pOperMoveToConf->GetAddPartyDetail()->SetMeetMeMethod(pConfParty->GetMeet_me_method());

	WORD indPhone = 0;
	Phone* pPhoneNum = pConfParty->GetActualPartyPhoneNumber(indPhone);
	while (pPhoneNum != NULL) {
		   pOperMoveToConf->GetAddPartyDetail()->AddPartyPhoneNumber(pPhoneNum->phone_number);
		   indPhone++;
		   pPhoneNum = pConfParty->GetActualPartyPhoneNumber(indPhone);
	}
	indPhone = 0;
	pPhoneNum = pConfParty->GetActualMCUPhoneNumber(indPhone);
	while (pPhoneNum != NULL) {
		   pOperMoveToConf->GetAddPartyDetail()->AddMcuPhoneNumber(pPhoneNum->phone_number);
		   indPhone++;
		   pPhoneNum = pConfParty->GetActualMCUPhoneNumber(indPhone);
	}
	pOperMoveToConf->GetAddPartyDetail()->SetNodeType(pConfParty->GetNodeType ());
	pOperMoveToConf->GetAddPartyDetail()->SetChair(pConfParty->GetIsLeader());
	pOperMoveToConf->GetAddPartyDetail()->SetPassword(pConfParty->GetPassword ());
	pOperMoveToConf->GetAddPartyDetail()->SetIpAddress((DWORD)((pConfParty->GetIpAddress()).addr.v4.ip));
	pOperMoveToConf->GetAddPartyDetail()->SetCallSignallingPort(pConfParty->GetCallSignallingPort ());
	pOperMoveToConf->GetAddPartyDetail()->SetVideoProtocol(pConfParty->GetVideoProtocol ());
	pOperMoveToConf->GetAddPartyDetail()->SetVideoRate(pConfParty->GetVideoRate ());

	BYTE bIpType = pConfParty->GetNetInterfaceType();
	if (H323_INTERFACE_TYPE == bIpType)
    {
		pOperMoveToConf->GetAddPartyDetail()->SetIpPartyAliasType(pConfParty->GetH323PartyAliasType ());
		pOperMoveToConf->GetAddPartyDetail()->SetIpPartyAlias(pConfParty->GetH323PartyAlias ());
	}
	else
	{
		if (SIP_INTERFACE_TYPE == bIpType)
	   {
		  pOperMoveToConf->GetAddPartyDetail()->SetIpPartyAliasType(pConfParty->GetSipPartyAddressType());
		  pOperMoveToConf->GetAddPartyDetail()->SetIpPartyAlias(pConfParty->GetSipPartyAddress());
	   }
	}

	pOperMoveToConf->GetAddPartyDetail()->SetAudioVolume(pConfParty->GetAudioVolume ());
	pOperMoveToConf->GetAddPartyDetail()->SetUndefinedType(pConfParty->GetUndefinedType ());
	pOperMoveToConf->GetAddPartyDetail()->SetNetInterfaceType(pConfParty->GetNetInterfaceType ());

	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(event);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetOperMoveToConf(pOperMoveToConf);

	CCdrLogApi cdrApi;
	cdrApi.ConferenceEvent(m_confId, cdrEvent);
	POBJDELETE(pOperMoveToConf);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::CDRPartyCallingNumber_Move_to_Cont_1(DWORD confID,const char* party_name,DWORD party_id, const char* partyCallingNum)
{
  CStructTm curTime;
  PASSERT(SystemGetTime(curTime));

  CCDRPartyCalling_NumMoveToCont1* pPartyCalling_NumMoveToCont1=new CCDRPartyCalling_NumMoveToCont1;

   pPartyCalling_NumMoveToCont1->SetPartyName(party_name);
   pPartyCalling_NumMoveToCont1->SetPartyId( party_id);
   pPartyCalling_NumMoveToCont1->SetPartyCallingNum(partyCallingNum);

  CCdrEvent cdrEvent;
  cdrEvent.SetCdrEventType(OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1);
  cdrEvent.SetTimeStamp(curTime);
  cdrEvent.SetPartyCalling_Num(pPartyCalling_NumMoveToCont1);

  CCdrLogApi cdrApi;
  cdrApi.ConferenceEvent(m_confId, cdrEvent);

  POBJDELETE(pPartyCalling_NumMoveToCont1);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::CDRPartyCalledNumber_Move_to_Cont_2(DWORD confID,const char* party_name,DWORD party_id, const char* partyCalledNum)
{
  CStructTm curTime;
  PASSERT(SystemGetTime(curTime));

  CCDRPartyCalled_NumMoveToCont2* pPartyCalled_NumMoveToCont2=new CCDRPartyCalled_NumMoveToCont2;

   pPartyCalled_NumMoveToCont2->SetPartyName(party_name);
   pPartyCalled_NumMoveToCont2->SetPartyId( party_id);
   pPartyCalled_NumMoveToCont2->SetPartyCalledNum(partyCalledNum);

  CCdrEvent cdrEvent;
  cdrEvent.SetCdrEventType(OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2);
  cdrEvent.SetTimeStamp(curTime);
  cdrEvent.SetPartyCalled_Num(pPartyCalled_NumMoveToCont2);
  CCdrLogApi cdrApi;
  cdrApi.ConferenceEvent(m_confId, cdrEvent);

  POBJDELETE(pPartyCalled_NumMoveToCont2);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::ChairPersonEnteredCDR( const CConfParty* pConfParty)
{
	PTRACE(eLevelInfoNormal,"CCommConf::ChairPersonEnteredCDR - CDR-Chair - begin");
	DWORD dwPartyMonitorID = pConfParty->GetPartyId();
	const char* pName = pConfParty->GetName();
	bool bChair = pConfParty->GetIsLeader();
	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));

	CCDRPartyChairPerson* pPartyChairPerson = new CCDRPartyChairPerson;

	pPartyChairPerson->SetPartyName(pName);
	pPartyChairPerson->SetPartyId(dwPartyMonitorID);
	pPartyChairPerson->SetIsChairPerson(bChair);

	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(PARTY_CHAIR_UPDATE);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetPartyChairPerson(pPartyChairPerson);
	CCdrLogApi cdrApi;
	cdrApi.ConferenceEvent(m_confId, cdrEvent);

	//Send PARTY_CHAIR_UPDATE to CdrManager
	PlcmCdrEventConfDataUpdate confDataUpdate;
	confDataUpdate.m_chair = bChair;
	SendCdrEvendToCdrManager((ApiBaseObjectPtr)&confDataUpdate);

	POBJDELETE(pPartyChairPerson);
	PTRACE(eLevelInfoNormal,"CCommConf::ChairPersonEnteredCDR - CDR-Chair - end");
}

////////////////////////////////////////////////////////////////////////////
WORD  CCommConf::ResetPublicOperatorAssistanceParties()
{
  WORD party_reset = 0;
  for (int i=0;i<(int)m_numParties;i++){
    if (m_pParty[i]!=NULL) {
      if ( ((CConfParty*)m_pParty[i])->GetWaitForOperAssistance() == WAIT_FOR_OPER_ON_REQ_PUBLIC){
	((CConfParty*)m_pParty[i])->SetWaitForOperAssistance(WAIT_FOR_OPER_NONE);
	party_reset++;

      }
    }
  }
  return party_reset;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::updateExclusiveContent(CConfParty* pConfParty, BOOL setExclusiveContent)
{
	TRACESTR(eLevelInfoNormal) << "CCommConf::updateExclusiveContent:" << " isExclusiveContentSet=" << (bool)setExclusiveContent << endl;

  for (int i = 0; i < (int)m_numParties; ++i)
  {
    CConfParty* pParty = ((CConfParty*)m_pParty[i]);

    if (pConfParty == NULL && setExclusiveContent == false) // Remove "exclusive content" for all parties
    {
      if (pParty->isExclusiveContentOwner())
      {
        TRACESTR(eLevelInfoNormal) << "CCommConf::updateExclusiveContent SetExclusiveContentOwner(FALSE) to partyId=:" << pParty->GetPartyId() << endl;
        pParty->SetExclusiveContentOwner(FALSE);
      }
      continue;
    }
    if( !pConfParty )
    	continue;

    if (setExclusiveContent)                                // Set "exclusive content" for specific party and remove to other
    {
      if (pParty->GetPartyId() == pConfParty->GetPartyId())
      {
        if (!pParty->isExclusiveContentOwner())
        {
          TRACESTR(eLevelInfoNormal) << "CCommConf::updateExclusiveContent SetExclusiveContentOwner(TRUE) to partyId=:" << pParty->GetPartyId() << endl;
          pParty->SetExclusiveContentOwner(TRUE);
        }
      }
      else
      {
        if (pParty->isExclusiveContentOwner())
        {
          TRACESTR(eLevelInfoNormal) << "CCommConf::updateExclusiveContent SetExclusiveContentOwner(FALSE) to partyId=:" << pParty->GetPartyId() << endl;
          pParty->SetExclusiveContentOwner(FALSE);
        }
      }
    }
    else                                                    // Remove "exclusive content" for specific party
    {
      if (pParty->GetPartyId() == pConfParty->GetPartyId())
      {
        if (pParty->isExclusiveContentOwner())
        {
          TRACESTR(eLevelInfoNormal) << "CCommConf::updateExclusiveContent SetExclusiveContentOwner(FALSE) to partyId=:" << pParty->GetPartyId() << endl;
          pParty->SetExclusiveContentOwner(FALSE);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
BOOL CCommConf::isExclusiveContent()
{
	PTRACE(eLevelInfoNormal,"CCommConf::isExclusiveContent");
  for (int i = 0; i < (int)m_numParties; ++i)
		if (((CConfParty*)m_pParty[i])->isExclusiveContentOwner())
			return TRUE;
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
const char* CCommConf::GetExclusiveContent()
{
	for (int i = 0; i < (int)m_numParties; ++i)
		if (((CConfParty*)m_pParty[i])->isExclusiveContentOwner())
			return m_pParty[i]->GetName();
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::UpdateHotBackupFields()
{
       DWORD maxMonitorPartyId = 0;
       DWORD currentMonitorPartyId = 0;

	for (int i=0; i < (int)m_numParties; i++)
	{
	  if (m_pParty[i] != NULL){
	    	((CConfParty*)m_pParty[i])->UpdateHotBackupFields();

		currentMonitorPartyId = ((CConfParty*)m_pParty[i])->GetPartyId();
		if(currentMonitorPartyId > maxMonitorPartyId){
		  maxMonitorPartyId = currentMonitorPartyId;
		}
	  }
	}
	maxMonitorPartyId++; // next = max existing + 1
	SetNextPartyId(maxMonitorPartyId);
	PTRACE2INT(eLevelInfoNormal,"CCommConf::UpdateHotBackupFields setting nextMonitorPartyId = ", maxMonitorPartyId);
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::RestoreHotBackupFields()
{
	for (int i=0; i < (int)m_numParties; i++)
	{
		if (m_pParty[i] != NULL)
			((CConfParty*)m_pParty[i])->RestoreHotBackupFields();
	}
	//check if the conference was in recording MODE  ==> Change to start immediatly
	if ( (eStartRecording == GetRecordingLinkControl()) || (eResumeRecording == GetRecordingLinkControl()) )
		SetStartRecPolicy(START_RECORDING_IMMEDIATELY);

}

////////////////////////////////////////////////////////////////////////////
void CCommConf::UpdateParamsIfSlaveExistInConf()
{
	 if(!IsCOPReservation())
	 {
	    PTRACE(eLevelInfoNormal,"CCommConf::UpdateParamsIfSlaveExistInConf -not cop returnning!");
	    return;
	 }

	for ( int i=0; i<(int)m_numParties; i++ )
	{
		if (m_pParty[i]->GetCascadeMode() == CASCADE_MODE_SLAVE  && !m_pParty[i]->GetVoice() )
		{
			UpdateLectureModeAndLayoutBecauseSlaveInConf(m_pParty[i]->GetName());
			break;
		}
	}

}

////////////////////////////////////////////////////////////////////////////
void CCommConf::UpdateLectureModeAndLayoutBecauseSlaveInConf(const char* lecturer_name)
{
	PTRACE(eLevelInfoNormal,"CCommConf::UpdateLectureModeAndLayoutBecauseSlaveInConf");

	CLectureModeParams newLectureMode;
	newLectureMode.SetLectureModeType(1);
	newLectureMode.SetTimerOnOff(YES);
	newLectureMode.SetLecturerName(lecturer_name);
	SetIsSameLayout(NO);
    m_isAutoLayout = NO;
    if(IsCOPReservation())
    {
    	PTRACE(eLevelInfoNormal,"CCommConf::UpdateLectureModeAndLayoutBecauseSlaveInConf - cop set lecture mode");
    	SetLectureMode(newLectureMode);
    }

	CVideoLayout newVidLayout;
	AddNewVideoLayout(ONE_ONE,YES,&newVidLayout);
	CVideoLayout* currentActiveLayout = GetActiveRsrvVidLayout();
	if (currentActiveLayout != NULL)
		CancelRsrvVidLayout(currentActiveLayout->GetScreenLayout());
	else    //Yoella Sep-05-11 add protection but we need also to find the reason for this core the videoLayout shouldn`t be NULL
			PTRACE(eLevelInfoNormal,"CCommConf::UpdateLectureModeAndLayoutBecauseSlaveInConf - ERROR !!! Protection was added to avoid exception but Layout has to be valid");
	AddRsrvVidLayout(newVidLayout);

}

////////////////////////////////////////////////////////////////////////////
void CCommConf::SetLectureMode(const CLectureModeParams& otherLectureMode)
{
	CCommRes::SetLectureMode(otherLectureMode);
	PTRACE2(eLevelInfoNormal,"CCommConf::SetLectureMode ",m_pLectureMode->GetLecturerName());
	m_pSubject->Notify(UPDATELECTUREMODE,0);
}

////////////////////////////////////////////////////////////////////////////
BYTE  CCommConf::AreAnyITPPartiesConnected()
{
	BYTE  areAnyITPPartiesConnected = 0;
	  for (int i=0;i<(int)m_numParties;i++){
	    if (m_pParty[i]!=NULL) {
	      if(m_pParty[i]->GetTelePresenceMode() != eTelePresencePartyNone)
	      {
	    	 ON(areAnyITPPartiesConnected);
	    	 return areAnyITPPartiesConnected;
	      }
	    }
	  }
	  return areAnyITPPartiesConnected;
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::SetSipTotRegistrationsStatus(const BYTE status)
{
	if(m_SipRegTotalSts != status){
        IncreaseSummaryUpdateCounter();
		FASTCHANGE;
	}
	m_SipRegTotalSts = status;
}

////////////////////////////////////////////////////////////////////////////
BYTE  CCommConf::GetSipTotRegistrationsStatus()
{
	return m_SipRegTotalSts;
}

////////////////////////////////////////////////////////////////////////////
void CCommConf::SetMsConversationId(char* msConvId)
{
	strncpy(m_msConversationId, msConvId, sizeof(m_msConversationId) - 1);
	m_msConversationId[sizeof(m_msConversationId) - 1] = '\0';
}

////////////////////////////////////////////////////////////////////////////
const char* CCommConf::GetMsConversationId() const
{
	return m_msConversationId;
}

STATUS CCommConf::SLOW_FAST_CHANGE_Terminal( const char* partyName, int slowFastAction )
{
	  CConfParty* pParty = GetCurrentParty( partyName );
	  if (!pParty)
		  return STATUS_FAIL;
	  pParty->SLOW_FAST_CHANGE_Terminal( slowFastAction );
	  return STATUS_OK;
}


// VNGR-22639: For later restore purpose
void CCommConf::SaveLecturerVideoLayout(CLectureModeParams* pSavedLectureMode,
        CVideoLayout* pSavedLecturerVideoLayout)
{
    m_pSavedLectureMode = pSavedLectureMode;
    m_pSavedLecturerVideoLayout = pSavedLecturerVideoLayout;
}

void CCommConf::GetLecturerVideoLayout(CLectureModeParams* &pSavedLectureMode,
        CVideoLayout* &pSavedLecturerVideoLayout)
{
    pSavedLectureMode = m_pSavedLectureMode;
    pSavedLecturerVideoLayout = m_pSavedLecturerVideoLayout;
}

void CCommConf::MccfIvrStartDialog(DialogState& state)
{
	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogStart* dialog = (DialogStart*) mscIvr.m_pResponseType;

	MccfIvrErrorCodesEnum status = mccf_ivr_OK;
	status = CMccfIvrPackageResponse::ResponseReportMsg(state, status); //TODO - should be here?

	if (status == mccf_ivr_OK)
	{

		//TODO start dialog

		//Create the message to return //TODO should be here?

		unsigned int dialogExitStatus = 1; //TODO ENUM
		Event* event = CMccfIvrPackageResponse::BuildControlMsg(&mscIvr, state.dialogID, dialogExitStatus);

		//TODO - adding params to the message that conect only to this spacific message
		event->m_dialogExit.m_promptInfo.m_termMode = "completed";

		CMccfIvrPackageResponse::ResponseControlMsg(state);
	}
}

BYTE CCommConf::IsConfHasDifferentChairPerson(const DWORD ChairPersonId) const
{
	int size = GetNumParties();

	for (int i=0; i < size && i < MAX_PARTIES_IN_CONF; ++i)
	{
		CConfParty* pConfParty = (CConfParty*)m_pParty[i];
		if (ChairPersonId != pConfParty->GetPartyId() && eOn == pConfParty->GetIsLeader())
		{
			return TRUE;
		}
	}
	return FALSE;
}
void CCommConf::UpdateActiveSpeakersList(DWORD *unActiveSpeakerMonitorIdList)
{
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
		m_activeSpeakerList[i] = unActiveSpeakerMonitorIdList[i];
	IncreaseSummaryUpdateCounter();
	//FASTCHANGE;
	DumpActiveSpeakersList();	// currently, will be removed after the integration(AmirK)
}
void CCommConf::UpdateActiveSpeakersListUponPartyLeftTheConf(DWORD id)
{
	eProductType ePT = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeSoftMCUMfw != ePT )
		return;
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
		if (id == m_activeSpeakerList[i])
		{
			m_activeSpeakerList[i] = 0xFFFFFFFF;
			IncreaseSummaryUpdateCounter();
			//FASTCHANGE;
			DumpActiveSpeakersList();
			return;
		}
}
void CCommConf::DumpActiveSpeakersList()
{
	CMedString str = "CCommConf::DumpActiveSpeakersList - ";
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
		str << (DWORD)m_activeSpeakerList[i] << "  ";
	PTRACE(eLevelInfoNormal, str.GetString());
}
void CCommConf::AddConfActiveSpeakersListToXmlResponse(CXMLDOMElement *pResNode)
{
	eProductType ePT = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeSoftMCUMfw != ePT )
		return;
	CXMLDOMElement *pActiveSpeakersNode;
	pActiveSpeakersNode = pResNode->AddChildNode("ACTIVE_SPEAKER_LIST");
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
		if (0xFFFFFFFF != m_activeSpeakerList[i])
			pActiveSpeakersNode->AddChildNode("SPEAKER_ID",m_activeSpeakerList[i]);
}

DWORD CCommConf::NextCGPartiesCounter()
{
	m_CGPartiesCounter++;
	return (m_CGPartiesCounter - 1);
}

////////////////////////////////////////////////////////////////////////////
void  CCommConf::ResetAdHocProfileId()
{
	// Bridge-10335
	SetAdHocProfileId(0xFFFFFFFF);
	SLOWCHANGE;
}

////////////////////////////////////////////////////////////////////////////
BYTE CCommConf::IncludeRDPGw()
{
	CRsrvParty * pCurrParty =0;
    for (int i=0;i<(int)m_numParties;i++)
    {
    	pCurrParty = m_pParty[i];
    	if(pCurrParty && pCurrParty->GetRsrvPartyIsRdpGw())
    	{
    		return YES;
    	}
    }
    return NO;
}
//////////////////////////////////////////////////////////////////////////////
CConfParty*  CCommConf::GetRDPGwParty()
{
	CRsrvParty * pCurrParty =0;
    for (int i=0;i<(int)m_numParties;i++)
    {
    	pCurrParty = m_pParty[i];
    	if(pCurrParty && pCurrParty->GetRsrvPartyIsRdpGw())
    	{
    		return (CConfParty*)pCurrParty;
    	}
    }
    return NULL;
}




