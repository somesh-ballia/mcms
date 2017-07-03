// FaultsManager.cpp: implementation of the CFaultsManager class.
//
//////////////////////////////////////////////////////////////////////


#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "SystemFunctions.h"
#include "Segment.h"
#include "FaultsProcess.h"
#include "HlogElement.h"
#include "FaultsManager.h"
#include "TraceStream.h"
#include "FaultDesc.h"
#include "NStream.h"
#include "FaultsDefines.h"
#include "ApiStatuses.h"
#include "OsFileIF.h"
#include "OpcodesMcmsInternal.h"
#include "TerminalCommand.h"
#include "StructTm.h"
#include "psosxml.h"
#include "FaultsContainer.h"
#include <fstream>
#include <errno.h>

#define		HLOG_FILE_NAME_PREFIX		"Faults/Faults_"
#define		HLOG_SHORT_FILE_NAME_PREFIX	"Faults/FaultsShort_"
#define         FAULTS_DIR                      "Faults/"



////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CFaultsManager)
  ONEVENT(XML_REQUEST    ,IDLE    ,  CFaultsManager::HandlePostRequest )
  ONEVENT(DUMP_FAULTS_FILE_REQ,		ANYCASE  , CFaultsManager::DumpAllFaultsToFile )
PEND_MESSAGE_MAP(CFaultsManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CFaultsManager)
//	ON_TRANS("TRANS_FAULTS_LIST","GET",COperCfg,CFaultsManager::HandleOperLogin)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CFaultsManager)
	ONCOMMAND("dump_faults", CFaultsManager::HandleTerminalDumpAllFaultsToFile, "dump_faults [fault file name] - dump all faults to a file.")
END_TERMINAL_COMMANDS

extern void FaultsMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void FaultsManagerEntryPoint(void* appParam)
{
	CFaultsManager * pFaultsManager = new CFaultsManager;
	pFaultsManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CFaultsManager::GetMonitorEntryPoint()
{
	return FaultsMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFaultsManager::CFaultsManager()
{

}

//////////////////////////////////////////////////////////////////////
CFaultsManager::~CFaultsManager()
{

}

//////////////////////////////////////////////////////////////////////
BOOL CFaultsManager::TaskHandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	switch(opCode)
	{
		case HLOG_LOGGER:
		{
			HandleHlogMessage(pMsg,msgLen,FALSE);
			return TRUE;
		}
		case HLOG_LOGGER_FULL_ONLY:
		{
			HandleHlogMessage(pMsg,msgLen,TRUE);
			return TRUE;
		}	
		default:
		{
			return FALSE;
		}
	}

}

//////////////////////////////////////////////////////////////////////
void CFaultsManager::HandleHlogMessage(CSegment* pMsgSeg,const DWORD msgLen,const BOOL isFullHlogOnly)
{

	CSegment  rSegTmp(*pMsgSeg);
	CHlogElement* pHlogElementTmp = new CHlogElement;
	pHlogElementTmp->DeSerialize(rSegTmp);

	CHlogElement* pHlogElement = NewCHlogElement(pHlogElementTmp->GetCode());
	if(pHlogElement)
	{
		pHlogElement->DeSerialize(*pMsgSeg);

		HlogsLogger(pHlogElement,isFullHlogOnly);
	}

	POBJDELETE(pHlogElementTmp);
	POBJDELETE(pHlogElement);
}

//////////////////////////////////////////////////////////////////////
void CFaultsManager::HlogsLogger(CHlogElement* pHlogElement,const BOOL isFullHlogOnly)
{
    	//Obtaing and updating time
	CStructTm curTime;
	::SystemGetTime(curTime);


/*	CMcuTime McuTime;
	if( ::isValidPObjectPtr(::GetpMcuTime()))
	{
		McuTime.LocalTime(curTime, ::GetpMcuTime()->GetGMTOffset(),
			!(::GetpMcuTime()->GetGMTOffsetSign()));
	}
	else
		DBGPASSERT(1);*/

	pHlogElement->SetTime(curTime);

    	// Write to LogFile
    CHlogList*  pHlogList = NULL;
    // allways store in full faults list
	pHlogList = ((CFaultsProcess*)CProcessBase::GetProcess())->GetFaultsListDB();
	if( pHlogList ) {
		pHlogList->AddElement(pHlogElement);
	}
	// sometimes store in Short faults list too
	if( FALSE == isFullHlogOnly ) {

		pHlogList = ((CFaultsProcess*)CProcessBase::GetProcess())->GetFaultsShortListDB();
		if( pHlogList ) {

			pHlogList->AddElement(pHlogElement);
		}
	}

	// AA printed in AA task
	if(FAULT_TASK_ACTIVE_ALARM != pHlogElement->GetCode())
	{
		COstrStream ostr;
		pHlogElement->Dump(ostr);
		TRACEINTO << "\nFault : " << ostr.str().c_str();
	}
}

//////////////////////////////////////////////////////////////////////
//STATUS CFaultsManager::HandleTerminalPing(CSegment * seg,std::ostream& answer)
//{
//	PTRACE(eLevelError,"pong to logger");
//	answer << "pong to console";
//	return STATUS_OK;
//}


////////////////////////////////////////////////////////////////////////////
void CFaultsManager::ManagerPostInitActionsPoint()
{
	CFaultsProcess *process = dynamic_cast<CFaultsProcess*>(CProcessBase::GetProcess());

	if (FALSE == IsHardDiskOk())
    {
    	process->SetIsHardDiskOk(NO);

        AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                                BAD_HARD_DISK,
                                MAJOR_ERROR_LEVEL,
                                "Hard disk not responding",
                                true,
                                true
                                );
    }

    else
    {

		// Creates folder if needed
		if (!IsFileExists(FAULTS_DIR))
		{		    

		    BOOL res = CreateDirectory(FAULTS_DIR);
	 	    if (false == res)
		    {	
		        PASSERTSTREAM_AND_RETURN(!res,
			        "CreateDirectory: " << FAULTS_DIR << ": Unable to create: " << strerror(errno));
		    }
		}
		CHlogList *pFaultsList      = new CHlogList(HLOG_FILE_NAME_PREFIX);
		process->SetFaultList(pFaultsList);

		CHlogList *pFaultsShortList  = new CHlogList(HLOG_SHORT_FILE_NAME_PREFIX);
		process->SetShortFaultList(pFaultsShortList);
    }
}


