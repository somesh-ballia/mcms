#include "ServicePrefixStr.h"
#include "NStream.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////////////
/////////  class CServicePrefixStr


CServicePrefixStr::CServicePrefixStr()
{
    m_netServiceName[0]='\0';
}
/////////////////////////////////////////////////////////////////////////////
CServicePrefixStr::CServicePrefixStr(const CServicePrefixStr &other)
  :CPObject(other)
{
  strncpy(m_netServiceName,other.m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN);
  m_dialIn_prefix = other.m_dialIn_prefix;
}

/////////////////////////////////////////////////////////////////////////////
CServicePrefixStr&  CServicePrefixStr::operator = (const CServicePrefixStr& other)
{
  strncpy(m_netServiceName,other.m_netServiceName, NET_SERVICE_PROVIDER_NAME_LEN);
  m_dialIn_prefix = other.m_dialIn_prefix;
    
    return *this;
}
///////////////////////////////////////////////////////////////////////////
// BYTE CServicePrefixStr::operator == (const CServicePrefixStr& other){
// 	BYTE bRes = TRUE;

	
//     if(0 != strncmp(m_netServiceName,other.m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN))
//         bRes = FALSE;
// 	if( bRes == TRUE)
//     {
//         if(strcmp(m_dialIn_prefix.m_aliasName,other.m_dialIn_prefix.m_aliasName))
//             bRes = FALSE;// comparing only names , type should be the same
// 	}
// 	return bRes;
// }
// /////////////////////////////////////////////////////////////////////////////
CServicePrefixStr::~CServicePrefixStr(){}

/////////////////////////////////////////////////////////////////////////////
// void  CServicePrefixStr::Serialize(WORD format, std::ostream & ostr)
// {
//   ostr << m_netServiceName << "\n";
//   m_dialIn_prefix.Serialize(format,ostr);//assuming OPERATOR_MCMS
// }

void  CServicePrefixStr::SerializeXml(CXMLDOMElement *pServiceListNode)
{
  
  CXMLDOMElement *pTempNode=pServiceListNode->AddChildNode("DIAL_IN_H323_SRV_PREFIX");
  pTempNode->AddChildNode("NAME",m_netServiceName);
  pTempNode->AddChildNode("PREFIX",m_dialIn_prefix.m_aliasName);
}

int CServicePrefixStr::DeSerializeXml(CXMLDOMElement *pParentNode,char *pszError)
{
  int nStatus;
  
  GET_VALIDATE_CHILD(pParentNode,"NAME",m_netServiceName,NET_SERVICE_PROVIDER_NAME_LENGTH);
  GET_VALIDATE_CHILD(pParentNode,"PREFIX",m_dialIn_prefix.m_aliasName,ALIAS_NAME_LENGTH);

  return STATUS_OK;
}
// /////////////////////////////////////////////////////////////////////////////
// void  CServicePrefixStr::CdrSerialize(WORD format, COstrStream  &m_ostr,DWORD apiNum)
// {
//     m_ostr <<"net service name:"<< m_netServiceName << "\n";
//     m_ostr <<"dial in prefix:";
//     if(apiNum>=42 || format != OPERATOR_MCMS)
//         m_dialIn_prefix.Serialize(format, m_ostr);
//     m_ostr<< "\n";
// }

// /////////////////////////////////////////////////////////////////////////////
// void  CServicePrefixStr::DeSerialize(WORD format,std::istream  & istr)
// {
//   istr.getline(m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN+1,'\n');
//   m_dialIn_prefix.DeSerialize(format,istr);
// }
// /////////////////////////////////////////////////////////////////////////////
// void  CServicePrefixStr::CdrSerialize(WORD format, COstrStream  &m_ostr)
// {
//     m_ostr << m_netServiceName << ",";
//     m_dialIn_prefix.Serialize(format, m_ostr);//assuming OPERATOR_MCMS
   
// }
// /////////////////////////////////////////////////////////////////////////////
// void  CServicePrefixStr::CdrDeSerialize(WORD format, CIstrStream &m_istr)
// {
//     // m_istr.ignore(1);
//     m_istr.getline(m_netServiceName,NET_SERVICE_PROVIDER_NAME_LEN+1,',');
//     m_dialIn_prefix.DeSerialize(format, m_istr);
// }

/////////////////////////////////////////////////////////////////////////////
void  CServicePrefixStr::SetDialInPrefix(const  CH323Alias &prefix)
{
	m_dialIn_prefix = prefix;
}

/////////////////////////////////////////////////////////////////////////////
void  CServicePrefixStr::SetDialInPrefix(const char* prefix, WORD prefix_type)
{
	strncpy(m_dialIn_prefix.m_aliasName, prefix, sizeof(m_dialIn_prefix.m_aliasName) - 1);
	m_dialIn_prefix.m_aliasName[sizeof(m_dialIn_prefix.m_aliasName) - 1] = 0;
	m_dialIn_prefix.m_aliasType = prefix_type;
}

/////////////////////////////////////////////////////////////////////////////
const  CH323Alias*  CServicePrefixStr::GetDialInPrefix() const
{
	return &m_dialIn_prefix;
}


/////////////////////////////////////////////////////////////////////////////
void   CServicePrefixStr::SetNetServiceName(const char*  name)
{
    strncpy(m_netServiceName, name, sizeof(m_netServiceName) - 1);
    // cheaper to simply assign the null, then to check it - int len=strlen(name); if (len>NET_SERVICE_PROVIDER_NAME_LEN)...
    m_netServiceName[sizeof(m_netServiceName)-1]='\0';
}

/////////////////////////////////////////////////////////////////////////////
const char*  CServicePrefixStr::GetNetServiceName () const
{
    return m_netServiceName;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CServicePrefixStr::NameOf() const
{
    return "CServicePrefixStr";
}
