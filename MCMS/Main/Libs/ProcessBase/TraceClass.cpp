// TraceClass.cpp

#include "TraceClass.h"

#include <iostream>
#include<fstream>
#include "Segment.h"
#include "TaskApi.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"
#include "SystemFunctions.h"
#include "OsFileIF.h"
#include "OpcodesMcmsCommon.h"
#include "FilterTraceContainer.h"
#include "InternalProcessStatuses.h"
#include "LoggerDefines.h"
#include "OutsideEntities.h"
#include  "StructTm.h"
#include <errno.h>



int CTrace::Init = InitTrace_Level_Names();
char CTrace::TRACE_LEVEL_NAMES	     [NUM_OF_TRACE_TYPES][STR_LEN_OF_TRACE_TYPE];
char CTrace::TRACE_LEVEL_SHORT_NAMES [NUM_OF_TRACE_TYPES][STR_LEN_OF_TRACE_TYPE];
long CTrace::m_localFileSize =0;
COMMON_HEADER_S			CTrace::m_prev_commonHeader;
PHYSICAL_INFO_HEADER_S	CTrace::m_prev_physicalHeader;
TRACE_HEADER_S			CTrace::m_prev_traceHeader;
MCMS_TRACE_HEADER_S		CTrace::m_prev_mcmsHeader;
std::string				CTrace::m_title;
std::map<std::string,long>     CTrace::m_filesEntitiesSize;

extern const char* MainEntityToString(APIU32 entityType);
static const char *EmaProcessNames[] =
{
	"EmaProcess",  // eTheOneTheOnlyEmaProcess
};
static const char *GetEmaProcessName(eEmaProcesses type)
{
	const char *name = (eTheOneTheOnlyEmaProcess <= type && type < NumOfEmaProcesses
						?
						EmaProcessNames[type] : "InvalideProcess");
	return name;
}





static const char *CSProcessNames[] =
{
	"NA",  // eTheOneTheOnlyCSModuleProcess
};
static const char *GetCSProcessName(eCSProcesses type)
{
	const char *name = (eTheOneTheOnlyCSProcess <= type && type < NumOfCSProcesses
						?
						CSProcessNames[type] : "InvalideProcess");
	return name;
}





static const char *MplProcessNames[] =
{
	"NA"						,  // eTheOneTheOnlyMplProcess
	"MfaCardManager"			,
	"SwitchCardManager"			,
	"VideoDsp"					,
	"ArtDsp"					,
	"EmbeddedApacheModule"		,
	"IceManagerProcess"			,
	"MfaLauncher"               ,
	"AMP"						,
	"VMP"						,
	"MPProxy"
};
static const char *GetMplProcessName(eMplProcesses type)
{
	const char *name = (type >= 0 && (unsigned int)type < ARRAYSIZE(MplProcessNames)
						?
						MplProcessNames[type] : "InvalideProcess");
	return name;
}

const char* CTrace::GetTraceLevelNameByValue(int index)		
{ 
	const char *name;
	if(index==eLevelOff)
	    name = TRACE_LEVEL_NAMES[index];
	else
	    name = (IsTraceLevelValid(index) ? TRACE_LEVEL_NAMES[index] : "Invalid");
	return name;			
}

const char* CTrace::GetTraceLevelShortNameByValue(int index)	
{ 
	const char *name;
	if(index==eLevelOff)
	    name = TRACE_LEVEL_SHORT_NAMES[index];
	else
	    name = (IsTraceLevelValid(index) ? TRACE_LEVEL_SHORT_NAMES[index] : "Invalid");
	return name;
}

void CTrace::FillTraceLevelNames(std::ostream& answer)
{
	for (DWORD i = 0 ; i < NUM_OF_TRACE_TYPES ; i++)
	{
		if (TRACE_LEVEL_NAMES[i][0] != 0)
		{
			answer << i << " :\t" << TRACE_LEVEL_NAMES[i] << std::endl;
		}
	}
}

