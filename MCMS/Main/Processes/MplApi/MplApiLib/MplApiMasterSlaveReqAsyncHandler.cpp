/*
 * MplApiMasterSlaveReqAsyncHandler.cpp
 *
 *  Created on: Nov 13, 2011
 *      Author: mhalfon
 */


#include "MplApiMasterSlaveReqAsyncHandler.h"
#include "TraceStream.h"
#include "MplApiOpcodes.h"
#include "ProcessBase.h"


//=========================================================================================================================================================================================//
CMplApiMasterSlaveReqAsyncHandler::CMplApiMasterSlaveReqAsyncHandler():m_inUse(0)
{
}
//=========================================================================================================================================================================================//
CMplApiMasterSlaveReqAsyncHandler::~CMplApiMasterSlaveReqAsyncHandler()
{
	map<DWORD, SMasterSlaveRequestData*>::iterator indexedItr = m_masterSlaveRequestIdToSubIdMap.begin();
	map<DWORD, SMasterSlaveRequestData*>::iterator endItr = m_masterSlaveRequestIdToSubIdMap.end();
	while(indexedItr != endItr)
	{
		delete (*indexedItr).second;
		indexedItr++;
	}
	m_masterSlaveRequestIdToSubIdMap.clear();
	m_masterSlaveRequestSubIdToIdMap.clear();

}
//=========================================================================================================================================================================================//
void CMplApiMasterSlaveReqAsyncHandler::onCleanMasterSlaveRequestTout()
{

  PTRACE(eLevelInfoNormal,"1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::onCleanMasterSlaveRequestTout start:");
  traceMasterSlaveRequestHandler();

  DWORD timeOutHandlerId = 0;
  while( GetTimeOutHandlerId(timeOutHandlerId)){
    deleteMasterSlaveHandlerAccordingToReqId(timeOutHandlerId);
  }

  PTRACE(eLevelInfoNormal,"1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::onCleanMasterSlaveRequestTout end:");
  traceMasterSlaveRequestHandler();


  /*
	SMasterSlaveRequestData* pMasterSlaveReqIdData = NULL;
	time_t currentTime;
	WORD delete_inteval_seconds = 20;

	CLargeString buff;
	buff << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::onCleanMasterSlaveRequestTout" << '\n';
	buff << "m_masterSlaveRequestIdToSubIdMap.size(): " << m_masterSlaveRequestIdToSubIdMap.size() <<'\n';


	// gets current time in seconds since ...1970
	GetCurrentTime(currentTime);


	

	map<DWORD, SMasterSlaveRequestData*>::iterator indexedItr = m_masterSlaveRequestIdToSubIdMap.begin();
	map<DWORD, SMasterSlaveRequestData*>::iterator endItr = m_masterSlaveRequestIdToSubIdMap.end();

	if(0 == m_masterSlaveRequestIdToSubIdMap.size()){
	  PTRACE(eLevelInfoNormal,"1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::onCleanMasterSlaveRequestTout , m_masterSlaveRequestIdToSubIdMap is empty - do nothing");
	  if(indexedItr != endItr){
	    PTRACE(eLevelInfoNormal,"1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::onCleanMasterSlaveRequestTout indexedItr!=endItr when  m_masterSlaveRequestIdToSubIdMap is empty");	    
	  }
	  return;
	}

	while( 0<m_masterSlaveRequestIdToSubIdMap.size() && indexedItr != endItr)
	{
		DWORD reqId = (*indexedItr).first;
		pMasterSlaveReqIdData = (*indexedItr).second;

		if(pMasterSlaveReqIdData != NULL)
		{
			if(pMasterSlaveReqIdData->m_initiateTime < currentTime - delete_inteval_seconds )
			{
				
				buff << "***** TIMOUT FOR REQUEST delete handlerId:" << reqId << '\n';
				deleteMasterSlaveHandlerAccordingToReqId(reqId);
			}
		}
		indexedItr++;

	}
	PTRACE(eLevelInfoNormal,buff.GetString());

	traceMasterSlaveRequestHandler();

  */
}
//=========================================================================================================================================================================================//
bool CMplApiMasterSlaveReqAsyncHandler::GetTimeOutHandlerId(DWORD& handlerId)
{

  if(0 ==m_masterSlaveRequestIdToSubIdMap.size() ){
    PTRACE(eLevelInfoNormal,"1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::GetTimeOutHandlerId , m_masterSlaveRequestIdToSubIdMap is empty");
    return false;
  }

  //  traceMasterSlaveRequestHandler();

  SMasterSlaveRequestData* pMasterSlaveReqIdData = NULL;
  time_t currentTime;
  WORD delete_inteval_seconds = 20;

  // gets current time in seconds since ...1970
  GetCurrentTime(currentTime);
	

  map<DWORD, SMasterSlaveRequestData*>::iterator indexedItr = m_masterSlaveRequestIdToSubIdMap.begin();
  map<DWORD, SMasterSlaveRequestData*>::iterator endItr = m_masterSlaveRequestIdToSubIdMap.end();

  while( indexedItr != endItr)
    {
      DWORD reqId = (*indexedItr).first;
      pMasterSlaveReqIdData = (*indexedItr).second;

      if(pMasterSlaveReqIdData != NULL)
	{
	  if(pMasterSlaveReqIdData->m_initiateTime < currentTime - delete_inteval_seconds )
	    {
	      PTRACE2INT(eLevelInfoNormal,"1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::GetTimeOutHandlerId - found TIMOUT FOR REQUEST found handlerId:", reqId);
	      handlerId = reqId;
	      return true;
	    }
	}
      indexedItr++;

    }
  PTRACE(eLevelInfoNormal,"1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::GetTimeOutHandlerId , not found timeout for request");  
  return false;
}
//=========================================================================================================================================================================================//
void CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveRequestHandlerSubIdData(DWORD opcode, DWORD reqId, DWORD subReqId)
{
  SMasterSlaveRequestData* pMasterSlaveReqIdData = NULL;
  time_t currentTime;

  CMedString buff;
  buff << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveRequestHandlerSubIdData reqId: " << reqId << " subReqId: " << subReqId << '\n';
  PTRACE(eLevelInfoNormal,buff.GetString());

  m_inUse = TRUE;
	
  if(m_masterSlaveRequestIdToSubIdMap.find(reqId) != m_masterSlaveRequestIdToSubIdMap.end())
    { // second sub req id for the req
      pMasterSlaveReqIdData = m_masterSlaveRequestIdToSubIdMap[reqId];
    }
  else // first sub req id for the req
    {
      pMasterSlaveReqIdData = new SMasterSlaveRequestData;
      GetCurrentTime(currentTime); // in seconds
      // enter time and opcode
      pMasterSlaveReqIdData->m_initiateTime = currentTime;
      pMasterSlaveReqIdData->m_opcode = opcode;
    }

  pMasterSlaveReqIdData->m_subIdsAckStatusMap[subReqId] = FALSE; // add the subReqId , FALSE = ack not received yet
  m_masterSlaveRequestSubIdToIdMap[subReqId] = reqId; // subReqId to reqId
  m_masterSlaveRequestIdToSubIdMap[reqId] = pMasterSlaveReqIdData;

  traceMasterSlaveRequestHandler();
}
//=========================================================================================================================================================================================//
// received ack for subReqId
BOOL CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveAckToHandlerSubId(DWORD subReqId, BOOL &isFinishedHandler)
{

  DWORD handlerId = 0;
	SMasterSlaveRequestData* pMasterSlaveReqIdData = NULL;
	isFinishedHandler = FALSE;

	CMedString buff;
	buff << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveAckToHandlerSubId subReqId: " << subReqId << '\n';
	PTRACE(eLevelInfoNormal,buff.GetString());

	CMedString buff2;
	if(m_masterSlaveRequestSubIdToIdMap.find(subReqId) == m_masterSlaveRequestSubIdToIdMap.end())
	{
	        buff2 << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveAckToHandlerSubId SubIdToId not found, return FALSE, subReqId: " << subReqId << '\n';
		PTRACE(eLevelInfoNormal,buff2.GetString());

		PTRACE(eLevelInfoNormal,"CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveAckToHandlerSubId - return TRUE for debug");
		isFinishedHandler = 1;
		return TRUE;

		//return FALSE;
	}

	handlerId = m_masterSlaveRequestSubIdToIdMap[subReqId];

	if(m_masterSlaveRequestIdToSubIdMap.find(handlerId) == m_masterSlaveRequestIdToSubIdMap.end())
	{
	  buff2 << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveAckToHandlerSubId handlerId not found, return FALSE, subReqId: " << subReqId << " , handlerId: " << handlerId <<'\n';
	  PTRACE(eLevelInfoNormal,buff2.GetString());
	  return FALSE;
	}

	pMasterSlaveReqIdData = m_masterSlaveRequestIdToSubIdMap[handlerId];
	if(pMasterSlaveReqIdData != NULL)
	{
		if(pMasterSlaveReqIdData->m_subIdsAckStatusMap.find(subReqId) == pMasterSlaveReqIdData->m_subIdsAckStatusMap.end())
		{
		        buff2 << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveAckToHandlerSubId subReqId not found, return FALSE, subReqId: " << subReqId << " , handlerId: " << handlerId <<'\n';
			PTRACE(eLevelInfoNormal,buff2.GetString());
			return FALSE;
		}
		pMasterSlaveReqIdData->m_subIdsAckStatusMap[subReqId] = TRUE;
		isFinishedHandler = isMasterSlaveRequestFinished(pMasterSlaveReqIdData);
		traceMasterSlaveRequestHandler();
		return TRUE;
	}
	buff2 << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::setMasterSlaveAckToHandlerSubId ERROR: can not find subReqId: " << subReqId << '\n';
	PTRACE(eLevelInfoNormal,buff2.GetString());
	return FALSE;
}
//=========================================================================================================================================================================================//
BOOL CMplApiMasterSlaveReqAsyncHandler::deleteMasterSlaveHandlerAccordingToReqId(DWORD handlerId)
{
	SMasterSlaveRequestData* pMasterSlaveReqIdData = NULL;
	BOOL isDeleted = FALSE;

	CMedString buff;
	buff << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::deleteMasterSlaveHandlerAccordingToReqId handlerId: " << handlerId << '\n';

	if(m_masterSlaveRequestIdToSubIdMap.find(handlerId) != m_masterSlaveRequestIdToSubIdMap.end())
	{
		map<DWORD, SMasterSlaveRequestData*>::iterator findItr = m_masterSlaveRequestIdToSubIdMap.find(handlerId);
		if(findItr != m_masterSlaveRequestIdToSubIdMap.end())
		{
			pMasterSlaveReqIdData = (*findItr).second;
			if(pMasterSlaveReqIdData != NULL)
			{
				map<DWORD, BOOL>::iterator indexedItr = pMasterSlaveReqIdData->m_subIdsAckStatusMap.begin();
				map<DWORD, BOOL>::iterator endItr = pMasterSlaveReqIdData->m_subIdsAckStatusMap.end();
				while(indexedItr != endItr)
				{
					m_masterSlaveRequestSubIdToIdMap.erase((*indexedItr).first);
					indexedItr++;
				}
			}
			delete pMasterSlaveReqIdData;
			m_masterSlaveRequestIdToSubIdMap.erase(findItr);


			traceMasterSlaveRequestHandler();
			isDeleted = TRUE;
		}
	}

	PTRACE(eLevelInfoNormal,buff.GetString());
	return isDeleted;

}
//=========================================================================================================================================================================================//
BOOL CMplApiMasterSlaveReqAsyncHandler::deleteMasterSlaveHandlerAccordingToSubId(DWORD subReqId)
{

	DWORD handlerId = 0;
	SMasterSlaveRequestData* pMasterSlaveReqIdData = NULL;
	BOOL isDeleted = FALSE;

	CMedString buff;
	buff << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::deleteMasterSlaveHandlerAccordingToSubId subReqId: " << subReqId << '\n';
	PTRACE(eLevelInfoNormal,buff.GetString());

	if(m_masterSlaveRequestSubIdToIdMap.find(subReqId) != m_masterSlaveRequestSubIdToIdMap.end())
	{
		handlerId = m_masterSlaveRequestSubIdToIdMap[subReqId];
		isDeleted = deleteMasterSlaveHandlerAccordingToReqId(handlerId);
	}

	return isDeleted;
}
//=========================================================================================================================================================================================//
BOOL CMplApiMasterSlaveReqAsyncHandler::isMasterSlaveRequestFinished(SMasterSlaveRequestData* pMasterSlaveReqIdData)
{
	BOOL isFinished = FALSE;

	if(pMasterSlaveReqIdData != NULL)
	{
		map<DWORD, BOOL>::iterator indexedItr = pMasterSlaveReqIdData->m_subIdsAckStatusMap.begin();
		map<DWORD, BOOL>::iterator endItr = pMasterSlaveReqIdData->m_subIdsAckStatusMap.end();
		while(indexedItr != endItr)
		{
			isFinished = (*indexedItr).second;
			if(isFinished == FALSE)
			{
				isFinished = FALSE;
				break;
			}
			else
			{
				isFinished = TRUE;
			}
			indexedItr++;
		}
	}

	CMedString buff;
	buff << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::isMasterSlaveRequestFinished isFinished: " << isFinished << '\n';
	PTRACE(eLevelInfoNormal,buff.GetString());

	return isFinished;
}
//=========================================================================================================================================================================================//
void CMplApiMasterSlaveReqAsyncHandler::traceMasterSlaveRequestHandler()
{
	CLargeString buff;
	SMasterSlaveRequestData* pMasterSlaveReqIdData = NULL;
	buff << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::traceMasterSlaveRequestHandler" << '\n';
	buff << "m_masterSlaveRequestIdToSubIdMap.size(): " << m_masterSlaveRequestIdToSubIdMap.size() <<'\n';

	map<DWORD, SMasterSlaveRequestData*>::iterator indexedItr = m_masterSlaveRequestIdToSubIdMap.begin();
	map<DWORD, SMasterSlaveRequestData*>::iterator endItr = m_masterSlaveRequestIdToSubIdMap.end();

	while(indexedItr != endItr)
	{
		buff << "HandlerId: " << (*indexedItr).first;
		pMasterSlaveReqIdData = (*indexedItr).second;

		if(pMasterSlaveReqIdData != NULL)
		{
 		    static const CProcessBase* process = CProcessBase::GetProcess();
		    const std::string &str = process->GetOpcodeAsString(pMasterSlaveReqIdData->m_opcode);

		    buff << " Opcode: " << str << " (" << pMasterSlaveReqIdData->m_opcode << ")" << '\n';
			buff << "HandlerInitialTime: " << pMasterSlaveReqIdData->m_initiateTime << '\n';

			map<DWORD, BOOL>::iterator indexedItrAck = pMasterSlaveReqIdData->m_subIdsAckStatusMap.begin();
			map<DWORD, BOOL>::iterator endItrAck = pMasterSlaveReqIdData->m_subIdsAckStatusMap.end();
			while(indexedItrAck != endItrAck)
			{
				buff << "HandlerSubId: " << (*indexedItrAck).first << " HandlerStatus: " << (*indexedItrAck).second << '\n';
				indexedItrAck++;
			}
		}
		indexedItr++;

	}
	PTRACE(eLevelInfoNormal,buff.GetString());

}
//=========================================================================================================================================================================================//
bool CMplApiMasterSlaveReqAsyncHandler::GetHandlerIdBySubReqId(DWORD& handler_id,DWORD subReqId)
{
	if(m_masterSlaveRequestSubIdToIdMap.find(subReqId) == m_masterSlaveRequestSubIdToIdMap.end())
	{
	  CMedString buff;
	  buff << "1080_60_req_sync: CMplApiMasterSlaveReqAsyncHandler::GetHandlerIdBySubReqId subReqId: not found " << subReqId << '\n';
	  PTRACE(eLevelInfoNormal,buff.GetString());
	  return false;
	}
	handler_id = m_masterSlaveRequestSubIdToIdMap[subReqId];
	return true;
}

