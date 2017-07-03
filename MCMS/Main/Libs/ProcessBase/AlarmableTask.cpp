// AlarmableTask.cpp

#include "AlarmableTask.h"

#include "HlogElement.h"
#include "FaultDesc.h"
#include "FaultsContainer.h"
#include "FaultsDefines.h"
#include "DefinesGeneral.h"
#include "HlogApi.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsInternal.h"
#include "StartupConditionContainer.h"
#include "StartupCondDependTree.h"
#include "TraceStream.h"
#include "ManagerApi.h"
#include "Segment.h"
#include "ObjString.h"
#include "OpcodesMcmsCommon.h"
#include "UniqueIndex.h"
#include "AlarmStrTable.h"
#include "SysConfig.h"
#include "AlarmParamValidator.h"



PBEGIN_MESSAGE_MAP(CAlarmableTask)
  ONEVENT(STARTUP_CONDITION_CONFIGURATION_TIMER, ANYCASE, CAlarmableTask::HandleStartupConditionTimerEnd)
	ONEVENT(REMOVE_ALL_AA, ANYCASE, CAlarmableTask::HandleRemoveAllAA)
	ONEVENT(SET_ENABLE_DISABLE_AA, ANYCASE, CAlarmableTask::HandleSetEnableDisableAA)

PEND_MESSAGE_MAP(CAlarmableTask, CStateMachine);


const string CAlarmableTask::CLEANING_FAULT_PREFIX = "Cleaning Existing Fault: ";

const string CAlarmableTask::CLEANING_ALARM_PREFIX = "Alarm Cleared: ";


const DWORD CAlarmableTask::MAX_FAULTLIST_LEN = 110 ;
const DWORD CAlarmableTask::RECOMMENDED_FAULTLIST_LEN = 100;

CAlarmableTask::CAlarmableTask()
{
	m_TaskStatus = eTaskNormal;
	m_TaskState	= eTaskStateIdle;

	m_AlarmFaultList = new CFaultList;
	m_FaultOnlyList = new CFaultList;

	m_StartupCondDependTree	= new CStartupCondDependForest;
	m_IsStartupConditionTreeEnabled = true;
	m_IsCanAddAA = true;
	m_IsCanAddAAFaultOnly = true;
	m_AlarmUniqueIndex = new CUniqueIndex;

}

CAlarmableTask::~CAlarmableTask()
{
	POBJDELETE(m_StartupCondDependTree);
	POBJDELETE(m_AlarmFaultList);
	POBJDELETE(m_AlarmUniqueIndex);
	POBJDELETE(m_FaultOnlyList);
}

void  CAlarmableTask::Create(CSegment& appParam, WORD limited)
{
	CTaskApp::Create(appParam, limited);
	AlarmableTaskCreateEndPoint();
}

void CAlarmableTask::AlarmableTaskCreateEndPoint()
{
	m_TaskState	 = eTaskStateReady;
	ComputeSetTaskStatus();
}

bool CAlarmableTask::IsThereStartupCondition()
{
    bool isWait = m_StartupCondDependTree->IsThereStatus(eStartupConditionWait, true);
    return isWait;
}

/////////////////////////////////////////////////////////////////////////////

void CAlarmableTask::UpdateActiveAlarmDescriptionByErrorCode(DWORD errorCode, const string & description)
{
	bool isFound = m_StartupCondDependTree->UpdateStartupConditionDescriptionByErrorCode(errorCode, description);
    isFound = m_AlarmFaultList->UpdateActiveAlarmByErrorCode(errorCode, description);
    if(isFound)
    {
        const char *strErrorCode = GetAlarmName(errorCode);
        CMedString message = "Update Alert : ";
		message << strErrorCode << " : " << description.c_str();
        PTRACE(eLevelInfoNormal, message.GetString());

        ComputeSetTaskStatus();
    }
}

/////////////////////////////////////////////////////////////////////////////

