// CTaskStateContainer.cpp: implementation of the CTaskStateContainer class.
//
//
//Date         Updated By         Description
//
//8/11/05	  Yuri Ratner		Data Structure for Task States
//========   ==============   =====================================================================

#include <algorithm>

#include "FaultsContainer.h"
#include "StatusesGeneral.h"
#include "FaultsDefines.h"
#include "TraceHeader.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "FaultDesc.h"
#include "ApiStatuses.h"
#include "AlarmStrTable.h"



class CFaultElementHunterByCode : public unary_function<CLogFltElement*, bool>
{
public:
	CFaultElementHunterByCode(WORD errorCode)
	{
		m_ErrorCode = errorCode;
	}
	
	bool operator () (CLogFltElement *& elem)
	{
		return elem->GetCode() == m_ErrorCode;
	}
	
private:
	WORD m_ErrorCode;
};


class CFaultElementHunterByCodeDesc : public unary_function<CLogFltElement*, bool>
{
public:
	CFaultElementHunterByCodeDesc(WORD errorCode, const char *desc)
	{
		m_ErrorCode = errorCode;
        m_Desc = desc;
	}
	
	bool operator () (CLogFltElement *& elem)
	{
		bool res  = (elem->GetCode() == m_ErrorCode &&
                     0 == strcmp(elem->GetDescription(), m_Desc));
        return res;
	}
	
private:
	WORD m_ErrorCode;
    const char *m_Desc;
};




class CFaultElementHunterByCodeUserId : public unary_function<CLogFltElement*, bool>
{
public:
	CFaultElementHunterByCodeUserId(WORD errorCode, DWORD userId)
	{
		m_ErrorCode = errorCode;
		m_UserId	= userId;
	}
	
	bool operator () (CLogFltElement *& elem)
	{
		return (elem->GetCode() == m_ErrorCode && elem->GetUserId() == m_UserId);
	}
	
private:
	WORD m_ErrorCode;
	DWORD m_UserId;
};

class CFaultElementHunterUserId : public unary_function<CLogFltElement*, bool>
{
public:
	CFaultElementHunterUserId(DWORD userId)
	{
		m_UserId	= userId;
	}
	
	bool operator () (CLogFltElement *& elem)
	{
		return (elem->GetUserId() == m_UserId);
	}
	
private:
	DWORD m_UserId;
};

class CFaultElementHunterUniqueIndex : public unary_function<CLogFltElement*, bool>
{
public:
	CFaultElementHunterUniqueIndex(DWORD uniqueIndex)
	{
		m_UniqueIndex = uniqueIndex;
	}
	
	bool operator () (CLogFltElement *& elem)
	{
		return (elem->GetIndex() == m_UniqueIndex);
	}
	
private:
	DWORD m_UniqueIndex;
};






/*-------------------------------------------------------------------------------
class CFaultList
-------------------------------------------------------------------------------*/

CFaultList::CFaultList()
{
	m_FaultElementList	= new CFaultElementList;
	m_faultId			= 0;
//	m_isItCardStatusesList = NO;
}

CFaultList::~CFaultList()
{
	Clear();
	PDELETE(m_FaultElementList);
}

void CFaultList::Clear()
{
	CFaultElementList::iterator iTer = m_FaultElementList->begin();
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	while(iEnd != iTer)
	{
		CLogFltElement *elem = *iTer;
		PDELETE(elem);
		
		iTer++;
	}
	
	m_FaultElementList->clear();
}

bool CFaultList::operator == (const CFaultList &rHnd)
{
	DWORD size = GetSize();
	DWORD rHndSize = rHnd.GetSize();
	if(size != rHndSize)
	{
		return false;
	}
	
	CFaultElementList::iterator iTer 		= m_FaultElementList->begin();
	CFaultElementList::iterator iTerRHnd 	= rHnd.m_FaultElementList->begin();
	CFaultElementList::iterator iEnd 		= m_FaultElementList->end();
	while(iEnd != iTer)
	{
		CLogFltElement *elem = *iTer;
		CLogFltElement *elemRHnd = *iTerRHnd;
		
		if(*elem != *elemRHnd)
		{
			return false;
		}
		
		iTer++;
		iTerRHnd++;
	}
	
	return true;
}

DWORD CFaultList::GetSize()const
{
	return m_FaultElementList->size();
}

