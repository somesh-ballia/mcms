/*
 * Gathering.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: bguelfand
 */

#include "Gathering.h"
#include "SystemFunctions.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"
#include "UnicodeStringTable.h"
#include "Conf.h"
#include "Trace.h"
#include "VideoBridgeInterface.h"
#include "VisualEffectsParams.h"
#include "VideoApiDefinitionsStrings.h"

#define SAFEPATRYNAME (pszPartyName ? pszPartyName : "NULL")
#define PARTYNOTFOUND (m_mapPartiesData.find(pszPartyName) == m_mapPartiesData.end())

////////////////////////////////////////////////////////////////////////////
//                        CGathering
////////////////////////////////////////////////////////////////////////////
CGathering::CGathering(CGatheringManager* pGatheringManager)
{
	m_bStarted                = false;
	m_pVideoLayoutPrev        = NULL;
	m_bAutoLayoutPrev         = false;
	m_bChangeLayout           = true;
	m_bInitialized            = false;
	m_pGatheringManager       = pGatheringManager;
	m_pConf                   = pGatheringManager->GetConf();
	m_bEndGathering           = false;
	m_bFullScreenGathering    = false;
	m_nBackGroundTransparency = 100;

	Initialize();
}

// --------------------------------------------------------------------------
CGathering::~CGathering()
{
	PDELETE(m_pVideoLayoutPrev)
}

// --------------------------------------------------------------------------
void CGathering::Initialize()
{
	if (m_bInitialized)
		return;

	m_bInitialized = true;

	SystemGetTime(m_dtStartGathering);
}

// --------------------------------------------------------------------------
bool CGathering::IsGatheringTimeout()
{
	CStructTm dtCurrent;
	SystemGetTime(dtCurrent);
	DWORD dif = dtCurrent - m_dtStartGathering;
	if (dif >= (DWORD)(m_nGatheringDuration))
	{
		SetEndGathering();
		return true;
	}

	return false;
}

// --------------------------------------------------------------------------
int CGathering::GetShiftX()
{
	return (m_bFullScreenGathering) ? SHIFT_X_FOR_FULL_SCREEN : 0;
}


////////////////////////////////////////////////////////////////////////////
//                        CConfGathering
////////////////////////////////////////////////////////////////////////////
CConfGathering::CConfGathering(CGatheringManager* pGatheringManager) : CGathering(pGatheringManager)
{
	m_nGatheringDuration = m_pGatheringManager->GetConfGatheringDuration();
}

// --------------------------------------------------------------------------
CConfGathering::~CConfGathering()
{
}

// --------------------------------------------------------------------------
bool CConfGathering::IsDisplayed()
{
	std::map<std::string, PARTY_DATA>::iterator it = m_mapPartiesData.begin();
	for (; it != m_mapPartiesData.end(); ++it)
	{
		if (!it->second.bDisplayed)
			return false;
	}

	return true;
}

// --------------------------------------------------------------------------
bool CConfGathering::IsPartyDisplayed(std::string& sParty)
{
	std::map<std::string, PARTY_DATA>::iterator it = m_mapPartiesData.find(sParty);
	if (it != m_mapPartiesData.end())
		return it->second.bDisplayed;

	return true;
}

// --------------------------------------------------------------------------
void CConfGathering::SetDisplayed(bool bDisplayed, const char* pszPartyName)
{
	m_mapPartiesData[pszPartyName].bDisplayed = bDisplayed;
	if (bDisplayed)
	{
		SetIsNeedFullRendering(false, pszPartyName);
		SystemGetTime(m_mapPartiesData[pszPartyName].dtLastShow);
		SetForceDisplay(false, pszPartyName);
	}
}

// --------------------------------------------------------------------------
bool CConfGathering::CheckTimeToShow(const char* pszPartyName)
{
	CStructTm dt;
	SystemGetTime(dt);

	DWORD dif = dt - m_mapPartiesData[pszPartyName].dtLastShow;

	char buff1[128] = "";
	char buff2[128] = "";
	m_mapPartiesData[pszPartyName].dtLastShow.DumpToBuffer(buff1);
	dt.DumpToBuffer(buff2);

	TRACEINTO
		<< "ConfName:"     << m_pConf->GetName()
		<< ", PartyName:"  << SAFEPATRYNAME
		<< ", LastShow:"   << buff1
		<< ", Current:"    << buff2
		<< ", Difference:" << dif;

	return (dif > 2);
}

// --------------------------------------------------------------------------
void CConfGathering::GetNotDisplayedParties(std::set<std::string>& setParties)
{
	std::map < std::string, PARTY_DATA >::iterator it = m_mapPartiesData.begin();
	for (; it != m_mapPartiesData.end(); ++it)
	{
		if (!it->second.bDisplayed)
		{
			setParties.insert(it->first);
		}
	}
}

// --------------------------------------------------------------------------
bool CConfGathering::IsNeedFullRendering(const char* pszPartyName)
{
	TRACECOND_AND_RETURN_VALUE(PARTYNOTFOUND, "Failed, Party not found, PartyName:" << SAFEPATRYNAME, true);

	TRACEINTO << "PartyName:" << SAFEPATRYNAME << ", NeedFullRendering:" << m_mapPartiesData[pszPartyName].bNeedFullRendering;
	return m_mapPartiesData[pszPartyName].bNeedFullRendering;
}

// --------------------------------------------------------------------------
void CConfGathering::SetIsNeedFullRendering(bool bNeedFullRendering, const char* pszPartyName)
{
	TRACECOND_AND_RETURN(PARTYNOTFOUND, "Failed, Party not found, PartyName:" << SAFEPATRYNAME);

	TRACEINTO << "PartyName:" << SAFEPATRYNAME << ", NeedFullRendering:" << bNeedFullRendering;
	m_mapPartiesData[pszPartyName].bNeedFullRendering = bNeedFullRendering;
}

// --------------------------------------------------------------------------
bool CConfGathering::IsHideGatheringText(const char* pszPartyName)
{
	TRACECOND_AND_RETURN_VALUE(PARTYNOTFOUND, "Failed, Party not found, PartyName:" << SAFEPATRYNAME, false);

	TRACEINTO << "PartyName:" << SAFEPATRYNAME << ", HideGatheringText:" << m_mapPartiesData[pszPartyName].bHideText;
	return m_mapPartiesData[pszPartyName].bHideText;
}

// --------------------------------------------------------------------------
void CConfGathering::SetIsHideGatheringText(bool bHideGatheringText, const char* pszPartyName)
{
	TRACECOND_AND_RETURN(PARTYNOTFOUND, "Failed, Party not found, PartyName:" << SAFEPATRYNAME);

	TRACEINTO << "PartyName:" << SAFEPATRYNAME << ", HideGatheringText:" << (int)bHideGatheringText;
	m_mapPartiesData[pszPartyName].bHideText = bHideGatheringText;
}

// --------------------------------------------------------------------------
EGatheringDisplayState CConfGathering::GetGatheringDisplayState(const char* pszPartyName)
{
	TRACECOND_AND_RETURN_VALUE(PARTYNOTFOUND, "Failed, Party not found, PartyName:" << SAFEPATRYNAME, eGatheringDisplayStateNone);

	TRACEINTO << "PartyName:" << SAFEPATRYNAME << ", GatheringDisplayState:" << m_mapPartiesData[pszPartyName].eDisplayState;
	return m_mapPartiesData[pszPartyName].eDisplayState;
}

// --------------------------------------------------------------------------
void CConfGathering::SetGatheringDisplayState(const char* pszPartyName, EGatheringDisplayState eState)
{
	TRACECOND_AND_RETURN(PARTYNOTFOUND, "Failed, Party not found, PartyName:" << SAFEPATRYNAME);

	TRACEINTO << "PartyName:" << SAFEPATRYNAME << ", GatheringDisplayState:" << eState;
	m_mapPartiesData[pszPartyName].eDisplayState = eState;
}

// --------------------------------------------------------------------------
bool CConfGathering::IsForceDisplay(const char* pszPartyName)
{
	TRACECOND_AND_RETURN_VALUE(PARTYNOTFOUND, "Failed, Party not found, PartyName:" << SAFEPATRYNAME, false);

	return m_mapPartiesData[pszPartyName].bForceDisplay;
}

// --------------------------------------------------------------------------
void CConfGathering::SetForceDisplay(bool bForceDisplay, const char* pszPartyName)
{
	TRACECOND_AND_RETURN(PARTYNOTFOUND, "Failed, Party not found, PartyName:" << SAFEPATRYNAME);

	m_mapPartiesData[pszPartyName].bForceDisplay = bForceDisplay;
}

// --------------------------------------------------------------------------
void CConfGathering::AddNewPartyData(std::string& sParty)
{
	PARTY_DATA pd;
	m_mapPartiesData[sParty] = pd;
}


