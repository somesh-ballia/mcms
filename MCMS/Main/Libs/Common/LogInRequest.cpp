// LogInRequest.cpp: implementation of the CLogInRequest class.
//
//////////////////////////////////////////////////////////////////////
#include "string.h"
#include "LogInRequest.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "Transactions.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CLogInRequest::CLogInRequest()
{
	m_bCompressionLevel = FALSE;
	m_bConferenceRecorderLogin = FALSE;
	m_bExternalDbAuthorized = FALSE;
	m_MasterSlaveCurrentState = 0;
	strncpy(m_Client_ip,"",IP_ADDRESS_STR_LEN);
}

/////////////////////////////////////////////////////////////////////////////
CLogInRequest::~CLogInRequest()
{
}

CLogInRequest& CLogInRequest::operator = (const CLogInRequest &other)
{
    m_loginName = other.m_loginName;
    m_password = other.m_password;
    m_newPassword = other.m_newPassword;
	m_strStationName = other.m_strStationName;
	m_bCompressionLevel = other.m_bCompressionLevel;
	m_bConferenceRecorderLogin = other.m_bConferenceRecorderLogin;
	m_bExternalDbAuthorized = other.m_bExternalDbAuthorized;
	m_MasterSlaveCurrentState = other.m_MasterSlaveCurrentState;
	strncpy(m_Client_ip,other.m_Client_ip,IP_ADDRESS_STR_LEN);

    return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CLogInRequest::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("USER_NAME", m_loginName );
	pFatherNode->AddChildNode("PASSWORD", m_password);
    pFatherNode->AddChildNode("NEW_PASSWORD", m_newPassword);
	pFatherNode->AddChildNode("STATION_NAME", m_strStationName);
	pFatherNode->AddChildNode("COMPRESSION", m_bCompressionLevel, _BOOL);
	pFatherNode->AddChildNode("CONFERENCE_RECORDER", m_bConferenceRecorderLogin, _BOOL);		//not supported in RMX
	pFatherNode->AddChildNode("EXTERNAL_DB_AUTHORIZED", m_bExternalDbAuthorized, _BOOL);
	pFatherNode->AddChildNode("HOTBACKUP_ACTUAL_TYPE", m_MasterSlaveCurrentState, FAILOVER_MASTER_SLAVE_STATE);
	pFatherNode->AddChildNode("CLIENT_IP", m_Client_ip);
}

/////////////////////////////////////////////////////////////////////////////

int CLogInRequest::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action)
{
	int nStatus;

    GET_VALIDATE_CHILD(pActionNode,"USER_NAME",m_loginName,_1_TO_OPERATOR_NAME_LENGTH);

    GET_VALIDATE_CHILD(pActionNode,"HOTBACKUP_ACTUAL_TYPE", &m_MasterSlaveCurrentState, FAILOVER_MASTER_SLAVE_STATE);
    if (m_MasterSlaveCurrentState == eMasterSlaveNone)
    {
    	GET_VALIDATE_CHILD(pActionNode,"PASSWORD",m_password,_0_TO_OPERATOR_NAME_LENGTH);
    }
    else
    {
    	GET_VALIDATE_CHILD(pActionNode,"PASSWORD",m_password,_0_TO_ENCRYPT_PASSWORD_SHA256_LENGTH);
    }

    GET_VALIDATE_CHILD(pActionNode,"NEW_PASSWORD",m_newPassword,_0_TO_OPERATOR_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"STATION_NAME",m_strStationName,_1_TO_STATION_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"COMPRESSION",&m_bCompressionLevel,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"CONFERENCE_RECORDER",&m_bConferenceRecorderLogin,_BOOL);	//not supported in RMX
	GET_VALIDATE_CHILD(pActionNode,"EXTERNAL_DB_AUTHORIZED", &m_bExternalDbAuthorized, _BOOL);
	GET_VALIDATE_CHILD(pActionNode,"CLIENT_IP", m_Client_ip,_1_TO_IP_ADDRESS_LENGTH);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
void CLogInRequest::SetExternalDbAuthorized(BYTE bAuthohrized)
{
	m_bExternalDbAuthorized = bAuthohrized;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CLogInRequest::GetCompressionLevel()
{
	return m_bCompressionLevel;
}
