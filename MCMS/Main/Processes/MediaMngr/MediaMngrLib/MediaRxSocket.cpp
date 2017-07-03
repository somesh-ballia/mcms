#include "MediaRxSocket.h"
#include "TraceStream.h"
#include "Segment.h"
#include "OsFileIF.h"
#include "MediaMngrCfg.h"
#include <cerrno>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CMediaRxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CMediaRxSocket)

	//socket operations
	ONEVENT( INIT_UDP_SOCKET_MSG,	ANYCASE,   CMediaRxSocket::OnMediaRxInitUdpSocketAll)
	ONEVENT( SET_PARAM_UDP_SOCKET_MSG,	ANYCASE,   CMediaRxSocket::OnMediaRxSetParamUdpSocketAll)
	ONEVENT( RECV_DATA_FROM_UDP_SOCKET_MSG,	ANYCASE,   CMediaRxSocket::OnMediaRxRecvFromUdpSocketAll)
	ONEVENT( CLOSE_UDP_SOCKET_MSG,	ANYCASE,   CMediaRxSocket::OnMediaRxCloseUdpSocketAll)
	
	//recv timer
	ONEVENT( TIMER_RECV_MEDIA,	ANYCASE,   	CMediaRxSocket::OnTimerRecvMedia)
	
	//recording
	ONEVENT( START_WRITE_STEAM_UDP_SOCKET_MSG,	ANYCASE,   CMediaRxSocket::OnMediaRxStartWriteUdpSocketAll)
	ONEVENT( STOP_WRITE_STEAM_UDP_SOCKET_MSG,	ANYCASE,   CMediaRxSocket::OnMediaRxStopWriteUdpSocketAll)
PEND_MESSAGE_MAP(CMediaRxSocket,CTaskApp);


CMediaRxSocket::CMediaRxSocket()
{
	//TRACEINTO << "CMediaRxSocket::CMediaRxSocket()";
	
	m_pUdpRxSocket = NULL;
	
	m_pFile = NULL;
	m_sFullFileName = "";
	m_mediaBuffer = NULL;
	m_mediaIndex = 0;
	m_bWriteFlag = FALSE;
}


CMediaRxSocket::~CMediaRxSocket()
{
	TRACEINTO << "CMediaRxSocket::~CMediaRxSocket()";
	
	POBJDELETE(m_pUdpRxSocket);
	
	
	if (m_bWriteFlag)
	{
		m_bWriteFlag = FALSE;
		WriteBuffer();
		FileClose();
	}	
	
	POBJDELETE(m_mediaBuffer);
}

/////////////////////////////////////////////////////////////////////////////

void  CMediaRxSocket::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);
}

/////////////////////////////////////////////////////////////////////////////

void*  CMediaRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CMediaRxSocket::GetTaskName() const
{
	return "MediaRxSocket";
}

/////////////////////////////////////////////////////////////////////////////

void CMediaRxSocket::SelfKill()
{
	DispatchEvent(CLOSE_UDP_SOCKET_MSG, NULL);
	
    DeleteAllTimers();
    
	CTaskApp::SelfKill();
}

/////////////////////////////////////////////////////////////////////////////

void CMediaRxSocket::InitTask()
{
	TRACEINTO << "CMediaRxSocket::InitTask pid: " << getpid() <<  " taskid:" << GetTaskId();
	
	DispatchEvent(INIT_UDP_SOCKET_MSG, NULL);
}

//////////////////////////////////////////////////////////////////////

