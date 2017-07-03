// ClientLoggerProcess.cpp: implementation of the CClientLoggerProcess class.
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include<fstream>
#include "ClientLoggerProcess.h"
#include "SystemFunctions.h"
#include "OsQueue.h"
#include "Segment.h"
#include "TaskApi.h"
#include "OpcodesMcmsCommon.h"
#include "NStream.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "TraceHeader.h"
#include "FilterTrace.h"
#include "TraceClass.h"
#include "OsFileIF.h"
#include "OpcodesMcmsInternal.h"
#include <stdlib.h>

#define Min(a, b) ((a) < (b) ? (a) : (b))
/*
enum eLogLevel
{
        eLevelOff          = 0,
        eLevelFatal        = 1,  //System crash, conference crash etc'. NULL pointer for example. (p0 jira issue)
        eLevelError        = 10, //Error which is not fatal but still a bug (p1,p2,p3,� jira issues)
        eLevelWarn         = 20, //Not a bug but has to be taken into consideration. High CPU usage for example.
        eLevelInfoHigh     = 30, //MPLAPI, Inter process and other high importance messages.
        eLevelInfoNormal   = 50, //Minimal debugging information
        eLevelDebug        = 70, //Debug messages (additional information on field)
        eLevelTrace        = 100 //Trace messages (mostly for lab)
};
*/
//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CClientLoggerProcess;
}

//////////////////////////////////////////////////////////////////////
CClientLoggerProcess::CClientLoggerProcess()
{
	m_validFilter = 0;
	m_numProcess = 0;
	m_isTarget = false;
	memset(m_printToScreen,0,NUM_OF_PROCESS_TYPES);
	memset(m_processFilesSize,0,sizeof(m_processFilesSize));
	memset(m_FiltersTraceLevels,eLevelInfoNormal,sizeof(m_FiltersTraceLevels));
	m_traceMap["Off"] = eLevelOff;
	m_traceMap["Fatal"] = eLevelFatal;
	m_traceMap["Error"] = eLevelError;
	m_traceMap["Warn"] = eLevelWarn;
	m_traceMap["InfoHigh"] = eLevelInfoHigh;
	m_traceMap["InfoNormal"] = eLevelInfoNormal;
	m_traceMap["Debug"] = eLevelDebug;
	m_traceMap["Trace"] = eLevelTrace;
	m_isAllProcess = false;
	m_isTraceLevel = false;
	m_exceptionFileSize =0;
	m_exceptionFile = "";
}

