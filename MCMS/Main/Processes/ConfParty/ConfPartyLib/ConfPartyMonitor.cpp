// ConfPartyMonitor.cpp: implementation of the ConfPartyMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "NStream.h"
#include "ConfPartyMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "StatusesGeneral.h"
#include "CommResDBGet.h"
#include "GetProfileSpecific.h"
#include "FileList.h"
#include "MeetingRoomDBGet.h"
#include "GetMRSpecific.h"
#include "ApiStatuses.h"
#include "ConfApi.h"
#include "RecordingLinkDBGet.h"
#include "ConfTemplateDBGet.h"
#include "GetConfTempSpecific.h"
#include "ConfPartyProcess.h"
#include "ConfPartyGlobals.h"
#include "ResRsrcCalculator.h"
#include "DummyEntry.h"
#include "CustomizeDisplaySettingForOngoingConfConfigurationDBGet.h"
#include "CGetDynamicContentRateResTable.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"
#include "IVRServiceGetConversionStatus.h"

static DWORD dwGetCounter = 0;
static DWORD dwGetLsCounter = 0;

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CConfPartyMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    ,  CConfPartyMonitor::HandlePostRequest )
  ONEVENT(GET_CONF_INFO_TIMER    ,IDLE	,    CConfPartyMonitor::OnTimerGetConfInformationForSNMP)
PEND_MESSAGE_MAP(CConfPartyMonitor,CMonitorTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CConfPartyMonitor)
	ON_TRANS("TRANS_CONF_LIST"            ,"GET_LS"               	 ,CCommConfDBGet      ,CConfPartyMonitor::OnServerConfList) //What is th meening CCommConfDBGet
	ON_TRANS("TRANS_CONF_2"               ,"GET"                  	 ,CCommConfSpecific   ,CConfPartyMonitor::OnServerConfSpecific)
	ON_TRANS("TRANS_PARTY"                ,"GET"                  	 ,CConfPartySpecific  ,CConfPartyMonitor::OnServerPartySpecific)
	ON_TRANS("TRANS_RES_LIST"             ,"GET_PROFILE_LIST"     	 ,CCommResDBGet       ,CConfPartyMonitor::OnServerProfileList)
	ON_TRANS("TRANS_RES_2"                ,"GET_PROFILE"          	 ,CGetProfileSpecific ,CConfPartyMonitor::OnServerGetProfileReq)
	ON_TRANS("TRANS_MCU"                  ,"GET_DIRECTORY"        	 ,CFileListGet        ,CConfPartyMonitor::OnServerFileList) // Check if the same action is received in other cases (not only IVR languages).
	ON_TRANS("TRANS_RES_LIST"             ,"GET_MEETING_ROOM_LIST"	 ,CMeetingRoomDBGet   ,CConfPartyMonitor::OnServerReqMRList)
	ON_TRANS("TRANS_RES_2"                ,"GET_MEETING_ROOM"     	 ,CGetMRSpecific      ,CConfPartyMonitor::OnServerGetMeetingRoomReq)
	ON_TRANS("TRANS_AV_MSG_SERVICE_LIST"  ,"GET_IVR_LIST"         	 ,CIVRServiceListGet  ,CConfPartyMonitor::OnServerIVRServiceList)
	ON_TRANS("TRANS_RECORDING_LINKS_LIST" ,"GET"                  	 ,CRecordingLinkDBGet ,CConfPartyMonitor::OnServerRecordingLinkList)
	ON_TRANS("TRANS_RES_LIST"             ,"GET_CONFERENCE_TEMPLATE_LIST"  ,CConfTemplateDBGet ,CConfPartyMonitor::OnServerConfTemplateList)
	ON_TRANS("TRANS_RES_2"                ,"GET_CONFERENCE_TEMPLATE"       ,CConfTempSpecific  ,CConfPartyMonitor::OnServerGetConfTemplateReq)
	//FailOver
	ON_TRANS("TRANS_CONF_LIST"            ,"GET_FULL_CONF_CHANGE_LS" ,CCommConfDBGetFull  ,CConfPartyMonitor::OnServerConfListFull)

	ON_TRANS("TRANS_MCU"                  ,"GET_RESOLUTIONS_SET"           ,CDummyEntry                    ,CConfPartyMonitor::OnServerGetResolutionThreshold)
	ON_TRANS("TRANS_MCU"                  ,"GET_DYNAMIC_CONTENT_RATE_TABLE",CGetDynamicContentRateResTable ,CConfPartyMonitor::OnServerGetContentRatingTableForCascade)
    ON_TRANS("TRANS_MCU"                  ,"GET_CUSTOMIZED_CONTENT_RATE_TABLE",CGetCustomizedContentRateResTable,CConfPartyMonitor::OnServerGetFixedContentRatingTableForCascade)
	ON_TRANS("TRANS_MCU"                  ,"GET_DYNAMIC_HP_CONTENT_RATE_TABLE",CGetDynamicContentRateResTable ,CConfPartyMonitor::OnServerGetContentRatingTableForCascade)
    ON_TRANS("TRANS_MCU"                  ,"GET_CUSTOMIZED_HP_CONTENT_RATE_TABLE",CGetCustomizedContentRateResTable,CConfPartyMonitor::OnServerGetFixedContentRatingTableForCascade)
