#include <algorithm>
#include "GKDispatcherTask.h"
#include "ProcessBase.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"


extern const CGKServiceManager*  GetServiceTask(DWORD serviceId);

CGKDispatcherTask::CGKDispatcherTask(BOOL isSync )
        :CDispatcherTask(isSync)
{
    m_Thread_Group = eTaskGroupRegular;
}

/////////////////////////////////////////////////////////////////////
CGKDispatcherTask::~CGKDispatcherTask()
{

}

BOOL CGKDispatcherTask::TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode)
{
	//printf("CGKDispatcherTask::TaskHandleEvent opcode %d\n", opCode);
	switch ( opCode ) 
	{
	  case CSAPI_MSG:
	  {
	    CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	    pMplMcmsProtocol->DeSerialize(*pMsg, CS_API_TYPE);
	
	    CGkMplMcmsProtocolTracer(*pMplMcmsProtocol).TraceMplMcmsProtocol("***CGKDispatcherTask::OnCSApiMsg",CS_API_TYPE);	
	
	    OPCODE opcode    = pMplMcmsProtocol->getOpcode();
	    APIU32 connId    = pMplMcmsProtocol->getPortDescriptionHeaderConnection_id();
	    APIU32 partyId   = pMplMcmsProtocol->getPortDescriptionHeaderParty_id();
	    APIU32 serviceId = pMplMcmsProtocol->getCentralSignalingHeaderServiceId();
	    APIS32 status    = pMplMcmsProtocol->getCentralSignalingHeaderStatus();
	
	    CSegment* pMsgToSend = new CSegment;
	    pMsgToSend->Put(connId);
	    pMsgToSend->Put(partyId);
	    pMsgToSend->Put(serviceId);
	    pMsgToSend->Put((APIU32)status);
	    if(pMplMcmsProtocol->getDataLen())
		pMsgToSend->Put((unsigned char*)pMplMcmsProtocol->GetData(),pMplMcmsProtocol->getDataLen());
	    POBJDELETE(pMplMcmsProtocol);
	
	    if(serviceId > 0)
	    {
	    	CGKServiceManager*  servTask = (CGKServiceManager*)(::GetServiceTask(serviceId-1));
	    	CTaskApi*  pTaskApi = new CTaskApi;
	    	pTaskApi->CreateOnlyApi(servTask->GetRcvMbx());
	    	pTaskApi->SendMsg(pMsgToSend, opcode);
	    	pTaskApi->DestroyOnlyApi();
	    	POBJDELETE(pTaskApi);
	    }
	    else
	    	PTRACE(eLevelInfoNormal,"CGKDispatcherTask::TaskHandleEvent : illegal serviceId");
	    break;
	  }
	  case  DESTROY : 
	  {
	  	  PTRACE(eLevelInfoNormal,"CGKDispatcherTask::TaskHandleEvent : DESTROY");
	  	  return FALSE; //new carmel CTaskApp::HandleEvent will take care for DESTROY EVENT
	  }
	 
	}
    
    return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
void CGKDispatcherTask::HandleOtherIdTypes(CMessageHeader & header)
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

				CGKServiceManager*  servTask = (CGKServiceManager*)(::GetServiceTask(header.m_addressee.m_id-1));
				if( header.m_opcode == CS_GKMNGR_IP_SERVICE_PARAM_IND )
				{
					PTRACE2INT(eLevelInfoNormal,"CGKDispatcherTask::HandleOtherIdTypes: Activate service. Service ID ", header.m_addressee.m_id);
					servTask->SetIsServiceActive(TRUE);
				}
				else if ( header.m_opcode == CS_GKMNGR_DELETE_IP_SERVICE_IND )
					servTask->SetIsServiceActive(FALSE);
				else if ( header.m_opcode != MCUMNGR_GK_MNGMNT_IND && header.m_opcode != MCUMNGR_TO_GK_LICENSING_IND)
				   {
					 if( servTask->GetIsServiceActive() == FALSE )
					 {
						 PTRACE2INT(eLevelInfoNormal,"CGKDispatcherTask::HandleOtherIdTypes: Message for inactive service. Service ID ", header.m_addressee.m_id);
						 PTRACE2INT(eLevelInfoNormal,"CGKDispatcherTask::HandleOtherIdTypes: Message for inactive service. Opcode ", header.m_opcode);
						 POBJDELETE(pMsg1);//B.S. klocwork 1016
						 break;
					 }
				   }
				CTaskApi*  pTaskApi = new CTaskApi;
				pTaskApi->CreateOnlyApi(servTask->GetRcvMbx());
				pTaskApi->SendMsg(pMsg1, header.m_opcode);
				pTaskApi->DestroyOnlyApi();
				POBJDELETE(pTaskApi);
			}
			else if( header.m_opcode == CP_GK_CONF_ID_CLEAN_UP || header.m_opcode == CP_GK_PARTY_ID_CLEAN_UP)
			{
				for( int i = 0; i < MAX_SERVICES_NUM; i++ )
				{
					CGKServiceManager*  servTask = (CGKServiceManager*)(::GetServiceTask(i));
					if( servTask->GetIsServiceActive() == TRUE )
					{
						CSegment* pMsg1 = new CSegment;
						if(header.m_bufferLen)
							*pMsg1 << *(header.m_segment);
						CTaskApi*  pTaskApi = new CTaskApi;
						pTaskApi->CreateOnlyApi(servTask->GetRcvMbx());
						pTaskApi->SendMsg(pMsg1, header.m_opcode);
						pTaskApi->DestroyOnlyApi();
						POBJDELETE(pTaskApi);
					}
				}

			}
			else
		    	PTRACE(eLevelInfoNormal,"CGKDispatcherTask::HandleOtherIdTypes : illegal serviceId");

			break;
		}
		default:
		{			
			PTRACE(eLevelInfoNormal,"CGKDispatcherTask::HandleOtherIdTypes : invalid m_idType");
			break;
		}
	}
}

