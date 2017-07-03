#if !defined(_COMMCONF_H__)
#define _COMMCONF_H__

#include "SerializeObject.h"
#include "ConfPartyDefines.h"
#include "CommRes.h"
#include "ConfParty.h"
#include "InitCommonStrings.h"
#include "VideoLayout.h"
#include "VideoLayout.h"
#include "VideoCellLayout.h"
#include "Observer.h"
#include "IpNetSetup.h"
#include "CallPart.h"
#include "IsdnNetSetup.h"
#include "MscIvr.h"
#include "MccfHelper.h"
#include "PlcmCdrEvent.h"
#include "PlcmCdrEventConfEnd.h"
#include "EventData.h"
#include "PlcmCdrEventConfDataUpdate.h"
#include "PlcmCdrEventConfRecord.h"
#include "PlcmCdrEventCallStart.h"
#include "PlcmCdrEventConfUserDataUpdate.h"
#include "PlcmCdrEventConfOperatorMoveParty.h"
#include "PlcmCdrEventConfBeginExtended.h"
#include "PlcmCdrEventOperatorConfActivity.h"
#include "PlcmCdrEventConfOperatorAddParty.h"
#include "PlcmCdrEventOperatorPartyActivity.h"
#include "AcIndicationStructs.h"

class CConfParty;
class CCommRes;

class PlcmCdrEventDisconnectedExtendedHelper;
class PlcmCdrEventCallStartExtendedHelper;

////////////////////////////////////////////////////////////////////////////
//                        CCommConf
////////////////////////////////////////////////////////////////////////////
class CCommConf : public CCommRes
{
	CLASS_TYPE_1(CCommConf, CCommRes)

public:
	                  CCommConf();
	                  CCommConf(const CCommConf& other);
	                  CCommConf(const CCommRes& other);
	virtual          ~CCommConf();

	CCommConf&        operator=(const CCommConf& other);
	const char*       NameOf() const { return "CCommConf"; }

	void              StartConference(BYTE GMTOffset, BYTE GMTOffsetSign);
	void              EndConference(BYTE cause);
	void              OperatorAddPartyCont1(CRsrvParty* pConfParty, const char* operName, WORD event);
	void              OperatorAddPartyCont2(CRsrvParty* pConfParty, WORD event, PlcmCdrEventCallStartExtendedHelper* pCdrEventCallStartExtendedHelper = NULL);

	void              OperatorAddParty(CRsrvParty* pConfParty, const char* operName, WORD event);
	void              OperatorMovePartyFromConf(const char* operName, const char* partyName, DWORD sourcePartyId, WORD event, char* destConfName, DWORD targetConfId = 0);
	void              UpdateUserDefinedInformation(CRsrvParty* pConfParty);
	void              IpChnnelConnToCDR(char* party_name, DWORD party_id, BYTE connectionStatus, CIpNetSetup* pNetSetup, DWORD callType);
	void              PartyCorrelationDataToCDR(const char* party_name, DWORD party_id, std::string sig_uuid);
	void              NetChnnelDisconnToCDR(char* party_name, DWORD party_id, BYTE channelNumber, BYTE discoInitiator, BYTE code_stndrd, BYTE location, BYTE cause_value, WORD event);
	void              NetChnnelConnToCDR(char* party_name, DWORD party_id, BYTE channel_id, WORD numChan, WORD dialType, CIsdnNetSetup* NetSetUp, char* PhoneNum);
	void              BillingCodeToCDR(DWORD confID, const char* partyName, DWORD partyRsrcID, const char* pBillingCode);
	void              PartyDtmfFailureIndicationToCDR(DWORD confID, const char* partyName, DWORD partyRsrcID, const char* dtmfCode, const char* rightData, DWORD failureState);
	void              ControlRecordingCDR(WORD recordingControl, CRsrvParty* pConfParty);

