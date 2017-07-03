
////////////////////////////////////////////////////////////////////////////////////////
//
//  Date         Created By         Description
//
//  6/7/2005	    ERAN		wrapper class for Get-IPService	   
//  ========   ==============   ========================================================
///////////////////////////////////////////////////////////////////////////////////////

#ifndef _CIPServiceListGet_   
#define _CIPServiceListGet_


#include "psosxml.h"
#include "SerializeObject.h"
#include "IpService.h"



class CIPServiceListGet : public CSerializeObject
{
CLASS_TYPE_1(CIPServiceListGet, CSerializeObject)
public:

	//Constructors
	CIPServiceListGet();
	CIPServiceListGet(const CIPServiceListGet &other);
	CIPServiceListGet& operator = (const CIPServiceListGet& other);
	virtual ~CIPServiceListGet();
	const char * NameOf(void) const {return "CIPServiceListGet";}

	virtual void SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CIPServiceListGet;}

  	
 };

#endif 
