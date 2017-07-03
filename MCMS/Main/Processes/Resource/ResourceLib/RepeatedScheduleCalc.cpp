#include "RepeatedScheduleCalc.h" 
#include "SystemFunctions.h"
#include "StatusesGeneral.h"
#include "ComResRepeatDetails.h"
#include "TraceStream.h"
#include "ObjString.h"
/////////////////////////////////////////////////////////////////////////////////////////
WORD operator==(const RepeatSched& lhs ,const RepeatSched& rhs)
{	
	if((lhs.Duration.m_hour == rhs.Duration.m_hour) &&
	   (lhs.Duration.m_min == rhs.Duration.m_min) &&
	   (lhs.OccurTime == rhs.OccurTime))
		return TRUE;
	else
		return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////
WORD operator<(const RepeatSched& lhs ,const RepeatSched& rhs)
{
	if (lhs.Duration.m_hour != rhs.Duration.m_hour)
	  return lhs.Duration.m_hour < rhs.Duration.m_hour;
    
	if (lhs.Duration.m_min != rhs.Duration.m_min)
	  return lhs.Duration.m_min < rhs.Duration.m_min;
    
	return lhs.OccurTime < rhs.OccurTime; 
}

/////////////////////////////////////////////////////////////////////////////////////////
CRepeatedScheduleCalc::CRepeatedScheduleCalc()
{
   m_pComResRepeatedDetails = new CComResRepeatDetails;
   
   m_pRepeatSched = new std::set<RepeatSched>;
   m_pTimeRepeatSched = new std::set<CStructTm>;
}
/////////////////////////////////////////////////////////////////////////////
CRepeatedScheduleCalc::~CRepeatedScheduleCalc()
{
   m_pRepeatSched->clear();
   m_pTimeRepeatSched->clear();

   PDELETE(m_pRepeatSched);
   PDELETE(m_pTimeRepeatSched);
   
   POBJDELETE(m_pComResRepeatedDetails);   
}
/////////////////////////////////////////////////////////////////////////////
void CRepeatedScheduleCalc::SetReservationData(CStructTm StartTime,CStructTm Duration)
{
	m_StartTime = StartTime;
	if(m_StartTime.m_year > 1900)
		m_StartTime.m_year -= 1900;
	
	m_Duration = Duration;
}
/////////////////////////////////////////////////////////////////////////////
void CRepeatedScheduleCalc::SetRepetedDetails(CComResRepeatDetails* pRepetedDetails)
{
	*m_pComResRepeatedDetails= *pRepetedDetails;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CRepeatedScheduleCalc::NumOccurCalculat()
{
	if(m_pComResRepeatedDetails->GetRecurrenceType() == eDaily)
		return CalculatDailyOccurNum();
	else if(m_pComResRepeatedDetails->GetRecurrenceType() == eWeekly)
		return CalculatWeeklyOccurNum();
	else if(m_pComResRepeatedDetails->GetRecurrenceType() == eMonthly)
		return CalculatMonthlyOccurNum();

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CRepeatedScheduleCalc::LimitTimeCalculat()
{
	if(m_pComResRepeatedDetails->GetLimitTime().m_year > 1900)
		m_pComResRepeatedDetails->GetLimitTime().m_year -= 1900;
	
	AddTimeOffset(m_pComResRepeatedDetails->GetLimitTime(),m_pComResRepeatedDetails->GetGMTOffsetMinutes());
 	if(m_pComResRepeatedDetails->GetRecurrenceType() == eDaily)
		return CalculatDailyLimitTime();
	else if(m_pComResRepeatedDetails->GetRecurrenceType() == eWeekly)
		return CalculatWeeklyLimitTime();
	else if(m_pComResRepeatedDetails->GetRecurrenceType() == eMonthly)
		return CalculatMonthlyLimitTime();

   return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CRepeatedScheduleCalc::Calculate() 
{
	BYTE bretVal = FALSE;

	AddTimeOffset(m_StartTime,m_pComResRepeatedDetails->GetGMTOffsetMinutes());

	if(!m_pComResRepeatedDetails->GetEndBy())
		bretVal = NumOccurCalculat();
	else
		bretVal = LimitTimeCalculat();

	ConvertSchedToTime();
	return bretVal;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CRepeatedScheduleCalc::CalculatDailyOccurNum()
{
   RepeatSched* pRepeatSched;	
   RepeatSched RepSched;	
   tm tmTime;
   int OccurNember = m_pComResRepeatedDetails->GetOccurNumber();
//   int nYear = (m_StartTime.m_year < 1900) ? ()

   int Year		= m_StartTime.m_year;
   int Month	= m_StartTime.m_mon;
   int Day		= m_StartTime.m_day;
   int Hour		= m_StartTime.m_hour;
   int Minute	= m_StartTime.m_min;
   int Second	= m_StartTime.m_sec;

   if(!m_StartTime.GetAndVerifyAsTm(tmTime))
	   return FALSE;

   int dayOfWeek = tmTime.tm_wday;
   int NextDay=dayOfWeek; 

   BYTE bStartDay = FALSE;

   m_pRepeatSched->clear();

   if(OccurNember<=0)
	   return FALSE;

   m_pComResRepeatedDetails->SetInterval(0); //interval has no meaning here (in daily), and sometimes we got wrong number from WC 
   for(int i=1 ;i<=OccurNember;i++)
   {
	   BYTE bReservDay=TRUE;

	   if(m_pComResRepeatedDetails->GetInterval()>1){
	     if(dayOfWeek<NextDay)
			  bReservDay = FALSE;
		 else if(dayOfWeek==NextDay){
			  NextDay = dayOfWeek+m_pComResRepeatedDetails->GetInterval();
		 }
	   }

	   if(bStartDay)
	   { 
		  int DaysInMonth = GetDaysInMonth(Year,Month);

		  if(Day<DaysInMonth && Day!=31 && Day != 0)
		     Day++;
		  else
		  {
			 Day =1;

			 if(Month<11)
			   Month++;
			 else
			 {
			   Month = 0;
			   Year++;
			 }
		  }	
	   }  

	  RepSched.Duration.m_hour = m_Duration.m_hour;
	  RepSched.Duration.m_min = m_Duration.m_min;

	  if(Year>=138 || Year <= 70)
		  return FALSE;

	  CStructTm OccurTime(Day,Month,Year,Hour,Minute,Second);

  	  if(CheckValidDate(OccurTime))
	  {	  
		  RepSched.OccurTime = OccurTime;

		  if(m_pComResRepeatedDetails->GetInterval()==0)
		  {
			  if(m_pComResRepeatedDetails->GetWeekDay(dayOfWeek))
			  {
				  if(m_pComResRepeatedDetails->GetEndBy() && OccurTime<= m_pComResRepeatedDetails->GetLimitTime())
				  {
					m_pRepeatSched->insert(RepSched);
				  }
				  else if(!m_pComResRepeatedDetails->GetEndBy() && m_pRepeatSched->size() < (DWORD) OccurNember)
				  {
					m_pRepeatSched->insert(RepSched);
				  }
			  }
			  else if(OccurNember!=1)
				 i--;
		  }
		  else
		  {
			  if(bReservDay)
			  { 
				m_pRepeatSched->insert(RepSched);
			  }
			  else if(OccurNember!=1)
				 i--;
		  }

		  dayOfWeek = dayOfWeek +1;

		  if(dayOfWeek>6){ 
			  dayOfWeek= 0;
			  NextDay  -= 7;
		  }

		  bStartDay = TRUE;
	  }
   }
  return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CRepeatedScheduleCalc::CalculatWeeklyOccurNum()
{
   RepeatSched* pRepeatSched;	
   tm tmTime;
   RepeatSched RepSched; 
   int OccurNember = m_pComResRepeatedDetails->GetOccurNumber();
   int Year		= m_StartTime.m_year;	   
   int Month	= m_StartTime.m_mon;  
   int Day		= m_StartTime.m_day;	  
   int Hour		= m_StartTime.m_hour;
   int Minute	= m_StartTime.m_min;
   int Second	= m_StartTime.m_sec;

   if(!m_StartTime.GetAndVerifyAsTm(tmTime))
	   return FALSE;

   int dayOfWeek = tmTime.tm_wday;
   int Week=1,NextWeek=Week+m_pComResRepeatedDetails->GetInterval(); 

   int	lYear	= m_pComResRepeatedDetails->GetLimitTime().m_year ;
   int	lMonth	= m_pComResRepeatedDetails->GetLimitTime().m_mon;
   int	lDay	= m_pComResRepeatedDetails->GetLimitTime().m_day;
    
   BYTE bStartDay = FALSE;

   m_pRepeatSched->clear();

   if(m_pComResRepeatedDetails->GetInterval()>1)
	  OccurNember = m_pComResRepeatedDetails->GetInterval()*OccurNember;

   for(int i=0 ;i<=OccurNember;i++)
   {
	  BYTE bReservWeek=TRUE;

	  if(m_pComResRepeatedDetails->GetInterval()>1){
		  if(Week>1 && Week<NextWeek)
			  bReservWeek = FALSE;
		  else if(Week==NextWeek)
			  NextWeek = Week+m_pComResRepeatedDetails->GetInterval();
	  }

	  for(int j=0; dayOfWeek <= 6;j++)
	  {
		  if(bStartDay)
		  { 
			 int DaysInMonth = GetDaysInMonth(Year,Month);

			 if(Day>DaysInMonth)
			 {
				 Day  = Day - DaysInMonth;
 				 if(Month<11)
				   Month++;
				 else
				 {
				   Month = 0;
				   Year++;
				 }
			 }
		  }  

		  RepSched.Duration.m_hour = m_Duration.m_hour;
		  RepSched.Duration.m_min = m_Duration.m_min;

		  if(Year>=138 || Year <= 70)
			  return FALSE;

		  CStructTm OccurTime(Day,Month,Year,Hour,Minute,Second);

		  if(m_pRepeatSched->size() ==(DWORD)  m_pComResRepeatedDetails->GetOccurNumber())
			   return TRUE;

  		  if(CheckValidDate(OccurTime))
		  {	  
			  RepSched.OccurTime = OccurTime;

			  if(bReservWeek)
			  {
				  if(m_pComResRepeatedDetails->GetWeekDay(dayOfWeek) && OccurTime<= m_pComResRepeatedDetails->GetLimitTime() && m_pComResRepeatedDetails->GetEndBy())
				  {
					m_pRepeatSched->insert(RepSched);
				  }
				  else if(m_pComResRepeatedDetails->GetWeekDay(dayOfWeek) && !m_pComResRepeatedDetails->GetEndBy() && m_pRepeatSched->size() < (DWORD)OccurNember)
				  {
					m_pRepeatSched->insert(RepSched);
				  }
			  }

 			  dayOfWeek = dayOfWeek +1;
			  Day++;
			  bStartDay = TRUE;
		  }
	  }

	  if(dayOfWeek>6){
		  dayOfWeek = 0;
		  Week++;
	  }
   }

  return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CRepeatedScheduleCalc::CalculatMonthlyOccurNum()
{
   RepeatSched* pRepeatSched;	
   RepeatSched RepSched;	
   int OccurNember = m_pComResRepeatedDetails->GetOccurNumber();
   int DaysInMonth,DayByWeek=0;
   BYTE bSkip=FALSE;
   
   int Year = m_StartTime.m_year;   
   int Month= m_StartTime.m_mon;  
   int Day  = m_StartTime.m_day;  
   int MonthCount=1,NextMonth=MonthCount+m_pComResRepeatedDetails->GetInterval(); 
   
   if (m_pComResRepeatedDetails->GetEndBy() == 1)
   {
	  int	lYear	= m_pComResRepeatedDetails->GetLimitTime().m_year ;	
	  int	lMonth	= m_pComResRepeatedDetails->GetLimitTime().m_mon;	
	  int	lDay	= m_pComResRepeatedDetails->GetLimitTime().m_day  ;	
   }

   int Hour		= m_StartTime.m_hour;
   int Minute	= m_StartTime.m_min;
   int Second	= m_StartTime.m_sec;

    m_pRepeatSched->clear();

   if(OccurNember<=0)
	   return FALSE;

   if(m_pComResRepeatedDetails->GetInterval()>1 && m_pComResRepeatedDetails->GetEndBy() != 1)
	  OccurNember = m_pComResRepeatedDetails->GetInterval()*OccurNember;

	if(m_pComResRepeatedDetails->GetDaysIndex() > 0)
		m_pComResRepeatedDetails->SetDaysIndex((m_pComResRepeatedDetails->GetDaysIndex())-1);

   for(int i=1; i<=OccurNember+1;i++)
   {
	  BYTE bReservMonth=TRUE;

	  if(m_pComResRepeatedDetails->GetInterval()>1){
		  if(MonthCount>1 && MonthCount<NextMonth)
			  bReservMonth = FALSE;
		  else if(MonthCount==NextMonth)
			  NextMonth = MonthCount+m_pComResRepeatedDetails->GetInterval();
	  }

	  RepSched.Duration.m_hour = m_Duration.m_hour;
	  RepSched.Duration.m_min = m_Duration.m_min;

	  if(Year>=138 || Year <= 70)
		  return FALSE;

	  if(m_pComResRepeatedDetails->GetByMonth() == eByDate)
	  {
		  Day = m_pComResRepeatedDetails->GetDayOfMonth();

//		  if(!m_pComResRepeatedDetails->GetEndBy())
//		  {
			  DaysInMonth = GetDaysInMonth(Year,Month);

			  if(Day > DaysInMonth)
				  Day = DaysInMonth;
//		  }
	  }
	  else
	  {
		  DayByWeek = GetDayByWeekInMonth(Year,Month);

		  if(DayByWeek>0)
			  Day = DayByWeek;
	  }

	  CStructTm OccurTime(Day,Month,Year,Hour,Minute,Second);

	  if(m_pRepeatSched->size() == (DWORD) m_pComResRepeatedDetails->GetOccurNumber())
		   return TRUE;

  	  if(CheckValidDate(OccurTime))
	  {	  
		  if(m_pComResRepeatedDetails->GetEndBy() && OccurTime > m_pComResRepeatedDetails->GetLimitTime())
			  bSkip = TRUE;

		  if(bReservMonth && !bSkip)
		  {
//			  DaysInMonth = GetDaysInMonth(Year,Month);

//			  if(Day<=DaysInMonth)
//			  {
				  RepSched.OccurTime = OccurTime;
				  m_pRepeatSched->insert(RepSched); 
/*			  }
			  else
			  {
				  if(Year>=138 || Year <= 70)
					  return 138;

				  CStructTm OccurTime(Day,Month,Year,Hour,Minute,Second);
				  RepSched.OccurTime = OccurTime;
				  pRepeatSched = new RepeatSched(RepSched);	 
				  m_pRepeatSched->insert(pRepeatSched); 
			  }*/
		  }
	  }

	  if(Month<11)
	    Month++;
	  else
	  {
	    Month = 0;
	    Year++;
	  }

	  MonthCount++;
   }
   return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CRepeatedScheduleCalc::CalculatDailyLimitTime()
{
   tm tmTime;
   int	lYear	 = m_pComResRepeatedDetails->GetLimitTime().m_year ;      
   int	lMonth	 = m_pComResRepeatedDetails->GetLimitTime().m_mon;     
   int	lDay	 = m_pComResRepeatedDetails->GetLimitTime().m_day  ;    
   
   int sYear = m_StartTime.m_year;
   int sMonth= m_StartTime.m_mon;
   int sDay  = m_StartTime.m_day;
   
   if(!CheckValidDate(m_pComResRepeatedDetails->GetLimitTime()))
   {
	  ResetDate();
	  return FALSE; 
   }

   int DaysInMonth = 31;
   BYTE bStartDay = FALSE;

   if(!m_StartTime.GetAndVerifyAsTm(tmTime))
	   return FALSE;

   int dayOfWeek = tmTime.tm_wday;

   int Counter = 1;

   while(sDay != lDay || sMonth != lMonth || sYear!=lYear)
   {
	   DaysInMonth = GetDaysInMonth(sYear,sMonth);

   	   if(bStartDay)
	   { 
		   if(sDay < DaysInMonth && sDay!=31 && sDay != 0) 
			   sDay++;
		   else
		   {
			 sDay =1;
			 if(sMonth<11)
			   sMonth++;
			 else
			 {
			   sMonth = 0;
			   sYear++;
			 }
		   }	
	   }

	   dayOfWeek++;

	   if(dayOfWeek>6)
	   	  dayOfWeek = 0;

	   if(m_pComResRepeatedDetails->GetWeekDay(dayOfWeek))
		  Counter++;

       bStartDay = TRUE;
   }

   m_pComResRepeatedDetails->SetOccurNumber(Counter);

   if(m_pComResRepeatedDetails->GetOccurNumber()>MAX_RSRV_IN_LIST_AMOS)
   {
		m_pComResRepeatedDetails->SetOccurNumber(1);
		ResetDate();
   }

   return CalculatDailyOccurNum();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CRepeatedScheduleCalc::CalculatWeeklyLimitTime() 
{
   tm tmTime;
   int	lYear	 = m_pComResRepeatedDetails->GetLimitTime().m_year ;       
   int	lMonth	 = m_pComResRepeatedDetails->GetLimitTime().m_mon;      
   int	lDay	 = m_pComResRepeatedDetails->GetLimitTime().m_day  ;      

   int sYear = m_StartTime.m_year;
   int sMonth= m_StartTime.m_mon;
   int sDay  = m_StartTime.m_day;
   
   if(!m_StartTime.GetAndVerifyAsTm(tmTime))
	   return FALSE;

   int dayOfWeek = tmTime.tm_wday;

   if(!CheckValidDate(m_pComResRepeatedDetails->GetLimitTime()))
   {
	  ResetDate();
	  return FALSE;
   }

   int DaysInMonth = 31;
   BYTE bStartDay = FALSE;

   int Counter = 1;

   while(sDay != lDay || sMonth != lMonth || sYear!=lYear)
   {
	   DaysInMonth = GetDaysInMonth(sYear,sMonth);

   	   if(bStartDay)
	   { 
		   if(sDay < DaysInMonth && sDay!=31 && sDay != 0) 
			   sDay++;
		   else
		   {
			 sDay =1;
			 if(sMonth<11)
			   sMonth++;
			 else
			 {
			   sMonth = 0;
			   sYear++;
			 }
		   }	
	   }

	   dayOfWeek++;

	   if(dayOfWeek>6)
	   	  dayOfWeek = 0;

	   if(m_pComResRepeatedDetails->GetWeekDay(dayOfWeek))
		  Counter++;

       bStartDay = TRUE;
   }

   m_pComResRepeatedDetails->SetOccurNumber(Counter);

   if(m_pComResRepeatedDetails->GetOccurNumber()>MAX_RSRV_IN_LIST_AMOS)
   {
		m_pComResRepeatedDetails->SetOccurNumber(1);
		ResetDate();
   }

   return CalculatWeeklyOccurNum();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CRepeatedScheduleCalc::CalculatMonthlyLimitTime()
{
   int	lYear	 = m_pComResRepeatedDetails->GetLimitTime().m_year ;      
   int	lMonth	 = m_pComResRepeatedDetails->GetLimitTime().m_mon;    
   int	lDay	 = m_pComResRepeatedDetails->GetLimitTime().m_day  ;    
   
   int sYear = m_StartTime.m_year;
   int sMonth= m_StartTime.m_mon; 
   int sDay  = m_StartTime.m_day;

   int Counter = 0;
   
   if(!CheckValidDate(m_pComResRepeatedDetails->GetLimitTime()))
   {
	  ResetDate();
	  return FALSE;
   }

   BYTE bStartDay = FALSE;

   while(sMonth != lMonth || sYear!=lYear)
   {
   	   if(bStartDay)
	   { 
		   if(sMonth<11)
			sMonth++;
		   else
		   {
			 sMonth = 0;
			 sYear++;
		   }	 
	   }

	   Counter++;
       bStartDay = TRUE;
   }

   if(Counter != 0)
	   m_pComResRepeatedDetails->SetOccurNumber(Counter);
   else
	   m_pComResRepeatedDetails->SetOccurNumber(1);

   if(m_pComResRepeatedDetails->GetOccurNumber()>MAX_RSRV_IN_LIST_AMOS)
   {
		m_pComResRepeatedDetails->SetOccurNumber(1);
		ResetDate();
   }

   return CalculatMonthlyOccurNum();
}
/////////////////////////////////////////////////////////////////////////////
void CRepeatedScheduleCalc::ResetDate()
{
	m_pComResRepeatedDetails->GetLimitTime() = m_StartTime;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CRepeatedScheduleCalc::CheckValidDate(CStructTm OccurTime)
{
   tm tmTime;

   if(OccurTime < m_StartTime)
	   return FALSE;

   if(!OccurTime.GetAndVerifyAsTm(tmTime))
	   return FALSE;

   return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
int CRepeatedScheduleCalc::GetDaysInMonth(int Year,int Monthly)
{
	tm tmTime;
	int nDay = 27;

	tmTime.tm_year = Year;
	tmTime.tm_mon = Monthly;
	tmTime.tm_mday = 27;
	tmTime.tm_hour = 10;
	tmTime.tm_min = 10;
	tmTime.tm_sec = 10;
	tmTime.tm_wday  = 0;
	tmTime.tm_yday  = 0;

	int nRes = timegm(&tmTime);

	while((nRes != -1) && (tmTime.tm_mon == Monthly))
	{
	  tmTime.tm_mday++;
	  nDay++;
	  nRes = timegm(&tmTime);
	}

	return nDay-1;
}
/////////////////////////////////////////////////////////////////////////////
int CRepeatedScheduleCalc::GetWeeksInMonth(int Year,int Month)
{
	int WeekInMonth=1;
	tm tmTime;

	tmTime.tm_year = Year;
	tmTime.tm_mon = Month;
	tmTime.tm_mday = 1;
	tmTime.tm_hour = 10;
	tmTime.tm_min = 10;
	tmTime.tm_sec = 10;
	tmTime.tm_wday  = 0;
	tmTime.tm_yday  = 0;

	int nRes = timegm(&tmTime);

	while((nRes != -1) && (tmTime.tm_mon == Month))
	{
		if(tmTime.tm_wday==6)
		   WeekInMonth++;

		tmTime.tm_mday++;

		nRes = timegm(&tmTime);
	}

	return WeekInMonth;
}
/////////////////////////////////////////////////////////////////////////////
int CRepeatedScheduleCalc::GetDayByWeekInMonth(int Year,int Month)
{
	int nRes=0, i=0;

	int EndDayOfWeek =0, WeekInMonth=1,DayInMonth = 0, DayOfWeek = 0;
	int StartDayOfWeek=0;

	if(m_pComResRepeatedDetails->GetInstance() == eFirst)
	{
		StartDayOfWeek = 1;
		EndDayOfWeek = 7;
	}
	else if(m_pComResRepeatedDetails->GetInstance() == eSecond)
	{
		StartDayOfWeek = 8;
		EndDayOfWeek = 14;
	}
	else if(m_pComResRepeatedDetails->GetInstance() == eThird)
	{
		StartDayOfWeek = 15;
		EndDayOfWeek = 21;
	}
	else if(m_pComResRepeatedDetails->GetInstance() == eFourth)
	{
		StartDayOfWeek = 22;
		EndDayOfWeek = 28;
	}
	else if(m_pComResRepeatedDetails->GetInstance() == eLast)
	{
		EndDayOfWeek = GetDaysInMonth(Year, Month);
		StartDayOfWeek = EndDayOfWeek - 6;
	}
	
	tm tmTime;
	tmTime.tm_year = Year;
	tmTime.tm_mon = Month;
	tmTime.tm_hour = 10;
	tmTime.tm_min = 10;
	tmTime.tm_sec = 10;
	tmTime.tm_wday  = 0;
	tmTime.tm_yday  = 0;
	
	for(int i=StartDayOfWeek;i<=EndDayOfWeek;i++)
	{
		tmTime.tm_mday = i;

		nRes = timegm(&tmTime);

		if((nRes == -1) || (tmTime.tm_mon != Month))		
			break;

		DayOfWeek = tmTime.tm_wday;

		if(DayOfWeek == m_pComResRepeatedDetails->GetDaysIndex())
		{
			DayInMonth = tmTime.tm_mday;
			break;
		}
	}
	
	PASSERTMSG(DayInMonth == 0, "Day not found!!!");
	
	return DayInMonth;
}
/////////////////////////////////////////////////////////////////////////////
void CRepeatedScheduleCalc::ConvertSchedToTime()
{	
	std::set<RepeatSched>::iterator i;
 		
   for(i = m_pRepeatSched->begin(); i != m_pRepeatSched->end(); i++)  
	{
		AddTimeOffset((CStructTm&)(i->OccurTime),(-1)*m_pComResRepeatedDetails->GetGMTOffsetMinutes());
		m_pTimeRepeatSched->insert(i->OccurTime);
	}
}
///////////////////////////////////////////////////////////////////////////////
void CRepeatedScheduleCalc::AddTimeOffset(CStructTm& time, int nOffset)
{
	tm tmTime;

	time.GetAsTm(tmTime);
	tmTime.tm_min += nOffset;
	
	timegm(&tmTime);

	CStructTm tmpTime(tmTime);
	time = tmpTime;
}

