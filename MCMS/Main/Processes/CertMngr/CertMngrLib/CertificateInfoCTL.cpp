// CertificateInfoCTL.cpp

#include "CertificateInfoCTL.h"

#include <algorithm>

#include <openssl/x509v3.h>
#include "OsFileIF.h"
#include "psosxml.h"
#include "Trace.h"
#include "TraceStream.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "SslFunc.h"
#include "SSLInterface.h"
#include "CertMngrProcess.h"
#include "OpcodesMcmsInternal.h"
#include "FaultsDefines.h"
////////////////////////////////////////////////////////////////////////////////
CCertificateInfoCTL::CCertificateInfoCTL(void)
{}

////////////////////////////////////////////////////////////////////////////////
const char* CCertificateInfoCTL::NameOf(void) const
{
    return GetCompileType();
}

////////////////////////////////////////////////////////////////////////////////
// Virtual
CSerializeObject* CCertificateInfoCTL::Clone(void)
{
    PASSERTMSG(true, "Unable to continue with the flow");
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Virtual
CCertificateInfo* CCertificateInfoCTL::Create(void) const
{
    return new CCertificateInfoCTL;
}

////////////////////////////////////////////////////////////////////////////////
STATUS CCertificateInfoCTL::GetSerialNumber(ASN1_INTEGER* serial)
{
    // Converts ASN1 string to big number
    BIGNUM* bn = ASN1_INTEGER_to_BN(serial, NULL);
    PASSERTSTREAM_AND_RETURN_VALUE(NULL == bn,
        "ASN1_INTEGER_to_BN: " << CSslFunctions::SSLErrMsg(),
        STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

    // Converts big number to hex
    char* bnstr = BN_bn2hex(bn);
    BN_free(bn);

    PASSERTSTREAM_AND_RETURN_VALUE(NULL == bnstr,
        "BN_bn2hex: " << CSslFunctions::SSLErrMsg(),
        STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

    m_serial = bnstr;
    OPENSSL_free(bnstr);

    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS CCertificateInfoCTL::Extract(X509* cer)
{
    // Issued to
    STATUS stat = GetCN(X509_get_subject_name(cer), m_subject);
    // change policy on CN in certificates VNGFE-8606
    //if (STATUS_OK != stat)
    	//	return stat;


    // Issued by
    stat = GetCN(X509_get_issuer_name(cer), m_issuer);
    if (STATUS_OK != stat)
        return stat;

    stat = GetSerialNumber(X509_get_serialNumber(cer));
    if (STATUS_OK != stat)
        return stat;

    // Valid from
    stat = GetTime(X509_get_notBefore(cer), m_notBefore);
    if (STATUS_OK != stat)
        return stat;

    // Valid to
    stat = GetTime(X509_get_notAfter(cer), m_notAfter);
    if (STATUS_OK != stat)
        return stat;

    FTRACEINTOFUNC << "m_subject " << m_subject << ", m_issuer " << m_issuer << ",m_serial" << m_serial <<", m_notBefore " << m_notBefore<<", m_notAfter" << m_notAfter;
    // Opens temporary file for certificate details
    FILE* out = fopen(TEMP_CERTIFICATE_DETAILS_FILE.c_str(), "w+");
    std::string str = "fopen: "+(std::string)TEMP_CERTIFICATE_DETAILS_FILE+": ";
    PASSERTSTREAM_AND_RETURN_VALUE(NULL == out,
        str << strerror(errno),
        STATUS_FILE_OPEN_ERROR);

    // Translates the X509 structure into human-readable format
    int res = X509_print_fp(out, cer);
    str = "fclose: "+(std::string)TEMP_CERTIFICATE_DETAILS_FILE+": ";
    PASSERTSTREAM(0 != fclose(out),
        str << strerror(errno));

    str = "X509_print_fp: "+(std::string)TEMP_CERTIFICATE_DETAILS_FILE+": ";
    PASSERTSTREAM_AND_RETURN_VALUE(res != 1,
       str
            << CSslFunctions::SSLErrMsg(),
        STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

    // Reads content of the file to a string
    stat = ReadFileToString(TEMP_CERTIFICATE_DETAILS_FILE.c_str(),
                            CertMngrLimits::kCertDetailsMaxSize,
                            m_details);
    if (STATUS_OK != stat)
        return stat;

    // Removes temporary file, the failure is not critical
    str = "DeleteFile: "+(std::string)TEMP_CERTIFICATE_DETAILS_FILE+": ";
    BOOL res2 = DeleteFile(TEMP_CERTIFICATE_DETAILS_FILE);
    PASSERTSTREAM(!res2,
        str << strerror(errno));

    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Static

STATUS CCertificateInfoCTL::ConvertIfNeedful(const char* fname)
{
    std::ostringstream cmd;
    eCertificateFormat format = CertificateFileFormat(fname);
    switch (format)
    {
    case eCertificatePEM:
        // Nothing to do
        return STATUS_OK;

    case eCertificateDER:
        cmd << "openssl x509 -inform der";
        break;

    case eCertificateP7B:
        cmd << "openssl pkcs7 -print_certs";
        break;

    case eCertificatePFX:
        cmd << "openssl pkcs12 -nodes";
        break;

    default:
        FPASSERTSTREAM_AND_RETURN_VALUE(true,
            CertificateFormatToStr(format) << " is not supported",
            STATUS_FAIL);
    }
    std::string outfile = fname;
    outfile.append(".tmp");
    cmd << " -in " << fname << " -out " << outfile.c_str() << " 2>&1";

    std::string ans;
    STATUS stat = SystemPipedCommand(cmd.str().c_str(), ans);
    if((!ans.empty()) && (eCertificateP7B ==format ))
    {
    	//Failed to open pkcs7 with pem encoded try again with der
    	 std::ostringstream cmdDer;
    	 cmdDer << "openssl pkcs7 -print_certs -in " << fname << " -inform der -out " << outfile.c_str() << " 2>&1";
    	 ans = "";
    	 stat = SystemPipedCommand(cmdDer.str().c_str(), ans);
    }

    FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,
        "SystemPipedCommand: " << cmd.str() << ": " << ans,
        STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);
    stat = rename(outfile.c_str(),fname);

    FPASSERTSTREAM_AND_RETURN_VALUE(STATUS_OK != stat,"Failed to override file" << fname << " " << outfile.c_str(),STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);



    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Static
STATUS CCertificateInfoCTL::Read(std::list<X509*>& certs, const char* fname)
{
    STATUS stat = ConvertIfNeedful(fname);
    if (STATUS_OK != stat)
        return stat;

    // open certificate file for use with SSL API
    FILE* in = fopen(fname, "r");

    FPASSERTSTREAM_AND_RETURN_VALUE(NULL == in,
        "fopen: " << fname << ": " << strerror(errno),
        STATUS_FILE_OPEN_ERROR);



    X509* x509;
    while ((x509 = PEM_read_X509(in, NULL, NULL, NULL)) != NULL)
    {
        certs.push_back(x509);
    }

    int res = fclose(in);

    FPASSERTSTREAM(0 != res, "fclose: " << fname << ": " << strerror(errno));

    unsigned long err = ERR_peek_error();
    if (err > 0)
    {
        // Makes sure that only the error is just an EOF
        bool eof = (ERR_GET_LIB(err) == ERR_LIB_PEM &&
                    ERR_GET_REASON(err) == PEM_R_NO_START_LINE);

        // At least one certificate should be
        if (!eof || certs.empty())
        {
            FPASSERTSTREAM(true,
                "PEM_read_X509: " << fname <<
                    ": " << CSslFunctions::SSLErrMsg());

            std::for_each(certs.begin(), certs.end(), X509_free);
            if(ERR_GET_REASON(err) == PEM_R_NO_START_LINE)
            {
            	FTRACEINTOFUNC << "Cert error PEM_R_NO_START_LINE ";
            	return STATUS_NO_START_LINE_CERTIFICATE;
            }
            else
            	return STATUS_CERTIFICATE_FILE_HAS_AN_ERROR;
        }

        while (ERR_get_error() > 0)
            ;
    }

    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Static

STATUS CCertificateInfoCTL::Verify(const std::list<X509*>& certs, bool is_ctl)
{
    std::list<X509*>::const_iterator it;
    for (it = certs.begin(); it != certs.end(); ++it)
    {
        size_t num = std::distance(certs.begin(), it);

        // Checks certificate expired date
        ASN1_TIME* valid_to = X509_get_notAfter(*it);
        int res = X509_cmp_current_time(valid_to);

        if (!is_ctl && (res < 0))
        {
        	FTRACEINTOFUNC << "Re CreateSelfSignedCertificate - for personal certificates";
        	ReCreateSelfSignedCertificate(*it);

        }
//        FPASSERTSTREAM_AND_RETURN_VALUE(res < 0,
//            "Certificate has expired: num = " << num,
//            STATUS_CERTIFICATE_ALREADY_EXPIRED);

        FPASSERTSTREAM_AND_RETURN_VALUE(0 == res,
            "X509_cmp_current_time: "<< CSslFunctions::SSLErrMsg()
                << ": num = " << num,
            STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

        // Checks certificate not before date
        ASN1_TIME* valid_from = X509_get_notBefore(*it);
        res = X509_cmp_current_time(valid_from);

        FPASSERTSTREAM_AND_RETURN_VALUE(res > 0,
            "Certificate is not valid yet: num = " << num,
            STATUS_CERTIFICATE_NOT_VALID_YET);

        FPASSERTSTREAM_AND_RETURN_VALUE(0 == res,
            "X509_cmp_current_time: "<< CSslFunctions::SSLErrMsg()
                << ": num = " << num,
            STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

        // Does not check for personal certificates
        if (is_ctl)
        {
            res = X509_check_ca(*it);
            FPASSERTSTREAM_AND_RETURN_VALUE(0 == res,
               "X509_check_ca: failed: num = " << num,
               STATUS_CERTIFICATE_IS_NOT_CTL);
        }
    }

    return STATUS_OK;
}
STATUS CCertificateInfoCTL::ReCreateSelfSignedCertificate(X509 *opensslcert)
{

	CCertificateInfoCTL* cert = new CCertificateInfoCTL;
	STATUS stat = cert->Extract(opensslcert);

	if (cert->GetIssuer() != cert->GetSubject())//means that it is not a self signed certificate
		return STATUS_FAIL;
	cert->CreateSelfSignedCert();


	delete cert;

	return STATUS_OK;

}
////////////////////////////////////////////////////////////////////////////////
// Static
bool CCertificateInfoCTL::IsCTLFile(const char* name)
{
    if (NULL == name)
        return false;

    // Apache puts all CTL files under the path
    static const std::string path = HOME_CERT;
    return 0 == strncmp(name, path.c_str(), strlen(path.c_str()));
}

////////////////////////////////////////////////////////////////////////////////
// Virtual
STATUS CCertificateInfoCTL::Init(const char* fname)
{
    PASSERTMSG_AND_RETURN_VALUE(NULL == fname, "Illegal parameter", STATUS_FAIL);
    PASSERTSTREAM_AND_RETURN_VALUE(!IsFileExists(fname),
        "IsFileExists: " << fname << ": No such file or directory",
        STATUS_FILE_NOT_EXISTS);
    
    STATUS stat = CheckDiskSpace(CA_CERTIFICATES_XML_FILE.c_str(), fname);
    if (STATUS_OK != stat)
        return stat;

    std::list<X509*> certs;
    stat = Read(certs, fname);
    if (stat != STATUS_OK)
        return stat;

    bool is_ctl = IsCTLFile(fname);
    if(is_ctl)
    	m_certType = eCertificateTrust;
    else
    	m_certType = eCertificatePersonal;
    stat = Verify(certs, is_ctl);
    if (STATUS_OK != stat)
    {
        if (is_ctl)
        {
            std::for_each(certs.begin(), certs.end(), X509_free);
            return stat;
        }

        // VNGR-20190: Empty Personal Cert list after cert installation
        // Skips time related errors for Machine Certificate
        if (STATUS_CERTIFICATE_NOT_VALID_YET != stat &&
            STATUS_CERTIFICATE_ALREADY_EXPIRED != stat)
        {
            std::for_each(certs.begin(), certs.end(), X509_free);
            return stat;
        }

        TRACEINTOFUNC << "Skipped time validation for Machine Certificate";
    }
    
    stat = Extract(certs.front());
    if (STATUS_OK != stat)
    {
        std::for_each(certs.begin(), certs.end(), X509_free);
        return stat;
    }
    TRACEINTOFUNC <<"Load chain certificates size " << (int)certs.size();
    // build chained objects, start from second in the list
    std::list<X509*>::const_iterator it;
    for (it = ++certs.begin(); it != certs.end(); ++it)
    {
        CCertificateInfoCTL* obj = new CCertificateInfoCTL;
        // extract all fields except m_raw
        stat = obj->Extract(*it);
        TRACEINTOFUNC <<"Load chain subject " << obj->GetSubject();
        if (STATUS_OK != stat)
        {
            delete obj;
            std::for_each(certs.begin(), certs.end(), X509_free);
            return stat;
        }

        m_chained.push_back(obj);
    }

    std::for_each(certs.begin(), certs.end(), X509_free);

    // Reads all content of the file to the string
    return ReadFileToString(fname, CertMngrLimits::kReadAllFile, m_raw);
}
//////////////////////////
int CCertificateInfoCTL::verifySelfSignCertificate(X509* cert,const char* CAfile)
{
	int ret=STATUS_FAILED_TO_CHECK_SELF_SIGN;
	X509_STORE *cert_ctx=NULL;

	cert_ctx=X509_STORE_new();
	if (cert_ctx == NULL)
		return STATUS_FAILED_TO_CHECK_SELF_SIGN;

	//if(X509_STORE_load_locations(cert_ctx,MCU_MCMS_DIR+"/CACert/ca-bundle-client.crt",MCU_MCMS_DIR+"/CACert"))
	if(X509_STORE_load_locations(cert_ctx,CA_CERTIFICATES_FILE.c_str(),HOME_CERT.c_str()))
	{
		ret = checkSelfSignCertificate(cert,cert_ctx);
	}

	if (cert_ctx != NULL)
		X509_STORE_free(cert_ctx);

	if(X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT == ret)
		ret =1;
	else
		ret = 0;

	return ret;
}

int CCertificateInfoCTL::checkSelfSignCertificate(X509* cert,X509_STORE *ctx)
{
	//X509 *cert=NULL;
	int i=0,ret=STATUS_FAILED_TO_CHECK_SELF_SIGN;
	X509_STORE_CTX *csc;
	bool done =false;

	if(NULL == cert)
		return STATUS_FAILED_TO_CHECK_SELF_SIGN;

	csc = X509_STORE_CTX_new();
	if(NULL == csc )
		done =true;
	if(!done)
	{
		X509_STORE_set_flags(ctx, 0);
		if(!X509_STORE_CTX_init(csc,ctx,cert,0))
			done = true;
		if(!done)
		{
			i=X509_verify_cert(csc);
		}
		X509_STORE_CTX_free(csc);
		ret=0;
	}

	if(i > 0)
	{
	 ret =STATUS_OK;
	}
	else
	{
		ret =  X509_STORE_CTX_get_error(csc);
	}

	//if (cert != NULL)
	  //    X509_free(cert);

    return ret;
}

////////////////////////////////////////
