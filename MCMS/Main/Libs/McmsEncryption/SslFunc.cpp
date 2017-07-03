// SSLInterfcae.cpp

#include <algorithm>
// fipssyms.h should be first to take FIPS version of EVP_MD_CTX_init...
#include <openssl/fipssyms.h>

#include "SslFunc.h"
#include <openssl/x509v3.h>

#include "Trace.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "OsFileIF.h"
#include "StructTm.h"
#include "Macros.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"

#define DAYS_BEFORE_CERTIFICATE_EXPIRED_ALERT 14

// Static.
std::string CSslFunctions::SSLErrMsg()
{
  // Loads error string table only before usage.
  SSL_load_error_strings();

  int line;
  int flag;
  const char* file;
  const char* data;
  unsigned long err;
  std::ostringstream buf;

  while ( (err = ERR_get_error_line_data(&file, &line, &data, &flag)) )
  {
    buf << std::endl
        << file << ": " << line << ": "
        << ERR_lib_error_string(err) << ": "
        << ERR_func_error_string(err) << ": "
        << ERR_reason_error_string(err);

    if (flag & ERR_TXT_STRING)
      buf << ", " << data;
  }

  // Frees error string table.
  ERR_free_strings();

  return buf.str();
}

// Static
STATUS CSslFunctions::GetSSLPassPhrase(const char* fname, std::string& out)
{
  FPASSERTSTREAM_AND_RETURN_VALUE(!IsFileExists(fname),
      "IsFileExists: " << fname << ": No such file",
      STATUS_FILE_NOT_EXISTS);

  std::string ans;
  std::ostringstream cmd;
  cmd << fname << " 2>&1";

  STATUS ret = SystemPipedCommand(cmd.str().c_str(), ans);
  FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != ret,
      "SystemPipedCommand: " << cmd.str() << ": " << ans,
      STATUS_FAIL);

  out = ans;
  return STATUS_OK;
}

// Verifies the private key with current passphrase, uses link.
static int pass_helper(const char* fname,
                       char* buf,
                       int size,
                       int rwflag,
                       void* password)
{
  std::string pass;
  STATUS stat = CSslFunctions::GetSSLPassPhrase(fname, pass);
  if (STATUS_OK != stat)
    return 0;

  strncpy(buf, pass.c_str(), size - 1);
  buf[size - 1] = '\0';

  return strlen(buf);
}

// Static, Public
int CSslFunctions::pem_passwd_cb(char* buf, int size, int rwflag, void* password)
{
  // Verifies the private key with default passphrase, uses MAC address file
  return pass_helper(SSL_PASS_PHRASE_DIALOG_LINK.c_str(), buf, size, rwflag, password);
}

static int pem_passwd_link_cb(char* buf, int size, int rwflag, void* password)
{
  // Verifies the private key with current passphrase, uses link
  return pass_helper(SSL_PASS_PHRASE_DIALOG_LINK.c_str(), buf, size, rwflag, password);
}

// Static
void CSslFunctions::print_err_trace(const char* str)
{
  FTRACEINTO << str << ": " << SSLErrMsg();
}

void CSslFunctions::print_trace(const char* str)
{
  FTRACEINTO << str;
}

