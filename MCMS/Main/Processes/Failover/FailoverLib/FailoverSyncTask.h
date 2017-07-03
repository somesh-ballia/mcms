// CFailoverSyncTask.h: interface for the CFailoverSyncTask class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FailoverSyncTask_H_
#define FailoverSyncTask_H_


#include "TaskApp.h"
#include "ManagerApi.h"
#include "FailoverDefines.h"

class CSyncedElement;

extern "C" void FailoverSyncEntryPoint(void* appParam);



class CFailoverSyncTask : public CTaskApp
{
CLASS_TYPE_1(CFailoverSyncTask, CTaskApp)

public:

	// Constructors
	CFailoverSyncTask();
	~CFailoverSyncTask();

	// Initializations
	void  InitTask();
	BOOL  IsSingleton() const {return NO;}

	// Operations
	virtual const char*  NameOf() const {return "CFailoverSyncTask";}
	virtual const char * GetTaskName() const {return "CFailoverSyncTask";}


protected:
	void InitSyncedElementsArray();
	void InitSyncTimersNamesArray();
	void InitSyncTimersTimeoutsArray();
	void InitIsSyncAlreadyDoneArray();

	void OnSocketRcvInd(CSegment* pMsg);

	void OnTimerSyncIpServiceTimeout(CSegment* pParam);
	void OnTimerSyncMngmntServiceTimeout(CSegment* pParam);
	void OnTimerSyncIvrServicesTimeout(CSegment* pParam);
	void OnTimerSyncConferenceProfilesTimeout(CSegment* pParam);
	void OnTimerSyncMeetingRoomsTimeout(CSegment* pParam);
	void OnTimerSyncOngoingConferencesTimeout(CSegment* pParam);
	void OnTimerSyncReservationsTimeout(CSegment* pParam);
	void OnTimerSyncRecordingLinksTimeout(CSegment* pParam);

	void SendSyncSpec(eSyncedElement theSyncElement);
	void TreatConfListResponse(const char* pXMLString);
	void TreatConfResponse(const char* pXMLString);	
	void TreatIpServiceListResponse(const char* pXMLString);	
	void TreatResListResponse(const char* pXMLString);
	void TreatResResponse(const char* pXMLString);
	void TreatRecordingLinksResponse(const char* pXMLString);
	void TreatIvrServicesResponse(const char* pXMLString);

	void TreatIpServiceChangeIfNeeded(const char* pXMLString, eSyncedElement curElement, char* specActionNodeName);
	bool IsChanged( CXMLDOMElement *pRootElem, eSyncedElement curElement,
					char* actionNodeName, char* specActionNodeName, char* elementName );
	
	void TreatWholeSyncCompletedOnce();
	bool IsWholeSyncCompletedOnce();

	
private:
	CSyncedElement *m_pSyncedElements[eNUM_OF_SYNCED_ELEMENTS];
	DWORD			m_syncTimersNames[eNUM_OF_SYNCED_ELEMENTS];
	DWORD			m_syncTimersTimeouts[eNUM_OF_SYNCED_ELEMENTS];
	bool			m_isSyncAlreadyDone[eNUM_OF_SYNCED_ELEMENTS];
	
	bool m_isWholeSyncCompletedOnce;

	PDECLAR_MESSAGE_MAP
};


#endif /* FailoverSyncTask_H_ */
