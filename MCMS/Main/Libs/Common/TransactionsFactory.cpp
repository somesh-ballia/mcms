// TransactionsFactory.cpp: implementation of the CTransactionsFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "SerializeObject.h"
#include "TransactionsFactory.h"
#include "TranEntry.h"
#include "Trace.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CTransactionsFactory::CTransactionsFactory()
{

}

//////////////////////////////////////////////////////////////////////
CTransactionsFactory::~CTransactionsFactory()
{
	while (!m_transList.empty())
	{
		delete m_transList.front();
		m_transList.pop_front();
	}
}

//////////////////////////////////////////////////////////////////////
CSerializeObject * CTransactionsFactory::CreateTransaction(const char * transName,
													       const char * actionName) const
{
	std::list<CTranEntry*>::const_iterator iter;
	iter = m_transList.begin();
	while (iter != m_transList.end())
	{

		const CTranEntry * temp = *iter;
		if ((strcmp(transName,temp->m_pTransName) == 0) &&
			(strcmp(actionName,temp->m_pActionName) == 0))
		{
			if (temp->m_pObject == NULL)
				return NULL;

			CSerializeObject * res = temp->m_pObject->Clone();
			res->SetRequestFunction(temp->m_pRequestfunction);
			return res;
		}
		iter++;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////
void CTransactionsFactory::AddTransaction(const char * transName,
										  const char * actionName,
										  CSerializeObject * object,
										  HANDLE_REQUEST handleFunction)
{
	if (!object || !transName || !actionName)
	{
		PASSERTMSG(1,"Parameters can't be NULL");
		return;
	}

	m_transList.push_back(new CTranEntry(transName,
                                         actionName,
                                         object,
                                         handleFunction));
}