void CTrace::OutMessage(const TRACE_HEADER_S& traceHeader,
                        const MCMS_TRACE_HEADER_S& mcmsHeader,
                        const std::string& content)
{
    // Internal check
    if (content.length() != traceHeader.m_messageLen)
    {
        perror("data.length() != traceHeader.m_messageLen");
        return;
    }

    //The message will be trimmed when its size exceeds MAX_CONTENT_SIZE
	UINT32 msgLen = traceHeader.m_messageLen > MAX_CONTENT_SIZE ? MAX_CONTENT_SIZE : traceHeader.m_messageLen;

    CProcessBase * process = CProcessBase::GetProcess();
    CFilterTraceContainer *fltContainer = process->GetTraceFilterContainer();

	bool resFilter = fltContainer->CheckFilter(traceHeader);
	if ((false == resFilter) && (!process->m_enableLocalTracer))
	{
		return;
	}
    
    const DWORD totalBufferLen  =  sizeof(TRACE_HEADER_S) + sizeof(MCMS_TRACE_HEADER_S) + msgLen + 1;
    BYTE *bufferToSend = new BYTE[totalBufferLen];
	if (NULL == bufferToSend)
	{
		return;
	}

    //              bufferToSend
	//|-------------|-----------|-------------------------------|"
	// traceHeader   mcmsHeader  content

    DWORD offset = 0;
    
    // 1) insert trace header
    memcpy(bufferToSend + offset, &traceHeader, sizeof(TRACE_HEADER_S));
    offset += sizeof(TRACE_HEADER_S);

    // 2) insert mcms header
    memcpy(bufferToSend + offset, &mcmsHeader, sizeof(MCMS_TRACE_HEADER_S));
    offset += sizeof(MCMS_TRACE_HEADER_S);
    
    // 3) insert content
    memcpy(bufferToSend + offset, (const BYTE*)content.c_str(), msgLen);
    offset += msgLen;

    *(bufferToSend + offset) = (BYTE)'\0';

    // Internal check
    if (totalBufferLen != offset + 1)
    {
        delete [] bufferToSend;
        perror("totalBufferLen != offset + 1");
        return;
    }
    if(process->m_enableLocalTracer)
    {
    	std::string processName = process->GetProcessName(process->GetProcessType());
    	std::ostringstream buf;
    	BuildMcmsMessage(buf,traceHeader,mcmsHeader,content.c_str());
    	logToFile(processName,buf.str().c_str());

    }
    if ((false == resFilter))
    {
    	delete [] bufferToSend;
    	return;
    }
	CSegment *seg = new CSegment;
	seg->Put((BYTE*)bufferToSend, totalBufferLen);

    CTaskApi api(eProcessLogger,eManager);
    STATUS res = api.SendMsg(seg,TRACE_MESSAGE);
	if (res == STATUS_BROKEN_PIPE || res == STATUS_TRANSPORT_NOTCONNECTED)
    {
        //Logger is dead...
        process->CloseOtherProcessQueue(eProcessLogger);
    }
    
	delete [] bufferToSend;

	process->m_numTrace++;
	if (STATUS_OK != res)
	{
		process->m_numTraceNotSent++;
	}
}
void CTrace::BuildOutMessage(std::ostringstream& buf,CMplMcmsProtocol& prot)
{
	const COMMON_HEADER_S&        commonHeader = prot.GetCommonHeaderConst();
	const PHYSICAL_INFO_HEADER_S& physicalHeader = prot.GetPhysicalHeaderConst();
	TRACE_HEADER_S&               traceHeader = prot.GetTraceHeader();
	const char*                   ptrContent = (const char*)(prot.GetData());

	MCMS_TRACE_HEADER_S mcmsHeader;
	memset(&mcmsHeader, 0, sizeof mcmsHeader);

	BuildGenericMessage(buf,
	                      commonHeader,
	                      physicalHeader,
	                      traceHeader,
	                      mcmsHeader,
	                      ptrContent);
}

DWORD CTrace::GetTraceLevelByName(const char *traceLevelName)
{
	for (DWORD i = 0 ; i < NUM_OF_TRACE_TYPES ; i++)
	{
		if (TRACE_LEVEL_NAMES[i] != 0 &&
			(0 == strcmp(TRACE_LEVEL_NAMES[i], traceLevelName) ||
			0 == strcmp(TRACE_LEVEL_SHORT_NAMES[i], traceLevelName))
			)
		{
			return i;
		}
	}

	return NUM_OF_TRACE_TYPES;
}

