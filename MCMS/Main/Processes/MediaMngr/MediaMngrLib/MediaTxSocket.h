#ifndef MEDIATXSOCKET_H_
#define MEDIATXSOCKET_H_

#include "SocketTxTask.h"

class CMediaTxSocket : public CSocketTxTask
{
CLASS_TYPE_1(CMediaTxSocket,CSocketTxTask )
public:
	CMediaTxSocket();
	virtual ~CMediaTxSocket();
	
	void* GetMessageMap();
		
	const char* NameOf() const {return "CMediaTxSocket";}
	const char * GetTaskName() const;
	
protected:
	// base overriding
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();

	// Operations

	// Action functions
	void OnWriteSocketAnycase(CSegment* pParam);


	// Attributes
	
	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////
// Video Tx Task Entry Point
////////////////////////////////////////////////
extern "C" void VideoTxEntryPoint(void* appParam);

class CVideoTxSocket : public CMediaTxSocket
{
CLASS_TYPE_1(CVideoTxSocket,CMediaTxSocket)
public:
	CVideoTxSocket();
	virtual ~CVideoTxSocket();
	
	void* GetMessageMap();
	
	const char* NameOf() const {return "CVideoTxSocket";}
	const char * GetTaskName() const;
	
protected:
				// base overriding
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();

				// Operations

				// Action functions
	void OnWriteSocketAnycase(CSegment* pParam);


				// Attributes

	PDECLAR_MESSAGE_MAP
};

////////////////////////////////////////////////
// Audio Tx Task Entry Point
////////////////////////////////////////////////
extern "C" void AudioTxEntryPoint(void* appParam);

class CAudioTxSocket : public CMediaTxSocket
{
CLASS_TYPE_1(CAudioTxSocket,CMediaTxSocket)
public:
	CAudioTxSocket();
	virtual ~CAudioTxSocket();
	
	void* GetMessageMap();
	
	const char* NameOf() const {return "CAudioTxSocket";}
	const char * GetTaskName() const;
	
protected:
				// base overriding
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();

				// Operations

				// Action functions
	void OnWriteSocketAnycase(CSegment* pParam);


				// Attributes

	PDECLAR_MESSAGE_MAP
};




////////////////////////////////////////////////
// Content Tx Task Entry Point
////////////////////////////////////////////////

extern "C" void ContentTxEntryPoint(void* appParam);

class CContentTxSocket : public CMediaTxSocket
{
CLASS_TYPE_1(CContentTxSocket,CMediaTxSocket)
public:
	CContentTxSocket();
	virtual ~CContentTxSocket();
	
	void* GetMessageMap();
	
	const char* NameOf() const {return "CContentTxSocket";}
	const char * GetTaskName() const;
	
protected:
				// base overriding
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();

				// Operations

				// Action functions
	void OnWriteSocketAnycase(CSegment* pParam);


				// Attributes

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////
// Fecc Tx Task Entry Point
////////////////////////////////////////////////
extern "C" void FeccTxEntryPoint(void* appParam);

class CFeccTxSocket : public CMediaTxSocket
{
CLASS_TYPE_1(CFeccTxSocket,CMediaTxSocket)
public:
	CFeccTxSocket();
	virtual ~CFeccTxSocket();
	
	void* GetMessageMap();
	
	const char* NameOf() const {return "CFeccTxSocket";}
	const char * GetTaskName() const;
	
protected:
				// base overriding
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();

				// Operations

				// Action functions
	void OnWriteSocketAnycase(CSegment* pParam);


				// Attributes

	PDECLAR_MESSAGE_MAP
};

#endif /*MEDIATXSOCKET_H_*/
