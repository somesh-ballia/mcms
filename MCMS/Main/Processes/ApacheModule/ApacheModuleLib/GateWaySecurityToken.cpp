/*
 * GateWaySecurityToken.cpp
 *
 *  Created on: May 3, 2012
 *      Author: stanny
 */


#include "GateWaySecurityToken.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "ApacheModuleEngine.h"

CGateWaySecurityTokenRequest::CGateWaySecurityTokenRequest()
{
	m_gatewayPort = "";
}

CGateWaySecurityTokenRequest::CGateWaySecurityTokenRequest(const CGateWaySecurityTokenRequest &other): CSerializeObject(other)
{
	m_gatewayPort = other.m_gatewayPort;
	
}

CGateWaySecurityTokenRequest& CGateWaySecurityTokenRequest::operator= (const CGateWaySecurityTokenRequest &other)
{	
	m_gatewayPort = other.m_gatewayPort;
	return *this;
}

CGateWaySecurityTokenRequest::~CGateWaySecurityTokenRequest()
{

}

void CGateWaySecurityTokenRequest::SerializeXml(CXMLDOMElement*& pActionsNode) const
{

}
int CGateWaySecurityTokenRequest::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement* tokeData;
	GET_CHILD_NODE(pActionNode,"V35_GATEWAY_SECURITY_CONTENT",tokeData);	
	GET_VALIDATE_CHILD(tokeData,"V35_GATEWAY_PORT",m_gatewayPort,_1_TO_NEW_FILE_NAME_LENGTH);

	return nStatus;
}


///////////// response
CGatewaySecurityTokenResponse::CGatewaySecurityTokenResponse()
{
	m_SecurityToken ="";
}

CGatewaySecurityTokenResponse::CGatewaySecurityTokenResponse(const CGatewaySecurityTokenResponse &other): CSerializeObject(other)
{
	
	m_SecurityToken = other.m_SecurityToken;
	
}

CGatewaySecurityTokenResponse& CGatewaySecurityTokenResponse::operator= (const CGatewaySecurityTokenResponse &other)
{
	m_SecurityToken = other.m_SecurityToken;
	return *this;
}

CGatewaySecurityTokenResponse::~CGatewaySecurityTokenResponse()
{

}

void CGatewaySecurityTokenResponse::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	pActionsNode->AddChildNode("GATEWAY_SECURITY_TOKEN",m_SecurityToken);
}

int CGatewaySecurityTokenResponse::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	return nStatus;
}
