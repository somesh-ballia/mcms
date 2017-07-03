
#include "OperatorConfInfo.h"


//==================================================================================================================================//
// constructors
//==================================================================================================================================//
COperatorConfInfo::COperatorConfInfo():
m_operator_conf_monitoring_id(0xFFFFFFFF),
m_operator_conf_name(""),
m_operator_party_monitoring_id(0xFFFFFFFF),
m_operator_party_name("")
{
}

//==================================================================================================================================//
COperatorConfInfo::~COperatorConfInfo()
{
}
//==================================================================================================================================//
const char* COperatorConfInfo::NameOf()const 
{
  return "COperatorConfInfo";
}
//==================================================================================================================================//
bool operator== (const COperatorConfInfo& op1, const COperatorConfInfo& op2)
{
  if (strncmp(op1.m_operator_conf_name.c_str(), op2.m_operator_conf_name.c_str(), H243_NAME_LEN) == 0)
    if (strncmp(op1.m_operator_party_name.c_str(), op2.m_operator_party_name.c_str(), H243_NAME_LEN) == 0)
    return true;
  return false;
}
//==================================================================================================================================//
// public API Serialize/DeSerialize
//==================================================================================================================================//
void COperatorConfInfo::Serialize(WORD format, ostream &ostr)
{
  if (format != NATIVE) PASSERT(1);

  ostr << m_operator_conf_monitoring_id << "\n";
  ostr << m_operator_party_monitoring_id << "\n";


  
  char tmp_name[H243_NAME_LEN];
  memset(tmp_name,'\0',H243_NAME_LEN);
  strncpy(tmp_name,m_operator_conf_name.c_str(),H243_NAME_LEN-1);
  ostr << tmp_name << "\n"; 

  memset(tmp_name,'\0',H243_NAME_LEN);
  strncpy(tmp_name,m_operator_party_name.c_str(),H243_NAME_LEN-1);
  ostr << tmp_name << "\n";

 PTRACE(eLevelInfoNormal,"COperatorConfInfo::Serialize:"); 
 Dump();
}
//==================================================================================================================================//
void COperatorConfInfo::DeSerialize(WORD format,istream &istr)
{
  if (format != NATIVE) PASSERT(1);

  DWORD tmp = 0;
  istr >> tmp;
  m_operator_conf_monitoring_id = tmp;
  // istr >> does not extract '\n'
  istr.ignore(1);

  tmp = 0;
  istr >> tmp;
  m_operator_party_monitoring_id = tmp;
  // istr >> does not extract '\n'
  istr.ignore(1);

  char tmp_name[H243_NAME_LEN];
  memset(tmp_name,'\0',H243_NAME_LEN);  
  istr.getline(tmp_name,H243_NAME_LEN);
  tmp_name[H243_NAME_LEN-1] = '\0';
  m_operator_conf_name = tmp_name;
  // istream getline extract the '\n' delimiter but not stores it in the buffer - no need for istr.ignore(1)

  memset(tmp_name,'\0',H243_NAME_LEN);
  istr.getline(tmp_name,H243_NAME_LEN);
  m_operator_party_name = tmp_name;

 PTRACE(eLevelInfoNormal,"COperatorConfInfo::DeSerialize:"); 
 Dump();
}
//==================================================================================================================================//
void COperatorConfInfo::Serialize(WORD format,CSegment& seg)
{
  if (format != NATIVE) PASSERT(1);

  COstrStream pOstr;
  Serialize(format, pOstr);
  pOstr.Serialize(seg);
}
//==================================================================================================================================//
void COperatorConfInfo::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1);

    CIstrStream istr (seg);
    DeSerialize(format, istr);
}
//==================================================================================================================================//


//==================================================================================================================================//
// public API
//==================================================================================================================================//
void COperatorConfInfo::SetOperatorConfName(const char* operatorConfName)
{
  m_operator_conf_name = operatorConfName;
}
//==================================================================================================================================//
void COperatorConfInfo::SetOperatorPartyName(const char* operatorPartyName)
{
  m_operator_party_name = operatorPartyName;
}
//==================================================================================================================================//
void COperatorConfInfo::SetOperatorConfMonitoringId(DWORD confId)
{
  m_operator_conf_monitoring_id = confId;
}
//==================================================================================================================================//
void COperatorConfInfo::SetOperatorPartyMonitoringId(DWORD partyId)
{
  m_operator_party_monitoring_id = partyId;
}
//==================================================================================================================================//
const char* COperatorConfInfo::GetOperatorConfName()
{
  return m_operator_conf_name.c_str();
}
//==================================================================================================================================//
const char* COperatorConfInfo::GetOperatorPartyName()
{
  return m_operator_party_name.c_str();
}
//==================================================================================================================================//
DWORD COperatorConfInfo::GetOperatorConfMonitoringId()
{
  return m_operator_conf_monitoring_id;
}
//==================================================================================================================================//
DWORD COperatorConfInfo::GetOperatorPartyMonitoringId()
{
  return m_operator_party_monitoring_id;
}
//==================================================================================================================================//


//==================================================================================================================================//
// public API - dump
//==================================================================================================================================//
void COperatorConfInfo::Dump()const
{
  COstrStream msg;
  Dump(msg);
  PTRACE2(eLevelInfoNormal,"COperatorConfInfo::Dump:\n",(char*)msg.str().c_str());

}
//==================================================================================================================================//
void COperatorConfInfo::Dump(COstrStream& msg)const
{
    msg << "m_operator_conf_name            = " << m_operator_conf_name.c_str() << "\n";
    msg << "m_operator_conf_monitoring_id   = " << m_operator_conf_monitoring_id << "\n";
    msg << "m_operator_party_name           = " << m_operator_party_name.c_str() << "\n";
    msg << "m_operator_party_monitoring_id  = " << m_operator_party_monitoring_id << "\n";
}
//==================================================================================================================================//
