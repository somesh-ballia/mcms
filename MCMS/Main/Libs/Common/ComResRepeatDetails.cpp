#include "ComResRepeatDetails.h" 
#include "SystemFunctions.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"

char* CComResRepeatDetails::m_szDays[] = {"SUNDAY","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY",
										  "SATURDAY"};


CComResRepeatDetails::CComResRepeatDetails()
{
	SystemGetTime(m_LimitTime);
	m_RecurrenceType = eWeekly;
	m_OccurNumber = 1;

	for(int i=0; i < 7; i++)
		m_Week[i] = FALSE;

	m_EndBy = 0;
	m_Interval = 1;
	m_DayOfMonth=1;
	m_Instance=eFirst;
	m_DaysIndex=1;
	m_ByMonth=eByDate;
	m_nGMTOffsetMinutes = 0;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pRepeatExNode, *pRepeatTypeNode, *pGMTOffsetNode;
	int nGMTOffsetHour, nGMTOffsetMinute;

	pRepeatExNode = pFatherNode->AddChildNode("REPEATED_EX");

	if((m_RecurrenceType == eDaily) || (m_RecurrenceType == eWeekly))
	{
		if(m_RecurrenceType == eDaily)
			pRepeatTypeNode = pRepeatExNode->AddChildNode("DAILY");
		else
			pRepeatTypeNode = pRepeatExNode->AddChildNode("WEEKLY");

		for(int i=0; i < 7; i++)
			pRepeatTypeNode->AddChildNode(m_szDays[i],m_Week[i],_BOOL);

		pRepeatTypeNode->AddChildNode("TIME_INTERVAL",m_Interval);
	}
	else if(m_RecurrenceType == eMonthly)
	{
		pRepeatTypeNode = pRepeatExNode->AddChildNode("MONTHLY");
		pRepeatTypeNode->AddChildNode("MONTHLY_PATTERN",m_ByMonth,MONTHLY_PATTERN_ENUM);

		if(m_ByMonth == eByDate)
			pRepeatTypeNode->AddChildNode("DAY_OF_MONTH",m_DayOfMonth);
		else
			pRepeatTypeNode->AddChildNode("DAY_OF_MONTH",m_DaysIndex);

		pRepeatTypeNode->AddChildNode("TIME_INTERVAL",m_Interval);
		pRepeatTypeNode->AddChildNode("INSTANCE",m_Instance,INSTANCE_ENUM);
	}

	pRepeatExNode->AddChildNode("OCCUR_NUM",m_OccurNumber);
	pRepeatExNode->AddChildNode("LIMIT",m_EndBy,_BOOL);
	pRepeatExNode->AddChildNode("END_TIME",m_LimitTime);

	pGMTOffsetNode = pRepeatExNode->AddChildNode("GMT_OFFSET");

	nGMTOffsetMinute = m_nGMTOffsetMinutes;
	int nOffsetSign = 1;
	if(nGMTOffsetMinute < 0)
	{
		nGMTOffsetMinute = (-1) * nGMTOffsetMinute;
		nOffsetSign = -1;
	}

	nGMTOffsetHour = nGMTOffsetMinute / 60;
	nGMTOffsetMinute -= nGMTOffsetHour * 60;
	pGMTOffsetNode->AddChildNode("HOUR",nGMTOffsetHour);
	pGMTOffsetNode->AddChildNode("MINUTE",nGMTOffsetMinute);
	pGMTOffsetNode->AddChildNode("SIGN",nOffsetSign, SIGN_TYPE);
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::DeSerializeXml(CXMLDOMElement *pFatherNode,char *pszError,const char * strAction) //tbd zoe: what's the additional param????
{
	CXMLDOMElement *pRepeatNode;
	int nStatus;

	GET_CHILD_NODE(pFatherNode, "REPEATED_EX", pRepeatNode);

	if(pRepeatNode)
	{
		nStatus = DeSerializeXmlRepeatedEx(pRepeatNode,pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}
	else
	{
		GET_MANDATORY_CHILD_NODE(pFatherNode, "REPEATED_EX", pRepeatNode); 
		nStatus = DeSerializeXmlRepeated(pRepeatNode,pszError);
		if (nStatus!=STATUS_OK)
			return nStatus;
	}

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::DeSerializeXmlRepeatedEx(CXMLDOMElement *pRepeatedExNode,char *pszError)
{
	CXMLDOMElement *pRepeatTypeNode, *pGMTOffsetNode;
	BYTE bDaily = FALSE, bWeekly = FALSE;
	DWORD dwTemp;
	int nStatus = STATUS_FAIL, nOffsetHour = 0, nOffsetMinute = 0, nOffsetSign = 0;

	GET_CHILD_NODE(pRepeatedExNode, "DAILY", pRepeatTypeNode);

	if(pRepeatTypeNode)
		bDaily = TRUE;
	else
	{
		GET_CHILD_NODE(pRepeatedExNode, "WEEKLY", pRepeatTypeNode);

		if(pRepeatTypeNode)
			bWeekly = TRUE;		
		else
			GET_MANDATORY_CHILD_NODE(pRepeatedExNode, "MONTHLY", pRepeatTypeNode);
	}

	if(bDaily || bWeekly)
	{
		for(int i=0; i<7; i++)
		{
			m_Week[i] = FALSE;
			GET_VALIDATE_CHILD(pRepeatTypeNode,m_szDays[i],&(m_Week[i]),_BOOL);
		}

		m_Interval = 0;

		if(bDaily)
			m_RecurrenceType=eDaily;
		else
			m_RecurrenceType=eWeekly;

		GET_VALIDATE_CHILD(pRepeatTypeNode,"TIME_INTERVAL",&(m_Interval),_0_TO_DWORD);
	}
	else
	{
		m_RecurrenceType = eMonthly;
		GET_VALIDATE_CHILD(pRepeatTypeNode,"MONTHLY_PATTERN",&(m_ByMonth),MONTHLY_PATTERN_ENUM);
		GET_VALIDATE_CHILD(pRepeatTypeNode,"DAY_OF_MONTH",&dwTemp,_1_TO_31_DECIMAL);

		if(nStatus == STATUS_OK)
		{
			if (m_ByMonth == eByDate)
				m_DayOfMonth = dwTemp;
			else
				m_DaysIndex = dwTemp;
		}

		GET_VALIDATE_CHILD(pRepeatTypeNode,"TIME_INTERVAL",&(m_Interval),_0_TO_DWORD);
		GET_VALIDATE_CHILD(pRepeatTypeNode,"INSTANCE",&(m_Instance),INSTANCE_ENUM);
	}

	GET_VALIDATE_CHILD(pRepeatedExNode,"LIMIT",&(m_EndBy),_BOOL);
	GET_VALIDATE_CHILD(pRepeatedExNode,"END_TIME",&(m_LimitTime),DATE_TIME);

	GET_VALIDATE_CHILD(pRepeatedExNode,"OCCUR_NUM",&(m_OccurNumber),_0_TO_DWORD);

	GET_CHILD_NODE(pRepeatedExNode, "GMT_OFFSET", pGMTOffsetNode);

	if(nStatus == STATUS_OK)
	{
		GET_VALIDATE_CHILD(pGMTOffsetNode,"HOUR",&nOffsetHour,_0_TO_23_DECIMAL);
		GET_VALIDATE_CHILD(pGMTOffsetNode,"MINUTE",&nOffsetMinute,_0_TO_59_DECIMAL);
		GET_VALIDATE_CHILD(pGMTOffsetNode,"SIGN",&nOffsetSign,SIGN_TYPE);
		m_nGMTOffsetMinutes = (nOffsetHour*60 + nOffsetMinute) * nOffsetSign;
	}

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::DeSerializeXmlRepeated(CXMLDOMElement *pRepeatedNode,char *pszError)
{
	CXMLDOMElement *pRepeatTypeNode;
	int nStatus;
	CStructTm EndTime;

	GET_MANDATORY_CHILD_NODE(pRepeatedNode, "REPEATED_TYPE", pRepeatTypeNode);

	GET_VALIDATE_CHILD(pRepeatedNode, "REPEATED_TYPE", &m_RecurrenceType, REPEATED_TYPE_ENUM);

	if((m_RecurrenceType == eDaily) || (m_RecurrenceType == eWeekly))
	{
		for(int i=0; i<7; i++)
		{
			m_Week[i] = FALSE;
			GET_VALIDATE_CHILD(pRepeatedNode,m_szDays[i],&(m_Week[i]),_BOOL);
		}

		m_Interval = 0;
	}

	GET_VALIDATE_CHILD(pRepeatedNode,"LIMIT",&(m_EndBy),_BOOL);

	GET_VALIDATE_CHILD(pRepeatedNode,"END_TIME",&(m_LimitTime),DATE_TIME);

	GET_VALIDATE_CHILD(pRepeatedNode,"OCCUR_NUM",&(m_OccurNumber),_0_TO_DWORD);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////////////
CComResRepeatDetails& CComResRepeatDetails::operator = (const CComResRepeatDetails& other)
{
	m_LimitTime = other.m_LimitTime;
	m_RecurrenceType = other.m_RecurrenceType;
	m_OccurNumber = other.m_OccurNumber;

	for(int i=0; i < 7; i++)
		m_Week[i] = other.m_Week[i];

	m_EndBy = other.m_EndBy;
	m_Interval = other.m_Interval;
	m_DayOfMonth = other.m_DayOfMonth;
	m_Instance = other.m_Instance;
	m_DaysIndex = other.m_DaysIndex;
	m_ByMonth = other.m_ByMonth;
	m_nGMTOffsetMinutes = other.m_nGMTOffsetMinutes;
	
	return *this;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetLimitTime(CStructTm LimitTime)
{
	m_LimitTime = LimitTime;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetRecurrenceType(int nRecurrenceType)
{
	m_RecurrenceType = nRecurrenceType;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetOccurNumber(int nOccurNumber)
{
	m_OccurNumber = nOccurNumber;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetWeekDay(int nDayIndex, int bDay)
{
	m_Week[nDayIndex] = bDay;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetEndBy(int bEndBy)
{
	m_EndBy = bEndBy;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetInterval(int nInterval)
{
	m_Interval = nInterval;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetDayOfMonth(int nDayOfMonth)
{
	m_DayOfMonth = nDayOfMonth;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetInstance(int nInstance)
{
	m_Instance = nInstance;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetDaysIndex(int nDaysIndex)
{
	m_DaysIndex = nDaysIndex;
}
/////////////////////////////////////////////////////////////////////////////////////
void CComResRepeatDetails::SetByMonth(int bByMonth)
{
	m_ByMonth = bByMonth;
}
/////////////////////////////////////////////////////////////////////////////////////
CStructTm& CComResRepeatDetails::GetLimitTime() 
{
	return m_LimitTime;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetRecurrenceType() const
{
	return m_RecurrenceType;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetOccurNumber() const
{
	return m_OccurNumber;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetWeekDay(int nDayIndex) const
{
	return m_Week[nDayIndex];
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetEndBy() const
{
	return m_EndBy;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetInterval() const
{
	return m_Interval;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetDayOfMonth() const
{
	return m_DayOfMonth;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetInstance() const
{
	return m_Instance;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetDaysIndex() const
{
	return m_DaysIndex;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetByMonth() const
{
	return m_ByMonth;
}
/////////////////////////////////////////////////////////////////////////////////////
int CComResRepeatDetails::GetGMTOffsetMinutes() const
{
	return m_nGMTOffsetMinutes;
}

