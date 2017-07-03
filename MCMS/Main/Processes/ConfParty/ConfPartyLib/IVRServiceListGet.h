// CIVRServiceListGet.h: interface for the CIVRServiceListGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Get XML IVR List
//========   ==============   =====================================================================


#if !defined(_IVRSERVICELISTGET_H__)
#define _IVRSERVICELISTGET_H__


#include "psosxml.h"
#include "SerializeObject.h"
#include "CommConfDB.h"


class CIVRServiceListGet : public CSerializeObject
{
CLASS_TYPE_1(CIVRServiceListGet, CSerializeObject)
public:

	//Constructors
	CIVRServiceListGet();
	virtual const char* NameOf() const { return "CIVRServiceListGet";}
	CIVRServiceListGet(const CIVRServiceListGet &other);
	CIVRServiceListGet& operator = (const CIVRServiceListGet& other);
	virtual ~CIVRServiceListGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError);
	CSerializeObject* Clone() {return new CIVRServiceListGet;}

	//int convertStrActionToNumber(const char * strAction);

	
  
protected:

	

};

#endif // !defined(_IVRSERVICELISTGET_H__)

