#ifndef COMMRESDBACTION_H_
#define COMMRESDBACTION_H_

#include "SerializeObject.h"

class CCommResDBAction : public CSerializeObject
{
CLASS_TYPE_1(CCommResDBAction, CSerializeObject)
public:

	//Constructors
	CCommResDBAction();
	CCommResDBAction(const CCommResDBAction &other);
	CCommResDBAction& operator = (const CCommResDBAction& other);
	virtual ~CCommResDBAction();
	
	virtual const char* NameOf() const { return "CCommResDBAction";}

	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError, const char* action);
	CSerializeObject* Clone() {return new CCommResDBAction();}
		
	const char * GetEqName() const {return m_transitEQ;}
	DWORD       GetConfID()const{return m_confID;}
protected:
	char m_transitEQ[H243_NAME_LEN];
	DWORD       m_confID;
};

#endif /*COMMRESDBACTION_H_*/
