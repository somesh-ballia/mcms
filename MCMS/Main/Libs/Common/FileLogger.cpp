/*
 * FileLogger.cpp
 *
 *  Created on: May 30, 2012
 *      Author: bg
 */

#include "ObjString.h"
#include "StructTm.h"
#include "SystemFunctions.h"
#include "ProcessBase.h"

#include "FileLogger.h"
#include "StlUtils.h"
#include "DataTypes.h"
#include <sys/time.h>
#include <iostream>
#include <pthread.h>

static const char END_HEADER_CRNL[] = {'\r', '\n', '\0'};
static const char END_TRACE_CRNL [] = {'\r', '\n', '\r', '\n', '\0'};

CFileLogger* CFileLogger::m_pInstance = NULL;

COMMON_HEADER_S			CFileLogger::m_prev_commonHeader;
PHYSICAL_INFO_HEADER_S	CFileLogger::m_prev_physicalHeader;
TRACE_HEADER_S			CFileLogger::m_prev_traceHeader;
MCMS_TRACE_HEADER_S		CFileLogger::m_prev_mcmsHeader;
std::string				CFileLogger::m_title;

CFileLogger::CFileLogger()
{
	eProcessType ePT = CProcessBase::GetProcess()->GetProcessType();
	const char* proc_name = GetProcessNameByMainEntity(eMcms, ePT);
	m_sFileName = MCU_TMP_DIR+"/Log_";
	m_sFileName += proc_name;
	m_sFileName += GetFormattedTime();
	m_sFileName += ".log";
//	m_fileStream.open (m_sFileName.c_str(), std::fstream::out | std::fstream::trunc);
//	m_fileStream.close();
}

CFileLogger* CFileLogger::GetInstance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new CFileLogger;
	}
	return m_pInstance;
}

void CFileLogger::LogRaw(const std::string& s)
{
	static pthread_mutex_t cs_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock( &cs_mutex );
	m_fileStream.open (m_sFileName.c_str(), std::fstream::out | std::fstream::app);
	m_fileStream << s;
	m_fileStream.close();
	pthread_mutex_unlock( &cs_mutex );
}

void CFileLogger::Log(const std::string& s, bool bEndLine /*= true*/)
{
	static pthread_mutex_t cs_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock( &cs_mutex );
	m_fileStream.open (m_sFileName.c_str(), std::fstream::out | std::fstream::app);
	m_fileStream << GetTime() <<  " " << s;
	if (bEndLine)
		m_fileStream << std::endl;
	m_fileStream.close();
	pthread_mutex_unlock( &cs_mutex );
}

void CFileLogger::Log(const std::string& s, const char* pszFileName, int iLine)
{
	std::string sFileName(pszFileName);
	size_t i = sFileName.rfind('/');
	if (i < sFileName.size())
		sFileName = sFileName.substr(i+1);
	m_fileStream.open (m_sFileName.c_str(), std::fstream::out | std::fstream::app);
	m_fileStream << GetFormattedTime() <<  " Lctn:" << sFileName.c_str() <<  "(" << iLine << ")" <<  std::endl << s << std::endl;
	m_fileStream.close();
}

std::string CFileLogger::GetTime()
{
	timeval tv;
	struct tm * timeinfo;
	char buffer [80];

	gettimeofday(&tv, NULL);
	timeinfo = localtime ( &(tv.tv_sec) );
	if(timeinfo)
	{
		strftime (buffer, 80, "D:%d/%m/%y-%X.", timeinfo);

		return std::string(buffer) + CStlUtils::ValueToString(tv.tv_usec);
	}
	return "";
}

std::string CFileLogger::GetFormattedTime()
{
	timeval tv;
	struct tm * timeinfo;
	char buffer [80];

	gettimeofday(&tv, NULL);
	timeinfo = localtime ( &(tv.tv_sec) );
	if(timeinfo)
		strftime (buffer, 80, "%Y_%m_%d-%H-%M-%S.", timeinfo);
	else
		FPASSERTMSG(1, "localtime return NULL"); 

	return std::string(buffer) + CStlUtils::ValueToString(tv.tv_usec);
}

void CFileLogger::OutMessage(const TRACE_HEADER_S& traceHeader, const MCMS_TRACE_HEADER_S& mcmsHeader, const std::string& content)
{
    std::ostringstream buff;
    FillBuffer(buff, traceHeader, mcmsHeader, content.c_str());
    FILE_LOGGER.Log(buff.str(), false);
}

void CFileLogger::FillBuffer(std::ostringstream& buf,
                                const TRACE_HEADER_S& traceHeader,
                                const MCMS_TRACE_HEADER_S& mcmsHeader,
                                const char* content)
{
  DWORD messageLen = (NULL == content) ? 0 : strlen(content);

  buf << " " << TITLE_MainEntity << ":" << "Mcms";

	const char* proc_name = GetProcessNameByMainEntity(eMcms, traceHeader.m_processType);
	if ('\0' != traceHeader.m_taskName)
		buf << " " << TITLE_ProcessName << ":" << proc_name;

	buf << " " << TITLE_SourceId << ":" << traceHeader.m_sourceId;

  if ('\0' != traceHeader.m_taskName[0])
    buf << " " << TITLE_TaskName << ":" << traceHeader.m_taskName;

  if ('\0' != traceHeader.m_objectName)
    buf << " " << TITLE_ObjectName << ":" << traceHeader.m_objectName;

  // Additional logger header
  if (DEFAULT_TOPIC_ID != traceHeader.m_topic_id)
    buf << " " << TITLE_TopicID << ":" << traceHeader.m_topic_id;

  if (DEFAULT_UNIT_ID != traceHeader.m_unit_id)
    buf << " " << TITLE_UnitID << ":" << traceHeader.m_unit_id;

  if (DEFAULT_CONF_ID != traceHeader.m_conf_id)
    buf << " " << TITLE_ConfID << ":" << traceHeader.m_conf_id;

  if (DEFAULT_PARTY_ID != traceHeader.m_party_id)
    buf << " " << TITLE_PartyID << ":" << traceHeader.m_party_id;

  if (DEFAULT_OPCODE != traceHeader.m_opcode)
    buf << " " << TITLE_Opcode << ":" << traceHeader.m_opcode;

  if ('\0' != traceHeader.m_str_opcode[0])
    buf << " " << TITLE_StrOpcode << ":" << traceHeader.m_str_opcode;

  if ('\0' != mcmsHeader.m_file_line_number[0])
    buf << " " << TITLE_FileLine << ":" << mcmsHeader.m_file_line_number;

  buf << END_HEADER_CRNL << ((NULL == content) ? "empty" : content) << END_TRACE_CRNL;
}

const char* CFileLogger::GetProcessNameByMainEntity(eMainEntities mainEntityType,
                                                       DWORD processType)
{
	static const char *sarrMplProcessNames[] =
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

	eProcessType type = (eProcessType)processType;
	switch (mainEntityType)
	{
		case eMcms:
			return CProcessBase::GetProcessName((eProcessType) processType);

		case eCentral_signaling:
			return  "NA";

		case eEma:
			return "EmaProcess";

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
		return (type >= 0 && (unsigned int)type < ARRAYSIZE(sarrMplProcessNames)) ? sarrMplProcessNames[type] : "InvalideProcess";

		default:
		return "Invalid";
	}

	return "Invalid";
}
