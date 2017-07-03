#if !defined(_ConfParty_H__)
  #define _ConfParty_H__

#include "SerializeObject.h"
#include "ConfPartyDefines.h"
#include "RsrvParty.h"
#include "VideoLayout.h"
#include "Observer.h"
#include "H323Alias.h"
#include "SecondaryParameters.h"
#include "SipConfPartyDefinitions.h"
#include "IPUtils.h"
#include "AttendedStruct.h"
#include "H323StrCap.h"
#include "Media.h"

class CPrtMontrBaseParams;
class CIpChannelDetails;
class CH323GatekeeperStatus;
class CH221Str;
class CH323StrCap;
class CAttendedStruct;
class CSIPBandwidthAllocationStatus;
class CMediaList;

//VNGR-26449 - unencrypted conference message
enum EIsUnencrypted
{
  eNo = 0,
  eYes,
  eNotSet
};

// Token/Recap Collision Detection (trcd for short)
typedef enum
{
	etrcdAvailable,
	etrcdTokenHandlingInProgress,
	etrcdRecapInProgress
} eTokenRecapCollisionDetectionType;

////////////////////////////////////////////////////////////////////////////
//                        CConfParty
////////////////////////////////////////////////////////////////////////////
class CConfParty : public CRsrvParty
{
  CLASS_TYPE_1(CConfParty, CRsrvParty)

public:
                                 CConfParty();
                                 CConfParty(const CConfParty& other);
                                 CConfParty(const CRsrvParty& other);
  virtual                       ~CConfParty();

  virtual const char*            NameOf() const                                     { return "CConfParty";}

  CConfParty&                    operator=(const CConfParty& other);

  void                           SerializeDetailsPartyXml(CXMLDOMElement* pConfPartyNode);
  void                           SerializeSpecPartyXml(CXMLDOMElement* pActionNode);
  void                           SerializeDetailsPartyXml_H221CommAndCap(CXMLDOMElement* pConfPartyNode);
  void                           SerializeDetailsPartyXml_IpCommAndCap(CXMLDOMElement* pConfPartyNode);
  void                           SerializeDetailsPartyXml_IpMonitoring(CXMLDOMElement* pConfPartyNode);

