// IpInterface.cpp: implementation of the CIpInterface class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "IpInterface.h"
#include "IpV4.h"
#include "IpV6.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ObjString.h"
#include "ApiStatuses.h"

extern char* IpTypeToString(APIU32 ipType, bool caps = false);


// ------------------------------------------------------------
CIpInterface::CIpInterface ()
{
}


// ------------------------------------------------------------
CIpInterface::CIpInterface (const IP_INTERFACE_S ipInterface)
{
	memcpy(&m_ipInterfaceStruct, &ipInterface, sizeof(IP_INTERFACE_S));
}


// ------------------------------------------------------------
CIpInterface::~CIpInterface ()
{
}


// ------------------------------------------------------------
void CIpInterface::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement *pIpInterfaceNode=NULL, *pAliasesListNode=NULL, *pCurAliasNode=NULL;

	pIpInterfaceNode = pFatherNode->AddChildNode("IP_INTERFACE");

	CIPv4 ipV4(m_ipInterfaceStruct.iPv4);
	ipV4.SerializeXml(pIpInterfaceNode);

	for(int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		CIPv6 ipV6(m_ipInterfaceStruct.iPv6s[i]);
		ipV6.SerializeXml(pIpInterfaceNode);
	}

	pIpInterfaceNode->AddChildNode("IP_TYPE", m_ipInterfaceStruct.ipType, IP_TYPE_ENUM);

	pAliasesListNode = pIpInterfaceNode->AddChildNode("ALIASES_LIST");
	for (int i=0; i<MAX_ALIAS_NAMES_NUM; i++)
	{
		pCurAliasNode = pAliasesListNode->AddChildNode("ALIAS");
		pCurAliasNode->AddChildNode("NAME",       (char*)(m_ipInterfaceStruct.aliasesList[i].aliasContent) );
		pCurAliasNode->AddChildNode("ALIAS_TYPE", m_ipInterfaceStruct.aliasesList[i].aliasType, ALIAS_TYPE_ENUM);
	}

	pIpInterfaceNode->AddChildNode("IS_SECURED", m_ipInterfaceStruct.isSecured, _BOOL);
	pIpInterfaceNode->AddChildNode("PERMANENT_NETWORK", m_ipInterfaceStruct.isPermanentOpen, _BOOL);
}


// ------------------------------------------------------------
int	 CIpInterface::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pIpV4Node=NULL, *pIpV6Node=NULL, *pAliasesListNode=NULL, *pCurAliasNode=NULL;

	GET_CHILD_NODE(pActionNode, "IP_V4_INTERFACE", pIpV4Node);
	if (pIpV4Node)
	{
		CIPv4 ipV4(m_ipInterfaceStruct.iPv4);
		ipV4.DeSerializeXml(pIpV4Node, pszError);
	}

	for(int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		GET_CHILD_NODE(pActionNode, "IP_V6_INTERFACE", pIpV6Node);
		if (pIpV6Node)
		{
			CIPv6 ipV6(m_ipInterfaceStruct.iPv6s[i]);
			ipV6.DeSerializeXml(pIpV6Node, pszError);
		}
	}

	GET_VALIDATE_CHILD(pActionNode,"IP_TYPE", (char*)(&m_ipInterfaceStruct.ipType), IP_TYPE_ENUM);

	GET_CHILD_NODE(pActionNode, "ALIASES_LIST", pAliasesListNode);
	if (pAliasesListNode)
	{
		GET_FIRST_CHILD_NODE(pAliasesListNode, "ALIAS", pCurAliasNode);

		int aliasIndex = 0;
		while( pCurAliasNode  &&  (aliasIndex < MAX_ALIAS_NAMES_NUM) )
		{
			GET_VALIDATE_ASCII_CHILD(pCurAliasNode,"NAME",(char*)m_ipInterfaceStruct.aliasesList[aliasIndex].aliasContent, ALIAS_NAME_LEN);
			GET_VALIDATE_CHILD(pCurAliasNode,"ALIAS_TYPE",&m_ipInterfaceStruct.aliasesList[aliasIndex].aliasType, ALIAS_TYPE_ENUM);

			aliasIndex++;
			GET_NEXT_CHILD_NODE(pAliasesListNode, "ALIAS", pCurAliasNode);
		}
	}

	GET_VALIDATE_CHILD(pActionNode,"IS_SECURED",&(m_ipInterfaceStruct.isSecured),_BOOL);
//Judith - we temporarly remove the permanent network support (until federal version is enable)
//	GET_VALIDATE_CHILD(pActionNode,"PERMANENT_NETWORK",&(m_ipInterfaceStruct.isPermanentOpen),_BOOL);
	m_ipInterfaceStruct.isPermanentOpen = TRUE;

	return STATUS_OK;
}


