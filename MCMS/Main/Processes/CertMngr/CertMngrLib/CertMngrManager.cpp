// CertMngrManager.cpp

#include "CertMngrManager.h"

#include <time.h>
#include <libgen.h>
#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include <stdio.h>
#include <fstream>

#include "Trace.h"
#include "DummyEntry.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "CertificateRequest.h"
#include "Certificate.h"
#include "SSLInterface.h"
#include "ApiStatuses.h"
#include "Request.h"
#include "TraceStream.h"
#include "ProcessBase.h"
#include "SslFunc.h"
#include "OpcodesMcmsInternal.h"
#include "ManagerApi.h"
#include "FipsMode.h"
#include "StructTm.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "CertMngrProcess.h"
#include "CertificateGet.h"
#include "OsFileIF.h"
#include "McmsDaemonApi.h"
#include "TerminalCommand.h"
#include "AuditorApi.h"
#include "FaultsDefines.h"
#include "SerializeObject.h"
#include "ConfigManagerOpcodes.h"
#include "AlarmStrTable.h"
#include "CertificateInfoCTL.h"
#include "CertificateInfoCRL.h"
#include "InternalProcessStatuses.h"

PBEGIN_MESSAGE_MAP(CCertMngrManager)
    ONEVENT(XML_REQUEST, IDLE, CCertMngrManager::HandlePostRequest)
    ONEVENT(MCUMNGR_TO_CERTMNGR_HOST_NAME_UPDATE, ANYCASE, CCertMngrManager::OnUpdateHostName)
    ONEVENT(CERTMNGR_GET_CERT_DETAILS, ANYCASE, CCertMngrManager::OnGetCertDetails)
    ONEVENT(CERTMNGR_VERIFY_CERTIFICATE, ANYCASE, CCertMngrManager::OnVerifyCertificate)
    ONEVENT(CERTMNGR_VERIFY_CS_CERTIFICATE, ANYCASE, CCertMngrManager::OnVerifyCSCertificate)
    ONEVENT(CERTMNGR_REMOVE_ACTIVE_ALARM, ANYCASE, CCertMngrManager::OnRemoveActiveAlarm)
    ONEVENT(HTTPD_TO_CERTMNGR_LINK_FILE, ANYCASE, CCertMngrManager::OnHttpdCertMngrLinkFile)
    ONEVENT(CS_CERTMNGR_IP_SERVICE_PARAM_IND, ANYCASE, CCertMngrManager::OnCsIpServiceParamsInd)
    ONEVENT(CS_CERTMNGR_IP_SERVICE_PARAM_END_IND, ANYCASE, CCertMngrManager::OnCsIpServiceParamsEndInd)
    ONEVENT(CS_CERTMNGR_DELETE_IP_SERVICE_IND, ANYCASE, CCertMngrManager::OnCsDeleteIpServiceInd)
    ONEVENT(MCUMNGR_TO_CERTMNGR_MULTIPLE_SERVICES_IND, ANYCASE, CCertMngrManager::OnMultipleServicesInd)
PEND_MESSAGE_MAP(CCertMngrManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CCertMngrManager)
    ON_TRANS("TRANS_CERTIFICATE_REQUEST", "CREATE", CCertificateRequest, CCertMngrManager::HandleCreateCertificateReq)
    ON_TRANS("TRANS_CERTIFICATE_REQUEST", "CREATE_CS", CCertificateRequest, CCertMngrManager::HandleCreateCertificateReqForCS)
    ON_TRANS("TRANS_CERTIFICATE", "SEND", CCertificate, CCertMngrManager::HandleSendCertificate)
    ON_TRANS("TRANS_CERTIFICATE", "SEND_CS", CCertificate, CCertMngrManager::HandleSendCertificateForCS)
    ON_TRANS("TRANS_CERTIFICATE", "SEND_CA", CCertificate, CCertMngrManager::HandleSendCACertificate)
    ON_TRANS("TRANS_CERTIFICATE", "DELETE_CA", CCertificateGet, CCertMngrManager::HandleDeleteCTL)
    ON_TRANS("TRANS_CERTIFICATE", "DELETE_CRL", CCertificateGet, CCertMngrManager::HandleDeleteCRL)
    ON_TRANS("TRANS_CERTIFICATE", "FINISH_UPLOAD_CERTIFICATE", CDummyEntry, CCertMngrManager::HandleFinishUploadCertificate)
    ON_TRANS("TRANS_CERTIFICATE_LIST", "UPDATE_CERTIFICATE_REPOSITORY", CDummyEntry, CCertMngrManager::HandleUpdateCertificateRepository)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CCertMngrManager)
	ONCOMMAND("self_sign_cert",	CCertMngrManager::HandleTerminalSelfSignCertificateRequest, "Creates self sign certificate request")
    ONCOMMAND("print_cert_details",	CCertMngrManager::HandleTerminalCertDetails, "Prints certificate details")
    ONCOMMAND("ls_cert", CCertMngrManager::HandleTerminalListCert, "Prints list of all certificates")
    ONCOMMAND("rm_cert", CCertMngrManager::HandleTerminalRemoveCert, "Removes certificate")
    ONCOMMAND("add_cert", CCertMngrManager::HandleTerminalAddCert, "Adds certificate from a file")
    ONCOMMAND("vfy_cert", CCertMngrManager::HandleTerminalVerifyCert, "Verifies certificate validity")
    ONCOMMAND("rma_cert", CCertMngrManager::HandleTerminalRemoveAACert, "Removes certificate validity active alarms")
END_TERMINAL_COMMANDS

extern void CertMngrMonitorEntryPoint(void* appParam);

void CertMngrManagerEntryPoint(void* appParam)
{  
	CCertMngrManager* pCertMngrManager = new CCertMngrManager;
	pCertMngrManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CCertMngrManager::GetMonitorEntryPoint(void)
{
	return CertMngrMonitorEntryPoint;
}

CCertMngrManager::CCertMngrManager(void)
{
	m_pProcess = (CCertMngrProcess*)CCertMngrProcess::GetProcess();
	PASSERTMSG(NULL == m_pProcess, "Unable to continue");

	m_bSystemMultipleServices = TRUE;
}

CCertMngrManager::~CCertMngrManager(void)
{}

// Virtual
const char* CCertMngrManager::NameOf(void) const
{
    return GetCompileType();
}

void CCertMngrManager::ManagerInitActionsPoint()
{
	TestAndEnterFipsMode();

	CSslFunctions::init_OpenSSL();
	CSslInterface::ConvertPrivateKey();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();

	// Increases MaxXMLFileSize for the process. It is the size of XML DB.
	DWORD old_size = CSerializeObject::SetMaxXMLFileSize(CertMngrLimits::kXMLFileMaxSize);
	TRACEINTO << "MaxXMLFileSize is changed from " << old_size
	          << " to " << CertMngrLimits::kXMLFileMaxSize
	          << " for " << CProcessBase::GetProcessName(m_pProcess->GetProcessType());

	m_pProcess->FillLists();
}

void CCertMngrManager::ManagerPostInitActionsPoint()
{}

void CCertMngrManager::ManagerStartupActionsPoint()
{
	// ask CentralSignaling to send mediaIpParams
	SendIpServiceParamsReqToCS();
}

void CCertMngrManager::DeclareStartupConditions()
{
	CActiveAlarm aa(FAULT_GENERAL_SUBJECT,
					AA_NO_IP_SERVICE_PARAMS,
					MAJOR_ERROR_LEVEL,
					"No IP service was received from CSMngr",
					false,
					false);
 	AddStartupCondition(aa);
}

void CCertMngrManager::SendIpServiceParamsReqToCS()
{
	TRACESTR(eLevelInfoNormal) << "CCertMngrManager::SendIpServiceParamsReqToCS";

	CSegment*  pRetParam = new CSegment;

	const COsQueue* pCsMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCSMngr, eManager);

	STATUS res = pCsMbx->Send(pRetParam,CSMNGR_CERTMNGR_IP_SERVICE_PARAM_REQ);
	
	if (res != STATUS_OK)
	{
		TRACESTR(eLevelInfoNormal) << "CCertMngrManager::SendIpServiceParamsReqToCS - failed to send request to the CSMngr";
	}
}

