#include <time.h>
#include <sys/stat.h>
#include "NStream.h"
#include "IVRManager.h"
#include "IVRServiceList.h"
#include "IVRService.h"
#include "IVRSlidesList.h"
#include "ConfPartyDefines.h"
#include "ConfPartyGlobals.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "OsFileIF.h"
#include "OpcodesMcmsInternal.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "IncludePaths.h"
#include "FilesCache.h"


// timer for RollCall delete files
#define TIMER_CHECK_ROLL_CALL_FILES     1
// delete RollCall files action type
#define ACTION_TYPE_DELETE_UPON_STARTUP 1
#define ACTION_TYPE_DELETE_UPON_TIMER   2

PBEGIN_MESSAGE_MAP(CIVRManager)
	ONEVENT(TIMER_CHECK_ROLL_CALL_FILES, IDLE, CIVRManager::OnTimerCheckRollCallFiles)
PEND_MESSAGE_MAP(CIVRManager, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CIVRManager
////////////////////////////////////////////////////////////////////////////
CIVRManager::CIVRManager()
{
	m_state = IDLE;
	for (int i = 0; i < MAX_MUSIC_SOURCE_NUMBER; i++)
		m_musicSourceList[i] = NULL;

	m_musicSourceNum = 0;
	StartTimer(TIMER_CHECK_ROLL_CALL_FILES, 3600*SECOND); // checks every 1 hour
}

//--------------------------------------------------------------------------
CIVRManager::~CIVRManager()
{
	for (int i = 0; i < MAX_MUSIC_SOURCE_NUMBER; i++)
		delete (m_musicSourceList[i]);

	// deallocate the IVR list
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
	if (NULL != pAVmsgServiceList)    // Only if not initialized before
	{
		delete pAVmsgServiceList;
		::SetpAVmsgServList(NULL);
	}

	// deallocate the Slides list
	CIVRSlidesList* pSlidesList = ::GetpSlidesList();
	if (NULL != pSlidesList)
	{
		delete pSlidesList;
		::SetpSlidesList(NULL);
	}
}

//--------------------------------------------------------------------------
void CIVRManager::OnTimerCheckRollCallFiles(CSegment* pParam)
{
	PTRACE(eLevelError, "CIVRManager::OnTimerCheckRollCallFiles - Checking Roll-Call files on disk ");

	CleanRollCallFiles(ACTION_TYPE_DELETE_UPON_TIMER);

	StartTimer(TIMER_CHECK_ROLL_CALL_FILES, 3600*SECOND); // checks every 1 hour
}

//--------------------------------------------------------------------------
void CIVRManager::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
}