void CAlarmableTask::UpdateActiveAlarmFaultOnlyDescriptionByErrorCode(DWORD errorCode, const string & description)
{
	m_StartupCondDependTree->UpdateStartupConditionDescriptionByErrorCode(errorCode, description);

	UpdateActiveAlarmFaultOnlyDescriptionIfExist(description, errorCode, 0xFFFFFFFF, false);
}
/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::UpdateActiveAlarmFaultOnlyDescriptionIfExist(const string & description,
														DWORD errorCode, DWORD userId, BOOL alwaysSend)
{

	CFaultElementList::iterator iFnd ;
	if (userId != 0xFFFFFFFF)
	{
			iFnd =m_FaultOnlyList->GetFaultElementByErrorCodeUserId(errorCode, userId);			
	}
	else
	{
			iFnd =m_FaultOnlyList->GetFaultElementByErrorCode(errorCode);			
	}
	
	if (iFnd == m_FaultOnlyList->end())
	{

		return 0xFFFFFFFF;
	}

	TRACEINTO << "UpdateActiveAlarmFaultOnlyDescription: " << errorCode;

	CLogFltElement* fltElement = (*iFnd);
	fltElement->UpdateDescription(description.c_str());

	CHlogApi::TaskFault(fltElement->GetDescription(), alwaysSend);


	return (DWORD)fltElement;
}


