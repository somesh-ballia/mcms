#ifndef SSLINTERFACE_H_
#define SSLINTERFACE_H_

#include <string>
#include <list>
#include <openssl/ssl.h>
#include "PObject.h"
#include "CertMngrDefines.h"

class CCertificateRequest;

typedef struct pw_cb_data
{
	const void *password;
	const char *prompt_info;
} PW_CB_DATA;
typedef std::pair<std::string,std::string> knownAlterNames;
class CSslInterface
{
public:
	static void print_err_trace(char* str);
	static int CreateSslCertificateRequest(CCertificateRequest* cert_req, BOOL bForCS=FALSE);
	static STATUS SaveSslCertificate(const char* certificate,
                                     const char* host_name,
                                     eCertificateType certificate_type, std::string folder_name="");
	static STATUS SaveSslCertificateEx(const char* certificate,
	                                         const char* host_name,
	                                         eCertificateType type,std::string folder_name="");

	static void CloseAll(EVP_PKEY* pkey, X509_REQ* req, BIO* out);
	static std::string GetFolderName(std::string service_name);
	static void ConvertPrivateKey();

private:
    static int make_REQ(X509_REQ *req, EVP_PKEY *pkey, CCertificateRequest* cert_req);
    static int auto_info(X509_REQ *req, CCertificateRequest* cert_req);
    static STATUS HandleMultipleOrganizationUnits(X509_REQ *req, CCertificateRequest* cert_req, X509_NAME *subj, const std::string sOrgUnit);
    static int CreateSslPrivateKey(EVP_PKEY** pkey, BOOL bForCS=FALSE, std::string folder_name="");
    static void DeleteCertificateTempFiles();
    static void DeleteCertificateTempFilesForCS();
    static void BuildKnownListAltNames(std::list<knownAlterNames>& knowNames);
    static void ParseSANFields(char *pBuffStr,std::list<std::string>& sanlist,std::list<knownAlterNames>& knowNames);
};
	
#endif /*SSLINTERFACE_H_*/
