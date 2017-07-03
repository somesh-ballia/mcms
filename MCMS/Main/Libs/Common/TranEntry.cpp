// TransactionEntry.cpp: implementation of the CTranEntry class.
//
//////////////////////////////////////////////////////////////////////

#include "TranEntry.h"
#include "SerializeObject.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CTranEntry::CTranEntry(const char * transName,
					   const char * actionName,
					   CSerializeObject * object,
					   HANDLE_REQUEST handleFunction)
{
	m_pTransName          = transName;
	m_pActionName         = actionName;
	m_pObject             = object;
	m_pRequestfunction    = handleFunction;
}

CTranEntry::~CTranEntry()
{
	POBJDELETE(m_pObject);
}
