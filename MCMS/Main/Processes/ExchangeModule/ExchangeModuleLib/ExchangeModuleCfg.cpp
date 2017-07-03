// ExchangeModuleCfg.cpp: implementation of the CExchangeModuleCfg class.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "Segment.h"
#include "EncodeHelper.h"

#include "ExchangeModuleCfg.h"


const char* CExchangeModuleCfg::EXCHANGE_CONFIG_FILE_NAME = "Cfg/ExchangeConfiguration.xml";

using namespace std;
////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////////////////////////////////////
CExchangeModuleCfg::CExchangeModuleCfg()
{
	m_bIsServiceEnabled = FALSE;
	m_webServicesUrl_configured = "";
	m_webServicesUrl_toUse = "";
    m_userName = "";
    m_userPassword = "";
    m_userPassword_dec = "";
    m_userPassword_enc = "";
	m_mailboxName = "";
	m_domainName = "";
	m_bIsWebServicesDelegate = TRUE;
	m_isFromEMA = TRUE;
//	m_pProcess = (CExchangeModuleProcess*)CExchangeModuleProcess::GetProcess();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CExchangeModuleCfg& CExchangeModuleCfg::operator = (const CExchangeModuleCfg &other)
{
	m_bIsServiceEnabled = other.m_bIsServiceEnabled;
	m_webServicesUrl_configured = other.m_webServicesUrl_configured;
	m_webServicesUrl_toUse = other.m_webServicesUrl_toUse;
    m_userName = other.m_userName;
    m_userPassword = other.m_userPassword;
    m_userPassword_enc = other.m_userPassword_enc;
    m_userPassword_dec = other.m_userPassword_dec;
    m_mailboxName = other.m_mailboxName;
	m_domainName = other.m_domainName;
	m_bIsWebServicesDelegate = other.m_bIsWebServicesDelegate;
	m_isFromEMA		= other.m_isFromEMA;
	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL operator == (const CExchangeModuleCfg& left, const CExchangeModuleCfg& rigth)
{
	return(left.m_bIsServiceEnabled == rigth.m_bIsServiceEnabled
		&& left.m_webServicesUrl_configured == rigth.m_webServicesUrl_configured
		&& left.m_userName == rigth.m_userName
		&& left.m_userPassword == rigth.m_userPassword
		&& left.m_mailboxName == rigth.m_mailboxName
		&& left.m_domainName == rigth.m_domainName
		&& left.m_bIsWebServicesDelegate == rigth.m_bIsWebServicesDelegate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeModuleCfg::SetParams(const CExchangeModuleCfg &other)
{
	*this = other;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CExchangeModuleCfg::~CExchangeModuleCfg()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeModuleCfg::SerializeXml(CXMLDOMElement*& pFatherNode,bool isForEma) const
{
	CXMLDOMElement* pMainNode;

	if(!pFatherNode)
		return;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("MCU_EXCHANGE_CONFIG_PARAMS");
		pMainNode = pFatherNode;
	}
	else
	{
		pMainNode = pFatherNode->AddChildNode("MCU_EXCHANGE_CONFIG_PARAMS");
	}
	pMainNode->AddChildNode("SERVICE_ENABLED",m_bIsServiceEnabled,_BOOL);
	pMainNode->AddChildNode("WEB_SERVICES_URL",m_webServicesUrl_configured);
	// m_webServicesUrl_toUse is not written to the file (it should be regenerated on each startup)
	pMainNode->AddChildNode("USER_NAME",m_userName);
	pMainNode->AddChildNode("DOMAIN_NAME",m_domainName);

	if (true == isForEma)
		pMainNode->AddChildNode("PASSWORD",m_userPassword);
	else // to file
		pMainNode->AddChildNode("PASSWORD",m_userPassword_enc);




	pMainNode->AddChildNode("WEB_SERVICES_DELEGATE",m_bIsWebServicesDelegate,_BOOL);
	pMainNode->AddChildNode("PRIMARY_SMTP_MAILBOX",m_mailboxName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeModuleCfg::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pMainNode;

	if(!pFatherNode)
		return;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("MCU_EXCHANGE_CONFIG_PARAMS");
		pMainNode = pFatherNode;
	}
	else
	{
		pMainNode = pFatherNode->AddChildNode("MCU_EXCHANGE_CONFIG_PARAMS");
	}
	pMainNode->AddChildNode("SERVICE_ENABLED",m_bIsServiceEnabled,_BOOL);
	pMainNode->AddChildNode("WEB_SERVICES_URL",m_webServicesUrl_configured);
	// m_webServicesUrl_toUse is not written to the file (it should be regenerated on each startup)
	pMainNode->AddChildNode("USER_NAME",m_userName);
	pMainNode->AddChildNode("DOMAIN_NAME",m_domainName);

	pMainNode->AddChildNode("PASSWORD",m_userPassword_enc);
	pMainNode->AddChildNode("WEB_SERVICES_DELEGATE",m_bIsWebServicesDelegate,_BOOL);
	pMainNode->AddChildNode("PRIMARY_SMTP_MAILBOX",m_mailboxName);
}
/////////////////////////////////////////////////////
void CExchangeModuleCfg::EncrptPassword()
{
	EncodeHelper eH;
	eH.EncryptPassword(m_userPassword , m_userPassword_enc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int CExchangeModuleCfg::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	CXMLDOMElement* pMcuExchangeCfgNode;
	char* ParentNodeName;
//	BOOL bReadTime=TRUE;

	pActionNode->get_nodeName(&ParentNodeName);
	if(!strcmp(ParentNodeName, "MCU_EXCHANGE_CONFIG_PARAMS"))
	{
 		pMcuExchangeCfgNode = pActionNode;
// 		bReadTime = FALSE;//no need to read time not relevant when loading from file
	}
	else
		GET_MANDATORY_CHILD_NODE(pActionNode, "MCU_EXCHANGE_CONFIG_PARAMS", pMcuExchangeCfgNode);

	if ( pMcuExchangeCfgNode )
	{
		GET_VALIDATE_CHILD(pMcuExchangeCfgNode,"SERVICE_ENABLED",&m_bIsServiceEnabled,_BOOL);
		GET_VALIDATE_CHILD(pMcuExchangeCfgNode,"WEB_SERVICES_URL",m_webServicesUrl_configured,_0_TO_IP_LIMIT_ADDRESS_CHAR_LENGTH);
		// m_webServicesUrl_toUse is not written to the file (it should be regenerated on each startup)
		GET_VALIDATE_CHILD(pMcuExchangeCfgNode,"USER_NAME",m_userName,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pMcuExchangeCfgNode,"DOMAIN_NAME",m_domainName, _0_TO_H243_NAME_LENGTH);




		if(m_isFromEMA)
		{
			GET_VALIDATE_CHILD(pMcuExchangeCfgNode,"PASSWORD",m_userPassword,_0_TO_H243_NAME_LENGTH);
			EncodeHelper eH;
			eH.EncryptPassword(m_userPassword , m_userPassword_enc);
		}
		else
		{
			GET_VALIDATE_CHILD(pMcuExchangeCfgNode,"PASSWORD",m_userPassword_enc,_0_TO_H243_NAME_LENGTH);
			EncodeHelper eH;
			eH.DecryptPassword(m_userPassword_enc,m_userPassword);
		}



		GET_VALIDATE_CHILD(pMcuExchangeCfgNode,"WEB_SERVICES_DELEGATE",&m_bIsWebServicesDelegate,_BOOL);
		GET_VALIDATE_CHILD(pMcuExchangeCfgNode,"PRIMARY_SMTP_MAILBOX",m_mailboxName, _0_TO_H243_NAME_LENGTH);
	}

	return nStatus;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeModuleCfg::Serialize(CSegment& rSegment) const
{
	rSegment << m_bIsServiceEnabled
			 << m_bIsWebServicesDelegate
			 << m_webServicesUrl_configured
			 << m_webServicesUrl_toUse
			 << m_userName
			 << m_domainName
			 << m_userPassword
			 << m_mailboxName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeModuleCfg::DeSerialize(CSegment& rSegment)
{
	rSegment >> m_bIsServiceEnabled
			 >> m_bIsWebServicesDelegate
			 >> m_webServicesUrl_configured
			 >> m_webServicesUrl_toUse
			 >> m_userName
			 >> m_domainName
			 >> m_userPassword
			 >> m_mailboxName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
std::string CExchangeModuleCfg::Dump() const
{
	std::string str;
	str.clear();
	str.append("\nCExchangeModuleCfg::Dump\n--------------------------------------------------");
	str.append("\n\tService enabled            = "); str.append((m_bIsServiceEnabled) ? "Yes" : "No");
	str.append("\n\tWeb service URL configured = " + m_webServicesUrl_configured);
	str.append("\n\tWeb service URL used       = " + m_webServicesUrl_toUse);
	str.append("\n\tUser name                  = " + m_userName);
	str.append("\n\tDomain                     = " + m_domainName);
	str.append("\n\tMailbox                    = " + m_mailboxName);
	str.append("\n\tAccept app                 = "); str.append((m_bIsWebServicesDelegate) ? "Yes" : "No");
	str.append("\n--------------------------------------------------\n");

	return str;
}








