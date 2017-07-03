// IVRLanguageAdd.h: interface for the CIVRLanguageAdd class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Add IVR Language
//========   ==============   =====================================================================


#if !defined(_IVRIVRLANGUAGEADD_H__)
#define _IVRIVRLANGUAGEADD_H__

#include "SerializeObject.h"
#include "IVRService.h"



class CIVRLanguageAdd : public CSerializeObject
{
CLASS_TYPE_1(CIVRLanguageAdd, CSerializeObject)
public:

	//Constructors
	CIVRLanguageAdd();
	virtual const char* NameOf() const { return "CIVRLanguageAdd";}
	CIVRLanguageAdd(const CIVRLanguageAdd &other);
	CIVRLanguageAdd& operator = (const CIVRLanguageAdd& other);
	virtual ~CIVRLanguageAdd();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CIVRLanguageAdd;}

	int convertStrActionToNumber(const char * strAction);
		
    
	const char * GetIVRLanguageName();
	void SetIVRLanguageName(char* langName);
	
protected:

	char 	m_ivrLanguageName[LANGUAGE_NAME_LEN];
	int 	m_NumAction;
	
	

};

#endif // !defined(_IVRIVRLANGUAGEADD_H__)

