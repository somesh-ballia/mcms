
#include "MoveInfo.h"
#include "CommResApi.h"


//==================================================================================================================================//
// constructors
//==================================================================================================================================//
CMoveConfDetails::CMoveConfDetails():m_conf_monitoring_id(0xFFFFFFFF),m_conf_name(""),m_conf_type(eMoveConfType_None)
{
}
//==================================================================================================================================//
CMoveConfDetails::CMoveConfDetails(DWORD conf_monitoring_id, const char* conf_name, eMoveConfType conf_type):m_conf_monitoring_id(conf_monitoring_id),m_conf_name(conf_name),m_conf_type(conf_type)
{
}
//==================================================================================================================================//
CMoveConfDetails::CMoveConfDetails(CCommResApi* pConf)
{
  m_conf_monitoring_id = pConf->GetMonitorConfId();
  m_conf_name = pConf->GetName();

  // set move info
  m_conf_type = eMoveConfType_Regular;
  if(pConf->GetEntryQ()){
    m_conf_type = eMoveConfType_Eq;
  }else if(pConf->GetOperatorConf()){
    m_conf_type = eMoveConfType_Operator;
  }
}
//==================================================================================================================================//
CMoveConfDetails::~CMoveConfDetails()
{
}
//==================================================================================================================================//
const char* CMoveConfDetails::NameOf()const 
{
  return "CMoveConfDetails";
}
//==================================================================================================================================//
void CMoveConfDetails::Serialize(WORD format, ostream &ostr)
{
  if (format != NATIVE) PASSERT(1);

  //  m_conf_monitoring_id = 77;
  //  m_conf_type = eMoveConfType_Operator;


  ostr << m_conf_monitoring_id << "\n";
  ostr << (DWORD)m_conf_type << "\n";

  char tmp_name[H243_NAME_LEN];
  memset(tmp_name,'\0',H243_NAME_LEN);
  strncpy(tmp_name,m_conf_name.c_str(),H243_NAME_LEN-1);
  ostr << tmp_name << "\n"; 

 
//  PTRACE(eLevelInfoNormal,"CMoveConfDetails::Serialize:\n"); 
//  Dump();
  
}
//==================================================================================================================================//
void CMoveConfDetails::DeSerialize(WORD format,istream &istr)
{
  if (format != NATIVE) PASSERT(1);

  DWORD tmp = 0;
  istr >> tmp;
  m_conf_monitoring_id = tmp;
  istr.ignore(1);

  tmp = 0;
  istr >> tmp;
  m_conf_type  = (eMoveConfType)tmp;
  istr.ignore(1);

  char tmp_name[H243_NAME_LEN];
  memset(tmp_name,'\0',H243_NAME_LEN);
  istr.getline(tmp_name,H243_NAME_LEN);
  m_conf_name = tmp_name;

//  PTRACE(eLevelInfoNormal,"CMoveConfDetails::DeSerialize:\n"); 
//  Dump();


}
//==================================================================================================================================//
void CMoveConfDetails::Serialize(WORD format,CSegment& seg)
{
  if (format != NATIVE) PASSERT(1);

  COstrStream pOstr;
  Serialize(format, pOstr);
  pOstr.Serialize(seg);
}
//==================================================================================================================================//
void CMoveConfDetails::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1);

    CIstrStream istr (seg);
    DeSerialize(format, istr);
}
//==================================================================================================================================//
//==================================================================================================================================//
// public API - dump
//==================================================================================================================================//
void CMoveConfDetails::Dump()const
{
  COstrStream msg;
  Dump(msg);
  PTRACE2(eLevelInfoNormal,"CMoveConfDetails::Dump:\n",(char*)msg.str().c_str());

}
//==================================================================================================================================//
void CMoveConfDetails::Dump(COstrStream& msg)const
{
  msg << "m_conf_name          = " << m_conf_name.c_str() << "\n";
  msg << "m_conf_monitoring_id = " << m_conf_monitoring_id << "\n";
  const char* name1 = MoveConfTypeStr(m_conf_type);
  msg << "m_conf_type          = " << name1 << "\n";
}
//==================================================================================================================================//

const char* CMoveConfDetails::MoveConfTypeStr(eMoveConfType eType)const
{
  switch(eType){
  case eMoveConfType_None:
    return "eMoveConfType_None";
  case eMoveConfType_Regular:
    return "eMoveConfType_Regular";
  case eMoveConfType_Eq:
    return "eMoveConfType_Eq";
  case eMoveConfType_Operator:
    return "eMoveConfType_Operator";
  default:
    return "Unknown eMoveConfType";
      }
  };
//==================================================================================================================================//
DWORD CMoveConfDetails::GetId()const
{
  return m_conf_monitoring_id;
}
//==================================================================================================================================//
eMoveConfType CMoveConfDetails::GetConfType()const
{
  return m_conf_type;
}
//==================================================================================================================================//
const char* CMoveConfDetails::GetName()const
{
  return m_conf_name.c_str();
}
//==================================================================================================================================//



//==================================================================================================================================//
// class CMoveInfo
//==================================================================================================================================//

//==================================================================================================================================//
// constructors
//==================================================================================================================================//
CMoveInfo::CMoveInfo():
m_is_valid_home_conf(0),
m_is_operator_party(0),
m_home_conf(0xFFFFFFFF,"",eMoveConfType_None),
m_current_conf(0xFFFFFFFF,"",eMoveConfType_None),
m_previous_conf(0xFFFFFFFF,"",eMoveConfType_None)
{
}

//==================================================================================================================================//
CMoveInfo::CMoveInfo(CMoveConfDetails first_conf,
		     BYTE is_operator_party):
