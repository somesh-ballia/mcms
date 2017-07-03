//============================================================================
// Name        : ExchangeClientCntl.h
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2009
// Description : Parse functions
//============================================================================



#ifndef   __ExchangeClientCntl_h__
#define   __ExchangeClientCntl_h__


#include "StateMachine.h"


class CSegment;
class CTaskApp;
class CExchangeModuleCfg;
class CurlHTTPS;
class CurlSoap;
class CDnsResolver;


class CExchangeClientCntl : public CStateMachine
{
CLASS_TYPE_1(CExchangeClientCntl,CStateMachine )
public:
				// Constructors
	CExchangeClientCntl(CTaskApp* pTask);
	virtual ~CExchangeClientCntl();
				// base class overriding
	virtual void* GetMessageMap();
	virtual void  HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	virtual const char* NameOf() const { return "CExchangeClientCntl"; }

				// Operations & API
	void UpdateParameters(const CExchangeModuleCfg* pExchangeModuleCfg);

				// Utilities
	static STATUS ValidateConfiguration(const CExchangeModuleCfg& cfg);
	STATUS TestConnection(CExchangeModuleCfg& cfg,std::string& statusDescription);
	static std::string ParseBadRequestReason(const std::string& strResponse);

protected:
				// Action functions

		// Update exchange server connection parameters
	void OnUpdateParamsIdle(CSegment* pParam);
	void OnUpdateParamsStandby(CSegment* pParam);
	void OnUpdateParamsConnected(CSegment* pParam);
		// Polling timer event
	void OnPollingTimerToutConnected(CSegment* pParam);
		// DNS resolv timer event
//	void OnDnsResolvTimerToutConnected(CSegment* pParam);

protected:
				// Utilities
	STATUS InitializeRequestsStrings();
	BOOL ConfigurationValidForConnection(const CExchangeModuleCfg& cfg) const;
	BOOL IsConfigurationChanged(const CExchangeModuleCfg* pExchangeModuleCfg) const;
	void StandBy();
	void Connect();

	std::string GetAccountName() const;
	std::string GetAccountName(const char* pszUserName, const char* pszDomain) const;

	STATUS SoapRequest(CurlSoap& curlSoapObj, const std::string& strRequest,
			const std::string& strSoapAction, std::string& strResponse,std::string& statusDescription) const;

	STATUS PrepareFindItemString(const CExchangeModuleCfg& cfg,std::string& strFindItem) const;
	STATUS PrepareFindItemInboxString(std::string& strFindItemInbox) const;
	STATUS PrepareAcceptItemString(std::string& strAccpetItem,std::string itemID,std::string itemChangeKey);
	STATUS PrepareGetItemString(const std::map<std::string,std::string>& mapItems,std::string& strGetItem);

	void AddActiveAlarm() const;
	void RemoveActiveAlarm() const;
	void AcceptItemsInInbox(CurlSoap& curlSoapObj);
	bool IsPrefixNeeded(const char* strUrl);
//	string GenerateActiveUrl(const char* strUrl, const bool isPrefixNeeded, const string prefix="") const;
	string GenerateActiveUrl(const std::string sPrefix, const std::string sIpAddress,
			const std::string sSuffix, const std::string sPort, const std::string sPossiblePrefix) const;

	STATUS SoapRequestWithFullParams( CExchangeModuleCfg& cfg,
									  const std::string& strRequest,
									  std::string& strResponse,
									  std::string& statusDescription );

	STATUS SoapRequestWithFullParams_UrlGenerated(const std::string& strRequest, const std::string& strSoapAction,
				std::string& strResponse,std::string& statusDescription,const char* strUrl,const char* strUserName
				,const char* strPassword)const;

	bool IsIpAddress(const std::string& strIpOrDnsName ) const;

protected:
				// Attributes
	CTaskApi*			m_pTaskApi;
	CExchangeModuleCfg*	m_pClientParams;
//	CDnsResolver*		m_pDnsResolver;
	//CurlHTTPS* m_pcurlHttps;

	std::string m_strFindItemsRequest;
	std::string m_strGetItemsRequest;
	std::string m_strGetFolderListRequest;
	std::string m_strAcceptItemsRequest;
	std::string m_strFindItemsInboxRequest;

	static const char* FILENAME_FIND_ITEM;
	static const char* FILENAME_GET_ITEM;
	static const char* FILENAME_ACCEPT_ITEM;
	static const char* FILENAME_GET_FOLDER_ITEM;
	static const char* FILENAME_GET_ITEM_INBOX;

	static const char* SOAP_ACTION_FIND_ITEM;
	static const char* SOAP_ACTION_GET_ITEM;
	static const char* SOAP_ACTION_CREATE_ITEM;
	static const char* SOAP_ACTION_GET_FOLDER;

	static const std::string TEMPLATE_EMAIL_ADDRESS_PLACE;
	static const std::string TEMPLATE_START_TIME_PLACE;
	static const std::string TEMPLATE_END_TIME_PLACE;

	PDECLAR_MESSAGE_MAP
};



#endif /* __ExchangeClientCntl_h__ */