CFaultList* CFaultList::Clone()
{
	CFaultList *newFaultList = new CFaultList;
	CFaultElementList::iterator iTer = m_FaultElementList->begin();
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	while(iEnd != iTer)
	{
		CLogFltElement *elem = *iTer;
		CLogFltElement *newElem = new CLogFltElement(*elem);
		newFaultList->AddFault(newElem, false);
		
		iTer++;
	}

	return newFaultList;
}

CFaultElementList* CFaultList::GetFaultElementList()
{
	return m_FaultElementList;
}

void CFaultList::AddFault(CLogFltElement *flt, bool isAllocateId)
{
    if(isAllocateId)
    {    
        flt->SetFaultId(m_faultId);
        m_faultId++;
        if (m_faultId >= MAX_FAULT_ID)
        {    
            m_faultId = 0;
        }
	}
	m_FaultElementList->push_back(flt);
}

void CFaultList::ConcatFaultList(CFaultElementList *rHndList, bool isExternal)
{
	CFaultElementList::iterator iTer = rHndList->begin();
	CFaultElementList::iterator iEnd = rHndList->end();
	while(iEnd != iTer)
	{
		CLogFltElement *rHndElem = *iTer;
		
		// isExternal     rHndElem->GetIsExternal()   result
		//---------------------------------------------------
		//		0					0					1
		//		0					1					1
		//		1					0					0
		//		1					1					1
		if(!isExternal || rHndElem->GetIsExternal())
		{
			CLogFltElement *elem = new CLogFltElement(*rHndElem);
			AddFault(elem, false);
		}
		
		iTer++;
	}
}

bool CFaultList::RemoveFaultByErrorCode(WORD errorCode, std::list<CFaultDesc *>* pFaultList)
{
	CFaultElementList::iterator iFnd = GetFaultElementByErrorCode(errorCode);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	if(iEnd == iFnd)
	{
		return false;
	}
	
	CLogFltElement *fltElement = *iFnd;
	if (pFaultList != NULL)
	{
	    CFaultDesc *pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(fltElement->GetDescription());
	    // allocated pointer will be deallocated from the calling function
	    pFaultList-> push_back(pFaultDesc);
	}

	m_FaultElementList->remove(fltElement);

	PDELETE(fltElement);
	
	return true;
}

bool CFaultList::RemoveFaultByErrorCodeUserId(WORD errorCode, DWORD userId, std::list<CFaultDesc *>* pFaultList)
{
	CFaultElementList::iterator iFnd = GetFaultElementByErrorCodeUserId(errorCode, userId);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	if(iEnd == iFnd)
	{
		return false;
	}
	
	CLogFltElement *fltElement = *iFnd;
	if (pFaultList != NULL)
	{
	    CFaultDesc *pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(fltElement->GetDescription());
	    // allocated pointer will be deallocated from the calling function
	    pFaultList-> push_back(pFaultDesc);
	}
	m_FaultElementList->remove(fltElement);
	PDELETE(fltElement);
	
	return true;
}

bool CFaultList::RemoveFaultByUserId(DWORD userId, std::list<CFaultDesc *>* pFaultList)
{
	CFaultElementList::iterator iFnd = GetFaultElementByUserId(userId);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	if(iEnd == iFnd)
	{
		return false;
	}
	
	while(iEnd != iFnd)
	{
		CLogFltElement *fltElement = *iFnd;

		if (pFaultList != NULL)
		{
		    CFaultDesc *pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(fltElement->GetDescription());
		    // allocated pointer will be deleted from the calling function
		    pFaultList-> push_back(pFaultDesc);
		}
		m_FaultElementList->remove(fltElement);
		PDELETE(fltElement);
		
		iFnd = GetFaultElementByUserId(userId);
	}
	
	return true;
}
DWORD CFaultList::RemoveFaultsIfThereAreMoreThanGivenMax(DWORD maxAllowedElements, DWORD numRecommendedElems)
{
	if(m_FaultElementList->size() > maxAllowedElements)
	{
		if (numRecommendedElems > maxAllowedElements)
		{
			numRecommendedElems = maxAllowedElements;
		}
		DWORD numElemsToDel = m_FaultElementList->size() - numRecommendedElems ;


		for (DWORD i = 0; i < numElemsToDel; ++i )
		{
			m_FaultElementList->erase(m_FaultElementList->begin());
		}

		return numElemsToDel;
	}
	return 0;
}
bool CFaultList::IsFaultExistByUserId(DWORD userId)
{


	CFaultElementList::iterator iFnd = GetFaultElementByUserId(userId);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	if(iEnd == iFnd)
	{
		return false;
	}


	return true;
}


