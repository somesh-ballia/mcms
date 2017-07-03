#ifndef _GATHERING_H_
#define _GATHERING_H_

#include "StructTm.h"

#include <string>
#include <vector>
#include <set>
#include <map>

#include "PObject.h"
#include "UnicodeStringTable.h"
#include "VideoLayout.h"

#define STR_MAX_LEN             128
#define SHIFT_X_FOR_FULL_SCREEN 162

class CConf;
class CVideoLayout;
class CVisualEffectsParams;
class CGatheringManager;
class CConfParty;
class CConfGathering;
class CGathering;

enum EGatheringDisplayState
{
  eGatheringDisplayStateNone      = 0,
  eGatheringDisplayStateDisplayed = 1,
  eGatheringDisplayStateHidden    = 2,
  eGatheringDisplayStateEnded     = 4
};

////////////////////////////////////////////////////////////////////////////
//                        CGatheringManager
////////////////////////////////////////////////////////////////////////////
class CGatheringManager : public CPObject
{
  CLASS_TYPE_1(CGatheringManager, CPObject)

public:
	                                       CGatheringManager(CConf* pConf);
	                                      ~CGatheringManager();

	virtual const char*                    NameOf() const { return "CGatheringManager"; }

private:
	CConf*                                 m_pConf;
	CConfGathering*                        m_pConfGathering;
	CStructTm                              m_dtStartConfMCU;
	CStructTm                              m_dtStartConf;
	CStructTm                              m_dtExchangeConfStartTime;
	CStructTm                              m_dtEndConf;
	CStructTm                              m_dtStartGathering;
	DWORD                                  m_nSecondsInCall;

	bool                                   m_bGatheringEnabled;
	bool                                   m_bRecordIndication;
	std::string                            m_sConfName;
	std::string                            m_sConfDuration;
	char                                   m_charrConfDuration[STR_MAX_LEN];
	std::string                            m_sTitle;
	std::string                            m_sOrganizer;
	std::string                            m_sIP;
	std::string                            m_sISDN;
	std::string                            m_sPSTN;
	std::string                            m_sFreeText1;
	std::string                            m_sFreeText2;
	std::string                            m_sFreeText3;

	char                                   m_sTimes[STR_MAX_LEN];

	typedef std::pair<std::string, bool>   STRING_BOOL;
	typedef std::pair<CConfParty*, int>    PARTY_INT;

	bool                                   m_bNewPartyConnected;
	bool                                   m_bRecordLinkExists;
	std::set<std::string>                  m_setGatheringParticipants;
	std::set<std::string>                  m_setParticipants;
	std::set<DWORD>                        m_setConnectedPartyID;
	std::vector<STRING_BOOL>               m_vParticipants;
	std::map<std::string, CGathering*>     m_mapPartGathering;
	std::vector<PARTY_INT>                 m_vConnectingParties;
	std::set<DWORD>                        m_setGatheringDonePartyID; // VNGR-17652 indication to mute EP they already had Gathering.

	int                                    m_nVideoParticipants;

	int                                    m_nConfGatheringDuration;
	int                                    m_nPartyGatheringDuration;
	bool                                   m_bConfPermanent;

	CUnicodeStringTable                    m_UnicodeStringTable;

	bool                                   m_bStarted;
	bool                                   m_bEndGathering;
	bool                                   m_bInitialized;
	bool                                   m_bTimerStarted;
	bool                                   m_bConnectingTimerStarted;

	CVideoLayout*                          m_pGatheringVideoLayout;
	CVideoLayout*                          m_pVideoLayoutPrev;
	CVisualEffectsParams*                  m_pVisualEffects;
	CVisualEffectsParams*                  m_pVisualEffectsPrev;
	bool                                   m_bAutoLayoutPrev;

