// ConnectionList.cpp: implementation of the CConnectionList class.
//
//////////////////////////////////////////////////////////////////////

#include <vector>
using namespace std;


#include "ConnectionList.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApacheDefines.h"
#include "TraceStream.h"
#include "OperatorDefines.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "ApiStatuses.h"
#include "ApacheModuleEngine.h"
#include "ApacheModuleProcess.h"
#include "GateWaySecurityToken.h"



//////////////////////////////////////////////////////////////////////
// CConnection class
//////////////////////////////////////////////////////////////////////


CConnection::CConnection()
{
	m_ConnectionId = 0;
	m_nAuthorization = ANONYMOUS;
	SetLoginGmtTimeToCurrent();
	m_nAliveCounter = 0;
	m_bSupportCompression = false;

	for(int i=0; i < 4; i++)
		m_nRequestCounters[i] = 0;

	m_Entity = ENTITY_MANAGEMENT;
	m_bMachineAccount = false;
	m_bHttpConnection = false;
	m_bSecuredConnection = false;
}

CConnection::CConnection(const char* pszLogin, const char* pszStationName,
                         const char *clientIp,
                         WORD nAuthorization,
						 CStructTm* pLoginGmtTime,
						 bool bSupportCompression,
						 BYTE bEntity,
						 const bool isMachineAccount)
{
	SetLogin(pszLogin);
	SetStationName(pszStationName);
	SetAuthorization(nAuthorization);
    SetClientIp(clientIp);
	SetEntity(bEntity);
	SetMachineAccount(isMachineAccount);

	if(pLoginGmtTime)
		SetLoginGmtTime(*pLoginGmtTime);
	else
		SetLoginGmtTimeToCurrent();

	SetSupportCompression(bSupportCompression);
	m_nAliveCounter = 0;

	for(int i=0; i < 4; i++)
		m_nRequestCounters[i] = 0;

	m_ConnectionId = 0;
	m_bHttpConnection = false;
	m_bSecuredConnection = false;
}

CConnection::CConnection(CConnection& other, int connectionId)
{
	*this = other;
	m_ConnectionId = connectionId;

	for(int i=0; i < 4; i++)
		m_nRequestCounters[i] = 0;
}

int CConnection::GetConnectionId() const
{
	return m_ConnectionId;
}

const char* CConnection::GetLogin() const
{
	return m_strLogin.c_str();
}

void CConnection::SetLogin(const char* pszLogin)
{
	if(!pszLogin)
		return;

	m_strLogin = pszLogin;
}

const char* CConnection::GetStationName() const
{
	return m_strStationName.c_str();
}

void CConnection::SetStationName(const char* pszStationName)
{
	if(!pszStationName)
		return;

	m_strStationName = pszStationName;
}

const char* CConnection::GetClientIp() const
{
    return m_ClientIp.c_str();
}

void CConnection::SetClientIp(const char* pszClientIp)
{
    m_ClientIp = pszClientIp;
}

WORD CConnection::GetAuthorization()
{
	return m_nAuthorization;
}

void CConnection::SetAuthorization(WORD nAuthorization)
{
	m_nAuthorization = nAuthorization;
}

CStructTm CConnection::GetLoginGmtTime()
{
	return m_loginGmtTime;
}

void CConnection::SetLoginGmtTime(CStructTm loginGmtTime)
{
	m_loginGmtTime = loginGmtTime;
}

void CConnection::SetLoginGmtTimeToCurrent()
{
	time_t currTime;
	tm* pCurrGmtTimeTm;

	time(&currTime);
	pCurrGmtTimeTm = gmtime(&currTime);

	m_loginGmtTime.m_hour = pCurrGmtTimeTm->tm_hour;
	m_loginGmtTime.m_min = pCurrGmtTimeTm->tm_min;
	m_loginGmtTime.m_sec = pCurrGmtTimeTm->tm_sec;
	m_loginGmtTime.m_day = pCurrGmtTimeTm->tm_mday;
	m_loginGmtTime.m_mon = pCurrGmtTimeTm->tm_mon + 1;
	m_loginGmtTime.m_year = pCurrGmtTimeTm->tm_year;
}

bool CConnection::GetSupportCompression()
{
	return m_bSupportCompression;
}

void CConnection::SetSupportCompression(bool bSupportCompression)
{
	m_bSupportCompression = bSupportCompression;
}

