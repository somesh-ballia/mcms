#include "NetPortsPerBoard.h"
#include "ConfResources.h"
#include "InternalProcessStatuses.h"
#include "TraceStream.h"
#include "SystemResources.h"
#include "HelperFuncs.h"
#include "NetServicesDB.h"

CNetPortsPerBoard::CNetPortsPerBoard()
{
	for(int i=0; i<	MAX_NUM_SPANS_ORDER; i++)
		m_RTMSpans[i] = NULL;
}

CNetPortsPerBoard::~CNetPortsPerBoard()
{
}

STATUS CNetPortsPerBoard::SpanConfiguredOnService(CSpanRTM* pSpan)
{
	WORD unitId = pSpan->GetUnitId(); 
	
	if(unitId> MAX_NUM_SPANS_ORDER  || unitId==0 ) //no need to check less than 0, because WORD is unsigned
	{
		PASSERT(1);
		return STATUS_FAIL;		
	}

	CSpanRTM* pExistSpan =  GetSpan(unitId); 
	if(pExistSpan != NULL)//we already have this span????
	{
		PASSERT(1);
		return STATUS_FAIL;		
	}
	
	m_RTMSpans[GetSpanIndexFromSpanId(unitId)] = pSpan;
	
	return STATUS_OK;	
}

STATUS CNetPortsPerBoard::SpanRemoved(CSpanRTM* pSpan)
{
	WORD unitId = pSpan->GetUnitId(); 
	
	if(unitId> MAX_NUM_SPANS_ORDER || unitId==0) //no need to check less than 0, because WORD is unsigned
	{
		PASSERT(1);
		return STATUS_FAIL;		
	}
	
	m_RTMSpans[GetSpanIndexFromSpanId(unitId)] = NULL;
	
	return STATUS_OK;		
}

CSpanRTM* CNetPortsPerBoard::GetSpan(WORD unitId)
{
	if(unitId>MAX_NUM_SPANS_ORDER  || unitId==0) //no need to check less than 0, because WORD is unsigned
	{
		PASSERT(1);
		return NULL;		
	}
	return m_RTMSpans[GetSpanIndexFromSpanId(unitId)];
}

STATUS CNetPortsPerBoard::CountTotalPorts(DWORD &numFree, DWORD &numDialOutReserved)
{
	numFree = numDialOutReserved = 0;
	for(int i=0; i<	MAX_NUM_SPANS_ORDER; i++)
	{
		if(m_RTMSpans[i]==NULL || m_RTMSpans[i]->GetIsEnabled() != TRUE )
			 continue;

		numFree += m_RTMSpans[i]->GetNumFreePorts();
		numDialOutReserved += m_RTMSpans[i]->GetNumDialOutReservedPorts();
	}

	return STATUS_OK;
}

void CNetPortsPerBoard::GetBestSpansList( CSpanRTM* spanList[MAX_NUM_SPANS_ORDER])
{
	WORD num_free_ports = 0 ;
	CSpanRTM* pCurSpan = NULL;
	
    CNetServicesDB* pNetServicesDB = CHelperFuncs::GetNetServicesDB();
    PASSERT_AND_RETURN(pNetServicesDB == NULL);
	
	eSpanAllocation spanAllocationOrder = pNetServicesDB->GetSpanAllocationOrder(); 
		
	// Initialize the list (might be already initialized)
	for(int k=0; k < MAX_NUM_SPANS_ORDER; k++)
		spanList[k] = NULL;
	
	int index = 0;
	
	if(spanAllocationOrder == eSpanAllocation_LoadBalancing)
	{
		// Build the list
		for(int l=0; l < MAX_NUM_SPANS_ORDER; l++)
		{
			if(m_RTMSpans[l]==NULL || m_RTMSpans[l]->GetIsEnabled() != TRUE )
				 continue;
			
			spanList[index++] = m_RTMSpans[l];
		}
		
		int length = index, j = 0;
	
		// Sort the list (Insertion Sort - descending order)
		for(int i=1; i < length; i++)
		{
			num_free_ports = spanList[i]->GetNumFreePortsThatAreNotReserved();
			pCurSpan = spanList[i];
			j = i-1;
			
			while ( j >= 0 && ( spanList[j] && spanList[j]->GetNumFreePortsThatAreNotReserved() < num_free_ports) )
			{
				spanList[j + 1] = spanList[j];
				j--;
			}
			spanList[j + 1] = pCurSpan;		
		}
	}
	else if(spanAllocationOrder == eSpanAllocation_FillFromStart)
	{
		//first take all the spans that are don't have zero free ports left
		for(int l=0; l < MAX_NUM_SPANS_ORDER; l++)
		{
			if(m_RTMSpans[l]==NULL || m_RTMSpans[l]->GetIsEnabled() != TRUE)
				 continue;
			if(m_RTMSpans[l]->GetNumFreePortsThatAreNotReserved() != 0)
				spanList[index++] = m_RTMSpans[l];
		}
		
		//now take all the ones that have zero left
		for(int l=0; l < MAX_NUM_SPANS_ORDER; l++)
		{
			if(m_RTMSpans[l]==NULL || m_RTMSpans[l]->GetIsEnabled() != TRUE)
				 continue;
			if(m_RTMSpans[l]->GetNumFreePortsThatAreNotReserved() == 0)
				spanList[index++] = m_RTMSpans[l];
		}
	}
	else if(spanAllocationOrder == eSpanAllocation_FillFromEnd)
	{
		//first take all the spans that are don't have zero free ports left
		for(int l=MAX_NUM_SPANS_ORDER-1; l >= 0 ; l--)
		{
			if(m_RTMSpans[l]==NULL || m_RTMSpans[l]->GetIsEnabled() != TRUE)
				 continue;
			if(m_RTMSpans[l]->GetNumFreePortsThatAreNotReserved() != 0)
				spanList[index++] = m_RTMSpans[l];
		}
		
		//now take all the ones that have zero left
		for(int l=MAX_NUM_SPANS_ORDER-1; l >= 0 ; l--)
		{
			if(m_RTMSpans[l]==NULL || m_RTMSpans[l]->GetIsEnabled() != TRUE)
				 continue;
			if(m_RTMSpans[l]->GetNumFreePortsThatAreNotReserved() == 0)
				spanList[index++] = m_RTMSpans[l];
		}	
	}
	else
	{
		PASSERT(1);
	}
}