	bool                                   m_bTelePresenceMode;
	bool                                   m_bPresentationMode;
	bool                                   m_bSameLayout;

protected:
	void                                   Initialize();
	bool                                   FillParticipantsSet(std::set<std::string>& setParticipants); // returns Is RecordLink exists
	void                                   RestoreConfLayout();
	void                                   SetConfLayout(CVideoLayout* pLayout, bool bAutoLayout, bool bRestore1X4 = false);
	void                                   SetPartLayout(CVideoLayout* pLayout, bool bAutoLayout, const std::string& sPartName);
	void                                   SetPartyToConfOrPrivateLayout(const std::string& sPartName);
	void                                   SetVisualEffectsParams(CVisualEffectsParams* pVisualEffects);
	void                                   SetVisualEffectsParams(CConfParty* pConfParty, CVisualEffectsParams* pVisualEffects);
	void                                   SetVisualEffectsParams(const std::string& sPartName, CVisualEffectsParams* pVisualEffects);
	CVideoLayout*                          IsConfLayoutChangedDuringGathering();
	void                                   RemoveNotExistingParticipantsFromMapPartGathering(const std::set<std::string>& setParticipants);

public:
	CConf*                                 GetConf() const                        { return m_pConf; }
	bool                                   IsGatheringEnabled()                   { return m_bGatheringEnabled; }
	bool                                   IsNeedParticipantGathering();
	void                                   OnPartyConnecting(CConfParty* pConfParty);
	void                                   OnPartyConnected(CConfParty* pConfParty);
	void                                   OnPartyDisConnected(CConfParty* pConfParty);
	void                                   ReDisplayGatheringText(const char* pPartyName);
	void                                   HideGatheringText(const char* pPartyName);
	CVideoLayout*                          GetGatheringLayout(CConfParty* pConfParty = NULL);
	CVisualEffectsParams*                  GetGatheringVisualEffects();
	void                                   Start();
	void                                   OnUpdateByTimer();
	void                                   OnConnectingTimer();
	void                                   UpdateParticipantsList(bool bDumpToLog = false);
	int                                    GetExceptionPartList(std::set<std::string>& setParts);
	void                                   AddParticipant(const char* sName, bool bVideo);
	void                                   RemoveParticipant(const char* sName);
	void                                   ClearParticipants();
	void                                   OnPartyLayoutChanged(const char* pPartyName, CVideoLayout* pVideoLayout, DWORD param);
	int                                    GetConfGatheringDuration()             { return m_nConfGatheringDuration; }
	int                                    GetPartGatheringDuration()             { return m_nPartyGatheringDuration; }
	std::map<std::string, CGathering*>*    GetMapPartGathering()                  { return &m_mapPartGathering; }
	CGathering*								GetGathering(const char* pszPartyName);
	bool                                   IsRecordIndicate()                     { return m_bRecordIndication; }
	void                                   SetRecordIndicate(bool bEnable = true) { m_bRecordIndication = bEnable; }
	void                                   CalculateDuration();
	void                                   GetTimeInCall(char* buff, int nBuffSize);

	void                                   GetVideoParticipantsCount(char* buff, int nBuffSize);
	void                                   GetAudioParticipantsCount(char* buff, int nBuffSize);
	void                                   GetIP(char* buff, int nBuffSize);
	void                                   GetISDN(char* buff, int nBuffSize);
	void                                   GetPSTN(char* buff, int nBuffSize);

	void                                   GetRecStr(char* buff, int nBuffSize);
	void                                   GetConfNameStr(char* buff, int nBuffSize);
	void                                   GetOrganizerStr(char* buff, int nBuffSize);
	void                                   GetTimesStr(char* buff, int nBuffSize);
	void                                   GetTitleStr(char* buff, int nBuffSize);

	void                                   GetParticipantStr(int iPart, char* buff, int nBuffSize);

	void                                   GetVideoPartsStr(char* buff, int nBuffSize);
	void                                   GetAudioPartsStr(char* buff, int nBuffSize);
	void                                   GetAccessNumberStr(char* buff, int nBuffSize);
	void                                   GetIPStr(char* buff, int nBuffSize);
	void                                   GetISDNStr(char* buff, int nBuffSize);
	void                                   GetPSTNStr(char* buff, int nBuffSize);
	void                                   GetFreeText1Str(char* buff, int nBuffSize);
	void                                   GetFreeText2Str(char* buff, int nBuffSize);
	void                                   GetFreeText3Str(char* buff, int nBuffSize);

