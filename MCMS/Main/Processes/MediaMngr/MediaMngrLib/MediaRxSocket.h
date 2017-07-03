#ifndef MEDIARXSOCKET_H_
#define MEDIARXSOCKET_H_

#include "TaskApp.h"
#include "UdpSocket.h"
#include "MediaMngr.h"

class CMediaRxSocket : public CTaskApp
{
CLASS_TYPE_1(CMediaRxSocket,CTaskApp)
public:
	CMediaRxSocket();
	virtual ~CMediaRxSocket();
	
	// Initializations
	void* GetMessageMap();
	void  Create(CSegment& appParam);
	
	// Operations
	const char* NameOf() const {return "CMediaRxSocket";}
	const char * GetTaskName() const;
	void  SelfKill();
	
	
	//Action Functions
	void OnMediaRxInitUdpSocketAll(CSegment* pSeg);
	void OnMediaRxSetParamUdpSocketAll(CSegment* pSeg);
	void OnMediaRxRecvFromUdpSocketAll(CSegment* pSeg);
	void OnMediaRxCloseUdpSocketAll(CSegment* pSeg);
	
	//Recv timer
	void OnTimerRecvMedia(CSegment* pParam);
	
	//Recording
	void OnMediaRxStartWriteUdpSocketAll(CSegment* pSeg);
	void OnMediaRxStopWriteUdpSocketAll(CSegment* pSeg);

	
protected:
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();
	
	
	int WriteBuffer();
	int FileOpen();
	int FileClose();
	int FileSize();
	
	
	CUdpSocket*  m_pUdpRxSocket;
	
	BYTE m_recvBuffer[MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM];
	struct sockaddr_in m_from_addr;
	
	
	
	FILE* m_pFile;
	string m_sFullFileName;
	BYTE* m_mediaBuffer;
	int   m_mediaIndex;
	BOOL  m_bWriteFlag;
	
	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////
// Video Rx Task Entry Point
////////////////////////////////////////////////
extern "C" void VideoRxEntryPoint(void* appParam);

class CVideoRxSocket : public CMediaRxSocket 
{
CLASS_TYPE_1(CVideoRxSocket,CMediaRxSocket)
public:
	CVideoRxSocket();
	virtual ~CVideoRxSocket();
	
	// Initializations
	void* GetMessageMap();
	void  Create(CSegment& appParam);

	// Operations
	const char* NameOf() const {return "CVideoRxSocket";}
	const char * GetTaskName() const;
	
protected:
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();
	
	
	PDECLAR_MESSAGE_MAP
};




////////////////////////////////////////////////
// Audio Rx Task Entry Point
////////////////////////////////////////////////
extern "C" void AudioRxEntryPoint(void* appParam);


class CAudioRxSocket : public CMediaRxSocket 
{
CLASS_TYPE_1(CAudioRxSocket,CMediaRxSocket)
public:
	CAudioRxSocket();
	virtual ~CAudioRxSocket();
	
	// Initializations
	void* GetMessageMap();
	void  Create(CSegment& appParam);

	// Operations
	const char* NameOf() const {return "CAudioRxSocket";}
	const char * GetTaskName() const;
	
protected:
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();
	
	
	PDECLAR_MESSAGE_MAP
};




////////////////////////////////////////////////
// Content Rx Task Entry Point
////////////////////////////////////////////////
extern "C" void ContentRxEntryPoint(void* appParam);


class CContentRxSocket : public CMediaRxSocket 
{
CLASS_TYPE_1(CContentRxSocket,CMediaRxSocket)
public:
	CContentRxSocket();
	virtual ~CContentRxSocket();
	
	// Initializations
	void* GetMessageMap();
	void  Create(CSegment& appParam);

	// Operations
	const char* NameOf() const {return "CContentRxSocket";}
	const char * GetTaskName() const;
	
protected:
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();
	
	
	PDECLAR_MESSAGE_MAP
};



////////////////////////////////////////////////
// Fecc Rx Task Entry Point
////////////////////////////////////////////////
extern "C" void FeccRxEntryPoint(void* appParam);


class CFeccRxSocket : public CMediaRxSocket 
{
CLASS_TYPE_1(CFeccRxSocket,CMediaRxSocket)
public:
	CFeccRxSocket();
	virtual ~CFeccRxSocket();
	
	// Initializations
	void* GetMessageMap();
	void  Create(CSegment& appParam);

	// Operations
	const char* NameOf() const {return "CFeccRxSocket";}
	const char * GetTaskName() const;
	
protected:
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();
	
	
	PDECLAR_MESSAGE_MAP
};

#endif /*MEDIARXSOCKET_H_*/
