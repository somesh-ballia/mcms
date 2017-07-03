#include "MediaTxSocket.h"
#include "SocketApi.h"

CMediaTxSocket::CMediaTxSocket()
{
}

CMediaTxSocket::~CMediaTxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

void*  CMediaTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CMediaTxSocket::GetTaskName() const
{
	return "MediaTxSocket";
}

/////////////////////////////////////////////////////////////////////////////

void CMediaTxSocket::InitTask()
{
	PTRACE(eLevelInfoNormal, "CMediaTxSocket::InitTask");
}






/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CVideoTxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CVideoTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CVideoTxSocket::OnWriteSocketAnycase)   
PEND_MESSAGE_MAP(CVideoTxSocket,CMediaTxSocket);
/////////////////////////////////////////////////////////////////////////////



//  task creation function
void VideoTxEntryPoint(void* appParam)
{
	CVideoTxSocket*  pTxSocket = new CVideoTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////

CVideoTxSocket::CVideoTxSocket()
{
	PTRACE(eLevelInfoNormal, "CVideoTxSocket::CVideoTxSocket");
}

/////////////////////////////////////////////////////////////////////////////

CVideoTxSocket::~CVideoTxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

void*  CVideoTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CVideoTxSocket::GetTaskName() const
{
	return "VideoTxSocket";
}

/////////////////////////////////////////////////////////////////////////////

void CVideoTxSocket::InitTask()
{
	PTRACE(eLevelInfoNormal, "CVideoTxSocket::InitTask");
}

//////////////////////////////////////////////////////////////////////

void CVideoTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CVideoTxSocket::OnWriteSocketAnycase");
	// fill LEVEL1 header information
	/*TPKT_HEADER_S   tTpktStruct;
	tTpktStruct.version_num		= TPKT_VERSION_NUM;
	tTpktStruct.reserved		= 0;
	
	static const DWORD tpktHeaderLen = sizeof(TPKT_HEADER_S);
	WORD payloadLen = pParam->GetWrtOffset() + tpktHeaderLen;
	tTpktStruct.payload_len		= (WORD)htons(payloadLen);*/

	CSegment* pMsg = new CSegment(*pParam); // pMsg will delete inside

	// put LEVEL1 header to message
	/*pMsg->Put((BYTE*)(&tTpktStruct), tpktHeaderLen);
	*pMsg << *pParam;*/

	//Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
	const char* buff = "123456";
	int size = sizeof(buff);
	Write(buff, size);
	
	POBJDELETE(pMsg);
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CAudioTxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CAudioTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CAudioTxSocket::OnWriteSocketAnycase)   
PEND_MESSAGE_MAP(CAudioTxSocket,CMediaTxSocket);
/////////////////////////////////////////////////////////////////////////////



//  task creation function
void AudioTxEntryPoint(void* appParam)
{
	CAudioTxSocket*  pTxSocket = new CAudioTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////

CAudioTxSocket::CAudioTxSocket()
{
	PTRACE(eLevelInfoNormal, "CAudioTxSocket::CAudioTxSocket");
}

/////////////////////////////////////////////////////////////////////////////

CAudioTxSocket::~CAudioTxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

void*  CAudioTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CAudioTxSocket::GetTaskName() const
{
	return "AudioTxSocket";
}

/////////////////////////////////////////////////////////////////////////////

void CAudioTxSocket::InitTask()
{
	PTRACE(eLevelInfoNormal, "CAudioTxSocket::InitTask");
}

//////////////////////////////////////////////////////////////////////

void CAudioTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CAudioTxSocket::OnWriteSocketAnycase");
	// fill LEVEL1 header information
	/*TPKT_HEADER_S   tTpktStruct;
	tTpktStruct.version_num		= TPKT_VERSION_NUM;
	tTpktStruct.reserved		= 0;
	
	static const DWORD tpktHeaderLen = sizeof(TPKT_HEADER_S);
	WORD payloadLen = pParam->GetWrtOffset() + tpktHeaderLen;
	tTpktStruct.payload_len		= (WORD)htons(payloadLen);*/

	CSegment* pMsg = new CSegment(*pParam); // pMsg will delete inside

	// put LEVEL1 header to message
	/*pMsg->Put((BYTE*)(&tTpktStruct), tpktHeaderLen);
	*pMsg << *pParam;*/

	//Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
	const char* buff = "123456";
	int size = sizeof(buff);
	Write(buff, size);
	
	POBJDELETE(pMsg);
}




