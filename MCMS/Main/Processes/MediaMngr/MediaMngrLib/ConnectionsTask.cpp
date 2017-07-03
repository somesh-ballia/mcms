//
//   ConnectionsTask - 
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "ConnectionsTask.h"

#include "SimApi.h"
#include "MplMcmsProtocol.h"
#include "MplMcmsProtocolTracer.h"
#include "IpRtpReq.h"
#include "HostCommonDefinitions.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsAudio.h"

#include "TraceStream.h"
#include "ProcessBase.h"
#include "AudioApiDefinitionsStrings.h"
#include "IpCmReq.h"
#include "IpMfaOpcodes.h"
#ifndef __DISABLE_ICE__
#include "OpcodesMcmsCardMngrICE.h"

#include "IceManager.h"
#include "IceSession.h"
#include "MMIceMplMsgSender.h"
#include "Lock.hxx"
#include "IceLogAdapter.h"
#endif	//__DISABLE_ICE__

//#include "ConfigManagerApi.h"

#ifndef __DISABLE_ICE__
IceManager *g_pIceMngr=0;
#endif	//__DISABLE_ICE__

PBEGIN_MESSAGE_MAP(CConnectionsTask)

	//event from Gideon Sim
	ONEVENT( SIM_API_H323_MSG, ANYCASE, CConnectionsTask::OnGideonSimCommandAll)
	
	//event coming from Ema-Engine
	ONEVENT( SIM_API_MEDIA_MSG,	ANYCASE, CConnectionsTask::OnEmaEngineCommandAll)
	
	//Global timer for All Tx Channels
	ONEVENT( GLOBAL_TX_TIMER, ANYCASE, CConnectionsTask::OnGlobalTxTimer)

	//Global timer for switch audio stream
	ONEVENT( GLOBAL_SWITCH_TIMER, ANYCASE, CConnectionsTask::OnGlobalSwitchTimer)

	//event coming from Command Line or HTTP XML interface
	ONEVENT( SIM_API_RESET_CHANNEL_OUT_MSG, ANYCASE, CConnectionsTask::OnResetChannelCommandAll)
	
	//event coming from Command Line or HTTP XML interface
	ONEVENT( SIM_API_VIDEO_UPDATE_PIC_MSG, ANYCASE, CConnectionsTask::OnVideoUpdatePicCommandAll)
	
PEND_MESSAGE_MAP(CConnectionsTask, CTaskApp);



