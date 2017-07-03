#include "SnmpTrapCommunity.h"
#include "psosxml.h"
#include "StringsLen.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////////

CSnmpTrapCommunity::CSnmpTrapCommunity() :
    m_snmpTrapVersionForConfig(eSnmpVer3Trap),
    m_trapInformEnabled(0)
{ }

//////////////////////////////////////////////////////////////////////////
CSnmpTrapCommunity::CSnmpTrapCommunity(const string generalIpAddr, const string & communityName, BOOL trapInformEnabled) :
	m_snmpTrapVersionForConfig(eSnmpVer3Trap),
	m_generalTrapDestination(generalIpAddr),
	m_communityName(communityName),
	m_trapInformEnabled((BYTE)trapInformEnabled)
{ }

//////////////////////////////////////////////////////////////////////////
CSnmpTrapCommunity::CSnmpTrapCommunity(const CSnmpTrapCommunity &other) :
	CSerializeObject(other)
{
   *this = other;
}

// virtual
CSerializeObject* CSnmpTrapCommunity::Clone()
{
	return new CSnmpTrapCommunity;
}

//////////////////////////////////////////////////////////////////////////
const char*  CSnmpTrapCommunity::NameOf() const
{
    return "CSnmpTrapCommunity";
}

//////////////////////////////////////////////////////////////////////////
CSnmpTrapCommunity& CSnmpTrapCommunity::operator= (const CSnmpTrapCommunity& other)
{
    m_communityName = other.m_communityName;
    m_generalTrapDestination = other.m_generalTrapDestination;
    m_trapInformEnabled = other.m_trapInformEnabled;
    m_snmpV3Params = other.m_snmpV3Params;
    m_snmpTrapVersionForConfig = other.m_snmpTrapVersionForConfig;
    return *this;
}
bool CSnmpTrapCommunity::operator== (const CSnmpTrapCommunity& other)
{
    return (m_communityName == other.m_communityName &&
    		m_generalTrapDestination == other.m_generalTrapDestination &&
    		m_trapInformEnabled == other.m_trapInformEnabled &&
    		m_snmpV3Params == other.m_snmpV3Params &&
    		m_snmpTrapVersionForConfig == other.m_snmpTrapVersionForConfig);

}

/////////////////////////////////////////////////////////////////////////
CSnmpTrapCommunity::~CSnmpTrapCommunity()
{ }

/////////////////////////////////////////////////////////////////////////
void CSnmpTrapCommunity::SerializeXml(CXMLDOMElement*& pParentNode)const
{

	CXMLDOMElement* pTrapCommunityNode = pParentNode->AddChildNode("TRAP_DESTINATION");

	// to support old EMA versions
	pTrapCommunityNode->AddChildNode("IP",m_generalTrapDestination);
	pTrapCommunityNode->AddChildNode("TRAP_ADDRESS",m_generalTrapDestination);
	pTrapCommunityNode->AddChildNode("COMMUNITY_NAME",m_communityName);
	m_snmpV3Params.SerializeXml(pTrapCommunityNode);
	pTrapCommunityNode->AddChildNode("TRAP_VERSION", m_snmpTrapVersionForConfig, SNMP_VER_ENUM);	

	pTrapCommunityNode->AddChildNode("TRAP_INFORM_ENABLED", m_trapInformEnabled, _BOOL);


}
/////////////////////////////////////////////////////////////////////////
void CSnmpTrapCommunity::SerializeXml(CXMLDOMElement*& pParentNode,bool isToEma)const
{
	CXMLDOMElement* pTrapCommunityNode = pParentNode->AddChildNode("TRAP_DESTINATION");

	// to support old EMA versions
	pTrapCommunityNode->AddChildNode("IP",m_generalTrapDestination);
	pTrapCommunityNode->AddChildNode("TRAP_ADDRESS",m_generalTrapDestination);
	pTrapCommunityNode->AddChildNode("COMMUNITY_NAME",m_communityName);
	m_snmpV3Params.SerializeXml(pTrapCommunityNode,isToEma);
	pTrapCommunityNode->AddChildNode("TRAP_VERSION", m_snmpTrapVersionForConfig, SNMP_VER_ENUM);

	pTrapCommunityNode->AddChildNode("TRAP_INFORM_ENABLED", m_trapInformEnabled, _BOOL);



}

/////////////////////////////////////////////////////////////////////////////
int CSnmpTrapCommunity::DeSerializeXml(CXMLDOMElement* pTrapCommunityNode,
									   char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
	bool isNewApi;

	STATUS statGeneralIp = pTrapCommunityNode->GetAndVerifyChildNodeValue("TRAP_ADDRESS",m_generalTrapDestination,pszError, _0_TO_IPV6_ADDRESS_LENGTH);

	isNewApi = (statGeneralIp != STATUS_NODE_MISSING);
	if (!isNewApi)
	{
		FTRACESTR(eLevelWarn) << "TRAP_ADDRESS node is missing,Using old api for adding snmp trap: support only ipv4 and no inform trap is supported";

		DWORD oldTrapDestination = 0 ;
		GET_VALIDATE_CHILD(pTrapCommunityNode,"IP",&oldTrapDestination,IP_ADDRESS);

    	char ip_address[32];
		SystemDWORDToIpString(oldTrapDestination, ip_address);
		m_generalTrapDestination = ip_address;
	}
	else
	{

		if(statGeneralIp != STATUS_OK &&
				statGeneralIp != STATUS_ENUM_VALUE_INVALID &&
				statGeneralIp != STATUS_NODE_LENGTH_TOO_SHORT &&
				statGeneralIp != STATUS_NODE_LENGTH_TOO_LONG &&
				statGeneralIp != STATUS_VALUE_OUT_OF_RANGE &&
				statGeneralIp != STATUS_IP_ADDRESS_INVALID)
		{
			return statGeneralIp;
		}

	}

	GET_VALIDATE_CHILD(pTrapCommunityNode,"COMMUNITY_NAME",m_communityName,_0_TO_SNMP_STRING_LENGTH);

	WORD trapVersion = (WORD)eSnmpVer3Trap;
	GET_VALIDATE_CHILD(pTrapCommunityNode,"TRAP_VERSION",&trapVersion,SNMP_VER_ENUM);
	m_snmpTrapVersionForConfig = (eSnmpVersionTrap) trapVersion;

	// yaela HERE FIX it and print error message

    if (isNewApi)
	{
    	GET_VALIDATE_CHILD(pTrapCommunityNode, "TRAP_INFORM_ENABLED", &m_trapInformEnabled,  _BOOL);
	}

	if (STATUS_OK == nStatus)
	{
		 
		nStatus = m_snmpV3Params.DeSerializeXml(pTrapCommunityNode,
												pszError, action);
		
		if (isNewApi && !m_trapInformEnabled && m_snmpV3Params.GetEngineID().length() > 0 )
		{
			FTRACESTR(eLevelWarn) << "Cannot set Engine ID for non inform traps. Will be ignored.";			
		}
	}

 	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
const string & CSnmpTrapCommunity::GetCommunityName() const
{
    return m_communityName;
}

/////////////////////////////////////////////////////////////////////////////
string  CSnmpTrapCommunity::GetGeneralTrapDestination() const
{
	return m_generalTrapDestination;
}

/////////////////////////////////////////////////////////////////////////////

BOOL	CSnmpTrapCommunity::GetTrapInformEnabled() const
{
	return m_trapInformEnabled ? TRUE : FALSE;
}

const CSnmpV3SecurityParams& CSnmpTrapCommunity::GetSnmpV3Param(void) const
{
	return m_snmpV3Params;
}
