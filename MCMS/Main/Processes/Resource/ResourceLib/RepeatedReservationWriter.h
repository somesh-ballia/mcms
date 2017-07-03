#ifndef REPEATEDRESERVATIONWRITER_H_
#define REPEATEDRESERVATIONWRITER_H_

#include "PObject.h"
#include "CommResApi.h"
#include "AllocateStructs.h"

class CRecurrenceData
{
public:
	CRecurrenceData(CCommResApi* pCommRes);

	void CopyDataTo(CCommResApi* pCommRes);

	char m_Name[H243_NAME_LEN];
	char m_DisplayName[H243_NAME_LEN];
	CStructTm m_StartTime;
	DWORD m_MonitorConfId;

};

class CRepeatedReservationWriter
{
public:
	CRepeatedReservationWriter();
	virtual ~CRepeatedReservationWriter();

	BOOL IsWriting() {return m_bWriting;}
	void StartWriting();
	void EndWriting();

	void SetBaseReservation(CCommResApi* pCommRes);
	CCommResApi* GetBaseReservation() {return &m_BaseReservation;}

	void AddRecurrence(int index, CCommResApi* pConfToAdd);
	void RemoveRecurrence(int index);
	CRecurrenceData* GetRecurrenceData(int index);

private:
	BOOL m_bWriting;
	CCommResApi m_BaseReservation;
	CRecurrenceData* m_RecurrenceDataToWriteToDisk[MAX_RSRV_IN_LIST_AMOS];
};

#endif /*REPEATEDRESERVATIONWRITER_H_*/