bool CFaultList::RemoveFault(CLogFltElement *fltElement, std::list<CFaultDesc *>* pFaultList)
{
	CFaultElementList::iterator iFnd = GetFaultElement(fltElement);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	
	if(iEnd == iFnd)
	{
		PASSERTMSG(1, "Illegal remove");
		return false;
	}

	if (pFaultList != NULL)
	{
	    CFaultDesc *pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(fltElement->GetDescription());
	    // allocated pointer will be deallocated from the calling function
	    pFaultList-> push_back(pFaultDesc);
	}

	m_FaultElementList->remove(fltElement);
	PDELETE(fltElement);
	
	return true;
}

bool CFaultList::IsFaultExist(CLogFltElement *fltElement)const
{	
	CFaultElementList::iterator iFnd = GetFaultElement(fltElement);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	
	return iEnd != iFnd;
}

bool CFaultList::IsFaultExist(WORD errorCode)const
{
	CFaultElementList::iterator iFnd = GetFaultElementByErrorCode(errorCode);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	
	return iEnd != iFnd;
}

bool CFaultList::IsFaultExist(WORD errorCode, const char *desc)const
{
	CFaultElementList::iterator iFnd = GetFaultElementByErrorCodeDescription(errorCode, desc);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	
	return iEnd != iFnd;
}

bool CFaultList::IsFaultExist(WORD errorCode, DWORD userId)const
{
	CFaultElementList::iterator iFnd = GetFaultElementByErrorCodeUserId(errorCode, userId);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	
	return iEnd != iFnd;
}

bool CFaultList::IsFaultExistByUniqueIndex(DWORD uniqueIndex)const
{
    CFaultElementList::iterator iFnd = GetFaultElementByUniqueIndex(uniqueIndex);
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	
	return iEnd != iFnd;
}


void CFaultList::Serialize(CSegment &seg)
{
	DWORD len = m_FaultElementList->size();
	seg << (DWORD)len;
	CFaultElementList::iterator iTer = m_FaultElementList->begin();
	CFaultElementList::iterator iEnd = m_FaultElementList->end();
	while(iEnd != iTer)
	{
		CLogFltElement *elem = (*iTer);
		((CHlogElement*)elem)->Serialize(seg);
		
		iTer++;
	}
}

void CFaultList::DeSerialize(CSegment &seg)
{
	DWORD len;
	seg >> len;
	
	while(len > 0 && !seg.EndOfSegment())
	{
		CLogFltElement *elem = new CLogFltElement;
		((CHlogElement*)elem)->DeSerialize(seg);
		AddFault(elem, false);	
		len--;
	}
}

void CFaultList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	SerializeXml(pFatherNode, UPDATE_CNT_BEGIN_END, 0);
}

void CFaultList::SerializeXml(CXMLDOMElement*& pFatherNode, DWORD objToken, DWORD id) const
{
	CXMLDOMElement* pListNode = NULL;
//	if (NO == m_isItCardStatusesList)
//	{
		pListNode = pFatherNode->AddChildNode("ACTIVE_ALARMS_LIST");
//	}
//	else
//	{
//		pListNode = pFatherNode->AddChildNode("CARD_STATUSES_LIST");
//	}
	

	DWORD listLen = GetSize();
	DWORD startIndex = (listLen > id ? id : 0);
	
//	if (NO == m_isItCardStatusesList)
//	{
		pListNode->AddChildNode("ID", listLen); // ???

		WORD isChanged = InsertUpdateCntChanged(pListNode, objToken);
		if(FALSE == isChanged)
		{
			return;
		}
//	}

	
	// loop over logElements
	// GetFirstFaultElement
	CFaultElementList::iterator iter = begin();
	CLogFltElement *pCurFaultElement = (end() != iter ? *iter : NULL);

	DWORD index = 0;
	while( NULL != pCurFaultElement )
	{
//		if (YES == m_isItCardStatusesList)
//		{
//			pCurFaultElement->SetIsItCardStatus(YES);
//		}
		
		if(index >= startIndex)
		{
			pCurFaultElement->SerializeXml(pListNode);
		}
		index++;
		
		// GetNextFaulconsttElement
		if( end() != iter )
		{
			iter++;	
		}
	
		pCurFaultElement = (end() != iter ? *iter : NULL);
	}
}



int CFaultList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	// cleanup
	Clear();

	// get <FAULTS_LIST> section
	CXMLDOMElement*	pFaultsListNode = NULL;
