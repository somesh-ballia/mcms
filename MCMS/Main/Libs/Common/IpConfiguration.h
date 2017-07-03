// IpConfiguration.h: interface for the CIpConfiguration class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IpConfiguration_H_
#define _IpConfiguration_H_


#include "IpInterface.h"
#include "NetworkParameters.h"

using namespace std;



class CIpConfiguration : public CPObject
{

CLASS_TYPE_1(CIpConfiguration, CPObject)

public:
	CIpConfiguration ();
	CIpConfiguration (const IP_CONFIGURATION_S ipConfig);
	virtual ~CIpConfiguration ();
	virtual const char* NameOf() const { return "CIpConfiguration";}
	
	void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	virtual void Dump(ostream& msg) const;


	CIpConfiguration& operator = (const CIpConfiguration &rOther);

	IP_INTERFACE_S*  GetIpInterfaceList ();
	void             SetIpInterfaceList (const IP_INTERFACE_S &ipInterfaceList);

	NETWORK_PARAMS_S GetNetworkParams ();
	void             SetNetworkParams (const NETWORK_PARAMS_S netParams);

	
protected:
	IP_CONFIGURATION_S  m_ipConfigurationStruct;
};

#endif // _IpConfiguration_H_
