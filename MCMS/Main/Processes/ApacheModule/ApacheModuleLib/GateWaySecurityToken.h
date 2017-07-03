#if !defined(_GateWaySecurityToken_H__)
#define _GateWaySecurityToken_H__

#include "SerializeObject.h"

class CXMLDOMElement;

struct RvgwCredential
{
	std::string m_port;
 	std::string m_userName;
 	std::string m_password;
};


class CGateWaySecurityTokenRequest : public CSerializeObject
{
public:

	CGateWaySecurityTokenRequest();

	CGateWaySecurityTokenRequest(const CGateWaySecurityTokenRequest &other);

	CGateWaySecurityTokenRequest& operator= (const CGateWaySecurityTokenRequest &other);

	~CGateWaySecurityTokenRequest();

	const char * NameOf(void) const {return "CGateWaySecurityTokenRequest";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;

	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CGateWaySecurityTokenRequest(*this);}
		
	std::string GetPort() const {return m_gatewayPort;}
private:	
	std::string m_gatewayPort;

};

class CGatewaySecurityTokenResponse : public CSerializeObject
{
public:

	CGatewaySecurityTokenResponse();

	CGatewaySecurityTokenResponse(const CGatewaySecurityTokenResponse &other);

	CGatewaySecurityTokenResponse& operator= (const CGatewaySecurityTokenResponse &other);

	~CGatewaySecurityTokenResponse();

	const char * NameOf(void) const {return "CGatewaySecurityTokenResponse";}

	void SerializeXml(CXMLDOMElement*& pActionsNode) const;
	int DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction);

	CSerializeObject* Clone() {return new CGatewaySecurityTokenResponse(*this);}
	void SetSecurityToken(std::string token){m_SecurityToken = token;};	
private:
	std::string m_SecurityToken;
};

#endif