//	if (NO == m_isItCardStatusesList)
//	{
		GET_CHILD_NODE(pActionNode,"ACTIVE_ALARMS_LIST",pFaultsListNode);
//	}
//	else
//	{
//		GET_CHILD_NODE(pActionNode,"CARD_STATUSES_LIST",pFaultsListNode);
//	}

	// get fields from <FAULTS_LIST> section
	if( pFaultsListNode )
	{
//		if (NO == m_isItCardStatusesList)
//		{
			DWORD	dummy;
			GET_VALIDATE_CHILD(pFaultsListNode,"ID",&dummy,_0_TO_DWORD);
//		}

		CXMLDOMElement*	pHlogNode		= NULL;
		char*			pszChildName	= NULL;


		CLogFltElement* pHlogFltElement = NULL;

		pFaultsListNode->firstChildNode(&pHlogNode);

		while( pHlogNode && nStatus == STATUS_OK ) 
		{
			pHlogNode->get_nodeName(&pszChildName);

//			if( !strcmp(pszChildName,"HLOG_ELEMENT") ) {
//				pHlogElement = new CHlogElement;
//			} else if( !strcmp(pszChildName,"HLOG_DRV_ELEMENT") ) {
//				pHlogElement = new CLogElementDrv;
//			} elsconste if( !strcmp(pszChildName,"LOG_ACCNT_ELEMENT") ) {
//				pHlogElement = new CLogAccntElement;
//			} else if( !strcmp(pszChildName,"LOG_FAULT_ELEMENT") ) {
				pHlogFltElement = new CLogFltElement;
//			} else if( !strcmp(pszChildName,"LOG_NET_ELEMENT") ) {
//				pHlogElement = new CLogNetElement;
//			}

			if( NULL != pHlogFltElement ) 
			{ 
				// if created successfully
//				if (YES == m_isItCardStatusesList)
//				{
//					pHlogFltElement->SetIsItCardStatus(YES);
//				}
				nStatus = pHlogFltElement->DeSerializeXml(pHlogNode,pszError,action);

				if( nStatus )
				{
					POBJDELETE(pHlogFltElement);
					break;
				}

				AddFault(pHlogFltElement);
			}

			pFaultsListNode->nextChildNode(&pHlogNode);
		}
	}

	return STATUS_OK;
}


CLogFltElement* CFaultList::GetFirstFaultElement()
{
	m_currentIter = begin();

	CLogFltElement *ret = (end() != m_currentIter ? *m_currentIter : NULL);
	return ret;
}


CLogFltElement* CFaultList::GetNextFaultElement()
{
	if( end() != m_currentIter )
	{
		m_currentIter++;	
	}
	
	CLogFltElement *ret = (end() != m_currentIter ? *m_currentIter : NULL);
	return ret;	
}




CFaultElementList::iterator CFaultList::GetFaultElementByErrorCode(WORD errorCode)const
{
	CFaultElementHunterByCode hunter(errorCode);
	CFaultElementList::iterator iFnd = find_if(m_FaultElementList->begin(), m_FaultElementList->end(), hunter);
	
	return iFnd;
}

CFaultElementList::iterator CFaultList::GetFaultElementByErrorCodeDescription(WORD errorCode, const char *desc)const
{
	CFaultElementHunterByCodeDesc hunter(errorCode, desc);
	CFaultElementList::iterator iFnd = find_if(m_FaultElementList->begin(), m_FaultElementList->end(), hunter);
	
	return iFnd;
}

CFaultElementList::iterator CFaultList::GetFaultElementByUserId(DWORD userId)const
{
	CFaultElementHunterUserId hunter(userId);
	CFaultElementList::iterator iFnd = find_if(m_FaultElementList->begin(), m_FaultElementList->end(), hunter);
	
	return iFnd;
}

CFaultElementList::iterator CFaultList::GetFaultElementByErrorCodeUserId(WORD errorCode, DWORD userId)const
{
	CFaultElementHunterByCodeUserId hunter(errorCode, userId);
	CFaultElementList::iterator iFnd = find_if(m_FaultElementList->begin(), m_FaultElementList->end(), hunter);
	
	return iFnd;
}

CFaultElementList::iterator CFaultList::GetFaultElementByUniqueIndex(DWORD uniqueIndex)const
{
    CFaultElementHunterUniqueIndex hunter(uniqueIndex);
	CFaultElementList::iterator iFnd = find_if(m_FaultElementList->begin(), m_FaultElementList->end(), hunter);
	
	return iFnd;
}


