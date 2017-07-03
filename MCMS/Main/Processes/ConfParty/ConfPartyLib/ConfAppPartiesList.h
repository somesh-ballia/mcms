#ifndef __CONF_APPLICATIONS_PARTIES_LIST_H__
#define __CONF_APPLICATIONS_PARTIES_LIST_H__

#include "Segment.h"
#include "Macros.h"
#include "ConfPartySharedDefines.h"
#include "ConfPartyDefines.h"
#include "ConfParty.h"

class CPartyApi;
class CConfAppInfo;
class CBridgePartyDisconnectParams;

#define MAX_ROLL_CALL_PARTY_LIST 100

enum EDTMFSourceEnum
{
	eDTMFSourceAuto    = 0x00,
	eDTMFSourceOutband = 0x01,
	eDTMFSourceInband  = 0x02,
	eDTMFSourceNone    = 0x03
};

using namespace std;

////////////////////////////////////////////////////////////////////////////
//                        CConfAppPartyParams
////////////////////////////////////////////////////////////////////////////
class CConfAppPartyParams : public CPObject
{
	CLASS_TYPE_1(CConfAppPartyParams, CPObject)

	friend class CFeatureObject;
	friend class CConfAppFeaturePartyIVR;
	friend class CConfAppPartiesList;

public:
	                  CConfAppPartyParams(CConfAppInfo* confAppInfo);
	                 ~CConfAppPartyParams();

	const char*       NameOf() const { return "CConfAppPartyParams";}

public:
	// state (video and audio)
	TAppPartyState    GetPartyAudioState() const                                 { return m_partyAudioState; }
	void              SetPartyAudioState(TAppPartyState state);

	TAppPartyState    GetPartyVideoState() const                                 { return m_partyVideoState; }
	void              SetPartyVideoState(TAppPartyState state);

	bool              IsPartyAudioStateConnected();
	bool              IsPartyVideoStateConnected();
	bool              IsSuitableForForce();

	// name
	const string&     GetPartyName() const                                       { return m_partyName; }
	void              SetPartyName(const string& name)                           { m_partyName = name; }

	// roll-call
	const string&     GetPartyRollCallName() const                               { return m_rollCallName; }
	const char*       GetPartyRollCallNamePtr() const                            { return m_rollCallName.c_str(); }
	void              SetPartyRollCallName(const string& name)                   { m_rollCallName = name; }
	void              ComposeAndSetRollCallName();

	WORD              GetRollCallRecDuration() const                             { return m_rollCallRecDuration; }
	void              SetRollCallRecDuration(WORD duration)                      { m_rollCallRecDuration = duration; }

	bool              IsRollCallInRecording() const                   			 { return m_rollCallInRecording; }
	void              SetRollCallInRecording(bool recording)            		 { m_rollCallInRecording = recording; }

	bool              IsInStopRecording() const                   				 { return m_isInStopRecording; }
	void              SetInStopRecording(bool stop)           					 { m_isInStopRecording = stop; }

	void			  StopRollCallRecording(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams);
	CBridgePartyDisconnectParams* GetBridgePartyDisconnectParams()const 		 { return m_pBridgePartyDisconnectParams; };

	WORD              GetPartyRollCallCheckSum() const                           { return m_rollCallCheckSum; }
	void              SetPartyRollCallCheckSum(WORD checksum)                    { m_rollCallCheckSum = checksum; }

	bool              IsRollCallRecordingExists() const                          { return m_isRollCallRecordingExists; }
	void              SetRollCallRecordingExists(bool rollCallExists)            { m_isRollCallRecordingExists = rollCallExists; }

	const string      GetRollCallRecFullPath() const;

	WORD              GetRollCallEntryTonePlayed() const                         { return m_isRollCallEntryTonePlayed; }
	void              SetRollCallEntryTonePlayed(bool isRollCallEntryTonePlayed) { m_isRollCallEntryTonePlayed = isRollCallEntryTonePlayed; }

