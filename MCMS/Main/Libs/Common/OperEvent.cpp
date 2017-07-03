// +========================================================================+
// OPREVENT.CPP                                                             |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       OPREVENT.CPP                                                 |
// SUBSYSTEM:  MCMSOPER                                                     |
// PROGRAMMER: Michel                                                       |
// -------------------------------------------------------------------------|
// Who | Date       | Description                                           |
// -------------------------------------------------------------------------|
//     |            |                                                       |
// +========================================================================+
#include "OperEvent.h"
#include <stdio.h>
#include "psosxml.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "Macros.h"
#include "CDRDefines.h"
#include "InterfaceType.h"
#include "SharedDefines.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        COperSetEndTime
////////////////////////////////////////////////////////////////////////////
void COperSetEndTime::Serialize(WORD format, std::ostream& m_ostr)
{
  m_new_end_time.SerializeBilling(m_ostr);
  m_ostr << m_operator_name << ";\n";
}

//--------------------------------------------------------------------------
void COperSetEndTime::Serialize(WORD format, std::ostream& m_ostr, BYTE fullformat)
{

  m_ostr << "new end time:";
  m_new_end_time.LongSerialize(m_ostr);
  m_ostr << "operator name:"<<m_operator_name << ";\n\n";
}

//--------------------------------------------------------------------------
void COperSetEndTime::DeSerialize(WORD format, std::istream& istr)
{
  // assuming format = OPERATOR_MCMS
  m_new_end_time.DeSerialize(istr);
  istr.ignore(1);
  istr.getline(m_operator_name, OPERATOR_NAME_LEN+1, ';');
}

//--------------------------------------------------------------------------
void COperSetEndTime::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pOperSetEndTimeNode = pFatherNode->AddChildNode("OPERATOR_SET_END_TIME");

  pOperSetEndTimeNode->AddChildNode("OPERATOR_NAME", m_operator_name);
  pOperSetEndTimeNode->AddChildNode("END_TIME", m_new_end_time);
}

//--------------------------------------------------------------------------
int COperSetEndTime::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "OPERATOR_NAME", m_operator_name, _1_TO_OPERATOR_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "END_TIME", &m_new_end_time, DATE_TIME);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        COperDelParty
////////////////////////////////////////////////////////////////////////////
void COperDelParty::Serialize(WORD format, std::ostream& m_ostr, DWORD apiNum)
{
  m_ostr << m_operator_name << ",";
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    m_ostr << m_party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    m_ostr << tmp << ",";
  }
  m_ostr << m_partyId << ";\n";
}

//--------------------------------------------------------------------------
void COperDelParty::Serialize(WORD format, std::ostream& m_ostr, BYTE fullformat, DWORD apiNum)
{
  m_ostr << "operator name:" << m_operator_name << "\n";
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    m_ostr << "party name:" << m_party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    m_ostr << "party name:" << tmp << "\n";
  }
  m_ostr << "party ID:" << m_partyId << ";\n\n";
}

//--------------------------------------------------------------------------
void COperDelParty::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  istr.getline(m_operator_name, OPERATOR_NAME_LEN+1, ',');
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_partyId;
  istr.ignore(1);
}

//--------------------------------------------------------------------------
void COperDelParty::SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType)
{
  CXMLDOMElement* pOperDelPartyNode = NULL;

  switch (nEventType)
  {
    case OPERATORS_DELETE_PARTY:
      pOperDelPartyNode = pFatherNode->AddChildNode("OPERATOR_DELETE_PARTY");
      break;

    case OPERATOR_DISCONNECTE_PARTY:
      pOperDelPartyNode = pFatherNode->AddChildNode("OPERATOR_DISCONNECT_PARTY");
      break;

    case OPERATOR_RECONNECT_PARTY:
      pOperDelPartyNode = pFatherNode->AddChildNode("OPERATOR_RECONNECT_PARTY");
      break;
  }

  if (pOperDelPartyNode)
  {
    pOperDelPartyNode->AddChildNode("OPERATOR_NAME", m_operator_name);
    pOperDelPartyNode->AddChildNode("NAME", m_party_name);
    pOperDelPartyNode->AddChildNode("ID", m_partyId);
  }
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int COperDelParty::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "OPERATOR_NAME", m_operator_name, _1_TO_OPERATOR_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "NAME", m_party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_partyId, _0_TO_DWORD);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventOperAddParty
////////////////////////////////////////////////////////////////////////////
ACCCDREventOperAddParty::ACCCDREventOperAddParty()
                        :ACCCDREventOperDelParty()
{
  memset(m_pPartyPhoneNumberList, 0, sizeof(ACCCdrPhone*)*MAX_CHANNEL_NUMBER);
  memset(m_pMcuPhoneNumberList  , 0, sizeof(ACCCdrPhone*)*MAX_CHANNEL_NUMBER);

  m_connection_type        = 0;
  m_bonding_mode           = 0;
  m_net_num_of_channel     = 0;
  m_net_channel_width      = 0;
  m_net_service_name[0]    = '\0';
  m_restrict               = NO;
  m_voice                  = NO;
  m_num_type               = 0;
  m_net_subservice_name[0] = '\0';
  m_ident_method           = CALLED_PHONE_NUMBER_IDENTIFICATION_METHOD;
  m_meet_me_method         = MEET_ME_PER_USER; // in mcmsoper.h
  m_NumPartyPhoneNumber    = 0;
  m_NumMcuPhoneNumber      = 0;
  m_indpartyphone          = 0;
  m_indmcuphone            = 0;
}

//--------------------------------------------------------------------------
ACCCDREventOperAddParty::ACCCDREventOperAddParty(const ACCCDREventOperAddParty& other)
                        :ACCCDREventOperDelParty(other)
{
  memset(m_pPartyPhoneNumberList, 0, sizeof(ACCCdrPhone*)*MAX_CHANNEL_NUMBER);
  memset(m_pMcuPhoneNumberList  , 0, sizeof(ACCCdrPhone*)*MAX_CHANNEL_NUMBER);

  *this = other;
}

//--------------------------------------------------------------------------
ACCCDREventOperAddParty::~ACCCDREventOperAddParty()
{
  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    PDELETE(m_pPartyPhoneNumberList[i]);
    PDELETE(m_pMcuPhoneNumberList[i]);
  }
}

//--------------------------------------------------------------------------
ACCCDREventOperAddParty& ACCCDREventOperAddParty::operator=(const ACCCDREventOperAddParty& other)
{
  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    PDELETE(m_pPartyPhoneNumberList[i])
    if (other.m_pPartyPhoneNumberList[i])
      m_pPartyPhoneNumberList[i] = new ACCCdrPhone(*other.m_pPartyPhoneNumberList[i]);

    PDELETE(m_pMcuPhoneNumberList[i])
    if (other.m_pMcuPhoneNumberList[i])
      m_pMcuPhoneNumberList[i] = new ACCCdrPhone(*other.m_pMcuPhoneNumberList[i]);
  }

  m_connection_type     = other.m_connection_type;
  m_bonding_mode        = other.m_bonding_mode;
  m_net_num_of_channel  = other.m_net_num_of_channel;
  m_net_channel_width   = other.m_net_channel_width;
  m_restrict            = other.m_restrict;
  m_voice               = other.m_voice;
  m_num_type            = other.m_num_type;
  m_ident_method        = other.m_ident_method;
  m_meet_me_method      = other.m_meet_me_method; // in mcmsoper.h
  m_NumPartyPhoneNumber = other.m_NumPartyPhoneNumber;
  m_NumMcuPhoneNumber   = other.m_NumMcuPhoneNumber;
  m_indpartyphone       = other.m_indpartyphone;
  m_indmcuphone         = other.m_indmcuphone;

  SAFE_COPY(m_net_service_name, other.m_net_service_name);
  SAFE_COPY(m_net_subservice_name, other.m_net_subservice_name);

  ((ACCCDREventOperDelParty&)*this) = (ACCCDREventOperDelParty&)other;
  return *this;
}