//--------------------------------------------------------------------------
int CIVRManager::InitIVRConfig()
{
	//FilesCache& filesCache = FilesCache::instance();
	//TRACEINTO << "The location of the url on disk is: "<<ex->GetFileLocation("http://www.polycom.com/1");

	// Create the AV message service list
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
	if (NULL == pAVmsgServiceList)    // Only if not initialized before
	{
		pAVmsgServiceList = new CAVmsgServiceList;
		::SetpAVmsgServList(pAVmsgServiceList);
	}

	// create Slides list
	CIVRSlidesList* pSlidesList = ::GetpSlidesList();
	if (NULL == pSlidesList)
	{
		pSlidesList = new CIVRSlidesList;
		::SetpSlidesList(pSlidesList);
	}

	// create all necessary folders for working if needed
	CreateIVRFolders(IVR_FOLDER_MAIN);

	CreateDirectory(IVR_FOLDER_ROLLCALL, 0777); // Roll Call folder needs write permissions: 'rwx rwx rwx'

	// Create all IVR directories (if needed) under the languages directories
	// the function gets all languages folders, passes each and add missing sub-folders if needed
	CIVRService* pIVRService = new CIVRService;
	pIVRService->UpdateIvrDirectories();
	POBJDELETE(pIVRService);

	// Fill IVR Service list from XML file
	int status = STATUS_OK;
	status = FillIvrListFromXmlFile(FILE_IVR_CONFIG_XML_LAST_GOOD_COPY);
	if (STATUS_OK != status)
	{
		TRACEINTO << "IVR-Error - FileName:" << FILE_IVR_CONFIG_XML_LAST_GOOD_COPY << ", File doesn't contain a proper list or couldn't be read";
		status = FillIvrListFromXmlFile(FILE_IVR_CONFIG_XML);
		if (STATUS_OK != status)
			{TRACEINTO << "IVR-Error - FileName:" << FILE_IVR_CONFIG_XML << ", File doesn't contain a proper list or couldn't be read";}
		else
			{TRACEINTO << " Read File OK: " << FILE_IVR_CONFIG_XML;}
	}
	else
	{
		TRACEINTO << " Read File OK: " << FILE_IVR_CONFIG_XML_LAST_GOOD_COPY;
	}

	BYTE isRMXInSlaveMode  = CProcessBase::GetProcess()->GetIsFailoverSlaveMode();

	if (STATUS_OK != status)
	{
		if (TRUE == isRMXInSlaveMode)
			TRACEINTO << "IVR-Error - FileName:" << FILE_IVR_CONFIG_XML_LAST_GOOD_COPY << ", Failed, but do nothing since RMX is in Slave MODE";
		else
			AddFaultForBadIVRListFiles();

		status = FillIvrListFromXmlFile(FILE_IVR_CONFIG_XML_STATIC);
		if (STATUS_OK != status)
		{
			TRACEINTO << " IVR-Error - Fill IVR File form Name: " << FILE_IVR_CONFIG_XML_STATIC;
		}
		else
		{
			TRACEINTO << " OK - Fill IVR File form Name: " << FILE_IVR_CONFIG_XML_STATIC;
		}
	}

	// VNGR-24993, add a file flag to distinguish the IVRDefaultServiceList.xml in StaticCfg is imported or not.
	// If DEFAULT_IVR_CONFIG_IMPORTED not exist, replace  IVRDefaultServiceList.xml with the one in StaticCfg
	if (!IsFileExists(DEFAULT_IVR_CONFIG_IMPORTED.c_str()))
	{
		if (IsFileExists(FILE_DEFAULT_IVR_CONFIG_XML_STATIC))
		{
			BOOL res = CopyFile(FILE_DEFAULT_IVR_CONFIG_XML_STATIC, FILE_DEFAULT_IVR_CONFIG_XML);
			if (!res)
				TRACEINTO << "IVR-Error - FileName:" << FILE_DEFAULT_IVR_CONFIG_XML_STATIC << ", Failed, Cannot copy the file, Status:" << (int)res;

			res = CreateFile(DEFAULT_IVR_CONFIG_IMPORTED.c_str());
			TRACEINTO << "IVR-Error - FileName:" << DEFAULT_IVR_CONFIG_IMPORTED << ", Failed, Cannot create the file, Status:" << (int)res;
		}
	}

	// Add IVR default service to IVR list from XML file
	AddDefaultIvrToIVRListFromXmlFile(FILE_DEFAULT_IVR_CONFIG_XML);

	// update default services if needed
	UpdateDefaultServices();

	// Save the list to disk
	SaveIvrServiceList();

	// Add Active Alarm if IVR Service is not valid
	if (STATUS_OK != ::GetpAVmsgServList()->CheckIvrListValidity())
	{
		if (TRUE == isRMXInSlaveMode)
			TRACEINTO << "IVR-Error - Failed, but do nothing since RMX is in Slave MODE";
		else
		{
			TRACEINTO << "IVR-Error - Validation failed, Creating AddActiveAlarmForBadIVRList";
			AddActiveAlarmForBadIVRList();
		}
	}

	// add music sources to the MFA (via Cards process)
	GetMusicSources();    // gets Music Sources
	AddAllMusicSources(); // sends all Music Sources to Cards process

	// fills slides list
	FillSlidesList();

	// clean RollCall files
	CleanRollCallFiles(ACTION_TYPE_DELETE_UPON_STARTUP);

	return STATUS_OK;
}

//--------------------------------------------------------------------------
int CIVRManager::FillIvrListFromXmlFile(const char* file_name)
{
	if (!IsFileExists(file_name))
	{
		TRACEINTO << "IVR-Error - FileName:" << file_name << ", Failed, File does not exist";
		return STATUS_FILE_NOT_EXIST;
	}

	// This function fills the IVR Service list from XML file
	TRACEINTO << "FileName:" << file_name << ", Fills Service IVR List from XML file";

	CAVmsgServiceList* pTmpAVmsgServiceList = new CAVmsgServiceList;

	int status = pTmpAVmsgServiceList->ReadXmlFile(file_name, eNoActiveAlarm, eRenameFile);
	if (status != STATUS_OK)
	{
		POBJDELETE(pTmpAVmsgServiceList);
		TRACEINTO << "IVR-Error - FileName:" << file_name << ", Failed to read file, Status:" << status;
		return status; // stays with empty IVR list
	}

	// Check IVR service list validity
	status = pTmpAVmsgServiceList->CheckIvrListValidity();
	// BRIDGE-5561 - status check remarked. IVR list will be loaded and if there are errors the user will be notified via active alarms.
//	if (STATUS_OK == status)
//	{
		// deletes the old pointer
		CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
		POBJDELETE(pAVmsgServiceList);

		// Set the global IVR service list to point on the temporary list
		::SetpAVmsgServList(pTmpAVmsgServiceList);
//	}

	return status;
}

