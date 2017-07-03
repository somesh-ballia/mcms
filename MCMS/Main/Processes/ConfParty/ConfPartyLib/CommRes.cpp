#include "NStream.h"
#include "CommRes.h"
#include "psosxml.h"
#include "CommConfDB.h"
#include "LectureModeParams.h"
#include "VisualEffectsParams.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "H263.h"
#include "NetCallingParty.h"
#include "CommResDB.h"
#include "IpService.h"
#include "ApiStatuses.h"
#include "IpServiceListManager.h"
#include "IPUtils.h"
#include "ConfPartyStatuses.h"
#include "SysConfig.h"
#include "CommResShort.h"
#include "IVRServiceList.h"
#include "SysConfigKeys.h"
#include "ConfPartyGlobals.h"
#include "ServicePrefixStr.h"
#include "TraceStream.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "ConfPartyProcess.h"
#include "RecordingLinkDB.h"
#include "OperatorConfInfo.h"
#include "MoveInfo.h"
#include "IpCommon.h"
#include "SipUtils.h"

extern CCommResDB*            GetpProfilesDB();
extern CIpServiceListManager* GetIpServiceListMngr();
extern CAVmsgServiceList*     GetpAVmsgServList();
extern CRecordingLinkDB*      GetRecordingLinkDB();

////////////////////////////////////////////////////////////////////////////
//                           CCommRes
////////////////////////////////////////////////////////////////////////////
CCommRes::CCommRes() : CCommResApi()
{
}

//--------------------------------------------------------------------------
CCommRes::CCommRes(const CCommRes& other) : CCommResApi(*(const CCommResApi*)&other)
{
}

//--------------------------------------------------------------------------
CCommRes::CCommRes(const CCommResApi& other) : CCommResApi(*(const CCommResApi*)&other)
{
}

//--------------------------------------------------------------------------
CCommRes& CCommRes::operator =(const CCommRes& other)
{
	if (this != &other)
		CCommResApi::operator =(other);

	return *this;
}

//--------------------------------------------------------------------------
CCommRes::~CCommRes()
{ }