	PartyRsrcID       GetPartyRsrcId() const                                     { return m_partyId; }
	void              SetPartyRsrcId(PartyRsrcID partyId)                        { m_partyId = partyId; }

	// PartyApi
	CPartyApi*        GetPartyApi()                                              { return m_pPartyApi; }
	const CPartyApi*  GetPartyApi() const                                        { return m_pPartyApi; }

	// leader
	bool              GetIsPartyLeader() const                                   { return m_isLeader; }
	void              SetPartyAsLeader(bool isLeader)                            { m_isLeader = isLeader; }

	// Last force indication
	int               GetLastForceInd() const                                    { return m_lastForceInd; }
	void              SetLastForceInd(int lastForceInd)                          { m_lastForceInd = lastForceInd; }

	// Noisy Line
	bool              GetIsNoisyLine() const                                     { return m_isNoisyLine; }
	void              SetIsNoisyLine(bool isNoisyLine)                           { m_isNoisyLine = isNoisyLine; }

	// Noisy line threshold
	BYTE              GetNoisyLineThresholdLevel() const                         { return m_noisyLineThresholdLevel; }
	void              SetNoisyLineThresholdLevel(BYTE noisyLineThresholdLevel)   { m_noisyLineThresholdLevel = noisyLineThresholdLevel; }

	// DTMF Indication Source
	void              SetDtmfIndSource(DWORD sourceOpcode);
	DWORD             GetDtmfIndSource() const                                   { return m_dtmfIndSource; }

	bool              isValidDtmfIndSource(DWORD sourceOpcode) const;

	// Slide is no longer permitted flag (used to indicate that video should be connected and not IVR video mode)
	void              SetSlideIsNoLongerPermitted(bool isSlideNoLongerPermitted) { m_isSlideNoLongerPermitted = isSlideNoLongerPermitted; }
	bool              GetSlideIsNoLongerPermitted() const                        { return m_isSlideNoLongerPermitted; }

	// set/get ready for slide
	bool              IsVideoBridgeReadyForSlide() const                         { return m_isVideoBridgeReadyForSlide; }
	void              SetVideoBridgeReadyForSlide(bool videoBridgeReadyForSlide) { m_isVideoBridgeReadyForSlide = videoBridgeReadyForSlide; }

	bool              IsPartyReadyForSlide() const                               { return m_isPartyReadyForSlide; }
	void              SetPartyReadyForSlide(bool isPartyReadyForSlide)           { m_isPartyReadyForSlide = isPartyReadyForSlide; }

	// Slide is on
	void              SetSlideIsOn(bool isSlideOn)                               { m_isSlideOn = isSlideOn; }
	bool              GetSlideIsOn() const                                       { return m_isSlideOn; }

	// Cascade Link party
	void              SetIsCascadeLinkParty(bool isCascadeLinkParty)             { m_isCascadeLinkParty = isCascadeLinkParty; }
	bool              GetIsCascadeLinkParty() const                              { return m_isCascadeLinkParty; }

	// Recording Link Party
	void              SetIsRecordingLinkParty(bool isRecordingLinkParty)         { m_isRecordingLinkParty = isRecordingLinkParty; }
	bool              GetIsRecordingLinkParty() const                            { return m_isRecordingLinkParty; }

	// Party Start IVR Mode
	void              SetPartyStartIvrMode(WORD startIvrMode)                    { m_partyStartIvrMode = startIvrMode; }
	WORD              GetPartyStartIvrMode() const                               { return m_partyStartIvrMode; }

	// Party type in GW conf (Inviter / initiator / normal / none)
	void              SetGatewayPartyType(eGatewayPartyType type)                { m_GatewayPartyType = type; }
	eGatewayPartyType GetGatewayPartyType() const                                { return m_GatewayPartyType; }

	// Party type in GW conf (Inviter / initiator / normal / none)
	void              SetLoggerPartyLastReport(DWORD loggerPartyLastReport)      { m_loggerPartyLastReport = loggerPartyLastReport; }
	DWORD             GetLoggerPartyLastReport() const                           { return m_loggerPartyLastReport; }
	DWORD             GetLoggerPartyStarted() const                              { return m_loggerPartyStarted; }

