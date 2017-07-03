#if !defined(_RvgwAliasName_H__)
#define _RvgwAliasName_H__

#include "SerializeObject.h"


class CXMLDOMElement;

class CRvgwAliasName : public CSerializeObject
{
public:

	CRvgwAliasName();

	CRvgwAliasName(const CRvgwAliasName &other);

	CRvgwAliasName& operator= (const CRvgwAliasName &other);

	~CRvgwAliasName();

	const char * NameOf(void) const {return "CRvgwAliasName";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CRvgwAliasName(*this);}

	std::string& getAliasName(){return m_strAliasName;};

private:

	std::string m_strAliasName;

};

#endif
