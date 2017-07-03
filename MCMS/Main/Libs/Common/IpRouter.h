// CIpRouter.h: interface for the CIpRouter class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IpRouter_H_
#define _IpRouter_H_


#include "PObject.h"
#include "McuMngrStructs.h"
#include "psosxml.h"

using namespace std;



class CIpRouter : public CPObject
{

CLASS_TYPE_1(CIpRouter, CPObject)

public:
	CIpRouter ();
	CIpRouter (const IP_ROUTER_S ipRouter);
	virtual ~CIpRouter ();
    void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	virtual void Dump(ostream& msg) const;
	virtual const char* NameOf() const { return "CIpRouter";}
	
	CIpRouter& operator = (const CIpRouter &rOther);

	IP_ROUTER_S  GetIpRouterStruct();
	void         SetIpRouterStruct(CIpRouter ipRouter);


	void         SetRouterIp (const DWORD router_Ip);
	DWORD        GetRouterIp ();

	void         SetRemoteIp (const DWORD remote_Ip);
	DWORD        GetRemoteIp ();

	void         SetRemoteFlag (const BYTE remote_Flag);
	BYTE         GetRemoteFlag ();

	void         SetSubnetMask (const DWORD netMask);
	DWORD        GetSubnetMask ();
	
	void         SetData(IP_ROUTER_S ipRouter);


protected:
	IP_ROUTER_S  m_ipRouterStruct;
};

#endif // _IpRouter_H_