//--------------------------------------------------------------------------
void CIVRManager::AddDefaultIvrToIVRListFromXmlFile(const char* file_name)
{
	// this function opens IVRDefaultServiceList.xml (if exists - no error if not) and adds its content (IVR Services)
	// to the IVR Service List (if not exists)

	// checks if file exists
	if (!IsFileExists(file_name))
	{
		TRACEINTO << "FileName:" << file_name << ", Failed, File does not exist";
		return; // stays with what we had
	}

	// Create Temporary IVR list for default service from XML file
	CAVmsgServiceList* pTempAVmsgList = new CAVmsgServiceList;

	// reads default services from XML file to temp list (eActiveAlarmInernal??)
	STATUS status = pTempAVmsgList->ReadXmlFile(file_name, eNoActiveAlarm, eRenameFile);
	if (status != STATUS_OK)
	{
		POBJDELETE(pTempAVmsgList);
		TRACEINTO << "FileName:" << file_name << ", Failed to read file, Status:" << status;
		return; // stays with what we had
	}

	// Update DTMF and general messages tables for the service in the list
	UpdateTables(pTempAVmsgList);

	// gets the first IVR Service from the temporary list
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();

	// VNGR-24993,Import  the new added IVR Events of IVR General Msgs Feature from the file
	ImportNewIVREvents(pAVmsgServiceList, pTempAVmsgList);

	CAVmsgService* pAVmsgService = pTempAVmsgList->GetFirstService();
	if (!pAVmsgService)
	{
		TRACEINTO << "FileName:" << file_name << ", is empty (OK)";
	}
	else
	{
		// VNGR-24993,IVRDefaultServiceList.xml is not empty,means a new image is installed.
		// After upgrade, we import the new added msg events from static config to mcms
		if (IsFileExists(FILE_IVR_CONFIG_XML_STATIC))
		{
			CAVmsgServiceList* pStaticAVmsgList = new CAVmsgServiceList;
			// reads static services from XML file to static AVmsg list
			if (STATUS_OK != pStaticAVmsgList->ReadXmlFile(FILE_IVR_CONFIG_XML_STATIC, eNoActiveAlarm, eRenameFile))
			{
				TRACEINTO << "FileName:" << FILE_IVR_CONFIG_XML_STATIC << ", Failed to read file, Status:" << status;
			}
			else
			{
				ImportNewIVREvents(pAVmsgServiceList, pStaticAVmsgList);
			}
			POBJDELETE(pStaticAVmsgList);
		}
	}

	// VNGR-24993,Originally, this function is called in  FillIvrListFromXmlFile()
	// I move it after ImportNewIVREvents() to avoid create a default NULL event for the new added one in constructor
	// Update DTMF and general messages tables for every service in the list
	UpdateTables(pAVmsgServiceList);

	// Add the IVR default services from the temporary list to the IVR list (one by one)
	while (NULL != pAVmsgService)
	{
		status = pAVmsgServiceList->AddOnlyMem(*pAVmsgService);
		if (status != STATUS_OK)
		{
			TRACEINTO << "ServiceName:" << pAVmsgService->GetName() << ", Failed to add to IVR Service list, Status:" << status;
		}
		else
		{
			TRACEINTO << "ServiceName:" << pAVmsgService->GetName() << ", Default IVR Service was added";
		}
		pAVmsgService = pTempAVmsgList->GetNextService(); // gets the next IVR Service from the temp list
	}

	// Delete the temporary default IVR list
	POBJDELETE(pTempAVmsgList);

	// Delete the IVR default service XML file from disk
	DeleteDefaultXMLFileFromDisk(file_name);

	// creates empty default list file as in every reset the default list will be copied if not exists
	pTempAVmsgList = new CAVmsgServiceList;

	status = pTempAVmsgList->WriteXmlFile(FILE_DEFAULT_IVR_CONFIG_XML, "IVRServiceList");
	if (STATUS_OK != status)
	{
		TRACEINTO << "FileName:" << FILE_IVR_CONFIG_XML_STATIC << ", Failed, Cannot  write empty file, Status:" << status;
	}

	// Delete the temporary default IVR list
	POBJDELETE(pTempAVmsgList);
}

//--------------------------------------------------------------------------
void CIVRManager::UpdateDefaultServices()
{
	// Set default services if not set (EQ & IVR)

	// gets the IVR list
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();

	// gets the first IVR Service
	CAVmsgService* pTempAVmsgService = pAVmsgServiceList->GetFirstService();
	if (!pTempAVmsgService)
	{
		TRACEINTO << "Failed, IVR Service list is empty";
	}
	else
	{
		if (0 == strcmp(pAVmsgServiceList->GetDefaultIVRName(), ""))
		{
			TRACEINTO << "Failed, No default IVR Service in list";
		}

		if (0 == strcmp(pAVmsgServiceList->GetDefaultEQName(), ""))
		{
			TRACEINTO << "Failed, No default EQ IVR Service in list";
		}
	}

	// loop on all services (if needed)
	while ((NULL != pTempAVmsgService) &&
	       ((0 == strcmp(pAVmsgServiceList->GetDefaultIVRName(), "")) ||
	        (0 == strcmp(pAVmsgServiceList->GetDefaultEQName(), ""))))
	{
		// get current service name
		const char* ivrServiceName = pTempAVmsgService->GetName();

		// Check if it is an IVR service (0) or an EQ service (1)
		DWORD isEQ = pAVmsgServiceList->GetIsEQService(ivrServiceName);

		// fills the EQ default service if needed
		if (isEQ && (0 == strcmp(pAVmsgServiceList->GetDefaultEQName(), "")))
		{
			pAVmsgServiceList->SetDefaultEQName(ivrServiceName);
			TRACEINTO << "ServiceName:" << ivrServiceName << ", Set default EQ IVR Service";
		}

		// fills the IVR for Conference default service if needed
		if (!isEQ && (0 == strcmp(pAVmsgServiceList->GetDefaultIVRName(), "")))
		{
			pAVmsgServiceList->SetDefaultIVRName(ivrServiceName);
			TRACEINTO << "ServiceName:" << ivrServiceName << ", Set default IVR Service";
		}

		// get the next service in the list
		pTempAVmsgService = pAVmsgServiceList->GetNextService();
	}
}