STATUS CCertMngrManager::HandleCreateCertificateReq(CRequest* pRequest)
{
	STATUS nRetStatus = STATUS_OK;

	CCertificateRequest *pCertificateRequest =
	    (CCertificateRequest*)pRequest->GetRequestObject();

	CCertificateRequest *pCertificateRequestResponse = NULL;
	
	if (pRequest->GetAuthorization() == SUPER)
	{	
		nRetStatus = CSslInterface::CreateSslCertificateRequest(pCertificateRequest);
		if (nRetStatus!=STATUS_OK)
		{
			pRequest->SetConfirmObject(new CDummyEntry());
		}
		else
		{
		        std::string cert_req;
			nRetStatus = GetCertificateRequest(cert_req);
			if (nRetStatus!=STATUS_OK)
			{
				pRequest->SetConfirmObject(new CDummyEntry());
			}
			else
			{
				pCertificateRequestResponse = new CCertificateRequest();
				pCertificateRequestResponse->SetCertificateRequest(cert_req);
				pRequest->SetConfirmObject(pCertificateRequestResponse);
			}
		}
		pRequest->SetStatus(nRetStatus);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_ACCESS_DENIED);
	}
	return STATUS_OK;
}

STATUS CCertMngrManager::HandleCreateCertificateReqForCS(CRequest* pRequest)
{
	STATUS nRetStatus = STATUS_OK;

	CCertificateRequest *pCertificateRequest = (CCertificateRequest*)pRequest->GetRequestObject();
	CCertificateRequest *pCertificateRequestResponse = NULL;
	
	if (pRequest->GetAuthorization() == SUPER)
	{	
		nRetStatus = CSslInterface::CreateSslCertificateRequest(pCertificateRequest, TRUE);
		if (nRetStatus!=STATUS_OK)
		{
			pRequest->SetConfirmObject(new CDummyEntry());
		}
		else
		{
		        std::string cert_req;
		
			std::string folder_name = CSslInterface::GetFolderName(pCertificateRequest->GetServiceName()); 
		
			nRetStatus = GetCertificateRequest(cert_req, TRUE, folder_name);
			if (nRetStatus!=STATUS_OK)
			{
				pRequest->SetConfirmObject(new CDummyEntry());
			}
			else
			{
				pCertificateRequestResponse = new CCertificateRequest();
				pCertificateRequestResponse->SetCertificateRequest(cert_req);
				pRequest->SetConfirmObject(pCertificateRequestResponse);
			}
		}
		pRequest->SetStatus(nRetStatus);
	}
	else
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_ACCESS_DENIED);
	}
	return STATUS_OK;
}

STATUS CCertMngrManager::HandleSendCertificate(CRequest* pRequest)
{
	STATUS nRetStatus = STATUS_OK;

	CCertificate *pCertificate = (CCertificate*)pRequest->GetRequestObject();

	pRequest->SetConfirmObject(new CDummyEntry());
	
	if (pRequest->GetAuthorization() == SUPER)
	{
		nRetStatus = CSslInterface::SaveSslCertificateEx(
		    pCertificate->GetCertificate().c_str(),
		    m_host_name.c_str(), eCertificatePersonal);

		pRequest->SetStatus(nRetStatus);
		
		if (nRetStatus==STATUS_OK)
			SendCertificateUpdateIndToMcuMngr();
	}
	else
		pRequest->SetStatus(STATUS_ACCESS_DENIED);
	
	return STATUS_OK;
}

STATUS CCertMngrManager::HandleSendCertificateForCS(CRequest* pRequest)
{
	STATUS nRetStatus = STATUS_OK;

	CCertificate *pCertificate = (CCertificate*)pRequest->GetRequestObject();

	pRequest->SetConfirmObject(new CDummyEntry());
	
	if (pRequest->GetAuthorization() == SUPER)
	{
		std::string folder_name = CSslInterface::GetFolderName(pCertificate->GetServiceName());

		nRetStatus = CSslInterface::SaveSslCertificateEx(
			    pCertificate->GetCertificate().c_str(),
			    m_host_name.c_str(), eOCS, folder_name);

		if (nRetStatus == STATUS_OK)
		{
			nRetStatus = STATUS_OK_WARNING;

			// VNGR-19276
			AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
				AA_CERTIFICATE_FOR_CS_NEED_RESET_TO_TAKE_EFFECT,
				MAJOR_ERROR_LEVEL,
				GetAlarmDescription(AA_CERTIFICATE_FOR_CS_NEED_RESET_TO_TAKE_EFFECT),
				true,
				true);


		    std::string buf;
		    int serviceId = m_pProcess->GetServiceId(pCertificate->GetServiceName());
		    BOOL isSecured  =m_pProcess->GetServiceIsSecured(serviceId);
		    BYTE revocation_method = m_pProcess->GetServiceRevocationMethod(serviceId);
		    BYTE isMSService = m_pProcess->GetIsMsService(serviceId);
			CStructTm now;
			SystemGetTime(now);

		    STATUS stat = m_pProcess->VerifyCSServiceCertificates(pCertificate->GetServiceName(),
		    												now,
		    												isSecured ? true : false,
		    												true,
		    												revocation_method,
		    												isMSService,
		    												buf);

		    if (!buf.empty())
		    {
		    	TRACEINTOFUNC << "CS Certificate Active Alarm Table:\n" << buf;
		    }

		}
		else
		{
			TRACEINTOFUNC << "Failed SaveSslCertificate for CS certificate";
		}
        
		pRequest->SetStatus(nRetStatus);
	}
	else
		pRequest->SetStatus(STATUS_ACCESS_DENIED);
		
	return STATUS_OK;
}


STATUS CCertMngrManager::HandleSendCACertificate(CRequest* pRequest)
{
	STATUS nRetStatus = STATUS_OK;

	CCertificate* pCertificate = (CCertificate*)pRequest->GetRequestObject();
	
	pRequest->SetConfirmObject(new CDummyEntry);
	
	if (pRequest->GetAuthorization() == SUPER)
	{
		nRetStatus = CSslInterface::SaveSslCertificateEx(
		    pCertificate->GetCertificate().c_str(),
		    m_host_name.c_str(), eCertificateTrust);

		pRequest->SetStatus(nRetStatus);
		if (nRetStatus == STATUS_OK)
		{
		    // Notifies clients
		    AACertRepositoryChanged();
		}	
	}
	else
		pRequest->SetStatus(STATUS_ACCESS_DENIED);
		
	return STATUS_OK;	
}


