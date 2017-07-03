// ApacheModuleManager.cpp: implementation of the CApacheModuleManager class.
//
//////////////////////////////////////////////////////////////////////

#include <libgen.h>


#include "ApacheModuleManager.h"
#include "DummyEntry.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"
#include "XmlMiniParser.h"
#include "LogInConfirm.h"
#include "Request.h"
#include "ApacheModuleEngine.h"
#include "psosxml.h"
#include "LogInRequest.h"
#include "ApiStatuses.h"
#include "CreateRemoveDir.h"
#include "Rename.h"
#include "FaultsDefines.h"
#include "TraceStream.h"
#include "Versions.h"
#include "TerminalCommand.h"
#include "ObjString.h"
#include "PostXmlHeader.h"
#include "TaskApi.h"
#include "scoreboard.h"
#include "AuditorApi.h"
#include "McmsAuthentication.h"
#include "FipsMode.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "OsFileIF.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "LogOutRequest.h"
#include "RvgwAliasName.h"
#include "GateWaySecurityToken.h"
#include "HlogApi.h"

#define GARBAGE_COLLECTOR_TIMER		3210
#define GARBAGE_COLLECTOR_TIMEOUT	12000	//every 2 minutes. 2000=20 second
#define LOOSE_REVOCATION_TIMEOUT	2000
#define INIT_LICENSING_TIMER		3211

extern void ApacheModuleMonitorEntryPoint(void* appParam);
extern char* ProductTypeToString(APIU32 productType);
extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);


////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CApacheModuleManager)
  ONCOMMAND("con_ls",    CApacheModuleManager::HandleTerminalConnectionsList,   "Check how many open connections exist")
  ONCOMMAND("show_trans_map",    CApacheModuleManager::HandleTerminalShowTransMap,   "Show action redirection map")
  ONCOMMAND("show_virtual_dir_map",    CApacheModuleManager::HandleTerminalShowVirtualDirMap,   "Show virtual directories  map")

END_TERMINAL_COMMANDS

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CApacheModuleManager)
  ONEVENT(XML_REQUEST    ,IDLE    ,  CApacheModuleManager::HandlePostRequest )
  ONEVENT(GARBAGE_COLLECTOR_TIMER  ,ANYCASE , CApacheModuleManager::ConnectionGarbageCollector)
  ONEVENT(MCUMNGR_AUTHENTICATION_STRUCT_REQ, ANYCASE, CApacheModuleManager::OnMcuMngrAuthenticationStruct )
  ONEVENT(MCUMNGR_TO_APACHE_MULTIPLE_SERVICES_IND, ANYCASE, CApacheModuleManager::OnMultipleServicesInd)
  ONEVENT(MCUMNGR_TO_APACHEMODULE_LICENSING_IND, ANYCASE, CApacheModuleManager::OnMcuMngrApacheModuleLicensingInd)
  ONEVENT(APACHE_SECURITY_PKI_LOGIN_STATUS_IND, ANYCASE, CApacheModuleManager::OnHttpdSeucreLoginInd)
  ONEVENT(INIT_LICENSING_TIMER  ,ANYCASE , CApacheModuleManager::OnLicensingInitialisationTimeout)
  ONEVENT(APACHE_BLOCK_REQUESTS  ,ANYCASE , CApacheModuleManager::OnBlockRequests)