//--------------------------------------------------------------------------
void CIVRManager::SaveIvrServiceList()
{
	// gets the IVR list
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();

	// Update IVR service list in disk
	STATUS status = pAVmsgServiceList->WriteXmlFile(FILE_IVR_CONFIG_XML, "IVRServiceList");
	if (STATUS_OK != status)
	{
		TRACEINTO << "IVR-Error - FileName:" << FILE_IVR_CONFIG_XML << ", Failed, Cannot write file, Status:" << status;
	}
	else
	{
		TRACEINTO << "FileName:" << FILE_IVR_CONFIG_XML << ", IVR list saved to disk";
	}

	// Save a copy of IVRServiceList.xml in IVRServiceList_LastGoodCopy.xml
	BOOL bCopied = CopyFile(FILE_IVR_CONFIG_XML, FILE_IVR_CONFIG_XML_LAST_GOOD_COPY);
	if (!bCopied)
	{
		TRACEINTO << "IVR-Error - FileName:" << FILE_IVR_CONFIG_XML << ", Failed, Cannot copy file";
	}
}

//--------------------------------------------------------------------------
void CIVRManager::AddActiveAlarmForBadIVRList()
{
	CProcessBase* pProcess = CProcessBase::GetProcess();
	pProcess->AddActiveAlarmFromProcess(FAULT_GENERAL_SUBJECT,
	                                    AA_IVR_SERVICE_LIST_MISSING_DEFAULT_SERVICE,
	                                    MAJOR_ERROR_LEVEL,
	                                    "No default IVR Service in IVR Services list; Ensure that one conference IVR Service and one EQ IVR Service are set as default",
	                                    true,  // isForEma
	                                    true); // inForFaults
}

//--------------------------------------------------------------------------
void CIVRManager::AddFaultForBadIVRListFiles()
{
	BOOL isFullOnly = FALSE;
	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
	                    AA_IVR_SERVICE_LIST_MISSING_DEFAULT_SERVICE,
	                    SYSTEM_MESSAGE,
	                    "The IVR service list is missing a default service, trying to retrieve the system default services: RMX IVR Service and RMX EQ Service",
	                    isFullOnly);
}

//--------------------------------------------------------------------------
void CIVRManager::UpdateTables(CAVmsgServiceList* pAVmsgServiceList)
{
	CAVmsgService* pAVmsgService = pAVmsgServiceList->GetFirstService();

	while (NULL != pAVmsgService)
	{
		CIVRService* pIVRService = (CIVRService*)pAVmsgService->GetIVRService(); // Try to remove casting
		if (pIVRService)
		{
			pIVRService->ChangeTable();            // Update DTMF table
			pIVRService->ChangeGeneralMsgTable();  // Update general messages table
		}
		else
		{
			PASSERT(1);
		}
		pAVmsgService = pAVmsgServiceList->GetNextService();
	}
}

// Compare the IVR events between pSourceServiceList and pDestServiceList
// Then copy the new IVR msg events from pSourceServiceList to pDestServiceList
//--------------------------------------------------------------------------
void CIVRManager::ImportNewIVREvents(CAVmsgServiceList* pDestServiceList, CAVmsgServiceList* pSourceServiceList)
{
	std::ostringstream msg;

	int numberOfImportedIVREvents = 0;
	CAVmsgService* pSourceAVmsgService = pSourceServiceList->GetFirstService();
	while (NULL != pSourceAVmsgService)
	{
		int destIndex = pDestServiceList->FindAVmsgServ(*pSourceAVmsgService);
		if (NOT_FIND != destIndex)
		{
			// Currently we only  add new IVR Event in general msg feature
			CIVRGeneralMsgsFeature* pDestGeneralMsg   = (CIVRGeneralMsgsFeature*)((CIVRService*)(pDestServiceList->m_pAVmsgService[destIndex]->GetIVRService())->GetGeneralMsgsFeature());
			CIVRGeneralMsgsFeature* pSourceGeneralMsg = (CIVRGeneralMsgsFeature*)((CIVRService*)(pSourceAVmsgService->GetIVRService())->GetGeneralMsgsFeature());

			CIVREvent* pSourceEvent = pSourceGeneralMsg->GetFirstEvent();
			while (NULL != pSourceEvent)
			{
				if (STATUS_OK == pDestGeneralMsg->AddEvent(*pSourceEvent))
				{
					if (numberOfImportedIVREvents)
						msg <<  ", " << pSourceEvent->GetEventOpcode();
					else
						msg << pSourceEvent->GetEventOpcode();
					numberOfImportedIVREvents++;
				}
				pSourceEvent = pSourceGeneralMsg->GetNextEvent();
			}
		}
		pSourceAVmsgService = pSourceServiceList->GetNextService();
	}
	if (numberOfImportedIVREvents)
		TRACEINTO << "Imported " << numberOfImportedIVREvents << " IVR events with following opcodes (" << msg.str().c_str() << ")";
}

