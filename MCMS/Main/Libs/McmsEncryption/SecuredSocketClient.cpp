
#include "SecuredSocketClient.h"
#include "SecuredSocketConnected.h"
#include "SslFunc.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"
#include "ProcessBase.h"
#include "FaultsDefines.h"


//////////////////////////////////////////////////////////////////
CSecuredSocketClient::CSecuredSocketClient()
        :COsSocketClient()
{
	m_pSsl = NULL;
	m_pCtx = NULL;
	m_isRequestPeerCertificate = FALSE;
}

//////////////////////////////////////////////////////////////////
CSecuredSocketClient::~CSecuredSocketClient()
{
	SSL_free (m_pSsl);
    m_pSsl = NULL;
}

//////////////////////////////////////////////////////////////////
void CSecuredSocketClient::Close()
{
    COsSocketClient::Close();
}


//////////////////////////////////////////////////////////////////
void CSecuredSocketClient::CreateSocketConnected(COsSocketConnected** pConnected)
{
	*pConnected = new CSecuredSocketConnected(128*1024-1,-1);
	(*pConnected)->SetDescriptor(m_descriptor);
	(*pConnected)->SetTlsParams(m_pSsl);
}

//////////////////////////////////////////////////////////////////
STATUS CSecuredSocketClient::ConfigureClientSocket()
{
	int err;
	m_pCtx = NULL;
	m_pSsl = NULL;
		
    SSL_METHOD *meth;
    
    CSslFunctions::init_OpenSSL();
    
    SSLeay_add_ssl_algorithms();
    meth = (SSL_METHOD*)TLSv1_client_method();
    m_pCtx = SSL_CTX_new (meth);
    if (!m_pCtx)
	{
		FPTRACE(eLevelInfoHigh, "CSecuredSocketClient::ConfigureClientSocket - failed creating ctx");
    }
    
	
	return COsSocketClient::ConfigureClientSocket();

}

//////////////////////////////////////////////////////////////////
STATUS CSecuredSocketClient::Connect()
{
    COsSocketClient::Connect();

    if (m_pSsl)
    {
        FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - Release old SSL connection";
        SSL_free (m_pSsl);
        m_pSsl = NULL;
    }
    FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - Start new SSL connection";
    
    if (m_pCtx==NULL)
    	FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - CTX can't be NULL!!";
    
    m_pSsl = SSL_new (m_pCtx);
    if (m_pSsl==NULL)
    {
        FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - Error - Can't start new SSL connection";
        return STATUS_FAIL;
    }
    
    SSL_set_fd (m_pSsl, m_descriptor);
	
    int ret_code, err;
    int end_loop=0;
    do 
    {
        err = SSL_connect (m_pSsl);
        ret_code = SSL_get_error(m_pSsl, err);
        SystemSleep(25);
        end_loop++;
    } while ((ret_code==SSL_ERROR_WANT_READ || ret_code==SSL_ERROR_WANT_WRITE)&&end_loop<30);
	
    //if the ssl connection wasn't succesful...
    if (err<=0)
    {
        FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - ssl connection - Failed!!!: " << ret_code<<" err = "<<err;
        if (ret_code==SSL_ERROR_SYSCALL)
        {
            if (err==0)
            {
                unsigned long errcode;
                const char    *errreason;
                errcode = ERR_get_error();
                errreason = ERR_reason_error_string(errcode);
                FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - errcode = "<< errcode << " errreason = "<<errreason;
            }
        }
        else
            return ret_code;
        SSL_free (m_pSsl);
        m_pSsl = NULL;
    }
    else
    {
        FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - TLS connection has been established, using "<<SSL_get_cipher (m_pSsl);
        
        X509* server_cert;
        server_cert = SSL_get_peer_certificate (m_pSsl);
        if (server_cert == NULL)
        {
            FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - Error in getting server certificate";
            return STATUS_OK;
        }
        char *str = X509_NAME_oneline (X509_get_subject_name (server_cert),0,0);
        if (str == NULL)
        {
            FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - Error in getting subject name";
            return STATUS_OK;				
        }
        FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - certificate subject = "<<str;
        OPENSSL_free (str);
        
        str = X509_NAME_oneline (X509_get_issuer_name  (server_cert),0,0);
        if (str == NULL)
        {
            FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - Error in getting issuer name";
            return STATUS_OK;
        }
        FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - certificate issuer = "<<str;
        OPENSSL_free (str);
        
        X509_free (server_cert);
        
        long verified = X509_V_OK;
        if (m_isRequestPeerCertificate)
        	SSL_get_verify_result(m_pSsl);


        // this code should not be in infrastructure code, should be in application
        CProcessBase *pProcess = (CProcessBase*)CProcessBase::GetProcess();
        
        if (verified == X509_V_OK )
        {
        	string msg = (m_isRequestPeerCertificate) ? "RequestPeerCertificate = YES " : "RequestPeerCertificate = NO ";
            FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - The verification succeeded or no peer certificate was presented" << msg;
            pProcess->RemoveActiveAlarmFromProcess(AA_EXTERNAL_DB_CERTIFICATE_ERROR);
        }
        else
        {
            string description = "Peer certificate has an error";
            description+=X509_verify_cert_error_string(verified);
            pProcess->AddActiveAlarmSingleToneFromProcess(FAULT_GENERAL_SUBJECT,
                                                          AA_EXTERNAL_DB_CERTIFICATE_ERROR,
                                                          MAJOR_ERROR_LEVEL,
                                                          description.c_str(),
                                                          true,
                                                          true);
            
            FTRACESTR(eLevelInfoNormal) << "CSecuredSocketClient::Connect - " << description.c_str();
            return STATUS_FAIL;
        }
    }

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////
void CSecuredSocketClient::SetRequestPeerCertificate(BOOL bisRequestPeerCertificate)
{
	m_isRequestPeerCertificate = bisRequestPeerCertificate;
}
//////////////////////////////////////////////////////////////////
BOOL CSecuredSocketClient::GetRequestPeerCertificate()
{
	return m_isRequestPeerCertificate;
}

