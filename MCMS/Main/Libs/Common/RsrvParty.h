#if !defined(_RsrvParty_H__)
#define _RsrvParty_H__

#include "SerializeObject.h"
#include "ConfPartyApiDefines.h"
#include "InterfaceType.h"
#include "VideoLayout.h"
#include "UserDefinedInfo.h"
#include "CommResApi.h"
#include "OperMask.h"

class CCommResApi;
class CVideoLayout;
class COperatorConfInfo;
class CMoveInfo;

////////////////////////////////////////////////////////////////////////////
//                        CRsrvParty
////////////////////////////////////////////////////////////////////////////
class CRsrvParty : public CSerializeObject
{
  CLASS_TYPE_1(CRsrvParty, CSerializeObject)

public:
                      CRsrvParty();
                      CRsrvParty(const CRsrvParty& other);
  virtual            ~CRsrvParty();

  CRsrvParty&         operator =(const CRsrvParty& other);

  virtual const char* NameOf() const { return "CRsrvParty";}

  void                SerializeXml(CXMLDOMElement*& pFatherNode) const;
  void                SerializeXml(CXMLDOMElement* pFatherNode, ePartyData party_data_amount /* = FULL_DATA*/);
  int                 DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);
  int                 DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int action);

  void                Serialize(WORD format, std::ostream& m_ostr);
  void                Serialize(WORD format, CSegment& seg);
  void                DeSerialize(WORD format, std::istream& m_istr);
  void                DeSerialize(WORD format, CSegment& seg);

  CSerializeObject*   Clone()                                                   { return new CRsrvParty; }

  int                 convertStrActionToNumber(const char* strAction);

  void                SetName(const char* name)                                 { strcpy_safe(m_H243_partyName, name); }
  const char*         GetName() const                                           { return m_H243_partyName; }

  void                SetPhoneNumber(const char* phoneNumber);
  const char*         GetPhoneNumber() const;

  // it's party monitor id, EMA uses it to communicate with MCMS
  void                SetPartyId(const PartyMonitorID partyId)                  { m_partyId = partyId; }
  PartyMonitorID      GetPartyId() const                                        { return m_partyId; }

  void                SetConnectionType(const BYTE connectionType)              { m_connectionType = connectionType; }
  BYTE                GetConnectionType() const;

  BYTE                GetConnectionTypeOper() const                             { return m_connectionType; }
  BYTE                GetProtocolType() const;

  void                SetNodeType(const BYTE nodeType)                          { m_nodeType = nodeType; }
  BYTE                GetNodeType() const                                       { return m_nodeType; }

  void                SetBondingMode1(const BYTE bondingMode1);
  BYTE                GetBondingMode1() const                                   { return m_bondingMode1; }

  void                SetNetChannelNumber(const BYTE netChannelNumber);
  BYTE                GetNetChannelNumber() const                               { return m_netChannelNumber; }

  void                SetRestrict(const BYTE restrict);
  BYTE                GetRestrict() const                                       { return m_restrict; }
  WORD                IsRestrictOnly() const;

  virtual void        SetVoice(const BYTE voice)                                { m_voice = voice; }
  BYTE                GetVoice() const                                          { return m_voice; } // NO = party is unframed, YES otherwise

  void                SetAGC(const BYTE AGC);
  BYTE                GetAGC() const                                            { return m_AGC; }

  void                SetAudioThreshold(const BYTE audioThreshold)              { m_audioThreshold = audioThreshold; }
  BYTE                GetAudioThreshold() const                                 { return m_audioThreshold; }

  void                SetIdentificationMethod(const BYTE identificationMethod)  { m_identificationMethod = identificationMethod; }
  BYTE                GetIdentificationMethod() const                           { return m_identificationMethod; }

  void                SetCascadeMode(const BYTE cascadeMode)                    { m_cascadeMode = cascadeMode; }
  BYTE                GetCascadeMode() const                                    { return m_cascadeMode; }

  bool                IsCascadeModeSlave() const                                { return (CASCADE_MODE_SLAVE == m_cascadeMode); }
  bool                IsCascadeModeMaster() const                               { return (CASCADE_MODE_MASTER == m_cascadeMode); }
  bool                IsCascadeModeMasterOrSlave() const                        { return ( (CASCADE_MODE_MASTER == m_cascadeMode) || (CASCADE_MODE_SLAVE == m_cascadeMode) ); }

  void                SetCascadedLinksNumber(BYTE cascadedLinksNumber)          { m_cascadedLinksNumber = cascadedLinksNumber; }
  BYTE                GetCascadedLinksNumber() const                            { return m_cascadedLinksNumber; }

  //Multiple links for ITP in cascaded conference feature:
  void                SetPartyType(eTypeOfLinkParty linkType)                   { m_linkType = linkType; }
  eTypeOfLinkParty    GetPartyType() const                                      { return m_linkType; }

  void                SetMeet_me_method(const BYTE meet_me_method)              { m_meet_me_method = meet_me_method; }
  BYTE                GetMeet_me_method() const                                 { return m_meet_me_method; }

  const char*         GetPassword() const                                       { return ""; } // temp!!

  void                SetServiceProviderName(const char* name)                  { strcpy_safe(m_netServiceProviderName, name); }
  const char*         GetServiceProviderName() const                            { return m_netServiceProviderName; }

  void                SetServiceId(WORD id)                                     { m_serviceId = id; }
  WORD                GetServiceId() const                                      { return m_serviceId; }

  void                SetSubServiceName(const char* name)                       { strcpy_safe(m_netSubService, name); }
  const char*         GetSubServiceName() const                                 { return m_netSubService; }

  void                SetBackupServiceName(const char* name)                    { strcpy_safe(m_backupService, name); }
  const char*         GetBackupServiceName() const                              { return m_backupService; }

  void                SetBackupSubServiceName(const char* name)                 { strcpy_safe(m_backupSubService, name); }
  const char*         GetBackupSubServiceName() const                           { return m_backupSubService; }

  void                SetNumType(const BYTE numType)                            { m_numType = numType; }
  BYTE                GetNumType() const                                        { return m_numType; }

  int                 AddCallingPhoneNumber(const char* phoneNumber);
  int                 AddCalledPhoneNumber(const char* phoneNumber);

  int                 CancelCallingPhoneNumber(const char* phoneNumber);
  int                 CancelCalledPhoneNumber(const char* phoneNumber);

  int                 FindCallingPhoneNumber(const char* phoneNumber);
  int                 FindCalledPhoneNumber(const char* phoneNumber);

  WORD                GetNumCallingPhoneNumbers()                               { return m_numCallingPhones; }
  Phone*              GetFirstCallingPhoneNumber();
  Phone*              GetFirstCallingPhoneNumber(int& nPos);
  Phone*              GetNextCallingPhoneNumber();
  Phone*              GetNextCallingPhoneNumber(int& nPos);

  WORD                GetNumCalledPhoneNumbers()                                { return m_numCalledPhones; }
  Phone*              GetFirstCalledPhoneNumber();
  Phone*              GetFirstCalledPhoneNumber(int& nPos);
  Phone*              GetNextCalledPhoneNumber();
  Phone*              GetNextCalledPhoneNumber(int& nPos);

  Phone*              GetNextCallingPhoneNumberOper();
  Phone*              GetNextCallingPhoneNumberOper(int& nPos);

  virtual void        SetNetInterfaceType(const BYTE interfaceType)             { m_netInterfaceType = interfaceType; }
  BYTE                GetNetInterfaceType() const                               { return m_netInterfaceType; }
  BYTE                IsIpNetInterfaceType() const                              {return (m_netInterfaceType == H323_INTERFACE_TYPE || m_netInterfaceType == SIP_INTERFACE_TYPE);}

  void                SetMultiRateMode(const BYTE multiRateMode)                { m_multiRateMode = multiRateMode; }
  BYTE                GetMultiRateMode() const                                  { return m_multiRateMode; }

  void                SetIpAddress(const mcTransportAddress ipAddress);
  mcTransportAddress  GetIpAddress() const                                      { return m_ipAddress; }

  void                SetCallSignallingPort(const WORD callSignallingPort)      { m_callSignallingPort = callSignallingPort; }
  WORD                GetCallSignallingPort() const                             { return m_callSignallingPort; }

  void                SetVideoProtocol(const BYTE videoProtocol)                { m_videoProtocol = videoProtocol; }
  BYTE                GetVideoProtocol() const                                  { return m_videoProtocol; }

  void                SetIsEncrypted(const BYTE isEncrypted)                    { m_encryption = isEncrypted; }
  BYTE                GetIsEncrypted() const                                    { return m_encryption; }

  void                SetVideoRate(const DWORD videoRate)                       { m_videoRate = videoRate; }
  DWORD               GetVideoRate() const                                      { return m_videoRate; }

  void                SetBondingPhoneNumber(const Phone& other)                 { strcpy_safe(m_bondingPhoneNumber.phone_number, other.phone_number); }
  const Phone*        GetBondingPhoneNumber() const                             { return &m_bondingPhoneNumber; }

  void                SetH323PartyAliasType(const WORD h323PartyAliasType)      { m_h323PartyAliasType = h323PartyAliasType; }
  WORD                GetH323PartyAliasType() const                             { return m_h323PartyAliasType; }

  void                SetSipPartyAddressType(const WORD partyAddressType);
  WORD                GetSipPartyAddressType() const                            { return m_sipPartyAddressType; }

  void                SetH323PartyAlias(const char* h323PartyAlias)             { strcpy_safe(m_h323PartyAlias, h323PartyAlias); }
  const char*         GetH323PartyAlias() const                                 { return m_h323PartyAlias; }

  void                SetSipPartyAddress(const char* partyAddress);
  const char*         GetSipPartyAddress() const                                { return m_sipPartyAddress; }
  void 				  StripSipPartyAddressPrefix();

  void                SetAudioVolume(const BYTE audioVolume);
  BYTE                GetAudioVolume() const                                    { return m_audioVolume; }

  void                SetListeningAudioVolume(const BYTE audioVolume);
  BYTE                GetListeningAudioVolume() const                           { return m_listening_audioVolume; }

  void                SetAutoDetect(const BYTE autoDetect)                      { m_autoDetect = autoDetect; }
  BYTE                GetAutoDetect() const                                     { return m_autoDetect; }

  void                SetUndefinedType(const BYTE undefinedType)                { m_undefinedType = undefinedType; }
  BYTE                GetUndefinedType() const                                  { return m_undefinedType; }
  WORD                IsUndefinedParty() const;

  void                SetHighQualityVideo(const BYTE high_qualityVideo);
  BYTE                GetHighQualityVideo() const;

  // Operator only functions
  void                CleanPhoneNumbers();
  void                CleanCallingPhoneNumbers();
  void                CleanCalledPhoneNumbers();

  // Functions for serialize changes
  // Party changes
  void                SetAddPartyMask(WORD onOff);

  void                SetInfoOpcode(WORD opcode)                                { m_infoOpcode = opcode; }
  WORD                GetInfoOpcode() const                                     { return m_infoOpcode; }

  BYTE                GetRecordingPort() const                                  { return FALSE; } // temp!!

  void                SetExtension(const char* extension)                       { strcpy_safe(m_extension, extension); }
  const char*         GetExtension() const                                      { return m_extension; }

  void                SetRefferedToUri(const char* refferedBy);
  const char*         GetRefferedToUri() const                                  { return m_refferedToUri; }

  void                SetRefferedBy(const char* refferedBy);
  const char*         GetRefferedBy() const                                     { return m_refferedByUri; }

  // Private layout
  CVideoLayout*       GetCurPrivateVideoLayout() const; // According to active layout
  WORD                GetNumPrivateVideoLayout() const                          { return m_numPrivateVideoLayout; }
  void                SetNumPrivateVideoLayout(const WORD num);

  int                 AddPrivateVideoLayout(const CVideoLayout& other, BYTE bExistLayout = FALSE);
  int                 UpdatePrivateVideoLayout(const CVideoLayout& other);
  int                 CancelPrivateVideoLayout(const BYTE screenLayout);
  int                 FindPrivateVideoLayout(const CVideoLayout& other);
  int                 FindPrivateVideoLayout(const BYTE screenLayout);

  CVideoLayout*       GetFirstPrivateVideoLayout();
  CVideoLayout*       GetNextPrivateVideoLayout();
  CVideoLayout*       GetFirstPrivateVideoLayout(int& nPos);
  CVideoLayout*       GetNextPrivateVideoLayout(int& nPos);

  CVideoLayout*       GetPrivateVideoLayout(const BYTE screenLayout);
  void                SetIsPrivateLayout(BYTE isPrivateLayout);
  BYTE                GetIsPrivateLayout()                                      { return m_isPrivate; }

  CVideoLayout*       GetRsrvVideoLayout()                                      { return m_pCurRsrvVideoLayout; }

  void                SetIsVip(BYTE isVip);
  BYTE                GetIsVip()                                                { return m_isVip; }

  DWORD               GetSlowUpdateCounter()                                    { return m_slowUpdateCounter; }
  DWORD               GetFastUpdateCounter()                                    { return m_fastUpdateCounter; }
  DWORD               GetSlow1UpdateCounter()                                   { return m_slow_1_UpdateCounter; }
  DWORD               GetCreationUpdateCounter()                                { return m_CreationUpdateCounter; }
  DWORD               GetCompleteUpdateCounter()                                { return m_completeUpdateCounter; }
  void                SetCreationUpdateCounter();
  DWORD               GetFullUpdateCounter();
  void                IncreaseFullUpdateCounter();

  void                RemoveAllLayouts();

  WORD                CheckReservRangeValidity(BYTE& errorCode);

  void                SetUserDefinedInfo(const char* UserInfo, int InfoNumber);
  const char*         GetUserDefinedInfo(int InfoNumber) const;

  void                SetAdditionalInfo(const char* additionalInfo);
  const char*         GetAdditionalInfo() const;

  void                SetRes(CCommResApi* pRes)                                 { m_pRes = pRes; }

  void                SetEnableH323Pstn(BYTE onOff);
  BYTE                GetEnableH323Pstn() const;

  void                SetRecordingLinkParty(BYTE isRecordingLinkParty);
  BYTE                GetRecordingLinkParty() const                             { return m_isRecordingLinkParty; }
  void                SetPlaybackLinkParty(BYTE isPlaybackLinkParty);
  BYTE                GetPlaybackLinkParty() const                             { return m_isPlaybackParty; }
    void                SetLastLayoutForRL(BYTE initLayout);
  BYTE                GetLastLayoutForRL() const                             { return m_lastLayoutForRL; }


  void                SetVideoMute(BYTE isVideoMute)                            { m_isVideoMute = isVideoMute; }
  BYTE                GetVideoMute()                                            { return m_isVideoMute; }

  const char*         GetUserIdentifierString() const                           { return m_UserIdentifierString; }
  void                SetUserIdentifierString(const char* userIdentifierString) { strcpy_safe(m_UserIdentifierString, userIdentifierString); }

  void                SetTelePresenceMode(BYTE telePresenceMode);
  BYTE                GetTelePresenceMode()                                     { return m_telePresenceMode; }

  void                SetMaxResolution(BYTE maxRes)                             { m_maxResolution = maxRes; }
  BYTE                GetMaxResolution() const                                  { return m_maxResolution; }

  void                SetPartyDefaultParams();

  void                SetOperatorParty(COperatorConfInfo& operatorConfInfo);
  BYTE                IsOperatorParty() const;

  CMoveInfo*          GetMoveInfo()                                             { return m_pMoveInfo; }
  COperatorConfInfo*  GetOperatorConfInfo()                                     { return m_pOperatorConfInfo; }

  void                SetSubServiceMode(BYTE bSubSerMode)                       { m_ipSubService = bSubSerMode; }
  BYTE                GetSubServiceMode()                                       { return m_ipSubService; }
  void                SetSubServiceParams();

  void                SetEnableICE(BOOL isEnableSipICE)                         { m_EnableSipICE = isEnableSipICE; }
  BOOL                GetEnableICE()                                            { return m_EnableSipICE; }

  BOOL                IsForceSystemBroadcastVolume();
  BOOL                IsForceSystemListeningVolume();
  void                ForceSystemBroadcastVolumeIfNeeded();
  void                ForceSystemListeningVolumeIfNeeded();
  BYTE                GetDefaultBroadcastVolume();
  BYTE                GetDefaultListeningVolume();

  void                SetTIPPartyType(eTipPartyModeType type)                   { m_partyTypeTIP = type; }
  BYTE                IsTIPSlaveParty() const                                   { return (eTipPartySlave == m_partyTypeTIP); }
  BYTE                IsTIPMasterParty() const                                  { return (eTipPartyMaster == m_partyTypeTIP); }

  void                SetRoomId(WORD roomId)                                    { m_RoomId = roomId; }
  WORD                GetRoomId()                                               { return m_RoomId; }

  BYTE                GetPartyMediaType() const;
  void                SetPartyMediaType(ePMediaType type);

  int                 GetPrecedenceLevel() const								{return m_precedenceLevel;}
  void                SetPrecedenceLevel(int level)								{m_precedenceLevel = level;}

  const char*		  GetPrecedenceDomain() const								{return m_precedenceDomain;}
  void                SetPrecedenceDomain(const char* precedenceDomain)			{SAFE_COPY(m_precedenceDomain, precedenceDomain);}

  void                SetPreDefinedIvrString(const char* szPreDefinedIvrString);
  const char*         GetPreDefinedIvrString() const                     { return m_PreDefinedIvrString; }

  //Multiple links for ITP in cascaded conference feature:
  void                SetMainPartyNumber(DWORD mainPartyNumber)     { m_mainPartyNumber = mainPartyNumber; }
  DWORD               GetMainPartyNumber()                          { return m_mainPartyNumber; }
  void                SetIsActive(BOOL isActive)                    { m_isActive = isActive; }
  BOOL                GetIsActive()                                 { return m_isActive; }

  void				  SetIsCallFromGW(BOOL isCallFromGW)            {m_isCallFromGW = isCallFromGW;}
  BOOL				  GetIsCallFromGW()                             {return m_isCallFromGW;}


  void 				  SetMsftAvmcuState(enMsftAvmcuState avmcuState);
  enMsftAvmcuState    GetMsftAvmcuState();

  void 				  SetIsDMAAVMCUParty(BYTE OnOff);
  BYTE				  GetIsDMAAVMCUParty();

  void                SetIsAvMcuNonCCCPLink(BYTE OnOff);
  BYTE                GetIsAvMcuNonCCCPLink();

  void                   SetFocusUri(const char* value)    		 { SAFE_COPY(m_FocusUri, value);}
  const char*            GetFocusUri() const                       { return m_FocusUri; }

  void                   SetMsConversationId(const char* value)    		 { SAFE_COPY(m_MsConversationId, value);}
  const char*            GetMsConversationId() const                       { return m_MsConversationId; }

  void                				SetMsMediaEscalationStatus(EMsftMediaEscalationStatus isEscalation)  { m_MsftMediaEscalationStatus = isEscalation; }
  EMsftMediaEscalationStatus        GetMsftMediaEscalationStatus()                                       { return m_MsftMediaEscalationStatus; }
   void				SetRsrvPartyIsRdpGw(BOOL   isGw=FALSE) {m_bIsRdpGw = isGw;}
   BOOL		GetRsrvPartyIsRdpGw() {return m_bIsRdpGw;}

  void					SetIsWebRtcCall(BYTE bIsWebRtcCall) {m_bIsWebRtcCall = bIsWebRtcCall;}
  BYTE					GetIsWebRtcCall() {return m_bIsWebRtcCall;}

