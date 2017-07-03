#include "McmsDaemonApi.h"
#include "Segment.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"




//////////////////////////////////////////////////////////////////////
CMcmsDaemonApi::CMcmsDaemonApi()
    :CTaskApi(eProcessMcmsDaemon, eManager)
{
}

//////////////////////////////////////////////////////////////////////
STATUS CMcmsDaemonApi::SendResetReq(const string & desc)
{
    CSegment *pParam = new CSegment;
    *pParam << desc;

    STATUS status = SendMsg(pParam, TO_MCMS_DAEMON_RESET_MCMS_REQ);
	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CMcmsDaemonApi::SendResetExternalReq(const string & desc)
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	PTRACE(eLevelInfoNormal,"CMcmsDaemonApi::SendResetExternalReq - ERROR - system is not CG!!");
    	return STATUS_FAIL;
    }

	CSegment *pParam = new CSegment;
    *pParam << desc;

    STATUS status = SendMsg(pParam, TO_MCMS_DAEMON_RESET_MCMS_EXT_REQ);
	return status;
}

//////////////////////////////////////////////////////////////////////
STATUS CMcmsDaemonApi::SendResetProcessReq(eProcessType processType)
{
    if(processType <= eProcessMcmsDaemon || NUM_OF_PROCESS_TYPES <= processType)
    {
        PASSERTMSG(processType + 1000, "Bad process type (code == processType + 1000)");
        return STATUS_FAIL;
    }

    CSegment *pParam = new CSegment;
    *pParam << (DWORD)processType;

    STATUS status = SendMsg(pParam, TO_MCMS_DAEMON_RESET_PROCESS_REQ);
	return status;
}
//////////////////////////////////////////////////////////////////////
STATUS CMcmsDaemonApi::SendConfigApacheInd()
{
	TRACEINTO << __FUNCTION__;
	//sleep(4);
    STATUS status = SendOpcodeMsg(CONFIG_APACHE_FINISHED_IND);
	return status;
}
