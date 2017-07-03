#ifndef __RENAME_OPERATOR_H_
#define __RENAME_OPERATOR_H_

#include "SerializeObject.h"


class CXMLDOMElement;
class COperator;


class CRenameOperator : public CSerializeObject 
{

public:

	CRenameOperator();

	CRenameOperator(const CRenameOperator &other);

	~CRenameOperator();

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CRenameOperator(*this);}

	const char * NameOf() const {return "CRenameOperator";}

	COperator* GetOperator() { return m_pOperator; }


private:

	COperator* m_pOperator;
};

#endif //__RENAME_OPERATOR_H_
