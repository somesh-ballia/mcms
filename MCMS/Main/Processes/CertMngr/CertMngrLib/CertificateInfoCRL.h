// CertificateInfoCRL.h

#ifndef CERTIFICATEINFOCRL_H_
#define CERTIFICATEINFOCRL_H_

#include "CertificateInfo.h"
#include "Macros.h"

class CTaskApp;

class CCertificateInfoCRL: public CCertificateInfo
{
CLASS_TYPE_1(CCertificateInfoCRL, CCertificateInfo)
public:
    CCertificateInfoCRL(void);
    virtual const char* NameOf(void) const;
    virtual CSerializeObject* Clone(void);
    virtual CCertificateInfo* Create(void) const;

    STATUS Init(const char* fname);

protected:
    static STATUS Verify(X509_CRL* crl);
    static STATUS WritePEM(const char* fname, X509_CRL* crl);

    STATUS Read(const char* fname, X509_CRL*& crl) const;
    STATUS Extract(X509_CRL* crl);

private:
    CTaskApp* m_task;

    DISALLOW_COPY_AND_ASSIGN(CCertificateInfoCRL);
};

#endif  // CERTIFICATEINFOCRL_H_
