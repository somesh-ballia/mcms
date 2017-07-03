#ifndef _COMMRESAPI_H__
	#define _COMMRESAPI_H__

#include <string.h>
#include "SerializeObject.h"
#include "ConfPartyApiDefines.h"
#include "StructTm.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "OperMask.h"
#include "Segment.h"
#include "Trace.h"
#include "ObjString.h"
#include "H221.h"
#include "IVRAvMsgStruct.h"
#include "LectureModeParams.h"
#include "ConfContactInfo.h"
#include "VisualEffectsParams.h"
#include "RsrvParty.h"
#include "AllocateStructs.h"
#include "CopConfigurationList.h"
#include "MessageOverlayInfo.h"
#include "DefinesIpServiceStrings.h"
#include "SiteNameInfo.h"
#include "ConfPartySharedDefines.h"

class CXMLDOMElement;
class CRsrvParty;
class CVisualEffectsParams;
class CVideoLayout;
class CConfContactInfo;
class CServicePrefixStr;
class COperatorConfInfo;
class CCOPConfigurationList;
class CAutoScanOrder;

#define COPERMASKON(a)      a.SetAllBitsOn();
#define COPERFLAGON(b)      b = TRUE;
#define COPERFLAGOFF(b)     b = FALSE;

#define SLOWCHANGE     {COPERMASKON(m_slowInfoMask); COPERFLAGON(m_slowInfoFlag); IncreaseFullUpdateCounter(); m_slowUpdateCounter = GetFullUpdateCounter();}
#define FASTCHANGE     {COPERMASKON(m_fastInfoMask); COPERFLAGON(m_fastInfoFlag); IncreaseFullUpdateCounter(); m_fastUpdateCounter = GetFullUpdateCounter();}
#define COMPLETECHANGE {COPERMASKON(m_completeInfoMask); COPERFLAGON(m_completeInfoFlag); IncreaseFullUpdateCounter(); m_completeUpdateCounter = GetFullUpdateCounter();}
#define COPERBITOFF(a, bit) a.SetInfoBit(bit, FALSE);

#define MOVE_TYPE_NONE 0
#define MOVE_TYPE_HOT  2

#define  H243          1

// ////////////////////////////////////////////////////////////////////////////////////////
// Reservation And Conference Flags
// ================================
//
// The following macros are used to update the DWORD (m_dwConFlags,m_dwResFlags).
// m_dwConFlags/m_dwResFlags used for sending the operator the Conference features
// in "One Word"
//
// UPDATERESFLAGSNEGATIV is UPDATE RES FLAGS NEGATIV and it meens that we are
// updating the flag ,using the negative value in equeation process.
// It used for flags that has the YES/NO or the TRUE/FALSE options only
//
// If we have only a positive value for the flag(the flag has only one state,and
// used for the extra values like AUTO), we are using the UPDATERESFLAGSPOSITIV
//
// Since in reservation ,the dwFlags is based on members in CCommRes to calc
// the flag in CCommResShort, we need the local_flag (we don't have the m_dwResFlags
// as a member so we cannot use it)
//
// Those macros are in use within the functions CalcRsrvFlags and Set[Attribute]
//
// What we have to do when adding a new flag??
// 1.Add define for the flag value in mcmsoper.
// if the flag has more than to states (Yes/No/Auto/Blocked) the Yes/No can get one
// flag and each of the others has to get a flag.
// 2.Add to the SetXXX function the macros,to set the attribute flag each
// time we are setting the attribute .
// 3.Add the relevant macros for the flag in the CalcRsrvFlags function.
//
// ////////////////////////////////////////////////////////////////////////////////////////

#define UPDATERESFLAGSNEGATIV(member, downValue, flagName, local_flag) \
	if (member == downValue) \
		local_flag &= ~flagName; \
	else \
		local_flag |= flagName;

#define UPDATERESFLAGSPOSITIV(member, upValue, flagName, local_flag) \
	if (member == upValue) \
		local_flag |= flagName; \
	else \
		local_flag &= ~flagName;

#define UPDATECONFLAGSNEGATIV(member, downValue, flagName) \
	if (member == downValue) \
		m_dwConfFlags &= ~flagName; \
	else \
		m_dwConfFlags |= flagName;

#define UPDATECONFLAGSPOSITIV(member, upValue, flagName) \
	if (member == upValue) \
		m_dwConfFlags |= flagName; \
	else \
		m_dwConfFlags &= ~flagName;

#define UPDATECONFLAGS2NEGATIV(member, downValue, flagName) \
	if (member == downValue) \
		m_dwConfFlags2 &= ~flagName; \
	else \
		m_dwConfFlags2 |= flagName;

////////////////////////////////////////////////////////////////////////////
//                        CCommResApi
////////////////////////////////////////////////////////////////////////////
class CCommResApi : public CSerializeObject
{
	CLASS_TYPE_1(CCommResApi, CSerializeObject)

public:
	CCommResApi();
	CCommResApi(const CCommResApi& other);
	CCommResApi& operator=(const CCommResApi& other);
	virtual ~CCommResApi();

	CSerializeObject*          Clone()                                            { return new CCommResApi; }

	virtual const char*        NameOf() const                                     { return "CCommResApi";}

	void                       SetName(const char* value)                         { strcpy_safe(m_H243confName, value); SLOWCHANGE; }
	const char*                GetName() const                                    { return m_H243confName; }

	void                       SetDisplayName(const char* value)                  { strcpy_safe(m_confDisplayName, value); SLOWCHANGE; }
	const char*                GetDisplayName() const                             { return m_confDisplayName; }

	void                       SetCorrelationId();
	const char*                GetCorrelationId() { return m_correlationId;}

	int                        convertStrActionToNumber(const char* strAction);

