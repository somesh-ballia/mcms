
#include "SerializeObject.h"

class CXMLDOMElement;

class CCreateRemoveDir : public CSerializeObject 
{
public:

	CCreateRemoveDir();

	CCreateRemoveDir(const CCreateRemoveDir &other);

	CCreateRemoveDir& operator= (const CCreateRemoveDir &other);

	~CCreateRemoveDir();
	const char * NameOf(void) const {return "CCreateRemoveDir";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CCreateRemoveDir(*this);}

	
	std::string GetVirtualPath();
	
private:

	std::string m_strVirtualPath;
};
