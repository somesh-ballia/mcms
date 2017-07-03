// NetworkParameters.h: interface for the CNetworkParameters class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _NetworkParameters_H_
#define _NetworkParameters_H_


#include "PObject.h"
#include "SharedMcmsCardsStructs.h"
#include "CsStructs.h"
#include "psosxml.h"

using namespace std;



class CNetworkParameters : public CPObject
{

CLASS_TYPE_1(CNetworkParameters, CPObject)

public:
	CNetworkParameters ();
	CNetworkParameters (const NETWORK_PARAMS_S netParams);
	virtual ~CNetworkParameters ();
    void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	virtual void Dump(ostream& msg) const;

	virtual const char* NameOf() const { return "CNetworkParameters";}
	
	CNetworkParameters& operator = (const CNetworkParameters &rOther);

	NETWORK_PARAMS_S GetNetworkParametersStruct();

	BOOL        GetIsDhcpInUse (); // TRUE/FALSE
	void        SetIsDhcpInUse (const BOOL isInUse);

	DWORD       GetDhcpServerIpAddress ();
	void        SetDhcpServerIpAddress (const DWORD ipAdd);

	eDHCPState  GetDhcpState ();
	void        SetDhcpState (const eDHCPState state);

	DWORD       GetSubnetMask ();
	void        SetSubnetMask (const DWORD subMask);

	DWORD       GetDefaultGatewayIPv4 ();
	void        SetDefaultGatewayIPv4 (const DWORD defGw);

    void        SetDefaultGatewayIPv6(const char* defaultGateway);
    char*       GetDefaultGatewayIPv6() const;

    DWORD       GetVLanMode ();
	void        SetVLanMode (const DWORD mode);

	DWORD       GetVLanId ();
	void        SetVLanId (const DWORD id);

	void        SetData(NETWORK_PARAMS_S netParams);


protected:
	NETWORK_PARAMS_S m_networkParametersStruct;
};

#endif // _NetworkParameters_H_
