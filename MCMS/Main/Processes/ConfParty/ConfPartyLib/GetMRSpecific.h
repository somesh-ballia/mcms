#ifndef _GET_MR_SPECIFIC_H_
#define _GET_MR_SPECIFIC_H_

#include "SerializeObject.h"

class CGetMRSpecific : public CSerializeObject
{
CLASS_TYPE_1(CGetMRSpecific, CSerializeObject)
public:

	//Constructors
	CGetMRSpecific();
	virtual const char* NameOf() const { return "CGetMRSpecific";}
	CGetMRSpecific(const CGetMRSpecific &other);
	CGetMRSpecific& operator = (const CGetMRSpecific & other);
	virtual ~CGetMRSpecific();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CGetMRSpecific();}
		
    
	DWORD       GetMRID()const;
protected:
	DWORD       m_MRID;
};

#endif 