//--------------------------------------------------------------------------
bool ACCCDREventOperAddParty::operator ==(const ACCCDREventOperAddParty& other)
{
  if (this == &other) return true;

  if (m_connection_type     != other.m_connection_type)                 return false;
  if (m_bonding_mode        != other.m_bonding_mode)                    return false;
  if (m_net_num_of_channel  != other.m_net_num_of_channel)              return false;
  if (m_net_channel_width   != other.m_net_channel_width)               return false;
  if (m_restrict            != other.m_restrict)                        return false;
  if (m_voice               != other.m_voice)                           return false;
  if (m_num_type            != other.m_num_type)                        return false;
  if (m_ident_method        != other.m_ident_method)                    return false;
  if (m_meet_me_method      != other.m_meet_me_method)                  return false;
  if (m_NumPartyPhoneNumber != other.m_NumPartyPhoneNumber)             return false;
  if (m_NumMcuPhoneNumber   != other.m_NumMcuPhoneNumber)               return false;
  if (m_ident_method        != other.m_ident_method)                    return false;
  if (m_indpartyphone       != other.m_indpartyphone)                   return false;
  if (m_indmcuphone         != other.m_indmcuphone)                     return false;

  if (0 != strcmp(m_net_service_name, other.m_net_service_name))        return false;
  if (0 != strcmp(m_net_subservice_name, other.m_net_subservice_name))  return false;

  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    if (NULL == m_pPartyPhoneNumberList[i] && NULL == other.m_pPartyPhoneNumberList[i])
      continue;

    if (NULL == m_pPartyPhoneNumberList[i] && NULL != other.m_pPartyPhoneNumberList[i])
      return false;

    if (NULL != m_pPartyPhoneNumberList[i] && NULL == other.m_pPartyPhoneNumberList[i])
      return false;

    if (*(m_pPartyPhoneNumberList[i]) != *(other.m_pPartyPhoneNumberList[i]))
      return false;
  }

  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    if (NULL == m_pMcuPhoneNumberList[i] && NULL == other.m_pMcuPhoneNumberList[i])
      continue;

    if (NULL == m_pMcuPhoneNumberList[i] && NULL != other.m_pMcuPhoneNumberList[i])
      return false;

    if (NULL != m_pMcuPhoneNumberList[i] && NULL == other.m_pMcuPhoneNumberList[i])
      return false;

    if (*(m_pMcuPhoneNumberList[i]) != *(other.m_pMcuPhoneNumberList[i]))
      return false;
  }

  return true;
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventOperAddParty::GetFirstPartyPhoneNumber()
{
  m_indpartyphone = 1;
  return m_pPartyPhoneNumberList[0];
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventOperAddParty::GetFirstPartyPhoneNumber(int& nPos)
{
  ACCCdrPhone* pPhone = ACCCDREventOperAddParty::GetFirstPartyPhoneNumber();
  nPos = m_indpartyphone;
  return pPhone;
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventOperAddParty::GetNextPartyPhoneNumber()
{
  if (m_indpartyphone >= m_NumPartyPhoneNumber)
    return m_pPartyPhoneNumberList[0];

  return m_pPartyPhoneNumberList[m_indpartyphone++];
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventOperAddParty::GetNextPartyPhoneNumber(int& nPos)
{
  m_indpartyphone = nPos;
  ACCCdrPhone* pPhone = ACCCDREventOperAddParty::GetNextPartyPhoneNumber();
  nPos = m_indpartyphone;
  return pPhone;
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventOperAddParty::GetFirstMcuPhoneNumber()
{
  m_indmcuphone = 1;
  return m_pMcuPhoneNumberList[0];
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventOperAddParty::GetFirstMcuPhoneNumber(int& nPos)
{
  ACCCdrPhone* pPhone = ACCCDREventOperAddParty::GetFirstMcuPhoneNumber();
  nPos = m_indmcuphone;
  return pPhone;
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventOperAddParty::GetNextMcuPhoneNumber()
{
  if (m_indmcuphone > m_NumMcuPhoneNumber)
    return NULL;

  return m_pMcuPhoneNumberList[m_indmcuphone++];
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventOperAddParty::GetNextMcuPhoneNumber(int& nPos)
{
  m_indmcuphone = nPos;
  ACCCdrPhone* pPhone = ACCCDREventOperAddParty::GetNextMcuPhoneNumber();
  nPos = m_indmcuphone;
  return pPhone;
}

//--------------------------------------------------------------------------
int ACCCDREventOperAddParty::FindMcuPhoneNumber(const char* mcuphoneNumber)
{
  std::string strMcuphoneNumber = mcuphoneNumber;
  for (int i = 0; i < (int)m_NumMcuPhoneNumber; i++)
    if (m_pMcuPhoneNumberList[i] && m_pMcuPhoneNumberList[i]->phone_number == strMcuphoneNumber)
      return i;
  return NOT_FIND;
}

//--------------------------------------------------------------------------
int ACCCDREventOperAddParty::FindPartyPhoneNumber(const char* partyphoneNumber)
{
  std::string strPartyphoneNumber = partyphoneNumber;
  for (int i = 0; i < (int)m_NumPartyPhoneNumber; i++)
    if (m_pPartyPhoneNumberList[i] && m_pPartyPhoneNumberList[i]->phone_number == strPartyphoneNumber)
      return i;
  return NOT_FIND;
}


////////////////////////////////////////////////////////////////////////////
//                        COperAddParty
////////////////////////////////////////////////////////////////////////////
void COperAddParty::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  ostr << m_operator_name << ",";
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    ostr << tmp << ",";
  }

  ostr << m_partyId                     << ",";
  ostr << (WORD)m_connection_type       << ",";
  ostr << (WORD)m_bonding_mode          << ",";
  ostr << (WORD)m_net_num_of_channel    << ",";
  ostr << (WORD)m_net_channel_width     << ",";
  ostr << m_net_service_name            << ",";
  ostr << (WORD)m_restrict              << ",";
  ostr << (WORD)m_voice                 << ",";
  ostr << (WORD)m_num_type              << ",";
  ostr << m_net_subservice_name         << ",";
  ostr << (WORD)m_NumPartyPhoneNumber   << ",";
  ostr << (WORD)m_NumMcuPhoneNumber     << ",";

  for (int i = 0; i < (int)m_NumPartyPhoneNumber; i++)
    ostr << m_pPartyPhoneNumberList[i]->phone_number << ",";

  for (int i = 0; i < (int)m_NumMcuPhoneNumber; i++)
    ostr << m_pMcuPhoneNumberList[i]->phone_number << ",";

  ostr << (WORD)m_ident_method   << ",";
  ostr << (WORD)m_meet_me_method << ";\n";
}

//--------------------------------------------------------------------------
void COperAddParty::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  WORD tmp;

  istr.getline(m_operator_name, OPERATOR_NAME_LEN+1, ',');
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_partyId;
  istr.ignore(1);
  istr >> tmp;
  m_connection_type = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_bonding_mode = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_net_num_of_channel = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_net_channel_width = (BYTE)tmp;
  istr.ignore(1);
  istr.getline(m_net_service_name, NET_SERVICE_PROVIDER_NAME_LEN+1, ',');
  istr >> tmp;
  m_restrict = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_voice = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_num_type = (BYTE)tmp;
  istr.ignore(1);
  istr.getline(m_net_subservice_name, NET_SERVICE_PROVIDER_NAME_LEN+1, ',');

  istr >> tmp;
  m_NumPartyPhoneNumber = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_NumMcuPhoneNumber = (BYTE)tmp;

  istr.ignore(1);
  int  i;
  char buff[PHONE_NUMBER_DIGITS_LEN];
  for (i = 0; i < MAX_CHANNEL_NUMBER && i < (int)m_NumPartyPhoneNumber; i++)
  {
    m_pPartyPhoneNumberList[i] = new ACCCdrPhone;
    istr.getline(buff, PHONE_NUMBER_DIGITS_LEN+1, ',');
    m_pPartyPhoneNumberList[i]->phone_number = buff;
  }

  for (i = 0; i < MAX_CHANNEL_NUMBER && i < (int)m_NumMcuPhoneNumber; i++)
  {
    m_pMcuPhoneNumberList[i] = new ACCCdrPhone;
    istr.getline(buff, PHONE_NUMBER_DIGITS_LEN+1, ',');
    m_pMcuPhoneNumberList[i]->phone_number = buff;
  }

  istr >> tmp;
  m_ident_method = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_meet_me_method = (BYTE)tmp;
  istr.ignore(1);
}

//--------------------------------------------------------------------------
void COperAddParty::Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (strcmp(m_operator_name, ""))
    ostr << "operator name:" << m_operator_name << "\n";

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    ostr << "party name:" << tmp << "\n";
  }

  ostr << "party ID:" << m_partyId << "\n";
  switch (m_connection_type)
  {
    case TYPE_DIAL_OUT   : { ostr << "connection type :dial out\n"; break; }
    case TYPE_DIAL_IN    : { ostr << "connection type :dial in\n" ; break; }
    case TYPE_DIRECT     : { ostr << "connection type :direct\n"  ; break; }
    default              : { ostr << "--\n"                       ; break; }
  }

  switch (m_bonding_mode)
  {
    case NO_BONDING      : { ostr << "no bonding\n"  ; break; }
    case BONDING         : { ostr << "bonding\n"     ; break; }
    case AUTO_BONDING    : { ostr << "auto bonding\n"; break; }
    default              : { ostr << "--\n"          ; break; }
  }

  switch (m_net_num_of_channel)
  {
    case AUTO_NUMBER     : { ostr << "channel number auto,\n"; break; }
    default              : { ostr << "number of channel:" << (WORD)m_net_num_of_channel << ",\n"; break; }
  }

  switch (m_net_channel_width)
  {
    case Xfer_64         : { ostr << "net channel width:1B\n"       ; break; }
    case Xfer_2x64       : { ostr << "net channel width:2B\n"       ; break; }
    case Xfer_3x64       : { ostr << "net channel width:3B\n"       ; break; }
    case Xfer_4x64       : { ostr << "net channel width:4B\n"       ; break; }
    case Xfer_5x64       : { ostr << "net channel width:5B\n"       ; break; }
    case Xfer_6x64       : { ostr << "net channel width:6B\n"       ; break; }
    case Xfer_96         : { ostr << "net channel width:96 kbps\n"  ; break; }
    case Xfer_128        : { ostr << "net channel width:128 kbps\n" ; break; }
    case Xfer_192        : { ostr << "net channel width:192 kbps\n" ; break; }
    case Xfer_256        : { ostr << "net channel width:256 kbps\n" ; break; }
    case Xfer_320        : { ostr << "net channel width:320 kbps\n" ; break; }
    case Xfer_384        : { ostr << "net channel width:384 kbps\n" ; break; }
    case Xfer_512        : { ostr << "net channel width:512 kbps\n" ; break; }
    case Xfer_768        : { ostr << "net channel width:768 kbps\n" ; break; }
    case Xfer_832        : { ostr << "net channel width:832 kbps\n" ; break; }
    case Xfer_1152       : { ostr << "net channel width:1152 kbps\n"; break; }
    case Xfer_1472       : { ostr << "net channel width:1472 kbps\n"; break; }
    case Xfer_1536       : { ostr << "net channel width:1536 kbps\n"; break; }
    case Xfer_1728       : { ostr << "net channel width:1728 kbps\n"; break; }
    case Xfer_1920       : { ostr << "net channel width:1920 kbps\n"; break; }
    case Xfer_2048       : { ostr << "net channel width:2048 kbps\n"; break; }
    case Xfer_1024       : { ostr << "net channel width:1024 kbps\n"; break; }
    case Xfer_4096       : { ostr << "net channel width:4096 kbps\n"; break; }
    case Xfer_6144       : { ostr << "net channel width:6144 kbps\n"; break; }
    case Xfer_2560       : { ostr << "net channel width:2560 kbps\n"; break; }
    case Xfer_3072       : { ostr << "net channel width:3072 kbps\n"; break; }
    case Xfer_3584       : { ostr << "net channel width:3584 kbps\n"; break; }
    case Xfer_8192       : { ostr << "net channel width:8192 kbps\n"; break; }
    default              : { ostr << "--\n"                         ; break; }
  }

  ostr << "net service name:" << m_net_service_name << "\n";
  switch (m_restrict)
  {
    case RESTRICT        : { ostr << "restrict mode\n"; break; }
    case NON_RESTRICT    : { ostr << "non restrict\n" ; break; }
    case UNKNOWN_RESTRICT: { ostr << "restrict:auto\n"; break; }
    default              : { ostr << "--\n"           ; break; }
  }

  switch (m_voice)
  {
    case VOICE           : { ostr << "voice mode:YES\n"    ; break; }
    case NO_VOICE        : { ostr << "voice mode:NO\n"     ; break; }
    case UNKNOWN_VOICE   : { ostr << "voice mode unknown\n"; break; }
    default              : { ostr << "--\n"                ; break; }
  }

  switch (m_num_type)
  {
    case UNKNOWN_TYPE    : { ostr << "number type:unknown\n"           ; break; }
    case INTERNATIONAL   : { ostr << "number type:international\n"     ; break; }
    case NATIONAL        : { ostr << "number type:national\n"          ; break; }
    case NETWORK_SPECIFIC: { ostr << "number type:network specific\n"  ; break; }
    case SUBSCRIBER      : { ostr << "number type:subscriber\n"        ; break; }
    case ABBREVIATED     : { ostr << "number type:abbreviated\n"       ; break; }
    case NUM_TYPE_DEF    : { ostr << "number type:taken from service\n"; break; }
    default              : { ostr << "--\n"                            ; break; }
  }

  ostr << "net subservice name:" << m_net_subservice_name << "\n";

  if (m_NumPartyPhoneNumber > 0)
  {
    ostr << "party phone number:";
    for (int i = 0; i < (int)m_NumPartyPhoneNumber; i++)
      ostr << m_pPartyPhoneNumberList[i]->phone_number << ",";
    ostr << "\n";
  }

  if (m_NumMcuPhoneNumber > 0)
  {
    ostr << "MCU phone number:";
    for (int j = 0; j < m_NumMcuPhoneNumber; j++)
      ostr <<m_pMcuPhoneNumberList[j]->phone_number << ",";
    ostr <<"\n";
  }

  switch (m_ident_method)
  {
    case PASSWORD       : { ostr << "ident. method:password\n"      ; break; }
    case CALLED_NUMBER  : { ostr << "ident. method:called number\n" ; break; }
    case CALLING_NUMBER : { ostr << "ident. method:calling number\n"; break; }
    default             : { ostr << "--\n"                          ; break; }
  }

  switch (m_meet_me_method)
  {
    case MEET_PER_MCU       : { ostr << "meet method: per MCU;\n\n"       ; break; }
    case MEET_PER_CONFERENCE: { ostr << "meet method: per conference;\n\n"; break; }
    case MEET_PER_PARTY     : { ostr << "meet method: per party;\n\n"     ; break; }
    case MEET_PER_CHANNEL   : { ostr << "meet method: per channel;\n\n"   ; break; }
    default                 : { ostr << "--;\n\n"                         ; break; }
  }
}

//--------------------------------------------------------------------------
void COperAddParty::SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType)
{
  CXMLDOMElement* pAddPartyNode = NULL;

  switch (nEventType)
  {
    case OPERATOR_ADD_PARTY:
      pAddPartyNode = pFatherNode->AddChildNode("OPERATOR_ADD_PARTY");
      break;

    case RESERVED_PARTY:
      pAddPartyNode = pFatherNode->AddChildNode("RESERVED_PARTY");
      break;

    case OPERATOR_UPDATE_PARTY:
      pAddPartyNode = pFatherNode->AddChildNode("OPERATOR_UPDATE_PARTY");
      break;
  }

  if (pAddPartyNode)
  {
    pAddPartyNode->AddChildNode("CONNECTION", m_connection_type, CONNECTION_ENUM);
    pAddPartyNode->AddChildNode("BONDING", m_bonding_mode, BONDING_ENUM);
    pAddPartyNode->AddChildNode("SERVICE_NAME", m_net_service_name);
    pAddPartyNode->AddChildNode("SUB_SERVICE_NAME", m_net_subservice_name);
    pAddPartyNode->AddChildNode("CALL_CONTENT", m_voice, CALL_CONTENT_ENUM);
    pAddPartyNode->AddChildNode("NUM_TYPE", m_num_type, NUM_TYPE_ENUM);
    pAddPartyNode->AddChildNode("MEET_ME_METHOD", m_meet_me_method, MEET_ME_METHOD_ENUM);
    pAddPartyNode->AddChildNode("TRANSFER_RATE", m_net_channel_width, TRANSFER_RATE_ENUM);

//VNGR-25242, map '0' to 'AUTO'
    BYTE  ucNetChannelNumber  = AUTO;
    if(0 != m_net_num_of_channel)
    {
         ucNetChannelNumber = m_net_num_of_channel;
    }
    pAddPartyNode->AddChildNode("NET_CHANNEL_NUMBER", ucNetChannelNumber, NET_CHANNEL_NUMBER_ENUM);

    CXMLDOMElement* pPartyPhoneListNode = pAddPartyNode->AddChildNode("PHONE_LIST");
    CXMLDOMElement* pMCUPhoneListNode   = pAddPartyNode->AddChildNode("MCU_PHONE_LIST");

    char szPhoneNodeName[10];
    for (int i = 0; i < 6; i++)
    {
      snprintf(szPhoneNodeName, sizeof(szPhoneNodeName), "PHONE%d", i+1);

      if (i < m_NumPartyPhoneNumber)
        pPartyPhoneListNode->AddChildNode(szPhoneNodeName, m_pPartyPhoneNumberList[i]->phone_number.c_str());

      if (i < m_NumMcuPhoneNumber)
        pMCUPhoneListNode->AddChildNode(szPhoneNodeName, m_pMcuPhoneNumberList[i]->phone_number.c_str());
    }

    pAddPartyNode->AddChildNode("OPERATOR_NAME", m_operator_name);
    pAddPartyNode->AddChildNode("NAME", m_party_name);
    pAddPartyNode->AddChildNode("ID", m_partyId);
    pAddPartyNode->AddChildNode("IDENTIFICATION_METHOD", m_ident_method, IDENT_METHOD_ENUM);
  }
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int COperAddParty:: DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "CONNECTION", &m_connection_type, CONNECTION_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "BONDING", &m_bonding_mode, BONDING_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "SERVICE_NAME", m_net_service_name, NET_SERVICE_PROVIDER_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "SUB_SERVICE_NAME", m_net_subservice_name, NET_SERVICE_PROVIDER_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "RESTRICT_MODE", &m_restrict, RESTRICT_MODE_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "CALL_CONTENT", &m_voice, CALL_CONTENT_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "NUM_TYPE", &m_num_type, NUM_TYPE_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "MEET_ME_METHOD", &m_meet_me_method, MEET_ME_METHOD_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "TRANSFER_RATE", &m_net_channel_width, TRANSFER_RATE_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "NET_CHANNEL_NUMBER", &m_net_num_of_channel, NET_CHANNEL_NUMBER_ENUM);

  char            szPhoneNodeName[10];
  CXMLDOMElement* pPhoneList = NULL;
  CXMLDOMElement* pChild     = NULL;

  m_NumPartyPhoneNumber = m_NumMcuPhoneNumber = 0;
  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    PDELETEA(m_pPartyPhoneNumberList[i]);
    PDELETEA(m_pMcuPhoneNumberList[i]);
  }

  GET_CHILD_NODE(pActionNode, "PHONE_LIST", pPhoneList);
  if (pPhoneList)
  {
    for (int i = 0; i < 6; i++)
    {
      snprintf(szPhoneNodeName, sizeof(szPhoneNodeName), "PHONE%d", i+1);

      GET_CHILD_NODE(pPhoneList, szPhoneNodeName, pChild);
      if (!pChild)
        break;

      m_pPartyPhoneNumberList[i] = new ACCCdrPhone;
      GET_VALIDATE_CHILD(pPhoneList, szPhoneNodeName, m_pPartyPhoneNumberList[i]->phone_number, PHONE_NUMBER_DIGITS_LENGTH);
      m_NumPartyPhoneNumber++;
    }
  }

  GET_CHILD_NODE(pActionNode, "MCU_PHONE_LIST", pPhoneList);
  if (pPhoneList)
  {
    for (int i = 0; i < 6; i++)
    {
      snprintf(szPhoneNodeName, sizeof(szPhoneNodeName), "PHONE%d", i+1);

      GET_CHILD_NODE(pPhoneList, szPhoneNodeName, pChild);
      if (!pChild)
        break;

      m_pMcuPhoneNumberList[i] = new ACCCdrPhone;
      GET_VALIDATE_CHILD(pPhoneList, szPhoneNodeName, m_pMcuPhoneNumberList[i]->phone_number, PHONE_NUMBER_DIGITS_LENGTH);
      m_NumMcuPhoneNumber++;
    }
  }

  GET_VALIDATE_CHILD(pActionNode, "OPERATOR_NAME", m_operator_name, _1_TO_OPERATOR_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "NAME", m_party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_partyId, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "IDENTIFICATION_METHOD", &m_ident_method, IDENT_METHOD_ENUM);

  return STATUS_OK;
}

//--------------------------------------------------------------------------
void COperAddParty::AddPartyPhoneNumber(const char* partyphoneNumber)
{
  m_pPartyPhoneNumberList[m_NumPartyPhoneNumber] = new ACCCdrPhone;
  m_pPartyPhoneNumberList[m_NumPartyPhoneNumber]->phone_number = partyphoneNumber;
  m_NumPartyPhoneNumber++;
}

//--------------------------------------------------------------------------
void COperAddParty::AddMcuPhoneNumber(const char* mcuphoneNumber)
{
  m_pMcuPhoneNumberList[m_NumMcuPhoneNumber] = new ACCCdrPhone;
  m_pMcuPhoneNumberList[m_NumMcuPhoneNumber]->phone_number = mcuphoneNumber;
  m_NumMcuPhoneNumber++;
}

