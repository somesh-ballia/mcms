#ifndef _GET_PROFILE_SPECIFIC_H_
#define _GET_PROFILE_SPECIFIC_H_

#include "SerializeObject.h"

class CGetProfileSpecific : public CSerializeObject
{
CLASS_TYPE_1(CGetProfileSpecific, CSerializeObject)
public:

	//Constructors
	CGetProfileSpecific();
	virtual const char* NameOf() const { return "CGetProfileSpecific";}
	CGetProfileSpecific(const CGetProfileSpecific &other);
	CGetProfileSpecific& operator = (const CGetProfileSpecific & other);
	virtual ~CGetProfileSpecific();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CGetProfileSpecific();}
		
    
	DWORD       GetConfID()const;
protected:
	DWORD       m_ConfID;
};

#endif 
