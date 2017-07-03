#include "CDRManager.h"

#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"

#include "CDRLog.h"
#include "CDRProcess.h"
#include "CDRShort.h"

#include "AllocateStructs.h"
#include "ManagerApi.h"
#include "WrappersResource.h"

#include "FaultsDefines.h"
#include "StatusesGeneral.h"
#include "CDRStatuses.h"
#include "CDRSettings.h"

#include "Segment.h"

#include "Request.h"
#include "DummyEntry.h"
#include "Versions.h"
#include "SysConfigEma.h"

#include "SysConfigKeys.h"
#include "SysConfig.h"

#include "CdrConvertToXml.h"
#include "CurlHTTP.h"
#include "TerminalCommand.h"
#include "OsFileIF.h"
#include "FaultsDefines.h"
#include "ActiveAlarmDefines.h"

#include "CdrPersistApiFactory.h"
#include "CdrRegistrationApiFactory.h"

#include "Trace.h"
#include "TraceStream.h"
#include <stdlib.h>


#include <fstream>

///////////////////////////////////////////////////////////////////////////
#define CDR_FILE_WARNING_TIMEOUT      1000           // (10 seconds)
#define CDR_FILE_WARNING_IDLE_TIMEOUT 15 * 60 * 100  // (15 minutes)
#define AWAKE_CDR_THREAD_TIMOUT       30             // (30 seconds)

#define NUMBER_OF_FAILURE_FOR_SENDING_CDR_EVENT 3
#define PERSISTENCE_QUEUE_CDR_NAME "cdr"

///////////////////////////////////////////////////////////////////////////
//pthread_timestruc_t to;
pthread_mutex_t queue_lock;
pthread_cond_t queue_nonempty;

static bool g_isAAServerConEnabled = false;
pthread_t g_LastCommunicationthreadCreated;
static DWORD m_countCdrErros = 0;

///////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CCDRManager)
	ONEVENT(XML_REQUEST, IDLE, CCDRManager::HandlePostRequest)
	ONEVENT(CDR_START_CONF, ANYCASE, CCDRManager::HandleCDRStartConf)
	ONEVENT(CDR_EVENT, ANYCASE, CCDRManager::HandleCDREvent)
	ONEVENT(CDR_END_CONF, ANYCASE, CCDRManager::HandleCDREndConf)
	ONEVENT(CDR_RSRC_GET_LAST_CONF_ID_REQ, ANYCASE, CCDRManager::HandleGetLastConfId)
	ONEVENT(CDR_FILE_SYSTEM_WARNING_TIMER, ANYCASE, CCDRManager::OnTimerFileSystemWarningTest)
	ONEVENT(CDR_RETRIVED_INTERNAL_NOTIFY, ANYCASE, CCDRManager::HandleHandleCDRRetrievedNotify)
	ONEVENT(CDR_CREATE_XML_FOLDER, ANYCASE, CCDRManager::HandleCDRCreateXmlFolder)
	ONEVENT(CDR_COLLECTOR_ABORT, ANYCASE, CCDRManager::HandleAbortCreateXmlFolder)
	ONEVENT(MCUMNGR_TO_CDR_PARAMS_IND, ANYCASE, CCDRManager::OnMcuMngrCdrParamsInd)

	ONEVENT(CDR_PERSISTENCE_QUEUE_DISABLE, ANYCASE, CCDRManager::HandlePersistenceDisable)
	ONEVENT(CDR_PERSISTENCE_QUEUE_ENABLE, ANYCASE, CCDRManager::HandlePersistenceEnable)
	ONEVENT(CDR_PERSISTENCE_QUEUE_ADD_CDR, ANYCASE, CCDRManager::HandlePersistenceAddCdr)
	ONEVENT(CDR_ENABLE_AA_SERVER_CONEECTION_DOWN, ANYCASE, CCDRManager::EnableActiveAlarmCdrServerConeectionDown)
	ONEVENT(CDR_DISABLE_AA_SERVER_CONEECTION_DOWN, ANYCASE, CCDRManager::DisableActiveAlarmCdrServerConeectionDown)
	ONEVENT(CDR_IS_REMOTE_CDR_ENABLE, ANYCASE, CCDRManager::IsRemoteCdrEnable)
PEND_MESSAGE_MAP(CCDRManager, CManagerTask);

///////////////////////////////////////////////////////////////////////////
BEGIN_SET_TRANSACTION_FACTORY(CCDRManager)
	ON_TRANS("TRANS_MCU",	"SET_CDR_SETTINGS",	CCDRSettings,	CCDRManager::HandleSetCDRSettings)
END_TRANSACTION_FACTORY

///////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CCDRManager)
  ONCOMMAND("ls_cdr", CCDRManager::HandleTerminalListCDR, "Displays list of CDR files")
  ONCOMMAND("get_last_conf_id", CCDRManager::HandleTerminalGetLastConfId, "get last conf id")
  ONCOMMAND("testPerst", CCDRManager::HandleTerminalTestPersistence, "test persistence")
  ONCOMMAND("testPerstAddItem", CCDRManager::HandleTerminalTestPersistenceAddItem, "test persistence add item")
  ONCOMMAND("testPerstAddItemSynced", CCDRManager::HandleTerminalTestPersistenceAddItemSynced, "test persistence add item synced")
  ONCOMMAND("testPerstAddMultipleItemsFromFile", CCDRManager::HandleTerminalTestPersistenceAddMultipleItemsFromFile, "a persistence add multiple items from files")
  ONCOMMAND("testPerstAddMultipleItems", CCDRManager::HandleTerminalTestPersistenceAddMultipleItems, "a persistence add multiple items")
  ONCOMMAND("testPerstAddMultipleItemsSynced", CCDRManager::HandleTerminalTestPersistenceAddMultipleItemsSynced, "a persistence add multiple items")
  ONCOMMAND("testPerstxml", CCDRManager::HandleTerminalTestXml2source, "a persistence add multiple items");
  ONCOMMAND("testqEnable", CCDRManager::HandleTerminalTestPersistenceEnable, "test persistence enable adding items")
  ONCOMMAND("testqPeekAndPopAll", CCDRManager::HandleTerminalTestPersistencePeekAndPopAll, "test persistence Peek and pop all")
  ONCOMMAND("testqCreateCdrThread", CCDRManager::HandleTerminalTestPersistenceCreateThread, "test persistence create cdr thread")
  ONCOMMAND("enableHttps", CCDRManager::HandleTerminalEnableReigsterHttps, "test persistence create cdr thread")
  ONCOMMAND("readcdrerrors", CCDRManager::HandleTerminalReadNumOfErrorsFromServer, "read number of errors from cdr Server")

