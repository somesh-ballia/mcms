#ifndef CONNECTIONSTASK_H_
#define CONNECTIONSTASK_H_

#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#include "TaskApp.h"
#include "MediaMngr.h"
#include "EPConnection.h"
#include "AudRequestStructs.h"
#include "ManagerApi.h"

#ifndef __DISABLE_ICE__
#include "Mutex.hxx"
#endif	//__DISABLE_ICE__

#include <time.h>

class CTimerCompensation
{
public:
	CTimerCompensation();
	~CTimerCompensation();

	timespec m_lastTime; 	//CLOCK_REALTIME
	int m_accTime; 			// 1/10000  sec
	int m_lastTimerTime;	//last timer sleep time
	int m_compensationCounter;
	int m_printCounter;
	int m_betweenCompensationCounter;
};



// task creation function
extern "C" void ConnectionsTaskEntryPoint(void* appParam);

#ifndef __DISABLE_ICE__
class IceManager;
#endif	//__DISABLE_ICE__

class CConnectionsTask : public CTaskApp
{
CLASS_TYPE_1(CConnectionsTask, CTaskApp)
public:
	CConnectionsTask();
	virtual ~CConnectionsTask();
	
	// Initializations
	void  Create(CSegment& appParam);
	void* GetMessageMap();

	//overrides
	const char* NameOf() const {return "CConnectionsTask";} 
	const char* GetTaskName() const {return "ConnectionsTask";}
	BOOL  IsSingleton() const {return NO;}
	virtual void  InitTask();
	virtual void  SelfKill();
	void SendToGideonSimForMplApi(CMplMcmsProtocol& rMplProtocol) const;
	
	
protected:

	// Action functions
	void OnGideonSimCommandAll(CSegment* pParam);
	void OnEmaEngineCommandAll(CSegment* pParam);
	void OnGlobalTxTimer(CSegment* pParam);
	void OnResetChannelCommandAll(CSegment* pParam);
	void OnVideoUpdatePicCommandAll(CSegment* pParam);
	
	void OnGlobalSwitchTimer(CSegment* pParam);
	BOOL FindEPSendAudio(const list<int> &lstSpeaker,int index);
	
	void HandleEvent(CMplMcmsProtocol* pMplProtocol);
	void DumpEPArray();
	
	//Methods	
	void CreateEPConnection(int index, DWORD participantId);
	void RemoveEPConnection(DWORD participantId);
	
	
	
	BOOL HandleDTMF(CMplMcmsProtocol* pMplProtocol, CEPAccess* epAccess);
	void TraceMplMcms(const CMplMcmsProtocol* pMplProt) const;
	

protected:
	
	CEPAccess* m_pEPAccessArray[MAX_NUM_OF_ENDPOINTS];

	//The Max number of speaker,read the value from configurable XML file
	int m_nMaxNumberSpeaker;
	
	//The interval,read the value from configurable XML file
	int m_nSwitchAudioInterval;
	
	//Enable or disable the audio switch
	BOOL m_bSwitchAudio;

	struct ENDPOINTS
	{
		list<int> lstEndpoint; //Endpoint index list per conference
		list<int> lstSpeaker;  //Speaker Endpoint index list per conference
	};
	//Conference information, map<ConferenceID,Endpoint list>
	map<int,ENDPOINTS> m_mapConf;

	//temp ep array with monitoring ids (conf+party) and media params
	TMonitoringIDEP* m_pMonitoringIDEPArray[MAX_NUM_OF_ENDPOINTS * 2];
	
	void InvokeEPGlobalTimer();
	
private:
	
	const std::string& GetOpcodeStr(CMplMcmsProtocol* pMplProtocol) const;
	
	//end-points counter
	DWORD m_nEPCounter;
	
	//compensation object
	CTimerCompensation m_timerCompensation;
	
#ifndef __DISABLE_ICE__
	IceManager *m_IceProcessor;
	mutable resip::Mutex m_GideonSimMplApiMutex;//because AFEngine is multi-thread process, m_GideonSimMplApi need be protected
#endif	//__DISABLE_ICE__
	CManagerApi m_GideonSimMplApi;
	
	PDECLAR_MESSAGE_MAP	
};



#endif /*CONNECTIONSTASK_H_*/