PEND_MESSAGE_MAP(CApacheModuleManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CApacheModuleManager)
	ON_TRANS("TRANS_MCU","LOGIN",CLogInRequest,CApacheModuleManager::HandleOperLogin)
	ON_TRANS("TRANS_MCU","LOGOUT",CLogOutRequest,CApacheModuleManager::HandleOperLogout)
	ON_TRANS("TRANS_MCU","CREATE_DIRECTORY",CCreateRemoveDir,CApacheModuleManager::HandleCreateDirectory)
	ON_TRANS("TRANS_MCU","REMOVE_DIRECTORY",CCreateRemoveDir,CApacheModuleManager::HandleRemoveDirectory)
	ON_TRANS("TRANS_MCU","REMOVE_DIRECTORY_CONTENT",CCreateRemoveDir,CApacheModuleManager::HandleRemoveDirectoryContent)
	ON_TRANS("TRANS_MCU","RENAME",CRename,CApacheModuleManager::HandleRename)
	ON_TRANS("TRANS_MCU","SET_GATEWAY_ALIAS",CRvgwAliasName,CApacheModuleManager::HandleSetRvgwAlias)
	ON_TRANS("TRANS_MCU","CREATE_GATEWAY_SECURITY_TOKEN",CGateWaySecurityTokenRequest,CApacheModuleManager::HandleCreateSecurityToken)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void ApacheModuleManagerEntryPoint(void* appParam)
{
	CApacheModuleManager * pApacheModuleManager = new CApacheModuleManager;
	pApacheModuleManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CApacheModuleManager::GetMonitorEntryPoint()
{
	return ApacheModuleMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CApacheModuleManager::CApacheModuleManager()
{
//	m_pMbxRcv = new COsQueue;
//	m_pMbxSend = new COsQueue;

	char szUniqueName[MAX_QUEUE_NAME_LEN];
	COsQueue::CreateUniqueQueueName(szUniqueName);

	m_bSystemMultipleServices = TRUE;
	m_bV35JitcSupport = FALSE;

//	m_pMbxRcv->CreateRead(eProcessApacheModule,szUniqueName);
//	m_pMbxSend->CreateWrite(eProcessApacheModule,szUniqueName);

	m_pszClientCertificateSubj = NULL;

	ResetAdditionalParams();
}

CApacheModuleManager::~CApacheModuleManager()
{
//	if(m_pMbxRcv)
//	{
//		m_pMbxRcv->Delete();
//		delete m_pMbxRcv;
//	}
//
//	if(m_pMbxSend)
//	{
//		m_pMbxSend->Delete();
//		delete m_pMbxSend;
//	}
}



/////////////////////////////////////////////////////////////////////////////
void CApacheModuleManager::SelfKill()
{
	CManagerTask::SelfKill();
}

//////////////////////////////////////////////////////////////////////
//void CApacheModuleManager::InitTransactionsFactory()
//{
//	CManagerTask::InitTransactionsFactory();
////
////	m_requestTransactionsFactory.AddTransaction
////		("TRANS_MCU","LOGIN", new COperCfg,(HANDLE_REQUEST)HandleOperLogin);
//		//("TRANS_MCU","LOGIN", new CDummyEntry,(HANDLE_SET_REQUEST)HandleOperLogin);
//
//	m_requestTransactionsFactory.AddTransaction
//		("TRANS_MCU","LOGIN", new CLogInRequest, (HANDLE_REQUEST)HandleOperLogin);
//}

//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::SendAuthenticationStructAndLicensingReqToMcuMngr()
{
	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::SendAuthenticationStructAndLicensingReqToMcuMngr";
	CSegment*  pRetParam = new CSegment;

	const COsQueue* pMcuMngrMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr, eManager);

	STATUS res = pMcuMngrMbx->Send(pRetParam, APACHE_MODULE_AUTHENTICATION_STRUCT_AND_LICENSING_REQ);
}

//////////////////////////////////////////////////////////////////////
STATUS CApacheModuleManager::HandleOperLogin(CRequest *pRequest)
{

	STATUS nRetStatus = STATUS_OK;
	CLogInConfirm *pLogInConfirm = NULL;
	CConnectionList* pConnList;
	int nLoginStatus = STATUS_PROCESS_FAILURE;

	char* pszRequestXmlString;

	BYTE bJitcMode = CApacheModuleProcess::IsFederalOn();

	CMessageHeader Header;

	pRequest->DumpSetRequestAsString(&pszRequestXmlString);
	OnHttpdSeucreLoginInd(NULL);
	if (!bJitcMode)
	{
		char *pszStartActionName=NULL;
		CSegmentFuncForRequestHandler::PrintRequestInTrace(eManager, pszStartActionName, pszRequestXmlString);
	}

/*	CSegment *pSegment = new CSegment;

	GetRspMbx()->Serialize(*pSegment);

	STATUS nStatus = CApacheModuleEngine::PrepareSegmentBeforeSend(false, strlen(pszRequestXmlString), pszRequestXmlString,
											 pSegment, 0, "",
											 eProcessAuthentication, eManager, 0);

 	CSegment LoginRespSegment;
    OPCODE resOpcode;

    CTaskApi AuthenticationManagerApi(eProcessAuthentication, eManager);
	nRetStatus = AuthenticationManagerApi.SendMessageSync(pSegment, XML_REQUEST, REQUEST_TIMEOUT_LONG, resOpcode, LoginRespSegment);*/

    ConnectionDetails_S connDetails;
    const CPostXmlHeader & currentMsgHdr = GetCurrentMsgHdr();
    connDetails.workStation = currentMsgHdr.GetWorkStation();
    //connDetails.userName = currentMsgHdr.GetUserName();
    connDetails.clientIp = currentMsgHdr.GetClientIp();
    m_dwConnId = currentMsgHdr.GetConnId();
    CStringToProcessEntry destProcessEntry(eProcessAuthentication, false, "", "", "");
	CLogInRequest* pLogInRequest = (CLogInRequest*)pRequest->GetRequestObject();
    UpdateUserName(pLogInRequest->GetLoginName());
    connDetails.userName = currentMsgHdr.GetUserName();
//	const COperator* pOperator = pLogInRequest->GetOperator();


    pConnList = CApacheModuleEngine::GetConnectionList();
	DWORD nConnectionNum = (DWORD)pConnList->GetCount();
	DWORD nNumConnectionsPerUser = (DWORD)pConnList->GetNumConnectionByLoginName(pLogInRequest->GetLoginName().c_str());

	if (!bJitcMode)
	{
		FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleOperLogin. cur of logins per sys = " << nConnectionNum;
		FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleOperLogin. cur of logins per user = " << pLogInRequest->GetLoginName().c_str() << " is "<< nNumConnectionsPerUser;
	}

	int connId = 0;

	DWORD dwNumConnectionPerSystem;
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetDWORDDataByKey(MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM, dwNumConnectionPerSystem);

	DWORD dwNumConnectionPerUser;
	sysConfig->GetDWORDDataByKey(MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER, dwNumConnectionPerUser);

	//dwNumConnectionPerSystem = 2;
	//dwNumConnectionPerUser =1;
	BYTE bSessionLimit = FALSE;

    FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleOperLogin. num of max logins per sys = " << dwNumConnectionPerSystem;
    FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleOperLogin. num of max logins per user = " << dwNumConnectionPerUser;
    if ( (nConnectionNum > (dwNumConnectionPerSystem -1)) || (nNumConnectionsPerUser > (dwNumConnectionPerUser -1)) )
    {
    	nRetStatus = STATUS_FAIL;
    	bSessionLimit = TRUE;

    	bSessionLimit = TRUE;

    	if (nConnectionNum > (dwNumConnectionPerSystem -1))
    	{
    		FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleOperLogin: STATUS_SYSTEM_SESSION_LIMIT_EXCEEDED";
	    	nLoginStatus = STATUS_SYSTEM_SESSION_LIMIT_EXCEEDED;
    	}
	    else
	    {
	    	FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleOperLogin: STATUS_USER_SESSION_LIMIT_EXCEEDED";
	    	nLoginStatus = STATUS_USER_SESSION_LIMIT_EXCEEDED;
	    }

    	// VNGR-10944
    	// Don't show the XML, it contains the password in clear-text.
    	SendEventToAuditor(connDetails,"");
    }
    else
    {
    	nRetStatus = CApacheModuleEngine::SendSyncMessage((COsQueue*)GetRspMbx(),(COsQueue*)GetRspMbxRead(),
    													  pszRequestXmlString,Header,
    													  destProcessEntry,eManager,false,"",
    													  strlen(pszRequestXmlString),
    													  0,0,connDetails,m_pszClientCertificateSubj);
		FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleOperLogin - send request to authentication. return status = "<<nRetStatus;
    }


	if(nRetStatus == STATUS_OK)
	{
		DWORD dwLoginRespLen;

		pLogInConfirm = new CLogInConfirm(m_bSystemMultipleServices, m_bV35JitcSupport);

		pRequest->SetConfirmObject(pLogInConfirm);
		CSegment *pLoginRespSegment = Header.m_segment;//back - jud

        CPostXmlHeader postXmlHeader;
        //postXmlHeader.DeSerialize(LoginRespSegment);
        postXmlHeader.DeSerialize(*pLoginRespSegment);	//back

        dwLoginRespLen = postXmlHeader.GetLen();

		ALLOCBUFFER(pszLoginResponse,dwLoginRespLen+1);

		char* pszBeginLoginResponse = pszLoginResponse;
		//LoginRespSegment.Get((BYTE*)pszLoginResponse,dwLoginRespLen);
		pLoginRespSegment->Get((BYTE*)pszLoginResponse,dwLoginRespLen);	//back
		pszLoginResponse[dwLoginRespLen] = '\0';

		nLoginStatus = CXmlMiniParser::GetResponseStatus(pszLoginResponse);

		if(nLoginStatus == STATUS_OK)
		{
			char szErrorMsg[ERROR_MESSAGE_LEN];

            CXMLDOMDocument xmlDoc;
            HRES parseStatus = xmlDoc.Parse((const char**)&pszLoginResponse);
			if(SEC_OK != parseStatus)
            {
                FTRACEINTO << "CApacheModuleManager::HandleOperLogin: CXMLDOMDocument::Parse has failed";
                return STATUS_FAIL;
            }

            CXMLDOMElement *pLoginResponseNode = xmlDoc.GetRootElement();
            CXMLDOMElement *pActionNode = NULL;

			pLoginResponseNode->getChildNodeByName(&pActionNode,"ACTION");
			if (pActionNode)
			{
				pActionNode->getChildNodeByName(&pActionNode,"LOGIN");
			}
			else
			{
				pLoginResponseNode->getChildNodeByName(&pActionNode,"LOGIN");
			}

			pLogInConfirm->DeSerializeXml(pActionNode,szErrorMsg);

//			const COperator* pOperator = pLogInRequest->GetOperator();
            CStructTm *pLoginGmtTime = NULL;
//            const CPostXmlHeader &currentMsgHdr = GetCurrentMsgHdr();

			/*CConnection connection(pLogInRequest->GetLoginName().c_str(),
                                   pLogInRequest->GetStationName().c_str(),
                                   connDetails.clientIp.c_str(),
                                   pLogInConfirm->GetAuthorization(),
                                   pLoginGmtTime,
                                   pLogInRequest->GetCompressionLevel(),
                                   m_bSHMRequest,
                                   pLogInConfirm->IsMachineAccount());*/

            pConnList = CApacheModuleEngine::GetConnectionList();

           // m_dwConnId = pRequest->GetConnectId();
            CConnection* pconn = pConnList->GetConnectionPtr(m_dwConnId);
            if(pconn)
            {
            	pconn->SetLogin(pLogInRequest->GetLoginName().c_str());
            	pconn->SetStationName(pLogInRequest->GetStationName().c_str());
            	pconn->SetAuthorization(pLogInConfirm->GetAuthorization());
            	pconn->SetSupportCompression(pLogInRequest->GetCompressionLevel());
            	pconn->SetEntity(m_bSHMRequest);
            	pconn->SetMachineAccount(pLogInConfirm->IsMachineAccount());
            }

            if (bJitcMode)
            	FTRACEINTO << "CApacheModuleManager::HandleOperLogin: Connection added. ConnId: " << m_dwConnId << "; Authorization: " << pConnList->GetAuthorization(m_dwConnId);
            else
            	FTRACEINTO << "CApacheModuleManager::HandleOperLogin: User Connection added. ConnId: " << m_dwConnId << "; Authorization: " << pConnList->GetAuthorization(m_dwConnId);

			CVersions version;
			std::string fname = MCU_MCMS_DIR+"/Versions.xml";
			version.ReadXmlFile(fname.c_str());

			pLogInConfirm->SetMCU_Version(version.GetMcuVersion());
			pLogInConfirm->SetMCMS_Version(version.GetMcmsVersion());
			pLogInConfirm->SetMcuPrivateDesc(version.GetMcuPrivateDescription());

			if(m_dwConnId)
			{
				pRequest->SetConnectId(m_dwConnId);
				pConnList = CApacheModuleEngine::GetConnectionList();
				pLogInConfirm->SetAuthorization(pConnList->GetAuthorization(m_dwConnId));
			}

			pLogInConfirm->SetApiNumber(API_NUMBER);

			// set product type
//			CApacheModuleProcess* pProcess = (CApacheModuleProcess*)CApacheModuleProcess::GetProcess();
//			if ( YES == pProcess->GetisAuthenticationStructureAlreadyReceived() )
//			{
//				pLogInConfirm->SetLoginConfirmProductType(pProcess->GetAuthenticationStruct()->productType);
//			}
//			else
//			{
//				pLogInConfirm->SetLoginConfirmProductType(7); // "rmx"
//			}
			eProductType curProductType =   CProcessBase::GetProcess()->GetProductType();

			pLogInConfirm->SetLoginConfirmProductType(curProductType);



			pLogInConfirm->SetHttpPort(m_dwPort);

			pLogInConfirm->SetOperatingSystem(OS_LINUX);

			//TBD set real login info flag
			pLogInConfirm->SetLoginInfoFlag(0);

			CApacheModuleProcess* pProcess = (CApacheModuleProcess*)CApacheModuleProcess::GetProcess();

			pLogInConfirm->SetRmxSystemCardsMode( (eSystemCardsMode)(pProcess->GetAuthenticationStruct()->rmxSystemCardsMode) );


			if (eProductTypeEdgeAxis == curProductType && pProcess->IsFlexeraLicenseInSysFlag() == true)
				pProcess->SetLicensingStructLicenseMode((int)eLicenseMode_flexera);

			else pProcess->SetLicensingStructLicenseMode((int)eLicenseMode_cfs);

			if (eProductTypeSoftMCUMfw == curProductType)
				pProcess->SetLicensingStructLicenseMode((int)eLicenseMode_none);



			 APACHEMODULE_LICENSING_S* pLicensingStruct = pProcess->GetLicensingStruct();

			TRACESTR(eLevelInfoNormal) << "\nCApacheModuleProcess::GetLicensingStruct curProductType"
					<<curProductType<<" isflexera "
					<<(int)pProcess->IsFlexeraLicenseInSysFlag()
					<<" pLicensingStruct->avcSvcCap.licenseMode " <<pLicensingStruct->avcSvcCap.licenseMode;



            pLogInConfirm->SetNumCopParties(pLicensingStruct->num_cop_parties);
            pLogInConfirm->SetNumCpParties(pLicensingStruct->num_cp_parties);
            pLogInConfirm->SetAvcSvcCap(pLicensingStruct->avcSvcCap);

            // no need to set RAM size; RAM size is being set at CLogInConfirm::CLogInConfirm()
            //pLogInConfirm->SetRmxSystemRamSize();
		}
		else
		{

			if (nLoginStatus==STATUS_LOGIN_INVALID && bJitcMode)//VNGR-16268 , only in JITC Mode the System will block such kind of DOS Attacks
			{
				PASSERTMSG(1,"CApacheModuleManager::HandleOperLogin: Sleep for 4 seconds");
				SystemSleep(400, FALSE); // 4 seconds
			}

		}

		DEALLOCBUFFER(pszBeginLoginResponse);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());

		//if we failed because there was no connection with the authentication process
		if (bSessionLimit == FALSE)
			PASSERTMSG(1,"CApacheModuleManager::HandleOperLogin: Failed communicating with Authentication process");
	}

	POBJDELETE(Header.m_segment);
	DEALLOCBUFFER(pszRequestXmlString);

	if (nLoginStatus == STATUS_OK)
		pRequest->SetStatus(nLoginStatus);
	else
	{
		int statusWithWarning = (nLoginStatus | WARNING_MASK);//in order to serialize also the content in case of failure to login to SM
		pRequest->SetStatus(statusWithWarning);
	}

	return STATUS_OK;
}

