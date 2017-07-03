// IpInterface.h: interface for the CIpInterface class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IpInterface_H_
#define _IpInterface_H_


#include "IpV4.h"
#include "IpV6.h"



class CIpInterface : public CPObject
{

CLASS_TYPE_1(CIpInterface, CPObject)

public:
	CIpInterface ();
	CIpInterface (const IP_INTERFACE_S ipInterface);
	virtual ~CIpInterface ();
    void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	virtual void Dump(std::ostream& msg) const;
	virtual const char* NameOf() const { return "CIpInterface";}
	CIpInterface& operator = (const CIpInterface &rOther);


	IPV4_S    GetIPv4 ();
	void      SetIPv4 (const IPV4_S iPv4);

	IPV6_S    GetIPv6 (const int idx);
	void      SetIPv6 (const int idx, const IPV6_S iPv6);

	eIpType   GetIpType ();
	void      SetIpType (const eIpType type);
	
	ALIAS_S*  GetAliasesList ();

	DWORD     GetBoardId ();
	void      SetBoardId (const DWORD id);

	DWORD     GetPqId ();
	void      SetPqId (const DWORD id);
	
	BOOL 	  GetIsSecured();
	void 	  SetIsSecured(BOOL isSecured);
	
	BOOL 	  GetIsPermanentNetworkOpen();
	void 	  SetIsPermanentNetworkOpen(BOOL isPermanentOpen);

	void      SetData(IP_INTERFACE_S ipInteface);


protected:
	IP_INTERFACE_S m_ipInterfaceStruct;
};

#endif // _IpInterface_H_
