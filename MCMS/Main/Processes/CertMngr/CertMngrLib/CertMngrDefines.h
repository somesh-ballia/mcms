// CertMngrDefines.h

#ifndef CERT_MNGR_DEFINES_H_
#define CERT_MNGR_DEFINES_H_

#include "DataTypes.h"
#include <string>

#define SELF_SIGN_STR "Self Signed"
struct CertMngrLimits
{
    // Defines maximum size of a XML file
    static const DWORD kXMLFileMaxSize;

    // Defines rate between size of DER and size of appropriate XML
    static const float kDERtoXMLRate;

    // Defines maximum size of process address space
    static const DWORD kAddressSpaceMaxSize;

    // Defines maximum number of symbols in certificate's detail
    static const unsigned long kCertDetailsMaxSize;

    // Tells to method CCertificateInfo::ReadFileToStr to read whole file
    static const unsigned long kReadAllFile;
};

enum eCertificateType
{
    eCertificateTypeUnknown,
    eCertificateTrust,
    eCertificatePersonal,
	eCertificateRevocation,
	eOCS,

	eMaxNumOfCertificateType
};

const char* CertificateTypeToStr(eCertificateType type);
std::string CertificateCAFileName(eCertificateType type, std::string serviceName = "");
const char* CertificateDBFileName(eCertificateType type);
const char* CertificateNodeTitle(eCertificateType type);
STATUS CertificateStatusNotExist(eCertificateType type);

enum eCertificateFormat
{
    eCertificateFormatUnknown,
    eCertificatePEM,
    eCertificateDER,
    eCertificateP7B,
    eCertificatePFX,

    eMaxNumOfCertificateFormat
};

const char* CertificateFormatToStr(eCertificateFormat fmt);
bool HasFileExtension(const char* fname, const char* ext[]);
eCertificateFormat CertificateFileFormat(const char* fname);

enum eCertificateStatus
{
    eCertificateStatusUnknown,
    eCertificateOK,
    eCertificateErr,

    eMaxNumOfCertificateStatus
};

const char* CertificateStatusToStr(eCertificateStatus stat);
eCertificateStatus CertificateStatusFromStr(const char* str);

#endif  // CERT_MNGR_DEFINES_H_