STATUS CCertMngrManager::AddCert(eCertificateType type, const char* fname)
{
    // Adds certificate to local DB and Apache's file
    STATUS stat = m_pProcess->AddCertificate(type, fname);
    if (STATUS_OK != stat)
    {
        AuditHttpCertificatesOperations(type, eAuditEventStatusFail);
        std::string error = m_pProcess->GetStatusAsString(stat);
        PASSERTSTREAM(STATUS_OK != stat,
            "Unable to add a certificate from " << fname
                << ": " << error);
        m_certUploadStatus = error;
        return stat;
    }

    AuditHttpCertificatesOperations(type, eAuditEventStatusOk);
    TRACEINTOFUNC << "Certificate file " << fname << " is added successfully";

    // Notifies clients
    AACertRepositoryChanged();

    return STATUS_OK;
}

STATUS CCertMngrManager::OpenPFXFile(string& folderName, string& fullPFXFileName,BOOL toUpdateList)
{
    PASSERTSTREAM_AND_RETURN_VALUE(!IsFileExists(fullPFXFileName),
   		 "File does not exist " << fullPFXFileName ,
   		 STATUS_FAIL);

     string passPFX;
     string passwordFileName = folderName + CS_PFX_PASSWORD_FILE_NAME;

	 std::ifstream ifs(passwordFileName.c_str());

	 if (ifs != NULL)
	 {
		 passPFX.assign( (std::istreambuf_iterator<char>(ifs) ),
										(std::istreambuf_iterator<char>() ) );

		 TRACEINTOFUNC << "OpenPFXFile:  " << fullPFXFileName << ". Reading password from file";
	 }
	 else
	 {
		 TRACEINTOFUNC << "OpenPFXFile:  " << fullPFXFileName << ". Using default password";
		 passPFX = "polycom";
	 }

	 string outCmd;

	 std::stringstream strCmd;

	 string tempKeyFileName = folderName   + "temp_private_for_cs.pem";
	 remove(tempKeyFileName.c_str());
	 strCmd << "echo '" << passPFX << "' | openssl pkcs12 -in " << fullPFXFileName << " -nocerts -out " << tempKeyFileName << " -nodes -passin stdin";


	 STATUS retCmd = SystemPipedCommand((const char *)strCmd.str().c_str(), outCmd);

	 if((STATUS_OK != retCmd )||(GetFileSize(tempKeyFileName) <= 0))
	 {
		 m_certUploadStatus ="Invalid password or private key does not exist in pfx.";
		 PASSERTSTREAM_AND_RETURN_VALUE( true,
				 "Failed extract pfx key. output: " << outCmd << " retCmd: " << (int)retCmd,
				 STATUS_FAIL);
	 }

	  ////message should not be displayed because it contains password
	 //TRACEINTOFUNC << " create key  " << strCmd.str()  << ". output " <<  outCmd;


	 strCmd.str("");
	 strCmd.clear();
	 outCmd = "";

	 string tempCertFileName = folderName + "temp_cert_off_for_cs.pem";

	 remove(tempCertFileName.c_str());
	 strCmd << "echo '" << passPFX << "' | openssl pkcs12 -in " << fullPFXFileName << " -clcerts -out " << tempCertFileName << " -nodes -passin stdin";

	 retCmd = SystemPipedCommand((const char *)strCmd.str().c_str(), outCmd, TRUE, FALSE, TRUE);

	 if((STATUS_OK != retCmd )||(GetFileSize(tempKeyFileName) <= 0))
	 {
		 m_certUploadStatus = "Failed to extract pfx certificate.";
		 PASSERTSTREAM_AND_RETURN_VALUE(true,
			  "Failed extract pfx certificate. output: " << outCmd << " retCmd: " << (int)retCmd,
	      STATUS_FAIL);
	 }

	 //message should not be displayed because it contains password
	 //TRACEINTOFUNC << "create certificate  " << strCmd.str()  << " output " <<  outCmd;

	 strCmd.str("");
	 strCmd.clear();
	 outCmd = "";

	 string tempCaFileName = folderName  + "temp_ca_cert_off_for_cs.pem" ;

	 remove(tempCaFileName.c_str());
	 strCmd << "echo '" << passPFX << "' | openssl pkcs12 -in " << fullPFXFileName << " -cacerts -out " << tempCaFileName << " -nodes -passin stdin";

	 retCmd = SystemPipedCommand((const char *)strCmd.str().c_str(), outCmd, TRUE, FALSE, TRUE);

	 if (STATUS_OK == retCmd && (GetFileSize(tempCaFileName) > 0) )
	 {
		 //message should not be displayed because it contains password
		 //TRACEINTOFUNC << "create ca  certificate  " << strCmd.str()  << " output " <<  outCmd;
		if (AddCert(eCertificateTrust, (const char *)tempCaFileName.c_str()) != STATUS_OK)
		{
			// we dont fail the operation
			TRACESTRFUNC(eLevelError) << "Failed adding CA certificate of PFX " << tempCaFileName ;
		}
	 }
	 else
	 {
		 TRACESTRFUNC(eLevelWarn) << "Failed extract pfx ca certificate. output: " << outCmd << " retCmd: " << (int)retCmd;
	 }
	 if(toUpdateList)
	 {
		 CCertMngrProcess* process = ((CCertMngrProcess*)CProcessBase::GetProcess());
		 FPASSERTMSG_AND_RETURN_VALUE(!process, "Unable to continue", STATUS_FAIL);
		 if (process->AddCertificate(eOCS, (const char *)tempCertFileName.c_str()) != STATUS_OK)
		 {
			 m_certUploadStatus = "failed to add ocs certificate";
			 TRACESTRFUNC(eLevelError) << "Failed adding certificate to repository for PFX " << fullPFXFileName;
			 return STATUS_FAIL;
		 }
	 }
	m_certUploadStatus = "status OK";
    return STATUS_OK;

}

STATUS CCertMngrManager::DelCert(eCertificateType type,
                                 const char* issuer,
                                 const char* serial)
{
    STATUS stat = m_pProcess->DeleteCertificate(type, issuer, serial);

    if (eCertificateRevocation != type)
    {
        PASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
            "Unable to delete " << CertificateTypeToStr(type)
                << ", issuer " << issuer
                << ", serial " << serial << ": "
                << m_pProcess->GetStatusAsString(stat),
            stat);
    }
    else
    {
        PASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
            "Unable to delete " << CertificateTypeToStr(type)
                << ", issuer " << issuer << ": "
                << m_pProcess->GetStatusAsString(stat),
            stat);
    }

    if (eCertificateRevocation != type)
    {
        TRACEINTOFUNC << "Deleted " << CertificateTypeToStr(type)
                      << ", issuer " << issuer
                      << ", serial " << serial;
    }
    else
    {
        TRACEINTOFUNC << "Deleted " << CertificateTypeToStr(type)
                      << ", issuer " << issuer;
    }

    // Notifies clients
    AACertRepositoryChanged();

    return STATUS_OK;
}

