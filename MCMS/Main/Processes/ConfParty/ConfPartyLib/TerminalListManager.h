#ifndef TERMINALLISTMANAGER_H__
#define TERMINALLISTMANAGER_H__

#include <set>

class CTerminalNumberingManager;
class CMcuListMng;
class CTaskApp;


////////////////////////////////////////////////////////////////////////////
//                        CTerminalNumberingItem
////////////////////////////////////////////////////////////////////////////
class CTerminalNumberingItem : public CPObject
	{
	CLASS_TYPE_1(CTerminalNumberingItem,CPObject)

	const CTaskApp*         m_pParty;
	WORD                    m_partyNumber;
	WORD                    m_mcuNumber;

public:
	                        CTerminalNumberingItem();
	                        CTerminalNumberingItem(const CTaskApp* partyId, WORD terminalNumber, WORD mcuNumber);
	                       ~CTerminalNumberingItem();
	virtual const char*     NameOf() const { return "CTerminalNumberingItem"; }

	bool                    operator==(const CTerminalNumberingItem & termListItem2) const;

	void                    Create(WORD mcuNumber, WORD partyNumber, const CTaskApp* pParty);
	const CTaskApp*         getPartyID() const        { return m_pParty; }
	WORD                    getPartyNumber() const    { return m_partyNumber; }
	WORD                    getMcuNumber() const      { return m_mcuNumber; }
	void                    setMcuNumber(WORD mcuNum) { m_mcuNumber = mcuNum; }
};

struct LessByTerminalNumber
{
	bool operator()(const CTerminalNumberingItem* p1, const CTerminalNumberingItem* p2) const
	{
		return p1->getPartyNumber() < p2->getPartyNumber();
	}
};

struct LessByMcuNumber
{
	bool operator()(const CTerminalNumberingItem* p1, const CTerminalNumberingItem* p2) const
	{
		return p1->getMcuNumber() < p2->getMcuNumber();
	}
};

typedef std::set<CTerminalNumberingItem*, LessByTerminalNumber> TERMINALLIST;
typedef std::set<CTerminalNumberingItem*, LessByMcuNumber> MCULIST;


////////////////////////////////////////////////////////////////////////////
//                        CTerminalNumberingManager
////////////////////////////////////////////////////////////////////////////
class CTerminalNumberingManager : public CPObject
{
	CLASS_TYPE_1(CTerminalNumberingManager,CPObject)

	TERMINALLIST*           m_pPartyList;
	MCULIST*                m_pMcuList;

public:
	                        CTerminalNumberingManager();
	                       ~CTerminalNumberingManager();
	virtual const char*     NameOf() const { return "CTerminalNumberingManager"; }
	STATUS                  Remove(const CTaskApp* pParty);
	void                    dump();
	WORD                    length();
	TERMINALLIST*           getPartyListPtr() { return m_pPartyList; }
	void                    ClearAndDestroy(void);

	void                    allocatePartyNumber(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber);
	void                    insertListItem(WORD partyNumber, WORD mcuNumber, const CTaskApp* pParty);
	STATUS                  GetPartyTerminalNumber(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber);
	CTerminalNumberingItem* GetTerminalNumberingItem(const CTaskApp* pParty);
	TERMINALLIST::iterator  FindPosition(const CTerminalNumberingItem* pTerminalNumberingItem);
	CTerminalNumberingItem* Find(CTerminalNumberingItem* pTerminalNumberingItem);

	void                    AllocateMcuNumber(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber);
	void                    insertListItemToMcuList(WORD partyNumber, WORD mcuNumber, const CTaskApp* pParty);
	STATUS                  GetPartyMcuNumberInMcuList(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber);
	CTerminalNumberingItem* GetTerminalNumberingItemFromMcuList(const CTaskApp* pParty);
	CTerminalNumberingItem* FindInMcuList(CTerminalNumberingItem* pTerminalNumberingItem);
	STATUS                  RemoveFromMcuList(const CTaskApp* pParty);
	MCULIST::iterator       FindPositionInMcuList(const CTerminalNumberingItem* pTerminalNumberingItem);
};

#endif // TERMINALLISTMANAGER_H__  
