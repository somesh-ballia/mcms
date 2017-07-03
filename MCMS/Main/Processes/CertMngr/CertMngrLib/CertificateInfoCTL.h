// CertificateInfoCTL.h

#ifndef CERTIFICATEINFOCTL_H_
#define CERTIFICATEINFOCTL_H_

#include <list>
#include "StructTm.h"
#include "CertificateInfo.h"

class CCertificateInfoCTL: public CCertificateInfo
{
CLASS_TYPE_1(CCertificateInfoCTL, CCertificateInfo)
public:
    CCertificateInfoCTL(void);

    virtual const char* NameOf(void) const;
    virtual CSerializeObject* Clone(void);
    virtual CCertificateInfo* Create(void) const;
    virtual STATUS Init(const char* fname);
private:
    static STATUS Verify(const std::list<X509*>& cer, bool is_ctl);
    static STATUS Read(std::list<X509*>& cer, const char* fname);
    static STATUS ConvertIfNeedful(const char* fname);
    static bool IsCTLFile(const char* name);
    static STATUS ReCreateSelfSignedCertificate(X509* opensslcert);
    STATUS GetSerialNumber(ASN1_INTEGER* serial);
    STATUS Extract(X509* cer);
    int verifySelfSignCertificate(X509* cert,const char* CAfile);
    int checkSelfSignCertificate(X509* cert,X509_STORE *ctx);

    DISALLOW_COPY_AND_ASSIGN(CCertificateInfoCTL);
};

#endif  // CERTIFICATEINFOCTL_H_
