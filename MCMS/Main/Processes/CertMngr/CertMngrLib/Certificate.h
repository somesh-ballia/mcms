#ifndef CERTIFICATE_H_
#define CERTIFICATE_H_

#include "SerializeObject.h"

class CXMLDOMElement;

class CCertificate : public CSerializeObject 
{
public:

	CCertificate();

	CCertificate(const CCertificate &other);

	CCertificate& operator= (const CCertificate &other);

	~CCertificate();

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CCertificate(*this);}

	const char * NameOf() const {return "CCertificate";}
	
	const std::string GetCertificate();
	const std::string GetServiceName();
		
private:

	std::string m_strCertificate;
	std::string m_strServiceName;
};

class CCertificateStatus : public CSerializeObject
{
	CLASS_TYPE_1(CCertificateStatus, CSerializeObject)
public:

	CCertificateStatus(std::string uploadStatus);

	CCertificateStatus(const CCertificateStatus &other);

	CCertificateStatus& operator= (const CCertificateStatus &other);

	virtual ~CCertificateStatus(){};

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;};

	CSerializeObject* Clone() {return new CCertificateStatus(*this);}

	const char * NameOf() const {return "CCertificateStatus";}

private:

	std::string m_uploadStatus;
};

#endif /*CERTIFICATE_H_*/