	void                       SerializeXml(CXMLDOMElement*& pFatherNode) const;
	void                       SerializeXml(CXMLDOMElement* pActionNode, DWORD ObjToken);
	int                        DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
	int                        DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const int numAction);

	char*                      Serialize(WORD format);
	void                       Serialize(WORD format, std::ostream& m_ostr);
	void                       Serialize(WORD format, CSegment& seg);
	void                       DeSerialize(WORD format, std::istream& m_istr);
	void                       DeSerialize(WORD format, CSegment& seg);

	int                        Add(const CRsrvParty& other);
	int                        FindName(const CRsrvParty& other);
	int                        FindName(const char* name);

	const CStructTm*           GetStartTime() const                               { return &m_startTime; }
	void                       SetStartTime(const CStructTm& other);

	const CStructTm*           GetExchangeConfStartTime() const                   { return &m_dtExchangeConfStartTime; }
	void                       SetExchangeConfStartTime(const CStructTm& other)   { m_dtExchangeConfStartTime = other; }

	const CStructTm*           GetDuration() const;
	void                       SetDuration(const CStructTm& duration);
	void                       SetEndTime(const CStructTm& startTime, const CStructTm& duration);
	const CStructTm*           GetEndTime();
	void                       SetEndTime(const CStructTm& other);
	void                       SetEndTime();

	void                       SetMessageOverlay(CMessageOverlayInfo* pMessageOverlay);
	CMessageOverlayInfo*       GetMessageOverlay()                                { return m_pMessageOverlayInfo; }

	void                       SetSiteName(CSiteNameInfo* pSiteName);
	CSiteNameInfo*             GetSiteNameInfo()                                  { return m_pSiteNameInfo; }

	void                       SetAutoScanOrder(CAutoScanOrder* pAutoScanOrder);
	CAutoScanOrder*            GetAutoScanOrder()                                 { return m_pAutoScanOrder; }

	DWORD                      ExtendDurationIfNeed(BOOL IsEnableAutoExtension, DWORD ExtensionTimeInterval);
	void                       TranslateExtensionTimeInterval(DWORD ExtensionTimeInterval);
	WORD                       IsRollCall() const;
	void                       SetRollCall(WORD bl);
	void                       SetContinuousPresenceScreenNumber(BYTE value)      { m_contPresScreenNumber = value; }
	int                        AddRsrvVidLayout(const CVideoLayout& other);
	int                        UpdateRsrvVidLayout(const CVideoLayout& other);
	int                        CancelRsrvVidLayout(BYTE screenLayout);
	int                        FindRsrvVidLayout(const CVideoLayout& other);
	int                        FindRsrvVidLayout(BYTE screenLayout);
	CVideoLayout*              GetFirstRsrvVidLayout();
	CVideoLayout*              GetNextRsrvVidLayout();
	CVideoLayout*              GetFirstRsrvVidLayout(int& nPos);
	CVideoLayout*              GetNextRsrvVidLayout(int& nPos);
	CVideoLayout*              GetCurrentRsrvVidLayout(BYTE screenLayout);
	CVideoLayout*              GetCurrentRsrvVidLayout();
	CVideoLayout*              GetActiveRsrvVidLayout();
	void                       AddResXmlToResponse(CXMLDOMElement* pResNode, int ConfOpcode = CONF_COMPLETE_INFO);
	void                       AddPartiesXmlToResponse(CXMLDOMElement* pResNode);
	int                        DeSerializePartyListXml(CXMLDOMElement* pListNode, char* pszError, int nAction);
	virtual void               AddVideoLayoutToResResponse(CXMLDOMElement* pResNode);
	virtual int                SetVideoLayoutParams(CXMLDOMElement* pResNode, char* pszError);

	BYTE                       IsAudioConf() const;

	void                       SetSIPFactory(const BYTE value);
	BYTE                       IsSIPFactory() const                               { return m_isSIPFactory; }

	void                       SetAutoConnectFactory(BYTE value)                  { m_isAutoConnectSIPFactory = value; }
	BYTE                       IsAutoConnectFactory() const                       { return m_isAutoConnectSIPFactory; }

	DWORD                      CalcRsrvFlags();
	DWORD                      CalcRsrvFlags2();

	virtual void               IncreaseFullUpdateCounter();

	DWORD                      GetFullUpdateCounter()                             { return m_dwFullUpdateCounter; }

	void                       SetTemplate(BYTE value)                            { m_isTemplate = value; UPDATECONFLAGSNEGATIV(m_isTemplate, NO, ACTIVE_PERMANEMT); }
	BYTE                       IsTemplate() const                                 { return m_isTemplate; }

	void                       SetEntryQ(BYTE value)                              { m_IsEntryQ = value; UPDATECONFLAGSNEGATIV(m_IsEntryQ, NO, ENTRY_QUEUE); SLOWCHANGE; }
	BYTE                       GetEntryQ() const                                  { return m_IsEntryQ; }

	void                       SetAdHocProfileId(DWORD value)                     { m_dwAdHocProfileId = value; }
	DWORD                      GetAdHocProfileId() const                          { return m_dwAdHocProfileId; }

	const char*                GetBaseProfileName() const                         { return m_baseProfileName; }

	void                       SetMeetingRoom(BYTE value)                         { m_isMeetingRoom = value; UPDATECONFLAGSNEGATIV(m_isMeetingRoom, NO, ACTIVE_PERMANEMT); }
	WORD                       IsMeetingRoom() const                              { return m_isMeetingRoom; }

	void                       SetGatheringEnabled(BYTE value)                    { m_bGatheringEnabled = value; }
	BYTE                       IsGatheringEnabled() const                         { return m_bGatheringEnabled; }

	void                       SetConfTemplate(BYTE value)                        { m_isConfTemplate = value; }
	WORD                       IsConfTemplate() const                             { return m_isConfTemplate; }

	BOOL                       IsNormalRes() const;

	void                       SetMeetingRoomReoccurrenceNum(BYTE value)          { m_meetingRoomReoccurrenceNum = value; }
	BYTE                       GetMeetingRoomReoccurrenceNum() const              { return m_meetingRoomReoccurrenceNum; }
	void                       DecreaseReoccurrenceNum();
	void                       IncreaseReoccurrenceNum();

	void                       SetMeetingRoomState(BYTE value);
	BYTE                       GetMeetingRoomState() const                        { return m_meetingRoomState; }

	void                       SetBillingData(const char* billingData);
	void                       SetContactInfo(const char* confContactInfo, int ContactNumber);

	void                       SetNumUndefParties(WORD value)                     { m_numUndefParties = value; }
	WORD                       GetNumUndefParties() const                         { return 0; /*14.11.06 second time fix, m_numUndefParties;*/ }

	void                       SetRepSchedulingId(DWORD value)                    { m_repSchedulingId = value; }
	DWORD                      GetRepSchedulingId() const                         { return m_repSchedulingId; }

	WORD                       GetNumParties() const                              { return m_numParties; }
	CRsrvParty*                GetFirstParty();
	CRsrvParty*                GetNextParty();

	BYTE                       IsConfFromProfile(DWORD& profileID);
	void                       SetMyProfileBasedParams(const CCommResApi* pProfile, BOOL updateServiceRegistrationContent = TRUE);

	void                       SetNumericConfId(const char* value)                { strcpy_safe(m_NumericConfId, value); SLOWCHANGE; }
	const char*                GetNumericConfId() const                           { return m_NumericConfId; }

	void                       SetMonitorConfId(ConfMonitorID value)              { m_confId = value; }
	ConfMonitorID              GetMonitorConfId() const                           { return m_confId; }

	void                       SetEntryPassword(const char* value)                { strcpy_safe(m_entry_password, value); SLOWCHANGE; }
	const char*                GetEntryPassword() const                           { return m_entry_password; }

	void                       SetH243Password(const char* value)                 { strcpy_safe(m_H243_password, value); SLOWCHANGE; }
	const char*                GetH243Password() const                            { return m_H243_password; }

	BYTE                       GetIsEncryption() const                            { return m_encryption; }
	BYTE                       GetEncryptionType() const                          { return m_eEncryptionType; }

	void                       SetEncryptionParameters(BYTE newIsEncryption, BYTE newEncryptionType);

	std::string                GetFileUniqueName(const std::string& dbPrefix) const;

	void                       SetIsTelePresenceMode(BYTE value);
	BYTE                       GetIsTelePresenceMode() const                      { return m_isTelePresenceMode; }
	BYTE                       GetManageTelepresenceLayoutInternaly() const       { return m_manageTelepresenceLayoutInternaly; }
	void                       SetManageTelepresenceLayoutInternaly(const BYTE yesNo);
	BYTE                       IsLayoutManagedInternally() const;

	void                       SetIsLPR(BYTE value);
	BYTE                       GetIsLPR() const                                   { return m_isLpr; }

	void                       SetEchoSuppression(BYTE value);
	BYTE                       GetEchoSuppression() const                         { return m_EchoSuppression; }

	void                       SetKeyboardSuppression(BYTE value);
	BYTE                       GetKeyboardSuppression() const                     { return m_KeyboardSuppression; }

	BYTE                       GetAutoMuteNoisyParties() const                 { return m_isAutoMuteNoisyParties; }

	void                       SetAutoScanInterval(WORD value)                    { m_AutoScanInterval = value; SLOWCHANGE; }
	WORD                       GetAutoScanInterval() const                        { return m_AutoScanInterval; }

	void                       SetLegacyShowContentAsVideo(BYTE value)            { m_ShowContentAsVideo = value; }
	BYTE                       IsLegacyShowContentAsVideo() const                 { return (m_videoSession == VIDEO_SESSION_COP) ? FALSE : m_ShowContentAsVideo; }

	// Exclusive Content Mode 7.0.2C
	void                       SetExclusiveContentMode(BOOL value)                { m_bIsExclusiveContentMode = value; }
	BOOL                       IsExclusiveContentMode() const                     { return m_bIsExclusiveContentMode; }

	void                       SetHDVSW(BYTE value)                               { m_HD = value; }
	BYTE                       GetIsHDVSW() const                                 { return m_HD; }

	void                       SetHDResolution(EHDResolution value)               { m_HDResolution = value; }
	EHDResolution              GetHDResolution() const                            { return m_HDResolution; }

	void                       SetH264VSWHighProfilePreference(BYTE value)        { m_H264VSWHighProfilePreference = value; }
	BYTE                       GetH264VSWHighProfilePreference() const            { return m_H264VSWHighProfilePreference; }

	void                       SetVideoQuality(eVideoQuality value)               { m_videoQuality = value; }
	eVideoQuality              GetVideoQuality() const                            { return m_videoQuality; }

	void                       SetConfTransferRate(BYTE value)                    { m_confTransferRate = value; }
	BYTE                       GetConfTransferRate() const                        { return m_confTransferRate; }

	void                       SetIsSiteNamesEnabled(BYTE value)                  { m_isSiteNamesEnabled = value; }
	BYTE                       GetIsSiteNamesEnabled() const                      { return m_isSiteNamesEnabled; }

	int                        CleanAllServicePrefix();
	int                        AddServicePrefix(const CServicePrefixStr& other);

	int                        AddServicePhone(const CServicePhoneStr& other);
	int                        FindServicePhone(const CServicePhoneStr& other);
	CServicePhoneStr*          GetFirstServicePhone();
	CServicePhoneStr*          GetNextServicePhone();
	BYTE                       GetNetwork() const                                 {return m_network;}

	void                       SetInternalConfStatus(DWORD value)                 { m_internalConfStatus = value; }
	DWORD                      GetInternalConfStatus() const                      { return m_internalConfStatus; }

	void                       SetNumOfMinAudioParties(WORD value)                { m_minNumAudioParties = value; }
	WORD                       GetNumOfMinAudioParties() const                    { return m_minNumAudioParties; }

	void                       SetNumOfMinVideoParties(WORD value)                { m_minNumVideoParties = value; }
	WORD                       GetNumOfMinVideoParties() const                    { return m_minNumVideoParties; }

	void                       SetIsGateway(BYTE value);
	BYTE                       GetIsGateway() const                               { return m_isGateway; }

	BOOL                       IsEnableIsdnPstnAccess() const                     { return (NETWORK_H320_H323 == GetNetwork() && m_meetMePerConf); }

	void                       SetOperatorConf(BYTE value)                        { m_operatorConf = value; }
	BYTE                       GetOperatorConf() const                            { return m_operatorConf; }

	void                       SetIsGWDialOutToH323(BYTE onOff)                   { UPDATERESFLAGSNEGATIV(onOff, NO, GW_H323_OUT, m_GWDialOutProtocols); }
	BYTE                       GetIsGWDialOutToH323() const                       { return ((m_GWDialOutProtocols & GW_H323_OUT) == GW_H323_OUT); }

	void                       SetIsGWDialOutToSIP(BYTE onOff)                    { UPDATERESFLAGSNEGATIV(onOff, NO, GW_SIP_OUT, m_GWDialOutProtocols); }
	BYTE                       GetIsGWDialOutToSIP() const                        { return ((m_GWDialOutProtocols & GW_SIP_OUT) == GW_SIP_OUT); }

	void                       SetIsGWDialOutToH320(BYTE onOff)                   { UPDATERESFLAGSNEGATIV(onOff, NO, GW_H320_OUT, m_GWDialOutProtocols); SLOWCHANGE; }
	BYTE                       GetIsGWDialOutToH320() const                       { return ((m_GWDialOutProtocols & GW_H320_OUT) == GW_H320_OUT); }

	void                       SetIsGWDialOutToPSTN(BYTE onOff)                   { UPDATERESFLAGSNEGATIV(onOff, NO, GW_PSTN_OUT, m_GWDialOutProtocols); SLOWCHANGE; }
	BYTE                       GetIsGWDialOutToPSTN() const                       { return ((m_GWDialOutProtocols & GW_PSTN_OUT) == GW_PSTN_OUT); }

	void                       SetGWDialOutProtocols(WORD value)                  { m_GWDialOutProtocols = value; SLOWCHANGE; }
	WORD                       GetGWDialOutProtocols() const                      { return m_GWDialOutProtocols; }
	BYTE                       IsDefinedGWDialOutProtocols() const                { return (m_GWDialOutProtocols) ? YES : NO; }

	void                       SetLastQuitType(BYTE value)                        { m_LastQuitType = value; }
	BYTE                       GetLastQuitType() const                            { return m_LastQuitType; }

	DWORD                      IsPermanent() const                                { return m_isPermanent; }
	void                       SetPermanent(DWORD value)                          { m_isPermanent = value; }

	void                       SetConfMaxResolution(BYTE value)                   { m_confMaxResolution = value; }
	BYTE                       GetConfMaxResolution() const                       { return m_confMaxResolution; }

	void                       SetCopConfigurationList(CCOPConfigurationList* pCOPConfigurationList);
	CCOPConfigurationList*     GetCopConfigurationList() const                    { return m_pCopConfigurationList; }

	BYTE                       IsAdHocConf() const                                { return m_isAdHocConf; }

	void                       SetAppointmentId(const char* value)                { strcpy_safe(m_appoitnmentID, value); SLOWCHANGE; }
	const char*                GetAppointmentId() const                           { return m_appoitnmentID; }

	void                       SetEnableRecording(BYTE value)                     { m_EnableRecording = value; SLOWCHANGE; }
	BYTE                       GetEnableRecording() const                         { return m_EnableRecording; }

	void                       SetStartRecordingPolicy(BYTE value)                { m_StartRecPolicy = value; SLOWCHANGE; }
	BYTE                       GetStartRecordingPolicy() const                    { return m_StartRecPolicy; }

	void                       SetEnableRecordingIcon(BYTE value)                 { m_EnableRecordingIcon = value; SLOWCHANGE; }
	BYTE                       GetEnableRecordingIcon() const                     { return m_EnableRecordingIcon; }

	void                       SetEnableRecordingNotify(BYTE value)                 { m_IsEnableRecNotify = value; SLOWCHANGE; }
	BYTE                       GetEnableRecordingNotify() const                     { return m_IsEnableRecNotify; }

	void                       SetEnableSelfNetworkQualityIcon(BYTE value)                 { m_EnableSelfNetworkQualityIcon = value; SLOWCHANGE; }
	BYTE                       GetEnableSelfNetworkQualityIcon() const                     { return m_EnableSelfNetworkQualityIcon; }

	void                       SetEnableAudioParticipantsIcon(BYTE value)                 { m_EnableAudioParticipantsIcon = value; SLOWCHANGE; }
	BYTE                       GetEnableAudioParticipantsIcon() const                     { return m_EnableAudioParticipantsIcon; }

	WORD                       GetIconDisplayPosition() const                     { return m_IconDisplayPosition; }
	BYTE                       GetAudioParticipantsIconDisplayMode() const                     { return m_AudioParticipantsIconDisplayMode; }
	WORD                       GetAudioParticipantsIconDuration() const                     { return m_AudioParticipantsIconDuration; }

	void                       SetIsStreaming(BYTE value)                         { m_isStreaming = value; SLOWCHANGE; }
	BYTE                       GetIsStreaming() const                             { return m_isStreaming; }

	void                       SetMeetingOrganizer(const char* value)             { strcpy_safe(m_meetingOrganizer, value); SLOWCHANGE; }
	const char*                GetMeetingOrganizer() const                        { return m_meetingOrganizer; }

	void                       SetIpNumberAccess(const char* value)               { strcpy_safe(m_sIpNumberAccess, value); SLOWCHANGE; }
	const char*                GetIpNumberAccess() const                          { return m_sIpNumberAccess; }

	void                       SetNumberAccess_1(const char* value)               { strcpy_safe(m_sNumberAccess_1, value); SLOWCHANGE; }
	const char*                GetNumberAccess_1() const                          { return m_sNumberAccess_1; }

	void                       SetNumberAccess_2(const char* value)               { strcpy_safe(m_sNumberAccess_2, value); SLOWCHANGE; }
	const char*                GetNumberAccess_2() const                          { return m_sNumberAccess_2; }

	void                       SetLanguageFromString(const char* pszLanguage);

	void                       SetStartConfRequiresLeaderOnOff(BYTE value)        { m_startConfRequiresLeaderOnOff = value; }
	BYTE                       GetStartConfRequiresLeaderOnOff() const            { return m_startConfRequiresLeaderOnOff; }

	void                       SetTerminateConfAfterChairDroppedOnOff(BYTE value) { m_terminateConfAfterChairDropped = value; }
	BYTE                       GetTerminateConfAfterChairDroppedOnOff() const     { return m_terminateConfAfterChairDropped; }

	void                       SetTelePresenceLayoutMode(BYTE value)              { m_telePresenceLayoutMode = value; }
	BYTE                       GetTelePresenceLayoutMode() const                  { return m_telePresenceLayoutMode; }

	void                       SetTelePresenceModeConfiguration(BYTE value)       { m_telePresenceModeConfiguration = value; }
	BYTE                       GetTelePresenceModeConfiguration() const           { return m_telePresenceModeConfiguration; }

	void                       SetIsCropping(BYTE value)                          { m_isCropping = value; }
	BYTE                       GetIsCropping() const                              { return m_isCropping; }

	void                       SetIsAudioClarity(BYTE value)                      { m_isAudioClarity = value; }
	BYTE                       GetIsAudioClarity() const                          { return m_isAudioClarity; }

	void                       SetConfSpeakerChangeMode(WORD value)               { m_confSpeakerChangeMode = value; }
	WORD                       GetConfSpeakerChangeMode() const                   { return m_confSpeakerChangeMode; }

	void                       SetIsAutoBrightness(BYTE value)                    { m_isAutoBrightness = value; }
	BYTE                       GetAutoBrightness() const                          { return m_isAutoBrightness; }

	void                       SetFECCEnabled(BYTE value)                         { m_FECC_Enabled = value; }
	BYTE                       GetFECCEnabled() const                             { return m_FECC_Enabled; }

	void                       SetMuteIncomingPartiesLectureMode(BYTE value)      { m_Mute_Incoming_Parties_Lecture_Mode = value; SLOWCHANGE; }  //Fix BRIDGE-2455
	BYTE                       GetMuteIncomingPartiesLectureMode() const          { return m_Mute_Incoming_Parties_Lecture_Mode; }

	void                       SetMuteAllPartiesAudioExceptLeader(BYTE value)     { m_muteAllPartiesAudioExceptLeader = value; }
	BYTE                       GetMuteAllPartiesAudioExceptLeader() const         { return m_muteAllPartiesAudioExceptLeader; }

	void                       SetMuteAllPartiesVideoExceptLeader(BYTE value)     { m_muteAllPartiesVideoExceptLeader = value; }
	BYTE                       GetMuteAllPartiesVideoExceptLeader() const         { return m_muteAllPartiesVideoExceptLeader; }

	void                       SetIsCOPReservation(BYTE value)                    { m_IsConfOnPort = value; }
	BYTE                       IsCOPReservation()                                 { return m_IsConfOnPort; }

	BYTE                       GetResSts()                                        { return m_ResSts; }
	void                       SetResSts(BYTE value)                              { m_ResSts = value; SLOWCHANGE; }

	DWORD                      GetConfID()                                        { return m_confId; }

	BOOL                       isIvrProviderEQ() const                            { return m_ivrProviderEQ ? TRUE : FALSE; }
	BOOL                       isExternalIvrControl() const                       { return m_externalIvrControl ? TRUE : FALSE; }

	void                       SetServiceNameForMinParties(const char* value)     { strcpy_safe(m_ServiceNameForMinParties, value); SLOWCHANGE; }
	const char*                GetServiceNameForMinParties() const                { return m_ServiceNameForMinParties; }

	void                       SetSipRegistrationTotalSts(BYTE value)             { m_SipRegistrationTotalSts = value; SLOWCHANGE; }
	BYTE                       GetSipRegistrationTotalSts()                       {return m_SipRegistrationTotalSts;}

	void                       SetIsTipCompatible(BYTE tipCompatible)             { m_TipCompatibility = tipCompatible; }
	BYTE                       GetIsTipCompatible() const                         { return m_TipCompatibility; }
	void                       SetMsSvcCascadeMode(BYTE avMcuCascadeVideoMode)    { m_avMcuCascadeVideoMode = avMcuCascadeVideoMode; }
	BYTE                       GetMsSvcCascadeMode() const                         { return m_avMcuCascadeVideoMode; }
	BOOL                       GetIsTipCompatibleVideo() const                    { return (eTipCompatibleVideoOnly == m_TipCompatibility || eTipCompatibleVideoAndContent == m_TipCompatibility || eTipCompatiblePreferTIP == m_TipCompatibility); }
	BOOL                       GetIsTipCompatibleContent() const                  { return (eTipCompatibleVideoAndContent == m_TipCompatibility || eTipCompatiblePreferTIP == m_TipCompatibility); }
	BOOL                       GetIsPreferTIP() const                             { return (m_TipCompatibility == eTipCompatiblePreferTIP); }
	BOOL                       GetIsVideoAndContent() const                       { return (m_TipCompatibility == eTipCompatibleVideoAndContent); }
	DWORD                      GetNatKAPeriod()                                   {return m_natKeepAlivePeriod;}
	void                       SetNatKAPeriod(DWORD period)                       { m_natKeepAlivePeriod = period;}
	void                       SetSpeakChangeParam(BYTE speakerChangeThreshold);

	const char*                GetServiceRegistrationContentServiceName(int serviceId);
	BOOL                       GetServiceRegistrationContentRegister(int serviceId);
	BOOL                       GetServiceRegistrationContentAcceptCall(int serviceId);
	void                       SetServiceRegistrationContentServiceName(int serviceId, const char* name);
	void                       SetServiceRegistrationContentRegister(int serviceId, BOOL reg);
	void                       SetServiceRegistrationContentAcceptCall(int serviceId, BOOL accept);
	BYTE                       GetServiceRegistrationContentStatus(int serviceId);
	void                       SetServiceRegistrationContentStatus(int serviceId, BYTE status);
	void                       UpdateServiceRegistrationTotalStatus(int serviceId, BYTE status);
	int                        GetServiceRegistrationContentServiceIndexByName(const char* name);

	bool                       TestMultiCascadeValidity(CRsrvParty& party);
	void                       OnPartyAdd(const CRsrvParty& party);
	void                       OnPartyDelete(const CRsrvParty& party);

	eConfMediaType             GetConfMediaType() const                           { return m_confMediaType; }
	void                       SetConfMediaType(eConfMediaType value);
	EOperationPointPreset      GetOperationPointPreset() const                    { return m_eOperationPointPreset; }
	void                       SetOperationPointPreset(EOperationPointPreset value);

	// Content Transcoding
	void                   SetContentMultiResolutionEnabled(const BYTE ContentMultiResolutionEnabled);
	BYTE                   GetContentMultiResolutionEnabled() const;
	void                   SetContentXCodeH264Supported(const BYTE ContentXCodeH264Supported);
	BYTE                   GetContentXCodeH264Supported() const;
	void                   SetContentXCodeH263Supported(const BYTE ContentXCodeH263Supported);
	BYTE                   GetContentXCodeH263Supported() const;
	void                   SetIsCascadeOptimized(const BYTE isCascadeOptimized);
	BYTE                   GetIsCascadeOptimized() const;
	void                   SetIsAsSipContent(const BYTE isAsSipContent);
	BYTE                   GetIsAsSipContent() const;
	void                   SetIsHighProfileContent(const BYTE isHighProfileContent);
	BYTE                   GetIsHighProfileContent() const;
	WORD                   GetMrcMcuId() const                               { return m_mrcMcuId; }
	void                   SetMrcMcuId(WORD value);
	void                   SetFocusUriScheduling(const char* value)                    { strcpy_safe(m_FocusUriScheduling, value); SLOWCHANGE; }
	const char*            GetFocusUriScheduling() const                               { return m_FocusUriScheduling; }
	void                   SetFocusUriCurrently(const char* value) ;
	const char*            GetFocusUriCurrently() const                               { return m_FocusUriCurrently; }
	void                   SetSrsPlaybackLayoutMode(BYTE value)              { m_SrsPlaybackLayout= value; }
	BYTE                   GetSrsPlaybackLayoutMode() const                  { return m_SrsPlaybackLayout; }
	bool                   isOverrideProfileLayout()const                    {return m_overideProfileLayout;}


