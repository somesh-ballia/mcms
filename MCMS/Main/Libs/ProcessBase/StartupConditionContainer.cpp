#include <algorithm>
#include "StartupConditionContainer.h"


struct CStartupConditionByErrorCodeHunter : public unary_function<CStartupCondition*, bool>
{
public:
	CStartupConditionByErrorCodeHunter(WORD errorCode)
	{
		m_ErrorCode = errorCode;
	}
	
	bool operator () (CStartupCondition *& elem)
	{
		return elem->m_ActiveAlarm.m_ErrorCode == m_ErrorCode;
	}
	
private:
	WORD m_ErrorCode;
};



CStartupConditionContainer::CStartupConditionContainer()
{
}

CStartupConditionContainer::~CStartupConditionContainer()
{
	CStartupConditionVector::iterator iTer = begin();
	CStartupConditionVector::iterator iEnd = end();
	while(iEnd != iTer)
	{
		delete *iTer;
		
		iTer++;
	}
}

DWORD CStartupConditionContainer::AddCondition(CStartupCondition *cond)
{
	push_back(cond);
	return (DWORD)cond;
}

bool CStartupConditionContainer::UpdateConditionById(DWORD id, eStartupConditionStatus status)
{
	bool isFound = false;
	CStartupCondition *cond = (CStartupCondition*)id;
	if(end() != find(begin(), end(), cond))
	{
		cond->m_eStatus = status;
		isFound = true;
	} 
	return isFound;
}

bool CStartupConditionContainer::UpdateConditionByErrorCode(WORD errorCode,
							    eStartupConditionStatus status)
{
	bool isFound = false;

	CStartupConditionVector::iterator iFound = FindByErrorCode(errorCode);

	if(end() != iFound)
	{
		CStartupCondition *cond = *iFound;
		cond->m_eStatus = status;
		isFound = true;
	}

	return isFound;
}

bool CStartupConditionContainer::UpdateConditionDescriptionByErrorCode(WORD errorCode, const string & description)
{
	bool isFound = false;
	CStartupConditionVector::iterator iFound = FindByErrorCode(errorCode);
	if(end() != iFound)
	{
		CStartupCondition *cond = *iFound;
		cond->m_ActiveAlarm.m_Description = description;
		isFound = true;
	}
	return isFound;
}

CStartupConditionVector::iterator CStartupConditionContainer::FindByErrorCode(WORD errorCode)
{
	CStartupConditionByErrorCodeHunter hunter(errorCode);
	CStartupConditionVector::iterator iFound = find_if(begin(), end(), hunter);
	return iFound;
}

bool CStartupConditionContainer::IsThereStatus(eStartupConditionStatus status)
{
	CStartupConditionVector::iterator iTer = begin();
	CStartupConditionVector::iterator iEnd = end();
	while(iEnd != iTer)
	{
		CStartupCondition *cond = *iTer;
		if(status == cond->m_eStatus)
		{
			return true;
		}
		
		iTer++;
	}
	return false;
}

void CStartupConditionContainer::SwitchStatus(eStartupConditionStatus statusFrom, eStartupConditionStatus statusTo)
{
	CStartupConditionVector::iterator iTer = begin();
	CStartupConditionVector::iterator iEnd = end();
	while(iEnd != iTer)
	{
		CStartupCondition *cond = *iTer;
		if(statusFrom == cond->m_eStatus)
		{
			cond->m_eStatus = statusTo;
		}
		iTer++;
	}
}






