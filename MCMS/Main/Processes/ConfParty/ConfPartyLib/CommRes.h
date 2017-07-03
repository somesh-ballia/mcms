#ifndef _COMMRES_H__
#define _COMMRES_H__

#include "CommResApi.h"
#include "RsrvParty.h"
#include "H221.h"
#include "CommResDB.h"
#include "ConfPartySharedDefines.h"
#include "ProcessBase.h"
#include "SysConfig.h"
#include "InternalProcessStatuses.h"
#include "CdrApiClasses.h"
#include "AutoScanOrder.h"
#include "ConfIpParameters.h"


class CRsrvParty;
class CCommConfDB;

extern CCommConfDB* GetpConfDB();
extern CCommResDB*  GetpMeetingRoomDB();
extern CCommResDB*  GetpProfilesDB();
extern CCommResDB*  GetpConfTemplateDB();

typedef eCascadeOptimizeResolutionEnum eCascadeResolution;

////////////////////////////////////////////////////////////////////////////
//                        CCommRes
////////////////////////////////////////////////////////////////////////////
class CCommRes : public CCommResApi
{
	CLASS_TYPE_1(CCommRes, CCommResApi)

public:
	                      CCommRes();
	                      CCommRes(const CCommRes& other);
	                      CCommRes(const CCommResApi& other);
	virtual              ~CCommRes();
	virtual const char*   NameOf() const { return "CCommRes";}
	CCommRes& operator    =(const CCommRes& other);

	int                   AddOld(const CRsrvParty& other);
	int                   AddTmp(const CRsrvParty& other);
	int                   Update(const CRsrvParty& other);
	int                   UpdateById(const CRsrvParty& other);
	int                   Cancel(const char* name);
	int                   Cancel(const DWORD partyId);
	int                   FindId(const DWORD partyId);

	CRsrvParty*           GetFirstParty();
	CRsrvParty*           GetNextParty();
	CRsrvParty*           GetFirstParty(int& nPos);
	CRsrvParty*           GetNextParty(int& nPos);

	CRsrvParty*           GetCurrentParty(const char* name) const;
	CRsrvParty*           GetCurrentParty(const DWORD partyId) const;
	DWORD                 GetLastPartyId() const;

	void                  SetName(const char* name);
	void                  SetDisplayName(const char* name);

	WORD                  GetNumVideoParties() const;       // returns the number of parties that are not unframed
	WORD                  GetNumHighQVideoParties() const;  // number of parties with high quality video

	void                  ResetStartTime();
	BOOL                  isValidStartTime();

	WORD                  IsEntryTone() const;
	WORD                  IsExitTone() const;
	WORD                  IsEndTimeAlertTone() const;

	void                  SetEndTimeAlertTone(WORD bl);
	WORD                  CorrectPartyTransferRate();
	void                  CorrectPartyMeetMeMethod();
	void                  CorrectSubService();
	void                  CorrectService();
	void                  CorrectDefaultValue();
	WORD                  TestValidReserv();
	STATUS                IsConfNameExists() const;
	STATUS                IsProfileExists(DWORD& profileId);

	BYTE                  GetNetChannelNumber() const;
	BYTE                  GetBonding() const;

	int                   TestAdvancedLayoutsValidity();
	int                   TestRegularLayoutsValidity();
	void                  SetOperatorConfInfo();
	STATUS                TestOperatorConfValidity(const char* loginName);

	WORD                  PhoneNumberLenTest();

	void                  SetServicePhoneToShort(CCommResShort& shortRes);

	WORD                  SetConfDialinPrefix();

	static CStructTm      CalculateConfEndTime(const CStructTm& startTime, const CStructTm& duration);
	static CStructTm      CalculateConfDuration(CStructTm& startTime, CStructTm& endTime);

	void                  SetAudioConf(BYTE audioConf);

	BYTE                  GetConfProtocol() const;
	void                  SetConfProtocol(const BYTE confProtocol);

	BYTE                  GetExternalMaster() const;
	void                  SetExternalMaster(const BYTE externalMaster);

	BYTE                  GetRestrict() const;
	void                  SetRestrict(const BYTE restrict);

