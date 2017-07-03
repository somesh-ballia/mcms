#ifndef _UserDefinedInfo_H_
#define _UserDefinedInfo_H_

#include "PObject.h"
#include "psosxml.h"
#include "ObjString.h"
#include "ConfPartyApiDefines.h"

#define MAX_USER_INFO_ITEMS 4

////////////////////////////////////////////////////////////////////////////
//                        CUserDefinedInfo
////////////////////////////////////////////////////////////////////////////
class CUserDefinedInfo : public CPObject
{
  CLASS_TYPE_1(CUserDefinedInfo, CPObject)

public:
               CUserDefinedInfo();
               CUserDefinedInfo(const CUserDefinedInfo& other);
  virtual     ~CUserDefinedInfo();

  const char*  NameOf() const                                { return "CUserDefinedInfo"; }

  void         Serialize(WORD format, std::ostream& m_ostr);
  void         DeSerialize(WORD format, std::istream& m_istr);
  void         SerializeXml(CXMLDOMElement* pParent);
  int          DeSerializeXml(CXMLDOMElement* pParent, char* pszError);

  void         SetUserDefinedInfo(const char* UserInfo, int InfoNumber);
  const char*  GetUserDefinedInfo(int InfoNumber) const;

  void         SetConfId(const DWORD confId)                 { m_dwConfId = confId; }
  DWORD        GetConfId()  const                            { return m_dwConfId; }
  void         SetPartyId(const DWORD partyId)               { m_dwPartyId = partyId; }
  DWORD        GetPartyId()  const                           { return m_dwPartyId; }
  void         SetAdditionalInfo(const char* additionalInfo) { m_AdditionalInfo = additionalInfo; }
  const char*  GetAdditionalInfo() const                     { return m_AdditionalInfo.GetString(); }

protected:
  DWORD        m_dwConfId;
  DWORD        m_dwPartyId;
  CSmallString m_UserDefinedInfo[MAX_USER_INFO_ITEMS];
  CSmallString m_AdditionalInfo;
};

#endif // _UserDefinedInfo_H_
