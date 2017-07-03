// LogFileListGet.h: interface for the CLogFileListGet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGFILELISTGET__)
#define _LOGFILELISTGET__


#include "psosxml.h"
#include "SerializeObject.h"



/////////////////////////////////////////////////////////
class CLogFileListGet : public CSerializeObject
{
CLASS_TYPE_1(CLogFileListGet, CSerializeObject)
public:

	//Constructors
	CLogFileListGet();
	virtual const char* NameOf() const { return "CLogFileListGet";}
	CLogFileListGet(const CLogFileListGet &other);
	CLogFileListGet& operator = (const CLogFileListGet& other);
	virtual ~CLogFileListGet();


	void   SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action);
	int    DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError);
	CSerializeObject* Clone() {return new CLogFileListGet;}

	
  
protected:

	
};


#endif // !defined(_LOGFILELISTGET__)

