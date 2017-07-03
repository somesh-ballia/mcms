// StructTm.cpp

#include "StructTm.h"

#include <stdlib.h>
#include <string>
#include <ostream>
#include <istream>
#include "NStream.h"
#include <stdio.h>

#include "SystemFunctions.h"

static int current_year = 1900;

static DWORD NumberOfDigits(int numValue)
{
    DWORD cnt = 0;
	while(numValue != 0)
	{
	    numValue /= 10;
		cnt++;
	}
	return cnt;
}

CStructTm::CStructTm()
{
	m_sec  = 0;
	m_min  = 0;
	m_hour = 0; // 00:00:00 (midnight)
	m_day  = 1; // first day of the month
	m_mon  = 0; // january
	m_year = 100; // year 2000 AC
}

CStructTm::CStructTm(int hour, int min, int sec)
{
	m_hour = hour;
	m_min  = min;
	m_sec  = sec;
	m_day  = 1; // first day of the month
	m_mon  = 0; // january
	m_year = 100; // year 2000 AC
}

CStructTm::CStructTm(int day,
					 int mon,
					 int year,
					 int hour,
					 int min,
					 int sec)
{
	m_day  = day;
	m_mon  = mon;
	m_year = year;
	m_hour = hour;
	m_min  = min;
	m_sec  = sec;
}

CStructTm::CStructTm(const CStructTm &other)
	:CPObject(other)
{
	m_sec  = other.m_sec;
	m_min  = other.m_min;
	m_hour = other.m_hour;
	m_day  = other.m_day;
	m_mon  = other.m_mon;
	m_year = other.m_year;
}

CStructTm::~CStructTm()
{}

void CStructTm::InitDefaults()
{
	m_sec  = 0;
	m_min  = 0;
	m_hour = 0; // 00:00:00 (midnight)
	m_day  = 1; // first day of the month
	m_mon  = 0; // january
	m_year = 100; // year 2000 AC
}

void CStructTm::Serialize(std::ostream &m_ostr)
{
	m_ostr << m_day  << "."
		   << m_mon  << "."
		   << m_year << "	"  // must be tab(1 character)
		   << m_hour << ":"
		   << m_min  << ":"
           << m_sec  << "\n";

}
void  CStructTm::SerializeV2(std::ostream &m_ostr)
{
	m_ostr << m_day  << "."
			   << m_mon  << "."
			   << m_year << ",";
    SerializeCdr(m_ostr);
			   //<< m_hour << ":"
			   //<< m_min  << ":"
	           //<< m_sec  << ",";
}

void CStructTm::DeSerialize(std::istream &m_istr)
{
	m_istr >> m_day;
	m_istr.ignore(1);
	m_istr >> m_mon;
	m_istr.ignore(1);
	m_istr >> m_year;
	m_istr.ignore(1);
	m_istr >> m_hour;
	m_istr.ignore(1);
	m_istr >> m_min;
	m_istr.ignore(1);
	m_istr >> m_sec;
}

void CStructTm::Serialize(WORD format,CSegment& seg)
{
	if (format != NATIVE) PASSERT(1);
	COstrStream pOstr;
	Serialize(pOstr);
	pOstr.Serialize(seg);
}

void CStructTm::DeSerialize(WORD format,CSegment& seg)
{
    if (format != NATIVE) PASSERT(1);
    CIstrStream istr (seg);
    DeSerialize(istr);
}

