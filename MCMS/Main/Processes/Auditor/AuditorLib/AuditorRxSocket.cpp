#include <netinet/in.h>
#include <map>
using namespace std;




#include "AuditorRxSocket.h"
#include "Segment.h"
#include "ProcessBase.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "StatusesGeneral.h"
#include "LoggerDefines.h"
#include "ManagerApi.h"
#include "AuditorProcess.h"
#include "WrappersCommon.h"
#include "TraceStream.h"
#include "ManagerApi.h"
#include "TraceClass.h"
#include "ObjString.h"
#include "AuditorApi.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsHeaderValidator.h"


static CAuditorProcess *pProcess = NULL;

extern const char* MainEntityToString(APIU32 entityType);

extern "C" void AuditorSocketRxEntryPoint(void* appParam)
{  	
	CAuditorRxSocket *pTaskApp = new CAuditorRxSocket;
	pTaskApp->Create(*(CSegment*)appParam);
	*(CSegment*)appParam << (void*)pTaskApp ;
}




//////////////////////////////////////////////////////////////////////
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





//////////////////////////////////////////////////////////////////////
CAuditorRxSocket::CAuditorRxSocket()
{
	pProcess = dynamic_cast<CAuditorProcess*>(CProcessBase::GetProcess());
//	m_IsNameFixed = false;
}

//////////////////////////////////////////////////////////////////////
CAuditorRxSocket::~CAuditorRxSocket()
{
}

//////////////////////////////////////////////////////////////////////
void CAuditorRxSocket::InitTask()
{
}

//////////////////////////////////////////////////////////////////////
void CAuditorRxSocket::ReceiveFromSocket()
{
	char buffTPKTHdr[sizeof(TPKT_HEADER_S)];
	int sizeRead = 0;
    
	STATUS status = Read(buffTPKTHdr, sizeof(TPKT_HEADER_S), sizeRead);	
	if (STATUS_OK != status)
	{
		//PASSERTMSG(status, "FAILED to Read TPKT");
		return;
	}

	DWORD messageLen = 0;
	bool res = ReadValidate_TPKT_Header(buffTPKTHdr, messageLen);
	if(false == res)
	{
		OnCorruptedTPKT(buffTPKTHdr);
		return;
	}
    
    sizeRead = 0;
	status = Read((char*)m_SocketBuffer, messageLen, sizeRead);
    m_SocketBuffer[messageLen - 1] = '\0';

    CMplMcmsProtocol prot;
    res = prot.DeSerialize(m_SocketBuffer, messageLen);
    if(false == res)
    {
        PrintCorruptedEvent(prot, "Deserialize Failed");
        return;
    }

    res = ValidateHeaders(prot);
    if(false == res)
    {
        PrintCorruptedEvent(prot, "Header validation Failed");
        return;
    }
    
    const AUDIT_EVENT_HEADER_S &auditHdr = prot.GetAuditHeaderConst();
    const char *pFreeData = prot.getpData();
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "Event Details",
                                 eFreeDataTypeText,
                                 pFreeData,
                                 "",
                                 eFreeDataTypeText,
                                 "");
    
    CAuditorApi api;
    api.SendEventOutsider(auditHdr, freeData);
}

//////////////////////////////////////////////////////////////////////
bool CAuditorRxSocket::ValidateHeaders(const CMplMcmsProtocol & prot)
{
    CLargeString errorString;
    
    const COMMON_HEADER_S & hdrCommon = prot.GetCommonHeaderConst();
    bool res = CCommonHeaderValidator(hdrCommon).Validate(errorString);
    if(!res)
    {
        TRACEINTO << "\nHeader's validation failed (Common) : " << errorString.GetString();
        return res;
    }

    const AUDIT_EVENT_HEADER_S & hdrAudit = prot.GetAuditHeaderConst();
    res = CAuditHeaderValidator(hdrAudit).Validate(errorString);
    if(!res)
    {
        TRACEINTO << "\nHeader's validation failed (Audit) : " << errorString.GetString();
        return res;
    }
    
    return res;
}

//////////////////////////////////////////////////////////////////////
void CAuditorRxSocket::PrintCorruptedEvent(CMplMcmsProtocol &prot, const string & errorMessage)
{    
    const COMMON_HEADER_S & commonHeader = prot.GetCommonHeaderConst();
    const AUDIT_EVENT_HEADER_S &auditHeader = prot.GetAuditHeaderConst();
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
    
    TRACEINTO << "\nCorrupted Audit Event : " << errorMessage << "\n"
              << protErrno.m_Message << "\n"
              << CCommonHeaderWrapper(commonHeader)  << "\n"
              << CAuditHeaderWrapper(auditHeader)    << "\n"
              << strDatabuffer;
    PASSERTMSG(TRUE, "Corrupted Audit Event");
}