protected:
  char                m_phoneNumber[PHONE_NUMBER_DIGITS_LEN];
  char                m_H243_partyName[H243_NAME_LEN];    // party name
  PartyMonitorID      m_partyId;
  BYTE                m_connectionType;
  BYTE                m_nodeType;
  BYTE                m_bondingMode1;
  BYTE                m_netChannelNumber;
  char                m_netServiceProviderName[NET_SERVICE_PROVIDER_NAME_LEN];   // service provider
  BYTE                m_restrict;
  BYTE                m_voice;
  BYTE                m_AGC;
  BYTE                m_audioThreshold;

  // dial-out
  char                m_netSubService[NET_SERVICE_PROVIDER_NAME_LEN];
  char                m_backupService[NET_SERVICE_PROVIDER_NAME_LEN];
  char                m_backupSubService[NET_SERVICE_PROVIDER_NAME_LEN];
  BYTE                m_numType;
  Phone*              m_pCallingPhoneNumberList[MAX_CHANNEL_NUMBER];

  //Multiple links for ITP in cascaded conference feature:
  BOOL                m_isActive;              //TRUE=active , FALSE=not active - we don't use it. optional for MLA
  DWORD               m_mainPartyNumber;       //for dial out from few slaves to undefined master - sub will find his main according to this common field
  BYTE                m_telePresenceMode;      //was before the feature, use this field for this feature too

  // dial-in
  BYTE                m_identificationMethod;
  Phone*              m_pCalledPhoneNumberList[MAX_CHANNEL_NUMBER];
  BYTE                m_meet_me_method;
  WORD                m_numCallingPhones;
  WORD                m_numCalledPhones;
  BYTE                m_netInterfaceType;
  BYTE                m_multiRateMode;
  BYTE                m_videoProtocol;         // H261, H263
  BYTE                m_encryption;            // AUTO, YES, NO
  DWORD               m_videoRate;             // relevant only for 323party.
  mcTransportAddress  m_ipAddress;             // relevant only for 323party.
  WORD                m_callSignallingPort;    // relevant only for 323party.
  Phone               m_bondingPhoneNumber;
  WORD                m_h323PartyAliasType;
  char                m_h323PartyAlias[IP_STRING_LEN];
  WORD                m_sipPartyAddressType;
  char                m_sipPartyAddress[IP_STRING_LEN];
  BYTE                m_audioVolume;
  BYTE                m_autoDetect;
  BYTE                m_undefinedType; // Ordinal party - NO

  // Fields for Serialize improvement
  // Following 3 fields contain info about party changes
  COperMask           m_slowInfoMask;
  COperMask           m_slow1InfoMask;
  COperMask           m_fastInfoMask;
  COperMask           m_addPartyMask;
  COperMask           m_completeInfoMask;

  DWORD               m_slowUpdateCounter;
  DWORD               m_fastUpdateCounter;
  DWORD               m_slow_1_UpdateCounter;
  DWORD               m_CreationUpdateCounter;
  DWORD               m_completeUpdateCounter;

  // These flags show whether we need to build the party Serialize stream
  WORD                m_slowInfoFlag;
  WORD                m_slow1InfoFlag;
  WORD                m_fastInfoFlag;
  WORD                m_completeInfoFlag;

  WORD                m_infoOpcode;

  // If this is a dialOut party who calls into a Gateway H320, then that gateway might need the alias
  // of the remote endPoint to where the call should arrive at the end.
  // This extension (=the remote's alias) is, therefore, will be transferred to that gateWay via TCS-4.
  char                m_extension[PARTY_EXTENSION_LENGTH];
  char                m_refferedToUri[IP_STRING_LEN];
  char                m_refferedByUri[IP_STRING_LEN];

  CVideoLayout*       m_pPrivateVideoLayout[MAX_VIDEO_LAYOUT_NUMBER];
  CVideoLayout*       m_pCurRsrvVideoLayout;
  BYTE                m_isPrivate; // YES/NO
  WORD                m_numPrivateVideoLayout;

  CUserDefinedInfo*   m_pUserDefinedInfo;
  BYTE                m_isVip; // YES/NO
  BYTE                m_listening_audioVolume;

  BYTE                m_EnableH323_PSTN;

  CCommResApi*        m_pRes;
  BYTE                m_isRecordingLinkParty;
  BYTE		m_isPlaybackParty;
  BYTE		m_lastLayoutForRL;
  BYTE                m_isVideoMute;
  char                m_UserIdentifierString[USER_IDENTIFIER_STRING_LEN];

  WORD                m_indCalling;
  WORD                m_indCalled;

  // Private layout
  WORD                m_ind_prv_vid_layout;
  BYTE                m_cascadeMode;
  BYTE                m_cascadedLinksNumber;
  BYTE                m_maxResolution;
  eTypeOfLinkParty    m_linkType; // Multiple links for ITP in cascaded conference feature

  BYTE                m_isOperatorParty;
  COperatorConfInfo*  m_pOperatorConfInfo;
  CMoveInfo*          m_pMoveInfo;

  BYTE                m_ipSubService;

  BYTE                m_EnableSipICE;
  BYTE                m_partyTypeTIP;
  WORD                m_RoomId;
  WORD                m_serviceId;
  BYTE                m_ePartyMediaType;

  char                m_PreDefinedIvrString[H243_NAME_LEN];

  enMsftAvmcuState	  m_eMsftAvmcuState;
  BYTE				  m_IsDmaAvMcuParty;
  BYTE                m_isAvMcuNonCCCPCascade;
  char                m_FocusUri[ONE_LINE_BUFFER_LEN];   //For AV_MCU conf MS
  char 				  m_MsConversationId[ONE_LINE_BUFFER_LEN];

  EMsftMediaEscalationStatus	  m_MsftMediaEscalationStatus;

  BYTE				  m_bIsWebRtcCall;


  //============================

  // DiffServ indexes for call
  //============================
  int m_precedenceLevel;
  char m_precedenceDomain[ONE_LINE_BUFFER_LEN];

  BOOL m_isCallFromGW;
   BOOL	m_bIsRdpGw;
};

#endif // !defined(_CRsrvParty_H__)
