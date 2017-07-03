// HTTPRcvr.h: interface for the CHTTPReceiver class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _HTTP_RCVR_
#define _HTTP_RCVR_

#include "PObject.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define HEADER_BUF_SIZE  256	//the maximum header size
class CSocketRxTask;


class CHTTPReceiver :public CPObject  
{
public:
	CHTTPReceiver(WORD bResponseReceiver = FALSE);
	virtual ~CHTTPReceiver();
	WORD	ReadHttp(CSocketRxTask &Socket);
	void	SetSocketHandle(int socketDesc);
	char*	GetHttpContent();
	char*	GetHttpHeader();
	virtual const char* NameOf() const { return "CHTTPReceiver";}

protected:
	//CClientSocket	m_socket;
	DWORD			m_HttpContentLength;
	char*			m_pEndOfHeader;
	char*			m_pHttpContent;
	char*			m_pHttpHeader;
	DWORD			m_HttpHeaderAllocatedLength;
	DWORD			m_HttpContentAllocatedLength;
	int				m_HttpType;
	int				m_nReadInHeader;
	WORD			m_bResponseReceiver; 

	WORD ReadHeader(CSocketRxTask & OSSocket);
	WORD ReadContent(CSocketRxTask & OSSocket);
	WORD CheckHttpType();
	void AllocateContentBuffer();
	void IncreaseAllocationOfHeaderBuffer();
	WORD GetEndOfHeader();
};

#endif // !defined(AFX_HTTPRCVR_H__D859E6DD_DE78_4E05_B9B8_EEC14EF28227__INCLUDED_)
