// SslFunc.h

#ifndef SSLFUNCTION_H_
#define SSLFUNCTION_H_

#include <string>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "DataTypes.h"
#include "DefinesGeneral.h"

#define DAYS_BEFORE_CERTIFICATE_EXPIRED_ALERT 14

#define HOME ((std::string)(MCU_MCMS_DIR+"/Keys/"))
#define HOME_CS ((std::string)(MCU_MCMS_DIR+"/KeysForCS/"))
#define HOME_CERT ((std::string)(MCU_MCMS_DIR+"/CACert/"))
#define HOME_CRL ((std::string)(MCU_MCMS_DIR+"/CRL/"))

#define HOME_CONFIG_OCS  ((std::string)(MCU_CONFIG_DIR+"/ocs"))
#define HOME_CONFIG_KEYS ((std::string)(MCU_CONFIG_DIR+"/keys/cs"))

#define TEMP_KEYF ((std::string)(HOME+"temp_private.pem"))
#define TEMP_CERTF ((std::string)(HOME+"temp_cert_off.pem"))
#define TEMP_CERT_REQ_F ((std::string)(HOME+"temp_cert.csr"))


#define TEMP_KEYF_FOR_CS ((std::string)(HOME_CS+"temp_private_for_cs.pem"))
#define TEMP_CERTF_FOR_CS ((std::string)(HOME_CS+"temp_cert_off_for_cs.pem"))
#define TEMP_CERT_REQ_F_FOR_CS ((std::string)(HOME_CS+"temp_cert_for_cs.csr"))

#define CS_PFX_PASSWORD_FILE_NAME  "certPassword.txt"
#define ORIG_CS_PFX_PASSWORD_FILE_NAME  "certPassword.txt.orig"


#define TEMP_CA_CERTIFICATE ((std::string)(MCU_TMP_DIR+"/temp_ca_cert.cer"))
#define TEMP_CERTIFICATE_DETAILS_FILE ((std::string)(MCU_TMP_DIR+"/temp_cert_details.pem"))

#define KEYF ((std::string)(HOME+"private.pem"))
#define CERTF ((std::string)(HOME+"cert_off.pem"))
#define KEYF_DES3 ((std::string)(HOME+"private3.pem"))
#define COMMON_KEY_F ((std::string)(MCU_TMP_DIR+"/privateKey.pem"))
#define KEYF_FOR_CS ((std::string)(HOME_CS+"pkey.pem"))
#define CERTF_FOR_CS ((std::string)(HOME_CS+"cert.pem"))

#define CA_CERTIFICATES_FILE ((std::string)(HOME_CERT+"ca-bundle-client.crt"))
#define CA_CERTIFICATES_XML_FILE ((std::string)(HOME_CERT+"ca-bundle-client.xml"))



#define CRL_CA_FILE ((std::string)(HOME_CRL+"ca.crl"))
#define CRL_DB_FILE ((std::string)(HOME_CRL+"db.xml"))
//change the path due to BRIDGE-5395
#define SSL_PASS_PHRASE_DIALOG_PLCM ((std::string)(MCU_TMP_DIR+"/passphraseplcm.sh"))
#define SSL_PASS_PHRASE_DIALOG_LINK ((std::string)(MCU_TMP_DIR+"/passphrase.sh"))

#define PFX_EXTENSION ".pfx"

class CStructTm;

struct CSslFunctions
{
  static std::string SSLErrMsg();
  static STATUS GetSSLPassPhrase(const char* fname, std::string& out);
  static void print_err_trace(const char* str);
  static void print_trace(const char* str);
  static int pem_passwd_cb(char* buf, int size, int rwflag, void* password);
  static int CheckValidationOfCerificateAndPrivateKey();
  static int CheckCertificateCommonName(X509* cert, const char* host_name);
  static int CheckCertificateExpirationDate(ASN1_TIME* not_after,
                                            const CStructTm* pStructTm,
                                            int& days_diff);
  static int CheckCertificateStartDate(ASN1_TIME* not_before,
                                       const CStructTm* pStructTm);
  static int GetCertificate(X509** cert);
  static int CheckIfCertificateExist();
  static ASN1_TIME* GetCertificateExpirationDate(X509* cert);
  static ASN1_TIME* GetCertificateStartDate(X509* cert);
  static void SHA1_Encryption(std::string& source, std::string& dest);
  static void SHA1_Encryption(const char* file_name, unsigned char* out);
  static void SHA256_Encryption(std::string& source, std::string& dest);
  static void SHA256_Encryption(const char* file_name, unsigned char* out);
  static std::string ToBase64(const unsigned char* input, int length);
  static std::string FromBase64(const unsigned char* input, int length);
  static STATUS GetCertificateDetails(std::string& host_name,
                                      CStructTm& cert_start_date,
                                      CStructTm& cert_expiration_date,
                                      std::string& errror_message);
  static STATUS ConvertAns1TimeToStructTm(const ASN1_TIME* time,
                                          CStructTm& out);

  static BYTE IsJitcMode();
  static void init_OpenSSL();
};

#endif  // SSLINTERFACE_H_