	BYTE                  GetLSDRate() const;
	void                  SetLSDRate(const BYTE LSDRate);

	BYTE                  GetHSDRate() const;
	void                  SetHSDRate(const BYTE HSDRate);

	WORD                  GetCoughDelay() const;
	void                  SetCoughDelay(const WORD coughDelay);

	BYTE                  GetT120Rate() const;
	void                  SetT120Rate(const BYTE T120_rate);

	BYTE                  GetGuestOperator() const;
	void                  SetGuestOperator(const BYTE guest_oper);

	WORD                  GetNumServicePhone() const;
	void                  SetNumServicePhone(const WORD num);

	BYTE                  GetAdvancedAudioConf() const;
	void                  SetAdvancedAudioConf(BYTE advancedAudioConf);

	BYTE                  GetDebugDummy();
	void                  SetDebugDummy(BYTE dummy);

	BYTE                  GetCascadeEQ() const;
	void                  SetCascadeEQ(const BYTE IsCascadeEQ);

	eLayoutBorderWidth    GetLayoutBorderWidth() const;
	void                  SetLayoutBorderWidth(BYTE Y, BYTE U, BYTE V);

	BYTE                  IsAutoConnectFactory() const                             { return m_isAutoConnectSIPFactory; }
	void                  SetAutoConnectFactory(const BYTE autoConnect)            { m_isAutoConnectSIPFactory = autoConnect; }

	DWORD                 GetH323videoRate() const                                 { return m_ipVideoRate; }
	void                  SetH323videoRate(const DWORD h323videoRate)              { m_ipVideoRate = h323videoRate; }

	DWORD                 GetConfFlags()                                           { return m_dwConfFlags; }
	void                  SetConfFlags(const DWORD conf_flags)                     { m_dwConfFlags = conf_flags; }
	void                  SetConfFlags2(const DWORD conf_flags2)                   { m_dwConfFlags2 = conf_flags2; }

	DWORD                 GetwebReservUId() const                                  { return m_webReservUId; }
	void                  SetwebReservUId(const DWORD newWebReservUId)             { m_webReservUId = newWebReservUId; }

	DWORD                 GetwebOwnerUId() const                                   { return m_webOwnerUId; }
	void                  SetwebOwnerUId(const DWORD newWebOwnerUId)               { m_webOwnerUId = newWebOwnerUId; }

	DWORD                 GetwebDBId() const                                       { return m_webDBId; }
	void                  SetwebDBId(const DWORD newWebDBId)                       { m_webDBId = newWebDBId; }

	BYTE                  GetwebReserved() const                                   { return m_webReserved; }
	void                  SetwebReserved(const BYTE newwebReserved)                { m_webReserved = newwebReserved; }

	BYTE                  GetIsSameLayout() const                                  { return m_isSameLayout; }
	void                  SetIsSameLayout(const BYTE newIsSameLayout)              { m_isSameLayout = (newIsSameLayout) ? YES : NO; }

	BYTE                  GetConfType() const                                      { return m_confType; }
	void                  SetConfType(const BYTE confType)                         { m_confType = confType; }

	BYTE                  GetMedia() const                                         { return m_media; }
	void                  SetMedia(const BYTE media)                               { m_media = media; }

	BYTE                  GetEnterpriseMode() const                                { return m_EnterpriseMode; }
	void                  SetEnterpriseMode(const BYTE enterpriseMode)             { m_EnterpriseMode = enterpriseMode; }

	BYTE                  GetEnterpriseModeFixedRate() const                       { return m_EnterpriseModeFixedRate; }
	void                  SetEnterpriseModeFixedRate(BYTE fixedRate)               { m_EnterpriseModeFixedRate = fixedRate; }

	const CStructTm*      GetStartTime() const                                     { return &m_startTime; }
	void                  SetStartTime(const CStructTm& other)                     { m_startTime = other; }

	BYTE                  IsStandBy() const                                        { return m_stand_by; }
	void                  SetStandBy(const BYTE stand_by)                          { m_stand_by = stand_by; }

	BYTE                  IsAutomaticTermination() const                           { return m_automaticTermination; }
	void                  SetAutomaticTermination(const BYTE automaticTermination) { m_automaticTermination = automaticTermination; }

