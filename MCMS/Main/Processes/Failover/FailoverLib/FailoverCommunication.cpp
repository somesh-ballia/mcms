/*
 * FailoverCommunication.cpp
 *
 *  Created on: Aug 31, 2009
 *      Author: yael
 */

#include "FailoverCommunication.h"
#include "TraceStream.h"
#include "SecuredSocketClient.h"

extern "C" void FailoverRxEntryPoint(void* appParam);
extern "C" void FailoverTxEntryPoint(void* appParam);



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFailoverCommunication::CFailoverCommunication()
{
	m_pProcess = (CFailoverProcess*)(CProcessBase::GetProcess());

	m_port = 0;
	m_strIP = "";
	m_strUrl = "";
	m_loginUser = "";
	m_loginPassword = "";

	m_bIsSocketDropped = FALSE;

	m_pSocket = NULL;
	m_bIsSecured = FALSE;
	m_ip = 0;
}

/////////////////////////////////////////////////////////////////////////////
CFailoverCommunication::~CFailoverCommunication()
{
	POBJDELETE(m_pSocket);
}

//////////////////////////////////////////////////////////////////////
void CFailoverCommunication::SetSecured(BYTE bIsSecured)
{
	eProductType curProductType  = CProcessBase::GetProcess()->GetProductType();
	
	m_bIsSecured = bIsSecured;

	if (m_bIsSecured)
	{
		/*BRIDGE-1896:  to support the Gesher and Ninja*/
		//BRIDGE-13305 - in 8.5 default port is 443 for all products except MFW , MFW does not support Hot backup
		m_port = 443;
	}
	else
	{
		/*BRIDGE-1896:  to support the Gesher and Ninja*/
		//BRIDGE-13305 - in 8.5 default port is 443 for all products except MFW , MFW does not support Hot backup
		m_port = 80;
	}

	if (m_bIsSecured)
		m_strUrl = "https:";
	else
		m_strUrl = "http:";
}

//////////////////////////////////////////////////////////////////////
void CFailoverCommunication::SetIp(std::string strIp)
{
	m_strIP =  strIp;

	DWORD dwTempIp = SystemIpStringToDWORD(strIp.c_str(), eNetwork);
	m_ip = dwTempIp;
}

//////////////////////////////////////////////////////////////////////
void CFailoverCommunication::SetUserAndPassword(char* loginUser, char* loginPassword)
{
	m_loginUser = loginUser;
	m_loginPassword = loginPassword;
}

//////////////////////////////////////////////////////////////////////
BYTE CFailoverCommunication::IsConfigurationReady()
{
	if ( (m_strIP.length() != 0) && (m_strUrl.length() != 0) && (m_loginUser.length() != 0) && (m_loginPassword.length() != 0))
		return TRUE;
	else
		return FALSE;
}

//////////////////////////////////////////////////////////////////////
void CFailoverCommunication::InitSocket(CTaskApp* pCreatorTask)
{
	char strPort[10]={0};
	snprintf(strPort,sizeof(strPort)-1,"%d" ,m_port);

	if (m_bIsSecured)
		m_strUrl = "https:";
	else
		m_strUrl = "http:";
	m_strUrl += "//" + m_strIP + ":" + strPort;



	if (m_bIsSecured)
	{
		CSecuredSocketClient* pSecureClientSock = new CSecuredSocketClient();
		pSecureClientSock->SetRequestPeerCertificate(m_pProcess->GetRequestPeerCertificate());
		m_pSocket = new CClientSocket(pCreatorTask, FailoverRxEntryPoint, FailoverTxEntryPoint, pSecureClientSock);
	}
	else
		m_pSocket = new CClientSocket(pCreatorTask, FailoverRxEntryPoint, FailoverTxEntryPoint);

	m_pSocket->Init(GetStrIp(), m_port);

	m_pSocket->SetRetryTime(1 * SECOND);
	m_pSocket->SetConnectionRetriesNumber(1);
	FTRACESTR(eLevelInfoNormal) << "\nCFailoverCommunication::InitSocket \nServer Ip - "
							<< GetStrIp() << " using the port - "<< m_port
							<< " IsSecured = " << (m_bIsSecured ? "yes" : "no") << " Url = "<< GetStrUrl();
}

//////////////////////////////////////////////////////////////////////
void CFailoverCommunication::FreeSocket()
{
	if (m_pSocket != NULL) {
		POBJDELETE(m_pSocket);
	}
}

/////////////////////////////////////////////////////////////////
const char* CFailoverCommunication::GetStrIp()
{
    return m_strIP.c_str();
}

/////////////////////////////////////////////////////////////////
const char* CFailoverCommunication::GetStrUrl()
{
    return m_strUrl.c_str();
}

/////////////////////////////////////////////////////////////////
const char* CFailoverCommunication::GetStrUser()
{
    return m_loginUser.c_str();
}