bool CConnection::IsAlive()
{
//	m_nAliveCounter++;

	if(m_nAliveCounter > 0)	//if it is the second time no response arrive to the connection-kill the connection
		return false;

	return true;
}

void CConnection::SetAlive()
{
	m_nAliveCounter = 0;
}

void CConnection::ResetAlive()
{
	m_nAliveCounter++;
}

void CConnection::SetEntity(BYTE entity)
{
	m_Entity = entity;
}

BYTE CConnection::GetEntity()
{
	return m_Entity;
}

bool CConnection::GetMachineAccount() const
{
	return m_bMachineAccount;
}

void CConnection::SetMachineAccount(const bool bMachineAccount)
{
	m_bMachineAccount = bMachineAccount;
}

CConnection& CConnection::operator=(CConnection &other)
{
	m_ConnectionId = other.m_ConnectionId;
	SetLogin(other.GetLogin());
	SetStationName(other.GetStationName());
	SetAuthorization(other.GetAuthorization());
	SetLoginGmtTime(other.GetLoginGmtTime());
    SetClientIp(other.GetClientIp());
	SetEntity(other.GetEntity());

	SetSupportCompression(other.GetSupportCompression());
	m_nAliveCounter = 0;
	m_bHttpConnection = true;
	m_bSecuredConnection = false;
	m_bMachineAccount = other.m_bMachineAccount;
	return *this;
}

void CConnection::SerializeXml(CXMLDOMElement* pNode)
{
	CXMLDOMElement* pConnectionNode = pNode->AddChildNode("OPERATOR_CONNECTION");

	pConnectionNode->AddChildNode("CONNECTION_ID",m_ConnectionId);
	pConnectionNode->AddChildNode("USER_NAME",m_strLogin);
	pConnectionNode->AddChildNode("STATION_NAME",m_strStationName);
	pConnectionNode->AddChildNode("AUTHORIZATION_GROUP",m_nAuthorization, AUTHORIZATION_GROUP_ENUM);
	pConnectionNode->AddChildNode("LOGIN_TIME",m_loginGmtTime);
	pConnectionNode->AddChildNode("HTTP_PROTOCOL",m_bHttpConnection,_BOOL);			//not supported in RMX
	pConnectionNode->AddChildNode("SECURED_CONNECTION",m_bSecuredConnection,_BOOL);	//not supported in RMX
	pConnectionNode->AddChildNode("ENTITY", m_Entity, ENTITY_ENUM);
	pConnectionNode->AddChildNode("MACHINE_ACCOUNT",m_bMachineAccount,_BOOL);
}

void CConnection::IncreaseRequestCounter(int nMethod)
{
	if(M_GET <= nMethod && nMethod <= M_DELETE)
		m_nRequestCounters[nMethod] += 1;
}

int CConnection::GetRequestCounter(int nMethod)
{
	if(M_GET <= nMethod && nMethod <= M_DELETE)
		return m_nRequestCounters[nMethod];

	return 0;
}


/////////////////////////////////////////////////////////////////////
/////  Connection Resource 								/////////////

void   ConnectionResource::SetWriteQueue(COsQueue *pQueue)
{
	m_pWriteQueue=pQueue;
}
void    ConnectionResource::SetReadQueue(COsQueue *pQueue)
{
	m_pReadQueue = pQueue;
}

COsQueue * ConnectionResource::GetWriteQueue()
{

	if(m_pWriteQueue && (m_pWriteQueue->m_idType == eInvalidId))
		InitQueues();

	return m_pWriteQueue;
}
COsQueue *  ConnectionResource::GetReadQueue()
{
	if(m_pReadQueue && (m_pReadQueue->m_idType == eInvalidId))
		InitQueues();

	return m_pReadQueue;
}

void     ConnectionResource::InitQueues()
{
	if(m_pReadQueue && m_pWriteQueue)
	{

		char szUniqueName[MAX_QUEUE_NAME_LEN];
		COsQueue::CreateUniqueQueueName(szUniqueName);

		STATUS statCreateRcv = m_pReadQueue->CreateRead(eProcessApacheModule,szUniqueName);
		STATUS statCreateWrite = m_pWriteQueue->CreateWrite(eProcessApacheModule,szUniqueName);

		if (STATUS_OK!=statCreateRcv || STATUS_OK!=statCreateWrite) // Amdocs Suspected scenario - if teh Queue creation failed then do not put teh Mailbox in Session Variables
		{
			m_pReadQueue->Delete();
			delete m_pReadQueue;
			m_pReadQueue = NULL;
			m_pWriteQueue->Delete();
			delete m_pWriteQueue;
			m_pWriteQueue = NULL;
		}
	}
}