	BYTE                  GetAudioRate() const                                     { return m_audioRate; }
	void                  SetAudioRate(const BYTE audioRate)                       { m_audioRate = audioRate; }

	BYTE                  IsVideo() const                                          { return m_video; }
	void                  SetVideo(const BYTE video)                               { m_video = video; }

	BYTE                  GetVideoPictureFormat() const                            { return m_videoPictureFormat; }
	void                  SetVideoPictureFormat(const BYTE videoPictureFormat)     { m_videoPictureFormat = videoPictureFormat; }

	BYTE                  GetCIFframeRate() const                                  { return m_CIFframeRate; }
	void                  SetCIFframeRate(const BYTE CIFframeRate)                 { m_CIFframeRate = CIFframeRate; }

	BYTE                  GetQCIFframeRate() const                                 { return m_QCIFframeRate; }
	void                  SetQCIFframeRate(const BYTE QCIFframeRate)               { m_QCIFframeRate = QCIFframeRate; }

	BYTE                  GetAlertToneTiming() const                               { return m_alertToneTiming; }
	void                  SetAlertToneTiming(const BYTE alertToneTiming)           { m_alertToneTiming = alertToneTiming; }

	WORD                  GetTalkHoldTime() const                                  { return m_talkHoldTime; }
	void                  SetTalkHoldTime(const WORD talkHoldTime)                 { m_talkHoldTime = talkHoldTime; }

	BYTE                  GetAudioMixDepth() const                                 { return m_audioMixDepth; }
	void                  SetAudioMixDepth(const BYTE audioMixDepth)               { m_audioMixDepth = audioMixDepth; }

	BYTE                  GetVideoSession() const                                  { return m_videoSession; }
	void                  SetVideoSession(const BYTE videoSession)                 { m_videoSession = videoSession; }

	BYTE                  GetEQVideoSession() const                                { return m_EQ_videoSession; }
	void                  SetEQVideoSession(const BYTE EQvideoSession)             { m_EQ_videoSession = EQvideoSession; }

	BYTE                  GetVideoProtocol() const                                 { return m_videoProtocol; }
	void                  SetVideoProtocol(const BYTE videoProtocol)               { m_videoProtocol = videoProtocol; }

	BYTE                  GetTimeBeforeFirstJoin() const                           { return m_time_beforeFirstJoin; }
	void                  SetTimeBeforeFirstJoin(const BYTE time_beforeFirstJoin)  { m_time_beforeFirstJoin = time_beforeFirstJoin; }

	BYTE                  GetTimeAfterLastQuit() const                             { return m_time_afterLastQuit; }
	void                  SetTimeAfterLastQuit(const BYTE time_afterLastQuit)      { m_time_afterLastQuit = time_afterLastQuit; }

	BYTE                  GetIsVideoInvite() const                                 { return m_isVideoInviteGateway; }
	void                  SetIsVideoInvite(const BYTE isVideoInvite)               { m_isVideoInviteGateway = isVideoInvite; }

	WORD                  GetMaxParties() const                                    { return m_max_parties; }
	void                  SetMaxParties(const WORD max_parties)                    { m_max_parties = max_parties; }

	WORD                  GetNumRsrvVidLayout() const                              { return m_numRsrvVideoSource; }
	void                  SetNumRsrvVidLayout(const WORD num)                      { m_numRsrvVideoSource = num; }

	BYTE                  GetIsVideoPlusConf() const                               { return m_isVideoPlusConf; }
	void                  SetIsVideoPlusConf(BYTE isVideoPlusConf)                 { m_isVideoPlusConf = isVideoPlusConf; }

	DWORD                 GetBackgroundImageID() const                             { return m_pVisualEffectsInfo->GetBackgroundImageID(); }
	void                  SetBackgroundImageID(DWORD imageID)                      { m_pVisualEffectsInfo->SetBackgroundImageID(imageID); }

	DWORD                 GetBckgColorYUV() const                                  { return m_pVisualEffectsInfo->GetBackgroundColorYUV(); }
	void                  SetBckgColorYUV(BYTE Y, BYTE U, BYTE V)                  { m_pVisualEffectsInfo->SetBackgroundColorYUV(m_pVisualEffectsInfo->CalcYUVinDWORD(Y, U, V)); }

