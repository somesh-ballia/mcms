//+========================================================================+
//                            OsSocketConnected.h                          |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       OsSocketConnected.h                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 3/8/95     |                                                      |
//+========================================================================+


#ifndef __OSSOCKETCONNECTED_H
#define __OSSOCKETCONNECTED_H

#include "DataTypes.h"
#include "SharedDefines.h"
#include "SystemTick.h"
#include "TaskApp.h"

//SOCKET STATUS
#define SOCKET_STATUS_OK                 		0
#define SOCKET_STATUS_ERROR               	   -1


class CSegment;
class COsSocketConnected
{

public:

	COsSocketConnected(int size=128*1024-1,int threashold=-1);
	virtual ~COsSocketConnected();
    virtual int Send(const char* buffer, int bytesToWrite);
    virtual int Receive(char * buffer,int bytesToRead);
    

	virtual void Serialize(CSegment& seg) const;
	virtual void DeSerialize(CSegment& seg);
	
	int SelectRead(TICKS timeout, int &socketHandles);
	virtual int Read(char * buffer,int len,int &sizeRead,const CTaskApp& task,BYTE partialRcv=FALSE);
	virtual int Select(int timeout);
	virtual STATUS Write(const char* buffer, int len, BOOL dropIfBlocked = FALSE);
	void Close();
	void SetDescriptor(int);
	int GetDescriptor() {return m_descriptor;}
	DWORD GetRemoteIp()const;
	void SetRemoteIp(DWORD ip);
	int IsPeerSocketDisconnected();
	
	virtual void SetTlsParams(void* ssl);
	virtual BYTE IsSecured();
    
	friend std::ostream& operator<< (std::ostream& os, const COsSocketConnected& socket);
	COsSocketConnected & operator= ( const COsSocketConnected& other);

	long long m_numberOfBytes;
	int m_numberOfMessages;
	int m_numberOfRetries;

protected:
	int m_descriptor;
	
	DWORD m_remoteIp;
private:
	// restirctions

	int m_bufferSize;
	int m_bufferThreshold;
   
};

	
std::ostream& operator<< (std::ostream& os, const COsSocketConnected& socket);


#endif /* __OSSOCKETCONNECTED_H */