END_TERMINAL_COMMANDS

///////////////////////////////////////////////////////////////////////////
extern void CDRMonitorEntryPoint(void* appParam);

///////////////////////////////////////////////////////////////////////////
void CDRManagerEntryPoint(void* appParam)
{
	CCDRManager* pCDRManager = new CCDRManager;
	pCDRManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CCDRManager::GetMonitorEntryPoint()
{
  return CDRMonitorEntryPoint;
}

CCDRManager::CCDRManager()
{
	m_persistenceQueue = NULL;
	m_CDRProcess = dynamic_cast<CCDRProcess*>(CProcessBase::GetProcess());
	CCdrLog* pCdrLog = new CCdrLog;
	m_CDRProcess->SetCdrLog(pCdrLog);

	// CDR Persistense
	ApiObjectsFactoriesRegistrar& r(ApiObjectsFactoriesRegistrar::instance());

	r.registerFactory(CdrPersistApiFactory::const_instance());
	r.registerFactory(CdrRegistrationApiFactory::const_instance());
	m_fileSystemWarning = FALSE;
	m_pConvertXmlMngApi = NULL;
	m_isHttpsEnabled = true;
	m_countCdrErros = 0;
	m_IsHardDiskOk = true;
}

void CCDRManager::SelfKill()
{
	if (m_pConvertXmlMngApi != NULL)
	{
		m_pConvertXmlMngApi->SyncDestroy();
		POBJDELETE(m_pConvertXmlMngApi);
		POBJDELETE (m_persistenceQueue);
	}
	CManagerTask::SelfKill();
}

CCDRManager::~CCDRManager()
{
	if (m_pConvertXmlMngApi != NULL)
	{
		m_pConvertXmlMngApi->SyncDestroy();
		POBJDELETE(m_pConvertXmlMngApi);
	}
	POBJDELETE (m_persistenceQueue);
}
// Virtual
const char* CCDRManager::NameOf() const
{
	return GetCompileType();
}

void CCDRManager::CreateConvertXmlTask()
{
	PTRACE(eLevelInfoNormal, "CCDRManager::CreateConvertXmlTask ");

	if( IsValidPObjectPtr(m_pConvertXmlMngApi) )
		m_pConvertXmlMngApi->SyncDestroy();
	else
		m_pConvertXmlMngApi = new CTaskApi;

	CreateTask(m_pConvertXmlMngApi, CdrConvertToXmlEntryPoint, m_pRcvMbx);
}

void CCDRManager::CreateCdrCommunicationThread()
{
	pthread_create(&g_LastCommunicationthreadCreated,NULL,CCDRManager::StartSendingCdrToServer,m_persistenceQueue);
	pthread_detach(g_LastCommunicationthreadCreated);
}

void CCDRManager::SetTimer(struct timespec* abstime)
{
	clock_gettime(CLOCK_REALTIME, abstime);
	abstime->tv_sec +=  AWAKE_CDR_THREAD_TIMOUT;
	abstime->tv_nsec = 0;
}

//while forever loop, wait on condition till get a signal (when other thread add cdr) or timout
void* CCDRManager::StartSendingCdrToServer(void *arg)
{
	CPersistenceQueue* pPersistQueue = (CPersistenceQueue*) arg;
	ApiBaseObjectPtr* itemFromfile;
	struct timespec abstime;
	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	CCDRSettings* pCdrSettings = pProcess->GetCdrSettings();
	bool isEventSent = false;
	while(pCdrSettings->GetIsRemoteCdrServer() && g_LastCommunicationthreadCreated == pthread_self())
	{
		pthread_mutex_lock(&queue_lock);
		while(pPersistQueue->IsEmpty() && pCdrSettings->GetIsRemoteCdrServer()==TRUE &&
				g_LastCommunicationthreadCreated == pthread_self())
		{
			SetTimer(&abstime);
			pthread_cond_timedwait(&queue_nonempty, &queue_lock, &abstime);
		}

		itemFromfile  = pPersistQueue->Peek();
		if (itemFromfile==NULL)
		{
			pthread_mutex_unlock(&queue_lock);
			continue;
		}
		PlcmCdrEvent* Plcm_cdr_event = (itemFromfile && itemFromfile->Contains(PlcmCdrEvent::classType())) ? (PlcmCdrEvent*)*itemFromfile : NULL;
		pthread_mutex_unlock(&queue_lock);

		isEventSent = TrySendEventToServer(Plcm_cdr_event);
		if(isEventSent == true)
		{
			pthread_mutex_lock(&queue_lock);
			pPersistQueue->PopAfterPeek();
			pthread_mutex_unlock(&queue_lock);
		}
		else
		{
			m_countCdrErros++;
		}
	}
	pthread_exit(NULL);

}
STATUS CCDRManager::HandleTerminalTestPersistenceAddItemSynced(CTerminalCommand& command,
		std::ostream& answer)
{
	if(0 == command.GetNumOfParams())
		return STATUS_OK;
	const string &strNum = command.GetToken(eCmdParam1);
	int id = atoi(strNum.c_str());

	pthread_mutex_lock(&queue_lock);
	bool isQueueBeforeAddEmpty = m_persistenceQueue->IsEmpty();
	PersistItem psItem1;
	psItem1.m_checkInt = id;
	ApiBaseObjectPtr item;
	item = &psItem1;
	m_persistenceQueue->AddItem(item);
	if (isQueueBeforeAddEmpty)
		pthread_cond_signal(&queue_nonempty);
	pthread_mutex_unlock(&queue_lock);

	return STATUS_OK;
}

STATUS CCDRManager::HandlePersistenceAddCdr(CSegment* pSeg)
{
	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	CCDRSettings* cdrSettings = pProcess->GetCdrSettings();
	if(cdrSettings->GetIsRemoteCdrServer() == false)
		return STATUS_OK;
	PlcmCdrEvent  Plcm_cdr_event;
	*pSeg >> Plcm_cdr_event;

	pthread_mutex_lock(&queue_lock);
	bool isQueueBeforeAddEmpty = m_persistenceQueue->IsEmpty();

	ApiBaseObjectPtr item(&Plcm_cdr_event);

	m_persistenceQueue->AddItem(item);
	if (isQueueBeforeAddEmpty)
		pthread_cond_signal(&queue_nonempty);
	pthread_mutex_unlock(&queue_lock);
	//TODO:python test with alot of events
	return STATUS_OK;
}

std::string CCDRManager::CreateMsgToServer(PlcmCdrEvent* itemFromfile)
{
	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	CCDRSettings* cdrSettings = pProcess->GetCdrSettings();
	itemFromfile->m_sourceId = cdrSettings->GetSourceId();
	PlcmCdrEventList eventList;
	eventList.m_plcmCdrEvent.push_back(*itemFromfile);
	eventList.m_sendTime = GetGmtTime();
	std::ostringstream xmlTosend;
	xmlTosend << eventList;
	return xmlTosend.str();
}
//retry send event to server every CDR_SERVICE_RETRY_TIME seconds
bool CCDRManager::TrySendEventToServer(PlcmCdrEvent* itemFromfile)
{
	bool isEventSentSuccessfuly = false;
	int numberOfFailures = 0;
	if (itemFromfile==NULL)
		return TRUE;
	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	DWORD	 retryTime;
	pProcess->GetSysConfig()->GetDWORDDataByKey(CFG_KEY_CDR_SERVICE_RETRY_TIME, retryTime);
	std::string msgToServer = CreateMsgToServer(itemFromfile);

	CSegment* pSeg = NULL;
	while(isEventSentSuccessfuly == false)
	{
		isEventSentSuccessfuly = SendEventToServer(msgToServer);
		if(isEventSentSuccessfuly == false)
		{
			numberOfFailures++;
			if(numberOfFailures == NUMBER_OF_FAILURE_FOR_SENDING_CDR_EVENT)
			{
				pSeg = new CSegment;
				pProcess->GetManagerApi()->SendMsg(pSeg, CDR_ENABLE_AA_SERVER_CONEECTION_DOWN);
				//server is down, so after 3 retries enlarge the timeout six times than the origin retry time
				retryTime = retryTime * 6;
			}
			sleep (retryTime);
		}
	}

	if (g_isAAServerConEnabled == true)
	{
		pSeg = new CSegment;
		pProcess->GetManagerApi()->SendMsg(pSeg, CDR_DISABLE_AA_SERVER_CONEECTION_DOWN);
	}

	return TRUE;
}




bool CCDRManager::SendEventToServer(std::string msgToServer)
{

	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	CurlHTTP* pcurlhttpConn = new CurlHTTP(true,true,true);

	CURLcode retVal;

	std::string url = "https://";
	CCDRSettings* cdrSettings = pProcess->GetCdrSettings();
	url+= cdrSettings->GetIp();
	url+= ":";
	url+= cdrSettings->GetPort();
	//https://10.47.13.47:8443/api/rest/cdr/event-collector
	url+="/api/rest/cdr/event-collector";

	//TODO: when CDR service will support NTLM change from basic authentication to NTLM (SetNtlmAuthenticationParams instead of SetBasicAuthenticationParams)
	pcurlhttpConn->SetBasicAuthenticationParams(cdrSettings->GetUser(), cdrSettings->GetPwd());
	bool bReponse = pcurlhttpConn->PostRequest(retVal,url,msgToServer.c_str()/*strReq*/);

	/*std::ostringstream cmd;
	cmd<< "SendEventToServer msgToServer=" << msgToServer << " GetResponseContent = " <<pcurlhttpConn->GetResponseContent()->c_str() << " retVal=" << retVal;
	PrintErrorToLocalFile(cmd);*/

	std::string::size_type start_pos = pcurlhttpConn->GetResponseHeader().find(REST_RESPONSE_BAD_REQUEST);
	if( std::string::npos != start_pos )
	{
		bReponse = true;
	}
	delete pcurlhttpConn;
	return bReponse;
}
std::string CCDRManager::GetGmtTime()
{
	CStructTm curTime;
	SystemGetTime(curTime);
	time_t currTime = curTime.GetAbsTime(true);
	tm* pCurrGmtTimeTm;

	// time(&currTime);
	pCurrGmtTimeTm = gmtime(&currTime);
	char yearSt[16];
	char monthSt[16];
	char daySt[16];
	char hourSt[16];
	char minSt[16];
	char secSt[16];
	sprintf(yearSt,"%2d",curTime.m_year);
	sprintf(monthSt,"%02d",(pCurrGmtTimeTm->tm_mon + 1));
	sprintf(daySt,"%02d",pCurrGmtTimeTm->tm_mday);

	sprintf(hourSt,"%02d",pCurrGmtTimeTm->tm_hour);
	sprintf(minSt,"%02d",pCurrGmtTimeTm->tm_min);
	sprintf(secSt,"%02d",pCurrGmtTimeTm->tm_sec);

	std::string timeRet = std::string(yearSt) + "-" + std::string(monthSt) + "-" + std::string(daySt) + "T" + std::string(hourSt) + ":" + std::string(minSt) + ":" + std::string(secSt) + ".0Z";
	return timeRet;

}

void CCDRManager::ManagerPostInitActionsPoint()
{
  if (!IsHardDiskOk())
  {
    AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            BAD_HARD_DISK,
                            MAJOR_ERROR_LEVEL,
                            "Hard disk not responding",
                            true,
                            true);
    return;
  }

  CCdrLog* pCdrLog = m_CDRProcess->GetCdrLog();
  pCdrLog->InitDB();


  if (!pCdrLog->IsReady())
  {
    AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            FAILED_TO_INIT_FILE_SYSTEM,
                            MAJOR_ERROR_LEVEL,
                            "Failed to initialize the file system and create the CDR index",
                            true,
                            true);
  }
  m_fileSystemWarning = FALSE;
  m_pConvertXmlMngApi = NULL;
  CreateConvertXmlTask();
  pthread_cond_init(&queue_nonempty, NULL);

  StartTimer(CDR_FILE_SYSTEM_WARNING_TIMER,
             CDR_FILE_WARNING_TIMEOUT);


  //Load CdrSettings
    CCDRSettings* pCdrSettings = new CCDRSettings();
    int status = pCdrSettings->ReadXmlFile();
    if (status != STATUS_OK)
    {
		//if file doesn't exist then write default values
    	pCdrSettings->m_isSerializeToEMA = FALSE;
		pCdrSettings->WriteXmlFile();
		pCdrSettings->m_isSerializeToEMA = TRUE;
    }
    m_CDRProcess->SetCdrSettings(pCdrSettings);

    if(pCdrSettings->GetIsRemoteCdrServer() == TRUE)
    {
    	m_persistenceQueue = new CPersistenceQueue(PERSISTENCE_QUEUE_CDR_NAME);
    	CreateCdrCommunicationThread();  //this function must be after m_persistenceQueue is initiated
    	EnableSystemMonitoringTimerIfAlive();

    }
 

	POBJDELETE(pCdrSettings);
	//TRACESTR(eLevelInfoNormal) << "ManagerPostInitActionsPoint " << m_CDRProcess->GetCdrSettings()->GetIp();
}