/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::AddStartupCondition(const CActiveAlarm &aa)
{
	CStartupCondition cond(aa, eStartupConditionWait);
	CStartupCondNode *pNode = new CStartupCondNode(cond);
	m_StartupCondDependTree->AddRoot(pNode);
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::AddStartupConditionFaultOnly(const CActiveAlarm &aa)
{
	CStartupCondition cond(aa, eStartupConditionWait, true);
	CStartupCondNode *pNode = new CStartupCondNode(cond);
	m_StartupCondDependTree->AddRoot(pNode);
}

/////////////////////////////////////////////////////////////////////////////
bool CAlarmableTask::AddStartupCondDependency(WORD fatherErrorCode, WORD childErrorCode)
{
	CStartupCondNode *nodeChild  = m_StartupCondDependTree->FindByErrorCode(childErrorCode);
	CStartupCondNode *nodeFather = m_StartupCondDependTree->FindByErrorCode(fatherErrorCode);
	if(NULL == nodeFather || NULL == nodeChild)
	{
		WORD badErrorCode = (NULL == nodeFather ? fatherErrorCode : childErrorCode);
		CMedString message = "Bad error code : ";
		message << badErrorCode << " : "
				<< GetAlarmName(badErrorCode);

		PASSERTMSG(TRUE, message.GetString());
		return false;
	}

	CStartupCondNode *nodeChildRemoved  = m_StartupCondDependTree->RemoveNodeByErrorCode(childErrorCode);
	if(nodeChildRemoved != nodeChild)
	{
		PASSERTMSG(childErrorCode, "Bad Flow");
		return false;
	}

	nodeFather->AddChild(nodeChild);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CAlarmableTask::UpdateStartupConditionByErrorCode(WORD errorCode, eStartupConditionStatus status)
{
	CStartupCondNode *node  = m_StartupCondDependTree->FindByErrorCode(errorCode);
	if(NULL != node)
	{
		node->SetStartupCondStatus(status);
		eTaskState state = GetTaskState();
		if(eTaskStateStartup == state)
		{
			TryToExitStartup();
		}
	}
	return (NULL != node);
}

/////////////////////////////////////////////////////////////////////////////
eStartupConditionStatus CAlarmableTask::GetStartupConditionStatusByErrorCode(WORD errorCode)
{
	eStartupConditionStatus status = eStartupConditionOk;
	CStartupCondNode *node = m_StartupCondDependTree->FindByErrorCode(errorCode);
	if(NULL != node)
	{
		status = node->GetStartupCondStatus();
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::ConvertStartupConditionToActiveAlarm()
{
	if (false == m_IsStartupConditionTreeEnabled)
		return;

	CStartupCondNode *root = m_StartupCondDependTree->GetMainRoot();
	CChildVector::iterator iTer = root->GetFirstChild();
	CChildVector::iterator iEnd = root->GetEndChild();
	while(iTer != iEnd)
	{
		CStartupCondNode *currentSubTree = *iTer;
		ConvertStartupConditionToActiveAlarmRecursive(currentSubTree);

		iTer++;
	}
	ComputeSetTaskStatus();
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::ConvertStartupConditionToActiveAlarmRecursive(CStartupCondNode *currentNode)
{
	eStartupConditionStatus currentStatus = currentNode->GetStartupCondStatus();

	if(eStartupConditionFail == currentStatus || eStartupConditionWait == currentStatus)
	{
		const CActiveAlarm &currentAA = currentNode->GetActiveAlarm();
		bool result = IsActiveAlarmExistByErrorCode(currentAA.m_ErrorCode);
		if(false == result)
		{
			if (!currentNode->IsStartupCondFaultOnly())
			{
				AddActiveAlarm(currentAA);
				return;
			}
			else
			{
				AddActiveAlarmFaultOnly(currentAA);
			}
		}
	}

	CChildVector::iterator iTer = currentNode->GetFirstChild();
	CChildVector::iterator iEnd = currentNode->GetEndChild();
	while(iTer != iEnd)
	{
		CStartupCondNode *childNode = *iTer;
		ConvertStartupConditionToActiveAlarmRecursive(childNode);

		iTer++;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::TryToExitStartup()
{
	bool isWait = m_StartupCondDependTree->IsThereStatus(eStartupConditionWait, true);
	if(false == isWait)
	{
		SetTaskState(eTaskStateReady);
		ConvertStartupConditionToActiveAlarm();
	}
}
/*----------------------------------------------------------------------------------
	Implementation of Not Startup interface(active alarms)
----------------------------------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlarmNoFlushSingleton(BYTE subject,
                                                     DWORD errorCode,
                                                     BYTE errorLevel,
                                                     const string &description,
                                                     bool isForEma,
                                                     bool isForFaults,
                                                     DWORD userId,
                                                     DWORD boardId,
                                                     DWORD unitId,
                                                     WORD  theType)
{
    bool isExist = IsActiveAlarmExistByErrorCode(errorCode);
	if(true == isExist)
	{
		return 0xFFFFFFFF;
	}

	DWORD id = AddActiveAlarmNoFlush(subject, errorCode, errorLevel,
                   description, isForEma, isForFaults,
                   userId, boardId, unitId, theType);
    return id;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlarmSingleton(
							BYTE subject,
							DWORD errorCode,
							BYTE errorLevel,
							const string &description,
							bool isForEma,
							bool isForFaults,
							DWORD userId,
							DWORD boardId,
							DWORD unitId,
							WORD  theType)
{
	bool isExist = IsActiveAlarmExistByErrorCode(errorCode);
	if(true == isExist)
	{
		return 0xFFFFFFFF;
	}

	DWORD id = AddActiveAlarm(subject, errorCode, errorLevel,
                   description, isForEma, isForFaults,
                   userId, boardId, unitId, theType);
    return id;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlarmNoFlush(BYTE subject,
                                            DWORD errorCode,
                                            BYTE errorLevel,
                                            const string &description,
                                            bool isForEma,
                                            bool isForFaults,
                                            DWORD userId,
                                            DWORD boardId,
                                            DWORD unitId,
                                            WORD  theType)
{
    if(false == CAlarmParamValidator::ValidateAlarmParams(subject,
                                                          errorCode))
    {
        return 0xFFFFFFFF;
    }

    if(true == isForFaults)
	{
		BOOL isFullOnly = FALSE;
		CHlogApi::TaskFault(subject,
                            errorCode,
                            errorLevel,
                            description.c_str(),
                            isFullOnly,
                            boardId,
                            unitId,
                            theType);
	}

	DWORD faultId = AddActiveAlertToLocalDB(subject,
                                            errorCode,
                                            errorLevel,
                                            description,
                                            isForEma,
                                            userId,
                                            boardId,
                                            unitId,
                                            theType);

	return faultId;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlarm(const CActiveAlarm &aa, bool doesFlush)
{
    DWORD id = 0;
    if(doesFlush)
    {
        id = AddActiveAlarm(aa.m_Subject,
                            aa.m_ErrorCode,
                            aa.m_ErrorLevel,
                            aa.m_Description,
                            aa.m_IsForEma,
                            aa.m_IsForFaults
                            );
    }
    else
    {
        id = AddActiveAlarmNoFlush(aa.m_Subject,
                                   aa.m_ErrorCode,
                                   aa.m_ErrorLevel,
                                   aa.m_Description,
                                   aa.m_IsForEma,
                                   aa.m_IsForFaults);
    }
	return id;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlarm( BYTE subject,
                                      DWORD errorCode,
                                      BYTE errorLevel,
                                      const string &description,
                                      bool isForEma,
                                      bool isForFaults,
                                      DWORD userId,
                                      DWORD boardId,
                                      DWORD unitId,
                                      WORD theType)
{
    if(false == CAlarmParamValidator::ValidateAlarmParams(subject,
                                                          errorCode))
    {
        return 0xFFFFFFFF;
    }

    if(true == isForFaults)
	{
		BOOL isFullOnly = FALSE;
		CHlogApi::TaskFault(subject,
                            errorCode,
                            errorLevel,
                            description.c_str(),
                            isFullOnly,
                            boardId,
                            unitId,
                            theType);
	}

	DWORD faultId = AddActiveAlertToLocalDB(subject,
                                            errorCode,
                                            errorLevel,
                                            description,
                                            isForEma,
                                            userId,
                                            boardId,
                                            unitId,
                                            theType);
    FlushActiveAlarm();

	return faultId;
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::FlushActiveAlarm()
{
    ComputeSetTaskStatus();
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlertToLocalDB(BYTE subject,
                                             DWORD errorCode,
                                             BYTE errorLevel,
                                             const string &description,
                                             bool isForEma,
                                             DWORD userId,
                                             DWORD boardId,
                                             DWORD unitId,
                                             WORD  theType)
{
    if(false == m_IsCanAddAA)
	{
		return 0xFFFFFFFF;
	}

	CLogFltElement* fltElement = PrepareLogFltElement(m_AlarmUniqueIndex->GetInc(),
									subject, errorCode, errorLevel, description, isForEma, userId,
									boardId, unitId, theType);

	if(fltElement == NULL)
	{
		return 0xFFFFFFFF;
	}


	m_AlarmFaultList->AddFault(fltElement, false);

	const char *strErrorCode = GetAlarmName(errorCode);
	TRACEINTO << "\nActive Alarm : " << strErrorCode << " : " << description.c_str();

    return (DWORD)fltElement;

}

/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlarmFaultOnlySingleton(BYTE subject,
                         DWORD errorCode,
                         BYTE errorLevel,
                         const string &description,
                         DWORD userId,
                         DWORD boardId,
                         DWORD unitId 	,
                         WORD  theType 	)
{
    if(false == m_IsCanAddAAFaultOnly)
	{
		return 0xFFFFFFFF;
	}
	CFaultElementList::iterator iFnd ;
	if (userId != 0xFFFFFFFF)
	{
		iFnd =m_FaultOnlyList->GetFaultElementByErrorCodeUserId(errorCode, userId);
	}
	else
	{
		iFnd =m_FaultOnlyList->GetFaultElementByErrorCode(errorCode);
	}

	if (iFnd != m_FaultOnlyList->end())
	{
		return 0xFFFFFFFF;

	}
	return AddActiveAlarmFaultOnly(subject, errorCode, errorLevel, description, userId, boardId, unitId, theType);
}
////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlarmFaultOnly(BYTE subject,
                     DWORD errorCode,
                     BYTE errorLevel,
                     const string &description,
                     DWORD userId ,
                     DWORD boardId ,
                     DWORD unitId ,
                     WORD  theType )
{

    if(false == m_IsCanAddAAFaultOnly)
	{
		return 0xFFFFFFFF;
	}

	DWORD elem = UpdateActiveAlarmFaultOnlyDescriptionIfExist(description, errorCode, userId);

	if (elem  != 0xFFFFFFFF) // alarm was exist and updated.
	{
		return elem;
	}

	// Adding alarm fault
    if(false == CAlarmParamValidator::ValidateAlarmParams(subject,
                                                          errorCode))
    {
        return 0xFFFFFFFF;
    }

    bool isForEma = true; // irellevant

    DWORD elemIndex = m_AlarmUniqueIndex->GetInc();

	CLogFltElement* fltElement = PrepareLogFltElement(elemIndex,
									subject, errorCode, errorLevel, description, isForEma, userId,
									boardId, unitId, theType);

	if(fltElement == NULL)
	{
		TRACEINTO << "\nFailed prepare alarm fault : "  << errorCode;
		return 0xFFFFFFFF;
	}

	bool wasSent = CHlogApi::TaskFault(fltElement->GetDescription(), true);

	if (wasSent)
	{
		DWORD elems = m_FaultOnlyList->RemoveFaultsIfThereAreMoreThanGivenMax(MAX_FAULTLIST_LEN, RECOMMENDED_FAULTLIST_LEN);
		if (elems > 0)
		{
			TRACEINTO << "\n AddActiveAlarmFaultOnly Deleted  " << elems ;
		}


		m_FaultOnlyList->AddFault(fltElement, false);

		const char *strErrorCode = GetAlarmName(errorCode);
		TRACEINTO << "\nActive Alarm Fault Only : " << strErrorCode << " : " << description.c_str();
		return (DWORD)fltElement;

	}
	else
	{
		TRACEINTO << "\nFault alarm wasn't sent errrrCode: " << errorCode << " description: " << description;
		PDELETE(fltElement);
		return 0xFFFFFFFF;
	}
}

/////////////////////////////////////////////////////////////////////////////
DWORD CAlarmableTask::AddActiveAlarmFaultOnly(const CActiveAlarm &aa)
{

	DWORD id = 0;
	id = AddActiveAlarmFaultOnly(aa.m_Subject,
                            aa.m_ErrorCode,
                            aa.m_ErrorLevel,
                            aa.m_Description,
                            aa.m_IsForEma,
                            aa.m_IsForFaults
                            );
	return id;
}



/////////////////////////////////////////////////////////////////////////////
CLogFltElement* CAlarmableTask::PrepareLogFltElement(DWORD uniquendex ,
								BYTE subject,
								DWORD errorCode,
								BYTE errorLevel,
								const string &description,
								bool isForEma,
								DWORD userId,
								DWORD boardId,
								DWORD unitId,
								WORD  theType)
{

    char *pszMsg = FactoryFaultDesc::GetAllocatedFaultDesciption(subject,
                                                                 errorCode,
                                                                 errorLevel,
                                                                 description,
                                                                 boardId,
                                                                 unitId,
                                                                 theType);

    CLogFltElement *fltElement = new CLogFltElement(subject, pszMsg);
    PDELETEA(pszMsg);

	CStructTm t;
	SystemGetTime(t);
	fltElement->SetTime(t);
	fltElement->SetMask(FAULTS_MASK);
	fltElement->SetCode(errorCode);
	fltElement->SetType(errorLevel);
	fltElement->SetIsExternal(isForEma);
	fltElement->SetUserId(userId);
	fltElement->SetIndex(uniquendex);
    fltElement->SetFaultId(uniquendex);

	return fltElement;

}


/////////////////////////////////////////////////////////////////////////////
bool CAlarmableTask::IsActiveAlarmExistById(DWORD faultId)
{
	CLogFltElement *fltElement = (CLogFltElement*)faultId;
	bool res = m_AlarmFaultList->IsFaultExist(fltElement);

	return res;
}

/////////////////////////////////////////////////////////////////////////////
bool CAlarmableTask::IsActiveAlarmExistByErrorCode(WORD errorCode)
{
	bool res = m_AlarmFaultList->IsFaultExist(errorCode);
	return res;
}

/////////////////////////Is ActiveAlarm Exist By Error Code UserId////////////////////////////////////////////////////
bool CAlarmableTask::IsActiveAlarmExistByErrorCodeUserId(WORD errorCode, DWORD userId)
{
	bool res = m_AlarmFaultList->IsFaultExist(errorCode, userId);
	return res;
}

////For fault only: ////////////////Is ActiveAlarm Fault Exist By ErrorCode/////////////////////////////////////////////////////////
bool CAlarmableTask::IsActiveAlarmFaultOnlyExistByErrorCode(WORD errorCode)
{
	bool res = m_FaultOnlyList->IsFaultExist(errorCode);
	return res;
}

///For fault only:////////////Is Active Alarm Fault Exist By ErrorCode UserId//////////////////////////////////////
bool CAlarmableTask::IsActiveAlarmFaultOnlyExistByErrorCodeUserId(WORD errorCode, DWORD userId)
{
	bool res = m_FaultOnlyList->IsFaultExist(errorCode, userId);
	return res;
}

/////////////////////////////////////////////////////////////////////////////
bool CAlarmableTask::RemoveActiveAlarmByErrorCode(WORD errorCode)
{
	// can be 3 states
	//	1) Startup		 - startup conditions are not aa
	//	2) After Startup - ready
	//	3) Configuration - timer works

	eStartupConditionStatus scStatusBefore = GetStartupConditionStatusByErrorCode(errorCode);

	bool isAAisaSC = UpdateStartupConditionByErrorCode(errorCode, eStartupConditionOk);


	std::list<CFaultDesc *>* pFaultList = new std::list<CFaultDesc *>();

	bool isFound = m_AlarmFaultList->RemoveFaultByErrorCode(errorCode, pFaultList);
	const char *strErrorCode = GetAlarmName(errorCode);
	if(true == isFound)
	{
		ComputeSetTaskStatus();

		AddRemoveAAIndicationByFault(*pFaultList);

		TRACEINTO << "\nRemove Active Alarm : " << strErrorCode;
	}
	else
		TRACEINTO << "Active Alarm : " << strErrorCode << " was not found.";


	PDELETE(pFaultList);


	if(false == isAAisaSC)
	{
		return isFound;
	}
	if(eStartupConditionOk == scStatusBefore)
	{
		return isFound;
	}

	if(STARTUP_CONDITION_CONFIGURATION == errorCode)
	{
		return isFound;
	}

	const eTaskState state = GetTaskState();
	if(eTaskStateStartup == state || eTaskStateConfiguration == state || eTaskStateIdle == state)
	{
		return isFound;
	}

	bool isFailStatus = m_StartupCondDependTree->IsThereStatus(eStartupConditionFail, true);
	bool isWaitStatus = m_StartupCondDependTree->IsThereStatus(eStartupConditionWait, true);

	if(true == isFailStatus || true == isWaitStatus)
	{
		StartStartupConditionConfiguration(errorCode);
	}
	return isFound;
}
/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::RemoveActiveAlarmByErrorCodeUserId(WORD errorCode, DWORD userId)
{
	std::list<CFaultDesc *>* pFaultList = new std::list<CFaultDesc *>();

	bool isFound = m_AlarmFaultList->RemoveFaultByErrorCodeUserId(errorCode, userId, pFaultList);
	if(true == isFound)
	{
		ComputeSetTaskStatus();

		AddRemoveAAIndicationByFault(*pFaultList);
	}
	PDELETE(pFaultList);
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::RemoveActiveAlarmByUserId(DWORD userId)
{
	std::list<CFaultDesc *>* pFaultList = new std::list<CFaultDesc *>();

	bool isFound = m_AlarmFaultList->RemoveFaultByUserId(userId, pFaultList);
	if(true == isFound)
	{
		ComputeSetTaskStatus();
		AddRemoveAAIndicationByFault(*pFaultList);
	}
	PDELETE(pFaultList);
}

////////Fault Only: /////////////////////////////////////////////////////////////////////
void CAlarmableTask::RemoveActiveAlarmFaultOnlyByErrorCode(WORD errorCode)
{
	bool isAAisaSC = UpdateStartupConditionByErrorCode(errorCode, eStartupConditionOk);

	CFaultElementList::iterator iFnd =m_FaultOnlyList->GetFaultElementByErrorCode(errorCode);

	if (iFnd != m_FaultOnlyList->end())
	{
		RemoveActiveAlarmFaultOnly(*iFnd);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::RemoveActiveAlarmFaultOnlyByErrorCodeUserId(WORD errorCode, DWORD userId)
{
	UpdateStartupConditionByErrorCode(errorCode, eStartupConditionOk);

	CFaultElementList::iterator it = m_FaultOnlyList->GetFaultElementByErrorCodeUserId(errorCode, userId);
	if (it != m_FaultOnlyList->end())
	{
		RemoveActiveAlarmFaultOnly(*it);
	}
}


//////////////Fault Only///////////////////////////////////////////////////////////////
void CAlarmableTask::RemoveActiveAlarmFaultOnlyByUserId(DWORD userId)
{
	CFaultElementList::iterator it = m_FaultOnlyList->GetFaultElementByUserId(userId);

	if (it != m_FaultOnlyList->end())
	{
		RemoveActiveAlarmFaultOnly(*it);
	}
}

/////////////////////////////////////////////////////////////////////////////

// Adding faults to indicate that active alarm were removed.CFaultDesc will be deleted after adding
void CAlarmableTask::AddRemoveAAIndicationByFault(std::list<CFaultDesc *>& faultList) const
{

	std::list<CFaultDesc *>::iterator it = faultList.begin();
	CFaultDesc *pFaultDesc;
	for (; it != faultList.end(); ++it)
	{
		pFaultDesc = *it;
		if (!pFaultDesc)
		{
	      	TRACEINTOFUNC << "NULL fault description\n";
			continue;
		}
	    pFaultDesc->SetFaultLevel(SYSTEM_MESSAGE);

	    stringstream positiveDescription;
	    positiveDescription << CLEANING_ALARM_PREFIX <<  pFaultDesc->GetDescription() ;
	    pFaultDesc->SetDescription(positiveDescription.str().c_str());

	    TRACEINTO << "Sending cleaning alarm : " <<  positiveDescription;

		CHlogApi::TaskFault(pFaultDesc, true);

	    PDELETE(pFaultDesc);
	    *it = NULL;
	}

}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::RemoveActiveAlarmFaultOnly(CLogFltElement* fltElement)
{
	PASSERT_AND_RETURN(NULL == fltElement);

    CFaultDesc *pFaultDesc = FactoryFaultDesc::GetAllocatedFaultDescByDescription(fltElement->GetDescription());
    if(pFaultDesc == NULL)
    {
      	TRACEINTOFUNC << "pFaultDesc is NULL";
    	return;
    }

    pFaultDesc->SetFaultLevel(SYSTEM_MESSAGE);

    stringstream positiveDescription;
    positiveDescription << CLEANING_FAULT_PREFIX <<  pFaultDesc->GetDescription() ;
    pFaultDesc->SetDescription(positiveDescription.str().c_str());


    TRACEINTO << "Sending positive fault: " <<  fltElement->GetCode() ;

	CHlogApi::TaskFault(pFaultDesc, true);

	m_FaultOnlyList->RemoveFault(fltElement);

    PDELETE(pFaultDesc);

}
/////////////////////////////////////////////////////////////////////////////

bool CAlarmableTask::IsActiveAlarmExistByUserId(DWORD userId)
{
	bool isFound = m_AlarmFaultList->IsFaultExistByUserId(userId);
	if(true == isFound)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CAlarmableTask::IsActiveAlarmFaultOnlyExistByUserId(DWORD userId)
{
	bool isFound = m_FaultOnlyList->IsFaultExistByUserId(userId);
	if(true == isFound)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::RemoveActiveAlarmById(DWORD faultId)
{
/*	Yuri R temp
	bool isFound = UpdateStartupConditionById(faultId, eStartupConditionOk);
	if(true == isFound)
	{
		return;
	}
*/
	CLogFltElement *fltElement = (CLogFltElement*)faultId;

	std::list<CFaultDesc *>* pFaultList = new std::list<CFaultDesc *>();

	bool isFound = m_AlarmFaultList->RemoveFault(fltElement, pFaultList);
	if(true == isFound)
	{
		ComputeSetTaskStatus();
		AddRemoveAAIndicationByFault(*pFaultList);
	}
	PDELETE(pFaultList);
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::RemoveActiveAlarmFaultOnlyById(DWORD faultId)
{
	CLogFltElement *fltElement = (CLogFltElement*)faultId;

	CFaultElementList::iterator  iFnd = m_FaultOnlyList->GetFaultElement((CLogFltElement*)faultId);
	if (iFnd != m_FaultOnlyList->end())
	{
		RemoveActiveAlarmFaultOnly(*iFnd);
	}

}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::SetTaskStatus(eTaskStatus taskStatus)
{
	m_TaskStatus = taskStatus;
	if(eTaskNormal == m_TaskStatus)
	{
		m_AlarmFaultList->Clear();
	}

	SendStateChangeIndToManagerTask();
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::SetTaskState(eTaskState taskState)
{
	m_TaskState = taskState;
	SendStateChangeIndToManagerTask();
}

/////////////////////////////////////////////////////////////////////////////
eTaskStatus CAlarmableTask::GetTaskStatus()const
{
	return m_TaskStatus;
}

/////////////////////////////////////////////////////////////////////////////
eTaskState CAlarmableTask::GetTaskState()const
{
	return m_TaskState;
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::ComputeSetTaskStatus()
{
	m_TaskStatus = ComputeTaskStatus();
	SetTaskStatus(m_TaskStatus);
}

void CAlarmableTask::SendStateChangeIndToManagerTask() const
{
  CTaskStateFaultList stateFaultList(GetTaskName(),
                                     m_TaskStatus,
                                     m_TaskState,
                                     m_AlarmFaultList->Clone());

  CSegment *pSeg = new CSegment;
  stateFaultList.Serialize(*pSeg);

  CManagerApi api(CProcessBase::GetProcess()->GetProcessType());
  STATUS status = api.SendMsg(pSeg, TASK_CHANGE_STATE_FAULT_IND);

  if (STATUS_OK != status)
    TRACEINTOFUNC << "WARNING: failed to send change state indication";
}

eTaskStatus CAlarmableTask::ComputeTaskStatus()
{

	eTaskStatus resStatus = eTaskNormal;

	CLogFltElement *flt = m_AlarmFaultList->GetFirstFaultElement();
	while(NULL != flt)
	{
		WORD errorLevel = flt->GetType();
		eTaskStatus newStatus = (MAJOR_ERROR_LEVEL == errorLevel ? eTaskMajor : eTaskMinor);
		resStatus = (resStatus < newStatus ? newStatus : resStatus);
		if(eTaskMajor == resStatus)
		{
			break;
		}

		flt = m_AlarmFaultList->GetNextFaultElement();
	}
	return resStatus;
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::SendFlushtoLogger()
{
	CManagerApi api(eProcessLogger);
	STATUS status = api.SendMsg(NULL, FLUSH_TO_LOGGER);
	PASSERTMSG(STATUS_OK != status, "FAILED to send flush to logger");
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::StartStartupConditionConfiguration(WORD errorCode)
{

	SetTaskState(eTaskStateConfiguration);

	// 16.01.07: description changed
	AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
					STARTUP_CONDITION_CONFIGURATION,
					MAJOR_ERROR_LEVEL,
					"Configuring...",
					true,
					true
					);

	const char *strErrorCode = GetAlarmName(errorCode);
	string msg = "\nConfiguration on ";
	msg += strErrorCode;
	TRACESTR(eLevelInfoNormal) << msg.c_str();

	DWORD configurationTime = 2*60*SECOND; // 2 minutes as default
    switch (CProcessBase::GetProcess()->GetProductFamily())
    {
        case eProductFamilyCallGenerator:
            configurationTime = 30*SECOND;
            break;
        case eProductFamilyRMX:
        case eProductFamilySoftMcu:
            configurationTime = 2*60*SECOND;
            break;
        default:
            PASSERT(1);

    }

    CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    if( pSysConfig )
    	pSysConfig->GetDWORDDataByKey("MAX_STARTUP_TIME", configurationTime);

    StartTimer(STARTUP_CONDITION_CONFIGURATION_TIMER, configurationTime);
}

/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::HandleStartupConditionTimerEnd(CSegment * seg)
{
	SetTaskState(eTaskStateReady);
	RemoveActiveAlarmByErrorCode(STARTUP_CONDITION_CONFIGURATION);
	ConvertStartupConditionToActiveAlarm();
}


/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::HandleRemoveAllAA(CSegment * seg)
{
	m_AlarmFaultList->Clear();
	ComputeSetTaskStatus();
}


/////////////////////////////////////////////////////////////////////////////
void CAlarmableTask::HandleSetEnableDisableAA(CSegment * pSeg)
{
	BOOL isEnable = TRUE;
	*pSeg >> isEnable;
	m_IsCanAddAA = (TRUE == isEnable ? true : false);
}

/////////////////////////////////////////////////////////////////////////////

