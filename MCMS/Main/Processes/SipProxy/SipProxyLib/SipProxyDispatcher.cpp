#include <algorithm>
#include "SipProxyDispatcher.h"
#include "ProcessBase.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"


extern const CSipProxyServiceManager*  GetServiceTask(DWORD serviceId);

CSipProxyDispatcherTask::CSipProxyDispatcherTask(BOOL isSync )
        :CDispatcherTask(isSync)
{
    m_Thread_Group = eTaskGroupRegular;
}

/////////////////////////////////////////////////////////////////////
CSipProxyDispatcherTask::~CSipProxyDispatcherTask()
{

}

BOOL CSipProxyDispatcherTask::TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode)
{
	
	//printf("CSipProxyDispatcherTask::TaskHandleEvent opcode %d\n", opCode);
	
	switch ( opCode ) 
	{
	  case CSAPI_MSG:
	  {
	   CSegment*  pMsgToSend = new CSegment(*pMsg);
	   CMplMcmsProtocol* mplMcmsProtocol = new CMplMcmsProtocol;
	   mplMcmsProtocol->DeSerialize(*pMsg, CS_API_TYPE);
	   CSipProxyMplMcmsProtocolTracer(*mplMcmsProtocol).TraceMplMcmsProtocol("***CSipProxyServiceManager::OnCSApiMsg",CS_API_TYPE);

	   OPCODE opcode = mplMcmsProtocol->getOpcode();
	   int data_len = mplMcmsProtocol->getDataLen();
	   DWORD serviceId = mplMcmsProtocol->getCentralSignalingHeaderCsId();
	
	   POBJDELETE(mplMcmsProtocol);

	   if(serviceId > 0)
	   {
	    	CSipProxyServiceManager*  servTask = (CSipProxyServiceManager*)(::GetServiceTask(serviceId-1));
	    	CTaskApi*  pTaskApi = new CTaskApi;
	    	pTaskApi->CreateOnlyApi(servTask->GetRcvMbx());
	    	pTaskApi->SendMsg(pMsgToSend, opcode);
	    	pTaskApi->DestroyOnlyApi();
	    	POBJDELETE(pTaskApi);
	   }
	   else
		   PTRACE(eLevelInfoNormal,"CSipProxyDispatcherTask::TaskHandleEvent : illegal serviceId");
	   break;
	  }
	  case  DESTROY : 
	  {
	  	  PTRACE(eLevelInfoNormal,"CSipProxyDispatcherTask::TaskHandleEvent : DESTROY");
	  	  return FALSE; //new carmel CTaskApp::HandleEvent will take care for DESTROY EVENT
	  }
	 
	}
    
    return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
void CSipProxyDispatcherTask::HandleOtherIdTypes(CMessageHeader & header)
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

				CSipProxyServiceManager*  servTask = (CSipProxyServiceManager*)(::GetServiceTask(header.m_addressee.m_id-1));
				if( header.m_opcode == CS_PROXY_IP_SERVICE_PARAM_IND )
				{
					PTRACE2INT(eLevelInfoNormal,"CSipProxyDispatcherTask::HandleOtherIdTypes: Activate service. Service ID ", header.m_addressee.m_id);
					servTask->SetIsServiceActive(TRUE);
				}
				else if ( header.m_opcode == CS_PROXY_DELETE_IP_SERVICE_IND )
					servTask->SetIsServiceActive(FALSE);
				else if ( header.m_opcode != SIPPROXY_MCUMNGR_CONFIGURATION_IND && header.m_opcode != CS_PROXY_CS_UP_IND
						&& header.m_opcode != MCUMNGR_TO_SIPPROXY_MULTIPLE_SERVICES_IND && header.m_opcode != SIPPROXY_CONFIG_CS_IND)
				   {
					 if( servTask->GetIsServiceActive() == FALSE )
					 {
						PTRACE2INT(eLevelInfoNormal,"CSipProxyDispatcherTask::HandleOtherIdTypes: Message for inactive service. Service ID ", header.m_addressee.m_id);
						PTRACE2INT(eLevelInfoNormal,"CSipProxyDispatcherTask::HandleOtherIdTypes: Message for inactive service. Opcode ", header.m_opcode);
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
				PTRACE2INT(eLevelInfoNormal, "CSipProxyDispatcherTask::HandleOtherIdTypes. Illegal Service id = ",header.m_addressee.m_id);

			break;
		}
		default:
		{			
			PTRACE(eLevelInfoNormal,"CSipProxyDispatcherTask::HandleOtherIdTypes : invalid m_idType");
			break;
		}
	}
}