CFaultElementList::iterator CFaultList::GetFaultElement(CLogFltElement *fltElement)const
{
	CFaultElementList::iterator iFnd = find(m_FaultElementList->begin(), m_FaultElementList->end(), fltElement);
	
	return iFnd;
}


bool CFaultList::UpdateActiveAlarmByErrorCode(WORD errorCode, const string & description)
{
	bool isFound = false;
	CFaultElementList::iterator iFound = GetFaultElementByErrorCode(errorCode);
	if(iFound != m_FaultElementList->end())
	{
		CLogFltElement *flt = *iFound;
		flt->UpdateDescription(description.c_str());
		isFound = true;
	}
	return isFound;
}


void CFaultList::SetFaultIdAccordingToLastElement()
{
	if ( !m_FaultElementList->empty() )
	{
		CFaultElementList::iterator iEnd = m_FaultElementList->end();
		CLogFltElement *elem = *iEnd;
		DWORD lastId = elem->GetFaultId();

		if (lastId >= m_faultId)
			m_faultId = lastId+1;
		else
			m_faultId++;
	}
	else
	{
		m_faultId = 0;
	}
	
	if (m_faultId >= MAX_FAULT_ID)
		m_faultId = 0;
}

DWORD CFaultList::GetFaultId()
{
	return m_faultId;
}

/*
void CFaultList::SetIsItCardStatusesList(BOOL isIt)
{
	m_isItCardStatusesList = isIt;
}

BOOL CFaultList::GetIsItCardStatusesList()
{
	return m_isItCardStatusesList;
}
*/







/*-------------------------------------------------------------------------------
class CStateFaultList
-------------------------------------------------------------------------------*/

CStateFaultList::CStateFaultList()
{
	m_pFaultList = new CFaultList;
}

CStateFaultList::CStateFaultList(const CStateFaultList &rHnd)
:CPObject(rHnd)
{
	m_pFaultList = rHnd.m_pFaultList->Clone();
}

CStateFaultList::CStateFaultList(CFaultList *list)
{
	m_pFaultList = list;
}

CStateFaultList::~CStateFaultList()
{
	PDELETE(m_pFaultList);
}

bool CStateFaultList::operator == (const CStateFaultList &rHnd)
{
	return true;
}

CFaultList* CStateFaultList::GetFaultList()
{
	return m_pFaultList;
}

void CStateFaultList::SetFaultList(CFaultList *faultList)
{
	PDELETE(m_pFaultList);
	m_pFaultList = faultList;
}

void CStateFaultList::Serialize(CSegment &seg)
{
	m_pFaultList->Serialize(seg);
}

void CStateFaultList::DeSerialize(CSegment &seg)
{
	m_pFaultList->DeSerialize(seg);
}

CLogFltElement* CStateFaultList::GetFirstFaultElement()
{
	m_CurrentIter = m_pFaultList->begin();
	CLogFltElement *ret = (m_pFaultList->end() != m_CurrentIter ? *m_CurrentIter : NULL);
	
	return ret;
}

CLogFltElement* CStateFaultList::GetNextFaultElement()
{
	if(m_pFaultList->end() != m_CurrentIter)
	{
		m_CurrentIter++;	
	}
	CLogFltElement *ret = (m_pFaultList->end() != m_CurrentIter ? *m_CurrentIter : NULL);
	
	return ret;	
}

void CStateFaultList::ClearList()
{
	m_pFaultList->Clear();
}

































/*-------------------------------------------------------------------------------
class CTaskStateFaultList
-------------------------------------------------------------------------------*/

CTaskStateFaultList::CTaskStateFaultList()
{
	m_TaskStatus = eTaskNormal;
	m_TaskState	 = eTaskStateIdle;
}

CTaskStateFaultList::CTaskStateFaultList(const string &taskName)
{
	m_TaskStatus = eTaskNormal;
	m_TaskState	 = eTaskStateIdle;
	m_TaskName 	 = taskName;
}

/*
CTaskStateFaultList::CTaskStateFaultList(string taskName, eTaskStatus status)
{
	m_TaskStatus = status;
	eTaskStateIdle = 
	m_TaskName = taskName;
}
*/
CTaskStateFaultList::CTaskStateFaultList(const string &taskName, eTaskStatus status, eTaskState state, CFaultList *list)
: CStateFaultList(list)
{
	m_TaskStatus 	= status;
	m_TaskState		= state;
	m_TaskName 		= taskName;
}