// Notifies client about changes in Certificate Repository without
// Apache restart. A client should restart Apache.
void CCertMngrManager::AACertRepositoryChanged(void)
{
    WORD aa = AA_CERTIFICATE_REPOSITORY_NOT_UPDATED;
    if (IsActiveAlarmExistByErrorCode(aa))
        return;

    DWORD res = AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                                        aa,
                                        MAJOR_ERROR_LEVEL,
                                        GetAlarmDescription(aa),
                                        true, true);

    PASSERTSTREAM_AND_RETURN(0xFFFFFFFF == res,
        "Unable to add Active Alarm " << GetAlarmName(aa));

    TRACEINTO << __PRETTY_FUNCTION__ << ": "
              << "Added Active Alarm " << GetAlarmName(aa);
}

STATUS CCertMngrManager::HandleDelete(CRequest* req, eCertificateType type)
{
    PASSERTMSG_AND_RETURN_VALUE(NULL == req, "Invalid parameter", STATUS_OK);

    // Dummy entry anyway
    req->SetConfirmObject(new CDummyEntry);

    // Checks type of received object
    CSerializeObject* obj = req->GetRequestObject();
    if (!obj->IsTypeOf(CCertificateGet::GetCompileType()))
    {
        req->SetStatus(STATUS_OBJECT_NOT_RECOGNIZED);
        PASSERTSTREAM(true,
            "Invalid class type " << obj->GetRTType()
                << ", should be " << CCertificateGet::GetCompileType());
        return STATUS_OK;
    }

    if (req->GetAuthorization() != SUPER)
    {
        req->SetStatus(STATUS_ACCESS_DENIED);
        PASSERTSTREAM(true, "User " << req->GetAuthorization()
            << " is not authorized to delete certificate");
        return STATUS_OK;
    }

    CCertificateGet* cert = (CCertificateGet*)obj;
    const char* issuer = cert->GetIssuerName();
    const char* serial = cert->GetSerialNumber();

    STATUS stat = DelCert(type, issuer, serial);
    req->SetStatus(stat);

    return STATUS_OK;
}

STATUS CCertMngrManager::HandleDeleteCTL(CRequest* req)
{
    return HandleDelete(req, eCertificateTrust);
}

STATUS CCertMngrManager::HandleDeleteCRL(CRequest* req)
{
    return HandleDelete(req, eCertificateRevocation);
}

// Request to restart Apache in order to read all changes
// in Certificate Repository
STATUS CCertMngrManager::HandleUpdateCertificateRepository(CRequest* req)
{
    PASSERT_AND_RETURN_VALUE(NULL == req, STATUS_OK);

    // Dummy entry anyway
    req->SetConfirmObject(new CDummyEntry);

    if (req->GetAuthorization() != SUPER)
    {
        req->SetStatus(STATUS_ACCESS_DENIED);
        PASSERTSTREAM(true, "User " << req->GetAuthorization()
            << " is not authorized to Update Certificate Repository");
        return STATUS_OK;
    }

    // Sends asynchronous request to restart Apache as root
    STATUS stat = CTaskApi::SendMsgWithTrace(eProcessConfigurator,
                                             eManager,
                                             NULL,
                                             CONFIGURATOR_RESTART_APACHE);
    if (STATUS_OK == stat)
    {
        // Removes Active Alarm
        WORD aa = AA_CERTIFICATE_REPOSITORY_NOT_UPDATED;
        if (IsActiveAlarmExistByErrorCode(aa))
            RemoveActiveAlarmByErrorCode(aa);
    }

    req->SetStatus(stat);

    // Always positive answer
    return STATUS_OK;
}

void CCertMngrManager::SendCertificateUpdateIndToMcuMngr()
{
    TRACESTR(eLevelInfoNormal) << "\nCCertMngrManager::SendCertificateUpdateIndToMcuMngr";

    CManagerApi apiMcuMngr(eProcessMcuMngr);
    apiMcuMngr.SendOpcodeMsg(CERTMNGR_TO_MCUMNGR_CERTIFICATE_UPDATE_IND); 
}

WORD CCertMngrManager::GetCertificateRequest(std::string& cert_req,
                                             BYTE bForCS/*=FALSE*/,
                                             std::string folder_name/*=""*/)
{
	char* cert = NULL;

	std::string temp_cert_req_file_name;

	if (bForCS == TRUE)
	{
		if (folder_name=="")
			temp_cert_req_file_name = TEMP_CERT_REQ_F_FOR_CS;
		else
			temp_cert_req_file_name = HOME_CS + folder_name + "/temp_cert_for_cs.csr";
	}
	else
		temp_cert_req_file_name = TEMP_CERT_REQ_F;
	DWORD fileSize =  GetFileSize(temp_cert_req_file_name);
	if(fileSize  <= 0 )
	{
		FTRACESTR(eLevelInfoNormal) << "CCertMngrManager::GetCertificateRequest , failed to get file size  fileSize= "<<fileSize;
		return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	}

	cert = new char[fileSize+1];
	memset(cert,'\0',fileSize+1);
	FILE* fp;
	if ((fp=fopen(temp_cert_req_file_name.c_str(),"r"))==NULL)
	{
		FTRACESTR(eLevelInfoNormal) << "CCertMngrManager::GetCertificateRequest , fopen failed. can't read the certificate request from file - "<<temp_cert_req_file_name;
		delete[] cert;
		return STATUS_UNABLE_TO_CREATE_SSL_CERTIFICATE_REQUEST;
	}


    DWORD bytes = fread(cert, sizeof(char), fileSize, fp);
    if(bytes != fileSize)
    {
    	FTRACESTR(eLevelInfoNormal) << "Failed to read all certificate read bytes " << bytes
    						        << "  file size " << fileSize;
    }
	cert_req = cert;
	fclose(fp);
	delete[] cert;
	return STATUS_OK;
}

void CCertMngrManager::OnUpdateHostName(CSegment* pMsg)
{
    PASSERTMSG_AND_RETURN(NULL == pMsg, "Invalid parameter");
    std::string prev = m_host_name.empty() ? "<empty>" : m_host_name;
    *pMsg >> m_host_name;

    TRACEINTOFUNC << "The host name changed from " << prev
                  << " to " << m_host_name;

   

}

STATUS CCertMngrManager::HandleTerminalSelfSignCertificateRequest(CTerminalCommand& command,
                                                                  std::ostream& answer)
{
    answer << "Not implemented yet";
    return STATUS_OK;
}

STATUS CCertMngrManager::GetCertificateDetails(std::string& host_name,
                                               CStructTm& cert_start_date,
                                               CStructTm& cert_expiration_date,
                                               std::string& errror_message)
{
	STATUS retStatus = CSslFunctions::CheckValidationOfCerificateAndPrivateKey();

	if (retStatus != STATUS_OK)
	{
        errror_message = "certificate validation failed";
        return retStatus;
	}
    
    retStatus = CSslFunctions::GetCertificateDetails(host_name,
                                                     cert_start_date,
                                                     cert_expiration_date,
                                                     errror_message);

    return retStatus;
}

STATUS CCertMngrManager::HandleTerminalCertDetails(CTerminalCommand & command,
                                                   std::ostream& answer)
{
    std::string host_name;
    CStructTm cert_start_date;
    CStructTm cert_expiration_date;
    std::string errror_message;
    
    STATUS res = GetCertificateDetails(host_name,
                                       cert_start_date,
                                       cert_expiration_date,
                                       errror_message);

    if (res == STATUS_OK)
    {
        answer << "host name:" <<  host_name << std::endl;
        answer << "start date:" <<  cert_start_date << std::endl;
        answer << "expiration date:" << cert_expiration_date << std::endl ;
    }
    else
    {
        answer << "failed due to status:" << res << std::endl;
        answer << "error message:" << errror_message << std::endl;
    }
    
	return STATUS_OK;
}