bool CCDRManager::IsSystemMonitoringUp()
{
	bool   is_alive = FALSE;
	STATUS status = IsProcAlive("SystemMonitoring", is_alive);
	FTRACECOND_AND_RETURN_VALUE(STATUS_OK != status,
			"Assumed that SystemMonitoring is down",
			TRUE);
	return is_alive;
}

void CCDRManager::EnableSystemMonitoringTimerIfAlive()
{
	if( IsSystemMonitoringUp() == FALSE )
	{
		PTRACE(eLevelInfoNormal,"CCDRManager::EnableSystemMonitoringTimerIfAlive - SystemMonitoring is not alive yet");
		return;
	}
	PTRACE(eLevelInfoNormal,"CCDRManager::EnableSystemMonitoringTimerIfAlive - SystemMonitoring is alive");
	EnableDisalbeRemoteCdrTimer(TRUE);
}
void CCDRManager::OnMcuMngrCdrParamsInd(CSegment* pSeg)
{
	std::ostringstream stChassis;
	pSeg->Get( (BYTE*)(&m_McmsInfoStruct), sizeof(MCMS_INFO_S) );
	m_serialNum = m_McmsInfoStruct.serialNumber;
	m_ipv4 = m_McmsInfoStruct.ipv4;
	m_ipv6 = m_McmsInfoStruct.ipv6;
	stChassis << m_McmsInfoStruct.chassisVersion.ver_major << "." << m_McmsInfoStruct.chassisVersion.ver_minor << "." << m_McmsInfoStruct.chassisVersion.ver_release << "." << m_McmsInfoStruct.chassisVersion.ver_internal;
	m_hardwareVersion = stChassis.str();
	TRACESTR(eLevelInfoNormal) << "CCDRManager::OnMcuMngrCdrParamsInd " << " serialNum=" << m_McmsInfoStruct.serialNumber << " ipv4=" << m_ipv4 << " ipv6=" << m_ipv6 << " m_hardwareVersion=" << m_hardwareVersion;
}