protected:
	virtual void               AddNewVideoLayout(int nLayout, int bActive, CVideoLayout* pRetLayout = NULL);

protected:
	char                       m_H243confName[H243_NAME_LEN];                                           // conferences name - US-ASCII
	char                       m_confDisplayName[H243_NAME_LEN];                                        // conferences UTF-8 name
	char                       m_correlationId[CORRELATION_ID_LENGTH];
	ConfMonitorID              m_confId;
	CStructTm                  m_startTime;
	CStructTm                  m_actualStartTime;
	CStructTm                  m_duration;
	BYTE                       m_stand_by;
	BYTE                       m_confControl;
	BYTE                       m_confProtocol;
	BYTE                       m_automaticTermination;
	BYTE                       m_externalMaster;
	BYTE                       m_confTransferRate;
	BYTE                       m_confMaxResolution;
	BYTE                       m_audioRate;
	BYTE                       m_video;
	BYTE                       m_videoSession;
	BYTE                       m_HD;
	BYTE                       m_EQ_videoSession;
	BYTE                       m_contPresScreenNumber;
	WORD                       m_numRsrvVideoSource;
	CVideoLayout*              m_pRsrvVideoLayout[MAX_VIDEO_LAYOUT_NUMBER];
	BYTE                       m_videoPictureFormat;
	BYTE                       m_CIFframeRate;
	BYTE                       m_QCIFframeRate;
	BYTE                       m_LSDRate;
	DWORD                      m_audioTone;
	BYTE                       m_alertToneTiming;
	WORD                       m_talkHoldTime;
	BYTE                       m_audioMixDepth;
	CAvMsgStruct*              m_pAvMsgStruct;
	BYTE                       m_videoProtocol;                                                         // H261, H263, Auto //VERSION 1.3
	BYTE                       m_meetMePerConf;                                                         // YES, NO          //VERSION 1.3
	WORD                       m_numServicePhoneStr;                                                    // VERSION 1.3
	CServicePhoneStr*          m_pServicePhoneStr[MAX_NET_SERV_PROVIDERS_IN_LIST];                      // VERSION 1.3
	char                       m_H243_password[H243_NAME_LEN];                                          // password         //VERSION 1.3
	WORD                       m_numServicePrefixStr;
	CServicePrefixStr*         m_pServicePrefixStr[MAX_NET_SERV_PROVIDERS_IN_LIST];                     // ori inbar
	BYTE                       m_cascadeMode;

	// Multiple links for ITP in cascaded conference feature:
	BYTE                       m_cascadedLinksNumber;
	WORD                       m_cascadedPartiesCounter;                                                // counter of (dis-)connected cascaded parties

	WORD                       m_numUndefParties;
	WORD                       m_minNumVideoParties;
	WORD                       m_minNumAudioParties;
	CRsrvParty*                m_pParty[MAX_PARTIES_IN_CONF];
	CStructTm                  m_endTime;                                                               // end   conferences time
	WORD                       m_numParties;                                                            // Number of parties in conference.
	CLectureModeParams*        m_pLectureMode;
	DWORD                      m_repSchedulingId;
	BYTE                       m_time_beforeFirstJoin;                                                  // Autoterimnate
	BYTE                       m_time_afterLastQuit;                                                    // Autoterimnate
	BYTE                       m_LastQuitType;
	BYTE                       m_confLockFlag;                                                          // YES / NO
	BYTE                       m_confOnHoldFlag;                                                        // YES / NO
	BYTE                       m_Mute_Incoming_Parties_Lecture_Mode;                                    // YES / NO
	BYTE                       m_muteAllPartiesAudioExceptLeader;                                      // YES / NO
	BYTE                       m_muteAllPartiesVideoExceptLeader;                                      // YES / NO
	BYTE                       m_FECC_Enabled;                                                          // YES / NO
	WORD                       m_max_parties;
	BYTE                       m_isMeetingRoom;                                                         // YES / NO
	BYTE                       m_meetingRoomReoccurrenceNum;                                            // 0 - one time; 0xFF - endless
	BYTE                       m_meetingRoomState;                                                      // MEETING_ROOM_PASSIVE_STATE - 0;
	BYTE                       m_isGateway;
	BYTE                       m_isVideoInviteGateway;
	WORD                       m_GWDialOutProtocols;
	BYTE                       m_startConfRequiresLeaderOnOff;
	BYTE                       m_terminateConfAfterChairDropped;
	char                       m_entry_password[CONFERENCE_ENTRY_PASSWORD_LEN];                         // conference le entry password
	DWORD                      m_ipVideoRate;                                                           // relevant only for 323conf.
	COperMask                  m_partyListSlowInfoMask;
	COperMask                  m_partyListFastInfoMask;
	COperMask                  m_partyListDelPartyMask;
	COperMask                  m_slowInfoMask;
	COperMask                  m_fastInfoMask;
	COperMask                  m_slow1InfoMask;
	DWORD                      m_slowUpdateCounter;
	DWORD                      m_fastUpdateCounter;
	DWORD                      m_dwSummaryUpdateCounter;
	DWORD                      m_dwFullUpdateCounter;
	DWORD                      m_SummeryCreationUpdateCounter;
	WORD                       m_slowInfoFlag;
	WORD                       m_fastInfoFlag;
	WORD                       m_slow1InfoFlag;
	WORD                       m_infoOpcode;
	BYTE                       m_isSameLayout;
	BYTE                       m_advancedAudioConf;
	DWORD                      m_dwConfFlags;                                                           // Serialized only in SerializeShortConf
	DWORD                      m_dwConfFlags2;                                                           // Serialized only in SerializeShortConf// Serialized only in SerializeShortConf
	char                       m_confRemarks[CONF_REMARKS_LEN];
	BYTE                       m_isVideoPlus;
	BYTE                       m_isVideoPlusConf;
	char*                      m_confRemarksHistory[NUM_REMARKS_HISTORY];
	CVisualEffectsParams*      m_pVisualEffectsInfo;
	CMessageOverlayInfo*       m_pMessageOverlayInfo;
	CSiteNameInfo*             m_pSiteNameInfo;
	CAutoScanOrder*            m_pAutoScanOrder;
	BYTE                       m_meetMePerEntryQ;                                                       // YES, NO
	BYTE                       m_IsEntryQ;                                                              // YES, NO
	BYTE                       m_IsAdHoc;                                                               // YES, NO
	DWORD                      m_dwAdHocProfileId;
	char                       m_baseProfileName[H243_NAME_LEN];
	BYTE                       m_isConfTemplate;
	BYTE                       m_InviteParty;                                                           // YES, NO
	BYTE                       m_confSecureFlag;
	BYTE                       m_dualVideoMode;
	BYTE                       m_EnterpriseMode;
	BYTE                       m_EnterpriseModeFixedRate;                                               // 'Manual Control of Content Line Rate' feature
	BYTE                       m_PresentationProtocol;
	BYTE                       m_cascadeOptimizeResolution;
	eSubCPtype                 m_SubCPtype;
	BYTE                       m_IsConfOnPort;                                                          // YES, NO
	char                       m_NumericConfId[NUMERIC_CONFERENCE_ID_LEN];
	CConfContactInfo*          m_pConfContactInfo;
	CSmallString               m_BillingInfo;
	BYTE                       m_isAutoLayout;
	BYTE                       m_isTemplate;
	BYTE                       m_network;
	BYTE                       m_encryption;
	BYTE                       m_eEncryptionType;
	BYTE                       m_RecordingLinkControl;
	BYTE                       m_bChanged;
	BYTE                       m_IsCascadeEQ;                                                           // YES, NO
	WORD                       m_current_numb_of_party;
	WORD                       m_ind;
	WORD                       m_ind_service_phone;
	WORD                       m_ind_rsrv_vid_layout;
	DWORD                      m_partyIdCounter;
	DWORD                      m_unrsrvPartiesCounter;
	BYTE                       m_confType;
	BYTE                       m_media;
	BYTE                       m_EnableRecording;                                                       // YES, NO
	BYTE                       m_EnableRecordingIcon;                                                   // YES, NO
	BYTE                       m_StartRecPolicy;
	char                       m_RecLinkName[H243_NAME_LEN];
	BYTE                       m_IsAudioOnlyRecording;                                                  // YES, NO
	BYTE                       m_IsEnableRecNotify;							//YES, NO
	BYTE                       m_isLpr;                                                                 // YES, NO
	BYTE                       m_EchoSuppression;                                                       // YES, NO
	BYTE                       m_KeyboardSuppression;                                                   // YES, NO
	BYTE                       m_isAutoMuteNoisyParties;                                                // YES, NO
	WORD                       m_AutoScanInterval;                                                      // auto scan interval in seconds
	BYTE                       m_isSIPFactory;                                                          // YES, NO
	BYTE                       m_isAutoConnectSIPFactory;                                               // YES, NO
	BYTE                       m_isAdHocConf;                                                           // YES, NO this conf born AdHoc by EntryQueue
	DWORD                      m_webReservUId;
	DWORD                      m_webOwnerUId;
	DWORD                      m_webDBId;
	BYTE                       m_webReserved;
	eVideoQuality              m_videoQuality;
	BYTE                       m_isTelePresenceMode;
	BYTE                       m_manageTelepresenceLayoutInternaly;
	EHDResolution              m_HDResolution;
	BYTE                       m_H264VSWHighProfilePreference;
	BYTE                       m_isVideoClarityEnabled;
	BYTE                       m_isSiteNamesEnabled;
	DWORD                      m_internalConfStatus;
	BYTE                       m_operatorConf;                                                          // create operator conf
	COperatorConfInfo*         m_pOperatorConfInfo;
	BYTE                       m_ShowContentAsVideo;                                                    // Legacy YES,NO
	BOOL                       m_bIsExclusiveContentMode;
	char                       m_appoitnmentID[APPOITNMENT_ID_LEN];
	char                       m_meetingOrganizer[H243_NAME_LEN];
	BYTE                       m_isStreaming;
	BYTE                       m_bGatheringEnabled;                                                     // YES, NO
	ELanguges                  m_eLanguage;
	char                       m_sIpNumberAccess[ONE_LINE_BUFFER_LEN];
	char                       m_sNumberAccess_1[ONE_LINE_BUFFER_LEN];
	char                       m_sNumberAccess_2[ONE_LINE_BUFFER_LEN];
	char                       m_sFreeText_1[ONE_LINE_BUFFER_LEN];
	char                       m_sFreeText_2[ONE_LINE_BUFFER_LEN];
	char                       m_sFreeText_3[ONE_LINE_BUFFER_LEN];
	CStructTm                  m_dtExchangeConfStartTime;
	BYTE                       m_telePresenceModeConfiguration;
	BYTE                       m_telePresenceLayoutMode;
	BYTE                       m_isCropping;
	BYTE                       m_AutoRedial;                                                            // YES,NO
	BYTE                       m_isPermanent;
	CCOPConfigurationList*     m_pCopConfigurationList;
	BYTE                       m_ResSts;                                                                // Status Field
	BYTE                       m_isAudioClarity;
	WORD                       m_confSpeakerChangeMode;
	BYTE                       m_speakerChangeThreshold;
	BYTE                       m_isAutoBrightness;
	BYTE                       m_ivrProviderEQ;
	BYTE                       m_externalIvrControl;                                                    // AT&T
	char                       m_ServiceNameForMinParties[ONE_LINE_BUFFER_LEN];
	ServiceRegistrationContent m_ServiceRegistrationContent[NUM_OF_IP_SERVICES];
	BYTE                       m_SipRegistrationTotalSts;                                               // sipProxySts
	BYTE                       m_TipCompatibility;
	BYTE                       m_avMcuCascadeVideoMode;
	eConfMediaType             m_confMediaType;                                                         // SVC-Only, AVC-Only, Mix(AVC or/and SVC)
	EOperationPointPreset      m_eOperationPointPreset;
	DWORD                      m_natKeepAlivePeriod;
	BYTE                       m_FontType;                                                              // 'Font Types' feature
	BYTE                       m_ContentMultiResolutionEnabled;                                                   //YES,NO
	BYTE                       m_ContentXCodeH264Supported;                                            //YES,NO
	BYTE                       m_ContentXCodeH263Supported;                                            //YES,NO
	BYTE                       m_isCascadeOptimized;                                                              // 'Font Types' feature
	BYTE                       m_isAsSipContent;
	BYTE                       m_isHighProfileContent;
	char                       m_FocusUriScheduling[ONE_LINE_BUFFER_LEN];   //For AV_MCU conf MS - Scheduled by DMA
	char                       m_FocusUriCurrently[ONE_LINE_BUFFER_LEN];   //For AV_MCU conf MS - P2P esculate/MeetMe

	WORD                       m_mrcMcuId;                                                             // SIP Cascade
	BYTE                       m_SrsPlaybackLayout;
	BYTE                       m_overideProfileLayout;

	//Indication for audio participants
	WORD                       m_IconDisplayPosition;
	BYTE                       m_EnableAudioParticipantsIcon;                                                   // YES, NO
	BYTE                       m_AudioParticipantsIconDisplayMode;							      //Permanent, on audio participants change
	WORD                       m_AudioParticipantsIconDuration;									  //3s - 300s when mode is audio participants change
	BYTE                       m_EnableSelfNetworkQualityIcon;
};


#endif
