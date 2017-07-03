//============================================================================
// Name        : ExchangeClientCntl.cpp
// Author      : Vasily
// Version     :
// Copyright   : Copyright (c) Polycom Inc 2009
// Description : Task of Exchange client
//============================================================================

#include <string>
#include <map>
#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "TaskApp.cpp"
#include "ApiStatuses.h"
#include "FaultsDefines.h"
#include "TaskApi.h"
#include "StructTm.h"
#include "ParseHelper.h"
#include "ExchangeModuleProcess.h"
#include "ExchangeModuleCfg.h"
#include "ExchangeDataTypes.h"

#include "ExchangeClientCntl.h"
#include "CurlSoap.h"
#include "CurlHTTP.h"
#include "DnsResolver.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   STATIC FIELDS:
//
const char* CExchangeClientCntl::FILENAME_FIND_ITEM = "StaticCfg/ReqFindItem.xml";
const char* CExchangeClientCntl::FILENAME_GET_ITEM = "StaticCfg/ReqGetItem.xml";
const char* CExchangeClientCntl::FILENAME_ACCEPT_ITEM = "StaticCfg/ReqAcceptItem.xml";
const char* CExchangeClientCntl::FILENAME_GET_FOLDER_ITEM = "StaticCfg/ReqGetFolderList.xml";
const char* CExchangeClientCntl::FILENAME_GET_ITEM_INBOX = "StaticCfg/ReqFindItemInbox.xml";


const char* CExchangeClientCntl::SOAP_ACTION_FIND_ITEM = "http://schemas.microsoft.com/exchange/services/2006/messages/FindItem";
const char* CExchangeClientCntl::SOAP_ACTION_GET_ITEM = "http://schemas.microsoft.com/exchange/services/2006/messages/GetItem";
const char* CExchangeClientCntl::SOAP_ACTION_CREATE_ITEM = "http://schemas.microsoft.com/exchange/services/2006/messages/CreateItem";
const char* CExchangeClientCntl::SOAP_ACTION_GET_FOLDER = "http://schemas.microsoft.com/exchange/services/2006/messages/GetFolder";

const std::string CExchangeClientCntl::TEMPLATE_EMAIL_ADDRESS_PLACE = "__EMAIL_ADDRESS_PLACE__";
const std::string CExchangeClientCntl::TEMPLATE_START_TIME_PLACE = "__START_TIME_PLACE__";
const std::string CExchangeClientCntl::TEMPLATE_END_TIME_PLACE = "__END_TIME_PLACE__";




////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   STATES:
//
//const WORD  IDLE        = 0;        // default state -  defined in base class
const WORD  STANDBY       = 1;
const WORD  CONNECTED     = 2;
//const WORD  ANYCASE     = 0xFFFF;   // any other state -  defined in base class


/////////////////////////////////////////////////////////////////////////////
//
//   CONSTANTS:
//
//const DWORD POLLING_TIME = 15 * SECOND; 	// VNGR-20857 - change it according to EXCHANGE_MODULE_CURLOPT_TIMEOUT
const DWORD DNS_RESOLV_TIME = 60 * SECOND;


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   EVENTS:
//
const WORD UPDATE_PARAMETERS  = 2001;
const WORD POLLING_TIMER_TOUT = 2021;
const WORD DNS_RESOLV_TIMER_TOUT = 2022;


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   ExchangeClientCntl - Exchange client State Machine
//
////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CExchangeClientCntl)

		// update exchange server connection parameters
	ONEVENT( UPDATE_PARAMETERS,		IDLE,		CExchangeClientCntl::OnUpdateParamsIdle )
	ONEVENT( UPDATE_PARAMETERS,		STANDBY,	CExchangeClientCntl::OnUpdateParamsStandby )
	ONEVENT( UPDATE_PARAMETERS,		CONNECTED,	CExchangeClientCntl::OnUpdateParamsConnected )
		// polling timer event
	ONEVENT( POLLING_TIMER_TOUT,	CONNECTED,	CExchangeClientCntl::OnPollingTimerToutConnected )
		// DNS resolv timer event
//	ONEVENT( DNS_RESOLV_TIMER_TOUT,	CONNECTED,	CExchangeClientCntl::OnDnsResolvTimerToutConnected )

PEND_MESSAGE_MAP(CExchangeClientCntl,CStateMachine);

/////////////////////////////////////////////////////////////////////////////
CExchangeClientCntl::CExchangeClientCntl(CTaskApp* pTask) // creator task
		: CStateMachine(pTask)
{
	m_pTaskApi = new CTaskApi;

	m_pTaskApi->CreateOnlyApi(pTask->GetRcvMbx(),
							  this,
							  pTask->GetLocalQueue());
	m_pClientParams = NULL;

//	m_pDnsResolver = new CDnsResolver();

	InitializeRequestsStrings();
}

/////////////////////////////////////////////////////////////////////////////
CExchangeClientCntl::~CExchangeClientCntl()     // destructor
{
	if( CPObject::IsValidPObjectPtr(m_pTaskApi) )
	{
		m_pTaskApi->DestroyOnlyApi();
		POBJDELETE(m_pTaskApi);
	}
	if( CPObject::IsValidPObjectPtr(m_pClientParams) )
		POBJDELETE(m_pClientParams);
//	delete m_pDnsResolver;
//	m_pDnsResolver = NULL;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::InitializeRequestsStrings()
{
	STATUS retVal = STATUS_OK;

	char* pBuffer = NULL;
	if( NULL != (pBuffer = CurlHTTP::LoadSoapBufferFromFile(FILENAME_FIND_ITEM) ) )
	{
		m_strFindItemsRequest = pBuffer;
		PDELETEA(pBuffer);
	}
	else
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::InitializeRequestsStrings - can't open file: ",FILENAME_FIND_ITEM);
		return STATUS_FAIL;
	}

	if( NULL != (pBuffer = CurlHTTP::LoadSoapBufferFromFile(FILENAME_GET_ITEM) ) )
	{
		m_strGetItemsRequest = pBuffer;
		PDELETEA(pBuffer);
	}
	else
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::InitializeRequestsStrings - can't open file: ",FILENAME_GET_ITEM);
		return STATUS_FAIL;
	}

	if( NULL != (pBuffer = CurlHTTP::LoadSoapBufferFromFile(FILENAME_ACCEPT_ITEM) ) )
	{
		m_strAcceptItemsRequest = pBuffer;
		PDELETEA(pBuffer);
	}
	else
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::InitializeRequestsStrings - can't open file: ",FILENAME_ACCEPT_ITEM);
		return STATUS_FAIL;
	}
	if( NULL != (pBuffer = CurlHTTP::LoadSoapBufferFromFile(FILENAME_GET_FOLDER_ITEM) ) )
	{
		m_strGetFolderListRequest = pBuffer;
		PDELETEA(pBuffer);
	}
	else
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::InitializeRequestsStrings - can't open file: ",FILENAME_GET_FOLDER_ITEM);
		return STATUS_FAIL;
	}
	if( NULL != (pBuffer = CurlHTTP::LoadSoapBufferFromFile(FILENAME_GET_ITEM_INBOX) ) )
	{
		m_strFindItemsInboxRequest = pBuffer;
		PDELETEA(pBuffer);
	}
	else
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::InitializeRequestsStrings - can't open file: ",FILENAME_GET_ITEM_INBOX);
		return STATUS_FAIL;
	}

	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
void* CExchangeClientCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities

/////////////////////////////////////////////////////////////////////////////
BOOL CExchangeClientCntl::ConfigurationValidForConnection(const CExchangeModuleCfg& cfg) const
{
	if( FALSE == cfg.GetServiceEnabled() )
		return FALSE;
	if( strlen(cfg.GetWebServiceUrl_configured()) < 3 )
		return FALSE;
	if( strlen(cfg.GetUserName()) < 1 )
		return FALSE;
	if( strlen(cfg.GetUserPassword()) < 1 )
		return FALSE;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::StandBy()
{
	PTRACE(eLevelInfoNormal,"CExchangeClientCntl::StandBy - go to STANDBY state");

	m_state = STANDBY;
	DeleteTimer(POLLING_TIMER_TOUT);
	DeleteTimer(DNS_RESOLV_TIMER_TOUT);

	RemoveActiveAlarm();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::Connect()
{
	PTRACE(eLevelInfoNormal,"CExchangeClientCntl::Connect - start timers POLLING_TIMER_TOUT");

	m_state = CONNECTED;
	OnPollingTimerToutConnected(NULL);

//	StartTimer(DNS_RESOLV_TIMER_TOUT,DNS_RESOLV_TIME);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
std::string CExchangeClientCntl::GetAccountName() const
{
	if( m_pClientParams )
		return GetAccountName(m_pClientParams->GetUserName(),m_pClientParams->GetDomainName());

	return "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
std::string CExchangeClientCntl::GetAccountName(const char* pszUserName, const char* pszDomain) const
{
	std::string strAccountName = "";
	if( pszUserName != NULL  &&  0 != strlen(pszUserName) )
	{
		strAccountName.append(pszUserName);
		if( pszDomain != NULL  &&  0 != strlen(pszDomain) )
		{
			strAccountName.append("@");
			strAccountName.append(pszDomain);
		}
	}
	return strAccountName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Operations & API

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::ValidateConfiguration(const CExchangeModuleCfg& cfg)
{
	if( FALSE == cfg.GetServiceEnabled() )
		return STATUS_OK;

	if( strlen(cfg.GetWebServiceUrl_configured()) < 1 )
		return STATUS_EXCHANGE_WEB_URL_EMPTY;

	if( strlen(cfg.GetUserName()) < 1 )
		return STATUS_EXCHANGE_USERNAME_EMPTY;

	if( strlen(cfg.GetUserPassword()) < 1 )
		return STATUS_EXCHANGE_PASSWORD_EMPTY;
	
	if( strlen(cfg.GetDomainName()) < 1 )
		return STATUS_EXCHANGE_DOMAIN_EMPTY;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::TestConnection(CExchangeModuleCfg& cfg,std::string& statusDescription)
{
	if( FALSE == cfg.GetServiceEnabled() )
		return STATUS_OK;
	
	if (NULL == m_pClientParams) // should not be NULL at this stage!
		return STATUS_FAIL;

	std::string strErrorCode, strErrorDesc;
	std::string strResponse, strRequest = m_strGetFolderListRequest;
	PrepareFindItemString(cfg,strRequest);
	
	STATUS status = SoapRequestWithFullParams(cfg, strRequest, strResponse, statusDescription);
	TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::TestConnection"
						   << "\nConfigured URL: " << cfg.GetWebServiceUrl_configured()
						   << "\nUsable URL:     " << cfg.GetWebServiceUrl_toUse();

	
	if( STATUS_OK != status )
	{
		//statusDescription = strResponseHeader;
		status = STATUS_EXCHANGE_FAILED_CONNECTING_TO_EXCHANGE;
	}
	else if( STATUS_OK != (status = CCalendarItemList::CheckFindItemResponseStatus(strResponse,strErrorCode,strErrorDesc) ) )
	{
		status = STATUS_EXCHANGE_FAILED_CONNECTING_TO_EXCHANGE;
		//printf("\n strErrorCode = [%s], strErrorDesc = [%s]\n",strErrorCode.c_str(),strErrorDesc.c_str());
		statusDescription = strErrorDesc;
	}
	PTRACE2(eLevelInfoNormal,"CExchangeClientCntl::TestConnection - \n",strResponse.c_str());
	return status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Operations & API

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::UpdateParameters(const CExchangeModuleCfg* pExchangeModuleCfg)
{
	CSegment* pMsg = new CSegment();
	pExchangeModuleCfg->Serialize(*pMsg);

	DispatchEvent(UPDATE_PARAMETERS,pMsg);

	POBJDELETE(pMsg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CExchangeClientCntl::IsConfigurationChanged(const CExchangeModuleCfg* pExchangeModuleCfg) const
{
	if( *pExchangeModuleCfg == *m_pClientParams )
		return FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ACTION FUNCTIONS

////////////////////////////////////////////////////////////////////////////////////////////////////
// Update exchange server connection parameters

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::OnUpdateParamsIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnUpdateParamsIdle");

	if( CPObject::IsValidPObjectPtr(m_pClientParams) )
		POBJDELETE(m_pClientParams);

	m_pClientParams = new CExchangeModuleCfg();
	m_pClientParams->DeSerialize(*pParam);
	
	if( TRUE == ConfigurationValidForConnection(*m_pClientParams) )
	{
		std::string statusDesc;
		TestConnection(*m_pClientParams, statusDesc);

		Connect();
	}
	else
	{
		StandBy();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::OnUpdateParamsStandby(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnUpdateParamsStandby");

	CExchangeModuleCfg* pCfg = new CExchangeModuleCfg();
	pCfg->DeSerialize(*pParam);

	*m_pClientParams = *pCfg;
	if( TRUE == ConfigurationValidForConnection(*m_pClientParams) )
		Connect();

	POBJDELETE(pCfg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::OnUpdateParamsConnected(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnUpdateParamsConnected");

	CExchangeModuleCfg* pCfg = new CExchangeModuleCfg();
	pCfg->DeSerialize(*pParam);

	if( FALSE == IsConfigurationChanged(pCfg)/* Should reconnect Client */)
	{
		POBJDELETE(pCfg);
		
		return;
	}

	// clean data cash
	CCalendarItemList* pCalendarItemList = new CCalendarItemList();
	CExchangeModuleProcess* pProcess = (CExchangeModuleProcess*)CProcessBase::GetProcess();
	pProcess->UpdateCalendarItemsList(*pCalendarItemList);
	POBJDELETE(pCalendarItemList);

	*m_pClientParams = *pCfg;
	if( TRUE == ConfigurationValidForConnection(*m_pClientParams) )
		Connect();
	else
		StandBy();

	POBJDELETE(pCfg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Polling timer event

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::OnPollingTimerToutConnected(CSegment* pParam)
{
	CExchangeModuleProcess* pProcess = (CExchangeModuleProcess*)CProcessBase::GetProcess();

	PTRACE2INT(eLevelInfoNormal,"CExchangeClientCntl::OnPollingTimerToutConnected, num items: ",(int)pProcess->GetCalendarItemList()->GetSize());

	// VNGR-20857 - change the polling timer according to EXCHANGE_MODULE_CURLOPT_TIMEOUT system flag
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	DWORD curloptTimeout = 5;
	sysConfig->GetDWORDDataByKey("EXCHANGE_MODULE_CURLOPT_TIMEOUT", curloptTimeout);
	DWORD pollingTime = curloptTimeout * 2 * SECOND;
	if (pollingTime < 15 * SECOND)
		pollingTime = 15 * SECOND;

	StartTimer(POLLING_TIMER_TOUT,/*POLLING_TIME*/pollingTime);

	STATUS status = STATUS_OK;

	std::string strResponse,strResponseHeader;
	std::string strFindItems;

	if( STATUS_OK != (status = PrepareFindItemString(*m_pClientParams,strFindItems)) )
	{
	    DBGPASSERT_AND_RETURN(1000+status);
	}

	CurlSoap curlSoapObj(CurlHTTP::IsSecuredUrl(m_pClientParams->GetWebServiceUrl_toUse()),true,true);
	curlSoapObj.SetTimeout(curloptTimeout);

	 eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	 if((eProductTypeRMX2000 == curProductType)&& IsTarget())
	 {
		 CExchangeModuleProcess* pProcess = (CExchangeModuleProcess*)CProcessBase::GetProcess();
		 if(pProcess->IsMngntIpIndRecieved())
		 {
			 curlSoapObj.SetOutGoingInterface(pProcess->GetMngntIp());
			 TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::OnPollingTimerToutConnected set outgoing interface ";
		 }
		 else
			 TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::OnPollingTimerToutConnected McuInd have not been recieved ";
	 }

	if( true == m_pClientParams->IsWebServiceUrl_toUseEmpty() )	// if no success until now...
	{
		std::string statusDesc;
		std::string strRequest = m_strGetFolderListRequest;
		PrepareFindItemString(*m_pClientParams, strRequest);
																// ...then try again to generate url
		status = SoapRequestWithFullParams(*m_pClientParams, strRequest, strResponse, statusDesc);
		TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::OnPollingTimerToutConnected"
							   << "\nConfigured URL: " << m_pClientParams->GetWebServiceUrl_configured()
							   << "\nUsable URL:     " << m_pClientParams->GetWebServiceUrl_toUse();
	}
	else
	{
		status = SoapRequest(curlSoapObj,strFindItems,SOAP_ACTION_FIND_ITEM,strResponse,strResponseHeader);
	}

	if( STATUS_OK != status )
	{
	    AddActiveAlarm();
	    DBGPASSERT_AND_RETURN(1000+status);
	}

	RemoveActiveAlarm();

	CCalendarItemList* pCalendarItemList = new CCalendarItemList();
	if( STATUS_OK == (status = pCalendarItemList->DeSerializeFindItemRequest(strResponse,CCalendarItem::CALENDAR_ITEM_TYPE) ) )
	{
		std::map<std::string,std::string> mapGetItem;
		if(  STATUS_OK == (status = pCalendarItemList->PrepareDifferentItemsMap(*(pProcess->GetCalendarItemList()),mapGetItem) ) )
		{
			if( mapGetItem.size() > 0 )
			{
				PTRACE2INT(eLevelInfoNormal,"CExchangeClientCntl::OnPollingTimerToutConnected - update items: ",mapGetItem.size());

				std::string strGetItems;
				if( STATUS_OK == (status = PrepareGetItemString(mapGetItem,strGetItems) ) )
				{
					if( STATUS_OK == (status = SoapRequest(curlSoapObj,strGetItems,SOAP_ACTION_GET_ITEM,strResponse,strResponseHeader)) )
					{
						if( STATUS_OK == (status = pCalendarItemList->DeSerializeGetItemRequest(strResponse,CCalendarItem::CALENDAR_ITEM_TYPE) ) )
						{
							// update calendar items list in process
							pProcess->UpdateCalendarItemsList(*pCalendarItemList);
						}
					}
					else
						DBGPASSERT(1000+status);
				}
				else
					DBGPASSERT(1000+status);
			}
			else
			{
				// changed size = one or more appointments go out of time scope
				if( pProcess->GetCalendarItemList()->GetSize() != pCalendarItemList->GetSize() )
				{
					// update calendar items list in process
					pProcess->UpdateCalendarItemsList(*pCalendarItemList);
				}
				else
				{
					PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnPollingTimerToutConnected - nothing to update");
				}
			}
		}
		else
			DBGPASSERT(1000+status);
	}
	else
		DBGPASSERT(1000+status);

	POBJDELETE(pCalendarItemList);

	if (m_pClientParams->GetWebServiceDelegate()) //if to accept incoming request We send an Accept
	{
		AcceptItemsInInbox(curlSoapObj);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::AcceptItemsInInbox(CurlSoap& curlSoapObj)
{
	PTRACE(eLevelInfoNormal,"CExchangeClientCntl::AcceptItemsInInbox");

	// not update calendar items list in process
	if ( !m_pClientParams->GetWebServiceDelegate() ) //if to accept incoming request We send an Accept
		return;

	STATUS status = STATUS_OK;
	std::string strFindItemsInbox;
	if( STATUS_OK != (status = PrepareFindItemInboxString(strFindItemsInbox)) )
	{
		DBGPASSERT_AND_RETURN(1000+status);
	}
	std::string strResponse,strResponseHeader;
	if( STATUS_OK != (status = SoapRequest(curlSoapObj,strFindItemsInbox,SOAP_ACTION_FIND_ITEM,strResponse,strResponseHeader)) )
	{
		PTRACE(eLevelInfoNormal,"CExchangeClientCntl::AcceptItemsInInbox Send Find Items request in Inbox Failed");
	    DBGPASSERT_AND_RETURN(1000+status);
	}

	CCalendarItemList* pCalendarItemList = new CCalendarItemList();
	if( STATUS_OK == (status = pCalendarItemList->DeSerializeFindItemRequest(strResponse,CCalendarItem::INBOX_ITEM_TYPE) ) )
	{
		std::map<std::string,std::string> mapGetItem;
		if(  STATUS_OK == (status = pCalendarItemList->PrepareItemsMap(mapGetItem) ) )
		{
			if( mapGetItem.size() > 0 )
			{
				PTRACE2INT(eLevelInfoNormal,"CExchangeClientCntl::AcceptItemsInInbox - update items for accept: ",mapGetItem.size());

				std::string strGetItems;
				if( STATUS_OK == (status = PrepareGetItemString(mapGetItem,strGetItems) ) )
				{
					if( STATUS_OK == (status = SoapRequest(curlSoapObj,strGetItems,SOAP_ACTION_GET_ITEM,strResponse,strResponseHeader)) )
					{
						if( STATUS_OK == (status = pCalendarItemList->DeSerializeGetItemRequest(strResponse,CCalendarItem::INBOX_MEETING_REQUEST_TYPE) ) )
						{
							CCalendarItem* pCalendarItem =  pCalendarItemList->GetFirstItem();
							while (pCalendarItem!=NULL)
							{
								// Exchange 2010 compatibility problem - remove check
								//if (pCalendarItem->GetItemMyResponseType() == "NoResponseReceived")
								//{
								std::string strResponseAccept;

								string strAcceptItem = "",strResponseHeader;
								if( STATUS_OK == (status = PrepareAcceptItemString(strAcceptItem,pCalendarItem->GetItemId(),pCalendarItem->GetItemChangeKey())) )
								{
									if( STATUS_OK == (status = SoapRequest(curlSoapObj,strAcceptItem,SOAP_ACTION_CREATE_ITEM,strResponseAccept,strResponseHeader)) )
									{
										// everything all right
									}
									else
									{
										DBGPASSERT(1000+status);
									}
								}
								else
								{
									DBGPASSERT(1000+status);
								}

								pCalendarItem = pCalendarItemList->GetNextItem();
							}
						}
					}
					else
						DBGPASSERT(1000+status);
				}
				else
					DBGPASSERT(1000+status);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CExchangeClientCntl::AcceptItemsInInbox - nothing to Accept");
			}
		}
		else
			DBGPASSERT(1000+status);
	}
	POBJDELETE(pCalendarItemList);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::SoapRequest(CurlSoap& curlSoapObj, const std::string& strRequest,
		const std::string& strSoapAction, std::string& strResponse,std::string& statusDescription) const
{
	CTaskApp* pTaskApp = COsTask::GetTaskApp();
	if(pTaskApp)
		pTaskApp->UnlockRelevantSemaphore();
	STATUS retVal = STATUS_OK;
	std::string userName = GetAccountName();

	CURLcode result = CURLE_OK;
	bool bSendSoapSuccess = false;

	if (curlSoapObj.SetNtlmAuthenticationParams(userName,m_pClientParams->GetUserPassword()) != false)
	{
		bSendSoapSuccess = curlSoapObj.PostSoapRequest(result,m_pClientParams->GetWebServiceUrl_toUse(),
				strSoapAction, strRequest, true, true);

		if( bSendSoapSuccess==false)
		{
			PTRACE(eLevelInfoNormal,"DEBUG>>> 23 bad");
			PTRACE2(eLevelError,"CExchangeClientCntl::SoapRequest - Request=",strRequest.c_str());
			PTRACE2INT(eLevelError,"CExchangeClientCntl::SoapRequest - BAD curl, result=",(int)result);
			retVal = STATUS_FAIL;
			statusDescription = CurlHTTP::GetCurlStatusAsString(result);
		}
		else
		{
			strResponse = *(curlSoapObj.GetResponseContent());
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"DEBUG>>> 25 bad");
		retVal = STATUS_FAIL;
	}

	if(pTaskApp)
			pTaskApp->LockRelevantSemaphore();

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::SoapRequestWithFullParams( CExchangeModuleCfg& cfg,
													   const std::string& strRequest,
													   std::string& strResponse,
													   std::string& statusDescription )
{
	TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::SoapRequestWithFullParams"
						   << "\ncfg.GetWebServiceUrl: " << cfg.GetWebServiceUrl_configured();
	
	CExchangeModuleProcess* pProcess = (CExchangeModuleProcess*)CExchangeModuleProcess::GetProcess();
	STATUS status = STATUS_OK;

	std::string	strPrefix;
	std::string	strIpOrDnsName;
	std::string	strSuffix;
	std::string	strPort;

	ParseHelper::EParserStatus stat;
	if( ParseHelper::eStatusOk != (stat = ParseHelper::ParseUrl(cfg.GetWebServiceUrl_configured(),strPrefix,strIpOrDnsName,strSuffix,strPort)) )
	{
		statusDescription = "Bad URL";
		return STATUS_FAIL;
	}
	std::string strIpAddress;
	//if( true == IsIpAddress(strIpOrDnsName) )
		strIpAddress = strIpOrDnsName;
	/*else
	{
		CDnsResolver* pDnsResolver = new CDnsResolver();

		// makes synchronous system call
		CDnsResolver::EDnsResolverStatus st = pDnsResolver->Resolv(strIpOrDnsName.c_str());

		if( CDnsResolver::eStatusOk == st)
			strIpAddress = pDnsResolver->GetIp();
		delete pDnsResolver;

		switch(st)
		{
		case CDnsResolver::eStatusOk:
			TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::SoapRequestWithFullParams - DNS resolved successfully, IP="
								   << strIpAddress;
			break;
		case CDnsResolver::eStatusHostNotFound:
			statusDescription = "Couldn't resolve host";
			break;
		case CDnsResolver::eStatusTryAgain:
			statusDescription = "DNS Operation timeout";
			break;
		case CDnsResolver::eStatusNoData:
			statusDescription = "No data received from DNS server";
			break;
		case CDnsResolver::eStatusUnknown:
			break;
		}
		if( st != CDnsResolver::eStatusOk )
		{
			TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::SoapRequestWithFullParams - DNS resolved successfully, status description="
					<< statusDescription;
			return STATUS_FAIL;
		}
	}*/

	// ===== 1. try first with 'https'
	string generatedUrl = GenerateActiveUrl(strPrefix,strIpAddress,strSuffix,strPort,"https://");
	
	status = SoapRequestWithFullParams_UrlGenerated(strRequest,SOAP_ACTION_FIND_ITEM, strResponse, statusDescription,
				generatedUrl.c_str(), cfg.GetUserName(), cfg.GetUserPassword());
	
	TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::SoapRequestWithFullParams"
						   << "\nStatus: " << pProcess->GetStatusAsString(status).c_str();

	if (STATUS_OK == status)
	{
		cfg.SetWebServiceUrl_toUse(generatedUrl);
	}
	
	else
	{
		if ( strPrefix.length() == 0) // meaning: the user did not specify the prefix (http or https)
		{
			// ===== 2. try with 'http'
			generatedUrl = GenerateActiveUrl(strPrefix,strIpAddress,strSuffix,strPort,"http://");
			
			status = SoapRequestWithFullParams_UrlGenerated(strRequest,SOAP_ACTION_FIND_ITEM, strResponse, statusDescription,
						generatedUrl.c_str(), cfg.GetUserName(), cfg.GetUserPassword());
	
			TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::SoapRequestWithFullParams - with 'http'"
								   << "\nStatus: " << pProcess->GetStatusAsString(status).c_str();

			if (STATUS_OK == status)
			{
				cfg.SetWebServiceUrl_toUse(generatedUrl);
			}
		}
	}
	
	return status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::SoapRequestWithFullParams_UrlGenerated(const std::string& strRequest, const std::string& strSoapAction,
			std::string& strResponse, std::string& statusDescription, const char* strUrl,const char* strUserName,
			const char* strPassword) const
{
	CTaskApp* pTaskApp = COsTask::GetTaskApp();
	if(pTaskApp)
		pTaskApp->UnlockRelevantSemaphore();

	STATUS retVal = STATUS_OK;
	std::string userName = strUserName;

	CURLcode result = CURLE_OK;
	bool bSendSoapSuccess = false;
	CurlSoap* pSoap = new CurlSoap(CurlHTTP::IsSecuredUrl(strUrl),true,true);
	if (pSoap->SetNtlmAuthenticationParams(userName,strPassword) != false)
	{
		bSendSoapSuccess = pSoap->PostSoapRequest(result,strUrl,
				strSoapAction, strRequest, true, true);

		if( bSendSoapSuccess==false)
		{
			PTRACE(eLevelInfoNormal,"DEBUG>>> 24 bad");
			//pSoap->WriteResponseHeaderToFile("badRequestResponse.txt");
			PTRACE2(eLevelError,"CExchangeClientCntl::SoapRequestWithFullParams_UrlGenerated - Request=",strRequest.c_str());
			PTRACE2INT(eLevelError,"CExchangeClientCntl::SoapRequestWithFullParams_UrlGenerated - BAD curl, result=",(int)result);

			/*if( CURLE_OK == result)
			{
				strResponse = *(pSoap->GetResponseContent());
				strResponseHeader = ParseBadRequestReason(strResponse);
			}*/
			if( result != CURLE_OK )
				statusDescription = CurlHTTP::GetCurlStatusAsString(result);
			else
				statusDescription = "Exchange authentication failed. Check user name, password or domain.";
			retVal = STATUS_FAIL;
		}
		else
		{
			strResponse = *(pSoap->GetResponseContent());
		}
	}
	else
	{
		retVal = STATUS_FAIL;
	}
	delete pSoap;

	if(pTaskApp)
		pTaskApp->LockRelevantSemaphore();	//VNGR-21461
	return retVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
std::string CExchangeClientCntl::ParseBadRequestReason(const std::string& strResponse)
{
	std::string strRetVal = "";

	std::string strTmp;

	if( STATUS_OK == ParseHelper::FindXmlNode(strTmp,strResponse,"HTML") )
	{
		if( STATUS_OK == ParseHelper::FindXmlNode(strTmp,strTmp,"HEAD") )
		{
			if( STATUS_OK == ParseHelper::FindXmlNode(strTmp,strTmp,"TITLE") )
			{
				ParseHelper::GetXmlNodeValue(strTmp,strRetVal);
			}
		}
//		if( STATUS_OK == ParseHelper::FindXmlNode(strTmp,strTmp,"BODY") )
//		{
//			if( STATUS_OK == ParseHelper::FindXmlNode(strTmp,strTmp,"h1") )
//			{
//
//			}
//		}
	}

	return strRetVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::PrepareFindItemString(const CExchangeModuleCfg& cfg, std::string& strFindItems) const
{
	strFindItems = m_strFindItemsRequest;

	std::string strSmtpMailbox = cfg.GetMailboxName();
	//std::string strAccountName = GetAccountName(cfg.GetUserName(),cfg.GetDomainName());

	// DEBUG
	//puts("\n\n\nBefore:\n---------------------------------");
	//puts(strFindItems.c_str());

	// if email box name is empty - remove all <Mailbox> structure
	if( strSmtpMailbox.empty() == true )
	{
		std::string::size_type start_pos = strFindItems.find("<Mailbox>");
		if( std::string::npos != start_pos )
		{
			std::string::size_type end_pos = strFindItems.find("</Mailbox>");
			if( std::string::npos != end_pos )
			{
				end_pos += sizeof("</Mailbox>");
				strFindItems.replace(start_pos,end_pos-start_pos-1,"");
				// DEBUG
				//puts("\n\n\nAfter:\n-------------------------------------");
				//puts(strFindItems.c_str());
				//puts("-----------------------------\n\n");
			}
		}
	}
	else
	{
		// fill email address with configuration data
		std::string::size_type start_pos = strFindItems.find(TEMPLATE_EMAIL_ADDRESS_PLACE);
		if( std::string::npos != start_pos )
		{
			strFindItems.replace(start_pos,TEMPLATE_EMAIL_ADDRESS_PLACE.length(),strSmtpMailbox);
		}
		else
		{
			PTRACE(eLevelError,"CExchangeClientCntl::PrepareFindItemString - TEMPLATE_EMAIL_ADDRESS_PLACE not found");
			return STATUS_FAIL;
		}
	}

		// change start time / end time dynamically
// StartDate="2009-08-27T08:00:57Z" - we prepare
// StartDate="2009-08-27T08:00:57.701875+03:00" - Polycom application prepares

	CStructTm localTime;
	SystemGetTime(localTime);

	// start time for polling = now - 2 hours
	CStructTm offset2hTime(0,0,0,-2,0,0);
	CStructTm startTime = localTime + offset2hTime;

	// end time for polling = now + 1 day
	CStructTm offset1dTime(1,0,0,0,0,0);
	CStructTm endTime = localTime + offset1dTime;

	char    str_time [300];
	sprintf(str_time,"%d-%02d-%02dT%02d:%02d:%02dZ",
			startTime.m_year, startTime.m_mon, startTime.m_day, startTime.m_hour, startTime.m_min, startTime.m_sec);

		// fill start time
	std::string::size_type start_pos = strFindItems.find(TEMPLATE_START_TIME_PLACE);
	if( std::string::npos != start_pos )
	{
		strFindItems.replace(start_pos,TEMPLATE_START_TIME_PLACE.length(),str_time);
	}
	else
	{
		PTRACE(eLevelError,"CExchangeClientCntl::PrepareFindItemString - TEMPLATE_START_TIME_PLACE not found");
		return STATUS_FAIL;
	}

	sprintf(str_time,"%d-%02d-%02dT%02d:%02d:%02dZ",
			endTime.m_year, endTime.m_mon, endTime.m_day, endTime.m_hour, endTime.m_min, endTime.m_sec);

		// fill end time
	start_pos = strFindItems.find(TEMPLATE_END_TIME_PLACE);
	if( std::string::npos != start_pos )
	{
		strFindItems = strFindItems.replace(start_pos,TEMPLATE_END_TIME_PLACE.length(),str_time);
	}
	else
	{
		PTRACE(eLevelError,"CExchangeClientCntl::PrepareFindItemString - TEMPLATE_END_TIME_PLACE not found");
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::PrepareFindItemInboxString(std::string& strFindItemsInbox) const
{
	strFindItemsInbox = m_strFindItemsInboxRequest;

	std::string strSmtpMailbox = m_pClientParams->GetMailboxName();
	//std::string strAccountName = GetAccountName();

	// DEBUG
	//puts("\n\n\nBefore:\n---------------------------------");
	//puts(strFindItemsInbox.c_str());

	// if email box name is empty - remove all <Mailbox> structure
	if( strSmtpMailbox.empty() == true )
	{
		std::string::size_type start_pos = strFindItemsInbox.find("<Mailbox>");
		if( std::string::npos != start_pos )
		{
			std::string::size_type end_pos = strFindItemsInbox.find("</Mailbox>");
			if( std::string::npos != end_pos )
			{
				end_pos += sizeof("</Mailbox>");
				strFindItemsInbox.replace(start_pos,end_pos-start_pos-1,"");
				// DEBUG
				//puts("\n\n\nAfter:\n-------------------------------------");
				//puts(strFindItemsInbox.c_str());
				//puts("-----------------------------\n\n");
			}
		}
	}
	else
	{
		// fill email address with configuration data
		std::string::size_type start_pos = strFindItemsInbox.find(TEMPLATE_EMAIL_ADDRESS_PLACE);
		if( std::string::npos != start_pos )
		{
			strFindItemsInbox.replace(start_pos,TEMPLATE_EMAIL_ADDRESS_PLACE.length(),strSmtpMailbox);
		}
		else
		{
			PTRACE(eLevelError,"CExchangeClientCntl::PrepareFindItemInboxString - TEMPLATE_EMAIL_ADDRESS_PLACE not found");
			return STATUS_FAIL;
		}
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::PrepareAcceptItemString(std::string& strAccpetItem,std::string itemID,std::string itemChangeKey)
{
   strAccpetItem = m_strAcceptItemsRequest;


	ParseHelper::EParserStatus status;

		// get <ItemIds> node
	std::string strItemIdsNode;
//    <ItemIds>
//      <ItemId Id="AAMkAGNlNjlhMTM3LTE5NGEtNDhiMC05YjMyLWYzMjBlNjUzMzFmNQBGAAAAAACUECrIla7BQb0D/U9fgCnLBwA8ttd2tcThRpOXnmwn7M7tAAKcsJxNAAA8ttd2tcThRpOXnmwn7M7tAAKcsKGqAAA=" ChangeKey="DwAAABYAAAA8ttd2tcThRpOXnmwn7M7tAAKcsLZm" xmlns="http://schemas.microsoft.com/exchange/services/2006/types" />
//    </ItemIds>

	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strItemIdsNode,strAccpetItem,"ReferenceItemId")) )
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::PrepareGetItemString - <ReferenceItemId> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

	// start position of Item value (for substitution)
	std::string::size_type start_pos = strAccpetItem.find(strItemIdsNode);
	int lengthToReplace = strItemIdsNode.length();
	if( std::string::npos == start_pos )
	{
		PTRACE(eLevelError,"CExchangeClientCntl::PrepareAcceptItemString - start_pos is BAD");
		return STATUS_FAIL;
	}

		// update Id attribute
	if( ParseHelper::eStatusOk != (status = ParseHelper::UpdateXmlNodeAttribute(strItemIdsNode,"Id",itemID) ) )
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::PrepareAcceptItemString - Update of \"Id\" attr FAILED, status=",ParseHelper::StatusAsString(status));
	}

		// update ChangeKey attribute
	if( ParseHelper::eStatusOk == status )
	{
		if( ParseHelper::eStatusOk != (status = ParseHelper::UpdateXmlNodeAttribute(strItemIdsNode,"ChangeKey",itemChangeKey) ) )
		{
			PTRACE2(eLevelError,"CExchangeClientCntl::PrepareAcceptItemString - Update of \"ChangeKey\" attr FAILED, status=",ParseHelper::StatusAsString(status));
		}
	}


	// substitute Ids default value with real ids
	strAccpetItem = strAccpetItem.replace(start_pos,lengthToReplace,strItemIdsNode);
	PTRACE2(eLevelError,"CExchangeClientCntl::PrepareAcceptItemString - itemID=",itemID.c_str());

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CExchangeClientCntl::PrepareGetItemString(const std::map<std::string,std::string>& mapItems,std::string& strGetItem)
{
	strGetItem = m_strGetItemsRequest;

	ParseHelper::EParserStatus status;

		// get <ItemIds> node
	std::string strItemIdsNode;
//    <ItemIds>
//      <ItemId Id="AAMkAGNlNjlhMTM3LTE5NGEtNDhiMC05YjMyLWYzMjBlNjUzMzFmNQBGAAAAAACUECrIla7BQb0D/U9fgCnLBwA8ttd2tcThRpOXnmwn7M7tAAKcsJxNAAA8ttd2tcThRpOXnmwn7M7tAAKcsKGqAAA=" ChangeKey="DwAAABYAAAA8ttd2tcThRpOXnmwn7M7tAAKcsLZm" xmlns="http://schemas.microsoft.com/exchange/services/2006/types" />
//    </ItemIds>
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strItemIdsNode,strGetItem,"ItemIds")) )
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::PrepareGetItemString - <ItemIds> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

		// get content of <ItemIds> node
	std::string strItemIdsNodeValue;
//      <ItemId Id="AAMkAGNlNjlhMTM3LTE5NGEtNDhiMC05YjMyLWYzMjBlNjUzMzFmNQBGAAAAAACUECrIla7BQb0D/U9fgCnLBwA8ttd2tcThRpOXnmwn7M7tAAKcsJxNAAA8ttd2tcThRpOXnmwn7M7tAAKcsKGqAAA=" ChangeKey="DwAAABYAAAA8ttd2tcThRpOXnmwn7M7tAAKcsLZm" xmlns="http://schemas.microsoft.com/exchange/services/2006/types" />
	if( ParseHelper::eStatusOk != (status = ParseHelper::GetXmlNodeValue(strItemIdsNode,strItemIdsNodeValue) ) )
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::PrepareGetItemString - <ItemIds> value is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

		// start position of Item value (for substitution)
	std::string::size_type start_pos = strGetItem.find(strItemIdsNodeValue);
	if( std::string::npos == start_pos )
	{
		PTRACE(eLevelError,"CExchangeClientCntl::PrepareGetItemString - start_pos is BAD");
		return STATUS_FAIL;
	}

		// find <ItemId> record for example
	std::string strItemIdSample;
	if( ParseHelper::eStatusOk != (status = ParseHelper::FindXmlNode(strItemIdSample,strItemIdsNodeValue,"ItemId")) )
	{
		PTRACE2(eLevelError,"CExchangeClientCntl::PrepareGetItemString - <ItemId> is BAD, status=",ParseHelper::StatusAsString(status));
		return STATUS_FAIL;
	}

	std::string strIdsForSubstitute;
	// for all elements of map:
	// append ItemId record with new Id and ChangeKey attributes
	std::map<std::string,std::string>::const_iterator iter = mapItems.begin();
	while( mapItems.end() != iter )
	{
		std::string strItemId = strItemIdSample;

			// update Id attribute
		if( ParseHelper::eStatusOk != (status = ParseHelper::UpdateXmlNodeAttribute(strItemId,"Id",iter->first) ) )
		{
			PTRACE2(eLevelError,"CExchangeClientCntl::PrepareGetItemString - Update of \"Id\" attr FAILED, status=",ParseHelper::StatusAsString(status));
		}

			// update ChangeKey attribute
		if( ParseHelper::eStatusOk == status )
		if( ParseHelper::eStatusOk != (status = ParseHelper::UpdateXmlNodeAttribute(strItemId,"ChangeKey",iter->second) ) )
		{
			PTRACE2(eLevelError,"CExchangeClientCntl::PrepareGetItemString - Update of \"ChangeKey\" attr FAILED, status=",ParseHelper::StatusAsString(status));
		}

		if( ParseHelper::eStatusOk == status )
			strIdsForSubstitute.append(strItemId);

		iter++;
	}

	if( strIdsForSubstitute.length() == 0 )
		return STATUS_FAIL;

	// substitute Ids default value with real ids
	strGetItem.replace(start_pos,strItemIdsNodeValue.length(),strIdsForSubstitute);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::AddActiveAlarm() const
{
	CProcessBase* pProcess = CProcessBase::GetProcess();
	if( CPObject::IsValidPObjectPtr(pProcess) )
	{
		pProcess->AddActiveAlarmSingleToneFromProcess( FAULT_GENERAL_SUBJECT, AA_EXCHANGE_SERVER_CONNECTION_ERROR, MAJOR_ERROR_LEVEL,
					"Connection to Exchange Server failed. Please verify server configuration.", true, true );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CExchangeClientCntl::RemoveActiveAlarm() const
{
	CProcessBase* pProcess = CProcessBase::GetProcess();
	if( CPObject::IsValidPObjectPtr(pProcess) )
	{
		pProcess->RemoveActiveAlarmFromProcess(AA_EXCHANGE_SERVER_CONNECTION_ERROR);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExchangeClientCntl::IsPrefixNeeded(const char* strUrl)
{
	bool isNeeded = true;

	if ( !strncmp(strUrl, "https://", strlen("https://")) ||
	     !strncmp(strUrl, "http://",  strlen("http://"))  )
	{
		isNeeded = false;
	}

	TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::IsPrefixNeeded"
						   << "\nURL:              " << strUrl
						   << "\nIs prefix needed: " << (isNeeded ? "yes" : "no");

	return isNeeded;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*string CExchangeClientCntl::GenerateActiveUrl( const char* strUrl,
											   const bool isPrefixNeeded,
											   const string prefix) const
{
	const string sUrl = strUrl;
	int urlLen = sUrl.length();

	string sOutUrl = "";


	// ==== 1. PREFIX
	if (isPrefixNeeded)
	{
		sOutUrl = prefix;
	}


	// ==== 2. URL
	sOutUrl += sUrl;


	// ==== 3. SUFFIX
	// ---- 3a. check if suffix already exists
	size_t idxOfSuffix = sUrl.rfind(".asmx");
	if ( (string::npos == idxOfSuffix) ||				// sUrl does not contain ".asmx" at all
		 (idxOfSuffix != urlLen - strlen(".asmx")) )	// ".asmx" exists but not as a suffix
	{
		// ---- 3b. add slash (if needed)
		const char lastChar = sUrl.at(urlLen - 1);
		if ( '/' !=  lastChar)
		{
			sOutUrl += "/";
		}

		// ---- 3c. add the suffix itself
		sOutUrl += "EWS/Exchange.asmx";
	}
	
	TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::GenerateActiveUrl"
						   << "\nidxOfSuffix: " << idxOfSuffix
						   << "\nurlLen:      " << urlLen
						   << "\nstrlen:      " << strlen(".asmx");

	
	
	TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::GenerateActiveUrl"
						   << "\nsUrl:    " << sUrl
						   << "\nsOutUrl: " << sOutUrl;

	return sOutUrl;
}*/

////////////////////////////////////////////////////////////////////////////////////////////////////
string CExchangeClientCntl::GenerateActiveUrl(const std::string sPrefix, const std::string sIpAddress,
		const std::string sSuffix, const std::string sPort, const std::string sPossiblePrefix) const
{
//	const string sUrl = strUrl;
//	int urlLen = sUrl.length();

	string sOutUrl = "";


	// ==== 1. PREFIX
	if (sPrefix.length() )
		sOutUrl = sPrefix;
	else
		sOutUrl = sPossiblePrefix;


	// ==== 2. IP address
	sOutUrl.append(sIpAddress);

	// ==== 3. Port
	if( sPort.length() > 0 )
	{
		sOutUrl.append(":");
		sOutUrl.append(sPort);
	}

	// ==== 4. SUFFIX
	if( sSuffix.length() > 0 )
		sOutUrl.append(sSuffix);

	// ---- 4a. check if suffix already exists
	size_t idxOfSuffix = sOutUrl.rfind(".asmx");
	if ( (string::npos == idxOfSuffix) ||				// sUrl does not contain ".asmx" at all
		 (idxOfSuffix != sOutUrl.length() - strlen(".asmx")) )	// ".asmx" exists but not as a suffix
	{
		// ---- 4b. add slash (if needed)
		const char lastChar = sOutUrl.at(sOutUrl.length() - 1);
		if ( '/' !=  lastChar)
		{
			sOutUrl += "/";
		}

		// ---- 4c. add the suffix itself
		sOutUrl += "EWS/Exchange.asmx";
	}

	TRACESTR(eLevelInfoNormal) << "\nCExchangeClientCntl::GenerateActiveUrl - generated URL=" << sOutUrl;

	return sOutUrl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CExchangeClientCntl::IsIpAddress(const std::string& strIpOrDnsName ) const
{
	// check only IPv4 rules
	for( int i=0; i<(int)strIpOrDnsName.length(); i++ )
	{
		if( strIpOrDnsName.at(i) != '.'  &&  !isdigit(strIpOrDnsName.at(i)) )
			return false;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// DNS resolv timer event

////////////////////////////////////////////////////////////////////////////////////////////////////
//void CExchangeClientCntl::OnDnsResolvTimerToutConnected(CSegment* pParam)
//{
//	PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnDnsResolvTimerToutConnected ");
//
//	StartTimer(DNS_RESOLV_TIMER_TOUT,DNS_RESOLV_TIME);

	/*std::string sPrefix, sIpOrDnsName, sSuffix, sPort;

	// check configured URL, if it's DNS name, resolv
	if( ParseHelper::eStatusOk != ParseHelper::ParseUrl(m_pClientParams->GetWebServiceUrl_configured(),sPrefix,sIpOrDnsName,sSuffix,sPort) )
	{
		PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnDnsResolvTimerToutConnected - BAD Configured URL parsing");
		return;
	}

	if( IsIpAddress(sIpOrDnsName) == false )
	{
		// makes synchronous system call
		CDnsResolver::EDnsResolverStatus status = m_pDnsResolver->Resolv(sIpOrDnsName.c_str());

		if( CDnsResolver::eStatusOk == status )
		{
			std::string sIpAddress;

			if( ParseHelper::eStatusOk == ParseHelper::ParseUrl(m_pClientParams->GetWebServiceUrl_toUse(),sPrefix,sIpAddress,sSuffix,sPort) )
			{
				if( sIpAddress != m_pDnsResolver->GetIp() )
				{
					string sGeneratedUrl = GenerateActiveUrl(sPrefix,m_pDnsResolver->GetIp(),sSuffix,sPort,sPrefix);
					m_pClientParams->SetWebServiceUrl_toUse(sGeneratedUrl);

					PTRACE2(eLevelInfoNormal,"CExchangeClientCntl::OnDnsResolvTimerToutConnected - set new URL to use: ",sGeneratedUrl.c_str());
				}
				else
					PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnDnsResolvTimerToutConnected - IP address the same");
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnDnsResolvTimerToutConnected - BAD used URL parsing");
				return;
			}
		}
		else
			PTRACE2(eLevelInfoNormal,"CExchangeClientCntl::OnDnsResolvTimerToutConnected - Resolv DNS not ok, status: ",m_pDnsResolver->StatusAsString(status));
	}
	else
		PTRACE(eLevelInfoNormal,"CExchangeClientCntl::OnDnsResolvTimerToutConnected - IP address in URL");
	*/
//}



