// Static
void CCertMngrManager::ManLsCert(std::ostream& ans)
{
    ans << "NAME\n"
        << "\tls_cert - list configured certificates\n\n"
        << "SYNOPSIS\n"
        << "\tca ls_cert CertMngr [TYPE] [FNAME]\n\n"
        << "DESCRIPTION\n"
        << "\tThe command without parameters prints configured certificates."
        << " The command with parameters prints info for the certificate\n\n"
        << "\tTYPE\t1 - Certificate Trust, 2 - Certificate Personal, "
        << "3 - Certificate Revocation\n\n"
        << "\tFNAME\tfile names of certificates\n";
}

////////////////////////////////////////////////////////////////////////////////
STATUS CCertMngrManager::HandleTerminalListCert(CTerminalCommand& cmd,
                                                std::ostream& ans)
{
    if (cmd.GetNumOfParams() > 2 || 1 == cmd.GetNumOfParams())
    {
        ManLsCert(ans);
        return STATUS_OK;
    }

    if (2 == cmd.GetNumOfParams())
    {
        DWORD type=0;
        std::istringstream buf1(cmd.GetToken(eCmdParam1));
        if (!(buf1 >> type))
        {
            ans << "Unable to get certificate type\n";
            ManLsCert(ans);
            return STATUS_OK;
        }

        CCertificateInfo* cer;
        switch (type)
        {
        case eCertificateTrust:
        case eCertificatePersonal:
            cer = new CCertificateInfoCTL;
            break;

        case eCertificateRevocation:
            cer = new CCertificateInfoCRL;
            break;

        default:
            ans << "Unsupported certificate type " << type << "\n";
            ManLsCert(ans);
            return STATUS_OK;
        }

        const char* stype = CertificateTypeToStr(static_cast<eCertificateType>(type));

        std::string fname = cmd.GetToken(eCmdParam2);

        // Copies file to temporary
        char* dname = strdup(fname.c_str());
        char* bname = basename(dname);
        std::string fname2 = MCU_TMP_DIR;
        fname2.append(bname);
        free(dname);

        // Continues with the copy
        if (!CopyFile(fname, fname2))
        {
            delete cer;
            ans << "Unable to copy " << fname << " to " << fname2 << "\n";
            ManLsCert(ans);
            return STATUS_OK;
        }

        STATUS stat = cer->Init(fname2.c_str());

        ans << m_pProcess->GetStatusAsString(stat)
            << " for " << stype << " " << fname;

        if (STATUS_OK == stat)
            PrintCertOutList(ans, stype)(cer);

        // Removes the copy
        if (!DeleteFile(fname2))
            ans << "\nUnable to delete " << fname2;

        delete cer;

        return STATUS_OK;
    }

    const eCertificateType types[] = {
        eCertificateTrust,
        eCertificatePersonal,
        eCertificateRevocation
    };

    for (const eCertificateType* type = types; type < ARRAYEND(types); ++type)
    {
        const CCertificateList* list = m_pProcess->ReadList(*type);
        if (!list)
            continue;

        ans << std::endl;
        list->PrintOut(ans);

        // Does not add new line after last print
        if (type + 1 < ARRAYEND(types))
            ans << std::endl;
    }

    return STATUS_OK;
}

// Static
void CCertMngrManager::ManRmCert(std::ostream& ans)
{
    ans << "NAME\n"
        << "\trm_cert - remove certificate from RMX\n"
        << std::endl
        << "SYNOPSIS\n"
        << "\tca rm_cert CertMngr TYPE ISSUER SERIAL\n"
        << std::endl
        << "DESCRIPTION\n"
        << "\tTYPE\t1 - Certificate Trust, 2 - Certificate Personal, "
        << "3 - Certificate Revocation\n"
        << std::endl
        << "\tISSUER\tname of an issuer of a certificate to delete\n"
        << std::endl
        << "\tSERIAL\tserial number of certificate, empty for type 3 CRL\n";
}

STATUS CCertMngrManager::HandleTerminalRemoveCert(CTerminalCommand& cmd,
                                                  std::ostream& ans)
{
    if (cmd.GetNumOfParams() < 2 || cmd.GetNumOfParams() > 3)
    {
        ManRmCert(ans);
        return STATUS_OK;
    }

    DWORD type=0;
    std::istringstream buf1(cmd.GetToken(eCmdParam1));
    if (!(buf1 >> type))
    {
        ans << "Unable to get certificate type\n";
        ManRmCert(ans);
        return STATUS_OK;
    }

    std::string issuer = cmd.GetToken(eCmdParam2);
    if (0 == issuer.compare("Invalide Token"))
    {
        ans << "Unable to get issuer\n";
        ManRmCert(ans);
        return STATUS_OK;
    }

    std::string serial;
    if (3 == cmd.GetNumOfParams())
    {
        std::istringstream buf3(cmd.GetToken(eCmdParam3));
        if (!(buf3 >> serial))
        {
            ans << "Unable to get serial\n";
            ManRmCert(ans);
            return STATUS_OK;
        }
    }

    STATUS stat = DelCert(static_cast<eCertificateType>(type),
                          issuer.c_str(), serial.c_str());

    ans << m_pProcess->GetStatusAsString(stat);

    return STATUS_OK;
}

// Static
void CCertMngrManager::ManAddCert(std::ostream& ans)
{
    ans << "NAME\n"
        << "\tad_cert - add certificate to RMX from a file\n"
        << std::endl
        << "SYNOPSIS\n"
        << "\tca add_cert CertMngr TYPE FNAME\n"
        << std::endl
        << "DESCRIPTION\n"
        << "\tTYPE\t1 - Certificate Trust, 2 - Certificate Personal, "
        << "3 - Certificate Revocation\n"
        << std::endl
        << "\tFNAME\tfile names of certificates"
        << std::endl;
}

STATUS CCertMngrManager::HandleTerminalAddCert(CTerminalCommand& cmd,
                                               std::ostream& ans)
{
    if (cmd.GetNumOfParams() < 2)
    {
        ManAddCert(ans);
        return STATUS_OK;
    }

    DWORD type=0;
    std::istringstream buf1(cmd.GetToken(eCmdParam1));
    if (!(buf1 >> type))
    {
        ans << "Unable to get certificate type\n";
        ManAddCert(ans);
        return STATUS_OK;
    }

    for (DWORD ix = eCmdParam2; ix < eCmdParam2 + cmd.GetNumOfParams() - 1; ++ix)
    {
        std::string fname;
        std::istringstream buf2(cmd.GetToken(static_cast<eCommandParamsIndex>(ix)));
        if (!(buf2 >> fname))
        {
            ans << "Unable to read a parameter #" << ix << std::endl;
            ManAddCert(ans);
            return STATUS_OK;
        }

        if (ix != eCmdParam2)
            ans << std::endl;

        if (!IsFileExists(fname))
        {
            ans << "Unable to find " << fname;
            continue;
        }

        // Copies file to temporary
        char* dname = strdup(fname.c_str());
        char* bname = basename(dname);
        std::string fname2 = MCU_TMP_DIR;
        fname2.append(bname);
        free(dname);

        // Continues with the copy
        if (!CopyFile(fname, fname2))
        {
            ans << "Unable to copy " << fname << " to " << fname2;
            continue;
        }

        STATUS stat = AddCert(static_cast<eCertificateType>(type),
                              fname2.c_str());

        ans << m_pProcess->GetStatusAsString(stat);

        // Removes the copy
        if (!DeleteFile(fname2))
            ans << "\nUnable to delete " << fname2;
    }

    return STATUS_OK;
}

