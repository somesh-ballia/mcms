// LoggerRxSocket.cpp

#include "LoggerRxSocket.h"

#include <map>
#include <netinet/in.h>

#include "Segment.h"
#include "ProcessBase.h"
#include "OpcodesMcmsCommon.h"
#include "StatusesGeneral.h"
#include "LoggerDefines.h"
#include "OpcodesLogger.h"
#include "ManagerApi.h"
#include "LoggerProcess.h"
#include "WrappersCommon.h"
#include "TraceStream.h"
#include "ManagerApi.h"
#include "LoggerStatuses.h"
#include "TraceClass.h"
#include "MplMcmsProtocol.h"
#include "ObjString.h"
#include "LoggerSocketStatus.h"

static CLoggerProcess *pProcess = NULL;

extern const char* MainEntityToString(APIU32 entityType);

extern "C" void LoggerSocketRxEntryPoint(void* appParam)
{  	
	CLoggerRxSocket *pTaskApp = new CLoggerRxSocket;
	pTaskApp->Create(*(CSegment*)appParam);
	*(CSegment*)appParam << (void*)pTaskApp ;
}

static void TraceOutTcpReceiveBuffer(const BYTE * buffer, DWORD len)
{   
    DWORD *pBuffer = (DWORD*)buffer;
    
    CLargeString strBuffer;
    DWORD numOfIter = len / 4;
    for(DWORD i = 0 ; i < numOfIter ; i++)
    {
        strBuffer << pBuffer[i] << ".";
    }
    FPTRACE(eLevelInfoNormal, strBuffer.GetString());
}

CLoggerRxSocket::CLoggerRxSocket(void) :
    m_IsNameFixed(false),
    m_proc(static_cast<CLoggerProcess*>(CProcessBase::GetProcess()))
{
    PASSERT(NULL == m_proc);
}

void CLoggerRxSocket::InitTask(void)
{
	m_TaskName = "LoggerRxSocket-";
	TRACEINTO_SOCKET << ".";
}

const char* CLoggerRxSocket::GetTaskName(void) const
{
	return m_TaskName.c_str();
}

//////////////////////////////////////////////////////////////////////
void CLoggerRxSocket::ReceiveFromSocket(void)
{
	char buffTPKTHdr[sizeof(TPKT_HEADER_S)];
	int sizeRead = 0;
    
	STATUS status = Read(buffTPKTHdr, sizeof(TPKT_HEADER_S), sizeRead);	
	if (STATUS_OK != status)
		return;

	DWORD messageLen = 0;
	bool res = ReadValidate_TPKT_Header(buffTPKTHdr, messageLen);
	if((false == res) || (messageLen > TRACE_BUFFER_LEN+1))
	{
		if (messageLen > TRACE_BUFFER_LEN+1)
		{
			TRACEINTO_SOCKET << "Invalid messageLen  : " << messageLen;
		}

		OnCorruptedTPKT(buffTPKTHdr);
		return;
	}
    
    sizeRead = 0;

	status = Read((char*)m_TraceBuffer, messageLen, sizeRead);
	if (sizeRead != (int )messageLen )
	{
		TRACEINTO_SOCKET << "Lost synchronization   : "
		              << "\nsizeRead " << sizeRead
		              <<"\nmessageLen " << messageLen;
		OnCorruptedTPKT(buffTPKTHdr);
		return;
	}

    eSocketStatus socketStatus = m_proc->GetSocketStatus();
    switch (socketStatus)
    {
        case eSocketStatusDrop:
            // Drops this message
            return;
            
        case eSocketStatusDead:
            HandleDisconnect();
            break;
            
        case eSocketStatusNormal:
            // Continues as usual
            break;
            
        default:
            PASSERTSTREAM_AND_RETURN(true, "Bad socket status " << socketStatus << ", FD:" << GetFD());
    }

    // Skips the message on high system load average
    if (m_proc->GetDropMessageFlagAndIncrease(CLoggerProcess::eSenderOut))
        return;

    CMplMcmsProtocol prot;
    res = prot.DeSerialize(m_TraceBuffer, messageLen);
    if (false == res)
    {
        PrintCorruptedTraceOncePerStatus(prot, STATUS_TRACE_DESERIALIZE_FAIL, "Deserialize Failed");
        return;
    }
    
    status = ValidateHeaders(prot);
    if (STATUS_OK != status)
    {
        PrintCorruptedTraceOncePerStatus(prot, status, "Validation Failed");
        return;
    }
    
	if (false == m_IsNameFixed)
	{
		FixTaskName(prot);
		m_IsNameFixed = true;
	}
	
	// VNGR-12054 - the socket will be closed after 60 seconds
	if (m_IsNameFixed == true && strstr(m_TaskName.c_str(),"CardManager") != NULL)
	{
		StartTimer(OPEN_SOCKET_TIMER, 60 * SECOND);
	}
		
	CSegment *pSeg = new CSegment;
    prot.SerializeLogger(*pSeg);
    if(m_proc->m_enableLocalTracer)
    {
    	std::ostringstream buf;
    	CTrace::BuildOutMessage(buf,prot);
    	CTrace::TraceOutMessage(buf,prot);
    }
	CManagerApi api(eProcessLogger);
	
	UnlockRelevantSemaphore();
	status = api.SendMsg(pSeg, OUT_TRACE_MESSAGE);
	LockRelevantSemaphore();

	if(STATUS_OK != status)
	{
	    m_proc->IncrementCntNotSentTrace();
	}
}

