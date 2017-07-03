// CSPinger.cpp: implementation of the CSPinger class.
//
//
//Date         Updated By         Description
//
//9/6/09	  Lior Baram		Pings via CS 
//========   ==============   =====================================================================
#include "CsPinger.h"

//////////////////////////////////////////////////////////////////////////
//  class CPinger - state machine for the  ping
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CCsPinger)
    ONEVENT(CS_PING_IND    ,IDLE       , CCsPinger::HandlePingIndIdle )
    ONEVENT(CS_PING_IND    ,PINGING    , CCsPinger::HandlePingIndPinging )
    ONEVENT(PING_TIMER    ,PINGING    , CCsPinger::PingTimeout )
    ONEVENT(PING_EMA_QUERY_TIMER    ,PINGED    , CCsPinger::PingEmaQueryTimeout )
    
    
PEND_MESSAGE_MAP(CCsPinger, CStateMachine);
/////////////////////////////////////////////////////////////////////////////
CCsPinger::CCsPinger (CTaskApp* pOwnerTask)    // constructor	
	:CStateMachine(pOwnerTask)
{
    m_currentPingId = 0;
    m_state = IDLE;
    m_CSMngrMplMcmsProtocolTracer 	= new CCSMngrMplMcmsProtocolTracer;
    m_CommServiceService = new CCommServiceService;
	m_CommServiceService->SetMplMcmsProtocolTracer(m_CSMngrMplMcmsProtocolTracer);
    m_pCSMngrProcess = NULL;

}
/////////////////////////////////////////////////////////////////////////////
CCsPinger::~CCsPinger ()
{
    POBJDELETE(m_CommServiceService);
    POBJDELETE(m_CSMngrMplMcmsProtocolTracer);
    m_currentPingId = 0;
}

