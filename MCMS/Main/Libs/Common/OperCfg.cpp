// OperCfg.cpp: implementation of the COperCfg class.
//
//////////////////////////////////////////////////////////////////////

#include "OperCfg.h"
#include "psosxml.h"
#include "Transactions.h"
#include "XmlApi.h"
#include "XmlDefines.h"
#include "Trace.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COperCfg::COperCfg()
{
	m_authorizationGroup       = ORDINARY;
	m_operVersion.ver_major    = 0;
	m_operVersion.ver_minor    = 0;
	m_operVersion.ver_release  = 0;
	m_operVersion.ver_internal = 0;
	m_compatibilityLevel       = 0x7FFF;
	m_apiNumber                = 0; // API_NUMBER; // YURIY WLIL FIX
	m_reserved1                = 0;
	m_reserved2                = 0;
	m_McuIp                    = 0;
	m_McuPort                  = 0;
	m_CompressionLevel         = 0;
	m_ConferenceRecorderLogin  = 0;
}

/////////////////////////////////////////////////////////////////////////////
COperCfg::COperCfg(const COperCfg &other)
	:CSerializeObject(other)
{
	m_authorizationGroup       = other.m_authorizationGroup;
	m_login                    = other.m_login;
	m_password                 = other.m_password;
	m_oldPassword              = other.m_oldPassword;
	m_stationName              = other.m_stationName;
	m_compatibilityLevel       = other.m_compatibilityLevel;
	m_operVersion.ver_major    = other.m_operVersion.ver_major;
	m_operVersion.ver_minor    = other.m_operVersion.ver_minor;
	m_operVersion.ver_release  = other.m_operVersion.ver_release;
	m_operVersion.ver_internal = other.m_operVersion.ver_internal;
	m_apiNumber                = other.m_apiNumber;
	m_reserved1                = other.m_reserved1;
	m_reserved2                = other.m_reserved2;
	m_McuIp                    = other.m_McuIp;
	m_McuPort                  = other.m_McuPort;
	m_CompressionLevel         = other.m_CompressionLevel;
	m_ConferenceRecorderLogin  = other.m_ConferenceRecorderLogin;
	m_McuHostName              = other.m_McuHostName;
}

/////////////////////////////////////////////////////////////////////////////
COperCfg::~COperCfg()
{

}

/////////////////////////////////////////////////////////////////////////////
int COperCfg::DeSerializeXml(CXMLDOMElement *pActionNode,
							 char *pszError,
							 const char* action)
{
	int nStatus;
	CXMLDOMElement *pMCUNode=NULL;

	GET_CHILD_NODE(pActionNode, "MCU_IP", pMCUNode);

	if(pMCUNode)
	{
		GET_VALIDATE_CHILD(pMCUNode,"IP",&m_McuIp,IP_ADDRESS);
		GET_VALIDATE_CHILD(pMCUNode,"LISTEN_PORT",&m_McuPort,_0_TO_WORD);
		GET_VALIDATE_CHILD(pMCUNode,"HOST_NAME",m_McuHostName,ONE_LINE_BUFFER_LENGTH);
	}

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,
								 "USER_NAME",
								 m_login,
								 0); // // YURIY WILL FIX - _1_TO_OPERATOR_NAME_LENGTH);

//	if(action == UPDATE_OPERATOR)
//	{
//		GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//									 "NEW_PASSWORD",
//									 m_password,
//									 _0_TO_OPERATOR_NAME_LENGTH);
//	}
//	else  if(action != DEL_OPERATOR)
//	{
//		GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//									 "PASSWORD",
//									 m_password,
//									 _0_TO_OPERATOR_NAME_LENGTH);
//	}

	//if (action == LOG_IN_OPERATOR)
	//{
	// YURIY WILL FIX
//		GET_VALIDATE_CHILD(pActionNode,
//						   "STATION_NAME",
//						   m_stationName,
//						   _1_TO_OPERATOR_NAME_LENGTH);

		GET_VALIDATE_CHILD(pActionNode,
						   "COMPRESSION",
						   &m_CompressionLevel,
						   _BOOL);

		GET_VALIDATE_CHILD(pActionNode,
						   "CONFERENCE_RECORDER",
						   &m_ConferenceRecorderLogin,
						   _BOOL);
	//}
