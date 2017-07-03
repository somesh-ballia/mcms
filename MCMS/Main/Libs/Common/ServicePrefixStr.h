#ifndef _SERVICE_PREFIX_STR_H__
#define _SERVICE_PREFIX_STR_H__

#include "SerializeObject.h"
#include "H323Alias.h"

// CServicePrefixStr - only for H323 service
class CServicePrefixStr : public CPObject
{
  CLASS_TYPE_1(CServicePrefixStr,CPObject)
    public:
  //Constructors   
  CServicePrefixStr();                   
  CServicePrefixStr(const CServicePrefixStr &other);                   
  CServicePrefixStr& operator = (const CServicePrefixStr& other);
  //BYTE operator == (const CServicePrefixStr& other);
  virtual ~CServicePrefixStr();
  
  // Implementation
  //void   Serialize(WORD format, std::ostream & ostr);     
  //void   DeSerialize(WORD format, std::istream & istr);    
  //void   CdrSerialize(WORD format, COstrStream  &m_ostr);     
  //void   CdrDeSerialize(WORD format, CIstrStream &m_istr);    
  //void   CdrSerialize(WORD format, COstrStream  &m_ostr,DWORD apiNum); 
  void   SerializeXml(CXMLDOMElement *pServiceListNode);
  int	   DeSerializeXml(CXMLDOMElement *pParentNode,char *pszError);
  
  const char*  NameOf() const;        
  
  void   SetDialInPrefix(const  CH323Alias &prefix);
  void   SetDialInPrefix(const char* prefix, WORD prefix_type = PARTY_H323_ALIAS_E164_TYPE);
  const  CH323Alias*  GetDialInPrefix() const;
  
  void   SetNetServiceName(const char*  name);                 
  const char*  GetNetServiceName () const;             
  
 public:
  // Attributes					
  char       m_netServiceName[NET_SERVICE_PROVIDER_NAME_LEN]; 
  CH323Alias     m_dialIn_prefix;
};


#endif