m_is_valid_home_conf(0),
m_is_operator_party(is_operator_party),
m_home_conf(0xFFFFFFFF,"",eMoveConfType_None),
m_current_conf(first_conf),
m_previous_conf(0xFFFFFFFF,"",eMoveConfType_None)
{
  eMoveConfType FirstConfType =  first_conf.GetConfType();
  if( (is_operator_party && FirstConfType==eMoveConfType_Operator) || FirstConfType == eMoveConfType_Regular){
    m_home_conf = first_conf;
    m_is_valid_home_conf = 1;
  }
}
//==================================================================================================================================//
CMoveInfo::~CMoveInfo()
{
}
//==================================================================================================================================//
const char* CMoveInfo::NameOf()const 
{
  return "CMoveInfo";
}
//==================================================================================================================================//
void CMoveInfo::Create(CMoveConfDetails& first_conf,BYTE is_operator_party)
{
  m_is_operator_party = is_operator_party;
  m_current_conf = first_conf;
  eMoveConfType FirstConfType =  first_conf.GetConfType();
  if( (is_operator_party && FirstConfType==eMoveConfType_Operator) || FirstConfType == eMoveConfType_Regular){
    m_home_conf = first_conf;
    m_is_valid_home_conf = 1;
  }
  PTRACE(eLevelInfoNormal,"CMoveInfo::Create");
  Dump();
  
}
//==================================================================================================================================//


//==================================================================================================================================//
// public API Serialize/DeSerialize
//==================================================================================================================================//
void CMoveInfo::Serialize(WORD format, ostream &ostr)
{
  if (format != NATIVE) PASSERT(1);

  //  m_is_valid_home_conf = 1;
  //  m_is_operator_party = 2;


  ostr << (WORD)m_is_valid_home_conf << "\n";
  ostr << (WORD)m_is_operator_party << "\n";
  m_home_conf.Serialize(format,ostr);
  m_current_conf.Serialize(format,ostr);
  m_previous_conf.Serialize(format,ostr);

//   PTRACE(eLevelInfoNormal,"CMoveInfo::Serialize:"); 
//   Dump();

}
//==================================================================================================================================//
void CMoveInfo::DeSerialize(WORD format,istream &istr)
{
  if (format != NATIVE) PASSERT(1);

  WORD tmp = 0;
  istr >> tmp;
  m_is_valid_home_conf = (BYTE)tmp;
  istr.ignore(1);

  tmp = 0;
  istr >> tmp; 
  m_is_operator_party = (BYTE)tmp;
  istr.ignore(1);

  m_home_conf.DeSerialize(format,istr);
  m_current_conf.DeSerialize(format,istr);
  m_previous_conf.DeSerialize(format,istr);

//   PTRACE(eLevelInfoNormal,"CMoveInfo::DeSerialize:"); 
//   Dump();
}
//==================================================================================================================================//
void CMoveInfo::Serialize(WORD format,CSegment& seg)
{
  if (format != NATIVE) PASSERT(1);

  COstrStream pOstr;
  Serialize(format, pOstr);
  pOstr.Serialize(seg);
}
//==================================================================================================================================//
void CMoveInfo::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1);

    CIstrStream istr (seg);
    DeSerialize(format, istr);
}
//==================================================================================================================================//

//==================================================================================================================================//
// public API
//==================================================================================================================================//
//==================================================================================================================================//
bool CMoveInfo::IsValidHomeConf()const
{
  if(m_is_valid_home_conf){
    return true;
  }else{
    return false;
  }
}
//==================================================================================================================================//
DWORD CMoveInfo::GetHomeConf()const
{
  return m_home_conf.GetId();
}
//==============================================================================================================================
DWORD CMoveInfo::GetPreviousConf()const
{
  return m_previous_conf.GetId();
}
//==============================================================================================================================
 DWORD CMoveInfo::GetCurrentConf()const
 {
   return m_current_conf.GetId();
 }
//==================================================================================================================================//
void CMoveInfo::UpdateMove(CMoveConfDetails source_conf, CMoveConfDetails target_conf)
{
  PTRACE(eLevelInfoNormal,"CMoveInfo::UpdateMove - before");
  Dump();

  m_previous_conf = m_current_conf;
  m_current_conf = target_conf;
  if(!m_is_operator_party){
    if(target_conf.GetConfType()==eMoveConfType_Regular){
      m_home_conf = target_conf;
      m_is_valid_home_conf = 1;      
    }
  }
  PTRACE(eLevelInfoNormal,"CMoveInfo::UpdateMove - after");
  Dump();
}
//==================================================================================================================================//
void CMoveInfo::GetMoveConfType(eMoveConfType& homeConfType, eMoveConfType& currentConfType, eMoveConfType& previousConfType){
  homeConfType = m_home_conf.GetConfType();
  currentConfType = m_current_conf.GetConfType();
  previousConfType = m_previous_conf.GetConfType();
}
//==================================================================================================================================//


//==================================================================================================================================//
// public API - dump
//==================================================================================================================================//
void CMoveInfo::Dump()const
{
  COstrStream msg;
  Dump(msg);
  PTRACE2(eLevelInfoNormal,"CMoveInfo::Dump:\n",(char*)msg.str().c_str());

}
//==================================================================================================================================//
void CMoveInfo::Dump(COstrStream& msg)const
{
    msg << "m_is_valid_home_conf  = " << (WORD)m_is_valid_home_conf << "\n";
    msg << "m_is_operator_party   = " << (WORD)m_is_operator_party << "\n";
    msg << "m_home_conf:\n";
    m_home_conf.Dump(msg);
    msg << "m_current_conf:\n";
    m_current_conf.Dump(msg);
    msg << "m_previous_conf:\n";
    m_previous_conf.Dump(msg);
}
//==================================================================================================================================//
