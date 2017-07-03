#ifndef ALARMABLETASK_H_
#define ALARMABLETASK_H_

#include "TaskApp.h"
#include "ActiveAlarmDefines.h"


class CStartupConditionContainer;
class CStartupCondDependForest;
class CStartupCondNode;
class CFaultList;
class CUniqueIndex;
class CLogFltElement;
class CFaultDesc;


// the order is importent for strings in names and calculating of state of process
enum eTaskStatus
{
	eTaskInvalid = -1,
	eTaskNormal = 0,
	eTaskMinor,
	eTaskMajor,
//	eTaskStartUp,
//	eTaskIdle,

	NumOfTaskStatus
};

static const char * GetTaskStatusName(eTaskStatus status)
{
    static const char* TaskStatusNames[] =
        {
            "Normal"	,  	// eTaskNormal
            "Minor"		,   // eTaskMinor
            "Major"		,   // eTaskMajor
//	"StartUp"	, 	// eTaskStartUp
//	"Idle"			// eTaskIdle
        };

	const char *name = (eTaskInvalid < status && status < NumOfTaskStatus
						?
						TaskStatusNames[status] : "Invalide");
	return name;
}


enum eTaskState
{
	eTaskStateIdle,
	eTaskStateStartup,
	eTaskStateReady,
	eTaskStateConfiguration,

	NumOfTaskStates
};

static const char * GetTaskStateName(eTaskState state)
{
    static const char* TaskStatesNames[] =
        {
            "Idle"			,  	// eTaskStateIdle
            "Startup"		,   // eTaskStateStartup
            "Ready"			,   // eTaskStateReady
            "Configuration"	    // eTaskStateConfiguration
        };

	const char *name = (eTaskStateIdle <= state && state < NumOfTaskStates
						?
						TaskStatesNames[state] : "Invalide");
	return name;
}

/*
	every derived class must override a function :virtual const char * CTAskApp::GetTaskName() const = 0;
	this function must return an UNIQUE name, even if the class is not singleton.
*/

/*
 * Fault Only support:
 * 2/2012 - Adding alarm faults support. This was added as part of the reduce active alarm feature.
 * The goal was giving infrastructure to relate some alarms as faults but still be able to send positive fault message when removing them
 * and be able to update their description ( similarly to the  real alarms).
 *
*/

class CAlarmableTask : public CTaskApp
{
CLASS_TYPE_1(CAlarmableTask, CTaskApp)
public:
	CAlarmableTask();
	virtual ~CAlarmableTask();

	virtual const char* NameOf() const { return "CAlarmableTask";}
	virtual void  Create(CSegment& appParam, WORD limited=FALSE); // called from entry point

	void SetTaskStatus(eTaskStatus taskStatus);
	eTaskStatus GetTaskStatus()const;

	void SetTaskState(eTaskState taskState);
	eTaskState GetTaskState()const;

    // interface of active alarms

    // flush means calculate process state and sent it to McuMngr
    // (alarm list is a part of state), from this point the alarms
    // will be seen in EMA. use this "NoFlush" when you have to add
    // a sequence of alarms, but after you finish with the sequence call
    // FlushActiveAlarm function.
	DWORD AddActiveAlarmNoFlush(BYTE subject,
                                DWORD errorCode,
                                BYTE errorLevel,
                                const string &description,
                                bool isForEma,
                                bool isForFaults,
                                DWORD userId 	= 0xFFFFFFFF,
                                DWORD boardId 	= 0,
                                DWORD unitId 	= 0,
                                WORD  theType 	= 0);

	DWORD AddActiveAlarmNoFlushSingleton(BYTE subject,
                                         DWORD errorCode,
                                         BYTE errorLevel,
                                         const string &description,
                                         bool isForEma,
                                         bool isForFaults,
                                         DWORD userId 	= 0xFFFFFFFF,
                                         DWORD boardId 	= 0,
                                         DWORD unitId 	= 0,
                                         WORD  theType 	= 0);

	// interface of active alarms
	DWORD AddActiveAlarm(BYTE subject,
                         DWORD errorCode,
                         BYTE errorLevel,
                         const string &description,
                         bool isForEma,
                         bool isForFaults,
                         DWORD userId 	= 0xFFFFFFFF,
                         DWORD boardId 	= 0,
                         DWORD unitId 	= 0,
                         WORD  theType 	= 0);


	DWORD AddActiveAlarmSingleton(BYTE subject,
                                  DWORD errorCode,
                                  BYTE errorLevel,
                                  const string &description,
                                  bool isForEma,
                                  bool isForFaults,
                                  DWORD userId 	= 0xFFFFFFFF,
                                  DWORD boardId 	= 0,
                                  DWORD unitId 	= 0,
                                  WORD  theType 	= 0);


    void FlushActiveAlarm();


    // Refer to "Fault Only support" note above
	// For Alarm Fault Only.
	// return the index of the Element in the list
	DWORD AddActiveAlarmFaultOnlySingleton(BYTE subject,
                         DWORD errorCode,
                         BYTE errorLevel,
                         const string &description,
                         DWORD userId 	= 0xFFFFFFFF,
                         DWORD boardId 	= 0,
                         DWORD unitId 	= 0,
                         WORD  theType 	= 0);

	// Refer to "Fault Only support" note above
	// For Alarm Fault Only
	// return the index of the Element in the list
	DWORD AddActiveAlarmFaultOnly(BYTE subject,
                         DWORD errorCode,
                         BYTE errorLevel,
                         const string &description,
                         DWORD userId 	= 0xFFFFFFFF,
                         DWORD boardId 	= 0,
                         DWORD unitId 	= 0,
                         WORD  theType 	= 0);