ConnectionResource::~ConnectionResource()
{
	if(m_pWriteQueue && m_pReadQueue)
	{
		m_pReadQueue->Delete();
		delete m_pReadQueue;
		m_pReadQueue = NULL;
		m_pWriteQueue->Delete();
		delete m_pWriteQueue;
		m_pWriteQueue = NULL;
	}
}

WrapperConnection::~WrapperConnection()
{
	if(m_pConnResource)
		delete m_pConnResource;
	if(m_pConn)
		delete m_pConn;
}
//////////////////////////////////////////////////////////////////////
// CConnectionList class
//////////////////////////////////////////////////////////////////////


apr_thread_mutex_t* CConnectionList::m_pAccessMutex = NULL;

CConnectionList::CConnectionList(apr_pool_t *pConfigPool)
{
	m_nCount = 0;
	m_pConnMap = new CConnectionsMap;

	m_nUpdateCounter = 0;
	m_pAccessMutex = NULL;

	m_nLastHttpConnectionId = 1;		//the first connection is number 1, and not 0.

	if(pConfigPool)
		apr_thread_mutex_create(&m_pAccessMutex,APR_THREAD_MUTEX_NESTED,pConfigPool);
}

int CConnectionList::GetCount()
{
	return GetcountWithoutAnonymousConnections();
}
int  CConnectionList::GetcountWithoutAnonymousConnections()
{
	int nconnections = 0;
	Lock();
	for (CConnectionsMap::const_iterator it = m_pConnMap->begin(); it != m_pConnMap->end();++it)
	{
		CConnection* pConn = NULL;
		pConn = (*it).second->m_pConn;
		if(pConn != NULL)
		{
			if(pConn->GetAuthorization() !=ANONYMOUS)
			{
				nconnections++;
			}
		}
	}
	UnLock();
	return nconnections;
}

CConnectionList::~CConnectionList()
{
	for (CConnectionsMap::const_iterator it = m_pConnMap->begin(); it != m_pConnMap->end();++it)
		delete (*it).second;

	m_pConnMap->clear();

	if (m_pConnMap!=NULL)
		delete m_pConnMap;

	if(m_pAccessMutex)
		apr_thread_mutex_destroy(m_pAccessMutex);
}

void CConnectionList::Lock()
{
	if(m_pAccessMutex)
		apr_thread_mutex_lock(m_pAccessMutex);
}

void CConnectionList::UnLock()
{
	if(m_pAccessMutex)
		apr_thread_mutex_unlock(m_pAccessMutex);
}

void CConnectionList::IncreaseUpdateCounter()
{
	m_nUpdateCounter++;

	if(m_nUpdateCounter == 0xFFFFFFFF)
		m_nUpdateCounter = 0;
}

void CConnectionList::IncreaseHttpConnectionId()
{
	m_nLastHttpConnectionId++;

	if (m_nLastHttpConnectionId == 0xFFFFFFFF)
		m_nLastHttpConnectionId = 1;
}

int CConnectionList::GetUpdateCounter()
{
	Lock();

	int nUpdateCounter = m_nUpdateCounter;

	UnLock();

	return nUpdateCounter;
}

int CConnectionList::AddConnection(CConnection& connection,ConnectionResource* pConResource)
{
	int nRetIndex = 0;

	Lock();

	if(m_nCount < MAX_CONNECTIONS)
	{

		m_pConnMap->insert(CConnectionsMap::value_type(m_nLastHttpConnectionId,new WrapperConnection( new CConnection(connection, m_nLastHttpConnectionId),pConResource)));
		m_nCount++;
		IncreaseUpdateCounter();
		nRetIndex = m_nLastHttpConnectionId;
		IncreaseHttpConnectionId();
	}

	UnLock();

	return nRetIndex;
}

 bool CConnectionList::GetConnectionAuditDetails(int nConnId, string& strLoginName, string& strStationName, string& clientIp)
 {
	Lock();
	bool bRet = false;
	CConnection* pConn = FindConnectionPtr(nConnId);
	bRet = (pConn != NULL);
	if (bRet)
	{
		strLoginName = pConn->GetLogin();
		strStationName = pConn->GetStationName();
		clientIp = pConn->GetClientIp();
	}
	UnLock();
	return bRet;

}

