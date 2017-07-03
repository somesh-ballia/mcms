#include "RepeatedReservationWriter.h"


CRecurrenceData::CRecurrenceData(CCommResApi* pCommRes)
{
	strncpy(m_Name, pCommRes->GetName(), H243_NAME_LEN - 1);
	strncpy(m_DisplayName, pCommRes->GetDisplayName(), H243_NAME_LEN - 1);
	m_Name[H243_NAME_LEN - 1] = 0;
	m_DisplayName[H243_NAME_LEN - 1] = 0;

	m_StartTime = *(pCommRes->GetStartTime());

	m_MonitorConfId = pCommRes->GetMonitorConfId();
}

void CRecurrenceData::CopyDataTo(CCommResApi* pCommRes)
{
	pCommRes->SetName(m_Name);
	pCommRes->SetDisplayName(m_DisplayName);
	pCommRes->SetStartTime(m_StartTime);
	pCommRes->SetMonitorConfId(m_MonitorConfId);
}

CRepeatedReservationWriter::CRepeatedReservationWriter()
{
   for(int i=0; i<MAX_RSRV_IN_LIST_AMOS; i++)
	   m_RecurrenceDataToWriteToDisk[i] = NULL;

   m_bWriting = FALSE;
}

CRepeatedReservationWriter::~CRepeatedReservationWriter()
{
}

void CRepeatedReservationWriter::SetBaseReservation(CCommResApi* pCommRes)
{
	m_BaseReservation = *pCommRes;
}

void CRepeatedReservationWriter::StartWriting()
{
	m_bWriting = TRUE;
}

void CRepeatedReservationWriter::EndWriting()
{
	m_bWriting = FALSE;
}

void CRepeatedReservationWriter::RemoveRecurrence(int index)
{
	if(index >= 0 && index < MAX_RSRV_IN_LIST_AMOS)
	{
		if(m_RecurrenceDataToWriteToDisk[index] != NULL)
		{
			POBJDELETE(m_RecurrenceDataToWriteToDisk[index]);
			m_RecurrenceDataToWriteToDisk[index] = NULL;
		}
	}
	else
	{
		FPASSERTSTREAM(1, "CRepeatedReservationWriter::RemoveRecurrence - Failed, index is out of range, index: " << index);
	}
}

void CRepeatedReservationWriter::AddRecurrence(int index, CCommResApi* pConfToAdd)
{
	CRecurrenceData* pRecurrenceData = new CRecurrenceData(pConfToAdd);
	m_RecurrenceDataToWriteToDisk[index-1] = pRecurrenceData; //the index is 1 based, while the array is 0-based
}

CRecurrenceData* CRepeatedReservationWriter::GetRecurrenceData(int index)
{
	return m_RecurrenceDataToWriteToDisk[index];
}