    // For real alarms
	bool RemoveActiveAlarmByErrorCode(WORD errorCode);
	void RemoveActiveAlarmByErrorCodeUserId(WORD errorCode, DWORD userId);

	// Refer to "Fault Only support" note above
	// For alarm faults only
	void RemoveActiveAlarmFaultOnlyByErrorCode(WORD errorCode);
	void RemoveActiveAlarmFaultOnlyByErrorCodeUserId(WORD errorCode, DWORD userId);


	// For real alarm
    bool IsActiveAlarmExistByErrorCode(WORD errorCode);
	bool IsActiveAlarmExistByErrorCodeUserId(WORD errorCode, DWORD userId);

	// Refer to "Fault Only support" note above
	// For alarm fault only
	bool IsActiveAlarmFaultOnlyExistByErrorCode(WORD errorCode);
	bool IsActiveAlarmFaultOnlyExistByErrorCodeUserId(WORD errorCode, DWORD userId);

protected:
	virtual void AlarmableTaskCreateEndPoint();

	void ComputeSetTaskStatus();

	// For real alarm
	void AddStartupCondition(const CActiveAlarm &aa);

	// For alarm fault only
	void AddStartupConditionFaultOnly(const CActiveAlarm &aa);

	eStartupConditionStatus GetStartupConditionStatusByErrorCode(WORD errorCode);
	bool UpdateStartupConditionByErrorCode(WORD errorCode, eStartupConditionStatus status);

	// Refer to "Fault Only support" note above
	void UpdateActiveAlarmFaultOnlyDescriptionByErrorCode(DWORD errorCode, const string & description);

	void ConvertStartupConditionToActiveAlarm();
	bool IsThereStartupCondition();

	bool AddStartupCondDependency(WORD fatherErrorCode, WORD childErrorCode);

	DWORD AddActiveAlarm(const CActiveAlarm &aa, bool doesFlush = true);

	DWORD AddActiveAlarmFaultOnly(const CActiveAlarm &aa);


	void RemoveActiveAlarmByUserId(DWORD userId);

	// Refer to "Fault Only support" note above
	void RemoveActiveAlarmFaultOnlyByUserId(DWORD userId);  // TODO TOCHECK


	// faultId is pointer to the fltElement (not the id of the fault)
	void RemoveActiveAlarmById(DWORD faultId);

	// Refer to "Fault Only support" note above
	void RemoveActiveAlarmFaultOnlyById(DWORD faultId);

// TODO TOCHEC
	// bool IsFaultExistByUserId(DWORD userId);
	bool IsActiveAlarmExistByUserId(DWORD userId);

	// Refer to "Fault Only support" note above
	bool IsActiveAlarmFaultOnlyExistByUserId(DWORD userId);


	// common SC + AA
	void UpdateActiveAlarmDescriptionByErrorCode(DWORD errorCode, const string& description);



	bool IsActiveAlarmExistById(DWORD faultId);

	bool GetIsStartupConditionTreeEnabled()const{return m_IsStartupConditionTreeEnabled;}
	void SetIsStartupConditionTreeEnabled(bool val){m_IsStartupConditionTreeEnabled = val;}

	void SendFlushtoLogger();

	PDECLAR_MESSAGE_MAP

private:
	// disabled
	CAlarmableTask(const CAlarmableTask&);
	CAlarmableTask& operator = (const CAlarmableTask&);

	void StartStartupConditionConfiguration(WORD errorCode);
	void HandleStartupConditionTimerEnd(CSegment * seg);
	void HandleRemoveAllAA(CSegment * seg);
	void HandleSetEnableDisableAA(CSegment * pSeg);
	void RemoveActiveAlarmFaultOnly(CLogFltElement* fltElement);

	DWORD UpdateActiveAlarmFaultOnlyDescriptionIfExist(const string & description,
														DWORD errorCode, DWORD userId 	= 0xFFFFFFFF, BOOL alwaysSend = FALSE);

    DWORD AddActiveAlertToLocalDB(BYTE subject,
                                  DWORD errorCode,
                                  BYTE errorLevel,
                                  const string &description,
                                  bool isForEma,
                                  DWORD userId,
                                  DWORD boardId,
                                  DWORD unitId,
                                  WORD  theType);


    CLogFltElement* PrepareLogFltElement(DWORD uniquendex,
									BYTE subject,
									DWORD errorCode,
									BYTE errorLevel,
									const string &description,
									bool isForEma,
									DWORD userId,
									DWORD boardId,
									DWORD unitId,
									WORD  theType);

	void SendStateChangeIndToManagerTask()const;
	eTaskStatus ComputeTaskStatus();
	void TryToExitStartup();

	void ConvertStartupConditionToActiveAlarmRecursive(CStartupCondNode *currentNode);

	void AddRemoveAAIndicationByFault(std::list<CFaultDesc *>& faultList) const ;

	eTaskState		m_TaskState;
	eTaskStatus     m_TaskStatus;

	CFaultList 	 	*m_AlarmFaultList; // real alarms
	CFaultList 	 	*m_FaultOnlyList; // alarm faults (not real alarms)

	CStartupCondDependForest *m_StartupCondDependTree;

	bool m_IsStartupConditionTreeEnabled;
	bool m_IsCanAddAA;
	bool m_IsCanAddAAFaultOnly;

    CUniqueIndex *m_AlarmUniqueIndex;

    static const string CLEANING_FAULT_PREFIX ;
    static const string CLEANING_ALARM_PREFIX ;

    static const DWORD MAX_FAULTLIST_LEN ;
    static const DWORD RECOMMENDED_FAULTLIST_LEN ;

};

#endif /*ALARMABLETASK_H_*/