	void                                   UpdateConfigData();
	void                                   ReDisplayGathering();
	bool                                   IsNeedReDisplayGathering();
	void                                   ShowGatheringToParty(const char* pszPartName);
	void                                   StopGathering(const char* pszPartyName);
};


////////////////////////////////////////////////////////////////////////////
//                        CGathering
////////////////////////////////////////////////////////////////////////////
class CGathering : public CPObject
{
	CLASS_TYPE_1(CGathering, CPObject)

public:
	                                       CGathering(CGatheringManager* pGatheringManager);
	virtual                               ~CGathering();

	virtual const char*                    NameOf() const { return "CGathering"; }

protected:
	CGatheringManager*                     m_pGatheringManager;
	CConf*                                 m_pConf;
	CVideoLayout*                          m_pVideoLayoutPrev;
	bool                                   m_bAutoLayoutPrev;
	bool                                   m_bChangeLayout;
	CStructTm                              m_dtStartGathering;
	int                                    m_nGatheringDuration;
	bool                                   m_bInitialized;
	bool                                   m_bStarted;
	bool                                   m_bEndGathering;
	int                                    m_iShiftX;
	bool                                   m_bFullScreenGathering;
	int                                    m_nBackGroundTransparency;

	void                                   Initialize();

public:
	bool                                   IsGatheringTimeout();

	void                                   SetEndGathering()                                       { m_bEndGathering = true; }
	bool                                   IsEndGathering()                                        { return m_bEndGathering; }

	CVideoLayout*                          GetVideoLayoutPrev()                                    { return m_pVideoLayoutPrev; }
	void                                   SetVideoLayoutPrev(CVideoLayout* pVideoLayoutPrev)      { m_pVideoLayoutPrev = new CVideoLayout(*pVideoLayoutPrev); }

	void                                   SetIsAutoLayoutPrev(bool bAutoLayoutPrev)               { m_bAutoLayoutPrev = bAutoLayoutPrev; }
	bool                                   GetIsAutoLayoutPrev()                                   { return m_bAutoLayoutPrev; }

	void                                   SetIsNeedToChangeLayout(bool bChangeLayout)             { m_bChangeLayout = bChangeLayout; }
	bool                                   GetIsNeedToChangeLayout()                               { return m_bChangeLayout; }

	void                                   SetBackGroundTransparency(int iTransparency)            { m_nBackGroundTransparency = iTransparency; }
	int                                    GetBackGroundTransparency()                             { return m_nBackGroundTransparency; }

	void                                   SetFullScreenGathering(bool bFullScreenGathering)       { m_bFullScreenGathering = bFullScreenGathering; }
	bool                                   IsFullScreenGathering()                                 { return m_bFullScreenGathering; }