bool CConnectionList::SetConnectionV35Details(int nConnId, const CGateWaySecurityTokenRequest* pSecTokenReq, const list<RvgwCredential>& rvgwCredentials, TICKS& v35Securityticket)
{
	Lock();

	bool bRet = false;
	CConnection* pConn = FindConnectionPtr(nConnId);
	bRet = (pConn != NULL);
	if (bRet)
	{
		FPASSERTMSG(pSecTokenReq == NULL, "Null pSecTokenReq");

		struct tm * timeinfo;
		TICKS tick = SystemGetTickCount();
		pConn->m_v35Securityticket = tick;
		list<RvgwCredential>::const_iterator it;
		for ( it=rvgwCredentials.begin() ; it != rvgwCredentials.end(); it++ )
		{
			if((*it).m_port == pSecTokenReq->GetPort())
			{
				pConn->m_v35GateWayUser =(*it).m_userName ;
				pConn->m_v35GateWayPassword = (*it).m_password;
				break;
			}
		}
		v35Securityticket = tick;
	}
	UnLock();
	return bRet;

}



 bool CConnectionList::GetConnectionV35Details(const char* securityTokenId, string& clientIp, std::string& v35GateWayUser, std::string& v35GateWayPassword, TICKS& v35Securityticket)
 {
	Lock();

 	bool bRet;
 	CConnection* pConn = FindConnectionPtr(securityTokenId);
 	bRet = (pConn != NULL);
 	if (bRet)
 	{
		clientIp = pConn->GetClientIp();
 		v35GateWayUser = pConn->m_v35GateWayUser;
 		v35GateWayPassword = pConn->m_v35GateWayPassword;
 		v35Securityticket = pConn->m_v35Securityticket;
 	}
	UnLock();
 	return bRet;

 }


CConnection* CConnectionList::FindConnectionPtr(DWORD dwConnId)
{
	CConnectionsMap::iterator mapIterator;

	CConnection* retConn = NULL;

	if(m_pConnMap!=NULL && (mapIterator = m_pConnMap->find(dwConnId)) != m_pConnMap->end())
		retConn = (*mapIterator).second->m_pConn;

	return retConn;
}

bool CConnectionList::IsValidConnection(int nConnId)
{
	Lock();

	bool bRet = (FindConnectionPtr(nConnId) != NULL);

	UnLock();

	return bRet;
}

bool CConnectionList::IsConnectionAlive(int nConnId)
{
	bool bRet = false;

	Lock();

	CConnection* pConn = FindConnectionPtr(nConnId);

	if(pConn != NULL)
		bRet = pConn->IsAlive();

	UnLock();

	return bRet;
}

void CConnectionList::RemoveConnection(DWORD dwConnId)
{
	Lock();

	CConnectionsMap::iterator mapIterator;


	BOOL bFound=FALSE;
	if((mapIterator = m_pConnMap->find(dwConnId)) != m_pConnMap->end())
	{
		bFound = TRUE;
		delete (*mapIterator).second;
		m_pConnMap->erase(mapIterator);
		m_nCount--;
		IncreaseUpdateCounter();
	}
	if (bFound)
		FTRACESTR(eLevelInfoNormal) << "CConnectionList::RemoveConnection - the connection "<<dwConnId<<" has been removed , m_nUpdateCounter: " << m_nUpdateCounter;
	else
		FTRACESTR(eLevelInfoNormal) << "CConnectionList::RemoveConnection - the connection "<<dwConnId<<" has been not found and not removed";
	UnLock();
}

void CConnectionList::CleanDeadConnections(vector<WrapperConnection*> & outDeadConnections)
{
	WrapperConnection* pConn;
	bool bDeleted = false;

	Lock();

	if (m_pConnMap!=NULL)
	{
		CConnectionsMap::iterator it;

		bool bNoMoreLoops = false;

		while(!bNoMoreLoops)
		{
			bNoMoreLoops = true;

			for (it = m_pConnMap->begin(); it != m_pConnMap->end(); ++it)
			{
				pConn = (*it).second;

				if (pConn!=NULL && pConn->m_pConn != NULL)
				{
					if (!pConn->m_pConn->IsAlive())
					{
						FTRACESTR(eLevelInfoNormal) << "CConnectionList::CleanDeadConnections - the connection "<<pConn->m_pConn->GetConnectionId()<<" has been removed(no activity)";
                        outDeadConnections.push_back(pConn);
						m_pConnMap->erase(it);
						bDeleted = true;
						m_nCount--;
						bNoMoreLoops = false;
						break;
					}
				}
			}
		}

		for (it = m_pConnMap->begin(); it != m_pConnMap->end(); ++it)
		{
			pConn = (*it).second;

			if (pConn!=NULL)
				pConn->m_pConn->ResetAlive();
		}
	}

	if(bDeleted)
		IncreaseUpdateCounter();

	UnLock();
}

