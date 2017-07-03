// IpConfiguration.cpp: implementation of the CIpConfiguration class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "IpConfiguration.h"
#include "StatusesGeneral.h"



// ------------------------------------------------------------
CIpConfiguration::CIpConfiguration ()
{
}


// ------------------------------------------------------------
CIpConfiguration::CIpConfiguration (const IP_CONFIGURATION_S ipConfig)
{
	memcpy(&m_ipConfigurationStruct, &ipConfig, sizeof(IP_CONFIGURATION_S));
}


// ------------------------------------------------------------
CIpConfiguration::~CIpConfiguration ()
{
}


// ------------------------------------------------------------
void CIpConfiguration::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement *pIpConfigNode=NULL, *pIpInterfacesListNode=NULL;
	
	pIpConfigNode = pFatherNode->AddChildNode("IP_CONFIGURATION");

	pIpInterfacesListNode = pIpConfigNode->AddChildNode("IP_INTERFACES_LIST");
	for (int i=0; i<MAX_NUM_OF_INTERFACES; i++)
	{
		CIpInterface ipInterface(m_ipConfigurationStruct.interfacesList[i]);
		ipInterface.SerializeXml(pIpInterfacesListNode);
	}

	CNetworkParameters netParams(m_ipConfigurationStruct.networkParams);
	netParams.SerializeXml(pIpConfigNode);
}


// ------------------------------------------------------------
int	 CIpConfiguration::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pIpInterfacesListNode=NULL, *pCurInterfaceNode=NULL, *pNetParamsNode=NULL;


	GET_CHILD_NODE(pActionNode, "IP_INTERFACES_LIST", pIpInterfacesListNode);
	if (pIpInterfacesListNode)
	{
		GET_FIRST_CHILD_NODE(pIpInterfacesListNode, "IP_INTERFACE", pCurInterfaceNode);

		int interfaceIndex = 0;
		while( pCurInterfaceNode  &&  (interfaceIndex < MAX_NUM_OF_INTERFACES) )
		{
			CIpInterface ipInterface(m_ipConfigurationStruct.interfacesList[interfaceIndex]);
			nStatus = ipInterface.DeSerializeXml(pCurInterfaceNode,pszError);

			if( STATUS_OK != nStatus )
				return nStatus;

			interfaceIndex++;
			GET_NEXT_CHILD_NODE(pIpInterfacesListNode, "IP_INTERFACE", pCurInterfaceNode);
		}
	}


	GET_CHILD_NODE(pActionNode, "NETWORK_PARAMETERS", pNetParamsNode);
	if (pNetParamsNode)
	{
		CNetworkParameters netParams(m_ipConfigurationStruct.networkParams);
		netParams.DeSerializeXml(pNetParamsNode, pszError);
	}

	return STATUS_OK;
}


// ------------------------------------------------------------
void  CIpConfiguration::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "IpConfiguration::Dump\n"
		<< "---------------------\n";

//	CPObject::Dump(msg);

	for (int i=0; i<MAX_NUM_OF_INTERFACES; i++)
	{
		msg << "IpInterface " << i << ":\n"
			<< "--------------\n";
		CIpInterface ipInterface(m_ipConfigurationStruct.interfacesList[i]);
		ipInterface.Dump(msg);
	}

	msg << CNetworkParameters(m_ipConfigurationStruct.networkParams)
		<< "\n\n";
}


// ------------------------------------------------------------
CIpConfiguration& CIpConfiguration::operator = (const CIpConfiguration &rOther)
{
	memcpy( &m_ipConfigurationStruct,
		    &rOther.m_ipConfigurationStruct,
			sizeof(IP_CONFIGURATION_S) );

	return *this;
}


// ------------------------------------------------------------
IP_INTERFACE_S*  CIpConfiguration::GetIpInterfaceList ()
{
	return m_ipConfigurationStruct.interfacesList;
}


// ------------------------------------------------------------
void CIpConfiguration::SetIpInterfaceList (const IP_INTERFACE_S &ipInterfaceList)
{
	*m_ipConfigurationStruct.interfacesList = ipInterfaceList;
}


// ------------------------------------------------------------
NETWORK_PARAMS_S CIpConfiguration::GetNetworkParams ()
{
	return m_ipConfigurationStruct.networkParams;
}


// ------------------------------------------------------------
void CIpConfiguration::SetNetworkParams (const NETWORK_PARAMS_S netParams)
{
	m_ipConfigurationStruct.networkParams = netParams;
}


// ------------------------------------------------------------
