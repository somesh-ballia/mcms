// FileListGet.h: interface for the CFileListGet class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for getting files List 
//========   ==============   =====================================================================


#if !defined(_FILELISTGET_H__)
#define _FILELISTGET_H__

#include "SerializeObject.h"
#include "FileList.h"
//#include "Transactions.h"
#include "ConfPartyApiDefines.h"


class CFileListGet : public CSerializeObject
{
CLASS_TYPE_1(CFileListGet, CSerializeObject)
public:

	//Constructors
	CFileListGet();
	virtual const char* NameOf() const { return "CFileListGet";}
	CFileListGet(const CFileListGet &other);
	CFileListGet& operator = (const CFileListGet& other);
	virtual ~CFileListGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int action);
	CSerializeObject* Clone() {return new CFileListGet;}

		
    
	char * GetPath();
	CFileList* GetFileList();
	
protected:

	char 	m_Path[MAX_FULL_PATH_LEN];
	CFileList* m_FileList;
	

};

#endif // !defined(_FILELISTGET_H__)

