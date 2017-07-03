#ifndef NETPORTSPERSERVICE_H_
#define NETPORTSPERSERVICE_H_

#include "PObject.h"
#include "SystemResources.h"
#include "NetPortsPerBoard.h"
#include "IPServiceResources.h"

class CRsrcDesc;

class CNetPortsPerService : public CPObject
{
	CLASS_TYPE_1(CNetPortsPerService,CPObject)
public:
	CNetPortsPerService();
	virtual ~CNetPortsPerService();
	CNetPortsPerService(const CNetPortsPerService& other);
    const char* NameOf() const {return "CNetPortsPerService";}
    
    STATUS SpanConfiguredOnService(CSpanRTM* pSpan);
    STATUS SpanRemoved(CSpanRTM* pSpan);
    
    STATUS AddDialInReservedPorts(WORD nNumOfDialIn);
    STATUS RemoveDialInReservedPorts(WORD nNumOfDialIn);
    
    void CountTotalPorts(DWORD &numFree, DWORD &numDialOutReserved, DWORD &numDialInReserved);
    void CountPortsPerBoard(DWORD &numFree, DWORD &numDialOutReserved, int zeroBasedBoardId); //0-based board id
    
	void GetBestSpansListPerBoard(CSpanRTM* best_span_per_board[BOARDS_NUM], int zeroBasedBoardId); //0-based board id

private:	
	DWORD	m_NumDialInReservedPorts;
	
	CNetPortsPerBoard m_NetPortsPerBoard[BOARDS_NUM];
};

#endif /*NETPORTSPERSERVICE_H_*/
