// DummyEntry.h: interface for the CDummyEntry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(DUMMYENTRY_H__)
#define DUMMYENTRY_H__


#include "SerializeObject.h"

class CDummyEntry : public CSerializeObject  
{
CLASS_TYPE_1(CDummyEntry, CSerializeObject)
public:
	CDummyEntry();
	virtual ~CDummyEntry();
	
	virtual const char* NameOf() const { return "CDummyEntry";}

	
   	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	int    DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action = 0);
	CSerializeObject* Clone() {return new CDummyEntry;}
	

};

#endif // !defined(DUMMYENTRY_H__)
