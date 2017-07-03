// IVRServiceSetDefault.h: interface for the CIVRServiceSetDefault class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Setting Default IVR Service 
//========   ==============   =====================================================================


#if !defined(_IVRSERVICESETDEFAULT_H__)
#define _IVRSERVICESETDEFAULT_H__

#include "SerializeObject.h"
#include "IVRService.h"



class CIVRServiceSetDefault : public CSerializeObject
{
CLASS_TYPE_1(CIVRServiceSetDefault, CSerializeObject)
public:

	//Constructors
	CIVRServiceSetDefault();
	virtual const char* NameOf() const { return "CIVRServiceSetDefault";}
	CIVRServiceSetDefault(const CIVRServiceSetDefault &other);
	CIVRServiceSetDefault& operator = (const CIVRServiceSetDefault& other);
	virtual ~CIVRServiceSetDefault();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CIVRServiceSetDefault;}

	int convertStrActionToNumber(const char * strAction);
		
    
	const char * GetIVRServiceName();
	
protected:

	char 	m_ivrServiceName[H243_NAME_LEN];
	int 	m_NumAction;
	
	

};

#endif // !defined(_IVRSERVICESETDEFAULT_H__)