	DWORD                 GetLayoutBorderColorYUV() const                          { return m_pVisualEffectsInfo->GetlayoutBorderColorYUV(); }
	void                  SetLayoutBorderColorYUV(BYTE Y, BYTE U, BYTE V)          { m_pVisualEffectsInfo->SetlayoutBorderColorYUV(m_pVisualEffectsInfo->CalcYUVinDWORD(Y, U, V)); }

	DWORD                 GetSpeakerNotColorYUV() const                            { return m_pVisualEffectsInfo->GetSpeakerNotationColorYUV(); }
	void                  SetSpeakerNotColorYUV(BYTE Y, BYTE U, BYTE V)            { m_pVisualEffectsInfo->SetSpeakerNotationColorYUV(m_pVisualEffectsInfo->CalcYUVinDWORD(Y, U, V)); }

	BOOL                  GetUseYUVcolor() const                                   { return m_pVisualEffectsInfo->UseYUVcolor(); }
	void                  SetUseYUVcolor(BOOL yesNo)                               { m_pVisualEffectsInfo->SetUseYUVcolor(yesNo); }

	BYTE                  IslayoutBorderEnable() const                             { return m_pVisualEffectsInfo->IslayoutBorderEnable(); }
	void                  SetlayoutBorderEnable(BYTE bEnable)                      { m_pVisualEffectsInfo->SetlayoutBorderEnable(bEnable); }

	BYTE                  IsSpeakerNotationEnable() const                          { return m_pVisualEffectsInfo->IsSpeakerNotationEnable(); }
	void                  SetSpeakerNotationEnable(BYTE bEnable)                   { m_pVisualEffectsInfo->SetSpeakerNotationEnable(bEnable); }

	DWORD                 GetSummaryUpdateCounter()                                { return m_dwSummaryUpdateCounter; }
	void                  SetSummaryUpdateCounter(DWORD Counter)                   { m_dwSummaryUpdateCounter = Counter; }

	BYTE                  GetIsAdHocConf() const                                   { return m_isAdHocConf; }
	void                  SetIsAdHocConf(const BYTE isAdHocConf)                   { m_isAdHocConf = isAdHocConf; }

	DWORD                 GetAdHocProfileId() const                                { return m_dwAdHocProfileId; }
	void                  SetAdHocProfileId(const DWORD dwAdHocProfileId)          { m_dwAdHocProfileId = dwAdHocProfileId; }

	BYTE                  GetIsAudioOnlyRecording() const                          { return m_IsAudioOnlyRecording; }
	void                  SetAudioOnlyRecording(const BYTE isAudioOnlyRecording)   { m_IsAudioOnlyRecording = isAudioOnlyRecording; }

	BYTE                  GetLpr() const                                           { return m_isLpr; }
	void                  SetLpr(const BYTE isLpr)                                 { m_isLpr = isLpr; }

	ELanguges             GetLanguage() const                                      { return m_eLanguage; }
	void                  SetLanguage(ELanguges eLang)                             { m_eLanguage = eLang; }

	void                  SetFullUpdateCounter(DWORD Counter)                      { m_dwFullUpdateCounter = Counter; }
	DWORD                 GetSummaryCreationUpdateCounter()                        { return m_SummeryCreationUpdateCounter; }

	WORD                  GetAudioTone() const                                     { return m_audioTone; }
	void                  SetEntryTone(WORD bl);
	void                  SetExitTone(WORD bl);

	BYTE                  GetNetwork() const                                       { return m_network; }
	void                  SetNetwork(const BYTE network);

	BYTE                  GetDualVideoMode() const                                 { return m_dualVideoMode; }
	void                  SetDualVideoMode(const WORD byDualVideoMode);

	BYTE                  GetIsVideoClarityEnabled() const                         { return m_isVideoClarityEnabled; }
	void                  SetIsVideoClarityEnabled(const BYTE IsVideoClarityEnabled);

	const CStructTm*      GetDurationTime() const                                  { return &m_duration; }
	void                  SetDurationTime(const CStructTm& other);

