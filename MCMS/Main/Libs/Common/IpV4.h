// CIPv4.h: interface for the CIPv4 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IPv4_H_
#define _IPv4_H_


#include "PObject.h"
#include "McuMngrStructs.h"
#include "psosxml.h"

using namespace std;



class CIPv4 : public CPObject
{

CLASS_TYPE_1(CIPv4, CPObject)

public:
	CIPv4 ();
	CIPv4 (const IPV4_S iPv4);
	virtual ~CIPv4 ();
	
	virtual const char* NameOf() const { return "CIPv4";}
	
    void  SerializeXml(CXMLDOMElement* pFatherNode);
    int   DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);
	virtual void Dump(ostream&) const;

	CIPv4& operator = (const CIPv4 &rOther);

	IPV4_S GetIpV4Struct();
	void   SetIpV4Struct(CIPv4 iPv4);

	BOOL   GetIsDHCPv4InUse ();
	void   SetIsDHCPv4InUse (const BOOL isInUse);

	DWORD  GetiPv4Address ();
	void   SetiPv4Address (const DWORD address);

	void   SetData(IPV4_S iPv4);


protected:
	IPV4_S m_ipV4Struct;
};

#endif // _IPv4_H_