	virtual void                           HideGatheringText(const char* pszPartyName)             { m_pGatheringManager->HideGatheringText(pszPartyName); }
	virtual void                           ShowGatheringText(const char* pszPartyName)             { m_pGatheringManager->ReDisplayGatheringText(pszPartyName); }
	void                                   GetRecStr(char* buff, int nBuffSize)                    { m_pGatheringManager->GetRecStr(buff, nBuffSize); }
	void                                   GetConfNameStr(char* buff, int nBuffSize)               { m_pGatheringManager->GetConfNameStr(buff, nBuffSize); }
	void                                   GetOrganizerStr(char* buff, int nBuffSize)              { m_pGatheringManager->GetOrganizerStr(buff, nBuffSize); }
	void                                   GetTimesStr(char* buff, int nBuffSize)                  { m_pGatheringManager->GetTimesStr(buff, nBuffSize); }
	void                                   GetTitleStr(char* buff, int nBuffSize)                  { m_pGatheringManager->GetTitleStr(buff, nBuffSize); }
	void                                   GetParticipantStr(int iPart, char* buff, int nBuffSize) { m_pGatheringManager->GetParticipantStr(iPart, buff, nBuffSize); }
	void                                   GetVideoPartsStr(char* buff, int nBuffSize)             { m_pGatheringManager->GetVideoPartsStr(buff, nBuffSize); }
	void                                   GetAudioPartsStr(char* buff, int nBuffSize)             { m_pGatheringManager->GetAudioPartsStr(buff, nBuffSize); }
	void                                   GetAccessNumberStr(char* buff, int nBuffSize)           { m_pGatheringManager->GetAccessNumberStr(buff, nBuffSize); }
	void                                   GetIPStr(char* buff, int nBuffSize)                     { m_pGatheringManager->GetIPStr(buff, nBuffSize); }
	void                                   GetISDNStr(char* buff, int nBuffSize)                   { m_pGatheringManager->GetISDNStr(buff, nBuffSize); }
	void                                   GetPSTNStr(char* buff, int nBuffSize)                   { m_pGatheringManager->GetPSTNStr(buff, nBuffSize); }
	void                                   GetFreeText1Str(char* buff, int nBuffSize)              { m_pGatheringManager->GetFreeText1Str(buff, nBuffSize); }
	void                                   GetFreeText2Str(char* buff, int nBuffSize)              { m_pGatheringManager->GetFreeText2Str(buff, nBuffSize); }
	void                                   GetFreeText3Str(char* buff, int nBuffSize)              { m_pGatheringManager->GetFreeText3Str(buff, nBuffSize); }
	int                                    GetShiftX();

	virtual bool                           IsDisplayed()                                                                     = 0;
	virtual void                           SetDisplayed(bool bDisplayed, const char* pszPartyName)                           = 0;
	virtual bool                           IsNeedFullRendering(const char* pszPartyName)                                     = 0;
	virtual void                           SetIsNeedFullRendering(bool bNeedFullRendering, const char* pszPartyName)         = 0;
	virtual bool                           IsHideGatheringText(const char* pszPartyName)                                     = 0;
	virtual void                           SetIsHideGatheringText(bool bHideGatheringText, const char* pszPartyName)         = 0;
	virtual bool                           IsForceDisplay(const char* pszPartyName)                                          = 0;
	virtual void                           SetForceDisplay(bool bForceDisplay, const char* pszPartyName)                     = 0;

	virtual bool                           CheckTimeToShow(const char* pszPartyName)                                         = 0;
	virtual bool                           IsInitiatedByUser()                                                               { return false; }
	virtual bool                           IsPrivateLayout()                                                                 { return false; }
	virtual EGatheringDisplayState         GetGatheringDisplayState(const char* pszPartyName)                                = 0;
	virtual void                           SetGatheringDisplayState(const char* pszPartyName, EGatheringDisplayState eState) = 0;
};


////////////////////////////////////////////////////////////////////////////
//                        CConfGathering
////////////////////////////////////////////////////////////////////////////
class CConfGathering : public CGathering
{
	CLASS_TYPE_1(CConfGathering, CGathering)

public:
	                                       CConfGathering(CGatheringManager* pGatheringManager);
	                                      ~CConfGathering();

	virtual const char*                    NameOf() const { return "CConfGathering"; }

	friend class CPartGathering;

	virtual bool                           IsDisplayed();
	bool                                   IsPartyDisplayed(std::string& sParty);
	virtual void                           SetDisplayed(bool bDisplayed, const char* pszPartyName);
	void                                   GetNotDisplayedParties(std::set<std::string>& setParties);
	virtual bool                           IsNeedFullRendering(const char* pszPartyName);
	virtual void                           SetIsNeedFullRendering(bool bNeedFullRendering, const char* pszPartyName);
	virtual bool                           IsHideGatheringText(const char* pszPartyName);
	virtual void                           SetIsHideGatheringText(bool bHideGatheringText, const char* pszPartyName);
	virtual bool                           IsForceDisplay(const char* pszPartyName);
	virtual void                           SetForceDisplay(bool bForceDisplay, const char* pszPartyName);
	virtual bool                           CheckTimeToShow(const char* pszPartyName);
	virtual EGatheringDisplayState         GetGatheringDisplayState(const char* pszPartyName);
	virtual void                           SetGatheringDisplayState(const char* pszPartyName, EGatheringDisplayState eState);

