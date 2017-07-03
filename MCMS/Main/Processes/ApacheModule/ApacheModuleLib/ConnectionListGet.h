#include "SerializeObject.h"


class CXMLDOMElement;


class CConnectionListGet : public CSerializeObject 

{

public:

	CConnectionListGet();

	CConnectionListGet(const CConnectionListGet &other);

	CConnectionListGet& operator= (const CConnectionListGet &other);

	~CConnectionListGet();

	const char * NameOf(void) const {return "CConnectionListGet";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CConnectionListGet(*this);}

};
