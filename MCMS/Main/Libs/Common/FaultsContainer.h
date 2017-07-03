// CTaskStateContainer.h: interface for the CTaskStateContainer class.
//
//
//Date         Updated By         Description
//
//8/11/05	  Yuri Ratner		Data Structure for Task States
//========   ==============   =====================================================================


#ifndef CTASKSTATECONTAINER_H_
#define CTASKSTATECONTAINER_H_

#include <map>
#include <string>
#include <list>
#include "PObject.h"
#include "SerializeObject.h"
#include "TaskApp.h"
#include "ProcessBase.h"
#include "McmsProcesses.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "HlogElement.h"

using namespace std;

class CTaskApp;
class CLogFltElement;
class CStateFaultList;
class FaultElement_S;
class CFaultDesc;

typedef list<CLogFltElement*> CFaultElementList;

// string(key) : Task name or process name.
typedef map<string, CStateFaultList*> CStateFaultListMap_Type;






/*-------------------------------------------------------------------------------
class CFaultList
-------------------------------------------------------------------------------*/
class CFaultList : public CSerializeObject
{
CLASS_TYPE_1(CFaultList, CPObject)
public:
	CFaultList();
	virtual ~CFaultList();
	virtual CFaultList* Clone();
	virtual const char* NameOf() const { return "CFaultList";}
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	void SerializeXml(CXMLDOMElement*& pFatherNode, DWORD objToken, DWORD id) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);

	bool operator == (const CFaultList &rHnd);

	CLogFltElement* GetFirstFaultElement();
	CLogFltElement* GetNextFaultElement();

	void AddFault(CLogFltElement *flt, bool isAllocateId = true);
	void ConcatFaultList(CFaultElementList *rHndList, bool isExternal);

	bool RemoveFaultByErrorCode(WORD errorCode, std::list<CFaultDesc *>* pFaultList = NULL);
	bool RemoveFaultByErrorCodeUserId(WORD errorCode, DWORD userId, std::list<CFaultDesc *>* pFaultList = NULL);
	bool RemoveFault(CLogFltElement *fltElement, std::list<CFaultDesc *>* pFaultList = NULL);

	DWORD RemoveFaultsIfThereAreMoreThanGivenMax(DWORD maxAllowedElements, DWORD numRecommendedElems);

	bool IsFaultExistByUserId(DWORD userId);
	bool RemoveFaultByUserId(DWORD userId, std::list<CFaultDesc *>* pFaultList = NULL);

	void Clear();
	DWORD GetSize()const;
	bool IsFaultExist(WORD errorCode)const;
	bool IsFaultExist(WORD errorCode, DWORD userId)const;
	bool IsFaultExist(CLogFltElement *fltElement)const;
    bool IsFaultExist(WORD errorCode, const char *desc)const;
    bool IsFaultExistByUniqueIndex(DWORD uniqueIndex)const;

	CFaultElementList* GetFaultElementList();

	void Serialize(CSegment &seg);
	void DeSerialize(CSegment &seg);

	CFaultElementList::iterator begin() const {return m_FaultElementList->begin();}
	CFaultElementList::iterator end() const {return m_FaultElementList->end();}

	bool UpdateActiveAlarmByErrorCode(WORD errorCode, const string & description);

	void  SetFaultIdAccordingToLastElement();
	DWORD GetFaultId();

	CFaultElementList::iterator GetFaultElementByErrorCodeUserId(WORD errorCode, DWORD userId)const;
	CFaultElementList::iterator GetFaultElementByErrorCode(WORD errorCode)const;
	CFaultElementList::iterator GetFaultElementByUserId(DWORD userId)const;
	CFaultElementList::iterator GetFaultElement(CLogFltElement *fltElement)const;

//	void SetIsItCardStatusesList(BOOL isIt);
//	BOOL GetIsItCardStatusesList();


protected:
	CFaultElementList	*m_FaultElementList;
	DWORD				 m_faultId;


private:
	// disabled
	CFaultList& operator=(const CFaultList&);
	CFaultList(const CFaultList &rHnd);


	CFaultElementList::iterator GetFaultElementByErrorCodeDescription(WORD errorCode, const char *desc)const;
    CFaultElementList::iterator GetFaultElementByUniqueIndex(DWORD uniqueIndex)const;


	CFaultElementList::iterator m_currentIter;

//	BOOL m_isItCardStatusesList;
};




/*-------------------------------------------------------------------------------
class CStateFaultList
-------------------------------------------------------------------------------*/
class CStateFaultList : public CPObject
{
CLASS_TYPE_1(CStateFaultList, CPObject)
public:
	CStateFaultList();
	CStateFaultList(CFaultList *list);
	CStateFaultList(const CStateFaultList&);
	virtual const char* NameOf() const { return "CStateFaultList";}

	virtual ~CStateFaultList();

	bool operator == (const CStateFaultList &rHnd);

	CFaultList* GetFaultList();
	void SetFaultList(CFaultList *faultList);

	void AddFault(CLogFltElement *flt);

	void ClearList();

	virtual void Serialize(CSegment &seg);
	virtual void DeSerialize(CSegment &seg);

	CLogFltElement* GetFirstFaultElement();
	CLogFltElement* GetNextFaultElement();


private:
	// disabled
	CStateFaultList& operator= (const CStateFaultList&);

