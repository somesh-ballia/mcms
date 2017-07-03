#include "ConnToCardManager.h"
#include "Trace.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CConnToCardManager
////////////////////////////////////////////////////////////////////////////
CConnToCardManager::CConnToCardManager()
{
	m_pConnToCardTable = new CSharedMemoryMap<ConnToCardTableEntry>(CONN_TO_CARD_TABLE_NAME, 1, CONN_TO_CARD_TABLE_SIZE);
}

////////////////////////////////////////////////////////////////////////////
CConnToCardManager::~CConnToCardManager()
{
	POBJDELETE(m_pConnToCardTable);
}

////////////////////////////////////////////////////////////////////////////
int CConnToCardManager::Add(ConnToCardTableEntry& Entry)
{
	int status = m_pConnToCardTable->Add(Entry);
	if (status != STATUS_OK)
	{
		CLargeString mstr;
		Entry.DumpRaw(mstr);
		PASSERTSTREAM(1, "Failed to Add entry: (status=" << status << ")\n" << mstr.GetString());

		if (status == STATUS_ENTRY_ALREDY_EXISTS) // entry occupied
		{
			ConnToCardTableEntry oldEntry;
			int getStatus = m_pConnToCardTable->Get(Entry.m_id, oldEntry);
			if (STATUS_OK == getStatus)
			{
				CLargeString mstr1;
				oldEntry.DumpRaw(mstr1);
				TRACESTR(eLevelError) << " entry for this id already exists, old entry: \n" << mstr1.GetString();
			}
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
int CConnToCardManager::Get(DWORD id, ConnToCardTableEntry& Entry)
{
	int status = m_pConnToCardTable->Get(id, Entry);
	if (status != STATUS_OK)
	{
		if (0 != id)
		{
			DBGPASSERT(id);
		}
		else
		{
			DBGPASSERT(10001);
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
int CConnToCardManager::Remove(DWORD id)
{
	int status = m_pConnToCardTable->Remove(id);

	if (status != STATUS_OK)
	{
		if (0 != id)
		{
			PASSERT(id);
		}
		else
		{
			PASSERT(10001);
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
int CConnToCardManager::Update(ConnToCardTableEntry& Entry)
{
	int status = m_pConnToCardTable->Update(Entry);

	if (status != STATUS_OK)
	{
		PASSERT(status);
		//trace???
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CConnToCardManager::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left, std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n" << "CConnToCardManager::Dump\n" << "----------------------------\n\n";

	DWORD passedEntries = 0;
	DWORD i = 0;
	while (i < m_pConnToCardTable->m_pHeader->m_maxEntries && passedEntries < m_pConnToCardTable->m_pHeader->m_numEntries)
	{
		if (m_pConnToCardTable->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			msg << "	Record " << i << "\n" << "-----------------------\n";

			m_pConnToCardTable->m_pEntries[i].Dump(msg);

			passedEntries++;
		}
		i++;
	}

	msg << "\n\n";
}

////////////////////////////////////////////////////////////////////////////
void CConnToCardManager::DumpRaw(const char* calledFrom) const
{
	if (!SHARED_MEMORY_DEBUG_PRINTS)
		return;

	DWORD passedEntries = 0;
	DWORD i = 0;
	CLargeString* mstr = new CLargeString;
	DWORD print_bulk = 25; // print every print_bulk lines
	*mstr << "CConnToCardManager::DumpRaw (called from " << calledFrom << "):\n";
	while (i < m_pConnToCardTable->m_pHeader->m_maxEntries && passedEntries < m_pConnToCardTable->m_pHeader->m_numEntries)
	{
		if (m_pConnToCardTable->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			*mstr << "(" << i << ")  ";
			m_pConnToCardTable->m_pEntries[i].DumpRaw(*mstr);
			*mstr << "\n";
			passedEntries++;
			if (passedEntries == print_bulk)
			{
				PTRACE1(eLevelInfoHigh, mstr->GetString());
				POBJDELETE(mstr);
				print_bulk += 25;
				mstr = new CLargeString;
			}
		}
		i++;
	}
	PTRACE1(eLevelInfoHigh, mstr->GetString());
	POBJDELETE(mstr);
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const CConnToCardManager& obj)
{
	obj.Dump(os);
	return os;
}

////////////////////////////////////////////////////////////////////////////
// Gets the connection id for an entry with a given rsrc type
// (and optionally by resource controller type)
// If no such found : returns 0xFFFFFFFF
// If more than one found, returns the first one.
// For rsrcCntlType : the default value is E_NORMAL, so if it wasn't changed, the
// search would be performed actually only by the rsrcType.
DWORD CConnToCardManager::GetConnIdByRsrcType(eResourceTypes physicalRsrcType, ECntrlType rsrcCntlType) const
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	while (i < m_pConnToCardTable->m_pHeader->m_maxEntries && passedEntries < m_pConnToCardTable->m_pHeader->m_numEntries)
	{
		if (m_pConnToCardTable->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			if (m_pConnToCardTable->m_pEntries[i].physicalRsrcType == physicalRsrcType && m_pConnToCardTable->m_pEntries[i].rsrcCntlType == rsrcCntlType)
			{
				return m_pConnToCardTable->m_pEntries[i].m_id;
			}

			passedEntries++;
		}
		i++;
	}

	return EMPTY_ENTRY;
}

////////////////////////////////////////////////////////////////////////////
DWORD CConnToCardManager::GetConnIdByRsrcTypeAndBId(eResourceTypes physicalRsrcType, ECntrlType rsrcCntlType, WORD BoardId) const
{
	DWORD passedEntries = 0;
	DWORD i = 0;
	while (i < m_pConnToCardTable->m_pHeader->m_maxEntries && passedEntries < m_pConnToCardTable->m_pHeader->m_numEntries)
	{
		if (m_pConnToCardTable->m_pEntries[i].m_id != EMPTY_ENTRY)
		{
			if (m_pConnToCardTable->m_pEntries[i].physicalRsrcType == physicalRsrcType && m_pConnToCardTable->m_pEntries[i].rsrcCntlType == rsrcCntlType && m_pConnToCardTable->m_pEntries[i].boardId == BoardId)
			{
				return m_pConnToCardTable->m_pEntries[i].m_id;
			}

			passedEntries++;
		}
		i++;
	}

	return EMPTY_ENTRY;
}

////////////////////////////////////////////////////////////////////////////
int CConnToCardManager::TestAddUpdateRemove()
{
	ConnToCardTableEntry Entry;

	Entry.m_id = 5;

	int status = m_pConnToCardTable->Add(Entry);

	//status = m_pConnToCardTable->Remove(4);

	Entry.m_id = 6;
	status = m_pConnToCardTable->Add(Entry);

	Entry.m_id = 7;
	status = m_pConnToCardTable->Add(Entry);

	//status = m_pConnToCardTable->Remove(5);
	//status = m_pConnToCardTable->Remove(6);
	Entry.m_id = 10;
	status = m_pConnToCardTable->Add(Entry);

	return status;
}

////////////////////////////////////////////////////////////////////////////
DWORD CConnToCardManager::GetNumOfEntries() const
{
	return m_pConnToCardTable->GetNumOfEntries();
}