//--------------------------------------------------------------------------
void CIVRManager::DeleteDefaultXMLFileFromDisk(const char* file_name)
{
	// Delete the default IVR XML File
	string defaultFileName = FILE_DEFAULT_IVR_CONFIG_XML;
	if (IsFileExists(defaultFileName))
		DeleteFile(defaultFileName);
}

//--------------------------------------------------------------------------
void CIVRManager::TestCreateXmlFile()
{
	// Create IVR services
	CAVmsgService* pIvr1 = new CAVmsgService;
	pIvr1->SetName("IVR1");

	// Create a temporary service list
	CAVmsgServiceList* pTempAVmsgList = new CAVmsgServiceList;
	pTempAVmsgList->SetDefaultIVRName("IVR1");
	pTempAVmsgList->SetDefaultEQName("EQ1");

	// Add the IVR service to the list
	int status = pTempAVmsgList->AddOnlyMem(*pIvr1);

	// Write the list to XML file in disk
	pTempAVmsgList->WriteXmlFile("Cfg/IVR/IVRCfg/IVRDefaultServiceList.xml", "IVRDefaultServiceList");

	POBJDELETE(pIvr1);
	POBJDELETE(pTempAVmsgList);
}

//--------------------------------------------------------------------------
void CIVRManager::TestSaveListToDisk()
{
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
	pAVmsgServiceList->WriteXmlFile("Cfg/TestSaveListToDisk.xml", "IVRServiceList");
}