int CTrace::InitTrace_Level_Names()
{
	char *levelName = "TRACE";
	memcpy(TRACE_LEVEL_NAMES[eLevelTrace], levelName, strlen(levelName) + 1);
	char *shortName = "t";
	memcpy(TRACE_LEVEL_SHORT_NAMES[eLevelTrace], shortName, strlen(shortName) + 1);

	levelName = "DEBUG";
	memcpy(TRACE_LEVEL_NAMES[eLevelDebug], levelName, strlen(levelName) + 1);
	shortName = "d";
	memcpy(TRACE_LEVEL_SHORT_NAMES[eLevelDebug], shortName, strlen(shortName) + 1);

	levelName = "INFO_NORMAL";
	memcpy(TRACE_LEVEL_NAMES[eLevelInfoNormal], levelName, strlen(levelName) + 1);
	shortName = "n";
	memcpy(TRACE_LEVEL_SHORT_NAMES[eLevelInfoNormal], shortName, strlen(shortName) + 1);

	levelName = "INFO_HIGH";
	memcpy(TRACE_LEVEL_NAMES[eLevelInfoHigh], levelName, strlen(levelName) + 1);
	shortName = "i";
	memcpy(TRACE_LEVEL_SHORT_NAMES[eLevelInfoHigh], shortName, strlen(shortName) + 1);

	levelName = "WARN";
	memcpy(TRACE_LEVEL_NAMES[eLevelWarn], levelName, strlen(levelName) + 1);
	shortName = "w";
	memcpy(TRACE_LEVEL_SHORT_NAMES[eLevelWarn], shortName, strlen(shortName) + 1);

	levelName = "ERROR";
	memcpy(TRACE_LEVEL_NAMES[eLevelError], levelName, strlen(levelName) + 1);
	shortName = "e";
	memcpy(TRACE_LEVEL_SHORT_NAMES[eLevelError], shortName, strlen(shortName) + 1);

	levelName = "FATAL";
	memcpy(TRACE_LEVEL_NAMES[eLevelFatal], levelName, strlen(levelName) + 1);
	shortName = "f";
	memcpy(TRACE_LEVEL_SHORT_NAMES[eLevelFatal], shortName, strlen(shortName) + 1);

	levelName = "OFF";
	memcpy(TRACE_LEVEL_NAMES[eLevelOff], levelName, strlen(levelName) + 1);
	shortName = "o";
	memcpy(TRACE_LEVEL_SHORT_NAMES[eLevelOff], shortName, strlen(shortName) + 1);

	return 42;
}

