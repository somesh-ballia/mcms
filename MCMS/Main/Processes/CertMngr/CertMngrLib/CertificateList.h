#ifndef __CERTIFICATE_AUTHORITHY_LIST_
#define __CERTIFICATE_AUTHORITHY_LIST_

#include <list>
#include <string>
#include <vector>

#include "SerializeObject.h"
#include "CertMngrDefines.h"
#include "CertAlarmInfo.h"
#include "ProcessBase.h"

class PrintCertOut;
class CCertificateInfo;

class CCertificateList : public CSerializeObject
{
CLASS_TYPE_1(CCertificateList, CSerializeObject)
public:
	CCertificateList(eCertificateType type);
    virtual ~CCertificateList(void);

    virtual CSerializeObject* Clone(void);
    virtual const char* NameOf(void) const;
    virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;

    void SerializeXml(CXMLDOMElement*& pFatherNode, DWORD dwObjectToken) const;

    void SerializeXml(CXMLDOMElement*& pFatherNode, DWORD dwObjectToken, const CCertificateList* additionListToSerialize) const;

	int DeSerializeXml(CXMLDOMElement* CCertificateList,
                       char* pszError, const char* action = NULL);
    
	STATUS Add(const char* fname, const std::string serviceName);
    STATUS Delete(const char* issuer, const char* serial);
    STATUS RemoveAA(CERT_ALARM_VECTOR& out) const;
    STATUS Verify(const char* host,
                  const CStructTm& now,
                  CERT_ALARM_VECTOR& out) const;

    STATUS VerifyCSService(const std::string serviceName,BYTE isMSService,
					const CStructTm& now,
					CERT_ALARM_VECTOR& out) const;

    STATUS Save(const char* fname, const char* title) const;
    void PopFront(void);
    void PopBack(void);

	int GetListSize(void) const;
	const CCertificateInfo* GetCertificate(const char* issuer,
                                           const char* serial) const;
	
	const std::list<CCertificateInfo*>& ReadList(void) const;
	void PrintOut(std::ostream& out) const;

private:
	// forbid those default operation
    CCertificateList(const CCertificateList& rhs);
	CCertificateList& operator=(const CCertificateList& rhs);

    STATUS Add(CCertificateInfo* cert);
    CCertificateInfo* IsExist(const CCertificateInfo& cer) const;
	CCertificateInfo* ProduceCertificateInfo(void);
	STATUS Get(const char* issuer,
	           const char* serial,
	           std::list<CCertificateInfo*>::iterator& out,
	           CCertificateInfo* pFatherCert=NULL);

	DWORD GetUpdateCounter(void) const;
	void IncreaseUpdateCounter(void);

	DWORD m_updateCounter;
	eCertificateType m_type;
	std::list<CCertificateInfo*> m_CertificateList;
};

#endif /*__CERTIFICATE_AUTHORITHY_LIST_*/
