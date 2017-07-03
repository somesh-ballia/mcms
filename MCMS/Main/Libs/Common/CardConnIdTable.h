#ifndef CARDCONNIDTABLE_H_
#define CARDCONNIDTABLE_H_

#include "PObject.h"
#include "SharedDefines.h"


#define DEFAULT_CONN_ID								0xff
#define CONN_ID_EXISTED_BEFORE						0xfe
#define CONN_ID_EXISTED_BEFORE_AND_ASSERT_PRODUCED	0xfd
#define CONN_ID_EXISTED_BEFORE_IN_UPGRADE_PROCESS   0xfc

class CCardOrCsConnIdTable : public CPObject
{
CLASS_TYPE_1(CCardOrCsConnIdTable, CPObject)	
public:
	CCardOrCsConnIdTable();
	virtual ~CCardOrCsConnIdTable();
	
	virtual const char* NameOf() const { return "CCardOrCsConnIdTable";}
	virtual void Dump(std::ostream&) const;
	
	WORD GetConnectionId(DWORD id);
	void UpdateCardOrCs2ConnectionId(DWORD id, WORD newConId);
	void CloseConnection(const WORD conId);
	void CloseConnectionNoFault(const WORD conId);
	/*m_boardIdDisconnectionsCounter[i] - counter for disconnection in card i */
	int m_boardIdDisconnectionsCounter[MAX_NUM_OF_BOARDS];
	
private:	
	// disabled
	CCardOrCsConnIdTable(const CCardOrCsConnIdTable&);
	CCardOrCsConnIdTable&operator=(const CCardOrCsConnIdTable&);
	void UpdateDisconnetcionCounters(const WORD boardId);


	WORD  m_cardOrCs2ConnectionId[MAX_NUM_OF_BOARDS];
};

#endif /*CARDCONNIDTABLE_H_*/