////////////////////////////////////////////////////////////////////////////
//                        CPartGathering
////////////////////////////////////////////////////////////////////////////
CPartGathering::CPartGathering(CGatheringManager* pGatheringManager, const std::string& sPartName) : CGathering(pGatheringManager)
{
	m_sPartName                 = sPartName;
	m_nGatheringDuration        = m_pGatheringManager->GetPartGatheringDuration();
	m_bDisplayed                = false;
	m_bNeedFullRendering        = true;
	m_bHideText                 = false;
	m_bForceDisplay             = false;
	m_bCreatedFromConfGathering = false;
	m_bInitiatedByUser          = false;
	m_eDisplayState             = eGatheringDisplayStateNone;
}

// --------------------------------------------------------------------------
CPartGathering::CPartGathering(CConfGathering* pConfGathering, const std::string& sPartName)  : CGathering(pConfGathering->m_pGatheringManager)
{
	m_sPartName                 = sPartName;
	m_nGatheringDuration        = pConfGathering->m_nGatheringDuration;
	m_bDisplayed                = pConfGathering->IsDisplayed();
	m_bNeedFullRendering        = true;
	m_bHideText                 = false;
	m_bForceDisplay             = false;
	m_bCreatedFromConfGathering = true;
	m_bInitiatedByUser          = false;
	m_eDisplayState             = eGatheringDisplayStateNone;

	m_pConf                     = pConfGathering->m_pConf;
	m_bAutoLayoutPrev           = pConfGathering->m_bAutoLayoutPrev;
	m_bChangeLayout             = pConfGathering->m_bChangeLayout;
	m_dtStartGathering          = pConfGathering->m_dtStartGathering;
	m_bInitialized              = pConfGathering->m_bInitialized;
	m_bStarted                  = pConfGathering->m_bStarted;
	m_iShiftX                   = pConfGathering->m_iShiftX;
	m_bFullScreenGathering      = pConfGathering->m_bFullScreenGathering;
	m_nBackGroundTransparency   = pConfGathering->m_nBackGroundTransparency;
	m_pVideoLayoutPrev          = new CVideoLayout(*(pConfGathering->m_pVideoLayoutPrev));
}

// --------------------------------------------------------------------------
bool CPartGathering::IsNeedFullRendering(const char* pszPartyName)
{
	return m_bNeedFullRendering;
}

// --------------------------------------------------------------------------
void CPartGathering::SetDisplayed(bool bDisplayed, const char* pszPartyName)
{
	m_bDisplayed = bDisplayed;
	if (bDisplayed)
	{
		SetIsNeedFullRendering(false, pszPartyName);
		SystemGetTime(m_dtLastShow);
		SetForceDisplay(false, pszPartyName);
	}
}

// --------------------------------------------------------------------------
bool CPartGathering::CheckTimeToShow(const char* pszPartyName)
{
	CStructTm dt;
	SystemGetTime(dt);
	DWORD dif = dt - m_dtLastShow;

	char buff1[128] = "";
	char buff2[128] = "";
	m_dtLastShow.DumpToBuffer(buff1);
	dt.DumpToBuffer(buff2);

	TRACEINTO
		<< "ConfName:"     << m_pConf->GetName()
		<< ", PartyName:"  << m_sPartName.c_str()
		<< ", LastShow:"   << buff1
		<< ", Current:"    << buff2
		<< ", Difference:" << dif;

	return (dif > 2);
}


////////////////////////////////////////////////////////////////////////////
//                        CTokenUser
////////////////////////////////////////////////////////////////////////////
CGatheringManager::CGatheringManager(CConf* pConf)
{
	m_pConfGathering          = NULL;
	m_bStarted                = false;
	m_pGatheringVideoLayout   = NULL;
	m_pVideoLayoutPrev        = NULL;
	m_pVisualEffects          = NULL;
	m_pVisualEffectsPrev      = NULL;
	m_bAutoLayoutPrev         = false;
	m_bInitialized            = false;
	m_pConf                   = pConf;
	m_nVideoParticipants      = 0;
	m_nSecondsInCall          = 0;
	m_bGatheringEnabled       = m_bEndGathering = false;
	m_bRecordIndication       = false;
	m_bRecordLinkExists       = false;
	m_bNewPartyConnected      = false;
	m_bTimerStarted           = false;
	m_bConnectingTimerStarted = false;

	m_nConfGatheringDuration  = 180;
	m_nPartyGatheringDuration = 15;
	m_bConfPermanent          = false;

	m_bTelePresenceMode = false;
	m_bPresentationMode = false;
	m_bSameLayout       = false;

	m_sIP = "VIDEO: ";

	memset(m_sTimes, 0, STR_MAX_LEN);
	UpdateConfigData();
}

// --------------------------------------------------------------------------
CGatheringManager::~CGatheringManager()
{
	std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.begin();
	for (; itMap != m_mapPartGathering.end(); ++itMap)
	{
		if (itMap->second != m_pConfGathering)
			delete itMap->second;
	}

	PDELETE(m_pGatheringVideoLayout)
	PDELETE(m_pVisualEffects)
	PDELETE(m_pVisualEffectsPrev)
	PDELETE(m_pConfGathering);
}

