/*
 * FailoverTxSocket.h
 *
 *  Created on: Sep 1, 2009
 *      Author: yael
 */

#ifndef FAILOVERTXSOCKET_H_
#define FAILOVERTXSOCKET_H_
#include "SocketTxTask.h"
#include "httpBldr.h"

extern "C" void FailoverTxEntryPoint(void* appParam);

class CFailoverTxSocket : public CSocketTxTask ,public CHTTPHeaderBuilder
{
CLASS_TYPE_1(CFailoverTxSocket,CSocketTxTask )
public:

	CFailoverTxSocket(COsSocketConnected * pSocketDesc);
	virtual ~CFailoverTxSocket();

	void* GetMessageMap();

	const char* NameOf() const {return "CFailoverTxSocket";}
	const char* GetTaskName() const;


protected:
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();

	void OnWriteSocketAnycase(CSegment* pParam);


	PDECLAR_MESSAGE_MAP

private:
    void AssertSendFailure(const char *str, STATUS status)const;
    std::string EncodeToBase64(const char* bytesToEncode);
    void AddAuthorizationHeader(char* strUser, char* strPassword);
};


#endif /* FAILOVERTXSOCKET_H_ */