void CMediaRxSocket::OnMediaRxInitUdpSocketAll(CSegment* pSeg)
{
	TRACEINTO << "CMediaRxSocket::OnMediaRxInitUdpSocketAll "<< GetTaskName();
	
	m_pUdpRxSocket = new CUdpSocket;
		
	if (m_pUdpRxSocket != NULL)
	{
		TRACEINTO << "CMediaRxSocket::OnMediaRxInitUdpSocketAll m_pUdpRxSocket created " << GetTaskName();
	}
	else
	{
		TRACEINTO << "MM ERROR CMediaRxSocket::OnMediaRxInitUdpSocketAll m_pUdpRxSocket is NULL " << GetTaskName();
		return;
	}
	
	/*int fDescriptor = m_pUdpRxSocket->CreateUdpSocket();
	
	if (fDescriptor == STATUS_ERROR)
	{
		TRACEINTO << "MM ERROR CMediaRxSocket::OnMediaRxInitUdpSocketAll ERROR " << GetTaskName();
		return;
	}*/
	
}

//////////////////////////////////////////////////////////////////////

void CMediaRxSocket::OnMediaRxSetParamUdpSocketAll(CSegment* pSeg)
{
	TRACEINTO << "CMediaRxSocket::OnMediaRxSetParamUdpSocketAll " << GetTaskName();
	
	DWORD family = AF_INET;
	DWORD port = 0xFFFFFFFF;
	DWORD ipaddress = 0xFFFFFFFF;
	
	*pSeg	>> family
			>> port
			>> ipaddress;
	
	int retVal = STATUS_ERROR;
	if (m_pUdpRxSocket != NULL)
	{
		delete m_pUdpRxSocket;
	}

	m_pUdpRxSocket=new CUdpSocket(ipaddress, port);
}

//////////////////////////////////////////////////////////////////////

void CMediaRxSocket::OnMediaRxRecvFromUdpSocketAll(CSegment* pSeg)
{
	TRACEINTO << "CMediaRxSocket::OnMediaRxRecvFromUdpSocketAll " << GetTaskName();
	
	StartTimer(TIMER_RECV_MEDIA, 2);	//(2 = 20msec)
}

/////////////////////////////////////////////////////////////////////////////


void CMediaRxSocket::OnMediaRxCloseUdpSocketAll(CSegment* pSeg)
{
	TRACEINTO << "CMediaRxSocket::OnMediaRxCloseUdpSocketAll " << GetTaskName();
	
	DeleteTimer(TIMER_RECV_MEDIA);
		
	if (m_pUdpRxSocket != NULL)
	{
		if (m_bWriteFlag)
		{
			m_bWriteFlag = FALSE;
			WriteBuffer();
			FileClose();
		}
		
		
		/*int retVal = STATUS_ERROR;
		
		retVal = m_pUdpRxSocket->CloseUdpSocket();
		
		if (retVal == STATUS_ERROR)
		{
			TRACEINTO << "MM ERROR CMediaRxSocket::OnMediaRxCloseUdpSocketAll. CloseUdpSocket Failed." << GetTaskName();
			return;
		}*/
		
		POBJDELETE(m_pUdpRxSocket);
		
		TRACEINTO << "CMediaRxSocket::OnMediaRxCloseUdpSocketAll - done " << GetTaskName();
	}
	else
	{
		TRACEINTO << "MM ERROR CMediaRxSocket::OnMediaRxCloseUdpSocketAll - m_pUdpRxSocket is NULL " << GetTaskName();
	}
}

/////////////////////////////////////////////////////////////////////////////