void CLoggerRxSocket::PrintTpktHeader(const TPKT_HEADER_S &tpkt)
{
    const int messageLen = ntohs(tpkt.payload_len) - sizeof(TPKT_HEADER_S);
    const int dataLen = messageLen - ( sizeof(COMMON_HEADER_S) + sizeof(TRACE_HEADER_S) + sizeof(PHYSICAL_INFO_HEADER_S));

    TRACEINTO_SOCKET << CTPKTHeaderWrapper(tpkt);
    TRACEINTO_SOCKET << "\nCOMMON_HEADER size   : " << sizeof(COMMON_HEADER_S)
              << "\nTRACE_HEADER size    : " << sizeof(TRACE_HEADER_S)
              << "\nPHYSICAL_HEADER size : " << sizeof(PHYSICAL_INFO_HEADER_S)
              << "\nData size(TPKT.len - (common_h + trace_h + physical_h) : " << dataLen
              << "\nSum : " << sizeof(COMMON_HEADER_S) + sizeof(TRACE_HEADER_S) + sizeof(PHYSICAL_INFO_HEADER_S) + dataLen;
}

//////////////////////////////////////////////////////////////////////
void CLoggerRxSocket::PrintCorruptedTraceOncePerStatus(CMplMcmsProtocol &prot, STATUS errorStatus, const char *errorMsg)
{
    // only 1 assert per error status
    const MplMcmsProtocolErrno_S & protErrno = prot.Errno();
    DWORD mergedErrorStatus = errorStatus | protErrno.m_Status;
        
    static map<STATUS, bool> AssertMemory;
    map<STATUS, bool>::iterator found = AssertMemory.find(mergedErrorStatus);
    if(AssertMemory.end() != found)
    {
        return;
    }
    else
    {
        AssertMemory[mergedErrorStatus] = true;
    }   

    PrintCorruptedTrace(prot, errorStatus);
    
    CLargeString str = "Corrupted message : ";

    str << errorMsg;
    TRACEINTO_SOCKET << str.GetString();
    //PASSERTMSG(TRUE, );
}

//////////////////////////////////////////////////////////////////////
void CLoggerRxSocket::PrintCorruptedTrace(CMplMcmsProtocol &prot, STATUS errorStatus)
{    
    const COMMON_HEADER_S & commonHeader = prot.GetCommonHeaderConst();
    const TRACE_HEADER_S & traceHeader = prot.GetTraceHeaderConst();
    const char *strDataNotNullTerminated = prot.GetDataConst();
    const DWORD strDataLen = (256 > prot.getDataLen() ? prot.getDataLen() : 256);

    const MplMcmsProtocolErrno_S & protErrno = prot.Errno();
    
    char strDatabuffer[strDataLen];
    if(NULL != strDataNotNullTerminated)
    {
        strncpy(strDatabuffer, strDataNotNullTerminated, strDataLen);
        strDatabuffer[strDataLen - 1] = '\0';
    }
    else
    {
        strDatabuffer[0] = '\0';
    }

    const std::string &strStatus = m_proc->GetStatusAsString(errorStatus);
    TRACEINTO_SOCKET << "\nCorrupted Trace : " << strStatus.c_str() << "\n"
              << protErrno.m_Message << "\n"
              << CCommonHeaderWrapper(commonHeader)  << "\n"
              << CTraceHeaderWrapper(traceHeader)    << "\n"
              << strDatabuffer;
}

//////////////////////////////////////////////////////////////////////
STATUS CLoggerRxSocket::ValidateHeaders(const CMplMcmsProtocol &prot)
{
    const COMMON_HEADER_S & commonHeader = prot.GetCommonHeaderConst();
    const TRACE_HEADER_S & traceHeader = prot.GetTraceHeaderConst();
    
    STATUS statusCommonHeaderValid = ValidateCommonHeader(commonHeader);
    if(STATUS_OK != statusCommonHeaderValid)
    {
        return statusCommonHeaderValid;
    }
    
    STATUS statusTraceHeaderValid = ValidateTraceHeader(traceHeader, prot.getDataLen());
    if(STATUS_OK != statusTraceHeaderValid)
    {
        return statusTraceHeaderValid;
    }
    
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CLoggerRxSocket::ValidateCommonHeader(const COMMON_HEADER_S & commonHeader)const
{
    STATUS retStatus = STATUS_OK;
    if(commonHeader.src_id >= NUM_OF_MAIN_ENTITIES)
	{
		 retStatus = STATUS_BAD_MAIN_ENTITY;
	}
    return retStatus;
}

//////////////////////////////////////////////////////////////////////
STATUS CLoggerRxSocket::ValidateTraceHeader(const TRACE_HEADER_S & traceHeader, DWORD expectedDataLen)const
{
    STATUS retStatus = STATUS_OK;
    if(false == CTrace::IsTraceLevelValid(traceHeader.m_level))
	{
		retStatus = STATUS_BAD_TRACE_LEVEL;
	}
    else if(traceHeader.m_messageLen > expectedDataLen)
    {
        retStatus = STATUS_BAD_MESSAGE_LEN;
    }

    return retStatus;
}

//////////////////////////////////////////////////////////////////////
void CLoggerRxSocket::FixTaskName(const CMplMcmsProtocol & prot)
{
	const char *sourceName = MainEntityToString(prot.getCommonHeaderSrc_id());
	m_TaskName += sourceName;
}