	void              OperatorTerminate(const char* operName);
	void              OperatorPartyAction(const char* pPartyName, DWORD partyId, const char* operName, WORD event);
	void              OperatorSetEndTime(const CStructTm& newTime, const char* operName);
	void              OperatorSetVisualName(const char* pPartyCurName, DWORD partyId, const char* pPartyPrevName, std::string pCorrelationId);
	void              PartyDisconnectCont1(WORD sumL_syncLost, WORD sumR_syncLost, WORD sumL_videoSyncLost, WORD sumR_videoSyncLost, PlcmCdrEventDisconnectedExtendedHelper* cdrEventDisconnectedExtendedHelper);
	void              NewUndefinedParty(CConfParty* pConfParty, WORD event);
	void              OperatorIpV6PartyCont1(CRsrvParty* pConfParty, const char* operName, WORD event);
	TransferRateType  ConvertTransferRateType(BYTE transferType);
	Connection        ConvertConnectionType(BYTE ConnectionType);
	VideoSessionType  ConvertVideoSessionType(BYTE sessionType);
	VideoFormatType   ConvertVideoFormatType(BYTE formatType);
	FrameRateType     ConvertVideoFormatRateType(BYTE formatRateType);
	NetworkLineType   ConvertNumType(BYTE numType);
	VideoProtocolType ConvertVideoProtocolType(BYTE videoProtocolType);
	void              OperatorPartyActionEventToCdr(const char* pPartyName, DWORD partyId, const char* operName, WORD event);
	AliasType         ConvertAliasType(BYTE AliasType);
	MeetMeMethodeType ConvertMeetMeMethod(BYTE meetMeType);
	CSerializeObject* Clone() { return new CCommConf; }
	const char*       GetName() const { return m_H243confName; }

