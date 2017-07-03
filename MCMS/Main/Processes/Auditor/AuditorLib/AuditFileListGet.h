// AuditFileListGet.h: interface for the CAuditFileListGet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__AUDIT_FILE_LIST_GET_H__)
#define __AUDIT_FILE_LIST_GET_H__


#include "psosxml.h"
#include "SerializeObject.h"



/////////////////////////////////////////////////////////
class CAuditFileListGet : public CSerializeObject
{
CLASS_TYPE_1(CAuditFileListGet, CSerializeObject)
public:

	//Constructors
	CAuditFileListGet();
	CAuditFileListGet(const CAuditFileListGet &other);
	CAuditFileListGet& operator = (const CAuditFileListGet& other);
	virtual ~CAuditFileListGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError);
	CSerializeObject* Clone() {return new CAuditFileListGet;}

	const char * NameOf() const {return "CAuditFileListGet";}
	
  
protected:

	
};


#endif // !defined(__AUDIT_FILE_LIST_GET_H__)

