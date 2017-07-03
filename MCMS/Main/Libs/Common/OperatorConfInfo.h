#ifndef __OPERATOR_CONF_INFO_H_
#define __OPERATOR_CONF_INFO_H_

#include <string>
#include <iomanip>

#include "PObject.h"
#include "Segment.h"
#include "NStream.h"
using namespace std;


class COperatorConfInfo : public CPObject
{
CLASS_TYPE_1(COperatorConfInfo, CPObject)
public:

  // constructots
  COperatorConfInfo();
  virtual ~COperatorConfInfo();
  const char * NameOf()const;

  friend bool operator== (const COperatorConfInfo& op1, const COperatorConfInfo& op2);

  // public API Serialize/DeSerialize
  void Serialize(WORD format,ostream &ostr);
  void DeSerialize(WORD format,istream &istr);
  void Serialize(WORD format,CSegment& seg);
  void DeSerialize(WORD format,CSegment& seg);

  // public API set
  void SetOperatorConfName(const char* operatorConfName);
  void SetOperatorPartyName(const char* operatorPartyName);
  void SetOperatorConfMonitoringId(DWORD confId);
  void SetOperatorPartyMonitoringId(DWORD partyId);

  // public API get
  const char* GetOperatorConfName();
  const char* GetOperatorPartyName();
  DWORD GetOperatorConfMonitoringId();
  DWORD GetOperatorPartyMonitoringId();

  // print
  void Dump()const;
  void Dump(COstrStream& msg)const;


protected:

  // attributes
  DWORD m_operator_conf_monitoring_id;
  string m_operator_conf_name;

  DWORD m_operator_party_monitoring_id;
  string m_operator_party_name;
};

#endif //__OPERATOR_CONF_INFO_H_