//--------------------------------------------------------------------------
void CIVRManager::TestCreateXmlFileOfMultipleServices()
{
	int status = STATUS_OK;

	// Create a sample of a full IVR service
	CAVmsgService* pIvr1 = new CAVmsgService;
	pIvr1->SetName("IVR1");
	const CIVRService* pIVRService = pIvr1->GetIVRService();

	pIvr1->m_pIVRService->m_pIVRLanguage[0] = new CIVRLanguage;
	pIvr1->m_pIVRService->m_pIVRLanguage[0]->SetLanguagePos(1);
	pIvr1->m_pIVRService->m_pIVRLanguage[0]->SetLanguageName("English");
	pIvr1->m_pIVRService->m_numb_of_language++;

	CIVRMessage pIvrMsg;
	pIvrMsg.SetMsgFileName("PolycomMsg.wav");
	pIvrMsg.SetMsgDuration(5);

	// Add the IVR messages to the service
	const CIVRLangMenuFeature* pIVRLangMenuFeature = pIVRService->GetLangMenuFeature();

	CIVREvent* pIVREvent = pIVRLangMenuFeature->GetCurrentIVREvent(IVR_EVENT_GET_LANGUAGE);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRLangMenuFeature->GetCurrentIVREvent(IVR_EVENT_LANGUAGE_RETRY);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	const CIVRWelcomeFeature* pIVRWelcomeFeature = pIVRService->GetWelcomeFeature();

	pIVREvent = pIVRWelcomeFeature->GetCurrentIVREvent(IVR_EVENT_WELCOME_MSG);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRWelcomeFeature->GetCurrentIVREvent(IVR_EVENT_ENTRANCE_MSG);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	const CIVRConfPasswordFeature* pIVRConfPasswordFeature = pIVRService->GetConfPasswordFeature();

	pIVREvent = pIVRConfPasswordFeature->GetCurrentIVREvent(IVR_EVENT_GET_CONFERENCE_PASSWORD);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRConfPasswordFeature->GetCurrentIVREvent(IVR_EVENT_CONFERENCE_PASSWORD_RETRY);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRConfPasswordFeature->GetCurrentIVREvent(IVR_EVENT_GET_DIGIT);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	const CIVRConfLeaderFeature* pIVRConfLeaderFeature = pIVRService->GetConfLeaderFeature();

	pIVREvent = pIVRConfLeaderFeature->GetCurrentIVREvent(IVR_EVENT_GET_LEADER_IDENTIFIER);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRConfLeaderFeature->GetCurrentIVREvent(IVR_EVENT_GET_LEADER_PASSWORD);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRConfLeaderFeature->GetCurrentIVREvent(IVR_EVENT_LEADER_PASSWORD_RETRY);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	const CIVROperAssistanceFeature* pIVROperAssistanceFeature = pIVRService->GetOperAssistanceFeature();

	pIVREvent = pIVROperAssistanceFeature->GetCurrentIVREvent(IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVROperAssistanceFeature->GetCurrentIVREvent(IVR_EVENT_SYSTEM_DISCONNECT_MESSAGE);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	// General Messages Feature
	int i = 0;

	const CIVRGeneralMsgsFeature* pIVRGeneralMsgsFeature = pIVRService->GetGeneralMsgsFeature();
	for (i = 101; i <= 104; i++)
	{
		pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(i);
		if (pIVREvent)
			pIVREvent->AddMessage(pIvrMsg);
	}

	for (i = 122; i <= 124; i++)
	{
		pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(i);
		if (pIVREvent)
			pIVREvent->AddMessage(pIvrMsg);
	}

	pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(127);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(130);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(131);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(133);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	for (i = 135; i <= 138; i++)
	{
		pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(i);
		if (pIVREvent)
			pIVREvent->AddMessage(pIvrMsg);
	}

	pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(145);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	for (i = 147; i <= 159; i++)
	{
		pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(i);
		if (pIVREvent)
			pIVREvent->AddMessage(pIvrMsg);
	}

	pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(167);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(177);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVREvent = pIVRGeneralMsgsFeature->GetCurrentIVREvent(178);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	// End of General Messages Feature

	const CIVR_BillingCodeFeature* pIVR_BillingCodeFeature = pIVRService->GetBillingCodeFeature();
	pIVREvent = pIVR_BillingCodeFeature->GetCurrentIVREvent(IVR_EVENT_BILLING_NUM);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVR_BillingCodeFeature = pIVRService->GetBillingCodeFeature();

	pIVREvent = pIVR_BillingCodeFeature->GetCurrentIVREvent(IVR_EVENT_CONFERENCE_PASSWORD_RETRY);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	const CIVRRollCallFeature* pIVRRollCallFeature = NULL;
	for (i = 160; i <= 166; i++)
	{
		pIVRRollCallFeature = pIVRService->GetRollCallFeature();
		pIVREvent = pIVRRollCallFeature->GetCurrentIVREvent(i);
		if (pIVREvent)
			pIVREvent->AddMessage(pIvrMsg);
	}

	const CIVRNumericConferenceIdFeature* pIVRNumericConferenceIdFeature = pIVRService->GetNumericConferenceIdFeature();

	pIVREvent = pIVRNumericConferenceIdFeature->GetCurrentIVREvent(IVR_EVENT_GET_NUMERIC_ID);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	pIVRNumericConferenceIdFeature = pIVRService->GetNumericConferenceIdFeature();
	pIVREvent = pIVRNumericConferenceIdFeature->GetCurrentIVREvent(IVR_EVENT_NUMERIC_ID_RETRY);
	if (pIVREvent)
		pIVREvent->AddMessage(pIvrMsg);

	const CIVRMuteNoisyLineFeature* pIVRMuteNoisyLineFeature = NULL;
	for (i = 170; i <= 176; i++)
	{
		pIVRMuteNoisyLineFeature = pIVRService->GetMuteNoisyLineFeature();
		pIVREvent = pIVRMuteNoisyLineFeature->GetCurrentIVREvent(i);
		if (pIVREvent)
			pIVREvent->AddMessage(pIvrMsg);
	}

	const CIVRRecordingFeature* pIVRRecordingFeature = NULL;
	for (i = 180; i <= 187; i++)
	{
		pIVRRecordingFeature = pIVRService->GetRecordingFeature();
		pIVREvent = pIVRRecordingFeature->GetCurrentIVREvent(i);
		if (pIVREvent)
			pIVREvent->AddMessage(pIvrMsg);
	}

	const CIVRPlaybackFeature* pIVRPlaybackFeature = NULL;

	for (i = 190; i <= 193; i++)
	{
		pIVRPlaybackFeature = pIVRService->GetPlaybackFeature();
		pIVREvent = pIVRPlaybackFeature->GetCurrentIVREvent(i);
		if (pIVREvent)
			pIVREvent->AddMessage(pIvrMsg);
	}

	// Create a temporary service list
	CAVmsgServiceList* pTempAVmsgList = new CAVmsgServiceList;
	pTempAVmsgList->SetDefaultIVRName("IVR1");
	pTempAVmsgList->SetDefaultEQName("EQ1");

	// Add IVR services to the list
	status = pTempAVmsgList->AddOnlyMem(*pIvr1);

	char name[3];
	for (i = 2; i <= 2; i++)
	{
		name[0] = '0' + i;
		name[1] = '\0';

		pIvr1->SetName(name);
		status = pTempAVmsgList->AddOnlyMem(*pIvr1);
	}

	// Write the list to XML file in disk
	pTempAVmsgList->WriteXmlFile("Cfg/IVR/IVRCfg/IVRServiceList.xml", "IVRServiceList");

	POBJDELETE(pIvr1);
	POBJDELETE(pTempAVmsgList);
}

//--------------------------------------------------------------------------
void CIVRManager::GetMusicSources()
{
	TRACEINTO;

	// checking music file format and length
	CIVRFeature temp;
	WORD ivrMsgDuration = 0;
	WORD ivrMsgCheckSum = 0;

	int status = temp.CheckLegalFileAndGetParams(IVR_FOLDER_MUSIC_TMP_FILE, &ivrMsgDuration, &ivrMsgCheckSum);
	if (status != STATUS_OK)
	{
		TRACEINTO << "Failed, Illegal music file format, will try to get is from StaticCfg. Status:" << status;

		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
							AA_NO_MUSIC_SOURCE,
		                    SYSTEM_MESSAGE,
		                    "The IVR music file is missing or invalid, trying the system default music file.",
		                    FALSE);


		std::string originalIVRMusicFile;
		if (IsTarget())
		{
			originalIVRMusicFile = "StaticCfg/" +string(IVR_FOLDER_MUSIC_TMP_FILE_RELATIVE_PATH);

		}
		else
		{
			originalIVRMusicFile = "VersionCfg/" + string(IVR_FOLDER_MUSIC_TMP_FILE_RELATIVE_PATH);

		}

		if (CopyFile(originalIVRMusicFile, (string)IVR_FOLDER_MUSIC_TMP_FILE))
		{
			status = temp.CheckLegalFileAndGetParams(IVR_FOLDER_MUSIC_TMP_FILE, &ivrMsgDuration, &ivrMsgCheckSum);

		}
		else
		{
			status = STATUS_FAIL;
			TRACEINTO << "Failed to copy IVR file from " <<  originalIVRMusicFile << " to " << IVR_FOLDER_MUSIC_TMP_FILE;
		}


		if (status != STATUS_OK)
		{
			TRACEINTO << "Failed, Illegal music file format after taking from " << originalIVRMusicFile << ". Status:" << status;
			PASSERT(1);
			return;
		}
	}

	if (MAX_IVR_MUSIC_MSG_DURATION < ivrMsgDuration)
	{
		TRACEINTO << "Failed, Illegal music file duration, Duration:" << ivrMsgDuration;
		PASSERT(1);
		return;
	}

	// currently, for the first version, we have one Music Source, and it defined in system.cfg
	m_musicSourceList[0] = new SIVRAddMusicSource;
	m_musicSourceNum     = 1;

	// zeroing Music Sources List
	memset(m_musicSourceList[0], 0, sizeof(SIVRAddMusicSource));

	// fills source struct
	m_musicSourceList[0]->sourceID      = START_MUSIC_SOURCE_ID;                  // in the future we should handle this ID by resource allocator
	m_musicSourceList[0]->mediaFilesNum = 1;                                      // hard coded for the first version
	// get from system.cfg file
	strcpy(m_musicSourceList[0]->mediaFiles[0], IVR_FOLDER_MUSIC_TMP_FILE_EMB);   // for the IC we send without "Cfg" at the start of the path
}

