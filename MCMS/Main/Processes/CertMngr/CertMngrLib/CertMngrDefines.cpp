// CertMngrDefines.cpp

#include "CertMngrDefines.h"

#include <string>
#include <limits>
#include <algorithm>

#include "Macros.h"
#include "SslFunc.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "SSLInterface.h"

// Static
const unsigned int CertMngrLimits::kXMLFileMaxSize = 128*1024*1024;

// Static
const float CertMngrLimits::kDERtoXMLRate = 1.35216f;

// Static
const unsigned int CertMngrLimits::kAddressSpaceMaxSize = 192*1024*1024;

// Static
// Update relevant value in CCertificateInfo::RawDeSerializeXml on changing
const unsigned long CertMngrLimits::kCertDetailsMaxSize = FIFTY_LINE_BUFFER_LEN;

// Static
const unsigned long CertMngrLimits::kReadAllFile =
  std::numeric_limits<unsigned long>::max();

#define CASE(t, s) case t: return s

const char* CertificateTypeToStr(eCertificateType type)
{
  switch (type)
  {
    CASE(eCertificateTypeUnknown, "Unknown certificate");
    CASE(eCertificateTrust,       "CA certificate");
    CASE(eCertificatePersonal,    "Machine certificate");
    CASE(eCertificateRevocation,  "CRL certificate");
    CASE(eOCS,                    "CS certificate");
    default: FPASSERTSTREAM(true, "Unhandled enumerator value " << type);
  }
  
  return "";
}

std::string CertificateCAFileName(eCertificateType type, std::string serviceName)
{
  switch (type)
  {
    case eCertificateTrust:      return CA_CERTIFICATES_FILE;
    case eCertificateRevocation: return CRL_CA_FILE;
    case eCertificatePersonal:   return CERTF;

    case eOCS:
    {
    	if (serviceName.length() == 0)
    	{
    		return "";
    	}
    	std::string folder_name = CSslInterface::GetFolderName(serviceName);
    	if (folder_name == "")
    	{
    		return "";
    	}
    	return  HOME_CS + folder_name + "/cert.pem";
    }

    default:
      FPASSERTSTREAM(true,
        "Unable to get CA file name for "
          << CertificateTypeToStr(type) << " (" << type << ")");
  }

  return "";
}

const char* CertificateDBFileName(eCertificateType type)
{
  switch (type)
  {
    case eCertificateTrust:      return CA_CERTIFICATES_XML_FILE.c_str();
    case eCertificateRevocation: return CRL_DB_FILE.c_str();
    case eCertificatePersonal:
    default:
      FPASSERTSTREAM(true,
          "Unable to get DB file name for " <<
          CertificateTypeToStr(type)
          << " (" << type << ")");
  }

  return NULL;
}

const char* CertificateNodeTitle(eCertificateType type)
{
  switch (type)
  {
    case eCertificateTrust:      return "CERTIFICATE_AUTHORITHY_LIST";
    case eCertificateRevocation: return "CERTIFICATE_UNTRUSTED_LIST";
    case eCertificatePersonal:
    default:
    FPASSERTSTREAM(true,
        "Unable to get title for " << CertificateTypeToStr(type)
        << " (" << type << ")");
  }

  return NULL;
}

STATUS CertificateStatusNotExist(eCertificateType type)
{
  switch (type)
  {
    case eCertificateTrust:      return STATUS_CERTIFICATE_CTL_NOT_EXIST;
    case eCertificatePersonal:   return STATUS_CERTIFICATE_OWN_NOT_EXIST;
    case eCertificateRevocation: return STATUS_CERTIFICATE_CRL_NOT_EXIST;
    default:
      FPASSERTSTREAM(true,
        "Unable to get status for " << CertificateTypeToStr(type)
        << " (" << type << ")");
  }

  return STATUS_CERTIFICATE_OWN_NOT_EXIST;
}

const char* CertificateFormatToStr(eCertificateFormat type)
{
  switch (type)
  {
    CASE(eCertificateFormatUnknown, "Certificate Format Unknown");
    CASE(eCertificatePEM,           "Certificate PEM");
    CASE(eCertificateDER,           "Certificate DER");
    CASE(eCertificateP7B,           "Certificate P7B");
    CASE(eCertificatePFX,           "eCertificatePFX");
    default: FPASSERTSTREAM(true, "Unhandled enumerator value " << type);
  }

  return "";
}

bool HasFileExtension(const char* fname, const char* ext[])
{
  if (NULL == ext)
    return true;

  if (NULL == fname)
    return false;

  std::string str(fname);

  // Finds extension position
  size_t dot = str.rfind(".");
  if (dot == std::string::npos)
    return false;

  // Transforms extension to low case
  std::transform(str.begin() + dot + 1, str.end(),
                 str.begin() + dot + 1, ::tolower);

  while (*ext != NULL)
  {
    // Compares extension of the name to pattern
    if (0 == str.compare(dot + 1, str.length() - dot - 1, *ext))
      return true;

    ++ext;
  }

  return false;
}

eCertificateFormat CertificateFileFormat(const char* fname)
{
  if (HasFileExtension(fname, (const char*[]){"pem", NULL}))
    return eCertificatePEM;

  if (HasFileExtension(fname, (const char*[]){"der", NULL}))
    return eCertificateDER;

  if (HasFileExtension(fname, (const char*[]){"p7b", NULL}))
    return eCertificateP7B;

  if (HasFileExtension(fname, (const char*[]){"pfx", NULL}))
    return eCertificatePFX;

  return eCertificateFormatUnknown;
}

static const char* cert_stat_str[] = {
  "Certificate Status Unknown",
  "ok",
  "expired_or_not_yet_valid",
};

const char* CertificateStatusToStr(eCertificateStatus stat)
{
  switch (stat)
  {
    CASE(eCertificateStatusUnknown, cert_stat_str[0]);
    CASE(eCertificateOK,            cert_stat_str[1]);
    CASE(eCertificateErr,           cert_stat_str[2]);
    default: FPASSERTSTREAM(true, "Unhandled enumerator value " << stat);
  }

  return "";
}

eCertificateStatus CertificateStatusFromStr(const char* str)
{
  FPASSERT_AND_RETURN_VALUE(NULL == str, eCertificateStatusUnknown);

  for (size_t i = 0; i < ARRAYSIZE(cert_stat_str); ++i)
  {
    FPASSERTSTREAM_AND_RETURN_VALUE(i >= eMaxNumOfCertificateStatus,
      "Update eCertificateStatus appropriately to the table size "
      << i,
      eCertificateStatusUnknown);

    if (0 == strcmp(str, cert_stat_str[i]))
      return static_cast<eCertificateStatus>(i);
  }

  return eCertificateStatusUnknown;
}