//--------------------------------------------------------------------------
void CCommRes::SetIsAutoLayout(const BYTE newIsAutoLayout)
{
	m_isAutoLayout = (newIsAutoLayout) ? YES : NO;
	UPDATECONFLAGSNEGATIV(m_isAutoLayout, NO, AUTO_LAYOUT_FLAG);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetNetwork(const BYTE network)
{
	m_network = network;
	UPDATECONFLAGSPOSITIV(m_network, NETWORK_H323, H323_ONLY);
}

//--------------------------------------------------------------------------
void CCommRes::SetDualVideoMode(const WORD byDualVideoMode)
{
	if (byDualVideoMode > eDualModeUnknown)
		m_dualVideoMode = eDualModeUnknown;
	else
		m_dualVideoMode = byDualVideoMode;
}

//--------------------------------------------------------------------------
void CCommRes::SetIsVideoClarityEnabled(const BYTE bIsVideoClarityEnabled)
{
	m_isVideoClarityEnabled = (bIsVideoClarityEnabled) ? YES : NO;
	UPDATECONFLAGSNEGATIV(m_isVideoClarityEnabled, NO, VIDEO_CLARITY_FLAG);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
int CCommRes::Update(const CRsrvParty& other)
{
	int ind = FindName(other);
	if (ind == NOT_FIND || ind >= MAX_PARTIES_IN_CONF)
		return STATUS_PARTY_DOES_NOT_EXIST;

	POBJDELETE(m_pParty[ind]);
	m_pParty[ind] = new CRsrvParty(other);
	m_pParty[ind]->SetRes(this);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
// used for updating party template only.
int CCommRes::UpdateById(const CRsrvParty& other)
{
	int ind = FindId(other.GetPartyId());
	if (ind == NOT_FIND || ind >= MAX_PARTIES_IN_CONF)
		return STATUS_PARTY_DOES_NOT_EXIST;

	delete m_pParty[ind];
	m_pParty[ind] = new CRsrvParty(other);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommRes::Cancel(const char* name)
{
	int ind = FindName(name);
	if (ind == NOT_FIND || ind >= MAX_PARTIES_IN_CONF)
		return STATUS_PARTY_DOES_NOT_EXIST;

	// Remove party from all layouts
	for (WORD k = 0; k < m_numRsrvVideoSource; k++)
		m_pRsrvVideoLayout[k]->DeletePartyFromCells(name);

	int partyId = m_pParty[ind]->GetPartyId();
	if(CPObject::IsValidPObjectPtr(m_pAutoScanOrder))
		m_pAutoScanOrder->RemoveItem(partyId);

	POBJDELETE(m_pParty[ind]);
	int i;
	for (i = 0; i < (int)m_numParties; i++)
	{
		if (m_pParty[i] == NULL)
			break;
	}

	for (int j = i; j < (int)m_numParties-1; j++)
		m_pParty[j] = m_pParty[j+1];

	m_pParty[m_numParties-1] = NULL;
	m_numParties--;
	TRACEINTO << "PartyName:" << name << ", NumParties:" << m_numParties;
	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommRes::Cancel(const DWORD PartyId)
{
	int ind = FindId(PartyId);
	if (ind == NOT_FIND || ind >= MAX_PARTIES_IN_CONF)
		return STATUS_PARTY_DOES_NOT_EXIST;

	// Remove party from all layouts
	const char* szPartyName = m_pParty[ind]->GetName();

	for (WORD k = 0; k < m_numRsrvVideoSource; k++)
		m_pRsrvVideoLayout[k]->DeletePartyFromCells(szPartyName);

	if(CPObject::IsValidPObjectPtr(m_pAutoScanOrder))
		m_pAutoScanOrder->RemoveItem(PartyId);

	POBJDELETE(m_pParty[ind]);

	int i;
	for (i = 0; i < (int)m_numParties; i++)
	{
		if (m_pParty[i] == NULL)
			break;
	}

	for (int j = i; j < (int)m_numParties-1; j++)
		m_pParty[j] = m_pParty[j+1];

	m_pParty[m_numParties-1] = NULL;
	m_numParties--;
	TRACEINTO << "PartyId:" << PartyId << ", NumParties:" << m_numParties;
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCommRes::SetAutoRedial(const BYTE autoRedial)
{
	m_AutoRedial = autoRedial;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetPresentationProtocol(BYTE presentationProtocol)
{
	m_PresentationProtocol =  presentationProtocol;
}

//--------------------------------------------------------------------------
int CCommRes::FindId(const DWORD partyId)
{
	for (int i = 0; i < (int)m_numParties; i++)
	{
		if (m_pParty[i] != NULL)
		{
			if (m_pParty[i]->GetPartyId() == partyId)
				return i;
		}
	}
	return NOT_FIND;
}

//--------------------------------------------------------------------------
CRsrvParty* CCommRes::GetFirstParty()
{
	m_ind = 1;
	return m_pParty[0];
}

//--------------------------------------------------------------------------
CRsrvParty* CCommRes::GetNextParty()
{
	if (m_ind >= m_numParties) return NULL;

	return m_pParty[m_ind++];
}

//--------------------------------------------------------------------------
CRsrvParty* CCommRes::GetFirstParty(int& nPos)
{
	CRsrvParty* pParty = GetFirstParty();
	nPos = m_ind;
	return pParty;
}

//--------------------------------------------------------------------------
CRsrvParty* CCommRes::GetNextParty(int& nPos)
{
	m_ind = nPos;
	CRsrvParty* pParty = GetNextParty();
	nPos = m_ind;
	return pParty;
}

//--------------------------------------------------------------------------
CRsrvParty* CCommRes::GetCurrentParty(const char* name) const
{
	for (int i = 0; i < (int)m_numParties; i++)
	{
		if (m_pParty[i] != NULL)
		{
			if (!strncmp(m_pParty[i]->GetName(), name, H243_NAME_LEN))
				return m_pParty[i];
		}
	}
	return NULL; // STATUS_PARTY_DOES_NOT_EXIST
}

//--------------------------------------------------------------------------
CRsrvParty* CCommRes::GetCurrentParty(const DWORD partyId) const
{
	for (int i = 0; i < (int)m_numParties; i++)
	{
		if (m_pParty[i] != NULL)
		{
			if (m_pParty[i]->GetPartyId() == partyId)
				return m_pParty[i];
		}
	}
	return NULL; // STATUS_PARTY_DOES_NOT_EXIST
}

//--------------------------------------------------------------------------
DWORD CCommRes::GetLastPartyId() const
{
	DWORD lastPartyId = 0xFFFFFFFF;
	for (int i = 0; i < (int)m_numParties; i++)
	{
		if (m_pParty[i] != NULL && m_pParty[i]->GetPartyId() <= HALF_MAX_DWORD)
			if (lastPartyId < m_pParty[i]->GetPartyId() || lastPartyId == 0xFFFFFFFF)
				lastPartyId =  m_pParty[i]->GetPartyId();
	}
	return lastPartyId;
}

//--------------------------------------------------------------------------
CStructTm CCommRes::CalculateConfEndTime(const CStructTm& startTime, const CStructTm& duration)
{
	CStructTm EndTime = startTime + duration;
	return EndTime;
}

//--------------------------------------------------------------------------
CStructTm CCommRes::CalculateConfDuration(CStructTm& startTime, CStructTm& endTime)
{
	tm tmStart, tmEnd;
	CStructTm duration;

	startTime.GetAsTm(tmStart);
	endTime.GetAsTm(tmEnd);

	tmStart.tm_mon -= 1;
	tmEnd.tm_mon   -= 1;

	time_t timeStart_t = mktime(&tmStart);
	time_t timeEnd_t   = mktime(&tmEnd);

	DWORD  dwDiff = timeEnd_t - timeStart_t;

	duration.m_hour = dwDiff / 3600;
	duration.m_min  = (dwDiff % 3600) / 60;

	return duration;
}

//--------------------------------------------------------------------------
void CCommRes::SetName(const char* name)
{
	m_H243confName[0] = '\0';
	strcpy_safe(m_H243confName, name);
}

//--------------------------------------------------------------------------
void CCommRes::SetDisplayName(const char* name)
{
	m_confDisplayName[0] = '\0';
	strcpy_safe(m_confDisplayName, name);
}

//--------------------------------------------------------------------------
void CCommRes::SetDurationTime(const CStructTm& other)
{
	m_duration = other;
	IncreaseSummaryUpdateCounter();
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetEntryTone(WORD bl)
{
	if (bl == FALSE)
		m_audioTone &= 0xFFFFFFFE;
	else
		m_audioTone |= 0x00000001;
}

//--------------------------------------------------------------------------
void CCommRes::SetExitTone(WORD bl)
{
	if (bl == FALSE)
		m_audioTone &= 0xFFFFFFFD;
	else
		m_audioTone |= 0x00000002;
}

//--------------------------------------------------------------------------
void CCommRes::SetEndTimeAlertTone(WORD bl)
{
	if (bl == FALSE)
		m_audioTone &= 0xFFFFFFFB;
	else
		m_audioTone |= 0x00000004;
}

//--------------------------------------------------------------------------
void CCommRes::SetH243Password(const char* password)
{
	m_H243_password[0] = '\0';
	strcpy_safe(m_H243_password, password);
	IncreaseSummaryUpdateCounter();
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetCascadeMode(const BYTE cascadeMode)
{
	m_cascadeMode = cascadeMode;
	UPDATECONFLAGSNEGATIV(m_cascadeMode, CASCADE_MODE_NONE, CASCADING);
	UPDATECONFLAGSPOSITIV(m_cascadeMode, CASCADE_MODE_AUTO, CASCADING_AUTO);
}

//--------------------------------------------------------------------------
void CCommRes::SetLectureMode(const CLectureModeParams& otherLectureMode)
{
	*m_pLectureMode = otherLectureMode;
	UPDATECONFLAGSNEGATIV(m_pLectureMode->GetLectureModeType(), NO, LECTURE_MODE);
	UPDATECONFLAGSPOSITIV(m_pLectureMode->GetLectureModeType(), LECTURE_SHOW, LECTURE_MODE_SHOW);
	UPDATECONFLAGSPOSITIV(m_pLectureMode->GetLectureModeType(), PRESENTATION_LECTURE_MODE, LECTURE_MODE_PRESENTATION);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetConfOnHoldFlag(const BYTE confOnHoldFlag)
{
	m_confOnHoldFlag = confOnHoldFlag;
	UPDATECONFLAGSNEGATIV(m_confOnHoldFlag, NO, ON_HOLD);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetConfLockFlag(const BYTE confLockFlag)
{
	if (m_confLockFlag != confLockFlag)
		IncreaseSummaryUpdateCounter();

	m_confLockFlag = confLockFlag;
	UPDATECONFLAGSNEGATIV(m_confLockFlag, NO, LOCKED);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetConfSecured(BYTE ConfSecureFlag)
{
	if (m_confSecureFlag != ConfSecureFlag)
		IncreaseSummaryUpdateCounter();

	if (ConfSecureFlag)
	{
		// mark all parties as new parties in order to refresh operator.
		CRsrvParty* pParty = GetFirstParty();
		while (pParty != NULL)
		{
			pParty->SetAddPartyMask(1);
			pParty = GetNextParty();
		}
	}

	m_confSecureFlag = ConfSecureFlag;
	UPDATECONFLAGSNEGATIV(m_confSecureFlag, NO, SECURED);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetIsInviteParty(const BYTE InviteParty)
{
	m_InviteParty = InviteParty;
	UPDATECONFLAGSNEGATIV(m_InviteParty, NO, INVITE_PARTICPANT);
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
DWORD CCommRes::NextPartyId()
{
	DWORD lastId = m_partyIdCounter;

	if (m_partyIdCounter < HALF_MAX_DWORD)
		m_partyIdCounter++;
	else
		m_partyIdCounter = 0;

	return lastId;
}

//--------------------------------------------------------------------------
DWORD CCommRes::NextUnrsrvPartiesCounter()
{
	m_unrsrvPartiesCounter++;
	return (m_unrsrvPartiesCounter - 1);
}

//--------------------------------------------------------------------------
void CCommRes::SetAudioConf(BYTE audioConf)
{
	PASSERTMSG_AND_RETURN(audioConf == FALSE, "Failed, set value is invalid");
	m_media = AUDIO_MEDIA;
	UPDATECONFLAGSNEGATIV(YES, NO, AUDIO_ONLY);
}

//--------------------------------------------------------------------------
void CCommRes::SetMeetMePerConf(const BYTE meetMePerConf)
{
  m_meetMePerConf = meetMePerConf;
  UPDATECONFLAGSNEGATIV(m_meetMePerConf, NO, ACTIVE_PERMANEMT)
}

//--------------------------------------------------------------------------
void CCommRes::TranslateRGBColorToYUV()
{
	DWORD dwBackgroundRGBcolor      = m_pVisualEffectsInfo->GetBackgroundColorRGB();
	DWORD dwLayoutRGBcolor          = m_pVisualEffectsInfo->GetlayoutBorderColorRGB();
	DWORD dwSpeakerNotationRGBcolor = m_pVisualEffectsInfo->GetSpeakerNotationColorRGB();

	DWORD dwBackgroundYUVcolor      = 0;
	DWORD dwLayoutYUVcolor          = 0;
	DWORD dwSpeakerNotationYUVcolor = 0;

	m_pVisualEffectsInfo->TranslateRGBColorToYUV(dwBackgroundRGBcolor, dwBackgroundYUVcolor);
	m_pVisualEffectsInfo->TranslateRGBColorToYUV(dwLayoutRGBcolor, dwLayoutYUVcolor);
	m_pVisualEffectsInfo->TranslateRGBColorToYUV(dwSpeakerNotationRGBcolor, dwSpeakerNotationYUVcolor);

	m_pVisualEffectsInfo->SetBackgroundColorYUV(dwBackgroundYUVcolor);
	m_pVisualEffectsInfo->SetlayoutBorderColorYUV(dwLayoutYUVcolor);
	m_pVisualEffectsInfo->SetSpeakerNotationColorYUV(dwSpeakerNotationYUVcolor);
}

//--------------------------------------------------------------------------
void CCommRes::UpdateVisualEffectsInfo(CVisualEffectsParams* pVisualEffects)
{
	*m_pVisualEffectsInfo = *pVisualEffects;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetEntryPassword(const char* entry_password)
{
	m_entry_password[0] = '\0';
	strcpy_safe(m_entry_password, entry_password);
	IncreaseSummaryUpdateCounter();
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
WORD CCommRes::GetNumOfVideoParties()
{
	WORD numOfVideoParties = 0;
	CRsrvParty* pParty = GetFirstParty();
	while (pParty != NULL)
	{
		if (!pParty->GetVoice())
			numOfVideoParties++;

		pParty = GetNextParty();
	}
	return numOfVideoParties;
}

//--------------------------------------------------------------------------
WORD CCommRes::GetNumOfAudioOnlyParties()
{
	WORD numOfAudioOnlyParties = 0;
	CRsrvParty* pParty = GetFirstParty();
	while (pParty != NULL)
	{
		if (pParty->GetVoice())
			numOfAudioOnlyParties++;

		pParty = GetNextParty();
	}
	return numOfAudioOnlyParties;
}

//--------------------------------------------------------------------------
void CCommRes::SetConfContactInfo(const char* ContactInfo, int ContactNumber)
{
	m_pConfContactInfo->SetContactInfo(ContactInfo, ContactNumber);
	IncreaseSummaryUpdateCounter();
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetBillingInfo(const char* BillingInfo)
{
	if (BillingInfo)
		m_BillingInfo = BillingInfo;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetConfRemarks(const char* remarks)
{
	m_confRemarks[0] = '\0';
	strcpy_safe(m_confRemarks, remarks);
}

//--------------------------------------------------------------------------
void CCommRes::IncreaseSummaryUpdateCounter()
{
	CCommConfDB* pConfDB = ::GetpConfDB();
	if (pConfDB)
	{
		pConfDB->IncreaseSummaryUpdateCounter();
		m_dwSummaryUpdateCounter = pConfDB->GetSummaryUpdateCounter();
	}
}

//--------------------------------------------------------------------------
void CCommRes::SetDefaultParams()
{
	TRACEINTO << "ConfName:" << GetName();

	// Parameters that are not in CARMEL (or hidden in background for EMA)
	// ======================================
	m_confRemarks[0]             = '\0';
	//m_media                      = AUDIO_VIDEO_MEDIA;
	m_videoProtocol              = AUTO;
	m_videoPictureFormat         = AUTO;
	m_CIFframeRate               = AUTO;
	m_audioRate                  = AUTO;
	m_confType                   = CONF_TYPE_STANDARD;
	m_numUndefParties            = 0;
  m_audioMixDepth              = RMX_AUDIO_MIX_DEPTH_DEFAULT; // of RMX -will be changed if need by SoftMCU below SetDefaultParamsAccordingToProductType
	m_ipVideoRate                = 384;     // H323_BIT_RATE - NEED TO ASK URI A
	m_meetingRoomReoccurrenceNum = 0xFF;    // MEETING_ROOM/ LIMITED_SEQ - off

	m_pAvMsgStruct->SetAttendedWelcome(CONF_IVR);

	// Just in case of reservation or conference we don't need to init the start time
	if (m_isMeetingRoom || m_isConfTemplate || m_isTemplate)
	{
		m_startTime.m_sec  = 0;
		m_startTime.m_min  = 0;
		m_startTime.m_hour = 0;
		m_startTime.m_day  = 0;
		m_startTime.m_mon  = 0;
		m_startTime.m_year = 0;
	}

	m_duration.m_sec  = 0;
	m_confLockFlag    = NO;
	m_LSDRate         = LSD_6400;
	m_meetMePerEntryQ = YES;
	m_confOnHoldFlag  = NO;
	m_InviteParty     = NO;
	m_isVideoPlusConf = NO;
	m_dualVideoMode   = eDualModeNone;
	m_confControl     = H243;
	m_confProtocol    = H243;
	m_video           = YES;
	m_QCIFframeRate   = AUTO;

	if (m_videoQuality == eVideoQualityAuto)
		m_videoQuality = eVideoQualitySharpness;

	SetRollCall(FALSE);

	// Make sure all layout force state are not in blank
	for (int i = 0; i < (int)m_numRsrvVideoSource; ++i)
	{
		m_pRsrvVideoLayout[i]->SetCellStatusToAllArr(AUDIO_ACTIVATED,isOverrideProfileLayout());
	}

	// Set party's params with default values
	CRsrvParty* pParty = GetFirstParty();
	while (pParty != NULL)
	{
		pParty->SetPartyDefaultParams();
		pParty = GetNextParty();
	}
	SetDefaultParamsAndAutoCorrectForConfMediaType();
	if (m_HD)
	{
		SetHDVSWDefaultSettings();
		TRACEINTO << "m_HD:" << (int)m_HD;
	}

	SetDefaultParamsAccordingToSystemCardsBasedMode();

	if (m_videoSession == VIDEO_SESSION_COP)
	{
		SetCopDefaultSettings();
		SetSetLectureModeSameLayoutRules(m_pLectureMode);
		SetConfMediaTypeForCOP();	// CMT@
	}

	if (m_pLectureMode->GetLectureModeType() == 1 && m_pLectureMode->GetTimerOnOff() == 0)
	{
		m_pLectureMode->SetTimerOnOff(YES);
	}
	TRACEINTO << "LectureModeType:" << (WORD)m_pLectureMode->GetLectureModeType();
	SetDefaultParamsAccordingToProductType();
}

//--------------------------------------------------------------------------
void CCommRes::SetConfMediaTypeForCOP()
{
	if (m_confMediaType != eAvcOnly)
	{
		m_confMediaType = eAvcOnly;
		TRACEINTO << "ConfMediaType:" << m_confMediaType << " - Invalid media type, changed to AVC-Only";
	}
}

//--------------------------------------------------------------------------
void CCommRes::SetAdHocConfBasicParams(char* confName, const char* numericID, DWORD dwAdHocProfileId)
{
	TRACEINTO << "ConfName:" << confName << ", ConfId:" << numericID << ", AdHocProfileId:" << dwAdHocProfileId;

	SetName(confName);
	SetDisplayName("");
	SetH243Password("");

	// set Numeric ID of target conference as provided by user
	SetNumericConfId(numericID);

	// Some more settings for target conference
	SetMonitorConfId(0xFFFFFFFF);
	SetEntryQ(NO);
	SetMeetingRoom(NO);
	SetMeetMePerEntryQ(YES);
	SetIsAdHocConf(YES);
	SetAdHocProfileId(dwAdHocProfileId);
	SetTemplate(NO);
}

//--------------------------------------------------------------------------
void CCommRes::SetAdHocConfExtDbBasedParams(char* display_name, char* maxParties, char* minParties, char* password,
                                            char* entryPwd, char* billingData, char* owner,
                                            char* info1, char* info2, char* info3)
{
  // Set conference display name
	if (!display_name)
		SetDisplayName("");
	else
		SetDisplayName(display_name);

	// Set Max & Min parties fields
	if (maxParties)
	{
		if (!strcmp(maxParties, "auto"))
			SetMaxParties(0xFFFF);
		else
			SetMaxParties(atoi(maxParties));
	}

	if (minParties)
		SetNumUndefParties(atoi(minParties));

	// Some more settings for target conference
	if (password)
		SetH243Password(password);

	if (entryPwd)
		SetEntryPassword(entryPwd);

	if (billingData)
		SetBillingInfo(billingData);

	if (owner)
		SetConfRemarks(owner);      // /in Carmel??

	if (info1)
		SetConfContactInfo(info1, 0);

	if (info2)
		SetConfContactInfo(info2, 1);

	if (info3)
		SetConfContactInfo(info3, 2);
}

//--------------------------------------------------------------------------
void CCommRes::SetHDVSWDefaultSettings()
{
	SetHDVSWDefaultlayout();
	m_pLectureMode->SetLectureModeType(0);

	m_isAutoLayout          = NO;
	m_isSameLayout          = NO;
	m_isVideoClarityEnabled = NO;
	m_ShowContentAsVideo    = NO;
	m_IsConfOnPort          = NO;
	m_bGatheringEnabled     = FALSE;  // VNGR-23736: disable Gathering for VSW conference
}

//--------------------------------------------------------------------------
void CCommRes::SetHDVSWDefaultlayout()
{
	if (m_contPresScreenNumber != ONE_ONE)
	{
		m_numRsrvVideoSource   = 0;
		m_contPresScreenNumber = ONE_ONE;

		memset(m_pRsrvVideoLayout, 0, sizeof(m_pRsrvVideoLayout));

		AddNewVideoLayout(ONE_ONE, YES);
	}
}

//--------------------------------------------------------------------------
void CCommRes::SetSysCfgParams()
{
	TRACEINTO << "ConfName:" << GetName();

	// Parameters that are from System.cfg
	m_alertToneTiming = 5;

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	eSystemCardsMode     systemCardMode      = GetSystemCardsBasedMode();

	DWORD time_interval = 15;
	sysConfig->GetDWORDDataByKey("LECTURE_MODE_TIMER_INTERVAL", time_interval);
	m_pLectureMode->SetLectureTimeInterval(time_interval);

	DWORD tmpData = 0;
	sysConfig->GetDWORDDataByKey("CONF_END_ALERT_TONE", tmpData);
	m_alertToneTiming = (BYTE)tmpData;

	BOOL is_internal = 0;
	sysConfig->GetBOOLDataByKey(CFG_KEY_INTERNAL_SCHEDULER, is_internal);
	if (!is_internal)
	{
		m_minNumVideoParties = 0;
		m_minNumAudioParties = 0;
	}

	// it can happen that user decreases MAX_CP_RESOLUTION in configuration dialog, then we need
	// to update conference max resolution field within the system max resolution
	EVideoResolutionType maxSystemResolution = CResRsrcCalculator::GetMaxCPResolutionType(systemCardMode);
	BYTE                 maxConfResolution   = GetConfMaxResolution();
	if (maxSystemResolution < maxConfResolution && maxConfResolution != eAuto_Res)
	{
		SetConfMaxResolution(maxSystemResolution);
		TRACEINTO << "Warning, The conference resolution higher than the system resolution, set the system resolution";
	}

	BOOL enableSelectiveMixing = FALSE;
	sysConfig->GetBOOLDataByKey( CFG_KEY_ENABLE_SELECTIVE_MIXING, enableSelectiveMixing);
	if(FALSE == enableSelectiveMixing)
	{
		m_isAutoMuteNoisyParties = NO;
	}
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestPartyRsrvValidity(CRsrvParty* pRsrvParty)
{
	TRACEINTO << "PartyName:" << pRsrvParty->GetName();

	pRsrvParty->SetPartyDefaultParams();

	BYTE errorCode = 0;
	STATUS status = pRsrvParty->CheckReservRangeValidity(errorCode);
	PASSERTSTREAM_AND_RETURN_VALUE(status != STATUS_OK, "PartyName:" << pRsrvParty->GetName() << ", Status:"    << CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), status);

	return TestPartyValidity(pRsrvParty);
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestValidity()
{
	TRACEINTO << "ConfName:" << GetName();

	SetDefaultParams();
	SetSysCfgParams();

	BYTE errorCode = 0;
	STATUS status  = CheckReservRangeValidity(errorCode);
	DBGPASSERT(errorCode);
	PASSERTSTREAM_AND_RETURN_VALUE(status != STATUS_OK, "ConfName:" << GetName() << ", Status:"   << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << ", errorCode:" << (int)errorCode, status);

	status = TestReservValidity();

	AutoCorrectTelePresenceParams();
	AutoCorrectShowContentAsVideo();
	AutoCorrectPermanentConf();
	AutoCorrectEncryptionParams();
	AutoCorrectManageLayoutInternally();
	AutoCorrectIndicationOnLayout();
	AutoCorrectConfMediaTypeForEQ();
	AutoCorrectConfRate();
	return status;
}

//--------------------------------------------------------------------------
// Romem - 21.7.08 According to Assaf W. Decision - MR/EQ  and Regular Conferences are treated the same.
// VNGR-8774 for MR/EQ the Service Name and Service Phone Number should be checked separately
STATUS CCommRes::TestMRValidity()
{
	TRACEINTO << "ConfName:" << GetName();

	SetMRDefaultParams();

	// Test Phones validity in case of PSTN
	if (IsEnableIsdnPstnAccess())      // NETWORK_H320_H323 == m_network)
	{
		for (int j = 0; j < m_numServicePhoneStr; j++)
		{
			Phone* phone = m_pServicePhoneStr[0]->m_pPhoneNumberList[j];
			if (phone != NULL)
			{
				if (::IsValidASCII(phone->phone_number, strlen(phone->phone_number), "ASCII_NUMERIC", NO) == NO)
					return STATUS_PHONE_MUST_BE_NUMERIC;
			}
		}
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestAndUpdateGWProfileValidity()
{
	std::ostringstream msg;
	msg << "CCommRes::TestAndUpdateGWProfileValidity - ConfName:" << GetName();

	STATUS internalStatus   = STATUS_OK;
	WORD   blockedProtocols = 0;

	CConfIpParameters* pConfIpParameters = NULL;
	CONF_IP_PARAMS_S*  serviceParams;
	const char* conf_serv_name = GetServiceNameForMinParties();

	msg << "\nconf service name :" << conf_serv_name;

	// VNGR-18702 take 2 - with [default service] MCMS received empty service name, and stay with first in list -
	// comment the if statement and GetRelevantService returns the default
	pConfIpParameters = ::GetIpServiceListMngr()->GetRelevantService(conf_serv_name, H323_INTERFACE_TYPE);
	if (pConfIpParameters == NULL)
	{
		DBGPASSERT(102);
		pConfIpParameters = ::GetIpServiceListMngr()->GetFirstServiceInList();
	}

	if (m_videoSession == VIDEO_SESSION_COP)
	{
		if (GetIsGWDialOutToH320() || GetIsGWDialOutToPSTN())
		{
			if (!GetIsGWDialOutToH323() && !GetIsGWDialOutToSIP())
			{
				TRACEINTO << "Currently ISDN not supported in COP conference";
				return STATUS_ISDN_CONNECTIONS_NOT_SUPPORTED;
			}
			else
			{
				TRACEINTO << "Currently ISDN not supported in COP conference, we will disable to call to H320 and PSTN";
				SetIsGWDialOutToPSTN(NO);
				SetIsGWDialOutToH320(NO);
			}
		}
	}

	if (GetIsGWDialOutToH323())
	{
		if (pConfIpParameters)
		{
			serviceParams = pConfIpParameters->GetConfIpParamsStruct();
			if (serviceParams->service_protocol_type == eIPProtocolType_SIP || serviceParams->service_protocol_type == (WORD)eIPProtocolType_None)
			{
				msg << "\nH323 dial out is not allowed, IP service does not support H323";
				blockedProtocols |= GW_H323_OUT;
			}
		}
		else
		{
			msg << "\nH323 dial out is not allowed, no IP service";
			blockedProtocols |= GW_H323_OUT;
		}
	}

	if (GetIsGWDialOutToSIP())
	{
		if (pConfIpParameters)
		{
			serviceParams = pConfIpParameters->GetConfIpParamsStruct();
			if (serviceParams->service_protocol_type == eIPProtocolType_H323 || serviceParams->service_protocol_type == (WORD)eIPProtocolType_None)
			{
				msg << "\nSIP dial out is not allowed, IP service does not support SIP";
				blockedProtocols |= GW_SIP_OUT;
			}
		}
		else
		{
			msg << "\nSIP dial out is not allowed, no IP service";
			blockedProtocols |= GW_SIP_OUT;
		}
	}

	CConfPartyProcess*      pConfPartyProcess  = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	RTM_ISDN_PARAMS_MCMS_S* pIsdnServiceStruct = pConfPartyProcess->GetIsdnService(""); // take the Default service

	if (!pIsdnServiceStruct) // no ISDN default service
	{
		if (GetIsGWDialOutToH320())
		{
			msg << "\nH320 dial out is not allowed, no ISDN service";
			blockedProtocols |= GW_H320_OUT;
		}

		if (GetIsGWDialOutToPSTN())
		{
			msg << "\nPSTN dial out is not allowed, no ISDN service";
			blockedProtocols |= GW_PSTN_OUT;
		}
	}

	msg << "\nblocked protocols :" << blockedProtocols;

	if (blockedProtocols)
	{
		internalStatus = (STATUS_ALL_DIAL_OUT_PROTOCOS_OK | blockedProtocols); // STATUS_FAIL;
		SetInternalConfStatus(internalStatus);
	}

	msg << "\ninternal status   :" << GetInternalConfStatus();

	STATUS ret_status = STATUS_OK;

	if (IsEnableIsdnPstnAccess())  // Daimler
	{
		CServicePhoneStr* pServicePhone = GetFirstServicePhone();
		if (pServicePhone && pServicePhone->IsUseServicePhonesAsRange())
		{
			Phone* phone1 = pServicePhone->GetFirstPhoneNumber();
			Phone* phone2 = pServicePhone->GetNextPhoneNumber();
			if (!phone1 || !strlen(phone1->phone_number) || !phone2 || !strlen(phone2->phone_number))
			{
				msg << "\nfirst or last dial-in number is empty";
				ret_status = STATUS_GATEWAY_MISSED_DIALIN_NUMBER;
			}

			if (STATUS_OK == ret_status)
			{
				DWORD phone1Num = atoi(phone1->phone_number);
				DWORD phone2Num = atoi(phone2->phone_number);
				if (phone2Num < phone1Num)
				{
					msg << "\nwrong dial-in range - first phone:" << phone1->phone_number << ", last phone:" << phone2->phone_number;
					ret_status = STATUS_GATEWAY_WRONG_DIALIN_NUMBERS_RANGE;
				}
			}
		}
	}
	PTRACE(eLevelInfoNormal, msg.str().c_str());
	return ret_status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::SetMRDefaultParams()
{
	CServicePhoneStr* pServicePhone = GetFirstServicePhone();

	if (IsEnableIsdnPstnAccess())
	{
		CConfPartyProcess* pConfPartyProcess  = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
		RTM_ISDN_PARAMS_MCMS_S* pIsdnServiceStruct = NULL;

		// Keep the service name when exists
		const char* rsrvServiceName = (pServicePhone) ? pServicePhone->GetNetServiceName() : "";

		if (rsrvServiceName[0] == '\0' || !strcmp(rsrvServiceName, "[Default Service]"))
			pIsdnServiceStruct = pConfPartyProcess->GetIsdnService("");   // take the Default service
		else
			pIsdnServiceStruct = pConfPartyProcess->GetIsdnService(rsrvServiceName);

		// Keep the service params and keep it in the reservation
		if (pIsdnServiceStruct && pServicePhone)
			pServicePhone->SetNetServiceName(((char*)pIsdnServiceStruct->serviceName));
		else
		{
			TRACEINTO << "Failed, EQ:" << GetName() << " do not have service";
			return STATUS_NET_SERVICE_NOT_FOUND;
		}
	}
	else
	{
		// In case EQ/MR/Conf does not enable PSTN - make the service name empty
		if (pServicePhone)
			pServicePhone->SetNetServiceName("");
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestAddValidity(BYTE isDefaultProfile)
{
	TRACEINTO << "ConfName:" << GetName();

	STATUS status = STATUS_OK;

	DWORD profileId = 0xFFFFFFFF;
	if (IsConfFromProfile(profileId) == YES)
	{
		CCommResApi* pProfile = (CCommResApi*)::GetpProfilesDB()->GetCurrentRsrv(profileId);
		if (pProfile)
			SetMyProfileBasedParams(pProfile);
		else
			status = STATUS_PROFILE_NOT_FOUND;

		POBJDELETE(pProfile);
	}
	else // new profile - set the video session according to the system mode
	{
		if (isDefaultProfile == FALSE)
			FitVideoSessionMode();

		TRACEINTO << "VideoSession:" << (int)m_videoSession << "- Profile does not exist";
	}

	if (STATUS_OK == status)
	{
		status = (static_cast<CCommRes*> (this))->IsConfNameExists();
		TRACEINTO << "IsConfNameExists:" << (int)status;
	}

	if (STATUS_OK == status)
	{
		status = (static_cast<CCommRes*> (this))->TestValidity();
		TRACEINTO << "TestValidity:" << (int)status;
	}

	// VNGR-17629 set a new function for Warning messages when status is STATUS_OK to avoid warning mssgs to override the create validity.
	// MUST BE THE LAST CHECK
	if (STATUS_OK == status)
	{
		status = (static_cast<CCommRes*> (this))->TestValidityAndSetWarnings();
		TRACEINTO << "TestValidityAndSetWarnings:" << (int)status;
	}

	// MUST BE THE LAST CHECK

	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestUpdateValidity()
{
	TRACEINTO << "ConfName:" << GetName();

	STATUS status = (static_cast<CCommRes*> (this))->TestUpdateDisplayName();
	FitVideoSessionMode();
	if (STATUS_OK == status)
		status = (static_cast<CCommRes*> (this))->TestValidity();

	if (STATUS_OK == status)
		status = (static_cast<CCommRes*>(this))->TestValidityAndSetWarnings();

	return status;
}

//--------------------------------------------------------------------------
void CCommRes::FitVideoSessionMode()
{
	if (m_HD)
		m_videoSession = VIDEO_SWITCH;
	else
		m_videoSession = ::GetIsCOPdongleSysMode() ? VIDEO_SESSION_COP : CONTINUOUS_PRESENCE;
}

//--------------------------------------------------------------------------
void CCommRes::SetOperatorConfFromProfile()
{
	DWORD profileId = 0xFFFFFFFF;
	if (IsConfFromProfile(profileId) == YES)
	{
		CCommResApi* pProfile = (CCommResApi*)::GetpProfilesDB()->GetCurrentRsrv(profileId);
		if (pProfile)
		{
			BYTE operator_conf_profile = (static_cast<CCommRes*> (pProfile))->GetOperatorConf();
			if (operator_conf_profile)
			{
				SetOperatorConf(operator_conf_profile);
				TRACEINTO << "IsOperatorConf:1";
			}
			POBJDELETE(pProfile);
		}
	}
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckCurrReservSysMode() // 2 modes cop/cp
{
	TRACEINTO << "ConfName:" << GetName();

	DWORD profileId      = 0xFFFFFFFF;
	BYTE  isCOPRsrvation = NO;
	BYTE  isHDVSW        = NO;
	BOOL  isCopLicense   = ::GetIsCOPdongleSysMode();

	if (::GetIsCOPdongleSysMode() && !IsFeatureSupportedBySystem(eFeatureCOP))
	{
		TRACEINTO << "Failed, System Cards mode is not supported in COP mode";
		SetResSts(eWrongSysMode);
		return STATUS_ILLEGAL_SYS_MODE;
	}

	if (IsConfFromProfile(profileId) == YES)
	{
		CCommResApi* pProfile = (CCommResApi*)::GetpProfilesDB()->GetCurrentRsrv(profileId);

		if (!pProfile)
		{
			POBJDELETE(pProfile);
			TRACEINTO << "ProfileId:" << profileId << " - Failed, Profile not found";
			return STATUS_ILLEGAL_SYS_MODE;
		}

		isCOPRsrvation = (static_cast<CCommRes*>(pProfile))->IsCOPReservation();
		isHDVSW        = (static_cast<CCommRes*>(pProfile))->GetIsHDVSW();
		POBJDELETE(pProfile);
	}
	else
	{
		isCOPRsrvation = IsCOPReservation();
		isHDVSW        = GetIsHDVSW();
	}

	if ((isCopLicense && isCOPRsrvation) || (!isCopLicense && !isCOPRsrvation) || isHDVSW)  // Olga - ask Carmit about VSW
	{
		SetResSts(eStsOK); // Status Field
		return STATUS_OK;
	}
	else
	{
		TRACEINTO << "Failed, License does not meet the the system mode";
		SetResSts(eWrongSysMode);
		return STATUS_ILLEGAL_SYS_MODE;
	}
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestReservValidity()
{
	TRACEINTO << "ConfName:" << GetName() << ", DisplayName:" << m_confDisplayName << ", NumericConfId:" << m_NumericConfId;

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	STATUS status = CheckCurrReservSysMode(); // 2 modes cop/cp
	if (status != STATUS_OK)
		return status;

	// If Regular MR (not SIP Factory/EQ) is saved as template, always zero the Meeting Room Field
	if (IsConfTemplate() && !IsSIPFactory() && !GetEntryQ())
		SetMeetingRoom(NO);

	// Clean start time in case of Conference Template
	if (IsConfTemplate())
	{
		CStructTm tm(0, 0, 0, 0, 0, 0);
		m_startTime = tm;
	}

	WORD max_parties = GetMaxParties();
	if ((max_parties < GetNumParties()) && (max_parties != 0xFFFF))
	{
		TRACEINTO << "MaxPartiesNum:" << max_parties << ", CurPartiesNum:" << GetNumParties() << " - Failed, Parties number limit exceeded";
		return STATUS_PARTIES_NUMBER_LIMIT_EXCEEDED;
	}

	if (m_videoSession == VIDEO_SESSION_COP)
	{
		status = TestCopConfigurationValidity();
		if (status != STATUS_OK)
		{
			m_pCopConfigurationList->Dump("CCommRes::TestReservValidity - Failed, Illegal COP configuration", eLevelInfoNormal);
			return status;
		}
	}

	// conflict between conference rate HD threshold in system.cfg
	if (GetIsHDVSW())
	{
		EHDResolution hdVswResolution = GetHDResolution();
		DWORD         dwThresholdRate = 0;

		if (m_H264VSWHighProfilePreference == eAlways)
		{
			switch (hdVswResolution)
			{
				case eHD1080p60Res: sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_HD_1080p60_HP_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eHD1080Res   : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_HD_1080p_HP_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eHD720p60Res : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_HD_720p50_60_HP_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eHD720Res    : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_HD_720p30_HP_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eSDRes       : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_SD_HP_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eH264Res     : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_CIF_HP_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eH263Res     : return STATUS_H261_H263_NOT_SUPPORTED_WITH_HIGH_PROFILE;
				case eH261Res     : return STATUS_H261_H263_NOT_SUPPORTED_WITH_HIGH_PROFILE;
			} // switch
		}
		else
		{
			switch (hdVswResolution)
			{
				case eHD1080p60Res: sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_HD_1080p60_BL_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eHD1080Res   : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_HD_1080p_BL_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eHD720p60Res : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_HD_720p50_60_BL_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eHD720Res    : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_HD_720p30_BL_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eSDRes       : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_SD_BL_THRESHOLD_BITRATE, dwThresholdRate); break;
				case eH264Res     :
				case eH263Res     :
				case eH261Res     : sysConfig->GetDWORDDataByKey(CFG_KEY_VSW_CIF_BL_THRESHOLD_BITRATE, dwThresholdRate); break;
			} // switch
		}

		CCapSetInfo lCapInfo(_H264, (WORD)m_confTransferRate);
		DWORD h323Rate = lCapInfo.TranslateReservationRateToIpRate(m_confTransferRate);
		if (h323Rate < (WORD)dwThresholdRate)
		{
			TRACEINTO << "LineRate:" << (int)h323Rate << ", ThresholdRate:" << dwThresholdRate << " - Failed, The selected line rate is less than threshold rate";
			return STATUS_CONFERENCE_RATE_BELLOW_HD_THRESHOLD;
		}

		if (m_ShowContentAsVideo)
		{
			TRACEINTO << "'Content as Video' is not allowed in VSW conference, change it to NO";
			m_ShowContentAsVideo = NO;
		}

		if (m_isHighProfileContent)
		{
			TRACEINTO << "'High profile content' is not allowed in VSW conference, change it to NO";
			SetIsHighProfileContent(NO);
		}
	}

	// BRIDGE-1296: Gesher should support H263 content?
	//  ||eProductTypeGesher == prodType || eProductTypeNinja == prodType

	if (m_confMediaType == eSvcOnly || m_confMediaType == eMixAvcSvc || m_confMediaType == eMixAvcSvcVsw)
	{

			eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
			if((eProductTypeSoftMCUMfw == curProductType) || (eProductTypeGesher == curProductType))
			{
				m_PresentationProtocol = eH264Fix;
				m_cascadeOptimizeResolution = (BYTE)e_res_720_5fps;
				SetIsHighProfileContent(NO);
				
			}
			if((eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily()) && 
			(eSystemCardsMode_breeze ==GetSystemCardsBasedMode()))
			{
				TRACEINTO << "'For MPMx, only support H.264 base profile up to 1080p15 or H.263.";
				SetIsHighProfileContent(NO);
				if(m_cascadeOptimizeResolution > e_res_1080_15fps)
					m_cascadeOptimizeResolution = e_res_1080_15fps;
			}
			
			if( m_TipCompatibility == eTipCompatibleVideoAndContent ||  m_TipCompatibility == eTipCompatiblePreferTIP  )
			{
				TRACEINTO << "TIP content changing to h264 fixed ";
				m_PresentationProtocol = eH264Fix;
				SetIsHighProfileContent(NO);
			}
			
			if ( strlen(GetFocusUriScheduling()) ) //AV-MCU schedule connection in SVC involved conf is NOT allowd.
			{
				TRACEINTO << "Scheduled AV-MCU not supported in SVC related conf ==> Conf changed to AvcOnly";
				m_confMediaType = eAvcOnly;
			}
	}
	else
	{
		if( m_TipCompatibility == eTipCompatibleVideoAndContent ||  m_TipCompatibility == eTipCompatiblePreferTIP  ) //BRIDGE-14495
		{
			TRACEINTO << "TIP content changing to h264 fixed ";
			m_PresentationProtocol = eH264Fix;
			SetIsHighProfileContent(NO);
		}
		// Romem - Do noy let EQ get new presentaion protocols
		/*if (GetEntryQ())
		{
			if ((m_PresentationProtocol == eH264Fix) || (m_PresentationProtocol == eH264Dynamic))
			{
				m_PresentationProtocol = ePresentationAuto;
			}
		}
		else*/
		{
			BOOL bIsForceH264Content;
			sysConfig->GetBOOLDataByKey("FORCE_CONTENT_TO_H264_HD", bIsForceH264Content);
			if (bIsForceH264Content && !GetContentMultiResolutionEnabled())
			{
				if (m_PresentationProtocol == ePresentationAuto)
					m_PresentationProtocol = eH264Dynamic;

			}
		}
	}

	status = CheckXCodeConfContentValidity();
	if (status != STATUS_OK)
		return status;

	if(!isExternalIvrControl()) //AT&T only add this limitation
	{
		status = CheckIvrServiceValidity();
		if (status != STATUS_OK)
			return status;
	}

	// EE-462
	status = CheckIvrProductTypeValidity();
	if (status != STATUS_OK)
		return status;
	//AT&T
	if (GetEntryQ())
	{
		if(isIvrProviderEQ())
		{
			status = CheckIvrProviderEQValidity();
			if (status != STATUS_OK)
				return status;
		}

		if(isExternalIvrControl())
		{
			status = CheckExternalIvrControlValidity();
			if (status != STATUS_OK)
				return status;
		}
	}

	if (GetIsEncryption())
	{
		status = TestEncryptionConfValidity();
		if (status != STATUS_OK)
			return status;
	}

	// Make sure encryption value is according to the Dongle
	if (::GetDongleEncryptionValue() == FALSE && GetIsEncryption() == YES)
	{
		PASSERTMSG_AND_RETURN_VALUE(IsTemplate(), "Failed update reservation profile with encryption while DONGLE settings does not allow it", STATUS_DONGLE_ENCRYPTION_VAL_IS_FALSE);

		TRACEINTO << "Reservation profile with encryption while DONGLE settings does not allow it, change Encryption to NO";
		SetEncryptionParameters(NO, eEncryptNone);
	}

	BOOL bIsH239;
	sysConfig->GetBOOLDataByKey("ENABLE_H239", bIsH239);
	if (!bIsH239)
	{
		TRACEINTO << "'Content as Video' is not allowed when ENABLE_H239 system flag is disabled, change it to NO";
		m_ShowContentAsVideo = NO;
	}

	// BRIDGE-12708 - Block the Motion option for SVC and MixedMode since SVC support only 30fps
	if (m_confMediaType == eSvcOnly || m_confMediaType == eMixAvcSvc || m_confMediaType == eMixAvcSvcVsw)
	{
		if (eVideoQualityMotion == GetVideoQuality())
		{
			SetVideoQuality(eVideoQualitySharpness);
			TRACEINTO << "Motion option was disabled for SVC and MixedMode since SVC support only 30fps";
		}
	}

	// VNGR-11671
	if (eVideoQualityMotion == GetVideoQuality() && GetIsVideoClarityEnabled())
	{
		SetIsVideoClarityEnabled(NO);
		TRACEINTO << "Video clarity was disabled since Video quality is motion";
	}

	// VNGR-11671
	status = TestLectureModeValidity(m_pLectureMode);
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestLectureModeValidity(...), status:" << status, status);

	status = TestPermanentConfValidity();
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestPermanentConfValidity(...), status:" << status, status);

	status = TestReservRecordingValidity();
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestReservRecordingValidity(...), status:" << status, status);

	status = TestReservValidityOfCommonParams();
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestReservValidityOfCommonParams(...), status:" << status, status);

	status = TestTipCompatibilityValidity();
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestTipCompatibilityValidity(...), status:" << status, status);

	status =TestLicenseExpiredValidity();
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestLicenseExpiredValidity(...), status:" << status, status);

	if (m_confMediaType == eSvcOnly)
	{
		status =TestLicenseSvcConfValidity();
		TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestLicenseSvcConfValidity(...), status:" << status, status);
	}

	//ms svc
	status = TestMsSvcVideoModeValidity();
		TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestTipCompatibilityValidity(...), status:" << status, status);

	// Add Forced Party Id a in case of ConfTemplate and add some more Auto Correction of certain parameters
	AutoCorrectPartyLayoutData();

	// parties test
	CRsrvParty* pParty = GetFirstParty();
	while (pParty != NULL)
	{
		if (pParty->IsUndefinedParty() && IsMeetingRoom() && GetMeetingRoomState() == MEETING_ROOM_PASSIVE_STATE)
		{
			pParty->SetUndefinedType(NO);
			TRACEINTO << "PartyId:" << pParty->GetPartyId() << " - Found undefined party, change it to defined";
		}

		status = TestPartyValidity(pParty);
		TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestPartyValidity(...), PatryId:" << pParty->GetPartyId(), status);

		pParty = GetNextParty();
	}

	if (FALSE == m_startConfRequiresLeaderOnOff && TRUE == m_terminateConfAfterChairDropped)
	{
		return STATUS_INCONSISTENT_PARAMETERS;
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestValidityAndSetWarnings()
{
	STATUS status = STATUS_OK;
	BOOL   isItp  = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ITP_CERTIFICATION", isItp);

	TRACEINTO << "ConfName:" << GetName() << ", ITP_CERTIFICATION:" << (int)isItp;

	if (isItp == NO && GetTelePresenceModeConfiguration() == YES)
	{
		SetTelePresenceModeConfiguration(NO);
		TRACEINTO << "'Telepresence Mode' cannot be enabled because the ITP_CERTIFICATION system flag is set to OFF";
		status = (STATUS_TELEPRESENCE_CHANGED_TO_OFF_ITP_CERTIFICATION_IS_OFF_IN_SYSTEM | WARNING_MASK);
	}
	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestCopConfigurationValidity()
{
	TRACEINTO << "ConfName:" << GetName();

	CCapSetInfo lCapInfo = eUnknownAlgorithemCapCode;

	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	CCopVideoParams* pCopVideoParams      = m_pCopConfigurationList->GetVideoMode(0);
	if (IsFeatureSupportedBySystem(eFeatureH264HighProfile))  // In MPMx mode the level 0 Video Protocol must be H264 or H264 (High Profile)
	{
		if (pCopVideoParams->GetProtocol() != VIDEO_PROTOCOL_H264 && pCopVideoParams->GetProtocol() != VIDEO_PROTOCOL_H264_HIGH_PROFILE)
		{
			TRACEINTO << "Failed, Level 0 Video Protocol must be H264 or H264 (High Profile)";
			return STATUS_COP_LEVEL_INVALID_VIDEO_PROTOCOL;
		}
	}
	else
	{
		if (pCopVideoParams->GetProtocol() != VIDEO_PROTOCOL_H264)
		{
			TRACEINTO << "Failed, Level 0 Video Protocol must be H264";
			return STATUS_COP_LEVEL_INVALID_VIDEO_PROTOCOL;
		}
	}

	// Fix difference between call rate and highest level bit rate:
	BYTE bitRate = pCopVideoParams->GetBitRate();
	if (m_confTransferRate != bitRate)
	{
		TRACEINTO << "Change conference call rate to be the same as the high level bit rate, NewBitRate:" << (int)bitRate;
		m_confTransferRate = bitRate;
	}

	std::ostringstream msg;
	msg.precision(0);
	msg << "CCommRes::TestCopConfigurationValidity" << endl;
	msg << " ------+---------+------------+--------+-----------+----------+" << endl;
	msg << " Level | BitRate | MinBitRate | Format | FrameRate | Protocol |" << endl;
	msg << " ------+---------+------------+--------+-----------+----------+" << endl;

	// Test inside level params:
	for (int index = 0; index < NUMBER_OF_COP_LEVELS; index++)
	{
		pCopVideoParams = m_pCopConfigurationList->GetVideoMode(index);
		BYTE format     = pCopVideoParams->GetFormat();
		BYTE frameRate  = pCopVideoParams->GetFrameRate();
		BYTE protocol   = pCopVideoParams->GetProtocol();

		// Fix frame rate (only for HD720 can be selected frame rate great than eCopVideoFrameRate_30)
		if (format != eCopLevelEncoderVideoFormat_HD720p && frameRate > eCopVideoFrameRate_30)
		{
			TRACEINTO << "Frame rate 30 selected when format different from 720p, CopLevelIndex:" << index;
			pCopVideoParams->SetFrameRate(eCopVideoFrameRate_30);
		}

		// Check bit rate thresholds:
		bitRate = pCopVideoParams->GetBitRate();
		DWORD dwBitRate    = lCapInfo.TranslateReservationRateToIpRate(bitRate);
		DWORD dwMinBitRate = GetMinBitRateForCopLevel(format, frameRate, protocol);

		msg << " " << setw( 5) << right << (WORD)index << " |"
		    << " " << setw( 7) << right << (WORD)dwBitRate << " |"
		    << " " << setw(10) << right << (WORD)dwMinBitRate << " |"
		    << " " << setw( 6) << right << (WORD)format << " |"
		    << " " << setw( 9) << right << (WORD)frameRate << " |"
		    << " " << setw( 8) << right << (WORD)protocol << " |" << endl;

		if (dwBitRate < dwMinBitRate)
		{
			TRACEINTO << "Failed, Low bit rate, Index:" << index;
			return STATUS_COP_LEVEL_LOW_BIT_RATE;
		}
	}
	msg << " ------+---------+------------+--------+-----------+----------+";

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	// Check identical levels:
	for (int i = 0; i < NUMBER_OF_COP_LEVELS - 1; i++)
	{
		for (int j = i + 1; j < NUMBER_OF_COP_LEVELS; j++)
		{
			CCopVideoParams* pPrevCopVideoParams = m_pCopConfigurationList->GetVideoMode(i);
			CCopVideoParams* pNextCopVideoParams = m_pCopConfigurationList->GetVideoMode(j);
			if (*pPrevCopVideoParams == *pNextCopVideoParams)
			{
				TRACEINTO << "Failed, Same level params, level:" << i << " and level:" << j << " are the same";
				return STATUS_COP_LEVELS_IDENTICAL;
			}
		}
	}

	BYTE isValidLevelParams = TRUE;
	msg.str("CCommRes::TestCopConfigurationValidity");

	// Check that each level is equal or lower than previous level:
	for (BYTE prevLevel = 0, nextLevel = 1; prevLevel < NUMBER_OF_COP_LEVELS - 1; prevLevel++, nextLevel++)
	{
		CCopVideoParams* pPrevCopVideoParams = m_pCopConfigurationList->GetVideoMode(prevLevel);
		CCopVideoParams* pNextCopVideoParams = m_pCopConfigurationList->GetVideoMode(nextLevel);

		int  prevBitRate   = lCapInfo.TranslateReservationRateToIpRate(pPrevCopVideoParams->GetBitRate());
		int  nextBitRate   = lCapInfo.TranslateReservationRateToIpRate(pNextCopVideoParams->GetBitRate());

		BYTE prevFormat    = pPrevCopVideoParams->GetFormat();
		BYTE nextFormat    = pNextCopVideoParams->GetFormat();
		BYTE prevFrameRate = pPrevCopVideoParams->GetFrameRate();
		BYTE nextFrameRate = pNextCopVideoParams->GetFrameRate();
		BYTE prevProtocol  = pPrevCopVideoParams->GetProtocol();
		BYTE nextProtocol  = pNextCopVideoParams->GetProtocol();

		if (IsFeatureSupportedBySystem(eFeatureH264HighProfile))  // In MPMx mode Video Protocol H264 and H264 (High Profile) are treated as same level
		{
			prevProtocol = (prevProtocol == VIDEO_PROTOCOL_H264_HIGH_PROFILE) ? VIDEO_PROTOCOL_H264 : prevProtocol;
			nextProtocol = (nextProtocol == VIDEO_PROTOCOL_H264_HIGH_PROFILE) ? VIDEO_PROTOCOL_H264 : nextProtocol;
		}

		if (nextProtocol > prevProtocol)
		{
			msg << " - Failed, COP level: " << nextLevel << ", video protocol:" << pNextCopVideoParams->GetProtocol()
					<< " is higher than COP level:" << prevLevel << ", video protocol:" << pPrevCopVideoParams->GetProtocol();
			isValidLevelParams = FALSE;
		}
		else if (nextBitRate > prevBitRate)
		{
			msg << " - Failed, COP level:" << nextLevel << ", video rate:" << nextBitRate
					<< " is higher than COP level:" << prevLevel << ", video rate:" << prevBitRate;
			isValidLevelParams = FALSE;
		}
		else if (nextProtocol == prevProtocol && nextFormat > prevFormat)
		{
			msg << " - Failed, COP level:" << nextLevel << ", video format:" << nextFormat
					<< " is higher than COP level:" << prevLevel << ", video format:" << prevFormat
					<< " and video protocol is the same in both levels";
			isValidLevelParams = FALSE;
		}
		else if (nextProtocol == prevProtocol && nextFormat == prevFormat && nextBitRate > prevBitRate && nextFrameRate > prevFrameRate)
		{
			msg << " - Failed, COP level:" << nextLevel << ", frame rate:" << nextFrameRate
					<< " is higher than COP level:" << prevLevel << ", frame rate:" << prevFrameRate
					<< " and all other level params are the same in both levels";
			isValidLevelParams = FALSE;
		}

		if (!isValidLevelParams)
		{
			PTRACE(eLevelInfoNormal, msg.str().c_str());
			return STATUS_COP_LEVEL_HIGHER_THAN_PREVIOUS;
		}
	}

	// Check max for level 0:
	pCopVideoParams = m_pCopConfigurationList->GetVideoMode(0);
	if (pCopVideoParams->GetFormat() == eCopLevelEncoderVideoFormat_HD1080p && pCopVideoParams->GetFrameRate() > eCopVideoFrameRate_30)
	{
		TRACEINTO << "Failed, Level 0 illegal";
		return STATUS_COP_LEVEL_PARAMS_HIGHER_THAN_MAX_PARAMS;  // can't occur because we fixed the frame rate before.
	}

	// Check max for level 1:
	pCopVideoParams = m_pCopConfigurationList->GetVideoMode(1);
	if ((pCopVideoParams->GetFormat() > eCopLevelEncoderVideoFormat_HD720p) ||
			(pCopVideoParams->GetFormat() == eCopLevelEncoderVideoFormat_HD720p && pCopVideoParams->GetFrameRate() > eCopVideoFrameRate_30))
	{
		TRACEINTO << "Failed, Level 1 illegal";
		return STATUS_COP_LEVEL_PARAMS_HIGHER_THAN_MAX_PARAMS;
	}

	// Check max for level 2:
	pCopVideoParams = m_pCopConfigurationList->GetVideoMode(2);
	int maxEncoderVideoFormat = eCopLevelEncoderVideoFormat_CIF;
	if (pCopVideoParams->GetProtocol() == VIDEO_PROTOCOL_H264 || pCopVideoParams->GetProtocol() == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
		maxEncoderVideoFormat = eCopLevelEncoderVideoFormat_4CIF_16_9;

	if (pCopVideoParams->GetFormat() > maxEncoderVideoFormat)
	{
		TRACEINTO << "Failed, Level 2 illegal";
		return STATUS_COP_LEVEL_PARAMS_HIGHER_THAN_MAX_PARAMS;
	}

	// Check max for level 3:
	pCopVideoParams = m_pCopConfigurationList->GetVideoMode(3);
	if (pCopVideoParams->GetFormat() > eCopLevelEncoderVideoFormat_CIF)
	{
		TRACEINTO << "Failed, Level 3 illegal";
		return STATUS_COP_LEVEL_PARAMS_HIGHER_THAN_MAX_PARAMS;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestReservValidityWithoutProfileBasedParams()
{
	TRACEINTO << "ConfName:" << GetName();

	STATUS status = STATUS_OK;

	if (!IsFeatureSupportedBySystem(eFeatureEchoSuppression))
	{
		SetEchoSuppression(FALSE);
	}
	if (!IsFeatureSupportedBySystem(eFeatureKeyboardNoiseSuppression))
	{
		SetKeyboardSuppression(FALSE);
	}

	if (!IsMeetingRoom() && !GetEntryQ() && !IsTemplate() && !IsConfTemplate())
	{
		// VNGR-11635 - Correct Reservation/OnGoing Conf duration if needed
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

		BOOL IsEnableAutoExtension = FALSE;
		sysConfig->GetBOOLDataByKey("ENABLE_AUTO_EXTENSION", IsEnableAutoExtension);

		DWORD ExtensionTimeInterval = 0;
		sysConfig->GetDWORDDataByKey("EXTENSION_TIME_INTERVAL", ExtensionTimeInterval);

		CStructTm zeroTm(0, 0, 0, 0, 0, 0);
		if (m_startTime == zeroTm)
		{
			CStructTm curTime;
			STATUS    timeStatus = SystemGetTime(curTime);
			SetStartTime(curTime);
			SetEndTime();
		}

		ExtendDurationIfNeed(IsEnableAutoExtension, ExtensionTimeInterval);
	}

	FillEmptyDiplayNameOrName();

	// Make sure Billing Data length is not over limit
	if (MAX_BILLING_INFO_SIZE < m_BillingInfo.GetStringLength())
	{
		TRACEINTO << "Failed, Illegal billing data length";
		return STATUS_ILLEGAL_BILLING_DATA_LENGTH;
	}

	const char* pNumericId = GetNumericConfId();
	if (pNumericId && IsSIPFactory() != YES)
	{
		DWORD len = strlen(pNumericId);
		int   LettersFoundInString = 0;
		// Verify NID has only digits
		for (DWORD i = 0; i < len && 0 == LettersFoundInString; i++)
		{
			if (pNumericId[i] < '0' || pNumericId[i] > '9')
				LettersFoundInString = 1;
		}

		if (LettersFoundInString)
		{
			TRACEINTO << "Failed, Conference Id must be numeric";
			return STATUS_CONFERENCE_ID_MUST_BE_NUMERIC;
		}
	}

	status = CheckPasswordsValidity();
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in CheckPasswordsValidity(...), status:" << status, status);

	// Check the conference duration (168 hours is the Maximum)
	CStructTm maxTimeAllowed(m_duration.m_day, m_duration.m_mon, m_duration.m_year, MAXIMUM_CONF_DURATION, 0, 0);
	if (maxTimeAllowed < m_duration)
	{
		TRACEINTO << "Failed, Illegal end time";
		return STATUS_ILLEGAL_END_TIME;
	}

	// Test duration validity
	const CStructTm* m_Time;
	m_Time = GetDurationTime();
	if (m_Time->m_day == 0 && m_Time->m_hour == 0 && m_Time->m_min == 0 && m_Time->m_mon == 0 && m_Time->m_sec == 0)
	{
		TRACEINTO << "Failed, Zero end time";
		return STATUS_ILLEGAL_END_TIME;
	}

	DWORD templateId = 0xFFFFFFFF;
	if (GetEntryQ() == YES)
	{
		if (GetAdHoc())  // The profile configured in Ad hoc EQ can't be SWCP or Quad Views
		{
			templateId = GetAdHocProfileId();
			if (templateId != (DWORD)-1)
			{
				status = IsProfileExists(templateId);
			}
			else
				status = STATUS_AD_HOC_EQ_MUST_HAVE_PROFILE;
		}
	}

	if (IsSIPFactory() == YES)
	{
		DWORD templateId = GetAdHocProfileId();
		if (templateId != (DWORD)-1)
		{
			status = IsProfileExists(templateId);
		}
		else
		{
			TRACEINTO << "Failed, Profile not found";
			return STATUS_PROFILE_NOT_FOUND;
		}
	}

	if (GetIsGateway() && GetGWDialOutProtocols() == 0)
	{
		TRACEINTO << "Failed, Gateway must have dial-out protocol";
		return STATUS_GATEWAY_MUST_HAVE_DIAL_OUT_PROTOCOL;
	}

	TRACECOND(status != STATUS_OK, "Failed with status:" << status);

	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestReservValidityOfCommonParams()
{
	DWORD profileId = 0xFFFFFFFF;
	STATUS status = IsProfileExists(profileId);
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in IsProfileExists(...), status:" << status, status);

	FitVideoSessionMode();
	TRACEINTO <<"m_HD:"<< (int)m_HD;
	SetSetLectureModeSameLayoutRules(m_pLectureMode);

	status = TestMRValidity(); // allow ISDN/PSTN participant direct dial-in to conferences and Meeting Rooms as well
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestMRValidity(...), status:" << status, status);

	if (GetIsGateway())
	{
		status = TestAndUpdateGWProfileValidity();
		TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestAndUpdateGWProfileValidity(...), status:" << status, status);
	}

	status = TestReservValidityWithoutProfileBasedParams();
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestReservValidityWithoutProfileBasedParams(...), status:" << status, status);

	status = TestMinAudioVideoPartiesValidity();
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestMinAudioVideoPartiesValidity(...), status:" << status, status);

	status =TestConfMediaTypeValidity(); // CMT@
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed in TestConfMediaTypeValidity(...), status:" << status, status); // CMT@

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestConfMediaTypeValidity()
{
	eProductType ePT = CProcessBase::GetProcess()->GetProductType();
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

	if (eProductTypeRMX1500 == ePT || eProductTypeRMX2000 == ePT || eProductTypeRMX4000 == ePT)
	{
		if (m_confMediaType != eAvcOnly && m_confMediaType != eSvcOnly && m_confMediaType != eMixAvcSvc)
		{
			TRACEINTO << "ConfMediaTye:" << m_confMediaType << " - Failed, invalid media type";
			return STATUS_ILLEGAL_CONFERENCE_MEDIA_TYPE;
		}
	}
	if (eProductTypeSoftMCUMfw == ePT )
	{
		if (!GetEntryQ() && (m_confMediaType != eSvcOnly && m_confMediaType != eMixAvcSvc && m_confMediaType != eMixAvcSvcVsw)) // eMixAvcSvc is replaced later by eMixAvcSvcVsw
		{
			TRACEINTO << "ConfMediaTye:" << m_confMediaType << " - Failed, invalid MFW media type";
			return STATUS_ILLEGAL_CONFERENCE_MEDIA_TYPE;
		}
	}

	if ((eProductFamilySoftMcu == curProductFamily) && (eProductTypeSoftMCUMfw != ePT)) /* all SoftMcu products except Mfw*/
	{
		if (!GetEntryQ() && (m_confMediaType != eAvcOnly && m_confMediaType != eSvcOnly && m_confMediaType != eMixAvcSvc))
		{
			TRACEINTO << "ConfMediaTye:" << m_confMediaType << " - Failed, invalid SW-MCU media type";
			return STATUS_ILLEGAL_CONFERENCE_MEDIA_TYPE;
		}
	}

	// EE-462
//	if (m_confMediaType == eSvcOnly || m_confMediaType == eMixAvcSvc || m_confMediaType == eMixAvcSvcVsw)
//	{
//		if (!IsIvrForSVCEnabled() && GetEntryQ())
//		{
//			TRACEINTO << "ConfMediaTye:" << m_confMediaType << " - Failed, invalid media type, only AVC-ONLY allowed for EQ";
//			return STATUS_EQ_NOT_ALLOWED_IN_MEDIA_RELAY_CONF;
//		}
//	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCommRes::SetDefaultParamsAndAutoCorrectForConfMediaType()
{
	// CreateConfMediaRelayTypeTest.py
	eProductType ePT = CProcessBase::GetProcess()->GetProductType();
	CMedString   sLog;

	TRACEINTO << "ConfMediaType: " << ConfMediaTypeToString(m_confMediaType);

	if ((eMixAvcSvc == m_confMediaType) && m_HD)
	{
		TRACEINTO << "In case of mixavcsvc and the VSW(HD) is on we will update the type to eMixAvcSvcVsw and disable VSW";
		m_confMediaType = eMixAvcSvcVsw;
		m_HD            = NO;
		m_videoSession  = CONTINUOUS_PRESENCE;   // need to disable the vsw value.
	}

	// Lior B. asked to add it on 16.10, maybe removed in future versions
	if ((eMixAvcSvc == m_confMediaType) && (eProductTypeSoftMCUMfw == ePT))
	{
		TRACEINTO << "In case of mixavcsvc and the product type is MFW(IBM) on we will update the type to eMixAvcSvcVsw";
		m_confMediaType = eMixAvcSvcVsw;
	}

	if (((eSvcOnly == m_confMediaType) || (eMixAvcSvc == m_confMediaType) || (eMixAvcSvcVsw == m_confMediaType)) && m_HD)
	{
		TRACEINTO << "In case of relay only/ mixavcsvc /mixavcsvc_vsw the VSW(HD)should be disabled";
		m_HD           = NO;
		m_videoSession = CONTINUOUS_PRESENCE;     // need to disable the vsw value.
	}

	if (m_confMediaType == eSvcOnly || m_confMediaType == eMixAvcSvc || m_confMediaType == eMixAvcSvcVsw)
	{
		// blocks dial-out Participants
		int pNum = m_numParties;
		for (int i = pNum-1; i >= 0; i--)
		{
			if (m_pParty[i] != NULL) // in SVC-Only it is NOT-OK to have dial-out participant, in Mix it is NOT-OK to have ISDN
			{ // qqqAmir: ISDN to remove party anyway
				// SVC only for Dial-Out
				if ((m_confMediaType == eSvcOnly && (m_pParty[i]->GetConnectionType() == DIAL_OUT))
				    || ((m_confMediaType == eMixAvcSvc || m_confMediaType == eMixAvcSvcVsw) && (m_pParty[i]->GetNetInterfaceType() == ISDN_INTERFACE_TYPE)))
				{
					Cancel(m_pParty[i]->GetPartyId());
				}
			}
		}


		// blocks cascade (also need to block party cascade?)
		SetCascadeMode(NO);

		// Blocks Gathering
		m_bGatheringEnabled = FALSE;

		// blocks ISDN
		// m_network = NETWORK_H323;	// still need to block ISDN/PSTN in the Lobby
		// block  Lecture Mode
		m_pLectureMode->SetLectureModeType(NO);

		
		// block TIP
		m_TipCompatibility      = eTipCompatibleNone;
		m_AutoRedial            = NO;
		m_isVideoClarityEnabled = NO;
		m_isAutoBrightness      = NO;

		
		if (m_confMediaType == eSvcOnly)
		{
			SetContentMultiResolutionEnabled(NO); // disable content TX for SVC ONLY
			m_ShowContentAsVideo    = FALSE;
		}

		// Message Overlay
		if (m_pMessageOverlayInfo)
			m_pMessageOverlayInfo->SetMessageOnOff(NO);

		if (m_confMediaType == eSvcOnly)
		{
			// skins
			if (m_pVisualEffectsInfo)
			{
				m_pVisualEffectsInfo->SetlayoutBorderEnable(NO);
				m_pVisualEffectsInfo->SetSpeakerNotationEnable(NO);
				m_pVisualEffectsInfo->SetSiteNamesEnable(NO);
			}

			// blocks RSS (recording link)
			SetEnableRecording(0);
			SetRecLinkName("");
			// LPR is not supported (so DBA is part of it)
			m_isLpr        = FALSE;
			m_operatorConf = NO;
		}

		if (m_confMediaType == eSvcOnly || m_confMediaType == eMixAvcSvcVsw)
		{
			m_isSameLayout = NO;
			m_isAutoLayout = NO;
			// Site Name
			m_isSiteNamesEnabled = NO; // for Jira Bridge-1961: Mix-CP enables SiteName for AVC EP
			if ((eCascadeOptimizeResolutionEnum)m_cascadeOptimizeResolution == e_res_dummy)
			{
				m_cascadeOptimizeResolution = (BYTE)e_res_720_5fps;
			}

			m_isAutoMuteNoisyParties = NO;
		}

		DWORD       reservationRate = GetConfTransferRate();
		CCapSetInfo lCapInfo        = eUnknownAlgorithemCapCode;
		DWORD       confRate        = lCapInfo.TranslateReservationRateToIpRate(reservationRate);
		TRACEINTO << "ConfRate: " << confRate << ", ReservationRate:" << reservationRate;

		if (confRate < VIDEO_RELAY_MIN_CALL_RATE_THRESHOLD)
		{
			SetConfTransferRate(Xfer_128);
			TRACEINTO << "ConfRate:" << confRate << ", VIDEO_RELAY_MIN_CALL_RATE_THRESHOLD:" << (WORD)VIDEO_RELAY_MIN_CALL_RATE_THRESHOLD << ", NewConfRate:" << (WORD)GetConfTransferRate();
		}

		if ((m_confMediaType == eSvcOnly || m_confMediaType == eMixAvcSvc) && confRate > VIDEO_RELAY_MAX_CALL_RATE_THRESHOLD)
		{
			SetConfTransferRate(Xfer_4096);
			TRACEINTO << "ConfRate:" << confRate << ", VIDEO_RELAY_MAX_CALL_RATE_THRESHOLD:" << (WORD)VIDEO_RELAY_MAX_CALL_RATE_THRESHOLD << ", NewConfRate:" << (WORD)GetConfTransferRate();
		}

		if (m_confMediaType == eMixAvcSvcVsw && confRate > AVC_SVC_VSW_MAX_CALL_RATE_THRESHOLD)
		{
			SetConfTransferRate(Xfer_4096);
			TRACEINTO << "ConfRate:" << confRate << ", AVC_SVC_VSW_MAX_CALL_RATE_THRESHOLD:" << (WORD)AVC_SVC_VSW_MAX_CALL_RATE_THRESHOLD << ", NewConfRate:" << (WORD)GetConfTransferRate();
		}

		if (m_confMediaType == eMixAvcSvcVsw)
		{
			SetMinConfTransferRateByOpPtPreset(confRate);
		}
	}
	else
	{
		TRACEINTO << "NOT SVCxx conf media type";
	}
}

//--------------------------------------------------------------------------
void CCommRes::SetMinConfTransferRateByOpPtPreset(DWORD dwConfRate)
{
	DWORD dwConfRateMinThreshold = AVC_SVC_VSW_MIN_CALL_RATE_THRESHOLD;
	char* pszMinThreshold = "AVC_SVC_VSW_MIN_CALL_RATE_THRESHOLD";
	BYTE  btTransferRate = Xfer_384;
	char* pszTransferRate = "Xfer_384";
	switch (m_eOperationPointPreset)
	{
		case eOPP_mobile:
			dwConfRateMinThreshold = AVC_SVC_VSW_MOBILE_MIN_CALL_RATE_THRESHOLD;
			btTransferRate = Xfer_192;
			pszMinThreshold = "AVC_SVC_VSW_MOBILE_MIN_CALL_RATE_THRESHOLD";
			pszTransferRate = "Xfer_192";
			break;
		case eOPP_qvga:
			dwConfRateMinThreshold = AVC_SVC_VSW_QVGA_MIN_CALL_RATE_THRESHOLD;
			btTransferRate = Xfer_384;
			pszMinThreshold = "AVC_SVC_VSW_QVGA_MIN_CALL_RATE_THRESHOLD";
			pszTransferRate = "Xfer_384";
			break;
		case eOPP_cif:
			dwConfRateMinThreshold = AVC_SVC_VSW_CIF_MIN_CALL_RATE_THRESHOLD;
			btTransferRate = Xfer_512;
			pszMinThreshold = "AVC_SVC_VSW_CIF_MIN_CALL_RATE_THRESHOLD";
			pszTransferRate = "Xfer_512";
			break;
		case eOPP_vga:
			dwConfRateMinThreshold = AVC_SVC_VSW_VGA_MIN_CALL_RATE_THRESHOLD;
			btTransferRate = Xfer_512;
			pszMinThreshold = "AVC_SVC_VSW_VGA_MIN_CALL_RATE_THRESHOLD";
			pszTransferRate = "Xfer_512";
			break;
		case eOPP_sd:
			dwConfRateMinThreshold = AVC_SVC_VSW_SD_MIN_CALL_RATE_THRESHOLD;
			btTransferRate = Xfer_768;
			pszMinThreshold = "AVC_SVC_VSW_SD_MIN_CALL_RATE_THRESHOLD";
			pszTransferRate = "Xfer_768";
			break;
		case eOPP_hd:
			dwConfRateMinThreshold = AVC_SVC_VSW_HD_MIN_CALL_RATE_THRESHOLD;
			btTransferRate = Xfer_832;
			pszMinThreshold = "AVC_SVC_VSW_HD_MIN_CALL_RATE_THRESHOLD";
			pszTransferRate = "Xfer_832";
			break;
		default:
			PASSERT(m_eOperationPointPreset);
			break;
	}
	FTRACESTRFUNC(eLevelDebug) <<"\nm_eOperationPointPreset = " << m_eOperationPointPreset <<  "\nm_confMediaType = " << m_confMediaType << "\nCurrent confRate "
			<< dwConfRate << "\nMinThreshold = " << pszMinThreshold << "(" << (WORD)dwConfRateMinThreshold
			<< ")\nMin confRate = " << pszTransferRate << "(" << (WORD)btTransferRate << ")";

	if (m_confMediaType == eMixAvcSvcVsw && dwConfRate < dwConfRateMinThreshold)
	{
		SetConfTransferRate(btTransferRate);
		TRACEINTO << "\n current confRate " << dwConfRate << " < " << pszMinThreshold << "(" << (WORD)dwConfRateMinThreshold
				<< ")\n Set new confRate = " << pszTransferRate << "(" << (WORD)GetConfTransferRate() << ")";
	}

}

//--------------------------------------------------------------------------
STATUS CCommRes::IsProfileExists(DWORD& profileId)
{
	if (IsConfFromProfile(profileId) == YES)
	{
		CCommResApi* pProfile = (CCommResApi*)::GetpProfilesDB()->GetCurrentRsrv(profileId);
		if (!pProfile)
			return STATUS_PROFILE_NOT_FOUND;

		TRACEINTO << "ProfileId:" << profileId << ", ProfileName:" << pProfile->GetName();
		POBJDELETE(pProfile)
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestPartyValidity(CRsrvParty* pParty)
{
	STATUS status = STATUS_OK;
	if (pParty)
	{
		BYTE btYesNo = NO;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_AGC, btYesNo);
		if (!btYesNo)
			pParty->SetAGC(btYesNo);

		status = TestPartyConfRelatedParamsValidity(pParty);
		if (status == STATUS_OK)
		{
			FixPartyUniqueParams(pParty);
			status = TestPartyUniqueParamsValidity(pParty);
		}
	}
	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestPartyConfRelatedParamsValidity(CRsrvParty* pParty)
{
	STATUS status = TestEncryptionPartyValidity(pParty);


	if (m_isLpr)
	{
		// In order to check if conf is VSW we need to check the HD flag
		// because in this stage the conference was set as CP.
		if ((pParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE) && m_HD && !pParty->GetVoice())
		{
			TRACEINTO << "Failed, Unable to add H320 party to VSW LPR conference, PartyName:" << pParty->GetPartyId();
			return STATUS_CAN_NOT_ADD_SIP_H320_TO_VSW_LPR_CONFERENCE;
		}
	}

	if (status == STATUS_OK)
	{
		if (GetIsHDVSW() && (ISDN_INTERFACE_TYPE == pParty->GetNetInterfaceType() && !pParty->GetVoice()))
		{
			TRACEINTO << "Failed, ISDN Video participant is not supported in VS conference";
			return STATUS_ISDN_VIDEO_PARTICIPANT_IS_NOT_SUPPORTED_IN_VS_CONF; // STATUS_FAIL;
		}
	}

	if (status == STATUS_OK)
	{
		if (( GetConfMediaType() == eMixAvcSvc || GetConfMediaType() == eMixAvcSvcVsw ) && pParty->GetMsftAvmcuState() != eMsftAvmcuNone)
		{
			TRACEINTO << "Failed, AV-MCU not supported in mixed mode conf";
			return STATUS_AV_MCU_NOT_SUPPORTED_IN_MIXED_MODE_CONF; // STATUS_FAIL;
		}
	}
	if(status == STATUS_OK && pParty->GetMsftAvmcuState() != eMsftAvmcuNone)
	{

		CRsrvParty*  pConfParty = GetFirstParty(); // get the first party
		  WORD numParties = GetNumParties();
			for (WORD i = 0; i < numParties; i++)
			{
				if (pConfParty)
				{
					if(pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone )
					{

						TRACEINTO << "Failed, more than 1 av-mcu link!! ";
						return STATUS_NO_MORE_THEN_1_AV_MCU_LINK_IN_CONF_IS_ALLOWED;
					}
				}
				pConfParty = GetNextParty();
			}

	}

	if (status == STATUS_OK)
	{
		if ((m_videoSession == VIDEO_SESSION_COP) && (ISDN_INTERFACE_TYPE == pParty->GetNetInterfaceType()))
		{
			TRACEINTO << "Failed, ISDN connection is not supported in COP conference";
			return STATUS_ISDN_CONNECTIONS_NOT_SUPPORTED;
		}
	}

	if (!TestMultiCascadeValidity(*pParty))
		status = STATUS_ILLEGAL; // TODO: put appropriate error status

	//===========================================================
	// BRIDGE-414 - Max resolution for VSW parties must be auto
	//              Overriding to Auto_Res, but not a failure
	//===========================================================
	if (STATUS_OK == status && GetIsHDVSW())
	{
		int maxRes = pParty->GetMaxResolution();
		if (maxRes != eAuto_Res)
		{
			TRACEINTO << "received resolution [" << maxRes << "] VSW conferences override their parties to AUTO max resolution";
			pParty->SetMaxResolution(eAuto_Res);
		}
	}

	if (status == STATUS_OK)
	{
		status = TestCascadeLinkValidity(pParty);
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestCascadeLinkValidity(CRsrvParty* pParty)
{
	bool bIsMasterLink	= pParty->IsCascadeModeMaster();
	bool bIsSlaveLink	= pParty->IsCascadeModeSlave();
	if ( !bIsMasterLink && !bIsSlaveLink)
	{
		return STATUS_OK;
	}

	bool bIsMfw		= ( (eProductTypeSoftMCUMfw == CProcessBase::GetProcess()->GetProductType()) ? true : false );
	bool bIsSip		= ( (SIP_INTERFACE_TYPE == pParty->GetNetInterfaceType()) ? true : false );
	bool bIsDialOut	= ( (DIAL_OUT == pParty->GetConnectionType()) ? true : false );
	if ( !bIsSip || !bIsDialOut || !bIsMfw)
	{
		TRACEINTO << " not SIP or DialOut or MFW - ok (ProductType: " << (int)(CProcessBase::GetProcess()->GetProductType()) << ", interface: " << (int)(pParty->GetNetInterfaceType()) << ", connection: " << (int)(pParty->GetConnectionType()) << ")";
		return STATUS_OK;
	}

	// Currently, encryption is not supported in Cascade
//	if ( NO != pParty->GetIsEncrypted() )
//	{
//		TRACEINTO << "Currently, encryption is not supported in Cascade link";
//		pParty->SetIsEncrypted(NO);
//	}

	// Master cannot dial out as a master; change to cascade_none
	if (bIsMasterLink)
	{
		TRACEINTO << "Master dials out - illegal. Changing CascadeMode to NONE";
		pParty->SetCascadeMode(CASCADE_MODE_NONE);
		return STATUS_OK;
	}

	else // bIsSlaveLink
	{
		for (int i=0; i<(int)m_numParties; i++)
		{
			if (m_pParty[i] != NULL)
			{
				if ( m_pParty[i]->IsCascadeModeMasterOrSlave() )
				{
					TRACEINTO << "Slave dials out while a link already exists in its conf - illegal";
					return STATUS_ILLEGAL_CASCADE_TOPOLOGY | WARNING_MASK;
//					return STATUS_ILLEGAL;
				}
			}
		} // end loop over parties
	}

	TRACEINTO << "ok";

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestMinAudioVideoPartiesValidity()
{
	TRACEINTO << "ConfName:" << GetName();

	STATUS status = STATUS_OK;

	if (IsMeetingRoom() && IsPermanent())
		m_minNumAudioParties = 0;

	// Auto correct min Audio/Video parties in case of MR/Profile/Add Hoc
	if (IsTemplate() || (IsMeetingRoom() && !IsPermanent()) || GetIsAdHocConf())
	{
		m_minNumVideoParties = 0;
		m_minNumAudioParties = 0;
		return status;
	}
	else
	{
		// If min Audio/Video parties or their sum exceed Max parties in profile, reject the reservation
		if (m_minNumVideoParties > GetMaxParties() || m_minNumAudioParties > GetMaxParties() || ((m_minNumVideoParties + m_minNumAudioParties) > GetMaxParties()))
			return STATUS_NUMBER_OF_MAX_PARTIES_EXCEEDED;

		// Auto correct min Audio/Video parties in case of Invalid values
		CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
		if (m_minNumVideoParties > pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf())
		{
			TRACEINTO << "Failed, Number of min Video participants exceeds number of allowed video parties per conference";
			m_minNumVideoParties = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();
			SetInternalConfStatus(STATUS_MIN_VIDEO_PARTIES_EXCEEDS_MAX_VIDEO_PARTIES_PER_CONF);
		}

		if (m_minNumAudioParties > pConfPartyProcess->GetMaxNumberOfPartiesInConf())
		{
			TRACEINTO << "Failed, Number of min Audio participants exceeds number of allowed audio parties per conference";
			SetInternalConfStatus(STATUS_MIN_AUDIO_PARTIES_EXCEEDS_MAX_PARTIES_PER_CONF);
			m_minNumAudioParties = pConfPartyProcess->GetMaxNumberOfPartiesInConf();
		}

		if ((m_minNumVideoParties+m_minNumAudioParties) > pConfPartyProcess->GetMaxNumberOfPartiesInConf())
		{
			TRACEINTO << "Failed, Number of min Audio+Video participants exceeds number of allowed allowed parties per conference";
			SetInternalConfStatus(STATUS_MIN_VIDEO_PLUS_AUDIO_PARTIES_EXCEEDS_MAX_PARTIES_PER_CONF);
			if (m_minNumVideoParties > pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf())
				m_minNumVideoParties = pConfPartyProcess->GetMaxNumberOfVideoPartiesInConf();

			m_minNumAudioParties = pConfPartyProcess->GetMaxNumberOfPartiesInConf()-m_minNumVideoParties;
		}
	}

	// Invoke warnings in case actual number video/audio/video+audio parties exceeds min video/audio or their sum
	if (m_minNumAudioParties && (GetNumOfAudioOnlyParties() > m_minNumAudioParties))
		SetInternalConfStatus(STATUS_NUM_OF_AUDIO_PARTIES_EXCEEDS_MIN_AUDIO_PARTIES);

	// VNGR-22078 - as written in the SRS - in case of permanent conference,
	// "Reserved Resources for video Participate" should be automatically corrected
	// according to the number of defined dial out participants
	// so the condition of m_minNumVideoParties is relevant only for non permanent conference
	if ((GetNumOfVideoParties() > m_minNumVideoParties))
	{
		if (IsPermanent())
		{
			m_minNumVideoParties = GetNumOfVideoParties();
		}
		else
		{
			if (m_minNumVideoParties)
			{
				SetInternalConfStatus(STATUS_NUM_OF_VIDEO_PARTIES_EXCEEDS_MIN_VIDEO_PARTIES);
			}
		}
	}

	if ((m_minNumAudioParties && m_minNumVideoParties) && (m_numParties > (m_minNumVideoParties+m_minNumAudioParties)))
		SetInternalConfStatus(STATUS_NUM_OF_PARTIES_EXCEEDS_MIN_VIDEO_PLUS_AUDIO_PARTIES);

	return status;
}

STATUS CCommRes::TestLicenseExpiredValidity()
{
	STATUS status = STATUS_OK;
	if (FALSE == ::GetDongleLicenseExpiredValue()  && !IsTemplate())
	{
		TRACEINTO << "Failed - license expire or invalid,can not creat conference";
		status = STATUS_INVALIDE_OR_EXPIRED_LICENSING;
	}
	return status;
}

STATUS CCommRes::TestLicenseSvcConfValidity()
{
	STATUS status = STATUS_OK;
	if (FALSE == ::GetDongleSvcValue()  && !IsTemplate())
	{
		TRACEINTO << "Failed - license SVC invalid,can not creat conference";
		status = STATUS_SVC_CONFERENCE_NOT_SUPPORTED_BY_LICENSING;
	}
	return status;
}
//--------------------------------------------------------------------------
STATUS CCommRes::TestTipCompatibilityValidity()
{
	//Please don't delete the comments in this funcution, maybe will use in the future.

	STATUS status = STATUS_OK;

	if (m_TipCompatibility != eTipCompatibleNone)
	{
		TRACEINTO << "TIP Compatibility mode is set to " << (WORD)m_TipCompatibility << ", so Gathering, Site Names, Layout Indications, Legacy Content and PCM will be disabled";
			//SRS Licensing flexera:If this license is false, TIP compatible conferences cannot be created.
		if (FALSE == ::GetDongleTipInteropValue()  && !IsTemplate())
		{
			TRACEINTO << "Failed - TIP Compatibility mode is not supported by licensing";
			status = STATUS_TIP_COMPATIBILITY_NOT_SUPPORTED_BY_LICENSING;
			return status;
		}
		if (m_videoSession == VIDEO_SWITCH)
		{
			TRACEINTO << "Failed - TIP Compatibility mode is not supported in VSW conference";
			status = STATUS_TIP_COMPATIBILITY_NOT_SUPPORTED_IN_VSW;
		}
		else
		{
			if (m_bGatheringEnabled)
			{
				//TRACEINTO << "CCommRes::TestTipCompatibilityValidity FAILED - Gathering is not supported in TIP Compatibility mode.\n";
				//status = STATUS_GATHERING_NOT_SUPPORTED_IN_TIP_COMPATIBILITY_MODE;
				m_bGatheringEnabled = FALSE;
				TRACEINTO << "Gathering is not supported in TIP Compatibility mode, so we set m_bGatheringEnabled to FALSE";
			}

			if (m_pMessageOverlayInfo && m_pMessageOverlayInfo->GetMessageOn())
			{
				//TRACEINTO << "CCommRes::TestTipCompatibilityValidity FAILED - Message Overlay is not supported in TIP Compatibility mode.\n";
				//status = STATUS_MESSAGE_OVERLAY_NOT_SUPPORTED_IN_TIP_COMPATIBILITY_MODE;
				m_pMessageOverlayInfo->SetMessageOnOff(FALSE);
				TRACEINTO << "Message Overlay is not supported in TIP Compatibility mode, so we set m_MessageOn to FALSE. \n";
			}

			if (m_pSiteNameInfo && m_pSiteNameInfo->GetDisplayMode() != eSiteNameOff)
			{
				//TRACEINTO << "CCommRes::TestTipCompatibilityValidity FAILED - Site Names are not supported in TIP Compatibility mode.\n";
				//status = STATUS_SITE_NAMES_NOT_SUPPORTED_IN_TIP_COMPATIBILITY_MODE;
				m_pSiteNameInfo->SetDisplayMode(eSiteNameOff);
				TRACEINTO << "Site Names are not supported in TIP Compatibility mode, so we set m_SiteNameDisplayMode to eSiteNameOff";
			}

			if (m_ShowContentAsVideo)
			{
				TRACEINTO << "Allow Content to Legacy Endpoints in TIP Compatibility mode \n";
			}
		}
	}
	return status;
}
/////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
STATUS CCommRes::TestMsSvcVideoModeValidity()
{
	//Please don't delete the comments in this funcution, maybe will use in the future.
	if(m_avMcuCascadeVideoMode != eMsSvcResourceOptimize && m_avMcuCascadeVideoMode != eMsSvcVideoOptimize)
		TRACEINTO << "wrong ms svc value \n";

	STATUS status = STATUS_OK;


	return status;
}

//--------------------------------------------------------------------------
void CCommRes::FixPartyUniqueParams(CRsrvParty* pParty)
{
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	if (pParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE)
	{
		if (!pParty->GetVoice())  // ISDN
		{
			if (pParty->GetNetChannelNumber() == 0xFF)
			{
				WORD isdnChannelsNumber = CalculateIsdnChannelsNumber(m_confTransferRate);
				TRACEINTO << "IsdnChannelsNumber:" << isdnChannelsNumber;
				pParty->SetNetChannelNumber(isdnChannelsNumber);
			}

			if (systemCardsBasedMode == eSystemCardsMode_breeze || systemCardsBasedMode == eSystemCardsMode_mpmrx)
			{
				DWORD maxIsdnRateAccordingToFlag = 0;
				CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("MAX_ISDN_BITRATE_MPMX", maxIsdnRateAccordingToFlag);

				CSmallString cstr;
				cstr << maxIsdnRateAccordingToFlag;

				int maxIsdnRate = 0;
				CStringsMaps::GetValue(TRANSFER_RATE_ENUM, maxIsdnRate, (char*)cstr.GetString());
				TRACEINTO << "MAX_ISDN_BITRATE_MPMX:" << maxIsdnRateAccordingToFlag << ", Value:" << maxIsdnRate;

				WORD isdnChannelsNumber = CalculateIsdnChannelsNumber(maxIsdnRate);

				if (isdnChannelsNumber < pParty->GetNetChannelNumber())
				{
					TRACEINTO << "IsdnChannelsNumber:" << isdnChannelsNumber << " - Set number of channels according to system flag";
					pParty->SetNetChannelNumber(isdnChannelsNumber);
				}
			}

			if (!pParty->GetBondingMode1())
				pParty->SetBondingMode1(YES);

			if (pParty->GetMultiRateMode())
				pParty->SetMultiRateMode(NO);

			const Phone* phoneNumber = pParty->GetBondingPhoneNumber();
			if (phoneNumber && phoneNumber->phone_number[0])
			{
				if (::IsValidASCII(phoneNumber->phone_number, strlen(phoneNumber->phone_number), "ASCII_NUMERIC", NO) == NO)
				{
					TRACEINTO << "BondingPhoneNumber:0";
					Phone emptyBondingPhoneNumber;
					emptyBondingPhoneNumber.phone_number[0] = '\0';
					pParty->SetBondingPhoneNumber(emptyBondingPhoneNumber);
				}
			}

			if (pParty->GetAutoDetect())
				pParty->SetAutoDetect(NO);

			BYTE restrict = pParty->GetRestrict();
			if (restrict != 28)
				pParty->SetRestrict(28); // deristrict
		}
	}
	else
	{
		pParty->SetNetChannelNumber(0);
	}

	// it can happen that user has saved in Address Book participant with Resolution bigger that current system settings
	// then we need to update party resolution field within the system max resolution
	EVideoResolutionType maxSystemResolution = CResRsrcCalculator::GetMaxCPResolutionType(systemCardsBasedMode);
	BYTE maxResolution = pParty->GetMaxResolution();
	if (maxSystemResolution < maxResolution && maxResolution != eAuto_Res)
	{
		pParty->SetMaxResolution(maxSystemResolution);
		TRACEINTO << "maxSystemResolution:" << maxSystemResolution << ", maxPartyResolution:" << maxResolution << " - Max party resolution bigger than system resolution";
	}
	// if there is "sip:" prefix in sipPartyAddress, cut it.
	pParty->StripSipPartyAddressPrefix();
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestPartyUniqueParamsValidity(CRsrvParty* pParty)
{
	PASSERT_AND_RETURN_VALUE(!pParty, STATUS_PARTY_DOES_NOT_EXIST);

	TRACEINTO << "PartyName:" << pParty->GetName();

	const char* partyname = pParty->GetName();
	TRACECOND_AND_RETURN_VALUE(partyname[0] == '\0', "Failed, Illegal party name", STATUS_ILLEGAL_PARTY_NAME);
	TRACECOND_AND_RETURN_VALUE(strstr(partyname, ","), "Failed, The character ',' is not allowed in party name", STATUS_INVALID_CHARACTER_IN_PARTY_NAME);
	TRACECOND_AND_RETURN_VALUE(strstr(partyname, ";"), "Failed, The character ';' is not allowed in party name", STATUS_INVALID_CHARACTER_IN_PARTY_NAME);

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	// system.cfg flags tests
	BOOL IsH263;
	sysConfig->GetBOOLDataByKey("H263", IsH263);
	if (IsH263 == FALSE && pParty->GetVideoProtocol() == VIDEO_PROTOCOL_H263) {
		TRACECOND_AND_RETURN_VALUE(1, "Failed, Unexpected video protocol VIDEO_PROTOCOL_H263", STATUS_UNEXPECTED_VIDEO_PROTOCOL);
	}

	BOOL IsH264;
	sysConfig->GetBOOLDataByKey("H264", IsH264);
	if (IsH264 == FALSE && pParty->GetVideoProtocol() == VIDEO_PROTOCOL_H264) {
		TRACECOND_AND_RETURN_VALUE(1, "Failed, Unexpected video protocol VIDEO_PROTOCOL_H264", STATUS_UNEXPECTED_VIDEO_PROTOCOL);
	}

	// VNGFE-6742, remove the IP address when there are valid E164 number. So RMX could dial out the EP successfully even the IP address is changed.
	BOOL bRemoveIpIfNumberExist = YES;
	sysConfig->GetBOOLDataByKey("REMOVE_IP_IF_NUMBER_EXISTS", bRemoveIpIfNumberExist);

	// For H323 participant,  this flag is YES when 1) GK is defined  2)Alias name is valid
	// For SIP participant, this flag is YES when 1)SIP proxy is defined 2)URI address is valid.
	BOOL bValidAliasAndConfiguration = NO;

	// net service name
	const char* serviceProviderName = pParty->GetServiceProviderName();
	BYTE bIsIpInterfaceType = pParty->IsIpNetInterfaceType();
	BYTE interfaceType = pParty->GetNetInterfaceType();

	if (GetIsTipCompatibleVideo())
	{
		BYTE videoProtocol = pParty->GetVideoProtocol();

		if (videoProtocol != VIDEO_PROTOCOL_H264 && videoProtocol != AUTO) {
			TRACECOND_AND_RETURN_VALUE(1, "Failed, Staus:STATUS_PARTY_VIDEO_PROTOCOL_IS_NOT_TIP_COMPATIBLE", STATUS_PARTY_VIDEO_PROTOCOL_IS_NOT_TIP_COMPATIBLE);
		}

		DWORD partyVideoRate = pParty->GetVideoRate();

		if (partyVideoRate != 0xFFFFFFFF)
		{
			DWORD minTIPlineRate = GetSystemCfgFlagInt<DWORD>(CFG_KEY_MIN_TIP_COMP_LINE_RATE);

			if (minTIPlineRate == 0)
				minTIPlineRate = 1024;

			if (partyVideoRate < minTIPlineRate) {
				TRACECOND_AND_RETURN_VALUE(1, "Failed, Staus:STATUS_PARTY_RATE_BELLOW_MIN_TIP_COMPATIBILITY_LINE_RATE", STATUS_PARTY_RATE_BELLOW_MIN_TIP_COMPATIBILITY_LINE_RATE);
			}
		}

		BYTE maxRes = pParty->GetMaxResolution();

		if (eCIF_Res == maxRes || eSD_Res == maxRes) {
			TRACECOND_AND_RETURN_VALUE(1, "Failed, Staus:STATUS_PARTY_VIDEO_RESOLUTION_IS_NOT_TIP_COMPATIBLE", STATUS_PARTY_VIDEO_RESOLUTION_IS_NOT_TIP_COMPATIBLE);
		}
	}

	// Begin ;IP address validity check
	if (bIsIpInterfaceType)  // 323_INTERFACE_TYPE or SIP_INTERFACE_TYPE
	{
		const char* party_serv_name = pParty->GetServiceProviderName();
		const char* conf_serv_name  = GetServiceNameForMinParties();
		if ((!party_serv_name || !strlen(party_serv_name)) && conf_serv_name && strlen(conf_serv_name))
		{
			party_serv_name = conf_serv_name;
			TRACEINTO << "Using conference service name, ServiceName:" << conf_serv_name;
		}

		CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
		CConfIpParameters* pServiceParams = pIpServiceListManager->GetRelevantService(party_serv_name, pParty->GetNetInterfaceType());
		if (pServiceParams == NULL) {
			TRACECOND_AND_RETURN_VALUE(1, "Failed, Staus:STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS", STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		}

		TRACEINTO << "ServiceName:" << pServiceParams->GetServiceName();

		// RTV
		if (pParty->GetVideoProtocol() == VIDEO_PROTOCOL_RTV && interfaceType != SIP_INTERFACE_TYPE) {
			TRACECOND_AND_RETURN_VALUE(1, "Failed, Unexpected video protocol VIDEO_PROTOCOL_RTV", STATUS_UNEXPECTED_VIDEO_PROTOCOL);
		}

		if (pParty->GetVideoProtocol() == VIDEO_PROTOCOL_RTV &&
		    interfaceType == SIP_INTERFACE_TYPE &&
		    pServiceParams->GetConfigurationOfSipServers() &&
		    pServiceParams->GetSipServerType() != eSipServer_ms) {
			TRACECOND_AND_RETURN_VALUE(1, "Failed, Unexpected video protocol VIDEO_PROTOCOL_RTV", STATUS_UNEXPECTED_VIDEO_PROTOCOL);
		}

		// In case service was not configured, fix the Default Service Name as the service name
		pParty->SetServiceProviderName((const char*)pServiceParams->GetServiceName());

		if (pParty && pParty->GetNetInterfaceType() == H323_INTERFACE_TYPE && pParty->GetConnectionType() == DIAL_OUT &&
		    GetCascadedLinksNumber() > 0 && (GetCascadedLinksNumber() != pParty->GetCascadedLinksNumber()) && (pParty->GetCascadedLinksNumber() > 1))
		{
			pParty->SetCascadedLinksNumber(GetCascadedLinksNumber());

			TRACEINTO << "CascadedLinksNumber:" << (int)GetCascadedLinksNumber() << ", CascadedLinksNumberUpdated:" << (int)pParty->GetCascadedLinksNumber();
		}

		// Check only for DIAL OUT and defined DIAL IN
		if ((pParty->GetConnectionType() == DIAL_OUT) || ((pParty->GetConnectionType() == DIAL_IN) && !pParty->IsUndefinedParty()))
		{
			mcTransportAddress partyIpAddr = pParty->GetIpAddress();
			// H323 Interface
			if (pParty->GetNetInterfaceType() == H323_INTERFACE_TYPE)
			{
				// if service has no GK must has a valid ip
				// IpV6
				DWORD isGkExt = pServiceParams->isGKExternal();
				int isIpNonValid = ::isIpTaNonValid(&partyIpAddr);
				int isIpZero = ::isApiTaNull(&partyIpAddr);
				if (partyIpAddr.ipVersion == eIpVersion6)
				{
					char tempName[64];
					memset(&tempName, '\0', IPV6_ADDRESS_LEN);
					ipToString(partyIpAddr, tempName, 1);
					int scope = ::getScopeId(tempName);
					TRACEINTO << "SipScopeName:" << scope;
					if (scope == (int)eScopeIdLink)
						isIpNonValid = TRUE;
				}

				eIpType typeOfNetworkService = pServiceParams->GetIPAddressTypesInService();
				if (!isIpZero && !isIpNonValid &&
				    ((partyIpAddr.ipVersion == eIpVersion6 && typeOfNetworkService == eIpType_IpV4) ||
				     (partyIpAddr.ipVersion == eIpVersion4 && typeOfNetworkService == eIpType_IpV6)))
				{
					TRACESTRFUNC(eLevelError) << "NetworkServiceIpType:" << typeOfNetworkService << ", IsIpZero:" << isIpZero << " - Failed, Statues:STATUS_BAD_IP_ADDRESS_VERSION";
					return STATUS_BAD_IP_ADDRESS_VERSION;
				}

				if (isGkExt == FALSE && (isIpNonValid || isIpZero)) {
					TRACECOND_AND_RETURN_VALUE(1, "Failed, Staus:STATUS_IP_ADDRESS_NOT_VALID", STATUS_IP_ADDRESS_NOT_VALID);
				}

				// if party has invalid ip alias must be non empty and valid
				if ((isIpNonValid || isIpZero) && strlen(pParty->GetH323PartyAlias()) == 0) {
					TRACECOND_AND_RETURN_VALUE(1, "Failed, Staus:STATUS_EMPTY_PARTY_ALIAS_NAME", STATUS_EMPTY_PARTY_ALIAS_NAME);
				}

				if (strlen(pParty->GetH323PartyAlias()) > XML_API_MAX_PARTY_ALIAS_LEN) {
					TRACECOND_AND_RETURN_VALUE(1, "Failed, Staus:STATUS_ALIAS_NAME_TOO_LONG", STATUS_ALIAS_NAME_TOO_LONG);
				}

				// if alias not empty check validity.
				if (strlen(pParty->GetH323PartyAlias()) != 0)
				{
					CH323Alias* partyAlias = new CH323Alias;
					partyAlias->SetAliasName(pParty->GetH323PartyAlias());
					partyAlias->SetAliasType(pParty->GetH323PartyAliasType());
					DWORD aliasStatus = partyAlias->TestValidity();

					POBJDELETE(partyAlias);

					if (aliasStatus != STATUS_OK)
						return aliasStatus;

					if (isGkExt)
						bValidAliasAndConfiguration = YES;
				}
			}
			// SIP Interface
			else if (pParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE)
			{
				BYTE bValidIP = (!::isApiTaNull(&partyIpAddr) && !::isIpTaNonValid(&partyIpAddr));
				// if ip is valid
				if (bValidIP)
				{
					PTRACE(eLevelInfoNormal, "CCommRes::TestPartyUniqueParamsValidity - Sip valid ip");
					if (partyIpAddr.ipVersion == eIpVersion6)
					{
						char tempName[64];
						memset(&tempName, '\0', IPV6_ADDRESS_LEN);
						ipToString(partyIpAddr, tempName, 1);
						int scope = ::getScopeId(tempName);
						PTRACE2INT(eLevelInfoNormal, "CCommRes::TestPartyUniqueParamsValidity - Sip valid ip-v6 scope by name is ", scope);
						if (scope == (int)eScopeIdLink)
						{
							TRACESTRFUNC(eLevelError) << "Failed, Status:STATUS_IP_ADDRESS_NOT_VALID";
							return STATUS_IP_ADDRESS_NOT_VALID;
						}
					}

					// valid ip dial direct
					WORD port = 0;
					const char* strSipAddress = pParty->GetSipPartyAddress();
					const char* strColon = strstr(strSipAddress, ":");
					if (strColon)
					{
						const char* strPort = strColon+1;
						port = ::IsValidPort(strPort);
					}

					if (port)
						pParty->SetCallSignallingPort(port);
				}

				// if ip is invalid
				if (!bValidIP || bRemoveIpIfNumberExist)
				{
					// check first if we have a URI address of format xxx@ip.
					// if not - we must have proxy
					STATUS status = CheckSipUriValidity(pParty, pServiceParams);
					if (STATUS_OK != status)
					{
						if (!bValidIP)                                               // the IP address and URI are both invalid,return.
							return status;
					}
					else                                                           // SIP URI is valid
					{
						if (pServiceParams->GetSipProxyStatus() != eServerStatusOff) // SIP proxy configured
							bValidAliasAndConfiguration = YES;
					}
				}                                                                // end of if ip is invalid
			}                                                                  // end of SIP interface

			// check if IP adress is ocuppied (VNGR-3598)
			CRsrvParty* pCurrParty = 0;
			mcTransportAddress partyTrAddr = pParty->GetIpAddress();
			APIU8 nullIP[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
			BYTE eqIp = 0;
			for (int i = 0; i < (int)m_numParties; i++)
			{
				pCurrParty = m_pParty[i];

				// Make sure IP is not the smae and different from 0 !
				mcTransportAddress currPartyTrAddr = pCurrParty->GetIpAddress();
				if (pParty != pCurrParty)
				{
					if (partyTrAddr.ipVersion == currPartyTrAddr.ipVersion)
					{
						if (partyTrAddr.ipVersion == eIpVersion4)
						{
							if (partyTrAddr.addr.v4.ip == currPartyTrAddr.addr.v4.ip && partyTrAddr.addr.v4.ip != 0)
								eqIp = 1;
						}
						else
						{
							if (!memcmp(partyTrAddr.addr.v6.ip, currPartyTrAddr.addr.v6.ip, IPV6_ADDRESS_BYTES_LEN)
							    && !memcmp(partyTrAddr.addr.v6.ip, nullIP, IPV6_ADDRESS_BYTES_LEN))
								eqIp = 1;
						}

						if (eqIp)
						{
							this->SetInternalConfStatus(STATUS_PARTY_IP_ALREADY_EXISTS);
						}
					}
				}
			}
		}

		// end of Check only for DIAL OUT and defined DIAL IN

		if (bRemoveIpIfNumberExist && bValidAliasAndConfiguration && !pParty->GetRecordingLinkParty())
			RemoveIPAddress(pParty);

		// block case of h.323 party with sip only service and vice versa
		CONF_IP_PARAMS_S* serviceParams = pServiceParams->GetConfIpParamsStruct();
		TRACEINTO << "ServiceType:" << serviceParams->service_protocol_type << ", PartyInterfaceType:" << (int)pParty->GetNetInterfaceType();
		if ((serviceParams->service_protocol_type == eIPProtocolType_SIP && pParty->GetNetInterfaceType() == H323_INTERFACE_TYPE) ||
		    (serviceParams->service_protocol_type == eIPProtocolType_H323 && pParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE))
			return STATUS_PARTY_INTERFACE_TYPE_DOES_NOT_MATCH_SERVICE_TYPE;
	}
	else if (pParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE)
	{
		// Check the Phone validity
		if (pParty->GetConnectionType() == DIAL_OUT)
		{
			std::string phoneStr  = "12345";
			const char* phoneStr1 = pParty->GetPhoneNumber();
			phoneStr = pParty->GetPhoneNumber();
			if (0 == phoneStr.size())
				return STATUS_ILLEGAL_PHONE_LENGTH;

			if (::IsValidASCII(phoneStr.c_str(), phoneStr.size(), "ASCII_NUMERIC", NO) == NO)
			{
				BYTE isLegal = YES;
				WORD sizeStr = phoneStr.size();
				for (WORD char_index = 0; char_index < sizeStr; char_index++)
				{
					if (phoneStr[char_index] < 48 || phoneStr[char_index] > 57)
					{
						BYTE faulty_char = phoneStr[char_index];
						if (faulty_char != '#' || char_index != (sizeStr-1))    // VNGR-8791
						{
							isLegal = NO;
							break;
						}
					}
				}

				if (NO == isLegal)
					return STATUS_PHONE_MUST_BE_NUMERIC;
			}
		}
		else
		{
			if (!(pParty->IsUndefinedParty()))
			{
				TRACEINTO << "Failed, ISDN defined dial in is not allowed";   // in V3
				return STATUS_NET_SERVICE_NOT_FOUND;
			}
		}

		const char* serviceName = pParty->GetServiceProviderName();
		CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();

		if (0 == pConfPartyProcess->numberOfIsdnServices() ||      // No services at all
		    !pConfPartyProcess->GetIsdnService(serviceName))       // Service name is not found
		{
			TRACEINTO << "Failed, ISDN Net Service not found";
			return STATUS_NET_SERVICE_NOT_FOUND;
		}
		else
		{
			// In case service was not configured, fix the Default Service Name as the service name
			const char* serviceNameDB = (char*)(pConfPartyProcess->GetIsdnService(serviceName))->serviceName;
			pParty->SetServiceProviderName(serviceNameDB);
		}

		if (!pParty->GetVoice())
		{
			switch (pParty->GetNetChannelNumber())
			{
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 8:
				case 12:
				case 18:
				case 23:
				case 24:
				case 30:
				{
					break;
				}

				default:
				{
					TRACEINTO << "Failed, Illegal bitrate for bonding";
					return STATUS_INVALID_PARTY_H320_BITRATE;
				}
			} // switch
		}
	}

	if (!IsTemplate() && !IsMeetingRoom() && !GetIsHDVSW())
	{
		eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
		if (systemCardsBasedMode == eSystemCardsMode_mpm)
		{
			DWORD partyVideoRate = pParty->GetVideoRate();
			if ((0xFFFFFFFF != partyVideoRate) && (partyVideoRate > 1856))
			{
				TRACEINTO << "In MPM Cards Based system the max call rate in CP conference is 1920";
				pParty->SetVideoRate(1856);
			}
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckSipUriValidity(CRsrvParty* pParty,CConfIpParameters* pServiceParams)
{
	PASSERT_AND_RETURN_VALUE(!pParty,STATUS_PARTY_DOES_NOT_EXIST);
	PASSERT_AND_RETURN_VALUE(!pServiceParams,STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);


	// check first if we have a URI address of format xxx@ip.
	// if not - we must have proxy
	BYTE bIsUriWithIp   = NO;
	BYTE bValidUri      = NO;

	WORD sipAddressType = pParty->GetSipPartyAddressType();
	if (sipAddressType == PARTY_SIP_SIPURI_ID_TYPE || sipAddressType == PARTY_SIP_TELURL_ID_TYPE)
	{

		// user@host - when host can be an ip or a domain name
		const char* strUriAddress = pParty->GetSipPartyAddress();
		if (strUriAddress && strUriAddress[0])
		{
			const char* strHost = strstr(strUriAddress, "@");
			strHost = strHost ? strHost+1 : NULL;
			if (strHost) // if we have a host
			{
				// even if after @ the domain isn't ip and proxy off we can send this message through via field
				bIsUriWithIp = YES;
				bValidUri    = YES;

				mcTransportAddress trAddr;
				memset(&trAddr, 0, sizeof(mcTransportAddress));
				char strTransportIp[IPV6_ADDRESS_LEN];
				strncpy(strTransportIp, strHost, sizeof(strTransportIp)-1);
				strTransportIp[IPV6_ADDRESS_LEN - 1] = '\0';
				TRACEINTO << "TransportIP:" << strTransportIp << ", UriAddress:" << strUriAddress;
				::stringToIp(&trAddr, strTransportIp);
				if (!::isApiTaNull(&trAddr) && !::isIpTaNonValid(&trAddr))
				{
					if (trAddr.ipVersion == eIpVersion6)
					{
						int scopeNum = ::getScopeId(strTransportIp);
						if (scopeNum == (int)eScopeIdLink)
							return STATUS_IP_ADDRESS_NOT_VALID;
					}
				}
			}
			else // / complete the host from the service domain
			{
				// add domain only in case if does not explicit IP address
				if (::IsValidIPAddress(strUriAddress) == YES)
				{
					bIsUriWithIp = YES;
					bValidUri    = YES;
					mcTransportAddress trAddr;
					memset(&trAddr, 0, sizeof(mcTransportAddress));
					char strTransportIp[IPV6_ADDRESS_LEN];
					strncpy(strTransportIp, strUriAddress, sizeof(strTransportIp)-1);
					strTransportIp[IPV6_ADDRESS_LEN - 1] = '\0';
					TRACEINTO << "TransportIP:" << strTransportIp;
					::stringToIp(&trAddr, strTransportIp);
					if (!::isApiTaNull(&trAddr) && !::isIpTaNonValid(&trAddr))
					{
						if (trAddr.ipVersion == eIpVersion6)
						{
							int scopeNum = ::getScopeId(strTransportIp);
							if (scopeNum == (int)eScopeIdLink)
								return STATUS_IP_ADDRESS_NOT_VALID;
						}
					}
				}
				else
				{
					CSmallString hostName = "";
					switch ((DWORD)pServiceParams->GetConfigurationOfSipServers())
					{
						// Andrey: I leaved the default here because it was as is before
						// expand the behaviour according to bug 17637
						default:
						case eConfSipServerManually:
						{
							hostName =  pServiceParams->GetRegistrarDomainName();
							if (hostName.IsEmpty())
								hostName = pServiceParams->GetSipProxyName();
							break;
						}

						case eConfSipServerAuto:
						{
							hostName = pServiceParams->GetLocalDomainName();
							break;
						}

						case STATUS_UNKNOWN_CONFIGURATION_FOR_SIP_SERVERS:
							break;
					} // switch ConfigurationOfSIPServers

					if (hostName.GetStringLength() > 0)
					{
						CSmallString newAddress = strUriAddress;
						newAddress += "@";
						newAddress += hostName;
						pParty->SetSipPartyAddress(newAddress.GetString());
						bValidUri = YES;
					}
				} // else URI without domain name
			} // URI without @
		} // retrieved some address
	}

	BYTE bNoProxy = (pServiceParams->GetSipProxyStatus() == eServerStatusOff);

	// a valid adress is : direct IP or name@host with a proxy.
	if (bIsUriWithIp == NO && (bValidUri == NO || bNoProxy))
		return STATUS_IP_ADDRESS_NOT_VALID;

	return STATUS_OK;
}

//-------------------------------------------------------------------------
//VNGFE-6742, remove the IP address when there are valid E164 numer. So RMX could dial out the EP successfully even the IP address is changed.
void CCommRes::RemoveIPAddress(CRsrvParty* pParty)
{
	PASSERT_AND_RETURN(!pParty);

	if(pParty->GetConnectionType() == DIAL_OUT)
	{
		mcTransportAddress nullIpAddr;
		memset(&nullIpAddr, 0, sizeof(mcTransportAddress));

		switch(pParty->GetNetInterfaceType())
		{
			case H323_INTERFACE_TYPE:
			{
				switch(pParty->GetH323PartyAliasType())
				{
					case PARTY_H323_ALIAS_E164_TYPE:
					case PARTY_H323_ALIAS_H323_ID_TYPE:
					case PARTY_H323_ALIAS_PARTY_NUMBER_TYPE:
					case PARTY_H323_ALIAS_EMAIL_ID_TYPE:
						pParty->SetIpAddress(nullIpAddr);
						TRACEINTO << "PartyId:" << pParty->GetPartyId() << " - Reset IP address for H323 party";
						break;
				}
				break;
			}
			case SIP_INTERFACE_TYPE:
			{
				switch(pParty->GetSipPartyAddressType())
				{
					case PARTY_SIP_SIPURI_ID_TYPE:
					case PARTY_SIP_TELURL_ID_TYPE:
						pParty->SetIpAddress(nullIpAddr);
						TRACEINTO << "PartyId:" << pParty->GetPartyId() << " - Reset IP address for SIP party";
						break;
				}
				break;
			}
		}
	}
}


//--------------------------------------------------------------------------
STATUS CCommRes::CheckReservRangeValidity(BYTE& errorCode)
{
	WORD status = STATUS_OK;
	errorCode = 0;

	if (GetIsTipCompatibleVideo())
	{
		DWORD minTIPlineRate = GetSystemCfgFlagInt<DWORD>(CFG_KEY_MIN_TIP_COMP_LINE_RATE);

		if (minTIPlineRate == 0)
			minTIPlineRate = 1024;

		CCapSetInfo lCapInfo(_H264, (WORD)m_confTransferRate);
		DWORD       h323Rate = lCapInfo.TranslateReservationRateToIpRate(m_confTransferRate);
		TRACEINTO << "This is a TIP conference, min line rate in SystemCfg:" << minTIPlineRate << ", h323Rate:" << h323Rate;
		if (h323Rate < minTIPlineRate)
			return STATUS_CONF_RATE_BELLOW_MIN_TIP_COMPATIBILITY_LINE_RATE;
	}

	switch (m_confTransferRate)
	{
		case Xfer_64:
		case Xfer_96:
		case Xfer_128:
		case Xfer_192:
		case Xfer_256:
		case Xfer_320:
		case Xfer_384:
		case Xfer_512:
		case Xfer_768:
		case Xfer_832:
		case Xfer_1024:
		case Xfer_1152:
		case Xfer_1280:
		case Xfer_1472:
		case Xfer_1536:
		case Xfer_1728:
		case Xfer_1920:
		case Xfer_2048:
		case Xfer_2560:
		case Xfer_3072:
		case Xfer_3584:
		case Xfer_4096:
			break;

		case Xfer_6144:
		case Xfer_8192:
		{
			 //for non-CP mode, we support up to 6144K, no matter card type
			 if(CONTINUOUS_PRESENCE != m_videoSession)
			 {
			   m_confTransferRate = Xfer_6144;
			 }
			//for CP mode, MPMRX/Ninja cards supports upto 6144K, others up to 4096K
			 else
			 {
			   if(IsFeatureSupportedBySystem(eFeatureLineRate_6M))
			   {
				   m_confTransferRate = Xfer_6144;
			   }
			   else
			   {
				   m_confTransferRate = Xfer_4096;
			   }
			 }
			break;
		}
		default:
		{
			errorCode = 2;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	if (AUTO != m_audioRate)
	{
		errorCode = 3;
		return STATUS_OUT_OF_RANGE;
	}

	switch (m_videoSession)
	{
		case VIDEO_TRANSCODING:
		case CONTINUOUS_PRESENCE:
		case VIDEO_SESSION_COP:
		{
			break;
		}
		case VIDEO_SWITCH:
		{
			if (!m_HD)
			{
				errorCode = 41;
				return STATUS_OUT_OF_RANGE;
			}

			break;
		}
		default:
		{
			errorCode = 4;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	if (m_contPresScreenNumber > TWO_TOP_PLUS_EIGHT)
	{
		errorCode = 6;
		return STATUS_OUT_OF_RANGE;
	}

	if (m_numRsrvVideoSource > MAX_VIDEO_LAYOUT_NUMBER)
	{
		errorCode = 7;
		return STATUS_OUT_OF_RANGE;
	}

	if (m_videoPictureFormat >= NUMBER_OF_H263_FORMATS
			&& V_Qcif != m_videoPictureFormat
			&& V_Cif != m_videoPictureFormat
			&& AUTO != m_videoPictureFormat)
	{
		errorCode = 8;
		return STATUS_OUT_OF_RANGE;
	}

	switch (m_CIFframeRate)
	{
		case V_1_29_97:
		case V_2_29_97:
		case V_3_29_97:
		case V_4_29_97:
		case AUTO:
		{
			break;
		}
		default:
		{
			errorCode = 9;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	switch (m_QCIFframeRate)
	{
		case V_1_29_97:
		case V_2_29_97:
		case V_3_29_97:
		case V_4_29_97:
		case AUTO:
		{
			break;
		}
		default:
		{
			errorCode = 10;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	switch (m_LSDRate)
	{
		case LSD_6400:
		{
			break;
		}
		default:
		{
			errorCode = 11;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	if (m_audioMixDepth > 5)
	{
		errorCode = 13;
		return STATUS_OUT_OF_RANGE;
	}

	switch (m_videoProtocol)
	{
		case AUTO:
		{
			break;
		}
		default:
		{
			errorCode = 14;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	if ((m_numUndefParties > 40) || ((m_numUndefParties > m_max_parties)))
	{
		errorCode = 16;
		return STATUS_MINIMUM_NUMBER_OF_PARTICIPANTS_EXCEEDS_THE_DEFINED_MAXIMUM;
	}

	if (m_numParties > MAX_PARTIES_IN_CONF)
	{
		errorCode = 17;
		return STATUS_OUT_OF_RANGE;
	}

	// Call Generator - temp
	if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
	{
		if ((m_max_parties > 160 /*80*/ || m_max_parties < 2) && m_max_parties != 0xFFFF)
		{
			errorCode = 19;
			return STATUS_OUT_OF_RANGE;
		}
	}
	else
	{
		if (m_max_parties != 0xFFFF && (m_max_parties > 80 || m_max_parties < 2))
		{
			errorCode = 19;
			return STATUS_OUT_OF_RANGE;
		}
	}

	if (m_meetingRoomState != MEETING_ROOM_PASSIVE_STATE
			&& m_meetingRoomState != MEETING_ROOM_ACTIVE_STATE)
	{
		errorCode = 20;
		return STATUS_OUT_OF_RANGE;
	}

	if (m_dualVideoMode != eDualModeNone)
	{
		m_dualVideoMode = eDualModeNone;
	}

	switch (m_cascadeMode)
	{
		case CASCADE_MODE_MASTER:
		case CASCADE_MODE_SLAVE:
		case CASCADE_MODE_AUTO:
			if (m_cascadedLinksNumber > MAX_CASCADED_LINKS_NUMBER)
			{
				errorCode = 25;
				return STATUS_OUT_OF_RANGE;
			}
			break;

		case CASCADE_MODE_NONE:
			if (m_cascadedLinksNumber > 1)
			{
				errorCode = 26;
				return STATUS_OUT_OF_RANGE;
			}
			break;

		default:
			errorCode = 27;
			return STATUS_OUT_OF_RANGE;
	}

	if (m_ind > MAX_PARTIES_IN_CONF) // Zoe: I chacked this with Ron L.
	{
		errorCode = 28;
		return STATUS_OUT_OF_RANGE;
	}

	if (m_ind_rsrv_vid_layout > MAX_VIDEO_LAYOUT_NUMBER)
	{
		errorCode = 30;
		return STATUS_OUT_OF_RANGE;
	}

	if (m_partyIdCounter > HALF_MAX_DWORD)
	{
		errorCode = 31;
		return STATUS_OUT_OF_RANGE;
	}

	switch (m_confType)
	{
		case CONF_TYPE_STANDARD:
		{
			break;
		}
		default:
		{
			errorCode = 32;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	switch (m_media)
	{
		case AUDIO_MEDIA:
		case VIDEO_MEDIA:
		case DATA_MEDIA:
		case AUDIO_VIDEO_MEDIA:
		case AUDIO_DATA_MEDIA:
		case VIDEO_DATA_MEDIA:
		case ALL_MEDIA:
		{
			break;
		}
		default:
		{
			errorCode = 33;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	switch (m_network)
	{
		case NETWORK_H323:
		case NETWORK_H320_H323:
		{
			break;
		}
		default:
		{
			errorCode = 34;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	if (m_duration.m_hour > 999999)
	{
		errorCode = 37;
		return STATUS_OUT_OF_RANGE;
	}

	if (m_duration.m_min > 59)
	{
		errorCode = 38;
		return STATUS_OUT_OF_RANGE;
	}

	if (m_pLectureMode->GetLectureTimeInterval() > 99)
	{
		errorCode = 39;
		return STATUS_OUT_OF_RANGE;
	}

	if (m_pVisualEffectsInfo->GetBackgroundImageID() > 2)
	{
		errorCode = 40;
		return STATUS_OUT_OF_RANGE;
	}

	switch (m_EnterpriseMode)
	{
		case eGraphics:
		case eHiResGraphics:
		case eLiveVideo:
		case eCustomizedRate:
		break;

		default:
		{
			errorCode = 41;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	if (m_EnterpriseMode == eCustomizedRate)
	{
		int fixedRateValue = CCapSetInfo::TranslateReservationRateToIpRate(m_EnterpriseModeFixedRate);
		int lineRateValue  = CCapSetInfo::TranslateReservationRateToIpRate(m_confTransferRate);

		//threshold is 2/3 of conference rate
		bool failed = 3 * fixedRateValue > 2 * lineRateValue;

		if (failed)
		{
			errorCode = 42;
			return STATUS_OUT_OF_RANGE;
		}
	}

	switch (m_videoQuality)
	{
		case eVideoQualityAuto:
		case eVideoQualityMotion:
		case eVideoQualitySharpness:
		{
			break;
		}
		default:
		{
			errorCode = 43;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	switch (m_isTelePresenceMode)
	{
		case TRUE:
		case FALSE:
		{
			break;
		}
		default:
		{
			errorCode = 44;
			PASSERT(m_isTelePresenceMode);
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	switch (m_HDResolution)
	{
		case eHD1080p60Res:
		case eHD720Res:
		case eHD1080Res:
		case eHD720p60Res:
		case eSDRes:
		case eH264Res:
		case eH263Res:
		case eH261Res:
		{
			break;
		}
		default:
		{
			errorCode = 45;
			PASSERT(m_HDResolution);
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	switch (m_PresentationProtocol)
	{
		case eH263Fix:
		case ePresentationAuto:
		case eH264Fix:
		case eH264Dynamic:
		{
			break;
		}
		default:
		{
			errorCode = 46;
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	switch (m_H264VSWHighProfilePreference)
	{
		case eAlways:
		case eBaseLineOnly:
		{
			break;
		}
		default:
		{
			errorCode = 47;
			PASSERT(m_H264VSWHighProfilePreference);
			return STATUS_OUT_OF_RANGE;
		}
	} // switch

	CRsrvParty* pParty = GetFirstParty();
	while (pParty != NULL)
	{
		status = pParty->CheckReservRangeValidity(errorCode);
		if (STATUS_OK != status)
			return status;

		pParty = GetNextParty();
	}

	if (m_videoSession == VIDEO_SESSION_COP)
	{
		status = m_pCopConfigurationList->CheckReservRangeValidity(errorCode);
		if (STATUS_OK != status)
			return status;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckIvrServiceValidity()
{
	STATUS status = STATUS_OK;

	CAvMsgStruct*      pAvMsgStruct      = GetpAvMsgStruct();
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();

	if ((NULL == pAvMsgStruct->GetAvMsgServiceName()) || (0 == strcmp(pAvMsgStruct->GetAvMsgServiceName(), "\0")))
	{
		// Set the default IVR service for the conf, in case no service has been set
		if ((!(GetEntryQ()) && (!(pAVmsgServiceList->GetDefaultIVRName()) || (0 == strcmp(pAVmsgServiceList->GetDefaultIVRName(), "\0"))))
				|| ((GetEntryQ()) && (!(pAVmsgServiceList->GetDefaultEQName()) || (0 == strcmp(pAVmsgServiceList->GetDefaultEQName(), "\0")))))
		{
			// REJECT!!!
			TRACEINTO << "Failed, No suitable service in IVR service list";
			if (GetEntryQ())  // EQ
				status = STATUS_NO_EQ_SERVICE_IN_LIST;
			else
				status = STATUS_NO_IVR_SERVICE_IN_LIST;
		}
		else
		{
			pAvMsgStruct->SetAttendedWelcome(CONF_IVR);         // Conf with IVR

			char defaultIVRService[AV_MSG_SERVICE_NAME_LEN];
		memset(defaultIVRService, 0, AV_MSG_SERVICE_NAME_LEN);

			if (GetEntryQ())      // EQ
				strcpy_safe(defaultIVRService, ::GetpAVmsgServList()->GetDefaultEQName());
			else        // Not EQ
				strcpy_safe(defaultIVRService, ::GetpAVmsgServList()->GetDefaultIVRName());

			pAvMsgStruct->SetAvMsgServiceName(defaultIVRService);
		}
	}
	else    // IVR service is set in conf
	{
		int index = pAVmsgServiceList->FindAVmsgServ(pAvMsgStruct->GetAvMsgServiceName());
		if (NOT_FIND == index)
		{
			// REJECT!!!
			TRACEINTO << "ServiceName:" << pAvMsgStruct->GetAvMsgServiceName() << " - Failed, IVR Service does not appear in list";
			status = STATUS_IVR_SERVICE_NAME_DOES_NOT_EXISTS;
		}
		// Romem klocwork
		else if (index >= MAX_IVR_SERV_IN_LIST)
		{
			// REJECT!!!
			TRACEINTO << "ServiceName:" << pAvMsgStruct->GetAvMsgServiceName() << " - Failed, Index of IVR service exceeds Service array size";
			status = STATUS_MAX_NUMBER_OF_IVR_SERVICES_EXCEEDED;
		}
		else
		{
			const CIVRService* pIVRService = pAVmsgServiceList->m_pAVmsgService[index]->GetIVRService();
			if (pIVRService)
				if (((GetEntryQ()) && !(pIVRService->GetEntryQueueService())) ||
						(!(GetEntryQ()) && (pIVRService->GetEntryQueueService())))
				{
					// REJECT!!!
					TRACEINTO << "ServiceName:" << pIVRService->GetName() << " - Failed, Incorrect service type";
					status = STATUS_INCOMPATIBLE_IVR_SERVICE;
				}
		}
	}
	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckPasswordsValidity()
{
	int status = STATUS_OK;

	// Entry Password
	const char* entryPassword = GetEntryPassword();

	// Check if entry password is set
	DWORD entryPasswordMinLength;
	DWORD entryPasswordMaxLength;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetDWORDDataByKey("NUMERIC_CONF_PASS_MIN_LEN", entryPasswordMinLength);
	sysConfig->GetDWORDDataByKey("NUMERIC_CONF_PASS_MAX_LEN", entryPasswordMaxLength);
	status = CheckPasswordValidity(entryPassword, entryPasswordMinLength, entryPasswordMaxLength);
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed, Conference password not valid", status);

	DWORD dwMaxRepeatedDigits;
	sysConfig->GetDWORDDataByKey("MAX_CONF_PASSWORD_REPEATED_DIGITS", dwMaxRepeatedDigits);
	status = CheckPasswordRepeatedDigitsValidity(entryPassword, dwMaxRepeatedDigits);
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed, Maximum number of permitted repeated characters in conference password has been exceeded", status);

	// In this stage the entry password is OK, now we test the leader password
	const char* leaderPassword = GetH243Password();
	DWORD leaderPasswordMinLength;
	DWORD leaderPasswordMaxLength;
	sysConfig->GetDWORDDataByKey("NUMERIC_CHAIR_PASS_MIN_LEN", leaderPasswordMinLength);
	sysConfig->GetDWORDDataByKey("NUMERIC_CHAIR_PASS_MAX_LEN", leaderPasswordMaxLength);
	sysConfig->GetDWORDDataByKey("NUMERIC_CHAIR_PASS_MAX_LEN", leaderPasswordMaxLength);

	// Check if leader password is set
	status = CheckPasswordValidity(leaderPassword, leaderPasswordMinLength, leaderPasswordMaxLength);
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed, Chairperson password not valid", status);

	status = CheckPasswordRepeatedDigitsValidity(leaderPassword, dwMaxRepeatedDigits);
	TRACECOND_AND_RETURN_VALUE(status != STATUS_OK, "Failed, Maximum number of permitted repeated characters in chairperson password has been exceeded", status);

	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckStrongPasswordValidity(CObjString& passwordStatusDescription)
{
	STATUS status = CMcmsAuthentication::VerifyStrongPassword((char*)GetEntryPassword(), passwordStatusDescription);

	if (status != STATUS_OK)
	{
		TRACEINTO << "Entry password  is not strong"
		          << ", ConfName:"    << GetName()
		          << ", Status:"      << CProcessBase::GetProcess()->GetStatusAsString(status).c_str()
		          << ", Description:" << passwordStatusDescription.GetString();
		return status;
	}

	status = CMcmsAuthentication::VerifyStrongPassword((char*)GetH243Password(), passwordStatusDescription);
	if (status != STATUS_OK)
	{
		TRACEINTO << "H243 password is not strong"
		          << ", ConfName:"    << GetName()
		          << ", Status:"      << CProcessBase::GetProcess()->GetStatusAsString(status).c_str()
		          << ", Description:" << passwordStatusDescription.GetString();
		return status;
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckPasswordValidity(const char* password, DWORD minPasswordLength, DWORD maxPasswordLength)
{
	int status = STATUS_OK;
	if ((password != NULL) && (password[0] != '\0')) // the password can be empty
	{
		// 1. Check if password length is legal
		size_t passLength = strlen(password);
		// a. Check the password is not too long
		TRACECOND_AND_RETURN_VALUE(passLength > MAX_PASSWORD_LENGTH, "Failed, The password is too long", STATUS_ILLEGAL_PASSWORD_LENGTH);

		// b. Check the password is not too short
		TRACECOND_AND_RETURN_VALUE(passLength < minPasswordLength, "Failed, The password is too short", STATUS_ILLEGAL_PASSWORD_LENGTH);

		// c. Check the password is not too long
		TRACECOND_AND_RETURN_VALUE(passLength > maxPasswordLength, "Failed, The password is too long", STATUS_ILLEGAL_PASSWORD_LENGTH);

		// 2. Check if password is legal
		TRACECOND_AND_RETURN_VALUE(!IsNumeric(password), "Failed, The password is not valid", STATUS_PASSWORD_NOT_VALID);
	}
	return status;
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckPasswordRepeatedDigitsValidity(const char* pszPassword, int nRepeatedMax)
{
	int status = STATUS_OK;

	  //BRIDGE-4400: 0 = no limit (in not JITC mode)
	  BOOL isJitcMode = NO;
	  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ULTRA_SECURE_MODE", isJitcMode);
	  if ( !isJitcMode && nRepeatedMax == 0 )
	  {
		  TRACEINTO << "not JITC mode and nRepeatedMax == 0, no limit";
		  return status;
	  }

	int n      = strlen(pszPassword);
	if (n < nRepeatedMax)
		return status;

	char chCurrent = pszPassword[0];
	int  nRepeated = 0;
	for (int i = 1; i < n; ++i)
	{
		if (pszPassword[i] == chCurrent)
		{
			++nRepeated;
			if ((nRepeated+1) > nRepeatedMax)
			{
				// REJECT!!!
				TRACEINTO << "Failed, Maximum password's repeated digits is:" << (DWORD)nRepeatedMax;
				return STATUS_MAX_PASSWORD_REPEATED_DIGITS_EXCEEDED;
			}
		}
		else
		{
			nRepeated = 0;
			chCurrent = pszPassword[i];
		}
	}
	return status;
}

//--------------------------------------------------------------------------
WORD CCommRes::SetConfDialinPrefix()
{
	WORD  status         = STATUS_OK;
	char* pServiceName   = 0;
	char* pServicePrefix = 0;

	// Clean all former services
	CleanAllServicePrefix();

	CIpServiceListManager::VectIterator it = ::GetIpServiceListMngr()->VectBegin();

	for (int i = 0; it != ::GetIpServiceListMngr()->VectEnd(); ++it, ++i)
	{
		// Keep only services with GK
		if (!(*it) || !(*it)->isGKExternal())
			continue;

		pServiceName   = (char*)((*it)->GetServiceName());
		pServicePrefix = (char*)((*it)->GetDialIn().aliasContent);

		m_pServicePrefixStr[m_numServicePrefixStr] = new CServicePrefixStr();
		m_pServicePrefixStr[m_numServicePrefixStr]->SetNetServiceName(pServiceName);
		m_pServicePrefixStr[m_numServicePrefixStr]->SetDialInPrefix(pServicePrefix);
		m_numServicePrefixStr++;
	}

	TRACEINTO << "ConfName:" << GetName() << ", NumServicePrefix:" << m_numServicePrefixStr;

	return status;
}

//--------------------------------------------------------------------------
BYTE CCommRes::IsDefinedIVRService(void) const
{
	BYTE bRes = FALSE;
	if (m_pAvMsgStruct)
	{
		BYTE bWelcomMode = m_pAvMsgStruct->GetAttendedWelcome();
		if (bWelcomMode == CONF_IVR)
			bRes = TRUE;
	}
	else
	{
		if(isExternalIvrControl())
		bRes = TRUE;
	}
	return bRes;
}

//--------------------------------------------------------------------------
void CCommRes::AutoCorrectIndicationOnLayout()
{
	if(eSvcOnly == m_confMediaType)
	{
		m_EnableAudioParticipantsIcon = FALSE;
		m_EnableSelfNetworkQualityIcon = FALSE;
		m_EnableRecordingIcon = FALSE;
		TRACEINTO << "Layout Indication  is disabled for SVC only conference";
	}

	if((eAvcOnly == m_confMediaType) && m_HD)
	{
		m_EnableAudioParticipantsIcon = FALSE;
		m_EnableSelfNetworkQualityIcon = FALSE;
		m_EnableRecordingIcon = FALSE;
		TRACEINTO << "Layout Indication  is disabled for  VSW conference";
	}

	if(eTipCompatibleNone != m_TipCompatibility)
	{
		m_EnableAudioParticipantsIcon = FALSE;
		m_EnableSelfNetworkQualityIcon = FALSE;
		m_EnableRecordingIcon = FALSE;
		TRACEINTO << "Layout Indication is disabled for TIP enabled conference";
	}

	if(!m_EnableRecording)
	{
		m_EnableRecordingIcon = FALSE;
	}

	if((eLocationLeft == m_IconDisplayPosition ) || (eLocationRight == m_IconDisplayPosition ))
	{
		m_IconDisplayPosition = eLocationTop;
		TRACEINTO << "Indication on Layout position does not support left and right, change to top middle";
	}

}

void CCommRes::AutoCorrectConfMediaTypeForEQ()
{
	TRACEINTO;
	if (GetEntryQ()
		&& (isIvrProviderEQ() || isExternalIvrControl())
		&& (m_confMediaType != eAvcOnly))
	{
		TRACEINTO << "EE-462 - EQ conf media type mismatch- changed to eAvcOnly, was " << ConfMediaTypeToString(m_confMediaType) << "";
		m_confMediaType = eAvcOnly;
//		TRACEINTO << "ConfMediaTye:" << m_confMediaType << " - Failed, invalid media type, only AVC-ONLY allowed for EQ";
//		return STATUS_EQ_NOT_ALLOWED_IN_MEDIA_RELAY_CONF;
	}
}
//--------------------------------------------------------------------------
void CCommRes::AutoCorrectTelePresenceParams()
{
// TELEPRESENCE_LAYOUTS - TODO - Ron
/*
	if (NO != m_telePresenceModeConfiguration){
		if(GetIsHDVSW()){
			TRACEINTO << "Video Feature HD VSW does not match TelePresence. Cancel TelePresence";
			CancelTelepresence();
			return;
		}
		if(::GetDongleTelepresenceValue() == FALSE){
			TRACEINTO << "No license for TelePresence. Cancel TelePresence";
			CancelTelepresence();
			return;
		}
	}
	if(m_telePresenceModeConfiguration == YES)
	{
		if(m_pLectureMode->GetLectureModeType() != 0){
			TRACEINTO << "Video Feature Lecture Mode does not match TelePresence. Cancel TelePresence";
			CancelTelepresence();
			return;
		}
		if(m_isSameLayout){
			TRACEINTO << "Video Feature Same Layout does not match TelePresence. Cancel TelePresence";
			CancelTelepresence();
			return;
		}
		if(m_isAutoLayout){
			TRACEINTO << "Video Feature Auto Layout does not match TelePresence. Cancel TelePresence";
			CancelTelepresence();
			return;
		}
	}
*/
	// For version 6, Cropping is always ON.
	m_isCropping = YES;
	if (m_telePresenceModeConfiguration == YES)
		m_isTelePresenceMode = YES;
	else
		m_isTelePresenceMode = NO;

	if (GetIsHDVSW())
	{
		m_isTelePresenceMode            = NO;
		m_telePresenceModeConfiguration = NO;
		m_telePresenceLayoutMode        = eTelePresenceLayoutManual;
	}

	if (m_isTelePresenceMode)
	{
		m_isAutoBrightness = FALSE;
		if ((m_pLectureMode->GetLectureModeType() != 0) || m_isSameLayout || m_isAutoLayout)
		{
			m_isTelePresenceMode            = NO;
			m_telePresenceModeConfiguration = NO;
			m_telePresenceLayoutMode        = eTelePresenceLayoutContinuousPresence;

			TRACEINTO << "Video Features does not match TelePresence. Cancel TelePresence mode and TelePresence mode configuration";
		}

		if (m_pVisualEffectsInfo->GetBackgroundImageID())
			m_pVisualEffectsInfo->SetBackgroundImageID(0);

		if (::GetDongleTelepresenceValue() == FALSE)
		{
			m_isTelePresenceMode            = NO;
			m_telePresenceModeConfiguration = NO;
			m_telePresenceLayoutMode        = eTelePresenceLayoutManual;
			TRACEINTO << "No license for TelePresence. Cancel TelePresence mode";
		}
	}
}

//--------------------------------------------------------------------------
void CCommRes::AutoCorrectShowContentAsVideo()
{
	if (m_ShowContentAsVideo)
	{
		if (m_isSameLayout)
		{
			TRACEINTO << "Can't set ShowContentAsVideo with SameLayout. Cancel ShowContentAsVideo";
			m_ShowContentAsVideo = NO;
		}

		if (m_operatorConf)
		{
			TRACEINTO << "Can't set ShowContentAsVideo with operator conference. Cancel ShowContentAsVideo";
			m_ShowContentAsVideo = NO;
		}

		if (!m_ShowContentAsVideo)
		{
			TRACEINTO << "XCode depends on ShowContentAsVideo, so disable it too";
			SetContentMultiResolutionEnabled(NO);
		}


	}

}

//--------------------------------------------------------------------------
void CCommRes::AutoCorrectConfRate()
{
	if (m_confMaxResolution != eAuto_Res && m_videoSession == CONTINUOUS_PRESENCE)
	{
		if ((m_confMaxResolution <= eCIF_Res) ||
			(m_confMaxResolution <= eSD_Res && m_videoQuality == eVideoQualitySharpness))
		{
			if (m_confTransferRate > Xfer_1024)
			{
				m_confTransferRate = Xfer_1024;
				TRACEINTO << "Conference line rate is modified to 1024 Kbps according to the People Video Definition";
			}
		}

		else if ((m_confMaxResolution <= eSD_Res) ||
			(m_confMaxResolution <= eHD720_Res && m_videoQuality == eVideoQualitySharpness))
		{
			if (m_confTransferRate > Xfer_4096)
			{
				m_confTransferRate = Xfer_4096;
				TRACEINTO << "Conference line rate is modified to 4096 Kbps according to the People Video Definition";
			}
		}
	}

}

//--------------------------------------------------------------------------
STATUS CCommRes::TestLectureModeValidity(CLectureModeParams* pLectureModeParams) const
{
	PASSERT_AND_RETURN_VALUE(!pLectureModeParams, STATUS_ILLEGAL);

	const char* pLecturerName = pLectureModeParams->GetLecturerName();
	CRsrvParty* pCurrParty    = GetCurrentParty(pLecturerName);
	STATUS      stat          = STATUS_OK;

	switch (pLectureModeParams->GetLectureModeType())
	{
		case 0: // lecture_none
		{
			stat = STATUS_OK;
			break;
		}

		case 1: // if lecturer was set we are in regular lecture mode
		{
			if (!strcmp(pLecturerName, "[Auto Select]"))
			{
				TRACEINTO << "Failed, Auto select not supported in lecture mode";
				stat = STATUS_AUTO_SELECT_NOT_SUPPORTED_IN_LECTURE_MODE;
				break;
			}

			// in COP when lecture mode type is 1 (lecture mode) and no lecturer it is valid lecture mode (audio activated)
			if (m_videoSession != VIDEO_SESSION_COP && NULL == pCurrParty)
			{
				pLectureModeParams->SetLectureModeType(0); // Block Lecture mode
				stat = STATUS_OK;
				break;
			}

			if (m_isSameLayout)
			{
				TRACEINTO << "Failed, 'Lecture Mode' and 'Same Layout' are no available in the same conference";
				stat = STATUS_LECTURE_MODE_AND_SAME_LAYOUT_ARE_NOT_AVAILABLE_IN_THE_SAME_CONFERENCE;
			}
			else if (NULL != pCurrParty && pCurrParty->GetVoice())
			{
				TRACEINTO << "Failed, Audio only participant cannot be selected as Lecturer";
				stat = STATUS_AN_AUDIO_ONLY_PARTICIPANT_CANNOT_BE_SELECTED_AS_LECTURER;
			}
			break;
		}

		case LECTURE_SHOW: // lecture show not supported in Carmel
		{
			TRACEINTO << "Failed, Lecture show is not supported";
			stat = STATUS_LECTURE_SHOW_IS_NOT_SUPPORTED;
			break;
		}

		case PRESENTATION_LECTURE_MODE: // presentation mode was set - do nothing
		{
			if (m_isSameLayout) // PresentationMode and same layout are illegal together
			{
				TRACEINTO << "Failed, 'Presentation Mode' and 'Same Layout' are no available in the same conference";
				stat = STATUS_PRESENTATION_MODE_AND_SAME_LAYOUT_ARE_NOT_AVAILABLE_IN_THE_SAME_CONFERENCE;
			}
			break;
		}

		default:
		{
			PASSERTSTREAM(1, "Illegal lecture mode type, LectureModeType:" << pLectureModeParams->GetLectureModeType());
			break;
		}
	}

	return stat;
}

//--------------------------------------------------------------------------
void CCommRes::SetSetLectureModeSameLayoutRules(CLectureModeParams* pLectureModeParams)
{
	if (m_videoSession == VIDEO_SESSION_COP)
	{
		if (pLectureModeParams->GetLectureModeType() != 0)
		{
			TRACEINTO << "Lecture mode is ON, so set m_isSameLayout to 0";
			m_pLectureMode->SetLectureModeType(1);
			m_pLectureMode->SetTimerOnOff(1);
			m_isSameLayout = 0;
		}
		else
		{
			TRACEINTO << "Lecture mode OFF, so set m_isSameLayout to 1";
			m_pLectureMode->SetLectureModeType(0);
			m_pLectureMode->SetTimerOnOff(0);
			m_isSameLayout = 1;
		}

		SLOWCHANGE;
	}
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestReservRecordingValidity()
{
	STATUS stat = STATUS_OK;
	if (GetEnableRecording())
	{
		CRecordingLinkDB* pRecordingLinkDB = ::GetRecordingLinkDB();
		if (!GetEntryQ()) // can't create a recorded EQ
		{
			const char* RecLinkName = GetRecLinkName();
			if (0 == strcmp(RecLinkName, "")) // set recording link default name
			{
				if (pRecordingLinkDB)
				{
					const char* defaultfRecLink = pRecordingLinkDB->GetDefaultRecordingLinkName();
					if (defaultfRecLink && (0 < strlen(defaultfRecLink)))
						SetRecLinkName(defaultfRecLink);
				}
				// leave the RecLink name blank otherwise  -- waiting for RSS dialin
			}
			else // verify that a recording link with that name exists
			{
				if (pRecordingLinkDB)
				{
					if (pRecordingLinkDB->GetParty(RecLinkName) == NULL)
						stat = STATUS_RECORDING_LINK_DOES_NOT_EXIST;
				}
				else
				{
					stat = STATUS_RECORDING_LINK_DOES_NOT_EXIST;
				}
			}

			// in case of work with exchange server - if no recording link - disable recording in conference
			if ((stat == STATUS_RECORDING_LINK_DOES_NOT_EXIST) && (strlen(m_appoitnmentID) > 0))
			{
				SetEnableRecording(0);
				stat = STATUS_OK;
				TRACEINTO << "Conference came back from exchange server with recording and there is no recording link, so disable recording. ConfName:" << m_confDisplayName;
			}
		}
		else
		{
			TRACEINTO << "Can't create a recorded EQ than the recording will be disabled";
			SetEnableRecording(0); // VNGR-7999
		}
	}
	else // in case the recording isn't enabled will reset all the rsrv recording fields
	{
		SetRecLinkName("\0");
		SetRecordingLinkControl(eStopRecording);
		SetStartRecPolicy(START_RECORDING_IMMEDIATELY);
		SetAudioOnlyRecording(NO);
	}

	return stat;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestEncryptionConfValidity()
{
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL IsEncryption;
	std::string key = "ENCRYPTION";
	sysConfig->GetBOOLDataByKey(key, IsEncryption);
	if (IsEncryption == FALSE)
	{
		TRACEINTO << "Failed, Encryption is not available";
		return STATUS_ENCRYPTION_IS_CURRENTLY_NOT_AVAILABLE;
	}
	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCommRes::SetAdHoc(const BYTE IsAdHoc)
{
	m_IsAdHoc = IsAdHoc;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetMeetMePerEntryQ(const BYTE meetMePerEntryQ)
{
	m_meetMePerEntryQ = meetMePerEntryQ;
	UPDATECONFLAGSNEGATIV(m_meetMePerEntryQ, NO, ENTRY_QUEUE_ACCESS)
}

//--------------------------------------------------------------------------
void CCommRes::SetAdHocConfParams()
{
	m_videoSession = CONTINUOUS_PRESENCE;

	// Get duration from system.cfg
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD dwCfgAdHocConfDuration;
	sysConfig->GetDWORDDataByKey(CFG_KEY_CHANGE_AD_HOC_CONF_DURATION, dwCfgAdHocConfDuration);

	// set time struct for set the duration
	int hour = dwCfgAdHocConfDuration/60;
	int min  =  dwCfgAdHocConfDuration%60;
	CStructTm updatedDuration(0, 0, 0, hour, min, 0);

	SetDurationTime(updatedDuration);
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestEncryptionPartyValidity(CRsrvParty* pParty)
{
	CSysConfig* sysConfig     = CProcessBase::GetProcess()->GetSysConfig();
	BYTE        interfaceType = pParty->GetNetInterfaceType();

	// VNGR-15765
	if (::GetDongleEncryptionValue() == FALSE && pParty->GetIsEncrypted() == YES)
	{
		pParty->SetIsEncrypted(NO);
	}

	BOOL shouldForceEncryptionForNonDefinedParties = YES;
	if (GetEncryptionType() == eEncryptWhenAvailable)
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("FORCE_ENCRYPTION_FOR_UNDEFINED_PARTICIPANT_IN_WHEN_AVAILABLE_MODE", shouldForceEncryptionForNonDefinedParties);

	//bool bIsCascadeLink	= pParty->IsCascadeModeMasterOrSlave();	//--- patch for ignoring Encryption in Cascade
	bool bIsWebRtcCall = pParty->GetIsWebRtcCall();

	TRACEINTO << "ConfEncriptionType:"   << (WORD)GetEncryptionType()
	          << ", ConfEncryption:"     << (WORD)GetIsEncryption()
	          << ", PartyEncryption:"    << (WORD)pParty->GetIsEncrypted()
	          << ", ForceForNonDefined:" << (WORD)shouldForceEncryptionForNonDefinedParties
	          //<< ", IsCascadeLink: "     << (bIsCascadeLink ? "true" : "false")
			  << ", bIsWebRtcCall: "     << (bIsWebRtcCall ? "true" : "false");

	// Currently, encryption is not supported in Cascade
	if ( (GetEncryptionType() == eEncryptAll) && (GetIsEncryption() == YES) /*&& !bIsCascadeLink */)	//--- patch for ignoring Encryption in Cascade
	{
		// isdn_encryption
		// VNGR-11391 - JITC: Non-encrypted voice participants (ISDN Phone/PSTN) can dial in and join encrypted conference
		if (pParty->GetIsEncrypted() == NO || (ISDN_INTERFACE_TYPE == pParty->GetNetInterfaceType() && pParty->GetVoice()))
		{
			TRACEINTO << "Failed, Participant encryption settings do not match the conference settings";
			return PARTICIPANT_ENCRYPTION_SETTINGS_DO_NOT_MATCH_THE_CONFERENCE_SETTINGS;
		}
	}

	if ( (GetEncryptionType() == eEncryptAll) && (pParty->GetRecordingLinkParty()) && (pParty->GetIsEncrypted() == NO)/* && !bIsCascadeLink*/ )	//--- patch for ignoring Encryption in Cascade
	{
		BOOL allowNonEncryptRecordingLinkInEncryptedConf = NO;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ALLOW_NON_ENCRYPT_RECORDING_LINK_IN_ENCRYPT_CONF", allowNonEncryptRecordingLinkInEncryptedConf);
		TRACEINTO << "Failed (2), Participant encryption settings do not match the conference settings";
		return PARTICIPANT_ENCRYPTION_SETTINGS_DO_NOT_MATCH_THE_CONFERENCE_SETTINGS;
	}

	if ( (GetEncryptionType() == eEncryptWhenAvailable) && (GetIsEncryption() == YES) /*&& !bIsCascadeLink*/ )	//--- patch for ignoring Encryption in Cascade
	{
		if ( shouldForceEncryptionForNonDefinedParties && pParty->IsUndefinedParty() &&
				(pParty->GetIsEncrypted() == NO || (ISDN_INTERFACE_TYPE == pParty->GetNetInterfaceType() && pParty->GetVoice())) )
		{
			TRACEINTO << "Failed (3), Participant encryption settings do not match the conference settings";
			return PARTICIPANT_ENCRYPTION_SETTINGS_DO_NOT_MATCH_THE_CONFERENCE_SETTINGS;
		}
	}

	BYTE isPartyEncryptionOn = pParty->GetIsEncrypted() == YES || (pParty->GetIsEncrypted() == AUTO && (GetIsEncryption() == YES));
	if ( isPartyEncryptionOn /*&& !bIsCascadeLink*/ )	//--- patch for ignoring Encryption in Cascade
	{
		if (SIP_INTERFACE_TYPE == interfaceType && ((GetEncryptionType() == eEncryptAll) || pParty->GetIsEncrypted() == YES || pParty->GetIsEncrypted() == AUTO))
		{
			const char*            serv_name             = pParty->GetServiceProviderName();
			CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
			CConfIpParameters*     pServiceParams        = NULL;
			pServiceParams = pIpServiceListManager->GetRelevantService(serv_name, pParty->GetNetInterfaceType());
			if (pServiceParams == NULL)
			{
				TRACEINTO << "Failed, Service provider name not exist";
				return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
			}

			if (pServiceParams->GetSipTransportType() != eTransportTypeTls && !GetIsTipCompatible() && (CProcessBase::GetProcess()->GetProductType()!=eProductTypeSoftMCUMfw)) /*eTransportTypeTls eTransportTypeTcp*/ //_dtls_
			{
				TRACEINTO << "Failed, Participant SIP encryption do not match service type";
				return PARTICIPANT_SIP_ENCRYPTION_SETTINGS_DO_NOT_MATCH_SERVICE_TYPE;
			}
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCommRes::SetEnableRecording(const BYTE EnableRecording)
{
	m_EnableRecording = EnableRecording;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetStartRecPolicy(const BYTE StartRecPolicy)
{
	m_StartRecPolicy = StartRecPolicy;
	SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CCommRes::SetRecordingLinkControl(BYTE RecordingLinkControl)
{
	if (m_RecordingLinkControl != RecordingLinkControl)
	{
		m_RecordingLinkControl = RecordingLinkControl;
		SLOWCHANGE;
		IncreaseSummaryUpdateCounter();
	}
}

//--------------------------------------------------------------------------
DWORD CCommRes::FindMyMRMonitorIdByConfName()
{
	return ::GetpMeetingRoomDB()->GetCurrentRsrvID(GetName());
}

//--------------------------------------------------------------------------
void CCommRes::SetGatheringEnabled(bool bEnable)
{
	m_bGatheringEnabled = ::GetIsCOPdongleSysMode() ? false : bEnable;
}

//--------------------------------------------------------------------------
std::string CCommRes::GetFileUniqueName(const std::string& dbPrefix) const
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
bool CCommRes::IsPartyDefined(const mcTransportAddress* pPhoneLenOrIP, const char* alias, DWORD& dwPartyId, BYTE interfaceType, CH323Alias* PartySrcAliasList, int numofalias)
{
	// Check if  party that arrived is a defined party.
	CRsrvParty* pParty    = GetFirstParty();
	WORD        isDefined = FALSE;
	for (; pParty; pParty = GetNextParty())
	{
		if (pParty->GetConnectionType() == DIAL_OUT) // 22/11/01 bug correct
			continue;

		if (pParty->IsUndefinedParty())
			continue;

		if (pParty->GetNetInterfaceType() == interfaceType)
		{
			DWORD partyState  = (CConfParty(*pParty)).GetPartyState(); // pParty->GetPartyState(); //pConfParty->GetPartyState();
			BYTE  isConnected = FALSE;
			if (partyState != PARTY_WAITING_FOR_DIAL_IN && partyState != PARTY_DISCONNECTED)
				isConnected = TRUE;

      if (!isApiTaNull(pPhoneLenOrIP) || pParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE) //In SIP dialIn the transport src IP isn't inialized as of now
			{
				// Compare arrived call sourceIP and party IP
				// Or called source alias to party source alias

				mcTransportAddress partyAddr = pParty->GetIpAddress();
				if (!isApiTaNull(&partyAddr) && !isIpTaNonValid(&partyAddr))
				{
					// IpV6
					if (isIpAddressEqual(&partyAddr, (mcTransportAddress*)pPhoneLenOrIP))
					{
						if ((pParty->GetNetInterfaceType() == H323_INTERFACE_TYPE) && strlen(pParty->GetH323PartyAlias()))
						{
							if (strcmp(pParty->GetH323PartyAlias(), alias) == 0 && !isConnected)
								isDefined = TRUE; // defined
							else
								isDefined = FALSE; // undefined

						}
						else if (!isConnected)
							isDefined = TRUE;  // defined
						else
							isDefined = FALSE;
					}
					else
						isDefined = FALSE; // undefined

				}
				else
				{
					if ((pParty->GetNetInterfaceType() == H323_INTERFACE_TYPE) && pParty->GetH323PartyAlias())
					{
						if (strcmp(pParty->GetH323PartyAlias(), alias) == 0 && !isConnected)
							isDefined = TRUE; // defined
						else
							isDefined = FALSE; // undefined

					}

					if (pParty->GetNetInterfaceType() == H323_INTERFACE_TYPE && isDefined == FALSE && PartySrcAliasList)
					{
						for (WORD i = 0; i < numofalias; i++)
						{
							if (CPObject::IsValidPObjectPtr(&PartySrcAliasList[i]))
							{
								if (PartySrcAliasList[i].GetAliasName())
								{
									if (!strcmp(pParty->GetH323PartyAlias(), PartySrcAliasList[i].GetAliasName()) && !isConnected)
									{
										TRACEINTO << "Here is where EP identified sent alias, AliasName:" << PartySrcAliasList[i].GetAliasName();
										isDefined = TRUE;
									}
								}
							}
							else
								break;
						}
					} // end h323 and search alias
        }
      }
      if(pParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE && isDefined == FALSE ) // sip since transport src address isn't initialized in SIP.
          {
            if (strlen(alias))
            {
              if ((pParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE) && strlen(pParty->GetSipPartyAddress()))
              {
                if (strcmp(pParty->GetSipPartyAddress(), alias) == 0 && !isConnected)
                  isDefined = TRUE;
          }
        }
      }

			if (isDefined) // we found the party in the conference
			{
				dwPartyId = pParty->GetPartyId();
				return true;
			}
		} // end interface type

	} // end for

	return false;
}

//--------------------------------------------------------------------------
STATUS CCommRes::IsConfNameExists() const
{
	TRACEINTO << "ConfName:" << GetName() << ", DisplayName:" << GetDisplayName() << ", IsTemplate:" << (int)IsTemplate() << ", IsConfTemplate:" << (int)IsConfTemplate();

	// Make sure that we do not have a reserv with the same name
	// Make sure we do not have ongoing conf/MR with the same name
	if (!IsTemplate() && !IsConfTemplate())
	{
		CCommResDB* pCommResDB = ::GetpMeetingRoomDB();
		//if (pCommResDB->IsNameExist(GetName()) || pCommResDB->IsNameExist(GetDisplayName(), 1))
		bool bRoutingNameExists = pCommResDB->IsNameExist(GetName());
		bool bDisplayNameExists = pCommResDB->IsNameExist(GetDisplayName(), 1);
		if (bRoutingNameExists || bDisplayNameExists)
		{
			CCommRes* pCommResConf = pCommResDB->GetCurrentRsrv(GetDisplayName()); AUTO_DELETE(pCommResConf);
			if (NULL != pCommResConf && pCommResConf->GetEntryQ())
				return STATUS_RESERVATION_NAME_EXISTS_IN_EQ_LIST;

			if (IsMeetingRoom())
			{
				if (bRoutingNameExists)
					return STATUS_RESERVATION_ROUTING_NAME_EXISTS;
				else
					return STATUS_RESERVATION_NAME_EXISTS;
			}
			else
			{
				if (bRoutingNameExists)
					return STATUS_RESERVATION_ROUTING_NAME_EXISTS_IN_MEETING_ROOM_DB;
				else
					return STATUS_RESERVATION_NAME_EXISTS_IN_MEETING_ROOM_DB;
			}
		}

		CCommConfDB* pCommConfDB = ::GetpConfDB();
		if (pCommConfDB->FindName(GetName()) != NOT_FIND || pCommConfDB->FindName(GetDisplayName(), 1) != NOT_FIND)
			return STATUS_CONF_NAME_EXISTS;
	}
	else if (IsConfTemplate())
	{
		if (IsSIPFactory() || GetEntryQ())
			return STATUS_CONFERENCE_CAN_NOT_BE_SAVED_AS_TEMPLATE;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestUpdateDisplayName() const
{
	if (IsTemplate())
	{
		CCommResDB* pProfiles = GetpProfilesDB();
		BYTE display_name_already_exist =  pProfiles->TestUpdateDisplayName(GetName(), GetDisplayName());
		if (display_name_already_exist)
		{
			TRACEINTO << "Name:" << GetName() << ", DisplayName:" << GetDisplayName() << " - Failed, Profile name already exist";
			return STATUS_PROFILE_NAME_ALREADY_EXISTS;
		}
	}
	else if (IsMeetingRoom())
	{
		CCommResDB* pMeetingRooms = ::GetpMeetingRoomDB();
		BYTE display_name_already_exist =  pMeetingRooms->TestUpdateDisplayName(GetName(), GetDisplayName());
		if (display_name_already_exist)
		{
			TRACEINTO << "Name:" << GetName() << ", DisplayName:" << GetDisplayName() << " - Failed, Meeting room name already exist";
			return STATUS_RESERVATION_NAME_EXISTS;
		}
	}
	else if (IsConfTemplate())
	{
		CCommResDB* pConfTemplates = ::GetpConfTemplateDB();
		BYTE display_name_already_exist = pConfTemplates->TestUpdateDisplayName(GetName(), GetDisplayName(), GetMonitorConfId());
		if (display_name_already_exist)
		{
			TRACEINTO << "Name:" << GetName() << ", DisplayName:" << GetDisplayName() << " - Failed, Conf Template name already exist";
			return STATUS_CONF_TEMPLATE_NAME_ALREADY_EXISTS;
		}
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCommRes::SetMonitorIdForAllParties()
{
	// Set the monitor id for each party
	TRACEINTO << "ConfName:" << GetName();

	CRsrvParty* pParty = GetFirstParty();
	while (pParty != NULL)
	{
		if (pParty->GetPartyId() == 0xFFFFFFFF || pParty->GetPartyId() <= HALF_MAX_DWORD)
			pParty->SetPartyId(NextPartyId());

		if (IsAudioConf())
			pParty->SetVoice(YES);

		pParty = GetNextParty();
	}
}

//--------------------------------------------------------------------------
void CCommRes::ResetMonitorIdForAllParties()
{
	// Reset Party Id Counter
	m_partyIdCounter = 0;
	// ReSet the monitor id for each party
	TRACEINTO << "ConfName:" << GetName();

	CRsrvParty* pParty = GetFirstParty();
	while (pParty != NULL)
	{
		pParty->SetPartyId(0xFFFFFFFF);
		pParty = GetNextParty();
	}
}

//--------------------------------------------------------------------------
void CCommRes::CopyToShortAllPrefixStr(CCommResShort& shortRes) const
{
	STATUS stat = STATUS_OK;
	for (int i = 0; i < (int)m_numServicePrefixStr; i++)
	{
		stat = shortRes.AddServicePrefix(*m_pServicePrefixStr[i]);
		PASSERTMSG_AND_RETURN(stat, "Failed, Cannot add prefix to short");
	}
}

//--------------------------------------------------------------------------
BYTE CCommRes::GetCascadeEQ() const
{
	if (GetEntryQ())
		return (m_cascadeMode) ? YES : NO;
	return NO;
}

//--------------------------------------------------------------------------
void CCommRes::FillEmptyDiplayNameOrName(const char* profileName)
{
	// just to make sure we use legal string
	m_H243confName[H243_NAME_LEN-1] = '\0';
	m_confDisplayName[H243_NAME_LEN-1] = '\0';
	m_NumericConfId[NUMERIC_CONFERENCE_ID_LEN-1] = '\0';

	// 1)if we received empty conf name (ascii) we fill it:
	// if display name is ascii - copy the display name
	// else copy the numeric id
	// if it is a profile we fill it with "profile_ProfileId" in AddProfile

	TRACEINTO << "ConfName:" << m_H243confName << ", DisplayName:" << m_confDisplayName << ", NumericConfId:" << m_NumericConfId;

	ReplaceCertainCharactersInDisplayNameWithSpaces();

	BOOL IsEnableRouteNameChangeInvalidCharToUnderscore = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ROUTE_NAME_CHANGE_INVALID_CHAR_TO_UNDERSCORE", IsEnableRouteNameChangeInvalidCharToUnderscore);

	if (IsEnableRouteNameChangeInvalidCharToUnderscore)
		ReplaceInvalidCharactersInRoutingNameWithUnderScore();

	// VNGFE-4875 - Fix for amdocs, if no need to replace invalid char, copy m_H243confName to m_confDisplayName
	if (m_confDisplayName[0] == '\0' && m_H243confName[0] != '\0' && IsEnableRouteNameChangeInvalidCharToUnderscore == FALSE)
	{
		strcpy_safe(m_confDisplayName, m_H243confName);
		TRACEINTO << "DisplayName:" << m_confDisplayName << " - Empty conference display name copied from conference name";
	}

	if ((m_H243confName[0] == '\0') ||
	    (!IsValidASCII(m_H243confName, H243_NAME_LEN, "CONF_NAME"))) /*VNGR-13351 - Routing Name is Automatically fixed if Empty or invalid*/
	{
		// profile: empty name will be filled only when add profile to allocated name "profile_ProfileId"
		// profile name will not filled from ascii display name because not shown in EMA and can be rejected if already exist (vngr-5053)
		if (m_isTemplate)
		{
			if (profileName != NULL && IsValidASCII(profileName, H243_NAME_LEN, "CONF_NAME"))
			{
				strcpy_safe(m_H243confName, profileName);
				TRACEINTO << "ConfName:" << m_H243confName << " - Empty conference name was overwritten by profile name";
			}
			else
				TRACEINTO << "Warning, Empty conference name was not filled";
		}
		else
		{
			if (IsValidASCII(m_confDisplayName, H243_NAME_LEN, "CONF_NAME"))
			{
				strcpy_safe(m_H243confName, m_confDisplayName);
				TRACEINTO << "ConfName:" << m_H243confName << " - Empty conference name was overwritten by display name";
			}
			else if (m_NumericConfId[0] != '\0')
			{
				// NUMERIC_CONFERENCE_ID_LEN = 17 , H243_NAME_LEN = 81
				strcpy_safe(m_H243confName, m_NumericConfId);
				TRACEINTO << "ConfName:" << m_H243confName << " - Empty conference name was overwritten by numeric conference id";
			}
			else if (m_isConfTemplate)
			{
				if (profileName != NULL && IsValidASCII(profileName, H243_NAME_LEN, "CONF_NAME"))
					strncpy(m_H243confName, profileName, H243_NAME_LEN-1);

				m_H243confName[H243_NAME_LEN-1] = '\0';
			}
			else
				TRACEINTO << "Warning, Empty conference name was not filled (2)";
		}
	}

	if (!m_isConfTemplate && strncmp(m_H243confName, _sTemplateUniqeRoutingName.c_str(), _sTemplateUniqeRoutingName.size()) == 0)
	{
		memset(m_H243confName, 0, H243_NAME_LEN);
		TRACEINTO << "Set empty ConfName";
	}

	// 2)if we received empty conf display name (utf-8) we fill it
	// from conf name (ascii)
	if (m_confDisplayName[0] == '\0')
	{
		strcpy_safe(m_confDisplayName, m_H243confName);
		TRACEINTO << "DisplayName:" << m_confDisplayName << " - Empty conference display name copied from conference name";
	}
}

//--------------------------------------------------------------------------
bool CCommRes::IsEmptyDiplayNameOrName()
{
	if ((m_H243confName[0] == '\0') || (m_confDisplayName[0] == '\0'))
		return true;
	return false;
}

//--------------------------------------------------------------------------
void CCommRes::SetServicePhoneToShort(CCommResShort& shortRes)
{
	STATUS stat = STATUS_OK;
	for (int i = 0; i < (int)m_numServicePhoneStr; i++)
		if (m_pServicePhoneStr[i] != NULL)
		{
			stat = shortRes.AddServicePhone(*m_pServicePhoneStr[i]);
			PASSERTMSG_AND_RETURN(stat, "Failed, Cannot add prefix to short");
		}
}

//--------------------------------------------------------------------------
void CCommRes::SetDefaultParamsAccordingToProductType()
{
	eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
	
	if (eProductFamilySoftMcu == prodFamily)
	{
		eProductType prodType = CProcessBase::GetProcess()->GetProductType();
		TRACEINTO << "ProductType:" << ::ProductTypeToString(prodType);

		m_audioMixDepth          = SFTMCU_AUDIO_MIX_DEPTH_DEFAULT;
		m_EchoSuppression        = NO;
		m_KeyboardSuppression    = NO;
		m_isAudioClarity         = NO;

		DWORD reservationRate = GetConfTransferRate();
		CCapSetInfo lCapInfo = eUnknownAlgorithemCapCode;
		DWORD confRate = lCapInfo.TranslateReservationRateToIpRate(reservationRate);
		if ((prodType != eProductTypeCallGeneratorSoftMCU) && (prodType != eProductTypeNinja) &&
		    (m_confMediaType != eSvcOnly) && (m_confMediaType != eMixAvcSvcVsw) && (confRate > AVC_SOFT_MCU_MAX_CALL_RATE))
		{
			SetConfTransferRate(Xfer_4096);
			TRACEINTO << "ConfTransferRate:" << (WORD)GetConfTransferRate() << ", ConfMediaType:" << m_confMediaType;
		}
        if (CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw)
        {
        	m_muteAllPartiesAudioExceptLeader = NO;
        	m_muteAllPartiesVideoExceptLeader = NO;
        	TRACEINTO << "Not in MFW, m_muteAllPartiesAudioExceptLeader: NO , m_muteAllPartiesVideoExceptLeader = NO";
        }
	}
}

//--------------------------------------------------------------------------
void CCommRes::SetDefaultParamsAccordingToSystemCardsBasedMode()
{
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	if (!IsTemplate())
	{
		if (systemCardsBasedMode == eSystemCardsMode_mpm)
		{
			if (m_HD) // in VSW conf
			{
				if (m_confTransferRate == Xfer_6144 || m_confTransferRate == Xfer_8192)
				{
					TRACEINTO << "In VSW conference update the m_confTransferRate to 4M - max in MPM based system";
					m_confTransferRate = Xfer_4096;
				}
				if (m_HDResolution == eHD1080Res || m_HDResolution == eHD720p60Res || m_HDResolution == eHD1080p60Res)
				{
					TRACEINTO << "Update the m_HDResolution to HD720 - MPM based system";
					m_HDResolution = eHD720Res;
				}
				m_H264VSWHighProfilePreference = eBaseLineOnly; // the H264 VSW High Profile feature is disabled in MPM mode.
			}
			else
			{
				if (m_confTransferRate == Xfer_4096 || m_confTransferRate == Xfer_6144 || m_confTransferRate == Xfer_8192)
				{
					TRACEINTO << "In CP conference update the m_confTransferRate to Xfer_1920 - max in MPM based system";
					m_confTransferRate = Xfer_1920;
				}

				// In CP conference the video rate for the participants in MPM based system is up to 1856
				CRsrvParty* pParty = GetFirstParty();
				while (pParty != NULL)
				{
					DWORD partyVideoRate = pParty->GetVideoRate();
					if ((0xFFFFFFFF != partyVideoRate) && (partyVideoRate > 1856))
					{
						TRACEINTO << "In CP conference the max call rate is 1920";
						pParty->SetVideoRate(1856);
					}
					pParty = GetNextParty();
				}
			}
			m_isVideoClarityEnabled = NO; // the video clarity feature is disabled in MPM mode.
			m_ShowContentAsVideo    = NO; // the content as video feature is disabled on MPM mode
		}
		else if (systemCardsBasedMode == eSystemCardsMode_mpm_plus || systemCardsBasedMode == eSystemCardsMode_breeze)
		{
			if (m_HD) // in VSW conf
			{
				if (m_confTransferRate == Xfer_8192)
				{
					TRACEINTO << "VSW conference the max call rate in MPM+/MPMX is 6144k";
					m_confTransferRate = Xfer_6144;
				}
				m_H264VSWHighProfilePreference = eBaseLineOnly; // the H264 VSW High Profile feature is disabled in MPM Plus mode.
			}
			else
			{
				if (m_confTransferRate == Xfer_6144 || m_confTransferRate == Xfer_8192)
				{
					TRACEINTO << "CP conference the max call rate in MPM+/MPMX is 4096";
					m_confTransferRate = Xfer_4096;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
void CCommRes::AutoCorrectPartyLayoutData()
{
	CRsrvParty* pParty = GetFirstParty();
	while (pParty != NULL)
	{
		CVideoLayout* pVideoLayout = pParty->GetRsrvVideoLayout();
		for (int i = 0; i < pVideoLayout->m_numb_of_cell; i++)
		{
			CVideoCellLayout* pVideoCellLayout = pVideoLayout->m_pCellLayout[i];
			if (pVideoCellLayout->GetName() && pVideoCellLayout->GetForcedPartyId() == 0xFFFFFFFF)
			{
				CRsrvParty* pPartyInCell = GetCurrentParty(pVideoCellLayout->GetName());
				if (pPartyInCell)
				{
					TRACEINTO << "PartyName:" << pParty->GetName() << ", ForcedParty:" << pVideoCellLayout->GetName() << ", ForcedPartyId:" << pPartyInCell->GetPartyId();
					pVideoCellLayout->SetForcedPartyId(pPartyInCell->GetPartyId(), pVideoCellLayout->GetCellStatus());
					pVideoCellLayout = pVideoLayout->m_pCellLayout[i];
					TRACEINTO << "PartyName:" << pParty->GetName() << ", ForcedParty:" << pVideoCellLayout->GetName() << ", ForcedPartyId:" << pVideoCellLayout->GetForcedPartyId() << " - After update";
				}
			}
		}

		pParty->UpdatePrivateVideoLayout(*(const_cast<CVideoLayout*>(pVideoLayout)));
		if (m_pLectureMode->GetLectureModeType() == PRESENTATION_LECTURE_MODE)
		{
			pVideoLayout->SetActive(NO);
			pParty->SetIsPrivateLayout(NO);
			pParty->UpdatePrivateVideoLayout(*(const_cast<CVideoLayout*>(pVideoLayout)));
		}

		if (m_isSameLayout)
		{
			pParty->CancelPrivateVideoLayout(pVideoLayout->GetScreenLayout());
			pParty->SetIsPrivateLayout(NO);
			CVideoLayout* pConfLayout = GetActiveRsrvVidLayout();
			// /VNGFE-1919
			if (!pConfLayout)
			{
				TRACEINTO << "No active layout for the reservation";
				CVideoLayout* pCurrentResVideoLayout = GetCurrentRsrvVidLayout(m_contPresScreenNumber);
				if (pCurrentResVideoLayout && IsValidPObjectPtr(pCurrentResVideoLayout))
				{
					CVideoLayout VideoLayout = *pCurrentResVideoLayout;
					VideoLayout.SetActive(YES);
					UpdateRsrvVidLayout(VideoLayout);
					pConfLayout   = GetActiveRsrvVidLayout();
					*pVideoLayout = *pConfLayout;
				}
				else
				{
					TRACEINTO << "The current RSRV video layout isn't valid, we will update it";
					CVideoLayout* pUpdatedConfLayout = new CVideoLayout();
					AddNewVideoLayout(m_contPresScreenNumber, YES, pUpdatedConfLayout);
					pConfLayout = pUpdatedConfLayout;
					*pVideoLayout = *pConfLayout;
					POBJDELETE(pUpdatedConfLayout);
				}
			}
			else
				*pVideoLayout = *pConfLayout;
		}

		// Always start party with TelePresenceMode NONE.
		pParty->SetTelePresenceMode(NONE);
		if (m_isTelePresenceMode == NO && m_telePresenceModeConfiguration == NO)
		{
			if (pParty->GetTelePresenceMode() != NONE)
			{
				pParty->CancelPrivateVideoLayout(pVideoLayout->GetScreenLayout());
				pParty->SetIsPrivateLayout(NO);
				CVideoLayout* pConfLayout = GetActiveRsrvVidLayout();
				*pVideoLayout = *pConfLayout;
			}
		}

		pParty = GetNextParty();
	}
}

//--------------------------------------------------------------------------
STATUS CCommRes::TestOperatorConfValidity(const char* loginName)
{
	STATUS ret_status = STATUS_OK;

	if (!GetOperatorConf())
	{
		TRACEINTO << "ConfName:" << GetName() << " - Not operator conference";
		return STATUS_OK;
	}

	if (ret_status == STATUS_OK && strncmp(loginName, m_H243confName, strlen(loginName)))
	{
		TRACEINTO << "ConfName:" << GetName() << ", LoginName:" << loginName << " - Failed, Login name does not equal to conference name";
		ret_status = STATUS_ILLEGAL_OPERATOR_CONFERENCE_NOT_LOGIN_NAME;
	}

	if (ret_status == STATUS_OK && IsConfNameExists() == STATUS_CONF_NAME_EXISTS)
	{
		TRACEINTO << "ConfName:" << GetName() << " - Failed, Conference name already exist";
		ret_status = STATUS_CONF_NAME_EXISTS;
	}

	if (ret_status == STATUS_OK && GetNumParties() != 1)
	{
		TRACEINTO << "ConfName:" << GetName() << " - Failed, Wrong number of participants";
		ret_status = STATUS_ILLEGAL_OPERATOR_CONFERENCE_WRONG_NUM_OF_PARTIES;
	}

	return ret_status;
}

//--------------------------------------------------------------------------
void CCommRes::SetOperatorConfInfo()
{
	TRACEINTO << "ConfName:" << GetName();

	COperatorConfInfo operatorConfInfo;
	operatorConfInfo.SetOperatorConfName(m_H243confName);
	operatorConfInfo.SetOperatorConfMonitoringId(GetMonitorConfId());

	if (GetNumParties() != 1)
	{
		PASSERT(STATUS_ILLEGAL_OPERATOR_CONFERENCE);
		return;
	}

	CRsrvParty* pRsrvParty = GetFirstParty();
	operatorConfInfo.SetOperatorPartyName(pRsrvParty->GetName());
	operatorConfInfo.SetOperatorPartyMonitoringId(pRsrvParty->GetPartyId());

	pRsrvParty->SetOperatorParty(operatorConfInfo);

	POBJDELETE(m_pOperatorConfInfo);
	m_pOperatorConfInfo = new COperatorConfInfo(operatorConfInfo);
	m_pOperatorConfInfo->Dump();
}

//--------------------------------------------------------------------------
void CCommRes::InitPartiesMoveInfo()
{
	CMoveConfDetails confMoveDetails(this);
	for (int i = 0; i < (int)m_numParties; i++)
	{
		if (m_pParty[i] != NULL)
		{
			CMoveInfo* partyMoveInfo = m_pParty[i]->GetMoveInfo();
			if (partyMoveInfo == NULL)
			{
				partyMoveInfo = new CMoveInfo();
			}

			partyMoveInfo->Create(confMoveDetails, m_pParty[i]->IsOperatorParty());
		}
	}
}

//--------------------------------------------------------------------------
BOOL CCommRes::isValidStartTime()
{
	BOOL isValidTime = TRUE;
	if ((m_startTime.m_sec > 0) || (m_startTime.m_min > 0) || (m_startTime.m_hour > 0)
			|| (m_startTime.m_day > 0) || (m_startTime.m_year > 0))
	{
		isValidTime = m_startTime.IsValid();
		if (isValidTime)
		{
			if ((m_startTime.m_day == 0 && m_startTime.m_mon != 0) || (m_startTime.m_day != 0 && m_startTime.m_mon == 0))
				isValidTime = FALSE;
		}
	}

	return isValidTime;
}

//--------------------------------------------------------------------------
void CCommRes::ResetStartTime()
{
	m_startTime.m_sec  = 0;
	m_startTime.m_min  = 0;
	m_startTime.m_hour = 0;
	m_startTime.m_day  = 0;
	m_startTime.m_mon  = 0;
	m_startTime.m_year = 0;
}

//--------------------------------------------------------------------------
BYTE CCommRes::GetIsCOP() const
{
	return (VIDEO_SESSION_COP == GetVideoSession()) ? YES : NO;
}

//--------------------------------------------------------------------------
void CCommRes::SetCopDefaultSettings()
{
  m_isVideoClarityEnabled = NO;
	m_ShowContentAsVideo    = NO;
	m_isTelePresenceMode    = NO;
	m_operatorConf          = NO;
	m_HD                    = NO;
	m_IsConfOnPort          = YES;
	m_media                 = AUDIO_VIDEO_MEDIA;
}

//--------------------------------------------------------------------------
void CCommRes::AutoCorrectPermanentConf() // Permanent Conf
{
	if (m_isPermanent && YES == m_automaticTermination)
	{
		m_automaticTermination = NO;
		PTRACE(eLevelInfoNormal, "CCommRes::AutoCorrectPermanentConf - Automatic termination disabled");
		SLOWCHANGE;
	}
}

//--------------------------------------------------------------------------
void CCommRes::AutoCorrectEncryptionParams()
{
	if ((GetIsEncryption() == NO) && (GetEncryptionType() != eEncryptNone))
		SetEncryptionParameters(NO, eEncryptNone);
	else if ((GetIsEncryption() == YES) && (GetEncryptionType() == eEncryptNone))
		SetEncryptionParameters(YES, eEncryptAll);
}

//-------------------------------------------------------------------------
void CCommRes::AutoCorrectManageLayoutInternally()
{
	SetManageTelepresenceLayoutInternaly(IsLayoutManagedInternally());

	if (GetManageTelepresenceLayoutInternaly() && (GetTelePresenceModeConfiguration() == YES))
	{
		TRACEINTO << "Profile parameters is changed because we manage the layout internally in room switch mode";
		m_pLectureMode->SetLectureModeType(0);
		m_isAutoLayout          = NO;
		m_isSameLayout          = NO;
	}

}

//--------------------------------------------------------------------------
DWORD CCommRes::TestPermanentConfValidity() // Permanent Conf
{
	if (m_isPermanent && (GetAdHoc() || IsSIPFactory() || GetEntryQ()))
	{
		TRACEINTO << "Failed, Permanent conference can't be Ad-Hoc or SIP Factory or EQ";
		return STATUS_ILLEGAL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CCommRes::ReplaceCertainCharactersInDisplayNameWithSpaces()
{
	if (m_confDisplayName[0] != '\0')
	{
		char* pSpecialChar1 = NULL;
		char* pSpecialChar2 = NULL;
		pSpecialChar1 = strchr(m_confDisplayName, ',');
		pSpecialChar2 = strchr(m_confDisplayName, ';');
		while (pSpecialChar1 || pSpecialChar2)
		{
			if (pSpecialChar1)
				pSpecialChar1[0] = ' ';

			if (pSpecialChar2)
				pSpecialChar2[0] = ' ';

			pSpecialChar1 = strchr(m_confDisplayName, ',');
			pSpecialChar2 = strchr(m_confDisplayName, ';');
		}
	}
}

//--------------------------------------------------------------------------
void CCommRes::ReplaceInvalidCharactersInRoutingNameWithUnderScore()
{
	int len = strlen(m_H243confName);
	BOOL IsCheckSipChars = TRUE;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetBOOLDataByKey("CHECK_SIP_CHARS_IN_CONF_NAME", IsCheckSipChars);

	if (m_H243confName[0] != '\0')
	{
		for (WORD char_index = 0; char_index < len; char_index++)
		{
			if (!((m_H243confName[char_index] > 47 && m_H243confName[char_index] < 58)
			      || (m_H243confName[char_index] > 64 && m_H243confName[char_index] < 91)
			      || (m_H243confName[char_index] > 96 && m_H243confName[char_index] < 123)
			      || m_H243confName[char_index] == ' ' || m_H243confName[char_index] == '_'
			      || m_H243confName[char_index] == '-' || m_H243confName[char_index] == '@'
			      || m_H243confName[char_index] == '(' || m_H243confName[char_index] == ')')
			    || (IsCheckSipChars && strchr(BAD_CHARACTERS_FOR_SIP_URI, m_H243confName[char_index])))
			{
				TRACEINTO << "faulty_char_index:" << char_index << ", faulty_char:" << (WORD)m_H243confName[char_index];
				m_H243confName[char_index] = '_';
			}
		}
	}
}
STATUS CCommRes::CheckIvrProductTypeValidity()
{
	eProductType ePT = CProcessBase::GetProcess()->GetProductType();
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

	switch (ePT)
	{
	case eProductTypeSoftMCUMfw:
		if(GetEntryQ() && !isExternalIvrControl())
			return STATUS_IVR_ILLEGAL_PARAMETERS;
		break;
	case eProductTypeSoftMCU:
	default:
		return STATUS_OK;
	}
	return STATUS_OK;
}
//--------------------------------------------------------------------------
STATUS CCommRes::CheckIvrProviderEQValidity()
{
	CAvMsgStruct*      pAvMsgStruct      = GetpAvMsgStruct();
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();

	int index = pAVmsgServiceList->FindAVmsgServ(pAvMsgStruct->GetAvMsgServiceName());
	if (NOT_FIND == index)
	{
		TRACEINTO << "ServiceName:" << pAvMsgStruct->GetAvMsgServiceName() << " - Failed, IVR default Service does not appear in list";
		return STATUS_IVR_SERVICE_NAME_DOES_NOT_EXISTS;
	}
	else
	{
		PASSERTSTREAM_AND_RETURN_VALUE(index >= MAX_IVR_SERV_IN_LIST, "Index:" << index << " - Invalid index", STATUS_FAIL);

		const CIVRService* pIVRService = pAVmsgServiceList->m_pAVmsgService[index]->GetIVRService();
		if (pIVRService)
		{
			const CIVROperAssistanceFeature* pCIVROperAssistFeature = pIVRService->GetOperAssistanceFeature();
			if (pCIVROperAssistFeature && pCIVROperAssistFeature->GetEnableDisable() == YES)   // VNGR-23506
			{
				TRACEINTO << "ServiceName:" << pIVRService->GetName() << " - Failed, Operator assistance is not allowed with ivrProviderEQ";
				return STATUS_INCOMPATIBLE_IVR_SERVICE;
			}

			//AT&T
			if (isExternalIvrControl())
			{
				TRACEINTO << "ServiceName:" << pIVRService->GetName() << " - Failed, externalIverControl is not allowed with ivrProviderEQ";
				return STATUS_INCOMPATIBLE_IVR_SERVICE;
			}

			STATUS status = CheckIvrProviderEQAndExternalIvrControlValidity();
			if (status != STATUS_OK)
				return status;

		} //end if (pIVRService)
	} //end else

	return STATUS_OK;
}

//--------------------------------------------------------------------------
//AT&T
STATUS CCommRes::CheckExternalIvrControlValidity()
{
	if (isIvrProviderEQ())
	{
		TRACEINTO << "Failed, externalIverControl is not allowed with externalIvrProtocol";
		return STATUS_INCOMPATIBLE_IVR_SERVICE;
	}

	STATUS status = CheckIvrProviderEQAndExternalIvrControlValidity();
	if (status != STATUS_OK)
		return status;

	//entry queue IVR service is disable when we in externalIvrControl mode
	CAvMsgStruct*      pAvMsgStruct      = GetpAvMsgStruct();
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();

	pAvMsgStruct->SetAvMsgServiceName("\0");
	pAvMsgStruct->SetAttendedWelcome(0);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckIvrProviderEQAndExternalIvrControlValidity()
{
	if (GetCascadeEQ() == YES)
	{
		TRACEINTO << "Failed, CascadeEQ is not allowed with ivrProviderEQ/ExternalIvrControl";
		return STATUS_INCOMPATIBLE_IVR_SERVICE;
	}

	if (GetAdHoc() == YES)
	{
		TRACEINTO << "Failed, AdHoc is not allowed with ivrProviderEQ/ExternalIvrControl";
		return STATUS_INCOMPATIBLE_IVR_SERVICE;
	}

	if (GetMeetMePerConf())
	{
		TRACEINTO << "Failed, ISDN is not allowed with ivrProviderEQ/ExternalIvrControl";
		return STATUS_INCOMPATIBLE_IVR_SERVICE;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CCommRes::CheckXCodeConfContentValidity()
{
	// 1. check the content transcoding validity
	if (!IsFeatureSupportedBySystem(eFeatureContentTranscoding))
	{
		if (GetContentMultiResolutionEnabled())
		{
			TRACEINTO << "Failed, Content Transcoding is not supported";
			return STATUS_CONTENT_XCODE_NOT_SUPPORTED;
		}
	}
	else
	{
		if (GetContentMultiResolutionEnabled())
		{
			if (m_confMediaType == eSvcOnly)
			{
				TRACEINTO << "Failed, Content Transcoding is not  supported in SVC only conference";
				return STATUS_CONTENT_XCODE_ONLY_SUPPORTED_FOR_CP;
			}

			if (GetIsTipCompatible() != eTipCompatibleNone && GetIsTipCompatible() != eTipCompatibleVideoOnly)
			{
				TRACEINTO << "XCode is not allowed in TIP compatible conferences with TIP content, so disable it";
				SetContentMultiResolutionEnabled(NO);
			}

			if (GetIsHDVSW())
			{
				TRACEINTO << "Failed, VSW and Content Transcoding could NOT enabled together";
				return STATUS_VSW_CONTENT_XCODE_CONFLICT;
			}

			SetLegacyShowContentAsVideo(YES);
			SetContentXCodeH264Supported(YES);
			if (GetContentXCodeH263Supported())
			{
				m_PresentationProtocol = ePresentationAuto;
			}
			else
			{
				m_PresentationProtocol = eH264Dynamic;
			}

			SetIsHighProfileContent(NO);
		}
		else
		{
			SetIsAsSipContent(NO);
			SetContentXCodeH264Supported(NO);
			SetContentXCodeH263Supported(NO);
		}
	}

	if (GetIsAsSipContent())
	{
		SetContentMultiResolutionEnabled(YES);
		SetContentXCodeH264Supported(YES);
		SetContentXCodeH263Supported(NO);
		SetIsCascadeOptimized(NO);
	}

	// 2. check the compliance with system flags
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	BOOL        bIsH239;
	sysConfig->GetBOOLDataByKey("ENABLE_H239", bIsH239);
	if (!bIsH239)
	{
		TRACEINTO << "Content multi-resolution is not allowed when ENABLE_H239 system flag is disabled, change it to NO";
		SetContentMultiResolutionEnabled(NO);
		m_ShowContentAsVideo = NO;
	}

	if (GetContentMultiResolutionEnabled())
	{
		TRACEINTO << "H.264 is set when Content-TX is set - it is mandatory for Content-TX";
		SetContentXCodeH264Supported(YES);
	}

	if(GetIsAsSipContent() && GetEntryQ())
	{
		TRACEINTO << "CCommRes::CheckXCodeConfContentValidity - Disable AS-Sip in EntryQ , Name: " << GetName();
		SetIsAsSipContent(NO);;
	}


	return STATUS_OK;
}
//--------------------------------------------------------------------------
WORD CCommRes::CalculateIsdnChannelsNumber(BYTE transferRate)
{
	WORD isdnChannelsNumber = 0;

	switch (transferRate)
	{
		case Xfer_64:   { isdnChannelsNumber = 0 ; break; }
		case Xfer_96:   { isdnChannelsNumber = 0 ; break; }
		case Xfer_128:  { isdnChannelsNumber = 2 ; break; }
		case Xfer_192:  { isdnChannelsNumber = 3 ; break; }
		case Xfer_256:  { isdnChannelsNumber = 4 ; break; }
		case Xfer_320:  { isdnChannelsNumber = 5 ; break; }
		case Xfer_384:  { isdnChannelsNumber = 6 ; break; }
		case Xfer_512:  { isdnChannelsNumber = 8 ; break; }
		case Xfer_768:  { isdnChannelsNumber = 12; break; }
		case Xfer_832:  { isdnChannelsNumber = 12; break; }
		case Xfer_1024: { isdnChannelsNumber = 12; break; }
		case Xfer_1152: { isdnChannelsNumber = 18; break; }
		case Xfer_1280: { isdnChannelsNumber = 18; break; }
		case Xfer_1472: { isdnChannelsNumber = 23; break; }
		case Xfer_1536: { isdnChannelsNumber = 24; break; }
		case Xfer_1728: { isdnChannelsNumber = 24; break; }
		case Xfer_1920: { isdnChannelsNumber = 30; break; }
		case Xfer_2048: { isdnChannelsNumber = 30; break; }
		case Xfer_2560: { isdnChannelsNumber = 30; break; }
		case Xfer_3072: { isdnChannelsNumber = 30; break; }
		case Xfer_3584: { isdnChannelsNumber = 30; break; }
		case Xfer_4096: { isdnChannelsNumber = 30; break; }
		case Xfer_6144: { isdnChannelsNumber = 30; break; }
		case Xfer_8192: { isdnChannelsNumber = 30; break; }

		default:
		{
			PASSERTSTREAM_AND_RETURN_VALUE(1, "TransferRateAsNumber:" << (int)transferRate << " - Invalid transfer rate", 0);
		}
	}

	const char *transferRateAsString = NULL;
	CStringsMaps::GetDescription(TRANSFER_RATE_ENUM, transferRate, &transferRateAsString);

	std::ostringstream msg;
	if (!transferRateAsString)
		msg << "TransferRateAsNumber:" << (int)transferRate << ", IsdnChannelsNumber:" << isdnChannelsNumber;
	else
		msg << "TransferRate:" << transferRateAsString << ", IsdnChannelsNumber:" << isdnChannelsNumber;

	if (isdnChannelsNumber == 0)
		msg << " - Illegal rate for bonding";

	TRACEINTO << msg.str().c_str();

	return isdnChannelsNumber;
}