//--------------------------------------------------------------------------
void CIVRManager::AddAllMusicSources()
{
	// sends ADD_MUSIC_SOURCE to Cards

	// in the future, the Music Sources will be in a list which initialize on startup, the
	// same way as IVR Service list

	for (int i = 0; i < m_musicSourceNum; i++)
	{
		if (m_musicSourceList[i])
		{
			TRACEINTO << "Sends Music Source to Cards";

			// fill segment
			CSegment* pSeg = new CSegment;
			pSeg->Put((BYTE*)(m_musicSourceList[i]), sizeof(SIVRAddMusicSource));

			// gets the Cards mailbox
			const COsQueue* cardsManagerQueue = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);
			if (!cardsManagerQueue)
			{
				TRACEINTO << "Error getting Process queue, no music in MCU";
				PASSERT(1);
			}
			else
			{
				// sends the command to Cards process
				STATUS res = cardsManagerQueue->Send(pSeg, IVR_MUSIC_ADD_SOURCE_REQ);
				if (STATUS_OK != res)
					TRACEINTO << "Failed to send message to Cards process, Status:" << res;
			}
		}
	}
}

//--------------------------------------------------------------------------
void CIVRManager::FillSlidesList()
{
	TRACEINTO;

	// gets the slides list to fill
	CIVRSlidesList* pSlidesList = ::GetpSlidesList();
	PASSERT_AND_RETURN(!pSlidesList);

	// pass over the IVR services and gets the slides name.
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
	PASSERT_AND_RETURN(!pAVmsgServiceList);

	// loop on all IVR services to get the used slides
	CAVmsgService* pAVmsgService = pAVmsgServiceList->GetFirstService();
	while (NULL != pAVmsgService)
	{
		const CIVRService* pIVRService = pAVmsgService->GetIVRService();
		if (NULL != pIVRService)
		{
			const CIVRVideoFeature* video = pIVRService->GetVideoFeature();
			if (NULL != video)
			{
				const char* baseName = pAVmsgService->GetVideoFileName();
				if (0 != strlen(baseName))
				{
					// this slide should be in the slides list
					char tmpName[NEW_FILE_NAME_LEN];
					memset(tmpName,0,NEW_FILE_NAME_LEN);
					strcpy_safe(tmpName, baseName);

					char* pPeriod = strchr(tmpName, '.');
					if (pPeriod)             // there is '.' in the name
					{
						if (0 == strcmp(pPeriod, "acv"))
							pPeriod[0] = '\0';   // cut the extension of the name
					}
					pSlidesList->AddSlide(tmpName);
				}
				else
				{
					TRACEINTO << "ServiceName:" << pAVmsgService->GetName() << ", Failed, slide name is empty";
				}
			}
			else
			{
				TRACEINTO << "ServiceName:" << pAVmsgService->GetName() << ", Failed, slide feature is empty";
			}
		}
		else
		{
			TRACEINTO << "ServiceName:" << pAVmsgService->GetName() << ", Failed, invalid pIVRService";
		}

		pAVmsgService = pAVmsgServiceList->GetNextService();
	} // while
}

