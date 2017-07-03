// TranEntry.h: interface for the CTranEntry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TRANSACTIONENTRY_H__)
#define _TRANSACTIONENTRY_H__

#include "RequestHandler.h"
#include "DataTypes.h"

class CSerializeObject;
class CMonitorTask;

class CTranEntry  
{
public:
	
	CTranEntry(const char * transName,
               const char * actionName,
               CSerializeObject * object,
               HANDLE_REQUEST handleFunction);

	virtual ~CTranEntry();

	const char * m_pTransName;
	const char * m_pActionName;
	CSerializeObject * m_pObject;
	HANDLE_REQUEST m_pRequestfunction;

};

#endif // !defined(_TRANSACTIONENTRY_H__)
