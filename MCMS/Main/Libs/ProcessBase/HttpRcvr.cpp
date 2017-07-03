// HTTPRcvr.cpp: implementation of the CHTTPReceiver class.
//
//////////////////////////////////////////////////////////////////////
#include "HttpRcvr.h"
#include "HTTPPars.h"
#include "HTTPDefi.h"
#include "SocketRxTask.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTTPReceiver::CHTTPReceiver(WORD bResponseReceiver )
{
	m_pHttpContent = NULL;
	m_HttpContentAllocatedLength = 0; 
	m_pHttpHeader = NULL;
	m_HttpHeaderAllocatedLength = 0; 
	m_nReadInHeader = 0;
	m_pEndOfHeader =NULL;
	m_bResponseReceiver = bResponseReceiver;
	m_HttpType = HTTP_TYPE_UNKNOWN;
	m_HttpContentLength = 0;

	IncreaseAllocationOfHeaderBuffer();
}

CHTTPReceiver::~CHTTPReceiver()
{
	PDELETEA(m_pHttpContent);
	PDELETEA(m_pHttpHeader);
}


WORD CHTTPReceiver::ReadHttp(CSocketRxTask &Socket)
{
	m_HttpType = HTTP_TYPE_UNKNOWN;
	m_HttpContentLength = 0; 
	m_pHttpHeader[0] = '\0';
	m_pEndOfHeader = NULL;

	if (m_pHttpContent != NULL)
		m_pHttpContent[0] = '\0';


	//if (m_socket.GetSocketHandle()==0)  //socket handle has not been set
	//	return FALSE;

	if (!ReadHeader(Socket))
		return FALSE;

	if (!CheckHttpType())
		return FALSE;

	if ((m_HttpType == HTTP_TYPE_POST)  || (m_HttpType == HTTP_TYPE_RESPONSE))
	{
		if (!CHTTPHeaderParser::GetContentLength(m_pHttpHeader, &m_HttpContentLength))
			return FALSE;

		AllocateContentBuffer();

		return ReadContent(Socket);
	}
	else
		return TRUE; //no need to read content for get or head requests, because they have no content
}

WORD CHTTPReceiver::ReadHeader(CSocketRxTask &Socket)
{
	int nRead;
	int recvSize = m_HttpHeaderAllocatedLength;
	char* recvBuffer = m_pHttpHeader;
	m_nReadInHeader = 0;
	STATUS status;

	m_pEndOfHeader = NULL;

	//PSelf	me; 

	while(1)
	{
/*
 *	When the wouldblock is true, we had bad http response and bad connections when using the api. 
 *  When the wouldblock is false, we solved the bad connection and external db problems, but 
 *  we had browse information problems (with headers)
 *  Changing it back to TRUE and adding retries loop solved the external db and browse info problems. 
 */
		//add later 
		status = Socket.Read(recvBuffer,recvSize,nRead,TRUE);

		m_nReadInHeader += nRead;

		if (GetEndOfHeader())
		{
			//PTRACE2(eLevelInfoNormal,"CHTTPReceiver::ReadHeader - end of header ",m_pHttpHeader);
			return TRUE;
		}
		
		//if we didn't read a thing, or socket finished. 
		if (status==STATUS_FAIL)
		{
			FTRACESTR(eLevelInfoNormal) << "CHTTPReceiver::ReadHeader - STATUS_FAIL - we didn't read a thing, or socket finished";
			return FALSE;
		}
/*		if (nRead < recvSize) 
		{
		//although we read everything in the socket, we didn't find the end of the header...
			PTRACE(eLevelError, "CHTTPReceiver::ReadHeader ,didn't find end of header");
			return FALSE;   
		}*/

		IncreaseAllocationOfHeaderBuffer();

		recvSize = HEADER_BUF_SIZE;
		recvBuffer = m_pHttpHeader + m_nReadInHeader;
	}
}

