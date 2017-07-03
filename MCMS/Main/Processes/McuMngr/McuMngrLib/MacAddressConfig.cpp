// MacAddressConfig.cpp: implementation of the CMacAddressConfig class.
//////////////////////////////////////////////////////////////////////////


#include "MacAddressConfig.h"
#include "StatusesGeneral.h"
#include "StringsMaps.h"
#include "psosxml.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMacAddressConfig::CMacAddressConfig()
{
	m_eth1MacAddress = "";
	m_eth2MacAddress = "";
}

/////////////////////////////////////////////////////////////////////////////
CMacAddressConfig::CMacAddressConfig(const CMacAddressConfig &other)
: CSerializeObject(other)
{
	m_eth1MacAddress = other.m_eth1MacAddress;
	m_eth2MacAddress = other.m_eth2MacAddress;
}
/////////////////////////////////////////////////////////////////////////////
CMacAddressConfig::~CMacAddressConfig()
{

}

/////////////////////////////////////////////////////////////////////////////
CMacAddressConfig& CMacAddressConfig::operator = (const CMacAddressConfig &other)
{
	m_eth1MacAddress = other.m_eth1MacAddress;
	m_eth2MacAddress = other.m_eth2MacAddress;

	return *this;
}

///////////////////////////////////////////////////////////////////////////
void CMacAddressConfig::SerializeXml(CXMLDOMElement*& pActionNode) const
{
	CXMLDOMElement *pMacAddressesNode;
	if(!pActionNode)
	{
		pActionNode =  new CXMLDOMElement();
		pActionNode->set_nodeName("MAC_ADDRESSES");
		pMacAddressesNode = pActionNode;
	}
	else
	{
		pMacAddressesNode = pActionNode->AddChildNode("MAC_ADDRESSES");
	}

	
	// ----------------  eth1  -----------------
	CXMLDOMElement *pEth1Node = pMacAddressesNode->AddChildNode("ETH1");
	if (pEth1Node)
	{
		pEth1Node->AddChildNode("MAC_ADDRESS", m_eth1MacAddress);
	}
	
	// ----------------  eth2  -----------------
	CXMLDOMElement *pEth2Node = pMacAddressesNode->AddChildNode("ETH2");
	if (pEth2Node)
	{
		pEth2Node->AddChildNode("MAC_ADDRESS", m_eth2MacAddress);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMacAddressConfig::DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* strAction)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pMacAddressesNode, *pEthNode;

	char* parentNodeName;
	pActionNode->get_nodeName(&parentNodeName); 
	if( !strcmp(parentNodeName, "MAC_ADDRESSES") )
	{ 
		pMacAddressesNode = pActionNode; 
	}  
	else
	{
		GET_MANDATORY_CHILD_NODE(pActionNode, "MAC_ADDRESSES", pMacAddressesNode);
	}
	

	// ----------------  eth1  -----------------
	GET_CHILD_NODE(pMacAddressesNode, "ETH1", pEthNode);
	if (pEthNode)
	{
		GET_VALIDATE_CHILD(pEthNode, "MAC_ADDRESS", m_eth1MacAddress, _0_TO_MAC_ADDRESS_CONFIG_LENGTH);	
	}

	// ----------------  eth2  -----------------
	GET_CHILD_NODE(pMacAddressesNode, "ETH2", pEthNode);
	if (pEthNode)
	{
		GET_VALIDATE_CHILD(pEthNode, "MAC_ADDRESS", m_eth2MacAddress, _0_TO_MAC_ADDRESS_CONFIG_LENGTH);	
	}

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////
const string CMacAddressConfig::GetEth1MacAddress()
{
	return m_eth1MacAddress;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMacAddressConfig::SetEth1MacAddress(const string sAddress)
{
	m_eth1MacAddress = sAddress;
}

////////////////////////////////////////////////////////////////////////////////////////////////
const string CMacAddressConfig::GetEth2MacAddress()
{
	return m_eth2MacAddress;	
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CMacAddressConfig::SetEth2MacAddress(const string sAddress)
{
	m_eth2MacAddress = sAddress;
}
