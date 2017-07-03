/*
 * FailoverTxSocket.cpp
 *
 *  Created on: Sep 1, 2009
 *      Author: yael
 */

#include "SocketApi.h"
#include "FailoverTxSocket.h"
#include "HTTPDefi.h"
#include "SecuredSocketConnected.h"

static const std::string base64CharsTable =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

PBEGIN_MESSAGE_MAP(CFailoverTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CFailoverTxSocket::OnWriteSocketAnycase)
PEND_MESSAGE_MAP(CFailoverTxSocket,CSocketTxTask);


/////////////////////////////////////////////////////////////////////////////
void FailoverTxEntryPoint(void* appParam)
{
	BYTE bSecured;
	CSegment* pSeg = (CSegment*)appParam;
	*pSeg >> (BYTE&)bSecured;
	pSeg->ResetRead();

	if (bSecured == TRUE)
	{
		CFailoverTxSocket*  pTxSocket = new CFailoverTxSocket(new CSecuredSocketConnected);
		pTxSocket->Create(*(CSegment*)appParam);
	}
	else
	{
		CFailoverTxSocket*  pTxSocket = new CFailoverTxSocket(new COsSocketConnected);
		pTxSocket->Create(*(CSegment*)appParam);
	}
}

/////////////////////////////////////////////////////////////////////////////
CFailoverTxSocket::CFailoverTxSocket(COsSocketConnected * pSocketDesc):CSocketTxTask(FALSE, pSocketDesc)
{
}


/////////////////////////////////////////////////////////////////////////////
CFailoverTxSocket::~CFailoverTxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////
void*  CFailoverTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////
const char* CFailoverTxSocket::GetTaskName() const
{
	return "CFailoverTxSocket";
}

/////////////////////////////////////////////////////////////////////////////
void CFailoverTxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
//void CFailoverRxSocket::HandleDisconnect()
//{
//	CSocketTask::HandleDisconnect();
//
//	CTaskApi*  pTaskApi = new CTaskApi;
//	pTaskApi->CreateOnlyApi(*m_pCreatorRcvMbx);
//	pTaskApi->SendMsg(NULL,SOCKET_DROPPED);
// 	pTaskApi->DestroyOnlyApi();
//	POBJDELETE(pTaskApi);
//}

//////////////////////////////////////////////////////////////////////
void CFailoverTxSocket::OnWriteSocketAnycase(CSegment* pMsg)
{
	DWORD len, contentLen, urlLen, userLen, passwordLen;
	DWORD port;
	*pMsg >> contentLen;

	ALLOCBUFFER(pRequest, contentLen+1);
	pRequest[contentLen] = '\0';
	*pMsg >> pRequest;
	*pMsg >> port;

	*pMsg >> len;
	ALLOCBUFFER(pIP, len+1);
	pIP[len] = '\0';
	*pMsg >> pIP;

	*pMsg >> urlLen;
	ALLOCBUFFER(strUrl, urlLen+1);
	strUrl[urlLen] = '\0';
	*pMsg >> strUrl;

	*pMsg >> userLen;
	ALLOCBUFFER(strUser, userLen+1);
	strUser[userLen] = '\0';
	*pMsg >> strUser;

	*pMsg >> passwordLen;
	ALLOCBUFFER(strPassword, passwordLen+1);
	strPassword[passwordLen] = '\0';
	*pMsg >> strPassword;

	AddPostHeader(strUrl);

	m_nContentType = CONTENT_TYPE_XML;
	AddContentTypeHeader();
	m_nContentLenght = contentLen;
	AddContentLengthHeader();

	AddAuthorizationHeader(strUser, strPassword);
	//Authorization: Basic U1VQUE9SVDpTVVBQT1JU


	AddHostHeader(pIP, port);
	AddLastCommonHeaders();

	/*STATUS status = Write((char*)m_pHTTPHeader,m_HTTPHeaderLen);
    if(STATUS_OK != status)
    {
        AssertSendFailure("FAILED to send a header length", status);
    }
    else
    {
        status = Write(pRequest, contentLen);
        if(STATUS_OK != status)
        {
            AssertSendFailure("FAILED to send a content", status);
        }
    }*/

	DWORD headerLen = m_HTTPHeaderLen;

	DWORD totalLen = m_HTTPHeaderLen + contentLen;
	ALLOCBUFFER(pHeaderAndContent, totalLen + 1);
	pHeaderAndContent[totalLen] = '\0';

	sprintf(pHeaderAndContent, "%s%s", m_pHTTPHeader, pRequest);

	STATUS status = Write(pHeaderAndContent, totalLen);
	if(STATUS_OK != status)
	{
		AssertSendFailure("FAILED to send header and content", status);
	}
	DEALLOCBUFFER(pHeaderAndContent);


    DEALLOCBUFFER(strUrl);
    DEALLOCBUFFER(strUser);
    DEALLOCBUFFER(pIP);
    DEALLOCBUFFER(strPassword);
    DEALLOCBUFFER(pRequest);
}

//////////////////////////////////////////////////////////////////////
void CFailoverTxSocket::AssertSendFailure(const char *str, STATUS status)const
{
    std::string  message = str;
    message += "\n";
    message += "Status : ";
    message += CProcessBase::GetProcess()->GetStatusAsString(status);
    PASSERTMSGONCE(TRUE, message.c_str());
}


//////////////////////////////////////////////////////////////////////
//Authorization: Basic U1VQUE9SVDpTVVBQT1JU
void CFailoverTxSocket::AddAuthorizationHeader(char* strUser, char* strPassword)
{
	std::string userAndPassword = strUser;
	userAndPassword += ":";
	userAndPassword += strPassword;
	std::string encodedStr = EncodeToBase64(userAndPassword.c_str());

	char strAuthorization[128];
	snprintf(strAuthorization, sizeof(strAuthorization), "Authorization: Basic %s\r\n", encodedStr.c_str());
	AddStrToHttpBuffer(strAuthorization);
}

//////////////////////////////////////////////////////////////////////
std::string CFailoverTxSocket::EncodeToBase64(const char* bytesToEncode)
{
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char charArray3[3];
	unsigned char charArray4[4];

	int initialStrLen = strlen(bytesToEncode);

	while (initialStrLen--)
	{
		charArray3[i++] = *(bytesToEncode++);
		if (i == 3)
		{
			charArray4[0] = (charArray3[0] & 0xfc) >> 2;
			charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
			charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
			charArray4[3] = charArray3[2] & 0x3f;

			for (i = 0; (i <4) ; i++)
				ret += base64CharsTable[charArray4[i]];

			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
		charArray3[j] = '\0';

		charArray4[0] = (charArray3[0] & 0xfc) >> 2;
		charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
		charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
		charArray4[3] = charArray3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64CharsTable[charArray4[j]];

		while (i++ < 3)
			ret += '=';
	}

	return ret;

}