////////////////////////////////////////CMediaTxTask/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CContentTxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CContentTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CContentTxSocket::OnWriteSocketAnycase)   
PEND_MESSAGE_MAP(CContentTxSocket,CMediaTxSocket);
/////////////////////////////////////////////////////////////////////////////



//  task creation function
void ContentTxEntryPoint(void* appParam)
{
	CContentTxSocket*  pTxSocket = new CContentTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////

CContentTxSocket::CContentTxSocket()
{
	PTRACE(eLevelInfoNormal, "CContentTxSocket::CContentTxSocket");
}

/////////////////////////////////////////////////////////////////////////////

CContentTxSocket::~CContentTxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

void*  CContentTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CContentTxSocket::GetTaskName() const
{
	return "ContentTxSocket";
}

/////////////////////////////////////////////////////////////////////////////

void CContentTxSocket::InitTask()
{
	PTRACE(eLevelInfoNormal, "CContentTxSocket::InitTask");
}

//////////////////////////////////////////////////////////////////////

void CContentTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CContentTxSocket::OnWriteSocketAnycase");
	// fill LEVEL1 header information
	/*TPKT_HEADER_S   tTpktStruct;
	tTpktStruct.version_num		= TPKT_VERSION_NUM;
	tTpktStruct.reserved		= 0;
	
	static const DWORD tpktHeaderLen = sizeof(TPKT_HEADER_S);
	WORD payloadLen = pParam->GetWrtOffset() + tpktHeaderLen;
	tTpktStruct.payload_len		= (WORD)htons(payloadLen);*/

	CSegment* pMsg = new CSegment(*pParam); // pMsg will delete inside

	// put LEVEL1 header to message
	/*pMsg->Put((BYTE*)(&tTpktStruct), tpktHeaderLen);
	*pMsg << *pParam;*/

	//Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
	const char* buff = "123456";
	int size = sizeof(buff);
	Write(buff, size);
	
	POBJDELETE(pMsg);
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CFeccTxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// message map
PBEGIN_MESSAGE_MAP(CFeccTxSocket)
   ONEVENT( SOCKET_WRITE, ANYCASE,  CFeccTxSocket::OnWriteSocketAnycase)   
PEND_MESSAGE_MAP(CFeccTxSocket,CMediaTxSocket);
/////////////////////////////////////////////////////////////////////////////



//  task creation function
void FeccTxEntryPoint(void* appParam)
{
	CFeccTxSocket*  pTxSocket = new CFeccTxSocket;
	pTxSocket->Create(*(CSegment*)appParam);
}

/////////////////////////////////////////////////////////////////////////////

CFeccTxSocket::CFeccTxSocket()
{
	PTRACE(eLevelInfoNormal, "CFeccTxSocket::CFeccTxSocket");
}

/////////////////////////////////////////////////////////////////////////////

CFeccTxSocket::~CFeccTxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

void*  CFeccTxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CFeccTxSocket::GetTaskName() const
{
	return "FeccTxSocket";
}

/////////////////////////////////////////////////////////////////////////////

void CFeccTxSocket::InitTask()
{
	PTRACE(eLevelInfoNormal, "CFeccTxSocket::InitTask");
}

//////////////////////////////////////////////////////////////////////

void CFeccTxSocket::OnWriteSocketAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CFeccTxSocket::OnWriteSocketAnycase");
	// fill LEVEL1 header information
	/*TPKT_HEADER_S   tTpktStruct;
	tTpktStruct.version_num		= TPKT_VERSION_NUM;
	tTpktStruct.reserved		= 0;
	
	static const DWORD tpktHeaderLen = sizeof(TPKT_HEADER_S);
	WORD payloadLen = pParam->GetWrtOffset() + tpktHeaderLen;
	tTpktStruct.payload_len		= (WORD)htons(payloadLen);*/

	CSegment* pMsg = new CSegment(*pParam); // pMsg will delete inside

	// put LEVEL1 header to message
	/*pMsg->Put((BYTE*)(&tTpktStruct), tpktHeaderLen);
	*pMsg << *pParam;*/

	//Write((char*)(pMsg->GetPtr()),pMsg->GetWrtOffset());
	const char* buff = "123456";
	int size = sizeof(buff);
	Write(buff, size);
	
	POBJDELETE(pMsg);
}