	BYTE                  GetMeetMePerConf() const                                 { return m_meetMePerConf; }
	void                  SetMeetMePerConf(const BYTE meetMePerConf);

	const char*           GetH243Password() const                                  { return m_H243_password; }
	void                  SetH243Password(const char* password);

	BYTE                  GetCascadeMode() const                                   { return m_cascadeMode; }
	void                  SetCascadeMode(const BYTE cascadeMode);

	BYTE                  GetCascadedLinksNumber() const                           { return m_cascadedLinksNumber; }
	void                  SetCascadedLinksNumber(BYTE cascadedLinksNumber)         { m_cascadedLinksNumber = cascadedLinksNumber; SLOWCHANGE; }

	CLectureModeParams*   GetLectureMode()                                         { return m_pLectureMode; }
	void                  SetLectureMode(const CLectureModeParams& otherLectureMode);

	BYTE                  GetConfOnHoldFlag() const                                { return m_confOnHoldFlag; }
	void                  SetConfOnHoldFlag(const BYTE confOnHoldFlag);

	BYTE                  GetConfLockFlag() const                                  { return m_confLockFlag; }
	void                  SetConfLockFlag(const BYTE confLockFlag);

	BYTE                  IsConfSecured()                                          { return m_confSecureFlag; }
	void                  SetConfSecured(BYTE ConfSecureFlag);

	BYTE                  GetIsInviteParty() const                                 { return m_InviteParty; }
	void                  SetIsInviteParty(const BYTE InviteParty);

	DWORD                 NextPartyId();
	void                  SetNextPartyId(DWORD nextPartyId)                        { m_partyIdCounter = nextPartyId; }

	DWORD                 NextUnrsrvPartiesCounter();
	void                  SetUnrsrvPartiesCounter(DWORD unrsrvPartiesCounter)      { m_unrsrvPartiesCounter = unrsrvPartiesCounter; }

	const char*           GetEntryPassword() const                                 { return m_entry_password; }
	void                  SetEntryPassword(const char* leader_password);

	const char*           GetConfContactInfo(int ContactNumber) const              { return m_pConfContactInfo->GetContactInfo(ContactNumber); }
	void                  SetConfContactInfo(const char* ContactInfo, int ContactNumber);

	const char*           GetBillingInfo() const                                   { return m_BillingInfo.GetString(); }
	void                  SetBillingInfo(const char* BillingInfo);

	BYTE                  GetAdHoc() const                                         { return m_IsAdHoc; }
	void                  SetAdHoc(const BYTE IsAdHoc);

	BYTE                  GetMeetMePerEntryQ() const                               { return m_meetMePerEntryQ; }
	void                  SetMeetMePerEntryQ(const BYTE meetMePerEntryQ);

	BYTE                  GetEnableRecording() const                               { return m_EnableRecording; }
	void                  SetEnableRecording(const BYTE EnableRecording);

	BYTE                  GetStartRecPolicy() const                                { return m_StartRecPolicy; }
	void                  SetStartRecPolicy(const BYTE StartRecPolicy);

	BYTE                  GetRecordingLinkControl()                                { return m_RecordingLinkControl; }
	void                  SetRecordingLinkControl(BYTE RecordingLinkControl);

	const char*           GetRecLinkName() const                                   { return m_RecLinkName; }
	void                  SetRecLinkName(const char* RecLinkName)                  { strcpy_safe(m_RecLinkName, RecLinkName); }

	BYTE                  GetIsAutoLayout() const                                  { return m_isAutoLayout; }
	void                  SetIsAutoLayout(const BYTE newIsAutoLayout);

	const char*           GetIpNumberAccess() const                                { return m_sIpNumberAccess; }
	void                  SetIpNumberAccess(const char* pszIpNo)                   { strcpy_safe(m_sIpNumberAccess, pszIpNo); }

	const char*           GetNumberAccess1() const                                 { return m_sNumberAccess_1; }
	void                  SetNumberAccess1(const char* pszNo)                      { strcpy_safe(m_sNumberAccess_1, pszNo); }

	const char*           GetNumberAccess2() const                                 { return m_sNumberAccess_2; }
	void                  SetNumberAccess2(const char* pszNo)                      { strcpy_safe(m_sNumberAccess_2, pszNo); }

