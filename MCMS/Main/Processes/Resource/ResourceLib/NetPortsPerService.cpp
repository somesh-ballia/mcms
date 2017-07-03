#include "TraceStream.h"
#include "NetPortsPerService.h"
#include "ConfResources.h"
#include "InternalProcessStatuses.h"
#include "HelperFuncs.h"

CNetPortsPerService::CNetPortsPerService()
{
	m_NumDialInReservedPorts = 0;
}

CNetPortsPerService::~CNetPortsPerService()
{
}

CNetPortsPerService::CNetPortsPerService(const CNetPortsPerService& other):CPObject(other)
{
	m_NumDialInReservedPorts = other.m_NumDialInReservedPorts;
	for(int i=0; i<BOARDS_NUM; i++)
		m_NetPortsPerBoard[i] = other.m_NetPortsPerBoard[i];
}


STATUS CNetPortsPerService::SpanConfiguredOnService(CSpanRTM* pSpan)
{
	WORD board_id = pSpan->GetBoardId();
	if(!CHelperFuncs::IsValidBoardId(board_id))
	{
		
		PTRACE2INT(eLevelError,"CNetPortsPerService::SpanConfiguredOnService - wrong board_id: ", board_id);
		return STATUS_FAIL;
	}
	
	return m_NetPortsPerBoard[board_id-1].SpanConfiguredOnService(pSpan);	
}

STATUS CNetPortsPerService::SpanRemoved(CSpanRTM* pSpan)
{
	WORD board_id = pSpan->GetBoardId();
	if(!CHelperFuncs::IsValidBoardId(board_id))
	{
		
		PTRACE2INT(eLevelError,"CNetPortsPerService::SpanRemoved - wrong board_id: ", board_id);
		return STATUS_FAIL;
	}
	
	return m_NetPortsPerBoard[board_id-1].SpanRemoved(pSpan);	
	
}

void CNetPortsPerService::CountTotalPorts(DWORD &numFree, DWORD &numDialOutReserved, DWORD &numDialInReserved)
{
	numFree = 0;
	numDialOutReserved = 0;
	
	DWORD numFreeTemp, numDialOutReservedTemp;
	
	for(int i=0; i<BOARDS_NUM; i++)
	{
		m_NetPortsPerBoard[i].CountTotalPorts(numFreeTemp, numDialOutReservedTemp);
		numFree += numFreeTemp;
		numDialOutReserved += numDialOutReservedTemp;
	}
	
	numDialInReserved = m_NumDialInReservedPorts;
}

void CNetPortsPerService::CountPortsPerBoard(DWORD &numFree, DWORD &numDialOutReserved, int zeroBasedBoardId)
{
	m_NetPortsPerBoard[zeroBasedBoardId].CountTotalPorts(numFree, numDialOutReserved);	
}

STATUS CNetPortsPerService::AddDialInReservedPorts(WORD nNumOfDialIn)
{
	DWORD numFree, numDialOutReserved, numDialInReserved;
	CountTotalPorts(numFree, numDialOutReserved, numDialInReserved);
	//VNGFE-7640 - For debugging
	TRACEINTO << "numDialInReserved:" << numDialInReserved << ", numDialOutReserved:" << numDialOutReserved << ", numFree:" << numFree;
	if(numFree > (numDialOutReserved + numDialInReserved + nNumOfDialIn))
	{
		m_NumDialInReservedPorts += nNumOfDialIn;
		return STATUS_OK;
	}
	else
	{
		return STATUS_INSUFFICIENT_RTM_RSRC;
	}
}

STATUS CNetPortsPerService::RemoveDialInReservedPorts(WORD nNumOfDialIn)
{
	if(m_NumDialInReservedPorts < nNumOfDialIn)
	{
		TRACESTR(eLevelError) << "CNetPortsPerService::RemoveDialInReservedPorts - trying to remove to much ports: nNumOfDialIn: " << nNumOfDialIn
			<< " m_NumDialInReservedPorts " << m_NumDialInReservedPorts;
		m_NumDialInReservedPorts = 0;
		return STATUS_FAIL;
	}
	
	m_NumDialInReservedPorts -= nNumOfDialIn;
	//VNGFE-7640 - For debugging
	TRACEINTO << "nNumOfDialIn:" << nNumOfDialIn << ", NumDialInReservedPorts:" << m_NumDialInReservedPorts;
	return STATUS_OK;	
}

void CNetPortsPerService::GetBestSpansListPerBoard(CSpanRTM* best_span_per_board[BOARDS_NUM], int zeroBasedBoardId)
{
	if(zeroBasedBoardId>=0 && zeroBasedBoardId<BOARDS_NUM)
		m_NetPortsPerBoard[zeroBasedBoardId].GetBestSpansList( best_span_per_board );
	else
		PASSERT(1);
	
}