int CSslFunctions::CheckIfCertificateExist()
{
  const int fileSize = GetFileSize(CERTF.c_str());
  const int KeyfileSize = GetFileSize(KEYF.c_str());
  const int KeyDes3fileSize = GetFileSize(KEYF_DES3.c_str());

  FTRACEINTOFUNC << "CSslFunctions::CheckIfCertificateExist fileSize " << fileSize << " KeyfileSize " << KeyfileSize << " KeyDes3fileSize " << KeyDes3fileSize ;

  BYTE bCreatePem3Link = TRUE;

  if (fileSize == -1 || ((KeyfileSize == -1) && (KeyDes3fileSize == -1)))
  {
    if (fileSize == -1)
      print_err_trace("certificate file does not exist");
    else
      print_err_trace("private key does not exist");

    return STATUS_CAN_NOT_SECURE_CONNECTION_WITHOUT_CERTIFICATE;
  }

  // old private key exist - rename to new name in non JITC mode
  if ((KeyfileSize != -1) && (KeyDes3fileSize == -1))
  {
    // in JITC mode we MUST have private key encrypted with des3
    if (IsJitcMode())
    {
      FTRACEINTOFUNC << "Need a new certificate using des3 for FIPS and JITC";
      return STATUS_CAN_NOT_SECURE_CONNECTION_WITHOUT_CERTIFICATE;
    }
    else
    {
      FTRACEINTOFUNC << "Working with old private key for non JITC systems";
      bCreatePem3Link = FALSE;
    }
  }

  remove(COMMON_KEY_F.c_str());

  if (bCreatePem3Link == TRUE)
    CreateSymbolicLink(KEYF_DES3, COMMON_KEY_F.c_str());
  else
    CreateSymbolicLink(KEYF, COMMON_KEY_F.c_str());

  

  // Distinguishes passphrase script by private key file size
  // 1041 bytes means it was created with polycom password, 1834 means is was
  // created with MAC-address password
  int res = GetFileSize(COMMON_KEY_F);

  if( 1041 == res)
  {
         remove(SSL_PASS_PHRASE_DIALOG_LINK.c_str());
	 rename(SSL_PASS_PHRASE_DIALOG_PLCM.c_str(),SSL_PASS_PHRASE_DIALOG_LINK.c_str());
  }	
  else
	 remove(SSL_PASS_PHRASE_DIALOG_PLCM.c_str());
 

  return STATUS_OK;
}

// Static
BYTE CSslFunctions::IsJitcMode()
{
	BYTE bJitcMode = FALSE;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

	return bJitcMode;
}