void CMediaRxSocket::OnTimerRecvMedia(CSegment* pParam)
{
	//TRACEINTO << "CMediaRxSocket::OnTimerRecvMedia";
	
	if (m_pUdpRxSocket == NULL)
	{
		TRACEINTO << "MM ERROR CMediaRxSocket::OnTimerRecvMedia - m_pUdpRxSocket is NULL " << GetTaskName();
		DeleteTimer(TIMER_RECV_MEDIA);
		return;
	}
	
	int selectResult = m_pUdpRxSocket->Select(0);
	
	//TRACEINTO << "CMediaRxSocket::OnTimerRecvMedia selectResult=" << selectResult;
		
	if (selectResult > 0)
	{// go bring data
		
		while (m_pUdpRxSocket)
		{
			//zero buffer & address struct - no need!
			//memset((void *)&m_from_addr, 0, sizeof(m_from_addr));
			//memset((BYTE *)m_recvBuffer, 0, sizeof(m_recvBuffer));
			
			int sizeRecv = m_pUdpRxSocket->RecvFrom((char*)m_recvBuffer, MAX_NUM_OF_BYTES_IN_UDP_DATAGRAM, (struct sockaddr*)&m_from_addr);
			if (sizeRecv == STATUS_ERROR)
			{
				TRACEINTO << "MM ERROR CMediaRxSocket::OnTimerRecvMedia - RecvFrom: STATUS_ERROR " << GetTaskName();
				//DeleteTimer(TIMER_RECV_MEDIA);
			}
			else
			{
				//TRACEINTO << "CMediaRxSocket::OnTimerRecvMedia RecvFrom buffer arrived - do something with buffer..."
				
				if (m_bWriteFlag)
				{
					//if buffer will be over written -> write buff to file
					if ( m_mediaIndex+sizeRecv > MEDIA_WRITE_BUFFER_SIZE)
					{
						WriteBuffer();
						
						//init buffer index
						m_mediaIndex = 0;
					}
					
					//copy bytes received from socket to write cache buffer
					
					/* code for entering a NULL header + remove the rtp header
					 * memset(&m_mediaBuffer[m_mediaIndex], 0, 1);
					memset(&m_mediaBuffer[m_mediaIndex+1], 0, 1);
					memset(&m_mediaBuffer[m_mediaIndex+2], 0, 1);
					memset(&m_mediaBuffer[m_mediaIndex+3], 1, 1);
					memcpy(&m_mediaBuffer[m_mediaIndex+4], &m_recvBuffer[12], sizeRecv);*/
					
					memcpy(&m_mediaBuffer[m_mediaIndex], m_recvBuffer, sizeRecv);
					m_mediaIndex += sizeRecv;
				}
			}
			
			selectResult = m_pUdpRxSocket->Select(0);
			//TRACEINTO << "CMediaRxSocket::OnTimerRecvMedia selectResult=" << selectResult;
			if (selectResult <= 0)
			{
				StartTimer(TIMER_RECV_MEDIA, 2);
				return;
			}

		}//while
		
		TRACEINTO << "MM ERROR CMediaRxSocket::OnTimerRecvMedia  RecvFrom exit recv loop - m_pUdpRxSocket is NULL " << GetTaskName();
		DeleteTimer(TIMER_RECV_MEDIA);
		return;
	}
	else
	{//set timer one more time
		StartTimer(TIMER_RECV_MEDIA, 2);
		return;
	}
}




/////////////////////////////////////////////////////////////////////////////


void CMediaRxSocket::OnMediaRxStartWriteUdpSocketAll(CSegment* pSeg)
{
	TRACEINTO << "CMediaRxSocket::OnMediaRxStartWriteUdpSocketAll " << GetTaskName();
	
	if (m_pUdpRxSocket == NULL)
	{
		TRACEINTO << "MM ERROR CMediaRxSocket::OnMediaRxStartWriteUdpSocketAll - m_pUdpRxSocket is NULL " << GetTaskName();
		return;
	}
			
	
	m_sFullFileName  = ::GetMediaMngrCfg()->GetMediaFileWritePath();
	
	string fileName = "";	
	*pSeg	>> fileName;
		
	if (fileName == "")
	{
		TRACEINTO << "MM ERROR CMediaRxSocket::OnMediaRxStartWriteUdpSocketAll - fileName is empty. " << GetTaskName();
		return;
	}
	
	m_sFullFileName += "/";
	m_sFullFileName += fileName;
	
	int openFileStatus = FileOpen();
	if (openFileStatus == STATUS_OK)
	{
		//init buffer in memory for writing upon buffer is full
		m_mediaBuffer = new BYTE[MEDIA_WRITE_BUFFER_SIZE];
		
		m_bWriteFlag = TRUE;
	}
}