WORD CHTTPReceiver::GetEndOfHeader()
{
	m_pEndOfHeader = strstr(m_pHttpHeader,"\r\n\r\n");
	if (m_pEndOfHeader!=NULL)
	{
		m_pEndOfHeader+=4;	//moving to the end of the header
		return TRUE;
	}

	m_pEndOfHeader = strstr(m_pHttpHeader,"\r\n\n");
	if (m_pEndOfHeader!=NULL)
	{
		m_pEndOfHeader+=3;	//moving to the end of the header
		return TRUE;
	}
	
	if(m_bResponseReceiver == TRUE)
	{
		m_pEndOfHeader = strstr(m_pHttpHeader,"<RESPONSE");
		if (m_pEndOfHeader!=NULL)
			return TRUE;

		m_pEndOfHeader = strstr(m_pHttpHeader,"<CONFIRM");
		if (m_pEndOfHeader!=NULL)
			return TRUE;
	}
	else
	{
		m_pEndOfHeader = strstr(m_pHttpHeader,"<TRANS");
		if (m_pEndOfHeader!=NULL)
			return TRUE;

		m_pEndOfHeader = strstr(m_pHttpHeader,"<REQUEST");
		if (m_pEndOfHeader!=NULL)
			return TRUE;
	}

	return FALSE; //didn't find the end of the header!!!!
}

WORD CHTTPReceiver::CheckHttpType()
{
	if (m_bResponseReceiver == TRUE)
	{
		m_HttpType = HTTP_TYPE_RESPONSE;
		return TRUE;
	}
	else 
	{
		return CHTTPHeaderParser::GetHttpRequestType(m_pHttpHeader, &m_HttpType);
	}
}

WORD CHTTPReceiver::ReadContent(CSocketRxTask &Socket)
{
	char*   pEndContentInHeader;
	DWORD	headerLength=0, remainingLength=0;
	int	nRead;

	headerLength=m_pEndOfHeader-m_pHttpHeader;
	remainingLength=m_nReadInHeader-headerLength;
	
	if (remainingLength>m_HttpContentLength)   //there's more left in the header, than the real content
		remainingLength = m_HttpContentLength;

	//copy the part of the content that has been read with the header
	memcpy(m_pHttpContent,m_pEndOfHeader,remainingLength);
	
	m_pEndOfHeader[0]='\0';	//cut the header so that it retains only the header and no content

	if (remainingLength < m_HttpContentLength)
	{ 
		//read what still needs to be read from socket
		pEndContentInHeader = m_pHttpContent + remainingLength; 
		int status=Socket.Read(pEndContentInHeader,m_HttpContentLength-remainingLength,nRead,FALSE);
		
		if (status==STATUS_FAIL)
		{
			FTRACESTR(eLevelInfoNormal) << "CHTTPReceiver::ReadContent - failed to read content";
			return FALSE;
		}
	}
	//PTRACE2(eLevelInfoNormal,"CHTTPReceiver::ReadContent - end of header ",m_pHttpContent);
	return TRUE;
}


void CHTTPReceiver::AllocateContentBuffer()
{
	DWORD neededAllocatedLength = m_HttpContentLength+1;
	if (neededAllocatedLength <= m_HttpContentAllocatedLength)
	{
		m_pHttpContent[m_HttpContentLength]= '\0';
		return;
	}

	if (m_pHttpContent != NULL)
		delete [] m_pHttpContent;

	m_pHttpContent = new char[neededAllocatedLength];
	memset(m_pHttpContent, 0, neededAllocatedLength);

	m_HttpContentAllocatedLength = neededAllocatedLength;
}


void CHTTPReceiver::IncreaseAllocationOfHeaderBuffer()
{
	char* tempHeader;
	int oldAllocatedLenght = m_HttpHeaderAllocatedLength;

	m_HttpHeaderAllocatedLength += HEADER_BUF_SIZE;

	tempHeader = new char[m_HttpHeaderAllocatedLength+1];
	memset(tempHeader, 0, m_HttpHeaderAllocatedLength+1);

	if (m_pHttpHeader != NULL)
	{
		strncpy(tempHeader, m_pHttpHeader, oldAllocatedLenght);
		delete [] m_pHttpHeader;
		m_pEndOfHeader = NULL;
	}
	m_pHttpHeader = tempHeader; 
}

void CHTTPReceiver::SetSocketHandle(int socketDesc)
{
	//m_socket.SetSocketHandle(socketDesc);
}

char* CHTTPReceiver::GetHttpContent()
{
	return m_pHttpContent;
}

char* CHTTPReceiver::GetHttpHeader()
{
	return m_pHttpHeader;
}
