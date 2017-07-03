#include <string>
#include <stdio.h>
#include <netinet/in.h>
#include "NStream.h"
#include "psosxml.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "DefinesIpService.h"
#include "AudHostApiDefinitions.h"
#include "RsrvParty.h"
#include "OperatorConfInfo.h"
#include "MoveInfo.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "TraceStream.h"
#include "PrecedenceSettings.h"

////////////////////////////////////////////////////////////////////////////
//                        CRsrvParty
////////////////////////////////////////////////////////////////////////////
CRsrvParty::CRsrvParty()
  : m_cascadedLinksNumber(0)
  , m_linkType(eRegularParty)
{
  m_phoneNumber[0]            = '\0';
  m_H243_partyName[0]         = '\0';
  m_partyId                   = 0xFFFFFFFF;
  m_connectionType            = DIAL_OUT;
  m_nodeType                  = 1;
  m_bondingMode1              = NO;
  m_netChannelNumber          = 1;
  m_netServiceProviderName[0] = '\0';
  m_restrict                  = AUTO;
  m_voice                     = NO;
  m_AGC                       = YES;
  m_audioThreshold            = 5;
  m_netSubService[0]          = '\0';
  m_backupService[0]          = '\0';
  m_backupSubService[0]       = '\0';
  m_identificationMethod      = CALLED_PHONE_NUMBER_IDENTIFICATION_METHOD;
  m_telePresenceMode          = eTelePresencePartyNone;
  m_isActive                  = FALSE;
  m_mainPartyNumber           = 0;
  m_meet_me_method            = MEET_ME_PER_USER;
  m_numType                   = 0xFF;

  int i;
  for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    m_pCallingPhoneNumberList[i] = NULL;
    m_pCalledPhoneNumberList[i]  = NULL;
  }

  m_numCallingPhones = 0;
  m_numCalledPhones  = 0;
  m_netInterfaceType = ISDN_INTERFACE_TYPE;
  m_multiRateMode    = AUTO;
  // IpV6
  memset(&m_ipAddress, 0, sizeof(mcTransportAddress));

  m_callSignallingPort                 = 1720;
  m_videoProtocol                      = AUTO;
  m_encryption                         = AUTO;
  m_videoRate                          = 0xFFFFFFFF;
  m_bondingPhoneNumber.phone_number[0] = '\0';
  m_h323PartyAliasType                 = PARTY_H323_ALIAS_H323_ID_TYPE;
  m_h323PartyAlias[0]                  = '\0';
  m_sipPartyAddressType                = PARTY_SIP_SIPURI_ID_TYPE;
  m_sipPartyAddress[0]                 = '\0';
  m_audioVolume                        = GetDefaultBroadcastVolume(); // 5;
  m_listening_audioVolume              = GetDefaultListeningVolume(); // 5;
  m_autoDetect                         = NO;
  m_undefinedType                      = NO;

  m_slowInfoMask.SetAllBitsOff();
  m_slow1InfoMask.SetAllBitsOff();
  m_fastInfoMask.SetAllBitsOff();
  m_addPartyMask.SetAllBitsOn();
  m_completeInfoMask.SetAllBitsOn();

  m_slowInfoFlag          = FALSE;
  m_fastInfoFlag          = FALSE;
  m_slow1InfoFlag         = FALSE;
  m_completeInfoFlag      = FALSE;
  m_slowUpdateCounter     = 0;
  m_fastUpdateCounter     = 0;
  m_slow_1_UpdateCounter  = 0;
  m_slowUpdateCounter     = 0;
  m_completeUpdateCounter = 0;
  m_CreationUpdateCounter = 0;
  m_infoOpcode            = PARTY_COMPLETE_INFO;
  m_refferedToUri[0]      = '\0';
  m_refferedByUri[0]      = '\0';
  m_extension[0]          = '\0';
  m_indCalling            = 0;
  m_indCalled             = 0;

  // Private layout
  for (i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
    m_pPrivateVideoLayout[i] = NULL;

  m_pCurRsrvVideoLayout     = new CVideoLayout;
  m_ind_prv_vid_layout      = 0;
  m_numPrivateVideoLayout   = 0;
  m_isPrivate               = NO;
  m_pUserDefinedInfo        = new CUserDefinedInfo;
  m_isVip                   = 0;
  m_pRes                    = NULL;
  m_EnableH323_PSTN         = NO;
  m_isRecordingLinkParty    = 0;
  m_isPlaybackParty 		=0;
  m_lastLayoutForRL		=0;
  m_isVideoMute             = NO;
  m_UserIdentifierString[0] = '\0';
  m_maxResolution           = eAuto_Res;
  m_cascadeMode             = CASCADE_MODE_NONE;
  m_isOperatorParty         = NO;
  m_pOperatorConfInfo       = NULL;
  m_pMoveInfo               = new CMoveInfo();
  m_EnableSipICE            = NO;
  m_partyTypeTIP            = FALSE;
  m_ipSubService            = ePrimaryIpSubService;
  m_RoomId                  = 0xFFFF;
  m_serviceId               = 0;
  m_ePartyMediaType         = eAvcPartyType;
  m_precedenceLevel			= NUM_PRECEDENCE_LEVELS;
  m_precedenceDomain[0]		= '\0';
  m_PreDefinedIvrString[0]  = '\0';
  m_FocusUri[0] = '\0';

  m_eMsftAvmcuState			= eMsftAvmcuNone;
  m_IsDmaAvMcuParty			= 0;

  m_isAvMcuNonCCCPCascade  = 0;

  m_MsConversationId[0]        = '\0';


  m_MsftMediaEscalationStatus  = eMsftEscalationNone;
  m_isCallFromGW			= 0;
   m_bIsRdpGw				= FALSE;
  m_bIsWebRtcCall			= FALSE;
}

//--------------------------------------------------------------------------
CRsrvParty::CRsrvParty(const CRsrvParty& other)
  : CSerializeObject(other)
{
  int i;
  for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    m_pCallingPhoneNumberList[i] = NULL;
    m_pCalledPhoneNumberList[i]  = NULL;
  }

  m_pUserDefinedInfo    = new CUserDefinedInfo;
  m_pCurRsrvVideoLayout = new CVideoLayout;
  // Private layout
  for (i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
  {
    m_pPrivateVideoLayout[i] = NULL;
  }

  m_pRes = NULL;

  m_isOperatorParty = other.m_isOperatorParty;
  if (other.m_pOperatorConfInfo != NULL)
  {
    m_pOperatorConfInfo = new COperatorConfInfo(*other.m_pOperatorConfInfo);
  }
  else
  {
    m_pOperatorConfInfo = NULL;
  }

  m_IsDmaAvMcuParty = other.m_IsDmaAvMcuParty;
  m_isAvMcuNonCCCPCascade = other.m_isAvMcuNonCCCPCascade;

  m_eMsftAvmcuState = other.m_eMsftAvmcuState;
  strcpy_safe(m_FocusUri, other.m_FocusUri);

  strcpy_safe(m_MsConversationId, other.m_MsConversationId);

  m_pMoveInfo = new CMoveInfo(*other.m_pMoveInfo);

  m_MsftMediaEscalationStatus = other.m_MsftMediaEscalationStatus;
  m_isCallFromGW = other.m_isCallFromGW;



  *this = other;
}

