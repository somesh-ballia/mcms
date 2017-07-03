#include "SerializeObject.h"
//#include <string>


class CXMLDOMElement;

class CRename : public CSerializeObject 
{
public:

	CRename();

	CRename(const CRename &other);

	CRename& operator= (const CRename &other);

	~CRename();

	const char * NameOf(void) const {return "CRename";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CRename(*this);}

	
	std::string GetInitialVirtualPath();
	std::string GetNewVirtualPath();	
	
private:

	std::string m_strInitialVirtualPath;
	std::string m_strNewVirtualPath;
};