//--------------------------------------------------------------------------
int COperAddParty::CancelMcuPhoneNumber(const char* mcuphoneNumber)
{

  if(m_NumPartyPhoneNumber > MAX_CHANNEL_NUMBER)
    return STATUS_MAX_PHONE_NUMBER_EXCEEDED;

  int ind = FindMcuPhoneNumber(mcuphoneNumber);
  if (ind == NOT_FIND || ind >= MAX_CHANNEL_NUMBER)
    return STATUS_PHONE_NUMBER_NOT_EXISTS;

  PDELETE(m_pMcuPhoneNumberList[ind]);

  BYTE i;
  for (i = 0; i < m_NumMcuPhoneNumber && i < MAX_CHANNEL_NUMBER; i++)
    if (m_pMcuPhoneNumberList[i] == NULL)
      break;

  for (BYTE j = i; j < m_NumMcuPhoneNumber-1 && j < MAX_CHANNEL_NUMBER-1 ; j++)
    m_pMcuPhoneNumberList[j] = m_pMcuPhoneNumberList[j+1];

  if (m_NumMcuPhoneNumber <=  MAX_CHANNEL_NUMBER)
  {
	  m_pMcuPhoneNumberList[m_NumMcuPhoneNumber-1] = NULL;
	  --m_NumMcuPhoneNumber;
  }

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int COperAddParty::CancelPartyPhoneNumber(const char* partyphoneNumber)
{
  int ind = FindPartyPhoneNumber(partyphoneNumber);
  if (ind == NOT_FIND || ind >= MAX_CHANNEL_NUMBER)
    return STATUS_PHONE_NUMBER_NOT_EXISTS;

  if(m_NumPartyPhoneNumber > MAX_CHANNEL_NUMBER)
    return STATUS_MAX_PHONE_NUMBER_EXCEEDED;

  PDELETE(m_pPartyPhoneNumberList[ind]);

  int i;
  for (i = 0; i < (int)m_NumPartyPhoneNumber; i++)
  {
    if (m_pPartyPhoneNumberList[i] == NULL)
      break;
  }

  for (int j = i; j < (int)m_NumPartyPhoneNumber-1; j++)
  {
    m_pPartyPhoneNumberList[j] = m_pPartyPhoneNumberList[j+1];
  }

  m_pPartyPhoneNumberList[m_NumPartyPhoneNumber-1] = NULL;
  m_NumPartyPhoneNumber--;

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventOperAddPartyCont1
////////////////////////////////////////////////////////////////////////////
ACCCDREventOperAddPartyCont1::ACCCDREventOperAddPartyCont1()
{
  memset(this, 0, sizeof(*this));

  m_nodeType            = 0;
  m_chair               = NO;
  m_netInterfaceType    = ISDN_INTERFACE_TYPE;
  m_ipAddress           = 0xFFFFFFFF;
  m_callSignallingPort  = 0xFFFF;
  m_videoProtocol       = AUTO;
  m_videoRate           = 0xFFFFFFFF;
  m_bondingPhoneNumber  = new ACCCdrPhone();
  m_IpPartyAliasType    = PARTY_H323_ALIAS_H323_ID_TYPE;
  m_sipPartyAddressType = PARTY_SIP_SIPURI_ID_TYPE;
  m_audioVolume         = 5;
  m_undefinedType       = NO;
}

//--------------------------------------------------------------------------
ACCCDREventOperAddPartyCont1::ACCCDREventOperAddPartyCont1(const ACCCDREventOperAddPartyCont1& other)
{
  memset(this, 0, sizeof(*this));
  *this = other;
}

//--------------------------------------------------------------------------
ACCCDREventOperAddPartyCont1& ACCCDREventOperAddPartyCont1::operator=(const ACCCDREventOperAddPartyCont1& other)
{
  if(this == &other){
    return *this;
  }

  PDELETE(m_bondingPhoneNumber);
  memcpy(this, &other, sizeof(*this));
  m_bondingPhoneNumber = new ACCCdrPhone();
  m_bondingPhoneNumber->phone_number = other.m_bondingPhoneNumber->phone_number;
  return *this;
}

//--------------------------------------------------------------------------
ACCCDREventOperAddPartyCont1::~ACCCDREventOperAddPartyCont1()
{
  PDELETE(m_bondingPhoneNumber);
}


////////////////////////////////////////////////////////////////////////////
//                        COperAddPartyCont1
////////////////////////////////////////////////////////////////////////////
void COperAddPartyCont1::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  ostr << (WORD)m_netInterfaceType  << ",";

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_H243_password  << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_H243_password);
    ostr << tmp  << ",";
  }

  ostr << (WORD)m_chair  << ",";
  ostr << (WORD)m_videoProtocol  << ",";
  ostr << (WORD)m_audioVolume << ",";
  ostr << (WORD)m_undefinedType << ",";
  ostr << (WORD)m_nodeType  << ",";
  ostr << m_bondingPhoneNumber->phone_number << ",";
  ostr << m_videoRate  << ",";
  ostr << m_ipAddress  << ",";
  ostr << m_callSignallingPort  << ",";
  if (m_netInterfaceType != SIP_INTERFACE_TYPE)
  {
    ostr << m_IpPartyAliasType << ",";
    ostr << m_IpPartyAlias << ";\n";
  }
  else
  {
    ostr << m_sipPartyAddressType << ",";
    ostr << m_sipPartyAddress << ";\n";
  }
}

//--------------------------------------------------------------------------
void COperAddPartyCont1::Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum)
{
  switch (m_netInterfaceType)
  {
    case ISDN_INTERFACE_TYPE : { ostr << "net interface type:ISDN\n" ; break; }
    case ATM_INTERFACE_TYPE  : { ostr << "net interface type:ATM\n"  ; break; }
    case H323_INTERFACE_TYPE : { ostr << "net interface type:323\n"  ; break; }
    case V35_INTERFACE_TYPE  : { ostr << "net interface type:MPI8\n" ; break; }
    case T1CAS_INTERFACE_TYPE: { ostr << "net interface type:T1CAS\n"; break; }
    case SIP_INTERFACE_TYPE  : { ostr << "net interface type:SIP\n"  ; break; }
    default                  : { ostr << "--\n"                      ; break; }
  }

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "H243 password:" << m_H243_password << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_H243_password);
    ostr << "H243 password:" << tmp << "\n";
  }

  switch (m_chair)
  {
    case 0 : { ostr << "chair:NO\n" ; break; }
    case 1 : { ostr << "chair:YES\n"; break; }
    default: { ostr << "--\n"       ; break; }
  }

  switch (m_videoProtocol)
  {
    case VIDEO_PROTOCOL_H261: { ostr << "video protocol:H261 only\n"; break; }
    case VIDEO_PROTOCOL_H263: { ostr << "video protocol:H263\n"     ; break; }
    case VIDEO_PROTOCOL_H26L: { ostr << "video protocol:H264*\n"    ; break; }
    case VIDEO_PROTOCOL_H264: { ostr << "video protocol:H264\n"     ; break; }
    case 0xff               : { ostr << "video protocol:auto\n"     ; break; }
    default                 : { ostr << "--\n"                      ; break; }
  }

  ostr << "audio volume:" << (WORD)m_audioVolume << "\n";

  switch ((WORD)m_undefinedType)
  {
    case 1 : { ostr << "undefined type:undefined party\n" ; break; }
    case 2 : { ostr << "undefined type:unreserved party\n"; break; }
    case 0 : { ostr << "undefined type:default\n"         ; break; }
    default: { ostr << "--\n"                             ; break; }
  }

  switch ((WORD)m_nodeType)
  {
    case 0 : { ostr << "node type:MCU\n"     ; break; }
    case 1 : { ostr << "node type:Terminal\n"; break; }
    default: { ostr << "--\n"                ; break; }
  }

  ostr << "bonding phone number:" << m_bondingPhoneNumber->phone_number << "\n";
  if (m_videoRate != 0xFFFFFFFF)
    ostr << "video rate:" << m_videoRate << "\n";
  else
    ostr << "video rate:auto\n";

  if (m_ipAddress != 0xFFFFFFFF)
  {
    char* tmp = TranslDwordToString(m_ipAddress);
    ostr << "Ip address:" << tmp  << "\n";
    PDELETEA(tmp);
  }
  else
    ostr << "Ip address:" << "\n";

  ostr << "call signalling port:" << m_callSignallingPort << "\n";

  if (m_netInterfaceType == SIP_INTERFACE_TYPE)
  {
    switch (m_sipPartyAddressType)
    {
      case PARTY_SIP_SIPURI_ID_TYPE: { ostr << "SIP party address type:SIP URI type\n"    ; break; }
      case PARTY_SIP_TELURL_ID_TYPE: { ostr << "SIP party address type:SIP TEL URL type\n"; break; }
      default                      : { ostr << "--\n"                                     ; break; }
    }

    ostr << "SIP party address: " << m_sipPartyAddress << ";\n\n";
  }

  if (m_netInterfaceType == H323_INTERFACE_TYPE)
  {
    switch (m_IpPartyAliasType)
    {
      case PARTY_H323_ALIAS_H323_ID_TYPE     : { ostr << "H323 party alias type:alias H323 type\n"        ; break; }
      case PARTY_H323_ALIAS_E164_TYPE        : { ostr << "H323 party alias type:alias E164 type\n"        ; break; }
      case PARTY_H323_ALIAS_URL_ID_TYPE      : { ostr <<"H323 party alias type:alias URL ID type\n"       ; break; }
      case PARTY_H323_ALIAS_TRANSPORT_ID_TYPE: { ostr << "H323 party alias type:alias transport ID type\n"; break; }
      case PARTY_H323_ALIAS_EMAIL_ID_TYPE    : { ostr << "H323 party alias type:alias email ID type\n"    ; break; }
      case PARTY_H323_ALIAS_PARTY_NUMBER_TYPE: { ostr << "H323 party alias type:alias party number type\n"; break; }
      default                                : { ostr << "--\n"                                           ; break; }
    }
    ostr << "H323 party alias: " << m_IpPartyAlias << ";\n\n";
  }
}

//--------------------------------------------------------------------------
void COperAddPartyCont1:: SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType)
{
  CXMLDOMElement* pAddPartyCont1Node = NULL;

  switch (nEventType)
  {
    case RESERVED_PARTY_CONTINUE_1:
      pAddPartyCont1Node = pFatherNode->AddChildNode("RESERVED_PARTY_1");
      break;

    case OPERATOR_ADD_PARTY_CONTINUE_1:
      pAddPartyCont1Node = pFatherNode->AddChildNode("OPERATOR_ADD_PARTY_1");
      break;

    case OPERATOR_UPDATE_PARTY_CONTINUE_1:
      pAddPartyCont1Node = pFatherNode->AddChildNode("OPERATOR_UPDATE_PARTY_1");
      break;
  }

  if (pAddPartyCont1Node)
  {
    pAddPartyCont1Node->AddChildNode("CHAIR", m_chair, _BOOL);
    pAddPartyCont1Node->AddChildNode("IP", m_ipAddress, IP_ADDRESS);
    pAddPartyCont1Node->AddChildNode("IP_V6", m_ipAddress, IP_ADDRESS);
    pAddPartyCont1Node->AddChildNode("SIGNALING_PORT", m_callSignallingPort);
    pAddPartyCont1Node->AddChildNode("VIDEO_PROTOCOL", m_videoProtocol, VIDEO_PROTOCOL_ENUM);
    pAddPartyCont1Node->AddChildNode("VIDEO_BIT_RATE", m_videoRate, VIDEO_BIT_RATE_ENUM);

    CXMLDOMElement* pAliasNode = pAddPartyCont1Node->AddChildNode("ALIAS");
    if (m_netInterfaceType != SIP_INTERFACE_TYPE)
    {
      pAliasNode->AddChildNode("NAME", m_IpPartyAlias);
      pAliasNode->AddChildNode("ALIAS_TYPE", m_IpPartyAliasType, ALIAS_TYPE_ENUM);
    }

    pAddPartyCont1Node->AddChildNode("VOLUME", m_audioVolume);
    pAddPartyCont1Node->AddChildNode("INTERFACE", m_netInterfaceType, INTERFACE_ENUM);
    pAddPartyCont1Node->AddChildNode("PASSWORD", m_H243_password);
    pAddPartyCont1Node->AddChildNode("UNDEFINED", m_undefinedType, _BOOL);
    pAddPartyCont1Node->AddChildNode("NODE_TYPE", m_nodeType, NODE_TYPE_ENUM);
    pAddPartyCont1Node->AddChildNode("BONDING_PHONE", m_bondingPhoneNumber->phone_number);
    if (m_netInterfaceType == SIP_INTERFACE_TYPE)
    {
      pAddPartyCont1Node->AddChildNode("SIP_ADDRESS", m_sipPartyAddress);
      pAddPartyCont1Node->AddChildNode("SIP_ADDRESS_TYPE", m_sipPartyAddressType, SIP_ADDRESS_TYPE_ENUM);
    }
  }
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int COperAddPartyCont1::  DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "CHAIR", &m_chair, _BOOL);

  GET_VALIDATE_CHILD(pActionNode, "IP", &m_ipAddress, IP_ADDRESS);
  GET_VALIDATE_CHILD(pActionNode, "SIGNALING_PORT", &m_callSignallingPort, _0_TO_WORD);
  GET_VALIDATE_CHILD(pActionNode, "VIDEO_PROTOCOL", &m_videoProtocol, VIDEO_PROTOCOL_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "VIDEO_BIT_RATE", &m_videoRate, VIDEO_BIT_RATE_ENUM);

  CXMLDOMElement* pChild = NULL;

  GET_CHILD_NODE(pActionNode, "ALIAS", pChild);

  if (pChild)
  {
    GET_VALIDATE_CHILD(pChild, "NAME", m_IpPartyAlias, IP_STRING_LENGTH);
    GET_VALIDATE_CHILD(pChild, "ALIAS_TYPE", &m_IpPartyAliasType, ALIAS_TYPE_ENUM);
  }

  GET_VALIDATE_CHILD(pActionNode, "VOLUME", &m_audioVolume, _0_TO_10_DECIMAL);
  GET_VALIDATE_CHILD(pActionNode, "INTERFACE", &m_netInterfaceType, INTERFACE_ENUM);

  // ver.7 new fields
  GET_VALIDATE_CHILD(pActionNode, "PASSWORD", m_H243_password, _0_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "UNDEFINED", &m_undefinedType, _BOOL);
  GET_VALIDATE_CHILD(pActionNode, "NODE_TYPE", &m_nodeType, NODE_TYPE_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "BONDING_PHONE", m_bondingPhoneNumber->phone_number, PHONE_NUMBER_DIGITS_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "SIP_ADDRESS", m_sipPartyAddress, IP_STRING_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "SIP_ADDRESS_TYPE", &m_sipPartyAddressType, SIP_ADDRESS_TYPE_ENUM);

  return STATUS_OK;
}

//--------------------------------------------------------------------------
char* COperAddPartyCont1::TranslDwordToString(DWORD addr)
{
	BYTE* c = (BYTE*)&addr;
	char* IP_string = new char[21];
	snprintf(IP_string, 21, "%d.%d.%d.%d", *(c + 3), *(c + 2), *(c + 1), *c);
	return IP_string;
}