bool CTrace::IsTraceLevelValid(DWORD level)
{
	switch (level)
	{
	case eLevelFatal:
	case eLevelError:
	case eLevelWarn:
	case eLevelInfoHigh:
	case eLevelInfoNormal:
	case eLevelDebug:
	case eLevelTrace:
	    return true;

	default:
	    return false;
	}

	return false;
}
void CTrace::TraceEMAOutMessage(std::ostringstream& buf,const TRACE_HEADER_S& traceHeader)
{
	COMMON_HEADER_S commonHeader;
	memset(&commonHeader, 0, sizeof commonHeader);

	commonHeader.src_id  = eEma;
	commonHeader.dest_id = eMcms;
	TraceOutGenericMessage(buf,commonHeader,traceHeader);
}
void CTrace::TraceOutMessage(std::ostringstream& buf,CMplMcmsProtocol& prot)
{
	TraceOutGenericMessage(buf,prot.GetCommonHeaderConst(),prot.GetTraceHeader());
}
void CTrace::TraceOutGenericMessage(std::ostringstream& buf,
		  const COMMON_HEADER_S& commonHeader
		  ,const TRACE_HEADER_S& traceHeader)
{

	std::string proc_name =
	    			   GetProcessNameByMainEntity
	    			   	   (
	    					(eMainEntities)(commonHeader.src_id),
	    					traceHeader.m_processType
	    			   	   	);
	std::string str_entity = MainEntityToString(commonHeader.src_id);
	std::string  filename = str_entity+ "_" +proc_name;
	logToFileEntity(filename,buf.str().c_str());
}
void CTrace::logToFileEntity(std::string& entityName,const char *log)
{
	 std::ofstream  log_file;
 	  std::map<std::string,long>::iterator it;
	  long filesize =0;
	  std::string filename = LOCAL_TRACER_PATH + entityName +".log";
	  if((it=m_filesEntitiesSize.find(entityName)) == m_filesEntitiesSize.end())
	  {
		  if(IsFileExists(filename))
		  {
			  filesize = GetFileSize(filename);
			  m_filesEntitiesSize[entityName]=filesize;
		  }
	  }
	  else
	  {
		  filesize = it->second;
	  }
	  if(filesize > MAX__FILTER_FILE_SIZE)
	  {
	  	  log_file.open(filename.c_str(), std::ios::out|std::ios::trunc);
	  	  filesize=0;
	  }
	  else
	    log_file.open(filename.c_str(), std::ios::out|std::ios::app);

	  if(log_file.is_open())
	  {
	    log_file << log;
	    filesize += strlen(log);
	    m_filesEntitiesSize[entityName]=filesize;
	  }
}
void CTrace::logToFile(std::string& processName,const char *log)
{

	  std::ofstream  log_file;
	  std::string filename = LOCAL_TRACER_PATH + processName +".log";

	  if(m_localFileSize == 0)
	  {
		  if(IsFileExists(filename))
		  {
			  m_localFileSize = GetFileSize(filename) + +MAX__FILTER_FILE_SIZE;
		  }
	  }
	  if(m_localFileSize > MAX__FILTER_FILE_SIZE)
	  {
		  log_file.open(filename.c_str(), std::ios::out|std::ios::trunc);
		  m_localFileSize=0;
	  }
	  else
		  log_file.open(filename.c_str(), std::ios::out|std::ios::app);
	  if(log_file.is_open())
	  {
		  m_localFileSize += strlen(log);
		  log_file << log;
	  }
}
//////////////////////////////////////////////////////////////
void CTrace::BuildEmaMessage(TRACE_HEADER_S* pTraceHeader,const char* content, std::ostringstream& buf)
{
	COMMON_HEADER_S commonHeader;
	memset(&commonHeader, 0, sizeof commonHeader);

	commonHeader.src_id  = eEma;
	commonHeader.dest_id = eMcms;

	PHYSICAL_INFO_HEADER_S physicalHeader;
	memset(&physicalHeader, 0, sizeof physicalHeader);

	MCMS_TRACE_HEADER_S mcmsHeader;
	memset(&mcmsHeader, 0, sizeof mcmsHeader);

  BuildGenericMessage(buf,
	                      commonHeader,
	                      physicalHeader,
	                      *pTraceHeader,
	                      mcmsHeader,
	                      content);
}

/////////////
void CTrace::BuildMcmsMessage(std::ostringstream& buf,
			    const  TRACE_HEADER_S& traceHeader,
	            const MCMS_TRACE_HEADER_S& mcmsHeader,
	            const char* content)
{
	  COMMON_HEADER_S _commonHeader;
	  memset(&_commonHeader, 0, sizeof _commonHeader);

	  _commonHeader.src_id = eMcms;
	  _commonHeader.dest_id = eMcms;

	  PHYSICAL_INFO_HEADER_S _physicalHeader;
	  memset(&_physicalHeader, 0, sizeof _physicalHeader);
	  TRACE_HEADER_S copyTraceHeader = traceHeader;
	  BuildGenericMessage(buf,_commonHeader,_physicalHeader,copyTraceHeader,mcmsHeader,content);
}
/////////
 void CTrace::BuildGenericMessage(std::ostringstream& buf,
            const COMMON_HEADER_S& commonHeader,
            const PHYSICAL_INFO_HEADER_S& physicalHeader,
            TRACE_HEADER_S& traceHeader,
            const MCMS_TRACE_HEADER_S& mcmsHeader,
            const char* content)
 {




	  if ('\0' == traceHeader.m_taskName[0])
	      strcpy(traceHeader.m_taskName, "NULL");

	  if ('\0' == traceHeader.m_objectName[0])
	      strcpy(traceHeader.m_objectName, "NULL");

	  traceHeader.m_taskName[MAX_TASK_NAME_LEN - 1] = '\0';
	  CObjString::ReplaceChar(traceHeader.m_taskName, ' ', '_');
	  traceHeader.m_objectName[MAX_OBJECT_NAME_LEN - 1] = '\0';
	  CObjString::ReplaceChar(traceHeader.m_objectName, ' ', '_');
	  traceHeader.m_str_opcode[STR_OPCODE_LEN - 1] = '\0';
	  CObjString::ReplaceChar(traceHeader.m_str_opcode, ' ', '_');
	  traceHeader.m_terminalName[MAX_TERMINAL_NAME_LEN - 1] = '\0';
	  CObjString::ReplaceChar(traceHeader.m_terminalName, ' ', '_');

	  FillBuffer(buf, commonHeader, physicalHeader, traceHeader, mcmsHeader, content);
 }