	CFaultElementList::iterator m_CurrentIter;
	CFaultList 		 *m_pFaultList;
};







/*-------------------------------------------------------------------------------
class CTaskStateFaultList
-------------------------------------------------------------------------------*/
class CTaskStateFaultList : public CStateFaultList
{
CLASS_TYPE_1(CTaskStateFaultList, CStateFaultList)
public:
	CTaskStateFaultList();
	CTaskStateFaultList(const CTaskStateFaultList &rHnd);
	CTaskStateFaultList(const string &taskName);
	virtual const char* NameOf() const { return "CTaskStateFaultList";}
	CTaskStateFaultList(const string &taskName, eTaskStatus status, eTaskState state, CFaultList *list);
	virtual ~CTaskStateFaultList();

	bool operator == (const CTaskStateFaultList &rHnd);

	CTaskStateFaultList* Clone()const;

	eTaskState GetTaskState()const;
	void SetTaskState(eTaskState state);

	const string& GetTaskName()const;
	void SetTaskName(const string &taskName);

	eTaskStatus GetTaskStatus()const;
	void SetTaskStatus(eTaskStatus taskStatus);

	virtual void Serialize(CSegment &seg);
	virtual void DeSerialize(CSegment &seg);

	void DumpActiveAlarmList(std::ostream& answer);

private:
	eTaskState		m_TaskState;
	eTaskStatus     m_TaskStatus;
	string 			m_TaskName;
};












/*-------------------------------------------------------------------------------
class CProcessStateFaultList
-------------------------------------------------------------------------------*/
class CProcessStateFaultList : public CStateFaultList
{
CLASS_TYPE_1(CProcessStateFaultList, CStateFaultList)
public:
	CProcessStateFaultList();
	CProcessStateFaultList(eProcessType processType, eProcessStatus processState, CFaultList *faultList);
	virtual ~CProcessStateFaultList();
	virtual const char* NameOf() const { return "CProcessStateFaultList";}

	eProcessType GetProcessType();
	void SetProcessType(eProcessType processType);
	eProcessStatus GetProcessStatus();
	void SetProcessStatus(eProcessStatus processState);

	virtual void Serialize(CSegment &seg);
	virtual void DeSerialize(CSegment &seg);

private:
	eProcessType 	  m_ProcessType;
	eProcessStatus     m_ProcessState;
};




















/*-------------------------------------------------------------------------------
class CStateFaultListMap
-------------------------------------------------------------------------------*/
class CStateFaultListMap : public CPObject
{
CLASS_TYPE_1(CStateFaultListMap, CPObject)
public:
	CStateFaultListMap();
	virtual ~CStateFaultListMap();

	virtual const char* NameOf() const { return "CStateFaultListMap";}
	void AddNewEntry(string entryKey, CStateFaultList *stateFaultList);
	bool UpdateEntry(string entryKey, CStateFaultList *entryStateFaultList);

	CStateFaultList* GetFirstStateFaultList();
	CStateFaultList* GetNextStateFaultList();

	bool IsEntryExist(string entryKey);

    CStateFaultList* GetStateFaultList(string entryKey);

private:
	// disabled
	CStateFaultListMap(const CStateFaultListMap&);
	CStateFaultListMap& operator= (const CStateFaultListMap&);


	void ClearFaultList(string entryKey);
	void CLear();

	CStateFaultListMap_Type::iterator m_CurrentIter;
	CStateFaultListMap_Type *m_StateFaultListMap;
};









/*-------------------------------------------------------------------------------
class CTaskStateFaultListMap
-------------------------------------------------------------------------------*/
class CTaskStateFaultListMap : public CStateFaultListMap
{
CLASS_TYPE_1(CTaskStateFaultListMap, CStateFaultListMap)
public:
	CTaskStateFaultListMap();
	virtual ~CTaskStateFaultListMap();

	virtual const char* NameOf() const { return "CTaskStateFaultListMap";}
	void AddNewTask(string taskName);
	void UpdateTask(string taskName, CTaskStateFaultList *taskStateFaultList);
	eProcessStatus CalculateProcessState();

	CTaskStateFaultList* GetFirstTaskStateFaultList();
	CTaskStateFaultList* GetNextTaskStateFaultList();

	void GetCommonFaultList(CFaultList *faultList);

private:
    eProcessStatus TaskProcessStateTable(	eProcessStatus 	oldProcessState,
                                            eTaskState 		taskState,
                                            eTaskStatus 	taskStatus);
};










/*-------------------------------------------------------------------------------
class CProcessStateFaultListMap
-------------------------------------------------------------------------------*/
class CProcessStateFaultListMap : public CStateFaultListMap
{
CLASS_TYPE_1(CProcessStateFaultListMap, CStateFaultListMap)
public:
	CProcessStateFaultListMap();
	virtual ~CProcessStateFaultListMap();

	virtual const char* NameOf() const { return "CProcessStateFaultListMap";}
	eProcessStatus GetWorstProcessState(eProcessType &worstProcessType);

	CProcessStateFaultList* GetFirstProcessStateFaultList();
	CProcessStateFaultList* GetNextProcessStateFaultList();

private:
};


#endif /*CTASKSTATECONTAINER_H_*/