//	else if((action == NEW_OPERATOR) || (action == OPERATORS_LIST))
//	{
//		GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//									 "AUTHORIZATION_GROUP",
//									 &m_authorizationGroup,
//									 AUTHORIZATION_GROUP_ENUM);
//	}
//	else if(action == UPDATE_OPERATOR)
//	{
//		GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//									 "OLD_PASSWORD",
//									 m_oldPassword,
//									 _0_TO_OPERATOR_NAME_LENGTH);
//	}

	return STATUS_OK;



	// OLD CODE

//	int nStatus;
//	CXMLDOMElement *pMCUNode=NULL;
//
//	GET_CHILD_NODE(pActionNode, "MCU_IP", pMCUNode);
//
//	if(pMCUNode)
//	{
//		GET_VALIDATE_CHILD(pMCUNode,"IP",&m_McuIp,IP_ADDRESS);
//		GET_VALIDATE_CHILD(pMCUNode,"LISTEN_PORT",&m_McuPort,_0_TO_WORD);
//		GET_VALIDATE_CHILD(pMCUNode,"HOST_NAME",m_McuHostName,ONE_LINE_BUFFER_LENGTH);
//	}
//
//	GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//								 "USER_NAME",
//								 m_login,
//								 _1_TO_OPERATOR_NAME_LENGTH);
//
//	if(action == UPDATE_OPERATOR)
//	{
//		GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//									 "NEW_PASSWORD",
//									 m_password,
//									 _0_TO_OPERATOR_NAME_LENGTH);
//	}
//	else  if(action != DEL_OPERATOR)
//	{
//		GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//									 "PASSWORD",
//									 m_password,
//									 _0_TO_OPERATOR_NAME_LENGTH);
//	}
//
//	if (action == LOG_IN_OPERATOR)
//	{
//		GET_VALIDATE_CHILD(pActionNode,
//						   "STATION_NAME",
//						   m_stationName,
//						   _1_TO_OPERATOR_NAME_LENGTH);
//
//		GET_VALIDATE_CHILD(pActionNode,
//						   "COMPRESSION",
//						   &m_CompressionLevel,
//						   _BOOL);
//
//		GET_VALIDATE_CHILD(pActionNode,
//						   "CONFERENCE_RECORDER",
//						   &m_ConferenceRecorderLogin,
//						   _BOOL);
//	}
//	else if((action == NEW_OPERATOR) || (action == OPERATORS_LIST))
//	{
//		GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//									 "AUTHORIZATION_GROUP",
//									 &m_authorizationGroup,
//									 AUTHORIZATION_GROUP_ENUM);
//	}
//	else if(action == UPDATE_OPERATOR)
//	{
//		GET_VALIDATE_MANDATORY_CHILD(pActionNode,
//									 "OLD_PASSWORD",
//									 m_oldPassword,
//									 _0_TO_OPERATOR_NAME_LENGTH);
//	}
//
//	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
void COperCfg::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
//	CXMLDOMElement* pMainNode,*pMCUNode;
//
//	if(!pFatherNode)
//		return;
//
//	pMainNode = pFatherNode;
//
//	pMCUNode=pMainNode->AddChildNode("MCU_IP");	
//	if (m_McuIp != 0xFFFFFFFF)
//	{
//		pMCUNode->AddChildNode("IP",m_McuIp,IP_ADDRESS);
//		pMCUNode->AddChildNode("LISTEN_PORT",m_McuPort);
//	}
//	else
//	{
//		pMCUNode->AddChildNode("LISTEN_PORT",m_McuPort);
//		pMCUNode->AddChildNode("HOST_NAME",m_McuHostName.c_str());
//	}
//
//
//	pMainNode->AddChildNode("USER_NAME",m_login);		
//
//
//
//
//	pMainNode->AddChildNode("STATION_NAME",
//							m_stationName);
//
//	pMainNode->AddChildNode("COMPRESSION",
//							m_CompressionLevel,_BOOL);
//
//	pMainNode->AddChildNode("CONFERENCE_RECORDER",
//							m_ConferenceRecorderLogin,
//							_BOOL);

	// just for operator list

	CXMLDOMElement* pMainNode;

	if(!pFatherNode)
		return;


	pMainNode = pFatherNode->AddChildNode("OPERATOR");


	pMainNode->AddChildNode("USER_NAME",m_login);		

	pMainNode->AddChildNode("PASSWORD",m_password);

	pMainNode->AddChildNode("AUTHORIZATION_GROUP",
							m_authorizationGroup,
							0); // // YURIY WILL FIX - AUTHORIZATION_GROUP_ENUM);


} 