//////////////

 void  CTrace::FillBuffer(std::ostringstream& buf,
                         const COMMON_HEADER_S& commonHeader,
                         const PHYSICAL_INFO_HEADER_S& physicalHeader,
                         const TRACE_HEADER_S& traceHeader,
                         const MCMS_TRACE_HEADER_S& mcmsHeader,
                         const char* content)
 {

	 DWORD messageLen = (NULL == content) ? 0 : strlen(content);

	 //The message will be trimmed when its size exceeds MAX_CONTENT_SIZE
	 messageLen = traceHeader.m_messageLen > MAX_CONTENT_SIZE ? MAX_CONTENT_SIZE : traceHeader.m_messageLen;

	 BOOL isNewHeaderNeeded = CheckIfNewHeaderIsNeeded(commonHeader,
	                                                   physicalHeader,
	                                                   traceHeader,
	                                                   mcmsHeader);


	  if (isNewHeaderNeeded)
	  {
	    // Keeps the title for possible future usage
	    GenerateTitle(commonHeader, traceHeader, m_title);

	    // Keeps the headers for possible future usage
	    memcpy(&m_prev_commonHeader, &commonHeader, sizeof m_prev_commonHeader);
	    memcpy(&m_prev_physicalHeader, &physicalHeader, sizeof m_prev_physicalHeader);
	    memcpy(&m_prev_traceHeader, &traceHeader, sizeof m_prev_traceHeader);
	    memcpy(&m_prev_mcmsHeader, &mcmsHeader, sizeof m_prev_mcmsHeader);
	  }


	  buf << m_title;

	  // Additional logger header
	  if (DEFAULT_TOPIC_ID != traceHeader.m_topic_id)
	    buf << " " << TITLE_TopicID << ":" << traceHeader.m_topic_id;

	  if (DEFAULT_UNIT_ID != traceHeader.m_unit_id)
	    buf << " " << TITLE_UnitID << ":" << traceHeader.m_unit_id;

	  if (DEFAULT_CONF_ID != traceHeader.m_conf_id)
	    buf << " " << TITLE_ConfID << ":" << traceHeader.m_conf_id;

	  if (DEFAULT_PARTY_ID != traceHeader.m_party_id)
	    buf << " " << TITLE_PartyID << ":" << traceHeader.m_party_id;

	  if ('\0' != mcmsHeader.m_file_line_number[0])
	    buf << " " << TITLE_FileLine << ":" << mcmsHeader.m_file_line_number;

	  if (0 != physicalHeader.board_id)
	    buf << " " << TITLE_BoardId << ":"
	        << static_cast<short>(physicalHeader.board_id);

	  if (0 != physicalHeader.port_id)
	    buf << " " << TITLE_PortId << ":"
	        << static_cast<short>(physicalHeader.port_id);

	  if (0 != physicalHeader.accelerator_id)
	    buf << " " << TITLE_AcceleratorId << ":"
	        << static_cast<short>(physicalHeader.accelerator_id);

	  buf << std::endl;
	  if(NULL == content)
	      buf << "empty";
	  else
	  {
		  std::string limitedContent(content, content+messageLen);
		  buf << limitedContent;
	  }
	  buf << "\n\n";

 }