// --------------------------------------------------------------------------
void CGatheringManager::Start()
{
	m_bStarted = true;
	Initialize();

	if (m_bGatheringEnabled)
	{
		if (!m_bPresentationMode && !m_bTelePresenceMode && !m_bSameLayout)
		{
			SetVisualEffectsParams(m_pVisualEffects);
			SetConfLayout(m_pGatheringVideoLayout, false);
		}
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::Initialize()
{
	if (m_bInitialized)
		return;

	m_bInitialized = true;

	SystemGetTime(m_dtStartConfMCU);
	m_dtStartGathering = m_dtStartConfMCU;

	const CCommConf* pCommConf = m_pConf->GetCommConf();
	bool             bEntryQ   = pCommConf->GetEntryQ() != NO;
	bool             bGW       = pCommConf->GetIsGateway() != NO;

	m_bGatheringEnabled = !bEntryQ && !bGW && pCommConf->IsGatheringEnabled();

	if (m_bGatheringEnabled)
	{
		string    sTmp;
		ELanguges eLanguage = pCommConf->GetLanguage();
		m_UnicodeStringTable.SetCurrentLanguage(eLanguage);

		m_bTelePresenceMode = pCommConf->GetIsTelePresenceMode();

		CLectureModeParams* pLectureMode = ((CCommConf*)pCommConf)->GetLectureMode();
		if (pLectureMode)
		{
			BYTE btLectureModeType = pLectureMode->GetLectureModeType();
			if (btLectureModeType != eLectureModeNone)
				m_bPresentationMode = true;
		}

		std::string sAppointmentID = pCommConf->GetAppointmentId();
		m_bSameLayout              = pCommConf->GetIsSameLayout() == YES;
		m_sConfName                = pCommConf->GetDisplayName();
		m_sOrganizer               = pCommConf->GetMeetingOrganizer();
		m_dtStartConf              = *(pCommConf->GetStartTime());
		m_dtExchangeConfStartTime  = *(pCommConf->GetExchangeConfStartTime());
		m_bConfPermanent           = pCommConf->IsPermanent();
		m_dtEndConf                = *(((CCommConf*)pCommConf)->GetEndTime());
		m_pConfGathering           = new CConfGathering(this);
		m_sFreeText1               = pCommConf->GetFreeText1();
		m_sFreeText2               = pCommConf->GetFreeText2();
		m_sFreeText3               = pCommConf->GetFreeText3();

		if (m_dtExchangeConfStartTime > m_dtStartConfMCU)
			m_nConfGatheringDuration += m_dtExchangeConfStartTime - m_dtStartConfMCU;

		if (m_bTelePresenceMode || m_bPresentationMode || m_bSameLayout)
		{
			m_pConfGathering->SetFullScreenGathering(true);
			m_pConfGathering->SetBackGroundTransparency(50);
		}

		CalculateDuration();
		sTmp = pCommConf->GetIpNumberAccess();
		if (!sTmp.empty())
			m_sIP += sTmp;
		else
			m_sIP = "";

		sTmp = pCommConf->GetNumberAccess1();
		if (!sTmp.empty())
			m_sISDN = pCommConf->GetNumberAccess1();

		sTmp = pCommConf->GetNumberAccess2();
		if (!sTmp.empty())
			m_sPSTN = pCommConf->GetNumberAccess2();

		m_pGatheringVideoLayout = CVideoLayout::CreateGatheringLayout();
		m_pVisualEffectsPrev    = new CVisualEffectsParams(((CCommConf*)pCommConf)->GetVisualEffects());
		m_pVisualEffects        = new CVisualEffectsParams(((CCommConf*)pCommConf)->GetVisualEffects());
		m_pVisualEffects->SetBackgroundImageID(3);

		m_bAutoLayoutPrev  = pCommConf->GetIsAutoLayout() != NO;
		m_pVideoLayoutPrev = ((CCommConf*)pCommConf)->GetVideoLayout();
		if (m_pVideoLayoutPrev == NULL)
			m_bAutoLayoutPrev = true;

		m_pConfGathering->SetIsAutoLayoutPrev(m_bAutoLayoutPrev);
		m_pConfGathering->SetVideoLayoutPrev(m_pVideoLayoutPrev);

		char buff1[128] = ""; m_dtStartConf.DumpToBuffer(buff1);
		char buff2[128] = ""; m_dtStartConfMCU.DumpToBuffer(buff2);
		char buff3[128] = ""; m_dtEndConf.DumpToBuffer(buff3);
		char buff4[128] = ""; m_dtExchangeConfStartTime.DumpToBuffer(buff4);

		TRACEINTO
			<< "  ConfName                :" << m_pConf->GetName()        << endl
			<< "  IsEntryQ                :" << bEntryQ                   << endl
			<< "  IsGateWay               :" << bGW                       << endl
			<< "  IsGatheringEnabled      :" << m_bGatheringEnabled       << endl
			<< "  IsTelePresenceMode      :" << m_bTelePresenceMode       << endl
			<< "  IsPresentationMode      :" << m_bPresentationMode       << endl
			<< "  IsSameLayout            :" << m_bSameLayout             << endl
			<< "  Organizer               :" << m_sOrganizer              << endl
			<< "  IP                      :" << m_sIP                     << endl
			<< "  AccessNumber1           :" << m_sISDN                   << endl
			<< "  AccessNumber2           :" << m_sPSTN                   << endl
			<< "  StartConfTime           :" << buff1                     << endl
			<< "  StartConfTimeMCU        :" << buff2                     << endl
			<< "  EndConfTime             :" << buff3                     << endl
			<< "  AppointmentID           :" << sAppointmentID            << endl
			<< "  ExchangeConfStartTime   :" << buff4                     << endl
			<< "  ConfGatheringDuration   :" << m_nConfGatheringDuration  << endl
			<< "  PartyGatheringDuration  :" << m_nPartyGatheringDuration << endl
			<< "  IsConfPermanent         :" << m_bConfPermanent          << endl
			<< "  eLanguage               :" << (int)eLanguage;
	}
	else
	{
		TRACEINTO
			<< "  ConfName                :" << m_pConf->GetName()        << endl
			<< "  IsEntryQ                :" << bEntryQ                   << endl
			<< "  IsGateWay               :" << bGW                       << endl
			<< "  IsGatheringEnabled      :" << m_bGatheringEnabled;
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::OnPartyLayoutChanged(const char* pszPartyName, CVideoLayout* pVideoLayout, DWORD param)
{
	PASSERT_AND_RETURN(!pVideoLayout);

	LayoutType layoutType = (LayoutType)pVideoLayout->GetScreenLayout();
	TRACEINTO << "PartyName:" << SAFEPATRYNAME << ", LayoutType:" << LayoutTypeAsString[layoutType];
}


// --------------------------------------------------------------------------
CVideoLayout* CGatheringManager::GetGatheringLayout(CConfParty* pConfParty /*= NULL*/)
{
	std::ostringstream msg;

	if (pConfParty)
		msg << "\n  PartyName           :" << pConfParty->GetName();

	CVideoLayout* pLayout = NULL;
	if (m_bGatheringEnabled)
	{
		m_bTelePresenceMode = m_pConf->GetCommConf()->GetIsTelePresenceMode();

		bool IsRecordingLinkParty = (pConfParty) ? pConfParty->GetRecordingLinkParty() : false;
		bool IsCascadeLinkParty   = (pConfParty) ? pConfParty->GetCascadeMode() != CASCADE_MODE_NONE : false;

		msg << "\n  IsPresentationMode  :" << m_bPresentationMode;
		msg << "\n  IsSameLayout        :" << m_bSameLayout;
		msg << "\n  IsTelePresenceMode  :" << m_bTelePresenceMode;
		msg << "\n  IsRecordingLinkParty:" << IsRecordingLinkParty;
		msg << "\n  IsCascadeLinkParty  :" << IsCascadeLinkParty;

		if (!m_bTelePresenceMode && !m_bPresentationMode && !m_bSameLayout && !IsRecordingLinkParty && !IsCascadeLinkParty)
		{
			if (pConfParty)
			{
				std::set<DWORD>::iterator it = m_setGatheringDonePartyID.find(pConfParty->GetPartyId());
				if (it != m_setGatheringDonePartyID.end())  // gathering was already done
				{
					msg << "\n  GatheringStatus     :Gathering was already done";
					TRACEINTO << msg.str().c_str();
					return NULL;
				}
				else
				{
					msg << "\n  GatheringStatus     :Gathering was not done yet";
				}
			}

			bool bIsNeedParticipantGathering  = IsNeedParticipantGathering();
			if (bIsNeedParticipantGathering)
			{
				msg << "  GatheringType       :Party Gathering, private layout";
				pLayout = m_pGatheringVideoLayout;
			}
			else
			{
				msg << "  GatheringType       :Conference Gathering, no need to private layout";
			}
		}
	}

	TRACEINTO << msg.str().c_str();
	return pLayout;
}

// --------------------------------------------------------------------------
CVisualEffectsParams* CGatheringManager::GetGatheringVisualEffects()
{
	m_bTelePresenceMode = m_pConf->GetCommConf()->GetIsTelePresenceMode();

	std::ostringstream msg;
	msg
		<< "IsPresentationMode:"   << m_bPresentationMode
		<< ", IsSameLayout:"       << m_bSameLayout
		<< ", IsTelePresenceMode:" << m_bTelePresenceMode;

	if (m_bTelePresenceMode || m_bPresentationMode || m_bSameLayout)
	{
		msg << ", ReturnValue:NULL";
		TRACEINTO << msg.str().c_str();
		return NULL;
	}

	msg << ", ReturnValue:" << uppercase << showbase << hex << (DWORD)m_pVisualEffects;
	TRACEINTO << msg.str().c_str();
	return m_pVisualEffects;
}

// --------------------------------------------------------------------------
void CGatheringManager::RestoreConfLayout()
{
	bool bPartGatheringExists = false;
	std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.begin();
	for (; itMap != m_mapPartGathering.end(); ++itMap)
	{
		if (itMap->second != m_pConfGathering)
		{
			bPartGatheringExists = true;
			break;
		}
	}

	if (bPartGatheringExists)
	{
		for (itMap = m_mapPartGathering.begin(); itMap != m_mapPartGathering.end(); ++itMap)
		{
			if (itMap->second == m_pConfGathering)
			{
				SetPartLayout(m_pVideoLayoutPrev, m_bAutoLayoutPrev, itMap->first);
			}
		}
	}
	else
	{
		SetConfLayout(m_pVideoLayoutPrev, m_bAutoLayoutPrev);
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::SetConfLayout(CVideoLayout* pLayout, bool bAutoLayout, bool bRestore1X4)
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();
	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

	if (bRestore1X4)
	{
		CVideoLayout* pVideoLayout1X4 = CVideoLayout::CreateNonGatheringLayout();
		if (pVideoLayout1X4)
		{
			TRACEINTO << "LayoutType:" << CVideoLayout::LayoutTypeToString(pVideoLayout1X4->GetScreenLayout());
			confApi.SetVideoConfLayoutSeeMeAll(*pVideoLayout1X4);
			PDELETE(pVideoLayout1X4);
		}
	}

	confApi.UpdateAutoLayout(bAutoLayout ? YES : NO);

	if (pLayout)
	{
		TRACEINTO << "LayoutType:" << CVideoLayout::LayoutTypeToString(pLayout->GetScreenLayout());
		confApi.SetVideoConfLayoutSeeMeAll(*pLayout, 1);
	}

	confApi.DestroyOnlyApi();
}

// --------------------------------------------------------------------------
void CGatheringManager::SetPartyToConfOrPrivateLayout(const std::string& sPartName)
{
	TRACEINTO << "PartyName:" << sPartName.c_str();

	const CCommConf* pCommConf = m_pConf->GetCommConf();
	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

	CVideoLayout* pConfVideoLayout = ((CCommConf*)pCommConf)->GetVideoLayout();
	PASSERT_AND_RETURN(!pConfVideoLayout);

	confApi.SetVideoConfLayoutSeeMeParty(sPartName.c_str(), *pConfVideoLayout);
	confApi.DestroyOnlyApi();
}

// --------------------------------------------------------------------------
void CGatheringManager::SetPartLayout(CVideoLayout* pLayout, bool bAutoLayout, const std::string& sPartName)
{
	TRACECOND_AND_RETURN(!pLayout, "Failed, invalid layout, PartyName:" << sPartName.c_str());

	const CCommConf* pCommConf = m_pConf->GetCommConf();
	CConfApi confApi;
	confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));

	string s;
	pLayout->ToString(s);
	TRACEINTO << "PartyName:" << sPartName.c_str() << "\n|" << s;

	confApi.SetVideoPrivateLayout(sPartName.c_str(), *pLayout);
	confApi.DestroyOnlyApi();
}

// --------------------------------------------------------------------------
void CGatheringManager::SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects)
{
	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	if (pVideoBridgeInterface)
		pVideoBridgeInterface->SetVisualEffectsParams(pVisualEffects);
}

// --------------------------------------------------------------------------
void CGatheringManager::SetVisualEffectsParams(CConfParty* pConfParty, CVisualEffectsParams* pVisualEffects)
{
	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	if (pVideoBridgeInterface)
	{
		std::string sPartName(pConfParty->GetName());
		TRACEINTO << "PartyName:" << sPartName.c_str();
		pVideoBridgeInterface->SetVisualEffectsParams(sPartName, pVisualEffects);
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::SetVisualEffectsParams(const std::string& sPartName, CVisualEffectsParams* pVisualEffects)
{
	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	if (pVideoBridgeInterface)
	{
		TRACEINTO << "PartyName:" << sPartName.c_str();
		pVideoBridgeInterface->SetVisualEffectsParams(sPartName, pVisualEffects);
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::StopGathering(const char* pszPartyName)
{
	std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.find(pszPartyName);
	TRACECOND_AND_RETURN(itMap == m_mapPartGathering.end(), "Failed, party not found, PartyName:" << pszPartyName);

	if (itMap->second == m_pConfGathering)
		itMap->second = new CPartGathering(this, pszPartyName);

	TRACEINTO << "PartyName:" << pszPartyName;

	CPartGathering* pPartyGathering = (CPartGathering*)(itMap->second);
	pPartyGathering->SetEndGathering();

	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	if (pVideoBridgeInterface)
		pVideoBridgeInterface->DisplayGatheringOnScreen(this);

	PDELETE(itMap->second);
	m_mapPartGathering.erase(itMap);
}

// --------------------------------------------------------------------------
void CGatheringManager::ShowGatheringToParty(const char* pszPartName)
{
	if (m_bGatheringEnabled)
	{
		TRACEINTO << "PartName:" << pszPartName;

		std::string sPartName(pszPartName);
		UpdateParticipantsList();

		std::set<std::string> setParticipants;
		FillParticipantsSet(setParticipants);

		RemoveNotExistingParticipantsFromMapPartGathering(setParticipants);

		if (m_mapPartGathering.find(sPartName) == m_mapPartGathering.end())
		{
			if (IsNeedParticipantGathering())
			{
				CPartGathering* pPartGathering = new CPartGathering(this, sPartName);
				pPartGathering->SetIsInitiatedByUser(true);
				pPartGathering->SetFullScreenGathering(true);
				pPartGathering->SetBackGroundTransparency(50);
				m_mapPartGathering[sPartName] = pPartGathering;
				pPartGathering->SetIsNeedToChangeLayout(false);

				CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
				if (pVideoBridgeInterface)
					pVideoBridgeInterface->DisplayGatheringOnScreen(this);

				if (!m_bTimerStarted)
				{
					m_pConf->StartGatheringUpdateTimer();
					m_bTimerStarted = true;
				}
			}
		}
	}
}

CGathering* CGatheringManager::GetGathering(const char* pszPartyName)
{
	std::string sPartName(pszPartyName);
	std::map<std::string, CGathering*>::iterator it = m_mapPartGathering.find(sPartName);
	if (it == m_mapPartGathering.end())
		return NULL;
	else
		return it->second;
}
// --------------------------------------------------------------------------
void CGatheringManager::RemoveNotExistingParticipantsFromMapPartGathering(const std::set<std::string>& setParticipants)
{
	if (!m_mapPartGathering.empty())
	{
		bool bFind = true;
		while (bFind)
		{
			bFind = false;
			std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.begin();
			for (; itMap != m_mapPartGathering.end(); ++itMap)
			{
				if (setParticipants.find(itMap->first) == setParticipants.end())
				{
					bFind = true;
					const CCommConf* pCommConf = m_pConf->GetCommConf();
					CConfParty*      pParty    = pCommConf->GetCurrentParty(itMap->first.c_str());
					if (IsValidPObjectPtr(pParty))
						OnPartyDisConnected(pParty);

					if ((itMap->second != NULL) &&(itMap->second != m_pConfGathering))
						PDELETE(itMap->second);

					TRACEINTO << "PartyName:" << itMap->first.c_str();
					m_mapPartGathering.erase(itMap);
					break;
				}
			}
		}
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::OnUpdateByTimer()
{
	CStructTm dtCurrent;
	SystemGetTime(dtCurrent);

	std::set<std::string> setParticipants;

	m_bTimerStarted = false;

	bool bStartConfTimer   = false;
	bool bStartPartTimer   = false;
	bool bEndGathering     = false;
	bool bRecordLinkExists = FillParticipantsSet(setParticipants);

	RemoveNotExistingParticipantsFromMapPartGathering(setParticipants);

	const CCommConf* pCommConf = m_pConf->GetCommConf();
	m_bTelePresenceMode = pCommConf->GetIsTelePresenceMode();
	if (m_pConfGathering && !m_pConfGathering->IsEndGathering())
	{
		if (m_pConfGathering->IsGatheringTimeout())
		{
			TRACEINTO << "Conference Gathering timeout, ConfName:" << m_pConf->GetName();
			if (!m_bTelePresenceMode && !m_bPresentationMode && !m_bSameLayout)
			{
				SetVisualEffectsParams(m_pVisualEffectsPrev);
				CVideoLayout* pChangedCondLayout = IsConfLayoutChangedDuringGathering();
				if (pChangedCondLayout)
				{
					DWORD dwProfieBackgroundImageID = m_pVisualEffectsPrev->GetBackgroundImageID();
					TRACEINTO << "ProfieBackgroundImageID:" << dwProfieBackgroundImageID;
					if (dwProfieBackgroundImageID == 0)
						SetConfLayout(pChangedCondLayout, pCommConf->GetIsAutoLayout(), true);
				}
				else
					SetConfLayout(m_pConfGathering->GetVideoLayoutPrev(), m_pConfGathering->GetIsAutoLayoutPrev(), true);
			}

			bEndGathering = true;
		}
		else
		{
			bStartConfTimer = true;
		}
	}

	std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.begin();
	for (; itMap != m_mapPartGathering.end(); ++itMap)
	{
		if (itMap->second != m_pConfGathering)
		{
			CConfParty* pParty = pCommConf->GetCurrentParty(itMap->first.c_str());
			if (pParty)
			{
				DWORD dwPartyID = pParty->GetPartyId();

				TRACEINTO << "Party already had Gathering, PartyName:" << pParty->GetName() << ", PartyId:" << (int)dwPartyID;

				m_setGatheringDonePartyID.insert(dwPartyID);

				CVideoLayout* pVideoLayout          = pParty->GetVideoLayout();
				CVideoLayout* pVideoPartyConfLayout = pParty->GetVideoPartyConfLayout();

				if (pVideoLayout)
				{
					string s;
					pVideoLayout->ToString(s);
					TRACEINTO << "VideoLayout:\n" << s.c_str();
				}

				if (pVideoPartyConfLayout)
				{
					string s;
					pVideoPartyConfLayout->ToString(s);
					TRACEINTO << "VideoPartyConfLayout:\n" << s.c_str();
				}

				if (itMap->second->IsGatheringTimeout())
				{
					TRACEINTO << "Participant Gathering timeout, PartyName:" << itMap->first.c_str();
					((CPartGathering*)(itMap->second))->SetIsCreatedFromConfGathering(false);
					if (!m_bTelePresenceMode && !m_bPresentationMode && !m_bSameLayout && itMap->second->GetIsNeedToChangeLayout())
					{
						CConfApi confApi;
						confApi.CreateOnlyApi(*(pCommConf->GetRcvMbx()));
						CVideoLayout* p1x4 = CVideoLayout::CreateNonGatheringLayout();
						confApi.SetVideoPrivateLayout(itMap->first.c_str(), *p1x4);
						confApi.DestroyOnlyApi();
						PDELETE(p1x4)

						SetVisualEffectsParams(itMap->first, ((CCommConf*)pCommConf)->GetVisualEffects());

						if ((!pVideoLayout) || (pVideoLayout && pVideoLayout->IsGatheringLayout()))
							SetPartyToConfOrPrivateLayout(itMap->first);         // VNGR-24155 - Fix memory corruption due to changes made in VNGR-16621
					}
					bEndGathering = true;
				}
				else
				{
					bStartPartTimer = true;
				}
			}
		}
	}

	if (bStartConfTimer || bStartPartTimer || bEndGathering)
	{
		UpdateParticipantsList(true);

		if (bStartConfTimer || bStartPartTimer)
		{
			m_pConf->StartGatheringUpdateTimer();
			m_bTimerStarted = true;
		}
	}

	TRACEINTO
		<< "bStartConfTimer:"       << bStartConfTimer
		<< ", bStartPartTimer:"     << bStartPartTimer
		<< ", bEndGathering:"       << bEndGathering
		<< ", bRecordLinkExists:"   << bRecordLinkExists
		<< ", m_bRecordLinkExists:" << m_bRecordLinkExists;

	m_bRecordIndication = bRecordLinkExists;

	if (m_bNewPartyConnected || setParticipants != m_setGatheringParticipants || bRecordLinkExists != m_bRecordLinkExists || bEndGathering)
	{
		m_bNewPartyConnected = false;
		m_bRecordLinkExists  = bRecordLinkExists;
		CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
		if (pVideoBridgeInterface)
			pVideoBridgeInterface->DisplayGatheringOnScreen(this);

		if (setParticipants != m_setGatheringParticipants)
		{
			m_setGatheringParticipants.clear();
			m_setGatheringParticipants.insert(setParticipants.begin(), setParticipants.end());
		}
	}

	// remove ended gathering parts from m_mapPartGathering
	bool       bFind = true;
	CMedString sLogRemove;
	CMedString sLogRetry;
	while (bFind)
	{
		bFind = false;
		std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.begin();
		for (; itMap != m_mapPartGathering.end(); ++itMap)
		{
			if (itMap->second->IsEndGathering())
			{
				if (itMap->second == m_pConfGathering)
				{
					std::string sName = itMap->first;
					if (m_pConfGathering->IsPartyDisplayed(sName))
					{
						bFind = true;
						sLogRemove << "\t\t" << itMap->first.c_str() << "\t\tm_pConfGathering\n";
					}
					else
						sLogRetry << "\t\t" << itMap->first.c_str() << "\t\tm_pConfGathering\n";
				}
				else
				{
					char buff[20] = "";
					sprintf(buff, "%p", itMap->second);
					if (itMap->second->IsDisplayed())
					{
						bFind = true;
						sLogRemove << "\t\t" << itMap->first.c_str() << "\t\tpPartGathering = " << buff << "\n";
						PDELETE(itMap->second);
					}
					else
						sLogRetry << "\t\t" << itMap->first.c_str() << "\t\tpPartGathering = " << buff << "\n";
				}

				if (bFind)
				{
					m_mapPartGathering.erase(itMap);
					break;
				}
			}
		}
	}

	// vngr-16864 - TBD by Boris
	// if (m_pConfGathering && m_pConfGathering->IsGatheringTimeout() && m_pConfGathering->IsDisplayed())
	// PDELETE(m_pConfGathering);

	if (sLogRemove.GetStringLength() > 0)
		TRACEINTO << "Remove parties from m_mapPartGathering:\n" << sLogRemove.GetString();

	if (sLogRetry.GetStringLength() > 0)
		TRACEINTO << "Remove parties from m_mapPartGathering:\n" << sLogRetry.GetString();

	if (IsNeedReDisplayGathering())
	{
		ReDisplayGathering();
		if (!m_bTimerStarted)
		{
			m_pConf->StartGatheringUpdateTimer();
			m_bTimerStarted = true;
		}
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::OnPartyDisConnected(CConfParty* pConfParty)
{
	DWORD dwPartyID = pConfParty->GetPartyId();

	std::ostringstream msg;
	msg << "PartyId:" << (int)dwPartyID;

	std::set<DWORD>::iterator it = m_setConnectedPartyID.find(dwPartyID);
	if (it != m_setConnectedPartyID.end())
	{
		m_setConnectedPartyID.erase(it);
		msg << ", removed from 'setConnectedPartyID'";
	}

	std::set<DWORD>::iterator itGatheringDone = m_setGatheringDonePartyID.find(dwPartyID);
	if (itGatheringDone != m_setGatheringDonePartyID.end())
	{
		m_setGatheringDonePartyID.erase(itGatheringDone);
		msg << ", removed from 'setGatheringDonePartyID'";
	}

    pConfParty->RemoveAllLayouts();//added for BRIDGE-13167

	TRACEINTO << msg.str().c_str();
}

// --------------------------------------------------------------------------
void CGatheringManager::OnPartyConnecting(CConfParty* pConfParty)
{
	if (!m_bGatheringEnabled)
		return;

	const DWORD partyId   = pConfParty->GetPartyId();
	const char* partyName = pConfParty->GetName();

	TRACEINTO << "PartyName:" << partyName << ", PartyId:" << (int)partyId;

	if (m_setConnectedPartyID.insert(partyId).second == false)
	{
		// Party already exist in 'ConnectedPartyID' set, so just redisplay gathering text
		ReDisplayGatheringText(partyName);
		return;
	}

	if (!m_bInitialized)
		Initialize();

	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	PASSERT_AND_RETURN(!pVideoBridgeInterface);

	if (pVideoBridgeInterface->IsBridgePartyVideoOutStateIsConnected(partyName))
	{
		OnPartyConnected(pConfParty);
	}
	else
	{
		m_vConnectingParties.push_back(pair<CConfParty*, int> (pConfParty, 0));

		TRACEINTO
			<< "Party not connected yet"
			<< ", PartyName:"               << partyName
			<< ", PartyId:"                 << (int)partyId
			<< ", bConnectingTimerStarted:" << m_bConnectingTimerStarted;

		if (!m_bConnectingTimerStarted)
		{
			m_pConf->StartGatheringPartyConnectingTimer();
			m_bConnectingTimerStarted = true;
		}
	}
}

// --------------------------------------------------------------------------
bool IsNull(pair<CConfParty*, int> pr)
{
	return pr.first == NULL;
}

// --------------------------------------------------------------------------
void CGatheringManager::OnConnectingTimer()
{
	if (!m_bGatheringEnabled)
		return;

	m_bConnectingTimerStarted = false;
	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	PASSERT_AND_RETURN(!pVideoBridgeInterface);

	std::vector<pair<CConfParty*, int> >::iterator it = m_vConnectingParties.begin();
	bool bNotConnectedExists = false;
	for (; it != m_vConnectingParties.end(); ++it)
	{
		if (it->first == NULL)
			continue;

		if (IsValidPObjectPtr(it->first))
		{
			if (pVideoBridgeInterface->IsBridgePartyVideoOutStateIsConnected(it->first->GetName()))
			{
				OnPartyConnected(it->first);
				it->first = NULL;
			}
			else
			{
				++it->second;
				if (it->second > 20)
					it->first = NULL;

				bNotConnectedExists = true;
			}
		}
		else
			it->first = NULL;
	}

	TRACEINTO << "bConnectingTimerStarted:" << bNotConnectedExists;

	if (bNotConnectedExists)
	{
		m_pConf->StartGatheringPartyConnectingTimer();
		m_bConnectingTimerStarted = true;
	}

	m_vConnectingParties.erase(remove_if(m_vConnectingParties.begin(), m_vConnectingParties.end(), IsNull), m_vConnectingParties.end());
}

// --------------------------------------------------------------------------
void CGatheringManager::ReDisplayGatheringText(const char* pPartyName)
{
	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	PASSERT_AND_RETURN(!pVideoBridgeInterface);

	PASSERT_AND_RETURN(!pPartyName);

	TRACECOND_AND_RETURN(m_mapPartGathering.find(pPartyName) == m_mapPartGathering.end(), "Failed, party not found, PartyName:" << pPartyName);

	m_mapPartGathering[pPartyName]->SetForceDisplay(true, pPartyName);
	m_mapPartGathering[pPartyName]->SetIsNeedFullRendering(true, pPartyName);
	m_mapPartGathering[pPartyName]->SetIsHideGatheringText(false, pPartyName);
	pVideoBridgeInterface->DisplayGatheringOnScreen(pPartyName, this);
}

// --------------------------------------------------------------------------
void CGatheringManager::HideGatheringText(const char* pPartyName)
{
	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	PASSERT_AND_RETURN(!pVideoBridgeInterface);

	PASSERT_AND_RETURN(!pPartyName);

	TRACECOND_AND_RETURN(m_mapPartGathering.find(pPartyName) == m_mapPartGathering.end(), "Failed, party not found, PartyName:" << pPartyName);

	m_mapPartGathering[pPartyName]->SetIsHideGatheringText(true, pPartyName);
	pVideoBridgeInterface->DisplayGatheringOnScreen(pPartyName, this);
}

// --------------------------------------------------------------------------
void CGatheringManager::OnPartyConnected(CConfParty* pConfParty)
{
	if (!m_bGatheringEnabled)
		return;

	std::ostringstream msg;
	msg
		<< "ConfName:"     << m_pConf->GetName()
		<< ", ConfId:"     << m_pConf->GetConfId()
		<< ", PartyName:"  << pConfParty->GetName()
		<< ", PartyId:"    << pConfParty->GetPartyId()
		<< ", PartyState:" << pConfParty->GetPartyState();

	const CCommConf* pCommConf = m_pConf->GetCommConf();
	m_bTelePresenceMode = pCommConf->GetIsTelePresenceMode();
	if (m_bTelePresenceMode)
	{
		if (pConfParty->GetTelePresenceMode() != NONE)
		{
			char* pVisualName = pConfParty->GetRemoteName();
			if (pVisualName)
			{
				std::string sVisualName(pVisualName);
				if (sVisualName.size() > 0 && sVisualName.substr(sVisualName.size()-1, 1) != "1")
				{
					msg << ", TelePresence:" << sVisualName.c_str();
					TRACEINTO << msg.str().c_str();
					return;
				}
			}
		}
	}

	TRACEINTO << msg.str().c_str();

	std::string sAppointmentID = pCommConf->GetAppointmentId();
	if (!sAppointmentID.empty() && m_sOrganizer.empty())
	{
		m_sConfName               = pCommConf->GetDisplayName();
		m_sOrganizer              = pCommConf->GetMeetingOrganizer();
		m_dtExchangeConfStartTime = *(pCommConf->GetExchangeConfStartTime());

		if (m_dtExchangeConfStartTime > m_dtStartConfMCU)
		{
			m_nConfGatheringDuration += m_dtExchangeConfStartTime - m_dtStartConfMCU;
		}

		std::string sTmp = pCommConf->GetNumberAccess1();
		if (!sTmp.empty())
			m_sISDN = pCommConf->GetNumberAccess1();

		sTmp = pCommConf->GetNumberAccess2();
		if (!sTmp.empty())
			m_sPSTN = pCommConf->GetNumberAccess2();

		char buff[128] = "";
		m_dtExchangeConfStartTime.DumpToBuffer(buff);
		TRACEINTO
			<< "Exchange data:\n"
			<< "  ConfName      :" << m_sConfName << "\n"
			<< "  Organizer     :" << m_sOrganizer << "\n"
			<< "  AccessNumber1 :" << m_sISDN << "\n"
			<< "  AccessNumber2 :" << m_sPSTN << "\n"
			<< "  ConfStartTime :" << buff;
	}

	std::string sNewParty(pConfParty->GetName());

	if (m_mapPartGathering.find(sNewParty) != m_mapPartGathering.end())
	{
		if (m_mapPartGathering[sNewParty] != m_pConfGathering)
		{
			PDELETE(m_mapPartGathering[sNewParty])
			m_mapPartGathering.erase(sNewParty);
		}
	}

	UpdateParticipantsList();

	std::set<std::string> setParticipants;
	bool bRecordLinkExists = FillParticipantsSet(setParticipants);
	if (setParticipants != m_setGatheringParticipants)
	{
		m_bRecordLinkExists = bRecordLinkExists;
		m_setGatheringParticipants.clear();
		m_setGatheringParticipants.insert(setParticipants.begin(), setParticipants.end());
		m_bNewPartyConnected = true;
	}

	RemoveNotExistingParticipantsFromMapPartGathering(setParticipants);

	if (IsNeedParticipantGathering())
	{
		TRACECOND_AND_RETURN(pConfParty->GetRecordingLinkParty(), "Recording link not need Gathering, PartyName:" << pConfParty->GetName());

		CPartGathering* pPartGathering = new CPartGathering(this, sNewParty);
		if (m_bTelePresenceMode || m_bPresentationMode || m_bSameLayout)
		{
			TRACEINTO
				<< "Using participant Gathering"
				<< ", PartyName:"         << pConfParty->GetName()
				<< ", pPartGathering:"    << uppercase << showbase << hex << (DWORD)(m_mapPartGathering[sNewParty])
				<< ", bTelePresenceMode:" << m_bTelePresenceMode
				<< ", bPresentationMode:" << m_bPresentationMode
				<< ", bSameLayout:"       << m_bSameLayout;

			pPartGathering->SetFullScreenGathering(true);
			pPartGathering->SetBackGroundTransparency(50);
		}
		else
		{
			TRACEINTO
				<< "Using participant Gathering"
				<< ", PartyName:"         << pConfParty->GetName()
				<< ", pPartGathering:"    << uppercase << showbase << hex << (DWORD)(m_mapPartGathering[sNewParty]);
		}

		m_mapPartGathering[sNewParty] = pPartGathering;

		CVideoLayout* pGatheringLayout = GetGatheringLayout(pConfParty);
		if (pGatheringLayout && IsValidPObjectPtr(pGatheringLayout))
		{
			CVisualEffectsParams* pParams = GetGatheringVisualEffects();
			if (pParams && IsValidPObjectPtr(pParams))
				SetVisualEffectsParams(pConfParty, pParams);
		}

		pPartGathering->SetIsNeedToChangeLayout(!(m_bTelePresenceMode || m_bPresentationMode || m_bSameLayout));
	}
	else
	{
		TRACEINTO
			<< "Using conference Gathering"
			<< ", PartyName:"       << pConfParty->GetName()
			<< ", pConfGathering:"  << uppercase << showbase << hex << (DWORD)m_pConfGathering;

		m_mapPartGathering[sNewParty] = m_pConfGathering;
		m_pConfGathering->AddNewPartyData(sNewParty);
	}

	if (!m_bStarted)
		Start();

	if (!m_mapPartGathering.empty())
	{
		CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
		if (pVideoBridgeInterface)
		{
			pVideoBridgeInterface->DisplayGatheringOnScreen(pConfParty->GetName(), this);
			// insert to m_setGatheringDonePartyID
			m_setGatheringDonePartyID.insert(pConfParty->GetPartyId());
		}

		if (!m_bTimerStarted)
		{
			if (IsNeedReDisplayGathering())
				m_pConf->StartGatheringUpdateTimer(GATHERING_UPDATE_TIMER_1_SEC);
			else
				m_pConf->StartGatheringUpdateTimer(GATHERING_UPDATE_TIMER_5_SEC);

			m_bTimerStarted = true;
		}

		if (m_mapPartGathering[sNewParty] == m_pConfGathering)
			m_mapPartGathering[sNewParty]->SetIsNeedFullRendering(!m_pConfGathering->IsPartyDisplayed(sNewParty), sNewParty.c_str());
		else
			m_mapPartGathering[sNewParty]->SetIsNeedFullRendering(!((CPartGathering*)m_mapPartGathering[sNewParty])->IsDisplayed(), "");
	}
}

// --------------------------------------------------------------------------
bool CGatheringManager::IsNeedReDisplayGathering()
{
	std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.begin();
	for (; itMap != m_mapPartGathering.end(); ++itMap)
		if (itMap->second && (!itMap->second->IsDisplayed()))
			return true;

	return false;
}

// --------------------------------------------------------------------------
void CGatheringManager::ReDisplayGathering()
{
	CVideoBridgeInterface* pVideoBridgeInterface = m_pConf->GetVideoBridgeInterface();
	PASSERT_AND_RETURN(!pVideoBridgeInterface);

	std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.begin();
	for (; itMap != m_mapPartGathering.end(); ++itMap)
	{
		if (m_pConfGathering && itMap->second == m_pConfGathering)
		{
			std::set<std::string> setParties;
			m_pConfGathering->GetNotDisplayedParties(setParties);
			if (!setParties.empty())
			{
				std::set<std::string>::iterator it = setParties.begin();
				for (; it != setParties.end(); ++it)
				{
					pVideoBridgeInterface->DisplayGatheringOnScreen(it->c_str(), this);
				}
			}
		}
		else if (!itMap->second->IsDisplayed())
		{
			pVideoBridgeInterface->DisplayGatheringOnScreen(itMap->first.c_str(), this);
		}
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::UpdateConfigData()
{
	string sTmp;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	pSysConfig->GetDataByKey(CONF_GATHERING_DURATION_SECONDS, sTmp);
	m_nConfGatheringDuration = atoi(sTmp.c_str());
	pSysConfig->GetDataByKey(PARTY_GATHERING_DURATION_SECONDS, sTmp);
	m_nPartyGatheringDuration = atoi(sTmp.c_str());
}

// --------------------------------------------------------------------------
CVideoLayout* CGatheringManager::IsConfLayoutChangedDuringGathering()
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();
	CVideoLayout* pConfVideoLayout = ((CCommConf*)pCommConf)->GetVideoLayout();
	if (pConfVideoLayout != NULL && !pConfVideoLayout->IsGatheringLayout())
	{
		std::string sLayout;
		pConfVideoLayout->ToString(sLayout);
		TRACEINTO << "ConfLayout:\n" << sLayout.c_str();
		return pConfVideoLayout;
	}

	return NULL;
}

// --------------------------------------------------------------------------
bool CGatheringManager::IsNeedParticipantGathering()
{
	if (m_nConfGatheringDuration <= m_nPartyGatheringDuration)
		return true;

	CStructTm dtCurrent;
	SystemGetTime(dtCurrent);
	DWORD     dif = dtCurrent - m_dtStartGathering;
	if (dif > (DWORD)(m_nConfGatheringDuration - m_nPartyGatheringDuration))
		return true;

	return false;
}

// --------------------------------------------------------------------------
bool CGatheringManager::FillParticipantsSet(std::set<std::string>& setParticipants)
{
	setParticipants.clear();

	const CCommConf* pCommConf         = m_pConf->GetCommConf();
	CConfParty*      pConfParty        = ((CCommConf*)pCommConf)->GetFirstParty();
	bool             bRecordLinkExists = false;

	if (!pConfParty)
	{
		TRACEINTO << "No parties in this conference, ConfName:" << pCommConf->GetName();
	}
	else
	{
		int numParties = pCommConf->GetNumParties();
		for (WORD i = 0; i < numParties; i++)
		{
			if (pConfParty == NULL)
			{
				PASSERT(1);
				break;
			}

			DWORD dwState = pConfParty->GetPartyState();
			if (dwState == PARTY_CONNECTED || dwState == PARTY_SECONDARY || dwState == PARTY_CONNECTED_WITH_PROBLEM)
			{
				if (pConfParty->GetRecordingLinkParty())
					bRecordLinkExists = true;

				std::string sName(pConfParty->GetName());
				if (m_bTelePresenceMode && pConfParty->GetTelePresenceMode() != NONE)
				{
					std::string sRemoteName(pConfParty->GetRemoteName());
					if (!sRemoteName.empty())
					{
						if (sRemoteName.substr(sRemoteName.size()-1, 1) == "1")
							setParticipants.insert(sName);
					}
				}
				else
				{
					setParticipants.insert(sName);
				}
			}

			pConfParty = ((CCommConf*)pCommConf)->GetNextParty();
		}
	}

	return bRecordLinkExists;
}

// --------------------------------------------------------------------------
void CGatheringManager::UpdateParticipantsList(bool bDumpToLog)
{
	m_vParticipants.clear();

	m_bRecordIndication  = false;
	m_nVideoParticipants = 0;

	const CCommConf* pCommConf  = m_pConf->GetCommConf();
	CConfParty*      pConfParty = ((CCommConf*)pCommConf)->GetFirstParty();
	int              numParties = pCommConf->GetNumParties();

	TRACECOND_AND_RETURN((!pConfParty || numParties == 0), "Participants list is empty");

	std::ostringstream msg;
	msg.precision(0);
	msg << endl;
	msg << " ----+-------+--------------------------------+--------------------------------" << endl;
	msg << " Id  | Type  | Visual Name                    | Remote Name"                     << endl;
	msg << " ----+-------+--------------------------------+--------------------------------" << endl;

	for (WORD i = 0; i < numParties; i++)
	{
		PASSERT(!pConfParty);

		if (pConfParty)
		{
			DWORD dwState  = pConfParty->GetPartyState();
			bool  bPartRec = pConfParty->GetRecordingLinkParty() != NO;

			if (bPartRec)
				m_bRecordIndication = true;

			// VNGFE_8038
			BOOL isLyncCcs = FALSE;
			BOOL isTipSlave = FALSE;
			BOOL isCascadeLink = FALSE;

			if (pConfParty->GetRemoteIdent() == MicrosoftEP_Lync_CCS)
			{
				TRACEINTO << "Lync_CCS: " << pConfParty->GetName();
				
				isLyncCcs = TRUE;
			}

			if (pConfParty->IsTIPSlaveParty())
			{
				TRACEINTO << "TIPSlave: " << pConfParty->GetName();
				
				isTipSlave = TRUE;
			}

			if (pConfParty->GetCascadeMode() != CASCADE_MODE_NONE &&
                             !pConfParty->GetIsCallFromGW())
			{
				TRACEINTO << "CascadeLink: " << pConfParty->GetName();
				
				isCascadeLink = TRUE;
			}

			if ((dwState == PARTY_CONNECTED || dwState == PARTY_SECONDARY || dwState == PARTY_CONNECTED_WITH_PROBLEM) 
				&& !bPartRec && !isLyncCcs && !isTipSlave && !isCascadeLink)
			{
				std::string sVisualName(pConfParty->GetVisualPartyName());
				std::string sRemoteName(pConfParty->GetRemoteName());

				if (m_bTelePresenceMode && pConfParty->GetTelePresenceMode() != NONE)  // Telepresence EP
				{
					if (!sRemoteName.empty())
					{
						if (sRemoteName.substr(sRemoteName.size()-1, 1) == "1")
							sVisualName = sRemoteName.substr(0, sRemoteName.size()-1);
						else
						{
							msg << " " << setw(3) << right << (WORD)pConfParty->GetPartyId() << " |"
							    << " " << setw(5) << left  << "TV P" << " |"              // Telepresence EP
							    << " " << setw(30) << left  << sVisualName.c_str() << " |"
							    << " " <<             left  << sRemoteName.c_str() << endl;

							pConfParty = ((CCommConf*)pCommConf)->GetNextParty();
							continue;
						}
					}
				}

				if (sVisualName.empty())
					sVisualName = pConfParty->GetName();

				bool                         bVideo = pConfParty->IsVideo_Member() == TRUE;
				std::pair<std::string, bool> part(sVisualName, bVideo);
				m_vParticipants.push_back(part);
				if (bVideo)
					++m_nVideoParticipants;

				msg << " " << setw(3) << right << (WORD)pConfParty->GetPartyId() << " |"
				    << " " << setw(5) << left  << (bVideo ? "Video" : "Audio") << " |"
				    << " " << setw(30) << left  << sVisualName.c_str() << " |"
				    << " " <<             left  << sRemoteName.c_str() << endl;
			}
		}

		pConfParty = ((CCommConf*)pCommConf)->GetNextParty();
	}

	msg << " ----+-------+--------------------------------+--------------------------------";
	if (bDumpToLog)
		TRACEINTO << msg.str().c_str();
}

// --------------------------------------------------------------------------
int CGatheringManager::GetExceptionPartList(std::set<std::string>& setParts)
{
	int setPartsNumber = 0;
	std::map < std::string, CGathering* >::iterator itMap = m_mapPartGathering.begin();
	for (; itMap != m_mapPartGathering.end(); ++itMap)
	{
		if (itMap->second != m_pConfGathering)
		{
			if (!((CPartGathering*)(itMap->second))->IsCreatedFromConfGathering())
			{
				setParts.insert(itMap->first);
				++setPartsNumber;
			}
		}
	}

	return setPartsNumber;
}

// --------------------------------------------------------------------------
void CGatheringManager::AddParticipant(const char* sName, bool bVideo)
{
	std::pair<std::string, bool> part(sName, bVideo);
	m_vParticipants.push_back(part);
	if (bVideo)
		++m_nVideoParticipants;
}

// --------------------------------------------------------------------------
void CGatheringManager::RemoveParticipant(const char* sName)
{
	std::vector<std::pair<std::string, bool> >::iterator it = m_vParticipants.begin();
	for (; it != m_vParticipants.end(); ++it)
	{
		if (it->first == sName)
		{
			if (it->second)
				--m_nVideoParticipants;

			m_vParticipants.erase(it);
			break;
		}
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::ClearParticipants()
{
	m_vParticipants.clear();
	m_nVideoParticipants = 0;
}

// --------------------------------------------------------------------------
void CGatheringManager::GetRecStr(char* buff, int nBuffSize)
{
	if (m_bRecordIndication)
	{
		m_UnicodeStringTable.TranslateUTF8ToUCS2("REC", buff, nBuffSize);
	}
	else
	{
		buff[0] = '\0';
		buff[1] = '\0';
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::GetConfNameStr(char* buff, int nBuffSize)
{
	m_UnicodeStringTable.TranslateUTF8ToUCS2(m_pConf->GetName(), buff, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetOrganizerStr(char* buff, int nBuffSize)
{
	if (m_sOrganizer.empty())
	{
		buff[0] = '\0';
		buff[1] = '\0';
		return;
	}

	WORD        n = m_UnicodeStringTable.GetBytes(STR_ORGANIZER, buff);
	std::string s(": ");
	s += m_sOrganizer;
	m_UnicodeStringTable.TranslateUTF8ToUCS2(s.c_str(), buff + n, STR_MAX_LEN - n);

	char dump[1024] = "";
	m_UnicodeStringTable.DumpBuffer(buff, nBuffSize, dump, 1024);
	TRACEINTO << "Organizer:" << dump;
}

// --------------------------------------------------------------------------
void CGatheringManager::GetTimesStr(char* buff, int nBuffSize)
{
	memcpy(buff, m_charrConfDuration, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::CalculateDuration()
{
	memset(m_charrConfDuration, 0, STR_MAX_LEN);
	char buff[15];

	WORD n = m_UnicodeStringTable.GetBytes(STR_DURATION, m_charrConfDuration);

	if (m_bConfPermanent)
	{
		strcpy(buff, ": ");
		if (n < STR_MAX_LEN-1)
		{
			n += m_UnicodeStringTable.TranslateUTF8ToUCS2(buff, m_charrConfDuration + n, STR_MAX_LEN - n) - 2;
			if (n < STR_MAX_LEN-1)
				n += m_UnicodeStringTable.GetBytes(STR_PERMANENT, m_charrConfDuration + n);
		}
	}
	else
	{
		DWORD nDiff = m_dtEndConf - m_dtStartConf; // Seconds
		int   nDays = (int)(((double)nDiff) / 3600 / 24);
		CStructTm dt    = m_dtEndConf.GetTimeDelta(m_dtStartConf);
		if (nDays > 0)
			dt.m_hour += nDays * 24;

		if (dt.m_hour == 0)
		{
			sprintf(buff, ": %02d ", dt.m_min);
			if (n < STR_MAX_LEN-1)
			{
				n += m_UnicodeStringTable.TranslateUTF8ToUCS2(buff, m_charrConfDuration + n, STR_MAX_LEN - n) - 2;
				if (n < STR_MAX_LEN-1)
					n += m_UnicodeStringTable.GetString(STR_MINUTES, m_charrConfDuration + n);
			}
		}
		else
		{
			sprintf(buff, ": %d ", dt.m_hour);
			if (n < STR_MAX_LEN-1)
			{
				n += m_UnicodeStringTable.TranslateUTF8ToUCS2(buff, m_charrConfDuration + n, STR_MAX_LEN - n) - 2;
				if (n < STR_MAX_LEN-1)
				{
					if (dt.m_hour == 1)
						n += m_UnicodeStringTable.GetBytes(STR_HOUR, m_charrConfDuration + n);
					else
						n += m_UnicodeStringTable.GetBytes(STR_HOURS, m_charrConfDuration + n);
				}
			}

			if (dt.m_min != 0)
			{
				sprintf(buff, " %02d ", dt.m_min);
				if (n < STR_MAX_LEN-1)
				{
					n += m_UnicodeStringTable.TranslateUTF8ToUCS2(buff, m_charrConfDuration + n, STR_MAX_LEN - n) - 2;
					if (n < STR_MAX_LEN-1)
						n += m_UnicodeStringTable.GetString(STR_MINUTES, m_charrConfDuration + n);
				}
			}
			else
			{
				m_charrConfDuration[n++] = '\0';
				m_charrConfDuration[n++] = '\0';
			}
		}
	}

	char dump[1024] = "";
	m_UnicodeStringTable.DumpBuffer(m_charrConfDuration, STR_MAX_LEN, dump, 1024);
	TRACEINTO << "Duration:" << dump;
}

// --------------------------------------------------------------------------
void CGatheringManager::GetTitleStr(char* buff, int nBuffSize)
{
	m_UnicodeStringTable.TranslateUTF8ToUCS2(m_sConfName.c_str(), buff, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetParticipantStr(int iPart, char* buff, int nBuffSize)
{
	int n = m_vParticipants.size();
	if (iPart < n)
	{
		if (iPart == 7 && n > 7)
			m_UnicodeStringTable.TranslateUTF8ToUCS2(". . .", buff, nBuffSize);
		else
			m_UnicodeStringTable.TranslateUTF8ToUCS2(m_vParticipants[iPart].first.c_str(), buff, nBuffSize);
	}
	else
	{
		buff[0] = '\0';
		buff[1] = '\0';
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::GetVideoPartsStr(char* buff, int nBuffSize)
{
	WORD n = m_UnicodeStringTable.GetBytes(STR_VIDEO_PARTS, buff);
	char buffTmp[20] = "";
	GetVideoParticipantsCount(buffTmp, 19);
	std::string s(": ");
	s += std::string(buffTmp);
	m_UnicodeStringTable.TranslateUTF8ToUCS2(s.c_str(), buff + n, STR_MAX_LEN - n);
	char dump[1024] = "";
	m_UnicodeStringTable.DumpBuffer(buff, nBuffSize, dump, 1024);
	TRACEINTO << "Video Parts:" << dump;
}

// --------------------------------------------------------------------------
void CGatheringManager::GetAudioPartsStr(char* buff, int nBuffSize)
{
	WORD n = m_UnicodeStringTable.GetBytes(STR_AUDIO_PARTS, buff);
	char buffTmp[20] = "";
	GetAudioParticipantsCount(buffTmp, 19);
	std::string s(": ");
	s += std::string(buffTmp);
	m_UnicodeStringTable.TranslateUTF8ToUCS2(s.c_str(), buff + n, STR_MAX_LEN - n);
	char dump[1024] = "";
	m_UnicodeStringTable.DumpBuffer(buff, nBuffSize, dump, 1024);
	TRACEINTO << "Audio Parts:" << dump;
}

// --------------------------------------------------------------------------
void CGatheringManager::GetAccessNumberStr(char* buff, int nBuffSize)
{
	if (m_sIP.empty() && m_sISDN.empty() && m_sPSTN.empty())
		buff[0] = '\0', buff[1] = '\0';
	else
		m_UnicodeStringTable.GetString(STR_ACCESS_NO, buff);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetIPStr(char* buff, int nBuffSize)
{
	if (m_sIP.empty())
		buff[0] = '\0', buff[1] = '\0';
	else
		m_UnicodeStringTable.TranslateUTF8ToUCS2(m_sIP.c_str(), buff, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetISDNStr(char* buff, int nBuffSize)
{
	if (m_sISDN.empty())
		buff[0] = '\0', buff[1] = '\0';
	else
		m_UnicodeStringTable.TranslateUTF8ToUCS2(m_sISDN.c_str(), buff, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetPSTNStr(char* buff, int nBuffSize)
{
	if (m_sPSTN.empty())
		buff[0] = '\0', buff[1] = '\0';
	else
		m_UnicodeStringTable.TranslateUTF8ToUCS2(m_sPSTN.c_str(), buff, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetFreeText1Str(char* buff, int nBuffSize)
{
	m_UnicodeStringTable.TranslateUTF8ToUCS2(m_sFreeText1.c_str(), buff, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetFreeText2Str(char* buff, int nBuffSize)
{
	m_UnicodeStringTable.TranslateUTF8ToUCS2(m_sFreeText2.c_str(), buff, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetFreeText3Str(char* buff, int nBuffSize)
{
	m_UnicodeStringTable.TranslateUTF8ToUCS2(m_sFreeText3.c_str(), buff, nBuffSize);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetVideoParticipantsCount(char* buff, int nBuffSize)
{
	if (nBuffSize > 9)
		snprintf(buff, nBuffSize, "%d", m_nVideoParticipants);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetAudioParticipantsCount(char* buff, int nBuffSize)
{
	if (nBuffSize > 9)
		snprintf(buff, nBuffSize, "%d", m_vParticipants.size()-m_nVideoParticipants);
}

// --------------------------------------------------------------------------
void CGatheringManager::GetTimeInCall(char* buff, int nBuffSize)
{
	if (nBuffSize >= 9)
	{
		CStructTm dtCurrent;
		SystemGetTime(dtCurrent);
		CStructTm dt = dtCurrent.GetTimeDelta(m_dtStartConfMCU);
		if (dt.m_hour == 0)
			snprintf(buff, nBuffSize, "%02d:%02d", dt.m_min, dt.m_sec);
		else
			snprintf(buff, nBuffSize, "%02d:%02d:%02d", dt.m_hour, dt.m_min, dt.m_sec);
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::GetIP(char* buff, int nBuffSize)
{
	if (nBuffSize > 20)
	{
		strncpy(buff, m_sIP.c_str(), 20);
		buff[20] = '\0';
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::GetISDN(char* buff, int nBuffSize)
{
	if (nBuffSize > 20)
	{
		strncpy(buff, m_sISDN.c_str(), 20);
		buff[20] = '\0';
	}
}

// --------------------------------------------------------------------------
void CGatheringManager::GetPSTN(char* buff, int nBuffSize)
{
	if (nBuffSize > 20)
	{
		strncpy(buff, m_sPSTN.c_str(), 20);
		buff[20] = '\0';
	}
}
