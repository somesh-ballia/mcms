
////////////////////////////////////////////////////////////////////////////////////////
//
//  Date          Created By         Description
//
//  11/9/2012	    Judith		 wrapper class for Get-SystemInterfaceList
//  =========   ==============   ========================================================
///////////////////////////////////////////////////////////////////////////////////////

#ifndef __CSystemInterfaceListGet_
#define __CSystemInterfaceListGet_


#include "SerializeObject.h"



class CSystemInterfaceListGet : public CSerializeObject
{
CLASS_TYPE_1(CSystemInterfaceListGet, CSerializeObject)
public:

	//Constructors
	CSystemInterfaceListGet();
	CSystemInterfaceListGet(const CSystemInterfaceListGet &other);
	CSystemInterfaceListGet& operator = (const CSystemInterfaceListGet& other);
	virtual ~CSystemInterfaceListGet();
	const char * NameOf(void) const {return "CSystemInterfaceListGet";}

	virtual void SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int		DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject* Clone() {return new CSystemInterfaceListGet;}

  	
 };

#endif //__CSystemInterfaceListGet_
