/*
 * CertificateGet.h
 */

#ifndef CERTIFICATE_GET_H_
#define CERTIFICATE_GET_H_

#include <string>
#include "SerializeObject.h"
#include "CertMngrDefines.h"

class CXMLDOMElement;
class CCertificateInfo;

class CCertificateGet : public CSerializeObject 
{
CLASS_TYPE_1(CCertificateGet, CSerializeObject)
public:
    CCertificateGet(void);

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int DeSerializeXml(CXMLDOMElement* pActionNode,
                       char* pszError, const char* strAction);

    virtual const char* NameOf(void) const;
    virtual CSerializeObject* Clone(void);
	
    void SetListType(eCertificateType type);
	const char* GetSerialNumber(void) const;
	const char* GetIssuerName(void) const;
	
	bool IsCertificateExist(void) const;
	
private:
    CCertificateGet(const CCertificateGet &rhs);
    CCertificateGet& operator=(const CCertificateGet& rhs);

	std::string      m_serial;
	std::string      m_issuer;
	eCertificateType m_type;
};

#endif	//CERTIFICATE_GET_H_