//--------------------------------------------------------------------------
void COperAddPartyCont1:: DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  WORD tmp;
  istr >> tmp;
  m_netInterfaceType = (BYTE)tmp;
  istr.ignore(1);
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_H243_password, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_H243_password, H243_NAME_LEN_OLD+1, ',');

  istr >> tmp;
  m_chair = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_videoProtocol = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_audioVolume = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_undefinedType = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_nodeType = (BYTE)tmp;
  istr.ignore(1);

  char buff[PHONE_NUMBER_DIGITS_LEN+1];
  istr.getline(buff, PHONE_NUMBER_DIGITS_LEN+1, ',');
  m_bondingPhoneNumber->phone_number = buff;

  istr >> m_videoRate;
  istr.ignore(1);
  istr >> m_ipAddress;
  istr.ignore(1);
  istr >> m_callSignallingPort;
  istr.ignore(1);

  if (m_netInterfaceType != SIP_INTERFACE_TYPE)
  {
    istr >> m_IpPartyAliasType;
    istr.ignore(1);
    istr.getline(m_IpPartyAlias, IP_STRING_LEN+1, ';');
  }
  else
  {
    istr >> m_sipPartyAddressType;
    istr.ignore(1);
    istr.getline(m_sipPartyAddress, IP_STRING_LEN+1, ';');
  }
}


////////////////////////////////////////////////////////////////////////////
//                        COperAddPartyCont2
////////////////////////////////////////////////////////////////////////////
void COperAddPartyCont2::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  switch (m_encrypted)
  {
    case NO  : { ostr << 0 << ","; break; }
    case YES : { ostr << 1 << ","; break; }
    case AUTO: { ostr << 2 << ","; break; }
    case 2   : { ostr << 2 << ","; break; }
  }

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_partyName << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_partyName);
    ostr << tmp << ",";
  }
  ostr << m_partyId << ";\n";
}

//--------------------------------------------------------------------------
void COperAddPartyCont2::Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum)
{
  switch (m_encrypted)
  {
    case NO  : { ostr << "Encryption: Off\n" ; break; }
    case YES : { ostr << "Encryption: On\n"  ; break; }
    case AUTO: { ostr << "Encryption: Auto\n"; break; }
    case 2   : { ostr << "Encryption: Auto\n"; break; }
  }

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_partyName << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_partyName);
    ostr << "party name:" << tmp << "\n";
  }
  ostr << "party ID:" << m_partyId << "\n\n";
}

//--------------------------------------------------------------------------
void COperAddPartyCont2::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  WORD tmp;
  istr >> tmp;
  m_encrypted = (BYTE)tmp;
  istr.ignore(1);

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_partyName, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_partyName, H243_NAME_LEN_OLD+1, ',');

  istr >> m_partyId;
  istr.ignore(1);
}

//--------------------------------------------------------------------------
void COperAddPartyCont2::SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType)
{
  CXMLDOMElement* pNode = NULL;

  switch (nEventType)
  {
    case RESERVED_PARTY_CONTINUE_2:
    {
      pNode = pFatherNode->AddChildNode("RESERVED_PARTY_2");
      break;
    }

    case OPERATOR_ADD_PARTY_CONTINUE_2:
    {
      pNode = pFatherNode->AddChildNode("OPERATOR_ADD_PARTY_2");
      break;
    }

    case OPERATOR_UPDATE_PARTY_CONTINUE_2:
    {
      pNode = pFatherNode->AddChildNode("OPERATOR_UPDATE_PARTY_CONTINUE_2");
      break;
    }

    case NEW_UNDEFINED_PARTY_CONTINUE_1:
    {
      pNode = pFatherNode->AddChildNode("NEW_UNDEFINED_PARTY_CONTINUE_1");
      break;
    }
  }

  pNode->AddChildNode("ENCRYPTION_EX", (NO == m_encrypted) ? "no" : ((YES == m_encrypted) ? "yes" : "auto")); // VNGR-8696
  pNode->AddChildNode("NAME", m_partyName);
  pNode->AddChildNode("ID", m_partyId);
}

//--------------------------------------------------------------------------
int COperAddPartyCont2::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;
  GET_VALIDATE_CHILD(pActionNode, "ENCRYPTION_EX", &m_encrypted, BOOL_AUTO_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "NAME", m_partyName, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_partyId, _0_TO_DWORD);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        COperMoveParty
////////////////////////////////////////////////////////////////////////////
void COperMoveParty::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  ostr << m_operator_name << ",";
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    ostr << tmp << ",";
  }

  ostr << m_partyId        << ",";
  ostr << m_dest_conf_name << ",";
  ostr << m_dest_conf_id   << ";\n";
}

//--------------------------------------------------------------------------
void COperMoveParty::Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum)
{
  ostr << "operator name:"<<m_operator_name << "\n";
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    ostr << "party name:" << tmp << "\n";
  }

  ostr << "party ID:"              << m_partyId        << "\n";
  ostr << "destination conf name:" << m_dest_conf_name << "\n";
  ostr << "destination conf ID:"   << m_dest_conf_id   << ";\n\n";
}


//--------------------------------------------------------------------------
void COperMoveParty::SerializeShort(WORD format, std::ostream& ostr, DWORD apiNum)
{
  ostr << m_operator_name << ",";
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    ostr << tmp << ",";
  }
  ostr << m_partyId << ";\n";
}

//--------------------------------------------------------------------------
void COperMoveParty::SerializeShort(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum)
{
  ostr << "operator name:" << m_operator_name << "\n";
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    ostr << "party name:" << tmp << "\n";
  }
  ostr << "party ID:" << m_partyId << ";\n\n";
}

//--------------------------------------------------------------------------
void COperMoveParty::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  istr.getline(m_operator_name, OPERATOR_NAME_LEN+1, ',');
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_partyId;
  istr.ignore(1);
  istr.getline(m_dest_conf_name, H243_NAME_LEN+1, ',');
  istr >> m_dest_conf_id;
  istr.ignore(1);
}

//--------------------------------------------------------------------------
void COperMoveParty::DeSerializeShort(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  istr.getline(m_operator_name, OPERATOR_NAME_LEN+1, ',');
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_partyId;
  istr.ignore(1);
}

//--------------------------------------------------------------------------
void COperMoveParty::SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType, BYTE isShortSerial)
{
  CXMLDOMElement* pOperMovePartyNode = NULL;

  switch (nEventType)
  {
    case OPERRATOR_ATTEND_PARTY:
      pOperMovePartyNode = pFatherNode->AddChildNode("OPERRATOR_ATTEND_PARTY");
      break;

    case OPERRATOR_MOVE_PARTY_FROM_CONFERENCE:
      pOperMovePartyNode = pFatherNode->AddChildNode("OPERRATOR_MOVE_PARTY_FROM_CONFERENCE");
      break;

    case OPERRATOR_BACK_TO_CONFERENCE_PARTY:
      pOperMovePartyNode = pFatherNode->AddChildNode("OPERRATOR_BACK_TO_CONFERENCE_PARTY");
      break;

    case OPERRATOR_ONHOLD_PARTY:
      pOperMovePartyNode = pFatherNode->AddChildNode("OPERRATOR_ONHOLD_PARTY");
      break;
  }

  if (pOperMovePartyNode)
  {
    pOperMovePartyNode->AddChildNode("OPERATOR_NAME", m_operator_name);
    pOperMovePartyNode->AddChildNode("NAME", m_party_name);
    pOperMovePartyNode->AddChildNode("ID", m_partyId);
    if (isShortSerial != YES)
    {
      pOperMovePartyNode->AddChildNode("DEST_CONF_NAME", m_dest_conf_name);
      pOperMovePartyNode->AddChildNode("DEST_CONF_ID", m_dest_conf_id);
    }
  }
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int COperMoveParty::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "OPERATOR_NAME", m_operator_name, _1_TO_OPERATOR_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "NAME", m_party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_partyId, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "DEST_CONF_NAME", m_dest_conf_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "DEST_CONF_ID", &m_dest_conf_id, _0_TO_DWORD);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventMoveToConf
////////////////////////////////////////////////////////////////////////////
ACCCDREventMoveToConf::ACCCDREventMoveToConf()
{
  memset(this, 0, sizeof(*this));
  m_pAddPartyDetailed = new ACCCDREventAddPartyDetailed;
}

//--------------------------------------------------------------------------
ACCCDREventMoveToConf::ACCCDREventMoveToConf(const ACCCDREventMoveToConf& other)
{
  memset(this, 0, sizeof(*this));
  *this = other;
}

//--------------------------------------------------------------------------
ACCCDREventMoveToConf::~ACCCDREventMoveToConf()
{
  PDELETE(m_pAddPartyDetailed);
}

//--------------------------------------------------------------------------
ACCCDREventMoveToConf& ACCCDREventMoveToConf::operator=(const ACCCDREventMoveToConf& other)
{
  PDELETE(m_pAddPartyDetailed);
  memcpy(this, &other, sizeof(*this));
  m_pAddPartyDetailed = new ACCCDREventAddPartyDetailed(*other.m_pAddPartyDetailed);
  return *this;
}


////////////////////////////////////////////////////////////////////////////
//                        COperMoveToConf
////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------
void COperMoveToConf::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  ostr << m_operator_name    << ",";
  ostr << m_source_conf_name << ",";
  ostr << m_source_conf_id   << ",";
  GetAddPartyDetail()->Serialize(format, ostr, apiNum);
}

//--------------------------------------------------------------------------
void COperMoveToConf::Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum)
{
  ostr << "operator name:"    << m_operator_name    << "\n";
  ostr << "source conf name:" << m_source_conf_name << "\n";
  ostr << "source conf ID:"   << m_source_conf_id   << "\n";
  GetAddPartyDetail()->Serialize(format, ostr, fullformat, apiNum);
}

//--------------------------------------------------------------------------
void COperMoveToConf::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  istr.getline(m_operator_name, OPERATOR_NAME_LEN+1, ',');
  istr.getline(m_source_conf_name, H243_NAME_LEN+1, ',');
  istr >> m_source_conf_id;
  istr.ignore(1);
  GetAddPartyDetail()->DeSerialize(format, istr, apiNum);
}

//--------------------------------------------------------------------------
void COperMoveToConf::SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType)
{
  CXMLDOMElement* pOperMoveToConfNode = NULL;

  switch (nEventType)
  {
    case OPERRATOR_ATTEND_PARTY_TO_CONFERENCE:
      pOperMoveToConfNode = pFatherNode->AddChildNode("OPERRATOR_ATTEND_PARTY_TO_CONFERENCE");
      break;

    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE:
      pOperMoveToConfNode = pFatherNode->AddChildNode("OPERRATOR_MOVE_PARTY_TO_CONFERENCE");
      break;
  }

  if (pOperMoveToConfNode)
  {
    pOperMoveToConfNode->AddChildNode("OPERATOR_NAME", m_operator_name);
    pOperMoveToConfNode->AddChildNode("NAME", m_source_conf_name);
    pOperMoveToConfNode->AddChildNode("ID", m_source_conf_id);
    GetAddPartyDetail()->SerializeXml(pOperMoveToConfNode, nEventType);
  }
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int COperMoveToConf::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "OPERATOR_NAME", m_operator_name, _1_TO_OPERATOR_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "NAME", m_source_conf_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_source_conf_id, _0_TO_DWORD);

  CXMLDOMElement* pChild = NULL;
  GET_CHILD_NODE(pActionNode, "PARTY_DETAILS", pChild);
  if (pChild)
  {
    nStatus = GetAddPartyDetail()->DeSerializeXml(pChild, pszError);
    if (nStatus)
      return nStatus;
  }

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        ACCCDREventAddPartyDetailed
////////////////////////////////////////////////////////////////////////////
ACCCDREventAddPartyDetailed::ACCCDREventAddPartyDetailed()
                            :ACCCDREventOperDelParty()
{
  memset(m_pPartyPhoneNumberList, 0, sizeof(ACCCdrPhone*)*MAX_CHANNEL_NUMBER);
  memset(m_pMcuPhoneNumberList  , 0, sizeof(ACCCdrPhone*)*MAX_CHANNEL_NUMBER);

  m_nodeType               = 0;
  m_connection_type        = 0;
  m_bonding_mode           = 0;
  m_net_num_of_channel     = 0;
  m_net_channel_width      = 0;
  m_net_service_name[0]    = '\0';
  m_restrict               = NO;
  m_voice                  = NO;
  m_num_type               = 0;
  m_net_subservice_name[0] = '\0';
  m_ident_method           = CALLED_PHONE_NUMBER_IDENTIFICATION_METHOD;
  m_meet_me_method         = MEET_ME_PER_USER; // in mcmsoper.h
  m_NumPartyPhoneNumber    = 0;
  m_NumMcuPhoneNumber      = 0;
  m_indpartyphone          = 0;
  m_indmcuphone            = 0;
  m_H243_password[0]       = '\0';
  m_chair                  = NO;
  m_netInterfaceType       = ISDN_INTERFACE_TYPE;
  m_ipAddress              = 0xFFFFFFFF;
  m_callSignallingPort     = 0xFFFF;
  m_videoProtocol          = AUTO;
  m_videoRate              = 0xFFFFFFFF;
  m_bondingPhoneNumber     = new ACCCdrPhone();
  m_IpPartyAliasType       = PARTY_H323_ALIAS_H323_ID_TYPE;
  m_IpPartyAlias[0]        = '\0';
  m_sipPartyAddressType    = PARTY_SIP_SIPURI_ID_TYPE;
  m_sipPartyAddress[0]     = '\0';
  m_audioVolume            = 5;
  m_undefinedType          = NO;
}

//--------------------------------------------------------------------------
ACCCDREventAddPartyDetailed::ACCCDREventAddPartyDetailed(const ACCCDREventAddPartyDetailed& other)
                            :ACCCDREventOperDelParty(*(const ACCCDREventOperDelParty*)&other)
{
  memset(m_pPartyPhoneNumberList, 0, sizeof(ACCCdrPhone*)*MAX_CHANNEL_NUMBER);
  memset(m_pMcuPhoneNumberList  , 0, sizeof(ACCCdrPhone*)*MAX_CHANNEL_NUMBER);
  *this = other;
}

//--------------------------------------------------------------------------
ACCCDREventAddPartyDetailed::~ACCCDREventAddPartyDetailed()
{
  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    PDELETE(m_pPartyPhoneNumberList[i]);
    PDELETE(m_pMcuPhoneNumberList[i]);
  }

  PDELETE(m_bondingPhoneNumber);
}

