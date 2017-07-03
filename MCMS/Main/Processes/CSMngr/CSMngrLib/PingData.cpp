//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  class CPingData - data structure for a single ping action
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#include "PingData.h"
#include "NStream.h"
#include "ProcessBase.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "StringsLen.h"
#include <netinet/in.h>
#include <arpa/inet.h>

CPingData::CPingData()
{
    m_ipType = ePingIpType_IPv4;
    strcpy(m_destination, "");
    m_pingId = 0;
    m_pingStatus = STATUS_IN_PROGRESS;
}
//////////////////////////////////////////////////////////////////////////
CPingData::CPingData (const CPingData &other)
    :CSerializeObject(other)
{
    m_ipType = other.m_ipType;
    strncpy(m_destination, other.m_destination, STR_LEN);
    m_pingId = other.m_pingId;
    m_pingStatus = other.m_pingStatus;
}

//////////////////////////////////////////////////////////////////////////
CPingData::CPingData(const ePingIpType ipType, const char *destination)
{
    m_ipType = ipType;
    strncpy(m_destination, destination, STR_LEN - 1);
    m_destination[STR_LEN - 1] = '\0';
    m_pingId = 0;
    m_pingStatus = STATUS_IN_PROGRESS;
}
//////////////////////////////////////////////////////////////////////////
void CPingData::Dump (const char* title, WORD level)
{
    COstrStream msg;
    msg << "=== " << NameOf() << "::Dump " << title << " ==="
        << "\n m_pingId : " << m_pingId
        << "\n m_ipType : " << m_ipType << " " << GetPingIpTypeStr(m_ipType)
        << "\n m_destination : " << m_destination
        << "\n m_pingStatus : " << CProcessBase::GetProcess()->GetStatusAsString(m_pingStatus);
    PTRACE (level, msg.str().c_str());
}
//////////////////////////////////////////////////////////////////////////
void  CPingData::DumpCsMsg (const char* title, WORD level)
{
    COstrStream msg;
    msg << "=== PingData::Dump " << title << " ==="
        << "\n m_ipType : " << m_ipType << " " << GetPingIpTypeStr(m_ipType)
        << "\n m_destination : " << m_destination;
    PTRACE (level, msg.str().c_str());
}

//////////////////////////////////////////////////////////////////////////
char * CPingData::GetPingIpTypeStr (const ePingIpType ipType)
{
    if (ePingIpType_IPv4 == ipType)
        return "IPV4";
    else if (ePingIpType_IPv6 == ipType)
        return "IPV6";
    else
        return "UNKNOWN";
}
//////////////////////////////////////////////////////////////////////////
void CPingData::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    PASSERTMSG (TRUE, "CPingData::SerializeXml - should only be used in derived classes");
}
//////////////////////////////////////////////////////////////////////////
int	 CPingData::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action)
{
    PASSERTMSG (TRUE, "CPingData::DeSerializeXml - should only be used in derived classes");
    return STATUS_FAIL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  class CPingGet - data structure for a GET_PING action
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CPingGet::CPingGet()
{
}
//////////////////////////////////////////////////////////////////////////
CPingGet::CPingGet (const CPingData &other)
    :CPingData(other)
{
}

//////////////////////////////////////////////////////////////////////////
CPingGet::CPingGet(const ePingIpType ipType, const char *destination)
    :CPingData(ipType, destination)
{
}
//////////////////////////////////////////////////////////////////////////
void CPingGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    CXMLDOMElement* pGetPingNode;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("PING_STATE");
		pGetPingNode = pFatherNode;
	}
	else
	{
		pGetPingNode = pFatherNode->AddChildNode("PING_STATE");
	}

     ePingStatus pingStatus = ePingStatus_ok;
    if (STATUS_FAIL == m_pingStatus)
        pingStatus = ePingStatus_fail;
    if (pGetPingNode)
        pGetPingNode->AddChildNode("PING_STATUS", pingStatus, PING_STATUS_ENUM);
}
//////////////////////////////////////////////////////////////////////////
int	 CPingGet::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action)
{

    int nStatus=STATUS_OK;
    if (pActionNode)
    {
        GET_VALIDATE_CHILD(pActionNode,"PING_ID",(DWORD*)&m_pingId, _0_TO_DWORD);
    }
    return nStatus;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  class CPingSet - data structure for a SET_PING action
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CPingSet::CPingSet()
{
}
//////////////////////////////////////////////////////////////////////////
CPingSet::CPingSet (const CPingData &other)
    :CPingData(other)
{
}

//////////////////////////////////////////////////////////////////////////
CPingSet::CPingSet(const ePingIpType ipType, const char *destination)
    :CPingData(ipType, destination)
{
}
//////////////////////////////////////////////////////////////////////////
void CPingSet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    if (pFatherNode)
        pFatherNode->AddChildNode("PING_ID", m_pingId);
}
//////////////////////////////////////////////////////////////////////////
int	 CPingSet::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action)
{
    int nStatus=STATUS_OK;

    CXMLDOMElement* pPingNode;
    char* ParentNodeName;
    BOOL bReadTime=TRUE;


    pActionNode->get_nodeName(&ParentNodeName);
    if(!strcmp(ParentNodeName, "PING"))
    {
        pPingNode=pActionNode;
    }
    else
    {
        GET_MANDATORY_CHILD_NODE(pActionNode, "PING", pPingNode);
    }


    if (pPingNode)
    {
        GET_VALIDATE_CHILD(pPingNode,"PING_IP_TYPE",(DWORD*)&m_ipType, PING_IP_TYPE_ENUM);
        GET_VALIDATE_CHILD(pPingNode,"PING_DESTINATION",m_destination, ONE_LINE_BUFFER_LENGTH);
        sockaddr_in6 sa6;
        sockaddr_in sa;

         if((m_ipType == ePingIpType_IPv4 ) && inet_pton(AF_INET, m_destination, &(sa.sin_addr))  != 1)
         {
        	 PASSERTMSG (TRUE, "CPingSet::DeSerializeXml - IpV4 is not valid");
        	 nStatus = STATUS_IP_ADDRESS_NOT_VALID;
         }

         if((m_ipType == ePingIpType_IPv6 ) && inet_pton(AF_INET6, m_destination, sa6.sin6_addr.s6_addr)  != 1)
         {
        	 PASSERTMSG (TRUE, "CPingSet::DeSerializeXml - IpV6 is not valid");
        	 nStatus = STATUS_IP_ADDRESS_NOT_VALID;
         }
    }
    return nStatus;
}