CTaskStateFaultList::CTaskStateFaultList(const CTaskStateFaultList &rHnd)
:CStateFaultList(rHnd)
{
	m_TaskStatus 	= rHnd.m_TaskStatus;
	m_TaskState	 	= rHnd.m_TaskState; 
	m_TaskName 		= rHnd.m_TaskName;
}

CTaskStateFaultList::~CTaskStateFaultList()
{}

bool CTaskStateFaultList::operator == (const CTaskStateFaultList &rHnd)
{
	bool result = CStateFaultList::operator ==(rHnd);
	if(false == result)
	{
		return false;
	}
	
	result = (	m_TaskStatus 	== rHnd.m_TaskStatus 	&&
				m_TaskState	 	== rHnd.m_TaskState		&& 
				m_TaskName 		== rHnd.m_TaskName);
	
	return result;
}

CTaskStateFaultList* CTaskStateFaultList::Clone()const
{
	CTaskStateFaultList *newEntry = new CTaskStateFaultList(*this);
	
	return newEntry;
}

const string& CTaskStateFaultList::GetTaskName()const
{
	return m_TaskName;
}
	
void CTaskStateFaultList::SetTaskName(const string & taskName)
{
	m_TaskName = taskName;
}

eTaskStatus CTaskStateFaultList::GetTaskStatus()const
{
	return m_TaskStatus;
}

void CTaskStateFaultList::SetTaskStatus(eTaskStatus taskStatus)
{
	m_TaskStatus = taskStatus;
}

eTaskState CTaskStateFaultList::GetTaskState()const
{
	return m_TaskState;
}

void CTaskStateFaultList::SetTaskState(eTaskState state)
{
	m_TaskState = state;
}

void CTaskStateFaultList::Serialize(CSegment &seg)
{
	seg << m_TaskName;
    
	DWORD tmp = static_cast<DWORD>(m_TaskStatus);
	seg << tmp;
	
	tmp = static_cast<DWORD>(m_TaskState);
	seg << tmp;
	
	CStateFaultList::Serialize(seg);
}

void CTaskStateFaultList::DeSerialize(CSegment &seg)
{
	seg >> m_TaskName;
	
	DWORD tmp;
	seg >> tmp;
	m_TaskStatus = static_cast<eTaskStatus>(tmp);
	
	seg >> tmp;
	m_TaskState = static_cast<eTaskState>(tmp);
	
	CStateFaultList::DeSerialize(seg);
}

void CTaskStateFaultList::DumpActiveAlarmList(std::ostream& answer)
{
	CLogFltElement * flt = GetFirstFaultElement();
	while(NULL != flt)
	{
        string description;
		flt->GetCleanDescription(description);

        const char *strErrorCode = GetAlarmName(flt->GetCode());
        const char *strIsExternal = (flt->GetIsExternal()
                                     ?
                                     "External" : "Internal");
        
		answer << strIsExternal << " : " << strErrorCode << " : " << description.c_str() << endl;
		
		flt = GetNextFaultElement();
	}
}


















/*-------------------------------------------------------------------------------
class CProcessStateFaultList
-------------------------------------------------------------------------------*/

CProcessStateFaultList::CProcessStateFaultList()
{
	m_ProcessType = eProcessTypeInvalid;
	m_ProcessState = eProcessInvalid;
}

CProcessStateFaultList::CProcessStateFaultList(eProcessType processType, eProcessStatus processState, CFaultList *faultList)
:CStateFaultList(faultList)
{
	m_ProcessType = processType;
	m_ProcessState = processState;
}

CProcessStateFaultList::~CProcessStateFaultList()
{
	
}

eProcessType CProcessStateFaultList::GetProcessType()
{
	return m_ProcessType;
}

void CProcessStateFaultList::SetProcessType(eProcessType processType)
{
	m_ProcessType = processType;
}

eProcessStatus CProcessStateFaultList::GetProcessStatus()
{
	return m_ProcessState;
}

void CProcessStateFaultList::SetProcessStatus(eProcessStatus processState)
{
	m_ProcessState = processState;
}

void CProcessStateFaultList::Serialize(CSegment &seg)
{
	DWORD tmp = static_cast<DWORD>(m_ProcessType);
	seg << tmp;
	tmp = static_cast<DWORD>(m_ProcessState);
	seg << tmp;
	CStateFaultList::Serialize(seg);
}

