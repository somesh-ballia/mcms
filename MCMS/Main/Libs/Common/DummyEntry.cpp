// DummyEntry.cpp: implementation of the CDummyEntry class.
//
//////////////////////////////////////////////////////////////////////

#include "DummyEntry.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDummyEntry::CDummyEntry()
{
	
}

CDummyEntry::~CDummyEntry()
{

}

void CDummyEntry::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

int CDummyEntry::DeSerializeXml(CXMLDOMElement *pActionNode,
								char *pszError,
								const char* action)
{
	return 0;
}