//////////////////////////////////////////////////////////////////////
CClientLoggerProcess::~CClientLoggerProcess()
{
}
bool CClientLoggerProcess::isHelp()
{
	int argc = this->GetArgc();
	char* const * pArr = this->GetArgv();
	if(argc == 1)
		return false;
	std::string param = pArr[1];
	unsigned found =0;
	if((param.find("help") != std::string::npos) ||(param.find("?") != std::string::npos))
	{
		cout << "******** Client Logger Help ************" << endl;
		cout << "Parameters pattern : {<process Name>,<process Name> ... } option {<trace level>,<trace level> ...}" << endl;
		cout << endl;
		std::string processList = "";
		int count =0;
		for (eProcessType i = eProcessMcmsDaemon; i < NUM_OF_PROCESS_TYPES; ++i,count++)
		{
			processList += ProcessTypeToStr(i);
			processList +=",";
			if(count == 5)
			{
				processList +="\n";
				count =0;
			}
		}
		cout <<"Process Names : " << processList << endl << endl;

		cout << "Trace Level : Off,Fatal,Error,Warn,InfoHigh,InfoNormal,Debug,Trace" << endl << endl;
		cout << "To set all process with trace level : {*}{<trace level>}" << endl << endl;
		cout << "Output file location : " << endl << endl;
		cout << "Simulator : " << MCU_TMP_DIR+"/ClientLogger/ " << endl << endl;
		cout << "target : "+MCU_OUTPUT_TMP_DIR+"/ClientLogger/ " << endl << endl;
		cout << "Exceptions of all process will be written in the exceptions file" << endl;
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////
int CClientLoggerProcess::Run()
{
    setgid(200);        
    setuid(200); // set user and group as mcms
    
    if(!isHelp())
    {

    	SetUp();


    	Subscribe();
	
    	while (m_selfKill == FALSE)
    	{
    		HandleLoggerDispatch();

//		SystemSleep(10);
    	}

    	std::cout << std::flush;
	
    	UnSubcribe();
    }
	TearDown();
	return TRUE;
}
//////////////////////////////////////////////////////////////////////
void CClientLoggerProcess::buildFilesPaths()
{
	std::string base;
	if(m_isTarget)
		base = MCU_OUTPUT_TMP_DIR+"/ClientLogger/";
	else
		base = MCU_TMP_DIR+"/ClientLogger/";

	CreateDirectory(base.c_str());
	for(int i=0;i < NUM_OF_PROCESS_TYPES;i++)
	{
		std::string file = base + ProcessNames[i] + "client_logger.txt";
		m_processFiles[i] = file;

	}
	m_exceptionFile = base + EXCEPTION_FILE_NAME;
}
void CClientLoggerProcess::removeOldLogFiles()
{
	for(int i=0;i < NUM_OF_PROCESS_TYPES;i++)
	{
		if(IsFileExists(m_processFiles[i]))
			DeleteFile(m_processFiles[i],true);

	}
	//delete exception file
	if(IsFileExists(m_exceptionFile))
		DeleteFile(m_exceptionFile,true);
}
//////////////////////////////////////////////////////////////////////
int CClientLoggerProcess::SetUp()
{
	if(IsTarget())
		m_isTarget = true;
	else
		m_isTarget = false;
	buildFilesPaths();
	removeOldLogFiles();

	m_validFilter = parseFiltersFromCmdLine();
	printf("m_validFilter = %d ,m_isAllProcess =%d ",m_validFilter,m_isAllProcess);
	if(m_validFilter && !m_isAllProcess && m_isTraceLevel)
	{
		for(int i=0;i< m_numProcess;i++)
		{
			eProcessType eProcess = m_processFilters[i];
			eLogLevel  level  = m_FiltersTraceLevels[i];
			std::string sProc = ProcessTypeToStr(eProcess);
			printf("  Update Process %s level %d \n",sProc.c_str(),level);
			UpdateLogLevel(level,eProcess);
		}
	}
	if(m_validFilter && m_isAllProcess)
	{
		eLogLevel  level  = m_FiltersTraceLevels[0];
		printf(" Update All Process with trace level %d \n",level);
		UpdateAllProcessLogLevel(level);
	}
	return 0;
}
eProcessType CClientLoggerProcess::getProcessType(const char* processName )
{
	for(int i=0;i < NUM_OF_PROCESS_TYPES;i++)
	{
		if(strcmp(processName,ProcessNames[i])== 0)
			return (eProcessType)i;
	}
	return eProcessTypeInvalid;
}

int CClientLoggerProcess::parseProcess(char del,std::string strProcess)
{
	//remove ',' delimiter to whitespaces

	replace(strProcess.begin(), strProcess.end(), del, ' ');
	string process;
	stringstream ss;
	// parse process from string
	ss << strProcess;
	while(ss >> process)
	{
		eProcessType processFilter = getProcessType(process.c_str());
		if(processFilter == eProcessTypeInvalid)
		{
			//printf("Warning parseProcess eProcessTypeInvalid =%s",process.c_str());
			if(process == "*")
				m_isAllProcess = true;
			return 0;
		}
		m_processFilters[m_numProcess]=processFilter;
		m_numProcess++;

	}

	return 1;
}
/////////////////////////////////////////////////////////////////
int CClientLoggerProcess::parseTraceLevels(char del,std::string strLevels)
{
	replace(strLevels.begin(), strLevels.end(), del, ' ');
	string sLevel;
	stringstream ss;
	 std::map<std::string,eLogLevel>::iterator it;
	// parse levels from string
	 int i=0;
	 ss << strLevels;
	while(ss >> sLevel)
	{

		if((it=m_traceMap.find(sLevel)) != m_traceMap.end())
		{
			//printf("trace level  %s  was found \n",sLevel.c_str());
			m_FiltersTraceLevels[i] = it->second;
		}
		else
		{
			printf("trace level  %s  was NOT found \n",sLevel.c_str());
		}
		i++;
	}
	return 1;
}
/////////////////////////////////////////////////////////////////
int CClientLoggerProcess::parseFiltersFromCmdLine()
{
	int result =1;
	int argc = this->GetArgc();
	char* const * pArr = this->GetArgv();
	if(argc == 1)
		return 0;
	std::string filterInput = pArr[1];

	char delGroup=',';
	std::string token;
	//printf("parseFiltersFromCmdLine\n");
    int tokenRes = nextFilter(filterInput,0,token);
    //printf("first token %s \n",token.c_str());
    if(tokenRes)
    {
    	//printf("* parse process\n");
    	//parse process
    	result = parseProcess(delGroup,token);
    	printf("*** parse process resutl %d",result);
    	//parse trace levels

    	tokenRes = nextFilter(filterInput,tokenRes,token);
    	if(tokenRes)
    	{
    		m_isTraceLevel = true;
    		printf("second token %s \n",token.c_str());
    		printf("* parse trace levels\n");
    		parseTraceLevels(delGroup,token);
    		if(m_isAllProcess)
    			return 1;
    	}
    }
    else
    	return 0;
	/*size_t currPos =string::npos;
	size_t endPos = string::npos;
	//parse the process
	currPos = filterInput.find(delGroupStart);
	endPos = filterInput.find(delGroupEnd);
	if((currPos != string::npos) && (endPos != string::npos))
	{
		std:string process = filterInput.substr(currPos+1,endPos-currPos-1);
		result = parseProcess(delGroup,process);
		//check for trace levels

	}
	else
		return 0;*/
		//printf("arg %s \n",pArr[i]);
	return result;
}
////////////////////////////////////////////////////////
int CClientLoggerProcess::nextFilter(std::string& filterInput,int index,std::string& token)
{
	char delGroupStart = '{';
	char delGroupEnd = '}';
	size_t currPos =string::npos;
	size_t endPos = string::npos;

	currPos = filterInput.find(delGroupStart,index);
	endPos = filterInput.find(delGroupEnd,index);
	//printf(" %s \n currPos =%d , endPos = %d \n",filterInput.c_str(),currPos,endPos);
	if((currPos != string::npos) && (endPos != string::npos))
	{
		token = filterInput.substr(currPos+1,endPos-currPos-1);
		//printf("token %s return  endPos = %d +1 \n",token.c_str(),endPos);
		return endPos+1;
	}
	else
		return 0;
}
/////////////////////////////////////////////////////////
void CClientLoggerProcess::logExceptionToFile(const char *log)
{
	std::ofstream  log_file;

	if(m_exceptionFileSize > MAX__FILTER_FILE_SIZE)
	{
	  log_file.open(m_exceptionFile.c_str(), ios::out|ios::trunc);
	  m_exceptionFileSize=0;
	}
	else
	  log_file.open(m_exceptionFile.c_str(), ios::out|ios::app);
	if(log_file.is_open())
	{
		log_file << log;
		m_exceptionFileSize += strlen(log);
	}
	else
	  printf("failed to open file \n");
}
////////////////////////////////////////////////////////
void CClientLoggerProcess::logToFile(int iProcessIndex,const char *log)
{

	  std::ofstream  log_file;
	  std::string filename = m_processFiles[iProcessIndex];
	  if(m_processFilesSize[iProcessIndex] > MAX__FILTER_FILE_SIZE)
	  {
		  log_file.open(filename.c_str(), ios::out|ios::trunc);
		  m_processFilesSize[iProcessIndex]=0;
	  }
	  else
		  log_file.open(filename.c_str(), ios::out|ios::app);
	  if(log_file.is_open())
	  {
		  log_file << log;
		  m_processFilesSize[iProcessIndex] += strlen(log);
	  }
	  else
		  printf("failed to open file \n");
}
//////////////////////////////////////////////////////////////////////
void CClientLoggerProcess::HandleLoggerDispatch()
{
	CMessageHeader header;
	
	STATUS status = m_TerminalMbx.Receive(header);
	if (status != STATUS_OK || 
		!header.IsValid()	||
		!header.m_segment)
	{
		POBJDELETE(header.m_segment);
		
		return;
	}
	
	*(header.m_segment) >> m_Buffer;
	if(m_validFilter)
	{
		string traceBuffer(m_Buffer);
		if(traceBuffer.find("EXCEPTION") != string::npos)
		{
			logExceptionToFile(m_Buffer);
		}
		for(int i=0;i< m_numProcess;i++)
		{
			string processname = ProcessTypeToStr(m_processFilters[i]);
			string filter = "P:" + processname;
			if(traceBuffer.find(filter) != string::npos)
			{
				if(m_printToScreen[i])
					std::cout << m_Buffer;
				logToFile((int)m_processFilters[i],m_Buffer);
				//eLogLevel level = eLevelFatal;
				//UpdateLogLevel(level);
			}
		}
	}
	else
		std::cout << m_Buffer;
	
	static DWORD cnt = 0;
	cnt++;
	if(0 == cnt % 100)
	{
		std::cout << std::flush;
	}
	
	POBJDELETE(header.m_segment);
}

//////////////////////////////////////////////////////////////////////
void CClientLoggerProcess::Subscribe()
{
	int count =0;
	while(count < 120)
	{
		if(!CProcessBase::IsProcessAlive(eProcessLogger))
		{
			//printf("* waiting for logger process %d\n",count);
			count++;
			sleep(2);
		}
		else
			break;
	}



	COsQueue::CreateUniqueQueueName(m_UniqueName);
	m_TerminalMbx.CreateRead(eProcessLogger,m_UniqueName);

	CSegment *seg = new CSegment;	
	*seg << m_UniqueName;

	STATUS res =  SendCommand(seg, eProcessLogger, SUBSCRIBE);
}	

//////////////////////////////////////////////////////////////////////
void CClientLoggerProcess::UnSubcribe()
{
	CSegment *seg = new CSegment;
	*seg << m_UniqueName;

	STATUS res =  SendCommand(seg, eProcessLogger, UNSUBSCRIBE);
}

//////////////////////////////////////////////////////////////////////
STATUS CClientLoggerProcess::SendCommand(CSegment *seg, eProcessType destinationProcess, OPCODE opcode)
{
	const COsQueue * managerQueue = GetOtherProcessQueue(destinationProcess, eManager);
	((COsQueue*)managerQueue)->m_process = (eProcessType) -1;

	CTaskApi api;
	api.CreateOnlyApi(*managerQueue);

	STATUS res = api.SendMsg(seg,opcode);	
	printf("ClientLogger : api.SendMsg status %d \n",res);
	return res;
}
//////////////////////////////////////////////////////////////////////
STATUS CClientLoggerProcess::UpdateLogLevel(eLogLevel new_level,eProcessType iProcess)
{
  // Updates the log level to all MCMS processes.
  //for (eProcessType i = eProcessMcmsDaemon; i < NUM_OF_PROCESS_TYPES; ++i)


    CSegment* msg = new CSegment;
    *msg << static_cast<unsigned int>(new_level);
    SendCommand(msg,iProcess,LOGGER_ALL_MAX_TRACE_LEVEL);

  return STATUS_OK;
}


STATUS CClientLoggerProcess::UpdateAllProcessLogLevel(eLogLevel new_level)
{
	for (eProcessType i = eProcessMcmsDaemon; i < NUM_OF_PROCESS_TYPES; ++i)
	{
	    if (!CProcessBase::IsProcessAlive(i))
	      return 0;
	    CSegment* msg = new CSegment;
	    *msg << static_cast<unsigned int>(new_level);
	    SendCommand(msg,i,LOGGER_ALL_MAX_TRACE_LEVEL);
	}
   return STATUS_OK;
}
