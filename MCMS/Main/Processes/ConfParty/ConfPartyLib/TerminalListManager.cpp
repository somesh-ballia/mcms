#include <algorithm>
#include <vector>
#include "NStream.h"
#include "TerminalListManager.h"
#include "Macros.h"
#include "ObjString.h"
#include "SystemFunctions.h"
#include "Trace.h"
#include "DataTypes.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"

class TaskApp;

////////////////////////////////////////////////////////////////////////////
//                        CTerminalNumberingItem
////////////////////////////////////////////////////////////////////////////
CTerminalNumberingItem::CTerminalNumberingItem()
	: m_pParty(0), m_partyNumber(0), m_mcuNumber(0)
{
}

//--------------------------------------------------------------------------
CTerminalNumberingItem::CTerminalNumberingItem(const CTaskApp* partyId, WORD terminalNumber, WORD mcuNumber)
	: m_pParty(partyId), m_partyNumber(terminalNumber), m_mcuNumber(mcuNumber)
{
}

//--------------------------------------------------------------------------
CTerminalNumberingItem::~CTerminalNumberingItem()
{
}

//--------------------------------------------------------------------------
void CTerminalNumberingItem::Create(WORD mcuNumber, WORD partyNumber, const CTaskApp* pParty)
{
	m_partyNumber = partyNumber;
	m_mcuNumber   = mcuNumber;
	m_pParty      = pParty;
}

//--------------------------------------------------------------------------
bool CTerminalNumberingItem::operator ==(const CTerminalNumberingItem& termListItem2) const
{
	return (m_pParty == termListItem2.m_pParty && m_partyNumber == termListItem2.m_partyNumber && m_mcuNumber == termListItem2.m_mcuNumber);
}


////////////////////////////////////////////////////////////////////////////
//                        CTerminalNumberingManager
////////////////////////////////////////////////////////////////////////////
CTerminalNumberingManager::CTerminalNumberingManager()
{
	m_pPartyList = new TERMINALLIST;
	m_pMcuList   = new MCULIST;
}

//--------------------------------------------------------------------------
CTerminalNumberingManager::~CTerminalNumberingManager()
{
	ClearAndDestroy();
}

//--------------------------------------------------------------------------
void CTerminalNumberingManager::insertListItem(WORD partyNumber, WORD mcuNumber, const CTaskApp* pParty)
{
	TRACEINTO << "partyNumber:" << partyNumber << ", mcuNumber:" << mcuNumber << ", partyId:" << pParty;

	CTerminalNumberingItem* pTermListItem = new CTerminalNumberingItem;
	pTermListItem->Create(mcuNumber, partyNumber, pParty);
	m_pPartyList->insert(pTermListItem);
}
//--------------------------------------------------------------------------
void CTerminalNumberingManager::allocatePartyNumber(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber)
{
	if (GetTerminalNumberingItem(pParty) != NULL) // insert only new objects to the list
	{
		TRACESTRFUNC(eLevelError) << "PartyId:" << pParty << " - Failed, party number already exists";
		return;
	}

	terminalNumber = 1;
	mcuNumber      = 1;

	for (TERMINALLIST::iterator itr = m_pPartyList->begin(); itr != m_pPartyList->end(); ++itr, ++terminalNumber)
	{
		if (!CPObject::IsValidPObjectPtr(*itr))
		{
			TRACESTRFUNC(eLevelError) << "PartyId:" << pParty << " - Failed, terminal item is not valid";
			PASSERT(1);
			break;
		}

		if ((*itr)->getPartyNumber() != terminalNumber)
			break;
	}

	insertListItem(terminalNumber, mcuNumber, pParty);
}

//--------------------------------------------------------------------------
STATUS CTerminalNumberingManager::GetPartyTerminalNumber(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber)
{
	for (TERMINALLIST::iterator itr = m_pPartyList->begin(); itr != m_pPartyList->end(); ++itr)
	{
		CTerminalNumberingItem* ptr = *itr;
		if (ptr->getPartyID() == pParty)
		{
			mcuNumber      = ptr->getMcuNumber();
			terminalNumber = ptr->getPartyNumber();
			return STATUS_OK;
		}
	}

	return STATUS_FAIL;
}

