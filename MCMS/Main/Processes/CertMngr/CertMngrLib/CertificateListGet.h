// CertificateListGet.h

#ifndef CERTIFICATE_LIST_GET_H_
#define CERTIFICATE_LIST_GET_H_

#include "SerializeObject.h"
#include "CertMngrDefines.h"

class CXMLDOMElement;

class CCertificateListGet : public CSerializeObject
{
CLASS_TYPE_1(CCertificateListGet, CSerializeObject)
public:
    virtual void SerializeXml(CXMLDOMElement*& father) const;
    virtual int DeSerializeXml(CXMLDOMElement* father,
                                               char* pszError,
                                               const char* strAction);
	virtual CSerializeObject* Clone(void);

private:
	// Disallows operator=
	void operator=(const CCertificateListGet&);
};

#endif  // CERTIFICATE_LIST_GET_H_
