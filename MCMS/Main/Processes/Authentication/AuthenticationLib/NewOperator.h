#include "SerializeObject.h"


class CXMLDOMElement;
class COperator;


class CNewOperator : public CSerializeObject 
{

public:

	CNewOperator();

	CNewOperator(std::string user, std::string pwd);

	CNewOperator(const CNewOperator &other);

	~CNewOperator();

	const char * NameOf(void) const {return "CNewOperator";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CNewOperator(*this);}


	COperator* GetOperator() { return m_pOperator; }


private:

	COperator* m_pOperator;
};
