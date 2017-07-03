// CIPv6.h: interface for the CIPv4 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IPv6_H_
#define _IPv6_H_


#include "PObject.h"
#include "McuMngrStructs.h"
#include "psosxml.h"

using namespace std;



class CIPv6 : public CPObject
{

CLASS_TYPE_1(CIPv6, CPObject)

public:
	CIPv6 ();
	CIPv6 (const IPV6_S iPv6);
	virtual ~CIPv6 ();
    void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	virtual void Dump(ostream& msg) const;

	virtual const char* NameOf() const { return "CIPv6";}
	
	CIPv6& operator = (const CIPv6 &rOther);

	IPV6_S                GetIpV6Struct();
	void                  SetIpV6Struct(CIPv6 iPv6);

	eV6ConfigurationType  GetConfigurationType ();
	void                  SetConfigurationType (const eV6ConfigurationType type);

	BYTE*                 GetiPv6Address ();
	void                  SetiPv6Address (const BYTE *address);

	DWORD                 Get6To4RelayAddress ();
	void                  Set6To4RelayAddress (const DWORD address);
	
	DWORD                 GetIpV6SubnetMask ();
	void                  SetIpV6SubnetMask (const DWORD subnetMask);

	void                  SetData(IPV6_S iPv6);


protected:
	IPV6_S                m_ipV6Struct;
};

#endif // _IPv6_H_
