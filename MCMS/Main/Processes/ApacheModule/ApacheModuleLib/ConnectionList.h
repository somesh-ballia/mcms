// ConnectionList.h: interface for the CConnectionList class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(_ConnectionList_H__)
#define _ConnectionList_H__

#include <vector>
#include <map>

#include "apr_pools.h"

//#include "ApacheIncludes.h"


#include "StructTm.h"

#include "ActionRedirection.h"
#include "SegmentFuncForRequestHandler.h"
#include "OsQueue.h"
#undef min
#undef max


class CXMLDOMElement;
class CGateWaySecurityTokenRequest;
class CConnectionListMutexGuard;
struct RvgwCredential;


class CConnection {

public:

	CConnection();

	CConnection(const char* pszLogin, const char* pszStationName,
                const char *clientIp,
                WORD nAuthorization,
				CStructTm* pLoginGmtTime,
				bool bSupportCompression,
				BYTE bEntity,
				const bool isMachineAccount);

	CConnection(CConnection& other, int connectionId);

	const char* GetLogin() const;
	void SetLogin(const char* pszLogin);

	const char* GetStationName() const;
	void SetStationName(const char* pszStationName);

    const char* GetClientIp() const;
	void SetClientIp(const char* pszClientIp);


	WORD GetAuthorization();
	void SetAuthorization(WORD nAuthorization);

	CStructTm GetLoginGmtTime();
	void SetLoginGmtTime(CStructTm loginGmtTime);

	bool GetSupportCompression();
	void SetSupportCompression(bool bSupportCompression);

	bool IsAlive();
	void SetAlive();
	void ResetAlive();

	int GetConnectionId() const;

	CConnection& operator=(CConnection& other);

	void SerializeXml(CXMLDOMElement* pNode);
	void IncreaseRequestCounter(int nMethod);
	int GetRequestCounter(int nMethod);

	void SetEntity(BYTE bEntity);
	BYTE GetEntity();

	bool GetMachineAccount() const;
	void SetMachineAccount(const bool bMachineAccount);


	std::string	GetV35GateWayUser() const;
	void	SetV35GateWayUser(const std::string v35GateWayUser);

	std::string	GetV35GateWayPassword() const;
	void	SetV35GateWayPassword(const std::string v35GateWayPassword);

	TICKS	GetV35Securityticket() const;
	void	SetV35Securityticket(const TICKS v35Securityticket);

private:

	void SetLoginGmtTimeToCurrent();

private:

	int m_ConnectionId;
	std::string m_strLogin;
	std::string m_strStationName;
    std::string m_ClientIp;
	WORD m_nAuthorization;
	CStructTm m_loginGmtTime;
	int m_nAliveCounter;
	bool m_bSupportCompression;
	bool m_bHttpConnection;
	bool m_bSecuredConnection;
	int m_nRequestCounters[4];
	BYTE m_Entity;
	bool m_bMachineAccount;

private:

	std::string m_v35GateWayUser;
	std::string m_v35GateWayPassword;
	TICKS m_v35Securityticket;

	friend class CConnectionList;
};

class ConnectionResource
{
public:
	ConnectionResource(COsQueue *pWriteQueue,COsQueue *pReadQueue):m_pReadQueue(pReadQueue),m_pWriteQueue(pWriteQueue){};
	~ConnectionResource();
	void    SetWriteQueue(COsQueue *pQueue);
	void    SetReadQueue(COsQueue *pQueue);
	COsQueue * GetWriteQueue();
	COsQueue *  GetReadQueue();
	void     InitQueues();

private:

	COsQueue *m_pReadQueue;
	COsQueue *m_pWriteQueue;
};

class WrapperConnection
{
  public:
	WrapperConnection(CConnection* conn,ConnectionResource* resource):m_pConnResource(resource),m_pConn(conn){};
	~WrapperConnection();
	ConnectionResource* m_pConnResource;
	CConnection* 		m_pConn;
};
typedef std::map<DWORD , WrapperConnection*> CConnectionsMap;

class CConnectionList {

public:

	CConnectionList(apr_pool_t *pConfigPool = NULL);
	~CConnectionList();
	int GetCount();

	int AddConnection(CConnection& connection,ConnectionResource* pConResource);
	void UpdateStationNameConnection(int nConnId,const char* pszStationName);
	bool IsValidConnection(int nConnId);
	int GetUpdateCounter();
	void RemoveConnection(DWORD dwConnId);
	void CleanDeadConnections(vector<WrapperConnection*> & outDeadConnections);
	WORD GetAuthorization(int nConnId);
	bool GetSupportCompression(int nConnId);
	std::string GetLogin(int nConnId);
	void SetConnectionAlive(int nConnId);
	void SerializeXml(CXMLDOMElement*& pActionNode, DWORD dwObjectToken);
	int DeSerializeXml(CXMLDOMElement *pConnectionListNode,char* pszError, const char* action);
	bool IsConnectionAlive(int nConnId);
	std::string WriteConnections();
	void IncreaseRequestCounter(int nConnId, int nMethod);
	int GetRequestCounter(int nConnId, int nMethod);

    int GetConnectionDetails(int nConnId, ConnectionDetails_S & outConnDetails);

    int GetNumConnectionByLoginName(const char* user_name);

    bool GetConnectionAuditDetails(int nConnId, string& strLoginName, string& strStationName, string& clientIp)  ;


    bool SetConnectionV35Details(int nConnId, const CGateWaySecurityTokenRequest* pSecTokenReq, const list<RvgwCredential>& rvgwCredentials, TICKS& v35Securityticket) ;

    bool GetConnectionV35Details(const char* securityTokenId, string& clientIp, std::string& v35GateWayUser, std::string& v35GateWayPassword, TICKS& v35Securityticket) ;

    bool GetConnectionQueues(int nConnId,COsQueue** pReadQueue,COsQueue** pWriteQueue);
    CConnection* GetConnectionPtr(DWORD dwConnId);
private:
    CConnection* FindConnectionPtr(DWORD dwConnId);
    ConnectionResource* FindConnectionResourcePtr(DWORD dwConnId);
    CConnection* FindConnectionPtr(const char* securityTokenId);
	void IncreaseUpdateCounter();
	void IncreaseHttpConnectionId();
	int  GetcountWithoutAnonymousConnections();
	void Lock();
	void UnLock();


private:

	CConnection* FindConnectionByLoginName(const char* user_name);

	int m_nCount;
	CConnectionsMap* m_pConnMap;
//	std::valarray<CConnection*> *m_pConnArray;
	DWORD m_nUpdateCounter;
	static apr_thread_mutex_t* m_pAccessMutex;

	DWORD m_nLastHttpConnectionId;
	
	
};



#endif // !defined(_ConnectionList_H__)