	void              SerializeXml(CXMLDOMElement* pFatherNode) const {}
	void              SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken, ePartyData party_data_amount = FULL_DATA);
	int               DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

	void              SerializeShortXml(CXMLDOMElement* pNode, int opcode);
	int               DeSerializeShortXml(CXMLDOMElement* pSummaryNode, char* pszError);
	int               DeserializeConfXmlResponse(CXMLDOMElement* pConfNode, char* pszError);

	// for RelayMedia Info
	void              SerializeXmlRelayInfo( CXMLDOMElement* pFatherNode, DWORD updateCounter, ePartyData party_data_amount = FULL_DATA );
	void              AddConfRelayInfoToResponse(CXMLDOMElement* pConfNode);
	void              AddConfRelayInfoPartiesToResponse(CXMLDOMElement* pConfNode, DWORD ResCounter, ePartyData party_data_amount);
	bool              AddPartyRelayMediaInfoToResponse( CXMLDOMElement* pConfPartyNode, CConfParty* pConfParty, ePartyData party_data_amount);
	void              AddPartyRelayGeneralInfoToResponse( CXMLDOMElement* pConfPartyNode, CConfParty* pConfParty, ePartyData party_data_amount, bool bChanged );

	void              AddConfXmlToResponse(CXMLDOMElement* pConfNode, int ConfOpcode);
	void              AddOnGoingPartyToResResponse(CXMLDOMElement* pConfPartyNode, CRsrvParty* pParty, int Opcode, ePartyData party_data_amount);

	virtual int       Add(const CConfParty& other);
	int               Update(const CConfParty& other);
	virtual int       Cancel(const DWORD id);
	virtual int       Cancel(const char* name);
	void              IncConnectedPartiesNum();
	void              DecConnectedPartiesNum();
	void              SetSourceAudioId(const DWORD source_audio_Id);
	DWORD             GetSourceAudioId() const;
	void              SetLSDSourceId(const DWORD source_LSD_Id);
	DWORD             GetLSDSourceId() const;
	void              SetHSDSourceId(const DWORD source_HSD_Id);
	DWORD             GetHSDSourceId() const;
	void              SetForceVideoSourceId(const DWORD force_video_source_Id);
	DWORD             GetForceVideoSourceId() const;
	void              SetChairId(const DWORD chair_Id);
	DWORD             GetChairId() const;
	void              SetEPCContentSourceId(const DWORD EPC_Content_source_Id);
	DWORD             GetEPCContentSourceId() const;

	void              SetStatus(const DWORD status);
	DWORD             GetStatus();

	CConfParty*       GetCurrentParty(const DWORD id) const;
	CConfParty*       GetCurrentParty(const char* name) const;
	CConfParty*       GetCurrentPartyAccordingToVisualName(const char* visual_name) const;
	CConfParty*       GetRecordLinkCurrentParty(void) const;
	CConfParty*       GetFirstParty(int& nPos);
	CConfParty*       GetFirstParty();
	CConfParty*       GetNextParty(int& nPos);
	CConfParty*       GetNextParty();
	CConfParty*       GetUnreservedParty(const DWORD lobbyId) const;

	//Multiple links for ITP in cascaded conference feature:
	BOOL              GetMainLinkNameAccordingToMainPartiesCounterAndReturnIsMainLinkDefined(const DWORD mainPartiesCounter, char* mainPartyName);
	DWORD             NextMainPartiesCounter();

	WORD              FindPartyByBondingTelNum(const char* calledPhoneNumber, CConfParty** ppConfParty);

	void              SetTerminatingState(WORD bl);
	WORD              Is_TerminatingState() const;

	DWORD             GetOperatorPartyId() const;

	int               ComparePhones(const char* incomming_telNumber, const char* db_telNumber) const;
	WORD              IsConfPhone(const char* telNumber);
	int               GetMeetMePerConfConfPhoneLength(char* telNumber, const char* party_service_name);
	Phone*            GetActualConfPhone(char* dialingTelNumber, const char* party_service_name);

	WORD              IsDownspeed() const;
	void              SetDownspeed(const BYTE downspeed);

	BYTE              GetLSDRate() const;
	void              SetLSDRate(BYTE LSDRate);

	BYTE              GetCurConfVideoLayout() const;
	void              SetCurConfVideoLayout(BYTE curConfVideoLayout);

	WORD              GetNumVideoLayout() const;
	void              SetNumVideoLayout(const WORD num);

	int               AddVideoLayout(const CVideoLayout& other);
	int               UpdateVideoLayout(const CVideoLayout& other);
	int               CancelVideoLayout(const BYTE screenLayout);
	int               FindVideoLayout(const CVideoLayout& other);
	int               FindVideoLayout(const BYTE screenLayout);
	CVideoLayout*     GetFirstVideoLayout();
	CVideoLayout*     GetNextVideoLayout();
	CVideoLayout*     GetFirstVideoLayout(int& nPos);
	CVideoLayout*     GetNextVideoLayout(int& nPos);
	CVideoLayout*     GetVideoLayout(const BYTE screenLayout);
	CVideoLayout*     GetVideoLayout();
	void              SetVideoLayout(const CVideoLayout& other);
	BYTE              IsConfHasDifferentChairPerson(const DWORD ChairPersonId) const;

	void              CopyVideoLayoutToReservationLayout();

	void              PartyConnectDisconnectToCDR(CConfParty* pConfParty, PlcmCdrEventDisconnectedExtendedHelper* cdrEventDisconnectedExtendedHelper);
	void              SvcSipPartyConnectCDR(CConfParty* pConfParty, std::list<SvcStreamDesc>* pStreams, ECodecSubType eAudioCodec, DWORD dwBitRateOut, DWORD dwBitRateIn);
	void              GkInfoToCDR(const char* pPartyName, const DWORD partyId, BYTE* gkCallId);
	void              NewRateInfoToCdr(const char* pPartyName, const DWORD partyId, const DWORD currentRate);
	void              CallInfoPerPartyAfterDisconnectionToCdr(const char* pPartyName, const DWORD partyId, const DWORD maxBitRate,
	                                                          const char* maxResolution, const char* maxFrameRate,  //const WORD maxFrameRate,
	                                                          const char* address);
	void              SipPrivateExtensionsToCDR(const char* pPartyName, const DWORD partyId, const char* pCalledPartyID, const char* pAssertedIdentity, const char* pChargingVector, const char* pPreferredIdentity);
	virtual void      SetRcvMbx(COsQueue* pConfRcvMbx)    { m_pConfRcvMbx = pConfRcvMbx; }
	virtual COsQueue* GetRcvMbx() const { return m_pConfRcvMbx; }
	virtual void      SetEndTime(const CStructTm& other);
	virtual void      MPIChnnelConnToCDR(char* party_name, DWORD party_id, BYTE channelNumber, WORD numChan, WORD dialType, CNetSetup* NetSetUp, char* PhoneNum) {}
	virtual void      SetSummeryCreationUpdateCounter();
	void              RemoveAllLayouts();

	WORD              GetConnectedPartiesNumber();
	void              SetConnectedPartiesNumber(WORD PartiesNum);

	BYTE              IsMeetingRoomUp(void) const { return m_meetingRoomIsUp; }
	void              SetMeetingRoomUp(void)      { m_meetingRoomIsUp = TRUE; }

	void              SetIsMuteAllButX(DWORD MuteAllButX_Id);
	WORD              IsMuteAllButX(DWORD PartyId);
	DWORD             GetMuteAllButX() const;

	void              AddConfStatusToResponse(CXMLDOMElement* pResNode);
	int               DeSerializeConfStatus(CXMLDOMElement* pStatusNode, char* pszError);
	int               DeSerializeFullXml(CXMLDOMElement* pConferenceNode, char* pszError);
	void              AddVideoLayoutToResResponse(CXMLDOMElement* pResNode);
	int               SetVideoLayoutParams(CXMLDOMElement* pResNode, char* pszError);
	void              AddNewVideoLayout(int nLayout, int bActive, CVideoLayout* pRetLayout = NULL);
	STATUS            SearchPartyByIPOrAlias(mcTransportAddress ipAddress, const std::string partyAlias, WORD PartyAliasType) const;
	STATUS            SearchPartyByIP(mcTransportAddress* ipAddress);
	STATUS            SearchPartyByAlias(const std::string partyAlias, WORD PartyAliasType);
	virtual void      IncreaseFullUpdateCounter();

	virtual DWORD     AttachObserver(COsQueue* pObserver, WORD event, WORD type = 0, DWORD observerInfo1 = 0);
	virtual int       DetachObserver(COsQueue* pObserver);
	BYTE              IncludeRecordingParty();
	BYTE              IncludePlaybackParty();
	BYTE              IsActiveSlaveConf();
	WORD              GetNumVideoParties() const;
	void              SetNumVideoParties(WORD numVideoParties);
	void              OperatorMovePartyToConf(CConfParty* pConfParty, const char* operName, char* sourceConfName, DWORD sourceConfId, WORD event);
	void              CDRPartyCallingNumber_Move_to_Cont_1(DWORD confID, const char* party_name, DWORD party_id, const char* partyCallingNum);
	void              CDRPartyCalledNumber_Move_to_Cont_2(DWORD confID, const char* party_name, DWORD party_id, const char* partyCalledNum);
	void              ChairPersonEnteredCDR(const CConfParty* pConfParty);
	void              OperatorMovePartyToConfEventToCdr(CConfParty* pConfParty, const char* operName, char* sourceConfName, DWORD sourceConfId, Phone* CallingNum, Phone* CalledNum);
	Bonding           ConvertBondingType(BYTE BondingType);
	void              OperatorAddPartyEventToCdr(CRsrvParty* pConfParty, const char* operName, OperatorAddPartyAction pAction);
	WORD              ResetPublicOperatorAssistanceParties();
	BYTE              AreAnyITPPartiesConnected();
	void              updateExclusiveContent(CConfParty* pConfParty, BOOL i_isExclusiveContent);
	BOOL              isExclusiveContent();
	const char*       GetExclusiveContent();

	void              UpdateHotBackupFields();
	void              RestoreHotBackupFields();

	void              UpdateParamsIfSlaveExistInConf();
	void              UpdateLectureModeAndLayoutBecauseSlaveInConf(const char* lecturer_name);

	void              SetLectureMode(const CLectureModeParams& otherLectureMode);

	// VNGR-22639
	void              SaveLecturerVideoLayout(CLectureModeParams* pSavedLectureMode, CVideoLayout* pSavedLecturerVideoLayout);
	void              GetLecturerVideoLayout(CLectureModeParams*& pSavedLectureMode, CVideoLayout*& pSavedLecturerVideoLayout);
	//AT&T
	void              MccfIvrStartDialog(DialogState& state);

	void              SetSipTotRegistrationsStatus(const BYTE status);
	BYTE              GetSipTotRegistrationsStatus();
	void              SetMsConversationId(char* msConvId);
	const char*       GetMsConversationId() const;
  void				  SetEnableHighVideoResInAvcToSvcMixMode( BOOL isEnable ) {m_isEnableHighVideoResInAvcToSvcMixMode = isEnable;}
  BOOL				  GetEnableHighVideoResInAvcToSvcMixMode() const {return m_isEnableHighVideoResInAvcToSvcMixMode;}
  void				  SetEnableHighVideoResInSvcToAvcMixMode( BOOL isEnable ) {m_isEnableHighVideoResInSvcToAvcMixMode = isEnable;}
  BOOL				  GetEnableHighVideoResInSvcToAvcMixMode() const {return m_isEnableHighVideoResInSvcToAvcMixMode;}

	void              SetIsTipEnable(BYTE bIsEnableTip = TRUE) { m_bIsTipEnable = bIsEnableTip; }
	BYTE              GetIsTipEnable()                         { return m_bIsTipEnable; }
	void              UpdateActiveSpeakersList(DWORD* unActiveSpeakerMonitorIdList);
	void              UpdateActiveSpeakersListUponPartyLeftTheConf(DWORD id);
	void              AddConfActiveSpeakersListToXmlResponse(CXMLDOMElement* pResNode);
	void              DumpActiveSpeakersList();
	STATUS            SLOW_FAST_CHANGE_Terminal( const char* partyName, int slowFastAction );

	void			  SetCurrentConfCascadeMode(BYTE currentConfCascadeMode);
	BYTE			  GetCurrentConfCascadeMode() {return m_currentConfCascadeMode;}

	std::string       GetCorrelationId() const { return std::string(m_correlationId); }
	std::string       GetGmtTime() const;
	std::string       GetTimeStr(CStructTm curTime);
	STATUS            SendCdrEvendToCdrManager(ApiBaseObjectPtr ConfEndEvent, bool isConference = true, DWORD monitoryPartyId = 0, std::string pCorrelationId = "") const;
	void              SetIsCallGeneratorConference(BOOL bIsCallGeneratorConf) { m_bIsCallGeneratorConf = bIsCallGeneratorConf; }
	BOOL              GetIsCallGeneratorConference()                          { return m_bIsCallGeneratorConf; }
	DWORD             NextCGPartiesCounter();
	void              ResetAdHocProfileId();

	/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
	void              SetVideoRecoveryStatus(bool isVideoRecoveryStatus) { m_isVideoRecoveryStatus = isVideoRecoveryStatus; }
	bool              GetVideoRecoveryStatus()                           { return m_isVideoRecoveryStatus; }
	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
	void		SetIsNotifyAvMcuUri(bool isNotifyAvMcuUri = FALSE) {m_isNotifyAvMcuUri = isNotifyAvMcuUri;}
	bool		GetisNotifyAvMcuUri(){return m_isNotifyAvMcuUri;}
	BYTE	IncludeRDPGw();
	CConfParty*   GetRDPGwParty();