  void                           SerializeXml(CXMLDOMElement* pFatherNode) const    { }
  int                            DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action) { return STATUS_OK; }
  void                           SerializeXml(CXMLDOMElement* pOngoPartyNode, int Opcode, ePartyData party_data_amount = FULL_DATA);
  int                            DeSerializeXml(CXMLDOMElement* pConfPartyNode, char* pszError);

  void                           UpdateExtDBUserInfo(const char* UserInfo, int InfoNumber);

  void                           SetVideoLayout(const CVideoLayout& other);
  CVideoLayout*                  GetVideoLayout()                                   { return m_pVideoLayout; }

  void                           SetPartyState(const DWORD partyState, DWORD confId = 0xFFFFFFFF);
  DWORD                          GetPartyState() const                              { return m_party_state; }

  void                           SetNet_channels(WORD onOff, WORD firstPort, WORD numPorts, WORD call_type);
  DWORD                          GetNet_channels() const                            { return m_net_channels; }

  WORD                           IsChannelConnected(const DWORD channel) const;
  WORD                           GetNumChannelConnected() const;

  void                           SetSyncLoss(WORD bl);
  WORD                           IsSyncLoss() const                                 { return ((m_H221_status & 0x00000001) == 1); }

  void                           Set_M_SyncLoss(WORD bl);
  WORD                           Is_M_SyncLoss() const                              { return ((m_H221_status & 0x00000002) == 2); }

  void                           Set_R_SyncLoss(WORD bl);
  WORD                           Is_R_Video_SyncLoss() const                        { return ((m_H221_status & 0x00000004) == 8); }

  void                           Set_L_Video_SyncLoss(WORD bl);
  WORD                           Is_L_Video_SyncLoss() const                        { return ((m_H221_status & 0x00000008) == 8); }

  void                           SetDisconnectCause(const DWORD disconnectCause, DWORD MipErrorNumber = 0);
  DWORD                          GetDisconnectCause() const                         { return m_disconnect_cause; }

  void                           SetQ931DisconnectCause(const DWORD Q931disconnectCause);
  DWORD                          GetQ931DisconnectCause() const                     { return m_Q931_disconnect_cause; }

  void                           SetMipErrorNumber(const DWORD mipErrorNumber);
  DWORD                          GetMipErrorNumber() const                          { return m_mipErrorNumber; }

  void                           SetSecondaryCause(const BYTE secondaryCause);
  BYTE                           GetSecondaryCause() const                          { return m_secondary_cause; }

  void                           SetSecondaryCauseParams(CSecondaryParams secParam);
  CSecondaryParams               GetSecondaryCauseParams() const                    { return m_secondary_cause_params; }

  void                           SetAudio_Member(WORD bl);
  WORD                           IsAudio_Member() const                             { return ((m_member_in_media & 0x00000001) == 1); }

  void                           SetVideo_Member(WORD bl);
  WORD                           IsVideo_Member() const                             { return ((m_member_in_media & 0x00000002) == 2); }

  void                           SetContent_Member(WORD bl);
  WORD                           IsContent_Member() const                           { return ((m_member_in_media & 0x00000020) == 32); }

  void                           SetAudioMuteByOperator(WORD bl);
  WORD                           IsAudioMutedByOperator() const                     { return ((m_mute_byOperator & 0x00000001) == 1); }

  void                           SetVideoMuteByOperator(WORD bl);
  WORD                           IsVideoMutedByOperator() const                     { return ((m_mute_byOperator & 0x00000002) == 2); }

  void                           SetAudioMuteByParty(WORD bl);
  WORD                           IsAudioMutedByParty() const                        { return ((m_mute_byParty & 0x00000001) == 1); }

  void                           SetVideoMuteByParty(WORD bl);
  WORD                           IsVideoMutedByParty() const                        { return ((m_mute_byParty & 0x00000002) == 2); }

  void                           SetAudioMuteByMCU(const WORD bl);
  WORD                           IsAudioMutedByMCU() const                          { return ((m_mute_byMCU & 0x00000001) == 1); }

  void                           SetVideoMuteByMCU(const WORD bl);
  WORD                           IsVideoMutedByMCU() const                          { return ((m_mute_byMCU & 0x00000002) == 2); }

  void                           SetAudioBlocked(WORD bl);
  WORD                           IsAudioBlocked() const                             { return ((m_blockMedia & 0x00000001) == 1); }

  void                           SetVideoBlocked(WORD bl);
  WORD                           IsVideoBlocked() const                             { return ((m_blockMedia & 0x00000002) == 2); }

	WORD                           IsH323AliasInclude(CH323Alias* pH323AliasArray, WORD wNumAlias);
  void                           SetCapabilities(const CH221Str& other);
  CH221Str*                      GetCapabilities()                                  { return m_pCapabilities; }

  void                           SetLocalCommMode(const CH221Str& other);
  CH221Str*                      GetLocalCommMode()                                 { return m_pLocalCommMode; }

  void                           SetRemoteCommMode(const CH221Str& other);
  CH221Str*                      GetRemoteCommMode()                                { return m_pRemoteCommMode; }

  void                           SetConnectTime(const CStructTm& other);
  const CStructTm*               GetConnectTime() const                             { return &m_connectTime; }

  void                           SetDisconnectTime(const CStructTm& other);
  const CStructTm*               GetDisconnectTime() const                          { return &m_disconnectTime; }

  void                           SetRetriesNumber(const WORD retNumber);
  WORD                           GetRetriesNumber() const                           { return m_retriesNumber; }

  void                           SetOrdinaryParty(const BYTE ordinary_party);
  BYTE                           GetOrdinaryParty() const                           { return m_pAttendedStruct->GetOrdinaryParty(); }

  void                           SetTransmitBaudRate(const DWORD baudRate);
  DWORD                          GetTransmitBaudRate() const                        { return m_trmBaudRate; }

  void                           SetReceiveBaudRate(const DWORD baudRate);
  DWORD                          GetReceiveBaudRate() const                         { return m_rcvBaudRate; }

  void                           L_syncLostCounterIncrease();
  void                           SetL_syncLostCounter(const WORD num);
  WORD                           GetL_syncLostCounter() const                       { return m_L_syncLostCounter; }

  void                           R_syncLostCounterIncrease();
  void                           SetR_syncLostCounter(const WORD num);
  WORD                           GetR_syncLostCounter() const                       { return m_R_syncLostCounter; }

  void                           L_videoSyncLostCounterIncrease();
  void                           SetL_videoSyncLostCounter(const WORD num);
  WORD                           GetL_videoSyncLostCounter() const                  { return m_L_videoSyncLostCounter; }

  void                           R_videoSyncLostCounterIncrease();
  void                           SetR_videoSyncLostCounter(const WORD num);
  WORD                           GetR_videoSyncLostCounter() const                  { return m_R_videoSyncLostCounter; }

  void                           SetIsNoiseDetected(BYTE wIsNoiseDetected);
  BYTE                           GetIsNoiseDetected()                               { return m_isNoiseDetected; }

  void                           SetOperatorName(const char* name);
  const char*                    GetOperatorName() const                            { return m_operatorName; }

  void                           SetBondingTmpNumber(const char* phone);
  const char*                    GetBondingTmpNumber() const                        { return m_bondingTmpNumber.phone_number; }

  void                           SetIpCapabilities(const CH221Str& other);
  CH221Str*                      GetIpCapabilities()                                { return m_pIpCapabilities; }

  void                           SetIpLocalCommMode(const CH221Str& other);
  CH221Str*                      GetIpLocalCommMode()                               { return m_pIpLocalCommMode; }

  void                           SetIpRemoteCommMode(const CH221Str& other);
  CH221Str*                      GetIpRemoteCommMode()                              { return m_pIpRemoteCommMode; }

  void                           SetIpVideoBch(BYTE videoBch);
  WORD                           IsIpVideoBch() const                               { return (m_ipVideoBch) ? TRUE : FALSE; }

  void                           SetIpProtocolSync(BYTE protocolSync);
  WORD                           IsIpProtocolSync() const                           { return (m_ipProtocolSync) ? TRUE : FALSE; }

  void                           SetIpChannelDetails(EIpChannelType channelType, const CIpChannelDetails& other);
  void                           SetIpChannelDetails(EIpChannelType channelType, BYTE connectionStatus, DWORD actualRate, mcTransportAddress* partyAddr, mcTransportAddress* mcuAddr, BYTE IsIce, mcTransportAddress* IcePartyAddr, mcTransportAddress* IceMcuAddr, EIceConnectionType IceConnectionType, DWORD packetsCounterIn, DWORD packetsCounterUse, WORD frameRate, int videoResolution);

  void                           SetIpChannelDetails(CPrtMontrBaseParams* pPrtMonitrParams, BYTE isUpdateChannelMonitor);
  void                           SetIpChannelDetails(EIpChannelType channelType);

  void                           SetH323GatekeeperStatus(const CH323GatekeeperStatus& other);
  void                           SetH323GatekeeperStatus(BYTE gkState, DWORD reqBandwidth, DWORD allocBandwidth, WORD requestIntoInterval, BYTE gkRouted);

  void                           SetH323GatekeeperStatus();
  CH323GatekeeperStatus*         GetH323GatekeeperStatus()                          { return m_pH323GatekeeperStatus; }

  void                           SetSipBWStatus(const CSIPBandwidthAllocationStatus& other);
  void                           SetSipBWStatus(DWORD reqBandwidth, DWORD allocBandwidth);

  void                           SetSipBWStatus();
  CSIPBandwidthAllocationStatus* GetSipBWStatus()                                   { return m_pSipBWStatus; }

  void                           IpVideoBchCounterIncrease();
  void                           SetIpVideoBchCounter(const WORD num);
  WORD                           GetIpVideoBchCounter() const                       { return m_ipVideoBchCounter; }

  void                           IpProtocolSyncCounterIncrease();
  void                           SetIpProtocolSyncCounter(const WORD num);
  WORD                           GetIpProtocolSyncCounter() const                   { return m_ipProtocolSyncCounter; }

  void                           SetBackupIpAddress(mcTransportAddress ipAddress);
  mcTransportAddress             GetBackupIpAddress()                               { return m_backupIpAddress; }

  void                           SetBackupH323PartyAlias(const char* name);
  const char*                    GetBackupH323PartyAlias() const                    { return m_h323BackupPartyAlias; }

  void                           SetIpVideoIntraSync(BYTE videoIntraSync);
  WORD                           IsIpVideoIntraSync() const                         { return (m_ipVideoIntraSync) ? TRUE : FALSE; }

  void                           SetWaitForOperAssistance(BYTE wait_for_oper_assistance);
  BYTE                           GetWaitForOperAssistance() const                   { return m_wait_for_oper_assistance; }

  void                           SetIsLeader(const WORD leader);
  WORD                           GetIsLeader() const                                { return m_isLeader; }

  void                           SetDefinedPartyAssigned(DWORD newVal)              { m_bDefinedPartyAssigned = newVal; }
  DWORD                          IsDefinedPartyAssigned() const                     { return m_bDefinedPartyAssigned; }

  void                           SetReferredBy(const char* pReferredBy);
  char*                          GetReferredBy()                                    { return m_referredByStr; }

  const char*                    GetName() const                                    { return m_H243_partyName; }

  void                           SetVisualPartyName(const char* visualname);
  char*                          GetVisualPartyName()                               { return m_visual_partyName; }

  void                           SetRemoteName(const char* remoteName);
  char*                          GetRemoteName()                                    { return m_remoteName; }

  void                           SetEPCContentProvider(const BYTE contentProvider);
  BYTE                           GetEPCContentProvider() const                      { return m_EPCContentProvider; }

  void                           SetPartyConfVideoLayout();
  CVideoLayout*                  GetVideoPartyConfLayout()                          { return m_pPartyConfVideoLayout; }

  void                           SetLobbyId(DWORD lobbyId)                          { m_lobbyId = lobbyId; }
  DWORD                          GetLobbyId()                                       { return m_lobbyId; }

  void                           Set_Lpr_Rmt_SyncLoss(WORD bl);
  WORD                           Is_Lpr_Rmt_SyncLoss() const                        { return ((m_lpr_sync_status & 0x00000002) == 2); }

  void                           Set_Lpr_Local_SyncLoss(WORD bl);
  WORD                           Is_Lpr_Local_SyncLoss() const                      { return ((m_lpr_sync_status & 0x00000008) == 8); }

  void                           Set_Is_Lpr_Headers_Activated(BOOL bl);
  BOOL                           Get_Is_Lpr_Headers_Activated()                     { return m_lpr_heders_activated; }

  BYTE                           GetIsTransparentGw()                               { return m_bTransparentGw; }
  void                           SetIsTransparentGw(BYTE bTransparentGw)            { m_bTransparentGw = bTransparentGw; }

  void                           SetGatewayPartyType(eGatewayPartyType type)        { m_GatewayPartyType = type; }
  eGatewayPartyType              GetGatewayPartyType() const                        { return m_GatewayPartyType; }

  void                           SetRequestToSpeak(BOOL isRequestToSpeak);
  BOOL                           IsRequestToSpeak() const                           { return m_bRequestToSpeak; }

  void                           SetExclusiveContentOwner(BOOL i_isExclusiveContent); // Restricted content
  BOOL                           isExclusiveContentOwner()                          { return m_isExclusiveContent; }

  void                           SetIsHighProfile(BYTE isHighProfile);
  BYTE                           isHighProfile()                                    { return m_isHighProfile; }

  void                           SetIsTipCall(BYTE isTipCall)                       { m_bIsTipCall = isTipCall; }
  BOOL                           GetIsTipCall()                                     { return m_bIsTipCall; }
  
  void                           SetAvMcuLinkType(eAvMcuLinkType AvMcuLinkType)     { m_AvMcuLinkType = AvMcuLinkType; }
  eAvMcuLinkType                 GetAvMcuLinkType() const                           { return m_AvMcuLinkType; }

  void                           SetIsTipHeader(BYTE isTipHeader)                   { m_bIsTipHeader = isTipHeader; }
  BOOL                           GetIsTipHeader()                                   { return m_bIsTipHeader; }
  RemoteIdent                    GetRemoteIdent()                                   { return m_remoteIdent; }
  void                           SetRemoteIdent(RemoteIdent rm)                     { m_remoteIdent = rm; }

  APIU16                         GetPlcmRequireMask() const                         { return m_plcmRequireMask; }
  void                           SetPlcmRequireMask(APIU16 plcmRequireMask)         { m_plcmRequireMask = plcmRequireMask; }

  BOOL                           GetIsCiscoTagExist() const                         { return m_bIsCiscoTagExist; }
  void                           SetIsCiscoTagExist(BOOL isCiscoTagExist)           { m_bIsCiscoTagExist = isCiscoTagExist; }

  void                           SetFoundInAddressBook(WORD bFound)                 { m_bFoundInAddrBook = bFound; }
  WORD                           IsFoundInAddressBook()                             { return m_bFoundInAddrBook; }

  void                           SetBfcpTransportType(enTransportType transportType){ m_BfcpTransportType = transportType;}

  void                           SetTask(CTaskApp* pParty)                          { m_pParty = pParty; }
  CTaskApp*                      GetTask()                                          { return m_pParty; }

  void                           SetIsPartyCurrentlyEncrypted(const BYTE isEncrypted);
  BYTE                           GetIsPartyCurrentlyEncrypted() const               { return m_isPartyCurrentlyEncrypted; }

  //VNGR-26449 - unencrypted conference message
  void                           SetIsDelPartyWasEncrypted(BYTE isEncrypted)        { m_isDelPartyWasEncrypted = isEncrypted; }
  BYTE                           GetIsDelPartyWasEncrypted() const                  { return m_isDelPartyWasEncrypted; }
  EIsUnencrypted                 GetIsUnencryptedParty() const                      { return m_isUnencryptedParty; }
  void                           SetIsUnencryptedParty(EIsUnencrypted flag)         { m_isUnencryptedParty = flag; }

  BOOL                           IsAllowReconnect() const;
  BOOL                           IsAllowDisconnect() const;

  void                           SetReceiveDtmfFromChairperson(BOOL bReceiveDtmfFromChairperson) {m_bReceiveDtmfFromChairperson =  bReceiveDtmfFromChairperson;}
  BOOL                           IsReceiveDtmfFromChairperson() { return m_bReceiveDtmfFromChairperson; }

  BOOL                           IsActiveVideoParty() const;

  WORD                           IsH323paramInclude(mcTransportAddress* pPartyIPaddress, CH323Alias* pH323AliasArray, WORD wNumAlias);
  WORD                           IsProblem() const;

  void                           SetActualMCUPhoneNumber(WORD ind, const char* phoneNumber);
  Phone*                         GetActualMCUPhoneNumber(WORD ind);
  int                            DeleteActualMCUPhoneNumber(WORD ind);

  void                           SetActualPartyPhoneNumber(WORD ind, const char* phoneNumber);
  Phone*                         GetActualPartyPhoneNumber(WORD ind);
  int                            DeleteActualPartyPhoneNumber(WORD ind);

  int                            FindActualMCUPhoneNumber(const char* phoneNumber);
  int                            FindActualPartyPhoneNumber(const char* phoneNumber);

  void                           SetActualMCUandPartyPhoneNumbers(BYTE channel, const char* MCUPhoneNumber, const char* partyPhoneNumber);

  void                           SetEventModeIntraSuppressed(BOOL isEventModeIntraSuppresed);
  void                           SetEventModeLevel(WORD eventModeLevel);

  void                           AllocateIpChannels();

  void                           UpdateHotBackupFields();
  void                           RestoreHotBackupFields();
  BOOL                           CanConnectParty();
  BOOL                           IsFirstConnectionAfterHotBackupRestore() { return m_bIsFirstConnectionAfterHotBackupRestore; }
  void                           SetFirstConnectionAfterHobackupRestore(BOOL flag) {m_bIsFirstConnectionAfterHotBackupRestore = flag; }
  virtual DWORD                  AttachObserver(COsQueue* pObserver, WORD event, WORD type = 0, DWORD observerInfo1 = 0);
  virtual int                    DetachObserver(COsQueue* pObserver);
  virtual int                    DetachObserver(COsQueue* pObserver, WORD event, WORD type, DWORD observerInfo1);
  EMoveType                      GetMoveType();
  void                           SetMoveType(EMoveType  enMoveType);

  void SetMediaList(CMediaList& mediaList);
  const CMediaList* GetMediaList() const;

  //do the statistics for SNMP
  void                           SetIsPartyConnected(BOOL bConnected)              { m_bFlagConnectedOnce = bConnected; }
  BOOL                           GetIsPartyConnected() const                       { return m_bFlagConnectedOnce; }

    	void                           SetIsLyncPlugin(BOOL bLyncPlugin)              { m_bIsLyncPlugin = bLyncPlugin; }
  	BOOL                           GetIsLyncPlugin() const                       { return m_bIsLyncPlugin; }

	void RemoveMedia(std::list<unsigned int> listMediaID, bool bUrgent);
	void SetMediaListUpdated();
	bool  IsRelayInfoExists();
	void  SLOW_FAST_CHANGE_Terminal( int slowFastAction );

	//_mccf_
	void        SetToTag(const char* tag);
	const char* GetToTag() const             { return m_strToTag; }
	void        SetFromTag(const char* tag);
	const char* GetFromTag() const           { return m_strFromTag; }
	std::string GetPartyTag() const;

	void 		SetCorrelationId(std::string correlation);
	std::string GetCorrelationId() { return m_correlationId; }

	// Token/Recap Collision Detection (trcd for short)
	//===================================================
	void								SetTokenRecapCollisionDetection(eTokenRecapCollisionDetectionType trCD)	{m_tokenRecapCollisionDetection = trCD;}
	void								PendTokenRecapDueToCollisionDetection() 								{m_trcdInPend = TRUE;}
	void								UnpendTokenRecapDueToCollisionDetection() 								{m_trcdInPend = FALSE;}
	eTokenRecapCollisionDetectionType	GetTokenRecapCollisionDetection()			const						{return m_tokenRecapCollisionDetection;}
	BOOL								IsTokenRecapPendedDueToCollisionDetection()	const						{return m_trcdInPend;}

	
	void						SetSipUsrName(const char* usrName);
	char*					GetSipUsrName()		 { return m_sipUsrName; }

	void                SetTIPPartySubType(ETipPartyTypeAndPosition type)            { m_partyTypeSubTIP = type; }
	//BYTE                IsTIPMasterParty() const                                  { return (eTipMasterCenter == m_partyTypeSubTIP); }
	BYTE                IsTIPAuxParty() const                                     { return (eTipSlaveAux == m_partyTypeSubTIP); }
	void                           SetCountedInAudioIndication(WORD isCountedInAudioIndication)      {  m_IsCountedInAudioIndication = isCountedInAudioIndication;}
	WORD                           IsCountedInAudioIndication() const                             { return m_IsCountedInAudioIndication; }
	/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
	void                SetConfPartyVideoRecoveryStatus()   {m_VideoRecoveryStatus = 1;}
	void                ClearConfPartyVideoRecoveryStatus()   {m_VideoRecoveryStatus = 0;}
	DWORD           GetConfPartyVideoRecoveryStatus()   {return m_VideoRecoveryStatus;}
	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

	//BRIDGE-14279
	  void                           SetSrcTypeIsEQ(BOOL bSrcTypeIsEQ) {m_bSrcTypeIsEQ =  bSrcTypeIsEQ;}
	  BOOL                           IsSrcTypeIsEQ() { return m_bSrcTypeIsEQ; }