// Virtual
int CCDRManager::GetTaskMbxBufferSize() const
{
  return 256 * 1024 - 1;
}



std::string CCDRManager::RegisterToCdrServer(CCDRSettings* pCdrSettings)
{
	std::string sourceId = "";
	CURLcode responseStatus;
	std::ostringstream st;
	std::ostringstream strVersion;
	CVersions ver;

	//build registration request
	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	std::string versionFilePath = VERSIONS_FILE_PATH;
	STATUS status = ver.ReadXmlFile(versionFilePath.c_str());
	VERSION_S mcuVer = ver.GetMcuVersion();
	strVersion << mcuVer.ver_major << "." <<mcuVer.ver_minor << "." << mcuVer.ver_release << "." << mcuVer.ver_internal;
	PlcmCdrEventSource pl;
	//TRACESTR(eLevelInfoNormal) << "CCDRManager::RegisterToCdrServer, \nm_ipv4: " << m_ipv4;
	pl.m_ipAddressList.m_ipAddress.push_back(m_ipv4);
	pl.m_productType = "Bridge";
	pl.m_name = "RP Collaboration Server (RMX)";
	pl.m_product = "RPCS"; //ProductFamilyToString(pProcess->GetProductFamily());
	pl.m_model = GetModel(pProcess);
	pl.m_softwareVersion = strVersion.str();
	pl.m_hardwareVersion = m_hardwareVersion;
	pl.m_serialNumber = m_serialNum;

	//pl.WriteToXmlFile(MCU_TMP_DIR+"/pl_test.xml");

	//build url
	CurlHTTP* pcurlhttpConn = new CurlHTTP(true,true,true);

//	TRACESTR(eLevelInfoNormal) << "CCDRManager::RegisterToCdrServer, \nusr: " << pCdrSettings->GetUser() << "\npwd:" <<  pCdrSettings->GetPwd();
	st << pl;
	std::string url; //= "https://";// + "10.47.17.79" + "/api/rest/cdr/event-sources";
	if(m_isHttpsEnabled == TRUE)
	{
		 url = "https://";
		 pcurlhttpConn->SetBasicAuthenticationParams(pCdrSettings->GetUser(), pCdrSettings->GetPwd());
	}
	else
	{
		url = "http://";
	}
	url.append(pCdrSettings->GetIp());
	url.append(":");
	url.append(pCdrSettings->GetPort());
	url.append("/api/rest/cdr/event-sources");
//	TRACESTR(eLevelInfoNormal) << "CCDRManager::RegisterToCdrServer, \nurl: " << url << "\ndata:\n" << st.str();

	//send request
	bool bResponse = pcurlhttpConn->PostRequest(responseStatus,url,st.str());
	/*TRACESTR(eLevelInfoNormal) << "CCDRManager::CCDRManager, responseHeader=" << pcurlhttpConn->GetResponseHeader();
	TRACESTR(eLevelInfoNormal) << "CCDRManager::CCDRManager, status=" << responseStatus;
	TRACESTR(eLevelInfoNormal) << "CCDRManager::CCDRManager, statusString=" << pcurlhttpConn->GetCurlStatusAsString(responseStatus);
	TRACESTR(eLevelInfoNormal) << "CCDRManager::CCDRManager, responseContent=" << pcurlhttpConn->GetResponseContent()->c_str();*/

	//deal with registration response
	if (bResponse)
	{
		PlcmCdrEventSourceId si;
		si.ReadFromXmlStream(pcurlhttpConn->GetResponseContent()->c_str(), pcurlhttpConn->GetResponseContent()->length());
		sourceId = (std::string)si.m_sourceId.value();
		//TRACESTR(eLevelInfoNormal) << "CCDRManager::CCDRManager, sourceId=" << sourceId;
	}

	delete pcurlhttpConn;
	return sourceId;
}