STATUS CApacheModuleManager::HandleOperLogout(CRequest *pRequest)
{
	if(m_dwConnId)
	{
		FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleOperLogout m_dwConnId: " << m_dwConnId;
        CLogOutRequest* pLogOutRequest = (CLogOutRequest*)pRequest->GetRequestObject();
	//	CConnectionList* pConnList = CApacheModuleEngine::GetConnectionList();
	//	pConnList->RemoveConnection(m_dwConnId);
		pRequest->SetConnectId(m_dwConnId);
        // if <ACTION><LOGOUT><REASON> = session_expired
        if (pLogOutRequest->GetLogourReason() == logoutSessionExpired)
        {
            SetAuditMoreInfo("Due to session timeout");
        }

	}

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

STATUS CApacheModuleManager::HandleCreateDirectory(CRequest *pRequest)
{
	std::string strPhysicalPath;

	CCreateRemoveDir *pRequestCreateRemoveDir = (CCreateRemoveDir*)pRequest->GetRequestObject();
	pRequest->SetConfirmObject(new CDummyEntry());

    const string &virtualPath = pRequestCreateRemoveDir->GetVirtualPath();

    if(virtualPath.empty())
    {
        FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleCreateDirectory. empty name received";
        pRequest->SetStatus(STATUS_ILLEGAL_REQUEST);
        return STATUS_OK;
    }

	if(!CApacheModuleEngine::GetPhysicalPath(pRequestCreateRemoveDir->GetVirtualPath(), strPhysicalPath))
		pRequest->SetStatus(STATUS_CANT_FIND_VIRTUAL_DIRECTORY);
	else
	{
		int virtual_dir_permission = CApacheModuleEngine::GetVirtualDirectoryPermission((char*)strPhysicalPath.c_str());

		if(pRequest->GetAuthorization() <= virtual_dir_permission)
		{
			int create_dir_permission = CApacheModuleEngine::GetCreateDirectoryPermission((char*)strPhysicalPath.c_str());
			if (create_dir_permission == CREATE_DIR_ALL || (create_dir_permission == CREATE_DIR_ADMIN && pRequest->GetAuthorization()==SUPER))
			{
				if(mkdir(strPhysicalPath.c_str(), S_IFDIR|S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != -1)
					pRequest->SetStatus(STATUS_OK);
				else
				{
					if (errno != EEXIST)
					{
						FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleCreateDirectory. Can't create the directory: " << strPhysicalPath.c_str();
						pRequest->SetStatus(STATUS_SERVER_ERROR);
					}
					else
						pRequest->SetStatus(STATUS_DIRECTORY_ALREADY_EXISTS);
				}
			}
			else
			{
				FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleCreateDirectory. No permission to create directory in this virtual directory. The create directory permission for this folder is: "<<create_dir_permission<<" and this user permission is: "<<pRequest->GetAuthorization();
				pRequest->SetStatus(STATUS_ACCESS_DENIED);
			}
		}
		else
		{
			FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleCreateDirectory. No permission to create the directory: " << pRequestCreateRemoveDir->GetVirtualPath().c_str() << ". The virtual directory permission is: "<<virtual_dir_permission<<" while the user permission is: "<<pRequest->GetAuthorization();

			pRequest->SetStatus(STATUS_ACCESS_DENIED);
		}
	}

	return STATUS_OK;
}

STATUS CApacheModuleManager::HandleRemoveDirectory(CRequest *pRequest)
{
	std::string strLogFilesPath 	= MCU_MCMS_DIR+"/LogFiles/";
	std::string strAuditFilePath 	= MCU_MCMS_DIR+"/Audit/";

	std::string strPhysicalPath;

	CCreateRemoveDir *pRequestCreateRemoveDir = (CCreateRemoveDir*)pRequest->GetRequestObject();
	pRequest->SetConfirmObject(new CDummyEntry());

    const string &virtualPath = pRequestCreateRemoveDir->GetVirtualPath();

    if(virtualPath.empty())
    {
        FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleRemoveDirectory. empty name received";
        pRequest->SetStatus(STATUS_ILLEGAL_REQUEST);
        return STATUS_OK;
    }

	if(pRequest->GetAuthorization() == SUPER)
	{
		bool pathFound = CApacheModuleEngine::GetPhysicalPath(pRequestCreateRemoveDir->GetVirtualPath(), strPhysicalPath);

		if (strPhysicalPath == strLogFilesPath || strPhysicalPath == strAuditFilePath)
		{
			// IT 2/4/09
			// VNGR-10006 : Non-privileged accounts can access and delete Audit files.
			pRequest->SetStatus(STATUS_ACCESS_DENIED);

			// Trace the error - it is already audited by default.
			FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleRemoveDirectory. access denied.";
		}
		else if(pathFound == false)
			pRequest->SetStatus(STATUS_NOT_FOUND);
		else
		{
			if(rmdir(strPhysicalPath.c_str()) != -1)
				pRequest->SetStatus(STATUS_OK);
			else
				pRequest->SetStatus(STATUS_SERVER_ERROR);
		}
	}
	else
	{
		pRequest->SetStatus(STATUS_ACCESS_DENIED);
	}

	return STATUS_OK;
}



STATUS CApacheModuleManager::HandleRemoveDirectoryContent(CRequest *pRequest)
{
	std::string strLogFilesPath 	= MCU_MCMS_DIR+"/LogFiles/";
	std::string strAuditFilePath 	= MCU_MCMS_DIR+"/Audit/";

	std::string strPhysicalPath;

	CCreateRemoveDir *pRequestCreateRemoveDir = (CCreateRemoveDir*)pRequest->GetRequestObject();
	pRequest->SetConfirmObject(new CDummyEntry());

	if(pRequest->GetAuthorization() == SUPER)
	{
		bool pathFound = CApacheModuleEngine::GetPhysicalPath(pRequestCreateRemoveDir->GetVirtualPath(), strPhysicalPath);

		if (strPhysicalPath == strLogFilesPath || strPhysicalPath == strAuditFilePath)
		{
			// IT 2/4/09
			// VNGR-10006 : Non-privileged accounts can access and delete Audit files.
			pRequest->SetStatus(STATUS_ACCESS_DENIED);

			// Trace the error - it is already audited by default.
			FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::HandleRemoveDirectoryContent. access denied.";
		}
		else if (pathFound == false)
		{
			pRequest->SetStatus(STATUS_NOT_FOUND);
		}
		else
		{
			string rmDirContentAnswer;
			string rmDirContentCmd = "rm -f " + strPhysicalPath + "/*";

		    STATUS stat = SystemPipedCommand(rmDirContentCmd.c_str(), rmDirContentAnswer);

			TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::HandleRemoveDirectoryContent"
			                       << "\nActivated:     " << rmDirContentCmd.c_str()
			                       << "\nReturn status: " << stat;

			pRequest->SetStatus(stat);
		}
	}
	else
	{
		pRequest->SetStatus(STATUS_ACCESS_DENIED);
	}

	return STATUS_OK;
}




STATUS CApacheModuleManager::HandleRename(CRequest *pRequest)
{
	std::string strInitialPhysicalPath, strNewPhysicalPath;

	CRename *pRequestRename = (CRename*)pRequest->GetRequestObject();
	pRequest->SetConfirmObject(new CDummyEntry());

	if(pRequest->GetAuthorization() == SUPER)
	{
		if(!CApacheModuleEngine::GetPhysicalPath(pRequestRename->GetInitialVirtualPath(), strInitialPhysicalPath) ||
		   !CApacheModuleEngine::GetPhysicalPath(pRequestRename->GetNewVirtualPath(), strNewPhysicalPath))
			pRequest->SetStatus(STATUS_NOT_FOUND);
		else
		{
			if(rename(strInitialPhysicalPath.c_str(),strNewPhysicalPath.c_str()) != -1)
				pRequest->SetStatus(STATUS_OK);
			else
			{
				switch(errno)
				{
					case EACCES:
						pRequest->SetStatus(STATUS_ACCESS_DENIED);
						break;

					case ENOENT:
						pRequest->SetStatus(STATUS_NOT_FOUND);
						break;

					default:
						pRequest->SetStatus(STATUS_SERVER_ERROR);
				}
			}
		}
	}
	else
	{
		pRequest->SetStatus(STATUS_ACCESS_DENIED);
	}

	return STATUS_OK;
}

void CApacheModuleManager::ReceiveAdditionalParams(CSegment* pSeg)
{
	if(!pSeg)
		return;

	*pSeg >> m_dwConnId;
	*pSeg >> m_dwPort;
	*pSeg >> m_bSHMRequest;


	PDELETEA(m_pszClientCertificateSubj);

	DWORD dwLen = 0;

	*pSeg >> dwLen;
	if( dwLen )
	{
		m_pszClientCertificateSubj = new char[dwLen+1];

		*pSeg >> m_pszClientCertificateSubj;
		//DEBUG
		//TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::ReceiveAdditionalParams VVVVVV : [" << m_pszClientCertificateSubj << "], dwLen=" << (int)dwLen << ";";
	}
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::SendAdditionalParams(CSegment* pSeg)
{
	if(!pSeg)
		return;

	*pSeg << m_dwConnId;
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::ResetAdditionalParams()
{
	m_dwConnId = 0;
	m_dwPort = 0;
	m_bSHMRequest = FALSE;

	PDELETEA(m_pszClientCertificateSubj);
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::ConnectionGarbageCollector(CSegment* pMsg)
{
    StartTimer(GARBAGE_COLLECTOR_TIMER, GARBAGE_COLLECTOR_TIMEOUT);

	FTRACESTR(eLevelInfoNormal) << "CApacheModuleManager::ConnectionGarbageCollector";

	CConnectionList* pConnList = CApacheModuleEngine::GetConnectionList();
    if(NULL == pConnList)
    {
        PASSERTMSG(TRUE, "NULL == pConnList");
        return;
    }

    vector<WrapperConnection*> deadConnections;
    pConnList->CleanDeadConnections(deadConnections);

    // send audit events
    for(vector<WrapperConnection*>::iterator iTer = deadConnections.begin();
        iTer != deadConnections.end();
        iTer++)
    {
        CConnection *pConn = (*iTer)->m_pConn;
        SendAuditEventDelConnection(pConn);
    }

    // free memory of dead connection
    for(vector<WrapperConnection*>::iterator iTer = deadConnections.begin();
        iTer != deadConnections.end();
        iTer++)
    {
    	WrapperConnection *pConn = *iTer;
        PDELETE(pConn);
    }
}

/////////////////////////////////////////////////////////////////////////////
void CApacheModuleManager::SendAuditEventDelConnection(CConnection *pConn)
{
    AUDIT_EVENT_HEADER_S outAuditHdr;
    CAuditorApi::PrepareAuditHeader(outAuditHdr,
                                    pConn->GetLogin(),
                                    eMcms,
                                    pConn->GetStationName(),
                                    pConn->GetClientIp(),
                                    eAuditEventTypeInternal,
                                    eAuditEventStatusOk,
                                    "Session timed out",
                                    "RMX Web Client session timed out due to no user activity.",
                                    "",
                                    "Garbage collector cleaned expired connection");
    CFreeData freeData;
    CAuditorApi api;
    api.SendEventMcms(outAuditHdr, freeData);
}

/////////////////////////////////////////////////////////////////////////////
void CApacheModuleManager::OnMcuMngrAuthenticationStruct(CSegment* pSeg)
{
	CApacheModuleProcess* pProcess = (CApacheModuleProcess*)CApacheModuleProcess::GetProcess();
	pProcess->SetIsAuthenticationStructureAlreadyReceived(YES);

	// ===== 1. get the parameters from the structure received into process's attribute
	pSeg->Get( (BYTE*)(pProcess->GetAuthenticationStruct()), sizeof(MCMS_AUTHENTICATION_S) );


	// ===== 2. print the data received
	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::OnMcuMngrAuthenticationStruct -"
				      << *(pProcess->GetAuthenticationStruct());
}


//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::OnMcuMngrApacheModuleLicensingInd(CSegment* pSeg)
{
	CApacheModuleProcess* pProcess = (CApacheModuleProcess*)CApacheModuleProcess::GetProcess();

	pProcess->SetWaitingLicensingInd(FALSE);

	// ===== 1. get the parameters from the structure received into process's attribute
	pSeg->Get( (BYTE*)(pProcess->GetLicensingStruct()), sizeof(APACHEMODULE_LICENSING_S) );

	// ===== 2. print the data received
	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::OnMcuMngrApacheModuleLicensingInd"
							  << "\nNumOfCop ports:  " << pProcess->GetLicensingStruct()->num_cop_parties
							  << "\nNumOfCp ports :  " << pProcess->GetLicensingStruct()->num_cp_parties
							  << "\nAvcSvcCap.SVC :  " << pProcess->GetLicensingStruct()->avcSvcCap.supportSvc
							  << "\nLicenseMode :  " << pProcess->GetLicensingStruct()->avcSvcCap.licenseMode;
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::ManagerPostInitActionsPoint()
{
	bool result = CApacheModuleEngine::FillActionRedirectionMap();
	if(false == result)
	{
		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
				AA_FAILED_TO_FILL_ACTION_REDIRECTION,
				MAJOR_ERROR_LEVEL,
				"Action redirection map incomplete", FALSE);


	}
	if (IsActiveAlarmExistByErrorCode(AA_HTTPD_FAILED_USE_REVOCATION_METHOD))
					RemoveActiveAlarmByErrorCode(AA_HTTPD_FAILED_USE_REVOCATION_METHOD);
	StartTimer(GARBAGE_COLLECTOR_TIMER, GARBAGE_COLLECTOR_TIMEOUT);
	//StartTimer(APACHE_SECURITY_PKI_LOGIN_STATUS_IND, LOOSE_REVOCATION_TIMEOUT);

	DWORD timeout = 0;
	CApacheModuleProcess* pProcess = (CApacheModuleProcess*)CApacheModuleProcess::GetProcess();
	CSysConfig *sysConfig = pProcess->GetSysConfig();
	sysConfig->GetDWORDDataByKey(CFG_KEY_APACHE_WAITING_LICENSING_TIME_SECONDS, timeout);

	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::ManagerPostInitActionsPoint - Start INIT_LICENSING_TIMER " << (int)timeout << ".";
	StartTimer(INIT_LICENSING_TIMER, timeout * SECOND);

	SendAuthenticationStructAndLicensingReqToMcuMngr();
}

//////////////////////////////////////////////////////////////////////
STATUS CApacheModuleManager::HandleTerminalShowTransMap(CTerminalCommand & command, std::ostream& answer)
{
    CApacheModuleEngine::FillActionRedirectionMap();
    CApacheModuleEngine::DumpActionRedirectionMap(answer);
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CApacheModuleManager::HandleTerminalShowVirtualDirMap(CTerminalCommand & command, std::ostream& answer)
{
    CApacheModuleEngine::DumpVirtualDir(answer);
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
static const char *GetHttpdClientStatusStd(int status)
{
    static const char* Names[] =
        {
            "SERVER_DEAD"            ,
            "SERVER_STARTING"        ,
            "SERVER_READY"           ,
            "SERVER_BUSY_READ"       ,
            "SERVER_BUSY_WRITE"      ,
            "SERVER_BUSY_KEEPALIVE"  ,
            "SERVER_BUSY_LOG"        ,
            "SERVER_BUSY_DNS"        ,
            "SERVER_CLOSING"         ,
            "SERVER_GRACEFUL"        ,
            "SERVER_IDLE_KILL"       ,
            "SERVER_NUM_STATUS"
        };
    const char *name = ((unsigned int)status < sizeof(Names) / sizeof(Names[0])
                        ?
                        Names[status] : "invalid");
    return name;
}

//////////////////////////////////////////////////////////////////////
STATUS CApacheModuleManager::HandleTerminalConnectionsList(CTerminalCommand & command, std::ostream& answer)
{

    //     #define SERVER_DEAD 0
// #define SERVER_STARTING 1	/* Server Starting up */
// #define SERVER_READY 2		/* Waiting for connection (or accept() lock) */
// #define SERVER_BUSY_READ 3	/* Reading a client request */
// #define SERVER_BUSY_WRITE 4	/* Processing a client request */
// #define SERVER_BUSY_KEEPALIVE 5	/* Waiting for more requests via keepalive */
// #define SERVER_BUSY_LOG 6	/* Logging the request */
// #define SERVER_BUSY_DNS 7	/* Looking up a hostname */
// #define SERVER_CLOSING 8	/* Closing the connection */
// #define SERVER_GRACEFUL 9	/* server is gracefully finishing request */
// #define SERVER_IDLE_KILL 10     /* Server is cleaning up idle children. */
// #define SERVER_NUM_STATUS 11	/* number of status settings */

    worker_score *ws_record = NULL;
    process_score *ps_record = NULL;
    int server_limit = 1;
    int thread_limit = 41;
    int threadNum = 0;
    int i = 0;

//     for (int i = 0; i < server_limit; ++i)
//     {
//        ps_record = ap_get_scoreboard_process(i);

        for (int j = 0; j < thread_limit; ++j)
        {
//            int indx = (i * thread_limit) + j;

            ws_record = ap_get_scoreboard_worker_from_indexes(i, j);
            if(NULL == ws_record)
            {
                continue;
            }

  //           if (//ws_record->access_count == 0 &&
//                 (ws_record->status == SERVER_READY ||
//                  ws_record->status == SERVER_DEAD))
//             {
//                 continue;
//             }

            answer << "Thread " << j
                   << ", PID " << ws_record->tid
                   << ", Access Cnt " << ws_record->access_count
                   << ", Client " << ws_record->client
                   << ", Request" << ws_record->request
                   << ", Status "<< GetHttpdClientStatusStd(ws_record->status)
                   << "\n";
            threadNum++;
        }
//    }


    answer << "Thread Cnt " << threadNum  << "\n";



	CConnectionList* pConnList = CApacheModuleEngine::GetConnectionList();
	answer << pConnList->WriteConnections().c_str();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::SendEventToAuditor(ConnectionDetails_S connDetails,char* pszRequestXmlString)
{
    eAuditEventStatus status = eAuditEventStatusFail;
    string statusDesc;
    string descEx;

    AUDIT_EVENT_HEADER_S outAuditHdr;
    CAuditorApi::PrepareAuditHeader(outAuditHdr,
                                   connDetails.userName,
                                   eMcms,
                                   connDetails.workStation,
                                   connDetails.clientIp,
                                   eAuditEventTypeHttp,
                                   status,
                                   "Login",
                                   "",
                                   "a user tried to log into the system.",
                                   "");
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "MAX CONNECTION",
                                 eFreeDataTypeXml,
                                 pszRequestXmlString,
                                 "",
                                 eFreeDataTypeText,
                                 "");

    CAuditorApi api;
    api.SendEventMcms(outAuditHdr, freeData);
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::OnMultipleServicesInd(CSegment* pParam)
{
	WORD serviceV35Counter =0;
	*pParam >> m_bSystemMultipleServices
			>> m_bV35JitcSupport
			>>serviceV35Counter;
	
	if(serviceV35Counter > 0)
	{
		
		for(int i=0;i<serviceV35Counter;i++)
		{
			RvgwCredential item;
			*pParam >> item.m_port
					>>item.m_userName
					>>item.m_password;
			m_rvgwCredentials.push_back(item);
		}
	}
	
	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::OnMultipleServicesInd"
						   << "\nMultiple services: : " << (YES == m_bSystemMultipleServices ? "YES" : "NO")
						   << "\nV35 JITC SUPPORT: : " << (YES == m_bV35JitcSupport ? "YES" : "NO")
						   << "\n Number V35 Services : " << serviceV35Counter;
}
/////////////////////////////////////////////////////////////////////
void CApacheModuleManager::OnBlockRequests(CSegment* pParam)
{
	CApacheModuleProcess* pProcess = (CApacheModuleProcess*)CApacheModuleProcess::GetProcess();
	TRACESTR(eLevelInfoNormal) <<"Received Block Requests from daemon";
	pProcess->SetBlockRequest(TRUE);
}
/////////////////////////////////////////////////////////////////////
void CApacheModuleManager::OnHttpdSeucreLoginInd(CSegment* pParam)
{

  //StartTimer(APACHE_SECURITY_PKI_LOGIN_STATUS_IND, LOOSE_REVOCATION_TIMEOUT);
  int bIsLooseRevocation=FALSE;
  std::string alarmDescription;
  bIsLooseRevocation = CApacheModuleEngine::m_Security_Loose_Revocation;
  TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::OnHttpdSeucreLoginInd bIsLooseRevocation = " << bIsLooseRevocation;
  if(!bIsLooseRevocation)
  {
	  if (IsActiveAlarmExistByErrorCode(AA_HTTPD_FAILED_USE_REVOCATION_METHOD))
				RemoveActiveAlarmByErrorCode(AA_HTTPD_FAILED_USE_REVOCATION_METHOD);
	  return;
  }
  if(bIsLooseRevocation == 1)
  {
		alarmDescription ="Failed to use OCSP revocation method";
  }
  else
  {
		alarmDescription ="Failed to use CRL revocation method";
  }
  if (!IsActiveAlarmExistByErrorCode(AA_HTTPD_FAILED_USE_REVOCATION_METHOD))
  {
     	AddActiveAlarm(FAULT_GENERAL_SUBJECT,
     						  AA_HTTPD_FAILED_USE_REVOCATION_METHOD,
  							   MAJOR_ERROR_LEVEL, alarmDescription, true, true);
  }





}
STATUS CApacheModuleManager::HandleSetRvgwAlias(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::HandleSetRvgwAlias";
	CRvgwAliasName* pRvGwReq = (CRvgwAliasName*)pRequest->GetRequestObject();;
	std::string alias = pRvGwReq->getAliasName();
	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::HandleSetRvgwAlias - alias name" << alias;
	CApacheModuleEngine::SetRvgwAliasName(alias);
	pRequest->SetConfirmObject(new CDummyEntry());
	pRequest->SetStatus(STATUS_OK);
	return STATUS_OK;
}

STATUS CApacheModuleManager::HandleCreateSecurityToken(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::HandleCreateSecurityToken";
	CGateWaySecurityTokenRequest* pSecTokenReq = (CGateWaySecurityTokenRequest*)pRequest->GetRequestObject();
	DWORD nConnId = pRequest->GetConnectId();
	
	TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::HandleCreateSecurityToken search for connection Id " << nConnId;
	CConnectionList*  conlist = CApacheModuleEngine::GetConnectionList();	

	TICKS v35Securityticket;
    bool isConExist = conlist->SetConnectionV35Details(nConnId, pSecTokenReq, m_rvgwCredentials,  v35Securityticket);

	if(isConExist)
	{
		   CGatewaySecurityTokenResponse *pSecToken = new CGatewaySecurityTokenResponse();
			char buffer[SECURITY_TOKEN_MAX]= {0};


			//DWORD tokenId = tick.GetMiliseconds();
			sprintf(buffer,"%u",v35Securityticket.GetMiliseconds());
			std:string token(buffer);	
			pSecToken->SetSecurityToken(token);
			pRequest->SetConfirmObject(pSecToken);
			pRequest->SetStatus(STATUS_OK);
			return STATUS_OK;
	}
	else
	{
		char *pszRequestXmlString = NULL;
		pRequest->DumpSetRequestAsString(&pszRequestXmlString);

		TRACESTR(eLevelInfoNormal) << "\nCApacheModuleManager::HandleCreateSecurityToken failed to find connection Id " << nConnId << " \n "
							<<pszRequestXmlString;
		DEALLOCBUFFER(pszRequestXmlString);
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_FAIL);
		return STATUS_FAIL;
	}
}

//////////////////////////////////////////////////////////////////////
void CApacheModuleManager::OnLicensingInitialisationTimeout(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "CApacheModuleManager::OnLicensingInitialisationTimeout";
	CApacheModuleProcess* pProcess = (CApacheModuleProcess*)CApacheModuleProcess::GetProcess();
	pProcess->SetWaitingLicensingInd(FALSE);
}

//////////////////////////////////////////////////////////////////////
