#ifndef APIPROCESS_H_
#define APIPROCESS_H_

#include "ProcessBase.h"
#include "CardConnIdTable.h"


class CApiProcess : public CProcessBase
{
CLASS_TYPE_1(CApiProcess,CProcessBase )	
public:
	CApiProcess();
	virtual ~CApiProcess();
	
	virtual const char* NameOf() const { return "CApiProcess";}
	WORD  GetConnectionId(DWORD id);
	void  UpdateCardOrCs2ConnectionId(DWORD id, WORD conId);
	void CloseConnection(const WORD conId);
	
	virtual void DumpProcessStatistics(std::ostream& answer, CTerminalCommand & command)const;
	
	DWORD GetCntMcmsToCS(){return m_CntMcmsToCsMfa;}
	DWORD GetCntCSToMcms(){return m_CntCsMfaToMcms;}
	
	void IncrementCntMcmsToCsMfa(){m_CntMcmsToCsMfa++;}
	void IncrementCntCsMfaToMcms(){m_CntCsMfaToMcms++;}
	
	void TraceCardOrCs2ConnectionId()const;
	void DumpConnectionTable(std::ostream &ostr)const;
    
    COsQueue * GetTxQueue(int id,bool showAssert = true);
protected:
    CCardOrCsConnIdTable m_CardOrCsConnIdTable;
private:
//	CCardOrCsConnIdTable m_CardOrCsConnIdTable;
		
	DWORD m_CntMcmsToCsMfa;
	DWORD m_CntCsMfaToMcms;
};





#endif /*APIPROCESS_H_*/