/////////////////////////////////////////////////////////////////////////////


void CMediaRxSocket::OnMediaRxStopWriteUdpSocketAll(CSegment* pSeg)
{
	TRACEINTO << "CMediaRxSocket::OnMediaRxStopWriteUdpSocketAll " << GetTaskName();
	
	if (m_bWriteFlag)
	{
		m_bWriteFlag = FALSE;
		WriteBuffer();
		FileClose();
	}
}

/////////////////////////////////////////////////////////////////////////////

int CMediaRxSocket::WriteBuffer()
{
	TRACEINTO << "CMediaRxSocket::WriteBuffer " << GetTaskName();
	
	size_t numWrite = fwrite(m_mediaBuffer, sizeof(BYTE), m_mediaIndex, m_pFile);
	
	TRACEINTO << "CMediaRxSocket::WriteBuffer num bytes writen = " << numWrite;
	
	if (numWrite == 0)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CMediaRxSocket::WriteBuffer fwrite() errno=" << myerrno << " description: " << myerrnostr.c_str();
		return STATUS_ERROR;
	}
	
	//flush the buffer to disk
	int flushStatus = fflush(m_pFile);
	if (flushStatus != STATUS_OK)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CMediaRxSocket::WriteBuffer fflush() errno=" << myerrno << " description: " << myerrnostr.c_str() << " flushStatus=" << flushStatus;
		return STATUS_ERROR;
	}
	
	//print current file size
	FileSize();
	
	return numWrite;
}

/////////////////////////////////////////////////////////////////////////////

int CMediaRxSocket::FileOpen()
{
	TRACEINTO << "CMediaRxSocket::FileOpen " << GetTaskName();
	
	// check if file exists
	if (IsFileExists( m_sFullFileName.c_str()))
	{
		TRACEINTO << "MM ERROR CMediaRxSocket::FileOpen - File allready exists: " <<  m_sFullFileName.c_str();
		return STATUS_ERROR;
	}
	
	// open file for writing binary
	m_pFile = fopen( m_sFullFileName.c_str(), "wb" );
	
	if (m_pFile == NULL)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CMediaRxSocket::FileOpen - File Open Error: " <<  m_sFullFileName.c_str() << " errno=" << myerrno << " description: " << myerrnostr.c_str();
		return STATUS_ERROR;
	}
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

int CMediaRxSocket::FileClose()
{
	TRACEINTO << "CMediaRxSocket::FileClose " << GetTaskName();
	
	//close the fd until flush operation will come
	int closeStatus = fclose(m_pFile);
	if (closeStatus != STATUS_OK)
	{
		int myerrno = errno;
		string myerrnostr = strerror(myerrno);
		TRACEINTO << "MM ERROR CMediaRxSocket::FileClose - fclose  errno=" << myerrno << " description: " << myerrnostr.c_str();
		return STATUS_ERROR;
	}
	
	m_pFile = NULL;
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////

int CMediaRxSocket::FileSize()
{
	int fileSize = GetFileSize(m_sFullFileName);
	
	TRACEINTO << "CMediaRxSocket::FileSize = " << fileSize << " " << GetTaskName();
		
	return fileSize;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CVideoRxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CVideoRxSocket)
   
PEND_MESSAGE_MAP(CVideoRxSocket, CMediaRxSocket);

/////////////////////////////////////////////////////////////////////////////
//  task creation function
void VideoRxEntryPoint(void* appParam)
{
	CVideoRxSocket*  pRxSocket = new CVideoRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}



/////////////////////////////////////////////////////////////////////////////

CVideoRxSocket::CVideoRxSocket()
{
	//TRACEINTO << "CVideoRxSocket::CVideoRxSocket()";
}

/////////////////////////////////////////////////////////////////////////////

CVideoRxSocket::~CVideoRxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

void CVideoRxSocket::InitTask()
{
	TRACEINTO << "CVideoRxSocket::InitTask()";
			
	CMediaRxSocket::InitTask();
}

/////////////////////////////////////////////////////////////////////////////

void*  CVideoRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CVideoRxSocket::GetTaskName() const
{
	return "VideoRxSocket";
}

//////////////////////////////////////////////////////////////////////

void  CVideoRxSocket::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);
}