/////////////////////////////////////////////////////////////////////////////
//  task creation function
void ConnectionsTaskEntryPoint(void* appParam)
{
	CConnectionsTask*  pConnectionsTask = new CConnectionsTask;
	pConnectionsTask->Create(*(CSegment*)appParam);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CConnectionsTask::CConnectionsTask()
	: m_GideonSimMplApi(eProcessGideonSim)
{
	TRACEINTO << "CConnectionsTask::CConnectionsTask";
	
	int i = 0;
	
	for (; i < MAX_NUM_OF_ENDPOINTS; i++)
	{		
		m_pEPAccessArray[i] = NULL;
	}
	
	m_nEPCounter = 0;
	
	for (i = 0; i < MAX_NUM_OF_ENDPOINTS*2; i++)
	{		
		m_pMonitoringIDEPArray[i] = NULL;
	}
	
	m_nMaxNumberSpeaker       = 1;
	m_nSwitchAudioInterval    = 3000;
	m_bSwitchAudio            = TRUE;

#ifndef __DISABLE_ICE__
	g_pIceMngr=new IceManager(*(new MMIceMplMsgSender(*this)));
	m_IceProcessor=g_pIceMngr;
#endif
}

/////////////////////////////////////////////////////////////////////////////

CConnectionsTask::~CConnectionsTask()
{
	TRACEINTO << "CConnectionsTask::~CConnectionsTask";
	
	DeleteTimer(GLOBAL_TX_TIMER);

	DeleteTimer(GLOBAL_SWITCH_TIMER);
	
	int i = 0;
	
	for (; i < MAX_NUM_OF_ENDPOINTS; i++)
	{		
		POBJDELETE(m_pEPAccessArray[i]);
	}
	
	for (i = 0; i < MAX_NUM_OF_ENDPOINTS*2; i++)
	{		
		POBJDELETE(m_pMonitoringIDEPArray[i]);
	}
}



/////////////////////////////////////////////////////////////////////////////

void* CConnectionsTask::GetMessageMap()
{
	return (void*)m_msgEntries;
}

/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::Create(CSegment& appParam)
{
	//TRACEINTO << "CConnectionsTask::Create";
	
	CTaskApp::Create(appParam);
}
/////////////////////////////////////////////////////////////////////////////

void  CConnectionsTask::InitTask()
{
	TRACEINTO << "CConnectionsTask::InitTask pid: " << getpid() <<  " taskid:" << GetTaskId();
    //    int pid = getpid();
    //   CConfigManagerApi api;
    //   api.ChangeMyNiceLevel(-15,pid);
    
	StartTimer (GLOBAL_TX_TIMER, 1);

	m_nMaxNumberSpeaker    = ::GetMediaMngrCfg()->GetMaxNumerSpeaker();
	m_nSwitchAudioInterval = ::GetMediaMngrCfg()->GetSwitchAudioInterval();
	if(::GetMediaMngrCfg()->GetStateSwitchAudio())
	{
		m_bSwitchAudio         = TRUE;	
	}
	else
	{
		m_bSwitchAudio         = FALSE;	
	}

	//fprintf(stderr,"CConnectionsTask::InitTask,m_nMaxNumberSpeaker = %d,m_nSwitchAudioInterval = %d,m_bSwitchAudio = %d\n",m_nMaxNumberSpeaker,m_nSwitchAudioInterval,m_bSwitchAudio);
	TRACEINTO << "CConnectionsTask::InitTask, - m_nMaxNumberSpeaker = " << m_nMaxNumberSpeaker << " m_nSwitchAudioInterval = " << m_nSwitchAudioInterval << "m_bSwitchAudio = " << ::GetMediaMngrCfg()->GetStateSwitchAudio();

	if(m_bSwitchAudio)
	{
		StartTimer (GLOBAL_SWITCH_TIMER, m_nSwitchAudioInterval);
	}

	m_timerCompensation.m_lastTimerTime = 100; //size of sleep time (1 = 100)
	clock_gettime(CLOCK_REALTIME, &m_timerCompensation.m_lastTime);
}

/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::SelfKill()
{
	TRACEINTO << "CConnectionsTask::SelfKill";
	
    DeleteAllTimers();
	CTaskApp::SelfKill();
}

/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::CreateEPConnection(int index, DWORD participantId)
{
	if (index < MAX_NUM_OF_ENDPOINTS)
	{
		//CEPAccess* epAccess = new CEPAccess(this);//threads
		CEPAccess* epAccess = new CEPAccessObj(this);//objects
		
		epAccess->SetEpRsrcId(participantId);
		epAccess->SetIndex(index);
		((CEPAccessObj*)epAccess)->PrepareParticipantTicket();
		
		m_pEPAccessArray[index] = epAccess;
		m_nEPCounter++;
		
		TRACEINTO << "CConnectionsTask::CreateEPConnection index:" << index << " participantId:" << participantId << " epCounter: " << m_nEPCounter;
	}
}


/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::OnGideonSimCommandAll(CSegment* pParam)
{
	CMplMcmsProtocol* pMplProtocol = new CMplMcmsProtocol;
	pMplProtocol->DeSerialize(*pParam);
	
	// Handle the event in the list
	HandleEvent(pMplProtocol);

	POBJDELETE(pMplProtocol);
}

/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::OnEmaEngineCommandAll(CSegment* pParam)
{
	TRACEINTO << "CConnectionsTask::OnEmaEngineCommandAll - msg from ema-engine";
	
	DWORD monitorPartyId, monitorConfId;
	CSegment* pSeg = new CSegment(*pParam);
	*pSeg >> monitorPartyId
		  >> monitorConfId;

	BOOL bIsFound = FALSE;
	int indexFound = -1;
	
	//find participant with {monitorPartyId , monitorConfId} in participant array 
	///////////////////////////////////////////////////////////////////////////
	CEPAccess* epAccess = NULL;
	DWORD currentMonitorPartyId, currentMonitorConfId;
	for (int i = 0; i < MAX_NUM_OF_ENDPOINTS; i++)
	{
		epAccess = (CEPAccessObj*)m_pEPAccessArray[i];
		
		if (epAccess == NULL)
			continue;
		
		currentMonitorPartyId = epAccess->GetMonitorPartyId();
		currentMonitorConfId = epAccess->GetMonitorConfId();	
		
		
		if (currentMonitorPartyId==monitorPartyId && currentMonitorConfId==monitorConfId)
		{
			bIsFound = TRUE;
			indexFound = i;
			TRACEINTO << "CConnectionsTask::OnEmaEngineCommandAll - found participant at index: " << indexFound;
			break;
		}
	}
	
	//if participant is found forward the media param message to this participant
	if (bIsFound)
	{
		epAccess = (CEPAccessObj*)m_pEPAccessArray[indexFound];		
		
		epAccess->ExecuteAPIMessage(pParam);
	}
	else // keep the media param message in a temp array, till dtmf info (rsrcid and monitoring ids) arrive 
	{
		TMonitoringIDEP* pMonitorIDEP = new TMonitoringIDEP();
		pMonitorIDEP->monitorPartyId = monitorPartyId;
		pMonitorIDEP->monitorConfId = monitorConfId;
		pMonitorIDEP->segment = pParam;
		
		for (int i = 0; i < MAX_NUM_OF_ENDPOINTS*2; i++)
		{
			if (m_pMonitoringIDEPArray[i] != NULL)
				continue;

			//add monitoringIDEP to tmp arr
			m_pMonitoringIDEPArray[i] = pMonitorIDEP;
			TRACEINTO << "CConnectionsTask::OnEmaEngineCommandAll - add monitoringIDEP to tmp array at index=" << i;
			break;
		}
		
	}
	
	POBJDELETE(pSeg);
	
}

/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::OnGlobalTxTimer(CSegment* pParam)
{
	// get current system time
	///////////////////////////////	
	struct timespec currentTP;
	clock_gettime(CLOCK_REALTIME, &currentTP);
	
	//test timer delta
	///////////////////////////////
	DWORD diff1000 = (currentTP.tv_sec - m_timerCompensation.m_lastTime.tv_sec) * 10000;
	DWORD curr1000 = currentTP.tv_nsec/100000;
	DWORD prev1000 = m_timerCompensation.m_lastTime.tv_nsec/100000;
	
	//calculate the deviation time
	///////////////////////////////
	curr1000 += diff1000;
	int diffTime = curr1000 - prev1000;
	int deviationTime  = diffTime - m_timerCompensation.m_lastTimerTime;
	
	//TRACEINTO << "CConnectionsTask::OnGlobalTxTimer - diffTime = " << diffTime << " deviationTime = " << deviationTime;
	
	//save current time stamp
	///////////////////////////////	
	m_timerCompensation.m_lastTime.tv_sec = currentTP.tv_sec;
	m_timerCompensation.m_lastTime.tv_nsec = currentTP.tv_nsec;
	
	//accumulate the time deviation
	///////////////////////////////
	m_timerCompensation.m_accTime = deviationTime;
	
	
	
	m_timerCompensation.m_printCounter++;
	if (m_timerCompensation.m_printCounter == 100)
	{
		//TRACEINTO << "CConnectionsTask::OnGlobalTxTimer - m_timerCompensation.m_accTime = " << m_timerCompensation.m_accTime << " (print every 100 timers)";
		m_timerCompensation.m_printCounter = 0;
	}
	
	m_timerCompensation.m_betweenCompensationCounter++;
	
	
	// limit the compensation time to 2 seconds (2*10000)
	if (m_timerCompensation.m_accTime > 20000)
	{
		TRACEINTO << "CConnectionsTask::OnGlobalTxTimer - ****! Compensation time limitation. m_timerCompensation.m_accTime = " << m_timerCompensation.m_accTime;
		m_timerCompensation.m_accTime = 20000;
	}
	
	//if accumulated time deviation is bigger than 100 ( 10 millisec) -> compensate for the 'lost' time
	///////////////////////////////
	
	//loop untill accumulated deviation time is below 100 
	while ( m_timerCompensation.m_accTime >= 100 )
	{
		m_timerCompensation.m_accTime -= 100;
		//invoke timer on all end points 
		InvokeEPGlobalTimer();
		
		TRACEINTO << "CConnectionsTask::OnGlobalTxTimer - ****! Compensation time. m_timerCompensation.m_accTime = " << m_timerCompensation.m_accTime << "(" << m_timerCompensation.m_betweenCompensationCounter << ")";
		m_timerCompensation.m_betweenCompensationCounter = 0;
	}
	
	/*
	m_timerCompensation.m_lastTimerTime = 0;
	m_timerCompensation.m_compensationCounter++;
	TRACEINTO << "CConnectionsTask::OnGlobalTxTimer - ***! Compensation time. m_timerCompensation.m_accTime = " << m_timerCompensation.m_accTime << "(" << m_timerCompensation.m_compensationCounter << ")" << "(" << m_timerCompensation.m_betweenCompensationCounter << ")";
	m_timerCompensation.m_betweenCompensationCounter = 0;
	*/
	
	//restart timer for the next loop
	//StartTimer (GLOBAL_TX_TIMER, 1);
	
	// start timer with compensation of the prev time
	int accMicroSec = 100*m_timerCompensation.m_accTime;
	int timerValMicroSec = 10000 - accMicroSec; //the time difference that need to be consumed
	m_timerCompensation.m_accTime = 0;
	StartTimer (GLOBAL_TX_TIMER, TICKS(0, timerValMicroSec));
	m_timerCompensation.m_lastTimerTime = 100;
		
	
	//invoke timer on all end points
	/////////////////////////////////////////////////////////////////////
	
	InvokeEPGlobalTimer();
}


/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::InvokeEPGlobalTimer()
{
	CEPAccess* epAccess = NULL;
	
	//invoke timer for each participant
	////////////////////////////////////////
	for (int i = 0; i < MAX_NUM_OF_ENDPOINTS; i++)
	{
		epAccess = (CEPAccessObj*)m_pEPAccessArray[i];
		
		if (epAccess != NULL)
		{
			epAccess->InvokeEPGlobalTxTimer();
		}
	}
}

void CConnectionsTask::OnGlobalSwitchTimer(CSegment* pParam)
{
	TRACEINTO << "CConnectionsTask::OnGlobalSwitchTimer,the number of conference:" << m_mapConf.size();	
	///////////////////////////////////////////////////////////////////////////////
	CEPAccess* epAccess = NULL;

	map<int,ENDPOINTS>::iterator it;
	for(it = m_mapConf.begin();it != m_mapConf.end();it++)
	{
		ENDPOINTS endpoint_info = it->second;	

		int nEndpointSize = endpoint_info.lstEndpoint.size();
		int nSpeakerSize = endpoint_info.lstSpeaker.size();

		TRACEINTO << "CConnectionsTask::OnGlobalSwitchTimer,conference ID = " << it->first;
		TRACEINTO << "CConnectionsTask::OnGlobalSwitchTimer,nEndpointSize = " << nEndpointSize << "nSpeakerSize = " << nSpeakerSize;		

		//it means that all the endpoints are speaker.
		if((nSpeakerSize >= nEndpointSize)||(m_nMaxNumberSpeaker<=0))
		{
			continue;
		}

		//begin
		///////////
		if(nSpeakerSize < m_nMaxNumberSpeaker)
		{
			//change a endpoint from a silent stream to real stream
			if(nSpeakerSize < nEndpointSize)
			{
				////////////////////////////////////////////////////////////////////////
				list<int>::iterator itEndpoint;
				for(itEndpoint = endpoint_info.lstEndpoint.begin(); itEndpoint != endpoint_info.lstEndpoint.end(); itEndpoint++)
				{
					epAccess = (CEPAccessObj*)m_pEPAccessArray[*itEndpoint];

					if (epAccess != NULL)
					{
						if(!FindEPSendAudio(endpoint_info.lstSpeaker,*itEndpoint))
						{
							//epAccess->GetEPConnectionPtr()->m_bSendSilentStream = FALSE;
							epAccess->GetEPConnectionPtr()->SetSilentStream(FALSE);							
							epAccess->InvokeEPAudioUpdateChannel();
							endpoint_info.lstSpeaker.push_back(*itEndpoint);
							break;
						}
					}
				}
				it->second = endpoint_info;
				////////////////////////////////////////////////////////////////////////			
			}
		}
		else
		{
			int i = 0;
			int j = 0;

			list<int>::iterator itSpeaker;
			list<int>::iterator itEndpoint;

			//switch the audio steam among all the endpoints
			if(nSpeakerSize < nEndpointSize)
			{
				//send silent stream to old endpoints
				for(itSpeaker = endpoint_info.lstSpeaker.begin(); itSpeaker != endpoint_info.lstSpeaker.end(); itSpeaker++)
				{
					epAccess = (CEPAccessObj*)m_pEPAccessArray[*itSpeaker];		
					//epAccess->GetEPConnectionPtr()->m_bSendSilentStream = TRUE;
					epAccess->GetEPConnectionPtr()->SetSilentStream(TRUE);			
					epAccess->InvokeEPAudioUpdateChannel();
					TRACEINTO << "CConnectionsTask::OnGlobalSwitchTimer, ##############mute index = " << *itSpeaker;			
				}

				//get index of EP
				int * nEPArray = new int [nEndpointSize];

				//assign another part of value
				for(itEndpoint = endpoint_info.lstEndpoint.begin(); itEndpoint != endpoint_info.lstEndpoint.end(); itEndpoint++)
				{
					epAccess = (CEPAccessObj*)m_pEPAccessArray[*itEndpoint];
					
					if (epAccess != NULL)
					{
						if(!FindEPSendAudio(endpoint_info.lstSpeaker,*itEndpoint))
						{
							nEPArray[j] = *itEndpoint;
							j++;
						}
					}
				}

				//assign part of value
				for(itSpeaker = endpoint_info.lstSpeaker.begin(); itSpeaker != endpoint_info.lstSpeaker.end(); itSpeaker++)
				{
					epAccess = (CEPAccessObj*)m_pEPAccessArray[*itSpeaker];
					
					if (epAccess != NULL)
					{
						nEPArray[j] = *itSpeaker;
						j++;
					}
				}

				//new speaker index
				itSpeaker = endpoint_info.lstSpeaker.begin();
				for(j = 0; j< nSpeakerSize; j++)
				{
					*itSpeaker = nEPArray[j];
					itSpeaker++;
				}

				//change order of the index of endpoint array
				itEndpoint = endpoint_info.lstEndpoint.begin();
				for(i = 0; i< nEndpointSize; i++)
				{
					 *itEndpoint = nEPArray[i];
					 itEndpoint++;
				}

				it->second = endpoint_info;
				
				delete nEPArray;
			}

			//send normal stream to new endpoint

			for(itSpeaker = endpoint_info.lstSpeaker.begin(); itSpeaker != endpoint_info.lstSpeaker.end(); itSpeaker++)
			{
				epAccess = (CEPAccessObj*)m_pEPAccessArray[*itSpeaker];	
				//epAccess->GetEPConnectionPtr()->m_bSendSilentStream = FALSE;
				epAccess->GetEPConnectionPtr()->SetSilentStream(FALSE);
				epAccess->InvokeEPAudioUpdateChannel();
				TRACEINTO << "CConnectionsTask::OnGlobalSwitchTimer, ##############speaker index = " << *itSpeaker;				
			}
		}
		///////////
		//end
	}
	
	StartTimer (GLOBAL_SWITCH_TIMER, m_nSwitchAudioInterval);
	///////////////////////////////////////////////////////////////////////////////
}


//added by huiyu
BOOL CConnectionsTask::FindEPSendAudio(const list<int> &lstSpeaker,int index)
{
	list<int>::const_iterator it;
	for(it = lstSpeaker.begin(); it != lstSpeaker.end(); it++)
	{
		if( index == *it)
		{
			return TRUE;
		}
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::HandleEvent(CMplMcmsProtocol* pMplProtocol)
{
	//DumpEPArray();
	
	//opcode
	OPCODE opCode = pMplProtocol->getOpcode();
	const std::string &opcodestr = GetOpcodeStr(pMplProtocol);	
	
	//media type
	eResourceTypes  resType = (eResourceTypes)pMplProtocol->getPhysicalInfoHeaderResource_type();
	const char* strResType = ::ResourceTypeToString(resType);
	
	//get Resource Id=Participant Id
	DWORD participantId = pMplProtocol->getPortDescriptionHeaderParty_id();
	
	TRACEINTO << "CConnectionsTask::HandleEvent - ResourceId:" << participantId << " Opcode: " << opcodestr << "(" << opCode  << ")"<< " Resource Type: " << strResType << "("<< resType << ")";

#ifndef __DISABLE_ICE__
	switch(opCode) {
		case ICE_MAKE_OFFER_REQ:
			{
				int sessID=0;
				m_IceProcessor->MakeOffer(*pMplProtocol, sessID);
				IceSession *sess=m_IceProcessor->GetIceSession(sessID);
				if(sess) {
					sess->SetAudioRtpCallBack(0);
					sess->SetVideoRtpCallBack(0);
				}
			}
			break;
		case ICE_MAKE_ANSWER_REQ:
			{
				int sessID=0;
				m_IceProcessor->MakeAnswer(*pMplProtocol, sessID);
				IceSession *sess=m_IceProcessor->GetIceSession(sessID);

				if(sess) {
					sess->SetAudioRtpCallBack(0);
					sess->SetVideoRtpCallBack(0);
				}
			}
			break;

		case ICE_INIT_REQ:
		case ICE_PROCESS_ANSWER_REQ:
		case ICE_CLOSE_SESSION_REQ:
		case ICE_MODIFY_SESSION_ANSWER_REQ:
		case ICE_MODIFY_SESSION_OFFER_REQ:
			m_IceProcessor->HandleIceMplMsg(*pMplProtocol);
			return;
			break;
	}
#endif 	//__DISABLE_ICE__
	
	////////////////////////////////////////////////////////////////////////////////////////////
	// TB_MSG_CLOSE_PORT_REQ - remove participant from list (all channels have allready removed)
	if (opCode == TB_MSG_CLOSE_PORT_REQ)
	{
		RemoveEPConnection(participantId);
		return;
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	
	
	
	int firstAvailableIndex = -1;
	BOOL bIsFound = FALSE;
	int indexFound = -1;
	CEPAccess* epAccess = NULL;
	
	//find participant in participant array
	////////////////////////////////////////
	for (int i = 0; i < MAX_NUM_OF_ENDPOINTS; i++)
	{
		epAccess = (CEPAccessObj*)m_pEPAccessArray[i];
		
		if (epAccess == NULL)
		{			
			if (firstAvailableIndex == -1)
			{
				firstAvailableIndex = i;
				TRACEINTO << "CConnectionsTask::HandleEvent - found first available index: " << firstAvailableIndex;
			}	
			continue;
		}
		
		DWORD currentRsrcID = epAccess->GetEpRsrcId();
		
		if (currentRsrcID == participantId)
		{
			bIsFound = TRUE;
			indexFound = i;
			TRACEINTO << "CConnectionsTask::HandleEvent - found participantId: " << currentRsrcID << " at index: " << indexFound;
			break;
		}
	}
	
	TRACEINTO << "CConnectionsTask::HandleEvent - bIsFound=" << (int)bIsFound << " indexFound=" << indexFound;
	
	
	
	//participant not found - create a new participant
	//////////////////////////////////////////
	if (bIsFound == FALSE)
	{
		 if(firstAvailableIndex != -1)
		 {
			TRACEINTO << "CConnectionsTask::HandleEvent - Create new participant. participantId = " << participantId;
	
			//create participant at first available index
			CreateEPConnection(firstAvailableIndex, participantId);
		
			bIsFound = TRUE;
			indexFound = firstAvailableIndex;	
		 }
		 else
		 {
		 	TRACEINTO << "CConnectionsTask::HandleEvent - Not found ResrID for participantId:" << participantId <<",Also no available index for it.";
			return;
		 }
	}
	
	
	//set the EndPoint Access pointer
	epAccess = (CEPAccessObj*)m_pEPAccessArray[indexFound];
	
	TRACEINTO << "CConnectionsTask::HandleEvent - index = " << indexFound << " participantId = " << participantId;

	if (opCode == H323_RTP_UPDATE_MT_PAIR_REQ)
	{
		TUpdateMtPairReq* pStruct = (TUpdateMtPairReq*)pMplProtocol->GetData();
		DWORD  aliasName = pStruct->unDestMcuId;
		TRACEINTO << "CConnectionsTask::HandleEvent - conference ID = " << aliasName;

		DWORD conferenceId = aliasName;
		//added by huiyu
		////////////////////////////////////////////////////////////////////////
		if(m_bSwitchAudio)
		{
			ENDPOINTS endpoint_info;
			map<int,ENDPOINTS>::iterator itFind;
			itFind = m_mapConf.find(conferenceId);
			
			if(itFind != m_mapConf.end())
			{
				endpoint_info = m_mapConf[conferenceId];		
			}

			BOOL bFind = FALSE;
			for(list<int>::iterator itList = endpoint_info.lstEndpoint.begin(); itList != endpoint_info.lstEndpoint.end(); itList++)
			{
				if( indexFound== *itList)
				{
					bFind = TRUE;
				}
			}

			//when the index is new, the index will be inserted into list
			if(!bFind)
			{
				endpoint_info.lstEndpoint.push_back(indexFound);
				
				if(int(endpoint_info.lstSpeaker.size())<m_nMaxNumberSpeaker)
				{
					endpoint_info.lstSpeaker.push_back(indexFound);
					//epAccess->GetEPConnectionPtr()->m_bSendSilentStream = FALSE;
					epAccess->GetEPConnectionPtr()->SetSilentStream(FALSE);
				}
				else
				{
					//epAccess->GetEPConnectionPtr()->m_bSendSilentStream = TRUE;
					epAccess->GetEPConnectionPtr()->SetSilentStream(TRUE);
				}
				m_mapConf[conferenceId] = endpoint_info;			
			}
		}
		/////////////////////////////////////////////////////////////////		
	}

	
	//handle send dtmf opcode
	if (opCode == AUDIO_PLAY_TONE_REQ)
	{
		BOOL flag = HandleDTMF(pMplProtocol, epAccess);
		if (flag == FALSE) // means that it is a dtmf-info
		{
			TRACEINTO << "CConnectionsTask::HandleEvent - AUDIO_PLAY_TONE_REQ - dtmf Info - return...";
			return;
		}
	}
	
	////////////////////////////////
	//forward protocol to end-point
	////////////////////////////////
	epAccess->ExecuteEventMessage(pMplProtocol);
}



/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::OnResetChannelCommandAll(CSegment* pParam)
{
	string endpointList;
	BYTE channelType = 0x0;
	CSegment* pSeg = new CSegment(*pParam);
	*pSeg >> endpointList
		  >> channelType;

	TRACEINTO << "CConnectionsTask::OnResetChannelCommandAll - start reset channels."
			  << " endpointList: " << endpointList.c_str() << " channelType: " << channelType;

	
	if (endpointList == "ALL")
	{
		CEPAccess* epAccess = NULL;
		
		//invoke reset media channel for each participant
		////////////////////////////////////////
		for (int i = 0; i < MAX_NUM_OF_ENDPOINTS; i++)
		{
			epAccess = (CEPAccessObj*)m_pEPAccessArray[i];
			
			if (epAccess != NULL)
			{
				epAccess->InvokeEPResetChannel(channelType);
			}
		}
	}
		
	POBJDELETE(pSeg);
	
	
	TRACEINTO << "CConnectionsTask::OnResetChannelCommandAll - end reset channels";
}



/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::OnVideoUpdatePicCommandAll(CSegment* pParam)
{
	TRACEINTO << "CConnectionsTask::OnVideoUpdatePicCommandAll ";

	CEPAccess* epAccess = NULL;
	
	//invoke Intra for the first participant we find (test)
	////////////////////////////////////////
	for (int i = 0; i < MAX_NUM_OF_ENDPOINTS; i++)
	{
		epAccess = (CEPAccessObj*)m_pEPAccessArray[i];
		
		if (epAccess != NULL)
		{
			TRACEINTO << "CConnectionsTask::OnVideoUpdatePicCommandAll - A connection was found ";
			epAccess->InvokeEPVideoUpdatePic();
			break;
		}
	}


}


/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::DumpEPArray()
{
	CEPAccess* epAccess = NULL;
	for (int i = 0; i < MAX_NUM_OF_ENDPOINTS; i++)
	{
		epAccess = (CEPAccessObj*)m_pEPAccessArray[i];
		if (epAccess == NULL)
			continue;
		
		TRACEINTO << "CConnectionsTask::DumpEPArray - index=" << i <<  " rsrcid=" << epAccess->GetEpRsrcId();		
	}
}

/////////////////////////////////////////////////////////////////////////////

void CConnectionsTask::RemoveEPConnection(DWORD participantId)
{
	TRACEINTO << "CConnectionsTask::RemoveEPConnection participantId: " << participantId;
	
	CEPAccess* removeEPAccess = NULL;
	DWORD removeRsrcID = 0;
	for (int index = 0; index < MAX_NUM_OF_ENDPOINTS; index++)
	{
		removeEPAccess = (CEPAccessObj*)m_pEPAccessArray[index];
		if (removeEPAccess != NULL)
		{
			removeRsrcID = removeEPAccess->GetEpRsrcId();
			if (removeRsrcID == participantId)
			{
				POBJDELETE(m_pEPAccessArray[index]);
				m_nEPCounter--;

				//added by huiyu 2010.2.20
				//////////////////////////////////////////////////
				if(m_bSwitchAudio)
				{
					map<int,ENDPOINTS>::iterator itConf;
					for(itConf = m_mapConf.begin();itConf != m_mapConf.end();itConf++)
					{
						ENDPOINTS endpoint_info = itConf->second;	

						BOOL bFind = FALSE;
						list<int>::iterator it;						
						for(it=endpoint_info.lstSpeaker.begin(); it!=endpoint_info.lstSpeaker.end(); it++)
						{
							if(*it == index)
							{
								bFind = TRUE;
								break;
							}
						}
						if(bFind)
						{
							endpoint_info.lstSpeaker.erase(it);			
						}
						
						bFind = FALSE;						
						for(it=endpoint_info.lstEndpoint.begin(); it!=endpoint_info.lstEndpoint.end(); it++)
						{
							if(*it == index)
							{
								bFind = TRUE;
								break;
							}
						}
						if(bFind)
						{
							TRACEINTO << "CConnectionsTask::RemoveEPConnection,remove a endpoint, index = " << index;		
							endpoint_info.lstEndpoint.erase(it);
							itConf->second = endpoint_info;
							break;
						}	
					}

					//check the number of participant in a conference, delete the conference that has no participant.
					for(itConf = m_mapConf.begin();itConf != m_mapConf.end();)
					{
						ENDPOINTS endpoint_info = itConf->second;
						int nEndpointSize = endpoint_info.lstEndpoint.size();
						if(0 == nEndpointSize)
						{
							m_mapConf.erase(itConf++);
						}
						else
						{
							++itConf;
						}
					}
				}
				//////////////////////////////////////////////////
				
				TRACEINTO << "CConnectionsTask::RemoveEPConnection participant found at index:" << index << " participantId: " << participantId << " epCounter: " << m_nEPCounter;			
				
				return;
			}
		}
	}
	
	TRACEINTO << "CConnectionsTask::RemoveEPConnection participantId: " << participantId << " NOT found";
	return;
}


/////////////////////////////////////////////////////////////////////////////


BOOL CConnectionsTask::HandleDTMF(CMplMcmsProtocol* pMplProtocol, CEPAccess* epAccess)
{
	TRACEINTO << "CConnectionsTask::HandleDTMF";
	
	//check if this dtmf is 'DTMF-Info' or regular dtmf message
	SPlayToneStruct* tPlayToneStruct = (SPlayToneStruct*)pMplProtocol->getpData();
	
	if (tPlayToneStruct == NULL)
	{
		TRACEINTO << "MM ERROR CConnectionsTask::HandleDTMF tPlayToneStruct is NULL";
		return FALSE;
	}
	
	//only if dtmf string is bigger than 7 , searching for 'ABC<confId>#<partyMonitorId>#'
	if (tPlayToneStruct->numOfTones > 7) 
	{
		if (tPlayToneStruct->tone[0].tTone == E_AUDIO_TONE_SILENCE &&
			tPlayToneStruct->tone[1].tTone == E_AUDIO_TONE_DTMF_A &&
			tPlayToneStruct->tone[2].tTone == E_AUDIO_TONE_SILENCE &&	
			tPlayToneStruct->tone[3].tTone == E_AUDIO_TONE_DTMF_B &&
			tPlayToneStruct->tone[4].tTone == E_AUDIO_TONE_SILENCE &&
			tPlayToneStruct->tone[5].tTone == E_AUDIO_TONE_DTMF_C &&
			tPlayToneStruct->tone[6].tTone == E_AUDIO_TONE_SILENCE)
		{
			DWORD monitorConfId;
			DWORD monitorPartyId;
			
			string dtmfTmp = "";
			char tone;
			// eject confId
			APIU32 i=7;
			for (; i<tPlayToneStruct->numOfTones; i++)
			{
				if (tPlayToneStruct->tone[i].tTone == E_AUDIO_TONE_DTMF_PAUND)
				{
					break;
				}
				else if (tPlayToneStruct->tone[i].tTone ==E_AUDIO_TONE_SILENCE)
				{
					continue;
				}
				else
				{
					tone = ::GetTone(tPlayToneStruct->tone[i].tTone);
					dtmfTmp += tone;
				}
			}
			
			monitorConfId = atoi(dtmfTmp.c_str());
			epAccess->SetMonitorConfId(monitorConfId);
			
			
			dtmfTmp = "";
			// eject partyId
			i++;
			for (; i<tPlayToneStruct->numOfTones; i++)
			{
				if (tPlayToneStruct->tone[i].tTone == E_AUDIO_TONE_DTMF_PAUND)
				{
					break;
				}
				else if (tPlayToneStruct->tone[i].tTone ==E_AUDIO_TONE_SILENCE)
				{
					continue;
				}
				else
				{
					tone = ::GetTone(tPlayToneStruct->tone[i].tTone);
					dtmfTmp += tone;
				}
			}
						
			monitorPartyId = atoi(dtmfTmp.c_str());
			epAccess->SetMonitorPartyId(monitorPartyId);
			
			
			TRACEINTO << "CConnectionsTask::HandleDTMF PartyRsrcId=" << epAccess->GetEpRsrcId() << 
												     " ConfMonitorId=" << epAccess->GetMonitorConfId() <<
												     " PartyMonitorId=" << epAccess->GetMonitorPartyId();
												     
						
			
			
			/////////////////////////////////////////////////////////////////////////////
			//loop on tmp array to find event to send to participant
			/////////////////////////////////////////////////////////////////////////////
			
			TMonitoringIDEP* tmpMonitorEP;
			CSegment* seg;
			for (int i = 0; i < MAX_NUM_OF_ENDPOINTS*2; i++)
			{
				if (m_pMonitoringIDEPArray[i] == NULL)
					continue;
				
				
				tmpMonitorEP = m_pMonitoringIDEPArray[i];
				
				if (tmpMonitorEP->monitorConfId==monitorConfId &&
						tmpMonitorEP->monitorPartyId==monitorPartyId)
				{
					//found event waiting for this endpoint -> get segment , removed event and send to endpoint
					TRACEINTO << "CConnectionsTask::HandleDTMF - found monitoringIDEP at index=" << i;
					seg = tmpMonitorEP->segment;
					epAccess->ExecuteAPIMessage(seg);
					
					m_pMonitoringIDEPArray[i] = NULL; // need to test this........
					
					return FALSE;// dtmf-info
				}
			}
			
			return FALSE; // dtmf-info
		} // end dtmf-info
	}
	
	
	return TRUE;
}

void CConnectionsTask::SendToGideonSimForMplApi(CMplMcmsProtocol& rMplProtocol) const
{
#ifndef __DISABLE_ICE__
	resip::Lock lock(m_GideonSimMplApiMutex);
#endif	//__DISABLE_ICE__
	
	CSegment* pParamSeg = new CSegment;
	rMplProtocol.Serialize(*pParamSeg);

#ifndef __DISABLE_ICE__
	m_GideonSimMplApi.SendMsg(pParamSeg, MM_GS_ICE_MSG);
#endif	//__DISABLE_ICE__

	CMplMcmsProtocolTracer(rMplProtocol).TraceMplMcmsProtocol("MM_CONN_TASK_SEND_TO_MPL_API");
	TraceMplMcms(&rMplProtocol);
}

void CConnectionsTask::TraceMplMcms(const CMplMcmsProtocol* pMplProt) const
{
	static const CProcessBase * process = CProcessBase::GetProcess();
	const std::string &OpcodeStr = process->GetOpcodeAsString(pMplProt->getCommonHeaderOpcode());

#ifndef __DISABLE_ICE__
	ICE_LOG_TRACE("MM_CONN_TASK_SEND_TO_MPL_API - REQ_ID: <%d>; HEADER: Board<%d>, SubBoard<%d>, Unit<%d>; OPCODE: <%s>", 
					(int)pMplProt->getMsgDescriptionHeaderRequest_id(),
					(int)pMplProt->getPhysicalInfoHeaderBoard_id(),
					(int)pMplProt->getPhysicalInfoHeaderSub_board_id(),
					(int)pMplProt->getPhysicalInfoHeaderUnit_id(),
					OpcodeStr.c_str());
#endif	//__DISABLE_ICE__

}

//////////////////////////////////////////////////////////////////////////////////////

const std::string& CConnectionsTask::GetOpcodeStr(CMplMcmsProtocol* pMplProtocol) const
{
	static const CProcessBase* process = CProcessBase::GetProcess();
	return process->GetOpcodeAsString(pMplProtocol->getCommonHeaderOpcode());
}

//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//CTimerCompensation


CTimerCompensation::CTimerCompensation()
{
	m_accTime = 0;
	m_lastTimerTime = 0;
	m_compensationCounter = 0;
	m_printCounter = 0;
	m_betweenCompensationCounter = 0;
}

CTimerCompensation::~CTimerCompensation()
{
	
}