/////////
 void CTrace::GenerateTitle(const COMMON_HEADER_S& commonHeader,
                                    const TRACE_HEADER_S& traceHeader,
                                    std::string& out)
 {
   const char* proc_name =
		   GetProcessNameByMainEntity((eMainEntities)commonHeader.src_id,
                                                traceHeader.m_processType);
   size_t proc_name_len = strlen(proc_name);

#if 0
   CStructTm localTime;
   SystemGetTime(localTime);
#endif

   const char* str_entity = MainEntityToString(commonHeader.src_id);
   size_t      str_entity_len = strlen(str_entity);

   const char* str_level = CTrace::GetTraceLevelShortNameByValue(traceHeader.m_level);
   size_t str_level_len = strlen(str_level);

   char   title[256];
   size_t len = 0;

   // Counts the title length
   size_t title_len = 29 + str_entity_len + 3 + proc_name_len
                      + 1 + ARRAYSIZE(TITLE_SourceId) + 10
                      + ARRAYSIZE(TITLE_TaskName)
                      + 1 + strlen(traceHeader.m_taskName)
                      + 3 + strlen(traceHeader.m_objectName)
                      + 3 + str_level_len;

   if (ARRAYSIZE(title) < title_len)
   {
     std::stringstream buf;
     buf << "buffer overflow: " << ARRAYSIZE(title) << " < " << title_len;
     out = buf.str();
     return;
   }

   title[len++] = TITLE_Date[0];
   title[len++] = ':';
   time_t deliveryTime = traceHeader.m_tv_sec;   
   len+=strftime(title+len,30,"%d/%m/%y-%T",gmtime(&deliveryTime));
#if 0
   title[len++] = (localTime.m_day / 10) % 10 + '0';
   title[len++] = localTime.m_day % 10 + '0';
   title[len++] = '/';
   title[len++] = (localTime.m_mon / 10) % 10 + '0';
   title[len++] = localTime.m_mon % 10 + '0';
   title[len++] = '/';
   title[len++] = ((localTime.m_year - 2000) / 10) % 10 + '0';
   title[len++] = (localTime.m_year - 2000) % 10 + '0';
   title[len++] = '-';
   title[len++] = (localTime.m_hour / 10) % 10 + '0';
   title[len++] = localTime.m_hour % 10 + '0';
   title[len++] = ':';
   title[len++] = (localTime.m_min / 10) % 10 + '0';
   title[len++] = localTime.m_min % 10 + '0';
   title[len++] = ':';
   title[len++] = (localTime.m_sec / 10) % 10 + '0';
   title[len++] = localTime.m_sec % 10 + '0';
#endif
   title[len++] = '.';
#if 0   
   title[len++] = (traceHeader.m_systemTick / 1000000) % 10 + '0';
   title[len++] = (traceHeader.m_systemTick / 100000) % 10 + '0';
   title[len++] = (traceHeader.m_systemTick / 10000) % 10 + '0';
   title[len++] = (traceHeader.m_systemTick / 1000) % 10 + '0';
   title[len++] = (traceHeader.m_systemTick / 100) % 10 + '0';
   title[len++] = (traceHeader.m_systemTick / 10) % 10 + '0';
   title[len++] = traceHeader.m_systemTick % 10 + '0';
#endif   
   //title[len++] = (traceHeader.m_tv_usec / 1000000) % 10 + '0';
   title[len++] = (traceHeader.m_tv_usec / 100000) % 10 + '0';
   title[len++] = (traceHeader.m_tv_usec / 10000) % 10 + '0';
   title[len++] = (traceHeader.m_tv_usec / 1000) % 10 + '0';
   title[len++] = (traceHeader.m_tv_usec / 100) % 10 + '0';
   title[len++] = (traceHeader.m_tv_usec / 10) % 10 + '0';
   title[len++] = traceHeader.m_tv_usec % 10 + '0';

   title[len++] = ' ';
   title[len++] = TITLE_MainEntity[0];
   title[len++] = ':';
   std::copy(str_entity, str_entity + str_entity_len, title + len);
   len += str_entity_len;

   title[len++] = ' ';
   title[len++] = TITLE_ProcessName[0];
   title[len++] = ':';
   std::copy(proc_name, proc_name + proc_name_len, title + len);
   len += proc_name_len;

   title[len++] = ' ';
   title[len++] = TITLE_SourceId[0];
   title[len++] = TITLE_SourceId[1];
   title[len++] = TITLE_SourceId[2];
   title[len++] = ':';
   title[len++] = (traceHeader.m_sourceId / 10000000) % 10 + '0';
   title[len++] = (traceHeader.m_sourceId / 1000000) % 10 + '0';
   title[len++] = (traceHeader.m_sourceId / 100000) % 10 + '0';
   title[len++] = (traceHeader.m_sourceId / 10000) % 10 + '0';
   title[len++] = (traceHeader.m_sourceId / 1000) % 10 + '0';
   title[len++] = (traceHeader.m_sourceId / 100) % 10 + '0';
   title[len++] = (traceHeader.m_sourceId / 10) % 10 + '0';
   title[len++] = traceHeader.m_sourceId % 10 + '0';

   title[len++] = ' ';
   title[len++] = TITLE_TaskName[0];
   title[len++] = TITLE_TaskName[1];
   title[len++] = ':';
   std::copy(traceHeader.m_taskName,
             traceHeader.m_taskName + strlen(traceHeader.m_taskName),
             title + len);
   len += strlen(traceHeader.m_taskName);

   title[len++] = ' ';
   title[len++] = TITLE_ObjectName[0];
   title[len++] = ':';
   std::copy(traceHeader.m_objectName,
             traceHeader.m_objectName + strlen(traceHeader.m_objectName),
             title + len);
   len += strlen(traceHeader.m_objectName);

   title[len++] = ' ';
   title[len++] = TITLE_Level[0];
   title[len++] = ':';
   std::copy(str_level, str_level + str_level_len, title + len);
   len += str_level_len;
   title[len] = '\0';

   out = title;
 }