void CProcessStateFaultList::DeSerialize(CSegment &seg)
{
	DWORD tmp;
	seg >> tmp;
	m_ProcessType = static_cast<eProcessType>(tmp);
	seg >> tmp;
	m_ProcessState = static_cast<eProcessStatus>(tmp);
	CStateFaultList::DeSerialize(seg);
}

















/*-------------------------------------------------------------------------------
class CStateFaultListMap
-------------------------------------------------------------------------------*/

CStateFaultListMap::CStateFaultListMap()
{
	m_StateFaultListMap = new CStateFaultListMap_Type;
	m_CurrentIter = m_StateFaultListMap->end();
}


CStateFaultListMap::~CStateFaultListMap()
{	
	CLear();
	PDELETE(m_StateFaultListMap);
}

void CStateFaultListMap::CLear()
{
	CStateFaultListMap_Type::iterator iTer = m_StateFaultListMap->begin();
	CStateFaultListMap_Type::iterator iEnd = m_StateFaultListMap->end();
	while(iEnd != iTer)
	{
		PDELETE(iTer->second);

		iTer++;
	}
	m_StateFaultListMap->clear();
}

CStateFaultList* CStateFaultListMap::GetFirstStateFaultList()
{
	m_CurrentIter = m_StateFaultListMap->begin();
	CStateFaultList *ret = (m_StateFaultListMap->end() != m_CurrentIter ? m_CurrentIter->second : NULL);
	
	return ret;
}

CStateFaultList* CStateFaultListMap::GetNextStateFaultList()
{
	if(m_StateFaultListMap->end() != m_CurrentIter)
	{
		m_CurrentIter++;	
	}
	CStateFaultList *ret = (m_StateFaultListMap->end() != m_CurrentIter ? m_CurrentIter->second : NULL);
	
	return ret;
}


void CStateFaultListMap::AddNewEntry(string entryKey, CStateFaultList *stateFaultList)
{
	(*m_StateFaultListMap)[entryKey] = stateFaultList;
}

bool CStateFaultListMap::UpdateEntry(string entryKey, CStateFaultList *entryStateFaultList)
{
	CStateFaultList *elem = GetStateFaultList(entryKey);
	if(NULL == elem)
	{
		return false;
	}

	PDELETE(elem);
	(*m_StateFaultListMap)[entryKey] = entryStateFaultList;
	
	return true;
}

CStateFaultList* CStateFaultListMap::GetStateFaultList(string entryKey)
{
	CStateFaultList *elem = (*m_StateFaultListMap)[entryKey];
	return elem;
}

bool CStateFaultListMap::IsEntryExist(string entryKey)
{
	CStateFaultList *elem = (*m_StateFaultListMap)[entryKey];
	return (NULL != elem);
}












/*-------------------------------------------------------------------------------
class CTaskStateFaultListMap
-------------------------------------------------------------------------------*/

CTaskStateFaultListMap::CTaskStateFaultListMap()
{
	
}

CTaskStateFaultListMap::~CTaskStateFaultListMap()
{
	
}

void CTaskStateFaultListMap::AddNewTask(string taskName)
{
	AddNewEntry(taskName, new CTaskStateFaultList(taskName));
}

void CTaskStateFaultListMap::UpdateTask(string taskName, CTaskStateFaultList *taskStateFaultList)
{
	bool result = UpdateEntry(taskName, taskStateFaultList); 	
	
	// the task is not registrated, it will registrate it now(CMfaTasks, CLobby)
	if(false == result)
	{
		AddNewEntry(taskName, new CTaskStateFaultList(taskName));
		UpdateEntry(taskName, taskStateFaultList); 		
	}
}


eProcessStatus CTaskStateFaultListMap::CalculateProcessState()
{
	eProcessStatus processState = eProcessNormal;
	CTaskStateFaultList *taskStateFaultlist = GetFirstTaskStateFaultList();
	while(NULL != taskStateFaultlist)
	{
		eTaskState taskState = taskStateFaultlist->GetTaskState();
		eTaskStatus taskStatus = taskStateFaultlist->GetTaskStatus();
		processState = TaskProcessStateTable(processState, taskState, taskStatus);
//		if(eProcessMajor == processState)
//		{
//			break;
//		}
		
		PASSERTMSG(eProcessInvalid == processState, "Bad flow");
		
		taskStateFaultlist = GetNextTaskStateFaultList();
	}
	return processState;
}


