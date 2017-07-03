#if !defined(AFX_CONNTOCARDMANAGER_H__CA055936_3C61_4B11_B772_E6F65E268445__INCLUDED_)
#define AFX_CONNTOCARDMANAGER_H__CA055936_3C61_4B11_B772_E6F65E268445__INCLUDED_

#include "SharedHeader.h"
#include "SharedMemory.h"
#include "SharedMemoryMap.h"
#include "Trace.h"
#include "AllocateStructs.h"
#include "PObject.h"

////////////////////////////////////////////////////////////////////////////
//                        CConnToCardManager
////////////////////////////////////////////////////////////////////////////
class CConnToCardManager : public CPObject
{
	CLASS_TYPE_1(CConnToCardManager, CPObject)

public:
	friend std::ostream& operator<<(std::ostream& os, const CConnToCardManager& obj);

	            CConnToCardManager();
	           ~CConnToCardManager();
	const char* NameOf() const { return "CConnToCardManager";}

	//***envelope functions for shared memory
	int         Add(ConnToCardTableEntry& pEntry);
	int         Remove(DWORD id);
	int         Update(ConnToCardTableEntry& pEntry);
	int         Get(DWORD id, ConnToCardTableEntry& Entry);
	DWORD       GetConnIdByRsrcType(eResourceTypes physicalRsrcType, ECntrlType rsrcCntlType = E_NORMAL) const;
	DWORD       GetConnIdByRsrcTypeAndBId( eResourceTypes physicalRsrcType, ECntrlType rsrcCntlType, WORD BoardId) const;

	void        Dump(std::ostream& msg) const;
	void        DumpRaw(const char* calledFrom = "Self") const;

	int         TestAddUpdateRemove();

	DWORD       GetNumOfEntries() const;
	//***envelope functions for shared memory

private:
	CSharedMemoryMap<ConnToCardTableEntry>* m_pConnToCardTable;
};

#endif // !defined(AFX_CONNTOCARDMANAGER_H__CA055936_3C61_4B11_B772_E6F65E268445__INCLUDED_)