//--------------------------------------------------------------------------
ACCCDREventAddPartyDetailed& ACCCDREventAddPartyDetailed::operator=(const ACCCDREventAddPartyDetailed& other)
{
  if(this == &other){
	return *this;
  }

  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    PDELETE(m_pPartyPhoneNumberList[i]);
    if (other.m_pPartyPhoneNumberList[i])
      m_pPartyPhoneNumberList[i] = new ACCCdrPhone(*other.m_pPartyPhoneNumberList[i]);

    PDELETE(m_pMcuPhoneNumberList[i]);
    if (other.m_pMcuPhoneNumberList[i])
      m_pMcuPhoneNumberList[i] = new ACCCdrPhone(*other.m_pMcuPhoneNumberList[i]);
  }

  m_nodeType            = other.m_nodeType;
  m_connection_type     = other.m_connection_type;
  m_bonding_mode        = other.m_bonding_mode;
  m_net_num_of_channel  = other.m_net_num_of_channel;
  m_net_channel_width   = other.m_net_channel_width;
  m_restrict            = other.m_restrict;
  m_voice               = other.m_voice;
  m_num_type            = other.m_num_type;
  m_ident_method        = other.m_ident_method;
  m_meet_me_method      = other.m_meet_me_method; // in mcmsoper.h
  m_NumPartyPhoneNumber = other.m_NumPartyPhoneNumber;
  m_NumMcuPhoneNumber   = other.m_NumMcuPhoneNumber;
  m_indpartyphone       = other.m_indpartyphone;
  m_indmcuphone         = other.m_indmcuphone;
  m_chair               = other.m_chair;
  m_netInterfaceType    = other.m_netInterfaceType;
  m_ipAddress           = other.m_ipAddress;
  m_callSignallingPort  = other.m_callSignallingPort;
  m_videoProtocol       = other.m_videoProtocol;
  m_videoRate           = other.m_videoRate;
  m_bondingPhoneNumber  = new ACCCdrPhone(*other.m_bondingPhoneNumber);
  m_IpPartyAliasType    = other.m_IpPartyAliasType;
  m_sipPartyAddressType = other.m_sipPartyAddressType;
  m_audioVolume         = other.m_audioVolume;
  m_undefinedType       = other.m_undefinedType;

  SAFE_COPY(m_net_service_name   , other.m_net_service_name);
  SAFE_COPY(m_net_subservice_name, other.m_net_subservice_name);
  SAFE_COPY(m_H243_password      , other.m_H243_password);
  SAFE_COPY(m_IpPartyAlias       , other.m_IpPartyAlias);
  SAFE_COPY(m_sipPartyAddress    , other.m_sipPartyAddress);

  return *this;
}

//--------------------------------------------------------------------------
bool ACCCDREventAddPartyDetailed::operator ==(const ACCCDREventAddPartyDetailed& other)
{
  if (this == &other)
    return true;

  for (int i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    if ((other.m_pPartyPhoneNumberList[i] == NULL && m_pPartyPhoneNumberList[i] != NULL) ||
        (other.m_pPartyPhoneNumberList[i] != NULL && m_pPartyPhoneNumberList[i] == NULL))
    {
      return false;
    }

    if ((other.m_pMcuPhoneNumberList[i] == NULL && m_pMcuPhoneNumberList[i] != NULL) ||
        (other.m_pMcuPhoneNumberList[i] != NULL && m_pMcuPhoneNumberList[i] == NULL))
    {
      return false;
    }
  }

  if (m_nodeType            != other.m_nodeType)            return false;
  if (m_connection_type     != other.m_connection_type)     return false;
  if (m_bonding_mode        != other.m_bonding_mode)        return false;
  if (m_net_num_of_channel  != other.m_net_num_of_channel)  return false;
  if (m_net_channel_width   != other.m_net_channel_width)   return false;
  if (m_restrict            != other.m_restrict)            return false;
  if (m_voice               != other.m_voice)               return false;
  if (m_num_type            != other.m_num_type)            return false;
  if (m_ident_method        != other.m_ident_method)        return false;
  if (m_meet_me_method      != other.m_meet_me_method)      return false;
  if (m_NumPartyPhoneNumber != other.m_NumPartyPhoneNumber) return false;
  if (m_NumMcuPhoneNumber   != other.m_NumMcuPhoneNumber)   return false;
  if (m_indpartyphone       != other.m_indpartyphone)       return false;
  if (m_indmcuphone         != other.m_indmcuphone)         return false;
  if (m_chair               != other.m_chair)               return false;
  if (m_netInterfaceType    != other.m_netInterfaceType)    return false;
  if (m_ipAddress           != other.m_ipAddress)           return false;
  if (m_callSignallingPort  != other.m_callSignallingPort)  return false;
  if (m_videoProtocol       != other.m_videoProtocol)       return false;
  if (m_videoRate           != other.m_videoRate)           return false;
  if (m_IpPartyAliasType    != other.m_IpPartyAliasType)    return false;
  if (m_sipPartyAddressType != other.m_sipPartyAddressType) return false;
  if (m_audioVolume         != other.m_audioVolume)         return false;
  if (m_undefinedType       != other.m_undefinedType)       return false;

  if (0 != strncmp(m_net_service_name, other.m_net_service_name, NET_SERVICE_PROVIDER_NAME_LEN))
    return false;

  if (0 != strncmp(m_net_subservice_name, other.m_net_subservice_name, NET_SERVICE_PROVIDER_NAME_LEN))
    return false;

  if (0 != strncmp(m_H243_password, other.m_H243_password, H243_NAME_LEN))
    return false;

  if (0 != strncmp(m_IpPartyAlias, other.m_IpPartyAlias, IP_STRING_LEN))
    return false;

  if (0 != strncmp(m_sipPartyAddress, other.m_sipPartyAddress, IP_STRING_LEN))
    return false;

  return true;
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventAddPartyDetailed::GetFirstPartyPhoneNumber()
{
  m_indpartyphone = 1;
  return m_pPartyPhoneNumberList[0];
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventAddPartyDetailed::GetFirstPartyPhoneNumber(int& nPos)
{
  ACCCdrPhone* pPhone = ACCCDREventAddPartyDetailed::GetFirstPartyPhoneNumber();
  nPos = m_indpartyphone;
  return pPhone;
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventAddPartyDetailed::GetNextPartyPhoneNumber()
{
  if (m_indpartyphone >= m_NumPartyPhoneNumber)
    return m_pPartyPhoneNumberList[0];

  return m_pPartyPhoneNumberList[m_indpartyphone++];
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventAddPartyDetailed::GetNextPartyPhoneNumber(int& nPos)
{
  m_indpartyphone = nPos;
  ACCCdrPhone* pPhone = ACCCDREventAddPartyDetailed::GetNextPartyPhoneNumber();
  nPos = m_indpartyphone;
  return pPhone;
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventAddPartyDetailed::GetFirstMcuPhoneNumber()
{
  m_indmcuphone = 1;
  return m_pMcuPhoneNumberList[0];
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventAddPartyDetailed::GetFirstMcuPhoneNumber(int& nPos)
{
  ACCCdrPhone* pPhone = ACCCDREventAddPartyDetailed::GetFirstMcuPhoneNumber();
  nPos = m_indmcuphone;
  return pPhone;
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventAddPartyDetailed::GetNextMcuPhoneNumber()
{
  if (m_indmcuphone > m_NumMcuPhoneNumber)
    return NULL;
  return m_pMcuPhoneNumberList[m_indmcuphone++];
}

//--------------------------------------------------------------------------
ACCCdrPhone* ACCCDREventAddPartyDetailed::GetNextMcuPhoneNumber(int& nPos)
{
  m_indmcuphone = nPos;
  ACCCdrPhone* pPhone = ACCCDREventAddPartyDetailed::GetNextMcuPhoneNumber();
  nPos = m_indmcuphone;
  return pPhone;
}

//--------------------------------------------------------------------------
int ACCCDREventAddPartyDetailed::FindMcuPhoneNumber(const char* mcuphoneNumber)
{
  for (int i = 0; i < (int)m_NumMcuPhoneNumber; i++)
    if (m_pMcuPhoneNumberList[i] && m_pMcuPhoneNumberList[i]->phone_number == mcuphoneNumber)
        return i;
  return NOT_FIND;
}

//--------------------------------------------------------------------------
int ACCCDREventAddPartyDetailed::FindPartyPhoneNumber(const char* partyphoneNumber)
{
  for (int i = 0; i < (int)m_NumPartyPhoneNumber; i++)
    if (m_pPartyPhoneNumberList[i] && m_pPartyPhoneNumberList[i]->phone_number == partyphoneNumber)
        return i;
  return NOT_FIND;
}


////////////////////////////////////////////////////////////////////////////
//                        CAddPartyDetailed
////////////////////////////////////////////////////////////////////////////
CAddPartyDetailed::CAddPartyDetailed(const CAddPartyDetailed& other)
                  :CPObject(other), ACCCDREventAddPartyDetailed(other)
{
}

//--------------------------------------------------------------------------
void CAddPartyDetailed::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    ostr << tmp << ",";
  }

  ostr << m_partyId << ",";
  ostr << (WORD)m_connection_type << ",";
  ostr << (WORD)m_bonding_mode  << ",";
  ostr << (WORD)m_net_num_of_channel  << ",";
  ostr << (WORD)m_net_channel_width  << ",";
  ostr << m_net_service_name  << ",";
  ostr << (WORD)m_restrict  << ",";
  ostr << (WORD)m_voice  << ",";
  ostr << (WORD)m_num_type  << ",";
  ostr << m_net_subservice_name << ",";
  ostr << (WORD)m_NumPartyPhoneNumber << ",";
  ostr << (WORD)m_NumMcuPhoneNumber << ",";

  for (int i = 0; i < (int)m_NumPartyPhoneNumber; i++)
    ostr << m_pPartyPhoneNumberList[i]->phone_number << ",";

  for (int i = 0; i < (int)m_NumMcuPhoneNumber; i++)
    ostr << m_pMcuPhoneNumberList[i]->phone_number << ",";

  ostr << (WORD)m_ident_method  << ",";
  ostr << (WORD)m_meet_me_method  << ",";
  ostr << (WORD)m_netInterfaceType  << ",";

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_H243_password  << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_H243_password);
    ostr << tmp  << ",";
  }

  ostr << (WORD)m_chair  << ",";
  ostr << (WORD)m_videoProtocol  << ",";
  ostr << (WORD)m_audioVolume << ",";
  ostr << (WORD)m_undefinedType << ",";
  ostr << (WORD)m_nodeType  << ",";
  ostr << m_bondingPhoneNumber->phone_number << ",";
  ostr << m_videoRate  << ",";
  ostr << m_ipAddress  << ",";
  ostr << m_callSignallingPort  << ",";

  // DON'T CHANGE EVENT FILEDS ORDER because it breaks backward compatibility of CDR and causes asserts like VNGR-5974/7157
  if (m_netInterfaceType != SIP_INTERFACE_TYPE)
  {
    ostr << m_IpPartyAliasType << ",";
    ostr << m_IpPartyAlias << ";\n";
  }
  else
  {
    ostr << m_sipPartyAddressType << ",";
    ostr << m_sipPartyAddress << ";\n";
  }
}

//--------------------------------------------------------------------------
void CAddPartyDetailed::Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_party_name);
    ostr << "party name:" << tmp << "\n";
  }

  ostr << "party ID:" << m_partyId << "\n";
  switch (m_connection_type)
  {
    case TYPE_DIAL_OUT   : { ostr << "connection type :dial out\n"; break; }
    case TYPE_DIAL_IN    : { ostr << "connection type :dial in\n" ; break; }
    case TYPE_DIRECT     : { ostr << "connection type :direct\n"  ; break; }
    default              : { ostr << "--\n"                       ; break; }
  }

  switch (m_bonding_mode)
  {
    case NO_BONDING      : { ostr << "no bonding\n"  ; break; }
    case BONDING         : { ostr << "bonding\n"     ; break; }
    case AUTO_BONDING    : { ostr << "auto bonding\n"; break; }
    default              : { ostr << "--\n"          ; break; }
  }

  switch (m_net_num_of_channel)
  {
    case AUTO_NUMBER     : { ostr << "channel number auto,\n"; break; }
    default              : { ostr << "number of channel:" << (WORD)m_net_num_of_channel << ",\n"; break; }
  }

  switch (m_net_channel_width)
  {
    case Xfer_64         : { ostr << "net channel width:1B\n"       ; break; }
    case Xfer_2x64       : { ostr << "net channel width:2B\n"       ; break; }
    case Xfer_3x64       : { ostr << "net channel width:3B\n"       ; break; }
    case Xfer_4x64       : { ostr << "net channel width:4B\n"       ; break; }
    case Xfer_5x64       : { ostr << "net channel width:5B\n"       ; break; }
    case Xfer_6x64       : { ostr << "net channel width:6B\n"       ; break; }
    case Xfer_96         : { ostr << "net channel width:96 kbps\n"  ; break; }
    case Xfer_128        : { ostr << "net channel width:128 kbps\n" ; break; }
    case Xfer_192        : { ostr << "net channel width:192 kbps\n" ; break; }
    case Xfer_256        : { ostr << "net channel width:256 kbps\n" ; break; }
    case Xfer_320        : { ostr << "net channel width:320 kbps\n" ; break; }
    case Xfer_384        : { ostr << "net channel width:384 kbps\n" ; break; }
    case Xfer_512        : { ostr << "net channel width:512 kbps\n" ; break; }
    case Xfer_768        : { ostr << "net channel width:768 kbps\n" ; break; }
    case Xfer_832        : { ostr << "net channel width:832 kbps\n" ; break; }
    case Xfer_1152       : { ostr << "net channel width:1152 kbps\n"; break; }
    case Xfer_1472       : { ostr << "net channel width:1472 kbps\n"; break; }
    case Xfer_1536       : { ostr << "net channel width:1536 kbps\n"; break; }
    case Xfer_1728       : { ostr << "net channel width:1728 kbps\n"; break; }
    case Xfer_1920       : { ostr << "net channel width:1920 kbps\n"; break; }
    case Xfer_2048       : { ostr << "net channel width:2048 kbps\n"; break; }
    case Xfer_1024       : { ostr << "net channel width:1024 kbps\n"; break; }
    case Xfer_4096       : { ostr << "net channel width:4096 kbps\n"; break; }
    case Xfer_6144       : { ostr << "net channel width:6144 kbps\n"; break; }
    case Xfer_2560       : { ostr << "net channel width:2560 kbps\n"; break; }
    case Xfer_3072       : { ostr << "net channel width:3072 kbps\n"; break; }
    case Xfer_3584       : { ostr << "net channel width:3584 kbps\n"; break; }
    case Xfer_8192       : { ostr << "net channel width:8192 kbps\n"; break; }
    default              : { ostr << "--\n"                         ; break; }
  }

  ostr << "net service name:"<<m_net_service_name  << "\n";
  switch (m_restrict)
  {
    case RESTRICT        : { ostr << "restrict mode\n"; break; }
    case NON_RESTRICT    : { ostr << "non restrict\n" ; break; }
    case UNKNOWN_RESTRICT: { ostr << "restrict:auto\n"; break; }
    default              : { ostr << "--\n"           ; break; }
  }

  switch (m_voice)
  {
    case VOICE           : { ostr << "voice mode:YES\n"    ; break; }
    case NO_VOICE        : { ostr << "voice mode:NO\n"     ; break; }
    case UNKNOWN_VOICE   : { ostr << "voice mode unknown\n"; break; }
    default              : { ostr << "--\n"                ; break; }
  }

  switch (m_num_type)
  {
    case UNKNOWN_TYPE    : { ostr << "number type:unknown\n"           ; break; }
    case INTERNATIONAL   : { ostr << "number type:international\n"     ; break; }
    case NATIONAL        : { ostr << "number type:national\n"          ; break; }
    case NETWORK_SPECIFIC: { ostr << "number type:network specific\n"  ; break; }
    case SUBSCRIBER      : { ostr << "number type:subscriber\n"        ; break; }
    case ABBREVIATED     : { ostr << "number type:abbreviated\n"       ; break; }
    case NUM_TYPE_DEF    : { ostr << "number type:taken from service\n"; break; }
    default              : { ostr << "--\n"                            ; break; }
  }

  ostr << "net subservice name:" << m_net_subservice_name << "\n";

  if (m_NumPartyPhoneNumber > 0)
  {
    ostr << "party phone number:";
    for (int i = 0; i < (int)m_NumPartyPhoneNumber; i++)
      ostr << m_pPartyPhoneNumberList[i]->phone_number << ",";
    ostr << "\n";
  }

  if (m_NumMcuPhoneNumber > 0)
  {
    ostr << "MCU phone number:";
    for (int j = 0; j < m_NumMcuPhoneNumber; j++)
      ostr <<m_pMcuPhoneNumberList[j]->phone_number << ",";
    ostr <<"\n";
  }

  switch (m_ident_method)
  {
    case PASSWORD       : { ostr << "ident. method:password\n"      ; break; }
    case CALLED_NUMBER  : { ostr << "ident. method:called number\n" ; break; }
    case CALLING_NUMBER : { ostr << "ident. method:calling number\n"; break; }
    default             : { ostr << "--\n"                          ; break; }
  }

  switch (m_meet_me_method)
  {
    case MEET_PER_MCU       : { ostr << "meet method: per MCU\n"       ; break; }
    case MEET_PER_CONFERENCE: { ostr << "meet method: per conference\n"; break; }
    case MEET_PER_PARTY     : { ostr << "meet method: per party\n"     ; break; }
    case MEET_PER_CHANNEL   : { ostr << "meet method: per channel\n"   ; break; }
    default                 : { ostr << "--\n"                         ; break; }
  }

  switch (m_netInterfaceType)
  {
    case ISDN_INTERFACE_TYPE : { ostr << "net interface type:ISDN\n" ; break; }
    case ATM_INTERFACE_TYPE  : { ostr << "net interface type:ATM\n"  ; break; }
    case H323_INTERFACE_TYPE : { ostr << "net interface type:323\n"  ; break; }
    case V35_INTERFACE_TYPE  : { ostr << "net interface type:MPI8\n" ; break; }
    case T1CAS_INTERFACE_TYPE: { ostr << "net interface type:T1CAS\n"; break; }
    case SIP_INTERFACE_TYPE  : { ostr << "net interface type:SIP\n"  ; break; }
    default                  : { ostr << "--\n"                      ; break; }
  }

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "H243 password:"<<m_H243_password << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_H243_password);
    ostr << "H243 password:" << tmp << "\n";
  }

  switch (m_chair)
  {
    case 0 : { ostr << "chair:NO\n" ; break; }
    case 1 : { ostr << "chair:YES\n"; break; }
    default: { ostr << "--\n"       ; break; }
  }

  switch (m_videoProtocol)
  {
    case 1   : { ostr << "video protocol:H261 only\n"; break; }
    case 2   : { ostr << "video protocol:H263\n"     ; break; }
    case 0xff: { ostr << "video protocol:auto\n"     ; break; }
    default  : { ostr << "--\n"                      ; break; }
  }

  ostr << "audio volume:" << (WORD)m_audioVolume << "\n";

  switch ((WORD)m_undefinedType)
  {
    case 1 : { ostr << "undefined type:undefined party\n" ; break; }
    case 2 : { ostr << "undefined type:unreserved party\n"; break; }
    case 0 : { ostr << "undefined type:default\n"         ; break; }
    default: { ostr << "--\n"                             ; break; }
  }

  switch ((WORD)m_nodeType)
  {
    case 0 : { ostr << "node type:MCU\n"     ; break; }
    case 1 : { ostr << "node type:Terminal\n"; break; }
    default: { ostr << "--\n"                ; break; }
  }

  ostr << "bonding phone number:" << m_bondingPhoneNumber->phone_number << "\n";

  if (m_videoRate != 0xFFFFFFFF)
    ostr << "video rate:" << m_videoRate << "\n";
  else
    ostr << "video rate:auto" <<"\n";

  if (m_ipAddress != 0xFFFFFFFF)
  {
    char* tmp = TranslDwordToString(m_ipAddress);
    ostr << "Ip address:"<< tmp  << "\n";
    PDELETEA(tmp);
  }
  else
    ostr << "Ip address:" << "\n";

  ostr << "call signalling port:" << m_callSignallingPort << "\n";

  if (m_netInterfaceType == SIP_INTERFACE_TYPE)
  {
    switch (m_sipPartyAddressType)
    {
      case PARTY_SIP_SIPURI_ID_TYPE: { ostr << "SIP party address type:SIP URI type\n"    ; break; }
      case PARTY_SIP_TELURL_ID_TYPE: { ostr << "SIP party address type:SIP TEL URL type\n"; break; }
      default                      : { ostr << "--\n"                                     ; break; }
    }
    ostr << "SIP party address: "<< m_sipPartyAddressType << ";\n\n";
  }

  if (m_netInterfaceType == H323_INTERFACE_TYPE)
  {
    switch (m_IpPartyAliasType)
    {
      case PARTY_H323_ALIAS_H323_ID_TYPE     : { ostr << "H323 party alias type:alias H323 type\n"        ; break; }
      case PARTY_H323_ALIAS_E164_TYPE        : { ostr << "H323 party alias type:alias E164 type\n"        ; break; }
      case PARTY_H323_ALIAS_URL_ID_TYPE      : { ostr << "H323 party alias type:alias URL ID type\n"      ; break; }
      case PARTY_H323_ALIAS_TRANSPORT_ID_TYPE: { ostr << "H323 party alias type:alias transport ID type\n"; break; }
      case PARTY_H323_ALIAS_EMAIL_ID_TYPE    : { ostr << "H323 party alias type:alias email ID type\n"    ; break; }
      case PARTY_H323_ALIAS_PARTY_NUMBER_TYPE: { ostr << "H323 party alias type:alias party number type\n"; break; }
      default                                : { ostr << "--\n"                                           ; break; }
    }
    ostr << "H323 party alias: " << m_IpPartyAlias << ";\n\n";
  }
}