void CFaultsManager::DumpAllFaultsToFile(CSegment* pSeg)
{
	TRACEINTO << "DumpAllFaultsToFile\n";

	std::string faultFileName;
	*pSeg >> faultFileName;


	STATUS retVal = WriteAllFaultsToFile(faultFileName);


    STATUS stat = ResponedClientRequest(retVal,NULL);

    if (STATUS_OK != stat)
    {
		CProcessBase* proc = CProcessBase::GetProcess();
		PASSERTMSG_AND_RETURN(NULL == proc, "Unable to continue - Failed respond to collector");
		 PASSERTSTREAM(STATUS_OK != retVal,
			 "Unable to send a message: " << proc->GetStatusAsString(stat));
    }

}
STATUS CFaultsManager::WriteAllFaultsToFile(const std::string& outputFaultFileName) const
{

	std::ofstream file(outputFaultFileName.c_str(),
			ios_base::out | ios_base::trunc);

	TRACEINTO << "outputFaultFileName " << outputFaultFileName << "\n";


	if (file.is_open())
	{
	    CHlogList* pHlogList = ((CFaultsProcess*)CProcessBase::GetProcess())->GetFaultsListDB();

	    pHlogList->DumpAllVectorAsString(file);

		file.flush();
		file.close();

		return STATUS_OK;
	}
	else
	{
		PASSERTSTREAM(TRUE, "Failed open faults file " << outputFaultFileName  );
		return STATUS_FILE_OPEN_ERROR;
	}
}

STATUS CFaultsManager::HandleTerminalDumpAllFaultsToFile(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if (1 > numOfParams)
	{
		//answer  << "usage: Bin/McuCmd dump_faults <Faults file name> [Start Time. E.g. 2014-04-09T12:05:20] [End Time. E.g 2024-04-09T18:05:20] \n";
		answer  << "usage: Bin/McuCmd dump_faults <Faults file name>  \n";
		return STATUS_FAIL;
	}

	const std::string &faultFileName = command.GetToken(eCmdParam1);

	OPCODE retVal = WriteAllFaultsToFile(faultFileName);

	if (retVal != STATUS_OK)
	{
		answer << "Failed to write Faults file\n";
	}
	else
	{
		answer << "Faults file " << faultFileName << " was created. \n";
	}


	return STATUS_OK;
}

