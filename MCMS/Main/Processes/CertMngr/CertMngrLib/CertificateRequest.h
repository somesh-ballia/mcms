#ifndef CERTIFICATE_REQUEST_H_
#define CERTIFICATE_REQUEST_H_

#include "SerializeObject.h"

class CXMLDOMElement;

class CCertificateRequest : public CSerializeObject 
{
public:

	CCertificateRequest();

	CCertificateRequest(const CCertificateRequest &other);

	CCertificateRequest& operator= (const CCertificateRequest &other);

	~CCertificateRequest();

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CCertificateRequest(*this);}

	const char * NameOf() const {return "CCertificateRequest";}

	std::string GetCountryName();
	eHashMethod GetHashMethod();
	std::string GetCommonName();
	std::string GetLocality();
	std::string GetOrganization();
	std::string GetOrganizationUnit();
	std::string GetStateOrProvince();
	
	std::string GetServiceName();
	std::string GetSubjectAlternateName();
	
	void SetCertificateRequest(std::string cert_req);
		
private:

	eHashMethod m_strHashMethod;
	std::string m_strCommonName;
	std::string m_strCountryName;
	std::string m_strLocality;
	std::string m_strOrganization;
	std::string m_strOrganizationUnit;
	std::string m_strStateOrProvince;
	
	std::string m_strCertificateRequest;
	std::string m_strSubjectAlternateName;
	std::string m_strServiceName;
};

#endif