	void                                   AddNewPartyData(std::string& sParty);

private:
	struct PARTY_DATA
	{
		bool                                 bDisplayed;
		bool                                 bNeedFullRendering;
		bool                                 bHideText;
		bool                                 bForceDisplay;
		EGatheringDisplayState               eDisplayState;
		CStructTm                            dtLastShow;
		PARTY_DATA()
		{
			bDisplayed         = false;
			bNeedFullRendering = true;
			bHideText          = false;
			bForceDisplay      = false;
			eDisplayState      = eGatheringDisplayStateNone;
		}
	};
	std::map<std::string, PARTY_DATA>      m_mapPartiesData;
};


////////////////////////////////////////////////////////////////////////////
//                        CPartGathering
////////////////////////////////////////////////////////////////////////////
class CPartGathering : public CGathering
{
	CLASS_TYPE_1(CPartGathering, CGathering)

public:
	                                       CPartGathering(CGatheringManager* pGatheringManager, const std::string& sPartName);
	                                       CPartGathering(CConfGathering* pConfGathering, const std::string& sPartName);
	virtual                               ~CPartGathering() { }

	virtual const char*                    NameOf() const                                                                    { return "CPartGathering"; }

	virtual bool                           CheckTimeToShow(const char* pszPartyName);

	bool                                   IsCreatedFromConfGathering()                                                      { return m_bCreatedFromConfGathering; }
	void                                   SetIsCreatedFromConfGathering(bool bCreatedFromConfGathering)                     { m_bCreatedFromConfGathering = bCreatedFromConfGathering; }

	virtual bool                           IsDisplayed()                                                                     { return m_bDisplayed; }
	virtual void                           SetDisplayed(bool bDisplayed, const char* pszPartyName);
	virtual bool                           IsNeedFullRendering(const char* pszPartyName);
	virtual void                           SetIsNeedFullRendering(bool bNeedFullRendering, const char* pszPartyName)         { m_bNeedFullRendering = bNeedFullRendering; }
	virtual bool                           IsHideGatheringText(const char* pszPartyName)                                     { return m_bHideText; }
	virtual void                           SetIsHideGatheringText(bool bHideGatheringText, const char* pszPartyName)         { m_bHideText = bHideGatheringText; }
	virtual bool                           IsInitiatedByUser()                                                               { return m_bInitiatedByUser; }
	virtual bool                           IsForceDisplay(const char* pszPartyName)                                          { return m_bForceDisplay; }
	virtual void                           SetForceDisplay(bool bForceDisplay, const char* pszPartyName)                     { m_bForceDisplay = bForceDisplay; }
	void                                   SetIsInitiatedByUser(bool bInitiatedByUser)                                       { m_bInitiatedByUser = bInitiatedByUser; }
	virtual bool                           IsPrivateLayout()                                                                 { return !IsInitiatedByUser(); }
	virtual EGatheringDisplayState         GetGatheringDisplayState(const char* pszPartyName)                                { return m_eDisplayState; }
	virtual void                           SetGatheringDisplayState(const char* pszPartyName, EGatheringDisplayState eState) { m_eDisplayState = eState; }

private:
	std::string                            m_sPartName;
	bool                                   m_bDisplayed;
	bool                                   m_bNeedFullRendering;
	bool                                   m_bHideText;
	bool                                   m_bForceDisplay;
	CStructTm                              m_dtLastShow;
	bool                                   m_bCreatedFromConfGathering;
	bool                                   m_bInitiatedByUser;
	EGatheringDisplayState                 m_eDisplayState;

protected:
	void                                   SetPartLayout(CVideoLayout* pLayout, bool bAutoLayout);
};

#endif // ifndef _GATHERING_H_
