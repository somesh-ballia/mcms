
#if !defined(_QAMANAGER_H__)
#define _QAMANAGER_H__

#include "ManagerTask.h"

class CClientSocket;
class CAppServerData;
class CurlHTTP;

void QAAPIManagerEntryPoint(void* appParam);

class CQAAPIManager : public CManagerTask
{
CLASS_TYPE_1(CQAAPIManager,CManagerTask )
public:
	CQAAPIManager();
	virtual ~CQAAPIManager();

	virtual const char* NameOf() const { return "CQAAPIManager";}
	void  Create(CSegment& appParam);	// called by entry point
	void SelfKill();
	TaskEntryPoint GetMonitorEntryPoint();

	void ManagerPostInitActionsPoint();
	
private:
			// Action functions
	// Manager: connect card
	virtual void OnSocketConnectIdle(CSegment* pMsg);
	// Socket: connection established
	virtual void OnSocketConnectedSetup(CSegment* pMsg);
	// Socket: connection failed
	virtual void OnSocketFailedSetup(CSegment* pMsg);
	// Socket: connection dropped
	virtual void OnSocketDroppedSetup(CSegment* pMsg);
	virtual void OnSocketDroppedConnect(CSegment* pMsg);
	// Socket : message received
	virtual void OnSocketRcvIdle(CSegment* pMsg);
	virtual void OnSocketRcvSetup(CSegment* pMsg);
	virtual void OnSocketRcvConnect(CSegment* pMsg);
	// Logical module: forward message to MPL-API
	virtual void OnSocketSndConnect(CSegment* pMsg);
	virtual void OnSocketSndConnecting(CSegment* pMsg);
	virtual void OnSocketSndIdle(CSegment* pMsg);
	virtual void OnKeepAliveTimerConnected(CSegment* paramSegment) ;
	virtual void OnExtDbResponseAny(CSegment* pMsg);
	void HandleSendExtDBReqLocaly(CSegment* pMsg);
	void HandleConectionLessResponse();
	
	void OnSocketDropped(CSegment* pMsg);
	void BuildKeepAliveRequest();
	void SendReject(CSegment* pMsg);
	void SendToSocket(char *& pRequest);
	CXMLDOMElement* GetElementByName(CXMLDOMDocument *pDom, const char *elemName);
	
	virtual void DeclareStartupConditions();
	virtual void ManagerStartupActionsPoint();

	virtual BOOL selfKillOnMemoryExhaustion();

	void AddEncodingType(char *& pRequest);
    
	void ReplaceMcuIp(char *& pXMLRequest, DWORD mcuIp);


	STATUS HandleTerminalSendLoginResp(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalGetConnState(CTerminalCommand & command,std::ostream& answer);


	int GetKACnt()const{return m_KeepAliveCnt;}
	void IncKACnt();
	void ZeroKACnt();

	void OnKeepAliveFailure(int cndOfFailure);
	void OnKeepAliveOk();

	void RemovePasswordFromString(char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag);
    
	// Attributes
	CClientSocket*  m_pSocketConnection;
	CurlHTTP* m_pcurlhttpConn;
	CAppServerData* m_pAppServerData;
	char *	m_pKeepAliveReq;
	DWORD 	m_MngmntIp;
	DWORD 	m_KeepAliveCnt;
	BYTE 	m_isSecured;
	eIpType m_curIpType;
	std::string m_sMngmntNI;


public:	
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS	

};

#endif // !defined(_QAAPIManager_H__)