// Static
void CCertMngrManager::ManVfyCert(std::ostream& ans)
{
    ans << "NAME\n"
        << "\tvfy_cert - verify certificate validity\n\n"
        << "SYNOPSIS\n"
        << "\tca vfy_cert CertMngr HOST_NAME DIFF_DAYS ADD_AA CA_VALIDATION CS_VALIDATION\n"
        << "DESCRIPTION\n"
        << "\tHOST_NAME\tname of MCU host\n\n"
        << "\tDIFF_DAYS\tnumber of days to add or remove to current time\n\n"
        << "\tADD_AA\t\t1 - add active alarms\n\n"
        << "\tCA_VALIDATION\t1 - do CA validation\n"
        << "\tCS_VALIDATION\t1 - do CS validation\n";
}


STATUS CCertMngrManager::HandleTerminalVerifyCert(CTerminalCommand& cmd,
                                                  std::ostream& ans)
{
    if (cmd.GetNumOfParams() < 5)
    {
        ManVfyCert(ans);
        return STATUS_OK;
    }

    std::string host_name;
    std::istringstream buf1(cmd.GetToken(eCmdParam1));
    if (!(buf1 >> host_name))
    {
        ans << "Unable to get MCU host name\n";
        ManVfyCert(ans);
        return STATUS_OK;
    }

	const string &sdays = cmd.GetToken(eCmdParam2);
	
    int days = atoi(sdays.c_str());

    int add_aa;
	const string &sAddAa = cmd.GetToken(eCmdParam3);
	if (sAddAa == "1")
	{
		add_aa = 1;
	}
	else if (sAddAa == "0")
	{
		add_aa = 0;
	}
    else
    {
        ans << "Unable to get Add Active Alarm flag\n";
        ManVfyCert(ans);
        return STATUS_OK;
    }

    int ca_validation;
	const string &sCaValidation = cmd.GetToken(eCmdParam4);
	if (sCaValidation == "1")
	{
		ca_validation = 1;
	}
	else if (sCaValidation == "0")
	{
		ca_validation = 0;
	}
    else
    {
        ans << "Unable to get skip CA validation flag\n";
        ManVfyCert(ans);
        return STATUS_OK;
    }


    int cs_validation;
	const string &sCsValidation = cmd.GetToken(eCmdParam5);
	if (sCsValidation == "1")
	{
		cs_validation = 1;
	}
	else if (sCsValidation == "0")
	{
		cs_validation = 0;
	}
    else
    {
        ans << "Unable to get skip CS validation flag\n";
        ManVfyCert(ans);
        return STATUS_OK;
    }

    // Gets current system time
    time_t tt = time(NULL);
    if ((time_t)-1 == tt)
    {
        ans << "Unable to get current time: " << strerror(errno);
        return STATUS_OK;
    }

    struct tm* lt = localtime(&tt);
    if (NULL == lt)
    {
        ans << "Unable to get local time: " << strerror(errno);
        return STATUS_OK;
    }

    CStructTm now = CStructTm(*lt)
                  + CStructTm(0, 1, 1900, 0, 0, 0)
                  + CStructTm(days, 0, 0, 0, 0, 0);

    ans << "Pseudo current time is " << now << std::endl;

    std::string buf;
    STATUS stat = m_pProcess->VerifyCertificates(host_name.c_str(),
                                                 now,
                                                 add_aa,
                                                 ca_validation,
                                                 ca_validation,
                                                 buf);

    ans << m_pProcess->GetStatusAsString(stat);
    if (!buf.empty())
        ans << ":\n" << buf;

    if (cs_validation)
    {
		std::string bufCS;
		STATUS statCS = m_pProcess->VerifyCSServiceCertificates("",
													 now,
													 add_aa,
													 false,
													 0,
													 0,
													 bufCS);
		ans << "\n cs validation status: " <<  m_pProcess->GetStatusAsString(statCS);
		if (!bufCS.empty())
		{
			ans << ":\n" << bufCS;
		}
    }

    return STATUS_OK;
}

STATUS CCertMngrManager::HandleTerminalRemoveAACert(CTerminalCommand& cmd,
                                                    std::ostream& ans)
{
    if (cmd.GetNumOfParams() > 0)
    {
        ans << "NAME\n"
            << "\trma_cert - remove active alarms of certificate validity\n"
            << std::endl
            << "SYNOPSIS\n"
            << "\tca rma_cert CertMngr"
            << std::endl;

        return STATUS_OK;
    }

    std::string buf;
    STATUS stat = m_pProcess->RemoveActiveAlarm(buf, true);
    ans << m_pProcess->GetStatusAsString(stat);
    if (!buf.empty())
        ans << ":\n" << buf;

    return STATUS_OK;
}

void CCertMngrManager::OnGetCertDetails(CSegment* pMsg)
{
    CSegment* seg = new CSegment;
    std::string host_name;
    CStructTm cert_start_date;
    CStructTm cert_expiration_date;
    std::string errror_message;
    
    STATUS stat = GetCertificateDetails(host_name,
                                        cert_start_date,
                                        cert_expiration_date,
                                        errror_message);

    *seg << host_name;
    seg->Put((BYTE*)&cert_start_date,
             sizeof(CStructTm));
    
    seg->Put((BYTE*)&cert_expiration_date,
             sizeof(CStructTm));
    
    *seg << errror_message;
    
    ResponedClientRequest(stat, seg);
}
void CCertMngrManager::OnVerifyCSCertificate(CSegment* msg)
{
	 CStructTm now;
	 SystemGetTime(now);

	 int numServ = m_pProcess->GetNumOfIpServices();
	 TRACEINTOFUNC << "OnVerifyCSCertificate: now time: " << now << " number services: " << numServ;
	 std::string buf;
	 for(int i=0;i < numServ; i++)
	 {
	  	  std::string srvName = m_pProcess->GetServiceName(i);
	   	  BOOL is_secure_service =m_pProcess->GetServiceIsSecured(i);
	   	  BOOL is_ca_validation = m_pProcess->GetIsRequestPeerCertificate(i);
	   	  BYTE revocation_method = m_pProcess->GetServiceRevocationMethod(i);
	   	  BYTE isMSService 		=  m_pProcess->GetIsMsService(i);
	   	  m_pProcess->VerifyCSServiceCertificates(srvName,now,is_secure_service,is_ca_validation,revocation_method,isMSService,buf);

	 }
}

