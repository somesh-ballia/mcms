#ifndef ACTIVEALARMDEFINES_H_
#define ACTIVEALARMDEFINES_H_

#include <string>
#include "DataTypes.h"

using namespace std;












enum eStartupConditionStatus
{
	eStartupConditionWait = 0,
	eStartupConditionOk,
	eStartupConditionFail,
	
	NumOfStartupConditionStatuses
};










struct CActiveAlarm
{
	BYTE 	m_Subject;
	DWORD 	m_ErrorCode;
	BYTE 	m_ErrorLevel;
	string 	m_Description;
	bool 	m_IsForEma;
	bool 	m_IsForFaults;
	
public:	
	CActiveAlarm()
	{
		m_Subject 		= 0;
		m_ErrorCode 	= 0;
		m_ErrorLevel 	= 0;
		m_IsForEma 		= false;
		m_IsForFaults 	= false;
	}
	CActiveAlarm(BYTE subject, DWORD errorCode, BYTE errorLevel, const string &description, bool isForEma, bool isForFaults)
	:	m_Subject(subject), 
		m_ErrorCode(errorCode), 
		m_ErrorLevel(errorLevel), 
		m_Description(description), 
		m_IsForEma(isForEma), 
		m_IsForFaults(isForFaults)
	{}
	CActiveAlarm(const CActiveAlarm &other)
	{
		m_Subject 		= other.m_Subject;
		m_ErrorCode 	= other.m_ErrorCode;
		m_ErrorLevel 	= other.m_ErrorLevel;
		m_Description 	= other.m_Description;
		m_IsForEma 		= other.m_IsForEma;
		m_IsForFaults 	= other.m_IsForFaults;
	}
	
private:
	// disabled
	CActiveAlarm& operator=(const CActiveAlarm&);	
};












struct CStartupCondition
{
	CActiveAlarm 			m_ActiveAlarm;
	eStartupConditionStatus m_eStatus;
	bool 					m_isFaultOnly;   // If true does not contain real active alarm. Actually acts as fault when time comes

public:
	CStartupCondition()
	{
		m_eStatus = eStartupConditionFail;
		m_isFaultOnly = false;
	}
	CStartupCondition(const CActiveAlarm &aa, eStartupConditionStatus status, bool isFaultOnly)
	:m_ActiveAlarm(aa), m_eStatus(status), m_isFaultOnly(isFaultOnly)
	{}

	CStartupCondition(const CActiveAlarm &aa, eStartupConditionStatus status = eStartupConditionWait)
	:m_ActiveAlarm(aa), m_eStatus(status), m_isFaultOnly(false)
	{}

	CStartupCondition(const CStartupCondition &rHnd)
	:m_ActiveAlarm(rHnd.m_ActiveAlarm), m_eStatus(rHnd.m_eStatus), m_isFaultOnly(rHnd.m_isFaultOnly)
	{

	}

private:
	CStartupCondition& operator=(const CStartupCondition&);
};






#endif /*ACTIVEALARMDEFINES_H_*/