	// Party Start IVR Mode
	void              SetIsPutOnHoldDuringSlide(bool isPutOnHoldDuringSlide)     { m_isPutOnHoldDuringSlide = isPutOnHoldDuringSlide; }
	bool              GetIsPutOnHoldDuringSlide() const                          { return m_isPutOnHoldDuringSlide; }

protected:
	CConfAppInfo*     m_confAppInfo;

	PartyRsrcID       m_partyId;

	CPartyApi*        m_pPartyApi;

	string            m_partyName;
	string            m_rollCallName;

	WORD              m_rollCallRecDuration;
	WORD              m_rollCallCheckSum;
	bool              m_isRollCallRecordingExists;
	bool              m_isRollCallEntryTonePlayed;
	bool              m_rollCallInRecording;
	bool              m_isInStopRecording;
	CBridgePartyDisconnectParams* m_pBridgePartyDisconnectParams;
	TAppPartyState    m_partyAudioState; // IVR / MIX / APP / DISCONNECTING / DISCONNECTED
	TAppPartyState    m_partyVideoState; // IVR / MIX / APP / DISCONNECTING / DISCONNECTED
	bool              m_isLeader;
	int               m_lastForceInd;
	bool              m_isNoisyLine;
	BYTE              m_noisyLineThresholdLevel;

	DWORD             m_dtmfIndSource;
	EDTMFSourceEnum   m_forcedDTMFIndSourceEnum;
	time_t            m_lastSetDtmfSourceTime;

	bool              m_isSlideNoLongerPermitted;

	bool              m_isVideoBridgeReadyForSlide;
	bool              m_isPartyReadyForSlide;
	bool              m_isSlideOn;
	bool              m_isCascadeLinkParty;
	bool              m_isRecordingLinkParty;
	WORD              m_partyStartIvrMode;
	DWORD             m_loggerPartyStarted;
	DWORD             m_loggerPartyLastReport;

	eGatewayPartyType m_GatewayPartyType;

	bool              m_isPutOnHoldDuringSlide;
};


typedef std::vector<CConfAppPartyParams*> PartyVector;

////////////////////////////////////////////////////////////////////////////
//                        CConfAppPartiesList
////////////////////////////////////////////////////////////////////////////
class CConfAppPartiesList : public CPObject
{
	CLASS_TYPE_1(CConfAppPartiesList, CPObject)

public:
	                     CConfAppPartiesList();
	                     CConfAppPartiesList(CConfAppInfo* confAppInfo);
	                    ~CConfAppPartiesList();

	virtual const char*  NameOf() const { return "CConfAppPartiesList";}

public:

	int                  AddOrUpdateParty(DWORD opcode, DWORD confWithLocalIVR, BYTE confWithExternalIVR, CSegment* pParam);
	int                  DelParty(PartyRsrcID partyId);
	int                  DelParty(string name);
	void                 ChangePartyState(PartyRsrcID partyId, DWORD opcode);
	TAppPartyState       GetPartyAudioState(PartyRsrcID partyId);
	void                 SetPartyAudioState(PartyRsrcID partyId, TAppPartyState state);
	void                 UpdateStateUponDisconnecting(PartyRsrcID partyId, DWORD audioOrVideo);

	TAppPartyState       GetPartyVideoState(PartyRsrcID partyId);
	void                 SetPartyVideoState(PartyRsrcID partyId, TAppPartyState state);

	int                  FindPartyIndex(PartyRsrcID partyId);

	void                 SetPartyAsLeader(PartyRsrcID partyId, bool isLeader);
	bool                 GetIsPartyLeader(PartyRsrcID partyId);