void CCertMngrManager::OnVerifyCertificate(CSegment* msg)
{
  // Reads MCU host name, add AA flag and skip CA validation flag
  std::string host_name;
  unsigned int add_aa;
  unsigned int ca_validation;
  BYTE         revocation_method;
  *msg >> host_name >> add_aa >> ca_validation >> revocation_method;

  // Reads MCU time
  CStructTm now;
  now.DeSerialize(NATIVE, *msg);

  std::string buf;
  STATUS stat = m_pProcess->VerifyCertificates(host_name.c_str(),
                                               now,
                                               add_aa ? true : false,
                                               ca_validation ? true : false,
                                               revocation_method,
                                               buf);


/*

*/
  CSegment* seg = new CSegment;
  *seg << buf;


  STATUS stat2 = ResponedClientRequest(stat, seg);
  PASSERTSTREAM(STATUS_OK != stat2,
      "Unable to send a message: " << m_pProcess->GetStatusAsString(stat));
}

STATUS CCertMngrManager::OnRemoveActiveAlarm(CSegment*)
{

	std::string buf;
    STATUS stat = m_pProcess->RemoveActiveAlarm(buf, false);

    TRACEINTOFUNC << m_pProcess->GetStatusAsString(stat) << ": "
                  << buf;

    return STATUS_OK;
}

void CCertMngrManager::OnMultipleServicesInd(CSegment* pParam)
{
	*pParam >> m_bSystemMultipleServices;

	TRACESTR(eLevelInfoNormal) << "\nCCertMngrManager::OnMultipleServicesInd"
						   << "\nMultiple services: : "
						   << (YES == m_bSystemMultipleServices ? "YES" : "NO");
}

//////////////////////////////////////////////////////////////////////
STATUS CCertMngrManager::OnCsIpServiceParamsInd(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCertMngrManager::OnCsIpServiceParamsInd";
    const IP_SERVICE_CERTMNGR_S *pParam = (IP_SERVICE_CERTMNGR_S*)pMsg->GetPtr();
    const char *ptr = (pParam->OcspURL)? pParam->OcspURL : "empty";
    std::string sOcspUrl = (pParam->OcspURL)? pParam->OcspURL : "";
//    TRACEINTO << "\nCCertMngrManager::HandleCsIpInd\n" << CSnmpCSInfoWrapper(*pParam);
    if (pParam->ServId <= MAX_NUMBER_OF_SERVICES_IN_RMX_4000)
    {
    	checkForUpgradeConflicts(pParam->ServId);
    	m_pProcess->SetServiceName(pParam->ServId-1, pParam->ServName, pParam->IsSecured, pParam->IsRequestPeerCertificate, pParam->HostName,
    								pParam->Revocation_method,sOcspUrl,pParam->IsMsService);
    }
    else
    	TRACESTR(eLevelInfoNormal) << "\nCCertMngrManager::OnCsIpServiceParamsInd - Error in Service ID!!!";
    
	RemoveActiveAlarmByErrorCode(AA_NO_IP_SERVICE_PARAMS);

//    PrintServiceList();

    return STATUS_OK;
}
//

//////////////////////////////////////////////////////////////////////
STATUS CCertMngrManager::OnCsIpServiceParamsEndInd(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCCertMngrManager::OnCsIpServiceParamsEndInd";
/*    const IP_SERVICE_CERTMNGR_S *pParam = (IP_SERVICE_CERTMNGR_S*)pMsg->GetPtr();

//    TRACEINTO << "\nCCertMngrManager::HandleCsIpInd\n" << CSnmpCSInfoWrapper(*pParam);
    if (pParam->ServId <= MAX_NUMBER_OF_SERVICES_IN_RMX_4000)
    	m_pProcess->SetServiceName(pParam->ServId-1, pParam->ServName);
    else
    	TRACESTR(eLevelInfoNormal) << "\nCCertMngrManager::OnCsIpServiceParamsEndInd - Error in Service ID!!!";
*/
	if (0 == m_pProcess->GetNumOfIpServices())
	{
		UpdateStartupConditionByErrorCode(AA_NO_IP_SERVICE_PARAMS, eStartupConditionFail);
	}

    PrintServiceList();

    return STATUS_OK;
}

void CCertMngrManager::PrintServiceList()
{
	std::string msg = "\nCCertMngrManager::PrintServiceList()";
	
	for (int i = 0; i < MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
	{
		char slot_id[2];
		snprintf(slot_id, sizeof(slot_id), "%d", i+1);
		
		std::string service_name = m_pProcess->GetServiceName(i);
		if (service_name=="")
		{
			msg += "\nslot ";
			msg += slot_id;
			msg += " is empty";
		}
		else
		{
			msg += "\nin slot ";
			msg += slot_id;
			msg += " Service Name is ";
			msg += service_name;
		}
	}
	
	TRACESTR(eLevelInfoNormal) << msg;
}

STATUS CCertMngrManager::OnCsDeleteIpServiceInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CCertMngrManager::OnCsDeleteIpServiceInd");

	STATUS lRemoveStatus = STATUS_OK;

/*	TODO - add remove code when needed
 *
 * Del_Ip_Service_S *param = (Del_Ip_Service_S*)pSeg->GetPtr();
	DWORD serviceID = param->service_id;
	lRemoveStatus = m_pProcess->removeIpService(serviceID);
	if (lRemoveStatus != STATUS_OK)
      PTRACE(eLevelError, "CConfPartyManager::OnConfDeleteIpeServiceParamsInd - Deleted IP service was not found in ConfParty List ");*/
    if (0 == m_pProcess->GetNumOfIpServices())
    	AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
						AA_NO_IP_SERVICE_PARAMS,
						MAJOR_ERROR_LEVEL,
						"IP Network Service parameters missing",
						true,
						false
					);

	return lRemoveStatus;
}

// Static
bool CCertMngrManager::IsCRLFile(const char* name)
{
    if (NULL == name)
        return false;

    // Apache puts all CRL files under the path
    static const std::string path = HOME_CRL;
    return 0 == strncmp(name, path.c_str(), strlen(path.c_str()));
}

// Static
bool CCertMngrManager::IsCTLFile(const char* name)
{
    if (NULL == name)
        return false;

    // Apache puts all CTL files under the path
    static const std::string path = HOME_CERT;
    return 0 == strncmp(name, path.c_str(), strlen(path.c_str()));
}


bool CCertMngrManager::IsCSPFXFile(const char* name, string& subFolderName, string& fileName)
{
	if(name ==NULL || strlen(name) <  (strlen(HOME_CS.c_str()) + strlen(PFX_EXTENSION) + 1) ||
			0 != strncmp(name + strlen(name) - strlen(PFX_EXTENSION) , PFX_EXTENSION, strlen(PFX_EXTENSION) ))
	{
		return false;
	}
	char*  startCSDir =  (char*)strstr(name, HOME_CS.c_str());
	int lenFolderName = 0;
	int lenFileName = 0;


	if (startCSDir != NULL && startCSDir < name + strlen(name) -1  )
	{
		char* endCSDir = strstr(startCSDir + strlen(HOME_CS.c_str()), "/");
        if(endCSDir != NULL && endCSDir  < name + strlen(name))
        {
            int lensubFolderName = (int)strlen(name) - (int)strlen(endCSDir) - strlen(HOME_CS.c_str())+  1;
            for (int i = 0; i < lensubFolderName; ++i)
            {
            	subFolderName += (name +strlen(HOME_CS.c_str())) [i];
            }
            for (int i = 1; i < (int)strlen(endCSDir); ++i)
            {
                    fileName += endCSDir[i];
            }
        }
	}

	return ( subFolderName.length() > 0 && fileName.length() > 0 );
}

