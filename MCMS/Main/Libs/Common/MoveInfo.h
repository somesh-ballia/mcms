
#ifndef __MOVE_INFO_H_
#define __MOVE_INFO_H_

#include <string>
#include <iomanip>

#include "PObject.h"
#include "Segment.h"
#include "NStream.h"
using namespace std;

class CCommResApi;


typedef enum {
  eMoveConfType_None = 0,
  eMoveConfType_Regular,
  eMoveConfType_Eq,
  eMoveConfType_Operator
}eMoveConfType;
  

class CMoveConfDetails : public CPObject
{
CLASS_TYPE_1(CMoveConfDetails, CPObject)

public:
  // constructots
  CMoveConfDetails();
  CMoveConfDetails(DWORD conf_monitoring_id, const char* conf_name, eMoveConfType conf_type);
  CMoveConfDetails(CCommResApi* pConf);
  virtual ~CMoveConfDetails();
  const char * NameOf()const;

  // public API Serialize/DeSerialize
  void Serialize(WORD format,ostream &ostr);
  void DeSerialize(WORD format,istream &istr);
  void Serialize(WORD format,CSegment& seg);
  void DeSerialize(WORD format,CSegment& seg);

  // print
  void Dump()const;
  void Dump(COstrStream& msg)const;

  DWORD GetId()const;
  eMoveConfType GetConfType()const;
  const char* GetName()const;

protected:

  const char* MoveConfTypeStr(eMoveConfType eType)const;

  // attributes
  DWORD m_conf_monitoring_id;
  string m_conf_name;
  eMoveConfType m_conf_type;
};
    

class CMoveInfo : public CPObject
{
CLASS_TYPE_1(CMoveInfo, CPObject)
  public:

  // constructots
  CMoveInfo();
  CMoveInfo(CMoveConfDetails first_conf,BYTE is_operator_party);
  virtual ~CMoveInfo();
  const char * NameOf()const;

  // public API Serialize/DeSerialize
  void Serialize(WORD format,ostream &ostr);
  void DeSerialize(WORD format,istream &istr);
  void Serialize(WORD format,CSegment& seg);
  void DeSerialize(WORD format,CSegment& seg);

  // public API set
  bool  IsValidHomeConf()const;
  DWORD GetHomeConf()const;
  DWORD GetPreviousConf()const;
  DWORD GetCurrentConf()const;

  void UpdateMove(CMoveConfDetails source_conf, CMoveConfDetails target_conf);
  void Create(CMoveConfDetails& first_conf,BYTE is_operator_party);
  
  void GetMoveConfType(eMoveConfType& homeConfType, eMoveConfType& currentConfType, eMoveConfType& previousConfType);
  //  void SetOperatorConfName(const char* operatorConfName);
  //  void SetOperatorPartyName(const char* operatorPartyName);
  //  void SetOperatorConfMonitoringId(DWORD confId);
  //  void SetOperatorPartyMonitoringId(DWORD partyId);

  // public API get
  //  const char* GetOperatorConfName();
  //  const char* GetOperatorPartyName();
  //  DWORD GetOperatorConfMonitoringId();
  //  DWORD GetOperatorPartyMonitoringId();

  // print
  void Dump()const;
  void Dump(COstrStream& msg)const;


protected:

  // attributes
  BYTE m_is_valid_home_conf;
  BYTE m_is_operator_party;
  CMoveConfDetails m_home_conf;
  CMoveConfDetails m_current_conf;
  CMoveConfDetails m_previous_conf;

};


#endif //__MOVE_INFO_H_