//    ON_TRANS("TRANS_CONF_2"     		  ,"GET_CONF_RELAY_INFO"	 		,CConfRelayMediaInfoGet				,CConfPartyMonitor::OnServerGetConfRelayMediaInfo)

    ON_TRANS("TRANS_AV_MSG_SERVICE"       ,"GET_CONVERSION_STATUS"   ,CIVRServiceGetConversionStatus,    CConfPartyMonitor::OnServerGetConversionStatus)

	ON_TRANS("TRANS_CUSTOMIZE_SETUP_ONGOING_CONF","GET",CCustomizeDisplaySettingForOngoingConfConfigurationDBGet,CConfPartyMonitor::OnServerGetCustomizeDisplayForConf)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void ConfPartyMonitorEntryPoint(void* appParam)
{
	CConfPartyMonitor *monitorTask = new CConfPartyMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfPartyMonitor::CConfPartyMonitor()
{
	PTRACE(eLevelInfoNormal, "I am the Conference Party Monitor - my goal is to handle all Get Req");
}


//////////////////////////////////////////////////////////////////////////
CConfPartyMonitor::~CConfPartyMonitor()
{

}

void  CConfPartyMonitor::InitTask() 
{
    CMonitorTask::InitTask();
	
	TRACEINTO << "CConfPartyMonitor::Start a timer, yuhui \n";
	StartTimer(GET_CONF_INFO_TIMER, GET_CONF_INFO_TIMER_TIME_OUT_VALUE);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerConfList(CRequest* pGetRequest)
{
	dwGetLsCounter++;
	if (20 == dwGetLsCounter)
	{
		TRACEINTO << "GET_LS 20 times";
		dwGetLsCounter = 0;
	}

	STATUS status = STATUS_OK;

	CCommConfDBGet* pCommConfDBGet = new CCommConfDBGet;
	*pCommConfDBGet = *(CCommConfDBGet*)pGetRequest->GetRequestObject() ;
	pGetRequest->SetConfirmObject(pCommConfDBGet);

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerConfListFull(CRequest* pGetRequest)
{
	STATUS status = STATUS_OK;

	CCommConfDBGetFull* pCommConfDBGetFull = new CCommConfDBGetFull();
	*pCommConfDBGetFull = *(CCommConfDBGetFull*)pGetRequest->GetRequestObject();
	pGetRequest->SetConfirmObject(pCommConfDBGetFull);

 	return status;
}

/*
////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerGetConfRelayMediaInfo(CRequest* pGetRequest)
{
	// phyton:  Scripts/TestConfRelayInfoGet2.py		--> send XML to the firt conf
	// phyton:  Scripts/TestConfRelayInfoGetAll.py      --> send XML to every conf that currently exist
	// XML:		Scripts/TransGetConfRelayMediaInfo.xml  --> XML with the suitable action

	PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerGetConfRelayPartiesMediaInfo - mmmmmm  - Start");
	STATUS status = STATUS_OK;

	CConfRelayMediaInfoGet* pConfRelayMediaInfo = new CConfRelayMediaInfoGet;
	*pConfRelayMediaInfo = *(CConfRelayMediaInfoGet*)pGetRequest->GetRequestObject() ;

	const DWORD confID = pConfRelayMediaInfo->GetConfID();

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confID);

	if (!pCommConf)
	{
	  PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerGetConfRelayPartiesMediaInfo - mmmmmm - Eror Getting Conf-ID");
 	  status=STATUS_CONF_NOT_EXISTS;
	}
	else
	{
		if (pCommConf->GetNumParties()>MAX_PARTIES_IN_CONF)
		{
			PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerGetConfRelayPartiesMediaInfo - mmmmmm - Eror Max-ParticipantsD");
			status=STATUS_ILLEGAL;
		}
		else
		{
			pCommConf->m_updateCounter = 1;	// xxxAMir
			pConfRelayMediaInfo->SetCommConf(pCommConf);
		}

	}

	std::string responseTrancsName("TRANS_CONF"); //instead of TRANS_CONF_2
	pGetRequest->SetTransName(responseTrancsName);
	pGetRequest->SetConfirmObject(pConfRelayMediaInfo);
	pGetRequest->SetStatus(status);

	return status;
}
*/

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerConfSpecific(CRequest* pGetRequest)
{
	dwGetCounter++;
	if (100 == dwGetCounter)
	{
		TRACEINTO << "GET 100 times";
		dwGetCounter = 0;
	}

	STATUS status = STATUS_OK;

	CCommConfSpecific* pCommConfSpecific = new CCommConfSpecific;
	*pCommConfSpecific = *(CCommConfSpecific*)pGetRequest->GetRequestObject() ;

	const DWORD confID = pCommConfSpecific->GetConfID();

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(confID);

	if (!pCommConf)
	{
  	  status=STATUS_CONF_NOT_EXISTS;
	  //getConfirm.SetStatus(status); //To see how to return the status to getconfirm
	}
	else
	{
		if (pCommConf->GetNumParties()>MAX_PARTIES_IN_CONF)
		{
			status=STATUS_ILLEGAL;
			//getConfirm.SetStatus(status);
		}
		else
		{
			pCommConf->m_updateCounter = 1;//pCommConfSpecific->m_updateCounter;//1;
			//SetUpdateFlag(YES);Confirm Flag to see if needed in the carmel

		}
		if (pCommConf->IsConfSecured())
		{
			status=STATUS_CONF_IS_SECURED;
		}

	}

	std::string responseTrancsName("TRANS_CONF"); //insteade of TRANS_CONF_2
	pGetRequest->SetTransName(responseTrancsName);
	//pGetRequest->SetConfirmStatus(sendStatus);//to write function for Request object
	//*pCommConfSpecific = *(CCommConfSpecific*)pGetRequest->GetRequestObject() ; //To set the updateCounter
	pGetRequest->SetConfirmObject(pCommConfSpecific);
	pGetRequest->SetStatus(status);

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerPartySpecific(CRequest* pGetRequest)
{
	//PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerPartySpecific  ");
	STATUS status = STATUS_OK;

	CConfPartySpecific* pConfPartySpecific = new CConfPartySpecific;
	*pConfPartySpecific = *(CConfPartySpecific*)pGetRequest->GetRequestObject() ;

	const DWORD confID = pConfPartySpecific->GetConfID();
	const DWORD partyID = pConfPartySpecific->GetPartyID();

	const CConfParty* pConfParty = ::GetpConfDB()->GetCurrentParty(confID,partyID);
	CCommConf* pCurCommConf = (CCommConf*) ::GetpConfDB()->GetCurrentConf(confID);

	if (pCurCommConf && pCurCommConf->IsConfSecured())
	{
		status=STATUS_CONF_IS_SECURED;
	}

	if(status == STATUS_OK)
	{
		if (!pConfParty || !pCurCommConf)
	  	{
	  	  status=STATUS_PARTY_DOES_NOT_EXIST;
		  //getConfirm.SetStatus(status); //To see how to return the status to getconfirme
		}
		else
	  	{
			CProcessBase* pProcess = CProcessBase::GetProcess();
			if (pProcess->GetIsFailoverSlaveMode() == FALSE) //no CConfApi in slave mode
			{
	    	//Sending Party monitoring Reques to the Conf which will forward it to the specified party
		    CConfApi confApi;
		    confApi.CreateOnlyApi(*(pCurCommConf->GetRcvMbx()));
	    	confApi.SendIpMonitoringEventToParty(partyID, pConfParty->GetName());
			}
		}
	}
	//pGetRequest->SetConfirmStatus(sendStatus);//to write function for Request object

	pGetRequest->SetConfirmObject(pConfPartySpecific);
	pGetRequest->SetStatus(status);

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerProfileList(CRequest* pGetRequest)
{
//	PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerProfileList  Got request for profile list");

	STATUS status = STATUS_OK;

	CCommResDBGet* pCommResDBGet = new CCommResDBGet;

	*pCommResDBGet = * (CCommResDBGet*)pGetRequest->GetRequestObject() ;

	pGetRequest->SetConfirmObject(pCommResDBGet);
 	return status;
}

/////////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerGetProfileReq(CRequest* pGetRequest)
{

//	PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerGetProfileReq  Got request for specified profile");

	STATUS status = STATUS_OK;

	CGetProfileSpecific * pProfileSpecific = new CGetProfileSpecific;

	*pProfileSpecific = * (CGetProfileSpecific*)pGetRequest->GetRequestObject();

	const DWORD profileId = pProfileSpecific->GetConfID();

	CCommRes*  pProfile= ::GetpProfilesDB()->GetCurrentRsrv(profileId);

	if (!pProfile)
	{
  	  status=STATUS_CONF_NOT_EXISTS;
	  //getConfirm.SetStatus(status); //To see how
	}
	else
	  POBJDELETE(pProfile);

	std::string responseTrancsName("TRANS_RES"); //insteade of TRANS_RES_2
	pGetRequest->SetTransName(responseTrancsName);

	pGetRequest->SetConfirmObject(pProfileSpecific);
	pGetRequest->SetStatus(status);

 	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerIVRServiceList(CRequest* pGetRequest)
{
	STATUS status = STATUS_OK;

	CIVRServiceListGet* pIVRServiceListGet = new CIVRServiceListGet;



	*pIVRServiceListGet = *(CIVRServiceListGet*)pGetRequest->GetRequestObject() ; //To set the updateCounter
	pGetRequest->SetConfirmObject(pIVRServiceListGet);

 	return status;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerFileList(CRequest* pGetRequest)
{
	STATUS status = STATUS_OK;

  	// Create instance of CFileList
	CFileListGet* pFileListGetTemp = (CFileListGet*)pGetRequest->GetRequestObject();
	if (NULL == pFileListGetTemp)
	{
		PTRACE(eLevelError,"CConfPartyMonitor::OnServerFileList - GetRequestObject failed");
		return STATUS_FAIL;
	}
	CFileListGet* pFileListGet = new CFileListGet;
	*pFileListGet = *pFileListGetTemp;

	// Get path
	char* path = pFileListGet->GetPath();
	WORD tempLen = 0;

	/// Temporary - Anat - for MGC manager compatibility
	WORD tempPathLen = strlen(path);
	tempLen = strlen("7.256/mcu");
	WORD tempLen1 = strlen("Cfg/IVR");
	if (0 == strncmp(path, "7.256/mcu", tempLen))
	{
		strncpy(path, "Cfg/IVR", tempLen1);
		strncpy(&path[tempLen1], &path[tempLen], tempPathLen - tempLen);
		path[tempPathLen - tempLen + tempLen1] = '\0';
	}
	/// Temporary - Anat

	WORD pathLen = strlen(path);
	WORD bNested = FALSE;
	tempLen = strlen("#feature#");
	if (!strncmp(&path[pathLen - tempLen], "#feature#", tempLen))
	{
		bNested = TRUE;
		path[pathLen - tempLen] = '\0';
	}

	// path should be ended with '/' for the GetFirst/NextFile functions
	strcat(path,"/");

	CFileList* pFileList = pFileListGet->GetFileList();
	status = pFileList->FillFileList(path, bNested);
	if (STATUS_OK != status)
		PTRACE2(eLevelError,"CConfPartyMonitor::OnServerIVRLangsList : Failed to open directory ",path);

	pGetRequest->SetStatus(status);//VNGR-11368 (User interface changes not saved)
	pGetRequest->SetConfirmObject(pFileListGet);

 	return status;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerReqMRList(CRequest* pGetRequest)
{
//	PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerReqMRList  Got request for meeting room list");

	STATUS status = STATUS_OK;

	CMeetingRoomDBGet* pMRDBGet = new CMeetingRoomDBGet;

	*pMRDBGet = * (CMeetingRoomDBGet*)pGetRequest->GetRequestObject() ;

	pGetRequest->SetConfirmObject(pMRDBGet);
 	return status;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerGetMeetingRoomReq(CRequest* pGetRequest)
{

//	PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerGetMeetingRoomReq  Got request for specified meeting room");

	STATUS status = STATUS_OK;

	CGetMRSpecific * pMRSpecific = new CGetMRSpecific;

	*pMRSpecific = * (CGetMRSpecific*)pGetRequest->GetRequestObject();

	const DWORD mrId = pMRSpecific->GetMRID();

	CCommRes*  pMR = ::GetpMeetingRoomDB()->GetCurrentRsrv(mrId);

	if (!pMR)
	{
  	  status=STATUS_CONF_NOT_EXISTS;
	  //getConfirm.SetStatus(status); //To see how
	}
	POBJDELETE(pMR);

	std::string responseTrancsName("TRANS_RES"); //insteade of TRANS_RES_2
	pGetRequest->SetTransName(responseTrancsName);

	pGetRequest->SetConfirmObject(pMRSpecific);
	pGetRequest->SetStatus(status);
 	return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerRecordingLinkList(CRequest* pGetRequest)
{
	STATUS status = STATUS_OK;
	CRecordingLinkDBGet* pRecordingLinkDBGet = new CRecordingLinkDBGet;


	*pRecordingLinkDBGet = *(CRecordingLinkDBGet*)pGetRequest->GetRequestObject() ; //To set the updateCounter

	pGetRequest->SetConfirmObject(pRecordingLinkDBGet);

	pGetRequest->SetStatus(status);
 	return status;
}
///////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerConfTemplateList(CRequest* pGetRequest)
{
	//PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerConfTemplateList  Got request for Conf Template list");

	STATUS status = STATUS_OK;
	CConfTemplateDBGet* pConfTemplateDBGet = new CConfTemplateDBGet;

	*pConfTemplateDBGet = * (CConfTemplateDBGet*)pGetRequest->GetRequestObject() ;

	pGetRequest->SetConfirmObject(pConfTemplateDBGet);
	return status;
}
///////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerGetConfTemplateReq(CRequest* pGetRequest)
{
//	PTRACE(eLevelInfoNormal, "CConfPartyMonitor::OnServerGetConfTemplateReq   Got request for specified Conf Tempalte");

	STATUS status = STATUS_OK;

	CConfTempSpecific* pConfTempSpecific = new CConfTempSpecific;

	*pConfTempSpecific = * (CConfTempSpecific*)pGetRequest->GetRequestObject();

	const DWORD confTemplateId = pConfTempSpecific->GetConfTemplateID();

	CCommRes*  pConfTemplate = ::GetpConfTemplateDB()->GetCurrentRsrv(confTemplateId);

	if (!pConfTemplate)
	{
	  status=STATUS_CONF_NOT_EXISTS;
		  //getConfirm.SetStatus(status); //To see how
    }
	POBJDELETE(pConfTemplate);

	std::string responseTrancsName("TRANS_RES"); //insteade of TRANS_RES_2
	pGetRequest->SetTransName(responseTrancsName);

    pGetRequest->SetConfirmObject(pConfTempSpecific);
	pGetRequest->SetStatus(status);
	return STATUS_OK;
}
//=====================================================================================================================================//
STATUS CConfPartyMonitor::OnServerGetResolutionThreshold(CRequest* pGetRequest)
{
//  PTRACE(eLevelInfoNormal,"CConfPartyMonitor::OnServerGetResolutionThreshold ");

    CResRsrcCalculator resRsrcCalculator;
    CResolutionSliderDetails* pResolutionSlider = new CResolutionSliderDetails( GetSystemCardsBasedMode() );
    resRsrcCalculator.GetResolutionSliderDetails( GetSystemCardsBasedMode(), pResolutionSlider );
	pGetRequest->SetConfirmObject( pResolutionSlider );

	return STATUS_OK;
}

///////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerGetCustomizeDisplayForConf(CRequest* pGetRequest)
{
//	PTRACE(eLevelInfoNormal,"CConfPartyMonitor::OnServerGetCustomizeDisplayForConf ");
	//printf("CConfPartyMonitor::OnServerGetCustomizeDisplayForConf \n");
    	CCustomizeDisplaySettingForOngoingConfConfigurationDBGet* pConfDisplaySettingDBGet = new CCustomizeDisplaySettingForOngoingConfConfigurationDBGet();
	*pConfDisplaySettingDBGet = *(CCustomizeDisplaySettingForOngoingConfConfigurationDBGet*)pGetRequest->GetRequestObject() ;

	pGetRequest->SetConfirmObject( pConfDisplaySettingDBGet );

	return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////
STATUS CConfPartyMonitor::OnServerGetContentRatingTableForCascade(CRequest* pGetRequest)
{
	STATUS status = STATUS_OK;
	CGetDynamicContentRateResTable* pDynamicContentRateResTable = new CGetDynamicContentRateResTable;

	*pDynamicContentRateResTable = * (CGetDynamicContentRateResTable*)pGetRequest->GetRequestObject() ;

	pGetRequest->SetConfirmObject(pDynamicContentRateResTable);
	return status;
}

STATUS CConfPartyMonitor::OnServerGetFixedContentRatingTableForCascade(CRequest* pGetRequest)
{
	STATUS status = STATUS_OK;
	CGetCustomizedContentRateResTable* pTable = new CGetCustomizedContentRateResTable(* (CGetCustomizedContentRateResTable*)pGetRequest->GetRequestObject());

	pGetRequest->SetConfirmObject(pTable);
	return status;
}

void CConfPartyMonitor::OnTimerGetConfInformationForSNMP()
{
	CConfPartyProcess* pProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();
	FPASSERT_AND_RETURN(!pProcess);
	
	if(pProcess->GetIsSNMPEnabled() == FALSE)
	{
		StartTimer(GET_CONF_INFO_TIMER, GET_CONF_INFO_TIMER_DISABLE_STATE_TIME_OUT_VALUE);
		return;
	}

	CCommConfDB* pCommConfDB = ::GetpConfDB();
	DWORD nAcitveConfNumber = 0;
	nAcitveConfNumber = pCommConfDB->GetConfNumber();

	DWORD nAcitvePartyNumber = 0;
	DWORD nAcitveAudioPartyNumber = 0;
	DWORD nAcitveVideoPartyNumber = 0;

	int i;
	int CommPos = 0;
	CCommConf* TempCommConf=::GetpConfDB()->GetFirstCommConf(CommPos);
	while(TempCommConf !=NULL)
	{
		for (i=0;i<(int)TempCommConf->GetNumParties();i++)
		{
		   if (!i)
		   {
			 CConfParty* pConfParty = TempCommConf->GetFirstParty();
			 if(PARTY_CONNECTED == pConfParty->GetPartyState())
			 {
			 	nAcitvePartyNumber++;
				if(pConfParty->GetVoice())
				{
					nAcitveAudioPartyNumber++;
				}
				else
				{
					nAcitveVideoPartyNumber++;
				}
			 }
		   }
		   else
		   {
			 CConfParty* pConfParty = TempCommConf->GetNextParty();
			 if(!pConfParty)
			 {
			 	PASSERT(1);
			 	continue;
			 }
			 if(PARTY_CONNECTED == pConfParty->GetPartyState())
			 {
			 	nAcitvePartyNumber++;
				if(pConfParty->GetVoice())
				{
					nAcitveAudioPartyNumber++;
				}
				else
				{
					nAcitveVideoPartyNumber++;
				}
			 }
		   }
		}

		TempCommConf=::GetpConfDB()->GetNextCommConf(CommPos);
	}

	//Maximum resolution for CP
	DWORD nResolution = CResRsrcCalculator::GetResolutionType();
	if((e_hd1080p60 == nResolution)||(e_hd1080p30 == nResolution))
	{
		nResolution = 1;
	}
	else if((e_hd720p60 == nResolution)||(e_hd720p30 == nResolution))
	{
		nResolution = 2;
	}
	else if(e_sd30 == nResolution)
	{
		nResolution = 4;	
	}
	else if(e_sd15 == nResolution)
	{
		nResolution = 5;	
	}
	else if(e_cif30 == nResolution)
	{
		nResolution = 6;	
	}
	else
	{
		nResolution = 7;	
	}
	
	//Resolution configuration for CP	
	DWORD nConfigType = CResRsrcCalculator::GetResolutionConfigType();
	if(e_balanced == nConfigType)
	{
		nConfigType = 1;
	}
	else if(e_resource_optimized == nConfigType)
	{
		nConfigType = 2;
	}
	else if(e_user_exp_optimized == nConfigType)
	{
		nConfigType = 3;
	}
	else if(e_hi_profile_optimized == nConfigType)
	{
		nConfigType = 5;	
	}
	else if(e_manual == nConfigType)
	{
		nConfigType = 4;
	}
	
	//The minimum bit rate required by endpoints to connect to an HD conference	
	eSystemCardsMode systemCardMode = GetSystemCardsBasedMode();
    DWORD nRate = CResRsrcCalculator::GetHDBitRateThrshld(systemCardMode);

	TRACESTRFUNC (eLevelDebug) << "Number of Active Conf: " << int(nAcitveConfNumber) << ", Number of Active Party: " << int(nAcitvePartyNumber)
			                   << "\nnResolution: " << int(nResolution) << ", nConfigType: " << int(nConfigType)<< ", nRate: " << nRate;
	//7 of parmeter
	DWORD nCount = 7;
	CSegment* pSeg = new CSegment;
	*pSeg << nCount;

	DWORD nTemp0 = 0;
	DWORD nTemp1 = 0;

	// Active Conf
	nTemp0 = eTT_ActiveConf;
	nTemp1 = nAcitveConfNumber;
	*pSeg << nTemp0 << nTemp1;
	
	// Active Party
	nTemp0 = eTT_ActiveParticipant;
	nTemp1 = nAcitvePartyNumber;
	*pSeg << nTemp0 << nTemp1;

	// Active Voice Party
	nTemp0 = eTT_VoiceParticipants;
	nTemp1 = nAcitveAudioPartyNumber;
	*pSeg << nTemp0 << nTemp1;

	// Active Video Party
	nTemp0 = eTT_VideoParticipants;
	nTemp1 = nAcitveVideoPartyNumber;
	*pSeg << nTemp0 << nTemp1;

	//Maximum resolution for CP
	nTemp0 = eTT_MaxCPRstln;
	nTemp1 = nResolution;
	*pSeg << nTemp0 << nTemp1;

	//Resolution configuration for CP	
	nTemp0 = eTT_MaxCPRstlnCfg;
	nTemp1 = nConfigType;
	*pSeg << nTemp0 << nTemp1;

	//The minimum bit rate required by endpoints to connect to an HD conference	
	nTemp0 = eTT_HDBitrateThrshld;
	nTemp1 = nRate;
	*pSeg << nTemp0 << nTemp1;

	CManagerApi apiSnmp(eProcessSNMPProcess);
	apiSnmp.SendMsg(pSeg, SNMP_UPDATE_MULTIPLE_TELEMETRY_DATA_IND);
	
	StartTimer(GET_CONF_INFO_TIMER, GET_CONF_INFO_TIMER_TIME_OUT_VALUE);
}


STATUS CConfPartyMonitor::OnServerGetConversionStatus(CRequest* pGetRequest)
{
    STATUS status = STATUS_OK;

	CIVRServiceGetConversionStatus * pGetConversionStatus = new CIVRServiceGetConversionStatus;

	*pGetConversionStatus = * (CIVRServiceGetConversionStatus*)pGetRequest->GetRequestObject();

	pGetConversionStatus->SetConnectId(pGetRequest->GetConnectId());
	pGetRequest->SetConfirmObject(pGetConversionStatus);
	pGetRequest->SetStatus(status);

 	return STATUS_OK;
}


