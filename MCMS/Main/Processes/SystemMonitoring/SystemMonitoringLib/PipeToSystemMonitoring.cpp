#include "PipeToSystemMonitoring.h"

#include <fcntl.h>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "TraceStream.h"
#include "Segment.h"
#include "McuMngrInternalStructs.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"

#include "802_1xApiReq.h"

#include "802_1xApiInd.h"


////////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////////
void pipeToSystemMonitoringEntryPoint(void* appParam)
{
	CPipeToSystemMonitoring* pipToSystemMonitoringTask = new CPipeToSystemMonitoring;
	pipToSystemMonitoringTask->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////////
// static
std::string CPipeToSystemMonitoring::m_pipe_fname = MCU_TMP_DIR+"/queue/SystemMonitoringPipe";

////////////////////////////////////////////////////////////////////////////////
CPipeToSystemMonitoring::CPipeToSystemMonitoring(void) : m_pipe_file_descriptor (-1)
{ }

////////////////////////////////////////////////////////////////////////////////
CPipeToSystemMonitoring::~CPipeToSystemMonitoring(void)
{
    if (m_pipe_file_descriptor == -1)
        return;

    int stat = close(m_pipe_file_descriptor);
    PASSERTSTREAM(-1 == stat,
        "Unable to close file " << m_pipe_fname.c_str() << ": " << strerror(errno));
}

////////////////////////////////////////////////////////////////////////////////
void CPipeToSystemMonitoring::InitTask(void)
{
    m_pipe_file_descriptor = open(m_pipe_fname.c_str(), O_RDONLY | O_NDELAY);

    PASSERTSTREAM(-1 == m_pipe_file_descriptor,
        "open: " << m_pipe_fname.c_str() << ": " << strerror(errno));
}

////////////////////////////////////////////////////////////////////////////////
int CPipeToSystemMonitoring::GetPrivateFileDescriptor()
{
    return m_pipe_file_descriptor;
}

////////////////////////////////////////////////////////////////////////////////
void CPipeToSystemMonitoring::HandlePrivateFileDescriptor()
{
    char line[1025];
    memset(line,0,1025);
    std::string ethName;
    std::string ethStatus;

    TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - START";

    ssize_t size = read(m_pipe_file_descriptor,
                        line,
                        1024);

    TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - got data " << line;

    if (size > 0)
    {
    	line[size] = 0;

     	char * pch;

     	pch = strtok (line," ");
     	if (pch != NULL)
     	{

     		TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - arrived data first token " << pch;
     		ethName = pch;
     		TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - eth name " << ethName;

     		pch = strtok (NULL, " ");
     		if (pch != NULL)
     		{
     			TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - arrived data second token " << pch;
     			ethStatus = pch;
     			TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - eth status " << ethStatus;

     		}
     		else
     		{
     			TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - missing second parameter ";

     			return;
     		}
     	}
     	else
     	{
     		TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - missing parameters ";

     		return;
     	}

     	s802_1x_CONNECTION_STATUS_UNSOLICITED_IND  connection_status_event;

    	//first we convert the state argv[1]
    		if(strncmp(ethName.c_str(), P802_1x_STR_ETH0, strlen(P802_1x_STR_ETH0)) == 0) {
    			connection_status_event.ulNicId = E_802_1x_ETH0;
    		}
    		else if(strncmp(ethName.c_str(), P802_1x_STR_ETH1, strlen(P802_1x_STR_ETH1)) == 0) {
    			connection_status_event.ulNicId  = E_802_1x_ETH1;
    		}
    		else if(strncmp(ethName.c_str(), P802_1x_STR_ETH2, strlen(P802_1x_STR_ETH2)) == 0) {
    			connection_status_event.ulNicId = E_802_1x_ETH2;
    		}
    		else if(strncmp(ethName.c_str(), P802_1x_STR_ETH3, strlen(P802_1x_STR_ETH3)) == 0) {
    			connection_status_event.ulNicId  = E_802_1x_ETH3;
    		}
    		else {
    			TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - eth name invalid " ;

    			return;
    		}

    		//now we work the vent type
    			if(strncmp(ethStatus.c_str(), P802_1x_STR_CONNECTED, strlen(P802_1x_STR_CONNECTED)) == 0) {
    				connection_status_event.eConnStatus = E_802_1x_STATE_CONNECTING;
    			}
    			else if(strncmp(ethStatus.c_str(), P802_1x_STR_DISCONNECTED, strlen(P802_1x_STR_DISCONNECTED)) == 0) {
    				connection_status_event.eConnStatus = E_802_1x_STATE_DISCONNECTED;
    			}
    			else {
    				TRACESTR(eLevelInfoNormal) << "\nCPipeToSystemMonitoring::HandlePrivateFileDescriptor - eth status invalid " << ethStatus;
    				return;
    			}



    			// ===== 3. send to McuMngr process
    			    CSegment* pSeg = new CSegment;
    			    pSeg->Put((BYTE*)&connection_status_event, sizeof(s802_1x_CONNECTION_STATUS_UNSOLICITED_IND) );



    				CManagerApi api(eProcessMcuMngr);
    				api.SendMsg(pSeg, OpInt802_1x_CONNECTION_STATUS_CHANGE_EVENT);



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
const char* CPipeToSystemMonitoring::NameOf(void) const
{
    return GetCompileType();
}

////////////////////////////////////////////////////////////////////////////////
const char* CPipeToSystemMonitoring::GetTaskName(void) const
{
    return "PipeToSystemMonitoring";
}

////////////////////////////////////////////////////////////////////////////////
unsigned char CPipeToSystemMonitoring::IsSingleton(void) const
{
    return FALSE;
}
