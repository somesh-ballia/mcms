// IVRServiceDel.h: interface for the CIVRServiceDel class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Delete XML IVR Service
//========   ==============   =====================================================================


#if !defined(_IVRSERVICEDEL_H__)
#define _IVRSERVICEDEL_H__

#include "SerializeObject.h"
#include "IVRService.h"



class CIVRServiceDel : public CSerializeObject
{
CLASS_TYPE_1(CIVRServiceDel, CSerializeObject)
public:

	//Constructors
	CIVRServiceDel();
	virtual const char* NameOf() const { return "CIVRServiceDel";}
	CIVRServiceDel(const CIVRServiceDel &other);
	CIVRServiceDel& operator = (const CIVRServiceDel& other);
	virtual ~CIVRServiceDel();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CIVRServiceDel;}

	int convertStrActionToNumber(const char * strAction);
		
    
	const char * GetIVRServiceName();
	
	///anat-test
	virtual void TestAnat(){}
	///anat-test
  
protected:

	char 	m_ivrServiceName[H243_NAME_LEN];
	int 	m_NumAction;
	
	

};

#endif // !defined(_IVRSERVICEDEL_H__)