	bool                 IsLeaderInConf();
	WORD                 GetNumOfParticipants();
	WORD                 GetNumOfInMixParticipants(DWORD* fileNameArray = NULL);
	const char*          GetPartyRollCallParams(PartyRsrcID partyId, WORD& checkSum, WORD& duration, DWORD opcode);
	DWORD                GetPartyInMix();
	DWORD                GetPartyToForce(PartyRsrcID partyId);
	DWORD                GetPartyByIndex(WORD ind);
	bool                 IsRollCallRecordingExists(PartyRsrcID partyId, WORD whereToSearch);
	WORD                 GetIsDeletedPartyLeader(PartyRsrcID partyId);
	CConfAppPartyParams* GetParty(PartyRsrcID partyId);
	void                 OnPartyExitToneEnded(PartyRsrcID partyId);
	void                 OnPartyExitToneEnded(int partyIndex);
	void                 RemoveAllIVREvents();
	int                  FindPartyIndex(string name);
	int                  FindPartyDelIndex(PartyRsrcID partyId);

	bool                 IsVideoBridgeReadyForSlide(DWORD ind);
	void                 SetVideoBridgeReadyForSlide(DWORD ind, bool videoBridgeReadyForSlide);

	bool                 IsPartyReadyForSlide(DWORD ind);
	void                 SetPartyReadyForSlide(DWORD ind, bool isPartyReadyForSlide);

	void                 SetIsCascadeLinkParty(PartyRsrcID partyId, bool isCascadeLinkParty);
	bool                 GetIsCascadeLinkParty(PartyRsrcID partyId);

	bool                 GetRollCallEntryTonePlayed(PartyRsrcID partyId);
	void                 SetRollCallEntryTonePlayed(PartyRsrcID partyId, bool isRollCallEntryTonePlayed);
	bool                 IsPartyAudioStateConnected(PartyRsrcID partyId);
	bool                 IsPartyVideoStateConnected(PartyRsrcID partyId);

	void                 SetIsRecordingLinkParty(PartyRsrcID partyId, bool isRecordingLinkParty);
	bool                 GetIsRecordingLinkParty(PartyRsrcID partyId);

	void                 DeleteAllRollCallFiles();
	int                  RemovePartyFromDeletedList(WORD ind);

	void                 SetSlideIsOn(PartyRsrcID partyId, bool isSlideOn);
	bool                 GetSlideIsOn(PartyRsrcID partyId);

	void                 SetPartyStartIvrMode(PartyRsrcID partyId, WORD startIvrMode);
	WORD                 GetPartyStartIvrMode(PartyRsrcID partyId);

	void                 SetGatewayPartyType(PartyRsrcID partyId, eGatewayPartyType type);
	eGatewayPartyType    GetGatewayPartyType(PartyRsrcID partyId);

	void                 SetSlideIsNoLongerPermitted(PartyRsrcID partyId, bool isSlideNoLongerPermitted);
	bool                 GetSlideIsNoLongerPermitted(PartyRsrcID partyId);
	void                 SetChairPartyRsrcId(PartyRsrcID partyId);
	PartyRsrcID          GetChairPartyRsrcId();
	bool                 IsTipParty(const char* partyName);
	bool                 IsTipParty(PartyRsrcID partyId);
	bool                 IsTipMaster(PartyRsrcID partyId);
	bool                 isMainITPEPOrSingleEP(PartyRsrcID partyId);
	void                 OnTimerPrintPartiesLogDetailsTbl(DWORD currentLogNumber);
	BOOL                 IsPartyOnHoldDuringSlide(PartyRsrcID partyId);
	void                 SetPartyOnHoldDuringSlide(PartyRsrcID partyId, WORD OnHoldDuringSlide);
	CConfParty*          GetConfParty(PartyRsrcID partyId);

protected:
	int                  DelPartyIndex(WORD ind);
	void                 MovePartyToDeletedList(WORD ind);
	int                  GetFirstIndexToSearch(int requesterPartyInd);
	void                 SendSetAsLeaderIfNeeded(PartyRsrcID partyId, int partyIndex);

public:
	PartyVector          m_partyList;
	PartyVector          m_partyDeletedList;

protected:
	WORD                 m_leaderInConf;
	CConfAppInfo*        m_confAppInfo;
	PartyRsrcID          m_chairPartyId;
};

#endif