int CSslFunctions::CheckValidationOfCerificateAndPrivateKey()
{
	SSL_METHOD *meth;
	SSL_CTX* ctx;

	if (CheckIfCertificateExist()==STATUS_CAN_NOT_SECURE_CONNECTION_WITHOUT_CERTIFICATE)
		return STATUS_CAN_NOT_SECURE_CONNECTION_WITHOUT_CERTIFICATE;

	meth = (SSL_METHOD*)SSLv23_server_method();
	ctx = SSL_CTX_new (meth);

	if (ctx == NULL)
	{
		print_err_trace("CSslFunctions::CheckValidationOfCerificateAndPrivateKey: error in creating ctx");
		return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
	}

	if (SSL_CTX_use_certificate_file(ctx,CERTF.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		print_err_trace("CSslFunctions::CheckValidationOfCerificateAndPrivateKey: error in reading certificate");
		SSL_CTX_free(ctx);
		return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
	}

	SSL_CTX_set_default_passwd_cb(ctx, pem_passwd_link_cb);

    std::string key_file = COMMON_KEY_F;
    if (IsJitcMode())
        key_file = KEYF_DES3.c_str();

	if (SSL_CTX_use_PrivateKey_file(ctx, key_file.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		print_err_trace("CSslFunctions::CheckValidationOfCerificateAndPrivateKey: error in reading private key");
		SSL_CTX_free(ctx);
		return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
	}

	if (!SSL_CTX_check_private_key(ctx))
	{
		//private key doesn't match the certificate public key
		print_err_trace("CSslFunctions::CheckValidationOfCerificateAndPrivateKey: certificate doesn't match the private key");
		SSL_CTX_free(ctx);
		return STATUS_SSL_CERTIFICATE_DOES_NOT_MATCH_THE_PRIVATE_KEY;
	}

	return STATUS_OK;
}


int CSslFunctions::GetCertificate(X509** cert)
{
	FILE *fp;

	// first read the client certificate
	if (!(fp = fopen(CERTF.c_str(), "r")))
	{
	  FTRACEINTOFUNC << "Error reading client certificate file";
		return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
	}

	if (!(*cert = PEM_read_X509(fp, NULL, NULL, NULL)))
	{
		fclose(fp);
		print_err_trace("CSslFunctions::GetCertificate - Error reading client certificate file");
		return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
	}
	fclose(fp);

	return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////

int CSslFunctions::CheckCertificateCommonName(X509* cert,
                                              const char* host_name)
{
  FPASSERT_AND_RETURN_VALUE(NULL == host_name, STATUS_INCONSISTENT_PARAMETERS);
  FPASSERTMSG_AND_RETURN_VALUE(host_name[0] == '\0',
      "MCU host name is empty. Check Embedded status.",
      STATUS_INCONSISTENT_PARAMETERS);

	char buf[1025];
	int ret = X509_NAME_get_text_by_NID(X509_get_subject_name(cert),
                                        NID_commonName, buf,
                                        ARRAYSIZE(buf) - 1);

	FPASSERTSTREAM_AND_RETURN_VALUE(ret < 1,
	    "Unable to get common name: " << SSLErrMsg(),
	    STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

	FTRACEINTOFUNC << "Common name is " << buf;

	buf[ARRAYSIZE(buf) - 1] = '\0';

  //remove \n
  if (buf[strlen(buf) - 1] == '\n')
    buf[strlen(buf) - 1] = '\0';

	if (strncmp(host_name, buf, ARRAYSIZE(buf)) != 0)
  {
    FPASSERTSTREAM(true, "Common name " << buf
        << " differs than host name " << host_name);
    return STATUS_CERTIFICATE_COMMON_NAME_DIFFER_THAN_RMX_HOST_NAME;
  }

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
ASN1_TIME* CSslFunctions::GetCertificateExpirationDate(X509 *cert)
{
	return X509_get_notAfter(cert);
}

////////////////////////////////////////////////////////////////////////////////
ASN1_TIME* CSslFunctions::GetCertificateStartDate(X509 *cert)
{
	return X509_get_notBefore(cert);
}



////////////////////////////////////////////////////////////////////////////////
int CSslFunctions::CheckCertificateExpirationDate(ASN1_TIME* not_after,
                                                  const CStructTm* pStructTm,
                                                  int& days_diff)
{
	//check if the certificate already expired
	int result = X509_cmp_current_time(not_after);

	FTRACEINTOFUNC << "Result is " << result;
	if (result<=0)
	{
	    FTRACEINTOFUNC << "Certificate has expired";
		return STATUS_CERTIFICATE_ALREADY_EXPIRED;
	}

	if (pStructTm!=NULL)
	{
		//check if the certificate is going to be expired in the next 2 weeks
		char* str = (char*)not_after->data;	//the date is yy mm dd hh mm ss z(year month day hour minute second 'Z' for UTC)

		tm tmpTime;
		tmpTime.tm_sec 	= (str[10]-'0')+10+str[11]-'0';
		tmpTime.tm_min 	= (str[8]-'0')+10+str[9]-'0';
		tmpTime.tm_hour = (str[6]-'0')*10+str[7]-'0';
		tmpTime.tm_mday = (str[4]-'0')*10+str[5]-'0';
		tmpTime.tm_mon 	= (str[2]-'0')*10+str[3]-'0'-1;; //0..11 month
		tmpTime.tm_year = (str[0]-'0')*10+str[1]-'0' + 100;	//year start in 1900.
		tmpTime.tm_wday  = 0;
		tmpTime.tm_yday  = 0;
		tmpTime.tm_isdst = 0;

		time_t not_after_time = mktime(&tmpTime);

		DWORD diff = (not_after_time > pStructTm->GetAbsTime() ? not_after_time - pStructTm->GetAbsTime() : 0);

		days_diff = ((diff/60)/60)/24;	//calculate the seconds in days

		FTRACESTR(eLevelInfoNormal) << "CSslFunctions::CheckCertificateExpirationDate - str = "<<str<<" days_diff = "<<days_diff;

		if (days_diff<DAYS_BEFORE_CERTIFICATE_EXPIRED_ALERT)
			return STATUS_CERTIFICATE_IS_GOING_TO_BE_EXPIRED;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
void CSslFunctions::init_OpenSSL()
{
    // The function doesn't have error indication
    SSL_library_init();
}

////////////////////////////////////////////////////////////////////////////////
void CSslFunctions::SHA1_Encryption(std::string& source, std::string& dest)
{
	unsigned char encPwd[21];
	memset(encPwd, '\0', 21);

	const EVP_MD* sha1_type = EVP_sha1();
	EVP_MD_CTX ctx;
	EVP_MD_CTX_init(&ctx);
	EVP_DigestInit(&ctx, sha1_type);
	EVP_DigestInit_ex(&ctx, sha1_type, NULL);

	EVP_DigestUpdate(&ctx, source.c_str(), strlen(source.c_str()));
	EVP_DigestFinal_ex(&ctx, encPwd, NULL);
	EVP_MD_CTX_cleanup(&ctx);

	dest = ToBase64(encPwd, 20);
}

////////////////////////////////////////////////////////////////////////////////
void CSslFunctions::SHA256_Encryption(std::string& source, std::string& dest)
{
	unsigned char encPwd[33];
	memset(encPwd, '\0', 33);

	const EVP_MD* sha256_type = EVP_sha256();
	EVP_MD_CTX ctx;
	EVP_MD_CTX_init(&ctx);
	EVP_DigestInit(&ctx, sha256_type);
	EVP_DigestInit_ex(&ctx, sha256_type, NULL);

	EVP_DigestUpdate(&ctx, source.c_str(), strlen(source.c_str()));
	EVP_DigestFinal_ex(&ctx, encPwd, NULL);
	EVP_MD_CTX_cleanup(&ctx);

	dest = ToBase64(encPwd, 32);
}

////////////////////////////////////////////////////////////////////////////////
void CSslFunctions::SHA1_Encryption(const char* file_name, unsigned char* out )
{
	SHA_CTX sha;
	SHA1_Init(&sha);
	FILE* f = fopen(file_name ,"rb");
	if(f == NULL)
		return;
	char buf[1024];
	int length = 0;
	while((length = fread(buf, 1, 1024, f)))
	{
		SHA1_Update(&sha, buf, length);
	}
	fclose(f);
	SHA1_Final(out, &sha);
}

////////////////////////////////////////////////////////////////////////////////
void CSslFunctions::SHA256_Encryption(const char* file_name, unsigned char* out )
{
	SHA256_CTX sha;
	SHA256_Init(&sha);
	FILE* f = fopen(file_name ,"rb");
	if(f == NULL)
		return;
	char buf[1024];
	int length = 0;
	while((length = fread(buf, 1, 1024, f)))
	{
		SHA256_Update(&sha, buf, length);
	}
	fclose(f);
	SHA256_Final(out, &sha);
}


////////////////////////////////////////////////////////////////////////////////
std::string CSslFunctions::ToBase64(const unsigned char *input, int length)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	ALLOCBUFFER(buff, bptr->length);
	memcpy(buff, bptr->data, bptr->length-1);
	buff[bptr->length-1] = '\0';

	BIO_free_all(b64);

	std::string result = buff;
	DEALLOCBUFFER(buff);

	return result;
}

////////////////////////////////////////////////////////////////////////////////
std::string CSslFunctions::FromBase64(const unsigned char *input, int length)
{
	BIO *bmem,*b64;
	BUF_MEM *bptr;
	   
    bmem = BIO_new_mem_buf((void*)input, length);        
    b64 = BIO_new(BIO_f_base64());      
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_push(b64, bmem);
    
    BIO_get_mem_ptr(bmem, &bptr);    
    ALLOCBUFFER(buff, bptr->length);
    
    BIO_read(bmem, buff, length);
    BIO_free_all(bmem);
   
    std::string result = buff;    
	DEALLOCBUFFER(buff);
	
	return result;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CSslFunctions::GetCertificateDetails(std::string & host_name,
                                            CStructTm & cert_start_date,
                                            CStructTm & cert_expiration_date,
                                            std::string& errror_message)
{
    X509 *cert = NULL;
    STATUS ret = CSslFunctions::GetCertificate(&cert);

    if (ret != STATUS_OK)
    {
        errror_message = "Security mode error - failed reading certificate";
        return ret;
    }

    char buf[1025];
	if (0 ==   X509_NAME_get_text_by_NID(X509_get_subject_name(cert),
                                         NID_commonName,
                                         buf,
                                         ARRAYSIZE(buf) - 1))
    {
        errror_message = "failed reading host name from certificate";
        return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
    }

    if (buf[strlen(buf)-1]=='\n')  //remove \n
        buf[strlen(buf)-1]='\0';

    host_name = buf;


    ////////////// READ START DATE ///////////////////

    ASN1_TIME* start_date = CSslFunctions::GetCertificateStartDate(cert);
    if (start_date == NULL)
    {
        errror_message = "Security mode error - failed reading certificate start date";
        return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
    }

    ret = ConvertAns1TimeToStructTm(start_date, cert_start_date);
    if (ret != STATUS_OK)
    {
        errror_message = "Security mode error - failed parsing certificate start date";
        return ret;
    }

     ////////////// READ END DATE ///////////////////

    ASN1_TIME* end_date = CSslFunctions::GetCertificateExpirationDate(cert);
    if (end_date == NULL)
    {
        errror_message = "Security mode error - failed reading certificate end date";
        return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
    }


    ret = ConvertAns1TimeToStructTm(end_date, cert_expiration_date);
    if (ret != STATUS_OK)
    {
        errror_message = "Security mode error - failed parsing certificate end date";
        return ret;
    }

	return STATUS_OK;
}

// http://www.mail-archive.com/openssl-users@openssl.org/msg33365.html
static time_t GetTimeFromASN1(const ASN1_TIME* aTime)
{
    time_t lResult = 0;

    char lBuffer[24];
    char * pBuffer = lBuffer;

    size_t lTimeLength = aTime->length;
    char * pString = (char *)aTime->data;


    if (aTime->type == V_ASN1_UTCTIME)
    {
        if ((lTimeLength < 11) || (lTimeLength > 17))
        {
        	FTRACESTR(eLevelInfoNormal) << "failed parse time from ans1 - lTimeLength " << lTimeLength;
            return 0;
        }

        memcpy(pBuffer, pString, 10);
        pBuffer += 10;
        pString += 10;
    }
    else
    {
        if (lTimeLength < 13)
        {
        	FTRACESTR(eLevelInfoNormal) << "failed parse time from aTime->type != V_ASN1_UTCTIME - lTimeLength " << lTimeLength;
            return 0;
        }

        memcpy(pBuffer, pString, 12);
        pBuffer += 12;
        pString += 12;
    }

    if ((*pString == 'Z') || (*pString == '-') || (*pString == '+'))
    {
        *(pBuffer++) = '0';
        *(pBuffer++) = '0';
    }
    else
    {
        *(pBuffer++) = *(pString++);
        *(pBuffer++) = *(pString++);
        // Skip any fractional seconds...
        if (*pString == '.')
        {
            pString++;
            while ((*pString >= '0') && (*pString <= '9'))
            {
                pString++;
            }
        }
    }

    *(pBuffer++) = 'Z';
    *(pBuffer++) = '\0';

    time_t lSecondsFromUCT;
    if (*pString == 'Z')
    {
        lSecondsFromUCT = 0;
    }
    else
    {
        if ((*pString != '+') && (pString[5] != '-'))
        {
        	FTRACESTR(eLevelInfoNormal) << "failed parse time could not find character + or - *pString" << *pString << " pString[5]=" <<pString[5] ;
            return 0;
        }

        lSecondsFromUCT = ((pString[1]-'0') * 10 + (pString[2]-'0')) * 60;
        lSecondsFromUCT += (pString[3]-'0') * 10 + (pString[4]-'0');
        if (*pString == '-')
        {
            lSecondsFromUCT = -lSecondsFromUCT;
        }
    }

    tm lTime;
    lTime.tm_sec  = ((lBuffer[10] - '0') * 10) + (lBuffer[11] - '0');
    lTime.tm_min  = ((lBuffer[8] - '0') * 10) + (lBuffer[9] - '0');
    lTime.tm_hour = ((lBuffer[6] - '0') * 10) + (lBuffer[7] - '0');
    lTime.tm_mday = ((lBuffer[4] - '0') * 10) + (lBuffer[5] - '0');
    lTime.tm_mon  = (((lBuffer[2] - '0') * 10) + (lBuffer[3] - '0')) - 1;
    lTime.tm_year = ((lBuffer[0] - '0') * 10) + (lBuffer[1] - '0');
    if (lTime.tm_year < 50)
    {
         lTime.tm_year += 100; // RFC 2459
    }
    lTime.tm_wday = 0;
    lTime.tm_yday = 0;
    lTime.tm_isdst = 0;  // No DST adjustment requested

    //on 32bytes system mktime is limited to 2038

    /* Note: we did not adjust the time based on time zone information */
    lResult = mktime(&lTime);

    if ((time_t)-1 != lResult)
    {
        if (0 != lTime.tm_isdst)
        {
            lResult -= 3600; // mktime may adjust for DST (OS dependent)
        }
        lResult += lSecondsFromUCT;
    }
    else
    {
    	FTRACESTR(eLevelInfoNormal) << "failed parse time could not set mktime  lResult =" << lResult
    	                            << "Day :" << lTime.tm_mday << " Month :" <<lTime.tm_mon << " Year :" << lTime.tm_year <<
    								"  Hour : " << lTime.tm_hour << " Min :" << lTime.tm_min << " Sec : " << lTime.tm_sec;

        lResult = 0;
    }

    return lResult;
}

////////////////////////////////////////////////////////////////////////////////
int CSslFunctions::CheckCertificateStartDate(ASN1_TIME* not_before, const CStructTm* pStructTm)
{
	// Checks if the certificate already expired
	//const ASN1_TIME* not_before2;
	time_t rt = GetTimeFromASN1(not_before);
	FTRACEINTOFUNC << "not_before is " << rt;
	int result = X509_cmp_current_time(not_before);
	if (result > 0)
	{
		FTRACEINTOFUNC << "Certificate is not valid yet";
		return STATUS_CERTIFICATE_NOT_VALID_YET;
	}

	return STATUS_OK;
}

// Static
STATUS CSslFunctions::ConvertAns1TimeToStructTm(const ASN1_TIME* tm,
                                                CStructTm& out)
{
	char* oldTZ = getenv("TZ");
	if(eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		putenv("TZ=UTC");
		tzset();
	}
    time_t rt = GetTimeFromASN1(tm);
    FPASSERTSTREAM_AND_RETURN_VALUE(0 == rt,
        "GetTimeFromASN1: " << tm->data, STATUS_FAIL);

	out.SetAbsTime(rt);

	if(eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		char* newTZ = getenv("TZ");
		if(oldTZ == NULL)
		{
			putenv("TZ=");
		}
		else
		{
			char buff[255];
			memset(buff,sizeof(buff),0);
			if(strlen(oldTZ) <= sizeof(buff) - 4)
			{
				snprintf(buff,sizeof(buff)-1,"TZ=%s",oldTZ);
				putenv(buff);
			}
			//else
				//PASSERT(1);
		}
		tzset();
	}
    FTRACEINTOFUNC << "Converted time from " << tm->data << " to " << out;

    return STATUS_OK;
}