//--------------------------------------------------------------------------
void CAddPartyDetailed::SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType)
{
  CXMLDOMElement* pAddPartyDetailedNode = NULL;

  switch (nEventType)
  {
    case EVENT_NEW_UNDEFINED_PARTY:
      pAddPartyDetailedNode = pFatherNode->AddChildNode("NEW_ISDN_UNDEFINED_PARTY");
      break;

    case OPERRATOR_ATTEND_PARTY_TO_CONFERENCE:
    case OPERRATOR_MOVE_PARTY_TO_CONFERENCE:
      pAddPartyDetailedNode = pFatherNode->AddChildNode("PARTY_DETAILS");
      break;
  }

  if (pAddPartyDetailedNode)
  {
    // old
    pAddPartyDetailedNode->AddChildNode("CONNECTION", m_connection_type, CONNECTION_ENUM);
    pAddPartyDetailedNode->AddChildNode("BONDING", m_bonding_mode, BONDING_ENUM);
    pAddPartyDetailedNode->AddChildNode("SERVICE_NAME", m_net_service_name);
    pAddPartyDetailedNode->AddChildNode("SUB_SERVICE_NAME", m_net_subservice_name);
    pAddPartyDetailedNode->AddChildNode("CALL_CONTENT", m_voice, CALL_CONTENT_ENUM);
    pAddPartyDetailedNode->AddChildNode("NUM_TYPE", m_num_type, NUM_TYPE_ENUM);
    pAddPartyDetailedNode->AddChildNode("MEET_ME_METHOD", m_meet_me_method, MEET_ME_METHOD_ENUM);
    pAddPartyDetailedNode->AddChildNode("TRANSFER_RATE", m_net_channel_width, TRANSFER_RATE_ENUM);

    //VNGR-25242, map '0' to 'AUTO'
    BYTE  ucNetChannelNumber  = AUTO;
    if(0 != m_net_num_of_channel)
     {
         ucNetChannelNumber = m_net_num_of_channel;
     }
    pAddPartyDetailedNode->AddChildNode("NET_CHANNEL_NUMBER", ucNetChannelNumber, NET_CHANNEL_NUMBER_ENUM);

    pAddPartyDetailedNode->AddChildNode("CHAIR", m_chair, _BOOL);
    pAddPartyDetailedNode->AddChildNode("IP", m_ipAddress, IP_ADDRESS);
    pAddPartyDetailedNode->AddChildNode("IP_V6", m_ipAddress, IP_ADDRESS);
    pAddPartyDetailedNode->AddChildNode("SIGNALING_PORT", m_callSignallingPort);
    pAddPartyDetailedNode->AddChildNode("VIDEO_PROTOCOL", m_videoProtocol, VIDEO_PROTOCOL_ENUM);
    pAddPartyDetailedNode->AddChildNode("VIDEO_BIT_RATE", m_videoRate, VIDEO_BIT_RATE_ENUM);
    CXMLDOMElement* pAliasNode = pAddPartyDetailedNode->AddChildNode("ALIAS");

    if (m_netInterfaceType != SIP_INTERFACE_TYPE)
    {
      pAliasNode->AddChildNode("NAME", m_IpPartyAlias);
      pAliasNode->AddChildNode("ALIAS_TYPE", m_IpPartyAliasType, ALIAS_TYPE_ENUM);
    }

    pAddPartyDetailedNode->AddChildNode("VOLUME", m_audioVolume);
    pAddPartyDetailedNode->AddChildNode("INTERFACE", m_netInterfaceType, INTERFACE_ENUM);
    pAddPartyDetailedNode->AddChildNode("IDENTIFICATION_METHOD", m_ident_method, IDENT_METHOD_ENUM);
    pAddPartyDetailedNode->AddChildNode("PASSWORD", m_H243_password);
    pAddPartyDetailedNode->AddChildNode("UNDEFINED", m_undefinedType, _BOOL);
    pAddPartyDetailedNode->AddChildNode("NODE_TYPE", m_nodeType, NODE_TYPE_ENUM);
    pAddPartyDetailedNode->AddChildNode("BONDING_PHONE", m_bondingPhoneNumber->phone_number);
    if (m_netInterfaceType == SIP_INTERFACE_TYPE)
    {
      pAddPartyDetailedNode->AddChildNode("SIP_ADDRESS", m_sipPartyAddress);
      pAddPartyDetailedNode->AddChildNode("SIP_ADDRESS_TYPE", m_sipPartyAddressType, SIP_ADDRESS_TYPE_ENUM);
    }

    CXMLDOMElement* pPartyPhoneListNode = pAddPartyDetailedNode->AddChildNode("PHONE_LIST");
    CXMLDOMElement* pMCUPhoneListNode   = pAddPartyDetailedNode->AddChildNode("MCU_PHONE_LIST");

    char szPhoneNodeName[10];

    for (int i = 0; i < 6; i++)
    {
      snprintf(szPhoneNodeName, sizeof(szPhoneNodeName), "PHONE%d", i+1);

      if (i < m_NumPartyPhoneNumber)
        pPartyPhoneListNode->AddChildNode(szPhoneNodeName, m_pPartyPhoneNumberList[i]->phone_number);

      if (i < m_NumMcuPhoneNumber)
        pMCUPhoneListNode->AddChildNode(szPhoneNodeName, m_pMcuPhoneNumberList[i]->phone_number);
    }

    // new fields ver.7
    pAddPartyDetailedNode->AddChildNode("NAME", m_party_name);
    pAddPartyDetailedNode->AddChildNode("ID", m_partyId);
  }
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CAddPartyDetailed::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  // old
  GET_VALIDATE_CHILD(pActionNode, "CONNECTION", &m_connection_type, CONNECTION_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "BONDING", &m_bonding_mode, BONDING_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "SERVICE_NAME", m_net_service_name, NET_SERVICE_PROVIDER_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "SUB_SERVICE_NAME", m_net_subservice_name, NET_SERVICE_PROVIDER_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "RESTRICT_MODE", &m_restrict, RESTRICT_MODE_ENUM); // "RESTRICT_MODE" -> "RESTRICT_MODE_ENUM"
  GET_VALIDATE_CHILD(pActionNode, "NUM_TYPE", &m_num_type, NUM_TYPE_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "MEET_ME_METHOD", &m_meet_me_method, MEET_ME_METHOD_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "TRANSFER_RATE", &m_net_channel_width, TRANSFER_RATE_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "NET_CHANNEL_NUMBER", &m_net_num_of_channel, NET_CHANNEL_NUMBER_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "CHAIR", &m_chair, _BOOL);
  GET_VALIDATE_CHILD(pActionNode, "IP", &m_ipAddress, IP_ADDRESS);
  GET_VALIDATE_CHILD(pActionNode, "SIGNALING_PORT", &m_callSignallingPort, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "VIDEO_PROTOCOL", &m_videoProtocol, VIDEO_PROTOCOL_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "VIDEO_BIT_RATE", &m_videoRate, VIDEO_BIT_RATE_ENUM);

  CXMLDOMElement* pChild = NULL;
  GET_CHILD_NODE(pActionNode, "ALIAS", pChild);
  if (pChild)
  {
    GET_VALIDATE_CHILD(pActionNode, "NAME", m_IpPartyAlias, IP_STRING_LENGTH);
    GET_VALIDATE_CHILD(pActionNode, "ALIAS_TYPE", &m_IpPartyAliasType, ALIAS_TYPE_ENUM);
  }

  GET_VALIDATE_CHILD(pActionNode, "VOLUME", &m_audioVolume, _1_TO_10_DECIMAL);
  GET_VALIDATE_CHILD(pActionNode, "INTERFACE", &m_netInterfaceType, INTERFACE_ENUM);

  // new fields ver.7
  GET_VALIDATE_CHILD(pActionNode, "IDENTIFICATION_METHOD", &m_ident_method, IDENT_METHOD_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "PASSWORD", m_H243_password, _0_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "UNDEFINED", &m_undefinedType, _BOOL);
  GET_VALIDATE_CHILD(pActionNode, "NODE_TYPE", &m_nodeType, NODE_TYPE_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "BONDING_PHONE", m_bondingPhoneNumber->phone_number, PHONE_NUMBER_DIGITS_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "SIP_ADDRESS", m_sipPartyAddress, IP_STRING_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "SIP_ADDRESS_TYPE", &m_sipPartyAddressType, SIP_ADDRESS_TYPE_ENUM);

  char szPhoneNodeName[10];
  CXMLDOMElement* pPhoneList = NULL;

  m_NumPartyPhoneNumber = m_NumMcuPhoneNumber = 0;
  int i;
  for (i = 0; i < MAX_CHANNEL_NUMBER; i++)
  {
    PDELETEA(m_pPartyPhoneNumberList[i]);
    PDELETEA(m_pMcuPhoneNumberList[i]);
  }

  GET_CHILD_NODE(pActionNode, "PHONE_LIST", pPhoneList);
  if (pPhoneList)
  {
    for (i = 0; i < 6; i++)
    {
      snprintf(szPhoneNodeName, sizeof(szPhoneNodeName), "PHONE%d", i+1);

      GET_CHILD_NODE(pPhoneList, szPhoneNodeName, pChild);
      if (pChild)
      {
        m_pPartyPhoneNumberList[i] = new ACCCdrPhone;
        GET_VALIDATE_CHILD(pPhoneList, szPhoneNodeName, m_pPartyPhoneNumberList[i]->phone_number, PHONE_NUMBER_DIGITS_LENGTH);
        m_NumPartyPhoneNumber++;
      }
      else
        break;
    }
  }

  GET_CHILD_NODE(pActionNode, "MCU_PHONE_LIST", pPhoneList);
  if (pPhoneList)
  {
    for (i = 0; i < 6; i++)
    {
      snprintf(szPhoneNodeName, sizeof(szPhoneNodeName), "PHONE%d", i+1);

      GET_CHILD_NODE(pPhoneList, szPhoneNodeName, pChild);
      if (pChild)
      {
        m_pMcuPhoneNumberList[i] = new ACCCdrPhone;
        GET_VALIDATE_CHILD(pPhoneList, szPhoneNodeName, m_pMcuPhoneNumberList[i]->phone_number, PHONE_NUMBER_DIGITS_LENGTH);
        m_NumMcuPhoneNumber++;
      }
      else
        break;
    }
  }

  // new fields ver.7
  GET_VALIDATE_CHILD(pActionNode, "NAME", m_party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_partyId, _0_TO_DWORD);

  return STATUS_OK;
}

