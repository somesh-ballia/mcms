/*
 * FailoverRxSocket.h
 *
 *  Created on: Sep 1, 2009
 *      Author: yael
 */

#ifndef FAILOVERRXSOCKET_H_
#define FAILOVERRXSOCKET_H_

#include "SocketRxTask.h"
#include "HttpRcvr.h"
#include "psosxml.h"

class CFailoverProcess;

// task entry point
extern "C" void FailoverRxEntryPoint(void* appParam);



class CFailoverRxSocket : public CSocketRxTask , public CHTTPReceiver
{
CLASS_TYPE_1(CFailoverRxSocket, CSocketRxTask )
public:

	CFailoverRxSocket(COsSocketConnected * pSocketDesc);
	virtual ~CFailoverRxSocket();

	void* GetMessageMap();

	const char* NameOf() const {return "CFailoverRxSocket";}
	const char * GetTaskName() const;

	virtual void ReceiveFromSocket();

protected:
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();
	
	void DispatchResponse(const char* rspnsStr, const char *pContent, CXMLDOMElement* pRoot);

	void TreatResponseTransMcu(CXMLDOMElement* pRootElement, const char *pContent);
	void TreatResponseOther(CXMLDOMElement* pRootElement, const char *pContent);

	STATUS DecompressedTheContent(char* pszNonZippedBuffer,
	                                                   int pszNonZippedBufferLen,
													   char* pszRequestContent,
													   int nContentLen);
	CFailoverProcess *m_pProcess;

	PDECLAR_MESSAGE_MAP
};


#endif /* FAILOVERRXSOCKET_H_ */