STATUS CCDRManager::HandleSetCDRSettings(CRequest *pRequest)
{
	pRequest->SetConfirmObject(new CDummyEntry());
	pRequest->SetStatus(STATUS_OK);
    if (pRequest->GetAuthorization() != SUPER)
	{
		//TRACESTR(eLevelInfoNormal) << "CCDRManager::HandleSetCDRSettings: No permission to update cdr settings";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}
    else
    {
    	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
    	CCDRSettings* pCdrSettingsOld = pProcess->GetCdrSettings();
    	CCDRSettings* pCdrSettings = (CCDRSettings*)(pRequest->GetRequestObject());
    	DWORD ifNoneMatch = pRequest->GetIfNoneMatch();

    	if( (ifNoneMatch  <  numeric_limits<DWORD>::max()) && ( pRequest->GetIfNoneMatch() != pCdrSettingsOld->GetUpdateCounter()))
    	{
    		pRequest->SetStatus(STATUS_PRECONDITION_FAILED);
    		return STATUS_OK;
    	}

		if ((*pCdrSettings) != (*pCdrSettingsOld))
    	{
			//BOOL isLocal = pCdrSettings->GetIsLocalCdrServer();
			BOOL isLocal = TRUE; //In next phase handle local CDR

			BOOL isRemote = pCdrSettings->GetIsRemoteCdrServer();
			if (isLocal == FALSE && isRemote == FALSE)
			{
				pRequest->SetStatus(STATUS_AT_LEAST_ONE_CDR_SERVICE);
			}
			else
			{
				if (isRemote == FALSE)
				{
					//TRACESTR(eLevelInfoNormal) << "CCDRManager::HandleSetCDRSettings:Saving Cdr Settings, IsRemoteCdrServer==FALSE.";
					DisableActiveAlarmCdrServerConeectionDown(NULL);
					pCdrSettingsOld->IncreaseUpdateCounter();
					pCdrSettings->SetUpdateCounter(pCdrSettingsOld->GetUpdateCounter());
					SaveCdrSettingsToFile(pCdrSettings);
					EnableDisalbeRemoteCdrTimer(FALSE);
				}
				else
				{
					std::string sourceId = "";
					//TRACESTR(eLevelInfoNormal) << "CCDRManager::HandleSetCDRSettings:before register. CDR_SERVICE_RETRY_TIME exists, iteration=" << i;
					sourceId = RegisterToCdrServer(pCdrSettings);
					if (!sourceId.empty())
					{
						BOOL bOldSetting = pCdrSettingsOld->GetIsRemoteCdrServer();
						pCdrSettings->m_sourceId = sourceId;
						pCdrSettingsOld->IncreaseUpdateCounter();
						pCdrSettings->SetUpdateCounter(pCdrSettingsOld->GetUpdateCounter());
						SaveCdrSettingsToFile(pCdrSettings);
						if (bOldSetting==FALSE && pCdrSettings->GetIsRemoteCdrServer()==TRUE)
						{
							POBJDELETE(m_persistenceQueue);
							//TRACESTR(eLevelInfoNormal) << "CCDRManager::HandleSetCDRSettings: in if";
							m_persistenceQueue = new CPersistenceQueue(PERSISTENCE_QUEUE_CDR_NAME);
							CreateCdrCommunicationThread();  //this function must be after m_persistenceQueue is initiated
							EnableDisalbeRemoteCdrTimer(TRUE);
						}
						//TRACESTR(eLevelInfoNormal) << "CCDRManager::HandleSetCDRSettings:after register sourceId = "<< sourceId;

					}
					else
					{
						EnableDisalbeRemoteCdrTimer(FALSE);
						pRequest->SetStatus(STATUS_FAILED_REGISTER_TO_CDR_SERVER);
					}
				}
			}
    	}
    }
    return STATUS_OK;
}

STATUS CCDRManager::IsRemoteCdrEnable(CSegment* msg)
{
	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	CCDRSettings* pCdrSettings = pProcess->GetCdrSettings();
	BOOL isRemote = pCdrSettings->GetIsRemoteCdrServer();

	    CSegment *pRetParam = new CSegment;
	    *pRetParam << (DWORD)isRemote;

	    STATUS status = ResponedClientRequest(CDR_IS_REMOTE_CDR_ENABLE, pRetParam);

	    return status;
}
void CCDRManager::EnableDisalbeRemoteCdrTimer(BOOL isEnable)
{
	CSegment*  pParam = new CSegment();
	*pParam << (BYTE)isEnable;

	CManagerApi apiSystemMonitoring(eProcessSystemMonitoring);
	apiSystemMonitoring.SendMsg(pParam, CDR_ENABLE_DISABLE_REMOTE_CDR_TIMER);
}

void CCDRManager::SaveCdrSettingsToFile(CCDRSettings* pCdrSettings)
{
	CCDRProcess* pProcess = (CCDRProcess*)CCDRProcess::GetProcess();
	pProcess->SetCdrSettings(pCdrSettings);
	pProcess->GetCdrSettings()->m_isSerializeToEMA = FALSE;
	pProcess->GetCdrSettings()->WriteXmlFile();
	pProcess->GetCdrSettings()->m_isSerializeToEMA = TRUE;
}

const char* CCDRManager::GetModel(CCDRProcess* pProcess)
{
	const char* productType = ProductTypeToString(pProcess->GetProductType());
	if (strcmp(productType,"RMX2000") == 0)
		return "2000";
	if (strcmp(productType,"RMX4000") == 0)
		return "4000";
	if (strcmp(productType,"RMX1500") == 0)
		return "1500";

	else
		return productType;
}

STATUS CCDRManager::HandleCDRStartConf(CSegment* pSeg)
{
  STATUS status = IsCanPerformHDOperation();
  if (STATUS_OK != status)
  {
    FormatTraceMessage("CCDRManager::HandleCDRStartConf",
                       "CDR start conference",
                       status);
    return status;
  }

  m_CDRProcess->GetCdrLog()->ConfStart(pSeg);

  return STATUS_OK;
}

STATUS CCDRManager::HandleCDREvent(CSegment* pSeg)
{
  STATUS status = IsCanPerformHDOperation();
  if (STATUS_OK != status)
  {
    FormatTraceMessage("CCDRManager::HandleCDREvent",
                       "CDR event",
                       status);
    return status;
  }

  m_CDRProcess->GetCdrLog()->ConfEvent(pSeg);

  return STATUS_OK;
}

STATUS CCDRManager::HandleCDREndConf(CSegment* pSeg)
{
  STATUS status = IsCanPerformHDOperation();
  if (STATUS_OK != status)
  {
    FormatTraceMessage("CCDRManager::HandleCDREndConf",
                       "End Conference",
                       status);
    return status;
  }

  m_CDRProcess->GetCdrLog()->ConfEnd(pSeg);

  return STATUS_OK;
}

STATUS CCDRManager::HandleGetLastConfId(CSegment* pSeg)
{
  DWORD lastConfId = 42;

  STATUS status = IsCanPerformHDOperation();
  if (STATUS_OK != status)
    FormatTraceMessage("CCDRManager::HandleGetLastConfId",
                       "Try to print conf list",
                       status);

  CCdrLog* pCdrLog = m_CDRProcess->GetCdrLog();
  lastConfId = pCdrLog->GetBiggestConfId();

  LAST_CONF_ID_S lastConfIdStruct;
  memset(&lastConfIdStruct, 0, sizeof(LAST_CONF_ID_S));
  lastConfIdStruct.last_conf_id = lastConfId;

  CSegment* pSegment = new CSegment;
  pSegment->Put((BYTE*)&lastConfIdStruct, sizeof(LAST_CONF_ID_S));

  TRACEINTO << CLastConfIdWrapper(lastConfIdStruct);

  CManagerApi api(eProcessResource);
  api.SendMsg(pSegment, CDR_RSRC_SET_LAST_CONF_ID_IND);

  return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalListCDR(CTerminalCommand& command,
                                          std::ostream& answer)
{
  CCdrList* cdrlist = m_CDRProcess->GetCdrLog()->GetCdrList();

  answer << "   ID |    ConfID | Part |  File |               Name |            Display \n"
         << "------+-----------+------+-------+--------------------+-------------------\n";

  size_t index = 0;
  for (const CCdrShort* cdr = cdrlist->GetFirstShort();
       NULL != cdr;
       cdr = cdrlist->GetNextShort(), ++index)
  {
    answer << " " << std::setw(4) << index << " | "
           << std::setw(9) << cdr->GetConfId() << " | "
           << std::setw(4) << cdr->GetFilePartIndex() << " | "
           << std::setw(5) << cdr->GetFileName() << " | "
           << std::setw(18) << cdr->GetH243ConfName() << " | "
           << std::setw(18) << cdr->GetDisplayName() << std::endl;
  }

  return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalGetLastConfId(CTerminalCommand& command,
                                                std::ostream& answer)
{
  CCdrLog* pCdrLog = m_CDRProcess->GetCdrLog();
  DWORD    lastConfId = pCdrLog->GetBiggestConfId();

  // script TestCDRGetLastId.py uses the same string, change it mutually.
  const char* header = "Last Conf Id : ";
  answer << header << lastConfId;

  return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalTestPersistenceEnable(CTerminalCommand& command,
        std::ostream& answer)
{
	m_psQDebug->EnableAddingItem();
	answer << "HandleTerminalTestPersistenceEnable";
	return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalTestPersistenceDisable(CTerminalCommand& command,
        std::ostream& answer)
{
	m_psQDebug->DisableAddingItem();
	answer << "HandleTerminalTestPersistenceDisable";
	return STATUS_OK;
}

STATUS CCDRManager::HandlePersistenceDisable()
{
	m_persistenceQueue->DisableAddingItem();
	return STATUS_OK;
}
STATUS CCDRManager::HandlePersistenceEnable()
{
	m_persistenceQueue->EnableAddingItem();
	return STATUS_OK;
}


STATUS CCDRManager::HandleTerminalTestPersistenceAddItem(CTerminalCommand& command,
        std::ostream& answer)
{

	if(0 == command.GetNumOfParams())
			return STATUS_OK;
	const string &strNum = command.GetToken(eCmdParam1);
		  int id = atoi(strNum.c_str());
		  PersistItem psItem1;
		  psItem1.m_checkInt = id;

		  ApiBaseObjectPtr item;
		  item = &psItem1;
		  m_persistenceQueue->AddItem(item);

	return STATUS_OK;
}
STATUS CCDRManager::HandleTerminalEnableReigsterHttps(CTerminalCommand& command,
		std::ostream& answer)
{

	if(0 == command.GetNumOfParams())
		return STATUS_OK;
	const string &strNum = command.GetToken(eCmdParam1);
	int id = atoi(strNum.c_str());
	if(id == 0)
	{
		m_isHttpsEnabled = FALSE;
	}
	else
	{
		m_isHttpsEnabled = TRUE;
	}


	return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalReadNumOfErrorsFromServer(CTerminalCommand& command,
		std::ostream& answer)
{
	char buffer [16];
	memset(buffer,'\0',16);

	sprintf(buffer, "%d", m_countCdrErros);
	answer << buffer;

	return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalTestPersistenceAddMultipleItemsSynced(CTerminalCommand& command,
		std::ostream& answer)
{
	if(0 == command.GetNumOfParams())
		return STATUS_OK;
	//m_persistenceQueue = new CPersistenceQueue(PERSISTENCE_QUEUE_CDR_NAME, 311, 3);
	const string &numOfIemsStr = command.GetToken(eCmdParam1);
	int numOfItems = atoi(numOfIemsStr.c_str());

	const string &secondsToWaitStr = command.GetToken(eCmdParam2);
	int secondsToWait = atoi(secondsToWaitStr.c_str());

	PersistItem psItem;
	int id = 0;

	while(numOfItems>0)
	{
		pthread_mutex_lock(&queue_lock);
		bool isQueueBeforeAddEmpty = m_persistenceQueue->IsEmpty();
		psItem.m_checkInt = id;
		psItem.m_checkstr = "strField";
		ApiBaseObjectPtr item;
		item = &psItem;
		m_persistenceQueue->AddItem(item);

		if(isQueueBeforeAddEmpty)
		{
			pthread_cond_signal(&queue_nonempty);
		}
		pthread_mutex_unlock(&queue_lock);
		id ++ ;
		numOfItems -- ;
		sleep(secondsToWait);
	}

	return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalTestXml2source(CTerminalCommand& command,	std::ostream& answer)
{

	int numOfItems = 5;
	POBJDELETE(m_persistenceQueue);
	m_persistenceQueue = new CPersistenceQueue(PERSISTENCE_QUEUE_CDR_NAME, numOfItems - 1, 3);
	PersistItem psItem;
	ApiBaseObjectPtr item;
	item = &psItem;
	m_persistenceQueue->AddItem(item);

	answer << m_persistenceQueue->GetInternalQueue();
	return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalTestPersistenceAddMultipleItems(CTerminalCommand& command,
		std::ostream& answer)
{
	if(0 == command.GetNumOfParams())
		return STATUS_OK;
	const string &numOfIemsStr = command.GetToken(eCmdParam1);
	int numOfItems = atoi(numOfIemsStr.c_str());
	POBJDELETE(m_persistenceQueue);
	m_persistenceQueue = new CPersistenceQueue(PERSISTENCE_QUEUE_CDR_NAME, numOfItems - 1, 3);
	PersistItem psItem;
	int id = 0;
	ApiBaseObjectPtr item;

	while(numOfItems>0)
	{
		psItem.m_checkInt = id;
		psItem.m_checkstr = "strField";
		item = &psItem;
		m_persistenceQueue->AddItem(item);

		id ++ ;
		numOfItems -- ;
	}
	answer << m_persistenceQueue->GetInternalQueue().m_PersistList.size();
	return STATUS_OK;
}
STATUS CCDRManager::HandleTerminalTestPersistenceAddMultipleItemsFromFile(CTerminalCommand& command,
        std::ostream& answer)
{
	if(0 == command.GetNumOfParams())
			return STATUS_OK;
	const string &numOfIemsStr = command.GetToken(eCmdParam1);
		  int numOfItems = atoi(numOfIemsStr.c_str());
		  POBJDELETE(m_persistenceQueue);
		  m_persistenceQueue = new CPersistenceQueue(PERSISTENCE_QUEUE_CDR_NAME, numOfItems*311, 3);

	PersistItem psItem;
	int id = 0;
	std::vector<FDStruct> files;

	// Gets all files in folder
	std::string dir = MCU_TMP_DIR+"/OldCdrs/";
	BOOL res = GetDirectoryContents(dir.c_str(), files);
	while(numOfItems>0)
	{
		for (std::vector<FDStruct>::const_iterator file = files.begin();
				file != files.end();
				file++)
		{
			string filename = MCU_TMP_DIR+"/OldCdrs/";
			filename+=file->name.c_str();
			std::ifstream ifs(filename.c_str());

			std::string content( (std::istreambuf_iterator<char>(ifs) ),
					(std::istreambuf_iterator<char>()    ) );
			psItem.m_checkInt = id;
			psItem.m_checkstr = content;
			ApiBaseObjectPtr item;
			item = &psItem;
			m_persistenceQueue->AddItem(item);
			id ++ ;

		}
		numOfItems -- ;
	}
	answer << m_persistenceQueue->GetInternalQueue().m_PersistList.size();
	return STATUS_OK;
}
STATUS CCDRManager::HandleTerminalTestPersistencePopAfterPeek(CTerminalCommand& command,
        std::ostream& answer)
{
	m_persistenceQueue->PopAfterPeek();
	return STATUS_OK;
}


STATUS CCDRManager::HandleTerminalTestPersistenceCreateThread(CTerminalCommand& command,
		std::ostream& answer)
{
	CreateCdrCommunicationThread();
	return STATUS_OK;
}
STATUS CCDRManager::HandleTerminalTestPersistencePeekAndPopAll(CTerminalCommand& command,
		std::ostream& answer)
{
	ApiBaseObjectPtr* itemFromfile;
	while(!m_persistenceQueue->IsEmpty())
	{
		itemFromfile  = m_persistenceQueue->Peek();
		answer << " \n  HandleTerminalTestPersistencePeekAndPopAll ";// <<itemFromfile->m_checkInt;
		m_persistenceQueue->PopAfterPeek();
	}
	return STATUS_OK;
}

STATUS CCDRManager::HandleTerminalTestPersistence(CTerminalCommand& command,
        std::ostream& answer)
{
	answer << " HandleTerminalTestPersistence here";
	PersistenceList listOfItemsToRead;
	std::string fname = MCU_TMP_DIR+"/saved.txt";
	listOfItemsToRead.ReadFromXmlFile(fname.c_str());
	return STATUS_OK;
}

STATUS CCDRManager::IsCanPerformHDOperation() const
{
  if (!IsHardDiskOk())
    return STATUS_NO_HARD_DISK;

  if (!m_CDRProcess->GetCdrLog()->IsReady())
    return STATUS_FAIL_TO_PARSE_INDEX_FILE;

  return STATUS_OK;
}

void CCDRManager::FormatTraceMessage(const std::string& location,
                                     const std::string& message,
                                     STATUS status)
{
  const std::string& strStatus = m_CDRProcess->GetStatusAsString(status);
  string             buff;

  buff += location;
  buff += "\n";
  buff += message;
  buff += "\n";
  buff += "Status : ";
  buff += strStatus;
  buff += "\n";

  PTRACE(eLevelInfoNormal, buff.c_str());
}

void CCDRManager::InformHttpGetFile(const std::string& file_name)
{
  m_CDRProcess->GetCdrLog()->SetCdrMarked(file_name.c_str());

  StartTimer(CDR_FILE_SYSTEM_WARNING_TIMER,
             CDR_FILE_WARNING_TIMEOUT);
}

void CCDRManager::OnTimerFileSystemWarningTest()
{
  BOOL make_alarm = FALSE;
  CProcessBase::GetProcess()->GetSysConfig()->
  GetBOOLDataByKey(CFG_KEY_ENABLE_CYCLIC_FILE_SYSTEM_ALARMS, make_alarm);

  BOOL warning_flag =
    m_CDRProcess->GetCdrLog()->GetCdrList()->TestForFileSystemWarning();
  if (warning_flag && !m_fileSystemWarning)
    if (make_alarm)
    {
      AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                     BACKUP_OF_CDR_FILES_IS_REQUIRED,
                     MAJOR_ERROR_LEVEL,
                     "Administrator has to backup old CDR files before system will delete them",
                     true,
                     true);
      m_fileSystemWarning = TRUE;
    }

  if (m_fileSystemWarning && !warning_flag)
  {
    RemoveActiveAlarmByErrorCode(BACKUP_OF_CDR_FILES_IS_REQUIRED);
    m_fileSystemWarning = FALSE;
  }

  StartTimer(CDR_FILE_SYSTEM_WARNING_TIMER, CDR_FILE_WARNING_IDLE_TIMEOUT);
}

STATUS CCDRManager::HandleHandleCDRRetrievedNotify(CSegment* pSeg)
{
  StartTimer(CDR_FILE_SYSTEM_WARNING_TIMER,
             CDR_FILE_WARNING_TIMEOUT);

  return STATUS_OK;
}
STATUS CCDRManager::HandleCDRCreateXmlFolder(CSegment* pSeg)
{

	if(pSeg == NULL)
		TRACEINTO << "HandleCDRCreateXmlFolder seg is null";

	if( IsValidPObjectPtr(m_pConvertXmlMngApi) )
	{

		CSegment* pnewSeg = new CSegment;
		//*pnewSeg << (WORD)555;
		*pnewSeg = *pSeg;
		m_pConvertXmlMngApi->SendMsg(pnewSeg, CDR_CONVERTOR_CREATE_XML);
	}
	return STATUS_OK;
}

STATUS CCDRManager::HandleAbortCreateXmlFolder()
{
	TRACEINTO << "HandleAbortCreateXmlFolder";
	if( IsValidPObjectPtr(m_pConvertXmlMngApi) )
	{
		m_pConvertXmlMngApi->SendOpcodeMsg(CDR_CONVERTOR_ABORT);
	}
	return STATUS_OK;
}

void CCDRManager::EnableActiveAlarmCdrServerConeectionDown(CSegment * pSeg)
{
	if(!IsActiveAlarmExistByErrorCode(AA_CDR_SERVER_CONNECTION_DOWN))

	AddActiveAlarm(
		FAULT_GENERAL_SUBJECT,
		AA_CDR_SERVER_CONNECTION_DOWN,
		MAJOR_ERROR_LEVEL,
		"CDR server connection down",
		true,
		true);

	g_isAAServerConEnabled = true;
}

void CCDRManager::DisableActiveAlarmCdrServerConeectionDown(CSegment * pSeg)
{
	if(IsActiveAlarmExistByErrorCode(AA_CDR_SERVER_CONNECTION_DOWN))
			RemoveActiveAlarmByErrorCode(AA_CDR_SERVER_CONNECTION_DOWN);
	g_isAAServerConEnabled = false;
}