// ------------------------------------------------------------
void  CIpInterface::Dump(std::ostream& msg) const
{
	if (m_ipInterfaceStruct.boardId == 0 && m_ipInterfaceStruct.pqId==0 && m_ipInterfaceStruct.iPv4.iPv4Address==0)
		return;

	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n===== IpInterface::Dump =====\n"
	    << "Board Id:"   << m_ipInterfaceStruct.boardId
	    << ", PQ Id:"    << m_ipInterfaceStruct.pqId
	    << ", Ip Type: ";

	char* ipTypeStr = ::IpTypeToString(m_ipInterfaceStruct.ipType);
	if (ipTypeStr)
	{
		msg << ipTypeStr;
	}
	else
	{
		msg << "(invalid: " << m_ipInterfaceStruct.ipType << ")";
	}

	msg << "\n===== IPv4 =====\n";
	msg << CIPv4(m_ipInterfaceStruct.iPv4);

	msg << "\n===== IPv6 =====\n";
	for(int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		msg << "\n--- IPv6 " << i << ":\n"
		    << CIPv6(m_ipInterfaceStruct.iPv6s[i]);
	}


	msg << "\n===== Aliases =====\n";
	for (int i=0; i<MAX_ALIAS_NAMES_NUM; i++)
	{
		msg << "== Alias " << i << ": "
		    << "Name: "    << m_ipInterfaceStruct.aliasesList[i].aliasContent << ", "
		    << "Type: "    << m_ipInterfaceStruct.aliasesList[i].aliasType << "\n";
	}

	msg << "====================\n";

	msg << "IsSucured: "<< m_ipInterfaceStruct.isSecured << "\n";
	msg << "isPermanentOpen: "<<m_ipInterfaceStruct.isPermanentOpen << "\n";

	msg << "\n===== iPv4Internal =====\n";
	msg << CIPv4(m_ipInterfaceStruct.iPv4Internal);
}


// ------------------------------------------------------------
CIpInterface& CIpInterface::operator = (const CIpInterface &rOther)
{
	memcpy( &m_ipInterfaceStruct,
		    &rOther.m_ipInterfaceStruct,
			sizeof(IP_INTERFACE_S) );

	return *this;
}


// ------------------------------------------------------------
IPV4_S CIpInterface::GetIPv4 ()
{
	return m_ipInterfaceStruct.iPv4;
}


// ------------------------------------------------------------
void CIpInterface::SetIPv4 (const IPV4_S iPv4)
{
	memcpy(&m_ipInterfaceStruct.iPv4, &iPv4, sizeof(IPV4_S));
}


// ------------------------------------------------------------
IPV6_S CIpInterface::GetIPv6 (const int idx)
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		return m_ipInterfaceStruct.iPv6s[idx];
	}
	else
	{
		return m_ipInterfaceStruct.iPv6s[0];
	}
}


// ------------------------------------------------------------
void CIpInterface::SetIPv6 (const int idx, const IPV6_S iPv6)
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
	{
		memcpy(&m_ipInterfaceStruct.iPv6s[idx], &iPv6, sizeof(IPV6_S));
	}
}


// ------------------------------------------------------------
eIpType CIpInterface::GetIpType ()
{
	return (eIpType)(m_ipInterfaceStruct.ipType);
}


// ------------------------------------------------------------
void CIpInterface::SetIpType (const eIpType type)
{
	m_ipInterfaceStruct.ipType = type;
}


// ------------------------------------------------------------
ALIAS_S*  CIpInterface::GetAliasesList ()
{
	return m_ipInterfaceStruct.aliasesList;
}


// ------------------------------------------------------------
DWORD CIpInterface::GetBoardId()
{
	return m_ipInterfaceStruct.boardId;
}


// ------------------------------------------------------------
void CIpInterface::SetBoardId(DWORD id)
{
	m_ipInterfaceStruct.boardId = id;
}

// ------------------------------------------------------------
DWORD CIpInterface::GetPqId()
{
	return m_ipInterfaceStruct.pqId;
}

// ------------------------------------------------------------
void CIpInterface::SetPqId(DWORD id)
{
	m_ipInterfaceStruct.pqId = id;
}

// ------------------------------------------------------------
BOOL CIpInterface::GetIsSecured()
{
	return m_ipInterfaceStruct.isSecured;
}

// ------------------------------------------------------------
void CIpInterface::SetIsSecured(BOOL isSecured)
{
	m_ipInterfaceStruct.isSecured = isSecured;
}
// ------------------------------------------------------------
BOOL CIpInterface::GetIsPermanentNetworkOpen()
{
	return m_ipInterfaceStruct.isPermanentOpen;
}

// ------------------------------------------------------------
void CIpInterface::SetIsPermanentNetworkOpen(BOOL isPermanentOpen)
{
	m_ipInterfaceStruct.isPermanentOpen = isPermanentOpen;
}
// ------------------------------------------------------------
void CIpInterface::SetData(IP_INTERFACE_S ipInteface)
{
	memcpy(&m_ipInterfaceStruct, &ipInteface, sizeof(IP_INTERFACE_S));
}

// ------------------------------------------------------------

