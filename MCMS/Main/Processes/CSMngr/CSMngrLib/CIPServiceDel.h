////////////////////////////////////////////////////////////////////////////////////////
//
//   Date         Created By         Description
//
//  11/7/2005	    ERAN		wrapper class for Delete-IPService	   
//  ========   ==============   ========================================================
///////////////////////////////////////////////////////////////////////////////////////

#ifndef _CIPServiceListGet_   
#define _CIPServiceListGet_


#include "psosxml.h"
#include "SerializeObject.h"
#include "IpService.h"
#include <string>

class CIPServiceDel : public CSerializeObject
{
CLASS_TYPE_1(CIPServiceDel, CSerializeObject)
public:

	//Constructors
	CIPServiceDel();
	CIPServiceDel(const CIPServiceDel &other);
	virtual ~CIPServiceDel();

	const char * NameOf(void) const {return "CIPServiceDel";}

	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	int	 DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	CSerializeObject*            Clone(){return new CIPServiceDel;}
	const char*       GetIPServiceName() const {return m_IPServiceName.c_str();}
	void SetIPServiceName(const char *name){m_IPServiceName = name;}
//protected:


private:
	std::string m_IPServiceName;
};

#endif 
