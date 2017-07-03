
#include "MediaMngrManager.h"
#include "MediaMngr.h"
#include "Macros.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "ListenSocketApi.h"
#include "DummyEntry.h"
#include "Request.h"
#include "MplMcmsProtocol.h"
#include "SystemFunctions.h"
#include "psosxml.h"
#include "OperatorDefines.h"
#include "CommMediaMngrSet.h"
#include "SimApi.h"
#include "TerminalCommand.h"


////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CMediaMngrManager)

  ONEVENT( XML_REQUEST    ,IDLE    ,  CMediaMngrManager::HandlePostRequest )
  ONEVENT( SIM_API_H323_MSG,  ANYCASE,  CMediaMngrManager::OnGideonSimMsgAnystate)
  
PEND_MESSAGE_MAP(CMediaMngrManager, CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CMediaMngrManager)

	ON_TRANS("TRANS_CALL_GENERATOR_PARAMS", "SET", CCommSetEPMediaParam, CMediaMngrManager::HandleSetEPMediaParam)
	ON_TRANS("TRANS_CALL_GENERATOR_PARAMS", "RESET_CHANNEL_OUT", CCommResetChannelOut, CMediaMngrManager::HandleResetChannelOut)
	ON_TRANS("TRANS_CALL_GENERATOR_PARAMS", "UPDATE_MEDIA_LIBRARY", CCommMediaLibrary, CMediaMngrManager::HandleUpdateMediaLibrary)
	
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CMediaMngrManager)

	ONCOMMAND("mm_state", CMediaMngrManager::HandleTerminalMMState, "display MediaMngr state")
	//ONCOMMAND("mm_connections", CMediaMngrManager::HandleTerminalMMConnections, "display MediaMngr connections")
	
	// Incoming channel params
	ONCOMMAND("mm_recording", CMediaMngrManager::HandleTerminalMMRecording, "enable MediaMngr recording")
	ONCOMMAND("mm_check_seq_num", CMediaMngrManager::HandleTerminalMMCheckSeqNum, "enable MediaMngr checking correctness of incoming media's sequence number")
	ONCOMMAND("mm_detect_intra", CMediaMngrManager::HandleTerminalMMDetectIntra, "enable MediaMngr detecting incoming intra frame")
	
	ONCOMMAND("mm_reset_channel", CMediaMngrManager::HandleTerminalMMResetChannel, "reset channel out")
	ONCOMMAND("mm_intra", CMediaMngrManager::HandleTerminalMMVideoUpdatePic, "trigger to produce intra request")

END_TERMINAL_COMMANDS

