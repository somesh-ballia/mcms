#include "PipeToLogger.h"

#include <fcntl.h>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "TraceStream.h"
#include "Segment.h"

////////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////////
void pipeToLoggerEntryPoint(void* appParam)
{  
	CPipeToLogger* pipToLoggerTask = new CPipeToLogger;
	pipToLoggerTask->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////////
// static
std::string CPipeToLogger::m_pipe_fname = MCU_TMP_DIR+"/queue/LoggerPipe";

////////////////////////////////////////////////////////////////////////////////
CPipeToLogger::CPipeToLogger(void) : m_pipe_file_descriptor (-1)
{ }

////////////////////////////////////////////////////////////////////////////////
CPipeToLogger::~CPipeToLogger(void)
{
    if (m_pipe_file_descriptor == -1)
        return;

    int stat = close(m_pipe_file_descriptor);
    PASSERTSTREAM(-1 == stat,
        "Unable to close file " << m_pipe_fname.c_str() << ": " << strerror(errno));
}

////////////////////////////////////////////////////////////////////////////////
void CPipeToLogger::InitTask(void)
{
    m_pipe_file_descriptor = open(m_pipe_fname.c_str(), O_RDONLY | O_NDELAY);

    PASSERTSTREAM(-1 == m_pipe_file_descriptor,
        "open: " << m_pipe_fname.c_str() << ": " << strerror(errno));
}

////////////////////////////////////////////////////////////////////////////////
int CPipeToLogger::GetPrivateFileDescriptor()
{
    return m_pipe_file_descriptor;
}

////////////////////////////////////////////////////////////////////////////////
void CPipeToLogger::HandlePrivateFileDescriptor()
{
    char line[1025];

    ssize_t size = read(m_pipe_file_descriptor,
                        line,
                        1024);
    if (size > 0)
    {
        line[size] = 0;

        if(strstr(line, "iptables:") != NULL)
        	TRACEINTO << "NIDSErrorLog: " << line;
        else
        	TRACEINTO << "ApacheErrorLog: " << line;
    }

    if (-1 == size)
    {
        TRACESTR(eLevelError) << "read: " << m_pipe_fname.c_str()
                                 << ": " << strerror(errno);
    }

    if (size == 0)
    {

        int stat = close(m_pipe_file_descriptor);
        PASSERTSTREAM(-1 == stat,
            "Unable to close file " << m_pipe_fname.c_str() << ": " << strerror(errno));

        m_pipe_file_descriptor = -1;
        InitTask();
    }

}

////////////////////////////////////////////////////////////////////////////////
// virtual
const char* CPipeToLogger::NameOf(void) const
{
    return GetCompileType();
}

////////////////////////////////////////////////////////////////////////////////
const char* CPipeToLogger::GetTaskName(void) const
{
    return "PipeToLogger";
}

////////////////////////////////////////////////////////////////////////////////
unsigned char CPipeToLogger::IsSingleton(void) const
{
    return FALSE;
}