//--------------------------------------------------------------------------
char* CAddPartyDetailed::TranslDwordToString(DWORD addr)
{
	BYTE* c = (BYTE*)&addr;
	char* IP_string = new char[21];
	snprintf(IP_string, 21, "%d.%d.%d.%d", *(c + 3), *(c + 2), *(c + 1), *c);
	return IP_string;
}

//--------------------------------------------------------------------------
void CAddPartyDetailed::DeSerialize(WORD format, char* msg_info, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS
  CIstrStream* pIstr;
  pIstr = new CIstrStream(msg_info);
  DeSerialize(format, *pIstr, apiNum);
  PDELETE(pIstr);
}

//--------------------------------------------------------------------------
void CAddPartyDetailed::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  WORD tmp;

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_partyId;
  istr.ignore(1);
  istr >> tmp;
  m_connection_type = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_bonding_mode = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_net_num_of_channel = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_net_channel_width = (BYTE)tmp;
  istr.ignore(1);
  istr.getline(m_net_service_name, NET_SERVICE_PROVIDER_NAME_LEN+1, ',');
  istr >> tmp;
  m_restrict = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_voice = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_num_type = (BYTE)tmp;
  istr.ignore(1);
  istr.getline(m_net_subservice_name, NET_SERVICE_PROVIDER_NAME_LEN+1, ',');

  istr >> tmp;
  m_NumPartyPhoneNumber = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_NumMcuPhoneNumber = (BYTE)tmp;

  istr.ignore(1);
  int  i;
  char buff[PHONE_NUMBER_DIGITS_LEN+1];
  for (i = 0; i < (int)m_NumPartyPhoneNumber; i++)
  {
    m_pPartyPhoneNumberList[i] = new ACCCdrPhone;
    istr.getline(buff, PHONE_NUMBER_DIGITS_LEN+1, ',');
    m_pPartyPhoneNumberList[i]->phone_number = buff;
  }

  for (i = 0; i < (int)m_NumMcuPhoneNumber; i++)
  {
    m_pMcuPhoneNumberList[i] = new ACCCdrPhone;
    istr.getline(buff, PHONE_NUMBER_DIGITS_LEN+1, ',');
    m_pMcuPhoneNumberList[i]->phone_number = buff;
  }

  istr >> tmp;
  m_ident_method = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_meet_me_method = (BYTE)tmp;
  istr.ignore(1);
  // //////////////////////////////////////
  istr >> tmp;
  m_netInterfaceType = (BYTE)tmp;
  istr.ignore(1);
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_H243_password, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_H243_password, H243_NAME_LEN_OLD+1, ',');

  istr >> tmp;
  m_chair = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_videoProtocol = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_audioVolume = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_undefinedType = (BYTE)tmp;
  istr.ignore(1);
  istr >> tmp;
  m_nodeType = (BYTE)tmp;
  istr.ignore(1);

  istr.getline(buff, PHONE_NUMBER_DIGITS_LEN+1, ',');
  m_bondingPhoneNumber->phone_number = buff;

  istr >> m_videoRate;
  istr.ignore(1);
  istr >> m_ipAddress;
  istr.ignore(1);
  istr >> m_callSignallingPort;
  istr.ignore(1);
  if (m_netInterfaceType != SIP_INTERFACE_TYPE)
  {
    istr >> m_IpPartyAliasType;
    istr.ignore(1);
    istr.getline(m_IpPartyAlias, IP_STRING_LEN+1, ';');
  }
  else
  {
    istr >> m_sipPartyAddressType;
    istr.ignore(1);
    istr.getline(m_sipPartyAddress, IP_STRING_LEN+1, ';');
  }
}

//--------------------------------------------------------------------------
void CAddPartyDetailed::AddPartyPhoneNumber(const char* partyphoneNumber)
{
  m_pPartyPhoneNumberList[m_NumPartyPhoneNumber] = new ACCCdrPhone;
  m_pPartyPhoneNumberList[m_NumPartyPhoneNumber]->phone_number = partyphoneNumber;
  m_NumPartyPhoneNumber++;
}

//--------------------------------------------------------------------------
void CAddPartyDetailed::AddMcuPhoneNumber(const char* mcuphoneNumber)
{
  m_pMcuPhoneNumberList[m_NumMcuPhoneNumber] = new ACCCdrPhone;
  m_pMcuPhoneNumberList[m_NumMcuPhoneNumber]->phone_number = mcuphoneNumber;
  m_NumMcuPhoneNumber++;
}

//--------------------------------------------------------------------------
int CAddPartyDetailed::CancelMcuPhoneNumber(const char* mcuphoneNumber)
{
  if(m_NumMcuPhoneNumber > MAX_CHANNEL_NUMBER)
    return STATUS_MAX_PHONE_NUMBER_EXCEEDED;

  int ind = FindMcuPhoneNumber(mcuphoneNumber);
  if (ind == NOT_FIND || ind >= MAX_CHANNEL_NUMBER)
    return STATUS_PHONE_NUMBER_NOT_EXISTS;

  PDELETE(m_pMcuPhoneNumberList[ind]);

  int i;
  for (i = 0; i < (int)m_NumMcuPhoneNumber && i < MAX_CHANNEL_NUMBER; i++)
  {
    if (m_pMcuPhoneNumberList[i] == NULL)
      break;
  }

  for (int j = i; j < (int)m_NumMcuPhoneNumber-1; j++)
  {
    m_pMcuPhoneNumberList[j] = m_pMcuPhoneNumberList[j+1];
  }

  m_pMcuPhoneNumberList[m_NumMcuPhoneNumber-1] = NULL;
  m_NumMcuPhoneNumber--;

  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CAddPartyDetailed::CancelPartyPhoneNumber(const char* partyphoneNumber)
{
  int ind = FindPartyPhoneNumber(partyphoneNumber);
  if (ind == NOT_FIND || ind >= MAX_CHANNEL_NUMBER)
    return STATUS_PHONE_NUMBER_NOT_EXISTS;

  if(m_NumPartyPhoneNumber > MAX_CHANNEL_NUMBER)
    return STATUS_MAX_PHONE_NUMBER_EXCEEDED;

  PDELETE(m_pPartyPhoneNumberList[ind]);
  int i;
  for (i = 0; i < (int)m_NumPartyPhoneNumber; i++)
  {
    if (m_pPartyPhoneNumberList[i] == NULL)
      break;
  }

  for (int j = i; j < (int)m_NumPartyPhoneNumber-1; j++)
  {
    m_pPartyPhoneNumberList[j] = m_pPartyPhoneNumberList[j+1];
  }

  m_pPartyPhoneNumberList[m_NumPartyPhoneNumber-1] = NULL;
  m_NumPartyPhoneNumber--;

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CPartySetVisualName
////////////////////////////////////////////////////////////////////////////
void CPartySetVisualName::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_h243party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }
  ostr << m_party_Id    << ",";
  ostr << m_visual_name << ";\n";
}

//--------------------------------------------------------------------------
void CPartySetVisualName::Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_h243party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:"<< tmp << "\n";
  }

  ostr << "party ID:"    << m_party_Id    << "\n";
  ostr << "Visual name:" << m_visual_name << ";\n\n";
}

//--------------------------------------------------------------------------
void CPartySetVisualName::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_h243party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_h243party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_party_Id;
  istr.ignore(1);
  istr.getline(m_visual_name, H243_NAME_LEN+1, ';');
}

//--------------------------------------------------------------------------
void CPartySetVisualName::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pSetVisualNameNode = pFatherNode->AddChildNode("VISUAL_NAME_CHANGED");

  pSetVisualNameNode->AddChildNode("NAME", m_h243party_name);
  pSetVisualNameNode->AddChildNode("ID", m_party_Id);
  pSetVisualNameNode->AddChildNode("VISUAL_NAME", m_visual_name);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CPartySetVisualName::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "NAME", m_h243party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_party_Id, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "VISUAL_NAME", m_visual_name, _1_TO_H243_NAME_LENGTH);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CTokenUser
////////////////////////////////////////////////////////////////////////////
void CCDRPartyCalling_NumMoveToCont1::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_h243party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }
  ostr << m_party_Id        << ",";
  ostr << m_partyCallingNum << ";\n";
}

//--------------------------------------------------------------------------
void CCDRPartyCalling_NumMoveToCont1::Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_h243party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }
  ostr << "party ID:"             << m_party_Id        << "\n";
  ostr << "party calling number:" << m_partyCallingNum << ";\n\n";
}

//--------------------------------------------------------------------------
void CCDRPartyCalling_NumMoveToCont1::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_h243party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_h243party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_party_Id;
  istr.ignore(1);
  istr.getline(m_partyCallingNum, PHONE_NUMBER_DIGITS_LEN+1, ';');
}

//--------------------------------------------------------------------------
void CCDRPartyCalling_NumMoveToCont1::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pMoveToConf1Node = pFatherNode->AddChildNode("OPERRATOR_MOVE_PARTY_TO_CONFERENCE_1");

  pMoveToConf1Node->AddChildNode("NAME", m_h243party_name);
  pMoveToConf1Node->AddChildNode("ID", m_party_Id);
  pMoveToConf1Node->AddChildNode("PHONE1", m_partyCallingNum);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCDRPartyCalling_NumMoveToCont1::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "NAME", m_h243party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_party_Id, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "PHONE1", m_partyCallingNum, _1_TO_H243_NAME_LENGTH);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CUpdateUserDefinedInfo
////////////////////////////////////////////////////////////////////////////
void CUpdateUserDefinedInfo::Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum)
{
  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
    ostr << "user defined "<< i+1 << ":" << m_UserDefinedInfo[i] << "\n";

  if (m_isVip)
    ostr << "Is Vip: Yes" << "\n";
  else
    ostr << "Is Vip: No" << "\n";

  ostr << "\n";
}

//--------------------------------------------------------------------------
void CUpdateUserDefinedInfo::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
    ostr << m_UserDefinedInfo[i] << ",";

  ostr << (WORD)m_isVip << ";\n";
}

//--------------------------------------------------------------------------
void CUpdateUserDefinedInfo::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
    istr.getline(m_UserDefinedInfo[i], 100+1, ',');

  WORD tmp;
  istr >> tmp;
  m_isVip = (BYTE)tmp;
  istr.ignore(1);
}

//--------------------------------------------------------------------------
void CUpdateUserDefinedInfo::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pUserInfoNode = pFatherNode->AddChildNode("USER_DEFINED_INFORMATION");

  char szNodeName[20];
  strcpy(szNodeName, "CONTACT_INFO");

  CXMLDOMElement* pChild = pUserInfoNode->AddChildNode("CONTACT_INFO_LIST");
  for (int i = 0; i < MAX_USER_INFO_ITEMS; i++)
  {
    if (strlen(m_UserDefinedInfo[i]))
    {
      if (i != 0)
        snprintf(szNodeName, sizeof(szNodeName), "%s_%d", szNodeName, i+1);

      pChild->AddChildNode(szNodeName, m_UserDefinedInfo[i]);
      strcpy(szNodeName, "CONTACT_INFO");
    }
  }

  pUserInfoNode->AddChildNode("VIP", m_isVip, _BOOL);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CUpdateUserDefinedInfo::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  // cleanup
  memset(m_UserDefinedInfo, 0, sizeof(m_UserDefinedInfo));

  CXMLDOMElement* pNodeList = NULL;
  CXMLDOMElement* pChild    = NULL;
  char            szNodeName[20];

  GET_MANDATORY_CHILD_NODE(pActionNode, "CONTACT_INFO_LIST", pNodeList);

  strcpy(szNodeName, "CONTACT_INFO");

  GET_FIRST_CHILD_NODE(pNodeList, szNodeName, pChild);

  int i = 0;
  while (i < MAX_USER_INFO_ITEMS)
  {
    if (pChild)
    {
      GET_VALIDATE(pChild, m_UserDefinedInfo[i], _0_TO_USER_INFO_ITEM_LENGTH);
    }
    else
      m_UserDefinedInfo[i][0] = '\0';

    i++;
    snprintf(szNodeName, sizeof(szNodeName), "%s_%d", szNodeName, i+1);
    GET_NEXT_CHILD_NODE(pNodeList, szNodeName, pChild);
    strcpy(szNodeName, "CONTACT_INFO");
  }

  GET_VALIDATE_CHILD(pActionNode, "VIP", &m_isVip, _BOOL);
  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyCalled_NumMoveToCont2
////////////////////////////////////////////////////////////////////////////
void CCDRPartyCalled_NumMoveToCont2::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_h243party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }
  ostr << m_party_Id       << ",";
  ostr << m_partyCalledNum << ";\n";
}

//--------------------------------------------------------------------------
void CCDRPartyCalled_NumMoveToCont2::Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_h243party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }
  ostr << "party ID:"            << m_party_Id       << "\n";
  ostr << "party Dialed number:" << m_partyCalledNum << ";\n\n";
}

//--------------------------------------------------------------------------
void CCDRPartyCalled_NumMoveToCont2::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_h243party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_h243party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_party_Id;
  istr.ignore(1);
  istr.getline(m_partyCalledNum, PHONE_NUMBER_DIGITS_LEN+1, ';');
}

//--------------------------------------------------------------------------
void CCDRPartyCalled_NumMoveToCont2::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pMoveToConf1Node = pFatherNode->AddChildNode("OPERATOR_MOVE_PARTY_TO_CONFERENCE_2");

  pMoveToConf1Node->AddChildNode("NAME", m_h243party_name);
  pMoveToConf1Node->AddChildNode("ID", m_party_Id);
  pMoveToConf1Node->AddChildNode("PHONE1", m_partyCalledNum);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCDRPartyCalled_NumMoveToCont2::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "NAME", m_h243party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_party_Id, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "PHONE1", m_partyCalledNum, _1_TO_H243_NAME_LENGTH);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyDTMFfailureIndication
////////////////////////////////////////////////////////////////////////////
void CCDRPartyDTMFfailureIndication::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_h243party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }

  BOOL isHidePsw = NO;
  CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("HIDE_CONFERENCE_PASSWORD", isHidePsw);

  ostr << m_party_Id << ",";
  ostr << (isHidePsw ? "" : m_DTMFcode) << ",";
  ostr << (isHidePsw ? "" : m_RightData) << ",";
  ostr << m_FailureState   <<  ";\n";
}

//--------------------------------------------------------------------------
void CCDRPartyDTMFfailureIndication::Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_h243party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:"<< tmp << "\n";
  }

  ostr << "party ID:"        << m_party_Id  << "\n";
  ostr << "wrong dtmf code:" << m_DTMFcode  << "\n";
  ostr << "right data:"      << m_RightData << "\n";

  switch (m_FailureState)
  {
    case IVR_FEATURE_CONF_PASSWORD        : { ostr << "failure state: CONFERENCE PASSWORD FAILURE;\n\n"  ; break; }
    case IVR_FEATURE_CONF_LEADER          : { ostr << "failure state: CHAIRPERSON PASSWORD FAILURE;\n\n" ; break; }
    case IVR_FEATURE_NUMERIC_CONFERENCE_ID: { ostr << "failure state: NUMERIC CONFERENCE ID FAILURE;\n\n"; break; }
    default                               : { ostr<<  "failure state: Unknown;\n\n"                      ; break; }
  }
}

//--------------------------------------------------------------------------
void CCDRPartyDTMFfailureIndication::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_h243party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_h243party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_party_Id;
  istr.ignore(1);
  istr.getline(m_DTMFcode, MAX_DTMF_STRING_LENGTH+1, ',');
  istr.getline(m_RightData, MAX_DTMF_STRING_LENGTH+1, ',');
  istr >> m_FailureState;
  istr.ignore(1); // ---------cdr
}

//--------------------------------------------------------------------------
void CCDRPartyDTMFfailureIndication::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pDtmfNode = pFatherNode->AddChildNode("DTMF_CODE_FAILURE");

  pDtmfNode->AddChildNode("NAME", m_h243party_name);
  pDtmfNode->AddChildNode("ID", m_party_Id);
  pDtmfNode->AddChildNode("DTMF_CODE", m_DTMFcode);
  pDtmfNode->AddChildNode("RIGHT_DATA", m_RightData);
  pDtmfNode->AddChildNode("FAILURE_STATE", m_FailureState, DTMF_FAILURE_ENUM);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCDRPartyDTMFfailureIndication::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "NAME", m_h243party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_party_Id, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "DTMF_CODE", m_DTMFcode, _0_TO_31_STRING_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "RIGHT_DATA", m_RightData, _0_TO_31_STRING_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "FAILURE_STATE", &m_FailureState, DTMF_FAILURE_ENUM);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyRecording
////////////////////////////////////////////////////////////////////////////
void CCDRPartyRecording::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_h243party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }

  ostr << m_party_Id          << ",";
  ostr << m_opType            << ",";
  ostr << m_opInitiator       << ",";
  ostr << m_recordingLinkName << ",";
  ostr << m_recordingLinkId   << ",";
  ostr << m_recordingPolicy   << ";\n";
}

//--------------------------------------------------------------------------
void CCDRPartyRecording::Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_h243party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }

  ostr << "party ID:"               << m_party_Id           << "\n";
  ostr << "Op Type :"               << m_opType             << "\n";
  ostr << "Initiator:"              << m_opInitiator        << "\n";
  ostr << "Recording Link Name:"    << m_recordingLinkName  << "\n";
  ostr << "Recording Link ID:"      << m_recordingLinkId    << "\n";
  ostr << "Start Recording Policy:" << m_recordingPolicy    << ";\n\n";
}

//--------------------------------------------------------------------------
void CCDRPartyRecording::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_h243party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_h243party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_party_Id;
  istr.ignore(1);

  istr >> m_opType;
  istr.ignore(1);

  istr >> m_opInitiator;
  istr.ignore(1);

  istr.getline(m_recordingLinkName, H243_NAME_LEN+1, ',');

  istr >> m_recordingLinkId;
  istr.ignore(1);

  istr >> m_recordingPolicy;
  istr.ignore(1);
}

//--------------------------------------------------------------------------
void CCDRPartyRecording::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pNode = pFatherNode->AddChildNode("RECORDING_LINK");

  pNode->AddChildNode("NAME", m_h243party_name);
  pNode->AddChildNode("PARTY_ID", m_party_Id);
  pNode->AddChildNode("START_REC_POLICY", m_recordingPolicy, START_REC_POLICY_ENUM);
  pNode->AddChildNode("REC_LINK_ID", m_recordingLinkId);
  pNode->AddChildNode("REC_LINK_NAME", m_recordingLinkName);
  pNode->AddChildNode("RECORDING_STATUS", m_opType, RECORDING_STATUS_ENUM);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCDRPartyRecording::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "NAME", m_h243party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "PARTY_ID", &m_party_Id, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "START_REC_POLICY", &m_recordingPolicy, START_REC_POLICY_ENUM);
  GET_VALIDATE_CHILD(pActionNode, "REC_LINK_ID", &m_recordingLinkId, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "REC_LINK_NAME", m_recordingLinkName, _0_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "RECORDING_STATUS", &m_opType, RECORDING_STATUS_ENUM);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartySystemRecording
////////////////////////////////////////////////////////////////////////////
void CCDRPartySystemRecording::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << m_h243party_name << ",";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }

  ostr << m_party_Id    << ",";
  ostr << m_opType      << ",";
  ostr << m_partyType   << ",";
  ostr << m_userAccount << ",";
  ostr << m_failed      << ",";
  ostr << m_failedCode  << ";\n";
}

//--------------------------------------------------------------------------
void CCDRPartySystemRecording::Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum)
{
  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    ostr << "party name:" << m_h243party_name << "\n";
  else
  {
    char tmp[H243_NAME_LEN_OLD];
    SAFE_COPY(tmp, m_h243party_name);
    ostr << "party name:" << tmp << "\n";
  }

  ostr << "party ID:"     <<  m_party_Id    << "\n";
  ostr << "Op Type :"     <<  m_opType      << "\n";
  ostr << "Party Type:"   <<  m_partyType   << "\n";
  ostr << "User Account:" <<  m_userAccount << "\n";
  ostr << "Failure:"      <<  m_failed      << "\n";
  ostr << "Failure Code:" <<  m_failedCode  << ";\n\n";
}

//--------------------------------------------------------------------------
void CCDRPartySystemRecording::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  if (apiNum >= API_NUM_LONG_NAMES || format != OPERATOR_MCMS)
    istr.getline(m_h243party_name, H243_NAME_LEN+1, ',');
  else
    istr.getline(m_h243party_name, H243_NAME_LEN_OLD+1, ',');

  istr >> m_party_Id;
  istr.ignore(1);

  istr >> m_opType;
  istr.ignore(1);

  istr >> m_partyType;
  istr.ignore(1);

  istr.getline(m_userAccount, MAX_USER_ACCOUNT_LEN+1, ',');

  istr >> m_failed;
  istr.ignore(1);

  istr >> m_failedCode;
  istr.ignore(1);
}

//--------------------------------------------------------------------------
void CCDRPartySystemRecording::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pNode = pFatherNode->AddChildNode("RECORDING_LINK");

  pNode->AddChildNode("NAME", m_h243party_name);
  pNode->AddChildNode("PARTY_ID", m_party_Id);
  pNode->AddChildNode("RECORDING_STATUS", m_opType, RECORDING_STATUS_ENUM);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCDRPartySystemRecording::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "NAME", m_h243party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "PARTY_ID", &m_party_Id, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "RECORDING_STATUS", &m_opType, RECORDING_STATUS_ENUM);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CCDRSipPrivateExtensions
////////////////////////////////////////////////////////////////////////////
void CCDRSipPrivateExtensions::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  ostr << m_party_name        << ",";
  ostr << m_partyId           << ",";
  ostr << m_calledPartyId     << ",";
  ostr << m_assertedIdentity  << ",";
  ostr << m_chargingVector    << ",";
  ostr << m_preferredIdentity << ";\n";
}

//--------------------------------------------------------------------------
void CCDRSipPrivateExtensions::Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum)
{
  ostr << "party name:"         << m_party_name        << "\n";
  ostr << "party ID:"           << m_partyId           << "\n";
  ostr << "Called Party ID:"    << m_calledPartyId     << "\n";
  ostr << "Asserted Identity:"  << m_assertedIdentity  << "\n";
  ostr << "Charging Vector:"    << m_chargingVector    << "\n";
  ostr << "Preferred Identity:" << m_preferredIdentity << "\n\n";
}

//--------------------------------------------------------------------------
void CCDRSipPrivateExtensions::DeSerialize(WORD format, std::istream& istr, DWORD apiNum)
{
  // assuming format = OPERATOR_MCMS

  istr.getline(m_party_name, H243_NAME_LEN+1, ',');
  istr >> m_partyId;
  istr.ignore(1);
  istr.getline(m_calledPartyId, MAX_SIP_PRIVATE_EXT_LEN+1, ',');
  istr.getline(m_assertedIdentity, MAX_SIP_PRIVATE_EXT_LEN+1, ',');
  istr.getline(m_chargingVector, MAX_SIP_PRIVATE_EXT_LEN+1, ',');
  istr.getline(m_preferredIdentity, MAX_SIP_PRIVATE_EXT_LEN+1, ';');
}

//--------------------------------------------------------------------------
void CCDRSipPrivateExtensions::SerializeXml(CXMLDOMElement* pFatherNode)
{
  CXMLDOMElement* pNode = pFatherNode->AddChildNode("SIP_PRIVATE_EXTENSIONS");

  pNode->AddChildNode("NAME", m_party_name);
  pNode->AddChildNode("ID", m_partyId);
  pNode->AddChildNode("P_CALLED_PARTY_ID", m_calledPartyId);
  pNode->AddChildNode("P_ASSERTED_IDENTITY", m_assertedIdentity);
  pNode->AddChildNode("P_CHARGING_VECTOR", m_chargingVector);
  pNode->AddChildNode("P_PREFERRED_IDENTITY", m_preferredIdentity);
}

//--------------------------------------------------------------------------
// schema file name:  obj_cdr_full.xsd
int CCDRSipPrivateExtensions::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "NAME", m_party_name, _1_TO_H243_NAME_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "ID", &m_partyId, _0_TO_DWORD);
  GET_VALIDATE_CHILD(pActionNode, "P_CALLED_PARTY_ID", m_calledPartyId, _0_TO_MAX_SIP_PRIVATE_EXT_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "P_ASSERTED_IDENTITY", m_assertedIdentity, _0_TO_MAX_SIP_PRIVATE_EXT_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "P_CHARGING_VECTOR", m_chargingVector, _0_TO_MAX_SIP_PRIVATE_EXT_LENGTH);
  GET_VALIDATE_CHILD(pActionNode, "P_PREFERRED_IDENTITY", m_preferredIdentity, _0_TO_MAX_SIP_PRIVATE_EXT_LENGTH);

  return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        COperIpV6PartyCont1
////////////////////////////////////////////////////////////////////////////
void COperIpV6PartyCont1::Serialize(WORD format, std::ostream& ostr, DWORD apiNum)
{
  ostr << m_ipV6Address << ";\n";
}

//--------------------------------------------------------------------------
void COperIpV6PartyCont1::DeSerialize(WORD format, std::istream& m_istr, DWORD apiNum)
{
  m_istr.getline(m_ipV6Address, 64, ';');
}

//--------------------------------------------------------------------------
void COperIpV6PartyCont1::SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType)
{
  CXMLDOMElement* pOperIpV6PartyCont1Node = NULL;

  switch (nEventType)
  {
    case USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      pOperIpV6PartyCont1Node = pFatherNode->AddChildNode("USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS");
      break;
    }

    case USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      pOperIpV6PartyCont1Node = pFatherNode->AddChildNode("USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS");
      break;
    }

    case EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS:
    {
      pOperIpV6PartyCont1Node = pFatherNode->AddChildNode("NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS");
      break;
    }

    case RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS:
    {
      pOperIpV6PartyCont1Node = pFatherNode->AddChildNode("RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS");
      break;
    }

    default:
    {
      PTRACE2INT(eLevelError, "COperIpV6PartyCont1::SerializeXml unknown opcode. opcode num is, ", nEventType);
      break;
    }
  }
  pOperIpV6PartyCont1Node->AddChildNode("IP_V6", m_ipV6Address);
}

//--------------------------------------------------------------------------
int COperIpV6PartyCont1::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError)
{
  int nStatus = STATUS_OK;

  GET_VALIDATE_CHILD(pActionNode, "IP_V6", m_ipV6Address, _0_TO_H243_NAME_LENGTH);
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************/
/*							class CConfCorrelationData					 */
/*************************************************************************/

////////////////////////////////////////////////////////////////////////////
CConfCorrelationData::CConfCorrelationData()
{
}

////////////////////////////////////////////////////////////////////////////
CConfCorrelationData::~CConfCorrelationData()
{
}

////////////////////////////////////////////////////////////////////////////
CConfCorrelationData& CConfCorrelationData::operator=(const CConfCorrelationData &other)
{
	if(&other != this)
	{
		CCDRConfCorrelationDataInfo::operator =(other);
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////
bool CConfCorrelationData::operator == (const CConfCorrelationData &rHnd)
{
	if(&rHnd == this)
	{
		return true;
	}
	return CCDRConfCorrelationDataInfo::operator ==(rHnd);
}

////////////////////////////////////////////////////////////////////////////
void CConfCorrelationData::Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum)
{
	if (m_conf_uuid.length() > 0 && m_conf_uuid[m_conf_uuid.length()-1] == ';')
	{
		m_ostr << m_conf_uuid << "\n";
	}
	else
	{
		m_ostr << m_conf_uuid << ";\n";
	}
}

////////////////////////////////////////////////////////////////////////////
void CConfCorrelationData::DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum)
{
	m_istr >> m_conf_uuid;
	m_istr.ignore(1);

}

////////////////////////////////////////////////////////////////////////////
void CConfCorrelationData::SerializeXml(CXMLDOMElement* pFatherNode)
{

	FPTRACE(eLevelInfoNormal,"CConfCorrelationData::SerializeXml -   ");
	CXMLDOMElement* pCorrelationData = NULL;

	pCorrelationData = pFatherNode->AddChildNode("CONF_CORRELATION_DATA");

	if (NULL == pCorrelationData)
	{
		return;
	}

	pCorrelationData->AddChildNode("CONF_UUID",m_conf_uuid);

}

////////////////////////////////////////////////////////////////////////////
int  CConfCorrelationData::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{


	PASSERTMSG(TRUE, "CConfCorrelationData::DeSerializeXml - Should not be called");

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
void  CConfCorrelationData::SetSigUuid(const std::string  sigUuid)
{
	m_conf_uuid = sigUuid;
}
/////////////////////////////////////////////////////////////////////////////
const char*  CConfCorrelationData::NameOf() const
{
	return "CConfCorrelationData";
}