void CStructTm::SerializeBilling(std::ostream &m_ostr)
{
  char		sday[3],smonth[3],syear[5],shour[3],smin[3],ssec[3],swork[3],sblank[3]="  ",yrblank[5]="    ";

	strncpy(sday,sblank,3);
	strncpy(smonth,sblank,3);
	strncpy(syear,yrblank,5);
	strncpy(shour,sblank,3);
	strncpy(smin,sblank,3);
    strncpy(ssec,sblank,3);
	strncpy(swork,sblank,3);

	sday[0]='0';
	smonth[0]='0';
	shour[0]='0';
	smin[0]='0';
	ssec[0]='0';

	if(m_day <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_day);
//		itoa(m_day,swork,10);
		sday[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
//		itoa(m_day,sday,10);
		snprintf(sday, sizeof(sday), "%d", m_day);
	}

	if(m_mon <= 9)
	{
//		itoa(m_mon,swork,10);
		snprintf(swork, sizeof(swork), "%d", m_mon);
		smonth[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(smonth, sizeof(smonth), "%d", m_mon);
//		itoa(m_mon,smonth,10);
	}

	if(m_hour <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_hour);
//		itoa(m_hour,swork,10);
		shour[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(shour, sizeof(shour), "%d", m_hour);
//		itoa(m_hour,shour,10);
	}

	if(m_min <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_min);
//		itoa(m_min,swork,10);
		smin[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(smin, sizeof(smin), "%d", m_min);
//		itoa(m_min,smin,10);
	}

	if(m_sec <=  9)
	{
		snprintf(swork, sizeof(swork), "%d", m_sec);
//		itoa(m_sec,swork,10);
		ssec[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(ssec, sizeof(ssec), "%d", m_sec);
//		itoa(m_sec,ssec,10);
	}

	if(m_year < current_year)
	{
		int year = current_year + m_year;
		snprintf(syear, sizeof(syear), "%d", year);
//		itoa(year,syear,10);
	}
	else
	{
		snprintf(syear, sizeof(syear), "%d", m_year);
//		itoa(m_year,syear,10);
	}

	  m_ostr << sday  << "." <<smonth << "." <<syear << "," <<
            shour << ":" << smin << ":" << ssec << ",";

/*
    m_ostr << m_day  << "." << m_mon << "." <<m_year << "   " <<
            m_hour << ":" << m_min << ":" << m_sec << ",";
	*/
}

/////////////////////////////////////////////////////////////////////////////
void CStructTm::DeSerializeBilling(std::istream &m_istr)
{
  char		sday[3],smonth[3],syear[5],shour[3],smin[3],ssec[3],sblank[3]="  ",yrblank[5]="    ";

  // assuming format = OPERATOR_MCMS

	strncpy(sday,sblank,3);
	strncpy(smonth,sblank,3);
	strncpy(syear,yrblank,5);
	strncpy(shour,sblank,3);
	strncpy(smin,sblank,3);
	strncpy(ssec,sblank,3);

// GetLine eats the delimeter(:)
  m_istr.getline(sday,3,'.');
//  m_istr.ignore(1);
  m_istr.getline(smonth,3,'.');
//	m_istr.ignore(1);
  m_istr.getline(syear,5,',');
//	m_istr.ignore(1);
  m_istr.getline(shour,3,':');
//  m_istr.ignore(1);
  m_istr.getline(smin,3,':');
//  m_istr.ignore(1);
  m_istr.getline(ssec,3,',');

/*
  m_istr >> m_day;
  m_istr.ignore(1);
  m_istr >> m_mon;
  m_istr.ignore(1);
  m_istr >> m_year;
  m_istr >> m_hour;
  m_istr.ignore(1);
  m_istr >> m_min;
  m_istr.ignore(1);
  m_istr >> m_sec;
*/
	m_day=atoi(sday);
	m_mon=atoi(smonth);
	m_year=atoi(syear);// - current_year;
	m_hour=atoi(shour);
	m_min=atoi(smin);
	m_sec=atoi(ssec);

}


/////////////////////////////////////////////////////////////////////////////
void CStructTm::SerializeCdr(std::ostream &m_ostr)
{
	const DWORD numDigitsHour = 4;

	DWORD hourDigits = NumberOfDigits(m_hour);
	if( (numDigitsHour - 1) < hourDigits)
	{
		m_hour = 666;
    }

  	char shour[numDigitsHour]="  ";
  	char smin[3]="  ";
  	char ssec[3]="  ";
  	char swork[3]="  ";
  	char sblank[3]="  ";

	strncpy(shour,sblank,numDigitsHour);
	strncpy(smin,sblank,3);
	strncpy(ssec,sblank,3);
	strncpy(swork,sblank,3);

	shour[0]='0';
	smin[0]='0';
	ssec[0]='0';

  // assuming format = OPERATOR_MCMS

	if(m_hour <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_hour);
//		itoa(m_hour,swork,10);
		shour[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(shour, sizeof(shour), "%d", m_hour);
//		itoa(m_hour,shour,10);
	}

	if(m_min <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_min);
//		itoa(m_min,swork,10);
		smin[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(smin, sizeof(smin), "%d", m_min);
//		itoa(m_min,smin,10);
	}

	if(m_sec <=  9)
	{
		snprintf(swork, sizeof(swork), "%d", m_sec);
//		itoa(m_sec,swork,10);
		ssec[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(ssec, sizeof(ssec), "%d", m_sec);
//		itoa(m_sec,ssec,10);
	}

	m_ostr <<shour <<":" <<smin <<":" <<ssec <<",";
}

void CStructTm::DeSerializeCdr(std::istream &m_istr)
{
	const DWORD numDigitsHour = 8;

  	char shour[numDigitsHour]="  ";
  	char smin[3]="  ";
  	char ssec[3]="  ";
  	char sblank[3]="  ";

	strncpy(shour,sblank,numDigitsHour);
	strncpy(smin,sblank,3);
	strncpy(ssec,sblank,3);

	// GetLine eats the delimeter(:)
  m_istr.getline(shour,numDigitsHour,':');
  m_istr.getline(smin,3,':');
  m_istr.getline(ssec,3,',');

	m_hour=atoi(shour);
  m_min=atoi(smin);
	m_sec=atoi(ssec);
}

/////////////////////////////////////////////////////////////////////////////
// DateAndTime ::= TEXTUAL-CONVENTION
//     DISPLAY-HINT "2d-1d-1d,1d:1d:1d.1d,1a1d:1d"
//     STATUS       current
//     DESCRIPTION
//             "A date-time specification.

//              field  octets  contents                  range
//              -----  ------  --------                  -----
//                1      1-2   year                      0..65536
//                2       3    month                     1..12
//                3       4    day                       1..31
//                4       5    hour                      0..23
//                5       6    minutes                   0..59
//                6       7    seconds                   0..60
//                             (use 60 for leap-second)
//                7       8    deci-seconds              0..9
//                8       9    direction from UTC        '+' / '-'
//                9      10    hours from UTC            0..11
//               10      11    minutes from UTC          0..59

//             For example, Tuesday May 26, 1992 at 1:30:15 PM EDT would be
//             displayed as:

//                              1992-5-26,13:30:15.0,-4:0

//             Note that if only local time is known, then timezone
//             information (fields 8-10) is not present.
void CStructTm::SerializeSNMP(std::ostream &m_ostr)
{
    char buffer [128];
    snprintf(buffer, sizeof(buffer), "%02d-%01d-%01d,%01d:%01d:%01d.0,0:0",
            m_year,
            m_mon,
            m_day,
            m_hour,
            m_min,
            m_sec);
    m_ostr << buffer;
}

void CStructTm::DumpToBuffer(char *buffer) const
{
	sprintf(buffer,
          "%02d.%02d.%02d %02d:%02d:%02d",
          m_day,
          m_mon,
          (m_year-100) % 100,
          m_hour,
          m_min,
          m_sec);
}

void CStructTm::LongSerialize(std::ostream &m_ostr)
{
	Serialize(m_ostr);

	char sday[3];
	char smonth[3];
	char syear[5];
	char shour[3];
	char smin[3];
	char ssec[3];
	char swork[3];
	char sblank[3] = "  ";
	char yrblank[5] = "    ";
	int year=1900;

	// assuming format = OPERATOR_MCMS
	strncpy(sday,sblank,3);
	strncpy(smonth,sblank,3);
	strncpy(syear,yrblank,5);
	strncpy(shour,sblank,3);
	strncpy(smin,sblank,3);
	strncpy(ssec,sblank,3);
	strncpy(swork,sblank,3);

	sday[0]='0';
	smonth[0]='0';
	shour[0]='0';
	smin[0]='0';
	ssec[0]='0';

	if(m_day <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_day);
		sday[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(swork, sizeof(swork), "%d", m_day);
	}

	if(m_mon <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_mon);
		smonth[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(smonth, sizeof(smonth), "%d", m_mon);
	}

	if(m_hour <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_hour);
		shour[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(shour, sizeof(shour), "%d", m_hour);
	}

	if(m_min <= 9)
	{
		snprintf(swork, sizeof(swork), "%d", m_min);
		smin[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(smin, sizeof(smin), "%d", m_min);
	}

	if(m_sec <=  9)
	{
		snprintf(swork, sizeof(swork), "%d", m_sec);
		ssec[1]=swork[0];
		strncpy(swork,sblank,3);
	}
	else
	{
		snprintf(ssec, sizeof(ssec), "%d", m_sec);
	}

	if(m_year < 1900)
	{
		year=year+m_year;
		snprintf(syear, sizeof(syear), "%d", year);
	}
	else
	{
		snprintf(syear, sizeof(syear), "%d", m_year);
	}

	m_ostr << sday  << "." <<smonth << "." <<syear << "," <<
		shour << ":" << smin << ":" << ssec << "\n";

}

int CStructTm::IsValidForCdr() const
{
	if (m_sec > 59) return FALSE;
	if (m_min > 59) return FALSE;
	if (m_hour > 23) return FALSE;
	if (m_day == 0 || m_day > 31) return FALSE;
	if (m_mon > 12) return FALSE;
	if (m_year < 95 || m_year > 2100) return FALSE;
	return TRUE;
}

int CStructTm::IsValid() const
{
	if (m_sec > 59) return FALSE;
	if (m_min > 59) return FALSE;
	if (m_hour > 23) return FALSE;
	if (m_day == 0 || m_day > 31) return FALSE;
	if (m_mon > 12) return FALSE;
	return TRUE;
}

CStructTm&  CStructTm::operator=(const CStructTm& other)
{
	m_sec=other.m_sec;
	m_min=other.m_min;
	m_hour=other.m_hour;
	m_day=other.m_day;
	m_mon=other.m_mon;
	m_year=other.m_year;
	return *this;
}

CStructTm CStructTm::GetTimeDelta(const CStructTm& other) const
{
    CStructTm dif(*this);
    dif.m_day = 0;
    dif.m_mon = 0;
    dif.m_year = 0;


    dif.m_sec -= other.m_sec;
    if(0 > dif.m_sec)
    {
        dif.m_sec += 60;
        dif.m_min -= 1;
    }

	dif.m_min -= other.m_min;
    if(0 > dif.m_min)
    {
        dif.m_min += 60;
        dif.m_hour -= 1;
    }

    dif.m_hour -= other.m_hour;
    if(0 > dif.m_hour)
    {
        dif.m_hour += 24;
    }

    return dif;
}

CStructTm::CStructTm(const tm &tm_other)
{
	m_day  = tm_other.tm_mday;
	m_mon  = tm_other.tm_mon;
	m_year = tm_other.tm_year;
	m_hour = tm_other.tm_hour;
	m_min  = tm_other.tm_min;
	m_sec  = tm_other.tm_sec;
}

void CStructTm::GetAsTm(tm& tm_other) const
{
	tm_other.tm_mday  = m_day;
	tm_other.tm_mon   = m_mon;
	tm_other.tm_year  = m_year;
	tm_other.tm_hour  = m_hour;
	tm_other.tm_min   = m_min;
	tm_other.tm_sec   = m_sec;
	tm_other.tm_wday  = 0;
	tm_other.tm_yday  = 0;
	tm_other.tm_isdst = 0;
	tm_other.tm_gmtoff = 0;
}

BYTE CStructTm::GetAndVerifyAsTm(tm& tm_other)
{
	tm tmTemp;

	GetAsTm(tmTemp);
	if(tmTemp.tm_year > 1900)
		tmTemp.tm_year -= 1900;

	tm_other = tmTemp;			// to return the stored values anyway

	int nRes = timegm(&tmTemp);

	if((nRes == -1) || (tmTemp.tm_hour != tm_other.tm_hour) || (tmTemp.tm_min != tm_other.tm_min) ||
	   (tmTemp.tm_sec != tm_other.tm_sec) || (tmTemp.tm_mon != tm_other.tm_mon) ||
	   (tmTemp.tm_year != tm_other.tm_year) || (tmTemp.tm_mday != tm_other.tm_mday))
		return FALSE;

	tm_other = tmTemp;

	return TRUE;
}

int CStructTm::operator<=(const CStructTm& other) const
{
	if ( m_year<other.m_year) return TRUE;
	if ( m_year>other.m_year) return FALSE;

	if ( m_mon<other.m_mon) return TRUE;
	if ( m_mon>other.m_mon) return FALSE;

	if ( m_day<other.m_day) return TRUE;
	if ( m_day>other.m_day) return FALSE;

	if ( m_hour<other.m_hour) return TRUE;
	if ( m_hour>other.m_hour) return FALSE;

	if ( m_min<other.m_min) return TRUE;
	if ( m_min>other.m_min) return FALSE;

	if ( m_sec<=other.m_sec) return TRUE;
	else                     return FALSE;

}

int CStructTm::operator>=(const CStructTm& other) const
{
    return (other <= *this);
}

int operator!=(const CStructTm& one, const CStructTm& two)
{
	if (one == two)
		return FALSE;
	return TRUE;
}

int GetNumOfDaysInMounth(int mounthInd, int year)
{
    static int mounths [] ={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int index = mounthInd % 12;
    int res = mounths[index];
    if( 0 == year % 4 && 1 == index) // febrary
    {
        res = 29;
    }
    return res;
}

void CStructTm::AddReferenceTime(const CStructTm &referenceTime)
{
	tm  tmC;
    int left_over_minutes = 0;
    int left_over_hours = 0;
    int left_over_day = 0;
    int left_over_mounth = 0;
    int left_over_year = 0;

	memset(&tmC,0,sizeof(tm));

	tmC.tm_sec  = m_sec + referenceTime.m_sec;
    while (tmC.tm_sec > 59)
    {
        tmC.tm_sec -= 60;
        left_over_minutes++;
    }
	tmC.tm_min  = m_min + referenceTime.m_min + left_over_minutes;

    while (tmC.tm_min > 59)
    {
        tmC.tm_min-= 60;
        left_over_hours++;
    }
	tmC.tm_hour  = m_hour + referenceTime.m_hour + left_over_hours;

    while (tmC.tm_hour > 23)
    {
        tmC.tm_hour-= 24;
        left_over_day++;
    }
 	tmC.tm_mday  = m_day + referenceTime.m_day + left_over_day;

    while(true)
    {
        int numOfDaysInMounth = GetNumOfDaysInMounth(m_mon + left_over_mounth, m_year);
        if(tmC.tm_mday > numOfDaysInMounth)
        {
            tmC.tm_mday = (tmC.tm_mday - numOfDaysInMounth);
            left_over_mounth++;
        }
        else
        {
            break;
        }
    }
    tmC.tm_mon = m_mon + referenceTime.m_mon + left_over_mounth;

    left_over_year = tmC.tm_mon / 12;
    tmC.tm_mon = tmC.tm_mon % 12;

    tmC.tm_year = m_year + referenceTime.m_year + left_over_year;

	mktime(&tmC); // another function should be set by Sagi - Problem with clock + 1H winter/summer time

	*this=tmC;
}

DWORD CStructTm::operator - (const CStructTm& other) const
{
	time_t time1 = GetAbsTime();
	time_t time2 = other.GetAbsTime();
	DWORD dif = (time1 > time2 ? time1 - time2 : 0);
	return dif;
}

void CStructTm::Dump(std::ostream& msg) const
{
	msg << m_day << "/" << m_mon << "/"  << m_year << "  " <<
		m_hour << ":" << m_min << ":" << m_sec << "  ";

}

int operator<(const CStructTm& one,const CStructTm& other)
{
	if ( one.m_year<other.m_year) return TRUE;
	if ( one.m_year>other.m_year) return FALSE;

	if ( one.m_mon<other.m_mon) return TRUE;
	if ( one.m_mon>other.m_mon) return FALSE;

	if ( one.m_day<other.m_day) return TRUE;
	if ( one.m_day>other.m_day) return FALSE;

	if ( one.m_hour<other.m_hour) return TRUE;
	if ( one.m_hour>other.m_hour) return FALSE;

	if ( one.m_min<other.m_min) return TRUE;
	if ( one.m_min>other.m_min) return FALSE;

	if ( one.m_sec<other.m_sec) return TRUE;
	else                     return FALSE;

}

int operator>(const CStructTm& one,const CStructTm& other)
{
    return (other < one);
}

int operator==(const CStructTm& one,const CStructTm& other)
{
	if ( one.m_year != other.m_year) return FALSE;

	if ( one.m_mon != other.m_mon) return FALSE;

	if ( one.m_day != other.m_day) return FALSE;

	if ( one.m_hour != other.m_hour) return FALSE;

	if ( one.m_min != other.m_min) return FALSE;

	if ( one.m_sec != other.m_sec) return FALSE;
	return TRUE;
}

CStructTm::operator time_t() const
{
	time_t time = GetAbsTime();
	return time;
}

time_t CStructTm::GetAbsTime(BOOL isGMT)const
{
	tm     tmpTime;
	time_t absTime;

	tmpTime.tm_sec 	= m_sec;
	tmpTime.tm_min 	= m_min;
	tmpTime.tm_hour = m_hour;
	tmpTime.tm_mday = m_day;
	tmpTime.tm_mon 	= m_mon;
	tmpTime.tm_year = m_year;
	tmpTime.tm_gmtoff = 0;
	tmpTime.tm_isdst= 0;
	tmpTime.tm_wday = 0;
	tmpTime.tm_yday = 0;

	tmpTime.tm_mon  -= 1;
	tmpTime.tm_year -= 1900;

	if( isGMT )
		absTime = timegm(&tmpTime);
	else
		absTime = mktime(&tmpTime);

	return absTime;
}

void CStructTm::SetAbsTime(time_t absTime)
{
    struct tm sysTm;
    gmtime_r(&absTime,&sysTm);

	m_year 	= sysTm.tm_year;
	m_mon 	= sysTm.tm_mon;
	m_day 	= sysTm.tm_mday;
	m_hour 	= sysTm.tm_hour;
	m_min 	= sysTm.tm_min;
	m_sec 	= sysTm.tm_sec;

    m_mon  += 1;
    m_year += 1900;
}



void CStructTm::SetGmtTime()
{
                CStructTm curTime;
                SystemGetTime(curTime);
                time_t currTime = curTime.GetAbsTime(true);
                tm* pCurrGmtTimeTm;

                // time(&currTime);
                pCurrGmtTimeTm = gmtime(&currTime);

                m_year = curTime.m_year;
                m_mon  = pCurrGmtTimeTm->tm_mon + 1;
                m_day  = pCurrGmtTimeTm->tm_mday;

                m_hour = pCurrGmtTimeTm->tm_hour;
                m_min  = pCurrGmtTimeTm->tm_min;
                m_sec  = pCurrGmtTimeTm->tm_sec;


}

//=============================================================================
// FUNCTION:  CStructmpTime::IsPeriodOverLapping (static)
//-----------------------------------------------------------------------------
// PURPOSE: test whether two time periods are overlapping
// PARAMETERS:
//   INPUT  : Time1Start,Time1End,Time2Start,Time2End
// RETURN VALUE : TRUE if the time Period s overlaps
// ASSUMES : (Time1Start <= Time1End) and (Time2Start <= Time2End)
//=============================================================================


WORD CStructTm::IsPeriodOverLapping(const CStructTm& Time1Start,
									const CStructTm& Time1End,
									const CStructTm& Time2Start,
									const CStructTm& Time2End)
{
	if ((Time1End < Time2Start) || (Time2End < Time1Start ))
		return FALSE;
	else
		return TRUE;
}

CStructTm operator+(const CStructTm& l,const CStructTm& r)
{
	tm tmTime;
	l.GetAsTm(tmTime);

	tmTime.tm_year += r.m_year;
	tmTime.tm_mon += r.m_mon;
	tmTime.tm_mday += r.m_day;
	tmTime.tm_hour += r.m_hour;
	tmTime.tm_min += r.m_min;
	tmTime.tm_sec += r.m_sec;

	BOOL normalDate = FALSE; //normal date: 1 based month year > 1900, instead of 0 based month and small year
	if(tmTime.tm_year > 1900)
	{
		tmTime.tm_year -= 1900;
		tmTime.tm_mon -= 1;
		normalDate = TRUE;
	}

	timegm(&tmTime);

	if(normalDate == TRUE)
	{
		tmTime.tm_year += 1900;
		tmTime.tm_mon += 1;
	}

	CStructTm temp(tmTime);
    return temp;
}

std::ostream& operator<< (std::ostream& os, const CStructTm& obj )
{
	obj.Dump(os);
	return os;
}