	const char*           GetFreeText1() const                                     { return m_sFreeText_1; }
	void                  SetFreeText1(const char* pszText)                        { strcpy_safe(m_sFreeText_1, pszText); }

	const char*           GetFreeText2() const                                     { return m_sFreeText_2; }
	void                  SetFreeText2(const char* pszText)                        { strcpy_safe(m_sFreeText_2, pszText); }

	const char*           GetFreeText3() const                                     { return m_sFreeText_3; }
	void                  SetFreeText3(const char* pszText)                        { strcpy_safe(m_sFreeText_3, pszText); }

	bool                  IsGatheringEnabled() const                               { return m_bGatheringEnabled; }
	void                  SetGatheringEnabled(bool bEnable = true);

	BYTE                  GetIsAutoRedial() const                                  { return m_AutoRedial; }
	void                  SetAutoRedial(const BYTE autoRedial);

	BYTE                  GetFontType() const                                      { return m_FontType; }
	void                  SetFontType(BYTE fontType)                               { m_FontType = fontType; }

	CAvMsgStruct*         GetpAvMsgStruct()                                        { return m_pAvMsgStruct; }
	const CStructTm*      GetCalculatedEndTime()                                   { return &m_endTime; }
	WORD                  GetNumParties() const                                    { return m_numParties; }
	BYTE                  GetPresentationProtocol() const                          { return m_PresentationProtocol; }
	void                  SetPresentationProtocol(BYTE presentationProtocol);
	eCascadeResolution    GetCascadeOptimizeResolution() const                     { return (eCascadeResolution)m_cascadeOptimizeResolution; }
	BYTE                  GetContinuousPresenceScreenNumber() const                { return m_contPresScreenNumber; }
	BYTE                  GetStartConfRequiresLeaderOnOff() const                  { return m_startConfRequiresLeaderOnOff; }
	BYTE                  GetTerminateConfAfterChairDropped() const                { return m_terminateConfAfterChairDropped;}
	CVisualEffectsParams* GetVisualEffects()                                       { return m_pVisualEffectsInfo; }
	COperatorConfInfo*    GetOperatorConfInfo()                                    { return m_pOperatorConfInfo; }
	BYTE                  GetChanged() const                                       { return m_bChanged; }

	void                  AutoCorrectPartyLayoutData();
	void                  AutoCorrectManageLayoutInternally();

	void                  SetConfRemarks(const char* remarks);

	void                  TranslateRGBColorToYUV();

	void                  UpdateVisualEffectsInfo(CVisualEffectsParams* pVisualEffects);

	BYTE                  IsDefinedIVRService(void) const;

	void                  IncreaseSummaryUpdateCounter();

	void                  SetDefaultParams();
	STATUS                SetMRDefaultParams();
	void                  SetAdHocConfBasicParams(char* confName, const char* numericID, DWORD dwAdHocProfileId);
	void                  SetAdHocConfExtDbBasedParams(char* display_name, char* maxParties, char* minParties, char* password, char* entryPwd, char* billingData, char* owner, char* info1, char* info2, char* info3);
	void                  SetSysCfgParams();

	STATUS                TestValidity();
	STATUS                TestValidityAndSetWarnings();
	STATUS                TestMRValidity();
	STATUS                TestReservValidityWithoutProfileBasedParams();
	STATUS                TestReservValidityOfCommonParams();
	STATUS                TestAddValidity(BYTE isDefaultProfile = FALSE);
	STATUS                TestAndUpdateGWProfileValidity();
	STATUS                TestUpdateValidity();
	STATUS                TestUpdateDisplayName() const;
	STATUS                TestMinAudioVideoPartiesValidity();
	STATUS                TestCopConfigurationValidity();
	STATUS                TestReservValidity();
	STATUS                TestPartyValidity(CRsrvParty* pParty);
	STATUS                TestPartyRsrvValidity(CRsrvParty* pRsrvParty);
	STATUS                TestPartyUniqueParamsValidity(CRsrvParty* pParty);
	STATUS                CheckSipUriValidity(CRsrvParty* pParty, CConfIpParameters* pServiceParams);
	void                  RemoveIPAddress(CRsrvParty* pParty);
	STATUS                TestPartyConfRelatedParamsValidity(CRsrvParty* pParty);
	STATUS                TestCascadeLinkValidity(CRsrvParty* pParty);
	STATUS                TestTipCompatibilityValidity();
	STATUS                TestMsSvcVideoModeValidity();
	STATUS                TestLicenseExpiredValidity();
	STATUS                TestLicenseSvcConfValidity();

