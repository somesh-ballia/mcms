// EndpointsSimProcess.cpp: implementation of the CEndpointsSimProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "EndpointsSimProcess.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StringsMaps.h"
#include "EndpointsSim.h"
#include "EpSimCapSetsList.h"

extern void EndpointsSimManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CEndpointsSimProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CEndpointsSimProcess::GetManagerEntryPoint()
{
	return EndpointsSimManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEndpointsSimProcess::CEndpointsSimProcess()
{ }

CEndpointsSimProcess::~CEndpointsSimProcess()
{
    PASSERTMSG(!m_csTasksQueue.empty(), "Some child tasks did not unregister");
}


//////////////////////////////////////////////////////////////////////
void CEndpointsSimProcess::AddExtraStringsToMap()
{
	CStringsMaps::AddItem(DTMF_SOURCE_TYPE_ENUM,eDtmfSourceAUDIO,"dtmf_source_audio");
	CStringsMaps::AddItem(DTMF_SOURCE_TYPE_ENUM,eDtmfSourceRTP,"dtmf_source_rtp");
	CStringsMaps::AddItem(DTMF_SOURCE_TYPE_ENUM,eDtmfSourceSIGNALLING,"dtmf_source_signalling");

	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoModeCif,"cif");
	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoMode2Cif30,"2cif30");
	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoModeSD15,"sd15");
	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoModeSD30,"sd30");
	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoModeW4CIF30,"w4cif30");
	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoModeHD720,"hd720");
	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoModeHD1080,"hd1080");
	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoModeSD60,"sd60");
	CStringsMaps::AddItem(VIDEO_MODE_H264_ENUM,eVideoModeSD7_5,"sd7_5");

	CStringsMaps::AddItem(SIM_RECAP_MODE_ENUM, 0,"none");
	CStringsMaps::AddItem(SIM_RECAP_MODE_ENUM, 1,"before");
	CStringsMaps::AddItem(SIM_RECAP_MODE_ENUM, 2,"after");

	CStringsMaps::AddMinMaxItem(SIM_VIDEO_H264_ASPECT_RATIO_LIMITS,-1,0xFFFF);
}

const char* EPSimTaskTypeToStr(eEPSimTaskType type)
{
    static const char* names[] =
    {
        "EPSimUnknownTask",
        "CSSimTask",
        "ProxySimTask",
        "GKSimTask"
    };

    return (type > 0 && (unsigned int)type < sizeof names / sizeof *names)
            ?
            names[type] : names[0];
}

void CEndpointsSimProcess::RegisterQueue(
        DWORD csID, eEPSimTaskType type, const COsQueue& queue)
{
    COsQueue val = RetrieveQueue(csID, type);
    PASSERTSTREAM_AND_RETURN(
        val.IsValid(),
        "The queue "
            << EPSimTaskTypeToStr(type)
            << "["
            << csID
            << "] already exists");

    LockSemaphore(m_TasksSemaphoreId);
    m_csTasksQueue[std::make_pair(csID, type)] = queue;
    UnlockSemaphore(m_TasksSemaphoreId);
}

void CEndpointsSimProcess::UnregisterQueue(DWORD csID, eEPSimTaskType type)
{
    COsQueue val = RetrieveQueue(csID, type);
    PASSERTSTREAM_AND_RETURN(
        !val.IsValid(),
        "The queue "
            << EPSimTaskTypeToStr(type)
            << "["
            << csID
            << "] is not exist");

    LockSemaphore(m_TasksSemaphoreId);
    m_csTasksQueue.erase(std::make_pair(csID, type));
    UnlockSemaphore(m_TasksSemaphoreId);
}

/*
 * TODO: instead of ineffective LockSemaphore/UnlockSemaphore should be
 * implemented Read Write Lock pattern
 */
COsQueue CEndpointsSimProcess::RetrieveQueue(DWORD csID,
                                             eEPSimTaskType type) const
{
    COsQueue val;
    std::pair<DWORD, eEPSimTaskType> key(csID, type);
    std::map<std::pair<DWORD, eEPSimTaskType>, COsQueue>::const_iterator it;

    LockSemaphore(m_TasksSemaphoreId);

    it = m_csTasksQueue.find(key);
    if(it != m_csTasksQueue.end())
        val = it->second;

    UnlockSemaphore(m_TasksSemaphoreId);

    return val;
}
