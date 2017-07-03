#include <algorithm>
#include "IceDispatcher.h"
#include "ProcessBase.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"
#include "TraceStream.h"

extern const CIceServiceManager*  GetServiceTask(DWORD serviceId);

CIceDispatcherTask::CIceDispatcherTask(BOOL isSync )
        :CDispatcherTask(isSync)
{
    m_Thread_Group = eTaskGroupRegular;
}

/////////////////////////////////////////////////////////////////////
CIceDispatcherTask::~CIceDispatcherTask()
{

}

BOOL CIceDispatcherTask::TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode)
{
	PTRACE(eLevelInfoNormal,"CIceDispatcherTask::TaskHandleEvent ");
    return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CIceDispatcherTask::HandleOtherIdTypes(CMessageHeader & header)
{
	switch ( header.m_addressee.m_idType ) 
	{	
		case eServiceId:
		{	
			if( header.m_addressee.m_id>0 )
			{
				CSegment* pMsg1 = new CSegment;
				if(header.m_bufferLen)
					*pMsg1 << *(header.m_segment);

				CIceServiceManager*  servTask = (CIceServiceManager*)(::GetServiceTask(header.m_addressee.m_id-1));
				if(NULL == servTask)
				{
					TRACEINTO << "Service not found. ID:" << header.m_addressee.m_id-1;
					break;
				}
				if ( header.m_opcode == CS_PROXY_DELETE_IP_SERVICE_IND )
					servTask->SetIsServiceActive(FALSE);
				else if ( header.m_opcode != SIPPROXY_MCUMNGR_CONFIGURATION_IND && header.m_opcode != CS_PROXY_CS_UP_IND
						&& header.m_opcode != MCUMNGR_TO_SIPPROXY_MULTIPLE_SERVICES_IND && header.m_opcode != SIPPROXY_CONFIG_CS_IND)
				   {
					 if( servTask->GetIsServiceActive() == FALSE )
					 {
						PTRACE2INT(eLevelInfoNormal,"CIceDispatcherTask::HandleOtherIdTypes: Message for inactive service. Service ID ", header.m_addressee.m_id);
						PTRACE2INT(eLevelInfoNormal,"CIceDispatcherTask::HandleOtherIdTypes: Message for inactive service. Opcode ", header.m_opcode);
						/*Need to free the pMsg1*/
						POBJDELETE(pMsg1);
						break;
					 }
				   }
				CTaskApi*  pTaskApi = new CTaskApi;
				pTaskApi->CreateOnlyApi(servTask->GetRcvMbx());
				pTaskApi->SendMsg(pMsg1, header.m_opcode);
				pTaskApi->DestroyOnlyApi();
				POBJDELETE(pTaskApi);
			}
			else
				PTRACE2INT(eLevelInfoNormal, "CIceDispatcherTask::HandleOtherIdTypes. Illegal Service id = ",header.m_addressee.m_id);

			break;
		}
		default:
		{			
			PTRACE(eLevelInfoNormal,"CIceDispatcherTask::HandleOtherIdTypes : invalid m_idType");
			break;
		}
	}
}