/////////////////////////////////////////////////////////////////////////////
void  CCsPinger::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);       
}
/////////////////////////////////////////////////////////////////////////////
void*  CCsPinger::GetMessageMap()                                        
{
  return (void*)m_msgEntries;    
}
/////////////////////////////////////////////////////////////////////////////
void CCsPinger::HandlePingIndIdle (CSegment* pParam)
{
    PTRACE (eLevelInfoNormal, "CCsPinger::HandlePingIndIdle - Ignore!");
    
}
/////////////////////////////////////////////////////////////////////////////
void CCsPinger::HandlePingIndPinging (CSegment* pParam)
{
    PTRACE (eLevelInfoNormal, "CCsPinger::HandlePingIndPinging");
    const CS_Ping_ind_S *pPingResponse = (CS_Ping_ind_S *)pParam->GetPtr();

    CPingData* currPingData = m_pCSMngrProcess->GetPing();
    m_pCSMngrProcess->GetPing()->Dump ( "CCsPinger::HandlePingIndPinging prev ping status", eLevelInfoNormal);
    
    if (!CPObject::IsValidPObjectPtr(currPingData))
        return;

    if (ePingStatus_ok == pPingResponse->pingStatus)		//succeeded to ping to one of the CS
    {
	    m_state = PINGED;		//one of the CS's answer the ping
        currPingData->SetPingStatus(STATUS_OK);
        //stop the timer
        if(IsValidTimer(PING_TIMER))
    		DeleteTimer(PING_TIMER);

        StartTimer (PING_EMA_QUERY_TIMER, 10 * SECOND);
    }
    else
    	PTRACE (eLevelInfoNormal, "CCsPinger::HandlePingIndPinging - ping from this CS failed");

    m_pCSMngrProcess->GetPing()->Dump ( "CCsPinger::HandlePingIndPinging current ping status", eLevelInfoNormal);

    //m_pCSMngrProcess->SetPing(currPingData, m_csId);
}
/////////////////////////////////////////////////////////////////////////////
STATUS CCsPinger::SetPing( CPingData * pNewPingData)
{
    STATUS returnStatus = STATUS_OK;
    if (!CPObject::IsValidPObjectPtr(pNewPingData))
    {
        PASSERTMSG (TRUE, "CCsPinger::SetPingIdle pNewPingData not valid!");
        return STATUS_FAIL;
    }

    CPingData * pCurrentPingData = m_pCSMngrProcess->GetPing();
    if (PINGING == m_state)
    {
        if (CPObject::IsValidPObjectPtr(pCurrentPingData))
            (m_pCSMngrProcess->GetPing())->Dump ( "CCsPinger::SetPing - failed! (state PINGING) already pinging the ping below", eLevelInfoNormal);
        else
            DBGPASSERT (m_state);
        returnStatus = STATUS_FAIL;
        pNewPingData->SetPingStatus(returnStatus);
    }
    else
    {
        if (PINGED == m_state)
        {
            if (NULL == pCurrentPingData)
            {
                //ping was deleted when queried, ok.
                PTRACE (eLevelInfoNormal, "CCsPinger::SetPing - state was PINGED but no current ping, assuming it was deleted and continue.");
                if(IsValidTimer(PING_EMA_QUERY_TIMER))
                    DeleteTimer(PING_EMA_QUERY_TIMER);
                returnStatus = STATUS_OK;
            }
            else
            {
                m_pCSMngrProcess->GetPing()->Dump ( "CCsPinger::SetPing - failed! (state PINGED) already pinging the ping below", eLevelInfoNormal);
                returnStatus = STATUS_FAIL;
                pNewPingData->SetPingStatus(returnStatus);
            }
        }
        if (STATUS_OK == returnStatus)
        {
            m_currentPingId ++;
            pNewPingData->SetPingId (m_currentPingId);
            m_state = PINGING;
            //start timer
            StartTimer (PING_TIMER, 50 * SECOND);
            
            CIPServiceList* list = m_pCSMngrProcess->GetIpServiceListStatic();
            if (list == NULL)
            {
            	PTRACE (eLevelInfoNormal, "CCsPinger::SetPing - Can't retrieve ip service list");
            	return returnStatus;
            }

			//Send Message to all the active cs's
			 for (CIPService* service = list->GetFirstService();
				 service != NULL; service = list->GetNextService())
			{
				m_CommServiceService->SetCsId(service->GetId());
				STATUS status = m_CommServiceService->SendPingToCs(pNewPingData->GetDestination(), pNewPingData->GetIpType());
				//Set the internal current ping object
				char msg[256];
				sprintf(msg, "CCsPinger::SetPing - ping was set to cs%d- pingData, status:%d", service->GetId(), status);

				pNewPingData->Dump (msg, eLevelInfoNormal);
			}

        }
    }
    return returnStatus;
}


/////////////////////////////////////////////////////////////////////////////
void CCsPinger::PingTimeout (CSegment* pParam)
{
    PTRACE (eLevelInfoNormal, "CCsPinger::PingTimeout");

    CPingData * currPingData = m_pCSMngrProcess->GetPing();
    if (CPObject::IsValidPObjectPtr(currPingData))
    {
        currPingData->Dump ("CCsPinger::PingTimeout -  m_currPingData", eLevelInfoNormal);
        currPingData->SetPingStatus (STATUS_FAIL);
        m_pCSMngrProcess->SetPing(currPingData);
    }
    if (m_state == PINGING)
        m_state = PINGED;

    StartTimer (PING_EMA_QUERY_TIMER, 10 * SECOND);
}
/////////////////////////////////////////////////////////////////////////////
void CCsPinger::PingEmaQueryTimeout(CSegment* pParam)
{
    CPingData * currPingData = m_pCSMngrProcess->GetPing();

    PTRACE (eLevelInfoNormal, "CCsPinger::PingEmaQueryTimeout");

    if (CPObject::IsValidPObjectPtr (currPingData))
        currPingData->Dump ("CCsPinger::PingEmaQueryTimeout - pingData", eLevelInfoNormal);
    else if (NULL == currPingData)
    {
        //ping was deleted when queried, ok
    }
    else
        PASSERTMSG (eLevelInfoNormal, "CCsPinger::PingEmaQueryTimeout - not valid current ping!!");
    
    m_state = IDLE;
    m_pCSMngrProcess->DeletePing();
}
/////////////////////////////////////////////////////////////////////////////