	void                  FitVideoSessionMode();

	void                  SetAdHocConfParams();

	void                  FixPartyUniqueParams(CRsrvParty* pParty);
	STATUS                CheckReservRangeValidity(BYTE& errorCode);

	STATUS                CheckIvrServiceValidity();
	STATUS                CheckPasswordsValidity();
	STATUS                CheckPasswordValidity(const char* password, DWORD minPasswordLength, DWORD maxPasswordLength);
	STATUS                CheckPasswordRepeatedDigitsValidity(const char* pszPassword, int nRepeatedMax);
	STATUS                CheckStrongPasswordValidity(CObjString& passwordStatusDescription);

	STATUS                TestLectureModeValidity(CLectureModeParams* pLectureModeParams) const;
	void                  SetSetLectureModeSameLayoutRules(CLectureModeParams* pLectureModeParams);
	void                  AutoCorrectTelePresenceParams();
	void                  AutoCorrectShowContentAsVideo();
	void                  AutoCorrectIndicationOnLayout();
	void                  AutoCorrectConfMediaTypeForEQ();
	void                  AutoCorrectConfRate();
	STATUS                TestReservRecordingValidity();
	STATUS                TestEncryptionConfValidity();
	STATUS                TestEncryptionPartyValidity(CRsrvParty* pParty);
	void                  ReplaceCertainCharactersInDisplayNameWithSpaces();
	void                  ReplaceInvalidCharactersInRoutingNameWithUnderScore();

	std::string           GetFileUniqueName(const std::string& dbPrefix) const;
	bool                  IsPartyDefined(const mcTransportAddress* pPhoneLenOrIP, const char* alias, DWORD& dwPartyId, BYTE interfaceType, CH323Alias* PartySrcAliasList = NULL, int numofalias = 0);
	DWORD                 FindMyMRMonitorIdByConfName();
	void                  SetMonitorIdForAllParties();
	void                  ResetMonitorIdForAllParties();

	void                  CopyToShortAllPrefixStr(CCommResShort& shortRes) const;
	void                  SetHDVSWDefaultSettings();
	void                  SetHDVSWDefaultlayout();

	void                  FillEmptyDiplayNameOrName(const char* profileName = NULL);
	bool                  IsEmptyDiplayNameOrName();

	void                  SetDefaultParamsAccordingToSystemCardsBasedMode();
	void                  SetDefaultParamsAccordingToProductType();
	WORD                  GetNumOfVideoParties();
	WORD                  GetNumOfAudioOnlyParties();

	void                  SetOperatorConfFromProfile();

	void                  InitPartiesMoveInfo();

	BYTE                  GetIsCOP() const;
	void                  SetCopDefaultSettings();

	void                  AutoCorrectPermanentConf();
	DWORD                 TestPermanentConfValidity();
	void                  AutoCorrectEncryptionParams();

	STATUS                CheckCurrReservSysMode();                          // 2 modes cop/cp
	STATUS                CheckIvrProviderEQValidity();
	STATUS                CheckExternalIvrControlValidity();                 // AT&T
	STATUS                CheckIvrProviderEQAndExternalIvrControlValidity(); // AT&T

	STATUS                CheckIvrProductTypeValidity();					 // EE-462 IVR phase 1: PRE-Conference IVR (VEQ)
	STATUS                TestConfMediaTypeValidity();                       // CMT@
	void                  SetConfMediaTypeForCOP();                          // CMT@
	void                  SetDefaultParamsAndAutoCorrectForConfMediaType();
	void                  SetMinConfTransferRateByOpPtPreset(DWORD dwConfRate);
	STATUS                CheckXCodeConfContentValidity();

private:
	WORD                  CalculateIsdnChannelsNumber(BYTE transferRate);
};


#endif // !defined(_COMMRES_H__)