///////////////
 const char* CTrace::GetProcessNameByMainEntity(eMainEntities mainEntityType,
                                                        DWORD processType)
 {
   switch (mainEntityType)
   {
   case eMcms:
     return CProcessBase::GetProcessName((eProcessType) processType);

   case eCentral_signaling:
     return GetCSProcessName((eCSProcesses) processType);

   case eEma:
     return GetEmaProcessName((eEmaProcesses) processType);

   case eCM_Switch:
   case eShelf:
   case eArtEntity:
   case eVideoEntity:
   case eCardManagerEntity:
   case eRTMEntity:
   case eMuxEntity:
   case eMpl:
   case eVmpEntity:
   case eAmpEntity:
   case eMpProxyEntity:
     return GetMplProcessName((eMplProcesses) processType);

   default:
     return "Invalid";
   }

   return "Invalid";
 }


 BOOL CTrace::CheckIfNewHeaderIsNeeded(const COMMON_HEADER_S& commonHeader,
                                               const PHYSICAL_INFO_HEADER_S&
                                               physicalHeader,
                                               const TRACE_HEADER_S& traceHeader,
                                               const MCMS_TRACE_HEADER_S&
                                               mcmsHeader)
 {
   m_prev_traceHeader.m_processMessageNumber = traceHeader.m_processMessageNumber;

   BOOL isDiffCommonHeader = memcmp(&commonHeader,
                                    &m_prev_commonHeader,
                                    sizeof m_prev_commonHeader);

   BOOL isDiffPhysicalHeader = memcmp(&physicalHeader,
                                      &m_prev_physicalHeader,
                                      sizeof m_prev_physicalHeader);

   BOOL isDiffTraceHeader = memcmp(&traceHeader,
                                   &m_prev_traceHeader,
                                   sizeof m_prev_traceHeader);

   BOOL isDiffMcmsTraceHeader = memcmp(&mcmsHeader,
                                       &m_prev_mcmsHeader,
                                       sizeof m_prev_mcmsHeader);

   return isDiffCommonHeader || isDiffPhysicalHeader ||
          isDiffTraceHeader || isDiffMcmsTraceHeader;
 }