extern void MediaMngrMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void MediaMngrManagerEntryPoint(void* appParam)
{  
	CMediaMngrManager * pMediaMngrManager = new CMediaMngrManager;
	pMediaMngrManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CMediaMngrManager::GetMonitorEntryPoint()
{
	return MediaMngrMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMediaMngrManager::CMediaMngrManager()
{
	m_pMediaMngrConfig = NULL;
	m_pConnectionsTaskApi = NULL;
	m_pMediaRepository = NULL;
}

//////////////////////////////////////////////////////////////////////
CMediaMngrManager::~CMediaMngrManager()
{
	POBJDELETE(m_pMediaMngrConfig);
	POBJDELETE(m_pConnectionsTaskApi);
	POBJDELETE(m_pMediaRepository);
}

//////////////////////////////////////////////////////////////////////
void CMediaMngrManager::ManagerPostInitActionsPoint()
{
	TRACEINTO << "CMediaMngrManager::ManagerPostInitActionsPoint";
	
///	CManagerTask::InitTask();
		
	// init application with XML configuration file
	m_pMediaMngrConfig = new CMediaMngrCfg;
	m_pMediaMngrConfig->Init();
	
	// init Media Repository
	m_pMediaRepository = new CMediaRepository;
	m_pMediaRepository->Init();
		
	// create connections task api
	m_pConnectionsTaskApi = new CConnectionsTaskApi;
	m_pConnectionsTaskApi->Create(*m_pRcvMbx);
}

//////////////////////////////////////////////////////////////////////

void CMediaMngrManager::SelfKill()
{
	TRACEINTO << "CMediaMngrManager::SelfKill - Start Killing Tasks";
	
	if ( CPObject::IsValidPObjectPtr(m_pConnectionsTaskApi) )
	{
		m_pConnectionsTaskApi->Destroy();
		POBJDELETE(m_pConnectionsTaskApi);
	}
	
	SystemSleep(5);
	
	CManagerTask::SelfKill();
}


/////////////////////////////////////////////////////////////////////////////
void CMediaMngrManager::OnGideonSimMsgAnystate(CSegment* pParam)
{
	TRACEINTO << "CMediaMngrManager::OnGideonSimMsgAnystate - send msg to ConnectionsTask";	
	
	if (m_pConnectionsTaskApi == NULL)
	{
		TRACEINTO << "MM ERROR CMediaMngrManager::OnGideonSimMsgAnystate - m_pConnectionsTaskApi==NULL";
		return;
	}	

	CSegment* pMsg =  new CSegment(*pParam);
	m_pConnectionsTaskApi->SendMsg(pMsg, SIM_API_H323_MSG);
}



/////////////////////////////////////////////////////////////////////////////
//  Transaction between Ema-Engine to MediaMngr (Set End-Point Media Params)

STATUS CMediaMngrManager::HandleSetEPMediaParam(CRequest* pRequest)
{
	CCommSetEPMediaParam* pAck = (CCommSetEPMediaParam*)pRequest->GetRequestObject();
	
	DWORD monitorPartyId = pAck->GetMonitorPartyId();
	DWORD monitorConfId = pAck->GetMonitorConfId();
	
	TRACEINTO << "CMediaMngrManager::HandleSetEPMediaParam - monitorPartyId: " << monitorPartyId 
														<< " monitorConfId: " << monitorConfId;
	
	
	CSegment* pMsg =  new CSegment();
	*pMsg << monitorPartyId
	      << monitorConfId
	      << pAck->GetVideoFileName()
	      << pAck->GetAudioFileName()
	      << pAck->GetContentFileName()
	      << pAck->GetSaveVideoFileName()
	      << pAck->GetSaveAudioFileName()
	      << pAck->GetSaveContentFileName();
	      
	
	m_pConnectionsTaskApi->SendMsg(pMsg, SIM_API_MEDIA_MSG);
	
	
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}



/////////////////////////////////////////////////////////////////////////////
//  Transaction between Python to MediaMngr (Reset Video Out)

STATUS CMediaMngrManager::HandleResetChannelOut(CRequest* pRequest)
{
	CCommResetChannelOut* pAck = (CCommResetChannelOut*)pRequest->GetRequestObject();
	
	const char* endpointListStr = pAck->GetEndPointList();
	const char* channelTypeStr = pAck->GetChannelType();
	
	TRACEINTO << "CMediaMngrManager::HandleResetChannelOut - endpointListStr: " << endpointListStr
														   << " channelTypeStr: " << channelTypeStr;
	
	BYTE channelType = 0x0;
	string channel = channelTypeStr;
	
	if (channel == "A")
	{
		channelType |= CHANNEL_TYPE_AUDIO;
	}
	else  if (channel == "V")
	{
		channelType |= CHANNEL_TYPE_VIDEO;
	}
	else  if (channel == "AV" || channel == "VA" || channel == "ALL")
	{
		channelType |= CHANNEL_TYPE_AUDIO;
		channelType |= CHANNEL_TYPE_VIDEO;
	}
	
	
	CSegment* pMsg =  new CSegment();
	*pMsg << endpointListStr
		  << channelType;
	
	m_pConnectionsTaskApi->SendMsg(pMsg, SIM_API_RESET_CHANNEL_OUT_MSG);	
	
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}



/////////////////////////////////////////////////////////////////////////////
//  Transaction between Python to MediaMngr (Update Media Library)

STATUS CMediaMngrManager::HandleUpdateMediaLibrary(CRequest* pRequest)
{
	TRACEINTO << "CMediaMngrManager::HandleUpdateMediaLibrary";
	
	CCommMediaLibrary* pAck = (CCommMediaLibrary*)pRequest->GetRequestObject();
	
	CMediaLibrary* audioLib = pAck->GetAudioLibrary();
	CMediaLibrary* videoLib = pAck->GetVideoLibrary();
	CMediaLibrary* contentLib = pAck->GetContentLibrary();
	
	m_pMediaMngrConfig->SetAudioLibrary(audioLib->CreateCopy());
	m_pMediaMngrConfig->SetVideoLibrary(videoLib->CreateCopy());
	m_pMediaMngrConfig->SetContentLibrary(contentLib->CreateCopy());
	
	
	m_pMediaMngrConfig->WriteXmlFile();
		

	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}



//Terminal Commands
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

STATUS CMediaMngrManager::HandleTerminalMMState(CTerminalCommand & command, std::ostream& answer)
{
	const string& param = command.GetToken(eCmdParam1);
	
	answer << "CMediaMngrManager::HandleTerminalMMState " << param << "\n";
	
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////

/*
STATUS CMediaMngrManager::HandleTerminalMMConnections(CTerminalCommand & command, std::ostream& answer)
{
	const string& param = command.GetToken(eCmdParam1);
	
	answer << "CMediaMngrManager::HandleTerminalMMConnections " << param << "\n";
	
	//m_pConnectionsTaskApi->SendMsg(pMsg, SIM_API_CMD_MEDIA_MSG);
	
	return STATUS_OK;
}
*/

/////////////////////////////////////////////////////////////////////

STATUS CMediaMngrManager::HandleTerminalMMRecording(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "error: number of parameters\n";
		answer << "usage: Bin/McuCmd mm_recording MediaMngr [A | V] [ON | OFF]\n";
		return STATUS_FAIL;
	}
	
	
	const string& channel = command.GetToken(eCmdParam1);
	const string& recording = command.GetToken(eCmdParam2);
	
	BOOL bOnOff = FALSE;
	
	if (recording == "ON")
	{
		bOnOff = TRUE;
	}
	else if (recording == "OFF")
	{
		bOnOff = FALSE;
	}
	else
	{
		answer << "error: recording not supported!";
		return STATUS_FAIL;
	}
	
	
	if (channel == "A")
	{
		SetAudioRecording(bOnOff);
	}
	else  if (channel == "V")
	{
		SetVideoRecording(bOnOff);
	}
	else  if (channel == "AV" || channel == "VA")
	{
		SetAudioRecording(bOnOff);
		SetVideoRecording(bOnOff);
	}
	else
	{
		answer << "error: channel not supported!";
		return STATUS_FAIL;
	}
	
	answer << "CMediaMngrManager::HandleTerminalMMRecording Channel: " << channel << " Recording: " << recording << "\n";
	
	
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////

STATUS CMediaMngrManager::HandleTerminalMMCheckSeqNum(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "error: number of parameters\n";
		answer << "usage: Bin/McuCmd mm_check_seq_num MediaMngr [A | V] [ON | OFF]\n";
		return STATUS_FAIL;
	}
	
	
	const string& channel = command.GetToken(eCmdParam1);
	const string& checkSeqNum = command.GetToken(eCmdParam2);
	
	BOOL bOnOff = FALSE;
	
	if (checkSeqNum == "ON")
	{
		bOnOff = TRUE;
	}
	else if (checkSeqNum == "OFF")
	{
		bOnOff = FALSE;
	}
	else
	{
		answer << "error: checking sequence number state is not supported!";
		return STATUS_FAIL;
	}
	
	
	if (channel == "A")
	{
		SetAudioCheckSeqNum(bOnOff);
	}
	else  if (channel == "V")
	{
		SetVideoCheckSeqNum(bOnOff);
	}
	else  if (channel == "AV" || channel == "VA")
	{
		SetAudioCheckSeqNum(bOnOff);
		SetVideoCheckSeqNum(bOnOff);
	}
	else
	{
		answer << "error: channel not supported!";
		return STATUS_FAIL;
	}
	
	answer << "CMediaMngrManager::HandleTerminalMMCheckSeqNum Channel: " << channel << " Check sequence number: " << checkSeqNum << "\n";
	
	
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////

STATUS CMediaMngrManager::HandleTerminalMMDetectIntra(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: number of parameters\n";
		answer << "usage: Bin/McuCmd mm_detect_intra MediaMngr [ON | OFF]\n";
		return STATUS_FAIL;
	}
	
	const string& detectIntra = command.GetToken(eCmdParam1);
	
	BOOL bOnOff = FALSE;
	
	if (detectIntra == "ON")
	{
		bOnOff = TRUE;
	}
	else if (detectIntra == "OFF")
	{
		bOnOff = FALSE;
	}
	else
	{
		answer << "error: detect intra state is not supported!";
		return STATUS_FAIL;
	}
	
	SetDetectIntra(bOnOff);
	
	answer << "CMediaMngrManager::HandleTerminalMMDetectIntra" << " Detect Intra: " << detectIntra << "\n";
	
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////

STATUS CMediaMngrManager::HandleTerminalMMResetChannel(CTerminalCommand & command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(numOfParams != 2)
	{
		answer << "error: number of parameters\n";
		answer << "usage: Bin/McuCmd mm_reset_channel MediaMngr [A|V|ALL] [{1-800}|ALL]\n";
		return STATUS_FAIL;
	}
	
	const string& channel = command.GetToken(eCmdParam1);
	const string& endpointList = command.GetToken(eCmdParam2);
	
	BYTE channelType = 0x0;
	
	if (channel == "A")
	{
		channelType |= CHANNEL_TYPE_AUDIO;
	}
	else  if (channel == "V")
	{
		channelType |= CHANNEL_TYPE_VIDEO;
	}
	else  if (channel == "AV" || channel == "VA" || channel == "ALL")
	{
		channelType |= CHANNEL_TYPE_AUDIO;
		channelType |= CHANNEL_TYPE_VIDEO;
	}
	else
	{
		answer << "error: channel not supported!";
		return STATUS_FAIL;
	}

	
	CSegment* pMsg =  new CSegment();
	*pMsg << endpointList.c_str()
	      << channelType;
	
	m_pConnectionsTaskApi->SendMsg(pMsg, SIM_API_RESET_CHANNEL_OUT_MSG);	

	
	answer << "CMediaMngrManager::HandleTerminalMMResetChannel endpointList: " << endpointList.c_str() << "\n";
	
	
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////


STATUS CMediaMngrManager::HandleTerminalMMVideoUpdatePic(CTerminalCommand &command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if(numOfParams != 0)
	{
		answer << "error: number of parameters, should be 0\n";
		answer << "usage: Bin/McuCmd mm_intra MediaMngr \n";
		return STATUS_FAIL;
	}
	
	//const string& channel = command.GetToken(eCmdParam1);
	
	CSegment* pMsg =  new CSegment();
	
	m_pConnectionsTaskApi->SendMsg(pMsg, SIM_API_VIDEO_UPDATE_PIC_MSG);	
	
	answer << "CMediaMngrManager::HandleTerminalMMIntra \n";
	
	return STATUS_OK;
}




/////////////////////////////////////////////////////////////////////
/////////          Externals Methods
/////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// Recording flags
////////////////////////////////////////////////////////

BOOL GetAudioRecording()
{
//	BOOL retVal = g_bAudioRecording;
//	g_bAudioRecording = FALSE;

	BOOL retVal = FALSE;
	if (g_tIncomingAudioChannelParam.bReadFlag)
		retVal = g_tIncomingAudioChannelParam.bWriteMedia;
	
	// unset flags for next channel connection
	if (g_tIncomingAudioChannelParam.bWriteMedia)
	{
		g_tIncomingAudioChannelParam.bReadFlag = FALSE;
		g_tIncomingAudioChannelParam.bWriteMedia = FALSE;
	}
	
	return retVal;
}

////////////////////////////////////////////////////////
void SetAudioRecording(BOOL audioRecording)
{
//	g_bAudioRecording = audioRecording;

	g_tIncomingAudioChannelParam.bReadFlag = audioRecording;
	g_tIncomingAudioChannelParam.bWriteMedia = audioRecording;
}

////////////////////////////////////////////////////////
BOOL GetVideoRecording()
{	
//	BOOL retVal = g_bVideoRecording;
//	g_bVideoRecording = FALSE;	

	BOOL retVal = FALSE;
	if (g_tIncomingVideoChannelParam.bReadFlag)
		retVal = g_tIncomingVideoChannelParam.bWriteMedia;
	
	// unset flags for next channel connection
	if (g_tIncomingVideoChannelParam.bWriteMedia)
	{
		g_tIncomingVideoChannelParam.bReadFlag = FALSE;
		g_tIncomingVideoChannelParam.bWriteMedia = FALSE;
	}
	
	return retVal;
}

////////////////////////////////////////////////////////
void SetVideoRecording(BOOL videoRecording)
{
//	g_bVideoRecording = videoRecording;

	g_tIncomingVideoChannelParam.bReadFlag = videoRecording;
	g_tIncomingVideoChannelParam.bWriteMedia = videoRecording;
}

////////////////////////////////////////////////////////
// Checking sequence number
////////////////////////////////////////////////////////

BOOL GetAudioCheckSeqNum()
{
	BOOL retVal = FALSE;
	if (g_tIncomingAudioChannelParam.bReadFlag)
		retVal = g_tIncomingAudioChannelParam.bCheckSeqNumber;
	
	// unset flags for next channel connection
	if (g_tIncomingAudioChannelParam.bCheckSeqNumber)
	{
		g_tIncomingAudioChannelParam.bReadFlag = FALSE;
		g_tIncomingAudioChannelParam.bCheckSeqNumber = FALSE;
	}
	
	return retVal;
}

////////////////////////////////////////////////////////
void SetAudioCheckSeqNum(BOOL audioCheckSeqNum)
{
	g_tIncomingAudioChannelParam.bReadFlag = audioCheckSeqNum;
	g_tIncomingAudioChannelParam.bCheckSeqNumber = audioCheckSeqNum;
}

////////////////////////////////////////////////////////
BOOL GetVideoCheckSeqNum()
{	
	BOOL retVal = FALSE;
	if (g_tIncomingVideoChannelParam.bReadFlag)
		retVal = g_tIncomingVideoChannelParam.bCheckSeqNumber;
	
	// unset flags for next channel connection
	if (g_tIncomingVideoChannelParam.bCheckSeqNumber)
	{
		g_tIncomingVideoChannelParam.bReadFlag = FALSE;
		g_tIncomingVideoChannelParam.bCheckSeqNumber = FALSE;
	}
	
	return retVal;
}

////////////////////////////////////////////////////////
void SetVideoCheckSeqNum(BOOL videoCheckSeqNum)
{
	g_tIncomingVideoChannelParam.bReadFlag = videoCheckSeqNum;
	g_tIncomingVideoChannelParam.bCheckSeqNumber = videoCheckSeqNum;
}


////////////////////////////////////////////////////////
// Detecting Intra
////////////////////////////////////////////////////////

BOOL GetDetectIntra()
{
	BOOL retVal = FALSE;
	if (g_tIncomingVideoChannelParam.bReadFlag)
		retVal = g_tIncomingVideoChannelParam.bDetectIntra;
	
	// unset flags for next channel connection
	if (g_tIncomingVideoChannelParam.bDetectIntra)
	{
		g_tIncomingVideoChannelParam.bReadFlag = FALSE;
		g_tIncomingVideoChannelParam.bDetectIntra = FALSE;
	}
	
	return retVal;
}

////////////////////////////////////////////////////////
void SetDetectIntra(BOOL detectIntra)
{
	g_tIncomingVideoChannelParam.bReadFlag = detectIntra;
	g_tIncomingVideoChannelParam.bDetectIntra = detectIntra;
}





////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

void SendMessageToGideonSimApp(CSegment& rParam)
{
	CSegment*  pMsg = new CSegment(rParam);

	const COsQueue* pGideonMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessGideonSim,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*pGideonMbx);
	api.SendMsg(pMsg, MM_INDICATIONS_MSG);
	api.DestroyOnlyApi();
}


