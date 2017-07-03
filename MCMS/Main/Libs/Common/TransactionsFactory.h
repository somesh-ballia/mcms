// TransactionsFactory.h: interface for the CTransactionsFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TRANSACTIONSFACTORY_H__)
#define _TRANSACTIONSFACTORY_H__

#include <list>
#include "PObject.h"


class CSerializeObject;
class CRequestHandler;
class CRequest;
class CTranEntry;

typedef STATUS (CRequestHandler::*HANDLE_REQUEST)(CRequest*);

class CTransactionsFactory : public CPObject
{
CLASS_TYPE_1(CTransactionsFactory,CPObject )	
public:
	CTransactionsFactory();
	virtual ~CTransactionsFactory();
	virtual const char* NameOf() const { return "CTransactionsFactory"; }

	CSerializeObject * CreateTransaction(const char * transName,
									 const char * actionName) const;

	void AddTransaction(const char * transName,
                           const char * actionName,
                           CSerializeObject * object,
                           HANDLE_REQUEST handleFunction);

private:	

	std::list<CTranEntry*> m_transList;
};

#endif // !defined(_TRANSACTIONSFACTORY_H__)