//--------------------------------------------------------------------------
CTerminalNumberingItem* CTerminalNumberingManager::GetTerminalNumberingItem(const CTaskApp* pParty)
{
	for (TERMINALLIST::iterator itr = m_pPartyList->begin(); itr != m_pPartyList->end(); ++itr)
	{
		CTerminalNumberingItem* ptr = *itr;
		if (ptr->getPartyID() == pParty)
			return ptr;
	}

	return NULL;
}

//--------------------------------------------------------------------------
CTerminalNumberingItem* CTerminalNumberingManager::Find(CTerminalNumberingItem* pTerminalNumberingItem)
{
	TERMINALLIST::iterator itr = FindPosition(pTerminalNumberingItem);
	return (itr == m_pPartyList->end()) ? NULL : (*itr);
}

//--------------------------------------------------------------------------
STATUS CTerminalNumberingManager::Remove(const CTaskApp* pParty)
{
	RemoveFromMcuList(pParty);

	CTerminalNumberingItem* pErasedTerminalNumberingItem = GetTerminalNumberingItem(pParty);
	if (pErasedTerminalNumberingItem == NULL)
	{
		TRACESTRFUNC(eLevelError) << "PartyId:" << pParty << " - Failed, party was not found";
		return STATUS_FAIL;
	}

	TERMINALLIST::iterator itr = FindPosition(pErasedTerminalNumberingItem);
	if (itr == m_pPartyList->end())
	{
		TRACESTRFUNC(eLevelError) << "PartyId:" << pParty << " - Failed, party was not found";
		return STATUS_FAIL;
	}

	m_pPartyList->erase(itr);
	POBJDELETE(pErasedTerminalNumberingItem);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
TERMINALLIST::iterator CTerminalNumberingManager::FindPosition(const CTerminalNumberingItem* pTerminalNumberingItem)
{
	TERMINALLIST::iterator itr = m_pPartyList->begin();

	while (itr != m_pPartyList->end())
	{
		if (*(*itr) == *pTerminalNumberingItem)
			return itr;
		itr++;
	}
	return itr;
}

//--------------------------------------------------------------------------
void CTerminalNumberingManager::ClearAndDestroy(void)
{
	CTerminalNumberingItem* pErasedTerminalNumberingItem = NULL;

	TERMINALLIST::iterator  itr = m_pPartyList->begin();

	while (itr != m_pPartyList->end())
	{
		pErasedTerminalNumberingItem = (*itr);
		m_pPartyList->erase(itr);
		POBJDELETE(pErasedTerminalNumberingItem);
		itr = m_pPartyList->begin();
	}

	m_pPartyList->clear();
	PDELETE(m_pPartyList);

	MCULIST::iterator it = m_pMcuList->begin();

	while (it != m_pMcuList->end())
	{
		pErasedTerminalNumberingItem = (*it);
		m_pMcuList->erase(it);
		POBJDELETE(pErasedTerminalNumberingItem);
		it = m_pMcuList->begin();
	}

	m_pMcuList->clear();
	PDELETE(m_pMcuList)
}

//--------------------------------------------------------------------------
WORD CTerminalNumberingManager::length()
{
	return m_pPartyList->size();
}

//--------------------------------------------------------------------------
void CTerminalNumberingManager::dump()
{
}

//--------------------------------------------------------------------------
void CTerminalNumberingManager::AllocateMcuNumber(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber)
{
	if (GetTerminalNumberingItemFromMcuList(pParty) != NULL) // insert only new objects to the list
	{
		TRACESTRFUNC(eLevelError) << "PartyId:" << pParty << " - Failed, party number already exists";
		return;
	}

	WORD mcuNumInPartiesList      = 0;
	WORD terminalNumInPartiesList = 0;

	GetPartyTerminalNumber(pParty, mcuNumInPartiesList, terminalNumInPartiesList);

	mcuNumber = 2;

	for (MCULIST::iterator itr = m_pMcuList->begin(); itr != m_pMcuList->end(); ++itr, ++mcuNumber)
	{
		if (!CPObject::IsValidPObjectPtr(*itr))
		{
			TRACESTRFUNC(eLevelError) << "PartyId:" << pParty << " - Failed, terminal item is not valid";
			PASSERT(1);
			break;
		}

		if ((*itr)->getMcuNumber() != mcuNumber)
			break;
	}

	insertListItemToMcuList(terminalNumInPartiesList, mcuNumber, pParty);
}

//--------------------------------------------------------------------------
void CTerminalNumberingManager::insertListItemToMcuList(WORD partyNumber, WORD mcuNumber, const CTaskApp* pParty)
{
	TRACEINTO << "partyNumber:" << partyNumber << ", mcuNumber:" << mcuNumber << ", partyId:" << pParty;

	CTerminalNumberingItem* pTermListItem = new CTerminalNumberingItem;
	pTermListItem->Create(mcuNumber, partyNumber, pParty);
	m_pMcuList->insert(pTermListItem);
}

//--------------------------------------------------------------------------
STATUS CTerminalNumberingManager::GetPartyMcuNumberInMcuList(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber)
{
	for (MCULIST::iterator itr = m_pMcuList->begin(); itr != m_pMcuList->end(); ++itr)
	{
		CTerminalNumberingItem* ptr = *itr;
		if (ptr->getPartyID() == pParty)
		{
			mcuNumber      = ptr->getMcuNumber();
			terminalNumber = ptr->getPartyNumber();
			return STATUS_OK;
		}
	}

	return STATUS_FAIL;
}

//--------------------------------------------------------------------------
CTerminalNumberingItem* CTerminalNumberingManager::GetTerminalNumberingItemFromMcuList(const CTaskApp* pParty)
{
	for (TERMINALLIST::iterator itr = m_pMcuList->begin(); itr != m_pMcuList->end(); ++itr)
	{
		CTerminalNumberingItem* ptr = *itr;
		if (ptr->getPartyID() == pParty)
			return ptr;
	}

	return NULL;
}

//--------------------------------------------------------------------------
CTerminalNumberingItem* CTerminalNumberingManager::FindInMcuList(CTerminalNumberingItem* pTerminalNumberingItem)
{
	TERMINALLIST::iterator itr = FindPositionInMcuList(pTerminalNumberingItem);
	return (itr == m_pMcuList->end()) ? NULL : (*itr);
}

//--------------------------------------------------------------------------
STATUS CTerminalNumberingManager::RemoveFromMcuList(const CTaskApp* pParty)
{
	CTerminalNumberingItem* pErasedTerminalNumberingItem = GetTerminalNumberingItemFromMcuList(pParty);
	if (pErasedTerminalNumberingItem == NULL)
	{
		TRACESTRFUNC(eLevelError) << "PartyId:" << pParty << " - Failed, party was not found";
		return STATUS_FAIL;
	}

	MCULIST::iterator itr = FindPositionInMcuList(pErasedTerminalNumberingItem);
	if (itr == m_pMcuList->end())
	{
		TRACESTRFUNC(eLevelError) << "PartyId:" << pParty << " - Failed, party was not found";
		return STATUS_FAIL;
	}

	TRACEINTO << "PartyId:" << pParty << " - Removed OK";
	m_pMcuList->erase(itr);
	POBJDELETE(pErasedTerminalNumberingItem);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
TERMINALLIST::iterator CTerminalNumberingManager::FindPositionInMcuList(const CTerminalNumberingItem* pTerminalNumberingItem)
{
	TERMINALLIST::iterator itr = m_pMcuList->begin();

	while (itr != m_pMcuList->end())
	{
		if (*(*itr) == *pTerminalNumberingItem)
			return itr;
		itr++;
	}
	return itr;
}
