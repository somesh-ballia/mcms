// CertificateInfoCRL.cpp

#include "CertificateInfoCRL.h"

#include <errno.h>
#include <algorithm>

#include "Trace.h"
#include "SslFunc.h"
#include "OsFileIF.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "ProcessBase.h"
#include "SSLInterface.h"
#include "StatusesGeneral.h"

////////////////////////////////////////////////////////////////////////////////
CCertificateInfoCRL::CCertificateInfoCRL(void)
{
    CProcessBase* proc = CProcessBase::GetProcess();
    PASSERT(NULL == proc);

    m_task = proc->GetCurrentTask();
    PASSERT(NULL == m_task);
}

////////////////////////////////////////////////////////////////////////////////
// virtual
const char* CCertificateInfoCRL::NameOf(void) const
{
    return GetCompileType();
}

////////////////////////////////////////////////////////////////////////////////
// virtual
CSerializeObject* CCertificateInfoCRL::Clone(void)
{
    PASSERTMSG(true, "Unable to continue with the flow");
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// virtual
CCertificateInfo* CCertificateInfoCRL::Create(void) const
{
    return new CCertificateInfoCRL;
}

////////////////////////////////////////////////////////////////////////////////
// static
STATUS CCertificateInfoCRL::WritePEM(const char* fname, X509_CRL* crl)
{
    // the file is destination
    FILE* out = fopen(fname, "w");
    FPASSERTSTREAM_AND_RETURN_VALUE(NULL == out,
        "fopen: " << fname << ": " << strerror(errno),
        STATUS_FILE_OPEN_ERROR);

    int res = PEM_write_X509_CRL(out, crl);

    int res2 = fclose(out);
    FPASSERTSTREAM(0 != res2,
        "fclose: " << fname << ": " << strerror(errno));

    FPASSERTSTREAM_AND_RETURN_VALUE(!res,
        "PEM_write_X509_CRL: unable to write to " << fname
            << " as PEM: " << CSslFunctions::SSLErrMsg(),
        STATUS_FILE_WRITE_ERROR);

    FTRACEINTOFUNC << "CRL is wrote to file " << fname << " as PEM";

    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS CCertificateInfoCRL::Read(const char* fname, X509_CRL*& crl) const
{
    // open certificate file for use with SSL API
    FILE* in = fopen(fname, "r");
    FPASSERTSTREAM_AND_RETURN_VALUE(NULL == in,
        "fopen: " << fname << ": " << strerror(errno),
        STATUS_FILE_OPEN_ERROR);

    // Reads X509_CRL from the file in actual format. In case of huge CRL files
    // (more than 30MB in DER) the process can take more than watchdog timeout
    // (15 seconds). Lock/Unlock semaphores allows to answer to watchdog's
    // keep alive pinging.
    eCertificateFormat format = CertificateFileFormat(fname);
    switch (format)
    {
    case eCertificatePEM:
        m_task->UnlockRelevantSemaphore();
        crl = PEM_read_X509_CRL(in, NULL, NULL, NULL);
        m_task->LockRelevantSemaphore();
        break;

    case eCertificateDER:
        m_task->UnlockRelevantSemaphore();
        crl = d2i_X509_CRL_fp(in, NULL);
        m_task->LockRelevantSemaphore();
        break;

    default:
        FPASSERTSTREAM(true,
            CertificateFormatToStr(format) << " is not supported");

        int res = fclose(in);
        FPASSERTSTREAM(0 != res,
            "fclose: " << fname << ": " << strerror(errno));

        return STATUS_FAIL;
    }

    int res = fclose(in);
    FPASSERTSTREAM(0 != res,
        "fclose: " << fname << ": " << strerror(errno));

    FPASSERTSTREAM_AND_RETURN_VALUE(NULL == crl,
        "Unable to read " << CertificateFormatToStr(format)
            << " from " << fname
            << ": " << CSslFunctions::SSLErrMsg(),
        STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

    // Writes certificate as PEM to original file
    if (eCertificateDER == format)
    {
        STATUS stat = WritePEM(fname, crl);
        if (STATUS_OK != stat)
            return stat;
    }

    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
STATUS CCertificateInfoCRL::Extract(X509_CRL* crl)
{
    // Reads issuer and dates from the structure
    STATUS stat = GetCN(X509_CRL_get_issuer(crl), m_issuer);
    if (STATUS_OK != stat)
        return stat;

    stat = GetTime(X509_CRL_get_lastUpdate(crl), m_notBefore);
    if (STATUS_OK != stat)
        return stat;

    stat = GetTime(X509_CRL_get_nextUpdate(crl), m_notAfter);
    if (STATUS_OK != stat)
        return stat;

    // Uses temporary file for certificate details
    FILE* out = fopen(TEMP_CERTIFICATE_DETAILS_FILE.c_str(), "w+");

    std::string str = "fopen: "+(std::string)TEMP_CERTIFICATE_DETAILS_FILE+": ";
    PASSERTSTREAM_AND_RETURN_VALUE(NULL == out,
        str << strerror(errno),
        STATUS_FILE_OPEN_ERROR);

    // Translates the X509_CRL structure into human-readable format. In case of
    // huge CRL files (more than 30MB in DER) the process can take more than
    // watchdog timeout (15 seconds). Lock/Unlock semaphores allows to answer
    // to watchdog's keep alive pinging.
    m_task->UnlockRelevantSemaphore();
    int res = X509_CRL_print_fp(out, crl);
    m_task->LockRelevantSemaphore();

    str = "fclose: "+(std::string)TEMP_CERTIFICATE_DETAILS_FILE+": ";
    PASSERTSTREAM(0 != fclose(out),
        str << strerror(errno));

    str = "X509_CRL_print_fp: "+(std::string)TEMP_CERTIFICATE_DETAILS_FILE+": ";
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
STATUS CCertificateInfoCRL::Verify(X509_CRL* crl)
{
    // Checks CRL valid to
    ASN1_TIME* valid_to = X509_CRL_get_nextUpdate(crl);
    int res = X509_cmp_current_time(valid_to);

    FPASSERTMSG_AND_RETURN_VALUE(res < 0,
       "CRL has expired", STATUS_CERTIFICATE_ALREADY_EXPIRED);

    FPASSERTSTREAM_AND_RETURN_VALUE(0 == res,
       "Invalid date: "<< CSslFunctions::SSLErrMsg(),
       STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

    // Checks CRL valid from
    ASN1_TIME* valid_from = X509_CRL_get_lastUpdate(crl);
    res = X509_cmp_current_time(valid_from);

    FPASSERTMSG_AND_RETURN_VALUE(res > 0,
       "CRL is not valid yet", STATUS_CERTIFICATE_NOT_VALID_YET);

    FPASSERTSTREAM_AND_RETURN_VALUE(0 == res,
       "Invalid date: "<< CSslFunctions::SSLErrMsg(),
       STATUS_CERTIFICATE_FILE_HAS_AN_ERROR);

    return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Virtual
STATUS CCertificateInfoCRL::Init(const char* fname)
{
    PASSERTMSG_AND_RETURN_VALUE(NULL == fname, "Illegal parameter", STATUS_FAIL);
    PASSERTSTREAM_AND_RETURN_VALUE(!IsFileExists(fname),
        "IsFileExists: " << fname << ": No such file or directory",
        STATUS_FILE_NOT_EXISTS);

    STATUS stat = CheckDiskSpace(CRL_DB_FILE.c_str(), fname);
    if (STATUS_OK != stat)
        return stat;

    X509_CRL* crl;
    stat = Read(fname, crl);
    if (STATUS_OK != stat)
        return stat;

    stat = Verify(crl);
    if (STATUS_OK != stat)
    {
        X509_CRL_free(crl);
        return stat;
    }

    stat = Extract(crl);
    X509_CRL_free(crl);

    if (STATUS_OK != stat)
        return stat;

    // Reads all content of the file to the string
    return ReadFileToString(fname, CertMngrLimits::kReadAllFile, m_raw);
}