//--------------------------------------------------------------------------
CRsrvParty& CRsrvParty::operator =(const CRsrvParty& other)
{
	if(this == &other){
		return *this;
	}

  // old
  strncpy(m_phoneNumber, other.m_phoneNumber, PHONE_NUMBER_DIGITS_LEN);

  // new
  strncpy(m_H243_partyName, other.m_H243_partyName, H243_NAME_LEN);

  m_partyId          = other.m_partyId;
  m_connectionType   = other.m_connectionType;
  m_nodeType         = other.m_nodeType;
  m_bondingMode1     = other.m_bondingMode1;
  m_netChannelNumber = other.m_netChannelNumber;
  strncpy(m_netServiceProviderName,
          other.m_netServiceProviderName, NET_SERVICE_PROVIDER_NAME_LEN);
  m_restrict       = other.m_restrict;
  m_voice          = other.m_voice;
  m_AGC            = other.m_AGC;
  m_audioThreshold = other.m_audioThreshold;

  strncpy(m_netSubService, other.m_netSubService, NET_SERVICE_PROVIDER_NAME_LEN);
  strncpy(m_backupService, other.m_backupService, NET_SERVICE_PROVIDER_NAME_LEN);
  strncpy(m_backupSubService, other.m_backupSubService, NET_SERVICE_PROVIDER_NAME_LEN);

  m_identificationMethod = other.m_identificationMethod;
  m_meet_me_method       = other.m_meet_me_method;
  m_numType              = other.m_numType;
  m_numCallingPhones     = other.m_numCallingPhones;
  m_numCalledPhones      = other.m_numCalledPhones;
  m_netInterfaceType     = other.m_netInterfaceType;
  m_multiRateMode        = other.m_multiRateMode;

  // IpV6
  memset(&m_ipAddress, 0, sizeof(mcTransportAddress));
  memcpy(&m_ipAddress, &other.m_ipAddress, sizeof(mcTransportAddress));

  m_callSignallingPort = other.m_callSignallingPort;
  m_videoProtocol      = other.m_videoProtocol;
  m_encryption         = other.m_encryption;
  m_videoRate          = other.m_videoRate;

  strncpy(m_bondingPhoneNumber.phone_number, other.m_bondingPhoneNumber.phone_number, PHONE_NUMBER_DIGITS_LEN);

  m_h323PartyAliasType = other.m_h323PartyAliasType;
  strncpy(m_h323PartyAlias, other.m_h323PartyAlias, IP_STRING_LEN);

  m_sipPartyAddressType = other.m_sipPartyAddressType;
  strncpy(m_sipPartyAddress, other.m_sipPartyAddress, IP_STRING_LEN);

  m_audioVolume           = other.m_audioVolume;
  m_listening_audioVolume = other.m_listening_audioVolume;
  m_autoDetect            = other.m_autoDetect;
  m_undefinedType         = other.m_undefinedType;

  int i;
  for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    PDELETE(m_pCallingPhoneNumberList[i]);
    PDELETE(m_pCalledPhoneNumberList[i]);
  }

  for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    if (other.m_pCallingPhoneNumberList[i] == NULL)
      m_pCallingPhoneNumberList[i] = NULL;
    else
      m_pCallingPhoneNumberList[i] = new Phone(*other.m_pCallingPhoneNumberList[i]);

    if (other.m_pCalledPhoneNumberList[i] == NULL)
      m_pCalledPhoneNumberList[i] = NULL;
    else
      m_pCalledPhoneNumberList[i] = new Phone(*other.m_pCalledPhoneNumberList[i]);
  }

  m_slowInfoMask          = other.m_slowInfoMask;
  m_slow1InfoMask         = other.m_slow1InfoMask;
  m_fastInfoMask          = other.m_fastInfoMask;
  m_addPartyMask          = other.m_addPartyMask;
  m_completeInfoMask      = other.m_completeInfoMask;
  m_slowInfoFlag          = other.m_slowInfoFlag;
  m_fastInfoFlag          = other.m_fastInfoFlag;
  m_slow1InfoFlag         = other.m_slow1InfoFlag;
  m_completeInfoFlag      = other.m_completeInfoFlag;
  m_slowUpdateCounter     = other.m_slowUpdateCounter;
  m_fastUpdateCounter     = other.m_fastUpdateCounter;
  m_slow_1_UpdateCounter  = other.m_slow_1_UpdateCounter;
  m_slowUpdateCounter     = other.m_slowUpdateCounter;
  m_completeUpdateCounter = other.m_completeUpdateCounter;
  m_CreationUpdateCounter = other.m_CreationUpdateCounter;
  m_infoOpcode            = other.m_infoOpcode;

  strncpy(m_refferedToUri, other.m_refferedToUri, IP_STRING_LEN);
  strncpy(m_refferedByUri, other.m_refferedByUri, IP_STRING_LEN);

  strncpy(m_extension, other.m_extension, PARTY_EXTENSION_LENGTH);
  *m_pUserDefinedInfo =  *other.m_pUserDefinedInfo;
  m_indCalling        = other.m_indCalling;
  m_indCalled         = other.m_indCalled;

  // Private layout
  for (i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
  {
    POBJDELETE(m_pPrivateVideoLayout[i]);

    if (other.m_pPrivateVideoLayout[i] == NULL)
      m_pPrivateVideoLayout[i] = NULL;
    else
    {
      m_pPrivateVideoLayout[i] = new CVideoLayout(*other.m_pPrivateVideoLayout[i]);
    }
  }

  if (other.m_pCurRsrvVideoLayout == NULL)
    m_pCurRsrvVideoLayout = NULL;
  else
  {
    *m_pCurRsrvVideoLayout = *(other.m_pCurRsrvVideoLayout);
  }

  m_ind_prv_vid_layout    = other.m_ind_prv_vid_layout;
  m_numPrivateVideoLayout = other.m_numPrivateVideoLayout;
  m_isPrivate             = other.m_isPrivate;
  m_isVip                 = other.m_isVip;
  m_EnableH323_PSTN       = other.m_EnableH323_PSTN;
  m_EnableSipICE          = other.m_EnableSipICE;
  m_eMsftAvmcuState          = other.m_eMsftAvmcuState;
  m_IsDmaAvMcuParty 	  = other.m_IsDmaAvMcuParty;
  m_isAvMcuNonCCCPCascade  = other.m_isAvMcuNonCCCPCascade;
  m_partyTypeTIP          = other.m_partyTypeTIP;
  m_isRecordingLinkParty  = other.m_isRecordingLinkParty;
  m_isPlaybackParty		=other.m_isPlaybackParty;
  m_lastLayoutForRL		=other.m_lastLayoutForRL;
  m_isVideoMute           = other.m_isVideoMute;
  strncpy(m_UserIdentifierString, other.m_UserIdentifierString, USER_IDENTIFIER_STRING_LEN);
  m_telePresenceMode      = other.m_telePresenceMode;
  m_isActive              = other.m_isActive;
  m_mainPartyNumber       = other.m_mainPartyNumber;
  m_maxResolution         = other.m_maxResolution;

  m_cascadeMode         = other.m_cascadeMode;
  m_cascadedLinksNumber = other.m_cascadedLinksNumber;

  m_linkType            = other.m_linkType;

  m_ipSubService     = other.m_ipSubService;
  m_RoomId           = other.m_RoomId;
  m_serviceId        = other.m_serviceId;
  m_isOperatorParty  = other.m_isOperatorParty;
  m_ePartyMediaType  = other.m_ePartyMediaType;

  m_precedenceLevel	= other.m_precedenceLevel;
  SAFE_COPY(m_precedenceDomain, other.m_precedenceDomain);
  POBJDELETE(m_pOperatorConfInfo);
  if (other.m_pOperatorConfInfo != NULL)
  {
    m_pOperatorConfInfo = new COperatorConfInfo(*other.m_pOperatorConfInfo);
  }

  POBJDELETE(m_pMoveInfo);
  if (other.m_pMoveInfo != NULL)
  {
    m_pMoveInfo = new CMoveInfo(*other.m_pMoveInfo);
  }

  strncpy(m_PreDefinedIvrString, other.m_PreDefinedIvrString, H243_NAME_LEN);
  strncpy(m_FocusUri, other.m_FocusUri, ONE_LINE_BUFFER_LEN);

  strncpy(m_MsConversationId, other.m_MsConversationId, ONE_LINE_BUFFER_LEN);


  m_MsftMediaEscalationStatus = other.m_MsftMediaEscalationStatus;

  m_bIsRdpGw		  = other.m_bIsRdpGw;
  m_bIsWebRtcCall = other.m_bIsWebRtcCall;



  return *this;
}

//--------------------------------------------------------------------------
CRsrvParty::~CRsrvParty()
{
  int i;
  for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    PDELETE(m_pCallingPhoneNumberList[i]);
    PDELETE(m_pCalledPhoneNumberList[i]);
  }

  POBJDELETE(m_pUserDefinedInfo);

  // Private layout
  for (i = 0; i < MAX_VIDEO_LAYOUT_NUMBER; i++)
    POBJDELETE(m_pPrivateVideoLayout[i]);

  POBJDELETE(m_pCurRsrvVideoLayout);
  POBJDELETE(m_pOperatorConfInfo);
  POBJDELETE(m_pMoveInfo);
}

//--------------------------------------------------------------------------
void CRsrvParty::SerializeXml(CXMLDOMElement* pFatherNode, ePartyData party_data_amount /* = FULL_DATA*/)
{
  CXMLDOMElement* pPartyNode, * pTempNode;

  pPartyNode = pFatherNode->AddChildNode("PARTY");
  pPartyNode->AddChildNode("NAME", m_H243_partyName);
  pPartyNode->AddChildNode("ID", m_partyId);

  if (party_data_amount == FULL_DATA)
  {
    pTempNode = pPartyNode->AddChildNode("PHONE_LIST");

    if (m_numCallingPhones >= 1)
      pTempNode->AddChildNode("PHONE1", m_pCallingPhoneNumberList[0]->phone_number);

    if (m_numCallingPhones >= 2)
      pTempNode->AddChildNode("PHONE2", m_pCallingPhoneNumberList[1]->phone_number);

    if (m_numCallingPhones >= 3)
      pTempNode->AddChildNode("PHONE3", m_pCallingPhoneNumberList[2]->phone_number);

    if (m_numCallingPhones >= 4)
      pTempNode->AddChildNode("PHONE4", m_pCallingPhoneNumberList[3]->phone_number);

    if (m_numCallingPhones >= 5)
      pTempNode->AddChildNode("PHONE5", m_pCallingPhoneNumberList[4]->phone_number);

    if (m_numCallingPhones >= 6)
      pTempNode->AddChildNode("PHONE6", m_pCallingPhoneNumberList[5]->phone_number);

    pPartyNode->AddChildNode("INTERFACE", m_netInterfaceType, INTERFACE_ENUM);
    pPartyNode->AddChildNode("CONNECTION", m_connectionType, CONNECTION_ENUM);
    pPartyNode->AddChildNode("MEET_ME_METHOD", m_meet_me_method, MEET_ME_METHOD_ENUM);
    pPartyNode->AddChildNode("NUM_TYPE", m_numType, NUM_TYPE_ENUM);
    pPartyNode->AddChildNode("BONDING", m_bondingMode1, BONDING_ENUM);
    pPartyNode->AddChildNode("MULTI_RATE", m_multiRateMode, MULTI_RATE_ENUM);

    // VNGR-25242, map '0' to 'AUTO'
    BYTE ucNetChannelNumber = AUTO;
    if (0 != m_netChannelNumber)
    {
      ucNetChannelNumber = m_netChannelNumber;
    }

    pPartyNode->AddChildNode("NET_CHANNEL_NUMBER", ucNetChannelNumber, NET_CHANNEL_NUMBER_ENUM);

    pPartyNode->AddChildNode("VIDEO_PROTOCOL", m_videoProtocol, VIDEO_PROTOCOL_ENUM);
    pPartyNode->AddChildNode("CALL_CONTENT", m_voice, CALL_CONTENT_ENUM);

    pTempNode = pPartyNode->AddChildNode("ALIAS");
    pTempNode->AddChildNode("NAME", m_h323PartyAlias);
    pTempNode->AddChildNode("ALIAS_TYPE", m_h323PartyAliasType, ALIAS_TYPE_ENUM);
    pPartyNode->AddIPChildNode("IP", m_ipAddress);
    pPartyNode->AddChildNode("SIGNALING_PORT", m_callSignallingPort);
    pPartyNode->AddChildNode("VOLUME", m_audioVolume);

    pTempNode = pPartyNode->AddChildNode("MCU_PHONE_LIST");

    if (m_numCalledPhones >= 1)
      pTempNode->AddChildNode("PHONE1", m_pCalledPhoneNumberList[0]->phone_number);

    if (m_numCalledPhones >= 2)
      pTempNode->AddChildNode("PHONE2", m_pCalledPhoneNumberList[1]->phone_number);

    if (m_numCalledPhones >= 3)
      pTempNode->AddChildNode("PHONE3", m_pCalledPhoneNumberList[2]->phone_number);

    if (m_numCalledPhones >= 4)
      pTempNode->AddChildNode("PHONE4", m_pCalledPhoneNumberList[3]->phone_number);

    if (m_numCalledPhones >= 5)
      pTempNode->AddChildNode("PHONE5", m_pCalledPhoneNumberList[4]->phone_number);

    if (m_numCalledPhones >= 6)
      pTempNode->AddChildNode("PHONE6", m_pCalledPhoneNumberList[5]->phone_number);

    pPartyNode->AddChildNode("BONDING_PHONE", m_bondingPhoneNumber.phone_number);
    pPartyNode->AddChildNode("SERVICE_NAME", m_netServiceProviderName);
    pPartyNode->AddChildNode("AUTO_DETECT", m_autoDetect, _BOOL);
    pPartyNode->AddChildNode("RESTRICT", IsRestrictOnly(), _BOOL);
    pPartyNode->AddChildNode("VIDEO_BIT_RATE", m_videoRate, VIDEO_BIT_RATE_ENUM);
  }

  pPartyNode->AddChildNode("LAYOUT_TYPE", m_isPrivate, LAYOUT_TYPE_ENUM);

  int           nCurPrivateLayout = ONE_ONE;
  CVideoLayout* pVideoLayout      = GetCurPrivateVideoLayout();

  if (pVideoLayout)
    nCurPrivateLayout = pVideoLayout->GetScreenLayoutITP();

  pPartyNode->AddChildNode("PERSONAL_LAYOUT", nCurPrivateLayout, LAYOUT_ENUM);

  CXMLDOMElement* pForceListNode = pPartyNode->AddChildNode("PERSONAL_FORCE_LIST");	
	
  pVideoLayout = GetFirstPrivateVideoLayout();
  while (pVideoLayout)
  {		
	//printf("pVideoLayout->GetScreenLayout() %d\n",pVideoLayout->GetScreenLayout());
    CXMLDOMElement* pForceNode = pForceListNode->AddChildNode("FORCE");
    pVideoLayout->SerializeXml(pForceNode);
    pVideoLayout = GetNextPrivateVideoLayout();
  }

  // Romem - Addition for forces in party reservation level
  if (m_pCurRsrvVideoLayout)
  {
    pTempNode = pPartyNode->AddChildNode("FORCE");
    m_pCurRsrvVideoLayout->SerializeXml(pTempNode);
  }

  if (party_data_amount == FULL_DATA)
    pPartyNode->AddChildNode("VIP", m_isVip, _BOOL);

  if (m_pUserDefinedInfo)
    m_pUserDefinedInfo->SerializeXml(pPartyNode);

  if (party_data_amount == FULL_DATA)
  {
    pPartyNode->AddChildNode("LISTEN_VOLUME", m_listening_audioVolume);
    pPartyNode->AddChildNode("AGC", m_AGC, _BOOL);
    pPartyNode->AddChildNode("SIP_ADDRESS", m_sipPartyAddress);
    pPartyNode->AddChildNode("SIP_ADDRESS_TYPE", m_sipPartyAddressType, SIP_ADDRESS_TYPE_ENUM);
    pPartyNode->AddChildNode("UNDEFINED", m_undefinedType, _BOOL);
    pPartyNode->AddChildNode("NODE_TYPE", m_nodeType, NODE_TYPE_ENUM);
    pPartyNode->AddChildNode("ENCRYPTION_EX", m_encryption, BOOL_AUTO_ENUM);
    pPartyNode->AddChildNode("IS_RECORDING_LINK_PARTY", m_isRecordingLinkParty, _BOOL);

    // USER_IDENTIFIER_STRING - supported again starting v5
    pPartyNode->AddChildNode("USER_IDENTIFIER_STRING", m_UserIdentifierString);
    pPartyNode->AddChildNode("IDENTIFICATION_METHOD", m_identificationMethod, IDENT_METHOD_ENUM);
    pPartyNode->AddChildNode("MAX_RESOLUTION", m_maxResolution, RESOLUTION_SLIDER_ENUM);

    // CASCADE
    pTempNode = pPartyNode->AddChildNode("CASCADE");
    pTempNode->AddChildNode("CASCADE_ROLE", m_cascadeMode, CASCADE_ROLE_ENUM);
    pTempNode->AddChildNode("CASCADED_LINKS_NUMBER", m_cascadedLinksNumber, _0_TO_4_DECIMAL);
    pTempNode->AddChildNode("LINK_TYPE", m_linkType, LINK_TYPE_ENUM);

    pPartyNode->AddIPChildNode("IP_V6", m_ipAddress, 1);
  }
  // fix for ITP multi cascade - the TP mode is needed even if not in FULL_DATA
  pPartyNode->AddChildNode("TELEPRESENCE_MODE", m_telePresenceMode, TELEPRESENCE_PARTY_TYPE_ENUM);

  pPartyNode->AddChildNode("SUB_IP_SERVICE", m_ipSubService, IP_SUB_SERVICE_ENUM);
	if (party_data_amount==FULL_DATA)
		pPartyNode->AddChildNode("ENDPOINT_MEDIA_TYPE", m_ePartyMediaType, PARTY_MEDIA_TYPE_ENUM);

	if (NUM_PRECEDENCE_LEVELS != m_precedenceLevel)
	{
		pPartyNode->AddChildNode("PRECEDENCE_DOMAIN_NAME", m_precedenceDomain, ONE_LINE_BUFFER_LENGTH);
		pPartyNode->AddChildNode("PRECEDENCE_LEVEL_TYPE", m_precedenceLevel, PRECEDENCE_LEVEL_TYPE_ENUM);
	}



}