eProcessStatus CTaskStateFaultListMap::TaskProcessStateTable(	eProcessStatus 	oldProcessState, 
																eTaskState 		taskState, 
																eTaskStatus 	taskStatus)
{
	eProcessStatus newProcessState = eProcessInvalid;
	
	// state
	if(eTaskStateIdle == taskState)
	{
		newProcessState = eProcessIdle;
	}
	else if(eTaskStateStartup == taskState)
	{
		newProcessState = eProcessStartup;
	}
	
	// status
	else if(eTaskMinor == taskStatus)
	{
		newProcessState = (eProcessMajor == oldProcessState ? oldProcessState : eProcessMinor);
	}
	else if(eTaskMajor == taskStatus)
	{
		newProcessState = eProcessMajor;
	}
	else if(eTaskNormal == taskStatus)
	{
		newProcessState = oldProcessState;
	}
	else
	{
		PASSERTMSG(1, "Illegal task status");
	}
	
/*	
	if(eTaskIdle == taskStatus)
	{
		newProcessState = (eProcessMinor == oldProcessState ? oldProcessState : eProcessIdle);
		newProcessState = (eProcessMajor == oldProcessState ? oldProcessState : eProcessIdle);
	}
	else if(eTaskStartUp == taskStatus)
	{
		newProcessState = (eProcessNormal == oldProcessState ? eProcessStartup : oldProcessState);
	}
	else if(eTaskMinor == taskStatus)
	{
		newProcessState = (eProcessMajor == oldProcessState ? oldProcessState : eProcessMinor);
	}
	else if(eTaskMajor == taskStatus)
	{
		newProcessState = eProcessMajor;
	}
	else if(eTaskNormal == taskStatus)
	{
		newProcessState = oldProcessState;
	}
	else
	{
		PASSERTMSG(1, "Illegal task status");
	}
*/	
	return newProcessState;	
}

CTaskStateFaultList* CTaskStateFaultListMap::GetFirstTaskStateFaultList()
{
	CTaskStateFaultList *taskStateFaultlist = dynamic_cast<CTaskStateFaultList*>(GetFirstStateFaultList());
	return taskStateFaultlist;
}

CTaskStateFaultList* CTaskStateFaultListMap::GetNextTaskStateFaultList()
{
	CTaskStateFaultList *taskStateFaultlist = dynamic_cast<CTaskStateFaultList*>(GetNextStateFaultList());
	return taskStateFaultlist;
}

void CTaskStateFaultListMap::GetCommonFaultList(CFaultList *faultList)
{
	CTaskStateFaultList *taskStateFaultlist = GetFirstTaskStateFaultList();
	while(NULL != taskStateFaultlist)
	{   
        faultList->ConcatFaultList(taskStateFaultlist->GetFaultList()->GetFaultElementList(), true);
		
		taskStateFaultlist = GetNextTaskStateFaultList();
	}
}















/*-------------------------------------------------------------------------------
class CProcessStateFaultListMap
-------------------------------------------------------------------------------*/

CProcessStateFaultListMap::CProcessStateFaultListMap()
{
	
}

CProcessStateFaultListMap::~CProcessStateFaultListMap()
{
	
}
	
eProcessStatus CProcessStateFaultListMap::GetWorstProcessState(eProcessType &worstProcessType)
{
	eProcessStatus tmpState   = eProcessNormal,
	              worstState = eProcessNormal;

	// loop over FaultLists in map
	CProcessStateFaultList *pStateFaultlist = GetFirstProcessStateFaultList();
	while( NULL != pStateFaultlist )
	{
		tmpState = pStateFaultlist->GetProcessStatus();
		
		if ( (eProcessInvalid == tmpState) || ( tmpState >  worstState) )
		{
			worstState       = tmpState;
			worstProcessType = pStateFaultlist->GetProcessType();
		}
		
		pStateFaultlist = GetNextProcessStateFaultList(); 
	}

	return worstState;
}

CProcessStateFaultList* CProcessStateFaultListMap::GetFirstProcessStateFaultList()
{
	CProcessStateFaultList *pProcessStateFaultlist = dynamic_cast<CProcessStateFaultList*>(GetFirstStateFaultList());
	return pProcessStateFaultlist;
}

CProcessStateFaultList* CProcessStateFaultListMap::GetNextProcessStateFaultList()
{
	CProcessStateFaultList *pProcessStateFaultlist = dynamic_cast<CProcessStateFaultList*>(GetNextStateFaultList());
	return pProcessStateFaultlist;
}