//--------------------------------------------------------------------------
STATUS CIVRManager::CreateIVRFolders(const char* szMainFolder)
{
	std::string base;
	base.reserve(256);

	base = szMainFolder;

	const char* folders[] = {
		IVR_FOLDER_SLIDES,
		IVR_FOLDER_MUSIC,
		IVR_FOLDER_MSG,
		IVR_FOLDER_TONES,
		IVR_FOLDER_CFG,
	};

	for (size_t i = 0; i < sizeof(folders)/sizeof(folders[0]); ++i)
		CreateDirectory((base + folders[i]).c_str(), 0755); // create folders if needed: 'rwx r-x r--'

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CIVRManager::CleanRollCallFiles(WORD actionType)
{
	// delete all files in RollCall folder (at startup time)

	string rollCallPath = IVR_FOLDER_ROLLCALL;

	TRACEINTO << "RollCallFolder:" << rollCallPath.c_str();

	vector <FDStruct>           dirFiles;
	vector <FDStruct>::iterator dirIterator;

	// gets all folder files
	BOOL bResult = GetDirectoryContents(rollCallPath.c_str(), dirFiles);
	if (bResult)                                      // return TRUE on success
	{
		if (0 == dirFiles.size())
			return;                                       // empty folder

		// get first file
		FDStruct* file = NULL;
		file = ::GetFirstFile(dirIterator, dirFiles);   // allocate struct inside, need to free

		// loop on all files in folder
		while (file != NULL)
		{
			if (file->type != 'D')    // if not folder
			{
				string      fullPathName;
				const char* rc_name = file->name.c_str();
				if (0 == strncmp("RC_", rc_name, 3))
				{
					fullPathName  = rollCallPath;
					fullPathName += file->name;
					if (ACTION_TYPE_DELETE_UPON_STARTUP == actionType)                // delete all RollCall filesfile
						DeleteFile(fullPathName);
					else if (ACTION_TYPE_DELETE_UPON_TIMER == actionType)             // delete RollCall filesfile that "older" than 5 days
					{
						time_t fileMsgLastModified = GetLastModified(fullPathName);     // gets file last update time
						if (((time_t)-1) != fileMsgLastModified)
						{
							// gets current time
							time_t currentTime = time(NULL);                              // gets current time in seconds since ...1970
							// time_t currentTime = GetCurrentTime();	// gets file last update time
							if (((time_t)-1) != currentTime)
							{
								DWORD seconds        = currentTime - fileMsgLastModified;
								DWORD seconds_in_day = 3600*24;
								DWORD numOfDays      = seconds / seconds_in_day;
								// get number of days since last update
								if (numOfDays >= 5)
									DeleteFile(fullPathName);
							}
						}
					}
				}
			}

			// Clear the struct file before the next setting
			file->name.clear();                             // string must be cleared before setting
			PDELETE(file);                                  // free and zeroing

			// get the next file
			file = ::GetNextFile(dirIterator, dirFiles);    // allocate inside, need to free
		}
	}
}

//--------------------------------------------------------------------------
void CIVRManager::DeleteXMLFileFromDisk(const char* file_name)
{
	DeleteFile(file_name);
}

//--------------------------------------------------------------------------
void CIVRManager::DeleteIVRServices()
{
	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
	if (pAVmsgServiceList != NULL)
		pAVmsgServiceList->DeleteServices();

	CIVRSlidesList* pSlidesList = ::GetpSlidesList();
	if (pSlidesList != NULL)
	{
		delete pSlidesList;
		::SetpSlidesList(NULL);
	}

	DeleteXMLFileFromDisk(FILE_IVR_CONFIG_XML_LAST_GOOD_COPY);
	DeleteXMLFileFromDisk(FILE_IVR_CONFIG_XML);
	// Fix the VNGR-21108 by Hui Yu.
	// DeleteXMLFileFromDisk(FILE_IVR_CONFIG_XML_STATIC);
	DeleteXMLFileFromDisk(FILE_DEFAULT_IVR_CONFIG_XML);
}