protected:
	void              StartQA();
	void              AddConfPartiesToResponse(CXMLDOMElement* pConfNode, int ResCounter, ePartyData party_data_amount = FULL_DATA);
	int               DeSerializeOngoingPartiesList(CXMLDOMElement* pPartiesListNode, char* pszError);
	const char*       PartyOpcodeToString(int opcode);
  BOOL				  m_isEnableHighVideoResInAvcToSvcMixMode;
  BOOL				  m_isEnableHighVideoResInSvcToAvcMixMode;

	// Attributes
	DWORD             m_conf_status;
	DWORD             m_chair_Id;
	DWORD             m_force_video_source_Id;
	WORD              m_number_of_connected_parties;
	DWORD             m_source_audio_Id;
	DWORD             m_source_LSD_Id;
	DWORD             m_source_HSD_Id;
	BYTE              m_downspeed;
	BYTE              m_LSDRate;
	BYTE              m_curConfVideoLayout;
	WORD              m_numVideoLayout;
	CVideoLayout*     m_pVideoLayout[MAX_VIDEO_LAYOUT_NUMBER];
	COsQueue*         m_pConfRcvMbx;
	CStructTm         m_endTimeLocal;
	WORD              m_number_of_active_recording_ports;

	// this flag help differ a meeting room with IVR and without it.
	// the flag goes ON when a party on simply connected to the m.r but
	// passed over IVR entrance proccedures.Before the first party passed those
	// procedures , the M.R. are still in temporary state
	BYTE              m_meetingRoomIsUp;
	DWORD             m_MuteAllButX_Id;
	BYTE              m_IsCandidateForTerminationMR;
	DWORD             m_EPC_Content_source_Id;
	DWORD             m_DeletedIdHistory[1000];
	DWORD             m_DeletedCounterHistory[1000];
	DWORD             m_LastDeletedIndex;
	BYTE              m_SipRegTotalSts;                                   // sipProxySts
	char              m_msConversationId[MS_CONVERSATION_ID_LEN];         // For adhoc conference created by MS click to conf
	BYTE              m_bIsTipEnable;

	// VNGR-22639: if in lecture mode, save the lecturer video layout
	// and restore it during Slave becomes Master
	CLectureModeParams* m_pSavedLectureMode;
	CVideoLayout*       m_pSavedLecturerVideoLayout;
  // current cascade conference mode (is the conference linked to another conf).
  BYTE				 m_currentConfCascadeMode;

	//Multiple links for ITP in cascaded conference feature:
	DWORD             m_mainPartiesCounter;
	DWORD             m_activeSpeakerList[MAX_ACTIVE_SPEAKER_LIST];
	bool              m_isVideoRecoveryStatus;
	bool		m_isNotifyAvMcuUri;

private:
	WORD              m_ind_conf;
	WORD              m_ind_vid_layout;
	BYTE              m_COPChairInVC;             // COP conf, a chair party in VC.
	CSubject*         m_pSubject;                 // Observer pattern, used only on mcms side.
	WORD              m_numVideoParties;          // Number of video parties in conference.
	BOOL              m_bIsCallGeneratorConf;
	DWORD             m_CGPartiesCounter;         //Bridge-8006

};


#endif // !defined(_COMMCONF_H__)

