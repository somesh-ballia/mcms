/*
 * FileLogger.h
 *
 *  Created on: May 30, 2012
 *      Author: bg
 */

#ifndef FILELOGGER_H_
#define FILELOGGER_H_

#include <string>
#include <fstream>

#include "LoggerDefines.h"
#include "TraceClass.h"

#define FILE_LOGGER (*CFileLogger::GetInstance())
//#define FILE_LOG(s) (CFileLogger::GetInstance()->Log(s))
#define FILE_LOG(s) (CFileLogger::GetInstance()->Log(s, __FILE__, __LINE__))

class CFileLogger
{
private:
	CFileLogger();
	static CFileLogger* m_pInstance;

public:
	void LogRaw(const std::string& s);
	static CFileLogger* GetInstance();
	void Log(const std::string& s, bool bEndLine = true);
	void Log(const std::string& s, const char* sFileName, int iLine);

    static void OutMessage(const TRACE_HEADER_S& traceHeader, const MCMS_TRACE_HEADER_S& mcmsHeader, const std::string& content);
    static void FillBuffer(std::ostringstream& buf,
                                    const TRACE_HEADER_S& traceHeader,
                                    const MCMS_TRACE_HEADER_S& mcmsHeader,
                                    const char* content);

protected:
	std::string GetTime();
	std::string GetFormattedTime();

	std::string m_sFileName;
	std::fstream m_fileStream;

private:
	static const char* GetProcessNameByMainEntity(eMainEntities mainEntityType, DWORD processType);

	static COMMON_HEADER_S			m_prev_commonHeader;
	static PHYSICAL_INFO_HEADER_S	m_prev_physicalHeader;
	static TRACE_HEADER_S			m_prev_traceHeader;
	static MCMS_TRACE_HEADER_S		m_prev_mcmsHeader;
	static std::string				m_title;

};

#endif /* FILELOGGER_H_ */