void CCertMngrManager::OnHttpdCertMngrLinkFile(CSegment* pSeg)
{
    COsQueue mbx;
    mbx.DeSerialize(*pSeg);

    std::string fname;
    *pSeg >> fname;


    eCertificateType type =
        IsCRLFile(fname.c_str()) ? eCertificateRevocation :
            IsCTLFile(fname.c_str()) ? eCertificateTrust :
                eCertificateTypeUnknown;

    OPCODE ret = STATUS_LINK_FILE_SUCCEEDED;

    // Doesn't process unknown files
    if (eCertificateTypeUnknown != type)
    {
        STATUS stat = AddCert(type, fname.c_str());
        if (STATUS_OK != stat)
            ret = STATUS_LINK_FILE_FAILED;

        // Removes file anyway
        BOOL res = DeleteFile(fname);
        PASSERTSTREAM(!res, "DeleteFile: " << fname << ": " << strerror(errno));
    }
    else
    {
    	ret = TryPFX(fname);
    }

    CTaskApi api;
    api.CreateOnlyApi(mbx);
    *m_pClientRspMbx = api.GetRcvMbx();

    STATUS stat = ResponedClientRequest(ret, NULL);
    FPASSERTSTREAM(STATUS_OK != stat,
        "Unable to send a message: " << m_pProcess->GetStatusAsString(stat));
}


OPCODE CCertMngrManager::TryPFX(std::string fname)
{
	string folderName = "";
	folderName = "";
	string subfolderName;
	string fileName;
	OPCODE retOpCode = STATUS_LINK_FILE_SUCCEEDED;

	if (IsCSPFXFile(fname.c_str(), subfolderName, fileName))
	{
		folderName = HOME_CS + subfolderName;
		if (OpenPFXFile(folderName, fname) != STATUS_OK)
		{

			retOpCode = STATUS_LINK_FILE_FAILED;
			pfxCleanUp(folderName,false);
		}
		else
		{

			pfxCleanUp(folderName,true);
		}

	}
	if(STATUS_LINK_FILE_SUCCEEDED == retOpCode)
	{
		AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
									AA_CERTIFICATE_FOR_CS_NEED_RESET_TO_TAKE_EFFECT,
									MAJOR_ERROR_LEVEL,
                                    GetAlarmDescription(AA_CERTIFICATE_FOR_CS_NEED_RESET_TO_TAKE_EFFECT),
									true,
									true);
	}
	return retOpCode;
}
void CCertMngrManager::pfxCleanUp(std::string folderName,bool success)
{
	if(success)
	{
		CopyFile(folderName  + "temp_private_for_cs.pem", folderName +  "pkey.pem");
		CopyFile(folderName + "temp_cert_off_for_cs.pem", folderName + "cert.pem" );
		string certPasswordFile = folderName + CS_PFX_PASSWORD_FILE_NAME;
		if (IsFileExists(certPasswordFile))
		{
			string origCertPasswordFile = folderName + ORIG_CS_PFX_PASSWORD_FILE_NAME;
			rename(certPasswordFile.c_str(), origCertPasswordFile.c_str() );
		}
	}
	// clean all temporary files.
	remove((folderName + "temp_private_for_cs.pem").c_str());   // delete temporary key file if exist
	remove((folderName + "temp_cert_off_for_cs.pem").c_str()); //  delete temporary certificate file
	remove((folderName + "temp_ca_cert_off_for_cs.pem").c_str()); //  delete temporary ca  file if exist

}

void CCertMngrManager::checkForUpgradeConflicts(int serviceId)
{
	//upgrading from <= 7.8 and cs has certificate can cause to lose original certificate
	//string origPassFile = HOME_CS + "cs"
	std::ostringstream origPassFile;
	origPassFile << HOME_CS << "cs" << serviceId<< "/" << CS_PFX_PASSWORD_FILE_NAME;

	std::ostringstream pfxFile;
	pfxFile << HOME_CS << "cs" << serviceId<< "/pfxCert.pfx";

	if(IsFileExists(origPassFile.str()) && IsFileExists(pfxFile.str()))
	{
		TRACESTR(eLevelInfoNormal) << "\nCCertMngrManager::checkForUpgradeConflicts - confilct is found extract cert from pfx";
		std::ostringstream csFolder;
		csFolder << HOME_CS << "cs" << serviceId << "/";
		std::string sFolder = csFolder.str();
		std::string sPfxFile =pfxFile.str();
		if (OpenPFXFile(sFolder,sPfxFile,FALSE) != STATUS_OK)
		{
			TRACESTR(eLevelInfoNormal) << "\nCCertMngrManager::checkForUpgradeConflicts - failed to open pfx";
			pfxCleanUp(csFolder.str(),false);
		}
		else
			pfxCleanUp(csFolder.str(),true);

	}


}

// Static
const char* CCertMngrManager::AuditOperStr(eCertificateType type)
{
    switch (type)
    {
    case eCertificateTrust:
        return "Send CA certificate.";

    case eCertificateRevocation:
        return "Send CRL certificate.";

    case eCertificatePersonal:
    default:
        FPASSERTSTREAM(true,
           "Unable to get Audit operation string for "
                << CertificateTypeToStr(type));
    }

    return "";
}

// Static
const char* CCertMngrManager::AuditDescStr(eCertificateType type,
                                           eAuditEventStatus stat)
{
    switch (type)
    {
    case eCertificateTrust:
        return (eAuditEventStatusOk == stat) ?
            "TLS/SSL CA certificate was sent to the RMX successfully." :
            "Failed to send TLS/SSL CA certificate to the RMX.";

    case eCertificateRevocation:
        return (eAuditEventStatusOk == stat) ?
            "TLS/SSL CRL certificate was sent to the RMX successfully." :
            "Failed to send TLS/SSL CRL certificate to the RMX.";

    case eCertificatePersonal:
    default:
        FPASSERTSTREAM(true,
           "Unable to get Audit description string for " << CertificateTypeToStr(type));
    }

    return "";
}

STATUS CCertMngrManager::AuditHttpCertificatesOperations(eCertificateType type,
                                                         eAuditEventStatus eventStatus)
{
	AUDIT_EVENT_HEADER_S outAuditHdr;
	CAuditorApi::PrepareAuditHeader(outAuditHdr,
									"",
									eMcms,
									"",
									"",
									eAuditEventTypeHttp,
									eventStatus,
									AuditOperStr(type),
									AuditDescStr(type, eventStatus),
									"",
									"");
	CFreeData freeData;
	CAuditorApi::PrepareFreeData(freeData,
								 "",
								 eFreeDataTypeXml,
								 "",
								 "",
								 eFreeDataTypeText,
								 "");
	CAuditorApi api;
	STATUS stat = api.SendEventMcms(outAuditHdr, freeData);
	FPASSERTSTREAM(STATUS_OK != stat,
	    "Unable to report audit log: " << m_pProcess->GetStatusAsString(stat));

	return stat;
}


STATUS CCertMngrManager::HandleFinishUploadCertificate(CRequest* req)
{
	STATUS stat = STATUS_OK;
	req->SetConfirmObject(new CCertificateStatus(m_certUploadStatus));
    req->SetStatus(stat);
	return STATUS_OK;
}