WORD CConnectionList::GetAuthorization(int nConnId)
{
	WORD nAuthorization = GUEST;

	Lock();

	CConnection* pConn = FindConnectionPtr(nConnId);

	if(pConn!=NULL)
		nAuthorization = pConn->GetAuthorization();

	UnLock();

	return nAuthorization;
}

std::string CConnectionList::GetLogin(int nConnId)
{
	std::string strLogin = "";

	Lock();

	CConnection* pConn = FindConnectionPtr(nConnId);

	if(pConn)
		strLogin = pConn->GetLogin();

	UnLock();

	return strLogin;
}

bool CConnectionList::GetSupportCompression(int nConnId)
{
	bool bSupportConnection = false;

	Lock();

	CConnection* pConn = FindConnectionPtr(nConnId);

	if(pConn)
		bSupportConnection = pConn->GetSupportCompression();

	UnLock();

	return bSupportConnection;
}

void CConnectionList::SetConnectionAlive(int nConnId)
{
	Lock();

	CConnection* pConn = FindConnectionPtr(nConnId);
	if (pConn!=NULL)
		pConn->SetAlive();

	UnLock();
}

void CConnectionList::SerializeXml(CXMLDOMElement*& pActionNode, DWORD dwObjectToken)
{
	BYTE bChanged = FALSE;
	Lock();

	BYTE bJitcMode = CApacheModuleProcess::IsFederalOn();

	DWORD reqConnId = 0;
	CConnection* pReqConn = NULL;

	if(bJitcMode)
	{
		char pszError[64]="";
		std::string reqUserName = "";
		int nStatus = pActionNode->GetAndVerifyChildNodeValue("REQ_USER", reqUserName, pszError, _0_TO_DWORD);
		if(nStatus == STATUS_OK)
		{
			CConnectionList* pConnList = CApacheModuleEngine::GetConnectionList();
			pReqConn = pConnList->FindConnectionByLoginName(reqUserName.c_str());
			if(pReqConn)
			reqConnId = pReqConn->GetConnectionId();
		}
	}

    CXMLDOMElement *pListNode=pActionNode->AddChildNode("CONNECTIONS_LIST");
	pListNode->AddChildNode("OBJ_TOKEN",m_nUpdateCounter);

	if (dwObjectToken == 0xFFFFFFFF)
		bChanged = TRUE;
	else
	{
		if(m_nUpdateCounter > dwObjectToken)
			bChanged=TRUE;
	}

	pListNode->AddChildNode("CHANGED",bChanged,_BOOL);

	if(bChanged)
	{
		for (CConnectionsMap::const_iterator it = m_pConnMap->begin(); it != m_pConnMap->end();++it)
		{
			CConnection* pConn = (*it).second->m_pConn;
			if (pConn!=NULL)
			{
				if(bJitcMode)
				{
					if((pReqConn && SUPER == pReqConn->GetAuthorization()) ||
						reqConnId == (DWORD)(pConn->GetConnectionId()))
					{
						pConn->SerializeXml(pListNode);
					}
				}
				else
					pConn->SerializeXml(pListNode);
			}
		}
	}
	UnLock();
}

int CConnectionList::DeSerializeXml(CXMLDOMElement *pConnectionListNode,char* pszError, const char* action)
{
	//no need for DeSerializeXml in carmel side
	return STATUS_OK;
}

std::string CConnectionList::WriteConnections()
{
	char msg[256];

	std::string str_connections = "";
	if (m_nCount==0)
		str_connections = "No connection exist on the carmel\n";
	else
	{
		for (CConnectionsMap::const_iterator it = m_pConnMap->begin(); it != m_pConnMap->end();++it)
		{
			CConnection* pConn = (*it).second->m_pConn;
			snprintf(msg, sizeof(msg), "Connection Id = %d User name = %s Station = %s Client Ip = %s\n",
                    pConn->GetConnectionId(), pConn->GetLogin(), pConn->GetStationName(), pConn->GetClientIp());

			str_connections += msg;
		}
	}

	return str_connections;
}