//////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CAudioRxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CAudioRxSocket)
   
PEND_MESSAGE_MAP(CAudioRxSocket,CMediaRxSocket);

/////////////////////////////////////////////////////////////////////////////
//  task creation function
void AudioRxEntryPoint(void* appParam)
{
	CAudioRxSocket*  pRxSocket = new CAudioRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}


/////////////////////////////////////////////////////////////////////////////

CAudioRxSocket::CAudioRxSocket()
{
	//TRACEINTO << "CAudioRxSocket::CAudioRxSocket()";
}

/////////////////////////////////////////////////////////////////////////////

CAudioRxSocket::~CAudioRxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

const char* CAudioRxSocket::GetTaskName() const
{
	return "AudioRxSocket";
}

//////////////////////////////////////////////////////////////////////

void CAudioRxSocket::InitTask()
{
	TRACEINTO << "CAudioRxSocket::InitTask()";
	
	CMediaRxSocket::InitTask();
}

//////////////////////////////////////////////////////////////////////

void* CAudioRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//////////////////////////////////////////////////////////////////////

void  CAudioRxSocket::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);
}

//////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CContentRxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



PBEGIN_MESSAGE_MAP(CContentRxSocket)

PEND_MESSAGE_MAP(CContentRxSocket,CMediaRxSocket);

/////////////////////////////////////////////////////////////////////////////
//  task creation function
void ContentRxEntryPoint(void* appParam)
{
	CContentRxSocket*  pRxSocket = new CContentRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}



/////////////////////////////////////////////////////////////////////////////

CContentRxSocket::CContentRxSocket()
{
	//TRACEINTO << "CContentRxSocket::CContentRxSocket()";
}

/////////////////////////////////////////////////////////////////////////////

CContentRxSocket::~CContentRxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

void CContentRxSocket::InitTask()
{
	TRACEINTO << "CContentRxSocket::InitTask()";
	
	CMediaRxSocket::InitTask();
}

/////////////////////////////////////////////////////////////////////////////

void*  CContentRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CContentRxSocket::GetTaskName() const
{
	return "ContentRxSocket";
}

//////////////////////////////////////////////////////////////////////

void  CContentRxSocket::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);
}


//////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//   CFeccRxSocket
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


PBEGIN_MESSAGE_MAP(CFeccRxSocket)
   
PEND_MESSAGE_MAP(CFeccRxSocket,CMediaRxSocket);

/////////////////////////////////////////////////////////////////////////////
//  task creation function
void FeccRxEntryPoint(void* appParam)
{
	CFeccRxSocket*  pRxSocket = new CFeccRxSocket;
	pRxSocket->Create(*(CSegment*)appParam);
}


/////////////////////////////////////////////////////////////////////////////

CFeccRxSocket::CFeccRxSocket()
{
	//TRACEINTO << "CFeccRxSocket::CFeccRxSocket()";
}

/////////////////////////////////////////////////////////

CFeccRxSocket::~CFeccRxSocket()
{
}

/////////////////////////////////////////////////////////////////////////////

void CFeccRxSocket::InitTask()
{
	TRACEINTO << "CFeccRxSocket::InitTask()";

	CMediaRxSocket::InitTask();
}

/////////////////////////////////////////////////////////////////////////////

void*  CFeccRxSocket::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

const char* CFeccRxSocket::GetTaskName() const
{
	return "FeccRxSocket";
}

//////////////////////////////////////////////////////////////////////

void  CFeccRxSocket::Create(CSegment& appParam)
{
	CTaskApp::Create(appParam);
}
