
#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "SnmpSecurity.h"
#include "SnmpTrapCommunity.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"
#include "FipsMode.h"


CSnmpSecurity::CSnmpSecurity()
:m_community("public",2)
{
	Init();
/*
	//test yuhui
    m_snmpTrapVersion         = eSnmpVer3Trap;	
	DWORD ip = SystemIpStringToDWORD("172.21.116.108",eHost);	
	CSnmpTrapCommunity temp_trap_com(ip,"yuhui");
	m_trapDestinations.push_back(temp_trap_com);

	ip = SystemIpStringToDWORD("172.21.98.45",eHost);
	CSnmpTrapCommunity temp_trap_com1(ip,"yuhui");
	m_trapDestinations.push_back(temp_trap_com1);
*/	
}  
void CSnmpSecurity::Init()
{
	m_sendAuthenticationTraps = TRUE;
	m_acceptAllRequests       = TRUE;
	m_snmpTrapVersion         = eSnmpVer3Trap;
	m_isFromEma = true;
}

void CSnmpSecurity::InitDefaults()
{
	m_community.InitDefaults();
	Init();
}

/////////////////////////////////////////////////////////////////////////////
CSnmpSecurity::~CSnmpSecurity()
{ }


void CSnmpSecurity::DeleteTraps(list<CSnmpTrapCommunity>& trapList)
{
	list<CSnmpTrapCommunity>::iterator itTrapInList = trapList.begin();

	for(; itTrapInList != trapList.end(); ++itTrapInList )
	{
		bool found = false;
		vector<CSnmpTrapCommunity>::iterator itTraps =  m_trapDestinations.begin();
		for (; itTraps != m_trapDestinations.end() && !found; ++itTraps)
		{
			if (*itTraps == *itTrapInList)
			{
				m_trapDestinations.erase(itTraps);
				found = true;
			}
		}
		if (!found)
		{
			TRACEINTO << "Trap wasnt deleted because it wasnt found. name " << itTrapInList->GetSnmpV3Param().GetUserName();
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
CSnmpSecurity& CSnmpSecurity::operator=(const CSnmpSecurity& other)
{
    m_snmpTrapVersion = other.m_snmpTrapVersion;
    m_sendAuthenticationTraps = other.m_sendAuthenticationTraps;
    m_acceptAllRequests       = other.m_acceptAllRequests;
    m_community = other.m_community;
    m_trapDestinations = other.m_trapDestinations; 		
    m_requestSources = other.m_requestSources;
    m_isFromEma = other.m_isFromEma;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
CSnmpSecurity:: CSnmpSecurity(const CSnmpSecurity &other)
:CSerializeObject(other)
{
    *this = other;
}

/////////////////////////////////////////////////////////////////////////////
const char* CSnmpSecurity::NameOf() const
{
    return "CSnmpSecurity" ;       
}

/////////////////////////////////////////////////////////////////////////////
WORD CSnmpSecurity::SendAuthenticationTraps() const
{
    return m_sendAuthenticationTraps;
}

////////////////////////////////////////////////////////////////////////////
void CSnmpSecurity::SetSendAuthenticationTraps(const WORD yesNo)
{
    m_sendAuthenticationTraps = yesNo;
}

////////////////////////////////////////////////////////////////////////////
WORD CSnmpSecurity::AcceptAllRequests()const
{
    return m_acceptAllRequests;
}

/////////////////////////////////////////////////////////////////////////
void CSnmpSecurity::SetAcceptAllRequests(const WORD yesNo)
{
    m_acceptAllRequests = yesNo;
}

////////////////////////////////////////////////////////////////////////////
const CSnmpCommunityPermission& CSnmpSecurity::GetCommunityPermission()const
{
    return m_community;
}
////////////////////////////////////////////////////////////////////////////
void CSnmpSecurity::SerializeXml(CXMLDOMElement*& pParentNode,bool isToEma) const
{
 	CXMLDOMElement* pSnmpSecurityNode = pParentNode->AddChildNode("SECURITY");
	CXMLDOMElement* pTempListNode = NULL;
	WORD i = 0;

	pSnmpSecurityNode->AddChildNode("ACCEPT_ALL_REQUESTS",m_acceptAllRequests,_BOOL);
	pSnmpSecurityNode->AddChildNode("SEND_AUTHENTICATION_TRAPS",m_sendAuthenticationTraps,_BOOL);
	
	pTempListNode = pSnmpSecurityNode->AddChildNode("TRAP_DESTINATION_LIST");
	
	for( i=0; i < m_trapDestinations.size(); i++ )
         m_trapDestinations[i].SerializeXml(pTempListNode,isToEma);

	pTempListNode = pSnmpSecurityNode->AddChildNode("REQUEST_SOURCE_LIST");
	
	for( i=0; i < m_requestSources.size(); i++ )
		pTempListNode->AddChildNode("REQUEST_SOURCE",m_requestSources[i],IP_ADDRESS);

    
	pTempListNode = pSnmpSecurityNode->AddChildNode("COMMUNITY_PERMISSION_LIST");
	m_community.SerializeXml(pTempListNode,isToEma);

    pSnmpSecurityNode->AddChildNode("TRAP_VERSION", m_snmpTrapVersion, SNMP_VER_ENUM);
}
////////////////////////////////////////////////////////////////////////////
void CSnmpSecurity::SerializeXml(CXMLDOMElement*& pParentNode) const
{
 	CXMLDOMElement* pSnmpSecurityNode = pParentNode->AddChildNode("SECURITY");
	CXMLDOMElement* pTempListNode = NULL;
	WORD i = 0;

	pSnmpSecurityNode->AddChildNode("ACCEPT_ALL_REQUESTS",m_acceptAllRequests,_BOOL);
	pSnmpSecurityNode->AddChildNode("SEND_AUTHENTICATION_TRAPS",m_sendAuthenticationTraps,_BOOL);

	pTempListNode = pSnmpSecurityNode->AddChildNode("TRAP_DESTINATION_LIST");

	for( i=0; i < m_trapDestinations.size(); i++ )
         m_trapDestinations[i].SerializeXml(pTempListNode);

	pTempListNode = pSnmpSecurityNode->AddChildNode("REQUEST_SOURCE_LIST");

	for( i=0; i < m_requestSources.size(); i++ )
		pTempListNode->AddChildNode("REQUEST_SOURCE",m_requestSources[i],IP_ADDRESS);


	pTempListNode = pSnmpSecurityNode->AddChildNode("COMMUNITY_PERMISSION_LIST");
	m_community.SerializeXml(pTempListNode);

    pSnmpSecurityNode->AddChildNode("TRAP_VERSION", m_snmpTrapVersion, SNMP_VER_ENUM);
}

/////////////////////////////////////////////////////////////////////////////
int CSnmpSecurity::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pSnmpSecurityNode=NULL, *pTempNode=NULL, *pTempSonNode=NULL;
	GET_CHILD_NODE(pActionNode, "SECURITY", pSnmpSecurityNode);
	m_trapDestinations.clear();
	m_requestSources.clear();
	
	if (!m_isFromEma)
	{
		m_community.GetSnmpV3Param().UnSetIsFromEma();
	}

	if (pSnmpSecurityNode)
	{
		GET_VALIDATE_CHILD(pSnmpSecurityNode,"ACCEPT_ALL_REQUESTS",&m_acceptAllRequests,_BOOL);
		GET_VALIDATE_CHILD(pSnmpSecurityNode,"SEND_AUTHENTICATION_TRAPS",&m_sendAuthenticationTraps,_BOOL);
		WORD trapVersion = (WORD)eSnmpVer3Trap;

		GET_VALIDATE_CHILD(pSnmpSecurityNode,"TRAP_VERSION",&trapVersion,SNMP_VER_ENUM);
		m_snmpTrapVersion = (eSnmpVersionTrap) trapVersion;

		//-------------- TrapDestinations --------------
		GET_CHILD_NODE(pSnmpSecurityNode, "TRAP_DESTINATION_LIST", pTempNode);
		
		if (pTempNode)
		{
	
			BOOL isFipsMode = GetSNMPFipsMode();

			GET_FIRST_CHILD_NODE(pTempNode,"TRAP_DESTINATION",pTempSonNode);
			while(pTempSonNode)
			{
				CSnmpTrapCommunity temp_trap_com;

				if (!m_isFromEma)
				{
					temp_trap_com.GetSnmpV3Param().UnSetIsFromEma();
				}

				nStatus = temp_trap_com.DeSerializeXml(pTempSonNode, pszError,action);
				if (nStatus != STATUS_OK)
					return nStatus;

				m_trapDestinations.push_back(temp_trap_com);

					
				GET_NEXT_CHILD_NODE(pTempNode,"TRAP_DESTINATION",pTempSonNode);
			}
		} // end if (pTempNode)
		else
		{
			return STATUS_FAIL;
		}

		//--------------- RequestSources ---------------
		GET_CHILD_NODE(pSnmpSecurityNode, "REQUEST_SOURCE_LIST", pTempNode);
		
		if (pTempNode)
		{
			GET_FIRST_CHILD_NODE(pTempNode,"REQUEST_SOURCE",pTempSonNode);

			while(pTempSonNode)
			{
				DWORD ip_address;
				GET_VALIDATE(pTempSonNode,&ip_address,IP_ADDRESS);
				m_requestSources.push_back(ip_address);
				GET_NEXT_CHILD_NODE(pTempNode,"REQUEST_SOURCE",pTempSonNode);
			}
		} // end if (pTempNode)

		//----------------- Communities ----------------
		GET_CHILD_NODE(pSnmpSecurityNode, "COMMUNITY_PERMISSION_LIST", pTempNode);

		if (pTempNode)
		{
			GET_FIRST_CHILD_NODE(pTempNode,"COMMUNITY_PERMISSION",pTempSonNode);
			if (pTempSonNode == NULL)
				return STATUS_OK; // one is must !!!

			nStatus = m_community.DeSerializeXml(pTempSonNode, pszError,action);

			if (nStatus != STATUS_OK)
				return nStatus;

			GET_NEXT_CHILD_NODE(pTempNode,"COMMUNITY_PERMISSION",pTempSonNode);
			if (pTempSonNode == STATUS_OK)
				return STATUS_OK; // only one community is allowed
		} // end if (pTempNode)
	}
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CSnmpSecurity::CommunityCfg(ostream & str) const
{
   if (AcceptAllRequests())
   {
   		str << "rocommunity " << m_community.GetCommunityName() << endl;
   		str << "ro6community " << m_community.GetCommunityName() << endl;
   }
   else
   {
   		for (DWORD i=0; i< m_requestSources.size(); i++)
   		{
	        char ip_address[32];
            SystemDWORDToIpString(m_requestSources[i] , ip_address); 
            str << "rocommunity " << m_community.GetCommunityName() << " " <<  ip_address << endl;
            str << "ro6community " << m_community.GetCommunityName() << " " <<  ip_address << endl;
   		}
   }
}

/////////////////////////////////////////////////////////////////////////////
void CSnmpSecurity::TrapCfg(ostream & str) const
{
	//yaela TOFIX
	std::string trap_sink;
    switch (GetSnmpTrapVersion())
    {
        case eSnmpVer1Trap:
            trap_sink = "trapsink ";
            break;

        case eSnmpVer2Trap:
            trap_sink = "trap2sink ";
            break;

        case eSnmpVer3Trap:
			{
				//str << "createUser traptest SHA mypassword  AES mypassword" <<endl;
				//str << "trapsess -v 3 -u traptest -a SHA -A mypassword -x AES -X mypassword -l authPriv -e 0x8000000001020304 172.21.116.108" <<endl;				
				for (DWORD i = 0; i < m_trapDestinations.size(); i++)
				{
					eSnmpVersionTrap verConfig = m_trapDestinations[i].GetSnmpTrapVersionForConfig();
					if((eSnmpVer1Trap == verConfig)||(eSnmpVer2Trap == verConfig))
					{
						continue;
					}
					m_trapDestinations[i].GetSnmpV3Param().TrapConfig(str, !m_trapDestinations[i].GetTrapInformEnabled());
					if (m_trapDestinations[i].GetTrapInformEnabled())
					{
						str << " -Ci";
					}

					str << " " << m_trapDestinations[i].GetGeneralTrapDestination() <<endl;
				}
			}
            return;

      	default:
      	    PASSERTSTREAM_AND_RETURN(1,
      	       "Unknown SNMP version: " << GetSnmpTrapVersion());
    }
    
    for (DWORD i = 0; i < m_trapDestinations.size(); i++)
    {
		eSnmpVersionTrap verConfig = m_trapDestinations[i].GetSnmpTrapVersionForConfig();
		if(eSnmpVer3Trap == verConfig)
		{
			continue;
		}
		
		if (eSnmpVer2Trap == verConfig &&  m_trapDestinations[i].GetTrapInformEnabled())
		{
			str <<  "informsink ";
		}
		else
		{
			str << trap_sink ;
		}

    	string tempDestName=m_trapDestinations[i].GetCommunityName();
    	if (tempDestName.compare(0, 1, ":"))
    	{
    		str <<  m_trapDestinations[i].GetGeneralTrapDestination()  << " " << m_trapDestinations[i].GetCommunityName() << endl;
    	}
    	else
    	{
    		str <<  m_trapDestinations[i].GetGeneralTrapDestination()  << m_trapDestinations[i].GetCommunityName() << endl;
    	}
    }
}


const CSnmpV3SecurityParams& CSnmpSecurity::GetSnmpV3Param(void) const
{
	return m_community.GetSnmpV3Param();
}



void CSnmpSecurity::UnSetIsFromEma() const
{
	m_isFromEma =false;

	for (DWORD i = 0; i < m_trapDestinations.size(); i++)
	{
		m_trapDestinations[i].GetSnmpV3Param().UnSetIsFromEma();
	}
}