bool CConnectionList::GetConnectionQueues(int nConnId,COsQueue** pReadQueue,COsQueue** pWriteQueue)
{
	bool status = false;
	Lock();

	ConnectionResource* pConn = FindConnectionResourcePtr(nConnId);

	if(pConn)
	{
		*pReadQueue = pConn->GetReadQueue();
		*pWriteQueue = pConn->GetWriteQueue();
		if(*pReadQueue && *pWriteQueue)
			status = true;
		else
			status = false;
	}
	else
		status = false;

	UnLock();
	return status;

}

ConnectionResource* CConnectionList::FindConnectionResourcePtr(DWORD dwConnId)
{
	CConnectionsMap::iterator mapIterator;

	ConnectionResource* retConn = NULL;

	if(m_pConnMap!=NULL && (mapIterator = m_pConnMap->find(dwConnId)) != m_pConnMap->end())
		retConn = (*mapIterator).second->m_pConnResource;

	return retConn;
}

void CConnectionList::UpdateStationNameConnection(int nConnId,const char* pszStationName)
{
	Lock();

	CConnectionsMap::iterator mapIterator;


	if(m_pConnMap!=NULL && (mapIterator = m_pConnMap->find(nConnId)) != m_pConnMap->end())
			(*mapIterator).second->m_pConn->SetStationName(pszStationName);


	UnLock();
}

void CConnectionList::IncreaseRequestCounter(int nConnId, int nMethod)
{
	Lock();

	CConnection* pConn = FindConnectionPtr(nConnId);

	if(pConn)
		pConn->IncreaseRequestCounter(nMethod);

	UnLock();
}

int CConnectionList::GetRequestCounter(int nConnId, int nMethod)
{
	int nCounter = 0;

	Lock();

	CConnection* pConn = FindConnectionPtr(nConnId);

	if(pConn)
		nCounter = pConn->GetRequestCounter(nMethod);

	UnLock();

	return nCounter;
}

int CConnectionList::GetConnectionDetails(int nConnId, ConnectionDetails_S & outConnDetails)
{
    int ret = -1;

    Lock();

    CConnection *pConn = FindConnectionPtr(nConnId);
	if(NULL != pConn)
    {
        ret = 0;
        outConnDetails.userName = pConn->GetLogin();
        outConnDetails.workStation = pConn->GetStationName();
        outConnDetails.clientIp = pConn->GetClientIp();
    }

    UnLock();

    return ret;
}
CConnection* CConnectionList::GetConnectionPtr(DWORD dwConnId)
{
	CConnection* pConn=NULL;
	Lock();
	  pConn = FindConnectionPtr(dwConnId);
	UnLock();
	return pConn;
}

int CConnectionList::GetNumConnectionByLoginName(const char* user_name)
{
	Lock();
	int count = 0;
	for (CConnectionsMap::const_iterator it = m_pConnMap->begin(); it != m_pConnMap->end();++it)
	{
		CConnection* pConn = (*it).second->m_pConn;
		if (pConn!=NULL)
		{
			if (strcmp(pConn->GetLogin(), user_name) == 0)
				count++;
		}
	}
	UnLock();
	return count;
}

//////////////////////////////////////////////////////////////////////
CConnection* CConnectionList::FindConnectionByLoginName(const char* user_name)
{
	CConnection* pResultConn = NULL;
	for (CConnectionsMap::const_iterator it = m_pConnMap->begin(); it != m_pConnMap->end();++it)
	{
		CConnection* pConn = (*it).second->m_pConn;
		if (pConn!=NULL)
		{
			if (strcmp(pConn->GetLogin(), user_name) == 0)
			{
				pResultConn = pConn;
				break;
			}
		}
	}
	return pResultConn;
}

CConnection* CConnectionList::FindConnectionPtr(const char* securityTokenId)
{
	
	char buffer[SECURITY_TOKEN_MAX]= {0};
	CConnection* pResultConn = NULL;
		for (CConnectionsMap::const_iterator it = m_pConnMap->begin(); it != m_pConnMap->end();++it)
		{
			CConnection* pConn = (*it).second->m_pConn;
			if (pConn!=NULL)
			{				
				sprintf(buffer,"%u",pConn->m_v35Securityticket.GetMiliseconds());
				if(strcmp(securityTokenId,buffer)==0)
				{
					pResultConn = pConn;
					break;
				}
				memset ( buffer, 0, SECURITY_TOKEN_MAX );
			}
		}
		return pResultConn;
}

