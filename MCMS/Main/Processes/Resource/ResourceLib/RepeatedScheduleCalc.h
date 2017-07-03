#ifndef _REP_CALC_H
#define _REP_CALC_H

#include <StructTm.h>
#include <set>
#include "PObject.h"
#include "AllocateStructs.h"

class CComResRepeatDetails;

/////////////////////////////////////////////////////////////////////////////
struct RepeatSched
{
	CStructTm OccurTime;
	CStructTm Duration;
};

WORD operator==(const RepeatSched& lhs ,const RepeatSched& rhs);
WORD operator<(const RepeatSched& lhs ,const RepeatSched& rhs);

/////////////////////////////////////////////////////////////////////////////

class CRepeatedScheduleCalc : public CPObject
{
	CLASS_TYPE_1(CRepeatedScheduleCalc,CPObject)
// Construction
public:
	CRepeatedScheduleCalc();
	~CRepeatedScheduleCalc();
	virtual const char* NameOf() const { return "CRepeatedScheduleCalc";}
	
	void SetRepetedDetails(CComResRepeatDetails* pRepetedDetails);
	void SetReservationData(CStructTm StartTime,CStructTm Duration);
	BOOL Calculate();
	
	std::set<CStructTm>* GetStartTimes() {return m_pTimeRepeatSched;};

protected:
	BYTE LimitTimeCalculat();
	BYTE NumOccurCalculat();
	BOOL CalculatDailyOccurNum();
	BOOL CalculatWeeklyOccurNum();
	BOOL CalculatMonthlyOccurNum();
	BOOL CalculatDailyLimitTime();
	BOOL CalculatWeeklyLimitTime(); 
	BOOL CalculatMonthlyLimitTime();

	BYTE CheckValidDate(CStructTm OccurTime);
	int  GetDaysInMonth(int Year,int Monthly);
	void ResetDate();
	int  GetWeeksInMonth(int Year,int Month);
	int  GetDayByWeekInMonth(int Year,int Month);

	void ConvertSchedToTime();

	std::set<RepeatSched> *m_pRepeatSched;
	std::set<CStructTm> *m_pTimeRepeatSched;
	static void AddTimeOffset(CStructTm& time, int nOffset);	
	
private:	
	CComResRepeatDetails* m_pComResRepeatedDetails;

	CStructTm m_StartTime;
	CStructTm m_Duration;
};

#endif