//--------------------------------------------------------------------------
void CRsrvParty::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//--------------------------------------------------------------------------
int CRsrvParty::DeSerializeXml(CXMLDOMElement* pResNode, char* pszError, const char* strAction)
{
  int numAction = convertStrActionToNumber(strAction);

  DeSerializeXml(pResNode, pszError, numAction);
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRsrvParty::convertStrActionToNumber(const char* strAction)
{
  int numAction = UNKNOWN_ACTION;
  if (strncmp("ADD_PARTY", strAction, 9))
    numAction = NEW_PARTY;

  return numAction;
}

//--------------------------------------------------------------------------
int CRsrvParty::DeSerializeXml(CXMLDOMElement* pPartyNode, char* pszError, int nAction)
{
  CXMLDOMElement* pChildNode;
  int             i, nStatus = STATUS_OK;
  char*           pszVal = NULL;
  char            szNodeName[20];
  CVideoLayout    VideoLayout;
  CStructTm       StartTime;
  BYTE            byteVal = 0;

  PTRACE(eLevelInfoNormal, "CRsrvParty::DeSerializeXml ");

  GET_VALIDATE_CHILD(pPartyNode, "NAME", m_H243_partyName, _1_TO_H243_NAME_LENGTH);

  if (nAction == ADD_RESERVE && nStatus == STATUS_NODE_MISSING)
    return nStatus;

  GET_VALIDATE_CHILD(pPartyNode, "ID", &m_partyId, _0_TO_DWORD);

  if (nAction != ADD_RESERVE && nStatus == STATUS_NODE_MISSING) // Action update
    return nStatus;

  GET_CHILD_NODE(pPartyNode, "PHONE_LIST", pChildNode);

  if (pChildNode)
  {
    // if a phone_list element is located ,then we delete all the phone numbers
    // and we are adding them according the the phone element the user sent
    // in case of an update the phone list will be loaded from the DB
    // so the user does not ha to send them unless he wants to change their content
    CleanCallingPhoneNumbers();

    for (i = 1; i < 7; i++)
    {
      sprintf(szNodeName, "PHONE%d", i);
      GET_VALIDATE_CHILD(pChildNode, szNodeName, &pszVal, PHONE_NUMBER_DIGITS_LENGTH);
      if (nStatus == STATUS_OK && pszVal != NULL)
      {
        if (strcmp(pszVal, "") != 0)
          AddCallingPhoneNumber(pszVal);
      }
    }
  }

  GET_VALIDATE_CHILD(pPartyNode, "INTERFACE", &m_netInterfaceType, INTERFACE_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "CONNECTION", &m_connectionType, CONNECTION_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "MEET_ME_METHOD", &m_meet_me_method, MEET_ME_METHOD_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "NUM_TYPE", &m_numType, NUM_TYPE_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "BONDING", &m_bondingMode1, BONDING_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "MULTI_RATE", &m_multiRateMode, MULTI_RATE_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "NET_CHANNEL_NUMBER", &m_netChannelNumber, NET_CHANNEL_NUMBER_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "VIDEO_PROTOCOL", &m_videoProtocol, VIDEO_PROTOCOL_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "CALL_CONTENT", &m_voice, CALL_CONTENT_ENUM);

  GET_CHILD_NODE(pPartyNode, "ALIAS", pChildNode);

  if (pChildNode)
  {
    GET_VALIDATE_MANDATORY_ASCII_CHILD(pChildNode, "NAME", m_h323PartyAlias, IP_STRING_LENGTH);
    if (nStatus == STATUS_NODE_VALUE_NOT_ASCII)
    {
      return STATUS_INVALID_ALIAS;
    }

    GET_VALIDATE_CHILD(pChildNode, "ALIAS_TYPE", &m_h323PartyAliasType, ALIAS_TYPE_ENUM);
  }
  

  GET_VALIDATE_CHILD(pPartyNode, "IP", &m_ipAddress, IP_ADDRESS);


  GET_VALIDATE_CHILD(pPartyNode, "SIGNALING_PORT", &m_callSignallingPort, _0_TO_WORD);
  GET_VALIDATE_CHILD(pPartyNode, "VOLUME", &m_audioVolume, _1_TO_10_DECIMAL);
  ForceSystemBroadcastVolumeIfNeeded();

  GET_CHILD_NODE(pPartyNode, "MCU_PHONE_LIST", pChildNode);

  if (pChildNode)
  {
    CleanCalledPhoneNumbers();

    for (i = 1; i < 7; i++)
    {
      sprintf(szNodeName, "PHONE%d", i);
      GET_VALIDATE_CHILD(pChildNode, szNodeName, &pszVal, PHONE_NUMBER_DIGITS_LENGTH);
      if (nStatus == STATUS_OK && pszVal != NULL)
      {
        if (strcmp(pszVal, "") != 0)
          AddCalledPhoneNumber(pszVal);
      }
    }
  }

  GET_VALIDATE_CHILD(pPartyNode, "BONDING_PHONE", m_bondingPhoneNumber.phone_number, PHONE_NUMBER_DIGITS_LENGTH);

  GET_VALIDATE_CHILD(pPartyNode, "SERVICE_NAME", m_netServiceProviderName, NET_SERVICE_PROVIDER_NAME_LENGTH);
  GET_VALIDATE_CHILD(pPartyNode, "AUTO_DETECT", &m_autoDetect, _BOOL);
  GET_VALIDATE_CHILD(pPartyNode, "RESTRICT", &byteVal, _BOOL);
  if (byteVal == 1)
    m_restrict = 27;
  else
    m_restrict = 0xff;

  GET_VALIDATE_CHILD(pPartyNode, "VIDEO_BIT_RATE", &m_videoRate, VIDEO_BIT_RATE_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "LAYOUT_TYPE", &m_isPrivate, LAYOUT_TYPE_ENUM);

  int nCurPrivateLayout = -1;

  GET_VALIDATE_CHILD(pPartyNode, "PERSONAL_LAYOUT", &nCurPrivateLayout, LAYOUT_ENUM);

  CXMLDOMElement* pForceListNode;

  GET_CHILD_NODE(pPartyNode, "PERSONAL_FORCE_LIST", pForceListNode);

  if (pForceListNode)
  {
    CXMLDOMElement* pForceNode;
    GET_FIRST_CHILD_NODE(pForceListNode, "FORCE", pForceNode);

    while (pForceNode)
    {
      CVideoLayout VideoLayout;

      nStatus = VideoLayout.DeSerializeXml(pForceNode, pszError, m_isPrivate);

      if (nStatus != STATUS_OK)
        return nStatus;

      if (VideoLayout.GetScreenLayout() == nCurPrivateLayout)
        VideoLayout.SetActive(YES);
      else
        VideoLayout.SetActive(NO);

	  //PASSERT(1);
      AddPrivateVideoLayout(VideoLayout, TRUE);
      GET_NEXT_CHILD_NODE(pForceListNode, "FORCE", pForceNode);
    }
  }

  GET_CHILD_NODE(pPartyNode, "FORCE", pChildNode);

  if (!m_pCurRsrvVideoLayout)
    m_pCurRsrvVideoLayout = new CVideoLayout;

  if (pChildNode)
  {
    if (m_isPrivate)
    {
      nStatus = m_pCurRsrvVideoLayout->DeSerializeXml(pChildNode, pszError, m_isPrivate);
      m_pCurRsrvVideoLayout->SetActive(YES);
      if (nStatus != STATUS_OK)
      {
        POBJDELETE(m_pCurRsrvVideoLayout);
        return nStatus;
      }

      AddPrivateVideoLayout(*m_pCurRsrvVideoLayout, TRUE);
    }
  }

  GET_VALIDATE_CHILD(pPartyNode, "VIP", &m_isVip, _BOOL);

  GET_CHILD_NODE(pPartyNode, "CONTACT_INFO_LIST", pChildNode);
  if (pChildNode)
  {
    nStatus = m_pUserDefinedInfo->DeSerializeXml(pChildNode, pszError);
    if (nStatus != STATUS_OK)
      return nStatus;
  }

  GET_VALIDATE_CHILD(pPartyNode, "LISTEN_VOLUME", &m_listening_audioVolume, _1_TO_10_DECIMAL);
  ForceSystemListeningVolumeIfNeeded();
  GET_VALIDATE_CHILD(pPartyNode, "AGC", &m_AGC, _BOOL);
  GET_VALIDATE_CHILD(pPartyNode, "SIP_ADDRESS", m_sipPartyAddress, IP_STRING_LENGTH);
  GET_VALIDATE_CHILD(pPartyNode, "SIP_ADDRESS_TYPE", &m_sipPartyAddressType, SIP_ADDRESS_TYPE_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "UNDEFINED", &m_undefinedType, _BOOL);
  GET_VALIDATE_CHILD(pPartyNode, "NODE_TYPE", &m_nodeType, NODE_TYPE_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "ENCRYPTION_EX", &m_encryption, BOOL_AUTO_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "IS_RECORDING_LINK_PARTY", &m_isRecordingLinkParty, _BOOL);

  // USER_IDENTIFIER_STRING - supported again starting v5
  GET_VALIDATE_CHILD(pPartyNode, "USER_IDENTIFIER_STRING", m_UserIdentifierString, _0_TO_USER_IDENTIFIER_STRING_LENGTH);
  GET_VALIDATE_CHILD(pPartyNode, "IDENTIFICATION_METHOD", &m_identificationMethod, IDENT_METHOD_ENUM);
  GET_VALIDATE_CHILD(pPartyNode, "MAX_RESOLUTION", &m_maxResolution, RESOLUTION_SLIDER_ENUM);
  GET_CHILD_NODE(pPartyNode, "CASCADE", pChildNode);

  if (pChildNode)
  {
    pChildNode->ResetChildList();
    GET_VALIDATE_CHILD(pChildNode, "CASCADE_ROLE", &m_cascadeMode, CASCADE_ROLE_ENUM);
    GET_VALIDATE_CHILD(pChildNode, "CASCADED_LINKS_NUMBER", &m_cascadedLinksNumber, _0_TO_4_DECIMAL);

    BYTE linkType = 0;
    GET_VALIDATE_CHILD(pChildNode, "LINK_TYPE", &linkType, LINK_TYPE_ENUM);
    m_linkType = static_cast<eTypeOfLinkParty>(linkType);
  }

  GET_VALIDATE_CHILD(pPartyNode, "TELEPRESENCE_MODE", &m_telePresenceMode, TELEPRESENCE_PARTY_TYPE_ENUM);
  mcTransportAddress tempIpV6Addr;
  memset(&tempIpV6Addr, 0, sizeof(mcTransportAddress));

  GET_VALIDATE_CHILD(pPartyNode, "IP_V6", &tempIpV6Addr, IP_ADDRESS);
  if (tempIpV6Addr.ipVersion == (DWORD)eIpVersion6)
    memcpy(&m_ipAddress, &tempIpV6Addr, sizeof(mcTransportAddress));

  GET_VALIDATE_CHILD(pPartyNode, "SUB_IP_SERVICE", &m_ipSubService, IP_SUB_SERVICE_ENUM);

  SetSubServiceParams();

	GET_VALIDATE_CHILD(pPartyNode,"ENDPOINT_MEDIA_TYPE", &m_ePartyMediaType, PARTY_MEDIA_TYPE_ENUM);	// avc/media_relay[/auto(future)]
	GET_VALIDATE_CHILD(pPartyNode,"PRECEDENCE_DOMAIN_NAME", m_precedenceDomain, ONE_LINE_BUFFER_LENGTH);
	GET_VALIDATE_CHILD(pPartyNode,"PRECEDENCE_LEVEL_TYPE", &m_precedenceLevel, PRECEDENCE_LEVEL_TYPE_ENUM);

  return STATUS_OK;
}

//--------------------------------------------------------------------------
void CRsrvParty::CleanPhoneNumbers()
{
  CleanCallingPhoneNumbers();
  CleanCalledPhoneNumbers();
}

//--------------------------------------------------------------------------
void CRsrvParty::CleanCallingPhoneNumbers()
{
  int i;
  for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
    PDELETE(m_pCallingPhoneNumberList[i]);

  m_numCallingPhones = 0;
}

//--------------------------------------------------------------------------
void CRsrvParty::CleanCalledPhoneNumbers()
{
  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
    PDELETE(m_pCalledPhoneNumberList[i]);

  m_numCalledPhones = 0;
}

//--------------------------------------------------------------------------
void CRsrvParty::Serialize(WORD format, CSegment& seg)
{
  if (format != NATIVE) PASSERT(1);

  COstrStream pOstr;
  Serialize(format, pOstr);
  pOstr.Serialize(seg);
}

//--------------------------------------------------------------------------
void CRsrvParty::Serialize(WORD format, std::ostream& m_ostr)
{
  // assuming format = OPERATOR_MCMS

  // old
  m_ostr <<  m_phoneNumber << "\n";   // PARTY_PHONE_NUMBER

  // new
  m_ostr << m_H243_partyName << "\n";

  m_ostr << m_partyId   << "\n";
  m_ostr << (WORD)m_connectionType   << "\n";
  m_ostr <<  m_PreDefinedIvrString << "\n";
  m_ostr <<  m_FocusUri << "\n";
  m_ostr <<  m_MsConversationId << "\n";
  m_ostr << (WORD)m_nodeType  << "\n";
  m_ostr << (WORD)m_bondingMode1  << "\n";
  m_ostr << (WORD)m_netChannelNumber  << "\n";
  m_ostr << m_netServiceProviderName  << "\n";
  m_ostr << (WORD)m_restrict  << "\n";
  m_ostr << (WORD)m_voice  << "\n";
  m_ostr << (WORD)m_AGC  << "\n";
  m_ostr << (WORD)m_audioThreshold  << "\n";
  m_ostr << m_numCallingPhones  << "\n";
  m_ostr << m_numCalledPhones  << "\n";
  m_ostr << (WORD)m_netInterfaceType  << "\n";
  m_ostr << (WORD)m_multiRateMode  << "\n";
  // IpV6
  m_ostr << (DWORD)m_ipAddress.ipVersion << "\n";
  m_ostr << (DWORD)m_ipAddress.port << "\n";
  m_ostr << (DWORD)m_ipAddress.distribution << "\n";
  m_ostr << (DWORD)m_ipAddress.transportType << "\n";
  if ((enIpVersion)m_ipAddress.ipVersion == eIpVersion4)
    m_ostr << (DWORD)m_ipAddress.addr.v4.ip << "\n";
  else
  {
    m_ostr << (DWORD)m_ipAddress.addr.v6.scopeId << "\n";
    char szIP[64];
    ::ipToString(m_ipAddress, szIP, 1); // With Brackets
    m_ostr << szIP << "\n";
  }

  m_ostr << m_callSignallingPort  << "\n";
  m_ostr << (WORD)m_videoProtocol  << "\n";
  m_ostr << m_videoRate  << "\n";

  if (m_connectionType != DIRECT)
  {
    int i = 0;
    for (i = 0; i < (int)m_numCallingPhones; i++)
      m_ostr << m_pCallingPhoneNumberList[i]->phone_number << "\n";

    for (i = 0; i < (int)m_numCalledPhones; i++)
      m_ostr << m_pCalledPhoneNumberList[i]->phone_number << "\n";

    m_ostr << (WORD)m_identificationMethod  << "\n";
    m_ostr << (WORD)m_meet_me_method  << "\n";
    m_ostr << m_netSubService  << "\n";
    m_ostr << m_backupService  << "\n";
    m_ostr << m_backupSubService  << "\n";
    m_ostr << (WORD)m_numType  << "\n";
  }

  m_ostr << m_bondingPhoneNumber.phone_number << "\n";
  m_ostr << m_h323PartyAliasType << "\n";
  m_ostr << m_h323PartyAlias << "\n";
  m_ostr << (WORD)m_audioVolume << "\n";
  m_ostr << (WORD)m_autoDetect << "\n";
  m_ostr << (WORD)m_undefinedType << "\n";
  m_ostr <<  m_extension << "\n";   // party extension

  // Private layout
  m_ostr << (WORD)m_isPrivate << "\n";
  m_ostr << m_numPrivateVideoLayout << "\n";
  for (WORD i = 0; i < m_numPrivateVideoLayout; i++)
  {
    m_pPrivateVideoLayout[i]->Serialize(format, m_ostr);
  }

  if (!m_pCurRsrvVideoLayout)
    m_pCurRsrvVideoLayout = new CVideoLayout;

  m_pCurRsrvVideoLayout->Serialize(format, m_ostr);

  m_ostr << (WORD)m_listening_audioVolume << "\n";
  m_ostr << m_sipPartyAddressType << "\n";
  m_ostr << m_sipPartyAddress << "\n";
  m_ostr << (WORD)m_EnableH323_PSTN << "\n";
  m_ostr << (WORD)m_EnableSipICE <<"\n";
  m_ostr << (WORD)m_MsftMediaEscalationStatus <<"\n";
  m_ostr << (WORD)m_eMsftAvmcuState << "\n";
  m_ostr << (WORD)m_IsDmaAvMcuParty << "\n";
  m_ostr << (WORD)m_isAvMcuNonCCCPCascade << "\n";
  m_ostr << (WORD)m_partyTypeTIP <<"\n";
  m_ostr << (WORD)m_encryption << "\n";
  m_ostr << (WORD)m_isRecordingLinkParty << "\n";
  m_ostr << (WORD)m_isVideoMute << "\n";
  m_ostr << (WORD)m_maxResolution << "\n";
  m_ostr << (WORD)m_cascadeMode<< "\n";
  m_ostr << (WORD)m_cascadedLinksNumber << "\n";
  m_ostr <<  m_refferedToUri << "\n";
  m_ostr <<  m_refferedByUri << "\n";
  m_ostr << (WORD)m_telePresenceMode << "\n";
  m_ostr << (WORD)m_isActive << "\n";
  m_ostr << (WORD)m_isOperatorParty<< "\n";

  WORD add_operator_conf_info = 0;
  if (m_pOperatorConfInfo != NULL)
  {
    add_operator_conf_info = 1;
    m_ostr << add_operator_conf_info << "\n";
    m_pOperatorConfInfo->Serialize(format, m_ostr);
  }
  else
  {
    m_ostr << add_operator_conf_info << "\n";
  }

  WORD add_move_info = 0;
  if (m_pMoveInfo != NULL)
  {
    add_move_info = 1;
    m_ostr << add_move_info << "\n";
    m_pMoveInfo->Serialize(format, m_ostr);
  }
  else
  {
    m_ostr << add_move_info << "\n";
  }

  m_ostr <<  m_UserIdentifierString << "\n";      // PARTY_PHONE_NUMBER
  m_ostr << (DWORD)m_ipSubService << "\n";
  m_ostr << (DWORD)m_RoomId << "\n";
  m_ostr << (DWORD)m_mainPartyNumber << "\n";

  m_pUserDefinedInfo->Serialize(format, m_ostr);
	m_ostr << (WORD)m_ePartyMediaType << "\n";

  m_ostr << (WORD) m_precedenceLevel << "\n";
  m_ostr << m_precedenceDomain << "\n";
  m_ostr << (WORD)m_isPlaybackParty << "\n";
  m_ostr << (WORD)m_lastLayoutForRL << "\n";

}

//--------------------------------------------------------------------------
void CRsrvParty::DeSerialize(WORD format, CSegment& seg)
{
  if (format != NATIVE) PASSERT(1);

  CIstrStream istr(seg);
  DeSerialize(format, istr);
}

//--------------------------------------------------------------------------
void CRsrvParty::DeSerialize(WORD format, std::istream& m_istr)
{
  // assuming format = OPERATOR_MCMS

  // old
  m_istr.getline(m_phoneNumber, PHONE_NUMBER_DIGITS_LEN+1, '\n'); // LEN+1 IN ORDER TO RETRIEVE '\N'

  // new
  m_istr.getline(m_H243_partyName, H243_NAME_LEN+1, '\n');
  m_istr >> m_partyId;
  WORD tmp;
  m_istr >> tmp;
  m_connectionType = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr.getline(m_PreDefinedIvrString, H243_NAME_LEN+1, '\n');
  m_istr.getline(m_FocusUri, ONE_LINE_BUFFER_LEN+1, '\n');
  m_istr.getline(m_MsConversationId, ONE_LINE_BUFFER_LEN+1, '\n');
  m_istr >> tmp;
  m_nodeType = (BYTE)tmp;
  m_istr >> tmp;
  m_bondingMode1 = (BYTE)tmp;
  m_istr >> tmp;
  m_netChannelNumber = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr.getline(m_netServiceProviderName, NET_SERVICE_PROVIDER_NAME_LEN+1, '\n');
  m_istr >> tmp;
  m_restrict = (BYTE)tmp;
  m_istr >> tmp;
  m_voice = (BYTE)tmp;
  m_istr >> tmp;
  m_AGC = (BYTE)tmp;
  m_istr >> tmp;
  m_audioThreshold = (BYTE)tmp;
  m_istr >> m_numCallingPhones;
  m_istr >> m_numCalledPhones;
  m_istr >> tmp;
  m_netInterfaceType = (BYTE)tmp;
  m_istr >> tmp;
  m_multiRateMode = (BYTE)tmp;
  // IpV6
  m_istr >> m_ipAddress.ipVersion;
  m_istr >> m_ipAddress.port;
  m_istr >> m_ipAddress.distribution;
  m_istr >> m_ipAddress.transportType;
  if ((enIpVersion)m_ipAddress.ipVersion == eIpVersion4)
    m_istr >> m_ipAddress.addr.v4.ip;
  else
  {
    m_istr >> m_ipAddress.addr.v6.scopeId;
    char szIP[64];
    memset(szIP, '\0', 64);
    m_istr >> szIP;
    ::stringToIp(&m_ipAddress, szIP); // With Brackets
  }

  m_istr >> m_callSignallingPort;
  m_istr >> tmp;
  m_videoProtocol = (BYTE)tmp;
  m_istr >> m_videoRate;

  m_istr.ignore(1);

  if (m_connectionType != DIRECT)
  {
    int i = 0;
    for (i = 0; i < (int)m_numCallingPhones; i++)
    {
      m_pCallingPhoneNumberList[i] = new Phone;
      m_istr.getline(m_pCallingPhoneNumberList[i]->phone_number, PHONE_NUMBER_DIGITS_LEN+1, '\n');
    }

    for (i = 0; i < (int)m_numCalledPhones; i++)
    {
      m_pCalledPhoneNumberList[i] = new Phone;
      m_istr.getline(m_pCalledPhoneNumberList[i]->phone_number, PHONE_NUMBER_DIGITS_LEN+1, '\n');
    }

    m_istr >> tmp;
    m_identificationMethod = (BYTE)tmp;
    m_istr >> tmp;
    m_meet_me_method = (BYTE)tmp;
    m_istr.ignore(1);
    m_istr.getline(m_netSubService, NET_SERVICE_PROVIDER_NAME_LEN+1, '\n');
    m_istr.getline(m_backupService, NET_SERVICE_PROVIDER_NAME_LEN+1, '\n');
    m_istr.getline(m_backupSubService, NET_SERVICE_PROVIDER_NAME_LEN+1, '\n');
    m_istr >> tmp;
    m_numType = (BYTE)tmp;
    m_istr.ignore(1);
  }

  m_istr.getline(m_bondingPhoneNumber.phone_number, PHONE_NUMBER_DIGITS_LEN+1, '\n');
  m_istr >> m_h323PartyAliasType;
  m_istr.ignore(1);

  m_istr.getline(m_h323PartyAlias, IP_STRING_LEN+1, '\n');

  m_istr >> tmp;
  m_audioVolume = (BYTE)tmp;
  m_istr.ignore(1);

  m_istr >> tmp;
  m_autoDetect = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_undefinedType = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr.getline(m_extension, PARTY_EXTENSION_LENGTH+1, '\n'); // LEN+1 IN ORDER TO RETRIEVE '\N'

  // Private layout
  m_istr >> tmp;
  m_isPrivate = (BYTE)tmp;
  m_istr >> m_numPrivateVideoLayout;

  for (WORD i = 0; i < m_numPrivateVideoLayout; i++)
  {
    m_pPrivateVideoLayout[i] = new CVideoLayout;
    m_pPrivateVideoLayout[i]->DeSerialize(format, m_istr);
  }

  if (!m_pCurRsrvVideoLayout)
    m_pCurRsrvVideoLayout = new CVideoLayout;

  m_pCurRsrvVideoLayout->DeSerialize(format, m_istr);

  m_istr.ignore(1);

  m_istr >> tmp;
  m_listening_audioVolume = (BYTE)tmp;
  m_istr.ignore(1);

  m_istr >> m_sipPartyAddressType;
  m_istr.ignore(1);
  m_istr.getline(m_sipPartyAddress, IP_STRING_LEN+1, '\n');

  m_istr >> tmp;
  m_EnableH323_PSTN = (BYTE)tmp;

  m_istr >> tmp;
  m_EnableSipICE = (BYTE)tmp;

  m_istr >> tmp;
  m_MsftMediaEscalationStatus = (EMsftMediaEscalationStatus)tmp;

  m_istr >> tmp;
  m_eMsftAvmcuState = (enMsftAvmcuState)tmp;

  m_istr >> tmp;
  m_IsDmaAvMcuParty = (BYTE)tmp;

  m_istr >> tmp;
  m_isAvMcuNonCCCPCascade  = (BYTE)tmp;

  m_istr >> tmp;
  m_partyTypeTIP = (BYTE)tmp;

  m_istr >> tmp;
  m_encryption = (BYTE)tmp;

  m_istr >> tmp;
  m_isRecordingLinkParty = (BYTE)tmp;

  m_istr >> tmp;
  m_isVideoMute = (BYTE)tmp;

  m_istr >> tmp;
  m_maxResolution = (BYTE)tmp;

  m_istr >> tmp;
  m_cascadeMode = (BYTE)tmp;

  m_istr >> tmp;
  m_cascadedLinksNumber = (BYTE)tmp;

  m_istr.ignore(1);

  m_istr.getline(m_refferedToUri, IP_STRING_LEN+1, '\n');   // LEN+1 IN ORDER TO RETRIEVE '\N'
  m_istr.getline(m_refferedByUri, IP_STRING_LEN+1, '\n');

  m_istr >> tmp;
  m_telePresenceMode = (BYTE)tmp;
  m_istr.ignore(1);

  m_istr >> tmp;
  m_isActive = (BYTE)tmp;
  m_istr.ignore(1);

  m_istr >> tmp;
  m_isOperatorParty = (BYTE)tmp;
  m_istr.ignore(1);

  // indicate if m_pOperatorConfInfo serialized
  WORD add_operator_conf_info = 0;
  m_istr >> add_operator_conf_info;
  m_istr.ignore(1);

  if (add_operator_conf_info)
  {
    if (m_pOperatorConfInfo == NULL)
    {
      m_pOperatorConfInfo = new COperatorConfInfo();
    }

    m_pOperatorConfInfo->DeSerialize(format, m_istr);
  }

  WORD add_move_info = 0;
  m_istr >> add_move_info;
  m_istr.ignore(1);

  PTRACE2INT(eLevelInfoNormal, "CRsrvParty::DeSerialize add_move_info = ", add_move_info);

  if (add_move_info)
  {
    if (m_pMoveInfo == NULL)
    {
      m_pMoveInfo = new CMoveInfo();
    }

    m_pMoveInfo->DeSerialize(format, m_istr);
  }

  m_istr.getline(m_UserIdentifierString, USER_IDENTIFIER_STRING_LEN+1, '\n');
  m_istr >> tmp;
  m_ipSubService = (DWORD)tmp;
  m_istr.ignore(1);

  m_istr >> tmp;
  m_RoomId = (DWORD)tmp;
  m_istr.ignore(1);

  m_istr >> tmp;
  m_mainPartyNumber = (DWORD)tmp;
  m_istr.ignore(1);

  m_pUserDefinedInfo->DeSerialize(format, m_istr);
  m_istr >> tmp;
  m_ePartyMediaType = (BYTE)tmp;
  m_istr.ignore(1);
  m_istr >> tmp;
  m_precedenceLevel = (int)tmp;
  m_istr.ignore(1);
  m_istr.getline(m_precedenceDomain, ONE_LINE_BUFFER_LEN+1, '\n');

  m_istr >> tmp;
  m_isPlaybackParty = (BYTE)tmp;  
  
  m_istr.ignore(1);
  m_istr >> tmp;
  m_lastLayoutForRL = (BYTE)tmp;
  m_istr.ignore(1);
}

//--------------------------------------------------------------------------
const char* CRsrvParty::GetPhoneNumber() const
{
  if (m_connectionType == DIAL_OUT) // Romem klocwork
  {
    if (m_numCallingPhones && m_numCallingPhones < MAX_CHANNEL_NUMBER)
      return m_pCallingPhoneNumberList[m_numCallingPhones-1]->phone_number;
    else
      return "";
  }
  else
  {
    if (m_numCalledPhones && m_numCalledPhones < MAX_CHANNEL_NUMBER)
      return m_pCalledPhoneNumberList[m_numCalledPhones-1]->phone_number;
    else
      return "";
  }

  return m_phoneNumber;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetPhoneNumber(const char* phoneNumber)
{
  strncpy(m_phoneNumber, phoneNumber, PHONE_NUMBER_DIGITS_LEN - 1);

  // Cheaper to just assign the null than to check:  int len=strlen(phoneNumber); if (len>PHONE_NUMBER_DIGITS_LEN)
  m_phoneNumber[PHONE_NUMBER_DIGITS_LEN-1] = '\0';

  if (m_connectionType == DIAL_OUT)
    AddCallingPhoneNumber(phoneNumber);
  else
    AddCalledPhoneNumber(phoneNumber);
}

//--------------------------------------------------------------------------
int CRsrvParty::AddCallingPhoneNumber(const char* phoneNumber)
{
  if (m_numCallingPhones >= MAX_CHANNEL_NUMBER)
    return STATUS_MAX_PHONE_NUMBER_EXCEEDED;

  m_pCallingPhoneNumberList[m_numCallingPhones] = new Phone;

  strncpy(m_pCallingPhoneNumberList[m_numCallingPhones]->phone_number,
          phoneNumber,
          PHONE_NUMBER_DIGITS_LEN - 1);

  m_pCallingPhoneNumberList[m_numCallingPhones]->phone_number[PHONE_NUMBER_DIGITS_LEN-1] = '\0';

  m_numCallingPhones++;

  strncpy(m_phoneNumber, phoneNumber, PHONE_NUMBER_DIGITS_LEN - 1);
  m_phoneNumber[PHONE_NUMBER_DIGITS_LEN-1] = '\0';

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRsrvParty::AddCalledPhoneNumber(const char* phoneNumber)
{
  if (m_numCalledPhones >= MAX_CHANNEL_NUMBER)
    return STATUS_MAX_PHONE_NUMBER_EXCEEDED;

  m_pCalledPhoneNumberList[m_numCalledPhones] = new Phone;

  strncpy(m_pCalledPhoneNumberList[m_numCalledPhones]->phone_number, phoneNumber, PHONE_NUMBER_DIGITS_LEN - 1);
  m_pCalledPhoneNumberList[m_numCalledPhones]->phone_number[PHONE_NUMBER_DIGITS_LEN-1] = '\0';

  m_numCalledPhones++;

  strncpy(m_phoneNumber, phoneNumber, PHONE_NUMBER_DIGITS_LEN - 1);
  m_phoneNumber[PHONE_NUMBER_DIGITS_LEN-1] = '\0';

  return STATUS_OK;
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetFirstCallingPhoneNumber()
{
  m_indCalling = 1;
  return m_pCallingPhoneNumberList[0];
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetFirstCallingPhoneNumber(int& nPos)
{
  Phone* pPhone = CRsrvParty::GetFirstCallingPhoneNumber();
  nPos = m_indCalling;

  return pPhone;
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetNextCallingPhoneNumber()
{
  if (m_indCalling >= m_numCallingPhones) return m_pCallingPhoneNumberList[0];

  return m_pCallingPhoneNumberList[m_indCalling++];
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetNextCallingPhoneNumber(int& nPos)
{
  m_indCalling = nPos;
  Phone* pPhone = CRsrvParty::GetNextCallingPhoneNumber();
  nPos = m_indCalling;

  return pPhone;
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetNextCallingPhoneNumberOper()
{
  if ((m_indCalling >= m_numCallingPhones) || (m_indCalling >= MAX_CHANNEL_NUMBER)) return NULL;

  return m_pCallingPhoneNumberList[m_indCalling++];
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetNextCallingPhoneNumberOper(int& nPos)
{
  m_indCalling = nPos;
  Phone* pPhone = CRsrvParty::GetNextCallingPhoneNumberOper();
  nPos = m_indCalling;

  return pPhone;
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetFirstCalledPhoneNumber()
{
  m_indCalled = 1;
  return m_pCalledPhoneNumberList[0];
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetFirstCalledPhoneNumber(int& nPos)
{
  Phone* pPhone = CRsrvParty::GetFirstCalledPhoneNumber();
  nPos = m_indCalled;

  return pPhone;
}

//--------------------------------------------------------------------------
Phone* CRsrvParty::GetNextCalledPhoneNumber()
{
  if ((m_indCalled >= m_numCalledPhones) || (m_indCalled >= MAX_CHANNEL_NUMBER))  return NULL;

  return m_pCalledPhoneNumberList[m_indCalled++];
}

//--------------------------------------------------------------------------
void CRsrvParty::StripSipPartyAddressPrefix()
{
	//PTRACE2(eLevelInfoNormal, "CRsrvParty::StripSipPartyAddressPrefix  sipPartyAddress: ", m_sipPartyAddress);
	// if there is "sip:" prefix in m_sipPartyAddress, cut it
	if(m_sipPartyAddress && !strncmp(m_sipPartyAddress, "sip:", 4) )
	{
		APIU16 prefixLen = strlen("sip:");
		APIU32 addrLen = strlen(m_sipPartyAddress) - prefixLen; // not including "sip:"
		memmove(m_sipPartyAddress, m_sipPartyAddress + prefixLen, (IP_STRING_LEN - prefixLen));
		memset(m_sipPartyAddress + addrLen, '\0', IP_STRING_LEN - addrLen);
		PTRACE2(eLevelInfoNormal, "CRsrvParty::StripSipPartyAddressPrefix  sipPartyAddress after cutting prefix: ", m_sipPartyAddress);
	}
	return;
}
//--------------------------------------------------------------------------
Phone* CRsrvParty::GetNextCalledPhoneNumber(int& nPos)
{
  m_indCalled = nPos;
  Phone* pPhone = CRsrvParty::GetNextCalledPhoneNumber();
  nPos = m_indCalled;

  return pPhone;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetCreationUpdateCounter()
{
  if (m_pRes)
    m_CreationUpdateCounter = m_pRes->GetFullUpdateCounter();
}

//--------------------------------------------------------------------------
BYTE CRsrvParty::GetProtocolType() const
{
  BYTE ret;
  // ISDN_CONNECTION - uses Q.931 signalling
  // DIRECT_CONNECTION - no channel signalling is used
  switch (m_connectionType)
  {
    case DIAL_OUT:
    case DIAL_IN:
    {
      ret = ISDN_CONNECTION;
      break;
    }

    case DIRECT:
    {
      ret = DIRECT_CONNECTION;
      break;
    }

    default:
    {
      ret = ISDN_CONNECTION;
      break;
    }
  } // switch

  return (ret);
}

//--------------------------------------------------------------------------
BYTE CRsrvParty::GetConnectionType() const
{
  // if the reservation is for DIRECT call
  // the mcms will handle it as a dial-out call
  // using the DIRECT_CONNECTION protocol
  if (m_connectionType == DIRECT)
    return (DIAL_OUT);
  else
    return m_connectionType;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetBondingMode1(const BYTE bondingMode1)
{
  m_bondingMode1 = bondingMode1;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetNetChannelNumber(const BYTE netChannelNumber)
{
  m_netChannelNumber = netChannelNumber;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::IncreaseFullUpdateCounter()
{
  if (m_pRes != NULL)
    m_pRes->IncreaseFullUpdateCounter();
}

//--------------------------------------------------------------------------
DWORD CRsrvParty::GetFullUpdateCounter()
{
  if (m_pRes == NULL)
    return 0;
  else
    return m_pRes->GetFullUpdateCounter();
}

//--------------------------------------------------------------------------
void CRsrvParty::SetRestrict(const BYTE restrict)
{
  m_restrict = restrict;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
WORD CRsrvParty::IsRestrictOnly() const
{
  if (m_restrict == 27)
    return TRUE;
  else
    return FALSE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetAGC(const BYTE AGC)
{
  if (m_AGC != AGC)
  {
    m_AGC = AGC;
    SLOWCHANGE;
  }
}

//--------------------------------------------------------------------------
void CRsrvParty::SetSubServiceParams()
{
  // Out going requests from EMA
  if (m_ipSubService != eSecondaryIpSubService)
  {
    m_ipSubService = ePrimaryIpSubService;
  }
  else
  {
    SetSubServiceName(SUB_SERVICE_NAME_SECONDARY);
  }
}

//--------------------------------------------------------------------------
int CRsrvParty::CancelCalledPhoneNumber(const char* phoneNumber)
{
  int ind;
  ind = FindCalledPhoneNumber(phoneNumber);
  if (ind == NOT_FIND) return STATUS_PHONE_NUMBER_NOT_EXISTS;

  // romem Klocwork
  if (ind >= MAX_CHANNEL_NUMBER) return STATUS_MAX_PHONE_NUMBER_EXCEEDED;

  PDELETE(m_pCalledPhoneNumberList[ind]);
  int i = 0;
  if (m_numCalledPhones < MAX_CHANNEL_NUMBER)
  {
    for (i = 0; i < (int)m_numCalledPhones; i++)
    {
      if (m_pCalledPhoneNumberList[i] == NULL)
        break;
    }

    for (int j = i; j < (int)m_numCalledPhones-1; j++)
    {
      m_pCalledPhoneNumberList[j] = m_pCalledPhoneNumberList[j+1];
    }

    m_pCalledPhoneNumberList[m_numCalledPhones-1] = NULL;
    m_numCalledPhones--;
  }

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRsrvParty::FindCalledPhoneNumber(const char* phoneNumber)
{
  for (int i = 0; i < (int)m_numCalledPhones; i++)
  {
    if (m_pCalledPhoneNumberList[i] != NULL)
    {
      if (!strncmp(m_pCalledPhoneNumberList[i]->phone_number, phoneNumber, PHONE_NUMBER_DIGITS_LEN))
        return i;
    }
  }

  return NOT_FIND;
}

//--------------------------------------------------------------------------
int CRsrvParty::CancelCallingPhoneNumber(const char* phoneNumber)
{
  int ind;
  ind = FindCallingPhoneNumber(phoneNumber);
  if (ind == NOT_FIND) return STATUS_PHONE_NUMBER_NOT_EXISTS;

  // ROmem  klocwork
  if (ind >= MAX_CHANNEL_NUMBER)
  {
    return STATUS_PHONE_NUMBER_NOT_EXISTS;
  }

  PASSERTSTREAM_AND_RETURN_VALUE(m_numCallingPhones > MAX_CHANNEL_NUMBER,
    "m_numCallingPhones has invalid value " << m_numCallingPhones, STATUS_FAIL);

  PDELETE(m_pCallingPhoneNumberList[ind]);
  int i;
  for (i = 0; i < (int)m_numCallingPhones; i++)
  {
    if (m_pCallingPhoneNumberList[i] == NULL)
      break;
  }

  int j;
  for (j = i; j < (int)m_numCallingPhones-1; j++)
  {
    m_pCallingPhoneNumberList[j] = m_pCallingPhoneNumberList[j+1];
  }

  m_pCallingPhoneNumberList[m_numCallingPhones-1] = NULL;
  m_numCallingPhones--;

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRsrvParty::FindCallingPhoneNumber(const char* phoneNumber)
{
  for (int i = 0; i < (int)m_numCallingPhones; i++)
  {
    if (m_pCallingPhoneNumberList[i] != NULL)
    {
      if (!strncmp(m_pCallingPhoneNumberList[i]->phone_number, phoneNumber, PHONE_NUMBER_DIGITS_LEN))
        return i;
    }
  }

  return NOT_FIND;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetIpAddress(const mcTransportAddress ipAddress)
{
  memcpy(&m_ipAddress, &ipAddress, sizeof(m_ipAddress));
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetSipPartyAddressType(const WORD partyAddressType)
{
  m_sipPartyAddressType = partyAddressType;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetSipPartyAddress(const char* name)
{
  strcpy_safe(m_sipPartyAddress, name);
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetAudioVolume(const BYTE audioVolume)
{
  m_audioVolume = audioVolume;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetListeningAudioVolume(const BYTE audioVolume)
{
  m_listening_audioVolume = audioVolume;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
WORD CRsrvParty::IsUndefinedParty() const
{
  WORD result = FALSE;
  if (m_undefinedType == UNDEFINED_PARTY || m_undefinedType == UNRESERVED_PARTY)
    result = TRUE;

  return result;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetAddPartyMask(WORD onOff)
{
  if (onOff)
    m_addPartyMask.SetAllBitsOn();
  else
    m_addPartyMask.SetAllBitsOff();
}

//--------------------------------------------------------------------------
CVideoLayout* CRsrvParty::GetCurPrivateVideoLayout() const
{
  for (int i = 0; i < m_numPrivateVideoLayout; i++)
  {
    if (m_pPrivateVideoLayout[i]->IsActive())
      return m_pPrivateVideoLayout[i];
  }

  return NULL;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetNumPrivateVideoLayout(const WORD num)
{
  m_numPrivateVideoLayout = num;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
int CRsrvParty::AddPrivateVideoLayout(const CVideoLayout& other, BYTE bExistLayout)
{
  if (m_numPrivateVideoLayout >= MAX_VIDEO_LAYOUT_NUMBER){
    PASSERTMSG_AND_RETURN_VALUE(1, "Failed, maximum video layouts exceeded", STATUS_MAX_VIDEO_LAYOUTS_EXCEEDED);
  }

  if (FindPrivateVideoLayout(other) != NOT_FIND)
    return UpdatePrivateVideoLayout(other);
	
  m_pPrivateVideoLayout[m_numPrivateVideoLayout] = new CVideoLayout(other);
  m_numPrivateVideoLayout++;

  SLOWCHANGE;

  // Set other layouts to inactive state
  if (!bExistLayout)
  {
    if (other.IsActive())
    {
      for (int i = 0; i < m_numPrivateVideoLayout - 1; i++)
        m_pPrivateVideoLayout[i]->SetActive(NO);
    }
  }

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRsrvParty::UpdatePrivateVideoLayout(const CVideoLayout& other)
{
  int ind;
  ind = FindPrivateVideoLayout(other);
  if (ind == NOT_FIND) return STATUS_VIDEO_SOURCE_NOT_EXISTS;

  // Romem klocwork
  if (ind < MAX_VIDEO_LAYOUT_NUMBER)
  {
    POBJDELETE(m_pPrivateVideoLayout[ind]);
    m_pPrivateVideoLayout[ind] = new CVideoLayout(other);

    // Set other layouts to inactive state
    if (other.IsActive())
    {
      if(m_numPrivateVideoLayout <= MAX_VIDEO_LAYOUT_NUMBER)    
      {
        for (int i = 0; i < m_numPrivateVideoLayout; i++)
          if (i != ind)
            m_pPrivateVideoLayout[i]->SetActive(NO);
      }
	  else
        PASSERTMSG(1, "m_numPrivateVideoLayout is exceed than MAX_VIDEO_LAYOUT_NUMBER"); 	  	

    }
  }

  SLOWCHANGE;

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRsrvParty::CancelPrivateVideoLayout(const BYTE screenLayout)
{
  int ind;
  ind = FindPrivateVideoLayout(screenLayout);
  if (ind == NOT_FIND) return STATUS_VIDEO_SOURCE_NOT_EXISTS;

  if (m_numPrivateVideoLayout > MAX_VIDEO_LAYOUT_NUMBER){
    PASSERTMSG_AND_RETURN_VALUE(1, "Failed, maximum video layouts exceeded", STATUS_MAX_VIDEO_LAYOUTS_EXCEEDED);
  }

  // Romem klocwork
  if (ind < MAX_VIDEO_LAYOUT_NUMBER)
  {
    POBJDELETE(m_pPrivateVideoLayout[ind]);
    int i;
    for (i = 0; i < (int)m_numPrivateVideoLayout; i++)
    {
      if (m_pPrivateVideoLayout[i] == NULL)
        break;
    }

    int j;
    for (j = i; j < (int)m_numPrivateVideoLayout-1; j++)
    {
      m_pPrivateVideoLayout[j] = m_pPrivateVideoLayout[j+1];
    }

    m_pPrivateVideoLayout[m_numPrivateVideoLayout-1] = NULL;
    m_numPrivateVideoLayout--;
  }

  SLOWCHANGE;
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CRsrvParty::FindPrivateVideoLayout(const CVideoLayout& other)
{
 	return FindPrivateVideoLayout(other.GetScreenLayout());
}

//--------------------------------------------------------------------------
int CRsrvParty::FindPrivateVideoLayout(const BYTE screenLayout)
{
  int i;
  for (i = 0; i < (int)m_numPrivateVideoLayout; i++)
  {
    if (m_pPrivateVideoLayout[i] != NULL)
    {
      if (m_pPrivateVideoLayout[i]->GetScreenLayoutITP() == ONE_PLUS_TWO_OVERLAY_ITP &&
	  	(screenLayout ==ONE_PLUS_THREE_OVERLAY_ITP 
	  	|| screenLayout == ONE_PLUS_FOUR_OVERLAY_ITP
	  	||screenLayout == ONE_PLUS_TWO_OVERLAY_ITP)
	  )
      	{
	  	return i;
      	}

	  
      if (m_pPrivateVideoLayout[i]->GetScreenLayout() == screenLayout)
        return i;
    }
  }

  return NOT_FIND;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetRefferedToUri(const char* refferedTo)
{
  int len = strlen(m_refferedToUri);
  strncpy(m_refferedToUri, refferedTo, IP_STRING_LEN - 1);
  m_refferedToUri[IP_STRING_LEN - 1] = 0;

  if (len > IP_STRING_LEN)
    m_refferedToUri[IP_STRING_LEN-1] = '\0';
}

//--------------------------------------------------------------------------
void CRsrvParty::SetRefferedBy(const char* refferedBy)
{
  int len = strlen(m_refferedByUri);
  strncpy(m_refferedByUri, refferedBy, IP_STRING_LEN - 1);
  m_refferedByUri[IP_STRING_LEN - 1] = 0;

  if (len > IP_STRING_LEN)
    m_refferedByUri[IP_STRING_LEN-1] = '\0';
}

//--------------------------------------------------------------------------
void CRsrvParty::SetPartyDefaultParams()
{
  m_meet_me_method       = MEET_ME_PER_USER;                              // MEET_ME_METHOD  - party = MEET_ME_PER_USER
  m_numType              = NUM_TYPE_DEF;                                  // NUM_TYPE - "taken_from_service" (0xFF)//new default is needed???
  m_isVip                = NO;                                            // VIP
  m_backupService[0]     = '\0';                                          // BACKUP_SERVICE_NAME
  m_backupSubService[0]  = '\0';                                          // BACKUP_SUB_SERVICE_NAME
  m_EnableH323_PSTN      = NO;                                            // H323_PSTN
  m_identificationMethod =  CALLED_PHONE_NUMBER_IDENTIFICATION_METHOD;    // IDENTIFICATION_METHOD - "called_phone_number"


}

//--------------------------------------------------------------------------
WORD CRsrvParty::CheckReservRangeValidity(BYTE& errorCode)
{
  errorCode = 0;

  PTRACE2INT(eLevelInfoNormal, "CRsrvParty::CheckReservRangeValidit, bonding mode is: ", m_bondingMode1);

  switch (m_connectionType)
  {
    case DIAL_OUT:
    case DIAL_IN:
    {
      break;
    }
    default:
    {
      errorCode = 101;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  switch (m_bondingMode1)
  {
    case 0:
    case 1:
    case 0xFF:
    {
      break;
    }
    default:
    {
        errorCode = 102;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  if (m_audioThreshold >= E_NOISE_DETECTION_THRESHOLD_LAST)
  {
      errorCode = 103;
    return STATUS_OUT_OF_RANGE;
  }

  switch (m_numType)
  {
    case UNKNOWN:
    case INTERNATIONAL_TYPE:
    case NATIONAL_TYPE:
    case NETWORK_SPECIFIC_TYPE:
    case SUBSCRIBER_TYPE:
    case ABBREVIATED_TYPE:
    case 0xFF:
    {
      break;
    }
    default:
    {
        errorCode = 105;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  switch (m_identificationMethod)
  {
    case PASSWORD_IDENTIFICATION_METHOD:
    case CALLED_PHONE_NUMBER_IDENTIFICATION_METHOD:
    case CALLING_PHONE_NUMBER_IDENTIFICATION_METHOD:
    {
      break;
    }
    default:
    {
        errorCode = 106;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  switch (m_meet_me_method)
  {
    case DIAL_OUT:
    case MEET_ME_PER_MCU:
    case MEET_ME_PER_CONFERENCE:
    case MEET_ME_PER_USER:
    case MEET_ME_PER_CHANNEL:
    case DIAL_IN:
    case DIRECT:
    {
      break;
    }
    default:
    {
        errorCode = 107;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  if (m_numCallingPhones > MAX_CHANNEL_NUMBER)
  {
      errorCode = 108;
    return STATUS_OUT_OF_RANGE;
  }

  if (m_numCalledPhones > MAX_CHANNEL_NUMBER)
  {
      errorCode = 109;
    return STATUS_OUT_OF_RANGE;
  }

  switch (m_netInterfaceType)
  {
    case ISDN_INTERFACE_TYPE:
    case H323_INTERFACE_TYPE:
    case SIP_INTERFACE_TYPE:
    {
      break;
    }
    default:
    {
        errorCode = 110;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  switch (m_videoProtocol)
  {
    case VIDEO_PROTOCOL_H261:
    case VIDEO_PROTOCOL_H263:
    case VIDEO_PROTOCOL_H264:
    case VIDEO_PROTOCOL_RTV:
    case VIDEO_PROTOCOL_MS_SVC:
    case (VIDEO_PROTOCOL_H264_HIGH_PROFILE):
    case AUTO:
    {
      break;
    }
    default:
    {
        errorCode = 111;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  switch (m_h323PartyAliasType)
  {
    case PARTY_H323_ALIAS_H323_ID_TYPE:
    case PARTY_H323_ALIAS_E164_TYPE:
    case PARTY_H323_ALIAS_EMAIL_ID_TYPE:
    case PARTY_H323_ALIAS_PARTY_NUMBER_TYPE:
    {
      break;
    }
    case NONE:
    {
      if (::isApiTaNull(&m_ipAddress))
      {
          errorCode = 112;
        return STATUS_IP_ADDRESS_NOT_VALID;
      }

      break;
    }
    default:
    {
        errorCode = 112;
      return STATUS_INVALID_ALIAS_TYPE;
    }
  } // switch

  if (m_audioVolume > AUDIO_VOLUME_MAX)
  {
      errorCode = 113;
    return STATUS_OUT_OF_RANGE;
  }

  switch (m_undefinedType)
  {
    case 0:
    case UNDEFINED_PARTY:
    case UNRESERVED_PARTY:
    {
      break;
    }
    default:
    {
        errorCode = 114;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  if (m_numPrivateVideoLayout > MAX_VIDEO_LAYOUT_NUMBER)
  {
      errorCode = 115;
    return STATUS_OUT_OF_RANGE;
  }

  if (m_indCalling > MAX_CHANNEL_NUMBER)
  {
      errorCode = 116;
    return STATUS_OUT_OF_RANGE;
  }

  if (m_indCalled > MAX_CHANNEL_NUMBER)
  {
      errorCode = 117;
    return STATUS_OUT_OF_RANGE;
  }

  if (m_ind_prv_vid_layout > MAX_VIDEO_LAYOUT_NUMBER)
  {
      errorCode = 118;
    return STATUS_OUT_OF_RANGE;
  }

  if (m_netChannelNumber > 30 && m_netChannelNumber != AUTO)
  {
      errorCode = 119;
    return STATUS_OUT_OF_RANGE;
  }

  switch (m_sipPartyAddressType)
  {
    case PARTY_SIP_SIPURI_ID_TYPE:
    case PARTY_SIP_TELURL_ID_TYPE:
    {
      break;
    }
    default:
    {
        errorCode = 120;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  if ((m_listening_audioVolume > AUDIO_VOLUME_MAX) || (0 == m_listening_audioVolume))
  {
      errorCode = 121;
    return STATUS_OUT_OF_RANGE;
  }

  if (0xFFFFFFFF != m_videoRate && (m_videoRate < 64 || m_videoRate > 4096))
  {
      errorCode = 122;
    return STATUS_OUT_OF_RANGE;
  }

  if (0 == m_callSignallingPort)
  {
      errorCode = 123;
    return STATUS_OUT_OF_RANGE;
  }

  switch (m_cascadeMode)
  {
    case CASCADE_MODE_MASTER:
    case CASCADE_MODE_SLAVE:
    case CASCADE_MODE_NONE:
      break;

    default:
    {
       errorCode = 124;
      return STATUS_OUT_OF_RANGE;
    }
  }

	if (m_cascadedLinksNumber > MAX_CASCADED_LINKS_NUMBER ||
		(m_cascadedLinksNumber > 1 && m_cascadeMode == CASCADE_MODE_NONE && m_connectionType == DIAL_OUT))
  {
      errorCode = 125;
      return STATUS_OUT_OF_RANGE;
  }

  switch (m_multiRateMode)
  {
    case 0:
    case 1:
    case 0xFF:
    {
      break;
    }
    default:
    {
       errorCode = 126;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  switch (m_restrict)
  {
    case 27: // restrict
    case 28: // derestrict
    case 0xFF:
    {
      break;
    }
    default:
    {
        errorCode = 127;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  switch (m_telePresenceMode)
  {
    case eTelePresencePartyNone:
    case eTelePresencePartyRPX:
    case eTelePresencePartyFlex:
    case eTelePresencePartyMaui:
    case eTelePresencePartyCTS:
    case eTelePresencePartyInactive:
    {
      break;
    }
    default:
    {
       errorCode = 128;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  switch (m_ipSubService)
  {
    case eAutoIpSubService:
    case ePrimaryIpSubService:
    case eSecondaryIpSubService:
    {
      break;
    }

    default:
    {
        errorCode = 129;
      return STATUS_OUT_OF_RANGE;
    }
  } // switch

  return STATUS_OK;
}

//--------------------------------------------------------------------------
CVideoLayout* CRsrvParty::GetFirstPrivateVideoLayout()
{
  m_ind_prv_vid_layout = 1;
  return m_pPrivateVideoLayout[0];
}

//--------------------------------------------------------------------------
CVideoLayout* CRsrvParty::GetNextPrivateVideoLayout()
{
  if (m_ind_prv_vid_layout >= m_numPrivateVideoLayout)
    return NULL;

  return m_pPrivateVideoLayout[m_ind_prv_vid_layout++];
}

//--------------------------------------------------------------------------
CVideoLayout* CRsrvParty::GetFirstPrivateVideoLayout(int& nPos)
{
  CVideoLayout* pPrivateVideoLayout = GetFirstPrivateVideoLayout();
  nPos = m_ind_prv_vid_layout;
  return pPrivateVideoLayout;
}

//--------------------------------------------------------------------------
CVideoLayout* CRsrvParty::GetNextPrivateVideoLayout(int& nPos)
{
  m_ind_prv_vid_layout = nPos;
  CVideoLayout* pPrivateVideoLayout = GetNextPrivateVideoLayout();
  nPos = m_ind_prv_vid_layout;
  return pPrivateVideoLayout;
}

//--------------------------------------------------------------------------
CVideoLayout* CRsrvParty::GetPrivateVideoLayout(const BYTE screenLayout)
{
  int ind = FindPrivateVideoLayout(screenLayout);
  // ROmem klocwork
  if (ind == NOT_FIND || ind >= MAX_VIDEO_LAYOUT_NUMBER)
    return NULL;

  return m_pPrivateVideoLayout[ind];
}

//--------------------------------------------------------------------------
void CRsrvParty::RemoveAllLayouts()
{
  for (int i = 0; i < (int)m_numPrivateVideoLayout; i++)
    POBJDELETE(m_pPrivateVideoLayout[i]);

  m_numPrivateVideoLayout = 0;
  m_ind_prv_vid_layout    = 0;

  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetIsPrivateLayout(BYTE isPrivateLayout)
{
  m_isPrivate = isPrivateLayout;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetUserDefinedInfo(const char* UserInfo, int InfoNumber)
{
  m_pUserDefinedInfo->SetUserDefinedInfo(UserInfo, InfoNumber);
  IncreaseFullUpdateCounter();
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetIsVip(BYTE isVip)
{
  m_isVip = isVip;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
const char* CRsrvParty::GetUserDefinedInfo(int InfoNumber) const
{
  return m_pUserDefinedInfo->GetUserDefinedInfo(InfoNumber);
}

//--------------------------------------------------------------------------
void CRsrvParty::SetAdditionalInfo(const char* additionalInfo)
{
  m_pUserDefinedInfo->SetAdditionalInfo(additionalInfo);
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
const char* CRsrvParty::GetAdditionalInfo() const
{
  return m_pUserDefinedInfo->GetAdditionalInfo();
}

//--------------------------------------------------------------------------
void CRsrvParty::SetEnableH323Pstn(BYTE onOff)
{
  m_EnableH323_PSTN = onOff;
  #ifdef __HIGHC__
    if (::GetpSystemCfg()->GetH323Pstn() == FALSE)
    {
      m_EnableH323_PSTN = NO;
    }
  #endif // ifdef __HIGHC__
}
//--------------------------------------------------------------------------
BYTE CRsrvParty::GetEnableH323Pstn() const
{
  BYTE rVal = m_EnableH323_PSTN;
  #ifdef __HIGHC__
    if (::GetpSystemCfg()->GetH323Pstn() == FALSE)
    {
      rVal = NO;
    }
  #endif // ifdef __HIGHC__
  return rVal;
}

void CRsrvParty::SetTelePresenceMode(BYTE telePresenceMode)
{
	m_telePresenceMode = telePresenceMode;
	SLOWCHANGE;
}
//--------------------------------------------------------------------------
void CRsrvParty::SetRecordingLinkParty(BYTE isRecordingLinkParty)
{
  m_isRecordingLinkParty = isRecordingLinkParty;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
//eFeatureRssDialin
void CRsrvParty::SetPlaybackLinkParty(BYTE isPlaybackLinkParty)
{
  m_isPlaybackParty = isPlaybackLinkParty;
  SLOWCHANGE;
}
//--------------------------------------------------------------------------
//eFeatureRssDialin
void CRsrvParty::SetLastLayoutForRL(BYTE initLayout)
{
  m_lastLayoutForRL= initLayout;
  SLOWCHANGE;
}

//--------------------------------------------------------------------------
void CRsrvParty::SetOperatorParty(COperatorConfInfo& operatorConfInfo)
{
  PTRACE2(eLevelInfoNormal, "void CRsrvParty::SetOperatorParty  [operator_conf_trace] Name:", m_H243_partyName);
  m_isOperatorParty = YES;
  POBJDELETE(m_pOperatorConfInfo);
  m_pOperatorConfInfo = new COperatorConfInfo(operatorConfInfo);
  m_pOperatorConfInfo->Dump();
}

//--------------------------------------------------------------------------
BYTE CRsrvParty::IsOperatorParty() const
{
  if (m_isOperatorParty)
  {
    PTRACE2(eLevelInfoNormal, "void CRsrvParty::IsOperatorParty, YES , Name:", m_H243_partyName);
  }
  else
  {
    PTRACE2(eLevelInfoNormal, "void CRsrvParty::IsOperatorParty, NO , Name:", m_H243_partyName);
  }

  return m_isOperatorParty;
}
//--------------------------------------------------------------------------
void CRsrvParty::ForceSystemListeningVolumeIfNeeded()
{
  if (IsForceSystemListeningVolume())
    m_listening_audioVolume = GetDefaultListeningVolume();
}

//--------------------------------------------------------------------------
BOOL CRsrvParty::IsForceSystemBroadcastVolume()
{
  BOOL isForceSystem = false;

  CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
  if (pSysConfig)
    pSysConfig->GetBOOLDataByKey("FORCE_SYSTEM_BROADCAST_VOLUME", isForceSystem);

  return isForceSystem;
}

//--------------------------------------------------------------------------
void CRsrvParty::ForceSystemBroadcastVolumeIfNeeded()
{
  if (IsForceSystemBroadcastVolume())
    m_audioVolume = GetDefaultBroadcastVolume();
}

//--------------------------------------------------------------------------
BYTE CRsrvParty::GetDefaultBroadcastVolume()
{
  BYTE broadcastVolume = 5;

  CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
  if (pSysConfig)
  {
    DWORD systemVolume = 5;
    pSysConfig->GetDWORDDataByKey("SYSTEM_BROADCAST_VOLUME", systemVolume);

    if (IsForceSystemBroadcastVolume() && systemVolume <= 10)
      broadcastVolume = (BYTE)systemVolume;
  }

  return broadcastVolume;
}

//--------------------------------------------------------------------------
BOOL CRsrvParty::IsForceSystemListeningVolume()
{
  BOOL isForceSystem = false;

  CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
  if (pSysConfig)
    pSysConfig->GetBOOLDataByKey("FORCE_SYSTEM_LISTENING_VOLUME", isForceSystem);

  return isForceSystem;
}

//--------------------------------------------------------------------------
BYTE CRsrvParty::GetDefaultListeningVolume()
{
  BYTE        listening_audioVolume = 5;
  CSysConfig* pSysConfig            = CProcessBase::GetProcess()->GetSysConfig();
  eProductType productType          = CProcessBase::GetProcess()->GetProductType();

  if (eProductTypeCallGeneratorSoftMCU == productType)
  {
	  listening_audioVolume = 1;
  }
  else if (pSysConfig)
  {
    DWORD systemVolume = 5;

    pSysConfig->GetDWORDDataByKey("SYSTEM_LISTENING_VOLUME", systemVolume);

    if (IsForceSystemListeningVolume() && systemVolume <= 10)
      listening_audioVolume = (BYTE)systemVolume;
  }

  return listening_audioVolume;
}
/////////////////////////////////////////////////////////////////////////////
void CRsrvParty::SetPartyMediaType(const ePMediaType type)
{
	if (type > ePartyMediaType_dummy && type < ePartyMediaType_last)
	{
		m_ePartyMediaType = type;
		SLOWCHANGE
	}
}
/////////////////////////////////////////////////////////////////////////////
BYTE CRsrvParty::GetPartyMediaType() const
{
  return m_ePartyMediaType;
}

/////////////////////////////////////////////////////////////////////////////
void CRsrvParty::SetPreDefinedIvrString(const char* szPreDefinedIvrString)
{
  if (szPreDefinedIvrString)
  {
    strncpy(m_PreDefinedIvrString, szPreDefinedIvrString, sizeof(m_PreDefinedIvrString) - 1);
    m_PreDefinedIvrString[sizeof(m_PreDefinedIvrString) - 1] = '\0';
  }
}
/////////////////////////////////////////////////////////////////////////////
void CRsrvParty::SetMsftAvmcuState(enMsftAvmcuState avmcuState)
{
	m_eMsftAvmcuState = avmcuState;
}

/////////////////////////////////////////////////////////////////////////////
enMsftAvmcuState CRsrvParty::GetMsftAvmcuState()
{
	return m_eMsftAvmcuState;
}
/////////////////////////////////////////////////////////////////////////////
void CRsrvParty::SetIsDMAAVMCUParty(BYTE OnOff)
{
	m_IsDmaAvMcuParty = OnOff;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CRsrvParty::GetIsDMAAVMCUParty()
{
	return m_IsDmaAvMcuParty;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRsrvParty::SetIsAvMcuNonCCCPLink(BYTE OnOff)
{
	m_isAvMcuNonCCCPCascade = OnOff;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CRsrvParty::GetIsAvMcuNonCCCPLink()
{
	return m_isAvMcuNonCCCPCascade;
}
