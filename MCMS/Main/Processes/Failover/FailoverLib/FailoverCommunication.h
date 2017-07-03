/*
 * FailoverCommunication.h
 *
 *  Created on: Aug 31, 2009
 *      Author: yael
 */

#ifndef FAILOVERCOMMUNICATION_H_
#define FAILOVERCOMMUNICATION_H_

#include "ClientSocket.h"
#include "FailoverProcess.h"



class CFailoverCommunication : public CPObject
{
	CLASS_TYPE_1(CFailoverCommunication, CPObject)
public:
	CFailoverCommunication();
	virtual ~CFailoverCommunication();
	virtual const char*  NameOf() const {return "CFailoverCommunication";}

	BYTE IsConfigurationReady();
	void InitSocket(CTaskApp* pCreatorTask);
	void FreeSocket();

	void SetSecured(BYTE bIsSecured);
	void SetUserAndPassword(char* loginUser, char* loginPassword);
	void SetIp(std::string strIp);
	void SetSocketDropped();

	const char* GetStrIp();
	const char* GetStrUrl();
	const char* GetStrUser();
	const char* GetStrPassword();

	BOOL SendToSocket(char *& pRequest);
	void ConnectOrReconnectSocket();
	void ConnectSocket();
	void ReconnectSocket();
	void DisconnectSocket();

private:

	void RemovePasswordFromString(const char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag);

	CFailoverProcess *m_pProcess;
	CClientSocket* m_pSocket;

	std::string    m_loginUser;
	std::string    m_loginPassword;
	std::string    m_strIP;
	std::string    m_strUrl;
	DWORD   m_ip;
	WORD    m_port;
	BYTE	m_bIsSecured;

	BOOL    m_bIsSocketDropped;

/*
	POST http://172.22.191.95:80 HTTP/1.1
	Content-Type: text/xml
	Content-Length: 235
	Cookie: XMLMCUToken=571;Path=/
	Pragma: MESSAGE_ID=39232
	Authorization: Basic U1VQUE9SVDpTVVBQT1JU
	Host: 172.22.191.95:80
	Server: PolycomHTTPServer
	Connection: Keep-Alive
	Cache-control: private*/

	std::string m_strBasicAuthorization;
};

#endif /* FAILOVERCOMMUNICATION_H_ */