protected:
  DWORD                          m_party_state;
  DWORD                          m_bDefinedPartyAssigned;
  DWORD                          m_net_channels;
  DWORD                          m_H221_status;
  DWORD                          m_disconnect_cause;
  DWORD                          m_Q931_disconnect_cause;
  BYTE                           m_secondary_cause;
  DWORD                          m_member_in_media;
  DWORD                          m_mute_byOperator;
  DWORD                          m_mute_byParty;
  DWORD                          m_mute_byMCU;
  DWORD                          m_blockMedia;
  WORD                           m_numVideoSource;
  DWORD                          m_video_source_id[MAX_VIDEO_SOURCE_PARTY];
  CVideoLayout*                  m_pVideoLayout;
  WORD                           m_audioActivatedflag;
  CH221Str*                      m_pCapabilities;
  CH221Str*                      m_pLocalCommMode;
  CH221Str*                      m_pRemoteCommMode;
  CStructTm                      m_connectTime;
  CStructTm                      m_disconnectTime;
  WORD                           m_retriesNumber;
  CAttendedStruct*               m_pAttendedStruct;
  DWORD                          m_internalPartyId;
  DWORD                          m_trmBaudRate;
  DWORD                          m_rcvBaudRate;
  WORD                           m_L_syncLostCounter;
  WORD                           m_R_syncLostCounter;
  WORD                           m_L_videoSyncLostCounter;
  WORD                           m_R_videoSyncLostCounter;
  Phone*                         m_pActualMCUPhoneNumbers[MAX_CHANNEL_NUMBER];
  Phone*                         m_pActualPartyPhoneNumbers[MAX_CHANNEL_NUMBER];
  CSecondaryParams               m_secondary_cause_params;    // API 532
  char                           m_operatorName[OPERATOR_NAME_LEN];
  Phone                          m_bondingTmpNumber;
  CH323StrCap*                   m_pIpCapabilities;
  CH323StrCap*                   m_pIpLocalCommMode;
  CH323StrCap*                   m_pIpRemoteCommMode;
  BYTE                           m_ipVideoBch;                // YES/NO
  BYTE                           m_ipProtocolSync;            // YES/NO
  WORD                           m_ipVideoBchCounter;
  WORD                           m_ipProtocolSyncCounter;
  CIpChannelDetails*             m_pIpChannelDetails[IP_CHANNEL_TYPES_NUMBER];
  CPrtMontrBaseParams*           m_pIpChannelMonitor[IP_CHANNEL_TYPES_NUMBER];
  CH323GatekeeperStatus*         m_pH323GatekeeperStatus;
  mcTransportAddress             m_backupIpAddress;
  char                           m_h323BackupPartyAlias[IP_STRING_LEN];
  BYTE                           m_ipVideoIntraSync;          // YES/NO
  BYTE                           m_wait_for_oper_assistance;  // 0 - None/PARTY_WAITS_FOR_PRIVATE_OPERATOR_ASSISTANCE/ PARTY_WAITS_FOR_PUBLIC_OPERATOR_ASSISTANCE
  WORD                           m_isLeader;
  char                           m_visual_partyName[H243_NAME_LEN];
  char                           m_remoteName[H243_NAME_LEN];
  BYTE                           m_isNoiseDetected;
  BYTE                           m_EPCContentProvider;
  CVideoLayout*                  m_pPartyConfVideoLayout;
  BYTE                           m_isPartyCurrentlyEncrypted; // YES, NO

  char                           m_referredByStr[H243_NAME_LEN];
  DWORD                          m_lobbyId;
  CSubject*                      m_pSubject;
  DWORD                          m_mipErrorNumber;
  DWORD                          m_lpr_sync_status;
  BYTE                           m_bTransparentGw;
  eGatewayPartyType              m_GatewayPartyType;
  BOOL                           m_isExclusiveContent;        // Restricted content
  BYTE                           m_bRequestToSpeak;
  BOOL                           m_isEventModeIntraSuppressed;
  WORD                           m_eventModeLevel;

  // Hot Backup fields:
  BOOL                           m_bIsFirstConnectionAfterHotBackupRestore;
  DWORD                          m_HotBackupPartyStateInMaster;
  BOOL                           m_lpr_heders_activated;

  // Token/Change-Mode collision detection
  eTokenRecapCollisionDetectionType m_tokenRecapCollisionDetection;
  BOOL								m_trcdInPend;

  // High Profile
  BYTE                           m_isHighProfile;
  BYTE                           m_bIsTipCall;
  BYTE                           m_bIsTipHeader;
  eAvMcuLinkType                 m_AvMcuLinkType;

  // SIP CAC monitoring
  CSIPBandwidthAllocationStatus* m_pSipBWStatus;

  RemoteIdent                    m_remoteIdent;
  APIU16                         m_plcmRequireMask;
  BOOL                           m_bIsCiscoTagExist;
  WORD                           m_bFoundInAddrBook;
  enTransportType                m_BfcpTransportType;
  CTaskApp*                      m_pParty;

  //VNGR-26449 - unencrypted conference message
  EIsUnencrypted                 m_isUnencryptedParty;
  BYTE                           m_isDelPartyWasEncrypted;

  EMoveType                      m_MoveType;

  CMediaList                     m_mediaList;

  //statistics for SNMP
  BOOL                           m_bFlagConnectedOnce;

  BOOL                           m_bIsLyncPlugin;
  char                           m_strToTag[H243_NAME_LEN];
  char                           m_strFromTag[H243_NAME_LEN];

  BOOL                           m_bReceiveDtmfFromChairperson;
  
  char                           m_sipUsrName[H243_NAME_LEN];

  BYTE                           m_partyTypeSubTIP;

  std::string                    m_correlationId;

  WORD                           m_IsCountedInAudioIndication;
  
  BOOL							 m_bSrcTypeIsEQ;

  /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
  DWORD                   m_VideoRecoveryStatus; // 1 for video recovery; 0 for none
  /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
};

#endif // !defined(_ConfParty_H__)