/////////////////////////////////////////////////////////////////
const char* CFailoverCommunication::GetStrPassword()
{
    return m_loginPassword.c_str();
}

/////////////////////////////////////////////////////////////////
void CFailoverCommunication::SetSocketDropped()
{
	m_bIsSocketDropped = TRUE;
	/*Need to drop the connection of the socket:  2012/2/29 David Liang*/
	m_pSocket->Dropconnect();

}

////////////////////////////////////////////////////////////////////////////
void CFailoverCommunication::ConnectOrReconnectSocket()
{
	if (m_pSocket->IsNotConnected() == TRUE) //only in case of idle
		m_pSocket->Connect();
	else if (m_bIsSocketDropped)
		ReconnectSocket();
}


/////////////////////////////////////////////////////////////////////////////
void CFailoverCommunication::ConnectSocket()
{
	if (m_pSocket->IsNotConnected() == TRUE) //only in case of idle
		m_pSocket->Connect();
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverCommunication::ReconnectSocket()
{
	m_bIsSocketDropped = FALSE;
	DisconnectSocket();
	ConnectSocket();
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverCommunication::DisconnectSocket()
{
	if (m_pSocket != NULL)
		if (m_pSocket->IsNotConnected() == FALSE) //only in case of connected or connecting
			m_pSocket->Disconnect();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CFailoverCommunication::SendToSocket(char *& pRequest)
{
	if ((m_pSocket==NULL) || (m_pSocket->IsConnected() == FALSE))
	{
		TRACESTR(eLevelInfoNormal) << "\nCFailoverCommunication - SendToSocket - SOCKET IS NOT CONNECTED!!!";
		return FALSE;
	}

	CSegment segment;

    //AddEncodingType(pRequest);

	// put parameters to message
	DWORD len = (DWORD)strlen(pRequest);
	segment  << len;
	segment  << pRequest;
	segment  << (DWORD)m_port;
	segment  << strlen(GetStrIp());
	segment  << GetStrIp();
	segment  << strlen(GetStrUrl());
	segment  << GetStrUrl();
	segment  << strlen(GetStrUser());
	segment  << GetStrUser();
	segment  << strlen(GetStrPassword());
	segment  << GetStrPassword();

	if (strstr(pRequest, "<PASSWORD>"))
	{
		ALLOCBUFFER(pszRequestWithShadowPassword, strlen(pRequest)+10);
		memset(pszRequestWithShadowPassword, 0, strlen(pRequest)+10);

		RemovePasswordFromString(pRequest, pszRequestWithShadowPassword, "<PASSWORD>", "</PASSWORD>");
		FTRACESTR(eLevelInfoNormal) << "\nCFailoverCommunication::SendToSocket \nServer Ip - "
								<< GetStrIp() << " using the port - "<< m_port<<"\n"
								<< pszRequestWithShadowPassword;
		DEALLOCBUFFER(pszRequestWithShadowPassword);
	}
	else
		FTRACESTR(eLevelInfoNormal) << "\nCFailoverCommunication::SendToSocket \nServer Ip - "
								<< GetStrIp() << " using the port - "<< m_port<<"\n"
								<< pRequest;

	m_pSocket->Send(&segment);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverCommunication::RemovePasswordFromString(const char* pszSearchString, char* pszNewString, const char* pszElementOpenTag, const char* pszElementCloseTag)
{
	char* pszStartPassword = (char*)strstr(pszSearchString,pszElementOpenTag);
	if (pszStartPassword)	//if password exist in the transaction
	{
		pszStartPassword += strlen(pszElementOpenTag);
		char *pszEndPassword = (char*)strstr(pszStartPassword,pszElementCloseTag);

		if(pszEndPassword)
		{
			strncpy(pszNewString, pszSearchString, pszStartPassword-pszSearchString);
			pszNewString[pszStartPassword-pszSearchString] = '\0';
			strcat(pszNewString, "*********");
			char* pszStartPassword2 = (char*)strstr(pszEndPassword,pszElementOpenTag);	//look if the password appear twice
			if (pszStartPassword2)
			{
				pszStartPassword2 += strlen(pszElementOpenTag);
				char *pszEndPassword2 = (char*)strstr(pszStartPassword2,pszElementCloseTag);

				if (pszEndPassword && pszEndPassword2)
				{
					strncat(pszNewString, pszEndPassword, pszStartPassword2-pszEndPassword);
					strcat(pszNewString, "*********");
					strcat(pszNewString, pszEndPassword2);
				}
				else
					PTRACE(eLevelError,"CFailoverCommunication::RemovePasswordFromString - either pszEndPassword or pszEndPassword2 is NULL");
			}
			else
				strcat(pszNewString, pszEndPassword);
		}
	}
}

