// ExchangeModuleCfg.h: interface for the CExchangeModuleCfg class.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ExchangeModuleCfg_H__
#define __ExchangeModuleCfg_H__


#include "psosxml.h"
#include "Request.h"
#include "SerializeObject.h"


class CExchangeModuleCfg : public CSerializeObject
{
CLASS_TYPE_1(CExchangeModuleCfg, CPObject)
public:

	//Constructors
	CExchangeModuleCfg();
	CExchangeModuleCfg(const CExchangeModuleCfg &other);
	CExchangeModuleCfg& operator = (const CExchangeModuleCfg& other);
	virtual ~CExchangeModuleCfg();

	friend BOOL operator == (const CExchangeModuleCfg& left ,const CExchangeModuleCfg& rigth);
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode) const;
	virtual void SerializeXml(CXMLDOMElement*& pFatherNode,bool isForEma=false) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action);
	virtual CSerializeObject* Clone() {return new CExchangeModuleCfg;}
	void Serialize(CSegment& rSegment) const;
	void DeSerialize(CSegment& seg);

	int ReadXmlFile() { return CSerializeObject::ReadXmlFile(EXCHANGE_CONFIG_FILE_NAME); }
	void WriteXmlFile() { CSerializeObject::WriteXmlFile(EXCHANGE_CONFIG_FILE_NAME,"MCU_EXCHANGE_CONFIGURATION"); }

	void SetParams(const CExchangeModuleCfg &other);

	BOOL GetServiceEnabled() const { return m_bIsServiceEnabled; }
	const char* GetWebServiceUrl_configured() const { return m_webServicesUrl_configured.c_str(); }
	const char* GetWebServiceUrl_toUse() const { return m_webServicesUrl_toUse.c_str(); }
	bool		IsWebServiceUrl_toUseEmpty() { return m_webServicesUrl_toUse.empty(); }
	const char* GetUserName() const { return m_userName.c_str(); }
	const char* GetUserPassword() const { return m_userPassword.c_str(); }
	const char* GetUserPassword_enc() const { return m_userPassword_enc.c_str(); }
	const char* GetUserPassword_dec() const { return m_userPassword_dec.c_str(); }
	const char* GetMailboxName() const { return m_mailboxName.c_str(); }
	const char* GetDomainName() const { return m_domainName.c_str(); }
	BOOL GetWebServiceDelegate() const { return m_bIsWebServicesDelegate; }
	//int convertStrActionToNumber(const char * strAction);
	void EncrptPassword();
	void SetWebServiceUrl_toUse(const string newUrl) { m_webServicesUrl_toUse = newUrl; }

	const char * NameOf() const { return "CExchangeModuleCfg"; }

	std::string Dump() const;

	static const char* EXCHANGE_CONFIG_FILE_NAME;
	void 	SetIsEMAState(BOOL val){m_isFromEMA = val;};
protected:
	BOOL m_bIsServiceEnabled;
	std::string m_webServicesUrl_configured;
	std::string m_webServicesUrl_toUse;
    std::string m_userName;
    std::string m_userPassword;

    std::string  			m_userPassword_enc;
    std::string  			m_userPassword_dec;


    std::string m_mailboxName;
	std::string m_domainName;
	BOOL m_bIsWebServicesDelegate;
	BOOL	m_isFromEMA;
	//CExchangeModuleProcess* m_pProcess;
};




#endif /*__ExchangeModuleCfg_H__*/


