#ifndef COMMRESRSRVDBACTION_H_
#define COMMRESRSRVDBACTION_H_

#include "SerializeObject.h"

class CCommResRsrvDBAction : public CSerializeObject
{
CLASS_TYPE_1(CCommResRsrvDBAction, CSerializeObject)
public:

	//Constructors
	CCommResRsrvDBAction();
	CCommResRsrvDBAction(const CCommResRsrvDBAction &other);
	CCommResRsrvDBAction& operator = (const CCommResRsrvDBAction& other);
	virtual ~CCommResRsrvDBAction();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CCommResRsrvDBAction();}
		
	const char * NameOf() const {return "CCommResRsrvDBAction";}

	DWORD       GetConfID()const{return m_confID;}
protected:

	DWORD       m_confID;
};

#endif 
