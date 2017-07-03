

#include "SerializeObject.h"
#include "FileList.h"


class CXMLDOMElement;

class CFileListGet : public CSerializeObject 
{
public:

	CFileListGet();

	CFileListGet(const CFileListGet &other);

	CFileListGet& operator= (const CFileListGet &other);

	~CFileListGet();

	const char * NameOf(void) const {return "CFileListGet";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CFileListGet(*this);}

	
	int FillFileList(WORD bNested);
	
private:

	std::string m_strVirtualPath;
	std::string m_strPhysicalPath;	
	CFileList *m_pFileList;
};
