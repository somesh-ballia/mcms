// SlotNumConvertor.cpp: implementation of the CSlotNumberConvertor class.
//
//////////////////////////////////////////////////////////////////////

/*
#include "SlotNumConvertor.h"
#include "TraceStream.h"
#include "DefinesGeneral.h"



// ------------------------------------------------------------
CSlotNumberConvertor::CSlotNumberConvertor ()
{
	int i=0;
	for (i=0; i < (MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS + 1); i++)
		m_slotNumConvertorArray[i] = 0;
		

	// === hard coded conversion:
	//      - first parameter (array index): slotNumForMonitoring
	//      - second parameter (array content): slotNumFromMpl
	SetSlotNumber(0, 20);
	SetSlotNumber(1, 41);
	SetSlotNumber(2, 42);
}


// ------------------------------------------------------------
CSlotNumberConvertor::~CSlotNumberConvertor ()
{
}


// ------------------------------------------------------------
int CSlotNumberConvertor::GetLogicalSlotNumber (const int physicalSlotNum)
{
	int i=0;
	for (i=0; i < (MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS + 1); i++)
	{
		if( physicalSlotNum == m_slotNumConvertorArray[i] )
			return i;
	}
	
	return NOT_FIND;
}


// ------------------------------------------------------------
int CSlotNumberConvertor::GetPhysicalSlotNumber (const int logicalSlotNum)
{
	if ( (MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS) < logicalSlotNum )
	{		
		TRACEINTO << "CSlotNumberConvertor::GetPhysicalSlotNumber - illegal logicalSlotNum: " <<  logicalSlotNum;
		return NOT_FIND;
	}
	
	else
	{
		return m_slotNumConvertorArray[logicalSlotNum];
	}
}


// ------------------------------------------------------------
void CSlotNumberConvertor::SetSlotNumber ( const int logicalSlotNum, const int physicalSlotNum )
{
	if ( (MAX_NUM_OF_BOARDS*MAX_NUM_OF_SUBBOARDS) < logicalSlotNum )
	{
		TRACEINTO << "CSlotNumberConvertor::SetSlotNumber - illegal logicalSlotNum: " <<  logicalSlotNum;
		return;
	}
	
	m_slotNumConvertorArray[logicalSlotNum] = physicalSlotNum;
}

*/
